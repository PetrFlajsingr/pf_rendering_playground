//
// Created by Petr on 04/06/2022.
//

#include "Buffer.h"
#include "Context.h"

namespace pf::audio {

Buffer::~Buffer() { alDeleteBuffers(1, &buffer); }

ALuint Buffer::getHandle() const { return buffer; }

Buffer::Buffer(ALuint handle, const std::shared_ptr<Context> &parent) : buffer(handle), owner(parent) {}

std::optional<OpenALError> Buffer::setData(std::span<std::byte> data, Format format, std::size_t frequency) {
  checkOwnerAsserts();
  alBufferData(buffer, static_cast<ALenum>(format), data.data(), static_cast<ALsizei>(data.size()),
               static_cast<ALsizei>(frequency));
  const auto err = details::checkOpenAlError();
  if (err.has_value()) { return OpenALError{err.value(), "Error setting buffer's data"}; }
  return std::nullopt;
}

std::size_t Buffer::getFrequency() const {
  checkOwnerAsserts();
  int result;
  alGetBufferi(buffer, AL_FREQUENCY, &result);
  return static_cast<std::size_t>(result);
}

std::size_t Buffer::getBitDepth() const {
  checkOwnerAsserts();
  int result;
  alGetBufferi(buffer, AL_BITS, &result);
  return static_cast<std::size_t>(result);
}

std::size_t Buffer::getChannels() const {
  checkOwnerAsserts();
  int result;
  alGetBufferi(buffer, AL_CHANNELS, &result);
  return static_cast<std::size_t>(result);
}

std::size_t Buffer::getSize() const {
  checkOwnerAsserts();
  int result;
  alGetBufferi(buffer, AL_SIZE, &result);
  return static_cast<std::size_t>(result);
}

void Buffer::checkOwnerAsserts() const {
  DEBUG_ASSERT(!owner.expired(), "Buffer's context is destroyed");
  DEBUG_ASSERT(owner.lock()->isCurrent(), "Buffer's context is not active");
}

}  // namespace pf::audio