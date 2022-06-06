//
// Created by Petr on 6/5/2022.
//

#include "AudioAssetsModel.h"
#include <fmt/chrono.h>

namespace pf {

std::string AudioAssetModel::getDebugString() const {
  // TODO: audio buffer
  return fmt::format("playback: '{}', gpu buffer: '{}', format: '{}', length: '{}', path: '{}'", *enablePlayback,
                     *gpuBuffer != nullptr ? (*gpuBuffer)->getDebugString() : "null", magic_enum::enum_name(*format),
                     *length, assetPath->string());
}

toml::table AudioAssetModel::toToml() const {
  return toml::table{{"enablePlayback", *enablePlayback}, {"path", assetPath->string()}};
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
}

}  // namespace pf