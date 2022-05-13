//
// Created by xflajs00 on 18.04.2022.
//

#include "ModeManager.h"
#include "spdlog/spdlog.h"

#include <utility>

namespace pf {
ModeManager::ModeManager(std::shared_ptr<ui::ig::ImGuiInterface> imGuiInterface, std::shared_ptr<glfw::Window> window,
                         toml::table config, std::size_t workerThreadCount)
    : imGuiInterface(std::move(imGuiInterface)), window(std::move(window)), config(std::move(config)),
      workerThreads(std::make_shared<ThreadPool>(workerThreadCount)),
      subMenu(this->imGuiInterface->createOrGetMenuBar().createChild(
          ui::ig::SubMenu::Config{.name = "modes_menu", .label = "Modes"})),
      statusBarText(this->imGuiInterface->createOrGetStatusBar().createChild<ui::ig::Text>("mode_mgr_sb_text",
                                                                                           "Current mode: <none>")) {}

ModeManager::~ModeManager() {
  imGuiInterface->createOrGetMenuBar().removeChild(subMenu.getName());
  deactivateModes();
  deinitializeModes();
}

toml::table ModeManager::getConfig() const {
  auto result = config;
  std::ranges::for_each(
      modes, [&result](const auto &mode) { result.insert_or_assign(mode->getName(), mode->getConfig()); },
      &ModeRecord::mode);
  return result;
}

std::optional<ModeManager::Error> ModeManager::addMode(std::shared_ptr<Mode> mode) {
  if (const auto modeOpt = findModeByName(mode->getName()); modeOpt.has_value()) {
    return "Model with the same name is already in ModeManager";
  }
  auto name = mode->getName();
  spdlog::info("[ModeManager] Adding mode '{}'", name);
  auto &menuItem = subMenu.createChild(
      ui::ig::MenuButtonItem::Config{.name = mode->getName() + "_menu_item", .label = mode->getName()});
  menuItem.addClickListener([this, mode] { activateMode(mode); });
  modes.emplace_back(std::move(name), std::move(mode), menuItem);
  return std::nullopt;
}

std::optional<ModeManager::Error> ModeManager::activateMode(const std::string &name) {
  spdlog::info("[ModeManager] Activating mode '{}'", name);
  if (activeMode != nullptr && activeMode->name == name) {
    spdlog::info("[NodeManager] Mode already active '{}'", name);
    return std::nullopt;
  }
  if (const auto modeOpt = findModeByName(name); modeOpt.has_value()) {
    const auto &mode = modeOpt.value();
    activateMode_impl(mode);
    return std::nullopt;
  }
  spdlog::error("[ModeManager] Mode '{}' not managed by ModeManager", name);
  return "Mode not managed by ModeManager";
}

std::optional<ModeManager::Error> ModeManager::activateMode(const std::shared_ptr<Mode> &mode) {
  spdlog::info("[ModeManager] Activating mode '{}'", mode->getName());
  if (activeMode != nullptr && activeMode->mode == mode) {
    spdlog::info("[NodeManager] Mode already active '{}'", mode->getName());
    return std::nullopt;
  }
  if (const auto iter = std::ranges::find(modes, mode, &ModeRecord::mode); iter != modes.end()) {
    const auto mode = &*iter;
    activateMode_impl(mode);
    spdlog::info("[ModeManager] Mode '{}' activated", mode->name);
    return std::nullopt;
  }
  spdlog::error("[ModeManager] Mode '{}' not managed by ModeManager", mode->getName());
  return "Mode not managed by ModeManager";
}

void ModeManager::render(std::chrono::nanoseconds timeDelta) {
  if (activeMode != nullptr) { activeMode->mode->render(timeDelta); }
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

std::optional<ModeManager::ModeRecord *> ModeManager::findModeByName(const std::string &name) {
  if (const auto iter = std::ranges::find(modes, name, &ModeRecord::name); iter != modes.end()) { return &*iter; }
  return std::nullopt;
}

void ModeManager::activateMode_impl(ModeManager::ModeRecord *mode) {
  deactivateModes();
  if (mode->mode->getState() != ModeState::Initialised) {
    auto modeConfig = toml::table{};
    if (const auto iter = config.find(mode->name); iter != config.end()) {
      if (auto configTable = iter->second.as_table(); configTable != nullptr) { modeConfig = *configTable; }
    }
    mode->mode->initialize(imGuiInterface, window, modeConfig, workerThreads);
  }
  if (mode->mode->getState() != ModeState::Active) { mode->mode->activate(); }

  if (activeMode != nullptr) {
    using namespace ui::ig;
    activeMode->buttonItem.setColor<style::ColorOf::Text>(
        Color{ImGui::GetColorU32(static_cast<ImGuiCol>(style::ColorOf::Text))});
  }
  activeMode = mode;
  using namespace ui::ig;
  activeMode->buttonItem.setColor<style::ColorOf::Text>(
      Color{ImGui::GetColorU32(static_cast<ImGuiCol>(style::ColorOf::NavHighlight))});
  statusBarText.setText("Current mode: {}", activeMode->name);
}

}  // namespace pf