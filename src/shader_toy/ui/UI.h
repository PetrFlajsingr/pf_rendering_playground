//
// Created by xflajs00 on 18.04.2022.
//

#pragma once

#include "InputWindow.h"
#include "OutputWindow.h"
#include <pf_glfw/Window.h>
#include <pf_imgui/ImGuiInterface.h>
#include <pf_imgui/elements/LogPanel.h>
#include <spdlog/spdlog.h>

#include "../controllers/ShaderVariablesController.h"
#include "common_ui/controllers/LogWindowController.h"
#include "shader_toy/controllers/ImageAssetsController.h"

namespace pf::shader_toy {

struct UI {
  UI(std::shared_ptr<ui::ig::ImGuiInterface> imGuiInterface, glfw::Window &window, const std::string &initShaderCode,
     const std::filesystem::path &resourcesPath, bool initializeDocking, std::shared_ptr<ImageLoader> imageLoader);

  void show();

  void hide();

  // clang-format off
  ui::ig::BackgroundDockingArea *dockingArea;
  std::unique_ptr<OutputWindow> outputWindow;
  std::unique_ptr<InputWindow> textInputWindow;

  std::unique_ptr<LogWindowController> logWindowController;
  std::unique_ptr<ShaderVariablesController> shaderVariablesController;
  std::unique_ptr<ImageAssetsController> imageAssetsController;
  // clang-format on
  std::shared_ptr<ui::ig::ImGuiInterface> interface;
};

}  // namespace pf::shader_toy
