//
// Created by xflajs00 on 18.04.2022.
//

#pragma once

#include "glm/ext/vector_uint2.hpp"
#include "modes/Mode.h"
#include "ui/UI.h"
#include <future>
#include "gpu/Texture.h"
#include "gpu/Program.h"
#include <glm/glm.hpp>
#include <utils/FPSCounter.h>

namespace pf::shader_toy {

enum class MouseState { None = 0, LeftDown = 1, RightDown = 2 };
// todo: divide this up into more classes
class ShaderToyMode : public Mode {
 public:
  constexpr static glm::uvec2 COMPUTE_LOCAL_GROUP_SIZE{8, 8};
  explicit ShaderToyMode(std::filesystem::path resourcesPath);

  [[nodiscard]] std::string getName() const override;

 protected:
  void initialize_impl(const std::shared_ptr<ui::ig::ImGuiInterface> &imguiInterface,
                       const std::shared_ptr<glfw::Window> &window, std::shared_ptr<ThreadPool> threadPool) override;
  std::vector<std::shared_ptr<spdlog::sinks::sink>> createLoggerSinks() override;
  void activate_impl() override;
  void deactivate_impl() override;
  void deinitialize_impl() override;
  void render(std::chrono::nanoseconds timeDelta) override;

  void updateConfig() override;

 private:
  void resetCounters();

  void initializeTexture(TextureSize textureSize);

  [[nodiscard]] glm::uvec2 getTextureSize() const;

  void compileShader(const std::string &shaderCode);
  void compileShader_impl(const std::string &shaderCode);

  void updateUI();

  struct ConfigData {
    std::filesystem::path resourcesPath;
  } configData;

  std::unique_ptr<UI> ui = nullptr;

  std::int32_t frameCounter = 0;
  std::chrono::nanoseconds totalTime{0};
  bool timeCounterPaused = false;

  std::shared_ptr<Texture> outputTexture = nullptr;

  std::unique_ptr<Program> mainProgram = nullptr;

  std::string currentShaderSrc{};
  std::shared_ptr<glfw::Window> glfwWindow = nullptr;
  glm::vec2 mousePos{};

  std::vector<std::pair<std::string, std::shared_ptr<Texture>>> userDefinedTextures;

  std::function<std::size_t(std::size_t)> shaderLineMapping;

  FPSCounter fpsCounter;

  bool autoCompileShader = false;
  bool isShaderChanged = true;
  std::chrono::time_point<std::chrono::steady_clock> lastShaderChangeTime = std::chrono::steady_clock::now();

  std::shared_ptr<ThreadPool> workerThreads = nullptr;
  std::vector<std::future<void>> unfinishedWorkerTasks{};

  bool previousShaderCompilationDone = true;

  std::chrono::steady_clock::time_point lastFPSVisualUpdate{};
  std::chrono::milliseconds FPSVisualUpdateFrequency{100};

  constexpr static auto DEFAULT_SHADER_SOURCE = R"glsl(
void main() {
  const ivec2 texSize = imageSize(outImage);
  const ivec2 texCoord = ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y);
  const vec2 uv = vec2(texCoord) / vec2(texSize);
  imageStore(outImage, texCoord, vec4(sin(time), cos(time), sin(time) * cos(time), 1.f));
})glsl";
};

}  // namespace pf::shader_toy
