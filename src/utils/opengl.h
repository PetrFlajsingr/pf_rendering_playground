//
// Created by xflajs00 on 03.05.2022.
//

#ifndef PF_RENDERING_PLAYGROUND_OPENGL_H
#define PF_RENDERING_PLAYGROUND_OPENGL_H

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

}  // namespace pf

#endif  //PF_RENDERING_PLAYGROUND_OPENGL_H
