//
// Created by Petr on 26/05/2022.
//

#include "GlslEditorView.h"
#include <pf_imgui/ImGuiInterface.h>
#include <pf_imgui/elements/MarkdownText.h>
#include <pf_imgui/interface/decorators/WidthDecorator.h>

namespace pf {

namespace gui = ui::ig;
// TODO: remake controls
// TODO: better help, best to just use a separate window for help - extract from PhysarumSim into pf_imgui?
GlslEditorView::GlslEditorView(gui::ImGuiInterface &interface, std::string_view windowName,
                               std::string_view windowTitle)
    : UIViewWindow(&interface.createWindow(std::string{windowName}, std::string{windowTitle})) {
  window->setIsDockable(true);
  layout = &window->createChild(gui::VerticalLayout::Config{.name = "text_input_layout", .size = gui::Size::Auto()});

  controlsLayout = &layout->createChild(gui::HorizontalLayout::Config{.name = "text_controls_layout",
                                                                      .size = gui::Size{gui::Width::Auto(), 80},
                                                                      .showBorder = true});
  compileButton = &controlsLayout->createChild(gui::Button::Config{"compile_btn", "Compile"});
  sep1 = &controlsLayout->createChild<gui::Separator>("sep1");
  timePausedCheckbox = &controlsLayout->createChild(gui::Checkbox::Config{.name = "pause_cbkx", .label = "Pause time"});
  restartButton = &controlsLayout->createChild(gui::Button::Config{"restart_btn", "Restart"});
  sep2 = &controlsLayout->createChild<gui::Separator>("sep2");
  autoCompileCheckbox =
      &controlsLayout->createChild(gui::Checkbox::Config{.name = "autocompile_cbkx", .label = "Auto compile"});
  autoCompilePeriodDrag = &controlsLayout->createChild(
      gui::WidthDecorator<gui::DragInput<float>>::Config{.width = 50,
                                                         .config = {.name = "autocompile_f_drag",
                                                                    .label = "Period",
                                                                    .speed = 0.01f,
                                                                    .min = 0.1f,
                                                                    .max = 10.f,
                                                                    .value = 1.f,
                                                                    .format = "%.1f sec"}});
  autoCompileCheckbox->addValueListener(
      [this](bool value) { autoCompilePeriodDrag->setEnabled(value ? Enabled::Yes : Enabled::No); });

  infoLayout = &layout->createChild(
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
  * vec3 mousePos - current mouse position in pixel coordinates (xy)
  * vec3 mousePosNormalized - current mouse position in [0.f, 1.f] range (xy)
)md";

  auto &tooltipLayout = tooltip.createChild<gui::VerticalLayout>("info_ttip_layout", gui::Size{400, 500});
  tooltipLayout.createChild<gui::MarkdownText>("info_md_text", interface, INFO_MD_TEXT);

  editor = &layout->createChild(gui::TextEditor::Config{.name = "text_editor"});
}

}  // namespace pf