//
// Created by xflajs00 on 18.04.2022.
//

#ifndef PF_RENDERING_PLAYGROUND_MODEMANAGER_H
#define PF_RENDERING_PLAYGROUND_MODEMANAGER_H

#include "Mode.h"

namespace pf {

class ModeManager {
 public:
  using Error = std::string;

  ModeManager(std::shared_ptr<ui::ig::ImGuiInterface> imGuiInterface, std::shared_ptr<glfw::Window> window,
              toml::table config);
  ~ModeManager();

  [[nodiscard]] toml::table getConfig() const;

  std::optional<Error> addMode(std::shared_ptr<Mode> mode);

  std::optional<Error> activateMode(const std::string &name);
  std::optional<Error> activateMode(const std::shared_ptr<Mode> &mode);

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

  std::shared_ptr<ui::ig::ImGuiInterface> imGuiInterface;
  std::shared_ptr<glfw::Window> window;
  toml::table config;

  ModeRecord *activeMode = nullptr;
  std::list<ModeRecord> modes;

  ui::ig::SubMenu &subMenu;
  ui::ig::Text &statusBarText;
};

}  // namespace pf

#endif  //PF_RENDERING_PLAYGROUND_MODEMANAGER_H
