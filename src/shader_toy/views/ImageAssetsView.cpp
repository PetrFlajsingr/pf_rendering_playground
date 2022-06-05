//
// Created by xflajs00 on 18.05.2022.
//

#include "ImageAssetsView.h"
#include "gpu/utils.h"
#include <pf_imgui/ImGuiInterface.h>

namespace pf {
namespace gui = ui::ig;
ImageTile::ImageTile(const std::string &name, ui::ig::Size size, std::shared_ptr<gpu::Texture> newTexture)
    : ui::ig::Element(name), Resizable(size), layout("layout", size), texture(std::move(newTexture)) {
  layout.setDrawBorder(true);

  nameText = &layout.createChild<ui::ig::Text>("name_txt", name);
  formatText = &layout.createChild<ui::ig::Text>("format_txt", "");
  formatText->setColor<gui::style::ColorOf::Text>(gui::Color::RGB(91, 142, 34));
  ImTextureID textureID = 0;  // TODO: some placeholder thing
  if (texture != nullptr) {
    formatText->setText("{}", magic_enum::enum_name(texture->getFormat()));
    textureID = getImTextureID(*this->texture);
  }

  image = &layout.createChild<ui::ig::Image>("img", textureID, calculateImageSize());
  controlsLayout = &layout.createChild<ui::ig::HorizontalLayout>("controls_layout", ui::ig::Size::Auto());
  removeButton = &controlsLayout->createChild<ui::ig::Button>("remove_btn", "Remove");

  image->setTooltip(name);
}

void ImageTile::setTexture(std::shared_ptr<gpu::Texture> newTexture) {
  texture = std::move(newTexture);
  if (texture != nullptr) {
    image->setTextureId(getImTextureID(*this->texture));
    formatText->setText("{}", magic_enum::enum_name(texture->getFormat()));
  } else {
    // TODO: placeholder thing
    image->setTextureId(0);
    formatText->setText("");
  }
  image->setSize(calculateImageSize());
}

void ImageTile::renderImpl() { layout.render(); }

ui::ig::Size ImageTile::calculateImageSize() const {
  // TODO: some placeholder thing
  // total height minus heights of other elements
  const auto maxImageHeight = static_cast<float>(getSize().height) - 80.f;
  const auto textureSize = texture != nullptr
      ? texture->getSize()
      : gpu::TextureSize{gpu::TextureWidth{1024}, gpu::TextureHeight{1024}, gpu::TextureDepth{1}};
  const auto textureHeightAspectRatio =
      static_cast<float>(textureSize.height.get()) / static_cast<float>(textureSize.width.get());
  const auto textureWidthAspectRatio =
      static_cast<float>(textureSize.width.get()) / static_cast<float>(textureSize.height.get());

  const auto imageHeight = std::min(maxImageHeight, static_cast<float>(getSize().width) * textureHeightAspectRatio);
  const auto imageWidth = imageHeight * textureWidthAspectRatio;
  return gui::Size{imageWidth, imageHeight};
}

ImageAssetsView::ImageAssetsView(std::shared_ptr<ui::ig::ImGuiInterface> imguiInterface, std::string_view windowName,
                                 std::string_view windowTitle)
    : UIViewWindow(std::move(imguiInterface), windowName, windowTitle) {
  window->setIsDockable(true);
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

  createTooltips();
}

ImageTile &ImageAssetsView::addImageTile(std::string_view name, std::shared_ptr<gpu::Texture> texture) {
  auto &newTile = imagesLayout->createChild<ImageTile>(std::string{name}, tileSize, std::move(texture));
  imageTiles.emplace_back(&newTile);
  return newTile;
}

ui::ig::Size ImageAssetsView::getTileSize() const { return tileSize; }

void ImageAssetsView::setTileSize(ui::ig::Size newTileSize) {
  tileSize = newTileSize;
  std::ranges::for_each(imageTiles, [this](auto tile) { tile->setSize(tileSize); });
}

void ImageAssetsView::createTooltips() {
  addImageButton->setTooltip("Load a new texture image from disk");
  searchInputText->setTooltip("Filter textures by name");
}

}  // namespace pf