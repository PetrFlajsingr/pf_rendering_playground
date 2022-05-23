//
// Created by xflajs00 on 18.05.2022.
//

#pragma once

#include "mvc/View.h"
#include "pf_imgui/elements/Button.h"
#include "pf_imgui/elements/Checkbox.h"
#include "pf_imgui/elements/ColorChooser.h"
#include "pf_imgui/elements/DragInput.h"
#include "pf_imgui/elements/InputText.h"
#include "pf_imgui/elements/MatrixDragInput.h"
#include "pf_imgui/elements/Separator.h"
#include "pf_imgui/layouts/HorizontalLayout.h"
#include "pf_imgui/layouts/VerticalLayout.h"
#include "utils/glsl_typenames.h"
#include <pf_common/specializations.h>
#include <pf_imgui/ImGuiInterface.h>
#include <pf_imgui/interface/Element.h>

namespace pf {

// TODO move this out
template<std::derived_from<ui::ig::Element> T>
class ShaderVariableRecordElement : public T {
 public:
  template<typename... Args>
  explicit ShaderVariableRecordElement(Args &&...args) : T{std::forward<Args>(args)...} {}

  Subscription addRemoveClickListener(std::invocable auto &&listener) {
    return removeObservableImpl.addListener(std::forward<decltype(listener)>(listener));
  }

  void renderImpl() override;

 private:
  ui::ig::Observable_impl<> removeObservableImpl;
};

class ShaderVariablesWindowView : public UIViewWindow {
 public:
  ShaderVariablesWindowView(ui::ig::ImGuiInterface &interface, std::string_view windowName,
                            std::string_view windowTitle);

  template<OneOf<PF_GLSL_TYPES> T>
    requires(OneOf<T, PF_IMGUI_DRAG_TYPE_LIST> || OneOf<T, PF_IMGUI_GLM_MAT_TYPES>)
  auto &addDragInput(std::string_view name, T initialValue);

  ShaderVariableRecordElement<ui::ig::Checkbox> &addCheckboxInput(std::string_view name, bool initialValue);

  ShaderVariableRecordElement<ui::ig::ColorEdit<ui::ig::ColorChooserFormat::RGBA>> &
  addColorInput(std::string_view name, ui::ig::Color initialValue);

  // TODO: add remove function which safely deletes froms layout and vector

  // clang-format off
  ui::ig::HorizontalLayout *controlsLayout;
    ui::ig::Button *addButton;
    ui::ig::Separator *controlsSep;
    ui::ig::InputText *searchTextInput;
  ui::ig::VerticalLayout *varsLayout;
    std::vector<ui::ig::Element*> elements;
  // clang-format on
};

template<std::derived_from<ui::ig::Element> T>
void ShaderVariableRecordElement<T>::renderImpl() {
  T::renderImpl();
  ImGui::BeginHorizontal("lay_mid_rm");
  ImGui::Spring(1.f);
  if (ImGui::Button("Remove")) { removeObservableImpl.notify(); }
  ImGui::Spring(1.f);
  ImGui::EndHorizontal();
  ImGui::Separator();
}

template<OneOf<PF_GLSL_TYPES> T>
  requires(OneOf<T, PF_IMGUI_DRAG_TYPE_LIST> || OneOf<T, PF_IMGUI_GLM_MAT_TYPES>)
auto &ShaderVariablesWindowView::addDragInput(std::string_view name, T initialValue) {
  if constexpr (OneOf<T, PF_IMGUI_GLM_MAT_TYPES>) {
    using ParamType = typename ui::ig::MatrixDragInput<T>::ParamType;
    constexpr auto speed = static_cast<ParamType>(std::same_as<ParamType, float> ? 0.1f : 1.f);
    constexpr auto min = std::numeric_limits<ParamType>::lowest();
    constexpr auto max = std::numeric_limits<ParamType>::max();
    auto &newDragElement = varsLayout->createChild<ShaderVariableRecordElement<ui::ig::MatrixDragInput<T>>>(
        std::string{name}, std::string{name}, speed, min, max, initialValue);
    elements.emplace_back(&newDragElement);
    return newDragElement;
  } else {
    using ParamType = typename ui::ig::DragInput<T>::ParamType;
    constexpr auto speed = static_cast<ParamType>(std::same_as<ParamType, float> ? 0.1f : 1.f);
    constexpr auto min = std::numeric_limits<ParamType>::lowest();
    constexpr auto max = std::numeric_limits<ParamType>::max();
    auto &newDragElement = varsLayout->createChild<ShaderVariableRecordElement<ui::ig::DragInput<T>>>(
        typename ui::ig::DragInput<T>::Config{.name = std::string{name},
                                              .label = name,
                                              .speed = speed,
                                              .min = min,
                                              .max = max,
                                              .value = initialValue});
    elements.emplace_back(&newDragElement);
    return newDragElement;
  }
}

}  // namespace pf
