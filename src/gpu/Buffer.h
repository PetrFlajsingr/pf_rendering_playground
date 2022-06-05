//
// Created by Petr on 6/5/2022.
//

#pragma once

#include "GpuObject.h"
#include "common.h"

namespace pf::gpu {

enum class BufferError {

};

//GL_ATOMIC_COUNTER_BUFFER, GL_TRANSFORM_FEEDBACK_BUFFER, GL_UNIFORM_BUFFER or GL_SHADER_STORAGE_BUFFER
enum class BufferTarget { AtomicCounter, TransformFeedback, Uniform, Storage };

class BufferMapping {
  friend class Buffer;

 public:
  template<typename T>
  [[nodiscard]] std::span<T> data();

  ~BufferMapping();

 private:
  BufferMapping(Buffer &parent, void *mappedDataPtr, std::invocable<Buffer &> auto &&unmapFnc);

  void *dataPtr;
  std::function<void(Buffer &)> unmap;
  Buffer &owner;
};

class Buffer : public GpuObject {
 public:
  PF_GPU_OBJECT_TYPE(GpuObject::Type::Buffer)

  [[nodiscard]] virtual GpuOperationResult<BufferError> create() = 0;
  [[nodiscard]] virtual GpuOperationResult<BufferError> create(std::size_t size) = 0;
  template<typename T>
  [[nodiscard]] GpuOperationResult<BufferError> create(std::span<const T> data) {
    return create(std::span{reinterpret_cast<std::byte *>(data.data()), data.size() * sizeof(T)});
  }
  [[nodiscard]] virtual GpuOperationResult<BufferError> create(std::span<const std::byte> data) = 0;
  [[nodiscard]] virtual std::size_t getSize() const = 0;

  template<typename T>
  [[nodiscard]] GpuOperationResult<BufferError> setData(std::span<const T> data) {
    return setData(std::span{reinterpret_cast<std::byte *>(data.data()), data.size() * sizeof(T)});
  }
  [[nodiscard]] virtual GpuOperationResult<BufferError> setData(std::span<const std::byte> data) = 0;

  template<typename T>
  [[nodiscard]] GpuOperationResult<BufferError> setData(std::size_t offsetInBytes, std::span<const T> data) {
    return setData(offsetInBytes, std::span{reinterpret_cast<std::byte *>(data.data()), data.size() * sizeof(T)});
  }
  [[nodiscard]] virtual GpuOperationResult<BufferError> setData(std::size_t offsetInBytes,
                                                                std::span<const std::byte> data) = 0;

  [[nodiscard]] virtual BufferMapping map() = 0;

  [[nodiscard]] virtual GpuOperationResult<BufferError> bind(BufferTarget target, Binding binding) = 0;

 protected:
  [[nodiscard]] BufferMapping createMapping(void *data, std::invocable<Buffer &> auto &&unmapFnc) {
    return BufferMapping(*this, data, std::forward<decltype(unmapFnc)>(unmapFnc));
  }
};

BufferMapping::BufferMapping(Buffer &parent, void *mappedDataPtr, std::invocable<Buffer &> auto &&unmapFnc)
    : dataPtr(mappedDataPtr), unmap(std::forward<decltype(unmapFnc)>(unmapFnc)), owner(parent) {}

template<typename T>
std::span<T> BufferMapping::data() {
  return std::span{reinterpret_cast<T *>(data), owner.getSize() / sizeof(T)};
}

}  // namespace pf::gpu
