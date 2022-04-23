//
// Created by xflajs00 on 18.04.2022.
//

#include "ShaderToyTextInputWindow.h"
#include "shader_toy/utils.h"
#include <pf_imgui/elements/Button.h>
#include <pf_imgui/elements/Combobox.h>
#include <pf_imgui/elements/InputText.h>
#include <pf_imgui/elements/MarkdownText.h>
#include <pf_imgui/interface/decorators/WidthDecorator.h>
#include <spdlog/spdlog.h>

namespace pf {

namespace gui = ui::ig;

ShaderToyTextInputWindow::ShaderToyTextInputWindow(gui::ImGuiInterface &imGuiInterface) {
  window = &imGuiInterface.createWindow("text_input_window", "Editor");
  window->setIsDockable(true);
  layout = &window->createChild(gui::VerticalLayout::Config{.name = "text_input_layout", .size = gui::Size::Auto()});
  tabBar = &layout->createChild<gui::TabBar>("tabbar", true);

  mainShaderTab =
      &tabBar->addTab("main_shader_tab", "Main", Flags{gui::TabMod::DisableMidMouseClose} | gui::TabMod::ForceLeft);

  controlsLayout = &mainShaderTab->createChild(
      gui::HorizontalLayout::Config{.name = "text_controls_layout", .size = gui::Size{gui::Width::Auto(), 80}});
  compileButton = &controlsLayout->createChild(gui::Button::Config{"compile_btn", "Compile"});
  //sep1 = &controlsLayout->createChild<gui::Separator>("sep1");
  timePausedCheckbox = &controlsLayout->createChild(gui::Checkbox::Config{"pause_cbkx", "Pause time"});
  restartButton = &controlsLayout->createChild(gui::Button::Config{"restart_btn", "Restart"});

  infoText = &mainShaderTab->createChild<gui::Text>("info_txt", "Info");
  auto &tooltip = infoText->createTooltip();

  const auto INFO_MD_TEXT = u8R"md(### Variables
  * float time - time since start in seconds
  * float timeDelta - time since last frame in seconds
  * int frameNum - current frame number
  * MOUSE_STATE mouseState - current state of mouse (one of MOUSE_STATE_NONE, MOUSE_STATE_LEFT_DOWN, MOUSE_STATE_RIGHT_DOWN)
  * vec3 mousePos - current mouse position (xy)
)md";

  auto &tooltipLayout = tooltip.createChild<gui::VerticalLayout>("info_ttip_layout", gui::Size{400, 500});
  tooltipLayout.createChild<gui::MarkdownText>("info_md_text", imGuiInterface, INFO_MD_TEXT);

  globalVarsTab = &tabBar->addTab("global_vars_tab", "Variables", Flags{gui::TabMod::DisableMidMouseClose});

  globalVarsLayout = &globalVarsTab->createChild<gui::VerticalLayout>("globalvars_layout", gui::Size::Auto());
  addVarButton = &globalVarsLayout->createChild<gui::Button>("add_var_btn", "Add variable");
  varPanel = &globalVarsLayout->createChild<ShaderToyGlobalVariablesPanel>("global_vars_panel", gui::Size::Auto(),
                                                                           gui::Persistent::Yes);

  addVarButton->addClickListener([&] {
    constexpr auto COLOR_RECORD = "Color (vec3)";
    auto &dlg = imGuiInterface.createDialog("add_var_dlg", "Select variable type and name");
    dlg.setSize(gui::Size{300, 110});
    auto &inLayout = dlg.createChild(
        gui::HorizontalLayout::Config{.name = "add_var_in_lay", .size = gui::Size{gui::Width::Auto(), 30}});
    auto &typeCombobox = inLayout.createChild(gui::WidthDecorator<gui::Combobox<std::string>>::Config{
        .width = 120,
        .config = {.name = "var_type_cb",
                   .label = "Type",
                   .preview = "Select type",
                   .shownItemCount = gui::ComboBoxCount::ItemsAll}});
    typeCombobox.setItems(getGlslTypeNames());
    typeCombobox.addItem(COLOR_RECORD);
    auto &varNameInput = inLayout.createChild(
        gui::WidthDecorator<gui::InputText>::Config{.width = 120,
                                                    .config = {.name = "var_name_txt", .label = "Name", .value = ""}});
    auto &errorText = dlg.createChild<gui::Text>("var_add_err_txt", "");
    errorText.setColor<gui::style::ColorOf::Text>(gui::Color::Red);
    auto &btnLayout = dlg.createChild(
        gui::HorizontalLayout::Config{.name = "add_var_btn_lay", .size = gui::Size{gui::Width::Auto(), 20}});
    auto &okBtn = btnLayout.createChild<gui::Button>("ok_btn", "Ok");
    auto &cancelBtn = btnLayout.createChild<gui::Button>("cancel_btn", "Cancel");
    okBtn.addClickListener([&] {
      if (const auto selectedTypeName = typeCombobox.getSelectedItem(); selectedTypeName.has_value()) {
        const auto varName = varNameInput.getValue();
        // TODO: valid identifier check
        if (!isValidGlslIdentifier(varName)) {
          errorText.setText("Invalid variable name");
          return;
        }
        if (*selectedTypeName == COLOR_RECORD) {
          varPanel->addColorVariable(varName, gui::Color::White);
        } else {
          auto createdVariable = true;
          getTypeForGlslName(*selectedTypeName, [&]<typename T>() {
            if constexpr (std::same_as<T, bool>) {
              varPanel->addBoolVariable(varName, false);
            } else if constexpr (OneOf<T, unsigned int, glm::uvec2, glm::uvec3, glm::uvec4>) {
              errorText.setText("Unsigned int is not supported for now");
              createdVariable = false;
            } else if constexpr (OneOf<T, glm::bvec2, glm::bvec3, glm::bvec4>) {
              errorText.setText("Bool vectors are not supported for now");
              createdVariable = false;
            } else {
              varPanel->addDragVariable<T>(varName, T{});
            }
          });
          if (!createdVariable) { return; }
        }
      } else {
        errorText.setText("Select type");
        return;
      }
      dlg.close();
    });
    cancelBtn.addClickListener([&dlg] { dlg.close(); });
  });

  editor = &mainShaderTab->createChild(gui::TextEditor::Config{.name = "text_editor", .persistent = true});
}

}  // namespace pf