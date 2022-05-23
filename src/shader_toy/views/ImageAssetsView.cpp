//
// Created by xflajs00 on 18.05.2022.
//

#include "ImageAssetsView.h"
#include "gpu/utils.h"
#include <pf_imgui/ImGuiInterface.h>

namespace pf {
namespace gui = ui::ig;
ImageTile::ImageTile(const std::string &name, ui::ig::Size size, std::shared_ptr<Texture> texture)
    : ui::ig::Element(name), Resizable(size), layout("layout", size), texture(std::move(texture)) {
  layout.setDrawBorder(true);

  const auto maxImageHeight = static_cast<float>(size.height) - 60.f;

  const auto textureSize = this->texture->getSize();
  const auto textureHeightAspectRatio =
      static_cast<float>(textureSize.height.get()) / static_cast<float>(textureSize.width.get());
  const auto textureWidthAspectRatio =
      static_cast<float>(textureSize.width.get()) / static_cast<float>(textureSize.height.get());

  const auto imageHeight = std::min(maxImageHeight, static_cast<float>(size.width) * textureHeightAspectRatio);
  const auto imageWidth = imageHeight * textureWidthAspectRatio;

  nameText = &layout.createChild<ui::ig::Text>("name_txt", name);
  image =
      &layout.createChild<ui::ig::Image>("img", getImTextureID(*this->texture), ui::ig::Size{imageWidth, imageHeight});
  controlsLayout = &layout.createChild<ui::ig::HorizontalLayout>("controls_layout", ui::ig::Size::Auto());
  removeButton = &controlsLayout->createChild<ui::ig::Button>("remove_btn", "Remove");
}

void ImageTile::renderImpl() { layout.render(); }

ImageAssetsView::ImageAssetsView(ui::ig::ImGuiInterface &interface, std::string_view windowName,
                                 std::string_view windowTitle)
    : UIViewWindow(&interface.createWindow(std::string{windowName}, std::string{windowTitle})), interface(interface) {
  layout = &window->createChild(gui::VerticalLayout::Config{.name = "img_win_layout", .size = gui::Size::Auto()});
  controlsLayout = &layout->createChild(gui::HorizontalLayout::Config{.name = "img_controls_layout",
                                                                      .size = gui::Size{gui::Width::Auto(), 35},
                                                                      .showBorder = true});
  addImageButton = &controlsLayout->createChild<gui::Button>("add_img_btn", "Add image");
  controlsSep1 = &controlsLayout->createChild<gui::Separator>("controls_sep1");
  searchInputText =
      &controlsLayout->createChild(gui::InputText::Config{.name = "img_search_text", .label = "Search", .value = ""});
  imagesLayout = &layout->createChild(gui::WrapLayout::Config{.name = "imgs_layout",
                                                              .layoutDirection = gui::LayoutDirection::LeftToRight,
                                                              .size = gui::Size::Auto()});
  imagesLayout->setScrollable(true);
}

ImageTile &ImageAssetsView::addImageTile(std::string_view name, std::shared_ptr<Texture> texture) {
  auto &newTile = imagesLayout->createChild<ImageTile>(std::string{name}, tileSize, std::move(texture));
  imageTiles.emplace_back(&newTile);
  return newTile;
}

ui::ig::Size ImageAssetsView::getTileSize() const { return tileSize; }

void ImageAssetsView::setTileSize(ui::ig::Size newTileSize) {
  tileSize = newTileSize;
  std::ranges::for_each(imageTiles, [this](auto tile) { tile->setSize(tileSize); });
}

}  // namespace pf