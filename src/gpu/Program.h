//
// Created by xflajs00 on 07.05.2022.
//

#pragma once

#include "GpuObject.h"
#include "Shader.h"
#include "Types.h"
#include "glad/glad.h"
#include <NamedType/named_type.hpp>
#include <memory>
#include <pf_common/concepts/ranges.h>
#include <utility>
#include <variant>
#include <vector>

namespace pf {

using UniformLocation = fluent::NamedType<std::uint32_t, struct UniformLocationTag>;
using AttributeLocation = fluent::NamedType<std::uint32_t, struct AttributeLocationTag>;
using Binding = fluent::NamedType<std::int32_t, struct BindingTag>;

struct UniformInfo {
  inline UniformInfo(UniformLocation location, ShaderValueType type, std::string name, uint32_t size)
      : location(location), type(type), name(std::move(name)), size(size) {}
  UniformLocation location;
  ShaderValueType type;
  std::string name;
  std::uint32_t size;
};
struct AttributeInfo {
  inline AttributeInfo(AttributeLocation location, ShaderValueType type, std::string name, uint32_t size)
      : location(location), type(type), name(std::move(name)), size(size) {}
  AttributeLocation location;
  ShaderValueType type;
  std::string name;
  std::uint32_t size;
};
struct BufferInfo {
  inline BufferInfo(std::string name, Binding binding, uint32_t size, uint32_t varCount,
                    Flags<ShaderType> activeShaders)
      : name(std::move(name)), binding(binding), size(size), varCount(varCount), activeShaders(activeShaders) {}
  std::string name;
  Binding binding;
  std::uint32_t size;
  std::uint32_t varCount;
  Flags<ShaderType> activeShaders;
};

enum class ProgramError { Link, UniformNotFound, InvalidOperationForType, WrongUniformType };
// TODO: buffer and attribute stuff
class Program : public GpuObject {
 public:
  PF_GPU_OBJECT_TYPE(GpuObject::Type::Program)

  explicit Program(RangeOf<std::shared_ptr<Shader>> auto &&programShaders)
      : shaders(std::ranges::begin(programShaders), std::ranges::end(programShaders)) {}
  explicit Program(std::shared_ptr<Shader> shader) : shaders{shader} {}

  [[nodiscard]] GpuOperationResult<ProgramError> create();

  [[nodiscard]] const std::vector<UniformInfo> &getUniforms() const;
  [[nodiscard]] ExpectedGpuOperationResult<ShaderValueType, ProgramError> getUniformType(const std::string &name) const;
  [[nodiscard]] GpuOperationResult<ProgramError> getUniformValue(const std::string &name, auto &&visitor) {
    if (const auto infoOpt = findUniformInfo(name); infoOpt.has_value()) {
      const auto uniformValue = getUniformValueImpl(*infoOpt.value());
      std::visit(visitor, uniformValue);
      return std::nullopt;
    }
    return GpuError{ProgramError::UniformNotFound, fmt::format("Uniform '{}' is not active.", name)};
  }

  [[nodiscard]] const std::vector<AttributeInfo> &getAttributes() const;
  [[nodiscard]] const std::vector<BufferInfo> &getBuffers() const;

  [[nodiscard]] GpuOperationResult<ProgramError> setUniform(const std::string &name, OneOf<PF_SHADER_VALUE_TYPES> auto value);

  void use();

  [[nodiscard]] GpuOperationResult<ProgramError> dispatch(std::uint32_t x, std::uint32_t y = 1, std::uint32_t z = 1);

  [[nodiscard]] std::string getDebugString() const override;

 protected:
  struct ProgramInfos {
    std::vector<UniformInfo> uniforms;
    std::vector<AttributeInfo> attributes;
    std::vector<BufferInfo> buffers;
  };

  [[nodiscard]] virtual GpuOperationResult<ProgramError> createImpl() = 0;
  [[nodiscard]] virtual ProgramInfos extractProgramInfos() = 0;
  virtual void useImpl() = 0;
  virtual void setUniformImpl(UniformLocation location, std::variant<PF_SHADER_VALUE_TYPES> value) = 0;
  [[nodiscard]] virtual std::variant<PF_SHADER_VALUE_TYPES> getUniformValueImpl(const UniformInfo &info) = 0;
  virtual void dispatchImpl(std::uint32_t x, std::uint32_t y, std::uint32_t z) = 0;

  [[nodiscard]] std::optional<const UniformInfo *> findUniformInfo(const std::string &name) const;

  std::vector<std::shared_ptr<Shader>> shaders;

  ProgramInfos infos;
};

GpuOperationResult<ProgramError> Program::setUniform(const std::string &name, OneOf<PF_SHADER_VALUE_TYPES> auto value) {
  if (const auto uniformInfo = findUniformInfo(name); uniformInfo.has_value()) {
    if (!getShaderValueTypeForType<decltype(value)>().is(uniformInfo.value()->type)) {
      return GpuError{ProgramError::WrongUniformType,
                      fmt::format("Uniform is of type '{}'.", magic_enum::enum_name(uniformInfo.value()->type))};
    }
    setUniformImpl(uniformInfo.value()->location, value);
    return std::nullopt;
  }
  return GpuError{ProgramError::UniformNotFound, fmt::format("Uniform '{}' is not active.", name)};
}

}  // namespace pf