//
// Created by xflajs00 on 18.04.2022.
//

#include "ModeManager.h"
#include "spdlog/spdlog.h"

#include "log/UISink.h"
#include "utils/logging.h"
#include <utility>

namespace pf {
namespace gui = ui::ig;
// TODO: ui controller
ModeManager::ModeManager(std::shared_ptr<gui::ImGuiInterface> imGuiInterface, std::shared_ptr<glfw::Window> glfwWindow,
                         std::shared_ptr<RenderThread> renderingThread, toml::table config,
                         std::size_t workerThreadCount)
    : imguiInterface(std::move(imGuiInterface)), window(std::move(glfwWindow)), config(std::move(config)),
      workerThreads(std::make_shared<ThreadPool>(workerThreadCount)), renderThread(std::move(renderingThread)),
      subMenu(imguiInterface->createOrGetMenuBar().addSubmenu("modes_menu", "Modes")),
      showMainLogWindowCheckboxItem(
          subMenu.addCheckboxItem("show_main_log", "Show combined log window", false, gui::Persistent::Yes)),
      subMenuSeparator(subMenu.addSeparator("sep1")),
      statusBarText(
          imguiInterface->createOrGetStatusBar().createChild<gui::Text>("mode_mgr_sb_text", "Current mode: <none>")),
      logger{std::make_shared<spdlog::logger>("NodeManager", pf::log::stdout_color_sink)} {
  VERIFY(imguiInterface != nullptr);
  VERIFY(window != nullptr);

  logWindowController = std::make_unique<LogWindowController>(
      std::make_unique<LogWindowView>(imguiInterface, "combined_log_win", "Combined log"),
      std::make_shared<LogModel>());

  showMainLogWindowCheckboxItem.addValueListener(
      [this](bool value) {
        if (value) {
          logWindowController->show();
        } else {
          logWindowController->hide();
        }
      },
      true);

  logger->sinks().emplace_back(logWindowController->createSpdlogSink());
  logger->sinks().emplace_back(log::createFileSink("ModeManager.log"));
}

ModeManager::~ModeManager() {
  imguiInterface->createOrGetMenuBar().removeChild(subMenu.getName());
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
  if (!ASSERT(mode != nullptr)) { return "Invalid value"; }
  if (const auto modeOpt = findModeByName(mode->getName()); modeOpt.has_value()) {
    return "Model with the same name is already in ModeManager";
  }
  auto name = mode->getName();
  logger->info("Adding mode '{}'", name);
  auto &menuItem = subMenu.createChild(
      gui::MenuButtonItem::Config{.name = mode->getName() + "_menu_item", .label = mode->getName()});
  menuItem.addClickListener([this, mode] { activateMode(mode); });
  modes.emplace_back(std::move(name), std::move(mode), menuItem);
  return std::nullopt;
}

std::optional<ModeManager::Error> ModeManager::activateMode(const std::string &name) {
  logger->info("Activating mode '{}'", name);
  if (activeMode != nullptr && activeMode->name == name) {
    logger->info("[NodeManager] Mode already active '{}'", name);
    return std::nullopt;
  }
  if (const auto modeOpt = findModeByName(name); modeOpt.has_value()) {
    const auto &mode = modeOpt.value();
    activateMode_impl(mode);
    return std::nullopt;
  }
  logger->error("Mode '{}' not managed by ModeManager", name);
  return "Mode not managed by ModeManager";
}

std::optional<ModeManager::Error> ModeManager::activateMode(const std::shared_ptr<Mode> &modeToActivate) {
  if (!ASSERT(modeToActivate != nullptr)) { return "Invalid value"; }
  logger->info("Activating mode '{}'", modeToActivate->getName());
  if (activeMode != nullptr && activeMode->mode == modeToActivate) {
    logger->info("[NodeManager] Mode already active '{}'", modeToActivate->getName());
    return std::nullopt;
  }
  if (const auto iter = std::ranges::find(modes, modeToActivate, &ModeRecord::mode); iter != modes.end()) {
    const auto mode = &*iter;
    activateMode_impl(mode);
    logger->info("Mode '{}' activated", mode->name);
    return std::nullopt;
  }
  logger->error("Mode '{}' not managed by ModeManager", modeToActivate->getName());
  return "Mode not managed by ModeManager";
}

bool ModeManager::isModeActive() const { return activeMode != nullptr; }

void ModeManager::render([[maybe_unused]] std::chrono::nanoseconds timeDelta) {
  if (activeMode != nullptr) { activeMode->mode->render(timeDelta); }
}

void ModeManager::deactivateModes() {
  std::ranges::for_each(modes, [this](auto &mode) {
    if (mode.mode->getState() == ModeState::Active) {
      logger->info("Deactivating mode '{}'", mode.name);
      mode.mode->deactivate();
    }
  });
}

void ModeManager::deinitializeModes() {
  std::ranges::for_each(modes, [this](auto &mode) {
    if (mode.mode->getState() == ModeState::Initialised) {
      logger->info("Deinitializing mode '{}'", mode.name);
      mode.mode->deinitialize();
    }
  });
}

std::optional<ModeManager::ModeRecord *> ModeManager::findModeByName(const std::string &name) {
  if (const auto iter = std::ranges::find(modes, name, &ModeRecord::name); iter != modes.end()) { return &*iter; }
  return std::nullopt;
}

void ModeManager::activateMode_impl(ModeManager::ModeRecord *mode) {
  DEBUG_ASSERT(mode != nullptr);
  deactivateModes();
  if (mode->mode->getState() != ModeState::Initialised) {
    auto modeConfig = toml::table{};
    if (const auto iter = config.find(mode->name); iter != config.end()) {
      if (auto configTable = iter->second.as_table(); configTable != nullptr) { modeConfig = *configTable; }
    }
    mode->mode->initialize(imguiInterface, window, modeConfig, workerThreads, renderThread);
    mode->mode->logger->sinks().emplace_back(logWindowController->createSpdlogSink());
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