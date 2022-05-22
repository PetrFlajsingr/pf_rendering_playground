//
// Created by xflajs00 on 17.05.2022.
//

#include "ImageAssetModel.h"

namespace pf {

TextureAssetModel::TextureAssetModel(std::string name, std::filesystem::path path, std::shared_ptr<Texture> texture)
    : name(std::move(name)), imagePath(std::move(path)), texture(std::move(texture)) {}

toml::table TextureAssetModel::toToml() const { return toml::table{{"name", *name}, {"path", imagePath->string()}}; }

void TextureAssetModel::setFromToml(const toml::table &src) {
  if (const auto iter = src.find("name"); iter != src.end()) {
    if (const auto nameStr = iter->second.as_string(); nameStr != nullptr) { *name.modify() = nameStr->get(); }
  }
  if (const auto iter = src.find("path"); iter != src.end()) {
    if (const auto pathStr = iter->second.as_string(); pathStr != nullptr) { *imagePath.modify() = pathStr->get(); }
  }
}

const UserImageAssetsModel::TextureModels &UserImageAssetsModel::getTextures() const { return textures; }

std::optional<std::string> UserImageAssetsModel::addTexture(std::string_view name, std::filesystem::path path,
                                                            std::shared_ptr<Texture> texture) {
  return addTexture(std::make_shared<TextureAssetModel>(std::string{name}, std::move(path), std::move(texture)));
}

std::optional<std::string> UserImageAssetsModel::addTexture(std::shared_ptr<TextureAssetModel> texture) {
  if (const auto iter = std::ranges::find(textures, *texture->name, [](const auto &texture) { return *texture->name; });
      iter != textures.end()) {
    return "Duplicate texture name";
  }
  imageAddedEvent.notify(textures.emplace_back(std::move(texture)));
  return std::nullopt;
}

void UserImageAssetsModel::removeTexture(std::string_view texName) {
  TextureModels toRemove;
  std::ranges::remove_copy_if(textures, std::back_inserter(toRemove),
                              [texName](const auto &texture) { return *texture->name != texName; });
  std::ranges::for_each(toRemove, [this](const auto &texture) { imageRemovedEvent.notify(texture); });
}

void UserImageAssetsModel::clearTextures() {
  const auto toRemove = textures;
  textures.clear();
  std::ranges::for_each(toRemove, [this](const auto &texture) { imageRemovedEvent.notify(texture); });
}

toml::table UserImageAssetsModel::toToml() const {
  auto imgArray = toml::array{};
  std::ranges::transform(textures, std::back_inserter(imgArray), &TextureAssetModel::toToml);
  return toml::table{{"images", std::move(imgArray)}};
}

void UserImageAssetsModel::setFromToml(const toml::table &src) {
  if (const auto iter = src.find("images"); iter != src.end()) {
    if (const auto imgsArray = iter->second.as_array(); imgsArray != nullptr) {
      std::ranges::for_each(*imgsArray, [&](const auto &record) {
        if (const auto tblPtr = record.as_table(); tblPtr != nullptr) {
          auto newTexture = std::make_shared<TextureAssetModel>("", "", nullptr);
          newTexture->setFromToml(*tblPtr);
          addTexture(std::move(newTexture));
        }
      });
    }
  }
}
}  // namespace pf