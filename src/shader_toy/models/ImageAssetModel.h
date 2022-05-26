//
// Created by xflajs00 on 17.05.2022.
//

#pragma once

#include "gpu/Texture.h"
#include "mvc/Model.h"
#include "mvc/reactive.h"
#include <filesystem>

namespace pf {

// TODO: use asset manager
// TODO: load when texture nullptr somehow
// maybe only created through a factory?
class TextureAssetModel : public SavableModel {
 public:
  TextureAssetModel(std::string name, std::filesystem::path path, std::shared_ptr<Texture> texture);
  Observable<std::string> name;
  Observable<std::filesystem::path> imagePath;
  Observable<std::shared_ptr<Texture>> texture;

  [[nodiscard]] toml::table toToml() const override;
  void setFromToml(const toml::table &src) override;
};

class UserImageAssetsModel : public SavableModel {
  using TextureModels = std::vector<std::shared_ptr<TextureAssetModel>>;
  template<typename... Args>
  using Event = ClassEvent<UserImageAssetsModel, Args...>;
  using ImageAddedEvent = Event<std::shared_ptr<TextureAssetModel>>;
  using ImageRemovedEvent = Event<std::shared_ptr<TextureAssetModel>>;

 public:
  [[nodiscard]] const TextureModels &getTextures() const;

  ImageAddedEvent imageAddedEvent;
  ImageRemovedEvent imageRemovedEvent;
  std::optional<std::string> addTexture(std::string_view name, std::filesystem::path path,
                                        std::shared_ptr<Texture> texture);
  std::optional<std::string> addTexture(std::shared_ptr<TextureAssetModel> texture);

  void removeTexture(std::string_view texName);

  void clearTextures();

  [[nodiscard]] toml::table toToml() const override;
  void setFromToml(const toml::table &src) override;

 private:
  TextureModels textures;
};

}  // namespace pf