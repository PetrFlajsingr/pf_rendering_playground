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

#include "log/UISink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <gpu/utils.h>

void debugOpengl(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message,
                 const void *) {
  spdlog::error("{} {}, {}, {}, {}", source, type, id, severity, std::string_view{message, message + length});
}

namespace pf::shader_toy {

ShaderToyMode::ShaderToyMode(std::filesystem::path resourcesPath) : configData{std::move(resourcesPath)} {}

std::string ShaderToyMode::getName() const { return "ShaderToy"; }

void ShaderToyMode::initialize_impl(const std::shared_ptr<ui::ig::ImGuiInterface> &imguiInterface,
                                    const std::shared_ptr<glfw::Window> &window,
                                    std::shared_ptr<ThreadPool> threadPool) {
  auto isFirstRun = true;
  if (const auto iter = config.find("initialized"); iter != config.end()) {
    getLogger().debug("First initialisation");
    isFirstRun = !iter->second.value_or(false);
  }
  config.insert_or_assign("initialized", true);
  glfwWindow = window;
  workerThreads = threadPool;

  imageLoader = std::make_shared<OpenGLStbImageLoader>(workerThreads);

  ui = std::make_unique<UI>(imguiInterface, *window, DEFAULT_SHADER_SOURCE, configData.resourcesPath, isFirstRun,
                            imageLoader);

  getLogger().sinks().emplace_back(ui->logWindowController->createSpdlogSink());

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

  const auto markShaderChanged = [this](auto...) {
    isShaderChanged = true;
    lastShaderChangeTime = std::chrono::steady_clock::now();
  };

  ui->shaderVariablesController->getModel()->variableAddedEvent.addEventListener(markShaderChanged);
  ui->shaderVariablesController->getModel()->variableRemovedEvent.addEventListener(markShaderChanged);
  if (const auto iter = config.find("shader_variables"); iter != config.end()) {
    if (const auto varsTbl = iter->second.as_table(); varsTbl != nullptr) {
      ui->shaderVariablesController->getModel()->setFromToml(*varsTbl);
    }
  }

  ui->imageAssetsController->getModel()->imageAddedEvent.addEventListener(markShaderChanged);
  ui->imageAssetsController->getModel()->imageRemovedEvent.addEventListener(markShaderChanged);
  if (const auto iter = config.find("images"); iter != config.end()) {
    if (const auto imagesTbl = iter->second.as_table(); imagesTbl != nullptr) {
      ui->imageAssetsController->getModel()->setFromToml(*imagesTbl);
    }
  }
  std::ranges::for_each(
      ui->imageAssetsController->getModel()->getTextures(), [this, imguiInterface](const auto &textureModel) {
        if (*textureModel->texture == nullptr) {
          const auto onLoadDone = [=, this](const tl::expected<std::shared_ptr<Texture>, std::string> &loadingResult) {
            MainLoop::Get()->enqueue([=, this] {
              if (loadingResult.has_value()) {
                *textureModel->texture.modify() = loadingResult.value();
              } else {
                imguiInterface->getNotificationManager()
                    .createNotification("notif_loading_err", "Texture loading failed")
                    .createChild<ui::ig::Text>("notif_txt",
                                               fmt::format("Texture loading failed: '{}'.\n{}", loadingResult.error(),
                                                           *textureModel->imagePath))
                    .setColor<ui::ig::style::ColorOf::Text>(ui::ig::Color::Red);
              }
            });
          };
          imageLoader->loadTextureAsync(*textureModel->imagePath, onLoadDone);
        }
      });

  ui->textInputWindow->editor->addTextListener(markShaderChanged);

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
  // TODO: load data from config
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
  checkShaderStatus();
  if (mainProgram == nullptr) { return; }

  const auto mouseState = getMouseState();

  const auto timeFloat = static_cast<float>(totalTime.count()) / 1'000'000'000.0f;
  const auto timeDeltaFloat = static_cast<float>(timeDelta.count()) / 1'000'000'000.0f;

  setUniforms(timeFloat, timeDeltaFloat, mouseState);
  setBindings();

  mainProgram->use();

  const auto textureSize = getTextureSize();
  const auto dispatchResult =
      mainProgram->dispatch(textureSize.x / COMPUTE_LOCAL_GROUP_SIZE.x, textureSize.y / COMPUTE_LOCAL_GROUP_SIZE.y);
  if (dispatchResult.has_value()) { getLogger().error("Program dispatch failed: '{}'", dispatchResult->message()); }
  fpsCounter.onFrame();
  updateUI();
  if (!timeCounterPaused) { totalTime += timeDelta; }
  ++frameCounter;
}

void ShaderToyMode::resetCounters() {
  getLogger().debug("Resetting frame and time counters");
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

void ShaderToyMode::checkShaderStatus() {
  if (autoCompileShader && isShaderChanged
      && (std::chrono::steady_clock::now() - lastShaderChangeTime) > std::chrono::milliseconds{
             static_cast<int>(ui->textInputWindow->autoCompileFrequencyDrag->getValue() * 1000.f)}) {
    if (previousShaderCompilationDone) {
      getLogger().trace("Auto recompiling shader");
      compileShader(ui->textInputWindow->editor->getText());
      isShaderChanged = false;
    }
  }
}

void ShaderToyMode::compileShader(const std::string &shaderCode) {
  ui->textInputWindow->compilationSpinner->setVisibility(ui::ig::Visibility::Visible);
  getLogger().trace("Compiling shader");
  compileShader_impl(shaderCode);
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
  // clang-format on

  std::ranges::for_each(ui->shaderVariablesController->getModel()->getVariables(),
                        [&](const std::shared_ptr<ShaderVariableModel> &variable) {
                          std::string_view typeName = "";
                          std::visit(
                              [&]<typename T>(T) {
                                if constexpr (std::same_as<T, ui::ig::Color>) {
                                  typeName = getGLSLTypeName<glm::vec4>();
                                } else {
                                  typeName = getGLSLTypeName<T>();
                                }
                              },
                              *variable->value);
                          builder.addUniform(std::string{typeName}, *variable->name);
                        });

  std::ranges::for_each(ui->imageAssetsController->getModel()->getTextures(),
                        [&](const std::shared_ptr<TextureAssetModel> &tex) {
                          const auto texFormat = (*tex->texture)->getFormat();
                          std::string formatStr;
                          switch (texFormat) {
                            case TextureFormat::R8: formatStr = "r8"; break;
                            case TextureFormat::RGBA8: formatStr = "rgba8"; break;
                            default:
                              getLogger().error("Unsupported image type '{}' in ShaderToyMode::compileShader_impl",
                                                magic_enum::enum_name(texFormat));
                              return;
                          }
                          builder.addImage2D(formatStr, *tex->name);
                        });

  const auto &[source, lineMapping] = builder.build(shaderCode);
  shaderLineMapping = lineMapping;

  getLogger().debug("Enqueueing shader compilation");
  previousShaderCompilationDone = false;
  unfinishedWorkerTasks.emplace_back(workerThreads->enqueue([=, this]() mutable {
    getLogger().debug("Shader compilation job started");
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
          getLogger().error("{}", shaderCreateResult.value().message());
        } else {
          auto newProgram = std::make_unique<OpenGlProgram>(std::move(shader));
          const auto programCreateResult = newProgram->create();
          if (programCreateResult.has_value()) {
            getLogger().error("Program creation failed:");
            getLogger().error("{}", programCreateResult.value().message());
          } else {
            mainProgram = std::move(newProgram);

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

MouseState ShaderToyMode::getMouseState() const {
  if (ui->outputWindow->image->isHovered()) {
    if (glfwWindow->getLastMouseButtonState(pf::glfw::MouseButton::Left) == pf::glfw::ButtonState::Down) {
      return MouseState::LeftDown;
    } else if (glfwWindow->getLastMouseButtonState(pf::glfw::MouseButton::Right) == pf::glfw::ButtonState::Down) {
      return MouseState::RightDown;
    }
  }
  return MouseState::None;
}

void ShaderToyMode::setUniforms(float timeFloat, float timeDeltaFloat, MouseState mouseState) {
  auto ignoredResult = mainProgram->setUniform("time", timeFloat);
  ignoredResult = mainProgram->setUniform("timeDelta", timeDeltaFloat);
  ignoredResult = mainProgram->setUniform("frameNum", frameCounter);
  ignoredResult = mainProgram->setUniform("mouseState", static_cast<int>(mouseState));
  ignoredResult = mainProgram->setUniform("mousePos", glm::vec3{mousePos, 0.f});

  std::ranges::for_each(ui->shaderVariablesController->getModel()->getVariables(), [&](const auto &variable) {
    std::visit(
        [&]<typename T>(T uniformValue) {
          if constexpr (std::same_as<T, ui::ig::Color>) {
            ignoredResult = mainProgram->setUniform(
                *variable->name,
                glm::vec4{uniformValue.red(), uniformValue.green(), uniformValue.blue(), uniformValue.alpha()});
          } else {
            ignoredResult = mainProgram->setUniform(*variable->name, uniformValue);
          }
        },
        *variable->value);
  });
}

void ShaderToyMode::setBindings() {
  auto ignoredResult = mainProgram->getUniformValue(
      "outImage",
      Visitor{[&](int binding) { outputTexture->bindImage(Binding{binding}, ImageTextureUnitAccess::ReadWrite); },
              [](auto) {}});
  // TODO: read only?
  std::ranges::for_each(ui->imageAssetsController->getModel()->getTextures(), [&](const auto &tex) {
    ignoredResult = mainProgram->getUniformValue(
        *tex->name,
        Visitor{[&](int binding) { (*tex->texture)->bindImage(Binding{binding}, ImageTextureUnitAccess::ReadWrite); },
                [](auto) {}});
  });
}

void ShaderToyMode::updateUI() {
  ui->outputWindow->fpsAveragePlot->addValue(fpsCounter.averageFPS());
  if (std::chrono::steady_clock::now() - lastFPSVisualUpdate > FPSVisualUpdateFrequency) {
    lastFPSVisualUpdate = std::chrono::steady_clock::now();
    ui->outputWindow->fpsText->setText("FPS: {}", fpsCounter.averageFPS());
  }
}

void ShaderToyMode::updateConfig() {
  auto shaderVarsToml = ui->shaderVariablesController->getModel()->toToml();
  config.insert_or_assign("shader_variables", std::move(shaderVarsToml));
  auto imagesToml = ui->imageAssetsController->getModel()->toToml();
  config.insert_or_assign("images", std::move(imagesToml));
}

}  // namespace pf::shader_toy