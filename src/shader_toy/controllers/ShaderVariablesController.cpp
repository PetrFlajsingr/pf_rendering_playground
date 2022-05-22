//
// Created by xflajs00 on 18.05.2022.
//

#include "ShaderVariablesController.h"
#include "pf_mainloop/MainLoop.h"
#include "shader_toy/ui/dialogs/GlslLVariableInputDialog.h"
#include "shader_toy/utils.h"
#include "spdlog/spdlog.h"

namespace pf {

namespace gui = ui::ig;
// FIXME
constexpr static auto isUnsupportedType = []<typename T>() {
  return OneOf<T, unsigned int, glm::uvec2, glm::uvec3, glm::uvec4, glm::bvec2, glm::bvec3, glm::bvec4>;
};

ShaderVariablesController::ShaderVariablesController(std::unique_ptr<ShaderVariablesWindowView> uiView,
                                                     std::shared_ptr<ShaderVariablesModel> mod,
                                                     std::shared_ptr<gui::ImGuiInterface> imguiInterface)
    : Controller(std::move(uiView), std::move(mod)), interface(std::move(imguiInterface)) {
  view->addButton->addClickListener(std::bind_front(&ShaderVariablesController::showAddVariableDialog, this));
  // TODO: initial setup from passed model

  view->searchTextInput->addValueListener(std::bind_front(&ShaderVariablesController::filterVariablesByName, this));

  std::ranges::for_each(model->getVariables(),
                        std::bind_front(&ShaderVariablesController::createUIForShaderVariableModel, this));

  model->variableAddedEvent.addEventListener(
      std::bind_front(&ShaderVariablesController::createUIForShaderVariableModel, this));
  // needs to be done in the next loop FIXME
  model->variableRemovedEvent.addEventListener([this](const auto &varModel) {
    MainLoop::Get()->forceEnqueue([this, varModel] {
      const auto iter = subscriptions.find(varModel);
      std::ranges::for_each(iter->second, &Subscription::unsubscribe);
      subscriptions.erase(iter);
      const auto [rmBeg, rmEnd] = std::ranges::remove(view->elements, *varModel->name, &gui::Element::getName);
      view->elements.erase(rmBeg, rmEnd);
      view->varsLayout->removeChild(*varModel->name);
    });
  });
}

void ShaderVariablesController::filterVariablesByName(std::string_view searchStr) {
  std::ranges::for_each(view->elements, [searchStr](const auto &element) {
    if (const auto labellable = dynamic_cast<gui::Labellable *>(element); labellable != nullptr) {
      const auto label = labellable->getLabel();
      const auto containsSearchStr = std::string_view{label}.find(searchStr) != std::string_view::npos;
      element->setVisibility(containsSearchStr ? gui::Visibility::Visible : gui::Visibility::Invisible);
    } else {
      element->setVisibility(gui::Visibility::Visible);
    }
  });
}

void ShaderVariablesController::showAddVariableDialog() {
  const auto varNameValidator = [&](std::string_view varName) -> std::optional<std::string> {
    if (!isValidGlslIdentifier(varName)) { return "Invalid variable name"; }
    {
      auto variables = model->getVariables();
      if (std::ranges::find(variables, std::string{varName}, [](const auto &val) { return *val->name; })
          != variables.end()) {
        return "Name is already in use";
      }

      if (std::ranges::find(varNamesInUse, std::string{varName}) != varNamesInUse.end()) {
        return "Name is already in use";
      }
    }
    return std::nullopt;
  };

  const auto varSelectValidator = [=](std::string_view typeName,
                                      std::string_view varName) -> std::optional<std::string> {
    if (typeName.empty()) { return "Select a type"; }
    if (const auto nameErr = varNameValidator(varName); nameErr.has_value()) { return nameErr.value(); }
    bool unsupportedType = false;
    getTypeForGlslName(typeName, [&]<typename T>() { unsupportedType = isUnsupportedType.operator()<T>(); });
    if (unsupportedType) { return "Selected type is not currently supported"; }
    return std::nullopt;
  };
  constexpr auto COLOR_RECORD = "Color (vec4)";

  shader_toy::GlslVariableInputDialogBuilder{*interface}
      .addTypeNames(getGlslTypeNames())
      .addTypeName(COLOR_RECORD)
      .inputValidator(varSelectValidator)
      .onInput([&](std::string_view typeName, std::string_view varName) {
        if (typeName == COLOR_RECORD) {
          model->addVariable(varName, gui::Color::White);
        } else {
          getTypeForGlslName(typeName, [&]<typename T>() {
            if constexpr (isUnsupportedType.operator()<T>()) {
              assert(false
                     && "This should never happen");  // this needs to be here due to template instantiation errors
            } else if constexpr (std::same_as<T, bool>) {
              model->addVariable(varName, false);
            } else {
              model->addVariable(varName, T{});
            }
          });
        }
      })
      .show();
}

void ShaderVariablesController::clearVarNamesInUse() { varNamesInUse.clear(); }

void ShaderVariablesController::addVarNameInUse(std::string name) { varNamesInUse.emplace_back(std::move(name)); }

void ShaderVariablesController::createUIForShaderVariableModel(const std::shared_ptr<ShaderVariableModel> &varModel) {
  std::visit(
      [&]<typename T>(T value) {
        const auto registerListeners = [&](auto &uiElement) {
          std::vector<Subscription> subscription;
          subscription.push_back(
              varModel->name.addValueListener([&](const auto &newLabel) { uiElement.setLabel(newLabel); }));
          subscription.push_back(varModel->value.addValueListener(
              [&](const auto &newValue) { uiElement.setValue(std::get<T>(newValue)); }));
          uiElement.setValue(value);
          subscription.push_back(
              uiElement.addValueListener([varModel](const auto &newVal) { *varModel->value.modify() = newVal; }));
          subscription.push_back(
              uiElement.addRemoveClickListener([this, varModel] { model->removeVariable(*varModel->name); }));
          subscriptions.emplace(varModel, std::move(subscription));
        };
        if constexpr (isUnsupportedType.operator()<T>()) {

        } else if constexpr (std::same_as<T, gui::Color>) {
          auto &colorInput = view->addColorInput(*varModel->name, value);
          registerListeners(colorInput);
        } else if constexpr (std::same_as<T, bool>) {
          auto &boolInput = view->addCheckboxInput(*varModel->name, value);
          registerListeners(boolInput);
        } else {
          auto &dragInput = view->addDragInput(*varModel->name, value);
          registerListeners(dragInput);
        }
      },
      *varModel->value);
}

}  // namespace pf