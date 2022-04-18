//
// Created by xflajs00 on 18.04.2022.
//

#ifndef PF_RENDERING_PLAYGROUND_COMPUTETOTEXTURERENDERER_H
#define PF_RENDERING_PLAYGROUND_COMPUTETOTEXTURERENDERER_H

#include "fmt/format.h"
#include <chrono>
#include <geGL/Program.h>
#include <geGL/Shader.h>
#include <geGL/Texture.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <regex>

namespace pf {

enum class RendererMouseState {
  None = 0,
  LeftDown = 1,
  RightDown = 2
};

namespace details {

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

std::string uniformsAsString(const std::vector<UniformInfo> &uniforms);
std::string image2DsAsString(const std::vector<Image2DInfo> &uniforms);
std::string definesAsString(const std::vector<ShaderDefine> &defines);

std::string addTextureAccessCheck(std::string src, const std::string &textureName);

std::string createShader(glm::uvec2 localGroupSize, const std::vector<UniformInfo> &uniforms, const std::vector<ShaderDefine> &defines, Image2DInfo image, std::string userCode);

}// namespace details

class ComputeToTextureRenderer {
 public:
  constexpr static glm::uvec2 DEFAULT_TEXTURE_SIZE{1024, 1024};
  constexpr static glm::uvec2 COMPUTE_LOCAL_GROUP_SIZE{8, 8};
  ComputeToTextureRenderer();
  void setTextureSize(glm::uvec2 textureSize);
  [[nodiscard]] glm::uvec2 getTextureSize() const;
  [[nodiscard]] Texture &getOutputTexture();
  [[nodiscard]] const Texture &getOutputTexture() const;
  /**
   * Only main basically
   * @param shaderSource
   */
  std::optional<std::string> setShader(std::string_view shaderSource);

  void render(std::chrono::nanoseconds totalTime, std::chrono::nanoseconds frameTime, std::int32_t frameNumber, RendererMouseState mouseState, glm::vec3 mousePos);

 private:
  std::shared_ptr<Texture> outputTexture = nullptr;
  std::shared_ptr<Program> renderProgram = nullptr;
  std::string currentShaderSrc{};
};

}// namespace pf
#endif//PF_RENDERING_PLAYGROUND_COMPUTETOTEXTURERENDERER_H
