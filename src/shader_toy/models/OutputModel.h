//
// Created by Petr on 26/05/2022.
//

#pragma once

#include "gpu/Texture.h"
#include "mvc/Model.h"
#include "mvc/reactive.h"
#include <memory>

namespace pf {

// TODO: move this
class NormalizedPosition {
 public:
  NormalizedPosition();
  NormalizedPosition(float xVal, float yVal);
  auto operator<=>(const NormalizedPosition &other) const noexcept = default;
  [[nodiscard]] float x() const;
  [[nodiscard]] float y() const;
  void x(float val);
  void y(float val);

 private:
  float x_;
  float y_;
};

class OutputModel : public SavableModel {
 public:
  OutputModel(std::pair<std::uint32_t, std::uint32_t> res, std::shared_ptr<gpu::Texture> tex);

  Observable<std::pair<std::uint32_t, std::uint32_t>> resolution;
  Observable<std::shared_ptr<gpu::Texture>> texture;
  Observable<NormalizedPosition> mousePositionOnImageUV{};
  Observable<bool> textureHovered{};

  [[nodiscard]] toml::table toToml() const override;
  void setFromToml(const toml::table &src) override;

  [[nodiscard]] std::string getDebugString() const override;
};

}  // namespace pf
