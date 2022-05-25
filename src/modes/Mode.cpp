//
// Created by xflajs00 on 22.04.2022.
//

#include "Mode.h"

#include "spdlog/spdlog.h"
#include <utility>

namespace pf {

const toml::table &Mode::getConfig() {
  updateConfig();
  return config;
}

ModeState Mode::getState() const { return state; }

void Mode::initialize(const std::shared_ptr<ui::ig::ImGuiInterface> &imguiInterface,
                      const std::shared_ptr<glfw::Window> &window, toml::table modeConfig,
                      std::shared_ptr<ThreadPool> workerThreads) {
  config = std::move(modeConfig);

  auto sinks = createLoggerSinks();
  logger = std::make_shared<spdlog::logger>(getName(), sinks.begin(), sinks.end());
  spdlog::register_logger(logger);
  logger->set_level(spdlog::level::trace);

  logger->info("Initializing", getName());
  initialize_impl(imguiInterface, window, std::move(workerThreads));
  state = ModeState::Initialised;
  logger->info("Initialized", getName());
}

void Mode::activate() {
  logger->info("Activating", getName());
  activate_impl();
  state = ModeState::Active;
  logger->info("Activated", getName());
}

void Mode::deactivate() {
  logger->info("Deactivating", getName());
  deactivate_impl();
  state = ModeState::Initialised;
  logger->info("Deactivated", getName());
}

void Mode::deinitialize() {
  logger->info("Deinitializing", getName());
  deinitialize_impl();
  state = ModeState::Uninitialised;
  logger->info("Deinitialized", getName());
  spdlog::drop(logger->name());
  logger = nullptr;
}

spdlog::logger &Mode::getLogger() const { return *logger; }

const std::shared_ptr<spdlog::logger> &Mode::getLoggerShared() {
  return logger;
}

}  // namespace pf