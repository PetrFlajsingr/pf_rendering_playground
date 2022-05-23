//
// Created by Petr on 23/05/2022.
//

#ifndef PF_RENDERING_PLAYGROUND_IMAGELOADER_H
#define PF_RENDERING_PLAYGROUND_IMAGELOADER_H

#include "gpu/Texture.h"
#include "pf_common/parallel/ThreadPool.h"
#include "pf_mainloop/MainLoop.h"
#include <filesystem>
#include <tl/expected.hpp>

namespace pf {

struct ImageInfo {
  TextureSize size;
  std::uint8_t channels;
};

struct ImageData {
  ImageInfo info;
  std::vector<std::byte> data;
};

class ImageLoader {
 public:
  explicit ImageLoader(std::shared_ptr<ThreadPool> threadPool);
  virtual ~ImageLoader() = default;

  virtual std::optional<ImageInfo> getImageInfo(const std::filesystem::path &imagePath) = 0;

  virtual tl::expected<ImageData, std::string> loadImage(const std::filesystem::path &imagePath) = 0;
  virtual void loadImageAsync(const std::filesystem::path &imagePath,
                              std::function<void(tl::expected<ImageData, std::string>)> onLoadDone) = 0;

  virtual tl::expected<std::shared_ptr<Texture>, std::string> loadTexture(const std::filesystem::path &imagePath) = 0;
  virtual void
  loadTextureAsync(const std::filesystem::path &imagePath,
                   std::function<void(tl::expected<std::shared_ptr<Texture>, std::string>)> onLoadDone) = 0;

 protected:
  void enqueue(std::invocable auto &&fnc) {
    futures.emplace_back(threadPool.enqueue(std::forward<decltype(fnc)>(fnc)));
  }

 private:
  std::shared_ptr<ThreadPool> threadPool;
  std::vector<std::future<void>> futures;
};

class StbImageLoader : public ImageLoader {
 public:
  std::optional<ImageInfo> getImageInfo(const std::filesystem::path &imagePath) override;

  tl::expected<ImageData, std::string> loadImage(const std::filesystem::path &imagePath) override;
  void loadImageAsync(const std::filesystem::path &imagePath,
                      std::function<void(tl::expected<ImageData, std::string>)> onLoadDone) override;
};

class OpenGLStbImageLoader : public StbImageLoader {
 public:
  tl::expected<std::shared_ptr<Texture>, std::string> loadTexture(const std::filesystem::path &imagePath) override;

  void loadTextureAsync(const std::filesystem::path &imagePath,
                        std::function<void(tl::expected<std::shared_ptr<Texture>, std::string>)> onLoadDone) override;
};

}  // namespace pf

#endif  //PF_RENDERING_PLAYGROUND_IMAGELOADER_H
