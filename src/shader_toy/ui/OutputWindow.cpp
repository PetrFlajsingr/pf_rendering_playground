//
// Created by xflajs00 on 18.04.2022.
//

#include "OutputWindow.h"
#include <pf_imgui/elements/Image.h>
#include <pf_imgui/interface/decorators/WidthDecorator.h>

namespace pf::shader_toy {
namespace gui = ui::ig;
OutputWindow::OutputWindow(ui::ig::ImGuiInterface &imGuiInterface) {
  window = &imGuiInterface.createWindow("output_window", "Output");
  window->setIsDockable(true);

  imageSettingsLayout = &window->createChild(
      gui::HorizontalLayout::Config{.name = "img_settings_layout", .size = gui::Size{gui::Width::Auto(), 30}});
  widthCombobox = &imageSettingsLayout->createChild(gui::WidthDecorator<gui::Combobox<int>>::Config{
      .width = 60,
      .config = {.name = "img_width_cb", .label = "Width", .preview = "Image width", .persistent = true}});
  heightCombobox = &imageSettingsLayout->createChild(gui::WidthDecorator<gui::Combobox<int>>::Config{
      .width = 60,
      .config = {.name = "img_height_cb", .label = "Height", .preview = "Image height", .persistent = true}});

  widthCombobox->setItems(IMAGE_SIZES);
  heightCombobox->setItems(IMAGE_SIZES);

  widthCombobox->setValue(1024);
  heightCombobox->setValue(1024);

  layout = &window->createChild(gui::StretchLayout::Config{.name = "output_layout",
                                                           .size = {gui::Width::Auto(), gui::Height::Fill(30)},
                                                           .stretch = gui::Stretch::All});

  image = &layout->createChild(
      gui::Image::Config{.name = "output_img", .textureId = 0, .size = gui::Size::Auto()});

  fpsInfoLayout = &window->createChild(
      gui::HorizontalLayout::Config{.name = "img_fps_layout", .size = gui::Size{gui::Width::Auto(), 25}});
  fpsAveragePlot = &fpsInfoLayout->createChild(gui::SimplePlot::Config{.name = "fps_plot",
                                                                       .label = "",
                                                                       .type = gui::PlotType::Histogram,
                                                                       .maxHistoryCount = 500});
  fpsText = &fpsInfoLayout->createChild<gui::Text>("fps_txt", "");
}

}  // namespace pf
