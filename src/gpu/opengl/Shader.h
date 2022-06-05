//
// Created by xflajs00 on 07.05.2022.
//

#pragma once

#include "../Shader.h"
#include "OpenGl.h"

namespace pf::gpu {

class OpenGlShader : public Shader, public OpenGlHandleOwner {
 public:
  PF_GPU_OBJECT_API(GpuApi::OpenGl)

  [[nodiscard]] GpuOperationResult<ShaderError> create(const SpirvCompilationResult &spirvData,
                                                       const std::string &entryPoint) override;
  [[nodiscard]] GpuOperationResult<ShaderError> create(const std::string &source) override;

  [[nodiscard]] constexpr static GLenum ShaderTypeToOpenGlConstant(ShaderType shaderType);

 protected:
  void deleteOpenGlObject(GLuint objectHandle) const override;

 private:
  [[nodiscard]] GpuOperationResult<ShaderError> createImpl(std::invocable<GLuint> auto &&compileFnc);
};

GpuOperationResult<ShaderError> OpenGlShader::createImpl(std::invocable<GLuint> auto &&compileFnc) {
  const auto shaderHandle = glCreateShader(ShaderTypeToOpenGlConstant(shaderType));

  compileFnc(shaderHandle);

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
    glDeleteShader(shaderHandle);
    return GpuError{ShaderError::Compilation, errorMessage};
  }
  handle = shaderHandle;
  return std::nullopt;
}

}  // namespace pf