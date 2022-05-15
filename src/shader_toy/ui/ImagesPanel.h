//
// Created by xflajs00 on 05.05.2022.
//

#pragma once

#include "gpu/Texture.h"
#include <filesystem>
#include <pf_imgui/ImGuiInterface.h>
#include <pf_imgui/elements/Button.h>
#include <pf_imgui/elements/Image.h>
#include <pf_imgui/elements/Text.h>
#include <pf_imgui/interface/Element.h>
#include <pf_imgui/interface/Resizable.h>
#include <pf_imgui/interface/Savable.h>
#include <pf_imgui/layouts/HorizontalLayout.h>
#include <pf_imgui/layouts/VerticalLayout.h>
#include <pf_imgui/layouts/WrapLayout.h>
#include <pf_mainloop/MainLoop.h>

namespace pf {
// TODO: image size based on aspect ratio
class ImageTile : public ui::ig::Element, public ui::ig::Resizable {
 public:
  ImageTile(const std::string &name, ui::ig::Size size, std::shared_ptr<Texture> texture, const std::string &varName,
            std::filesystem::path imagePath);

  // clang-format off
   ui::ig::VerticalLayout layout;
    ui::ig::Text *nameText;
    ui::ig::Image *image;
    ui::ig::HorizontalLayout *controlsLayout;
      ui::ig::Button *removeButton;
  // clang-format on

  std::shared_ptr<Texture> texture;

  std::filesystem::path imagePath;

 protected:
  void renderImpl() override;
};

// TODO: refactor this
struct ImageLoader {
  virtual ~ImageLoader() = default;
  virtual tl::expected<std::shared_ptr<Texture>, std::string> createTexture(const std::filesystem::path &imagePath) = 0;
  virtual void createTextureAsync(const std::filesystem::path &imagePath,
                             std::function<void(tl::expected<std::shared_ptr<Texture>, std::string>)> onLoadDone) = 0;
};

class ImagesPanel : public ui::ig::Element, public ui::ig::Resizable, public ui::ig::Savable {
 public:
  ImagesPanel(const std::string &name, ui::ig::ImGuiInterface &imguiInterface,
              std::unique_ptr<ImageLoader> &&imageLoader, const ui::ig::Size &s, ui::ig::Persistent persistent);

  [[nodiscard]] toml::table toToml() const override;
  void setFromToml(const toml::table &src) override;

  void addImageTile(std::shared_ptr<Texture> texture, const std::string &varName, std::filesystem::path imagePath) {
    auto &newTile = imageTiles.emplace_back(
        &imagesLayout->createChild<ImageTile>(getName() + "_img_" + std::to_string(IdCounter++), ui::ig::Size{220, 150},
                                              std::move(texture), varName, imagePath));
    newTile->removeButton->addClickListener([this, newTile] {
      const auto [rmBeg, rmEnd] = std::ranges::remove(imageTiles, newTile);
      imageTiles.erase(rmBeg, rmEnd);
      MainLoop::Get()->forceEnqueue([&] {
        imagesLayout->removeChild(newTile->getName());
        imagesChangedObservable.notify();
      });
    });
    imagesChangedObservable.notify();
  }

  void clearImageTiles() {
    std::ranges::for_each(imageTiles,
                          [this](const auto &imageTile) { imagesLayout->removeChild(imageTile->getName()); });
    imageTiles.clear();
  }

  // TODO: refactoring
  [[nodiscard]] std::vector<std::pair<std::string, std::shared_ptr<Texture>>> getTextures() const {
    std::vector<std::pair<std::string, std::shared_ptr<Texture>>> result{};
    std::ranges::for_each(
        imageTiles, [&](const ImageTile *tile) { result.emplace_back(tile->nameText->getText(), tile->texture); });
    return result;
  }

  Subscription addImagesChangedListener(std::invocable auto &&listener) {
    return imagesChangedObservable.addListener(std::forward<decltype(listener)>(listener));
  }

  // clang-format off
   ui::ig::VerticalLayout layout;
     ui::ig::HorizontalLayout *controlsLayout;
       ui::ig::Button *addImageButton;
     ui::ig::WrapLayout *imagesLayout;
       std::vector<ImageTile *> imageTiles;
  // clang-format on

  std::unique_ptr<ImageLoader> imageLoader;

 protected:
  void renderImpl() override;

 private:
  ui::ig::Observable_impl<> imagesChangedObservable;

  static inline std::size_t IdCounter{};
};

}  // namespace pf
