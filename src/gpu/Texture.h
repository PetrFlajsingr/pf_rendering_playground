//
// Created by xflajs00 on 06.05.2022.
//

#pragma once

#include "GpuObject.h"
#include "Program.h"
#include "TextureTypes.h"
#include <glad/glad.h>

namespace pf {

enum class TextureError { InvalidCoordinates, WrongDataLength };

// TODO: divide this up to different texture targets
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

  // TODO: type is std::byte only for now, only R8G8B8A8
  template<std::same_as<const std::byte> T>
  GpuOperationResult<TextureError>
  set2Ddata(std::span<T> data, TextureLevel level, TextureOffset xOffset = TextureOffset{0},
            TextureOffset yOffset = TextureOffset{0}, std::optional<TextureWidth> width = std::nullopt,
            std::optional<TextureHeight> height = std::nullopt) {
    const auto targetWidth = width.value_or(size.width);
    const auto targetHeight = height.value_or(size.height);
    if (xOffset.get() >= size.width.get() || xOffset.get() + targetWidth.get() > size.width.get()) {
      return GpuError{TextureError::InvalidCoordinates, "X coords out of bounds"};
    }
    if (yOffset.get() >= size.height.get() || yOffset.get() + targetHeight.get() > size.height.get()) {
      return GpuError{TextureError::InvalidCoordinates, "Y coords out of bounds"};
    }

    const auto expectedDataLength = targetWidth.get() * targetHeight.get() * 4;  // R8G8B8A8
    if (data.size() != expectedDataLength) {
      return GpuError{
          TextureError::WrongDataLength,
          fmt::format("Input data have wrong length, is {}, should be {}", expectedDataLength, data.size())};
    }

    return set2DdataImpl({reinterpret_cast<const std::byte *>(data.data()), data.size() * sizeof(T)}, level, xOffset, yOffset,
                         targetWidth, targetHeight);
  }

  virtual void setParam(TextureMinificationFilter filter) = 0;
  virtual void setParam(TextureMagnificationFilter filter) = 0;
  virtual void bindTextureUnit(Binding unit) = 0;
  virtual void bindImage(Binding unit, ImageTextureUnitAccess access = ImageTextureUnitAccess::ReadWrite) = 0;

 protected:
  virtual GpuOperationResult<TextureError> set2DdataImpl(std::span<const std::byte> data, TextureLevel level,
                                                         TextureOffset xOffset, TextureOffset yOffset,
                                                         TextureWidth width, TextureHeight height) = 0;

  TextureTarget target;
  TextureFormat format;
  TextureLevel levels;
  TextureSize size;
};

}  // namespace pf
