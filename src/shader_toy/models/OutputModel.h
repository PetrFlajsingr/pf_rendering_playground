//
// Created by Petr on 26/05/2022.
//

#pragma once

#include "gpu/Texture.h"
#include "mvc/Model.h"
#include "mvc/reactive.h"
#include <memory>

namespace pf {

class OutputModel : public SavableModel {
 public:
  OutputModel(std::pair<std::uint32_t, std::uint32_t> res, std::shared_ptr<Texture> tex);

  Observable<std::pair<std::uint32_t, std::uint32_t>> resolution;
  Observable<std::shared_ptr<Texture>> texture;

  [[nodiscard]] toml::table toToml() const override;
  void setFromToml(const toml::table &src) override;
};

}  // namespace pf
