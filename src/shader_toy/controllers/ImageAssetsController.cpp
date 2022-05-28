//
// Created by Petr on 23/05/2022.
//

#include "ImageAssetsController.h"
#include "../ui/dialogs/GlslLVariableInputDialog.h"
#include "pf_imgui/dialogs/FileDialog.h"
#include "shader_toy/utils.h"
#include "spdlog/spdlog.h"
#include <assert.hpp>
#include <pf_imgui/ImGuiInterface.h>
#include <pf_imgui/elements/Spinner.h>
#include <pf_mainloop/MainLoop.h>

namespace pf {
namespace gui = ui::ig;

ImageAssetsController::ImageAssetsController(std::unique_ptr<ImageAssetsView> uiView,
                                             std::shared_ptr<UserImageAssetsModel> mod,
                                             std::shared_ptr<ui::ig::ImGuiInterface> imguiInterface,
                                             std::shared_ptr<ImageLoader> imgLoader)
    : Controller(std::move(uiView), std::move(mod)), interface(std::move(imguiInterface)),
      imageLoader(std::move(imgLoader)) {
  VERIFY(interface != nullptr);
  VERIFY(imageLoader != nullptr);
  view->addImageButton->addClickListener(std::bind_front(&ImageAssetsController::showAddImageDialog, this));

  view->searchInputText->addValueListener(std::bind_front(&ImageAssetsController::filterImagesByName, this));

  std::ranges::for_each(model->getTextures(), std::bind_front(&ImageAssetsController::createUIForImageModel, this));

  model->imageAddedEvent.addEventListener(std::bind_front(&ImageAssetsController::createUIForImageModel, this));
  model->imageRemovedEvent.addEventListener([this](const auto &imageModel) {
    MainLoop::Get()->forceEnqueue([this, imageModel] {
      const auto iter = subscriptions.find(imageModel);
      std::ranges::for_each(iter->second, &Subscription::unsubscribe);
      subscriptions.erase(iter);
      const auto [rmBeg, rmEnd] = std::ranges::remove(view->imageTiles, *imageModel->name, &gui::Element::getName);
      view->imageTiles.erase(rmBeg, rmEnd);
      view->imagesLayout->removeChild(*imageModel->name);
    });
  });
}

void ImageAssetsController::filterImagesByName(std::string_view searchStr) {
  std::ranges::for_each(view->imageTiles, [searchStr](const auto &element) {
    const auto label = element->nameText->getText();
    const auto containsSearchStr = std::string_view{label}.find(searchStr) != std::string_view::npos;
    element->setVisibility(containsSearchStr ? gui::Visibility::Visible : gui::Visibility::Invisible);
  });
}

void ImageAssetsController::showAddImageDialog() {
  const auto varNameValidator = [this](std::string_view varName) -> std::optional<std::string> {
    if (!isValidGlslIdentifier(varName)) { return "Invalid variable name"; }
    {
      auto variables = model->getTextures();
      if (std::ranges::find(variables, std::string{varName}, [](const auto &val) { return *val->name; })
          != variables.end()) {
        return "Name is already in use";
      }

      if (disallowedNames.contains(std::string{varName})) { return "Name is already in use"; }
    }
    return std::nullopt;
  };

  interface->getDialogManager()
      .buildFileDialog(gui::FileDialogType::File)
      .size(ui::ig::Size{500, 300})
      .label("Select an image")
      .extension({{"jpg", "png", "bmp"}, "Image file", ui::ig::Color::Red})
      .onSelect([&, varNameValidator](const std::vector<std::filesystem::path> &selected) {
        VERIFY(!selected.empty());
        const auto &imgFile = selected[0];

        auto &waitDlg = interface->getDialogManager().createDialog("img_wait_dlg", "Loading image");
        waitDlg.setSize(ui::ig::Size{300, 100});
        waitDlg.createChild<ui::ig::Spinner>("img_wait_spinner", 20.f, 4);
        // TODO: clean this up
        const auto onLoadDone = [=, &waitDlg,
                                 this](const tl::expected<std::shared_ptr<Texture>, std::string> &loadingResult) {
          MainLoop::Get()->enqueue([=, &waitDlg, this] {
            waitDlg.close();
            if (loadingResult.has_value()) {
              shader_toy::GlslVariableNameInputDialogBuilder{*interface}
                  .inputValidator(varNameValidator)
                  .onInput(
                      [=](std::string_view varName) { model->addTexture(varName, imgFile, loadingResult.value()); })
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
        const auto imgInfo = imageLoader->getImageInfo(imgFile);
        if (imgInfo.has_value()) {
          ChannelCount requiredChannels{imgInfo->channels};
          if (requiredChannels.get() == 3) { requiredChannels = ChannelCount{4}; }
          imageLoader->loadTextureWithChannelsAsync(imgFile, requiredChannels, onLoadDone);
        }
      })
      .modal()
      .build();
}

void ImageAssetsController::createUIForImageModel(const std::shared_ptr<TextureAssetModel> &imgModel) {
  // TODO: placeholder if texture nullptr
  std::vector<Subscription> modelsSubscriptions;
  auto &newTile = view->addImageTile(*imgModel->name, *imgModel->texture);
  modelsSubscriptions.emplace_back(
      imgModel->name.addValueListener([&newTile](const auto &newName) { newTile.nameText->setText(newName); }));
  modelsSubscriptions.emplace_back(imgModel->texture.addValueListener([&newTile](const auto &newTexture) {
    if (newTexture == nullptr) {
      // TODO: used some placeholder for it or make it invisible
    }
    newTile.setTexture(newTexture);
  }));
  modelsSubscriptions.emplace_back(
      newTile.removeButton->addClickListener([this, imgModel] { model->removeTexture(*imgModel->name); }));
  subscriptions.emplace(imgModel, std::move(modelsSubscriptions));
}

}  // namespace pf