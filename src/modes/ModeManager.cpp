//
// Created by xflajs00 on 18.04.2022.
//

#include "ModeManager.h"
#include "spdlog/spdlog.h"

#include <utility>

namespace pf {
ModeManager::ModeManager(std::shared_ptr<ui::ig::ImGuiInterface> imGuiInterface, std::shared_ptr<glfw::Window> window) : imGuiInterface(std::move(imGuiInterface)), window(std::move(window)) {
}
ModeManager::~ModeManager() {
  deactivateModes();
  deinitializeModes();
}

void ModeManager::addMode(std::string name, std::shared_ptr<Mode> mode) {
  spdlog::info("[ModeManager] Adding mode '{}'", name);
  modes.emplace_back(std::move(name), std::move(mode));
}

std::optional<ModeManager::Error> ModeManager::activateMode(const std::string &name) {
  spdlog::info("[ModeManager] Activating mode '{}'", name);
  if (const auto iter = std::ranges::find(modes, name, &ModeRecord::name); iter != modes.end()) {
    deactivateModes();
    if (!iter->mode->isInitialized()) {
      iter->mode->initialize(imGuiInterface, window);
    }
    if (!iter->mode->isActive()) {
      iter->mode->activate();
    }
    spdlog::info("[ModeManager] Mode activated");
    activeMode = iter->mode;
    return std::nullopt;
  }
  spdlog::error("[ModeManager] Mode not managed by ModeManager");
  return "Mode not managed by ModeManager";
}

std::optional<ModeManager::Error> ModeManager::activateMode(const std::shared_ptr<Mode> &mode) {
  spdlog::info("[ModeManager] Activating mode");
  if (const auto iter = std::ranges::find(modes, mode, &ModeRecord::mode); iter == modes.end()) {
    spdlog::error("[ModeManager] Mode not managed by ModeManager");
    return "Mode not managed by ModeManager";
  }
  deactivateModes();
  if (!mode->isInitialized()) {
    mode->initialize(imGuiInterface, window);
  }
  if (!mode->isActive()) {
    mode->activate();
  }
  activeMode = mode;
  spdlog::info("[ModeManager] Mode '{}' activated");
  return std::nullopt;
}

void ModeManager::render(std::chrono::nanoseconds timeDelta) {
  if (activeMode != nullptr) {
    activeMode->render(timeDelta);
  }
}

void ModeManager::deactivateModes() {
  std::ranges::for_each(modes, [](auto &mode) {
    if (mode.mode->isActive()) {
      spdlog::info("[ModeManager] Deactivating mode '{}'", mode.name);
      mode.mode->deactivate();
    }
  });
}

void ModeManager::deinitializeModes() {
  std::ranges::for_each(modes, [](auto &mode) {
    if (mode.mode->isInitialized()) {
      spdlog::info("[ModeManager] Deinitializing mode '{}'", mode.name);
      mode.mode->deinitialize();
    }
  });
}

}// namespace pf