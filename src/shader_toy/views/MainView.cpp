//
// Created by Petr on 30/05/2022.
//

#include "MainView.h"
#include <pf_imgui/ImGuiInterface.h>

namespace pf {

MainView::MainView(ui::ig::ImGuiInterface &imGuiInterface) {
  dockingArea = &imGuiInterface.createOrGetBackgroundDockingArea();
}

}  // namespace pf