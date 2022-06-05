//
// Created by Petr on 04/06/2022.
//

#pragma once
#include "common.h"
#include "pf_common/RAII.h"
#include "tl/expected.hpp"
#include <AL/alc.h>

namespace pf::audio {

enum class DistanceModel {
  Inverse = AL_INVERSE_DISTANCE,
  InverseClamped = AL_INVERSE_DISTANCE_CLAMPED,
  Linear = AL_LINEAR_DISTANCE,
  LinearClamped = AL_LINEAR_DISTANCE_CLAMPED,
  Exponent = AL_EXPONENT_DISTANCE,
  ExponentClamped = AL_EXPONENT_DISTANCE_CLAMPED,
  None = AL_NONE
};

class Source;
class Listener;
class Buffer;
// TODO: attributes
class Context : std::enable_shared_from_this<Context> {
  friend class Device;

 public:
  ~Context();
  [[nodiscard]] ALCcontext *getHandle() const;

  [[nodiscard]] std::optional<OpenALError> makeCurrent();
  [[nodiscard]] bool isCurrent() const;

  [[nodiscard]] const std::shared_ptr<Listener> &getListener();
  [[nodiscard]] std::shared_ptr<const Listener> getListener() const;

  [[nodiscard]] tl::expected<std::shared_ptr<Source>, OpenALError> createSource();
  [[nodiscard]] tl::expected<std::shared_ptr<Buffer>, OpenALError> createBuffer();

  void setDopplerFactor(float factor);
  [[nodiscard]] float getDopplerFactor() const;

  void setSpeedOfSound(float speed);
  [[nodiscard]] float getSpeedOfSound() const;

  void setDistanceModel(DistanceModel model);
  [[nodiscard]] DistanceModel getDistanceModel() const;

  void suspend();
  void resume();
  [[nodiscard]] RAII suspendScoped();

 private:
  Context(ALCcontext *handle, const std::shared_ptr<Device> &parent);
  ALCcontext *context;
  std::weak_ptr<Device> owner;
  std::shared_ptr<Listener> listener;
};

}  // namespace pf::audio
