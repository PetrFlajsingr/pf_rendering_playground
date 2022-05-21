//
// Created by xflajs00 on 17.05.2022.
//

#include "ShaderVariableModel.h"

namespace pf {
ShaderVariableModel::ShaderVariableModel(std::string_view varName, std::variant<PF_GLSL_TYPES, ui::ig::Color> val)
    : name(std::string{varName}), value(val) {}

const ShaderVariablesModel::ShaderVariableModels &ShaderVariablesModel::getVariables() const {
  return variables;
}

std::optional<std::string> ShaderVariablesModel::addVariable(std::shared_ptr<ShaderVariableModel> variable) {
  if (const auto iter = std::ranges::find(variables, *variable->name, [](const auto &var) {
      return *var->name;
    }); iter != variables.end()) {
    return "Duplicate variable name";
  }
  variableAddedEvent.notify(variables.emplace_back(std::move(variable)));
  return std::nullopt;
}

void ShaderVariablesModel::removeVariable(std::string_view varName) {
  ShaderVariableModels toRemove;
  std::ranges::remove_copy_if(variables, std::back_inserter(toRemove), [varName](const auto &var) {
    return *var->name != varName;
  });
  std::ranges::for_each(toRemove, [this](const auto &var) {
    variableRemovedEvent.notify(var);
  });
}

}  // namespace pf