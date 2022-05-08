//
// Created by xflajs00 on 05.05.2022.
//

#include "ImagesPanel.h"
#include "gpu/utils.h"

namespace pf {

ImageTile::ImageTile(const std::string &name, ui::ig::Size size, std::shared_ptr<Texture> texture,
                     const std::string &varName, std::filesystem::path imagePath)
    : ui::ig::Element(name), Resizable(size), layout("layout", size), texture(std::move(texture)),
      imagePath(std::move(imagePath)) {
  layout.setDrawBorder(true);
  // TODO: use StretchLayout for the image once aspect ratio gets added to it
  const auto maxImageHeight = static_cast<float>(size.height) - 60.f;

  const auto textureSize = this->texture->getSize();
  const auto textureHeightAspectRatio =
      static_cast<float>(textureSize.height.get()) / static_cast<float>(textureSize.width.get());
  const auto textureWidthAspectRatio =
      static_cast<float>(textureSize.width.get()) / static_cast<float>(textureSize.height.get());

  const auto imageHeight = std::min(maxImageHeight, static_cast<float>(size.width) * textureHeightAspectRatio);
  const auto imageWidth = imageHeight * textureWidthAspectRatio;

  nameText = &layout.createChild<ui::ig::Text>("name_txt", varName);
  image =
      &layout.createChild<ui::ig::Image>("img", getImTextureID(*this->texture), ui::ig::Size{imageWidth, imageHeight});
  controlsLayout = &layout.createChild<ui::ig::HorizontalLayout>("controls_layout", ui::ig::Size::Auto());
  removeButton = &controlsLayout->createChild<ui::ig::Button>("remove_btn", "Remove");
}

void ImageTile::renderImpl() { layout.render(); }

ImagesPanel::ImagesPanel(const std::string &name, ui::ig::ImGuiInterface &imguiInterface,
                         std::unique_ptr<ImageLoader> &&imageLoader, const ui::ig::Size &s,
                         ui::ig::Persistent persistent)
    : Element(name), Resizable(s), Savable(persistent), layout("layout", s), imageLoader(std::move(imageLoader)) {
  controlsLayout =
      &layout.createChild<ui::ig::HorizontalLayout>("controls_layout", ui::ig::Size{ui::ig::Width::Auto(), 30});
  addImageButton = &controlsLayout->createChild<ui::ig::Button>("add_img_btn", "Add image");
  imagesLayout = &layout.createChild<ui::ig::WrapLayout>("images_layout", ui::ig::LayoutDirection::LeftToRight,
                                                         ui::ig::Size::Auto());
  imagesLayout->setScrollable(true);
}

toml::table ImagesPanel::toToml() const { return {}; }

void ImagesPanel::setFromToml(const toml::table &src) {}

void ImagesPanel::renderImpl() { layout.render(); }

}  // namespace pf