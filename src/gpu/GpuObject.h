//
// Created by xflajs00 on 06.05.2022.
//

#pragma once

#include "magic_enum.hpp"
#include <concepts>
#include <fmt/format.h>
#include <optional>
#include <pf_common/enums.h>
#include <string_view>
#include <tl/expected.hpp>
#include <type_traits>
#include <utility>

namespace pf::gpu {

enum class GpuApi {
  OpenGl  // TODO more
};

template<Enum E>
struct GpuError {
 public:
  GpuError(const E code, std::string message) : code_(code), message_(std::move(message)) {}
  [[nodiscard]] E code() const { return code_; }
  [[nodiscard]] std::string_view message() const { return message_; }

 private:
  E code_;
  std::string message_;
};

template<Enum E>
using GpuOperationResult = std::optional<GpuError<E>>;
template<typename Expected, Enum E>
using ExpectedGpuOperationResult = tl::expected<Expected, GpuError<E>>;

#define PF_GPU_OBJECT_TYPE(x)                                                                                          \
  static_assert(std::same_as<decltype(x), GpuObject::Type>);                                                           \
  [[nodiscard]] inline Type getObjectType() const final { return GetClassType(); }                                     \
  [[nodiscard]] inline static GpuObject::Type GetClassType() { return x; }

#define PF_GPU_OBJECT_API(x)                                                                                           \
  static_assert(std::same_as<decltype(x), GpuApi>);                                                                    \
  [[nodiscard]] inline GpuApi getObjectApi() const final { return GetClassApi(); }                                     \
  [[nodiscard]] inline static GpuApi GetClassApi() { return x; }

// TODO: i guess just create a custom RTTI for this thing
/**
 * @warning Implement `GPUObject::Type T::GetClassType()` otherwise some functions won't work
 * @warning Implement `GpuApi T::GetClassApi()` otherwise some functions won't work
 */
class GpuObject {
 public:
  enum class Type {
    Texture,
    Shader,
    Program,
    Buffer  // TODO more
  };
  virtual ~GpuObject() = 0;

  /**
   * @attention implement static GetClassType() as well
   * @return type of gpu object this class represents
   */
  [[nodiscard]] virtual Type getObjectType() const = 0;
  /**
   * @attention implement static GetClassApi() as well
   * @return gpu api this class implements
   */
  [[nodiscard]] virtual GpuApi getObjectApi() const = 0;

  [[nodiscard]] virtual std::string getDebugString() const {
    return fmt::format("Gpu object - type: '{}'\tApi: '{}'", magic_enum::enum_name(getObjectType()),
                       magic_enum::enum_name(getObjectApi()));
  }

  template<std::derived_from<GpuObject> T>
  [[nodiscard]] std::optional<T *> as() {
    if (is<T>()) { return static_cast<T *>(this); }
    return std::nullopt;
  }
  template<std::derived_from<GpuObject> T>
  [[nodiscard]] std::optional<const T *> as() const {
    if (is<T>()) { return static_cast<T *>(this); }
    return std::nullopt;
  }
  template<std::derived_from<GpuObject> T>
  [[nodiscard]] bool is() const {
    const auto isSameObjectType = getObjectType() == T::GetClassType();
    if (IsTypeInterfaceClass<T>()) { return isSameObjectType; }
    return isSameObjectType && (getObjectApi() == T::GetClassApi());
  }

  template<std::derived_from<GpuObject> T>
  [[nodiscard]] static bool IsTypeInterfaceClass() {
    const auto hasOverridenGetObjectType = (&T::getObjectType != &GpuObject::getObjectType);
    const auto hasOverridenGetObjectApi = (&T::getObjectApi != &GpuObject::getObjectApi);
    return hasOverridenGetObjectType || !hasOverridenGetObjectApi;
  }
};

inline GpuObject::~GpuObject() = default;

}  // namespace pf::gpu

template<std::derived_from<pf::gpu::GpuObject> TGPUObject>
struct fmt::formatter<TGPUObject> : fmt::formatter<std::string> {
  auto format(const TGPUObject &object, fmt::format_context &ctx) {
    return fmt::format_to(ctx.out(), "{}", object.getDebugString());
  }
};
