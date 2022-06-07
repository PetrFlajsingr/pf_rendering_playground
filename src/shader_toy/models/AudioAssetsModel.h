//
// Created by Petr on 6/5/2022.
//

#pragma once

#include "audio/Buffer.h"
#include "gpu/Buffer.h"
#include "mvc/Model.h"
#include "mvc/reactive.h"
#include "utils/enums.h"
#include <chrono>
#include <filesystem>

namespace pf {

class AudioAssetModel : public SavableModel {
 public:
  AudioAssetModel(std::string assetName, bool playback, std::shared_ptr<audio::Buffer> aBuffer,
                  std::shared_ptr<gpu::Buffer> gBuffer, AudioPCMFormat dataFormat, std::chrono::seconds duration,
                  std::filesystem::path path);

  Observable<std::string> name;
  Observable<bool> enablePlayback;
  Observable<std::shared_ptr<audio::Buffer>> audioBuffer;
  Observable<std::shared_ptr<gpu::Buffer>> gpuBuffer;
  Observable<AudioPCMFormat> format;
  Observable<std::chrono::seconds> length;
  Observable<std::filesystem::path> assetPath;

  [[nodiscard]] std::string getDebugString() const override;
  [[nodiscard]] toml::table toToml() const override;
  void setFromToml(const toml::table &src) override;
};
// TODO: create a template for thus model container kinda class
class AudioAssetsModel : public SavableModel {
  using AudioModels = std::vector<std::shared_ptr<AudioAssetModel>>;
  template<typename... Args>
  using Event = ClassEvent<AudioAssetsModel, Args...>;
  using AudioAddedEvent = Event<std::shared_ptr<AudioAssetModel>>;
  using AudioRemovedEvent = Event<std::shared_ptr<AudioAssetModel>>;

 public:
  [[nodiscard]] const AudioModels &getAudioModels() const;

  AudioAddedEvent audioAddedEvent;
  AudioRemovedEvent audioRemovedEvent;
  std::optional<std::string> addAudio(std::string name, bool playback, std::shared_ptr<audio::Buffer> aBuffer,
                                      std::shared_ptr<gpu::Buffer> gBuffer, AudioPCMFormat dataFormat,
                                      std::chrono::seconds duration, std::filesystem::path path);
  std::optional<std::string> addAudio(std::shared_ptr<AudioAssetModel> audioAssetModel);

  void removeAudio(std::string_view modelName);

  void clearAudios();

  [[nodiscard]] toml::table toToml() const override;
  void setFromToml(const toml::table &src) override;

  [[nodiscard]] std::string getDebugString() const override;

 private:
  AudioModels audioModels;
};

}  // namespace pf
