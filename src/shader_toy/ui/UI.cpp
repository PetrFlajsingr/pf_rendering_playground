//
// Created by xflajs00 on 18.04.2022.
//

#include "UI.h"
#include <pf_imgui/styles/dark.h>

#include <utility>

namespace pf::shader_toy {

namespace gui = ui::ig;
UI::UI(std::shared_ptr<gui::ImGuiInterface> imGuiInterface, glfw::Window &window,
       [[maybe_unused]] const std::string &initShaderCode, const std::filesystem::path &resourcesPath,
       bool initializeDocking, std::shared_ptr<ImageLoader> imageLoader)
    : interface(std::move(imGuiInterface)) {
  gui::setDarkStyle(*interface);

  dockingArea = &interface->createOrGetBackgroundDockingArea();

  // TODO: textInputWindow->editor->setText(initShaderCode);

  logWindowController = std::make_unique<LogWindowController>(
      std::make_unique<LogWindowView>(*interface, "log_window", "Log"), std::make_shared<LogModel>());

  shaderVariablesController = std::make_unique<ShaderVariablesController>(
      std::make_unique<ShaderVariablesWindowView>(*interface, "shader_vars_win", "Shader variables"),
      std::make_shared<ShaderVariablesModel>(), interface);

  imageAssetsController = std::make_unique<ImageAssetsController>(
      std::make_unique<ImageAssetsView>(*interface, "image_assets_win", "Images"),
      std::make_shared<UserImageAssetsModel>(), interface, imageLoader);

  glslEditorController = std::make_unique<GlslEditorController>(
      std::make_unique<GlslEditorView>(*interface, "glsl_editor_win", "Code"),
      std::make_shared<GlslEditorModel>(true, std::chrono::milliseconds{1000}, false, "test code boi"));

  outputController =
      std::make_unique<OutputController>(std::make_unique<OutputView>(*interface, "output_win", "Output"),
                                         std::make_shared<OutputModel>(std::pair{2048u, 1024u}, nullptr));

  const auto fontPath = resourcesPath / "fonts" / "RobotoMono-Regular.ttf";
  if (std::filesystem::exists(fontPath)) {
    auto codeFont = interface->getFontManager().fontBuilder("RobotoMono-Regular", fontPath).setFontSize(15.f).build();
    // TODO: textInputWindow->editor->setFont(codeFont);
  }

  if (initializeDocking) {
    // TODO: fix this up
    const auto windowSize = window.getSize();
    const auto dockAreaSize =
        gui::Size{gui::Width{static_cast<float>(windowSize.width)}, gui::Height{static_cast<float>(windowSize.height)}};
    auto &dockBuilder = interface->createDockBuilder(dockingArea->getDockSpace());
    dockBuilder.setSize(dockAreaSize);
    //dockBuilder.setWindow(*outputWindow->window);

    auto &editorDockBuilder = dockBuilder.split(gui::HorizontalDirection::Right);
    editorDockBuilder.setSplitRatio(0.4f);
    //editorDockBuilder.setWindow(*textInputWindow->window);

    auto &logDockBuilder = dockBuilder.split(gui::VerticalDirection::Down);
    logDockBuilder.setSplitRatio(0.3f);
    logDockBuilder.setWindow(logWindowController->getView().getWindow());
  }

  interface->setStateFromConfig();
}

void UI::show() {
  // TODO: add new views/controllers here
  dockingArea->setVisibility(gui::Visibility::Visible);
  logWindowController->show();
}

void UI::hide() {
  // TODO: add new views/controllers here
  dockingArea->setVisibility(gui::Visibility::Invisible);
  logWindowController->hide();
}

}  // namespace pf::shader_toy