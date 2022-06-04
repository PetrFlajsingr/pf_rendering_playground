//
// Created by Petr on 04/06/2022.
//

#include "Listener.h"
#include "Context.h"
#include <AL/al.h>

namespace pf::audio {

Listener::Listener(const std::shared_ptr<Context> &parent) : owner(parent) {}

void Listener::setGain(float gain) {
  checkOwnerAsserts();
  alListenerf(AL_GAIN, gain);
}

float Listener::getGain() const {
  checkOwnerAsserts();
  float result;
  alGetListenerf(AL_GAIN, &result);
  return result;
}

void Listener::setPosition(glm::vec3 position) {
  checkOwnerAsserts();
  alListener3f(AL_POSITION, position.x, position.y, position.z);
}

glm::vec3 Listener::getPosition() const {
  checkOwnerAsserts();
  glm::vec3 result;
  alGetListener3f(AL_POSITION, &result[0], &result[1], &result[2]);
  return result;
}

void Listener::setVelocity(glm::vec3 velocity) {
  checkOwnerAsserts();
  alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z);
}

glm::vec3 Listener::getVelocity() const {
  checkOwnerAsserts();
  glm::vec3 result;
  alGetListener3f(AL_VELOCITY, &result[0], &result[1], &result[2]);
  return result;
}

void Listener::setOrientation(glm::vec3 front, glm::vec3 up) {
  checkOwnerAsserts();
  std::array<float, 6> data{front.x, front.y, front.z, up.x, up.y, up.z};
  alListenerfv(AL_ORIENTATION, data.data());
}

std::pair<glm::vec3, glm::vec3> Listener::getOrientation() const {
  checkOwnerAsserts();
  std::array<float, 6> data{};
  alListenerfv(AL_ORIENTATION, data.data());
  return std::pair{glm::vec3{data[0], data[1], data[2]}, glm::vec3{data[3], data[4], data[5]}};
}

void Listener::checkOwnerAsserts() const {
  DEBUG_ASSERT(!owner.expired(), "Listener's context is destroyed");
  DEBUG_ASSERT(owner.lock()->isCurrent(), "Listener's context is not active");
}
}  // namespace pf::audio