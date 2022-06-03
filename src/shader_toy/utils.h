//
// Created by xflajs00 on 23.04.2022.
//

#pragma once

#include <algorithm>
#include <cctype>
#include <string_view>

namespace pf {

inline bool isValidGlslIdentifier(std::string_view identifier) {
  if (identifier.empty()) { return false; }
  if (identifier.find(' ') != std::string_view::npos) { return false; }
  if (!::isalpha(identifier[0])) { return false; }
  if (!std::ranges::all_of(identifier, ::isalnum)) { return false; }
  return true;
}

}  // namespace pf
