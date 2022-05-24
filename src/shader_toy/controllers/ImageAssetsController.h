//
// Created by Petr on 23/05/2022.
//

#ifndef PF_RENDERING_PLAYGROUND_IMAGEASSETSCONTROLLER_H
#define PF_RENDERING_PLAYGROUND_IMAGEASSETSCONTROLLER_H

#include "../models/ImageAssetModel.h"
#include "../views/ImageAssetsView.h"
#include "mvc/Controller.h"
#include "utils/ImageLoader.h"

namespace pf {
// TODO: rename everything related
// TODO: error reporting callbacks for each controller instead of calling logger directly
class ImageAssetsController : public Controller<ImageAssetsView, UserImageAssetsModel> {
 public:
  ImageAssetsController(std::unique_ptr<ImageAssetsView> uiView, std::shared_ptr<UserImageAssetsModel> mod,
                        std::shared_ptr<ui::ig::ImGuiInterface> imguiInterface, std::shared_ptr<ImageLoader> imageLoader);


  void filterImagesByName(std::string_view searchStr);
  // TODO: this is unused for now, gotta add variable names
  void clearDisallowedNames();
  void addDisallowedName(std::string name);

  void showAddImageDialog();

 private:
  void createUIForImageModel(const std::shared_ptr<TextureAssetModel> &imgModel);

  std::vector<std::string> disallowedNames;
  std::shared_ptr<ui::ig::ImGuiInterface> interface;
  std::shared_ptr<ImageLoader> imageLoader;

  std::unordered_map<std::shared_ptr<TextureAssetModel>, std::vector<Subscription>> subscriptions;
};

}  // namespace pf

#endif  //PF_RENDERING_PLAYGROUND_IMAGEASSETSCONTROLLER_H
