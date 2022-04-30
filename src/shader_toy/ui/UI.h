//
// Created by xflajs00 on 18.04.2022.
//

#ifndef PF_RENDERING_PLAYGROUND_UI_H
#define PF_RENDERING_PLAYGROUND_UI_H

#include "GlobalVariablesPanel.h"
#include "InputWindow.h"
#include "OutputWindow.h"
#include <pf_imgui/ImGuiInterface.h>
#include <pf_imgui/elements/LogPanel.h>
#include <spdlog/spdlog.h>

namespace pf::shader_toy {

struct UI {
  explicit UI(const std::shared_ptr<ui::ig::ImGuiInterface> &imGuiInterface,
                       const std::string &initShaderCode);

  void show();

  void hide();

  // clang-format off
  ui::ig::BackgroundDockingArea *dockingArea;
  std::unique_ptr<OutputWindow> outputWindow;
  std::unique_ptr<InputWindow> textInputWindow;
  ui::ig::Window *logWindow;
    ui::ig::LogPanel<spdlog::level::level_enum, 512> *logPanel;
  // clang-format on
  std::shared_ptr<ui::ig::ImGuiInterface> interface;
};

}  // namespace pf
#endif  //PF_RENDERING_PLAYGROUND_UI_H
