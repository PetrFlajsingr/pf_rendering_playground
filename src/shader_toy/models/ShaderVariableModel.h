//
// Created by xflajs00 on 17.05.2022.
//

#pragma once

#include "mvc/reactive.h"
#include "mvc/Model.h"
#include "utils/glsl_typenames.h"
#include <pf_imgui/Color.h>
#include <string>
#include <optional>
#include <variant>
#include <algorithm>
#include <range/v3/view/transform.hpp>

namespace pf {
// TODO: change this so it can't be changed later (value type)
class ShaderVariableModel : public SavableModel {
 public:
  ShaderVariableModel(std::string_view varName, std::variant<PF_GLSL_TYPES, ui::ig::Color> val);
  Observable<std::string> name;
  Observable<std::variant<PF_GLSL_TYPES, ui::ig::Color>> value;

  [[nodiscard]] toml::table toToml() const override;
  void setFromToml(const toml::table &src) override;
};

class ShaderVariablesModel : public SavableModel {
  using ShaderVariableModels = std::vector<std::shared_ptr<ShaderVariableModel>>;
  template<typename ...Args>
  using Event = Event<ShaderVariablesModel, Args...>;
  using VariableAddedEvent = Event<std::shared_ptr<ShaderVariableModel>>;
  using VariableRemovedEvent = Event<std::shared_ptr<ShaderVariableModel>>;

 public:

  [[nodiscard]] const ShaderVariableModels &getVariables() const;

  VariableAddedEvent variableAddedEvent;
  VariableRemovedEvent variableRemovedEvent;

  std::optional<std::string> addVariable(std::string_view varName, OneOf<PF_GLSL_TYPES, ui::ig::Color> auto value) {
    return addVariable(std::make_shared<ShaderVariableModel>(varName, std::forward<decltype(value)>(value)));
  }

  std::optional<std::string> addVariable(std::shared_ptr<ShaderVariableModel> variable);

  void removeVariable(std::string_view varName);

  void clearVariables();

  [[nodiscard]] toml::table toToml() const override;
  void setFromToml(const toml::table &src) override;

 private:
  ShaderVariableModels variables;
};

}  // namespace pf
