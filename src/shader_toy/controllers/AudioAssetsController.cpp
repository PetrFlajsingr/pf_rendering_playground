//
// Created by Petr on 6/7/2022.
//

#include "AudioAssetsController.h"

namespace pf {

AudioAssetsController::AudioAssetsController(std::unique_ptr<AudioAssetsView> uiView, std::shared_ptr<AudioAssetsModel> mod,
                      std::shared_ptr<ui::ig::ImGuiInterface> imguiInterface)
    : Controller<AudioAssetsView, AudioAssetsModel>(std::move(uiView), std::move(mod)),
      interface(std::move(imguiInterface)) {}

}  // namespace pf