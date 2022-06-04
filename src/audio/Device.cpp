//
// Created by Petr on 04/06/2022.
//

#include "Device.h"
#include "Context.h"

namespace pf::audio {

std::vector<DeviceId> Device::ListDevices() {
  ALboolean enumeration;
  enumeration = alcIsExtensionPresent(nullptr, "ALC_ENUMERATION_EXT");
  if (enumeration == AL_FALSE) { return {}; }
  auto devices = alcGetString(nullptr, ALC_DEVICE_SPECIFIER);

  const ALCchar *device = devices;
  const ALCchar *next = devices + 1;

  auto result = std::vector<DeviceId>{};
  while (device && *device != '\0' && next && *next != '\0') {
    result.emplace_back(DeviceId{device});
    const auto len = strlen(device);
    device += (len + 1);
    next += (len + 2);
  }
  return result;
}

tl::expected<std::shared_ptr<Device>, OpenALError> Device::Create() { return CreateImpl(nullptr); }

tl::expected<std::shared_ptr<Device>, OpenALError> Device::Create(DeviceId deviceId) {
  return CreateImpl(deviceId.get().c_str());
}

tl::expected<std::shared_ptr<Device>, OpenALError> Device::CreateImpl(const ALCchar *deviceId) {
  auto handle = alcOpenDevice(deviceId);
  if (!handle) {
    return tl::make_unexpected(
        OpenALError{details::checkOpenAlError().value_or(ErrorType::Unknown), "Failure while creating device"});
  }
  return std::shared_ptr<Device>(new Device{handle});
}

Device::~Device() { alcCloseDevice(device); }

Device::Device(ALCdevice *handle) : device(handle) {}

tl::expected<std::shared_ptr<Context>, OpenALError> Device::createContext() {
  const auto handle = alcCreateContext(device, nullptr);
  return std::shared_ptr<Context>(new Context{handle, shared_from_this()});
}

}  // namespace pf