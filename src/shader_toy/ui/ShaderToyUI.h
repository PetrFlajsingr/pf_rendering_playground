//
// Created by xflajs00 on 18.04.2022.
//

#ifndef PF_RENDERING_PLAYGROUND_SHADERTOYUI_H
#define PF_RENDERING_PLAYGROUND_SHADERTOYUI_H

#include "ShaderToyOutputWindow.h"
#include "ShaderToyTextInputWindow.h"
#include <pf_imgui/ImGuiInterface.h>
#include <pf_imgui/elements/LogPanel.h>
#include <spdlog/spdlog.h>

namespace pf {

struct ShaderToyUI {
  explicit ShaderToyUI(const std::shared_ptr<ui::ig::ImGuiInterface> &imGuiInterface);

  void show();

  void hide();

  // clang-format off
  ui::ig::BackgroundDockingArea *dockingArea;
  std::unique_ptr<ShaderToyOutputWindow> outputWindow;
  std::unique_ptr<ShaderToyTextInputWindow> textInputWindow;
  ui::ig::Window *logWindow;
    ui::ig::LogPanel<spdlog::level::level_enum, 512> *logPanel;
  // clang-format on
  std::shared_ptr<ui::ig::ImGuiInterface> interface;
};

}// namespace pf
#endif//PF_RENDERING_PLAYGROUND_SHADERTOYUI_H
