//
// Created by xflajs00 on 18.04.2022.
//

#include "ShaderToyShaderBuilder.h"
#include <algorithm>
#include <fmt/format.h>
#include <regex>

namespace pf {

std::string ShaderToyShaderBuilder::uniformsAsString(const std::vector<UniformInfo> &uniforms) {
  std::string result;
  std::ranges::for_each(uniforms, [&](const UniformInfo &uniformInfo) {
    result.append(fmt::format("uniform {} {};\n", uniformInfo.type, uniformInfo.varName));
  });
  return result;
}

std::string ShaderToyShaderBuilder::image2DsAsString(const std::vector<Image2DInfo> &uniforms) {
  std::string result;
  std::ranges::for_each(uniforms, [&](const Image2DInfo &imageInfo) {
    result.append(fmt::format("layout({}, binding = {}) uniform image2D {};\n", imageInfo.format, imageInfo.binding,
                              imageInfo.name));
  });
  return result;
}

std::string ShaderToyShaderBuilder::definesAsString(const std::vector<ShaderDefine> &defines) {
  std::string result;
  std::ranges::for_each(defines, [&](const ShaderDefine &defineInfo) {
    result.append(fmt::format("#define {} {}\n", defineInfo.name, defineInfo.value));
  });
  return result;
}

std::string ShaderToyShaderBuilder::addTextureAccessCheck(std::string src, const std::string &textureName) {
  constexpr auto dimensionCheckTemplate = R"glsl(
const ivec2 _pf_generated_renderTextureSize = imageSize({});

if (gl_GlobalInvocationID.x >= _pf_generated_renderTextureSize.x) {{
    return;
}}
if (gl_GlobalInvocationID.y >= _pf_generated_renderTextureSize.y) {{
    return;
}}
)glsl";
  std::regex regex{R"(void\s+main\s*\(\s*\)\s*\{)", std::regex_constants::ECMAScript | std::regex_constants::icase};
  std::smatch match;
  std::regex_search(src, match, regex);
  if (match.empty()) { return src; }
  const auto &lastMatch = match[match.size() - 1];
  const auto matchStr = std::string{lastMatch.first, lastMatch.second};
  src.replace(lastMatch.first, lastMatch.second, matchStr + fmt::format(dimensionCheckTemplate, textureName));
  return src;
}

ShaderToyShaderBuilder &ShaderToyShaderBuilder::addUniform(std::string type, std::string name) {
  uniforms.emplace_back(std::move(type), std::move(name));
  return *this;
}

ShaderToyShaderBuilder &ShaderToyShaderBuilder::addImage2D(std::string format, std::uint32_t binding,
                                                           std::string name) {
  image2Ds.emplace_back(std::move(format), binding, std::move(name));
  return *this;
}

ShaderToyShaderBuilder &ShaderToyShaderBuilder::addDefine(std::string name, std::string value) {
  defines.emplace_back(std::move(name), std::move(value));
  return *this;
}

ShaderToyShaderBuilder &ShaderToyShaderBuilder::setLocalGroupSize(glm::uvec2 size) {
  localGroupSize = size;
  return *this;
}

std::string ShaderToyShaderBuilder::build(std::string userCode) {
  if (!image2Ds.empty()) { userCode = addTextureAccessCheck(userCode, image2Ds[0].name); }
  constexpr auto srcTemplate = R"glsl(
#version 460 core

layout(local_size_x={}, local_size_y={})in;

{}

{}

{}

{}
)glsl";

  return fmt::format(srcTemplate, localGroupSize.x, localGroupSize.y, definesAsString(defines),
                     uniformsAsString(uniforms), image2DsAsString(image2Ds), userCode);
}

}  // namespace pf