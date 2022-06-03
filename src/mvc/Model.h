//
// Created by xflajs00 on 22.05.2022.
//

#pragma once

#include "fmt/core.h"
#include <toml++/toml.h>

namespace pf {

/**
 * MVC model base class.
 */
class Model {
 public:
  [[nodiscard]] virtual std::string getDebugString() const = 0;
};

/**
 * MVC model base class which can be serialized to TOML.
 */
class SavableModel : public Model {
 public:
  [[nodiscard]] virtual toml::table toToml() const = 0;
  virtual void setFromToml(const toml::table &src) = 0;
};

}  // namespace pf

//template<std::derived_from<pf::Model> TModel>
//struct fmt::formatter<TModel> : fmt::formatter<std::string> {
//  auto format(const TModel &object, fmt::format_context &ctx) {
//    return fmt::format_to(ctx.out(), "{}", object.getDebugString());
//  }
//};
