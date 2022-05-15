//
// Created by xflajs00 on 18.04.2022.
//

#include "ShaderToyMode.h"
#include "ShaderBuilder.h"
#include "gpu/opengl/Program.h"
#include "gpu/opengl/Shader.h"
#include "gpu/opengl/Texture.h"
#include "gpu/utils.h"
#include <future>
#include <glslang/Include/ResourceLimits.h>
#include <glslang/Include/glslang_c_interface.h>
#include <pf_imgui/elements/Image.h>
#include <pf_mainloop/MainLoop.h>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/split.hpp>
#include <range/v3/view/trim.hpp>
#include <utility>
#include <utils/GlslToSpirv.h>
#include <utils/opengl_utils.h>
#include <utils/profiling.h>

#include <gpu/utils.h>
#include "spdlog/sinks/stdout_color_sinks.h"
#include "log/UISink.h"

void debugOpengl(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message,
                 const void *) {
  spdlog::error("{} {}, {}, {}, {}", source, type, id, severity, std::string_view{message, message + length});
}

// TODO: refactor this
namespace pf {
class OpenGlImageLoader : public ImageLoader {
 public:
  explicit OpenGlImageLoader(std::shared_ptr<ThreadPool> threadPool) : threadPool(std::move(threadPool)) {}
  tl::expected<std::shared_ptr<Texture>, std::string> createTexture(const std::filesystem::path &imagePath) override {
    if (const auto imgSize = getTextureFileSize(imagePath); imgSize.has_value()) {
      auto texture =
          std::make_shared<OpenGlTexture>(TextureTarget::_2D, TextureFormat::RGBA8, TextureLevel{0}, imgSize.value());
      if (const auto errOpt = texture->create(); errOpt.has_value()) {
        return tl::make_unexpected(errOpt.value().message);
      }
      const auto textureData = getTextureData(imagePath);
      if (textureData.has_value()) {
        texture->set2Ddata(std::span{textureData->data(), textureData->size()}, TextureLevel{0});
      } else {
        return tl::make_unexpected(textureData.error());
      }
      return texture;
    } else {
      return tl::make_unexpected("File could not be open");
    }
  }
  void
  createTextureAsync(const std::filesystem::path &imagePath,
                     std::function<void(tl::expected<std::shared_ptr<Texture>, std::string>)> onLoadDone) override {
    // TODO: do the texture creation on rendering thread
    threadPool->enqueue([=] {
      if (const auto imgSize = getTextureFileSize(imagePath); imgSize.has_value()) {
        MainLoop::Get()->enqueue([=] {
          auto texture = std::make_shared<OpenGlTexture>(TextureTarget::_2D, TextureFormat::RGBA8, TextureLevel{0},
                                                         imgSize.value());
          if (const auto errOpt = texture->create(); errOpt.has_value()) {
            onLoadDone(tl::make_unexpected(errOpt.value().message));
          }
          threadPool->enqueue([=] {
            const auto textureData = getTextureData(imagePath);
            MainLoop::Get()->enqueue([=] {
              if (textureData.has_value()) {
                texture->set2Ddata(std::span{textureData->data(), textureData->size()}, TextureLevel{0});
                onLoadDone(texture);
              } else {
                onLoadDone(tl::make_unexpected(textureData.error()));
              }
            });
          });
        });
      } else {
        onLoadDone(tl::make_unexpected("File could not be open"));
      }
    });
  }

 private:
  std::shared_ptr<ThreadPool> threadPool;
};
}  // namespace pf

namespace pf::shader_toy {

ShaderToyMode::ShaderToyMode(std::filesystem::path resourcesPath) : configData{std::move(resourcesPath)} {}

std::string ShaderToyMode::getName() const { return "ShaderToy"; }

void ShaderToyMode::initialize_impl(const std::shared_ptr<ui::ig::ImGuiInterface> &imguiInterface,
                                    const std::shared_ptr<glfw::Window> &window,
                                    std::shared_ptr<ThreadPool> threadPool) {
  //setDefaultDebugMessage();
  //setDebugMessage(debugOpengl, nullptr);

  auto isFirstRun = true;
  if (const auto iter = config.find("initialized"); iter != config.end()) {
    isFirstRun = !iter->second.value_or(false);
  }
  config.insert_or_assign("initialized", true);
  glfwWindow = window;
  workerThreads = threadPool;

  ui = std::make_unique<UI>(imguiInterface, *window, std::make_unique<OpenGlImageLoader>(workerThreads),
                            DEFAULT_SHADER_SOURCE, configData.resourcesPath, isFirstRun);

  getLogger().sinks().emplace_back(std::make_shared<PfImguiLogSink_st>(*ui->logPanel));

  const auto updateTextureSizeFromUI = [this](auto) {
    const TextureSize textureSize{
        TextureWidth{static_cast<std::uint32_t>(ui->outputWindow->widthCombobox->getValue())},
        TextureHeight{static_cast<std::uint32_t>(ui->outputWindow->heightCombobox->getValue())}, TextureDepth{0u}};
    initializeTexture(textureSize);
  };

  ui->outputWindow->widthCombobox->addValueListener(updateTextureSizeFromUI);
  ui->outputWindow->heightCombobox->addValueListener(updateTextureSizeFromUI, true);

  ui->textInputWindow->compileButton->addClickListener([&] { compileShader(ui->textInputWindow->editor->getText()); });
  ui->textInputWindow->restartButton->addClickListener([&] {
    getLogger().info("Restarting time");
    totalTime = std::chrono::nanoseconds{0};
  });
  ui->textInputWindow->timePausedCheckbox->bind(timeCounterPaused);

  autoCompileShader = ui->textInputWindow->autoCompileCheckbox->getValue();
  ui->textInputWindow->autoCompileCheckbox->bind(autoCompileShader);

  ui->textInputWindow->varPanel->addVariablesChangedListener([this] {
    isShaderChanged = true;
    lastShaderChangeTime = std::chrono::steady_clock::now();
  });
  ui->textInputWindow->imagesPanel->addImagesChangedListener([this] {
    isShaderChanged = true;
    lastShaderChangeTime = std::chrono::steady_clock::now();
  });

  ui->textInputWindow->editor->addTextListener([this](std::string_view) {
    isShaderChanged = true;
    lastShaderChangeTime = std::chrono::steady_clock::now();
  });

  ui->textInputWindow->codeToClipboardButton->addClickListener(
      [this] { ImGui::SetClipboardText(currentShaderSrc.c_str()); });

  ui->outputWindow->image->addMousePositionListener([&](auto pos) {
    const auto size = ui->outputWindow->image->getSize();
    const auto nX = pos.x / static_cast<float>(size.width);
    const auto nY = pos.y / static_cast<float>(size.height);
    const auto result = glm::vec2{getTextureSize()} * glm::vec2{nX, nY};
    mousePos.x = result.x;
    mousePos.y = result.y;
  });

  ui->hide();
}


std::vector<std::shared_ptr<spdlog::sinks::sink>> ShaderToyMode::createLoggerSinks() {
  return std::vector<std::shared_ptr<spdlog::sinks::sink>>{std::make_shared<spdlog::sinks::stdout_color_sink_st>()};
}

void ShaderToyMode::activate_impl() {
  resetCounters();
  ui->show();
  ui->interface->setStateFromConfig();
}

void ShaderToyMode::deactivate_impl() {
  ui->interface->updateConfig();
  ui->hide();
}

void ShaderToyMode::deinitialize_impl() {
  ui = nullptr;
  TimeMeasure workerThreadWaitMeasure;
  getLogger().info("Waiting for worker threads");
  std::ranges::for_each(unfinishedWorkerTasks, &std::future<void>::wait);
  workerThreads = nullptr;
  getLogger().debug("Took {}", workerThreadWaitMeasure.getTimeElapsed());
}

void ShaderToyMode::render(std::chrono::nanoseconds timeDelta) {
  if (autoCompileShader && isShaderChanged
      && (std::chrono::steady_clock::now() - lastShaderChangeTime) > std::chrono::milliseconds{
             static_cast<int>(ui->textInputWindow->autoCompileFrequencyDrag->getValue() * 1000.f)}) {
    if (previousShaderCompilationDone) {
      getLogger().trace("Auto recompiling shader");
      compileShader(ui->textInputWindow->editor->getText());
      isShaderChanged = false;
    }
  }
  if (mainProgram == nullptr) { return; }

  auto mouseState = MouseState::None;
  if (ui->outputWindow->image->isHovered()) {
    if (glfwWindow->getLastMouseButtonState(pf::glfw::MouseButton::Left) == pf::glfw::ButtonState::Down) {
      mouseState = MouseState::LeftDown;
    } else if (glfwWindow->getLastMouseButtonState(pf::glfw::MouseButton::Right) == pf::glfw::ButtonState::Down) {
      mouseState = MouseState::RightDown;
    }
  }

  const auto timeFloat = static_cast<float>(totalTime.count()) / 1'000'000'000.0f;
  const auto timeDeltaFloat = static_cast<float>(timeDelta.count()) / 1'000'000'000.0f;

  mainProgram->setUniform("time", timeFloat);
  mainProgram->setUniform("timeDelta", timeDeltaFloat);
  mainProgram->setUniform("frameNum", frameCounter);
  mainProgram->setUniform("mouseState", static_cast<int>(mouseState));
  mainProgram->setUniform("mousePos", glm::vec3{mousePos, 0.f});

  std::ranges::for_each(userDefinedUniforms, [&](const auto &valueRecord) {
    std::visit([&]<typename T>(T uniformValue) { mainProgram->setUniform(valueRecord->name, uniformValue); },
               valueRecord->data);
  });

  mainProgram->getUniformValue(
      "outImage",
      Visitor{[&](int binding) { outputTexture->bindImage(Binding{binding}, ImageTextureUnitAccess::ReadWrite); },
              [](auto) {}});

  std::ranges::for_each(userDefinedTextures, [&](const auto &textureRecord) {
    const auto &[name, texture] = textureRecord;
    mainProgram->getUniformValue(
        name,
        Visitor{[&](int binding) { texture->bindImage(Binding{binding}, ImageTextureUnitAccess::ReadWrite); },
                [](auto) {}});
  });

  mainProgram->use();

  const auto textureSize = getTextureSize();
  mainProgram->dispatch(textureSize.x / COMPUTE_LOCAL_GROUP_SIZE.x, textureSize.y / COMPUTE_LOCAL_GROUP_SIZE.y);

  fpsCounter.onFrame();
  updateUI();
  if (!timeCounterPaused) { totalTime += timeDelta; }
  ++frameCounter;
}

void ShaderToyMode::resetCounters() {
  frameCounter = 0;
  totalTime = std::chrono::nanoseconds{0};
}

void ShaderToyMode::initializeTexture(TextureSize textureSize) {
  getLogger().info("Updating texture size to {}x{}", textureSize.width.get(), textureSize.height.get());
  outputTexture =
      std::make_shared<OpenGlTexture>(TextureTarget::_2D, TextureFormat::RGBA32F, TextureLevel{0}, textureSize);
  if (const auto err = outputTexture->create(); err.has_value()) {
    // can't happen for now
  }
  getLogger().debug("Texture created: {}", *outputTexture);
  outputTexture->setParam(TextureMinificationFilter::Linear);
  outputTexture->setParam(TextureMagnificationFilter::Linear);

  ui->outputWindow->image->setTextureId(getImTextureID(*outputTexture));
}

glm::uvec2 ShaderToyMode::getTextureSize() const {
  return {outputTexture->getSize().width.get(), outputTexture->getSize().height.get()};
}

void ShaderToyMode::compileShader(const std::string &shaderCode) {
  ui->textInputWindow->compilationSpinner->setVisibility(ui::ig::Visibility::Visible);
  getLogger().info("Compiling shader");
  compileShader_impl(ui->textInputWindow->editor->getText());
}

// TODO: clean this up
void ShaderToyMode::compileShader_impl(const std::string &shaderCode) {
  // clang-format off
  auto builder = ShaderBuilder{};
  builder.addUniform<float>("time")
      .addUniform<float>("timeDelta")
      .addUniform<int>("frameNum")
      .addEnum<MouseState>()
      .addUniform<MouseState>("mouseState")
      .addUniform<glm::vec3>("mousePos")
      .addImage2D("rgba32f", "outImage")
      .setLocalGroupSize(COMPUTE_LOCAL_GROUP_SIZE);
  for (const auto &valueRecord : ui->textInputWindow->varPanel->getValueRecords()) {
    builder.addUniform(valueRecord->typeName, valueRecord->name);
  }
  for (const auto &[name, texture] : ui->textInputWindow->imagesPanel->getTextures()) {
    builder.addImage2D("rgba8", name);
  }
  // clang-format on
  const auto &[source, lineMapping] = builder.build(shaderCode);
  shaderLineMapping = lineMapping;

  previousShaderCompilationDone = false;
  unfinishedWorkerTasks.emplace_back(workerThreads->enqueue([=, this]() mutable {
    const auto compilationStartTime = std::chrono::steady_clock::now();
    auto spirvResult = glslComputeShaderSourceToSpirv(source);
    const auto compilationDuration = std::chrono::steady_clock::now() - compilationStartTime;
    getLogger().debug("Compilation took {}",
                      std::chrono::duration_cast<std::chrono::milliseconds>(compilationDuration));
    pf::MainLoop::Get()->enqueue([spirvResult = std::move(spirvResult), source = std::move(source), this]() mutable {
      auto onDone = RAII{[this] {
        previousShaderCompilationDone = true;
        ui->textInputWindow->compilationSpinner->setVisibility(ui::ig::Visibility::Invisible);
      }};
      ui->textInputWindow->editor->clearWarningMarkers();
      ui->textInputWindow->editor->clearErrorMarkers();
      if (spirvResult.has_value()) {
        auto shader = std::make_shared<OpenGlShader>();
        const auto shaderCreateResult = shader->create(spirvResult.value(), "main");
        if (shaderCreateResult.has_value()) {
          getLogger().error("Shader creation failed:");
          getLogger().error("{}", shaderCreateResult.value().message);
        } else {
          auto newProgram = std::make_unique<OpenGlProgram>(std::move(shader));
          const auto programCreateResult = newProgram->create();
          if (programCreateResult.has_value()) {
            getLogger().error("Program creation failed:");
            getLogger().error("{}", programCreateResult.value().message);
          } else {
            mainProgram = std::move(newProgram);

            userDefinedUniforms = ui->textInputWindow->varPanel->getValueRecords();
            userDefinedTextures = ui->textInputWindow->imagesPanel->getTextures();

            totalTime = std::chrono::nanoseconds{0};
            frameCounter = 0;
            currentShaderSrc = std::move(source);
            getLogger().info("Compiling program success");
          }
        }
      } else {
        getLogger().info("Compiling shader failed");
        auto errors = spirvResult.error().getInfoRecords();
        for (SpirvErrorRecord rec : errors) {
          using enum SpirvErrorRecord::Type;
          if (!rec.line.has_value()) { continue; }
          const auto errMessage = fmt::format("{}: {}", rec.error, rec.errorDesc);
          const auto marker =
              ui::ig::TextEditorMarker{static_cast<uint32_t>(shaderLineMapping(rec.line.value())), errMessage};
          getLogger().error("{}", errMessage);
          switch (rec.type) {
            case Warning: ui->textInputWindow->editor->addWarningMarker(marker); break;
            case Error: ui->textInputWindow->editor->addErrorMarker(marker); break;
          }
        }
      }
    });
  }));
}

void ShaderToyMode::updateUI() {
  ui->outputWindow->fpsAveragePlot->addValue(fpsCounter.averageFPS());
  if (std::chrono::steady_clock::now() - lastFPSVisualUpdate > FPSVisualUpdateFrequency) {
    lastFPSVisualUpdate = std::chrono::steady_clock::now();
    ui->outputWindow->fpsText->setText("FPS: {}", fpsCounter.averageFPS());
  }
}

}  // namespace pf::shader_toy