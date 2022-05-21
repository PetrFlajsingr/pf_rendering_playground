//
// Created by Petr on 01/05/2022.
//

#pragma once

#include <range/v3/range/conversion.hpp>
#include <range/v3/view/split.hpp>
#include <range/v3/view/transform.hpp>
#include <string>
#include <string_view>
#include <vector>

namespace pf {

[[nodiscard]] inline std::vector<std::string> splitByDelimiter(std::string_view src, char delimiter) {
  return src | ranges::views::split(delimiter) | ranges::views::transform([](auto &&part) {
           return std::string{&*part.begin(), static_cast<std::string::size_type>(ranges::distance(part))};
         })
      | ranges::to_vector;
}

}  // namespace pf

