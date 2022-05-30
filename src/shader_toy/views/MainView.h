//
// Created by Petr on 30/05/2022.
//
#pragma once

#include "mvc/View.h"
#include <pf_imgui/dialogs/BackgroundDockingArea.h>

namespace pf {

class MainView : public UIViewBase {
 public:
  explicit MainView(ui::ig::ImGuiInterface &imGuiInterface);
  // TODO: make it possible to create more of these in pf_imgui, so they can be switched around
  // and implement it here later as well
  ui::ig::BackgroundDockingArea *dockingArea;

};

}  // namespace pf
