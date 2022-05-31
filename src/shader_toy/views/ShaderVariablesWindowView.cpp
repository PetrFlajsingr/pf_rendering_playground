//
// Created by xflajs00 on 18.05.2022.
//

#include "ShaderVariablesWindowView.h"

namespace pf {

namespace gui = ui::ig;

ShaderVariablesWindowView::ShaderVariablesWindowView(std::shared_ptr<ui::ig::ImGuiInterface> imguiInterface,
                                                     std::string_view windowName, std::string_view windowTitle)
    : UIViewWindow{std::move(imguiInterface), windowName, windowTitle} {
  window->setIsDockable(true);
  controlsLayout = &window->createChild(
      gui::HorizontalLayout::Config{.name = "layout", .size = gui::Size{gui::Width::Auto(), 35}, .showBorder = true});
  addButton = &controlsLayout->createChild<gui::Button>("add_var_btn", "Add variable");
  controlsSep = &controlsLayout->createChild<gui::Separator>("controls_separator");
  searchTextInput = &controlsLayout->createChild(gui::InputText::Config{
      .name = "search_input",
      .label = "Search",
  });
  varsLayout = &window->createChild(
      gui::VerticalLayout::Config{.name = "vars_layout", .size = gui::Size::Auto(), .showBorder = false});
  varsLayout->setScrollable(true);

  createTooltips();
}

ShaderVariableRecordElement<ui::ig::Checkbox> &ShaderVariablesWindowView::addCheckboxInput(std::string_view name,
                                                                                           bool initialValue) {
  auto &newCheckboxElement = varsLayout->createChild<ShaderVariableRecordElement<ui::ig::Checkbox>>(
      std::string{name}, std::string{name}, initialValue);
  elements.emplace_back(&newCheckboxElement);
  return newCheckboxElement;
}

ShaderVariableRecordElement<ui::ig::ColorEdit<ui::ig::ColorChooserFormat::RGBA>> &
ShaderVariablesWindowView::addColorInput(std::string_view name, ui::ig::Color initialValue) {
  auto &newColorElement =
      varsLayout->createChild<ShaderVariableRecordElement<ui::ig::ColorEdit<ui::ig::ColorChooserFormat::RGBA>>>(
          typename ui::ig::ColorEdit<ui::ig::ColorChooserFormat::RGBA>::Config{.name = std::string{name},
                                                                               .label = name,
                                                                               .value = initialValue});
  elements.emplace_back(&newColorElement);
  return newColorElement;
}

void ShaderVariablesWindowView::createTooltips() {
  addButton->setTooltip("Add a new shader variable");
  searchTextInput->setTooltip("Filter variables by name");
}

}  // namespace pf