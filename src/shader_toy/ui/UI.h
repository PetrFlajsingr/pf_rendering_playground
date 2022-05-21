//
// Created by xflajs00 on 18.04.2022.
//

#pragma once

#include "GlobalVariablesPanel.h"
#include "InputWindow.h"
#include "OutputWindow.h"
#include <pf_glfw/Window.h>
#include <pf_imgui/ImGuiInterface.h>
#include <pf_imgui/elements/LogPanel.h>
#include <spdlog/spdlog.h>

#include "common_ui/controllers/LogWindowController.h"
#include "../controllers/ShaderVariablesController.h"

namespace pf::shader_toy {

struct UI {
  UI(std::shared_ptr<ui::ig::ImGuiInterface> imGuiInterface, glfw::Window &window,
     std::unique_ptr<ImageLoader> imageLoader, const std::string &initShaderCode,
     const std::filesystem::path &resourcesPath, bool initializeDocking);

  void show();

  void hide();

  // clang-format off
  ui::ig::BackgroundDockingArea *dockingArea;
  std::unique_ptr<OutputWindow> outputWindow;
  std::unique_ptr<InputWindow> textInputWindow;

  std::unique_ptr<LogWindowController> logWindowController;
  std::unique_ptr<ShaderVariablesController> shaderVariablesController;
  // clang-format on
  std::shared_ptr<ui::ig::ImGuiInterface> interface;
};

}  // namespace pf::shader_toy
