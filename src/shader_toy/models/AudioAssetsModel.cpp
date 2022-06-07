//
// Created by Petr on 6/5/2022.
//

#include "AudioAssetsModel.h"
#include "utils/algorithms.h"
#include <fmt/chrono.h>

namespace pf {

AudioAssetModel::AudioAssetModel(std::string assetName, bool playback, std::shared_ptr<audio::Buffer> aBuffer,
                                 std::shared_ptr<gpu::Buffer> gBuffer, AudioPCMFormat dataFormat,
                                 std::chrono::seconds duration, std::filesystem::path path)
    : name(std::move(assetName)), enablePlayback(playback), audioBuffer(std::move(aBuffer)),
      gpuBuffer(std::move(gBuffer)), format(dataFormat), length(duration), assetPath(std::move(path)) {}

std::string AudioAssetModel::getDebugString() const {
  // TODO: audio buffer
  return fmt::format("name: '{}', playback: '{}', gpu buffer: '{}', format: '{}', length: '{}', path: '{}'", *name,
                     *enablePlayback, *gpuBuffer != nullptr ? (*gpuBuffer)->getDebugString() : "null",
                     magic_enum::enum_name(*format), *length, assetPath->string());
}

toml::table AudioAssetModel::toToml() const {
  return toml::table{{"name", *name}, {"enablePlayback", *enablePlayback}, {"path", assetPath->string()}};
}

void AudioAssetModel::setFromToml(const toml::table &src) {
  if (const auto iter = src.find("enablePlayback"); iter != src.end()) {
    if (const auto enablePlaybackVal = iter->second.as_boolean(); enablePlaybackVal != nullptr) {
      *enablePlayback.modify() = enablePlaybackVal->get();
    }
  }
  if (const auto iter = src.find("path"); iter != src.end()) {
    if (const auto pathStr = iter->second.as_string(); pathStr != nullptr) { *assetPath.modify() = pathStr->get(); }
  }
  if (const auto iter = src.find("name"); iter != src.end()) {
    if (const auto nameStr = iter->second.as_string(); nameStr != nullptr) { *name.modify() = nameStr->get(); }
  }
}

const AudioAssetsModel::AudioModels &AudioAssetsModel::getAudioModels() const { return audioModels; }

std::optional<std::string> AudioAssetsModel::addAudio(std::string name, bool playback,
                                                       std::shared_ptr<audio::Buffer> aBuffer,
                                                       std::shared_ptr<gpu::Buffer> gBuffer, AudioPCMFormat dataFormat,
                                                       std::chrono::seconds duration, std::filesystem::path path) {
  if (const auto iter = std::ranges::find(audioModels, name, [](const auto &audio) { return *audio->name; });
      iter != audioModels.end()) {
    return "Duplicate audio asset name";
  }
  audioAddedEvent.notify(audioModels.emplace_back(std::make_shared<AudioAssetModel>(
      std::move(name), playback, std::move(aBuffer), std::move(gBuffer), dataFormat, duration, std::move(path))));
  return std::nullopt;
}

std::optional<std::string> AudioAssetsModel::addAudio(std::shared_ptr<AudioAssetModel> audioAssetModel) {
  if (const auto iter =
          std::ranges::find(audioModels, *audioAssetModel->name, [](const auto &audio) { return *audio->name; });
      iter != audioModels.end()) {
    return "Duplicate audio asset name";
  }
  audioAddedEvent.notify(audioModels.emplace_back(std::move(audioAssetModel)));
  return std::nullopt;
}

void AudioAssetsModel::removeAudio(std::string_view modelName) {
  const auto toRemove =
      erase_and_extract_if(audioModels, [modelName](const auto &audio) { return *audio->name == modelName; });
  std::ranges::for_each(toRemove, [this](const auto &var) { audioRemovedEvent.notify(var); });
}

void AudioAssetsModel::clearAudios() {
  const auto toRemove = audioModels;
  audioModels.clear();
  std::ranges::for_each(toRemove, [this](const auto &audio) { audioRemovedEvent.notify(audio); });
}

toml::table AudioAssetsModel::toToml() const {
  auto audioArray = toml::array{};
  std::ranges::transform(audioModels, std::back_inserter(audioArray), &AudioAssetModel::toToml);
  return toml::table{{"audioAssets", std::move(audioArray)}};
}

void AudioAssetsModel::setFromToml(const toml::table &src) {
  if (const auto iter = src.find("audioAssets"); iter != src.end()) {
    if (const auto audioArray = iter->second.as_array(); audioArray != nullptr) {
      std::ranges::for_each(*audioArray, [&](const auto &record) {
        if (const auto tblPtr = record.as_table(); tblPtr != nullptr) {
          auto newAudio = std::make_shared<AudioAssetModel>("", false, nullptr, nullptr, AudioPCMFormat::U8Mono,
                                                            std::chrono::seconds{0}, "");
          newAudio->setFromToml(*tblPtr);
          addAudio(std::move(newAudio));
        }
      });
    }
  }
}

std::string AudioAssetsModel::getDebugString() const {
  return fmt::format("audio assets count: '{}'", audioModels.size());
}

}  // namespace pf