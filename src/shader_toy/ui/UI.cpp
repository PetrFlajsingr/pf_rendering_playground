//
// Created by xflajs00 on 18.04.2022.
//

#include "UI.h"
#include <pf_imgui/styles/dark.h>

#include <utility>

namespace pf::shader_toy {

namespace gui = ui::ig;
UI::UI(std::shared_ptr<gui::ImGuiInterface> imGuiInterface, glfw::Window &window, const std::string &initShaderCode,
       const std::filesystem::path &resourcesPath, bool initializeDocking, std::shared_ptr<ImageLoader> imageLoader)
    : interface(std::move(imGuiInterface)) {
  gui::setDarkStyle(*interface);

  dockingArea = &interface->createOrGetBackgroundDockingArea();

  outputWindow = std::make_unique<OutputWindow>(*interface);
  textInputWindow = std::make_unique<InputWindow>(*interface);

  textInputWindow->editor->setText(initShaderCode);

  logWindowController = std::make_unique<LogWindowController>(
      std::make_unique<LogWindowView>(*interface, "log_window", "Log"), std::make_shared<LogModel>());

  shaderVariablesController = std::make_unique<ShaderVariablesController>(
      std::make_unique<ShaderVariablesWindowView>(*interface, "shader_vars_win", "Shader variables"),
      std::make_shared<ShaderVariablesModel>(), interface);

  imageAssetsController = std::make_unique<ImageAssetsController>(
      std::make_unique<ImageAssetsView>(*interface, "image_assets_win", "Images"),
      std::make_shared<UserImageAssetsModel>(), interface, imageLoader);

  const auto fontPath = resourcesPath / "fonts" / "RobotoMono-Regular.ttf";
  if (std::filesystem::exists(fontPath)) {
    auto codeFont = interface->getFontManager().fontBuilder("RobotoMono-Regular", fontPath).setFontSize(15.f).build();
    textInputWindow->editor->setFont(codeFont);
  }

  if (initializeDocking) {
    const auto windowSize = window.getSize();
    const auto dockAreaSize =
        gui::Size{gui::Width{static_cast<float>(windowSize.width)}, gui::Height{static_cast<float>(windowSize.height)}};
    auto &dockBuilder = interface->createDockBuilder(dockingArea->getDockSpace());
    dockBuilder.setSize(dockAreaSize);
    dockBuilder.setWindow(*outputWindow->window);

    auto &editorDockBuilder = dockBuilder.split(gui::HorizontalDirection::Right);
    editorDockBuilder.setSplitRatio(0.4f);
    editorDockBuilder.setWindow(*textInputWindow->window);

    auto &logDockBuilder = dockBuilder.split(gui::VerticalDirection::Down);
    logDockBuilder.setSplitRatio(0.3f);
    logDockBuilder.setWindow(logWindowController->getView().getWindow());
  }

  interface->setStateFromConfig();
}

void UI::show() {
  dockingArea->setVisibility(gui::Visibility::Visible);
  outputWindow->window->setVisibility(gui::Visibility::Visible);
  textInputWindow->window->setVisibility(gui::Visibility::Visible);
  logWindowController->show();
}

void UI::hide() {
  dockingArea->setVisibility(gui::Visibility::Invisible);
  outputWindow->window->setVisibility(gui::Visibility::Invisible);
  textInputWindow->window->setVisibility(gui::Visibility::Invisible);
  logWindowController->hide();
}

}  // namespace pf::shader_toy