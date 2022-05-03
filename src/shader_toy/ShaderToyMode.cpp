//
// Created by xflajs00 on 18.04.2022.
//

#include "ShaderToyMode.h"
#include "ShaderBuilder.h"
#include <geGL/DebugMessage.h>
#include <glslang/Include/ResourceLimits.h>
#include <glslang/Include/glslang_c_interface.h>
#include <pf_imgui/elements/Image.h>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/split.hpp>
#include <range/v3/view/trim.hpp>
#include <utility>
#include <utils/GlslToSpirv.h>
#include <utils/opengl.h>

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

  glfwWindow = window;
  ui = std::make_unique<UI>(imguiInterface, DEFAULT_SHADER_SOURCE, configData.resourcesPath);

  const auto updateTextureSizeFromUI = [this](auto) {
    initializeTexture({ui->outputWindow->widthCombobox->getValue(), ui->outputWindow->heightCombobox->getValue()});
  };

  ui->outputWindow->widthCombobox->addValueListener(updateTextureSizeFromUI);
  ui->outputWindow->heightCombobox->addValueListener(updateTextureSizeFromUI, true);

  ui->textInputWindow->compileButton->addClickListener([&] {
    spdlog::info("[ShaderToy] Compiling shader");
    if (auto err = compileShader(ui->textInputWindow->editor->getText()); err.has_value()) {
      spdlog::error("[ShaderToy] {}", err.value());
      std::ranges::for_each(err.value() | ranges::view::split('\n'), [&](const auto &line) {
        auto parts = line | ranges::view::split(':')
            | ranges::view::transform([](auto part) { return part | ranges::to<std::string>; }) | ranges::to_vector;
        if (parts.size() < 3) { return; }
        auto openBracketPos = std::ranges::find(parts[0], '(') + 1;
        auto closeBracketPos = std::ranges::find(parts[0], ')');
        int lineNum;
        std::from_chars(&*openBracketPos, &*closeBracketPos, lineNum);
        spdlog::debug("{}", shaderLineMapping(lineNum));
      });
    } else {
      totalTime = std::chrono::nanoseconds{0};
      frameCounter = 0;
      spdlog::info("[ShaderToy] Compiling shader success");
    }
  });
  ui->textInputWindow->restartButton->addClickListener([&] {
    spdlog::info("[ShaderToy] Restarting time");
    totalTime = std::chrono::nanoseconds{0};
  });
  ui->textInputWindow->timePausedCheckbox->bind(timeCounterPaused);

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

void ShaderToyMode::deinitialize_impl() {}

void ShaderToyMode::render(std::chrono::nanoseconds timeDelta) {
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

  mainProgram->setUniformValue("time", timeFloat);
  mainProgram->setUniformValue("timeDelta", timeDeltaFloat);
  mainProgram->setUniformValue("frameNum", frameCounter);
  mainProgram->setUniformValue("mouseState", static_cast<int>(mouseState));
  mainProgram->setUniformValue("mousePos", glm::vec3{mousePos, 0.f});

  for (const auto &[name, value] : userDefinedUniforms) {
    std::visit([&]<typename T>(T uniformValue) { mainProgram->setUniformValue(name, uniformValue); }, value->data);
  }

  mainProgram->activate();

  // TODO: move auto binding to mainProgram as well
  outputTexture->bindImage(0);

  const auto textureSize = getTextureSize();
  glDispatchCompute(textureSize.x / COMPUTE_LOCAL_GROUP_SIZE.x, textureSize.y / COMPUTE_LOCAL_GROUP_SIZE.y, 1);

  fpsCounter.onFrame();
  updateUI();
  if (!timeCounterPaused) { totalTime += timeDelta; }
}
void ShaderToyMode::resetCounters() {
  frameCounter = 0;
  totalTime = std::chrono::nanoseconds{0};
}

void ShaderToyMode::initializeTexture(glm::uvec2 textureSize) {
  spdlog::info("[ShaderToy] Updating texture size to {}x{}", textureSize.x, textureSize.y);
  outputTexture = std::make_shared<Texture>(GL_TEXTURE_2D, GL_RGBA32F, 0, textureSize.x, textureSize.y);
  outputTexture->texParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  outputTexture->texParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  ui->outputWindow->image->setTextureId(
      reinterpret_cast<ImTextureID>(static_cast<std::uintptr_t>(outputTexture->getId())));
}

glm::uvec2 ShaderToyMode::getTextureSize() const { return {outputTexture->getWidth(0), outputTexture->getHeight(0)}; }

// TODO: clean this up
// TODO: create a class that takes care of the compiled shader program & uniform setting etc
std::optional<std::string> ShaderToyMode::compileShader(const std::string &shaderCode) {
  ui->textInputWindow->editor->clearWarningMarkers();
  ui->textInputWindow->editor->clearErrorMarkers();
  using Uniform = ComputeShaderProgram::Uniform;
  auto computeShaderUniforms = std::vector<Uniform>{};
  computeShaderUniforms.emplace_back(Uniform::Create<float>("time"));
  computeShaderUniforms.emplace_back(Uniform::Create<float>("timeDelta"));
  computeShaderUniforms.emplace_back(Uniform::Create<int>("frameNum"));
  computeShaderUniforms.emplace_back(Uniform::Create<int>("mouseState"));
  computeShaderUniforms.emplace_back(Uniform::Create<glm::vec3>("mousePos"));
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
  for (const auto &[name, value] : ui->textInputWindow->varPanel->getValueRecords()) {
    builder.addUniform(value->typeName, name);
    getTypeForGlslName(value->typeName, [&]<typename T> {
        computeShaderUniforms.emplace_back(Uniform::Create<T>(name));
    });
  }
  // clang-format on
  const auto &[source, lineMapping] = builder.build(shaderCode);
  shaderLineMapping = lineMapping;
  spdlog::trace(source);

  const auto spirvResult = glslComputeShaderSourceToSpirv(source);
  if (spirvResult.has_value()) {
    auto newProgram =
        ComputeShaderProgram::CreateUnique(std::span{spirvResult.value().spirvData}, std::move(computeShaderUniforms));
    if (newProgram.has_value()) {
      mainProgram = std::move(newProgram.value());
    } else {
      spdlog::error("Shader creation failed:");
      spdlog::error("\t{}", newProgram.error());
    }
  } else {
    spdlog::error(spirvResult.error().info);
    spdlog::debug(spirvResult.error().debugInfo);

    auto errors = spirvResult.error().getInfoRecords();
    for (SpirvErrorRecord rec : errors) {
      using enum SpirvErrorRecord::Type;
      if (!rec.line.has_value()) { continue; }
      const auto marker = ui::ig::TextEditorMarker{static_cast<uint32_t>(shaderLineMapping(rec.line.value())),
                                                   fmt::format("{}: {}", rec.error, rec.errorDesc)};
      switch (rec.type) {
        case Warning: ui->textInputWindow->editor->addWarningMarker(marker); break;
        case Error: ui->textInputWindow->editor->addErrorMarker(marker); break;
      }
    }
  }

  userDefinedUniforms = ui->textInputWindow->varPanel->getValueRecords();
  return std::nullopt;
}

void ShaderToyMode::updateUI() {
  ui->outputWindow->fpsAveragePlot->addValue(fpsCounter.averageFPS());
  ui->outputWindow->fpsText->setText("FPS: {}", fpsCounter.averageFPS());
}

}  // namespace pf::shader_toy