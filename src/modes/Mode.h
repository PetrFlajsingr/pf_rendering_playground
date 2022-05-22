//
// Created by xflajs00 on 18.04.2022.
//

#pragma once

#include <pf_common/parallel/ThreadPool.h>
#include <pf_glfw/Window.h>
#include <pf_imgui/ImGuiInterface.h>
#include <spdlog/spdlog.h>

namespace pf {

enum class ModeState { Uninitialised, Initialised, Active };

class Mode {
  friend class ModeManager;

 public:
  virtual ~Mode() = default;

  [[nodiscard]] const toml::table &getConfig();

  [[nodiscard]] ModeState getState() const;

  [[nodiscard]] virtual std::string getName() const = 0;

 protected:
  void initialize(const std::shared_ptr<ui::ig::ImGuiInterface> &imguiInterface,
                  const std::shared_ptr<glfw::Window> &window, toml::table modeConfig,
                  std::shared_ptr<ThreadPool> workerThreads);

  virtual void initialize_impl(const std::shared_ptr<ui::ig::ImGuiInterface> &imguiInterface,
                               const std::shared_ptr<glfw::Window> &window,
                               std::shared_ptr<ThreadPool> workerThreads) = 0;

  virtual std::vector<std::shared_ptr<spdlog::sinks::sink>> createLoggerSinks() = 0;

  void activate();
  virtual void activate_impl() = 0;

  void deactivate();
  virtual void deactivate_impl() = 0;
  void deinitialize();
  virtual void deinitialize_impl() = 0;

  virtual void updateConfig() = 0;

  virtual void render(std::chrono::nanoseconds timeDelta) = 0;

  [[nodiscard]] spdlog::logger &getLogger();

  toml::table config;

 private:
  ModeState state = ModeState::Uninitialised;
  std::shared_ptr<spdlog::logger> logger;
};

}  // namespace pf
