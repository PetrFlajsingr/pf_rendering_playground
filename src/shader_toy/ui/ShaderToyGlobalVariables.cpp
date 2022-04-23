//
// Created by xflajs00 on 22.04.2022.
//

#include "ShaderToyGlobalVariables.h"

namespace pf {
namespace gui = ui::ig;

ShaderToyGlobalVariablesPanel::ShaderToyGlobalVariablesPanel(const std::string &name, ui::ig::Size s,
                                                             ui::ig::Persistent persistent)
    : Element(name), Resizable(s), Savable(persistent) {}

toml::table ShaderToyGlobalVariablesPanel::toToml() const { return toml::table(); }

void ShaderToyGlobalVariablesPanel::setFromToml(const toml::table &src) {}

void ShaderToyGlobalVariablesPanel::renderImpl() {
  ui::ig::Element *toRemove = nullptr;
  std::size_t index = 0;
  std::ranges::for_each(elements, [&](auto &element) {
    element->render();
    ImGui::SameLine();
    if (ImGui::Button((std::string{"Remove##rm_btn"} + std::to_string(index++)).c_str())) { toRemove = element.get(); }
    ImGui::Separator();
  });
  if (toRemove != nullptr) {
    removeValueRecord(getVarNameFromElementName(toRemove->getName()));
    const auto [rmBeg, rmEnd] = std::ranges::remove(elements, toRemove, &std::unique_ptr<ui::ig::Element>::get);
    elements.erase(rmBeg, rmEnd);
  }
}
constexpr static auto ELEMENT_POSTFIX = "glsl_var";
static auto ELEMENT_POSTFIX_LEN = std::strlen(ELEMENT_POSTFIX);
std::string ShaderToyGlobalVariablesPanel::getElementName(std::string_view varName) {
  return std::string{varName} + ELEMENT_POSTFIX;
}

std::string ShaderToyGlobalVariablesPanel::getVarNameFromElementName(std::string_view elementName) {
  return std::string{elementName.substr(0, elementName.size() - ELEMENT_POSTFIX_LEN)};
}

void ShaderToyGlobalVariablesPanel::addBoolVariable(std::string_view name, bool initialValue) {
  if (variableExists(name)) { return; }
  auto newCheckboxElement = std::make_unique<ui::ig::Checkbox>(getElementName(name), std::string{name}, initialValue);
  addValueRecord(*newCheckboxElement, name);
  elements.emplace_back(std::move(newCheckboxElement));
}

void ShaderToyGlobalVariablesPanel::addColorVariable(std::string_view name, ui::ig::Color initialValue) {
  if (variableExists(name)) { return; }
  auto newColorElement = std::make_unique<ui::ig::ColorEdit<glm::vec4>>(typename ui::ig::ColorEdit<glm::vec4>::Config{
      .name = getElementName(name),
      .label = name,
      .value = glm::vec4{initialValue.red(), initialValue.green(), initialValue.blue(), initialValue.alpha()}});
  addValueRecord(*newColorElement, name);
  elements.emplace_back(std::move(newColorElement));
}

const std::unordered_map<std::string, std::shared_ptr<ValueRecord>> &
ShaderToyGlobalVariablesPanel::getValueRecords() const {
  return valueRecords;
}

bool ShaderToyGlobalVariablesPanel::variableExists(std::string_view name) {
  return valueRecords.find(std::string{name}) != valueRecords.end();
}

void ShaderToyGlobalVariablesPanel::removeValueRecord(std::string_view name) { valueRecords.erase(std::string{name}); }

}  // namespace pf