//
// Created by xflajs00 on 22.04.2022.
//

#include "GlobalVariablesPanel.h"

namespace pf::shader_toy {
namespace gui = ui::ig;

GlobalVariablesPanel::GlobalVariablesPanel(const std::string &name, gui::Size s, gui::Persistent persistent)
    : Element(name), Resizable(s), Savable(persistent) {}

toml::table GlobalVariablesPanel::toToml() const {
  auto values = toml::array{};

  std::ranges::for_each(valueRecords, [&](const auto &record) {
    const auto &[name, valueRecord] = record;
    auto recordToml = toml::table{{"name", name}, {"typeName", valueRecord->typeName}};

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

    values.push_back(recordToml);
  });

  return toml::table{{"values", values}};
}

void GlobalVariablesPanel::setFromToml(const toml::table &src) {}

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
}

void GlobalVariablesPanel::addColorVariable(std::string_view name, gui::Color initialValue) {
  if (variableExists(name)) { return; }
  auto newColorElement = std::make_unique<gui::ColorEdit<gui::ColorChooserFormat::RGBA>>(
      typename gui::ColorEdit<gui::ColorChooserFormat::RGBA>::Config{.name = getElementName(name),
                                                                     .label = name,
                                                                     .value = initialValue});

  auto newRecord =
      valueRecords.emplace(std::string{name},
                           std::make_shared<ValueRecord>(glm::vec4{initialValue.red(), initialValue.green(),
                                                                   initialValue.blue(), initialValue.alpha()}));
  newColorElement->addValueListener([valueRecord = newRecord.first->second.get()](ui::ig::Color newValue) mutable {
    valueRecord->data = glm::vec4{newValue.red(), newValue.green(), newValue.blue(), newValue.alpha()};
  });

  elements.emplace_back(std::move(newColorElement));
}

const std::unordered_map<std::string, std::shared_ptr<ValueRecord>> &GlobalVariablesPanel::getValueRecords() const {
  return valueRecords;
}

bool GlobalVariablesPanel::variableExists(std::string_view name) {
  return valueRecords.find(std::string{name}) != valueRecords.end();
}

void GlobalVariablesPanel::removeValueRecord(std::string_view name) { valueRecords.erase(std::string{name}); }

}  // namespace pf::shader_toy