//
// Created by Petr on 6/5/2022.
//

#include "Buffer.h"
#include "assert.hpp"

namespace pf::gpu {

GpuOperationResult<BufferError> OpenGlBuffer::create() {
  createHandle();
  return std::nullopt;
}

GpuOperationResult<BufferError> OpenGlBuffer::create(std::size_t size) {
  createHandle();
  // TODO: usage
  glNamedBufferData(*handle, static_cast<GLsizeiptr>(size), nullptr, GL_DYNAMIC_DRAW);
  return std::nullopt;
}

GpuOperationResult<BufferError> OpenGlBuffer::create(std::span<const std::byte> data) {
  createHandle();
  // TODO: usage
  glNamedBufferData(*handle, static_cast<GLsizeiptr>(data.size()), data.data(), GL_DYNAMIC_DRAW);
  return std::nullopt;
}

std::size_t OpenGlBuffer::getSize() const {
  DEBUG_ASSERT(handle.isValid(), "It's likely create hasn't been called");
  GLint64 result;
  glGetNamedBufferParameteri64v(*handle, GL_BUFFER_SIZE, &result);
  return static_cast<std::size_t>(result);
}

void OpenGlBuffer::deleteOpenGlObject(GLuint objectHandle) const { glDeleteBuffers(1, &objectHandle); }

BufferMapping OpenGlBuffer::map() {
  DEBUG_ASSERT(handle.isValid(), "It's likely create hasn't been called");
  auto ptr = glMapNamedBuffer(*handle, GL_READ_WRITE);
  return createMapping(ptr, [handle = *handle](Buffer &) { glUnmapNamedBuffer(handle); });
}

void OpenGlBuffer::createHandle() {
  GLuint bufferHandle;
  glGenBuffers(1, &bufferHandle);
  handle = bufferHandle;
}

GpuOperationResult<BufferError> OpenGlBuffer::setData(std::span<const std::byte> data) {
  DEBUG_ASSERT(handle.isValid(), "It's likely create hasn't been called");
  glNamedBufferSubData(*handle, 0, static_cast<GLsizeiptr>(data.size()), data.data());
  return std::nullopt;
}

GpuOperationResult<BufferError> OpenGlBuffer::setData(std::size_t offsetInBytes, std::span<const std::byte> data) {
  DEBUG_ASSERT(handle.isValid(), "It's likely create hasn't been called");
  glNamedBufferSubData(*handle, static_cast<GLintptr>(offsetInBytes), static_cast<GLsizeiptr>(data.size()),
                       data.data());
  return std::nullopt;
}

GpuOperationResult<BufferError> OpenGlBuffer::bind(BufferTarget target, Binding binding) {
  DEBUG_ASSERT(handle.isValid(), "It's likely create hasn't been called");
  glBindBufferBase(*handle, static_cast<GLuint>(binding.get()), BufferTargetToGlEnum(target));
  return std::nullopt;
}

GLenum OpenGlBuffer::BufferTargetToGlEnum(BufferTarget target) {
  switch (target) {
    case BufferTarget::AtomicCounter: return GL_ATOMIC_COUNTER_BUFFER;
    case BufferTarget::TransformFeedback: return GL_TRANSFORM_FEEDBACK_BUFFER;
    case BufferTarget::Uniform: return GL_UNIFORM_BUFFER;
    case BufferTarget::Storage: return GL_SHADER_STORAGE_BUFFER;
  }
  VERIFY(false, "Unhandled enum value");
  return {};
}

}  // namespace pf::gpu