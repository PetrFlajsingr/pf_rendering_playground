//
// Created by xflajs00 on 16.05.2022.
//

#include "LogWindowController.h"

namespace pf {
namespace gui = ui::ig;

LogWindowController::LogWindowController(std::unique_ptr<LogWindowView> uiView, std::shared_ptr<LogModel> mod)
    : Controller(std::move(uiView), std::move(mod)) {}

std::shared_ptr<PfImguiLogSink_mt> LogWindowController::createSpdlogSink() {
  return std::make_shared<PfImguiLogSink_mt>(*view->logPanel);
}

void LogWindowController::show() { view->getWindow().setVisibility(gui::Visibility::Visible); }

void LogWindowController::hide() { view->getWindow().setVisibility(gui::Visibility::Invisible); }

}  // namespace pf