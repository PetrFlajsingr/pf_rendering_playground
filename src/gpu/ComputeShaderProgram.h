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
#include <utils/opengl.h>
#include <variant>

namespace pf {

class BindingObject {
 public:
  explicit BindingObject(GLuint unit) : unit(unit) {}
  [[nodiscard]] GLuint getUnit() const { return unit; }
  virtual void bind() = 0;

 protected:
  GLuint unit;
};

class ImageBindingObject : public BindingObject {
 public:
  ImageBindingObject(GLuint unit, GLuint textureHandle, GLenum format, GLint level = 0,
                     GLboolean layered = GL_FALSE, GLenum access = GL_READ_WRITE, GLint layer = 0)
      : BindingObject(unit), textureHandle(textureHandle), level(level), format(format), access(access),
        layered(layered), layer(layer) {}
  void bind() override {
    glBindImageTexture(getUnit(), textureHandle, level, layered, layer, access, format);
  }

 private:
  GLuint textureHandle;
  GLint level;
  GLenum format;
  GLenum access;
  GLboolean layered;
  GLint layer;
};

class ComputeShaderProgram {
 public:
  using Error = std::string;
  class Uniform {
    friend class ComputeShaderProgram;

   public:
    template<OneOf<PF_GLSL_TYPES> T>
    [[nodiscard]] static Uniform Create(std::string uniformName) {
      return Uniform{std::move(uniformName), std::string{getGLSLTypeName<T>()}};
    }

   private:
    inline explicit Uniform(std::string uniformName, std::string typeName)
        : name(std::move(uniformName)), typeName(std::move(typeName)) {}
    std::string name;
    std::string typeName;  // TODO: replace internal usage of types with an enum
    std::optional<GLint> location = std::nullopt;
    std::variant<std::monostate, PF_GLSL_TYPES> newValue = std::monostate{};
    std::variant<std::monostate, PF_GLSL_TYPES> previousValue = std::monostate{};
  };
  // TODO: maybe create some CRTP interface for this
  [[nodiscard]] static tl::expected<ComputeShaderProgram, Error> Create(std::span<const unsigned int> spirvData,
                                                                        RangeOf<Uniform> auto &&shaderUniforms);
  [[nodiscard]] static tl::expected<std::unique_ptr<ComputeShaderProgram>, Error>
  CreateUnique(std::span<const unsigned int> spirvData, RangeOf<Uniform> auto &&shaderUniforms);
  [[nodiscard]] static tl::expected<std::shared_ptr<ComputeShaderProgram>, Error>
  CreateShared(std::span<const unsigned int> spirvData, RangeOf<Uniform> auto &&shaderUniforms);

  ComputeShaderProgram(GLuint programHandle, RangeOf<Uniform> auto &&shaderUniforms)
      : programHandle(programHandle), uniforms{std::ranges::begin(shaderUniforms), std::ranges::end(shaderUniforms)} {
    findUniformLocations();
  }
  ~ComputeShaderProgram();

  [[nodiscard]] bool isUniformActive(const std::string &name);
  template<OneOf<PF_GLSL_TYPES> T>
  std::optional<Error> setUniformValue(const std::string &name, T newValue);

  void setBinding(std::unique_ptr<BindingObject> binding);

  void dispatch(std::uint32_t x, std::uint32_t y = 1, std::uint32_t z = 1);

 private:
  [[nodiscard]] static tl::expected<GLuint, Error> Create_impl(std::span<const unsigned int> spirvData);

  [[nodiscard]] static std::optional<GLuint> CreateShaderHandle(std::span<const unsigned int> spirvData);
  [[nodiscard]] static std::optional<GLuint> CreateProgramHandle(GLuint shaderHandle);

  [[nodiscard]] std::optional<Uniform *> findUniformByName(const std::string &name);

  void findUniformLocations();

  GLuint programHandle;
  std::vector<Uniform> uniforms;
  std::vector<std::unique_ptr<BindingObject>> bindings;
};

tl::expected<ComputeShaderProgram, ComputeShaderProgram::Error>
ComputeShaderProgram::Create(std::span<const unsigned int> spirvData, RangeOf<Uniform> auto &&shaderUniforms) {
  const auto programHandle = Create_impl(spirvData);
  if (programHandle.has_value()) {
    return ComputeShaderProgram{
        programHandle.value(),
        std::vector<Uniform>{std::ranges::begin(shaderUniforms), std::ranges::end(shaderUniforms)}};
  } else {
    return tl::make_unexpected(programHandle.error());
  }
}
tl::expected<std::unique_ptr<ComputeShaderProgram>, ComputeShaderProgram::Error>
ComputeShaderProgram::CreateUnique(std::span<const unsigned int> spirvData, RangeOf<Uniform> auto &&shaderUniforms) {
  const auto programHandle = Create_impl(spirvData);
  if (programHandle.has_value()) {
    return std::make_unique<ComputeShaderProgram>(
        programHandle.value(),
        std::vector<Uniform>{std::ranges::begin(shaderUniforms), std::ranges::end(shaderUniforms)});
  } else {
    return tl::make_unexpected(programHandle.error());
  }
}
tl::expected<std::shared_ptr<ComputeShaderProgram>, ComputeShaderProgram::Error>
ComputeShaderProgram::CreateShared(std::span<const unsigned int> spirvData, RangeOf<Uniform> auto &&shaderUniforms) {
  const auto programHandle = Create_impl(spirvData);
  if (programHandle.has_value()) {
    return std::make_shared<ComputeShaderProgram>(
        programHandle.value(),
        std::vector<Uniform>{std::ranges::begin(shaderUniforms), std::ranges::end(shaderUniforms)});
  } else {
    return tl::make_unexpected(programHandle.error());
  }
}

template<OneOf<PF_GLSL_TYPES> T>
std::optional<ComputeShaderProgram::Error> ComputeShaderProgram::setUniformValue(const std::string &name, T newValue) {
  if (const auto uniform = findUniformByName(name); uniform.has_value()) {
    const auto uniformPtr = uniform.value();
    if (!uniformPtr->location.has_value()) {
      return fmt::format("Uniform named '{}' is not enabled (possibly optimized out)", name);
    }
    std::optional<std::string> result = std::nullopt;
    getTypeForGlslName(uniformPtr->typeName, [&]<typename U> {
      if constexpr (!std::same_as<U, T>) {
        result = fmt::format("Invalid type '{:s}' for uniform '{}'", getGLSLTypeName<T>(), name);
      } else {
        uniformPtr->newValue = newValue;
      }
    });
    return result;
  }
  return fmt::format("No uniform with name '{}' found", name);
}
}  // namespace pf
#endif  //PF_RENDERING_PLAYGROUND_COMPUTESHADERPROGRAM_H
