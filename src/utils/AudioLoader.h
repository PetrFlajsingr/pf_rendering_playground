//
// Created by Petr on 04/06/2022.
//

#pragma once

#include "enums.h"
#include <filesystem>
#include <pf_common/parallel/ThreadPool.h>
#include <string>
#include <tl/expected.hpp>
#include <vector>

namespace pf {

struct AudioData {
  std::size_t sampleRate;
  std::uint8_t channelCount;
  std::vector<std::byte> data;

  [[nodiscard]] std::chrono::seconds getLength() const;
};

class AudioLoader {
 public:
  explicit AudioLoader(std::shared_ptr<ThreadPool> threadPool);
  virtual ~AudioLoader() = 0;

  [[nodiscard]] virtual tl::expected<AudioData, std::string> loadAudioFile(const std::filesystem::path &path,
                                                                           AudioPCMFormat requestedFormat,
                                                                           std::optional<int> requestedSampleRate) = 0;
  [[nodiscard]] virtual void loadAudioFileAsync(const std::filesystem::path &path, AudioPCMFormat requestedFormat,
                                            std::optional<int> requestedSampleRate,
                                            std::function<void(tl::expected<AudioData, std::string>)> onLoadDone);

 protected:
  void enqueue(std::invocable auto &&fnc) { pool->enqueue(std::forward<decltype(fnc)>(fnc)); };

 private:
  std::shared_ptr<ThreadPool> pool;
};

class AVAudioLoader : public AudioLoader {
 public:
  AVAudioLoader(std::shared_ptr<ThreadPool> threadPool);
  ~AVAudioLoader() override;
  tl::expected<AudioData, std::string> loadAudioFile(const std::filesystem::path &path, AudioPCMFormat requestedFormat,
                                                     std::optional<int> requestedSampleRate) override;
};

inline AudioLoader::~AudioLoader() = default;

}  // namespace pf
