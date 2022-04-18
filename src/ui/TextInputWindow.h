//
// Created by xflajs00 on 18.04.2022.
//

#ifndef PF_RENDERING_PLAYGROUND_TEXTINPUTWINDOW_H
#define PF_RENDERING_PLAYGROUND_TEXTINPUTWINDOW_H

#include <pf_imgui/layouts/VerticalLayout.h>
#include <pf_imgui/elements/TextEditor.h>
#include <pf_imgui/ImGuiInterface.h>

namespace pf {

class TextInputWindow {
 public:
  explicit TextInputWindow(ui::ig::ImGuiInterface &imGuiInterface);

  // clang-format off
  ui::ig::Window *window;
    ui::ig::VerticalLayout *layout;
      ui::ig::HorizontalLayout *controlsLayout;
        ui::ig::Button *compileButton;
      ui::ig::TextEditor *editor;
  // clang-format on
};

}

#endif//PF_RENDERING_PLAYGROUND_TEXTINPUTWINDOW_H
