//
// Created by xflajs00 on 18.04.2022.
//

#include "ShaderToyUI.h"
#include "log/UISink.h"
#include <pf_imgui/styles/dark.h>

#undef RGB
namespace pf {

ShaderToyUI::ShaderToyUI(const std::shared_ptr<ui::ig::ImGuiInterface> &imGuiInterface) : interface(imGuiInterface) {
  ui::ig::setDarkStyle(*imGuiInterface);

  dockingArea = &imGuiInterface->createOrGetBackgroundDockingArea();

  outputWindow = std::make_unique<ShaderToyOutputWindow>(*imGuiInterface);
  textInputWindow = std::make_unique<ShaderToyTextInputWindow>(*imGuiInterface);

  logWindow = &imGuiInterface->createWindow("log_window", "Log");
  logWindow->setIsDockable(true);
  logPanel = &logWindow->createChild(ui::ig::LogPanel<spdlog::level::level_enum, 512>::Config{
      .name = "log_panel"});
  logPanel->setCategoryAllowed(spdlog::level::level_enum::n_levels, false);

  logPanel->setCategoryColor(spdlog::level::warn, ui::ig::Color::RGB(255, 213, 97));
  logPanel->setCategoryColor(spdlog::level::err, ui::ig::Color::RGB(173, 23, 23));
  logPanel->setCategoryColor(spdlog::level::info, ui::ig::Color::RGB(44, 161, 21));

  spdlog::default_logger()->sinks().emplace_back(std::make_shared<PfImguiLogSink_st>(*logPanel));

  imGuiInterface->setStateFromConfig();
}

void ShaderToyUI::show() {
  dockingArea->setVisibility(ui::ig::Visibility::Visible);
  outputWindow->window->setVisibility(ui::ig::Visibility::Visible);
  textInputWindow->window->setVisibility(ui::ig::Visibility::Visible);
  logWindow->setVisibility(ui::ig::Visibility::Visible);
}

void ShaderToyUI::hide() {
  dockingArea->setVisibility(ui::ig::Visibility::Invisible);
  outputWindow->window->setVisibility(ui::ig::Visibility::Invisible);
  textInputWindow->window->setVisibility(ui::ig::Visibility::Invisible);
  logWindow->setVisibility(ui::ig::Visibility::Invisible);
}

}// namespace pf