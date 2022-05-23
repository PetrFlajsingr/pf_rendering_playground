//
// Created by Petr on 23/05/2022.
//

#ifndef PF_RENDERING_PLAYGROUND_IMAGEASSETSCONTROLLER_H
#define PF_RENDERING_PLAYGROUND_IMAGEASSETSCONTROLLER_H

#include "../models/ImageAssetModel.h"
#include "../views/ImageAssetsView.h"
#include "mvc/Controller.h"

namespace pf {
// TODO: rename everything related
class ImageAssetsController : public Controller<ImageAssetsView, UserImageAssetsModel> {
 public:
  ImageAssetsController(std::unique_ptr<ImageAssetsView> uiView, std::shared_ptr<UserImageAssetsModel> mod);
};

}  // namespace pf

#endif  //PF_RENDERING_PLAYGROUND_IMAGEASSETSCONTROLLER_H
