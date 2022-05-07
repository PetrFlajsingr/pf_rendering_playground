//
// Created by xflajs00 on 07.05.2022.
//

#pragma once

namespace pf {

#define PF_SHADER_VALUE_TYPES                                                                                          \
  bool, float, unsigned int, int, glm::vec2, glm::vec3, glm::vec4, glm::ivec2, glm::ivec3, glm::ivec4, glm::bvec2,     \
      glm::bvec3, glm::bvec4, glm::uvec2, glm::uvec3, glm::uvec4, glm::mat2, glm::mat3, glm::mat4

// TODO: add missing
enum class ShaderValueType {
  Bool,
  Float,
  Uint,
  Int,
  Vec2,
  Vec3,
  Vec4,
  Ivec2,
  Ivec3,
  Ivec4,
  Bvec2,
  Bvec3,
  Bvec4,
  Uvec2,
  Uvec3,
  Uvec4,
  Mat2,
  Mat3,
  Mat4,
  Image2D
};

constexpr auto getTypeForShaderValueType(ShaderValueType valueType, auto &&visitor) {
  using enum ShaderValueType;
  switch (valueType) {
    case Bool: visitor.template operator()<bool>(); break;
    case Float: visitor.template operator()<float>(); break;
    case Uint: visitor.template operator()<unsigned int>(); break;
    case Int: visitor.template operator()<int>(); break;
    case Vec2: visitor.template operator()<glm::vec2>(); break;
    case Vec3: visitor.template operator()<glm::vec3>(); break;
    case Vec4: visitor.template operator()<glm::vec4>(); break;
    case Ivec2: visitor.template operator()<glm::ivec2>(); break;
    case Ivec3: visitor.template operator()<glm::ivec3>(); break;
    case Ivec4: visitor.template operator()<glm::ivec4>(); break;
    case Bvec2: visitor.template operator()<glm::bvec2>(); break;
    case Bvec3: visitor.template operator()<glm::bvec3>(); break;
    case Bvec4: visitor.template operator()<glm::bvec4>(); break;
    case Uvec2: visitor.template operator()<glm::uvec2>(); break;
    case Uvec3: visitor.template operator()<glm::uvec3>(); break;
    case Uvec4: visitor.template operator()<glm::uvec4>(); break;
    case Mat2: visitor.template operator()<glm::mat2>(); break;
    case Mat3: visitor.template operator()<glm::mat3>(); break;
    case Mat4: visitor.template operator()<glm::mat4>(); break;
    case Image2D: visitor.template operator()<int>(); break;
  }
  assert(false && "this can't happen");
}

template<OneOf<PF_SHADER_VALUE_TYPES> T>
constexpr Flags<ShaderValueType> getShaderValueTypeForType() {
  if (std::same_as<T, bool>) { return ShaderValueType::Bool; }
  if (std::same_as<T, float>) { return ShaderValueType::Float; }
  if (std::same_as<T, unsigned int>) { return ShaderValueType::Uint; }
  if (std::same_as<T, int>) { return Flags{ShaderValueType::Int} | ShaderValueType::Image2D; }
  if (std::same_as<T, glm::vec2>) { return ShaderValueType::Vec2; }
  if (std::same_as<T, glm::vec3>) { return ShaderValueType::Vec3; }
  if (std::same_as<T, glm::vec4>) { return ShaderValueType::Vec4; }
  if (std::same_as<T, glm::ivec2>) { return ShaderValueType::Ivec2; }
  if (std::same_as<T, glm::ivec3>) { return ShaderValueType::Ivec3; }
  if (std::same_as<T, glm::ivec4>) { return ShaderValueType::Ivec4; }
  if (std::same_as<T, glm::bvec2>) { return ShaderValueType::Bvec2; }
  if (std::same_as<T, glm::bvec3>) { return ShaderValueType::Bvec3; }
  if (std::same_as<T, glm::bvec4>) { return ShaderValueType::Bvec4; }
  if (std::same_as<T, glm::uvec2>) { return ShaderValueType::Uvec2; }
  if (std::same_as<T, glm::uvec3>) { return ShaderValueType::Uvec3; }
  if (std::same_as<T, glm::uvec4>) { return ShaderValueType::Uvec4; }
  if (std::same_as<T, glm::mat2>) { return ShaderValueType::Mat2; }
  if (std::same_as<T, glm::mat3>) { return ShaderValueType::Mat3; }
  if (std::same_as<T, glm::mat4>) { return ShaderValueType::Mat4; }
  return Flags<ShaderValueType>{};
}

}  // namespace pf