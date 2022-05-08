//
// Created by xflajs00 on 03.05.2022.
//

#pragma once

#include "glm/gtc/type_ptr.hpp"
#include "glsl_typenames.h"
#include <glad/glad.h>
#include <glm/glm.hpp>

namespace pf {

template<OneOf<PF_GLSL_TYPES> T>
void setOGLUniform(GLuint programHandle, GLint uniformLocation, T value) {
  if constexpr (std::same_as<T, bool>) { glProgramUniform1i(programHandle, uniformLocation, static_cast<int>(value)); }
  if constexpr (std::same_as<T, float>) { glProgramUniform1f(programHandle, uniformLocation, value); }
  if constexpr (std::same_as<T, unsigned int>) { glProgramUniform1ui(programHandle, uniformLocation, value); }
  if constexpr (std::same_as<T, int>) { glProgramUniform1i(programHandle, uniformLocation, value); }
  if constexpr (std::same_as<T, glm::vec2>) {
    glProgramUniform2fv(programHandle, uniformLocation, 1, glm::value_ptr(value));
  }
  if constexpr (std::same_as<T, glm::vec3>) {
    glProgramUniform3fv(programHandle, uniformLocation, 1, glm::value_ptr(value));
  }
  if constexpr (std::same_as<T, glm::vec4>) {
    glProgramUniform4fv(programHandle, uniformLocation, 1, glm::value_ptr(value));
  }
  if constexpr (std::same_as<T, glm::ivec2>) {
    glProgramUniform2iv(programHandle, uniformLocation, 1, glm::value_ptr(value));
  }
  if constexpr (std::same_as<T, glm::ivec3>) {
    glProgramUniform3iv(programHandle, uniformLocation, 1, glm::value_ptr(value));
  }
  if constexpr (std::same_as<T, glm::ivec4>) {
    glProgramUniform4iv(programHandle, uniformLocation, 1, glm::value_ptr(value));
  }
  if constexpr (std::same_as<T, glm::bvec2>) {
    const glm::ivec2 data = value;
    glProgramUniform3iv(programHandle, uniformLocation, 1, glm::value_ptr(data));
  }
  if constexpr (std::same_as<T, glm::bvec3>) {
    const glm::ivec3 data = value;
    glProgramUniform3iv(programHandle, uniformLocation, 1, glm::value_ptr(data));
  }
  if constexpr (std::same_as<T, glm::bvec4>) {
    const glm::ivec4 data = value;
    glProgramUniform4iv(programHandle, uniformLocation, 1, glm::value_ptr(data));
  }
  if constexpr (std::same_as<T, glm::uvec2>) {
    glProgramUniform2uiv(programHandle, uniformLocation, 1, glm::value_ptr(value));
  }
  if constexpr (std::same_as<T, glm::uvec3>) {
    glProgramUniform3uiv(programHandle, uniformLocation, 1, glm::value_ptr(value));
  }
  if constexpr (std::same_as<T, glm::uvec4>) {
    glProgramUniform4uiv(programHandle, uniformLocation, 1, glm::value_ptr(value));
  }
  if constexpr (std::same_as<T, glm::mat2>) {
    glProgramUniformMatrix2fv(programHandle, uniformLocation, 1, GL_FALSE, glm::value_ptr(value));
  }
  if constexpr (std::same_as<T, glm::mat3>) {
    glProgramUniformMatrix3fv(programHandle, uniformLocation, 1, GL_FALSE, glm::value_ptr(value));
  }
  if constexpr (std::same_as<T, glm::mat4>) {
    glProgramUniformMatrix4fv(programHandle, uniformLocation, 1, GL_FALSE, glm::value_ptr(value));
  }
}

inline std::variant<PF_SHADER_VALUE_TYPES> getOGLuniform(GLuint programHandle, GLint uniformLocation,
                                                         ShaderValueType type) {
#define GET_UNIFORM_VALUE(type, function)                                                                              \
  type result;                                                                                                         \
  function(programHandle, uniformLocation, &result)
#define GET_UNIFORM_VALUE_GLM(type, function)                                                                          \
  type result;                                                                                                         \
  function(programHandle, uniformLocation, type::length(), glm::value_ptr(result))
#define GET_UNIFORM_VALUE_GLM_MAT(type, function)                                                                      \
  type result;                                                                                                         \
  function(programHandle, uniformLocation, type::length() * type::col_type::length(), glm::value_ptr(result))
  switch (type) {
    case ShaderValueType::Bool: {
      GET_UNIFORM_VALUE(GLint, glGetUniformiv);
      return result != 0;
    }
    case ShaderValueType::Float: {
      GET_UNIFORM_VALUE(GLfloat, glGetUniformfv);
      return result;
    }
    case ShaderValueType::Uint: {
      GET_UNIFORM_VALUE(GLuint, glGetUniformuiv);
      return result;
    }
    case ShaderValueType::Int: {
      GET_UNIFORM_VALUE(GLint, glGetUniformiv);
      return result;
    }
    case ShaderValueType::Image2D: {
      GET_UNIFORM_VALUE(GLint, glGetUniformiv);
      return result;
    }
    case ShaderValueType::Vec2: {
      GET_UNIFORM_VALUE_GLM(glm::vec2, glGetnUniformfv);
      return result;
    }
    case ShaderValueType::Vec3: {
      GET_UNIFORM_VALUE_GLM(glm::vec3, glGetnUniformfv);
      return result;
    }
    case ShaderValueType::Vec4: {
      GET_UNIFORM_VALUE_GLM(glm::vec4, glGetnUniformfv);
      return result;
    }
    case ShaderValueType::Ivec2: {
      GET_UNIFORM_VALUE_GLM(glm::ivec2, glGetnUniformiv);
      return result;
    }
    case ShaderValueType::Ivec3: {
      GET_UNIFORM_VALUE_GLM(glm::ivec3, glGetnUniformiv);
      return result;
    }
    case ShaderValueType::Ivec4: {
      GET_UNIFORM_VALUE_GLM(glm::ivec4, glGetnUniformiv);
      return result;
    }
    case ShaderValueType::Bvec2: {
      GET_UNIFORM_VALUE_GLM(glm::ivec2, glGetnUniformiv);
      return glm::bvec2{result.x != 0, result.y != 0};
    }
    case ShaderValueType::Bvec3: {
      GET_UNIFORM_VALUE_GLM(glm::ivec3, glGetnUniformiv);
      return glm::bvec3{result.x != 0, result.y != 0, result.z != 0};
    }
    case ShaderValueType::Bvec4: {
      GET_UNIFORM_VALUE_GLM(glm::ivec4, glGetnUniformiv);
      return glm::bvec4{result.x != 0, result.y != 0, result.z != 0, result.w != 0};
    }
    case ShaderValueType::Uvec2: {
      GET_UNIFORM_VALUE_GLM(glm::uvec2, glGetnUniformuiv);
      return result;
    }
    case ShaderValueType::Uvec3: {
      GET_UNIFORM_VALUE_GLM(glm::uvec3, glGetnUniformuiv);
      return result;
    }
    case ShaderValueType::Uvec4: {
      GET_UNIFORM_VALUE_GLM(glm::uvec4, glGetnUniformuiv);
      return result;
    }
    case ShaderValueType::Mat2: {
      GET_UNIFORM_VALUE_GLM_MAT(glm::mat2, glGetnUniformfv);
      return result;
    }
    case ShaderValueType::Mat3: {
      GET_UNIFORM_VALUE_GLM_MAT(glm::mat3, glGetnUniformfv);
      return result;
    }
    case ShaderValueType::Mat4: {
      GET_UNIFORM_VALUE_GLM_MAT(glm::mat4, glGetnUniformfv);
      return result;
    }
  }

#undef GET_UNIFORM_VALUE
#undef GET_UNIFORM_VALUE_GLM
#undef GET_UNIFORM_VALUE_GLM_MAT
  assert(false && "This can't happen");
  return {};
}

}  // namespace pf
