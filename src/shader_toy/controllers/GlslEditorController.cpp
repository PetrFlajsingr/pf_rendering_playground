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
  view->editor->addTextListener([this](auto code) { *model->code.modify() = code; });
  model->code.addValueListener([this](auto code) { view->editor->setText(*model->code); });
}

}  // namespace pf