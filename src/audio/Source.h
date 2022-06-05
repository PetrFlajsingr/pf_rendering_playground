//
// Created by Petr on 04/06/2022.
//

#pragma once

#include "Buffer.h"
#include "common.h"
#include "glm/vec3.hpp"
#include <AL/al.h>
#include <memory>
#include <pf_common/concepts/ranges.h>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/transform.hpp>

namespace pf::audio {

class Source : std::enable_shared_from_this<Source> {
  friend class Context;

 public:
  ~Source();

  void play();
  void pause();
  void stop();
  void rewind();

  void setBuffer(std::shared_ptr<Buffer> buffer);
  void setBuffers(RangeOf<std::shared_ptr<Buffer>> auto &&buffers) {
    checkOwnerAsserts();
    clearBuffers();
    appendBuffersImpl(buffers | ranges::views::transform(&Buffer::getHandle) | ranges::to_vector);
    std::ranges::copy(buffers, std::back_inserter(enqueuedBuffers));
  }

  void appendBuffer(std::shared_ptr<Buffer> buffer);
  void appendBuffers(RangeOf<std::shared_ptr<Buffer>> auto &&buffers) {
    checkOwnerAsserts();
    appendBuffersImpl(buffers | ranges::views::transform(&Buffer::getHandle) | ranges::to_vector);
    std::ranges::copy(buffers, std::back_inserter(enqueuedBuffers));
  }

  void clearBuffers();

  [[nodiscard]] const std::vector<std::shared_ptr<Buffer>> &getBuffers() const;

  void setPitch(float pitch);
  [[nodiscard]] float getPitch() const;

  void setGain(float gain);
  [[nodiscard]] float getGain() const;

  void setMinGain(float minGain);
  [[nodiscard]] float getMinGain() const;

  void setMaxGain(float maxGain);
  [[nodiscard]] float getMaxGain() const;

  void setMaxDistance(float maxDistance);
  [[nodiscard]] float getMaxDistance() const;

  void setRolloffFactor(float factor);
  [[nodiscard]] float getRolloffFactor() const;

  void setConeOuterGain(float coneOuterGain);
  [[nodiscard]] float getConeOuterGain() const;

  void setConeInnerAngle(float angle);
  [[nodiscard]] float getConeInnerAngle() const;

  void setConeOuterAngle(float angle);
  [[nodiscard]] float getConeOuterAngle() const;

  void setReferenceDistance(float referenceDistance);
  [[nodiscard]] float getReferenceDistance() const;

  void setPosition(glm::vec3 position);
  [[nodiscard]] glm::vec3 getPosition() const;

  void setVelocity(glm::vec3 velocity);
  [[nodiscard]] glm::vec3 getVelocity() const;

  void setDirection(glm::vec3 direction);
  [[nodiscard]] glm::vec3 getDirection() const;

 private:
  explicit Source(ALuint handle, const std::shared_ptr<Context> &parent);
  void checkOwnerAsserts() const;
  [[nodiscard]] float getSourceF(ALenum param) const;

  void appendBuffersImpl(std::vector<ALuint> buffers);
  void clearBuffersImpl(std::vector<ALuint> buffers);

  ALuint source;
  std::weak_ptr<Context> owner;
  std::vector<std::shared_ptr<Buffer>> enqueuedBuffers;
};

}  // namespace pf::audio
