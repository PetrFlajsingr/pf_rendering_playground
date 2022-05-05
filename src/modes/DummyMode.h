//
// Created by xflajs00 on 02.05.2022.
//

#ifndef PF_RENDERING_PLAYGROUND_DUMMYMODE_H
#define PF_RENDERING_PLAYGROUND_DUMMYMODE_H

#include "Mode.h"

namespace pf {
class DummyMode : public Mode {
 public:
  [[nodiscard]] inline std::string getName() const override { return "Dummy"; }

 protected:
  inline void initialize_impl(const std::shared_ptr<ui::ig::ImGuiInterface> &,
                              const std::shared_ptr<glfw::Window> &) override {}
  inline void activate_impl() override {}
  inline void deactivate_impl() override {}
  inline void deinitialize_impl() override {}
  inline void render(std::chrono::nanoseconds timeDelta) override {}
};
}  // namespace pf

#endif  //PF_RENDERING_PLAYGROUND_DUMMYMODE_H
