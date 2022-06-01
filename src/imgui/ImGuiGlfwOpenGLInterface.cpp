//
// Created by xflajs00 on 22.10.2021.
//

#include "ImGuiGlfwOpenGLInterface.h"
#include "impl/imgui_impl_glfw.h"
#include "impl/imgui_impl_opengl3.h"
#include <pf_mainloop/MainLoop.h>

namespace pf::ui::ig {

ImGuiGlfwOpenGLInterface::ImGuiGlfwOpenGLInterface(ImGuiGlfwOpenGLConfig config)
    : ImGuiInterface(std::move(config.imgui)), renderThread(std::move(config.renderThread)) {
  ImGui_ImplGlfw_InitForOpenGL(config.windowHandle, true);
  /*renderThread->enqueue([] {*/ ImGui_ImplOpenGL3_Init(); /*});*/
  renderThread->waitForDone();
  updateFonts();
}

ImGuiGlfwOpenGLInterface::~ImGuiGlfwOpenGLInterface() {
  /*renderThread->enqueue([] {*/ ImGui_ImplOpenGL3_Shutdown(); /*});*/
  renderThread->waitForDone();
  ImGui_ImplGlfw_Shutdown();
}

void ImGuiGlfwOpenGLInterface::updateFonts() {
  // no need to implement this for OpenGL
}

void ImGuiGlfwOpenGLInterface::processInput() {
  // done via callbacks
}

void ImGuiGlfwOpenGLInterface::newFrame_impl() {
  /*renderThread->enqueue([] {*/ ImGui_ImplOpenGL3_NewFrame(); /*});*/
  renderThread->waitForDone();
  ImGui_ImplGlfw_NewFrame();
}

void ImGuiGlfwOpenGLInterface::renderDrawData_impl(ImDrawData *drawData) { ImGui_ImplOpenGL3_RenderDrawData(drawData); }

void ImGuiGlfwOpenGLInterface::render() {
  setContext();
  if (shouldUpdateFontAtlas) {
    shouldUpdateFontAtlas = false;
    updateFonts();
  }
  newFrame_impl();
  ImGui::NewFrame();
  RAII endFrameRAII{[&] {
    ImGui::Render();
    const auto drawData = ImGui::GetDrawData();
    /*renderThread->enqueue([this, drawData] {*/ renderDrawData_impl(drawData); /*});*/
    renderThread->waitForDone();
    if (getIo().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
    }
  }};
  if (getVisibility() == Visibility::Visible) {
    // FIXME: auto fontScoped = globalFont.applyScoped();
    if (getEnabled() == Enabled::No) {
      ImGui::BeginDisabled();
      RAII raiiEnabled{ImGui::EndDisabled};
      renderImpl();
    } else {
      renderImpl();
    }
  }
}

}  // namespace pf::ui::ig
