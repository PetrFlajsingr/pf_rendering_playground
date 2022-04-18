//
// Created by xflajs00 on 18.04.2022.
//

#ifndef PF_RENDERING_PLAYGROUND_MODE_H
#define PF_RENDERING_PLAYGROUND_MODE_H

#include <pf_glfw/Window.h>
#include <pf_imgui/ImGuiInterface.h>

namespace pf {

class Mode {
  friend class ModeManager;

 public:
  virtual ~Mode() = default;

  [[nodiscard]] inline bool isInitialized() const {
    return initialized;
  }
  [[nodiscard]] inline bool isActive() const {
    return active;
  }

 protected:
  inline void initialize(const std::shared_ptr<ui::ig::ImGuiInterface> &imguiInterface, const std::shared_ptr<glfw::Window> &window) {
    initialize_impl(imguiInterface, window);
    initialized = true;
  }
  virtual void initialize_impl(const std::shared_ptr<ui::ig::ImGuiInterface> &imguiInterface, const std::shared_ptr<glfw::Window> &window) = 0;

  inline void activate() {
    activate_impl();
    active = true;
  }
  virtual void activate_impl() = 0;

  inline void deactivate() {
    deactivate_impl();
    active = false;
  }
  virtual void deactivate_impl() = 0;
  inline void deinitialize() {
    deinitialize_impl();
    initialized = false;
  }
  virtual void deinitialize_impl() = 0;

  virtual void render(std::chrono::nanoseconds timeDelta) = 0;

 private:
  bool initialized = false;
  bool active = false;
};

}// namespace pf

#endif//PF_RENDERING_PLAYGROUND_MODE_H
