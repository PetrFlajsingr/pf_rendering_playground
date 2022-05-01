//
// Created by xflajs00 on 18.04.2022.
//

#include "ShaderToyMode.h"
#include "ShaderBuilder.h"
#include <pf_imgui/elements/Image.h>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/split.hpp>
#include <range/v3/view/trim.hpp>
#include <glslang/Include/glslang_c_interface.h>
#include <glslang/Include/ResourceLimits.h>
#include <utils/GlslToSpirv.h>

namespace pf::shader_toy {

std::string ShaderToyMode::getName() const { return "ShaderToy"; }

void ShaderToyMode::initialize_impl(const std::shared_ptr<ui::ig::ImGuiInterface> &imguiInterface,
                                    const std::shared_ptr<glfw::Window> &window) {
  glfwWindow = window;
  ui = std::make_unique<UI>(imguiInterface, DEFAULT_SHADER_SOURCE);

  const auto updateTextureSizeFromUI = [this](auto) {
    initializeTexture({ui->outputWindow->widthCombobox->getValue(), ui->outputWindow->heightCombobox->getValue()});
  };

  ui->outputWindow->widthCombobox->addValueListener(updateTextureSizeFromUI);
  ui->outputWindow->heightCombobox->addValueListener(updateTextureSizeFromUI, true);

  ui->textInputWindow->compileButton->addClickListener([&] {
    spdlog::info("[ShaderToy] Compiling shader");
    if (auto err = compileShader(ui->textInputWindow->editor->getText()); err.has_value()) {
      spdlog::error("[ShaderToy] {}", err.value());
      std::ranges::for_each(err.value() | ranges::view::split('\n'), [&] (const auto &line) {
        auto parts = line | ranges::view::split(':') | ranges::view::transform([] (auto part) {
                       return part | ranges::to<std::string>;
                     }) | ranges::to_vector;
        if (parts.size() < 3) {
          return;
        }
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
  if (programHandle == -1) { return; }

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

  glUseProgram(programHandle);
  const auto textureSize = getTextureSize();
 /* renderProgram->use();
  outputTexture->bindImage(0);
  const auto textureSize = getTextureSize();
  // todo: change this
  if (renderProgram->isActiveUniform("time")) { renderProgram->set1f("time", timeFloat); }
  if (renderProgram->isActiveUniform("timeDelta")) { renderProgram->set1f("timeDelta", timeDeltaFloat); }
  if (renderProgram->isActiveUniform("frameNum")) { renderProgram->set1i("frameNum", frameCounter); }
  if (renderProgram->isActiveUniform("mouseState")) {
    renderProgram->set1i("mouseState", static_cast<int>(mouseState));
  }
  if (renderProgram->isActiveUniform("mousePos")) { renderProgram->set3fv("mousePos", &mousePos[0]); }
  for (const auto &[name, value] : userDefinedUniforms) {
    if (renderProgram->isActiveUniform(name)) {
      std::visit([&]<typename T>(T uniformValue) {
        if constexpr (std::same_as<T, bool>) { renderProgram->set1i(name, static_cast<int>(uniformValue)); }
        if constexpr (std::same_as<T, float>) { renderProgram->set1f(name, uniformValue); }
        if constexpr (std::same_as<T, unsigned int>) { renderProgram->set1ui(name, uniformValue); }
        if constexpr (std::same_as<T, int>) { renderProgram->set1i(name, uniformValue); }
        if constexpr (std::same_as<T, glm::vec2>) { renderProgram->set2fv(name, glm::value_ptr(uniformValue)); }
        if constexpr (std::same_as<T, glm::vec3>) { renderProgram->set3fv(name, glm::value_ptr(uniformValue)); }
        if constexpr (std::same_as<T, glm::vec4>) { renderProgram->set4fv(name, glm::value_ptr(uniformValue)); }
        if constexpr (std::same_as<T, glm::ivec2>) { renderProgram->set2iv(name, glm::value_ptr(uniformValue)); }
        if constexpr (std::same_as<T, glm::ivec3>) { renderProgram->set3iv(name, glm::value_ptr(uniformValue)); }
        if constexpr (std::same_as<T, glm::ivec4>) { renderProgram->set4iv(name, glm::value_ptr(uniformValue)); }
        if constexpr (std::same_as<T, glm::bvec2>) {
          const glm::ivec2 data = uniformValue;
          renderProgram->set3iv(name, glm::value_ptr(data));
        }
        if constexpr (std::same_as<T, glm::bvec3>) {
          const glm::ivec3 data = uniformValue;
          renderProgram->set3iv(name, glm::value_ptr(data));
        }
        if constexpr (std::same_as<T, glm::bvec4>) {
          const glm::ivec4 data = uniformValue;
          renderProgram->set4iv(name, glm::value_ptr(data));
        }
        if constexpr (std::same_as<T, glm::uvec2>) {
          renderProgram->set2uiv(name, glm::value_ptr(uniformValue));
        }
        if constexpr (std::same_as<T, glm::uvec3>) {
          renderProgram->set3uiv(name, glm::value_ptr(uniformValue));
        }
        if constexpr (std::same_as<T, glm::uvec4>) {
          renderProgram->set4uiv(name, glm::value_ptr(uniformValue));
        }
        if constexpr (std::same_as<T, glm::mat2>) {
          renderProgram->setMatrix2fv(name, glm::value_ptr(uniformValue));
        }
        if constexpr (std::same_as<T, glm::mat3>) {
          renderProgram->setMatrix3fv(name, glm::value_ptr(uniformValue));
        }
        if constexpr (std::same_as<T, glm::mat4>) {
          renderProgram->setMatrix4fv(name, glm::value_ptr(uniformValue));
        }
      }, value->data);
    }
  }
  renderProgram->dispatch(textureSize.x / COMPUTE_LOCAL_GROUP_SIZE.x, textureSize.y / COMPUTE_LOCAL_GROUP_SIZE.y);*/

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

std::optional<std::string> ShaderToyMode::compileShader(const std::string &shaderCode) {
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
  }
  // clang-format on
  const auto &[source, lineMapping] = builder.build(shaderCode);
  shaderLineMapping = lineMapping;
  spdlog::trace(source);

  const auto spirvResult = glslSourceToSpirv(source);
  if (spirvResult.has_value()) {
    spdlog::info(spirvResult->messages);
    static GLuint gl_shader = -1;
    if (gl_shader != -1) {
      glDeleteShader(gl_shader);
    }
    gl_shader = glCreateShader(GL_COMPUTE_SHADER);

    if (programHandle != -1) {
      glDeleteProgram(programHandle);
    }
    glShaderBinary(1, &gl_shader, GL_SHADER_BINARY_FORMAT_SPIR_V, spirvResult->spirvData.data(), spirvResult->spirvData.size() * sizeof(decltype(spirvResult->spirvData)::value_type));
    glSpecializeShader(gl_shader, "main", 0, 0, 0);

    int compiled = 0;
    glGetShaderiv(gl_shader, GL_COMPILE_STATUS, &compiled);
    if (compiled) {
      glAttachShader(programHandle, gl_shader);
    } else {
      spdlog::error("This is fucked up");
    }
  } else {
    spdlog::error(spirvResult.error().info);
    spdlog::debug(spirvResult.error().debugInfo);
  }



  /*auto renderShader = std::make_shared<Shader>(GL_COMPUTE_SHADER, source);
  if (!renderShader->getCompileStatus()) {
    return renderShader->getInfoLog();
  }
  renderProgram = std::make_shared<Program>(std::move(renderShader));*/
  userDefinedUniforms = ui->textInputWindow->varPanel->getValueRecords();
  return std::nullopt;
}

void ShaderToyMode::updateUI() {
  ui->outputWindow->fpsAveragePlot->addValue(fpsCounter.averageFPS());
  ui->outputWindow->fpsText->setText("FPS: {}", fpsCounter.averageFPS());
}

}  // namespace pf