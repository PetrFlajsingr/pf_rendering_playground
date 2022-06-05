//
// Created by Petr on 02/06/2022.
//

#pragma once

#include "../RenderThread.h"
#include <pf_glfw/Window.h>

namespace pf::gpu {

class OpenGlRenderThread : public RenderThread {
 public:
  OpenGlRenderThread(std::shared_ptr<glfw::Window> window) : context(std::move(window)) {}
  ~OpenGlRenderThread() override = default;

  void waitForDone() override;

 private:
  std::shared_ptr<glfw::Window> context;
};

}  // namespace pf::gpu
