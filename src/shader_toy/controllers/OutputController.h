//
// Created by Petr on 26/05/2022.
//

#pragma once

#include "../models/OutputModel.h"
#include "../views/OutputView.h"
#include "mvc/Controller.h"

namespace pf {

class OutputController : public Controller<OutputView, OutputModel> {
 public:
  OutputController(std::unique_ptr<OutputView> uiView, std::shared_ptr<OutputModel> mod);
};

}  // namespace pf
