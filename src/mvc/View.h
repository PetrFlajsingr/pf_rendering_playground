//
// Created by xflajs00 on 15.05.2022.
//

#pragma once

#include "pf_imgui/dialogs/Window.h"
#include "pf_imgui/interface/Layout.h"

namespace pf {

class UIViewBase {
 public:
  inline virtual ~UIViewBase() = 0;
};
UIViewBase::~UIViewBase() = default;

class UIViewWindow : public UIViewBase {
 public:
  explicit inline UIViewWindow(ui::ig::Window *window) : window(window) {}
  [[nodiscard]] inline ui::ig::Window &getWindow() { return *window; }
  [[nodiscard]] inline const ui::ig::Window &getWindow() const { return *window; }

 protected:
  ui::ig::Window *window;
};


}  // namespace pf