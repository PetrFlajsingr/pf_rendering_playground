//
// Created by Petr on 26/05/2022.
//

#pragma once

#include "../models/OutputModel.h"
#include "../views/OutputView.h"
#include "mvc/Controller.h"

namespace pf {

class OutputController : public Controller<OutputView, OutputModel> {
  constexpr static auto IMAGE_SIZES = make_array(1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 9182);

 public:
  OutputController(std::unique_ptr<OutputView> uiView, std::shared_ptr<OutputModel> mod);

  void setFps(float currentFps, float averageFps);
};

}  // namespace pf
