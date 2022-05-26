//
// Created by Petr on 26/05/2022.
//

#ifndef PF_RENDERING_PLAYGROUND_OUTPUTVIEW_H
#define PF_RENDERING_PLAYGROUND_OUTPUTVIEW_H

#include "mvc/View.h"
#include <pf_imgui/elements/Combobox.h>
#include <pf_imgui/elements/Image.h>
#include <pf_imgui/elements/Text.h>
#include <pf_imgui/elements/plots/SimplePlot.h>
#include <pf_imgui/layouts/HorizontalLayout.h>
#include <pf_imgui/layouts/StretchLayout.h>

namespace pf {

// TODO: handle fps through controller
class OutputView : public UIViewWindow {
 public:
  OutputView(ui::ig::ImGuiInterface &interface, std::string_view windowName, std::string_view windowTitle);

  // clang-format off
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

#endif  //PF_RENDERING_PLAYGROUND_OUTPUTVIEW_H
