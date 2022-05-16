//
// Created by xflajs00 on 16.05.2022.
//

#pragma once

#include "mvc/View.h"
#include <pf_imgui/ImGuiInterface.h>
#include <pf_imgui/elements/LogPanel.h>
#include <spdlog/spdlog.h>

namespace pf {

class LogWindowView : public UIViewWindow {
 public:
  LogWindowView(ui::ig::ImGuiInterface &interface, std::string_view windowName, std::string_view windowTitle);

  // clang-format off
  ui::ig::LogPanel<spdlog::level::level_enum, 512> *logPanel;
  // clang-format on
};

}  // namespace pf