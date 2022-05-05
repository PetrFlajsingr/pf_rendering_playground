//
// Created by xflajs00 on 18.04.2022.
//

#ifndef PF_RENDERING_PLAYGROUND_INPUTWINDOW_H
#define PF_RENDERING_PLAYGROUND_INPUTWINDOW_H

#include "ImagesPanel.h"
#include "GlobalVariablesPanel.h"
#include <pf_imgui/ImGuiInterface.h>
#include <pf_imgui/elements/Button.h>
#include <pf_imgui/elements/Checkbox.h>
#include <pf_imgui/elements/Separator.h>
#include <pf_imgui/elements/TabBar.h>
#include <pf_imgui/elements/Text.h>
#include <pf_imgui/elements/TextEditor.h>
#include <pf_imgui/elements/Spinner.h>
#include <pf_imgui/layouts/VerticalLayout.h>

namespace pf::shader_toy {

class InputWindow {
 public:
  explicit InputWindow(ui::ig::ImGuiInterface &imGuiInterface);

  // clang-format off
  ui::ig::Window *window;
    ui::ig::VerticalLayout *layout;
      ui::ig::TabBar *tabBar;
        ui::ig::Tab *mainShaderTab;
          ui::ig::HorizontalLayout *controlsLayout;
            ui::ig::Button *compileButton;
            //ui::ig::Separator *sep1;
            ui::ig::Checkbox *timePausedCheckbox;
            ui::ig::Button *restartButton;
            //ui::ig::Separator *sep2;
            ui::ig::Checkbox *autoCompileCheckbox;
            ui::ig::DragInput<float> *autoCompileFrequencyDrag;
          ui::ig::Button *codeToClipboardButton;
          ui::ig::HorizontalLayout *infoLayout;
            ui::ig::Text *infoText;
            ui::ig::Spinner *compilationSpinner;
          ui::ig::TextEditor *editor;
        ui::ig::Tab *globalVarsTab;
          ui::ig::VerticalLayout *globalVarsLayout;
            ui::ig::Button *addVarButton;
            GlobalVariablesPanel *varPanel;
        ui::ig::Tab *imagesTab;
          ImagesPanel *imagesPanel;
  // clang-format on
};

}  // namespace pf

#endif  //PF_RENDERING_PLAYGROUND_INPUTWINDOW_H
