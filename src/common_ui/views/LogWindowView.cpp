//
// Created by xflajs00 on 16.05.2022.
//

#include "LogWindowView.h"

namespace pf {

namespace gui = ui::ig;

LogWindowView::LogWindowView(std::shared_ptr<ui::ig::ImGuiInterface> imguiInterface, std::string_view windowName,
                             std::string_view windowTitle)
    : UIViewWindow{std::move(imguiInterface), windowName, windowTitle},
      logPanel{&window->createChild(
          gui::LogPanel<spdlog::level::level_enum, 512>::Config{.name = std::string{windowName} + "_log_panel",
                                                                .persistent = true})} {
  window->setIsDockable(true);

  logPanel->setCategoryAllowed(spdlog::level::level_enum::n_levels, false);
  logPanel->setCategoryAllowed(spdlog::level::level_enum::off, false);

  logPanel->setCategoryTextColor(spdlog::level::trace, gui::Color::RGB(120, 120, 120));
  logPanel->setCategoryTextColor(spdlog::level::debug, gui::Color::RGB(235, 161, 52));
  logPanel->setCategoryTextColor(spdlog::level::info, gui::Color::White);
  logPanel->setCategoryTextColor(spdlog::level::warn, gui::Color::RGB(255, 213, 97));
  logPanel->setCategoryTextColor(spdlog::level::err, gui::Color::RGB(173, 23, 23));
  logPanel->setCategoryTextColor(spdlog::level::critical, gui::Color::RGB(82, 0, 0));
}

}  // namespace pf