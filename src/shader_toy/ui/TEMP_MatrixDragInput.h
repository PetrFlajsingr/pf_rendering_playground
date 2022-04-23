//
// Created by xflajs00 on 23.04.2022.
//

#ifndef PF_RENDERING_PLAYGROUND_TEMP_MATRIXDRAGINPUT_H
#define PF_RENDERING_PLAYGROUND_TEMP_MATRIXDRAGINPUT_H

#include <glm/glm.hpp>
#include <pf_imgui/elements/DragInput.h>
#include <pf_imgui/interface/Customizable.h>
#include <pf_imgui/interface/Element.h>
#include <pf_imgui/interface/Labellable.h>
#include <pf_imgui/interface/Savable.h>
#include <pf_imgui/interface/ValueObservable.h>

namespace pf::ui::ig {
#define PF_IMGUI_GLM_MAT_TYPES                                                                                         \
  glm::mat2, glm::mat3, glm::mat4, glm::mat2x3, glm::mat2x4, glm::mat3x2, glm::mat3x4, glm::mat4x2, glm::mat4x3
// TODO: move this to pf_imgui eventually
template<OneOf<PF_IMGUI_GLM_MAT_TYPES> M>
class MatrixDragInput
    : public Element,
      public ValueObservable<M>,
      public Labellable,
      public Savable,
      public ColorCustomizable<style::ColorOf::Text, style::ColorOf::TextDisabled, style::ColorOf::DragDropTarget,
                               style::ColorOf::FrameBackground, style::ColorOf::FrameBackgroundHovered,
                               style::ColorOf::FrameBackgroundActive, style::ColorOf::NavHighlight,
                               style::ColorOf::Border, style::ColorOf::BorderShadow, style::ColorOf::SliderGrab,
                               style::ColorOf::SliderGrabActive>,
      public StyleCustomizable<style::Style::FramePadding, style::Style::FrameRounding, style::Style::FrameBorderSize> {
 public:
  using ParamType = details::DragInputUnderlyingType<typename M::col_type>;
  constexpr static auto Height = M::length();
  constexpr static auto Width = M::col_type::length();
  MatrixDragInput(const std::string &name, const std::string &label, ParamType speed, ParamType min, ParamType max,
                  M value, Persistent persistent = Persistent::No)
      : Element(name), ValueObservable<M>(value), Labellable(label), Savable(persistent), speed(speed), min(min),
        max(max) {}

  [[nodiscard]] toml::table toToml() const override {
    toml::array rows;
    for (std::size_t row = 0; row < Height; ++row) {
      rows.push_back(serializeGlmVec(ValueObservable<M>::getValue()[row]));
    }
    return toml::table{{"value", rows}};
  }
  void setFromToml(const toml::table &src) override {
    if (auto newValIter = src.find("value"); newValIter != src.end()) {
      if (auto newVal = newValIter->second.as_array(); newVal != nullptr) {
        M value;
        std::size_t row = 0;
        for (const auto &rowVal : *newVal) {
          if (auto rowValArr = rowVal.as_array(); rowValArr != nullptr) {
            if (const auto rowVec = safeDeserializeGlmVec<typename M::col_type>(*rowValArr); rowVec.has_value()) {
              value[row] = rowVec.value();
            } else {
              return;
            }
          }
          ++row;
        }
        ValueObservable<M>::setValueAndNotifyIfChanged(value);
      }
    }
  }

 protected:
  void renderImpl() override {
    ImGui::Text(getLabel().c_str());
    auto valueChanged = false;
    for (std::size_t row = 0; row < Height; ++row) {
      valueChanged = valueChanged
          | ImGui::DragScalarN((std::string{"##drag_"} + std::to_string(row)).c_str(), ImGuiDataType_Float,
                               glm::value_ptr((*ValueObservable<M>::getValueAddress())[row]), Width, speed, &min, &max);
    }
    if (valueChanged) { ValueObservable<M>::notifyValueChanged(); }
  }

 private:
  float min;
  float max;
  float speed;
};
}  // namespace pf::ui::ig

#endif  //PF_RENDERING_PLAYGROUND_TEMP_MATRIXDRAGINPUT_H
