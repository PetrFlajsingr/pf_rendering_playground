//
// Created by xflajs00 on 07.05.2022.
//

#pragma once

#include "Texture.h"
#include "gpu/opengl/Texture.h"
#include <imgui.h>

namespace pf {

[[nodiscard]] inline ImTextureID getImTextureID(Texture &texture) {
  if (const auto oGlTexture = texture.as<OpenGlTexture>(); oGlTexture.has_value()) {
    return reinterpret_cast<ImTextureID>(static_cast<std::intptr_t>(oGlTexture.value()->getHandle()));
  } else {
    assert(false && "Missing implementation");
    return {};
  }
}

}  // namespace pf