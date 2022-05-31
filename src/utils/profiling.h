//
// Created by xflajs00 on 09.05.2022.
//

#pragma once

#include <pf_common/specializations.h>
#include <chrono>

namespace pf {

template <direct_specialization_of<std::chrono::duration> Duration = std::chrono::milliseconds>
class TimeMeasure {
 public:

  [[nodiscard]] Duration getTimeElapsed() const {
    return std::chrono::duration_cast<Duration>(std::chrono::steady_clock::now() - startTimePoint);
  }
 private:
  std::chrono::steady_clock::time_point startTimePoint = std::chrono::steady_clock::now();
};

}