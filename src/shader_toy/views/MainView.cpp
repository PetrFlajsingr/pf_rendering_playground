//
// Created by Petr on 30/05/2022.
//

#include "MainView.h"
#include <pf_imgui/ImGuiInterface.h>

namespace pf {

MainView::MainView(std::shared_ptr<ui::ig::ImGuiInterface> imguiInterface) : UIViewBase{std::move(imguiInterface)} {
  dockingArea = &interface->createOrGetBackgroundDockingArea();
}

}  // namespace pf