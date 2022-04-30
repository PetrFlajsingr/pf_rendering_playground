//
// Created by xflajs00 on 18.04.2022.
//

#include "ShaderToyTextInputWindow.h"
#include "shader_toy/ui/dialogs/ShaderToyGLSLVariableInputDialog.h"
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
  auto &tooltip = infoText->createOrGetTooltip();

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

  constexpr static auto isUnsupportedType = []<typename T>() {
    return OneOf<T, unsigned int, glm::uvec2, glm::uvec3, glm::uvec4, glm::bvec2, glm::bvec3, glm::bvec4>;
  };

  addVarButton->addClickListener([&] {
    constexpr auto COLOR_RECORD = "Color (vec4)";
    shader_toy::GLSLVariableInputDialogBuilder{imGuiInterface}
        .addTypeNames(getGlslTypeNames())
        .addTypeName(COLOR_RECORD)
        .inputValidator([&](std::string_view typeName, std::string_view varName) -> std::optional<std::string> {
          if (typeName.empty()) { return "Select a type"; }
          if (!isValidGlslIdentifier(varName)) { return "Invalid variable name"; }
          bool unsupportedType = false;
          getTypeForGlslName(typeName, [&]<typename T>() { unsupportedType = isUnsupportedType.operator()<T>(); });
          if (unsupportedType) { return "Selected type is not currently supported"; }
          return std::nullopt;
        })
        .onInput([&](std::string_view typeName, std::string_view varName) {
          if (typeName == COLOR_RECORD) {
            varPanel->addColorVariable(varName, gui::Color::White);
          } else {
            getTypeForGlslName(typeName, [&]<typename T>() {
              if constexpr (isUnsupportedType.operator()<T>()) {
                assert(false
                       && "This should never happen");  // this needs to be here due to template instantiation errors
              } else if constexpr (std::same_as<T, bool>) {
                varPanel->addBoolVariable(varName, false);
              } else {
                varPanel->addDragVariable<T>(varName, T{});
              }
            });
          }
        })
        .show();
    });
  editor = &mainShaderTab->createChild(gui::TextEditor::Config{.name = "text_editor", .persistent = true});
}

}  // namespace pf