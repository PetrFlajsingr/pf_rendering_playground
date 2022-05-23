//
// Created by Petr on 23/05/2022.
//

#include "ImageAssetsController.h"

namespace pf {
ImageAssetsController::ImageAssetsController(std::unique_ptr<ImageAssetsView> uiView, std::shared_ptr<UserImageAssetsModel> mod)
    : Controller(std::move(uiView), std::move(mod)) {}
}  // namespace pf