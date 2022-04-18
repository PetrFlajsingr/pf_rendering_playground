//
// Created by xflajs00 on 18.04.2022.
//

#ifndef PF_RENDERING_PLAYGROUND_OUTPUTWINDOW_H
#define PF_RENDERING_PLAYGROUND_OUTPUTWINDOW_H

#include <pf_imgui/ImGuiInterface.h>
#include <pf_imgui/layouts/StretchLayout.h>

namespace pf {

class OutputWindow {
 public:
  explicit OutputWindow(ui::ig::ImGuiInterface &imGuiInterface);

  // clang-format off
  ui::ig::Window *window;
    ui::ig::StretchLayout *layout;
      ui::ig::Image *image;
  // clang-format on
};

}// namespace pf
#endif//PF_RENDERING_PLAYGROUND_OUTPUTWINDOW_H
