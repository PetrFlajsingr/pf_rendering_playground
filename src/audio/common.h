//
// Created by Petr on 04/06/2022.
//

#pragma once
#include <AL/al.h>
#include <assert.hpp>
#include <optional>
#include <string>

namespace pf::audio {

enum class ErrorType {
  Unknown = 0,
  InvalidName = AL_INVALID_NAME,
  InvalidEnum = AL_INVALID_ENUM,
  InvalidValue = AL_INVALID_VALUE,
  InvalidOperation = AL_INVALID_OPERATION,
  OutOfMemory = AL_OUT_OF_MEMORY
};

struct OpenALError {
  ErrorType type;
  std::string message;
};

namespace details {
[[nodiscard]] inline std::optional<ErrorType> checkOpenAlError() {
  using enum ErrorType;
  const auto error = alGetError();
  switch (error) {
    case AL_NO_ERROR: return std::nullopt;
    case AL_INVALID_NAME: return InvalidName;
    case AL_INVALID_ENUM: return InvalidEnum;
    case AL_INVALID_VALUE: return InvalidValue;
    case AL_INVALID_OPERATION: return InvalidOperation;
    case AL_OUT_OF_MEMORY: return OutOfMemory;
    default: VERIFY(false); return {};
  }
}
}  // namespace details

}  // namespace pf::audio