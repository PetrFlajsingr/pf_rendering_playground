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
#include <pf_glfw/Window.h>

namespace pf::shader_toy {

struct UI {
  UI(std::shared_ptr<ui::ig::ImGuiInterface> imGuiInterface, glfw::Window &window, const std::string &initShaderCode,
     const std::filesystem::path &resourcesPath, bool initializeDocking);

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

}  // namespace pf::shader_toy
#endif  //PF_RENDERING_PLAYGROUND_UI_H
