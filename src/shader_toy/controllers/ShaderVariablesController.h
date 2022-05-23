//
// Created by xflajs00 on 18.05.2022.
//

#pragma once

#include "mvc/Controller.h"
#include "../views/ShaderVariablesWindowView.h"
#include "../models/ShaderVariableModel.h"

namespace pf {

class ShaderVariablesController : public Controller<ShaderVariablesWindowView, ShaderVariablesModel> {
 public:
  ShaderVariablesController(std::unique_ptr<ShaderVariablesWindowView> uiView,
                            std::shared_ptr<ShaderVariablesModel> mod, std::shared_ptr<ui::ig::ImGuiInterface> imguiInterface);

  void filterVariablesByName(std::string_view searchStr);

  void showAddVariableDialog();
  // TODO: this is unused for now, gotta add image names
  void clearDisallowedNames();
  void addDisallowedName(std::string name);

 private:
  void createUIForShaderVariableModel(const std::shared_ptr<ShaderVariableModel> &varModel);

  std::shared_ptr<ui::ig::ImGuiInterface> interface;
  std::vector<std::string> disallowedNames;

  std::unordered_map<std::shared_ptr<ShaderVariableModel>, std::vector<Subscription>> subscriptions;
};

}  // namespace pf
