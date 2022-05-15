//
// Created by xflajs00 on 22.04.2022.
//

#include "GlobalVariablesPanel.h"

namespace pf::shader_toy {
namespace gui = ui::ig;

GlobalVariablesPanel::GlobalVariablesPanel(const std::string &name, gui::Size s, gui::Persistent persistent)
    : Element(name), Resizable(s), Savable(persistent) {}
// FIXME: doesn't support color type
toml::table GlobalVariablesPanel::toToml() const {
  auto values = toml::array{};

  std::ranges::for_each(valueRecords, [&](const auto &valueRecord) {
    auto recordToml = toml::table{{"name", valueRecord->name}, {"typeName", valueRecord->typeName}};

    std::visit(
        [&]<typename T>(T value) {
          if constexpr (OneOf<T, bool, float, unsigned int, int>) { recordToml.insert("value", value); }
          if constexpr (OneOf<T, glm::vec2, glm::vec3, glm::vec4, glm::ivec2, glm::ivec3, glm::ivec4, glm::uvec2,
                              glm::uvec3, glm::uvec4, glm::bvec2, glm::bvec3, glm::bvec4>) {
            recordToml.insert("value", ui::ig::serializeGlmVec(value));
          }
          if constexpr (OneOf<T, glm::mat2, glm::mat3, glm::mat4>) {
            toml::array rows;
            for (std::size_t row = 0; row < T::length(); ++row) { rows.push_back(ui::ig::serializeGlmVec(value[row])); }
            recordToml.insert("value", rows);
          }
        },
        valueRecord->data);
    if (valueRecord->isColor) { recordToml.insert("isColor", true); }

    values.push_back(recordToml);
  });

  return toml::table{{"values", values}};
}

void GlobalVariablesPanel::setFromToml(const toml::table &src) {
  if (const auto valArrToml = src.find("values"); valArrToml != src.end()) {
    if (const auto valArr = valArrToml->second.as_array(); valArr != nullptr) {
      for (const auto &valToml : *valArr) {
        if (const auto val = valToml.as_table(); val != nullptr) {
          const auto nameIter = val->find("name");
          if (nameIter == val->end()) { continue; }
          const auto typeNameIter = val->find("typeName");
          if (typeNameIter == val->end()) { continue; }
          const auto valueIter = val->find("value");
          if (valueIter == val->end()) { continue; }
          auto isColor = false;
          if (const auto isColorIter = val->find("isColor"); isColorIter != val->end()) {
            isColor = isColorIter->second.value_or(false);
          }

          if (auto name = nameIter->second.as_string(); name != nullptr) {
            if (auto typeName = typeNameIter->second.as_string(); typeName != nullptr) {
              getTypeForGlslName(typeName->get(), [&]<typename T> {
                if constexpr (std::same_as<T, bool>) {
                  if (auto value = valueIter->second.as_boolean(); value != nullptr) {
                    addBoolVariable(name->get(), value->get());
                  }
                }
                if constexpr (OneOf<T, PF_IMGUI_DRAG_TYPE_LIST> || OneOf<T, PF_IMGUI_GLM_MAT_TYPES>) {
                  if constexpr (std::same_as<T, float>) {
                    if (auto value = valueIter->second.as_floating_point(); value != nullptr) {
                      addDragVariable(name->get(), static_cast<float>(value->get()));
                    }
                  }
                  if constexpr (OneOf<T, unsigned int, int>) {
                    if (auto value = valueIter->second.as_integer(); value != nullptr) {
                      addDragVariable(name->get(), static_cast<T>(value->get()));
                    }
                  }
                  if constexpr (OneOf<T, glm::vec2, glm::vec3, glm::vec4, glm::ivec2, glm::ivec3, glm::ivec4,
                                      glm::uvec2, glm::uvec3, glm::uvec4, glm::bvec2, glm::bvec3, glm::bvec4>) {
                    if (auto value = valueIter->second.as_array(); value != nullptr) {
                      if (auto vecValue = ui::ig::safeDeserializeGlmVec<T>(*value); vecValue.has_value()) {
                        if constexpr (std::same_as<T, glm::vec4>) {
                          if (isColor) {
                            addColorVariable(name->get(),
                                             gui::Color::RGB(vecValue.value().r, vecValue.value().g, vecValue.value().b,
                                                             vecValue.value().a));
                          } else {
                            addDragVariable(name->get(), vecValue.value());
                          }
                        } else {
                          addDragVariable(name->get(), vecValue.value());
                        }
                      }
                    }
                  }
                  if constexpr (OneOf<T, glm::mat2, glm::mat3, glm::mat4>) {
                    if (auto value = valueIter->second.as_array(); value != nullptr) {
                      if (value->size() != T::length()) { return; }
                      T matValue;
                      std::size_t i{};
                      for (const auto &row : *value) {
                        if (auto rowArray = row.as_array(); rowArray != nullptr) {
                          if (auto vecValue = ui::ig::safeDeserializeGlmVec<T::col_type>(*rowArray);
                              vecValue.has_value()) {
                            matValue[i] = vecValue.value();
                          } else {
                            return;
                          }
                        } else {
                          return;
                        }
                        ++i;
                      }
                      addDragVariable(name->get(), matValue);
                    }
                  }
                }
              });
            }
          }
        }
      }
    }
  }
}

void GlobalVariablesPanel::renderImpl() {
  gui::Element *toRemove = nullptr;
  std::size_t index = 0;
  std::ranges::for_each(elements, [&](auto &element) {
    element->render();
    ImGui::SameLine();
    if (ImGui::Button((std::string{"Remove##rm_btn"} + std::to_string(index++)).c_str())) { toRemove = element.get(); }
    ImGui::Separator();
  });
  if (toRemove != nullptr) {
    removeValueRecord(getVarNameFromElementName(toRemove->getName()));
    const auto [rmBeg, rmEnd] = std::ranges::remove(elements, toRemove, &std::unique_ptr<gui::Element>::get);
    elements.erase(rmBeg, rmEnd);
  }
}
constexpr static auto ELEMENT_POSTFIX = "glsl_var";
static auto ELEMENT_POSTFIX_LEN = std::strlen(ELEMENT_POSTFIX);
std::string GlobalVariablesPanel::getElementName(std::string_view varName) {
  return std::string{varName} + ELEMENT_POSTFIX;
}

std::string GlobalVariablesPanel::getVarNameFromElementName(std::string_view elementName) {
  return std::string{elementName.substr(0, elementName.size() - ELEMENT_POSTFIX_LEN)};
}

void GlobalVariablesPanel::addBoolVariable(std::string_view name, bool initialValue) {
  if (variableExists(name)) { return; }
  auto newCheckboxElement = std::make_unique<gui::Checkbox>(getElementName(name), std::string{name}, initialValue);
  addValueRecord(*newCheckboxElement, name);
  elements.emplace_back(std::move(newCheckboxElement));
  variablesChangedObservable.notify();
}

void GlobalVariablesPanel::addColorVariable(std::string_view name, gui::Color initialValue) {
  if (variableExists(name)) { return; }
  auto newColorElement = std::make_unique<gui::ColorEdit<gui::ColorChooserFormat::RGBA>>(
      typename gui::ColorEdit<gui::ColorChooserFormat::RGBA>::Config{.name = getElementName(name),
                                                                     .label = name,
                                                                     .value = initialValue});

  auto newRecord = valueRecords.emplace_back(std::make_shared<ValueRecord>(
      glm::vec4{initialValue.red(), initialValue.green(), initialValue.blue(), initialValue.alpha()}, std::string{name},
      true));
  newColorElement->addValueListener([valueRecord = newRecord](ui::ig::Color newValue) mutable {
    valueRecord->data = glm::vec4{newValue.red(), newValue.green(), newValue.blue(), newValue.alpha()};
  });

  elements.emplace_back(std::move(newColorElement));
  variablesChangedObservable.notify();
}

const std::vector<std::shared_ptr<ValueRecord>> &GlobalVariablesPanel::getValueRecords() const { return valueRecords; }

bool GlobalVariablesPanel::variableExists(std::string_view name) {
  return std::ranges::find(valueRecords, std::string{name}, &ValueRecord::name) != valueRecords.end();
}

void GlobalVariablesPanel::removeValueRecord(std::string_view name) {
  const auto [remBg, remEnd] = std::ranges::remove(valueRecords, std::string{name}, &ValueRecord::name);
  valueRecords.erase(remBg, remEnd);
  variablesChangedObservable.notify();
}

}  // namespace pf::shader_toy