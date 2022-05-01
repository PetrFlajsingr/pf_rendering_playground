//
// Created by xflajs00 on 18.04.2022.
//

#include "UI.h"
#include "log/UISink.h"
#include <pf_imgui/styles/dark.h>

#undef RGB
namespace pf::shader_toy {

namespace gui = ui::ig;
UI::UI(const std::shared_ptr<gui::ImGuiInterface> &imGuiInterface, const std::string &initShaderCode)
    : interface(imGuiInterface) {
  gui::setDarkStyle(*imGuiInterface);

  dockingArea = &imGuiInterface->createOrGetBackgroundDockingArea();

  outputWindow = std::make_unique<OutputWindow>(*imGuiInterface);
  textInputWindow = std::make_unique<InputWindow>(*imGuiInterface);

  textInputWindow->editor->setText(initShaderCode);

  logWindow = &imGuiInterface->createWindow("log_window", "Log");
  logWindow->setIsDockable(true);
  logPanel = &logWindow->createChild(gui::LogPanel<spdlog::level::level_enum, 512>::Config{.name = "log_panel"});
  logPanel->setCategoryAllowed(spdlog::level::level_enum::n_levels, false);

  logPanel->setCategoryColor(spdlog::level::warn, gui::Color::RGB(255, 213, 97));
  logPanel->setCategoryColor(spdlog::level::err, gui::Color::RGB(173, 23, 23));
  logPanel->setCategoryColor(spdlog::level::info, gui::Color::RGB(44, 161, 21));
  logPanel->setCategoryColor(spdlog::level::debug, gui::Color::RGB(235, 161, 52));

  spdlog::default_logger()->sinks().emplace_back(std::make_shared<PfImguiLogSink_st>(*logPanel));

  imGuiInterface->setStateFromConfig();
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

}  // namespace pf