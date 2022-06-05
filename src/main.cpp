#include "glad/glad.h"
#include "gpu/opengl/RenderThread.h"
#include "imgui/ImGuiGlfwOpenGLInterface.h"
#include "modes/DummyMode.h"
#include "modes/ModeManager.h"
#include "shader_toy/ShaderToyMode.h"
#include "utils/files.h"
#include "utils/logging.h"
#include <argparse/argparse.hpp>
#include <filesystem>
#include <fmt/format.h>
#include <magic_enum.hpp>
#include <pf_glfw/GLFW.h>
#include <pf_imgui/elements/Image.h>
#include <pf_imgui/styles/dark.h>
#include <pf_mainloop/MainLoop.h>
#include <spdlog/spdlog.h>
#include <toml++/toml.h>

argparse::ArgumentParser createArgParser() {
  auto parser = argparse::ArgumentParser{"pf_rendering_playground"};
  parser.add_description("Toy rendering project");
  return parser;
}

/**
 * Load toml config located next to the exe - config.toml
 * @return
 * @todo: make config optional
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
void saveConfig(toml::table config, pf::ui::ig::ImGuiInterface &imguiInterface,
                const std::shared_ptr<pf::glfw::Window> &window, const pf::ModeManager &modeManager) {
  const auto configPath = pf::getExeFolder() / "config.toml";
  const auto configPathStr = configPath.string();
  spdlog::info("Saving config file to: '{}'", configPathStr);
  imguiInterface.updateConfig();
  config.insert_or_assign("imgui", imguiInterface.getConfig());
  const auto &[width, height] = window->getSize();
  config["window"].as_table()->insert_or_assign("width", width);
  config["window"].as_table()->insert_or_assign("height", height);
  config.insert_or_assign("modes", modeManager.getConfig());
  auto ofstream = std::ofstream(configPathStr);
  ofstream << config;
}

int main(int argc, char *argv[]) {
  spdlog::default_logger()->set_level(spdlog::level::trace);
  spdlog::default_logger()->sinks().clear();
  spdlog::default_logger()->sinks().emplace_back(pf::log::stdout_color_sink);
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

  auto renderThread = std::make_shared<pf::OpenGlRenderThread>(window);
  // setting opengl api context ownership to rendering thread
  glfwMakeContextCurrent(nullptr);
  renderThread->enqueue([&window] { window->setCurrent(); });

  const auto imguiConfig = *config["imgui"].as_table();
  auto imguiInterface = std::make_shared<pf::ui::ig::ImGuiGlfwOpenGLInterface>(pf::ui::ig::ImGuiGlfwOpenGLConfig{
      .imgui{.flags = pf::ui::ig::ImGuiConfigFlags::DockingEnable, .config = imguiConfig},
      .windowHandle = window->getHandle(),
      .renderThread = renderThread});

  const auto fontPath = resourcesFolder / "fonts" / "Roboto-Regular.ttf";
  if (std::filesystem::exists(fontPath)) {
    auto robotoFont =
        imguiInterface->getFontManager().fontBuilder("Roboto-Regular", fontPath).setFontSize(14.f).build();
    imguiInterface->setGlobalFont(robotoFont);
  }
  pf::ui::ig::setDarkStyle(*imguiInterface);

  auto modeManagerConfig = toml::table{};
  if (const auto iter = config.find("modes"); iter != config.end()) {
    if (auto configTable = iter->second.as_table(); configTable != nullptr) { modeManagerConfig = *configTable; }
  }

  pf::ModeManager modeManager{imguiInterface, window, renderThread, modeManagerConfig, 4};

  modeManager.addMode(std::make_shared<pf::shader_toy::ShaderToyMode>(resourcesFolder));
  modeManager.activateMode("ShaderToy");
  modeManager.addMode(std::make_shared<pf::DummyMode>());

  renderThread->enqueue([&glfw] { glfw.setSwapInterval(0); });

  pf::MainLoop::Get()->setOnMainLoop([&](auto time) {
    if (window->shouldClose()) { pf::MainLoop::Get()->stop(); }

    // gotta render imgui first, since we need to wait for render thread in there
    imguiInterface->render();
    modeManager.render(time);

    renderThread->enqueue([&window] { window->swapBuffers(); });
    renderThread->waitForDone();

    glfw.pollEvents();
  });

  pf::MainLoop::Get()->run();

  saveConfig(config, *imguiInterface, window, modeManager);
  return 0;
}
