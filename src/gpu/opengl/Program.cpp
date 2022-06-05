//
// Created by xflajs00 on 07.05.2022.
//

#include "Program.h"
#include "spdlog/spdlog.h"
#include "utils/opengl_utils.h"
#include <assert.hpp>

namespace pf::gpu {

OpenGlProgram::OpenGlProgram(std::shared_ptr<Shader> shader) : Program(std::move(shader)) {}

GpuOperationResult<ProgramError> OpenGlProgram::createImpl() {
  const auto programHandle = glCreateProgram();

  std::ranges::for_each(shaders, [&](const auto &shader) {
    const auto openGlShader = shader->as<OpenGlShader>();
    DEBUG_ASSERT(openGlShader.has_value());
    glAttachShader(programHandle, openGlShader.value()->getHandle());
  });

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
    glDeleteProgram(programHandle);
    return GpuError{ProgramError::Link, errorMessage};
  }
  handle = programHandle;
  return std::nullopt;
}

Program::ProgramInfos OpenGlProgram::extractProgramInfos() {
  return {extractUniforms(), extractAttributes(), extractBuffers()};
}

std::vector<UniformInfo> OpenGlProgram::extractUniforms() {
  std::vector<UniformInfo> result{};
  GLint uniformCount;
  glGetProgramiv(*handle, GL_ACTIVE_UNIFORMS, &uniformCount);
  result.reserve(uniformCount);
  GLint uniformMaxLength;
  glGetProgramiv(*handle, GL_ACTIVE_UNIFORM_MAX_LENGTH, &uniformMaxLength);
  auto nameBuffer = std::make_unique<char[]>(uniformMaxLength + 1);
  for (GLint i = 0; i < uniformCount; ++i) {
    GLenum type;
    GLint size;
    GLint location;
    GLsizei length;
    glGetActiveUniform(*handle, i, uniformMaxLength, &length, &size, &type, nameBuffer.get());
    std::string name = nameBuffer.get();
    {
      std::size_t pos = name.find("[0]");
      if (pos != std::string::npos) { name = name.substr(0, pos); }
    }
    location = glGetUniformLocation(*handle, name.c_str());
    if (const auto shaderValueType = ShaderValueTypeFromGlConstant(type); shaderValueType.has_value()) {
      result.emplace_back(UniformLocation{static_cast<std::uint32_t>(location)}, shaderValueType.value(),
                          std::move(name), length);
    } else {
      DEBUG_ASSERT(false, "Came across unsupported type in shader program");
    }
  }
  return result;
}

std::vector<AttributeInfo> OpenGlProgram::extractAttributes() {
  std::vector<AttributeInfo> result{};
  GLint attribCount;
  glGetProgramiv(*handle, GL_ACTIVE_ATTRIBUTES, &attribCount);
  result.reserve(attribCount);
  GLint attribMaxLength;
  glGetProgramiv(*handle, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &attribMaxLength);
  auto nameBuffer = std::make_unique<char[]>(attribMaxLength + 1);
  for (GLint i = 0; i < attribCount; ++i) {
    GLenum type;
    GLint size;
    GLint location;
    GLsizei length;
    glGetActiveAttrib(*handle, i, attribMaxLength, &length, &size, &type, nameBuffer.get());
    std::string name = nameBuffer.get();
    {
      std::size_t pos = name.find("[0]");
      if (pos != std::string::npos) { name = name.substr(0, pos); }
    }
    location = glGetAttribLocation(*handle, name.c_str());
    if (const auto shaderValueType = ShaderValueTypeFromGlConstant(type); shaderValueType.has_value()) {
      result.emplace_back(AttributeLocation{static_cast<std::uint32_t>(location)}, shaderValueType.value(),
                          std::move(name), length);
    } else {
      DEBUG_ASSERT(false, "Came across unsupported type in shader program");
    }
  }
  return result;
}

std::vector<BufferInfo> OpenGlProgram::extractBuffers() {
  const auto getResourceParam = [&](GLenum interf, GLenum pname, GLuint index) {
    GLint param;
    glGetProgramResourceiv(*handle, interf, index, 1, &pname, 1, nullptr, &param);
    return param;
  };
  const auto getResourceName = [&](GLenum interf, GLuint index) {
    GLint maxLength;
    glGetProgramInterfaceiv(*handle, interf, GL_MAX_NAME_LENGTH, &maxLength);
    auto nameBuffer = std::make_unique<char[]>(maxLength + 1);
    glGetProgramResourceName(*handle, interf, index, maxLength, nullptr, nameBuffer.get());
    return std::string{nameBuffer.get()};
  };

  std::vector<BufferInfo> result{};
  GLint bufferCount;
  glGetProgramInterfaceiv(*handle, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &bufferCount);
  for (GLint i = 0; i < bufferCount; ++i) {
    std::string name = getResourceName(GL_SHADER_STORAGE_BLOCK, i);
    {
      std::size_t pos = name.find("[0]");
      if (pos != std::string::npos) { name = name.substr(0, pos); }
    }
    GLint binding = getResourceParam(GL_SHADER_STORAGE_BLOCK, GL_BUFFER_BINDING, i);
    GLint dataSize = getResourceParam(GL_SHADER_STORAGE_BLOCK, GL_BUFFER_DATA_SIZE, i);
    GLint varCount = getResourceParam(GL_SHADER_STORAGE_BLOCK, GL_NUM_ACTIVE_VARIABLES, i);
    Flags<ShaderType> activeShaders;
    if (getResourceParam(GL_SHADER_STORAGE_BLOCK, GL_REFERENCED_BY_VERTEX_SHADER, i) != 0) {
      activeShaders |= ShaderType::Vertex;
    }
    if (getResourceParam(GL_SHADER_STORAGE_BLOCK, GL_REFERENCED_BY_TESS_CONTROL_SHADER, i) != 0) {
      activeShaders |= ShaderType::TesselationControl;
    }
    if (getResourceParam(GL_SHADER_STORAGE_BLOCK, GL_REFERENCED_BY_TESS_EVALUATION_SHADER, i) != 0) {
      activeShaders |= ShaderType::TesselationEvaluation;
    }
    if (getResourceParam(GL_SHADER_STORAGE_BLOCK, GL_REFERENCED_BY_GEOMETRY_SHADER, i) != 0) {
      activeShaders |= ShaderType::Geometry;
    }
    if (getResourceParam(GL_SHADER_STORAGE_BLOCK, GL_REFERENCED_BY_FRAGMENT_SHADER, i) != 0) {
      activeShaders |= ShaderType::Fragment;
    }
    if (getResourceParam(GL_SHADER_STORAGE_BLOCK, GL_REFERENCED_BY_COMPUTE_SHADER, i) != 0) {
      activeShaders |= ShaderType::Compute;
    }
    result.emplace_back(std::move(name), Binding{binding}, dataSize, varCount, activeShaders);
  }
  return result;
}

constexpr std::optional<ShaderValueType> OpenGlProgram::ShaderValueTypeFromGlConstant(GLenum glEnum) {
  using enum ShaderValueType;
  if (glEnum == GL_BOOL) { return Bool; }
  if (glEnum == GL_FLOAT) { return Float; }
  if (glEnum == GL_UNSIGNED_INT) { return Uint; }
  if (glEnum == GL_INT) { return Int; }
  if (glEnum == GL_FLOAT_VEC2) { return Vec2; }
  if (glEnum == GL_FLOAT_VEC3) { return Vec3; }
  if (glEnum == GL_FLOAT_VEC4) { return Vec4; }
  if (glEnum == GL_INT_VEC2) { return Ivec2; }
  if (glEnum == GL_INT_VEC3) { return Ivec3; }
  if (glEnum == GL_INT_VEC4) { return Ivec4; }
  if (glEnum == GL_BOOL_VEC2) { return Bvec2; }
  if (glEnum == GL_BOOL_VEC3) { return Bvec3; }
  if (glEnum == GL_BOOL_VEC4) { return Bvec4; }
  if (glEnum == GL_UNSIGNED_INT_VEC2) { return Uvec2; }
  if (glEnum == GL_UNSIGNED_INT_VEC3) { return Uvec3; }
  if (glEnum == GL_UNSIGNED_INT_VEC4) { return Uvec4; }
  if (glEnum == GL_FLOAT_MAT2) { return Mat2; }
  if (glEnum == GL_FLOAT_MAT3) { return Mat3; }
  if (glEnum == GL_FLOAT_MAT4) { return Mat4; }
  if (glEnum == GL_IMAGE_2D) { return Image2D; }
  return std::nullopt;
}

void OpenGlProgram::deleteOpenGlObject(GLuint objectHandle) const { glDeleteProgram(objectHandle); }

void OpenGlProgram::useImpl() { glUseProgram(*handle); }

void OpenGlProgram::setUniformImpl(UniformLocation location, std::variant<PF_SHADER_VALUE_TYPES> value) {
  std::visit([&](auto value) { setOGLUniform(*handle, location.get(), value); }, value);
}

void OpenGlProgram::dispatchImpl(std::uint32_t x, std::uint32_t y, std::uint32_t z) { glDispatchCompute(x, y, z); }

std::variant<PF_SHADER_VALUE_TYPES> OpenGlProgram::getUniformValueImpl(const UniformInfo &info) {
  return getOGLuniform(*handle, static_cast<GLint>(info.location.get()), info.type);
}

}  // namespace pf::gpu