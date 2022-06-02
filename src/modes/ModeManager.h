//
// Created by xflajs00 on 18.04.2022.
//

#pragma once

#include "Mode.h"
#include "common_ui/controllers/LogWindowController.h"
#include "gpu/RenderThread.h"
#include <pf_common/parallel/ThreadPool.h>
#include <pf_imgui/elements/LogPanel.h>

namespace pf {

class ModeManager {
 public:
  using Error = std::string;

  ModeManager(std::shared_ptr<ui::ig::ImGuiInterface> imGuiInterface, std::shared_ptr<glfw::Window> glfwWindow,
              std::shared_ptr<RenderThread> renderingThread, toml::table config, std::size_t workerThreadCount);
  ~ModeManager();

  [[nodiscard]] toml::table getConfig() const;

  std::optional<Error> addMode(std::shared_ptr<Mode> mode);

  std::optional<Error> activateMode(const std::string &name);
  std::optional<Error> activateMode(const std::shared_ptr<Mode> &modeToActivate);

  [[nodiscard]] bool isModeActive() const;

  void render(std::chrono::nanoseconds timeDelta);

 private:
  struct ModeRecord {
    std::string name;
    std::shared_ptr<Mode> mode;
    ui::ig::MenuButtonItem &buttonItem;
  };

  void deactivateModes();
  void deinitializeModes();

  void activateMode_impl(ModeManager::ModeRecord *mode);

  [[nodiscard]] std::optional<ModeRecord *> findModeByName(const std::string &name);

  std::shared_ptr<ui::ig::ImGuiInterface> imguiInterface;
  std::shared_ptr<glfw::Window> window;
  toml::table config;

  std::shared_ptr<ThreadPool> workerThreads;
  std::shared_ptr<RenderThread> renderThread;

  ModeRecord *activeMode = nullptr;
  std::list<ModeRecord> modes;

  ui::ig::SubMenu &subMenu;
  ui::ig::MenuCheckboxItem &showMainLogWindowCheckboxItem;
  ui::ig::MenuSeparatorItem &subMenuSeparator;

  std::unique_ptr<LogWindowController> logWindowController;

  ui::ig::Text &statusBarText;
  std::shared_ptr<spdlog::logger> logger;
};

}  // namespace pf
