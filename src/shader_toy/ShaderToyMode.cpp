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
#include <pf_imgui/elements/Image.h>
#include <pf_mainloop/MainLoop.h>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/split.hpp>
#include <range/v3/view/trim.hpp>
#include <utility>
#include <utils/GlslToSpirv.h>
#include <utils/opengl_utils.h>

void debugOpengl(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message,
                 const void *) {
  spdlog::error("{} {}, {}, {}, {}", source, type, id, severity, std::string_view{message, message + length});
}

namespace pf::shader_toy {

ShaderToyMode::ShaderToyMode(std::filesystem::path resourcesPath) : configData{std::move(resourcesPath)} {}

std::string ShaderToyMode::getName() const { return "ShaderToy"; }

void ShaderToyMode::initialize_impl(const std::shared_ptr<ui::ig::ImGuiInterface> &imguiInterface,
                                    const std::shared_ptr<glfw::Window> &window) {
  //setDefaultDebugMessage();
  //setDebugMessage(debugOpengl, nullptr);

  auto isFirstRun = true;
  if (const auto iter = config.find("initialized"); iter != config.end()) {
    isFirstRun = !iter->second.value_or(false);
  }
  config.insert_or_assign("initialized", true);

  glfwWindow = window;
  ui = std::make_unique<UI>(imguiInterface, *window, DEFAULT_SHADER_SOURCE, configData.resourcesPath, isFirstRun);

  const auto updateTextureSizeFromUI = [this](auto) {
    TextureSize textureSize;
    textureSize.width = TextureWidth{static_cast<std::uint32_t>(ui->outputWindow->widthCombobox->getValue())};
    textureSize.height = TextureHeight{static_cast<std::uint32_t>(ui->outputWindow->heightCombobox->getValue())};
    textureSize.depth = TextureDepth{0u};
    initializeTexture(textureSize);
  };

  ui->outputWindow->widthCombobox->addValueListener(updateTextureSizeFromUI);
  ui->outputWindow->heightCombobox->addValueListener(updateTextureSizeFromUI, true);

  ui->textInputWindow->compileButton->addClickListener([&] { compileShader(ui->textInputWindow->editor->getText()); });
  ui->textInputWindow->restartButton->addClickListener([&] {
    spdlog::info("[ShaderToy] Restarting time");
    totalTime = std::chrono::nanoseconds{0};
  });
  ui->textInputWindow->timePausedCheckbox->bind(timeCounterPaused);

  autoCompileShader = ui->textInputWindow->autoCompileCheckbox->getValue();
  ui->textInputWindow->autoCompileCheckbox->bind(autoCompileShader);

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
  if (shaderCompilationFuture.valid()) {
    spdlog::info("[ShaderToy] Waiting for async shader compilation");
    shaderCompilationFuture.wait();
  }
}

void ShaderToyMode::render(std::chrono::nanoseconds timeDelta) {
  if (autoCompileShader && isShaderChanged
      && (std::chrono::steady_clock::now() - lastShaderChangeTime) > std::chrono::milliseconds{
             static_cast<int>(ui->textInputWindow->autoCompileFrequencyDrag->getValue() * 1000.f)}) {
    if (previousShaderCompilationDone) {
      spdlog::trace("[ShaderToy] Auto recompiling shader");
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

  for (const auto &valueRecord : userDefinedUniforms) {
    std::visit([&]<typename T>(T uniformValue) { mainProgram->setUniform(valueRecord->name, uniformValue); },
               valueRecord->data);
  }

  mainProgram->use();

  outputTexture->bindImage(Binding{0}, ImageTextureUnitAccess::ReadWrite);

  const auto textureSize = getTextureSize();
  mainProgram->dispatch(textureSize.x / COMPUTE_LOCAL_GROUP_SIZE.x, textureSize.y / COMPUTE_LOCAL_GROUP_SIZE.y);

  fpsCounter.onFrame();
  updateUI();
  if (!timeCounterPaused) { totalTime += timeDelta; }
}

void ShaderToyMode::resetCounters() {
  frameCounter = 0;
  totalTime = std::chrono::nanoseconds{0};
}

void ShaderToyMode::initializeTexture(TextureSize textureSize) {
  ui->textInputWindow->imagesPanel->clearImageTiles();
  spdlog::info("[ShaderToy] Updating texture size to {}x{}", textureSize.width.get(), textureSize.height.get());
  outputTexture =
      std::make_shared<OpenGlTexture>(TextureTarget::_2D, TextureFormat::RGBA32F, TextureLevel{0}, textureSize);
  if (const auto err = outputTexture->create(); err.has_value()) {
    // can't happen for now
  }
  spdlog::debug("[ShaderToy] Texture created: {}", *outputTexture);
  outputTexture->setParam(TextureMinificationFilter::Linear);
  outputTexture->setParam(TextureMagnificationFilter::Linear);

  ui->outputWindow->image->setTextureId(getImTextureID(*outputTexture));

  for (auto i : std::views::iota(0, 100)) ui->textInputWindow->imagesPanel->addImageTile(outputTexture, "Texture");
}

glm::uvec2 ShaderToyMode::getTextureSize() const {
  return {outputTexture->getSize().width.get(), outputTexture->getSize().height.get()};
}

void ShaderToyMode::compileShader(const std::string &shaderCode) {
  ui->textInputWindow->compilationSpinner->setVisibility(ui::ig::Visibility::Visible);
  spdlog::info("[ShaderToy] Compiling shader");
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
      .addImage2D("rgba32f", 0, "outImage")
      .setLocalGroupSize(COMPUTE_LOCAL_GROUP_SIZE);
  for (const auto &valueRecord : ui->textInputWindow->varPanel->getValueRecords()) {
    builder.addUniform(valueRecord->typeName, valueRecord->name);
  }
  // clang-format on
  const auto &[source, lineMapping] = builder.build(shaderCode);
  shaderLineMapping = lineMapping;

  previousShaderCompilationDone = false;
  shaderCompilationFuture = std::async(std::launch::async, [=, this]() mutable {
    const auto compilationStartTime = std::chrono::steady_clock::now();
    auto spirvResult = glslComputeShaderSourceToSpirv(source);
    const auto compilationDuration = std::chrono::steady_clock::now() - compilationStartTime;
    spdlog::debug("[ShaderToy] Compilation took {}",
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
          spdlog::error("[ShaderToy] Shader creation failed:");
          spdlog::error("[ShaderToy] \t{}", shaderCreateResult.value().message);
        } else {
          auto newProgram = std::make_unique<OpenGlProgram>(std::move(shader));
          const auto programCreateResult = newProgram->create();
          if (programCreateResult.has_value()) {
            spdlog::error("[ShaderToy] Program creation failed:");
            spdlog::error("[ShaderToy] \t{}", programCreateResult.value().message);
          } else {
            mainProgram = std::move(newProgram);

            userDefinedUniforms = ui->textInputWindow->varPanel->getValueRecords();

            totalTime = std::chrono::nanoseconds{0};
            frameCounter = 0;
            currentShaderSrc = std::move(source);
            spdlog::info("[ShaderToy] Compiling program success");
          }
        }
      } else {
        spdlog::info("[ShaderToy] Compiling shader failed");
        auto errors = spirvResult.error().getInfoRecords();
        for (SpirvErrorRecord rec : errors) {
          using enum SpirvErrorRecord::Type;
          if (!rec.line.has_value()) { continue; }
          const auto errMessage = fmt::format("{}: {}", rec.error, rec.errorDesc);
          const auto marker =
              ui::ig::TextEditorMarker{static_cast<uint32_t>(shaderLineMapping(rec.line.value())), errMessage};
          spdlog::error("[ShaderToy] {}", errMessage);
          switch (rec.type) {
            case Warning: ui->textInputWindow->editor->addWarningMarker(marker); break;
            case Error: ui->textInputWindow->editor->addErrorMarker(marker); break;
          }
        }
      }
    });
  });
}

void ShaderToyMode::updateUI() {
  ui->outputWindow->fpsAveragePlot->addValue(fpsCounter.averageFPS());
  if (std::chrono::steady_clock::now() - lastFPSVisualUpdate > FPSVisualUpdateFrequency) {
    lastFPSVisualUpdate = std::chrono::steady_clock::now();
    ui->outputWindow->fpsText->setText("FPS: {}", fpsCounter.averageFPS());
  }
}

}  // namespace pf::shader_toy