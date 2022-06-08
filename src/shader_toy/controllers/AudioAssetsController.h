//
// Created by Petr on 6/7/2022.
//

#pragma once

#include "../models/AudioAssetsModel.h"
#include "../views/AudioAssetsView.h"
#include "gpu/RenderThread.h"
#include "mvc/Controller.h"
#include "utils/AudioLoader.h"
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include "audio/Context.h"

namespace pf {

class AudioAssetsController : public Controller<AudioAssetsView, AudioAssetsModel> {
 public:
  AudioAssetsController(std::unique_ptr<AudioAssetsView> uiView, std::shared_ptr<AudioAssetsModel> mod,
                        std::shared_ptr<ui::ig::ImGuiInterface> imguiInterface,
                        std::shared_ptr<AudioLoader> audioLoader, std::shared_ptr<gpu::RenderThread> renderingThread,
                        std::shared_ptr<audio::Context> aContext);

  void filterAssetsByName(std::string_view searchStr);

  std::unordered_set<std::string> disallowedNames;

  void showAddAssetDialog();

 private:
  void createUIForAudioModel(const std::shared_ptr<AudioAssetModel> &audioModel);
  std::shared_ptr<ui::ig::ImGuiInterface> interface;
  std::shared_ptr<AudioLoader> loader;
  std::shared_ptr<gpu::RenderThread> renderThread;
  std::shared_ptr<audio::Context> audioContext;

  std::unordered_map<std::shared_ptr<AudioAssetModel>, std::vector<Subscription>> subscriptions;
};

}  // namespace pf
