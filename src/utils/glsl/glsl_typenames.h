//
// Created by xflajs00 on 22.04.2022.
//

#pragma once

#include "glm/glm.hpp"
#include "pf_common/array.h"
#include "pf_common/concepts/OneOf.h"
#include <string_view>

namespace pf {

// FIXME: missing double types, different matrix dimensions, samplers, images, atomic counter

#define PF_GLSL_TYPES                                                                                                  \
  bool, float, unsigned int, int, glm::vec2, glm::vec3, glm::vec4, glm::ivec2, glm::ivec3, glm::ivec4, glm::bvec2,     \
      glm::bvec3, glm::bvec4, glm::uvec2, glm::uvec3, glm::uvec4, glm::mat2, glm::mat3, glm::mat4

template<OneOf<PF_GLSL_TYPES> T>
consteval std::string_view getGLSLTypeName() {
  if (std::same_as<T, bool>) { return "bool"; }
  if (std::same_as<T, float>) { return "float"; }
  if (std::same_as<T, unsigned int>) { return "uint"; }
  if (std::same_as<T, int>) { return "int"; }
  if (std::same_as<T, glm::vec2>) { return "vec2"; }
  if (std::same_as<T, glm::vec3>) { return "vec3"; }
  if (std::same_as<T, glm::vec4>) { return "vec4"; }
  if (std::same_as<T, glm::ivec2>) { return "ivec2"; }
  if (std::same_as<T, glm::ivec3>) { return "ivec3"; }
  if (std::same_as<T, glm::ivec4>) { return "ivec4"; }
  if (std::same_as<T, glm::bvec2>) { return "bvec2"; }
  if (std::same_as<T, glm::bvec3>) { return "bvec3"; }
  if (std::same_as<T, glm::bvec4>) { return "bvec4"; }
  if (std::same_as<T, glm::uvec2>) { return "uvec2"; }
  if (std::same_as<T, glm::uvec3>) { return "uvec3"; }
  if (std::same_as<T, glm::uvec4>) { return "uvec4"; }
  if (std::same_as<T, glm::mat2>) { return "mat2"; }
  if (std::same_as<T, glm::mat3>) { return "mat3"; }
  if (std::same_as<T, glm::mat4>) { return "mat4"; }
  return "<unknown>";
}

inline auto getGlslTypeNames() {
  using namespace std::string_literals;
  return make_array("bool"s, "float"s, "uint"s, "int"s, "vec2"s, "vec3"s, "vec4"s, "ivec2"s, "ivec3"s, "ivec4"s,
                    "bvec2"s, "bvec3"s, "bvec4"s, "uvec2"s, "uvec3"s, "uvec4"s, "mat2"s, "mat3"s, "mat4"s);
}

constexpr auto getTypeForGlslName(std::string_view typeName, auto &&visitor) {
  if (typeName == "bool") {
    visitor.template operator()<bool>();
  } else if (typeName == "float") {
    visitor.template operator()<float>();
  } else if (typeName == "uint") {
    visitor.template operator()<unsigned int>();
  } else if (typeName == "int") {
    visitor.template operator()<int>();
  } else if (typeName == "vec2") {
    visitor.template operator()<glm::vec2>();
  } else if (typeName == "vec3") {
    visitor.template operator()<glm::vec3>();
  } else if (typeName == "vec4") {
    visitor.template operator()<glm::vec4>();
  } else if (typeName == "ivec2") {
    visitor.template operator()<glm::ivec2>();
  } else if (typeName == "ivec3") {
    visitor.template operator()<glm::ivec3>();
  } else if (typeName == "ivec4") {
    visitor.template operator()<glm::ivec4>();
  } else if (typeName == "bvec2") {
    visitor.template operator()<glm::bvec2>();
  } else if (typeName == "bvec3") {
    visitor.template operator()<glm::bvec3>();
  } else if (typeName == "bvec4") {
    visitor.template operator()<glm::bvec4>();
  } else if (typeName == "uvec2") {
    visitor.template operator()<glm::uvec2>();
  } else if (typeName == "uvec3") {
    visitor.template operator()<glm::uvec3>();
  } else if (typeName == "uvec4") {
    visitor.template operator()<glm::uvec4>();
  } else if (typeName == "mat2") {
    visitor.template operator()<glm::mat2>();
  } else if (typeName == "mat3") {
    visitor.template operator()<glm::mat3>();
  } else if (typeName == "mat4") {
    visitor.template operator()<glm::mat4>();
  }
}

}  // namespace pf
