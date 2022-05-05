//
// Created by xflajs00 on 18.04.2022.
//

#ifndef PF_RENDERING_PLAYGROUND_MODE_H
#define PF_RENDERING_PLAYGROUND_MODE_H

#include <pf_glfw/Window.h>
#include <pf_imgui/ImGuiInterface.h>

namespace pf {

enum class ModeState { Uninitialised, Initialised, Active };

class Mode {
  friend class ModeManager;

 public:
  virtual ~Mode() = default;

  [[nodiscard]] const toml::table &getConfig() const;

  [[nodiscard]] ModeState getState() const;

  [[nodiscard]] virtual std::string getName() const = 0;

 protected:
  void initialize(const std::shared_ptr<ui::ig::ImGuiInterface> &imguiInterface,
                  const std::shared_ptr<glfw::Window> &window, toml::table modeConfig);

  virtual void initialize_impl(const std::shared_ptr<ui::ig::ImGuiInterface> &imguiInterface,
                               const std::shared_ptr<glfw::Window> &window) = 0;

  void activate();
  virtual void activate_impl() = 0;

  void deactivate();
  virtual void deactivate_impl() = 0;
  void deinitialize();
  virtual void deinitialize_impl() = 0;

  virtual void render(std::chrono::nanoseconds timeDelta) = 0;

  toml::table config;

 private:
  ModeState state = ModeState::Uninitialised;
};

}  // namespace pf

#endif  //PF_RENDERING_PLAYGROUND_MODE_H
