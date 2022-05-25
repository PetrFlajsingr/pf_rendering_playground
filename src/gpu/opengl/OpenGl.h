//
// Created by xflajs00 on 06.05.2022.
//

#pragma once

#include <cassert>
#include <functional>
#include <glad/glad.h>
#include <optional>

namespace pf {

template<std::invocable<GLuint> Deleter>
class OpenGlHandle {
  constexpr static GLuint INVALID_HANDLE = static_cast<GLuint>(-1);

 public:
  OpenGlHandle()
    requires(std::is_default_constructible_v<Deleter>)
  : handle(INVALID_HANDLE) {}
  explicit OpenGlHandle(GLuint handle)
    requires(std::is_default_constructible_v<Deleter>)
  : handle(handle) {}

  OpenGlHandle(GLuint handle, Deleter &&deleter) : handle(handle), deleter(std::forward<Deleter>(deleter)) {}
  explicit OpenGlHandle(Deleter &&deleter) : handle(INVALID_HANDLE), deleter(std::forward<Deleter>(deleter)) {}

  OpenGlHandle(const OpenGlHandle &) = delete;
  OpenGlHandle &operator=(const OpenGlHandle &) = delete;

  OpenGlHandle &operator=(GLuint newHandle) {
    deleteHandle();
    handle = newHandle;
    return *this;
  }

  OpenGlHandle(OpenGlHandle &&other) noexcept : handle(other.handle) { other.handle = INVALID_HANDLE; }
  OpenGlHandle &operator=(OpenGlHandle &&other) noexcept {
    handle = other.handle;
    other.handle = INVALID_HANDLE;
    return *this;
  }

  ~OpenGlHandle() { deleteHandle(); }

  [[nodiscard]] bool isValid() const { return handle != INVALID_HANDLE; }
  void reset() {
    deleteHandle();
    handle = INVALID_HANDLE;
  }
  [[nodiscard]] GLuint release() {
    const auto result = handle;
    handle = INVALID_HANDLE;
    return result;
  }

  [[nodiscard]] std::optional<GLuint> getHandle() const {
    return isValid() ? std::optional<GLuint>{handle} : std::optional<GLuint>{};
  }

  [[nodiscard]] GLuint operator*() const { return handle; }

 private:
  void deleteHandle() {
    if (isValid()) { deleter(handle); }
  }

  GLuint handle;
  Deleter deleter;
};

class OpenGlHandleOwner {
 private:
  struct Deleter {
    void operator()(GLuint objectHandle) const { owner->deleteOpenGlObject(objectHandle); }
    const OpenGlHandleOwner *owner;
  };

 public:
  [[nodiscard]] GLuint getHandle() const { return *handle; }
  [[nodiscard]] bool isValid() const { return handle.isValid(); }

 protected:
  OpenGlHandleOwner() : handle(Deleter{this}) {}

  virtual void deleteOpenGlObject(GLuint objectHandle) const = 0;

  OpenGlHandle<Deleter> handle;
};

}  // namespace pf
