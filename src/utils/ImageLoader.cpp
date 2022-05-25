//
// Created by Petr on 23/05/2022.
//

#include "ImageLoader.h"
#include "gpu/opengl/Texture.h"
#include "pf_common/RAII.h"
#include "pf_common/array.h"
#include "stb_image.h"
#include <assert.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/cache1.hpp>
#include <range/v3/view/chunk.hpp>
#include <range/v3/view/join.hpp>
#include <range/v3/view/transform.hpp>

namespace pf {
ImageLoader::ImageLoader(std::shared_ptr<ThreadPool> threadPool) : threadPool(std::move(threadPool)) {}

void ImageLoader::loadImageAsync(const std::filesystem::path &imagePath,
                                 std::function<void(tl::expected<ImageData, std::string>)> onLoadDone) {
  enqueue([this, imagePath, onLoadDone] { onLoadDone(loadImage(imagePath)); });
}

tl::expected<ImageData, std::string> ImageLoader::loadImageWithChannels(const std::filesystem::path &imagePath,
                                                                        ChannelCount requiredChannels) {
  auto data = loadImage(imagePath);
  if (data.has_value()) {
    return convertImageChannels(std::move(data.value()), requiredChannels);
  } else {
    return tl::make_unexpected(data.error());
  }
}

tl::expected<ImageData, std::string> ImageLoader::convertImageChannels(ImageData &&data,
                                                                       ChannelCount requiredChannels) {
  if (requiredChannels == data.info.channels) { return data; }
  const auto rgbToGray = [](std::byte r, std::byte g, std::byte b) {
    return static_cast<std::byte>(0.299 * static_cast<float>(r) + 0.587 * static_cast<float>(g)
                                  + 0.114 * static_cast<float>(b));
  };
  switch (requiredChannels.get()) {
    case 1: {
      if (data.info.channels.get() == 3) {
        data.data = data.data | ranges::views::transform([](auto value) { return make_array(value, value, value); })
            | ranges::views::cache1 | ranges::views::join | ranges::to_vector;
      } else {
        data.data = data.data | ranges::views::transform([](auto value) {
                      return make_array(value, value, value, static_cast<std::byte>(255u));
                    })
            | ranges::views::cache1 | ranges::views::join | ranges::to_vector;
      }
      data.info.channels = ChannelCount{1};
      break;
    }
    case 3: {
      if (data.info.channels.get() == 1) {
        data.data = data.data | ranges::views::chunk(3)
            | ranges::views::transform([&](auto rgb) { return rgbToGray(rgb[0], rgb[1], rgb[2]); }) | ranges::to_vector;
      } else {
        data.data = data.data | ranges::views::chunk(3) | ranges::views::transform([](auto rgb) {
                      return make_array(rgb[0], rgb[1], rgb[2], static_cast<std::byte>(255u));
                    })
            | ranges::views::cache1 | ranges::views::join | ranges::to_vector;
      }
      data.info.channels = ChannelCount{3};
      break;
    }
    case 4: {
      if (data.info.channels.get() == 1) {
        data.data = data.data | ranges::views::chunk(4)
            | ranges::views::transform([&](auto rgb) { return rgbToGray(rgb[0], rgb[1], rgb[2]); }) | ranges::to_vector;
      } else {
        data.data = data.data | ranges::views::chunk(4)
            | ranges::views::transform([](auto rgb) { return make_array(rgb[0], rgb[1], rgb[2]); })
            | ranges::views::cache1 | ranges::views::join | ranges::to_vector;
      }
      data.info.channels = ChannelCount{4};
      break;
    }
    default: return tl::make_unexpected(fmt::format("Unsupported channel count '{}'", requiredChannels.get()));
  }
  return data;
}

void ImageLoader::loadImageWithChannelsAsync(const std::filesystem::path &imagePath, ChannelCount requiredChannels,
                                             std::function<void(tl::expected<ImageData, std::string>)> onLoadDone) {
  enqueue([this, imagePath, onLoadDone, requiredChannels] {
    onLoadDone(loadImageWithChannels(imagePath, requiredChannels));
  });
}

std::optional<ImageInfo> StbImageLoader::getImageInfo(const std::filesystem::path &imagePath) {
  int x;
  int y;
  int n;
  if (stbi_info(imagePath.string().c_str(), &x, &y, &n)) {
    const auto textureSize = TextureSize{TextureWidth{static_cast<std::uint32_t>(x)},
                                         TextureHeight{static_cast<std::uint32_t>(y)}, TextureDepth{1}};
    const auto channels = static_cast<std::uint8_t>(n);
    return ImageInfo{textureSize, ChannelCount{channels}};
  }
  return std::nullopt;
}

StbImageLoader::StbImageLoader(const std::shared_ptr<ThreadPool> &threadPool) : ImageLoader(threadPool) {}

tl::expected<ImageData, std::string> StbImageLoader::loadImage(const std::filesystem::path &imagePath) {
  int x;
  int y;
  int n;
  const auto data = stbi_load(imagePath.string().c_str(), &x, &y, &n, 0);
  if (data == nullptr) { return tl::make_unexpected("Image loading failed"); }
  const auto dataSpan = std::span{reinterpret_cast<const std::byte *>(data), static_cast<std::size_t>(x * y * n)};
  auto stbFree = RAII{[&] { stbi_image_free(data); }};
  return ImageData{{TextureSize{TextureWidth{static_cast<std::uint32_t>(x)},
                                TextureHeight{static_cast<std::uint32_t>(y)}, TextureDepth{1}},
                    ChannelCount{static_cast<std::uint8_t>(n)}},
                   std::vector<std::byte>{dataSpan.begin(), dataSpan.end()}};
}

OpenGLStbImageLoader::OpenGLStbImageLoader(const std::shared_ptr<ThreadPool> &threadPool)
    : StbImageLoader(threadPool) {}

tl::expected<std::shared_ptr<Texture>, std::string>
OpenGLStbImageLoader::loadTexture(const std::filesystem::path &imagePath) {
  const auto imageData = loadImage(imagePath);
  if (imageData.has_value()) {
    return createAndFillTexture(imageData.value());
  } else {
    return tl::make_unexpected(imageData.error());
  }
}

void OpenGLStbImageLoader::loadTextureAsync(
    const std::filesystem::path &imagePath,
    std::function<void(tl::expected<std::shared_ptr<Texture>, std::string>)> onLoadDone) {
  enqueue([this, imagePath, onLoadDone] {
    auto imageData = loadImage(imagePath);
    if (imageData.has_value()) {
      MainLoop::Get()->enqueue([this, onLoadDone, imageData = std::move(imageData.value())] {
        onLoadDone(createAndFillTexture(imageData));
      });
    } else {
      onLoadDone(tl::make_unexpected(imageData.error()));
    }
  });
}
tl::expected<std::shared_ptr<Texture>, std::string>
OpenGLStbImageLoader::loadTextureWithChannels(const std::filesystem::path &imagePath, ChannelCount requiredChannels) {
  const auto imageData = loadImageWithChannels(imagePath, requiredChannels);
  if (imageData.has_value()) {
    return createAndFillTexture(imageData.value());
  } else {
    return tl::make_unexpected(imageData.error());
  }
}

void OpenGLStbImageLoader::loadTextureWithChannelsAsync(
    const std::filesystem::path &imagePath, ChannelCount requiredChannels,
    std::function<void(tl::expected<std::shared_ptr<Texture>, std::string>)> onLoadDone) {
  enqueue([this, imagePath, onLoadDone, requiredChannels] {
    auto imageData = loadImageWithChannels(imagePath, requiredChannels);
    if (imageData.has_value()) {
      MainLoop::Get()->enqueue([this, onLoadDone, imageData = std::move(imageData.value())] {
        onLoadDone(createAndFillTexture(imageData));
      });
    } else {
      onLoadDone(tl::make_unexpected(imageData.error()));
    }
  });
}

tl::expected<std::shared_ptr<Texture>, std::string> OpenGLStbImageLoader::createAndFillTexture(const ImageData &data) {
  TextureFormat textureFormat;
  switch (data.info.channels.get()) {
    case 1: textureFormat = TextureFormat::R8; break;
    case 3: textureFormat = TextureFormat::RGB8; break;
    case 4: textureFormat = TextureFormat::RGBA8; break;
    default: return tl::make_unexpected(fmt::format("Unsupported channel count '{}'", data.info.channels.get()));
  }
  auto texture = std::make_shared<OpenGlTexture>(TextureTarget::_2D, textureFormat, TextureLevel{0}, data.info.size);
  if (const auto errOpt = texture->create(); errOpt.has_value()) {
    return tl::make_unexpected(std::string{errOpt.value().message()});
  }
  const auto setResult = texture->set2Ddata(std::span{data.data.data(), data.data.size()}, TextureLevel{0});
  if (setResult.has_value()) { return tl::make_unexpected(std::string{setResult->message()}); }
  return texture;
}

}  // namespace pf