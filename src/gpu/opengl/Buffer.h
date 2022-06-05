//
// Created by Petr on 6/5/2022.
//

#pragma once

#include "../Buffer.h"
#include "OpenGl.h"

namespace pf::gpu {

class OpenGlBuffer final : public Buffer, public OpenGlHandleOwner {
 public:
  PF_GPU_OBJECT_API(GpuApi::OpenGl)

  GpuOperationResult<BufferError> create() override;
  GpuOperationResult<BufferError> create(std::size_t size) override;
  GpuOperationResult<BufferError> create(std::span<const std::byte> data) override;

  [[nodiscard]] std::size_t getSize() const override;
  BufferMapping map() override;
  GpuOperationResult<BufferError> setData(std::span<const std::byte> data) override;
  GpuOperationResult<BufferError> setData(std::size_t offsetInBytes, std::span<const std::byte> data) override;

  GpuOperationResult<BufferError> bind(BufferTarget target, Binding binding) override;

 protected:
  void deleteOpenGlObject(GLuint objectHandle) const override;

 private:
  void createHandle();
};

}  // namespace pf::gpu
