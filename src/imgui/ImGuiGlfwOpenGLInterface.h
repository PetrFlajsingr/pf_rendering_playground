//
// Created by xflajs00 on 22.10.2021.
//

#ifndef OPENGL_TEMPLATE_CMAKE_BUILD_DEBUG__DEPS_PF_IMGUI_SRC_SRC_PF_IMGUI_BACKENDS_IMGUIGLFWOPENGLINTERFACE_H
#define OPENGL_TEMPLATE_CMAKE_BUILD_DEBUG__DEPS_PF_IMGUI_SRC_SRC_PF_IMGUI_BACKENDS_IMGUIGLFWOPENGLINTERFACE_H

#include <GLFW/glfw3.h>
#include <pf_imgui/ImGuiInterface.h>
#include "gpu/RenderThread.h"

namespace pf::ui::ig {

struct ImGuiGlfwOpenGLConfig {
  ImGuiConfig imgui;
  GLFWwindow *windowHandle;
  std::shared_ptr<RenderThread> renderThread;
};

class ImGuiGlfwOpenGLInterface final : public ImGuiInterface {
 public:
  ImGuiGlfwOpenGLInterface(ImGuiGlfwOpenGLConfig config);
  virtual ~ImGuiGlfwOpenGLInterface();

  void updateFonts() override;

  void processInput() override;

  void render();

 protected:
  void newFrame_impl() override;
  void renderDrawData_impl(ImDrawData *drawData) override;

 private:
  std::shared_ptr<RenderThread> renderThread;
};

}  // namespace pf::ui::ig
#endif  //OPENGL_TEMPLATE_CMAKE_BUILD_DEBUG__DEPS_PF_IMGUI_SRC_SRC_PF_IMGUI_BACKENDS_IMGUIGLFWOPENGLINTERFACE_H
