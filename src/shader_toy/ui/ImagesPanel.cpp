//
// Created by xflajs00 on 05.05.2022.
//

#include "ImagesPanel.h"

namespace pf {

ImageTile::ImageTile(const std::string &name, ui::ig::Size size, ImTextureID textureId, std::string varName)
    : ui::ig::Element(name), Resizable(size), layout("layout", size) {
  layout.setDrawBorder(true);
  nameText = &layout.createChild<ui::ig::Text>("name_txt", varName);
  image = &layout.createChild<ui::ig::Image>("img", textureId,
                                             ui::ig::Size{size.width, size.height - 60});
  controlsLayout = &layout.createChild<ui::ig::HorizontalLayout>("controls_layout", ui::ig::Size::Auto());
  removeButton = &controlsLayout->createChild<ui::ig::Button>("remove_btn", "Remove");
}

void ImageTile::renderImpl() { layout.render(); }

ImagesPanel::ImagesPanel(const std::string &name, const ui::ig::Size &s, ui::ig::Persistent persistent)
    : Element(name), Resizable(s), Savable(persistent), layout("layout", s) {
   controlsLayout = &layout.createChild<ui::ig::HorizontalLayout>("controls_layout", ui::ig::Size{ui::ig::Width::Auto(), 30});
   addImageButton = &controlsLayout->createChild<ui::ig::Button>("add_img_btn", "Add image");
   imagesLayout = &layout.createChild<ui::ig::WrapLayout>("images_layout", ui::ig::LayoutDirection::LeftToRight, ui::ig::Size::Auto());
   imagesLayout->setScrollable(true);
}

toml::table ImagesPanel::toToml() const { return {}; }

void ImagesPanel::setFromToml(const toml::table &src) {}

void ImagesPanel::renderImpl() {
  layout.render();
}

}