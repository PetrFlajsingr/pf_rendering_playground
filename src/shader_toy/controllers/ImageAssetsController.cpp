//
// Created by Petr on 23/05/2022.
//

#include "ImageAssetsController.h"
#include <pf_imgui/ImGuiInterface.h>
#include <pf_mainloop/MainLoop.h>
#include <pf_imgui/elements/Spinner.h>
#include "../ui/dialogs/GlslLVariableInputDialog.h"

namespace pf {

ImageAssetsController::ImageAssetsController(std::unique_ptr<ImageAssetsView> uiView,
                                             std::shared_ptr<UserImageAssetsModel> mod,
                                             std::shared_ptr<ui::ig::ImGuiInterface> imguiInterface)
    : Controller(std::move(uiView), std::move(mod)), interface(std::move(imguiInterface)) {
  view->addImageButton->addClickListener([this] {
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
          const auto onLoadDone = [=, &waitDlg,
                                   this](tl::expected<std::shared_ptr<Texture>, std::string> loadingResult) {
            MainLoop::Get()->enqueue([=, &waitDlg, this] {
              waitDlg.close();
              // need to update pf_imgui for this to work since there was a bug
              if (loadingResult.has_value()) {
                GlslVariableNameInputDialogBuilder{*interface}
                    .inputValidator(varNameValidator)
                    .onInput([=](std::string_view varName) {
                      imagesPanel->addImageTile(loadingResult.value(), std::string{varName}, imgFile);
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
          imagesPanel->imageLoader->createTextureAsync(imgFile, onLoadDone);
        })
        .modal()
        .build();
  });
}
void ImageAssetsController::clearDisallowedNames() { disallowedNames.clear(); }

void ImageAssetsController::addDisallowedName(std::string name) { disallowedNames.emplace_back(std::move(name)); }

}  // namespace pf