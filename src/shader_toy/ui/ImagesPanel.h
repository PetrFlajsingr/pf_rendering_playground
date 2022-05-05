//
// Created by xflajs00 on 05.05.2022.
//

#ifndef PF_RENDERING_PLAYGROUND_IMAGESPANEL_H
#define PF_RENDERING_PLAYGROUND_IMAGESPANEL_H

#include <pf_imgui/interface/Element.h>
#include <pf_imgui/interface/Savable.h>
#include <pf_imgui/interface/Resizable.h>
#include <pf_imgui/layouts/WrapLayout.h>
#include <pf_imgui/layouts/VerticalLayout.h>
#include <pf_imgui/layouts/HorizontalLayout.h>
#include <pf_imgui/elements/Button.h>
#include <pf_imgui/elements/Image.h>

namespace pf {

class ImageTile : public ui::ig::Element, public ui::ig::Resizable {
 public:

 protected:

 private:
 // clang-format off
   ui::ig::VerticalLayout *layout;
    ui::ig::Image *image;
    ui::ig::HorizontalLayout *controlsLayout;
      ui::ig::Button *removeButton;
 // clang-format on
};

class ImagesPanel : public ui::ig::Element, public ui::ig::Resizable, public ui::ig::Savable {
 public:

 protected:

 private:
 // clang-format off
   ui::ig::VerticalLayout *layout;
     ui::ig::HorizontalLayout *controlsLayout;
       ui::ig::Button *addImageButton;
     ui::ig::WrapLayout *imagesLayout;
       std::vector<ImageTile *> imageTiles;
 // clang-format on
};

}
#endif  //PF_RENDERING_PLAYGROUND_IMAGESPANEL_H
