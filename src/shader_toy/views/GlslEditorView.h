//
// Created by Petr on 26/05/2022.
//

#pragma once

#include "mvc/View.h"
#include <pf_imgui/elements/Button.h>
#include <pf_imgui/elements/Checkbox.h>
#include <pf_imgui/elements/DragInput.h>
#include <pf_imgui/elements/Separator.h>
#include <pf_imgui/elements/Spinner.h>
#include <pf_imgui/elements/Text.h>
#include <pf_imgui/elements/TextEditor.h>
#include <pf_imgui/layouts/HorizontalLayout.h>
#include <pf_imgui/layouts/VerticalLayout.h>
#include <string_view>

namespace pf {
// TODO: copy to clipboard
// TODO: hint from controller
class GlslEditorView : public UIViewWindow {
 public:
  GlslEditorView(std::shared_ptr<ui::ig::ImGuiInterface> imguiInterface, std::string_view windowName,
                 std::string_view windowTitle);

  // clang-format off
  ui::ig::VerticalLayout *layout;
     ui::ig::HorizontalLayout *controlsLayout;
       ui::ig::Button *compileButton;
       ui::ig::Separator *sep1;
       ui::ig::Checkbox *timePausedCheckbox;
       ui::ig::Button *restartButton;
       ui::ig::Separator *sep2;
       ui::ig::Checkbox *autoCompileCheckbox;
       ui::ig::DragInput<float> *autoCompilePeriodDrag;
     ui::ig::HorizontalLayout *infoLayout;
       ui::ig::Text *infoText;
       ui::ig::Spinner *compilationSpinner;
     ui::ig::TextEditor *editor;
  // clang-format on
 private:
  void createTooltips();
};

}  // namespace pf
