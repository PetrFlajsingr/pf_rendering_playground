//
// Created by xflajs00 on 18.04.2022.
//

#include "InputWindow.h"
#include "shader_toy/ui/dialogs/GlslLVariableInputDialog.h"
#include "shader_toy/utils.h"
#include <pf_imgui/elements/Button.h>
#include <pf_imgui/elements/Combobox.h>
#include <pf_imgui/elements/InputText.h>
#include <pf_imgui/elements/MarkdownText.h>
#include <pf_imgui/interface/decorators/WidthDecorator.h>
#include <pf_mainloop/MainLoop.h>
#include <spdlog/spdlog.h>

namespace pf::shader_toy {

namespace gui = ui::ig;

InputWindow::InputWindow(gui::ImGuiInterface &imGuiInterface) {
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
  timePausedCheckbox = &controlsLayout->createChild(
      gui::Checkbox::Config{.name = "pause_cbkx", .label = "Pause time", .persistent = true});
  restartButton = &controlsLayout->createChild(gui::Button::Config{"restart_btn", "Restart"});
  autoCompileCheckbox = &controlsLayout->createChild(
      gui::Checkbox::Config{.name = "autocompile_cbkx", .label = "Auto compile", .persistent = true});
  autoCompileFrequencyDrag = &controlsLayout->createChild(
      gui::WidthDecorator<gui::DragInput<float>>::Config{.width = 50,
                                                         .config = {.name = "autocompile_f_drag",
                                                                    .label = "Frequency",
                                                                    .speed = 0.01f,
                                                                    .min = 0.1f,
                                                                    .max = 10.f,
                                                                    .value = 1.f,
                                                                    .format = "%.1f sec",
                                                                    .persistent = true}});
  autoCompileCheckbox->addValueListener(
      [this](bool value) { autoCompileFrequencyDrag->setEnabled(value ? Enabled::Yes : Enabled::No); });

  codeToClipboardButton =
      &mainShaderTab->createChild<gui::Button>("copy_shtoy_to_clip", "Generated shader to clipboard");

  infoLayout = &mainShaderTab->createChild(
      gui::HorizontalLayout::Config{.name = "info_layout", .size = gui::Size{gui::Width::Auto(), 30}});
  infoText = &infoLayout->createChild<gui::Text>("info_txt", "Info");
  compilationSpinner =
      &infoLayout->createChild(gui::Spinner::Config{.name = "compilation_bar", .radius = 6, .thickness = 4});
  compilationSpinner->setVisibility(gui::Visibility::Invisible);

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

  editor = &mainShaderTab->createChild(gui::TextEditor::Config{.name = "text_editor", .persistent = true});
}

}  // namespace pf::shader_toy