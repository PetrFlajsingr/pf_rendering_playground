//
// Created by xflajs00 on 16.05.2022.
//

#pragma once

#include "../models/LogModel.h"
#include "../views/LogWindowView.h"
#include "log/UISink.h"
#include "mvc/Controller.h"

namespace pf {

class LogWindowController : public Controller<LogWindowView, LogModel> {
 public:
  LogWindowController(std::unique_ptr<LogWindowView> uiView, std::shared_ptr<LogModel> mod);

  [[nodiscard]] std::shared_ptr<PfImguiLogSink_st> createSpdlogSink();

  void show();
  void hide();
};

}  // namespace pf
