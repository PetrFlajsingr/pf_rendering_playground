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

class AudioAssetsModel : public SavableModel {
 public:
};

}  // namespace pf
