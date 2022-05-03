//
// Created by xflajs00 on 03.05.2022.
//

#ifndef PF_RENDERING_PLAYGROUND_COMPUTESHADERPROGRAM_H
#define PF_RENDERING_PLAYGROUND_COMPUTESHADERPROGRAM_H

#include "spdlog/spdlog.h"
#include <glad/glad.h>
#include <pf_common/concepts/ranges.h>
#include <span>
#include <tl/expected.hpp>
#include <unordered_map>
#include <utils/glsl_typenames.h>

namespace pf {

class ComputeShaderProgram {
 public:
  using Error = std::string;
  class Uniform {
    friend class ComputeShaderProgram;

   public:
    template<OneOf<PF_GLSL_TYPES> T>
    explicit Uniform(std::string uniformName) : name(std::move(uniformName)), typeName(getGLSLTypeName<T>()) {}

   private:
    std::string name;
    std::string typeName;
    std::optional<GLint> location = std::nullopt;
  };

  [[nodiscard]] static tl::expected<ComputeShaderProgram, Error> Create(std::span<unsigned int> spirvData,
                                                                        RangeOf<Uniform> auto &&shaderUniforms);

  ComputeShaderProgram(GLuint programHandle, RangeOf<Uniform> auto &&shaderUniforms)
      : programHandle(programHandle), uniforms{std::ranges::begin(shaderUniforms), std::ranges::end(shaderUniforms)} {
    findUniformLocations();
  }
  ~ComputeShaderProgram();

 private:
  [[nodiscard]] static std::optional<GLuint> CreateShaderHandle(std::span<unsigned int> spirvData);
  [[nodiscard]] static std::optional<GLuint> CreateProgramHandle(GLuint shaderHandle);

  void findUniformLocations();

  GLuint programHandle;
  std::vector<Uniform> uniforms;
};

tl::expected<ComputeShaderProgram, ComputeShaderProgram::Error>
ComputeShaderProgram::Create(std::span<unsigned int> spirvData, RangeOf<Uniform> auto &&shaderUniforms) {
  const auto shaderHandle = CreateShaderHandle(spirvData);
  if (!shaderHandle.has_value()) { return tl::make_unexpected("Shader creation failed"); }

  const auto programHandle = CreateProgramHandle(shaderHandle.value());
  // either we need to delete shader because program is not valid or mark it for deletion when program gets deleted
  glDeleteShader(shaderHandle.value());
  if (!programHandle.has_value()) { return tl::make_unexpected("Program creation failed"); }

  return ComputeShaderProgram{
      programHandle.value(),
      std::vector<Uniform>{std::ranges::begin(shaderUniforms), std::ranges::end(shaderUniforms)}};
}

}  // namespace pf
#endif  //PF_RENDERING_PLAYGROUND_COMPUTESHADERPROGRAM_H
