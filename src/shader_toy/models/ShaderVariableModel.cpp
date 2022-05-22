//
// Created by xflajs00 on 17.05.2022.
//

#include "ShaderVariableModel.h"
#include "pf_imgui/serialization.h"

namespace pf {
ShaderVariableModel::ShaderVariableModel(std::string_view varName, std::variant<PF_GLSL_TYPES, ui::ig::Color> val)
    : name(std::string{varName}), value(val) {}

toml::table ShaderVariableModel::toToml() const {
  auto result = toml::table{{"name", *name}};
  std::visit(
      [&]<typename T>(T value) {
        if constexpr (std::same_as<T, ui::ig::Color>) {
          result.insert("type", "color");
          result.insert("value",
                        ui::ig::serializeGlmVec(glm::vec4{value.red(), value.green(), value.blue(), value.alpha()}));
        } else {
          result.insert("type", getGLSLTypeName<T>());
          if constexpr (OneOf<T, bool, float, unsigned int, int>) { result.insert("value", value); }
          if constexpr (OneOf<T, glm::vec2, glm::vec3, glm::vec4, glm::ivec2, glm::ivec3, glm::ivec4, glm::uvec2,
                              glm::uvec3, glm::uvec4, glm::bvec2, glm::bvec3, glm::bvec4>) {
            result.insert("value", ui::ig::serializeGlmVec(value));
          }
          if constexpr (OneOf<T, glm::mat2, glm::mat3, glm::mat4>) {
            result.insert("value", ui::ig::serializeGlmMat(value));
          }
        }
      },
      *value);
  return result;
}

void ShaderVariableModel::setFromToml(const toml::table &src) {
  if (const auto iter = src.find("name"); iter != src.end()) {
    if (const auto nameStr = iter->second.as_string(); nameStr != nullptr) { *name.modify() = nameStr->get(); }
  }
  if (const auto iterType = src.find("type"); iterType != src.end()) {
    if (const auto typeStr = iterType->second.as_string(); typeStr != nullptr) {
      const auto valueIter = src.find("value");
      if (valueIter == src.end()) { return; }
      if (*typeStr == "color") {
        if (auto valuePtr = valueIter->second.as_array(); valuePtr != nullptr) {
          if (auto vecValue = ui::ig::safeDeserializeGlmVec<glm::vec4>(*valuePtr); vecValue.has_value()) {
            *value.modify() = ui::ig::Color::RGB(vecValue->r, vecValue->g, vecValue->b, vecValue->a);
          }
        }
      } else {
        getTypeForGlslName(typeStr->get(), [&]<typename T> {
          if constexpr (std::same_as<T, bool>) {
            if (auto valuePtr = valueIter->second.as_boolean(); valuePtr != nullptr) {
              *value.modify() = valuePtr->get();
            }
          }
          if constexpr (std::same_as<T, float>) {
            if (auto valuePtr = valueIter->second.as_floating_point(); valuePtr != nullptr) {
              *value.modify() = static_cast<float>(valuePtr->get());
            }
          }
          if constexpr (OneOf<T, unsigned int, int>) {
            if (auto valuePtr = valueIter->second.as_integer(); valuePtr != nullptr) {
              *value.modify() = static_cast<T>(valuePtr->get());
            }
          }
          if constexpr (OneOf<T, glm::vec2, glm::vec3, glm::vec4, glm::ivec2, glm::ivec3, glm::ivec4, glm::uvec2,
                              glm::uvec3, glm::uvec4, glm::bvec2, glm::bvec3, glm::bvec4>) {
            if (auto valuePtr = valueIter->second.as_array(); valuePtr != nullptr) {
              if (auto vecValue = ui::ig::safeDeserializeGlmVec<T>(*valuePtr); vecValue.has_value()) {
                *value.modify() = vecValue.value();
              }
            }
          }
          if constexpr (OneOf<T, glm::mat2, glm::mat3, glm::mat4>) {
            if (auto valuePtr = valueIter->second.as_array(); valuePtr != nullptr) {
              if (const auto matValue = ui::ig::safeDeserializeGlmMat<T>(*valuePtr); matValue.has_value()) {
                *value.modify() = matValue.value();
              }
            }
          }
        });
      }
    }
  }
}

const ShaderVariablesModel::ShaderVariableModels &ShaderVariablesModel::getVariables() const { return variables; }

std::optional<std::string> ShaderVariablesModel::addVariable(std::shared_ptr<ShaderVariableModel> variable) {
  if (const auto iter = std::ranges::find(variables, *variable->name, [](const auto &var) { return *var->name; });
      iter != variables.end()) {
    return "Duplicate variable name";
  }
  variableAddedEvent.notify(variables.emplace_back(std::move(variable)));
  return std::nullopt;
}

void ShaderVariablesModel::removeVariable(std::string_view varName) {
  ShaderVariableModels toRemove;
  std::ranges::remove_copy_if(variables, std::back_inserter(toRemove),
                              [varName](const auto &var) { return *var->name != varName; });
  std::ranges::for_each(toRemove, [this](const auto &var) { variableRemovedEvent.notify(var); });
}

void ShaderVariablesModel::clearVariables() {
  const auto toRemove = variables;
  variables.clear();
  std::ranges::for_each(toRemove, [this](const auto &var) { variableRemovedEvent.notify(var); });
}

toml::table ShaderVariablesModel::toToml() const {
  auto varArray = toml::array{};
  std::ranges::transform(variables, std::back_inserter(varArray), &ShaderVariableModel::toToml);
  return toml::table{{"variables", std::move(varArray)}};
}

void ShaderVariablesModel::setFromToml(const toml::table &src) {
  if (const auto iter = src.find("variables"); iter != src.end()) {
    if (const auto varsArray = iter->second.as_array(); varsArray != nullptr) {
      std::ranges::for_each(*varsArray, [&](const auto &record) {
        if (const auto tblPtr = record.as_table(); tblPtr != nullptr) {
          auto newVariable = std::make_shared<ShaderVariableModel>("", 0);
          newVariable->setFromToml(*tblPtr);
          addVariable(std::move(newVariable));
        }
      });
    }
  }
}

}  // namespace pf