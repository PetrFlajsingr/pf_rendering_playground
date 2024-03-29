//
// Created by xflajs00 on 18.04.2022.
//

#include "ShaderBuilder.h"
#include <algorithm>
#include <fmt/format.h>
#include <regex>

namespace pf::shader_toy {
using namespace std::string_view_literals;
constexpr auto dimensionCheckTemplate = R"glsl(
const ivec2 _pf_generated_renderTextureSize = imageSize({});

if (gl_GlobalInvocationID.x >= _pf_generated_renderTextureSize.x) {{
    return;
}}
if (gl_GlobalInvocationID.y >= _pf_generated_renderTextureSize.y) {{
    return;
}}
)glsl"sv;

std::string ShaderBuilder::uniformsAsString(const std::vector<UniformInfo> &uniformInfos) {
  std::string result;
  std::ranges::for_each(uniformInfos, [&](const UniformInfo &uniformInfo) {
    result.append(fmt::format("layout(location = {}) uniform {} {};\n", layoutLocationCounter++, uniformInfo.type,
                              uniformInfo.varName));
  });
  return result;
}

std::string ShaderBuilder::image2DsAsString(const std::vector<Image2DInfo> &image2DInfos) {
  std::string result;
  std::ranges::for_each(image2DInfos, [&](const Image2DInfo &imageInfo) {
    result.append(fmt::format("layout({}, binding = {}) uniform image2D {};\n", imageInfo.format, bindingCounter++,
                              imageInfo.name));
  });
  return result;
}

std::string ShaderBuilder::definesAsString(const std::vector<ShaderDefine> &shaderDefines) {
  std::string result;
  std::ranges::for_each(shaderDefines, [&](const ShaderDefine &defineInfo) {
    result.append(fmt::format("#define {} {}\n", defineInfo.name, defineInfo.value));
  });
  return result;
}

std::string ShaderBuilder::addTextureAccessCheck(std::string src, const std::string &textureName) {
  std::regex regex{R"(void\s+main\s*\(\s*\)\s*\{[^\n]*)",
                   std::regex_constants::ECMAScript | std::regex_constants::icase};
  std::smatch match;
  std::regex_search(src, match, regex);
  if (match.empty()) { return src; }
  const auto &lastMatch = match[match.size() - 1];
  const auto matchStr = std::string{lastMatch.first, lastMatch.second};
  src.replace(lastMatch.first, lastMatch.second, matchStr + fmt::format(dimensionCheckTemplate, textureName));
  return src;
}

ShaderBuilder &ShaderBuilder::addUniform(std::string type, std::string name) {
  uniforms.emplace_back(UniformInfo{std::move(type), std::move(name)});
  return *this;
}

ShaderBuilder &ShaderBuilder::addImage2D(std::string format, std::string name) {
  image2Ds.emplace_back(Image2DInfo{std::move(format), std::move(name)});
  return *this;
}

ShaderBuilder &ShaderBuilder::addDefine(std::string name, std::string value) {
  defines.emplace_back(ShaderDefine{std::move(name), std::move(value)});
  return *this;
}

ShaderBuilder &ShaderBuilder::setLocalGroupSize(glm::uvec2 size) {
  localGroupSize = size;
  return *this;
}

ShaderBuilder::Result ShaderBuilder::build(std::string userCode) {
  constexpr auto srcTemplate = R"glsl(#version 460 core

layout(local_size_x={}, local_size_y={})in;
{}
{}
{}
)glsl";
  const auto sourceWithoutUserCode =
      fmt::format(srcTemplate, localGroupSize.x, localGroupSize.y, definesAsString(defines), uniformsAsString(uniforms),
                  image2DsAsString(image2Ds));
  if (!image2Ds.empty()) { userCode = addTextureAccessCheck(userCode, image2Ds[0].name); }
  const auto sourceWithUserCode = sourceWithoutUserCode + userCode;
  const auto startOffset = static_cast<std::size_t>(std::ranges::count(sourceWithoutUserCode, '\n') + 1);
  const auto dimensionCheckPos = sourceWithUserCode.find("_pf_generated_renderTextureSize");
  const auto dimensionCheckLine = static_cast<std::size_t>(
      dimensionCheckPos != std::string::npos
          ? std::ranges::count(sourceWithUserCode.begin(), sourceWithUserCode.begin() + dimensionCheckPos, '\n')
          : -1);
  const auto dimensionCheckLineCount = static_cast<std::size_t>(std::ranges::count(dimensionCheckTemplate, '\n'));

  Result result;
  result.sourceLineToUserSourceLine = [=](std::size_t line) {
    if (line > dimensionCheckLine) { line -= dimensionCheckLineCount; }
    line -= startOffset;
    return line + 1;
  };
  result.sourceCode = sourceWithUserCode;
  return result;
}

}  // namespace pf::shader_toy