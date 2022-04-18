//
// Created by xflajs00 on 18.04.2022.
//

#include "ShaderToyTextInputWindow.h"
#include <pf_imgui/elements/Button.h>
#include <pf_imgui/elements/Expander.h>

namespace pf {

ShaderToyTextInputWindow::ShaderToyTextInputWindow(ui::ig::ImGuiInterface &imGuiInterface) {
  window = &imGuiInterface.createWindow("text_input_window", "Editor");
  window->setIsDockable(true);
  layout = &window->createChild(ui::ig::VerticalLayout::Config{
      .name = "text_input_layout",
      .size = ui::ig::Size::Auto()});
  controlsLayout = &layout->createChild(ui::ig::HorizontalLayout::Config{
      .name = "text_controls_layout",
      .size = ui::ig::Size{ui::ig::Width::Auto(), 80}});
  compileButton = &controlsLayout->createChild(ui::ig::Button::Config{"compile_btn", "Compile"});
  editor = &layout->createChild(ui::ig::TextEditor::Config{
      .name = "text_editor",
      .persistent = true});
}

}// namespace pf