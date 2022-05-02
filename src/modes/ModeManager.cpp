//
// Created by xflajs00 on 18.04.2022.
//

#include "ModeManager.h"
#include "spdlog/spdlog.h"

#include <utility>

namespace pf {
ModeManager::ModeManager(std::shared_ptr<ui::ig::ImGuiInterface> imGuiInterface, std::shared_ptr<glfw::Window> window)
    : imGuiInterface(std::move(imGuiInterface)), window(std::move(window)),
      subMenu(this->imGuiInterface->getMenuBar().createChild(
          ui::ig::SubMenu::Config{.name = "modes_menu", .label = "Modes"})) {}

ModeManager::~ModeManager() {
  imGuiInterface->getMenuBar().removeChild(subMenu.getName());
  deactivateModes();
  deinitializeModes();
}

std::optional<ModeManager::Error> ModeManager::addMode(std::shared_ptr<Mode> mode) {
  if (const auto modeOpt = findModeByName(mode->getName()); modeOpt.has_value()) {
    return "Model with the same name is already in ModeManager";
  }
  auto name = mode->getName();
  spdlog::info("[ModeManager] Adding mode '{}'", name);
  subMenu.createChild(ui::ig::MenuButtonItem::Config{.name = mode->getName() + "_menu_item", .label = mode->getName()})
      .addClickListener([this, mode] { activateMode(mode); });
  modes.emplace_back(std::move(name), std::move(mode));
  return std::nullopt;
}

std::optional<ModeManager::Error> ModeManager::activateMode(const std::string &name) {
  spdlog::info("[ModeManager] Activating mode '{}'", name);
  if (activeMode != nullptr && activeMode->getName() == name) {
    spdlog::info("[NodeManager] Mode already active '{}'", name);
    return std::nullopt;
  }
  if (const auto modeOpt = findModeByName(name); modeOpt.has_value()) {
    const auto &mode = modeOpt.value();
    deactivateModes();
    if (mode->getState() != ModeState::Initialised) { mode->initialize(imGuiInterface, window); }
    if (mode->getState() != ModeState::Active) { mode->activate(); }
    spdlog::info("[ModeManager] Mode activated '{}'", name);
    activeMode = mode;
    return std::nullopt;
  }
  spdlog::error("[ModeManager] Mode '{}' not managed by ModeManager", name);
  return "Mode not managed by ModeManager";
}

std::optional<ModeManager::Error> ModeManager::activateMode(const std::shared_ptr<Mode> &mode) {
  spdlog::info("[ModeManager] Activating mode '{}'", mode->getName());
  if (activeMode != nullptr && activeMode == mode) {
    spdlog::info("[NodeManager] Mode already active '{}'", mode->getName());
    return std::nullopt;
  }
  if (const auto iter = std::ranges::find(modes, mode, &ModeRecord::mode); iter == modes.end()) {
    spdlog::error("[ModeManager] Mode '{}' not managed by ModeManager", mode->getName());
    return "Mode not managed by ModeManager";
  }
  deactivateModes();
  if (mode->getState() != ModeState::Initialised) { mode->initialize(imGuiInterface, window); }
  if (mode->getState() != ModeState::Active) { mode->activate(); }
  activeMode = mode;
  spdlog::info("[ModeManager] Mode '{}' activated", mode->getName());
  return std::nullopt;
}

void ModeManager::render(std::chrono::nanoseconds timeDelta) {
  if (activeMode != nullptr) { activeMode->render(timeDelta); }
}

void ModeManager::deactivateModes() {
  std::ranges::for_each(modes, [](auto &mode) {
    if (mode.mode->getState() == ModeState::Active) {
      spdlog::info("[ModeManager] Deactivating mode '{}'", mode.name);
      mode.mode->deactivate();
    }
  });
}

void ModeManager::deinitializeModes() {
  std::ranges::for_each(modes, [](auto &mode) {
    if (mode.mode->getState() == ModeState::Initialised) {
      spdlog::info("[ModeManager] Deinitializing mode '{}'", mode.name);
      mode.mode->deinitialize();
    }
  });
}

std::optional<std::shared_ptr<Mode>> ModeManager::findModeByName(const std::string &name) {
  if (const auto iter = std::ranges::find(modes, name, &ModeRecord::name); iter != modes.end()) { return iter->mode; }
  return std::nullopt;
}

}  // namespace pf