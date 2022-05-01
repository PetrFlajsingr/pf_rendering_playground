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
#include <utils/GlslToSpirv.h>

void debugOpengl(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message,
                 const void *userParam) {
  spdlog::error("{} {}, {}, {}, {}", source, type, id, severity, std::string_view{message, message + length});
}

namespace pf::shader_toy {

std::string ShaderToyMode::getName() const { return "ShaderToy"; }

void ShaderToyMode::initialize_impl(const std::shared_ptr<ui::ig::ImGuiInterface> &imguiInterface,
                                    const std::shared_ptr<glfw::Window> &window) {
  //setDefaultDebugMessage();
  //setDebugMessage(debugOpengl, nullptr);

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

  // TODO: move all this to a separate shader program class
  outputTexture->bindImage(0);
  const auto textureSize = getTextureSize();

  const auto timeLoc = glGetUniformLocation(programHandle, "time");
  if (timeLoc != -1) { glProgramUniform1f(programHandle, timeLoc, timeFloat); }
  const auto timeDeltaLoc = glGetUniformLocation(programHandle, "timeDelta");
  if (timeDeltaLoc != -1) { glProgramUniform1f(programHandle, timeDeltaLoc, timeDeltaFloat); }
  const auto frameNumLoc = glGetUniformLocation(programHandle, "frameNum");
  if (frameNumLoc != -1) { glProgramUniform1i(programHandle, frameNumLoc, frameCounter); }
  const auto mouseStateLoc = glGetUniformLocation(programHandle, "mouseState");
  if (mouseStateLoc != -1) { glProgramUniform1i(programHandle, mouseStateLoc, static_cast<int>(mouseState)); }
  const auto mousePosLos = glGetUniformLocation(programHandle, "mousePos");
  if (mousePosLos != -1) { glProgramUniform3fv(programHandle, mousePosLos, 1, glm::value_ptr(mousePos)); }

  for (const auto &[name, value] : userDefinedUniforms) {
    const auto uniformLoc = glGetUniformLocation(programHandle, name.c_str());
    if (uniformLoc != -1) {
      std::visit(
          [&]<typename T>(T uniformValue) {
            if constexpr (std::same_as<T, bool>) {
              glProgramUniform1i(programHandle, uniformLoc, static_cast<int>(uniformValue));
            }
            if constexpr (std::same_as<T, float>) { glProgramUniform1f(programHandle, uniformLoc, uniformValue); }
            if constexpr (std::same_as<T, unsigned int>) {
              glProgramUniform1ui(programHandle, uniformLoc, uniformValue);
            }
            if constexpr (std::same_as<T, int>) { glProgramUniform1i(programHandle, uniformLoc, uniformValue); }
            if constexpr (std::same_as<T, glm::vec2>) {
              glProgramUniform2fv(programHandle, uniformLoc, 1, glm::value_ptr(uniformValue));
            }
            if constexpr (std::same_as<T, glm::vec3>) {
              glProgramUniform3fv(programHandle, uniformLoc, 1, glm::value_ptr(uniformValue));
            }
            if constexpr (std::same_as<T, glm::vec4>) {
              glProgramUniform4fv(programHandle, uniformLoc, 1, glm::value_ptr(uniformValue));
            }
            if constexpr (std::same_as<T, glm::ivec2>) {
              glProgramUniform2iv(programHandle, uniformLoc, 1, glm::value_ptr(uniformValue));
            }
            if constexpr (std::same_as<T, glm::ivec3>) {
              glProgramUniform3iv(programHandle, uniformLoc, 1, glm::value_ptr(uniformValue));
            }
            if constexpr (std::same_as<T, glm::ivec4>) {
              glProgramUniform4iv(programHandle, uniformLoc, 1, glm::value_ptr(uniformValue));
            }
            if constexpr (std::same_as<T, glm::bvec2>) {
              const glm::ivec2 data = uniformValue;
              glProgramUniform3iv(programHandle, uniformLoc, 1, glm::value_ptr(data));
            }
            if constexpr (std::same_as<T, glm::bvec3>) {
              const glm::ivec3 data = uniformValue;
              glProgramUniform3iv(programHandle, uniformLoc, 1, glm::value_ptr(data));
            }
            if constexpr (std::same_as<T, glm::bvec4>) {
              const glm::ivec4 data = uniformValue;
              glProgramUniform4iv(programHandle, uniformLoc, 1, glm::value_ptr(data));
            }
            if constexpr (std::same_as<T, glm::uvec2>) {
              glProgramUniform2uiv(programHandle, uniformLoc, 1, glm::value_ptr(uniformValue));
            }
            if constexpr (std::same_as<T, glm::uvec3>) {
              glProgramUniform3uiv(programHandle, uniformLoc, 1, glm::value_ptr(uniformValue));
            }
            if constexpr (std::same_as<T, glm::uvec4>) {
              glProgramUniform4uiv(programHandle, uniformLoc, 1, glm::value_ptr(uniformValue));
            }
            if constexpr (std::same_as<T, glm::mat2>) {
              glProgramUniformMatrix2fv(programHandle, uniformLoc, 1, GL_FALSE, glm::value_ptr(uniformValue));
            }
            if constexpr (std::same_as<T, glm::mat3>) {
              glProgramUniformMatrix3fv(programHandle, uniformLoc, 1, GL_FALSE, glm::value_ptr(uniformValue));
            }
            if constexpr (std::same_as<T, glm::mat4>) {
              glProgramUniformMatrix4fv(programHandle, uniformLoc, 1, GL_FALSE, glm::value_ptr(uniformValue));
            }
          },
          value->data);
    }
  }

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
    if (gl_shader != -1) { glDeleteShader(gl_shader); }
    gl_shader = glCreateShader(GL_COMPUTE_SHADER);

    glShaderBinary(1, &gl_shader, GL_SHADER_BINARY_FORMAT_SPIR_V, spirvResult->spirvData.data(),
                   spirvResult->spirvData.size() * sizeof(decltype(spirvResult->spirvData)::value_type));
    glSpecializeShader(gl_shader, "main", 0, nullptr, nullptr);

    GLint compiled = 0;
    glGetShaderiv(gl_shader, GL_COMPILE_STATUS, &compiled);
    if (GL_FALSE != compiled) {

      if (programHandle != -1) { glDeleteProgram(programHandle); }
      programHandle = glCreateProgram();
      glAttachShader(programHandle, gl_shader);

      glLinkProgram(programHandle);

      GLint success;
      glGetProgramiv(programHandle, GL_LINK_STATUS, &success);
      if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(programHandle, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
      }
    } else {
      int tmpLen;
      glGetShaderiv(gl_shader, GL_INFO_LOG_LENGTH, &tmpLen);
      if (tmpLen != 0) {
        std::string hihi;
        hihi.resize(tmpLen);
        glGetShaderInfoLog(gl_shader, tmpLen, nullptr, hihi.data());
        spdlog::debug("{}", hihi);
      }
    }
  } else {
    spdlog::error(spirvResult.error().info);
    spdlog::debug(spirvResult.error().debugInfo);

    auto errors = spirvResult.error().getInfoRecords();
    for (SpirvErrorRecord rec : errors) {
      spdlog::debug("type {}, err {}, desc {}", magic_enum::enum_name(rec.type), rec.error, rec.errorDesc);
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