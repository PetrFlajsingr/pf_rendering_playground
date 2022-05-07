//
// Created by xflajs00 on 05.05.2022.
//

#pragma once

#include <filesystem>
#include <pf_imgui/elements/Button.h>
#include <pf_imgui/elements/Image.h>
#include <pf_imgui/elements/Text.h>
#include <pf_imgui/interface/Element.h>
#include <pf_imgui/interface/Resizable.h>
#include <pf_imgui/interface/Savable.h>
#include <pf_imgui/layouts/HorizontalLayout.h>
#include <pf_imgui/layouts/VerticalLayout.h>
#include <pf_imgui/layouts/WrapLayout.h>

namespace pf {
// TODO: image size based on aspect ratio
class ImageTile : public ui::ig::Element, public ui::ig::Resizable {
 public:
  ImageTile(const std::string &name, ui::ig::Size size, ImTextureID textureId, std::string varName);

  // clang-format off
   ui::ig::VerticalLayout layout;
    ui::ig::Text *nameText;
    ui::ig::Image *image;
    ui::ig::HorizontalLayout *controlsLayout;
      ui::ig::Button *removeButton;
  // clang-format on

 protected:
  void renderImpl() override;
};

class ImagesPanel : public ui::ig::Element, public ui::ig::Resizable, public ui::ig::Savable {
 public:
  ImagesPanel(const std::string &name, const ui::ig::Size &s, ui::ig::Persistent persistent);

  [[nodiscard]] toml::table toToml() const override;
  void setFromToml(const toml::table &src) override;

  void addImageTile(ImTextureID textureId, std::string varName) {
    static int cnt = 0;
    imageTiles.emplace_back(&imagesLayout->createChild<ImageTile>(getName() + "_img_" + std::to_string(cnt++), ui::ig::Size{220, 150}, textureId, varName));
  }

  void clearImageTiles() {
    std::ranges::for_each(imageTiles, [this](const auto &imageTile) {
      imagesLayout->removeChild(imageTile->getName());
    });
    imageTiles.clear();
  }


 protected:
  void renderImpl() override;

 private:
  struct ImageRecord {
    ImageTile *tile;
    std::filesystem::path path;
  };
  // clang-format off
   ui::ig::VerticalLayout layout;
     ui::ig::HorizontalLayout *controlsLayout;
       ui::ig::Button *addImageButton;
     ui::ig::WrapLayout *imagesLayout;
       std::vector<ImageTile *> imageTiles;
  // clang-format on
};

}  // namespace pf
