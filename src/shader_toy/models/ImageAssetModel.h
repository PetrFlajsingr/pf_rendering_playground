//
// Created by xflajs00 on 17.05.2022.
//

#pragma once

#include "gpu/Texture.h"
#include "mvc/reactive.h"
#include <filesystem>

namespace pf {

// TODO: use asset manager
class ImageAssetModel {
 public:
  Observable<std::string> name;
  const std::filesystem::path imagePath;
  Observable<std::shared_ptr<Texture>> texture;
};

class UserImageAssetsModel {
 public:
  Observable<std::vector<ImageAssetModel>, VectorLengthChangeDetector<std::vector<ImageAssetModel>>> imageAssets{{}};
};

}  // namespace pf