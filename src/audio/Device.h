//
// Created by Petr on 04/06/2022.
//

#pragma once

#include "common.h"
#include "tl/expected.hpp"
#include <AL/alc.h>
#include <NamedType/named_type.hpp>

namespace pf::audio {

class Context;
using DeviceId = fluent::NamedType<std::string, struct DeviceIdTag>;

class Device : public std::enable_shared_from_this<Device> {
 public:
  [[nodiscard]] static std::vector<DeviceId> ListDevices();

  [[nodiscard]] static tl::expected<std::shared_ptr<Device>, OpenALError> Create();
  [[nodiscard]] static tl::expected<std::shared_ptr<Device>, OpenALError> Create(DeviceId deviceId);

  ~Device();

  [[nodiscard]] ALCdevice *getHandle() const;

  [[nodiscard]] tl::expected<std::shared_ptr<Context>, OpenALError> createContext();

 private:
  [[nodiscard]] static tl::expected<std::shared_ptr<Device>, OpenALError> CreateImpl(const ALCchar *deviceId);
  explicit Device(ALCdevice *handle);
  ALCdevice *device = nullptr;
};

// TODO: capture device

}  // namespace pf::audio
