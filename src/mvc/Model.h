//
// Created by xflajs00 on 22.05.2022.
//

#pragma once

#include <toml++/toml.h>

namespace pf {

class Model {};

class SavableModel : public Model {
 public:
  [[nodiscard]] virtual toml::table toToml() const = 0;
  virtual void setFromToml(const toml::table &src) = 0;
};

}  // namespace pf
