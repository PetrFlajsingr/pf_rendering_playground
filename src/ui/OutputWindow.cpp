//
// Created by xflajs00 on 18.04.2022.
//

#include "OutputWindow.h"
#include <pf_imgui/elements/Image.h>
#include <pf_imgui/layouts/StretchLayout.h>
#include <pf_imgui/layouts/VerticalLayout.h>

namespace pf {

OutputWindow::OutputWindow(ui::ig::ImGuiInterface &imGuiInterface) {
  window = &imGuiInterface.createWindow("output_window", "Output");
  window->setIsDockable(true);
  layout = &window->createChild(ui::ig::StretchLayout::Config{
      .name = "output_layout",
      .size = ui::ig::Size::Auto(),
      .stretch = ui::ig::Stretch::All});

  image = &layout->createChild(ui::ig::Image::Config{
      .name = "output_img",
      .textureId = 0,
      .size = ui::ig::Size::Auto(),
      .isButton = false});
}

}// namespace pf
