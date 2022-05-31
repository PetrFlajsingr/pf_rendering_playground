//
// Created by Petr on 26/05/2022.
//

#include "OutputView.h"
#include <pf_imgui/ImGuiInterface.h>
#include <pf_imgui/interface/decorators/WidthDecorator.h>

namespace pf {

namespace gui = ui::ig;

OutputView::OutputView(std::shared_ptr<ui::ig::ImGuiInterface> imguiInterface, std::string_view windowName,
                       std::string_view windowTitle)
    : UIViewWindow(std::move(imguiInterface), windowName, windowTitle) {
  imageSettingsLayout = &window->createChild(
      gui::HorizontalLayout::Config{.name = "img_settings_layout", .size = gui::Size{gui::Width::Auto(), 30}});
  widthCombobox = &imageSettingsLayout->createChild(gui::WidthDecorator<gui::Combobox<int>>::Config{
      .width = 60,
      .config = {.name = "img_width_cb", .label = "Width", .preview = "Image width", .persistent = true}});
  heightCombobox = &imageSettingsLayout->createChild(gui::WidthDecorator<gui::Combobox<int>>::Config{
      .width = 60,
      .config = {.name = "img_height_cb", .label = "Height", .preview = "Image height", .persistent = true}});

  layout = &window->createChild(gui::StretchLayout::Config{.name = "output_layout",
                                                           .size = {gui::Width::Auto(), gui::Height::Fill(30)},
                                                           .stretch = gui::Stretch::All});

  image = &layout->createChild(gui::Image::Config{.name = "output_img", .textureId = 0, .size = gui::Size::Auto()});

  fpsInfoLayout = &window->createChild(
      gui::HorizontalLayout::Config{.name = "img_fps_layout", .size = gui::Size{gui::Width::Auto(), 25}});
  fpsAveragePlot = &fpsInfoLayout->createChild(gui::SimplePlot::Config{.name = "fps_plot",
                                                                       .label = "",
                                                                       .type = gui::PlotType::Histogram,
                                                                       .maxHistoryCount = 500});
  fpsText = &fpsInfoLayout->createChild<gui::Text>("fps_txt", "");

  createTooltips();
}

void OutputView::createTooltips() {
  widthCombobox->setTooltip("Output width");
  heightCombobox->setTooltip("Output height");
}

}  // namespace pf