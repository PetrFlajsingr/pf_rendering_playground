//
// Created by Petr on 30/04/2022.
//

#pragma once

#include "pf_common/concepts/ranges.h"
#include "pf_imgui/ImGuiInterface.h"

namespace pf::shader_toy {

class GlslVariableInputDialogBuilder {
 public:
  using TypeName = std::string_view;
  using VarName = std::string_view;

  explicit GlslVariableInputDialogBuilder(ui::ig::ImGuiInterface &interface) : interface_(interface) {}

  GlslVariableInputDialogBuilder &inputValidator(std::invocable<TypeName, VarName> auto &&validator)
    requires(std::same_as<std::optional<std::string>, std::invoke_result_t<decltype(validator), TypeName, VarName>>)
  {
    inputValidator_ = std::forward<decltype(validator)>(validator);
    return *this;
  }

  GlslVariableInputDialogBuilder &onInput(std::invocable<TypeName, VarName> auto &&callable) {
    onInput_ = std::forward<decltype(callable)>(callable);
    return *this;
  }

  GlslVariableInputDialogBuilder &addTypeName(TypeName typeName);
  GlslVariableInputDialogBuilder &addTypeNames(RangeOf<TypeName> auto &&typeNames) {
    std::ranges::for_each(typeNames, [this](const auto typeName) { addTypeName(typeName); });
    return *this;
  }
  GlslVariableInputDialogBuilder &addTypeNames(RangeOf<std::string> auto &&typeNames) {
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

class GlslVariableNameInputDialogBuilder {
 public:
  using VarName = std::string_view;

  explicit GlslVariableNameInputDialogBuilder(ui::ig::ImGuiInterface &interface) : interface_(interface) {}

  GlslVariableNameInputDialogBuilder &inputValidator(std::invocable<VarName> auto &&validator)
    requires(std::same_as<std::optional<std::string>, std::invoke_result_t<decltype(validator), VarName>>)
  {
    inputValidator_ = std::forward<decltype(validator)>(validator);
    return *this;
  }

  GlslVariableNameInputDialogBuilder &onInput(std::invocable<VarName> auto &&callable) {
    onInput_ = std::forward<decltype(callable)>(callable);
    return *this;
  }

  void show();

 private:
  std::function<std::optional<std::string>(VarName)> inputValidator_;
  std::function<void(VarName)> onInput_;

  ui::ig::ImGuiInterface &interface_;
};

}  // namespace pf::shader_toy

