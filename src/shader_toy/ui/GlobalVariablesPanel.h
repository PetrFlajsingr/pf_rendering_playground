//
// Created by xflajs00 on 22.04.2022.
//

#pragma once

#include <algorithm>
#include <glm/glm.hpp>
#include <pf_imgui/ImGuiInterface.h>
#include <pf_imgui/elements/Checkbox.h>
#include <pf_imgui/elements/ColorChooser.h>
#include <pf_imgui/elements/DragInput.h>
#include <pf_imgui/elements/MatrixDragInput.h>
#include <pf_imgui/interface/Element.h>
#include <pf_imgui/interface/Savable.h>
#include <pf_imgui/layouts/VerticalLayout.h>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/addressof.hpp>
#include <utils/glsl_typenames.h>

namespace pf::shader_toy {

struct ValueRecord {
  template<typename T>
  ValueRecord(T initValue, std::string name, bool isColor = false);

  std::string name;
  std::variant<PF_GLSL_TYPES> data;
  const std::string typeName;
  const bool isColor;
};

// TODO: support all glsl types
class GlobalVariablesPanel : public ui::ig::Element, public ui::ig::Resizable, public ui::ig::Savable {
 public:
  GlobalVariablesPanel(const std::string &name, ui::ig::Size s, ui::ig::Persistent persistent);

  template<OneOf<PF_GLSL_TYPES> T>
    requires(OneOf<T, PF_IMGUI_DRAG_TYPE_LIST> || OneOf<T, PF_IMGUI_GLM_MAT_TYPES>)
  void addDragVariable(std::string_view name, T initialValue);

  void addBoolVariable(std::string_view name, bool initialValue);

  void addColorVariable(std::string_view name, ui::ig::Color initialValue);

  [[nodiscard]] toml::table toToml() const override;
  void setFromToml(const toml::table &src) override;

  [[nodiscard]] const std::vector<std::shared_ptr<ValueRecord>> &getValueRecords() const;

  Subscription addVariablesChangedListener(std::invocable auto &&listener) {
    return variablesChangedObservable.addListener(std::forward<decltype(listener)>(listener));
  }

 protected:
  void renderImpl() override;

 private:
  [[nodiscard]] bool variableExists(std::string_view name);

  template<typename T>
  void addValueRecord(ui::ig::ValueObservable<T> &observable, std::string_view name);
  void removeValueRecord(std::string_view name);

  static std::string getElementName(std::string_view varName);
  static std::string getVarNameFromElementName(std::string_view elementName);

  std::vector<std::unique_ptr<ui::ig::Element>> elements;
  std::vector<std::shared_ptr<ValueRecord>> valueRecords;

  ui::ig::Observable_impl<> variablesChangedObservable;
};

template<typename T>
ValueRecord::ValueRecord(T initValue, std::string name, bool isColor)
    : name(std::move(name)), data(initValue), typeName(getGLSLTypeName<T>()), isColor(isColor) {
  assert(!(!std::same_as<glm::vec4, T> && isColor));
}

template<OneOf<PF_GLSL_TYPES> T>
  requires(OneOf<T, PF_IMGUI_DRAG_TYPE_LIST> || OneOf<T, PF_IMGUI_GLM_MAT_TYPES>)
void GlobalVariablesPanel::addDragVariable(std::string_view name, T initialValue) {
  if (variableExists(name)) { return; }
  if constexpr (OneOf<T, PF_IMGUI_GLM_MAT_TYPES>) {
    using ParamType = typename ui::ig::MatrixDragInput<T>::ParamType;
    constexpr auto speed = static_cast<ParamType>(std::same_as<ParamType, float> ? 0.1f : 1.f);
    constexpr auto min = std::numeric_limits<ParamType>::lowest();
    constexpr auto max = std::numeric_limits<ParamType>::max();
    auto newDragElement = std::make_unique<ui::ig::MatrixDragInput<T>>(getElementName(name), std::string{name}, speed,
                                                                       min, max, initialValue);
    addValueRecord(*newDragElement, name);
    elements.emplace_back(std::move(newDragElement));
  } else {
    using ParamType = typename ui::ig::DragInput<T>::ParamType;
    constexpr auto speed = static_cast<ParamType>(std::same_as<ParamType, float> ? 0.1f : 1.f);
    constexpr auto min = std::numeric_limits<ParamType>::lowest();
    constexpr auto max = std::numeric_limits<ParamType>::max();
    auto newDragElement =
        std::make_unique<ui::ig::DragInput<T>>(typename ui::ig::DragInput<T>::Config{.name = getElementName(name),
                                                                                     .label = name,
                                                                                     .speed = speed,
                                                                                     .min = min,
                                                                                     .max = max,
                                                                                     .value = initialValue});
    addValueRecord(*newDragElement, name);
    elements.emplace_back(std::move(newDragElement));
    variablesChangedObservable.notify();
  }
}

template<typename T>
void GlobalVariablesPanel::addValueRecord(ui::ig::ValueObservable<T> &observable, std::string_view name) {
  auto newRecord = valueRecords.emplace_back(std::make_shared<ValueRecord>(observable.getValue(), std::string{name}));
  observable.addValueListener([valueRecord = newRecord](const T &newValue) mutable { valueRecord->data = newValue; });
}

}  // namespace pf::shader_toy
