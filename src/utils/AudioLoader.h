//
// Created by Petr on 04/06/2022.
//

#pragma once

#include <vector>
#include <filesystem>
#include <string>
#include <tl/expected.hpp>

namespace pf {

struct AudioData {
  std::size_t sampleRate;
  std::uint8_t channelCount;
  std::vector<std::byte> data;
};

class AudioLoader {
 public:
  virtual ~AudioLoader() = 0;

  [[nodiscard]] virtual tl::expected<AudioData, std::string> loadAudioFile(const std::filesystem::path &path) = 0;
};

class AVAudioLoader : public AudioLoader {
 public:
  ~AVAudioLoader() override;
  tl::expected<AudioData, std::string> loadAudioFile(const std::filesystem::path &path) override;
};

inline AudioLoader::~AudioLoader() = default;

}  // namespace pf

