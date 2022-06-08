//
// Created by Petr on 30/05/2022.
//

#pragma once

#include "../models/MainModel.h"
#include "../views/MainView.h"
#include "GlslEditorController.h"
#include "ImageAssetsController.h"
#include "OutputController.h"
#include "ShaderVariablesController.h"
#include "AudioAssetsController.h"
#include "common_ui/controllers/LogWindowController.h"
#include "mvc/Controller.h"

namespace pf {

struct ShaderToyControllers {
  std::unique_ptr<OutputController> output;
  std::unique_ptr<LogWindowController> logWindow;
  std::unique_ptr<GlslEditorController> glslEditor;
  std::unique_ptr<ShaderVariablesController> shaderVariables;
  std::unique_ptr<ImageAssetsController> imageAssets;
  std::unique_ptr<AudioAssetsController> audioAssets;
};

// TODO: MainModel -> ShaderToyModel containing all other models?
class MainController : public Controller<MainView, MainModel> {
 public:
  MainController(std::unique_ptr<MainView> uiView, std::shared_ptr<MainModel> mod,
                 std::shared_ptr<ui::ig::ImGuiInterface> imguiInterface, ShaderToyControllers &&controllers);

  std::unique_ptr<OutputController> outputController{};
  std::unique_ptr<LogWindowController> logWindowController{};
  std::unique_ptr<GlslEditorController> glslEditorController{};
  std::unique_ptr<ShaderVariablesController> shaderVariablesController{};
  std::unique_ptr<ImageAssetsController> imageAssetsController{};
  std::unique_ptr<AudioAssetsController> audioAssetsController{};

  void show();
  void hide();

  void resetDocking(ui::ig::Size windowSize);

 private:
  std::shared_ptr<ui::ig::ImGuiInterface> interface;
};

}  // namespace pf
