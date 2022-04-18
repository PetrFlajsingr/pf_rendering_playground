//
// Created by xflajs00 on 22.10.2021.
//

#ifndef OPENGL_TEMPLATE_SRC_UI_DEMOIMGUI_H
#define OPENGL_TEMPLATE_SRC_UI_DEMOIMGUI_H

#include "OutputWindow.h"
#include "TextInputWindow.h"
#include "spdlog/common.h"
#include <GLFW/glfw3.h>
#include <pf_glfw/Window.h>
#include <pf_imgui/ImGuiInterface.h>
#include <pf_imgui/elements/LogPanel.h>
#include <toml++/toml.h>

namespace gui = pf::ui::ig;

namespace pf::ogl {

/**
 * @brief Simple demo UI.
 */
class UI {
 public:
  UI(const toml::table &config, const std::shared_ptr<glfw::Window> &window);

  // clang-format off
  gui::AppMenuBar *appMenuBar;
  gui::BackgroundDockingArea *dockingArea;
  std::unique_ptr<OutputWindow> outputWindow;
  std::unique_ptr<TextInputWindow> textInputWindow;
  gui::Window *logWindow;
    gui::LogPanel<spdlog::level::level_enum, 512> *logPanel;
  // clang-format on

  std::unique_ptr<ui::ig::ImGuiInterface> imguiInterface;
};

}// namespace pf::ogl

#endif//OPENGL_TEMPLATE_SRC_UI_DEMOIMGUI_H
