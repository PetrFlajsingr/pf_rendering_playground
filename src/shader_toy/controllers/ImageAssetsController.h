//
// Created by Petr on 23/05/2022.
//

#pragma once

#include "../models/ImageAssetModel.h"
#include "../views/ImageAssetsView.h"
#include "mvc/Controller.h"
#include "utils/ImageLoader.h"
#include <unordered_set>

namespace pf {
// TODO: rename everything related
// TODO: error reporting callbacks for each controller instead of calling logger directly
class ImageAssetsController : public Controller<ImageAssetsView, UserImageAssetsModel> {
 public:
  ImageAssetsController(std::unique_ptr<ImageAssetsView> uiView, std::shared_ptr<UserImageAssetsModel> mod,
                        std::shared_ptr<ui::ig::ImGuiInterface> imguiInterface, std::shared_ptr<ImageLoader> imgLoader);

  void filterImagesByName(std::string_view searchStr);

  std::unordered_set<std::string> disallowedNames;

  void showAddImageDialog();

  void show();
  void hide();

 private:
  void createUIForImageModel(const std::shared_ptr<TextureAssetModel> &imgModel);

  std::shared_ptr<ui::ig::ImGuiInterface> interface;
  std::shared_ptr<ImageLoader> imageLoader;

  std::unordered_map<std::shared_ptr<TextureAssetModel>, std::vector<Subscription>> subscriptions;
};

}  // namespace pf
