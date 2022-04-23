//
// Created by xflajs00 on 18.04.2022.
//

#ifndef PF_RENDERING_PLAYGROUND_SHADERTOYMODE_H
#define PF_RENDERING_PLAYGROUND_SHADERTOYMODE_H

#include "glm/ext/vector_uint2.hpp"
#include "modes/Mode.h"
#include "ui/ShaderToyUI.h"
#include <geGL/Program.h>
#include <geGL/Shader.h>
#include <geGL/Texture.h>
#include <glm/glm.hpp>
#include <utils/FPSCounter.h>

namespace pf {

enum class MouseState { None = 0, LeftDown = 1, RightDown = 2 };
// todo: divide this up into more classes
class ShaderToyMode : public Mode {
 public:
  constexpr static glm::uvec2 COMPUTE_LOCAL_GROUP_SIZE{8, 8};

  [[nodiscard]] std::string getName() const override;

 protected:
  void initialize_impl(const std::shared_ptr<ui::ig::ImGuiInterface> &imguiInterface,
                       const std::shared_ptr<glfw::Window> &window) override;
  void activate_impl() override;
  void deactivate_impl() override;
  void deinitialize_impl() override;
  void render(std::chrono::nanoseconds timeDelta) override;

 private:
  void resetCounters();

  void initializeTexture(glm::uvec2 textureSize);

  [[nodiscard]] glm::uvec2 getTextureSize() const;

  std::optional<std::string> compileShader(const std::string &shaderCode);

  void updateUI();

  std::unique_ptr<ShaderToyUI> ui = nullptr;

  std::int32_t frameCounter = 0;
  std::chrono::nanoseconds totalTime{0};
  bool timeCounterPaused = false;

  std::shared_ptr<Texture> outputTexture = nullptr;
  std::shared_ptr<Program> renderProgram = nullptr;
  std::string currentShaderSrc{};
  std::shared_ptr<glfw::Window> glfwWindow = nullptr;
  glm::vec2 mousePos{};

  std::unordered_map<std::string, std::shared_ptr<ValueRecord>> userDefinedUniforms;

  std::function<std::size_t(std::size_t)> shaderLineMapping;

  FPSCounter fpsCounter;

  constexpr static auto DEFAULT_SHADER_SOURCE = R"glsl(
void main() {
  const ivec2 texSize = imageSize(outImage);
  const ivec2 texCoord = ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y);
  const vec2 uv = vec2(texCoord) / vec2(texSize);
  imageStore(outImage, texCoord, vec4(sin(time), cos(time), sin(time) * cos(time), 1.f));
})glsl";
};

}  // namespace pf
#endif  //PF_RENDERING_PLAYGROUND_SHADERTOYMODE_H
