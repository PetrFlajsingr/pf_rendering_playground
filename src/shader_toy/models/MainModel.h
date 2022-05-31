//
// Created by Petr on 30/05/2022.
//

#pragma once

#include "mvc/Model.h"

namespace pf {

class MainModel : public Model {
 public:
  [[nodiscard]] inline std::string getDebugString() const override { return ""; }
};

}  // namespace pf
