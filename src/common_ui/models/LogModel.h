//
// Created by xflajs00 on 16.05.2022.
//

#pragma once

#include "mvc/Model.h"

namespace pf {
class LogModel : public Model {
 public:
  inline std::string getDebugString() const override { return {}; }
};
}  // namespace pf
