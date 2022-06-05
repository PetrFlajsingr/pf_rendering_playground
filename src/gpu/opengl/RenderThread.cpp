//
// Created by Petr on 02/06/2022.
//

#include "RenderThread.h"
#include <glad/glad.h>

namespace pf::gpu {
void OpenGlRenderThread::waitForDone() {
  auto promise = std::make_shared<std::promise<void>>();
  auto fence = promise->get_future();
  enqueue([promise = std::move(promise)] {
    glFinish();
    promise->set_value();
  });

  fence.get();
}

}  // namespace pf::gpu