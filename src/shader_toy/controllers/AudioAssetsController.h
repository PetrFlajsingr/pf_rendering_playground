//
// Created by Petr on 6/7/2022.
//

#pragma once

#include "../models/AudioAssetsModel.h"
#include "../views/AudioAssetsView.h"
#include "mvc/Controller.h"

namespace pf {

class AudioAssetsController : public Controller<AudioAssetsView, AudioAssetsModel> {
 public:
  AudioAssetsController(std::unique_ptr<AudioAssetsView> uiView, std::shared_ptr<AudioAssetsModel> mod,
                        std::shared_ptr<ui::ig::ImGuiInterface> imguiInterface);

 private:
  std::shared_ptr<ui::ig::ImGuiInterface> interface;
};

}  // namespace pf
