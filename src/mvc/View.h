//
// Created by xflajs00 on 15.05.2022.
//

#pragma once

#include <assert.hpp>
#include <pf_imgui/ImGuiInterface.h>
#include <pf_imgui/dialogs/Window.h>
#include <pf_imgui/interface/Layout.h>

namespace pf {

class UIViewBase {
 public:
  inline explicit UIViewBase(std::shared_ptr<ui::ig::ImGuiInterface> imguiInterface)
      : interface(std::move(imguiInterface)) {
    VERIFY(interface != nullptr);
  }
  inline virtual ~UIViewBase() = 0;

 protected:
  std::shared_ptr<ui::ig::ImGuiInterface> interface;
};
UIViewBase::~UIViewBase() = default;

class UIViewWindow : public UIViewBase {
 public:
  explicit inline UIViewWindow(std::shared_ptr<ui::ig::ImGuiInterface> imguiInterface, std::string_view windowName,
                               std::string_view windowTitle)
      : UIViewBase(std::move(imguiInterface)),
        window(&interface->createWindow(std::string{windowName}, std::string{windowTitle})) {}
  inline ~UIViewWindow() override { interface->removeWindow(*window); }
  [[nodiscard]] inline ui::ig::Window &getWindow() { return *window; }
  [[nodiscard]] inline const ui::ig::Window &getWindow() const { return *window; }

 protected:
  ui::ig::Window *window;
};

}  // namespace pf