//
// Created by Petr on 26/05/2022.
//

#include "OutputController.h"
#include "gpu/utils.h"
#include <assert.hpp>

namespace pf {

OutputController::OutputController(std::unique_ptr<OutputView> uiView, std::shared_ptr<OutputModel> mod)
    : Controller<OutputView, OutputModel>(std::move(uiView), std::move(mod)) {
  DEBUG_ASSERT(isIn(model->resolution->first, IMAGE_SIZES));
  DEBUG_ASSERT(isIn(model->resolution->second, IMAGE_SIZES));

  view->widthCombobox->setItems(IMAGE_SIZES);
  view->heightCombobox->setItems(IMAGE_SIZES);

  view->widthCombobox->setValue(static_cast<int>(model->resolution->first));
  view->heightCombobox->setValue(static_cast<int>(model->resolution->second));

  model->resolution.addValueListener([this](auto newResolution) {
    DEBUG_ASSERT(isIn(newResolution.first, IMAGE_SIZES));
    DEBUG_ASSERT(isIn(newResolution.second, IMAGE_SIZES));
    view->widthCombobox->setValue(static_cast<int>(newResolution.first));
    view->heightCombobox->setValue(static_cast<int>(newResolution.second));
  });
  view->widthCombobox->addValueListener([this](auto newWidth) { model->resolution.modify()->first = newWidth; });
  view->heightCombobox->addValueListener([this](auto newHeight) { model->resolution.modify()->second = newHeight; });

  if (*model->texture != nullptr) { view->image->setTextureId(getImTextureID(**model->texture)); }
  model->texture.addValueListener([this](auto newTexture) {
    if (newTexture != nullptr) {
      view->image->setTextureId(getImTextureID(*newTexture));
    } else {
      view->image->setTextureId(static_cast<ImTextureID>(0));
    }
  });

  view->image->addMousePositionListener([this](auto pos) {
    if (*model->texture == nullptr) { return; }

    auto result = NormalizedPosition{pos.x / static_cast<float>(view->image->getSize().width),
                                     pos.y / static_cast<float>(view->image->getSize().height)};
    *model->mousePositionOnImageUV.modify() = result;
  });
  view->image->addHoverListener([this](auto hovered) { *model->textureHovered.modify() = hovered; });
}

void OutputController::setFps(float currentFps, float averageFps) {
  DEBUG_ASSERT(currentFps >= 0.f);
  DEBUG_ASSERT(averageFps >= 0.f);
  view->fpsText->setText("FPS: {}", currentFps);
  view->fpsAveragePlot->addValue(averageFps);
}

}  // namespace pf