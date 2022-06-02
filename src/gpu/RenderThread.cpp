//
// Created by Petr on 01/06/2022.
//

#include "RenderThread.h"

namespace pf {

void SimpleRenderCommand::execute() { job(); }

RenderThread::RenderThread() : thread(&RenderThread::run, this) {}

RenderThread::~RenderThread() {
  shutdown();
  thread.join();
}

void RenderThread::enqueue(std::unique_ptr<RenderCommand> command) { commandQueue.enqueue(std::move(command)); }

void RenderThread::shutdown() { commandQueue.shutdown(); }

void RenderThread::runImpl() {
  while (true) {
    auto command = commandQueue.dequeue();
    if (command.has_value()) {
      command.value()->execute();
    } else {
      return;
    }
  }
}

void RenderThread::run() { runImpl(); }

}  // namespace pf