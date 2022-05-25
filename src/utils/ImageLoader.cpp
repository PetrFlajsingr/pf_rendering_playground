//
// Created by Petr on 23/05/2022.
//

#include "ImageLoader.h"
#include "gpu/opengl/Texture.h"
#include "pf_common/RAII.h"
#include "stb_image.h"

namespace pf {
ImageLoader::ImageLoader(std::shared_ptr<ThreadPool> threadPool) : threadPool(std::move(threadPool)) {}

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

void StbImageLoader::loadImageAsync(const std::filesystem::path &imagePath,
                                    std::function<void(tl::expected<ImageData, std::string>)> onLoadDone) {
  enqueue([this, imagePath, onLoadDone] { onLoadDone(loadImage(imagePath)); });
}

OpenGLStbImageLoader::OpenGLStbImageLoader(const std::shared_ptr<ThreadPool> &threadPool)
    : StbImageLoader(threadPool) {}

tl::expected<std::shared_ptr<Texture>, std::string>
OpenGLStbImageLoader::loadTexture(const std::filesystem::path &imagePath) {
  const auto imageData = loadImage(imagePath);
  if (imageData.has_value()) {
    TextureFormat textureFormat;
    switch (imageData.value().info.channels.get()) {
      case 1: textureFormat = TextureFormat::R8; break;
      case 3: textureFormat = TextureFormat::RGB8; break;
      case 4: textureFormat = TextureFormat::RGBA8; break;
      default:
        return tl::make_unexpected(
            fmt::format("Unsupported channel count '{}'", imageData.value().info.channels.get()));
    }
    auto texture = std::make_shared<OpenGlTexture>(TextureTarget::_2D, textureFormat, TextureLevel{0},
                                                   imageData.value().info.size);
    if (const auto errOpt = texture->create(); errOpt.has_value()) {
      return tl::make_unexpected(std::string{errOpt.value().message()});
    }
    const auto setResult =
        texture->set2Ddata(std::span{imageData->data.data(), imageData->data.size()}, TextureLevel{0});
    if (setResult.has_value()) { return tl::make_unexpected(std::string{setResult->message()}); }
    return texture;
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
      TextureFormat textureFormat;
      switch (imageData.value().info.channels.get()) {
        case 1: textureFormat = TextureFormat::R8; break;
        case 3: textureFormat = TextureFormat::RGB8; break;
        case 4: textureFormat = TextureFormat::RGBA8; break;
        default:
          onLoadDone(tl::make_unexpected(
              fmt::format("Unsupported channel count '{}'", imageData.value().info.channels.get())));
          return;  // unsupported channel count
      }
      MainLoop::Get()->enqueue([onLoadDone, textureFormat, imageData = std::move(imageData)] {
        auto texture = std::make_shared<OpenGlTexture>(TextureTarget::_2D, textureFormat, TextureLevel{0},
                                                       imageData.value().info.size);
        if (const auto errOpt = texture->create(); errOpt.has_value()) {
          onLoadDone(tl::make_unexpected(std::string{errOpt.value().message()}));
          return;  // texture creation failed
        }
        const auto setResult =
            texture->set2Ddata(std::span{imageData->data.data(), imageData->data.size()}, TextureLevel{0});
        if (setResult.has_value()) { onLoadDone(tl::make_unexpected(std::string{setResult->message()})); }
        onLoadDone(std::move(texture));
      });
    } else {
      onLoadDone(tl::make_unexpected(imageData.error()));
    }
  });
}

}  // namespace pf