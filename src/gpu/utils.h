//
// Created by xflajs00 on 07.05.2022.
//

#pragma once

#include "Texture.h"
#include "gpu/opengl/Texture.h"
#include <assert.hpp>
#include <filesystem>
#include <imgui.h>
#include <stb/stb_image.h>

namespace pf {

[[nodiscard]] inline ImTextureID getImTextureID(Texture &texture) {
  if (const auto oGlTexture = texture.as<OpenGlTexture>(); oGlTexture.has_value()) {
    return reinterpret_cast<ImTextureID>(static_cast<std::intptr_t>(oGlTexture.value()->getHandle()));
  } else {
    VERIFY(false, "Missing implementation for non OpenGL texture");
    return {};
  }
}

[[nodiscard]] inline std::optional<TextureSize> getTextureFileSize(const std::filesystem::path &path) {
  int x;
  int y;
  int n;
  if (stbi_info(path.string().c_str(), &x, &y, &n)) {
    return TextureSize{TextureWidth{static_cast<std::uint32_t>(x)}, TextureHeight{static_cast<std::uint32_t>(y)},
                       TextureDepth{1}};
  }
  return std::nullopt;
}

[[nodiscard]] inline tl::expected<std::vector<std::byte>, std::string>
getTextureData(const std::filesystem::path &path) {
  int x;
  int y;
  int n;
  const auto data = stbi_load(path.string().c_str(), &x, &y, &n, 4);
  if (data == nullptr) { return tl::make_unexpected("Image loading failed"); }
  const auto dataSpan = std::span{reinterpret_cast<const std::byte *>(data), static_cast<std::size_t>(x * y * 4)};
  auto stbFree = RAII{[&] { stbi_image_free(data); }};
  return std::vector<std::byte>{dataSpan.begin(), dataSpan.end()};
}

}  // namespace pf