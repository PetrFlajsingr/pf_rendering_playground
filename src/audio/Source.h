//
// Created by Petr on 04/06/2022.
//

#pragma once

#include <AL/al.h>
#include "glm/vec3.hpp"
#include <memory>
#include "common.h"

namespace pf::audio {

// TODO: stopped at alSourceQueueBuffers
class Source : std::enable_shared_from_this<Source> {
  friend class Context;
 public:
  ~Source();

  void play();
  void pause();
  void stop();
  void rewind();

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
  ALuint source;
  std::weak_ptr<Context> owner;
};

}  // namespace pf

