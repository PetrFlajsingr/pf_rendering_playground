//
// Created by xflajs00 on 18.04.2022.
//

#ifndef PF_RENDERING_PLAYGROUND_SHADERTOYSHADERBUILDER_H
#define PF_RENDERING_PLAYGROUND_SHADERTOYSHADERBUILDER_H

#include <glm/glm.hpp>
#include <pf_common/concepts/OneOf.h>
#include <string>
#include <vector>

namespace pf {

#define GLSL_TYPES bool, float, unsigned int, int, glm::vec2, glm::vec3, glm::vec4, glm::ivec2, glm::ivec3, glm::ivec4, glm::bvec2, glm::bvec3, glm::bvec4, glm::uvec2, glm::uvec3, glm::uvec4, glm::mat2, glm::mat3, glm::mat4

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

class ShaderToyShaderBuilder {
 public:
  template<OneOf<GLSL_TYPES> T>
  ShaderToyShaderBuilder &addUniform(std::string name);
  ShaderToyShaderBuilder &addUniform(std::string type, std::string name);

  ShaderToyShaderBuilder &addImage2D(std::string format, std::uint32_t binding, std::string name);

  ShaderToyShaderBuilder &addDefine(std::string name, std::string value = "");

  ShaderToyShaderBuilder &setLocalGroupSize(glm::uvec2 size);

  [[nodiscard]] std::string build(std::string userCode);

 private:
  std::string uniformsAsString(const std::vector<UniformInfo> &uniforms);
  std::string image2DsAsString(const std::vector<Image2DInfo> &uniforms);
  std::string definesAsString(const std::vector<ShaderDefine> &defines);

  std::string addTextureAccessCheck(std::string src, const std::string &textureName);

  std::vector<UniformInfo> uniforms;
  std::vector<Image2DInfo> image2Ds;
  std::vector<ShaderDefine> defines;

  glm::uvec2 localGroupSize;
};

template<OneOf<GLSL_TYPES> T>
consteval const char *getGLSLTypeName() {
  if (std::same_as<T, bool>) {
    return "bool";
  }
  if (std::same_as<T, float>) {
    return "float";
  }
  if (std::same_as<T, unsigned int>) {
    return "uint";
  }
  if (std::same_as<T, int>) {
    return "int";
  }
  if (std::same_as<T, glm::vec2>) {
    return "vec2";
  }
  if (std::same_as<T, glm::vec3>) {
    return "vec3";
  }
  if (std::same_as<T, glm::vec4>) {
    return "vec4";
  }
  if (std::same_as<T, glm::ivec2>) {
    return "ivec2";
  }
  if (std::same_as<T, glm::ivec3>) {
    return "ivec3";
  }
  if (std::same_as<T, glm::ivec4>) {
    return "ivec4";
  }
  if (std::same_as<T, glm::bvec2>) {
    return "bvec2";
  }
  if (std::same_as<T, glm::bvec3>) {
    return "bvec3";
  }
  if (std::same_as<T, glm::bvec4>) {
    return "bvec4";
  }
  if (std::same_as<T, glm::uvec2>) {
    return "uvec2";
  }
  if (std::same_as<T, glm::uvec3>) {
    return "uvec3";
  }
  if (std::same_as<T, glm::uvec4>) {
    return "uvec4";
  }
  if (std::same_as<T, glm::mat2>) {
    return "mat2";
  }
  if (std::same_as<T, glm::mat3>) {
    return "mat3";
  }
  if (std::same_as<T, glm::mat4>) {
    return "mat4";
  }
  return "<unknown>";
}

template<OneOf<GLSL_TYPES> T>
ShaderToyShaderBuilder &ShaderToyShaderBuilder::addUniform(std::string name) {
  const auto typeName = getGLSLTypeName<T>();
  uniforms.emplace_back(typeName, std::move(name));
  return *this;
}

}// namespace pf

#undef GLSL_TYPES
#endif//PF_RENDERING_PLAYGROUND_SHADERTOYSHADERBUILDER_H
