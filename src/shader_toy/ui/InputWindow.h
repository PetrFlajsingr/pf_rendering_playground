//
// Created by xflajs00 on 18.04.2022.
//

#pragma once

#include "ImagesPanel.h"
#include <pf_imgui/ImGuiInterface.h>
#include <pf_imgui/elements/Button.h>
#include <pf_imgui/elements/Checkbox.h>
#include <pf_imgui/elements/DragInput.h>
#include <pf_imgui/elements/Separator.h>
#include <pf_imgui/elements/Spinner.h>
#include <pf_imgui/elements/TabBar.h>
#include <pf_imgui/elements/Text.h>
#include <pf_imgui/elements/TextEditor.h>
#include <pf_imgui/layouts/VerticalLayout.h>

namespace pf::shader_toy {

class InputWindow {
 public:
  InputWindow(ui::ig::ImGuiInterface &imGuiInterface, std::unique_ptr<ImageLoader> &&imageLoader);

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
        ui::ig::Tab *imagesTab;
          ImagesPanel *imagesPanel;
  // clang-format on
};

}  // namespace pf::shader_toy
