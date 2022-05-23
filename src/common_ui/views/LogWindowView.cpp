//
// Created by xflajs00 on 16.05.2022.
//

#include "LogWindowView.h"

namespace pf {

namespace gui = ui::ig;

// TODO: make persistent when new pf_imgui version is released
LogWindowView::LogWindowView(ui::ig::ImGuiInterface &interface, std::string_view windowName,
                             std::string_view windowTitle)
    : UIViewWindow{&interface.createWindow(std::string{windowName}, std::string{windowTitle})},
      logPanel{&window->createChild(gui::LogPanel<spdlog::level::level_enum, 512>::Config{.name = "log_panel"})} {
  window->setIsDockable(true);

  logPanel->setCategoryAllowed(spdlog::level::level_enum::n_levels, false);
  logPanel->setCategoryAllowed(spdlog::level::level_enum::off, false);

  logPanel->setCategoryColor(spdlog::level::trace, gui::Color::RGB(120, 120, 120));
  logPanel->setCategoryColor(spdlog::level::debug, gui::Color::RGB(235, 161, 52));
  logPanel->setCategoryColor(spdlog::level::info, gui::Color::White);
  logPanel->setCategoryColor(spdlog::level::warn, gui::Color::RGB(255, 213, 97));
  logPanel->setCategoryColor(spdlog::level::err, gui::Color::RGB(173, 23, 23));
  logPanel->setCategoryColor(spdlog::level::critical, gui::Color::RGB(82, 0, 0));
}

}  // namespace pf