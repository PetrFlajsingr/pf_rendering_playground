//
// Created by Petr on 04/06/2022.
//

#pragma once

#include "common.h"
#include <span>

namespace pf::audio {

enum class Format {
  Mono8 = AL_FORMAT_MONO8,
  Mono16 = AL_FORMAT_MONO16,
  Stereo8 = AL_FORMAT_STEREO8,
  Stereo16 = AL_FORMAT_STEREO16
};

class Context;
class Buffer {
  friend class Context;

 public:
  ~Buffer();

  [[nodiscard]] ALuint getHandle() const;

  [[nodiscard]] std::optional<OpenALError> setData(std::span<std::byte> data, Format format, std::size_t frequency);

  [[nodiscard]] std::size_t getFrequency() const;
  [[nodiscard]] std::size_t getBitDepth() const;
  [[nodiscard]] std::size_t getChannels() const;
  [[nodiscard]] std::size_t getSize() const;

 private:
  Buffer(ALuint handle, const std::shared_ptr<Context> &parent);
  void checkOwnerAsserts() const;
  ALuint buffer;
  std::weak_ptr<Context> owner;
};

}  // namespace pf::audio
