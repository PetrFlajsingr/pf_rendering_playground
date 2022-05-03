//
// Created by xflajs00 on 03.05.2022.
//

#include "ComputeShaderProgram.h"

namespace pf {

ComputeShaderProgram::~ComputeShaderProgram() { glDeleteProgram(programHandle); }

std::optional<GLuint> ComputeShaderProgram::CreateShaderHandle(std::span<unsigned int> spirvData) {
  const auto shaderHandle = glCreateShader(GL_COMPUTE_SHADER);

  glShaderBinary(1, &shaderHandle, GL_SHADER_BINARY_FORMAT_SPIR_V, spirvData.data(),
                 spirvData.size() * sizeof(decltype(spirvData)::value_type));
  glSpecializeShader(shaderHandle, "main", 0, nullptr, nullptr);

  GLint compiled{};
  glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compiled);
  if (compiled == GL_FALSE) {
    int errMessageLength{};
    std::string errorMessage;
    glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &errMessageLength);
    if (errMessageLength != 0) {
      errorMessage.resize(errMessageLength);
      glGetShaderInfoLog(shaderHandle, errMessageLength, nullptr, errorMessage.data());
    }
    spdlog::error("Shader compilation failed");
    spdlog::error("Message: {}", errorMessage);
    glDeleteShader(shaderHandle);
    return std::nullopt;
  }
  return shaderHandle;
}

std::optional<GLuint> ComputeShaderProgram::CreateProgramHandle(GLuint shaderHandle) {
  const auto programHandle = glCreateProgram();
  glAttachShader(programHandle, shaderHandle);

  glLinkProgram(programHandle);

  GLint linkSuccess;
  glGetProgramiv(programHandle, GL_LINK_STATUS, &linkSuccess);
  if (linkSuccess == GL_FALSE) {
    int errMessageLength{};
    std::string errorMessage;
    glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH, &errMessageLength);
    if (errMessageLength != 0) {
      errorMessage.resize(errMessageLength);
      glGetProgramInfoLog(programHandle, errMessageLength, nullptr, errorMessage.data());
    }
    spdlog::error("Program linking failed");
    spdlog::error("Message: {}", errorMessage);
    glDeleteProgram(programHandle);
    return std::nullopt;
  }
  return programHandle;
}

void ComputeShaderProgram::findUniformLocations() {
  // TODO: infer type on its own
  std::ranges::for_each(uniforms, [this](Uniform &uniform) {
    const auto uniformLocation = glGetUniformLocation(programHandle, uniform.name.c_str());
    if (uniformLocation != -1) { uniform.location = uniformLocation; }
  });
}

}  // namespace pf