//
// Created by xflajs00 on 06.05.2022.
//

#pragma once

#include "GpuObject.h"
#include "Program.h"
#include "TextureTypes.h"
#include <glad/glad.h>

namespace pf {

class Texture : public GpuObject {
 public:
  PF_GPU_OBJECT_TYPE(GpuObject::Type::Texture)

  Texture(TextureTarget target, TextureFormat format, TextureLevel levels, TextureSize size);

  [[nodiscard]] virtual GpuOperationResult<TextureError> create() = 0;

  [[nodiscard]] std::string getDebugString() const override;

  [[nodiscard]] TextureTarget getTarget() const;
  [[nodiscard]] TextureFormat getFormat() const;
  [[nodiscard]] TextureLevel getTextureLevels() const;
  [[nodiscard]] TextureSize getSize() const;

  virtual void setParam(TextureMinificationFilter filter) = 0;
  virtual void setParam(TextureMagnificationFilter filter) = 0;
  virtual void bindTextureUnit(Binding unit) = 0;
  virtual void bindImage(Binding unit, ImageTextureUnitAccess access = ImageTextureUnitAccess::ReadWrite) = 0;

 protected:
  TextureTarget target;
  TextureFormat format;
  TextureLevel levels;
  TextureSize size;
};

}  // namespace pf
