//
// Created by xflajs00 on 18.04.2022.
//

#include "ComputeToTextureRenderer.h"
#include <spdlog/spdlog.h>

namespace pf {

namespace details {
std::string uniformsAsString(const std::vector<UniformInfo> &uniforms) {
  std::string result;
  std::ranges::for_each(uniforms, [&](const UniformInfo &uniformInfo) {
    result.append(fmt::format("uniform {} {};\n", uniformInfo.type, uniformInfo.varName));
  });
  return result;
}

std::string image2DsAsString(const std::vector<Image2DInfo> &uniforms) {
  std::string result;
  std::ranges::for_each(uniforms, [&](const Image2DInfo &imageInfo) {
    result.append(fmt::format("layout({}, binding = {}) uniform image2D {};\n", imageInfo.format, imageInfo.binding, imageInfo.name));
  });
  return result;
}

std::string definesAsString(const std::vector<ShaderDefine> &defines) {
  std::string result;
  std::ranges::for_each(defines, [&](const ShaderDefine &defineInfo) {
    result.append(fmt::format("#define {} {}\n", defineInfo.name, defineInfo.value));
  });
  return result;
}

std::string addTextureAccessCheck(std::string src, const std::string &textureName) {
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

std::string createShader(glm::uvec2 localGroupSize, const std::vector<UniformInfo> &uniforms, const std::vector<ShaderDefine> &defines, Image2DInfo image, std::string userCode) {
  userCode = addTextureAccessCheck(userCode, image.name);
  constexpr auto srcTemplate = R"glsl(
#version 460 core

layout(local_size_x={}, local_size_y={})in;

{}

{}

{}

{}
)glsl";

  return fmt::format(srcTemplate, localGroupSize.x, localGroupSize.y, definesAsString(defines), uniformsAsString(uniforms), image2DsAsString(std::vector{image}), userCode);
}

}// namespace details

ComputeToTextureRenderer::ComputeToTextureRenderer() {
  outputTexture = std::make_shared<Texture>(GL_TEXTURE_2D, GL_RGBA32F, 0, DEFAULT_TEXTURE_SIZE.x, DEFAULT_TEXTURE_SIZE.y);
  outputTexture->texParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  outputTexture->texParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void ComputeToTextureRenderer::setTextureSize(glm::uvec2 textureSize) {
  outputTexture = std::make_shared<Texture>(GL_TEXTURE_2D, GL_RGBA32F, 0, textureSize.x, textureSize.y);
  outputTexture->texParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  outputTexture->texParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

glm::uvec2 ComputeToTextureRenderer::getTextureSize() const {
  return {outputTexture->getWidth(0), outputTexture->getHeight(0)};
}

std::optional<std::string> ComputeToTextureRenderer::setShader(std::string_view shaderSource) {
  const auto source = details::createShader(COMPUTE_LOCAL_GROUP_SIZE,
                                            {{"float", "time"}, {"float", "timeDelta"}, {"int", "frameNum"}, {"MOUSE_STATE", "mouseState"}, {"vec3", "mousePos"}},
                                            {{"MOUSE_STATE", "int"} ,{"MOUSE_LEFT_DOWN", "1"}, {"MOUSE_RIGHT_DOWN", "2"}},
                                            {"rgba32f", 0, "outImage"}, std::string{shaderSource});
  spdlog::trace(source);
  auto renderShader = std::make_shared<Shader>(GL_COMPUTE_SHADER, source);
  if (!renderShader->getCompileStatus()) {
    return renderShader->getInfoLog();
  }
  renderProgram = std::make_shared<Program>(std::move(renderShader));
  return std::nullopt;
}

void ComputeToTextureRenderer::render(std::chrono::nanoseconds totalTime, std::chrono::nanoseconds frameTime, std::int32_t frameNumber, RendererMouseState mouseState, glm::vec3 mousePos) {
  if (renderProgram == nullptr) {
    return;
  }
  const auto timeFloat = static_cast<float>(totalTime.count()) / 1'000'000'000.0f;
  const auto timeDeltaFloat = static_cast<float>(frameTime.count()) / 1'000'000'000.0f;

  renderProgram->use();
  outputTexture->bindImage(0);
  const auto textureSize = getTextureSize();
  if (renderProgram->isActiveUniform("time")) {
    renderProgram->set1f("time", timeFloat);
  }
  if (renderProgram->isActiveUniform("timeDelta")) {
    renderProgram->set1f("timeDelta", timeDeltaFloat);
  }
  if (renderProgram->isActiveUniform("frameNum")) {
    renderProgram->set1i("frameNum", frameNumber);
  }
  if (renderProgram->isActiveUniform("mouseState")) {
    renderProgram->set1i("mouseState", static_cast<int>(mouseState));
  }
  if (renderProgram->isActiveUniform("mousePos")) {
    renderProgram->set3fv("mousePos", &mousePos[0]);
  }
  renderProgram->dispatch(textureSize.x / COMPUTE_LOCAL_GROUP_SIZE.x, textureSize.y / COMPUTE_LOCAL_GROUP_SIZE.y);
}

Texture &ComputeToTextureRenderer::getOutputTexture() {
  return *outputTexture;
}

const Texture &ComputeToTextureRenderer::getOutputTexture() const {
  return *outputTexture;
}

}// namespace pf