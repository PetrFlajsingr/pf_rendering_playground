//
// Created by Petr on 6/7/2022.
//

#include "AudioAssetsController.h"
#include "gpu/opengl/Buffer.h"
#include "shader_toy/dialogs/GlslLVariableInputDialog.h"
#include "shader_toy/utils.h"
#include <pf_imgui/elements/Spinner.h>
#include <pf_mainloop/MainLoop.h>

namespace pf {

namespace gui = ui::ig;

AudioAssetsController::AudioAssetsController(std::unique_ptr<AudioAssetsView> uiView,
                                             std::shared_ptr<AudioAssetsModel> mod,
                                             std::shared_ptr<ui::ig::ImGuiInterface> imguiInterface,
                                             std::shared_ptr<AudioLoader> audioLoader,
                                             std::shared_ptr<gpu::RenderThread> renderingThread,
                                             std::shared_ptr<audio::Context> aContext)
    : Controller<AudioAssetsView, AudioAssetsModel>(std::move(uiView), std::move(mod)),
      interface(std::move(imguiInterface)), loader(std::move(audioLoader)), renderThread(std::move(renderingThread)),
      audioContext(std::move(aContext)) {
  VERIFY(interface != nullptr);
  VERIFY(loader != nullptr);
  VERIFY(renderThread != nullptr);
  VERIFY(audioContext != nullptr);

  view->addButton->addClickListener(std::bind_front(&AudioAssetsController::showAddAssetDialog, this));
  view->searchTextInput->addValueListener(std::bind_front(&AudioAssetsController::filterAssetsByName, this));

  std::ranges::for_each(model->getAudioModels(), std::bind_front(&AudioAssetsController::createUIForAudioModel, this));

  model->audioAddedEvent.addEventListener(std::bind_front(&AudioAssetsController::createUIForAudioModel, this));
  model->audioRemovedEvent.addEventListener([this](const auto &audioModel) {
    MainLoop::Get()->forceEnqueue([this, audioModel] {
      const auto iter = subscriptions.find(audioModel);
      DEBUG_ASSERT(iter != subscriptions.end(), "Model should always have subscriptions record");
      std::ranges::for_each(iter->second, &Subscription::unsubscribe);
      subscriptions.erase(iter);
      // FIXME: this is messed up when model's name changes - need to fix that maybe with some unique identifier or something
      // same goes for other models/UIs
      const auto [rmBeg, rmEnd] = std::ranges::remove(view->audioTiles, *audioModel->name, &gui::Element::getName);
      view->audioTiles.erase(rmBeg, rmEnd);
      view->audioLayout->removeChild(*audioModel->name);
    });
  });
}

void AudioAssetsController::filterAssetsByName(std::string_view searchStr) {
  std::ranges::for_each(view->audioTiles, [searchStr](const auto &element) {
    const auto label = element->getLabel();
    const auto containsSearchStr = std::string_view{label}.find(searchStr) != std::string_view::npos;
    element->setVisibility(containsSearchStr ? gui::Visibility::Visible : gui::Visibility::Invisible);
  });
}

void AudioAssetsController::showAddAssetDialog() {
  const auto varNameValidator = [this](std::string_view varName) -> std::optional<std::string> {
    if (!isValidGlslIdentifier(varName)) { return "Invalid variable name"; }
    {
      auto variables = model->getAudioModels();
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
      .label("Select an audio file")
      .extension({{"mp3", "ogg", "wav"}, "Audio file", ui::ig::Color::Red})
      .onSelect([&, varNameValidator](const std::vector<std::filesystem::path> &selected) {
        VERIFY(!selected.empty());
        const auto &audioFile = selected[0];

        auto &waitDlg = interface->getDialogManager().createDialog("audio_wait_dlg", "Loading audio file");
        waitDlg.setSize(ui::ig::Size{300, 100});
        waitDlg.createChild<ui::ig::Spinner>("audio_wait_spinner", 20.f, 4);

        const auto onLoadDone = [=, this, &waitDlg](tl::expected<AudioData, std::string> audioData) {
          MainLoop::Get()->enqueue([&waitDlg] { waitDlg.close(); });
          if (audioData.has_value()) {
            shader_toy::GlslVariableNameInputDialogBuilder{*interface}
                .inputValidator(varNameValidator)
                .onInput([=, audioData = std::move(*audioData)](std::string_view varName) {
                  renderThread->enqueue([=, this, audioData = std::move(audioData), varName = std::string{varName}] {
                    auto gBuffer = std::make_shared<gpu::OpenGlBuffer>();
                    gBuffer->create(std::span{audioData.data});  // FIXME: add check
                    MainLoop::Get()->enqueue([=, this, gBuffer = std::move(gBuffer), audioData = std::move(audioData),
                                              varName = std::move(varName)]() mutable {
                      auto aBuffer = audioContext->createBuffer().value();  // FIXME: add check
                      [[maybe_unused]] auto res = aBuffer->setData(std::span{audioData.data}, audio::Format::Mono8,
                                                                   audioData.sampleRate);  // FIXME: add check
                      auto aSource = audioContext->createSource().value();                 // FIXME: add check
                      model->addAudio(varName, false, std::move(aSource), std::move(gBuffer), AudioPCMFormat::U8Mono,
                                      audioData.getLength(), audioFile);
                    });
                  });
                })
                .show();
          } else {
            MainLoop::Get()->enqueue([this, audioFile, audioData = std::move(audioData)] {
              interface->getNotificationManager()
                  .createNotification("notif_loading_err", "Audio loading failed")
                  .createChild<ui::ig::Text>(
                      "notif_txt", fmt::format("Audio loading failed: '{}'.\n{}", audioData.error(), audioFile))
                  .setColor<ui::ig::style::ColorOf::Text>(ui::ig::Color::Red);
            });
          }
        };

        loader->loadAudioFileAsync(audioFile, AudioPCMFormat::U8Mono, std::nullopt, onLoadDone);
      })
      .modal()
      .build();
}

void AudioAssetsController::createUIForAudioModel(const std::shared_ptr<AudioAssetModel> &audioModel) {
  std::vector<Subscription> modelsSubscriptions;

  auto &record = view->addAssetElement(*audioModel->name, std::string{magic_enum::enum_name(*audioModel->format)},
                                       *audioModel->enablePlayback, *audioModel->length);
  modelsSubscriptions.emplace_back(
      record.addPlayListener([audioModel](bool play) { *audioModel->enablePlayback.modify() = play; }));
  modelsSubscriptions.emplace_back(
      record.addRemoveClickListener([this, audioModel] { model->removeAudio(*audioModel->name); }));

  modelsSubscriptions.emplace_back(
      audioModel->name.addValueListener([&record](const auto &newName) { record.setLabel(newName); }));
  modelsSubscriptions.emplace_back(audioModel->format.addValueListener(
      [&record](const auto newFormat) { record.setFormat(std::string{magic_enum::enum_name(newFormat)}); }));
  modelsSubscriptions.emplace_back(
      audioModel->length.addValueListener([&record](const auto newLength) { record.setLength(newLength); }));
  modelsSubscriptions.emplace_back(
      audioModel->enablePlayback.addValueListener([&record](const auto newPlay) { record.setPlay(newPlay); }));

  subscriptions.emplace(audioModel, std::move(modelsSubscriptions));
}

void AudioAssetsController::show() { view->getWindow().setVisibility(gui::Visibility::Visible); }

void AudioAssetsController::hide() { view->getWindow().setVisibility(gui::Visibility::Invisible); }

}  // namespace pf