//
// Created by Petr on 30/04/2022.
//

#include "GlslLVariableInputDialog.h"
#include <pf_imgui/elements/Combobox.h>
#include <pf_imgui/interface/decorators/WidthDecorator.h>

namespace gui = pf::ui::ig;
namespace pf::shader_toy {

GlslVariableInputDialogBuilder &
GlslVariableInputDialogBuilder::addTypeName(GlslVariableInputDialogBuilder::TypeName typeName) {
  typeNames_.emplace_back(std::string{typeName});
  return *this;
}

void GlslVariableInputDialogBuilder::show() {
  auto &dlg = interface_.createDialog("add_var_dlg", "Select variable type and name");
  dlg.setSize(gui::Size{300, 110});
  auto &inLayout = dlg.createChild(
      gui::HorizontalLayout::Config{.name = "add_var_in_lay", .size = gui::Size{gui::Width::Auto(), 30}});
  auto &typeCombobox = inLayout.createChild(gui::WidthDecorator<gui::Combobox<std::string>>::Config{
      .width = 120,
      .config = {.name = "var_type_cb",
                 .label = "Type",
                 .preview = "Select type",
                 .shownItemCount = gui::ComboBoxCount::ItemsAll}});
  typeCombobox.addItems(typeNames_);
  auto &varNameInput = inLayout.createChild(
      gui::WidthDecorator<gui::InputText>::Config{.width = 120,
                                                  .config = {.name = "var_name_txt", .label = "Name", .value = ""}});
  auto &errorText = dlg.createChild<gui::Text>("var_add_err_txt", "");
  errorText.setColor<gui::style::ColorOf::Text>(gui::Color::Red);
  auto &btnLayout = dlg.createChild(
      gui::HorizontalLayout::Config{.name = "add_var_btn_lay", .size = gui::Size{gui::Width::Auto(), 20}});
  auto &okBtn = btnLayout.createChild<gui::Button>("ok_btn", "Ok");
  auto &cancelBtn = btnLayout.createChild<gui::Button>("cancel_btn", "Cancel");
  okBtn.addClickListener([&errorText, &dlg, &typeCombobox, &varNameInput, *this]() mutable {
    if (const auto selectedTypeName = typeCombobox.getSelectedItem(); selectedTypeName.has_value()) {
      const auto varName = varNameInput.getValue();

      if (const auto err = inputValidator_(*selectedTypeName, varName); err.has_value()) {
        errorText.setText("Invalid variable name");
        return;
      }

      onInput_(*selectedTypeName, varName);
    } else {
      errorText.setText("Select type");
      return;
    }
    dlg.close();
  });
  cancelBtn.addClickListener([&dlg] { dlg.close(); });
}
}  // namespace pf::shader_toy