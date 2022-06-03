//
// Created by Petr on 23/05/2022.
//

#pragma once

#include "gpu/RenderThread.h"
#include "gpu/Texture.h"
#include "pf_common/parallel/ThreadPool.h"
#include <filesystem>
#include <tl/expected.hpp>

namespace pf {

using ChannelCount = fluent::NamedType<std::uint8_t, struct ChannelCountTag, fluent::Comparable>;

struct ImageInfo {
  TextureSize size;
  ChannelCount channels;
};

struct ImageData {
  ImageInfo info;
  std::vector<std::byte> data;
};
// TODO: split this up into a 'normal' and an async loader and also separate texture creation from this
class ImageLoader {
 public:
  explicit ImageLoader(std::shared_ptr<ThreadPool> threadPool);
  virtual ~ImageLoader() = default;

  [[nodiscard]] virtual std::optional<ImageInfo> getImageInfo(const std::filesystem::path &imagePath) = 0;

  [[nodiscard]] virtual tl::expected<ImageData, std::string> loadImage(const std::filesystem::path &imagePath) = 0;
  virtual void loadImageAsync(const std::filesystem::path &imagePath,
                              std::function<void(tl::expected<ImageData, std::string>)> onLoadDone);

  [[nodiscard]] virtual tl::expected<ImageData, std::string>
  loadImageWithChannels(const std::filesystem::path &imagePath, ChannelCount requiredChannels);
  virtual void loadImageWithChannelsAsync(const std::filesystem::path &imagePath, ChannelCount requiredChannels,
                                          std::function<void(tl::expected<ImageData, std::string>)> onLoadDone);

  [[nodiscard]] virtual tl::expected<std::shared_ptr<Texture>, std::string>
  loadTexture(const std::filesystem::path &imagePath) = 0;
  virtual void
  loadTextureAsync(const std::filesystem::path &imagePath,
                   std::function<void(tl::expected<std::shared_ptr<Texture>, std::string>)> onLoadDone) = 0;

  [[nodiscard]] virtual tl::expected<std::shared_ptr<Texture>, std::string>
  loadTextureWithChannels(const std::filesystem::path &imagePath, ChannelCount requiredChannels) = 0;
  virtual void
  loadTextureWithChannelsAsync(const std::filesystem::path &imagePath, ChannelCount requiredChannels,
                               std::function<void(tl::expected<std::shared_ptr<Texture>, std::string>)> onLoadDone) = 0;

 protected:
  void enqueue(std::invocable auto &&fnc) {
    futures.emplace_back(threadPool->enqueue(std::forward<decltype(fnc)>(fnc)));
  }

  [[nodiscard]] tl::expected<ImageData, std::string> convertImageChannels(ImageData &&data,
                                                                          ChannelCount requiredChannels);

 private:
  std::shared_ptr<ThreadPool> threadPool;
  std::vector<std::future<void>> futures;
};

class StbImageLoader : public ImageLoader {
 public:
  explicit StbImageLoader(const std::shared_ptr<ThreadPool> &threadPool);
  [[nodiscard]] std::optional<ImageInfo> getImageInfo(const std::filesystem::path &imagePath) override;

  [[nodiscard]] tl::expected<ImageData, std::string> loadImage(const std::filesystem::path &imagePath) override;
};

class OpenGLStbImageLoader : public StbImageLoader {
 public:
  OpenGLStbImageLoader(const std::shared_ptr<ThreadPool> &threadPool, std::shared_ptr<RenderThread> renderingThread);

  [[nodiscard]] tl::expected<std::shared_ptr<Texture>, std::string>
  loadTexture(const std::filesystem::path &imagePath) override;
  void loadTextureAsync(const std::filesystem::path &imagePath,
                        std::function<void(tl::expected<std::shared_ptr<Texture>, std::string>)> onLoadDone) override;

  tl::expected<std::shared_ptr<Texture>, std::string> loadTextureWithChannels(const std::filesystem::path &imagePath,
                                                                              ChannelCount requiredChannels) override;
  void loadTextureWithChannelsAsync(
      const std::filesystem::path &imagePath, ChannelCount requiredChannels,
      std::function<void(tl::expected<std::shared_ptr<Texture>, std::string>)> onLoadDone) override;

 private:
  tl::expected<std::shared_ptr<Texture>, std::string> createAndFillTexture(const ImageData &data);

  std::shared_ptr<RenderThread> renderThread;
};

}  // namespace pf
