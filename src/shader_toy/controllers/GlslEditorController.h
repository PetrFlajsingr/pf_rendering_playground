//
// Created by Petr on 26/05/2022.
//

#ifndef PF_RENDERING_PLAYGROUND_GLSLEDITORCONTROLLER_H
#define PF_RENDERING_PLAYGROUND_GLSLEDITORCONTROLLER_H

#include "../models/GlslEditorModel.h"
#include "../views/GlslEditorView.h"
#include "mvc/Controller.h"

namespace pf {

class GlslEditorController : public Controller<GlslEditorView, GlslEditorModel> {
 public:
  GlslEditorController(std::unique_ptr<GlslEditorView> uiView, std::shared_ptr<GlslEditorModel> mod);

 private:
};

}  // namespace pf

#endif  //PF_RENDERING_PLAYGROUND_GLSLEDITORCONTROLLER_H
