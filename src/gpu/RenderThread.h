//
// Created by Petr on 01/06/2022.
//

#pragma once

#include <concepts>
#include <functional>
#include <future>
#include <pf_common/parallel/SafeQueue.h>
#include <thread>
#include <vector>

namespace pf::gpu {

struct RenderCommand {
  RenderCommand() = default;
  virtual ~RenderCommand() = 0;
  virtual void execute() = 0;
};
class SimpleRenderCommand : public RenderCommand {
 public:
  explicit SimpleRenderCommand(std::invocable auto &&fnc) : job(std::forward<decltype(fnc)>(fnc)) {}
  ~SimpleRenderCommand() override = default;

  void execute() override;

 private:
  std::function<void()> job;
};

inline RenderCommand::~RenderCommand() = default;

class RenderThread {
 public:
  RenderThread();
  virtual ~RenderThread();

  void enqueue(std::invocable auto &&fnc) {
    commandQueue.enqueue(std::make_unique<SimpleRenderCommand>(std::forward<decltype(fnc)>(fnc)));
  }
  void enqueue(std::unique_ptr<RenderCommand> command);

  // waits for last command currently in the queue to be finished
  virtual void waitForDone() = 0;

  void shutdown();

 protected:
  virtual void runImpl();
  // TODO: use a more efficient queue (same goes for ThreadPool, just replace it in pf_common with concurrentqueue or something)
  SafeQueue<std::unique_ptr<RenderCommand>> commandQueue;

 private:
  void run();

  std::thread thread;
};

}  // namespace pf
