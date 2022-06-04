//
// Created by Petr on 04/06/2022.
//

#pragma once

#include "common.h"
#include "glm/vec3.hpp"
#include <memory>

namespace pf::audio {

class Listener : std::enable_shared_from_this<Listener> {
  friend class Context;
 public:

  void setGain(float gain);
  [[nodiscard]] float getGain() const;

  void setPosition(glm::vec3 position);
  [[nodiscard]] glm::vec3 getPosition() const;

  void setVelocity(glm::vec3 velocity);
  [[nodiscard]] glm::vec3 getVelocity() const;

  void setOrientation(glm::vec3 front, glm::vec3 up);
  [[nodiscard]] std::pair<glm::vec3, glm::vec3> getOrientation() const;

 private:
  explicit Listener(const std::shared_ptr<Context> &parent);
  void checkOwnerAsserts() const;
  std::weak_ptr<Context> owner;
};

}  // namespace pf

