//
// Created by xflajs00 on 07.05.2022.
//

#pragma once

#include "Texture.h"
#include "gpu/opengl/Texture.h"
#include <imgui.h>
#include <stb/stb_image.h>

namespace pf {

[[nodiscard]] inline ImTextureID getImTextureID(Texture &texture) {
  if (const auto oGlTexture = texture.as<OpenGlTexture>(); oGlTexture.has_value()) {
    return reinterpret_cast<ImTextureID>(static_cast<std::intptr_t>(oGlTexture.value()->getHandle()));
  } else {
    assert(false && "Missing implementation");
    return {};
  }
}

inline std::optional<TextureSize> getTextureFileSize(const std::filesystem::path &path) {
  int x;
  int y;
  int n;
  if (stbi_info(path.string().c_str(), &x, &y, &n)) {
    return TextureSize{TextureWidth{static_cast<std::uint32_t>(x)}, TextureHeight{static_cast<std::uint32_t>(y)},
                       TextureDepth{1}};
  }
  return std::nullopt;
}

inline std::optional<std::string> setTextureFromFile(Texture &texture, const std::filesystem::path &path) {
  int x;
  int y;
  int n;
  const auto data = stbi_load(path.string().c_str(), &x, &y, &n, 4);
  if (data == nullptr) { return "Image loading failed"; }
  auto stbFree = RAII{[&] { stbi_image_free(data); }};
  texture.set2Ddata(std::span{reinterpret_cast<const std::byte *>(data), static_cast<std::size_t>(x * y * n)},
                    TextureLevel{0});
}

}  // namespace pf