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

  ModeManager(std::shared_ptr<ui::ig::ImGuiInterface> imGuiInterface, std::shared_ptr<glfw::Window> window);
  ~ModeManager();

  void addMode(std::shared_ptr<Mode> mode);

  std::optional<Error> activateMode(const std::string &name);
  std::optional<Error> activateMode(const std::shared_ptr<Mode> &mode);

  void render(std::chrono::nanoseconds timeDelta);

 private:
  void deactivateModes();
  void deinitializeModes();

  std::shared_ptr<ui::ig::ImGuiInterface> imGuiInterface;
  std::shared_ptr<glfw::Window> window;

  std::shared_ptr<Mode> activeMode = nullptr;
  struct ModeRecord {
    std::string name;
    std::shared_ptr<Mode> mode;
  };
  std::vector<ModeRecord> modes;

  ui::ig::SubMenu &subMenu;
};

}  // namespace pf

#endif  //PF_RENDERING_PLAYGROUND_MODEMANAGER_H
