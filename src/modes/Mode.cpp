//
// Created by xflajs00 on 22.04.2022.
//

#include "Mode.h"

#include "spdlog/spdlog.h"
#include <utility>

namespace pf {

const toml::table &Mode::getConfig() const { return config; }

ModeState Mode::getState() const { return state; }

void Mode::initialize(const std::shared_ptr<ui::ig::ImGuiInterface> &imguiInterface,
                      const std::shared_ptr<glfw::Window> &window, toml::table modeConfig,
                      std::shared_ptr<ThreadPool> workerThreads) {
  config = std::move(modeConfig);
  spdlog::info("[{}] Initializing", getName());
  initialize_impl(imguiInterface, window, std::move(workerThreads));
  state = ModeState::Initialised;
  spdlog::info("[{}] Initialized", getName());
}

void Mode::activate() {
  spdlog::info("[{}] Activating", getName());
  activate_impl();
  state = ModeState::Active;
  spdlog::info("[{}] Activated", getName());
}

void Mode::deactivate() {
  spdlog::info("[{}] Deactivating", getName());
  deactivate_impl();
  state = ModeState::Initialised;
  spdlog::info("[{}] Deactivated", getName());
}

void Mode::deinitialize() {
  spdlog::info("[{}] Deinitializing", getName());
  deinitialize_impl();
  state = ModeState::Uninitialised;
  spdlog::info("[{}] Deinitialized", getName());
}

}  // namespace pf