//
// Created by xflajs00 on 06.05.2022.
//

#pragma once

#include "../Texture.h"
#include "OpenGl.h"

namespace pf {

// TODO: format support check?
class OpenGlTexture final : public Texture, public OpenGlHandleOwner {
 public:
  PF_GPU_OBJECT_API(GpuApi::OpenGl)

  OpenGlTexture(TextureTarget target, TextureFormat format, TextureLevel levels, TextureSize size);

  GpuOperationResult<TextureError> create() override;


  void setParam(TextureMinificationFilter filter) override;
  void setParam(TextureMagnificationFilter filter) override;

  [[nodiscard]] constexpr static GLint FormatToOpenGLConstant(TextureFormat format);
  [[nodiscard]] constexpr static GLenum TargetToOpenGLConstant(TextureTarget target);
  [[nodiscard]] constexpr static GLenum MinifyingFilterToGlConstant(TextureMinificationFilter filter);
  [[nodiscard]] constexpr static GLenum MagnifyingFilterToGlConstant(TextureMagnificationFilter filter);
  [[nodiscard]] constexpr static GLenum ImageTextureUnitAccessToGlConstant(ImageTextureUnitAccess access);

  void bindTextureUnit(Binding unit) override;
  void bindImage(Binding unit, ImageTextureUnitAccess access = ImageTextureUnitAccess::ReadWrite) override;

 protected:
  void deleteOpenGlObject(GLuint objectHandle) const override;
};

}  // namespace pf
