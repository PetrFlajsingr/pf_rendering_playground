//
// Created by Petr on 04/06/2022.
//

#include "Source.h"
#include "Context.h"

namespace pf::audio {

Source::~Source() {
  alDeleteSources(1, &source);
  DEBUG_ASSERT(!owner.expired(), "Source has to be destroyed before Context");
}
Source::Source(ALuint handle, const std::shared_ptr<Context> &parent) : source(handle), owner(parent) {}

void Source::play() {
  checkOwnerAsserts();
  alSourcePlay(source);
}

void Source::pause() {
  checkOwnerAsserts();
  alSourcePause(source);
}

void Source::stop() {
  checkOwnerAsserts();
  alSourceStop(source);
}

void Source::rewind() {
  checkOwnerAsserts();
  alSourceRewind(source);
}

void Source::setBuffer(std::shared_ptr<Buffer> buffer) {
  checkOwnerAsserts();
  clearBuffers();
  appendBuffersImpl({buffer->getHandle()});
  enqueuedBuffers.emplace_back(std::move(buffer));
}

void Source::appendBuffer(std::shared_ptr<Buffer> buffer) {
  checkOwnerAsserts();
  appendBuffersImpl({buffer->getHandle()});
  enqueuedBuffers.emplace_back(std::move(buffer));
}

void Source::clearBuffers() {
  checkOwnerAsserts();
  clearBuffersImpl(enqueuedBuffers | ranges::views::transform(&Buffer::getHandle) | ranges::to_vector);
  enqueuedBuffers.clear();
}

const std::vector<std::shared_ptr<Buffer>> &Source::getBuffers() const { return enqueuedBuffers; }

void Source::setPitch(float pitch) {
  checkOwnerAsserts();
  alSourcef(source, AL_PITCH, pitch);
}

float Source::getPitch() const {
  checkOwnerAsserts();
  return getSourceF(AL_PITCH);
}

void Source::setGain(float gain) {
  checkOwnerAsserts();
  alSourcef(source, AL_GAIN, gain);
}

float Source::getGain() const {
  checkOwnerAsserts();
  return getSourceF(AL_GAIN);
}

void Source::setMinGain(float minGain) {
  checkOwnerAsserts();
  alSourcef(source, AL_MIN_GAIN, minGain);
}

float Source::getMinGain() const {
  checkOwnerAsserts();
  return getSourceF(AL_MIN_GAIN);
}

void Source::setMaxGain(float maxGain) {
  checkOwnerAsserts();
  alSourcef(source, AL_MAX_GAIN, maxGain);
}

float Source::getMaxGain() const {
  checkOwnerAsserts();
  return getSourceF(AL_MAX_GAIN);
}

void Source::setMaxDistance(float maxDistance) {
  checkOwnerAsserts();
  alSourcef(source, AL_MAX_DISTANCE, maxDistance);
}

float Source::getMaxDistance() const {
  checkOwnerAsserts();
  return getSourceF(AL_MAX_DISTANCE);
}

void Source::setRolloffFactor(float factor) {
  checkOwnerAsserts();
  alSourcef(source, AL_ROLLOFF_FACTOR, factor);
}

float Source::getRolloffFactor() const {
  checkOwnerAsserts();
  return getSourceF(AL_ROLLOFF_FACTOR);
}

void Source::setConeOuterGain(float coneOuterGain) {
  checkOwnerAsserts();
  alSourcef(source, AL_CONE_OUTER_GAIN, coneOuterGain);
}

float Source::getConeOuterGain() const {
  checkOwnerAsserts();
  return getSourceF(AL_CONE_OUTER_GAIN);
}

void Source::setConeInnerAngle(float angle) {
  checkOwnerAsserts();
  alSourcef(source, AL_CONE_INNER_ANGLE, angle);
}

float Source::getConeInnerAngle() const {
  checkOwnerAsserts();
  return getSourceF(AL_CONE_INNER_ANGLE);
}

void Source::setConeOuterAngle(float angle) {
  checkOwnerAsserts();
  alSourcef(source, AL_CONE_OUTER_ANGLE, angle);
}

float Source::getConeOuterAngle() const {
  checkOwnerAsserts();
  return getSourceF(AL_CONE_OUTER_ANGLE);
}

void Source::setReferenceDistance(float distance) {
  checkOwnerAsserts();
  alSourcef(source, AL_REFERENCE_DISTANCE, distance);
}

float Source::getReferenceDistance() const {
  checkOwnerAsserts();
  return getSourceF(AL_REFERENCE_DISTANCE);
}

void Source::setPosition(glm::vec3 position) {
  checkOwnerAsserts();
  alSource3f(source, AL_POSITION, position.x, position.y, position.z);
}

glm::vec3 Source::getPosition() const {
  checkOwnerAsserts();
  glm::vec3 result;
  alGetSource3f(source, AL_POSITION, &result.x, &result.y, &result.z);
  return result;
}

void Source::setVelocity(glm::vec3 velocity) {
  checkOwnerAsserts();
  alSource3f(source, AL_VELOCITY, velocity.x, velocity.y, velocity.z);
}

glm::vec3 Source::getVelocity() const {
  checkOwnerAsserts();
  glm::vec3 result;
  alGetSource3f(source, AL_VELOCITY, &result.x, &result.y, &result.z);
  return result;
}

void Source::setDirection(glm::vec3 direction) {
  DEBUG_ASSERT(direction.x + direction.y + direction.z - 1.0f < 0.000001f, "Direction is not normalized");
  checkOwnerAsserts();
  alSource3f(source, AL_VELOCITY, direction.x, direction.y, direction.z);
}

glm::vec3 Source::getDirection() const {
  checkOwnerAsserts();
  glm::vec3 result;
  alGetSource3f(source, AL_VELOCITY, &result.x, &result.y, &result.z);
  return result;
}

void Source::checkOwnerAsserts() const {
  DEBUG_ASSERT(!owner.expired(), "Context is destroyed");
  DEBUG_ASSERT(owner.lock()->isCurrent(), "Context is not current");
}

float Source::getSourceF(ALenum param) const {
  float result;
  alGetSourcef(source, param, &result);
  return result;
}

void Source::appendBuffersImpl(std::vector<ALuint> buffers) {
  alSourceQueueBuffers(source, static_cast<ALsizei>(buffers.size()), buffers.data());
}

void Source::clearBuffersImpl(std::vector<ALuint> buffers) {
  alSourceUnqueueBuffers(source, static_cast<ALsizei>(buffers.size()), buffers.data());
}

}  // namespace pf::audio