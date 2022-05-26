//
// Created by Petr on 26/05/2022.
//

#include "OutputModel.h"

namespace pf {
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

}  // namespace pf