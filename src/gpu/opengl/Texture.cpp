//
// Created by xflajs00 on 06.05.2022.
//

#include "Texture.h"

namespace pf {

OpenGlTexture::OpenGlTexture(TextureTarget target, TextureFormat format, TextureLevel levels, TextureSize size)
    : Texture(target, format, levels, size) {}

GpuOperationResult<TextureError> OpenGlTexture::create() {
  const auto rawFormat = FormatToOpenGLConstant(format);
  const auto rawTarget = TargetToOpenGLConstant(target);

  GLuint rawHandle;
  glCreateTextures(rawTarget, 1, &rawHandle);
  handle = rawHandle;

  if (levels > TextureLevel{0}) {
    if (size.height == TextureHeight{0}) {
      glTextureStorage1D(*handle, levels.get(), rawFormat, size.width.get());
    } else if (size.depth == TextureDepth{0}) {
      glTextureStorage2D(*handle, levels.get(), rawFormat, size.width.get(), size.height.get());
    } else {
      glTextureStorage3D(*handle, levels.get(), rawFormat, size.width.get(), size.height.get(), size.depth.get());
    }
  } else {
    // TODO: pixel data format
    if (target == TextureTarget::CubeMap) {
      for (auto i = 0u; i < 6u; ++i) {
        if (size.height == TextureHeight{0}) {
          glTextureImage1DEXT(*handle, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, rawFormat, size.width.get(), 0, GL_RGBA,
                              GL_UNSIGNED_BYTE, nullptr);
        } else if (size.depth == TextureDepth{0}) {
          glTextureImage2DEXT(*handle, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, rawFormat, size.width.get(),
                              size.height.get(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        } else {
          glTextureImage3DEXT(*handle, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, rawFormat, size.width.get(),
                              size.height.get(), size.depth.get(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        }
      }
    } else {
      // TODO: this might fail
      if (size.height == TextureHeight{0}) {
        glTextureImage1DEXT(*handle, rawTarget, 0, rawFormat, size.width.get(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
      } else if (size.depth == TextureDepth{0}) {
        glTextureImage2DEXT(*handle, rawTarget, 0, rawFormat, size.width.get(), size.height.get(), 0, GL_RGBA,
                            GL_UNSIGNED_BYTE, nullptr);
      } else {
        glTextureImage3DEXT(*handle, rawTarget, 0, rawFormat, size.width.get(), size.height.get(), size.depth.get(), 0,
                            GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
      }
    }
  }

  return std::nullopt;
}

void OpenGlTexture::setParam(TextureMinificationFilter filter) {
  glTextureParameteri(*handle, GL_TEXTURE_MIN_FILTER, MinifyingFilterToGlConstant(filter));
}

void OpenGlTexture::setParam(TextureMagnificationFilter filter) {
  glTextureParameteri(*handle, GL_TEXTURE_MAG_FILTER, MagnifyingFilterToGlConstant(filter));
}

constexpr GLint OpenGlTexture::FormatToOpenGLConstant(TextureFormat format) {
  using enum TextureFormat;
  switch (format) {
    case R8: return GL_R8;
    case R8_SNORM: return GL_R8_SNORM;
    case R16F: return GL_R16F;
    case R32F: return GL_R32F;
    case R8UI: return GL_R8UI;
    case R8I: return GL_R8I;
    case R16UI: return GL_R16UI;
    case R16I: return GL_R16I;
    case R32UI: return GL_R32UI;
    case R32I: return GL_R32I;
    case RG8: return GL_RG8;
    case RG8_SNORM: return GL_RG8_SNORM;
    case RG16F: return GL_RG16F;
    case RG32F: return GL_RG32F;
    case RG8UI: return GL_RG8UI;
    case RG8I: return GL_RG8I;
    case RG16UI: return GL_RG16UI;
    case RG16I: return GL_RG16I;
    case RG32UI: return GL_RG32UI;
    case RG32I: return GL_RG32I;
    case RGB8: return GL_RGB8;
    case SRGB8: return GL_SRGB8;
    case RGB565: return GL_RGB565;
    case RGB8_SNORM: return GL_RGB8_SNORM;
    case R11F_G11F_B10F: return GL_R11F_G11F_B10F;
    case RGB9_E5: return GL_RGB9_E5;
    case RGB16F: return GL_RGB16F;
    case RGB32F: return GL_RGB32F;
    case RGB8UI: return GL_RGB8UI;
    case RGB8I: return GL_RGB8I;
    case RGB16UI: return GL_RGB16UI;
    case RGB16I: return GL_RGB16I;
    case RGB32UI: return GL_RGB32UI;
    case RGB32I: return GL_RGB32I;
    case RGBA8: return GL_RGBA8;
    case SRGB8_ALPHA8: return GL_SRGB8_ALPHA8;
    case RGBA8_SNORM: return GL_RGBA8_SNORM;
    case RGB5_A1: return GL_RGB5_A1;
    case RGBA4: return GL_RGBA4;
    case RGB10_A2: return GL_RGB10_A2;
    case RGBA16F: return GL_RGBA16F;
    case RGBA32F: return GL_RGBA32F;
    case RGBA8UI: return GL_RGBA8UI;
    case RGBA8I: return GL_RGBA8I;
    case RGB10_A2UI: return GL_RGB10_A2UI;
    case RGBA16UI: return GL_RGBA16UI;
    case RGBA16I: return GL_RGBA16I;
    case RGBA32I: return GL_RGBA32I;
    case RGBA32UI: return GL_RGBA32UI;
    case DepthComponent16: return GL_DEPTH_COMPONENT16;
    case DepthComponent24: return GL_DEPTH_COMPONENT24;
    case DepthComponent32F: return GL_DEPTH_COMPONENT32F;
    case Depth24_Stencil8: return GL_DEPTH24_STENCIL8;
    case Depth32F_Stencil8: return GL_DEPTH32F_STENCIL8;
  }
  assert(false && "this can't happen");
  return {};
}

constexpr GLenum OpenGlTexture::TargetToOpenGLConstant(TextureTarget target) {
  using enum TextureTarget;
  switch (target) {
    case _1D: return GL_TEXTURE_1D;
    case _2D: return GL_TEXTURE_2D;
    case _3D: return GL_TEXTURE_3D;
    case Rectangle: return GL_TEXTURE_RECTANGLE;
    case CubeMap: return GL_TEXTURE_CUBE_MAP;
    case Buffer: return GL_TEXTURE_BUFFER;
    case _2DMultisample: return GL_TEXTURE_2D_MULTISAMPLE;
  }
  assert(false && "this can't happen");
  return {};
}

constexpr GLenum OpenGlTexture::MinifyingFilterToGlConstant(TextureMinificationFilter filter) {
  using enum TextureMinificationFilter;
  switch (filter) {
    case Nearest: return GL_NEAREST;
    case Linear: return GL_LINEAR;
    case NearestMipmapNearest: return GL_NEAREST_MIPMAP_NEAREST;
    case NearestMipmapLinear: return GL_NEAREST_MIPMAP_LINEAR;
    case LinearMipmapLinear: return GL_LINEAR_MIPMAP_LINEAR;
  }
  assert(false && "this can't happen");
  return {};
}

constexpr GLenum OpenGlTexture::MagnifyingFilterToGlConstant(TextureMagnificationFilter filter) {
  using enum TextureMagnificationFilter;
  switch (filter) {
    case Nearest: return GL_NEAREST;
    case Linear: return GL_LINEAR;
  }
  assert(false && "this can't happen");
  return {};
}

constexpr GLenum OpenGlTexture::ImageTextureUnitAccessToGlConstant(ImageTextureUnitAccess access) {
  using enum ImageTextureUnitAccess;
  switch (access) {
    case ReadOnly: return GL_READ_ONLY;
    case ReadWrite: return GL_READ_WRITE;
    case WriteOnly: return GL_WRITE_ONLY;
  }
  assert(false && "this can't happen");
  return {};
}

void OpenGlTexture::bindTextureUnit(Binding unit) { glBindTextureUnit(unit.get(), *handle); }

void OpenGlTexture::bindImage(Binding unit, ImageTextureUnitAccess access) {
  glBindImageTexture(unit.get(), *handle, levels.get(), GL_FALSE, 0, ImageTextureUnitAccessToGlConstant(access),
                     FormatToOpenGLConstant(format));
}

void OpenGlTexture::deleteOpenGlObject(GLuint objectHandle) const { glDeleteTextures(1, &objectHandle); }

}  // namespace pf