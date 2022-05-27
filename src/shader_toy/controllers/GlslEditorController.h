//
// Created by Petr on 26/05/2022.
//

#pragma once

#include "../models/GlslEditorModel.h"
#include "../views/GlslEditorView.h"
#include "mvc/Controller.h"

namespace pf {

class GlslEditorController : public Controller<GlslEditorView, GlslEditorModel> {
 public:
  GlslEditorController(std::unique_ptr<GlslEditorView> uiView, std::shared_ptr<GlslEditorModel> mod);

  // TODO: change this to just WarningInfo and ErrorInfo or add compilation result to model?
  void clearWarningMarkers();
  void clearErrorMarkers();
  void addWarningMarker(const ui::ig::TextEditorMarker &marker);
  void addErrorMarker(const ui::ig::TextEditorMarker &marker);

 private:
  bool ignoreNextEditorUpdate = false;
};

}  // namespace pf
