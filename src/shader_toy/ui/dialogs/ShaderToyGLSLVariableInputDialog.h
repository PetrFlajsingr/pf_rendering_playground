//
// Created by Petr on 30/04/2022.
//

#ifndef PF_RENDERING_PLAYGROUND_SHADERTOYGLSLVARIABLEINPUTDIALOG_H
#define PF_RENDERING_PLAYGROUND_SHADERTOYGLSLVARIABLEINPUTDIALOG_H

#include <pf_common/concepts/ranges.h>
#include <pf_imgui/ImGuiInterface.h>

namespace pf::shader_toy {

class GLSLVariableInputDialogBuilder {
 public:
  using TypeName = std::string_view;
  using VarName = std::string_view;

  explicit GLSLVariableInputDialogBuilder(ui::ig::ImGuiInterface &interface) : interface_(interface) {}

  GLSLVariableInputDialogBuilder &inputValidator(std::invocable<TypeName, VarName> auto &&validator)
    requires(std::same_as<std::optional<std::string>, std::invoke_result_t<decltype(validator), TypeName, VarName>>)
  {
    inputValidator_ = std::forward<decltype(validator)>(validator);
    return *this;
  }

  GLSLVariableInputDialogBuilder &onInput(std::invocable<TypeName, VarName> auto &&callable) {
    onInput_ = std::forward<decltype(callable)>(callable);
    return *this;
  }

  GLSLVariableInputDialogBuilder &addTypeName(TypeName typeName);
  GLSLVariableInputDialogBuilder &addTypeNames(RangeOf<TypeName> auto &&typeNames) {
    std::ranges::for_each(typeNames, [this](const auto typeName) { addTypeName(typeName); });
    return *this;
  }
  GLSLVariableInputDialogBuilder &addTypeNames(RangeOf<std::string> auto &&typeNames) {
    std::ranges::for_each(typeNames, [this](const auto typeName) { addTypeName(typeName); });
    return *this;
  }

  void show();

 private:
  std::function<std::optional<std::string>(TypeName, VarName)> inputValidator_;
  std::function<void(TypeName, VarName)> onInput_;
  std::vector<std::string> typeNames_;

  ui::ig::ImGuiInterface &interface_;
};

}  // namespace pf::shader_toy
#endif  //PF_RENDERING_PLAYGROUND_SHADERTOYGLSLVARIABLEINPUTDIALOG_H
