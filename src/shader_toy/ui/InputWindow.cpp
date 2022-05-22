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

InputWindow::InputWindow(gui::ImGuiInterface &imGuiInterface, std::unique_ptr<ImageLoader> &&imageLoader) {
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

  imagesTab = &tabBar->addTab("images_tab", "Images", Flags{gui::TabMod::DisableMidMouseClose});
  imagesPanel = &imagesTab->createChild<ImagesPanel>("img_panel", imGuiInterface, std::move(imageLoader),
                                                     ui::ig::Size::Auto(), ui::ig::Persistent::Yes);

  constexpr static auto isUnsupportedType = []<typename T>() {
    return OneOf<T, unsigned int, glm::uvec2, glm::uvec3, glm::uvec4, glm::bvec2, glm::bvec3, glm::bvec4>;
  };

  const auto varNameValidator = [&](std::string_view varName) -> std::optional<std::string> {
    if (!isValidGlslIdentifier(varName)) { return "Invalid variable name"; }
    {
      const auto &textures = imagesPanel->getTextures();
      if (std::ranges::find(textures, std::string{varName}, &std::pair<std::string, std::shared_ptr<Texture>>::first)
          != textures.end()) {
        return "Name is already in use";
      }
    }
    return std::nullopt;
  };

  const auto varSelectValidator = [&, varNameValidator](std::string_view typeName,
                                                        std::string_view varName) -> std::optional<std::string> {
    if (typeName.empty()) { return "Select a type"; }
    if (const auto nameErr = varNameValidator(varName); nameErr.has_value()) { return nameErr.value(); }
    bool unsupportedType = false;
    getTypeForGlslName(typeName, [&]<typename T>() { unsupportedType = isUnsupportedType.operator()<T>(); });
    if (unsupportedType) { return "Selected type is not currently supported"; }
    return std::nullopt;
  };

  imagesPanel->addImageButton->addClickListener([&, varNameValidator] {
    imGuiInterface.getDialogManager()
        .buildFileDialog(ui::ig::FileDialogType::File)
        .size(ui::ig::Size{500, 300})
        .label("Select an image")
        .extension({{"jpg", "png", "bmp"}, "Image file", ui::ig::Color::Red})
        .onSelect([&](const std::vector<std::filesystem::path> &selected) {
          const auto &imgFile = selected[0];

          auto &waitDlg = imGuiInterface.getDialogManager().createDialog("img_wait_dlg", "Loading image");
          waitDlg.setSize(ui::ig::Size{300, 100});
          waitDlg.createChild<ui::ig::Spinner>("img_wait_spinner", 20.f, 4.f);

          const auto onLoadDone = [=, &waitDlg,
                                   &imGuiInterface](tl::expected<std::shared_ptr<Texture>, std::string> loadingResult) {
            MainLoop::Get()->enqueue([=, &waitDlg, &imGuiInterface] {
              waitDlg.close();
              // need to update pf_imgui for this to work since there was a bug
              if (loadingResult.has_value()) {
                GlslVariableNameInputDialogBuilder{imGuiInterface}
                    .inputValidator(varNameValidator)
                    .onInput([=](std::string_view varName) {
                      imagesPanel->addImageTile(loadingResult.value(), std::string{varName}, imgFile);
                    })
                    .show();
              } else {
                imGuiInterface.getNotificationManager()
                    .createNotification("notif_loading_err", "Texture loading failed")
                    .createChild<ui::ig::Text>(
                        "notif_txt", fmt::format("Texture loading failed: '{}'.\n{}", loadingResult.error(), imgFile))
                    .setColor<ui::ig::style::ColorOf::Text>(ui::ig::Color::Red);
              }
            });
          };
          imagesPanel->imageLoader->createTextureAsync(imgFile, onLoadDone);
        })
        .modal()
        .build();
  });

  editor = &mainShaderTab->createChild(gui::TextEditor::Config{.name = "text_editor", .persistent = true});
}

}  // namespace pf::shader_toy