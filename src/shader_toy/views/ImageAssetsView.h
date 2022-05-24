//
// Created by xflajs00 on 18.05.2022.
//

#pragma once

#include "gpu/Texture.h"
#include "mvc/View.h"
#include <pf_imgui/elements/Button.h>
#include <pf_imgui/elements/Image.h>
#include <pf_imgui/elements/Separator.h>
#include <pf_imgui/elements/Text.h>
#include <pf_imgui/interface/Element.h>
#include <pf_imgui/interface/Resizable.h>
#include <pf_imgui/layouts/HorizontalLayout.h>
#include <pf_imgui/layouts/VerticalLayout.h>
#include <pf_imgui/layouts/WrapLayout.h>

namespace pf {

// TODO move this out
// TODO: add texture format
class ImageTile : public ui::ig::Element, public ui::ig::Resizable {
 public:
  ImageTile(const std::string &name, ui::ig::Size size, std::shared_ptr<Texture> newTexture);

  void setTexture(std::shared_ptr<Texture> newTexture);

  // clang-format off
   ui::ig::VerticalLayout layout;
    ui::ig::Text *nameText;
    ui::ig::Text *formatText;
    ui::ig::Image *image;
    ui::ig::HorizontalLayout *controlsLayout;
      ui::ig::Button *removeButton;
  // clang-format on

 protected:
  void renderImpl() override;

 private:
  std::shared_ptr<Texture> texture;
};

class ImageAssetsView : public UIViewWindow {
 public:
  ImageAssetsView(ui::ig::ImGuiInterface &interface, std::string_view windowName, std::string_view windowTitle);

  ImageTile &addImageTile(std::string_view name, std::shared_ptr<Texture> texture);

  // TODO: add remove function which safely deletes froms layout and vector

  [[nodiscard]] ui::ig::Size getTileSize() const;
  void setTileSize(ui::ig::Size newTileSize);

  // clang-format off
  ui::ig::VerticalLayout *layout;
    ui::ig::HorizontalLayout *controlsLayout;
      ui::ig::Button *addImageButton;
      ui::ig::Separator *controlsSep1;
      ui::ig::InputText *searchInputText;
    ui::ig::WrapLayout *imagesLayout;
      std::vector<ImageTile *> imageTiles;
  // clang-format on

 private:
  ui::ig::ImGuiInterface &interface;

  ui::ig::Size tileSize{220, 180};
};

}  // namespace pf
