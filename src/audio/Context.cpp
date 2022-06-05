//
// Created by Petr on 04/06/2022.
//

#include "Context.h"
#include "Buffer.h"
#include "Device.h"
#include "Listener.h"
#include "Source.h"

namespace pf::audio {

Context::~Context() {
  alcDestroyContext(context);
  DEBUG_ASSERT(!owner.expired(), "Context has to be destroyed before Device");
}

ALCcontext *Context::getHandle() const { return context; }

Context::Context(ALCcontext *handle, const std::shared_ptr<Device> &parent)
    : context(handle), owner(parent) {}

std::optional<OpenALError> Context::makeCurrent() {
  DEBUG_ASSERT(!owner.expired(), "Context's device is destroyed");
  if (!alcMakeContextCurrent(context)) {
    return OpenALError{details::checkOpenAlError().value_or(ErrorType::Unknown), "Failure activating context"};
  }
  return std::nullopt;
}

bool Context::isCurrent() const {
  DEBUG_ASSERT(!owner.expired(), "Context's device is destroyed");
  return alcGetCurrentContext() == context;
}

const std::shared_ptr<Listener> &Context::getListener() {
  DEBUG_ASSERT(!owner.expired(), "Context's device is destroyed");
  if (listener == nullptr) {
    listener = std::shared_ptr<Listener>{new Listener{shared_from_this()}};
  }
  return listener;
}

tl::expected<std::shared_ptr<Source>, OpenALError> Context::createSource() {
  DEBUG_ASSERT(!owner.expired(), "Context's device is destroyed");
  DEBUG_ASSERT(isCurrent(), "Context is not current");
  ALuint handle;
  alGenSources(1, &handle);
  return std::shared_ptr<Source>(new Source{handle, shared_from_this()});
}

tl::expected<std::shared_ptr<Buffer>, OpenALError> Context::createBuffer() {
  DEBUG_ASSERT(!owner.expired(), "Context's device is destroyed");
  DEBUG_ASSERT(isCurrent(), "Context is not current");
  ALuint handle;
  alGenBuffers(1, &handle);
  return std::shared_ptr<Buffer>(new Buffer{handle, shared_from_this()});
}

void Context::setDopplerFactor(float factor) {
  DEBUG_ASSERT(!owner.expired(), "Context's device is destroyed");
  DEBUG_ASSERT(isCurrent(), "Context is not current");
  alDopplerFactor(factor);
}

float Context::getDopplerFactor() const {
  DEBUG_ASSERT(!owner.expired(), "Context's device is destroyed");
  DEBUG_ASSERT(isCurrent(), "Context is not current");
  return alGetFloat(AL_DOPPLER_FACTOR);
}

void Context::setSpeedOfSound(float speed) {
  DEBUG_ASSERT(!owner.expired(), "Context's device is destroyed");
  DEBUG_ASSERT(isCurrent(), "Context is not current");
  alSpeedOfSound(speed);
}

float Context::getSpeedOfSound() const {
  DEBUG_ASSERT(!owner.expired(), "Context's device is destroyed");
  DEBUG_ASSERT(isCurrent(), "Context is not current");
  return alGetFloat(AL_SPEED_OF_SOUND);
}

void Context::setDistanceModel(DistanceModel model) {
  DEBUG_ASSERT(!owner.expired(), "Context's device is destroyed");
  DEBUG_ASSERT(isCurrent(), "Context is not current");
  alDistanceModel(static_cast<int>(model));
}

DistanceModel Context::getDistanceModel() const {
  DEBUG_ASSERT(!owner.expired(), "Context's device is destroyed");
  DEBUG_ASSERT(isCurrent(), "Context is not current");
  return static_cast<DistanceModel>(alGetInteger(AL_DISTANCE_MODEL));
}

void Context::suspend() {
  DEBUG_ASSERT(!owner.expired(), "Context's device is destroyed");
  alcSuspendContext(context);
}

void Context::resume() {
  DEBUG_ASSERT(!owner.expired(), "Context's device is destroyed");
  alcProcessContext(context);
}

RAII Context::suspendScoped() {
  suspend();
  return RAII{[this] { resume(); }};
}

}  // namespace pf::audio