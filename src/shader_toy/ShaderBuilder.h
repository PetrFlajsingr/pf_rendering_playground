//
// Created by xflajs00 on 18.04.2022.
//

#pragma once

#include <functional>
#include <glm/glm.hpp>
#include <pf_common/concepts/OneOf.h>
#include <pf_common/enums.h>
#include <string>
#include <utils/glsl_typenames.h>
#include <vector>

namespace pf::shader_toy {

struct UniformInfo {
  std::string type;
  std::string varName;
};

struct Image2DInfo {
  std::string format;
  std::uint32_t binding;
  std::string name;
};

struct ShaderDefine {
  std::string name;
  std::string value;
};

class ShaderBuilder {
 public:
  struct Result {
    std::string sourceCode;
    std::function<std::size_t(std::size_t)> sourceLineToUserSourceLine;
  };
  template<typename T>
    requires(OneOf<T, PF_GLSL_TYPES> || Enum<T>)
  ShaderBuilder &addUniform(std::string name);
  ShaderBuilder &addUniform(std::string type, std::string name);
  template<Enum E>
    requires(std::same_as<std::underlying_type_t<E>, int>)  // for now int only
  ShaderBuilder &addEnum();

  ShaderBuilder &addImage2D(std::string format, std::uint32_t binding, std::string name);

  ShaderBuilder &addDefine(std::string name, std::string value = "");

  ShaderBuilder &setLocalGroupSize(glm::uvec2 size);

  [[nodiscard]] Result build(std::string userCode);

 private:
  std::string uniformsAsString(const std::vector<UniformInfo> &uniforms);
  std::string image2DsAsString(const std::vector<Image2DInfo> &uniforms);
  std::string definesAsString(const std::vector<ShaderDefine> &defines);

  std::string addTextureAccessCheck(std::string src, const std::string &textureName);

  template<Enum E>
  [[nodiscard]] std::string getEnumTypeName() const;

  std::vector<UniformInfo> uniforms;
  std::vector<Image2DInfo> image2Ds;
  std::vector<ShaderDefine> defines;

  glm::uvec2 localGroupSize;

  std::size_t layoutLocationCounter{};
};

template<typename T>
  requires(OneOf<T, PF_GLSL_TYPES> || Enum<T>)
ShaderBuilder &ShaderBuilder::addUniform(std::string name) {
  if constexpr (Enum<T>) {
    uniforms.emplace_back(getEnumTypeName<T>(), std::move(name));
  } else {
    uniforms.emplace_back(std::string{getGLSLTypeName<T>()}, std::move(name));
  }
  return *this;
}

template<Enum E>
  requires(std::same_as<std::underlying_type_t<E>, int>)  // for now int only
ShaderBuilder &ShaderBuilder::addEnum() {
  const auto enumTypeName = getEnumTypeName<E>();
  addDefine(enumTypeName, "int");  //for now int only
  std::ranges::for_each(magic_enum::enum_values<E>(), [enumTypeName, this](auto enumValue) {
    auto enumValueName = std::string{magic_enum::enum_name(enumValue)};
    std::ranges::transform(enumValueName, enumValueName.begin(), ::toupper);
    addDefine(enumTypeName + "_" + enumValueName, std::to_string(static_cast<int>(enumValue)));
  });
  return *this;
}

template<Enum E>
std::string ShaderBuilder::getEnumTypeName() const {
  auto enumTypeName = std::string{magic_enum::enum_type_name<E>()};
  if (const auto lastDividerPos = enumTypeName.find_last_of("::"); lastDividerPos != std::string::npos) {
    enumTypeName =
        std::string{enumTypeName.begin() + static_cast<std::string::iterator::difference_type>(lastDividerPos) + 1,
                    enumTypeName.end()};
  }
  std::string result{};
  result.reserve(enumTypeName.size());

  auto iter1 = enumTypeName.begin();
  auto iter2 = iter1 + 1;
  for (; iter2 != enumTypeName.end(); ++iter1, ++iter2) {
    result.push_back(::toupper(*iter1));
    if (::islower(*iter1) && ::isupper(*iter2)) { result.push_back('_'); }
  }
  result.push_back(::toupper(*iter1));
  return result;
}
}  // namespace pf::shader_toy
