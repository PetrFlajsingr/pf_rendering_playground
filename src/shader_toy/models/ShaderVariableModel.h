//
// Created by xflajs00 on 17.05.2022.
//

#pragma once

#include "mvc/Model.h"
#include "mvc/reactive.h"
#include "utils/glsl/glsl_typenames.h"
#include <algorithm>
#include <optional>
#include <pf_imgui/Color.h>
#include <range/v3/view/transform.hpp>
#include <string>
#include <variant>

namespace pf {
// TODO: change this so it can't be changed later (value type) - maybe wrap it in asserts somehow
class ShaderVariableModel : public SavableModel {
 public:
  ShaderVariableModel(std::string_view varName, std::variant<PF_GLSL_TYPES, ui::ig::Color> val);
  Observable<std::string> name;
  Observable<std::variant<PF_GLSL_TYPES, ui::ig::Color>> value;

  [[nodiscard]] toml::table toToml() const override;
  void setFromToml(const toml::table &src) override;

  [[nodiscard]] std::string getDebugString() const override;
};

class ShaderVariablesModel : public SavableModel {
  using ShaderVariableModels = std::vector<std::shared_ptr<ShaderVariableModel>>;
  template<typename ...Args>
  using Event = ClassEvent<ShaderVariablesModel, Args...>;
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

  [[nodiscard]] std::string getDebugString() const override;

 private:
  ShaderVariableModels variables;
};

}  // namespace pf
