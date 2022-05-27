//
// Created by Petr on 26/05/2022.
//

#include "GlslEditorController.h"

namespace pf {

GlslEditorController::GlslEditorController(std::unique_ptr<GlslEditorView> uiView, std::shared_ptr<GlslEditorModel> mod)
    : Controller(std::move(uiView), std::move(mod)) {
  view->compileButton->addClickListener([this] { model->compilationRequested.notify(); });

  view->timePausedCheckbox->setValue(*model->timePaused);
  view->timePausedCheckbox->addValueListener([this](auto timePaused) { *model->timePaused.modify() = timePaused; });
  model->timePaused.addValueListener([this](auto timePaused) { view->timePausedCheckbox->setValue(timePaused); });

  view->restartButton->addClickListener([this] { model->restartRequested.notify(); });

  view->autoCompileCheckbox->setValue(*model->autoCompile);
  view->autoCompileCheckbox->addValueListener([this](auto autoCompile) { *model->autoCompile.modify() = autoCompile; });
  model->autoCompile.addValueListener([this](auto autoCompile) { view->autoCompileCheckbox->setValue(autoCompile); });

  view->autoCompilePeriodDrag->setValue(static_cast<float>(model->autoCompilePeriod->count()) / 1000.f);
  view->autoCompilePeriodDrag->addValueListener([this](auto periodSeconds) {
    const auto periodMs = std::chrono::milliseconds{static_cast<int>(periodSeconds * 1000.f)};
    *model->autoCompilePeriod.modify() = periodMs;
  });
  model->autoCompilePeriod.addValueListener(
      [this](auto period) { view->autoCompilePeriodDrag->setValue(static_cast<float>(period.count()) / 1000.f); });

  view->editor->setText(*model->code);
  view->editor->addTextListener([this](auto code) {
    ignoreNextEditorUpdate = true;
    *model->code.modify() = code;
  });
  model->code.addValueListener([this](auto code) {
    // FIXME: this is idiotic, but it avoids the editor resetting
    if (ignoreNextEditorUpdate) {
      ignoreNextEditorUpdate = false;
      return;
    }
    view->editor->setText(code);
  });

  // TODO react to model->compiling with some UI cue
}
void GlslEditorController::clearWarningMarkers() { view->editor->clearWarningMarkers(); }

void GlslEditorController::clearErrorMarkers() { view->editor->clearErrorMarkers(); }

void GlslEditorController::addWarningMarker(const ui::ig::TextEditorMarker &marker) {
  view->editor->addWarningMarker(marker);
}

void GlslEditorController::addErrorMarker(const ui::ig::TextEditorMarker &marker) {
  view->editor->addErrorMarker(marker);
}

}  // namespace pf