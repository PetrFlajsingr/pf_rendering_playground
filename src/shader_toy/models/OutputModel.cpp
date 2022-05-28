//
// Created by Petr on 26/05/2022.
//

#include "OutputModel.h"
#include <assert.hpp>

namespace pf {

NormalizedPosition::NormalizedPosition() : x_(0), y_(0) {}

NormalizedPosition::NormalizedPosition(float xVal, float yVal) : x_(xVal), y_(yVal) {
  DEBUG_ASSERT(0.f <= x_ && x_ <= 1.f);
  DEBUG_ASSERT(0.f <= y_ && y_ <= 1.f);
}

float NormalizedPosition::x() const { return x_; }

float NormalizedPosition::y() const { return y_; }

void NormalizedPosition::x(float val) {
  DEBUG_ASSERT(0.f <= val && val <= 1.f);
  x_ = val;
}

void NormalizedPosition::y(float val) {
  DEBUG_ASSERT(0.f <= val && val <= 1.f);
  y_ = val;
}

OutputModel::OutputModel(std::pair<std::uint32_t, std::uint32_t> res, std::shared_ptr<Texture> tex)
    : resolution{res}, texture{std::move(tex)} {}

toml::table OutputModel::toToml() const {
  return toml::table{{"width", resolution->first}, {"height", resolution->second}};
}

void OutputModel::setFromToml(const toml::table &src) {
  auto newResolution = *resolution;
  if (const auto iter = src.find("width"); iter != src.end()) {
    if (const auto widthVal = iter->second.as_integer(); widthVal != nullptr) {
      newResolution.first = static_cast<std::uint32_t>(widthVal->get());
    }
  }
  if (const auto iter = src.find("height"); iter != src.end()) {
    if (const auto heightVal = iter->second.as_integer(); heightVal != nullptr) {
      newResolution.second = static_cast<std::uint32_t>(heightVal->get());
    }
  }
  *resolution.modify() = newResolution;
}
std::string OutputModel::getDebugString() const {
  return fmt::format("resolution: '{}x{}', texture: '{}', mouse UV pos: '{}x{}', texture hovered: '{}'",
                     resolution->first, resolution->second,
                     *texture == nullptr ? std::string{"nullptr"} : (*texture)->getDebugString(),
                     mousePositionOnImageUV->x(), mousePositionOnImageUV->y(), *textureHovered);
}

}  // namespace pf