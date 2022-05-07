//
// Created by xflajs00 on 18.04.2022.
//

#pragma once

#include <pf_common/array.h>
#include <pf_imgui/ImGuiInterface.h>
#include <pf_imgui/elements/Combobox.h>
#include <pf_imgui/elements/Text.h>
#include <pf_imgui/elements/plots/SimplePlot.h>
#include <pf_imgui/layouts/StretchLayout.h>

namespace pf::shader_toy {

class OutputWindow {
  constexpr static auto IMAGE_SIZES = make_array(1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 9182);

 public:
  explicit OutputWindow(ui::ig::ImGuiInterface &imGuiInterface);

  // clang-format off
  ui::ig::Window *window;
    ui::ig::HorizontalLayout *imageSettingsLayout;
      ui::ig::Combobox<int> *widthCombobox;
      ui::ig::Combobox<int> *heightCombobox;
    ui::ig::StretchLayout *layout;
      ui::ig::Image *image;
    ui::ig::HorizontalLayout *fpsInfoLayout;
      ui::ig::SimplePlot *fpsAveragePlot;
      ui::ig::Text *fpsText;
  // clang-format on
};

}  // namespace pf
