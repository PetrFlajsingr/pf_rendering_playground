//
// Created by xflajs00 on 18.04.2022.
//

#include "ShaderToyUI.h"
#include "log/UISink.h"
#include <pf_imgui/styles/dark.h>

#undef RGB
namespace pf {

namespace gui = ui::ig;
ShaderToyUI::ShaderToyUI(const std::shared_ptr<gui::ImGuiInterface> &imGuiInterface, const std::string &initShaderCode)
    : interface(imGuiInterface) {
  gui::setDarkStyle(*imGuiInterface);

  dockingArea = &imGuiInterface->createOrGetBackgroundDockingArea();

  outputWindow = std::make_unique<ShaderToyOutputWindow>(*imGuiInterface);
  textInputWindow = std::make_unique<ShaderToyTextInputWindow>(*imGuiInterface);

  textInputWindow->editor->setText(initShaderCode);

  logWindow = &imGuiInterface->createWindow("log_window", "Log");
  logWindow->setIsDockable(true);
  logPanel = &logWindow->createChild(gui::LogPanel<spdlog::level::level_enum, 512>::Config{.name = "log_panel"});
  logPanel->setCategoryAllowed(spdlog::level::level_enum::n_levels, false);

  logPanel->setCategoryColor(spdlog::level::warn, gui::Color::RGB(255, 213, 97));
  logPanel->setCategoryColor(spdlog::level::err, gui::Color::RGB(173, 23, 23));
  logPanel->setCategoryColor(spdlog::level::info, gui::Color::RGB(44, 161, 21));

  spdlog::default_logger()->sinks().emplace_back(std::make_shared<PfImguiLogSink_st>(*logPanel));

  imGuiInterface->setStateFromConfig();
}

void ShaderToyUI::show() {
  dockingArea->setVisibility(gui::Visibility::Visible);
  outputWindow->window->setVisibility(gui::Visibility::Visible);
  textInputWindow->window->setVisibility(gui::Visibility::Visible);
  logWindow->setVisibility(gui::Visibility::Visible);
}

void ShaderToyUI::hide() {
  dockingArea->setVisibility(gui::Visibility::Invisible);
  outputWindow->window->setVisibility(gui::Visibility::Invisible);
  textInputWindow->window->setVisibility(gui::Visibility::Invisible);
  logWindow->setVisibility(gui::Visibility::Invisible);
}

}  // namespace pf