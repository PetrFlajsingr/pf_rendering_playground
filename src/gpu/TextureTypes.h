//
// Created by xflajs00 on 06.05.2022.
//

#pragma once

#include <NamedType/named_type.hpp>

namespace pf {

using TextureLevel = fluent::NamedType<std::uint32_t, struct TextureLevelTag, fluent::Comparable>;
using TextureWidth = fluent::NamedType<std::uint32_t, struct TextureWidthTag, fluent::Comparable, fluent::Addable,
                                       fluent::Subtractable, fluent::Multiplicable>;
using TextureHeight = fluent::NamedType<std::uint32_t, struct TextureHeightTag, fluent::Comparable, fluent::Addable,
                                        fluent::Subtractable, fluent::Multiplicable>;
using TextureDepth = fluent::NamedType<std::uint32_t, struct TextureDepthTag, fluent::Comparable, fluent::Addable,
                                       fluent::Subtractable, fluent::Multiplicable>;

using TextureOffset = fluent::NamedType<std::uint32_t, struct TextureOffsetTag>;

enum class ImageTextureUnitAccess { ReadOnly, ReadWrite, WriteOnly };

struct TextureSize {
  TextureSize() = default;
  inline TextureSize(const TextureWidth &width, const TextureHeight &height, const TextureDepth &depth)
      : width(width), height(height), depth(depth) {}
  TextureWidth width{};
  TextureHeight height{};
  TextureDepth depth{};
};

// TODO: texture arrays
// maybe use static factory methods for this
enum class TextureTarget { _1D, _2D, _3D, Rectangle, CubeMap, Buffer, _2DMultisample };

// TODO: more tex parameters
enum class TextureMinificationFilter { Nearest, Linear, NearestMipmapNearest, NearestMipmapLinear, LinearMipmapLinear };
enum class TextureMagnificationFilter { Nearest, Linear };

enum class TextureFormat {
  R8,
  R8_SNORM,
  R16F,
  R32F,
  R8UI,
  R8I,
  R16UI,
  R16I,
  R32UI,
  R32I,
  RG8,
  RG8_SNORM,
  RG16F,
  RG32F,
  RG8UI,
  RG8I,
  RG16UI,
  RG16I,
  RG32UI,
  RG32I,
  RGB8,
  SRGB8,
  RGB565,
  RGB8_SNORM,
  R11F_G11F_B10F,
  RGB9_E5,
  RGB16F,
  RGB32F,
  RGB8UI,
  RGB8I,
  RGB16UI,
  RGB16I,
  RGB32UI,
  RGB32I,
  RGBA8,
  SRGB8_ALPHA8,
  RGBA8_SNORM,
  RGB5_A1,
  RGBA4,
  RGB10_A2,
  RGBA16F,
  RGBA32F,
  RGBA8UI,
  RGBA8I,
  RGB10_A2UI,
  RGBA16UI,
  RGBA16I,
  RGBA32I,
  RGBA32UI,
  DepthComponent16,
  DepthComponent24,
  DepthComponent32F,
  Depth24_Stencil8,
  Depth32F_Stencil8
};

}  // namespace pf
