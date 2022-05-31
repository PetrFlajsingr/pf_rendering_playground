//
// Created by Petr on 30/05/2022.
//

#include "MainController.h"
#include <assert.hpp>
#include <pf_imgui/ImGuiInterface.h>

namespace pf {

namespace gui = ui::ig;

MainController::MainController(std::unique_ptr<MainView> uiView, std::shared_ptr<MainModel> mod,
                               std::shared_ptr<ui::ig::ImGuiInterface> imguiInterface,
                               ShaderToyControllers &&controllers)
    : Controller<MainView, MainModel>(std::move(uiView), std::move(mod)),
      outputController(std::move(controllers.output)), logWindowController(std::move(controllers.logWindow)),
      glslEditorController(std::move(controllers.glslEditor)),
      shaderVariablesController(std::move(controllers.shaderVariables)),
      imageAssetsController(std::move(controllers.imageAssets)), interface(std::move(imguiInterface)) {
  VERIFY(model != nullptr);
  VERIFY(view != nullptr);
  VERIFY(logWindowController != nullptr);
  VERIFY(shaderVariablesController != nullptr);
  VERIFY(imageAssetsController != nullptr);
  VERIFY(glslEditorController != nullptr);
  VERIFY(outputController != nullptr);
  VERIFY(interface != nullptr);
}

void MainController::show() {
  view->dockingArea->setVisibility(gui::Visibility::Visible);
  outputController->show();
  logWindowController->show();
  glslEditorController->show();
  shaderVariablesController->show();
  imageAssetsController->show();
}

void MainController::hide() {
  view->dockingArea->setVisibility(gui::Visibility::Invisible);
  outputController->hide();
  logWindowController->hide();
  glslEditorController->hide();
  shaderVariablesController->hide();
  imageAssetsController->hide();
}

void MainController::resetDocking(gui::Size windowSize) {
  VERIFY(logWindowController != nullptr);
  VERIFY(shaderVariablesController != nullptr);
  VERIFY(imageAssetsController != nullptr);
  VERIFY(glslEditorController != nullptr);
  VERIFY(outputController != nullptr);

  const auto dockAreaSize =
      gui::Size{gui::Width{static_cast<float>(windowSize.width)}, gui::Height{static_cast<float>(windowSize.height)}};
  auto &dockBuilder = interface->createDockBuilder(view->dockingArea->getDockSpace());
  dockBuilder.setSize(dockAreaSize);
  dockBuilder.setWindow(outputController->getView().getWindow());

  auto &editorDockBuilder = dockBuilder.split(gui::HorizontalDirection::Right);
  editorDockBuilder.setSplitRatio(0.4f);
  editorDockBuilder.setWindow(glslEditorController->getView().getWindow());
  editorDockBuilder.setWindow(shaderVariablesController->getView().getWindow());
  editorDockBuilder.setWindow(imageAssetsController->getView().getWindow());

  auto &logDockBuilder = dockBuilder.split(gui::VerticalDirection::Down);
  logDockBuilder.setSplitRatio(0.3f);
  logDockBuilder.setWindow(logWindowController->getView().getWindow());
}

}  // namespace pf