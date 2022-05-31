//
// Created by xflajs00 on 18.05.2022.
//

#pragma once

#include "../models/ShaderVariableModel.h"
#include "../views/ShaderVariablesWindowView.h"
#include "mvc/Controller.h"
#include <unordered_set>

namespace pf {

class ShaderVariablesController : public Controller<ShaderVariablesWindowView, ShaderVariablesModel> {
 public:
  ShaderVariablesController(std::unique_ptr<ShaderVariablesWindowView> uiView,
                            std::shared_ptr<ShaderVariablesModel> mod,
                            std::shared_ptr<ui::ig::ImGuiInterface> imguiInterface);

  void filterVariablesByName(std::string_view searchStr);

  void showAddVariableDialog();

  std::unordered_set<std::string> disallowedNames;

  void show();
  void hide();

 private:
  void createUIForShaderVariableModel(const std::shared_ptr<ShaderVariableModel> &varModel);

  std::shared_ptr<ui::ig::ImGuiInterface> interface;

  std::unordered_map<std::shared_ptr<ShaderVariableModel>, std::vector<Subscription>> subscriptions;
};

}  // namespace pf
