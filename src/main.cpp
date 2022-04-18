#include "renderers/ComputeToTextureRenderer.h"
#include "ui/UI.h"
#include "utils/files.h"
#include <argparse/argparse.hpp>
#include <filesystem>
#include <fmt/format.h>
#include <magic_enum.hpp>
#include <pf_glfw/GLFW.h>
#include <pf_imgui/elements/Image.h>
#include <pf_mainloop/MainLoop.h>
#include <spdlog/spdlog.h>
#include <toml++/toml.h>
#include <ui/UI.h>

argparse::ArgumentParser createArgParser() {
  auto parser = argparse::ArgumentParser{"OpenGL template"};
  parser.add_description("OpenGL template project using pf_ libraries");
  return parser;
}

/**
 * Load toml config located next to the exe - config.toml
 * @return
 */
toml::table loadConfig() {
  const auto configPath = pf::getExeFolder() / "config.toml";
  const auto configPathStr = configPath.string();
  spdlog::info("Loading config from: '{}'", configPathStr);
  return toml::parse_file(configPathStr);
}

/**
 * Serialize UI, save it to the config and save the config next to the exe into config.toml
 */
void saveConfig(toml::table config, pf::ui::ig::ImGuiInterface &imguiInterface, const std::shared_ptr<pf::glfw::Window> &window) {
  const auto configPath = pf::getExeFolder() / "config.toml";
  const auto configPathStr = configPath.string();
  spdlog::info("Saving config file to: '{}'", configPathStr);
  imguiInterface.updateConfig();
  config.insert_or_assign("imgui", imguiInterface.getConfig());
  const auto &[width, height] = window->getSize();
  config["window"].as_table()->insert_or_assign("width", width);
  config["window"].as_table()->insert_or_assign("height", height);
  auto ofstream = std::ofstream(configPathStr);
  ofstream << config;
}

int main(int argc, char *argv[]) {
  spdlog::default_logger()->set_level(spdlog::level::trace);
  auto parser = createArgParser();
  try {
    parser.parse_args(argc, argv);
  } catch (const std::runtime_error &e) {
    spdlog::error("{}", e.what());
    fmt::print("{}", parser.help().str());
    return 1;
  }
  const auto config = loadConfig();
  const auto resourcesFolder = std::filesystem::path{config["files"]["resources_path"].value<std::string>().value()};

  spdlog::info("Initializing window and OpenGL");
  pf::glfw::GLFW glfw{};
  auto window = glfw.createWindow({.width = static_cast<std::size_t>(config["window"]["width"].value_or(1200)),
                                   .height = static_cast<std::size_t>(config["window"]["height"].value_or(900)),
                                   .title = "OpenGL",
                                   .majorOpenGLVersion = 4,
                                   .minorOpenGLVersion = 6});
  window->setCurrent();
  if (!gladLoadGLLoader((GLADloadproc) glfw.getLoaderFnc())) {
    spdlog::error("Error while initializing GLAD");
    return -1;
  }

  auto mainUI = pf::ogl::UI{*config["imgui"].as_table(), window};
  pf::ComputeToTextureRenderer renderer{};

  mainUI.outputWindow->image->setTextureId(reinterpret_cast<ImTextureID>(static_cast<std::uintptr_t>(renderer.getOutputTexture().getId())));

  std::chrono::nanoseconds totalTime{0};
  std::uint32_t frameCounter = 0;
  glm::vec3 mousePos{};
  mainUI.textInputWindow->compileButton->addClickListener([&] {
    spdlog::info("Compiling shader: start");
    if (auto err = renderer.setShader(mainUI.textInputWindow->editor->getText()); err.has_value()) {
      spdlog::error(err.value());
    } else {
      totalTime = std::chrono::nanoseconds{0};
      frameCounter = 0;
      spdlog::info("Compiling shader: success");
    }
  });
  mainUI.outputWindow->image->addMousePositionListener([&](auto pos) {
    const auto size = mainUI.outputWindow->image->getSize();
    const auto nX = pos.x / static_cast<float>(size.width);
    const auto nY = pos.y / static_cast<float>(size.height);// TODO: check this on windows
    const auto result = glm::vec2{renderer.getTextureSize()} * glm::vec2{nX, nY};
    mousePos.x = result.x;
    mousePos.y = result.y;
  });

  pf::MainLoop::Get()->setOnMainLoop([&](auto time) {
    if (window->shouldClose()) {
      pf::MainLoop::Get()->stop();
    }
    mainUI.imguiInterface->render();
    pf::RendererMouseState mouseState = pf::RendererMouseState::None;
    if (mainUI.outputWindow->image->isHovered()) {
      if (window->getLastMouseButtonState(pf::glfw::MouseButton::Left) == pf::glfw::ButtonState::Down) {
        mouseState = pf::RendererMouseState::LeftDown;
      } else if (window->getLastMouseButtonState(pf::glfw::MouseButton::Right) == pf::glfw::ButtonState::Down) {
        mouseState = pf::RendererMouseState::RightDown;
      }
    }
    renderer.render(totalTime, time, frameCounter, mouseState, mousePos);
    window->swapBuffers();
    glfw.pollEvents();
    totalTime += time;
  });

  spdlog::info("Starting main loop");
  pf::MainLoop::Get()->run();
  spdlog::info("Main loop ended");

  saveConfig(config, *mainUI.imguiInterface, window);
  return 0;
}
