//
// Created by xflajs00 on 06.05.2022.
//

#include "Texture.h"

namespace pf {

Texture::Texture(TextureTarget target, TextureFormat format, TextureLevel levels, TextureSize size)
    : target(target), format(format), levels(levels), size(size) {}

std::string Texture::getDebugString() const {
  return fmt::format("{}\tTarget: {}\tFormat: {}\tTexture levels: {}\t Size: {}x{}x{}", GpuObject::getDebugString(),
                     magic_enum::enum_name(target), magic_enum::enum_name(format), levels.get(), size.width.get(),
                     size.height.get(), size.depth.get());
}

TextureTarget Texture::getTarget() const { return target; }

TextureFormat Texture::getFormat() const { return format; }

TextureLevel Texture::getTextureLevels() const { return levels; }

TextureSize Texture::getSize() const { return size; }

}  // namespace pf