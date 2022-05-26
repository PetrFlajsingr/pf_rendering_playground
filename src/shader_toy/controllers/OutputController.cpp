//
// Created by Petr on 26/05/2022.
//

#include "OutputController.h"

namespace pf {
OutputController::OutputController(std::unique_ptr<OutputView> uiView, std::shared_ptr<OutputModel> mod)
    : Controller<OutputView, OutputModel>(std::move(uiView), std::move(mod)) {}
}  // namespace pf