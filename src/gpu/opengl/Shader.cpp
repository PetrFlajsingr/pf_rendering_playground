//
// Created by xflajs00 on 07.05.2022.
//

#include "Shader.h"
#include <assert.hpp>

namespace pf::gpu {

GpuOperationResult<ShaderError> OpenGlShader::create(const SpirvCompilationResult &spirvData,
                                                     const std::string &entryPoint) {
  return createImpl([&](GLuint shaderHandle) {
    const auto dataLength =
        static_cast<GLsizei>(spirvData.spirvData.size() * sizeof(decltype(spirvData.spirvData)::value_type));
    glShaderBinary(1, &shaderHandle, GL_SHADER_BINARY_FORMAT_SPIR_V, spirvData.spirvData.data(), dataLength);
    glSpecializeShader(shaderHandle, entryPoint.c_str(), 0, nullptr, nullptr);
  });
}

GpuOperationResult<ShaderError> OpenGlShader::create(const std::string &source) {
  return createImpl([&](GLuint shaderHandle) {
    const auto sourcePtr = source.c_str();
    glShaderSource(shaderHandle, 1, &sourcePtr, nullptr);
    glCompileShader(shaderHandle);
  });
}

constexpr GLenum OpenGlShader::ShaderTypeToOpenGlConstant(ShaderType shaderType) {
  using enum ShaderType;
  switch (shaderType) {
    case Compute: return GL_COMPUTE_SHADER;
    case Vertex: return GL_VERTEX_SHADER;
    case TesselationControl: return GL_TESS_CONTROL_SHADER;
    case TesselationEvaluation: return GL_TESS_EVALUATION_SHADER;
    case Geometry: return GL_COMPUTE_SHADER;
    case Fragment: return GL_FRAGMENT_SHADER;
  }
  VERIFY(false, "This should be unreachable, since all shader types should be covered");
  return {};
}

void OpenGlShader::deleteOpenGlObject(GLuint objectHandle) const { glDeleteShader(objectHandle); }
}  // namespace pf::gpu