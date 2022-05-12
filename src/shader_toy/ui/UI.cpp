//
// Created by xflajs00 on 18.04.2022.
//

#include "UI.h"
#include "log/UISink.h"
#include <pf_imgui/styles/dark.h>

#include <utility>

namespace pf::shader_toy {

namespace gui = ui::ig;
UI::UI(std::shared_ptr<gui::ImGuiInterface> imGuiInterface, glfw::Window &window,
       std::unique_ptr<ImageLoader> imageLoader, const std::string &initShaderCode,
       const std::filesystem::path &resourcesPath, bool initializeDocking)
    : interface(std::move(imGuiInterface)) {
  gui::setDarkStyle(*interface);

  dockingArea = &interface->createOrGetBackgroundDockingArea();

  outputWindow = std::make_unique<OutputWindow>(*interface);
  textInputWindow = std::make_unique<InputWindow>(*interface, std::move(imageLoader));

  textInputWindow->editor->setText(initShaderCode);

  logWindow = &interface->createWindow("log_window", "Log");
  logWindow->setIsDockable(true);
  logPanel = &logWindow->createChild(gui::LogPanel<spdlog::level::level_enum, 512>::Config{.name = "log_panel"});
  logPanel->setCategoryAllowed(spdlog::level::level_enum::n_levels, false);

  logPanel->setCategoryColor(spdlog::level::warn, gui::Color::RGB(255, 213, 97));
  logPanel->setCategoryColor(spdlog::level::err, gui::Color::RGB(173, 23, 23));
  logPanel->setCategoryColor(spdlog::level::info, gui::Color::RGB(44, 161, 21));
  logPanel->setCategoryColor(spdlog::level::debug, gui::Color::RGB(235, 161, 52));

  spdlog::default_logger()->sinks().emplace_back(std::make_shared<PfImguiLogSink_st>(*logPanel));

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
    logDockBuilder.setWindow(*logWindow);
  }

  interface->setStateFromConfig();
}

void UI::show() {
  dockingArea->setVisibility(gui::Visibility::Visible);
  outputWindow->window->setVisibility(gui::Visibility::Visible);
  textInputWindow->window->setVisibility(gui::Visibility::Visible);
  logWindow->setVisibility(gui::Visibility::Visible);
}

void UI::hide() {
  dockingArea->setVisibility(gui::Visibility::Invisible);
  outputWindow->window->setVisibility(gui::Visibility::Invisible);
  textInputWindow->window->setVisibility(gui::Visibility::Invisible);
  logWindow->setVisibility(gui::Visibility::Invisible);
}

}  // namespace pf::shader_toy