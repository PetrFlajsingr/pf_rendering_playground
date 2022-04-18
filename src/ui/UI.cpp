//
// Created by xflajs00 on 22.10.2021.
//

#include "UI.h"
#include "imgui/ImGuiGlfwOpenGLInterface.h"
#include <pf_imgui/styles/dark.h>
#include <pf_mainloop/MainLoop.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/spdlog.h>

#undef RGB

template<typename Mutex>
class PfImguiLogSink : public spdlog::sinks::base_sink<Mutex> {
 public:
  template<std::size_t LogLimit>
  explicit PfImguiLogSink(pf::ui::ig::LogPanel<spdlog::level::level_enum, LogLimit> &logPanel)
      : spdlog::sinks::base_sink<Mutex>(),
        addRecord([this, &logPanel](auto level, auto message) { logPanel.addRecord(level, message); }) {
    logPanel.addDestroyListener([this] { panelValid = false; });
  }
  template<std::size_t LogLimit>
  PfImguiLogSink(pf::ui::ig::LogPanel<spdlog::level::level_enum, LogLimit> &logPanel,
                 const std::unique_ptr<spdlog::formatter> &formatter)
      : spdlog::sinks::base_sink<Mutex>(formatter),
        addRecord([this, &logPanel](auto level, auto message) { logPanel.addRecord(level, message); }) {
    logPanel.addDestroyListener([this] { panelValid = false; });
  }

 protected:
  void sink_it_(const spdlog::details::log_msg &msg) override {
    if (!panelValid) { return; }
    spdlog::memory_buf_t formatted;
    spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
    addRecord(msg.level, std::string_view{formatted.begin(), formatted.end()});
  }
  void flush_() override {}

 private:
  bool panelValid = true;
  std::function<void(spdlog::level::level_enum, std::string_view)> addRecord;
};
using PfImguiLogSink_mt = PfImguiLogSink<std::mutex>;
using PfImguiLogSink_st = PfImguiLogSink<spdlog::details::null_mutex>;

pf::ogl::UI::UI(const toml::table &config, const std::shared_ptr<glfw::Window> &window) {
  using namespace ui::ig;
  imguiInterface = std::make_unique<ImGuiGlfwOpenGLInterface>(ImGuiGlfwOpenGLConfig{
      .imgui{.flags = ui::ig::ImGuiConfigFlags::DockingEnable,
             .config = config,
             .iconFontDirectory = *config["path_icons"].value<std::string>(),
             .enabledIconPacks = IconPack::FontAwesome5Regular,
             .iconSize = 13.f},
      .windowHandle = window->getHandle()});
  setDarkStyle(*imguiInterface);

  appMenuBar = &imguiInterface->getMenuBar();

  dockingArea = &imguiInterface->createOrGetBackgroundDockingArea();

  outputWindow = std::make_unique<OutputWindow>(*imguiInterface);
  textInputWindow = std::make_unique<TextInputWindow>(*imguiInterface);

  logWindow = &imguiInterface->createWindow("log_window", "Log");
  logWindow->setIsDockable(true);
  logPanel = &logWindow->createChild(LogPanel<spdlog::level::level_enum, 512>::Config{
      .name = "log_panel"});
  logPanel->setCategoryAllowed(spdlog::level::level_enum::n_levels, false);

  logPanel->setCategoryColor(spdlog::level::warn, Color::RGB(255, 213, 97));
  logPanel->setCategoryColor(spdlog::level::err, Color::RGB(173, 23, 23));
  logPanel->setCategoryColor(spdlog::level::info, Color::RGB(44, 161, 21));

  spdlog::default_logger()->sinks().emplace_back(std::make_shared<PfImguiLogSink_st>(*logPanel));

  imguiInterface->setStateFromConfig();
}
