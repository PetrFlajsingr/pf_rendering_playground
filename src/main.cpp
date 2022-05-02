#include "glad/glad.h"
#include "imgui/ImGuiGlfwOpenGLInterface.h"
#include "modes/ModeManager.h"
#include "modes/DummyMode.h"
#include "shader_toy/ShaderToyMode.h"
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

  const auto imguiConfig = *config["imgui"].as_table();
  auto imguiInterface = std::make_shared<pf::ui::ig::ImGuiGlfwOpenGLInterface>(pf::ui::ig::ImGuiGlfwOpenGLConfig{
      .imgui{.flags = pf::ui::ig::ImGuiConfigFlags::DockingEnable,
             .config = imguiConfig,
             .iconFontDirectory = *imguiConfig["path_icons"].value<std::string>(),
             .enabledIconPacks = pf::ui::ig::IconPack::FontAwesome5Regular,
             .iconSize = 13.f},
      .windowHandle = window->getHandle()});

  const auto fontPath = resourcesFolder / "fonts" / "Roboto-Regular.ttf";
  if (std::filesystem::exists(fontPath)) {
    auto robotoFont = imguiInterface->getFontManager().fontBuilder("roboto", fontPath).setFontSize(14.f).build();
    imguiInterface->setGlobalFont(robotoFont);
  }

  pf::ModeManager modeManager{imguiInterface, window};

  modeManager.addMode(std::make_shared<pf::shader_toy::ShaderToyMode>());
  modeManager.activateMode("ShaderToy");
  modeManager.addMode(std::make_shared<pf::DummyMode>());

  //glfw.setSwapInterval(0);
  pf::MainLoop::Get()->setOnMainLoop([&](auto time) {
    if (window->shouldClose()) {
      pf::MainLoop::Get()->stop();
    }
    imguiInterface->render();
    modeManager.render(time);
    window->swapBuffers();
    glfw.pollEvents();
  });

  spdlog::info("Starting main loop");
  pf::MainLoop::Get()->run();
  spdlog::info("Main loop ended");

  saveConfig(config, *imguiInterface, window);
  return 0;
}
