//
// Created by Petr on 23/05/2022.
//

#include "ImageAssetsController.h"
#include "../ui/dialogs/GlslLVariableInputDialog.h"
#include "pf_imgui/dialogs/FileDialog.h"
#include "shader_toy/utils.h"
#include <pf_imgui/ImGuiInterface.h>
#include <pf_imgui/elements/Spinner.h>
#include <pf_mainloop/MainLoop.h>

namespace pf {
namespace gui = ui::ig;

ImageAssetsController::ImageAssetsController(std::unique_ptr<ImageAssetsView> uiView,
                                             std::shared_ptr<UserImageAssetsModel> mod,
                                             std::shared_ptr<ui::ig::ImGuiInterface> imguiInterface,
                                             std::shared_ptr<ImageLoader> imageLoader)
    : Controller(std::move(uiView), std::move(mod)), interface(std::move(imguiInterface)),
      imageLoader(std::move(imageLoader)) {
  view->addImageButton->addClickListener(std::bind_front(&ImageAssetsController::showAddImageDialog, this));
}

void ImageAssetsController::clearDisallowedNames() { disallowedNames.clear(); }

void ImageAssetsController::addDisallowedName(std::string name) { disallowedNames.emplace_back(std::move(name)); }

void ImageAssetsController::showAddImageDialog() {
  const auto varNameValidator = [&](std::string_view varName) -> std::optional<std::string> {
    if (!isValidGlslIdentifier(varName)) { return "Invalid variable name"; }
    {
      auto variables = model->getTextures();
      if (std::ranges::find(variables, std::string{varName}, [](const auto &val) { return *val->name; })
          != variables.end()) {
        return "Name is already in use";
      }

      if (std::ranges::find(disallowedNames, std::string{varName}) != disallowedNames.end()) {
        return "Name is already in use";
      }
    }
    return std::nullopt;
  };

  interface->getDialogManager()
      .buildFileDialog(gui::FileDialogType::File)
      .size(ui::ig::Size{500, 300})
      .label("Select an image")
      .extension({{"jpg", "png", "bmp"}, "Image file", ui::ig::Color::Red})
      .onSelect([&](const std::vector<std::filesystem::path> &selected) {
        const auto &imgFile = selected[0];

        auto &waitDlg = interface->getDialogManager().createDialog("img_wait_dlg", "Loading image");
        waitDlg.setSize(ui::ig::Size{300, 100});
        waitDlg.createChild<ui::ig::Spinner>("img_wait_spinner", 20.f, 4.f);
        // TODO: clean this up
        const auto onLoadDone = [=, &waitDlg, this](tl::expected<std::shared_ptr<Texture>, std::string> loadingResult) {
          MainLoop::Get()->enqueue([=, &waitDlg, this] {
            waitDlg.close();
            if (loadingResult.has_value()) {
              shader_toy::GlslVariableNameInputDialogBuilder{*interface}
                  .inputValidator(varNameValidator)
                  .onInput([=](std::string_view varName) {
                    model->addTexture(varName, imgFile, std::move(loadingResult.value()));
                  })
                  .show();
            } else {
              interface->getNotificationManager()
                  .createNotification("notif_loading_err", "Texture loading failed")
                  .createChild<ui::ig::Text>(
                      "notif_txt", fmt::format("Texture loading failed: '{}'.\n{}", loadingResult.error(), imgFile))
                  .setColor<ui::ig::style::ColorOf::Text>(ui::ig::Color::Red);
            }
          });
        };
        imageLoader->loadTextureAsync(imgFile, onLoadDone);
      })
      .modal()
      .build();
}

}  // namespace pf