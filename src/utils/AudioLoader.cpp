//
// Created by Petr on 04/06/2022.
//

#pragma warning( disable : 4996 )
#include "AudioLoader.h"
#include "fmt/format.h"
#include "pf_common/RAII.h"
#include "pf_common/algorithms.h"
#include "range/v3/view/transform.hpp"
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
}
#include <span>
#include <ztd/out_ptr.hpp>

namespace pf {

std::chrono::seconds AudioData::getLength() const {
  return std::chrono::seconds{data.size() / sampleRate / channelCount};
}

AudioLoader::AudioLoader(std::shared_ptr<ThreadPool> threadPool) : pool(std::move(threadPool)) {}

void AudioLoader::loadAudioFileAsync(const std::filesystem::path &path, AudioPCMFormat requestedFormat,
                                 std::optional<int> requestedSampleRate,
                                 std::function<void(tl::expected<AudioData, std::string>)> onLoadDone) {
  enqueue([=, this] { onLoadDone(loadAudioFile(path, requestedFormat, requestedSampleRate)); });
}

AVAudioLoader::AVAudioLoader(std::shared_ptr<ThreadPool> threadPool) : AudioLoader(std::move(threadPool)) {}
AVAudioLoader::~AVAudioLoader() = default;
tl::expected<AudioData, std::string> AVAudioLoader::loadAudioFile(const std::filesystem::path &path,
                                                                  AudioPCMFormat requestedFormat,
                                                                  std::optional<int> requestedSampleRate) {
  // get format from audio file
  const auto avFormatCtxDeleter = [](auto format) { avformat_free_context(format); };
  auto format =
      std::unique_ptr<AVFormatContext, decltype(avFormatCtxDeleter)>(avformat_alloc_context(), avFormatCtxDeleter);
  if (avformat_open_input(ztd::out_ptr::inout_ptr(format), path.string().c_str(), nullptr, nullptr) != 0) {
    return tl::make_unexpected(fmt::format("Could not open file '{}'", path.string()));
  }
  if (avformat_find_stream_info(format.get(), nullptr) < 0) {
    return tl::make_unexpected(fmt::format("Could not create stream for file '{}'", path.string()));
  }

  // Find the index of the first audio stream
  auto streams = std::span{format->streams, format->nb_streams};
  const auto streamIter =
      std::ranges::find_if(streams, [](auto stream) { return stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO; });
  if (streamIter == streams.end()) {
    return tl::make_unexpected(fmt::format("Could not create audio stream for file '{}'", path.string()));
  }
  AVStream *stream = *streamIter;

  // find & open codec
  AVCodecParameters *codecParameters = stream->codecpar;
  const auto avCodecCtxDeleter = [](auto context) { avcodec_free_context(&context); };
  auto codecContext =
      std::unique_ptr<AVCodecContext, decltype(avCodecCtxDeleter)>(avcodec_alloc_context3(nullptr), avCodecCtxDeleter);
  avcodec_parameters_to_context(codecContext.get(), codecParameters);
  if (avcodec_open2(codecContext.get(), avcodec_find_decoder(codecContext->codec_id), nullptr) < 0) {
    return tl::make_unexpected(fmt::format("Could not open decoder for file '{}'", path.string()));
  }
  auto closeCodecScopeExit = RAII{[&codecContext] { avcodec_close(codecContext.get()); }};

  int outChannels = 0;
  switch (requestedFormat) {
    case AudioPCMFormat::U8Mono: [[fallthrough]];
    case AudioPCMFormat::U8Stereo: outChannels = 1; break;
    case AudioPCMFormat::U16Mono: [[fallthrough]];
    case AudioPCMFormat::U16Stereo: outChannels = 2; break;
  }
  int channelLayout = AV_CH_LAYOUT_MONO;
  if (isIn(requestedFormat, std::vector{AudioPCMFormat::U8Stereo, AudioPCMFormat::U16Stereo})) {
    channelLayout = AV_CH_LAYOUT_STEREO;
  }
  AVSampleFormat sampleFormat = AV_SAMPLE_FMT_U8;
  if (isIn(requestedFormat, std::vector{AudioPCMFormat::U8Stereo, AudioPCMFormat::U16Stereo})) {
    sampleFormat = AV_SAMPLE_FMT_S16;
  }
  // prepare resampler
  const auto swrDeleter = [](auto swr) { swr_free(&swr); };
  auto swr = std::unique_ptr<struct SwrContext, decltype(swrDeleter)>(swr_alloc(), swrDeleter);
  av_opt_set_int(swr.get(), "in_channel_count", codecContext->channels, 0);
  av_opt_set_int(swr.get(), "out_channel_count", outChannels, 0);
  av_opt_set_int(swr.get(), "in_channel_layout", static_cast<std::int64_t>(codecContext->channel_layout), 0);
  av_opt_set_int(swr.get(), "out_channel_layout", AV_CH_LAYOUT_MONO, 0);
  av_opt_set_int(swr.get(), "in_sample_rate", codecContext->sample_rate, 0);
  av_opt_set_int(swr.get(), "out_sample_rate", requestedSampleRate.value_or(codecContext->sample_rate), 0);
  av_opt_set_sample_fmt(swr.get(), "in_sample_fmt", codecContext->sample_fmt, 0);
  av_opt_set_sample_fmt(swr.get(), "out_sample_fmt", sampleFormat, 0);
  swr_init(swr.get());
  if (!swr_is_initialized(swr.get())) { return tl::make_unexpected("Error during resampler initialisation"); }

  // prepare to read data
  const auto avPacketDeleter = [](auto packet) { av_packet_free(&packet); };
  auto packet = std::unique_ptr<AVPacket, decltype(avPacketDeleter)>(av_packet_alloc(), avPacketDeleter);
  if (packet == nullptr) { return tl::make_unexpected("Error allocating packet"); }
  const auto avFrameDeleter = [](auto frame) { av_frame_free(&frame); };
  auto frame = std::unique_ptr<AVFrame, decltype(avFrameDeleter)>(av_frame_alloc(), avFrameDeleter);
  if (frame == nullptr) { return tl::make_unexpected("Error allocating frame"); }

  AudioData audioData{};
  audioData.sampleRate = codecContext->sample_rate;
  audioData.channelCount = 1;
  // iterate through frames
  while (av_read_frame(format.get(), packet.get()) >= 0) {
    // decode one frame
    int gotFrame;
    if (avcodec_decode_audio4(codecContext.get(), frame.get(), &gotFrame, packet.get()) < 0) {
      break;
    }
    if (!gotFrame) {
      continue;
    }
    // resample frames
    std::uint8_t* buffer;
    av_samples_alloc((uint8_t**) &buffer, nullptr, 1, frame->nb_samples, sampleFormat, 0);
    int frame_count = swr_convert(swr.get(), (uint8_t**) &buffer, frame->nb_samples, (const uint8_t**) frame->data, frame->nb_samples);

    auto frameData = std::span{buffer, static_cast<std::size_t>(frame_count)};

    audioData.data.reserve(audioData.data.size() + frameData.size());
    std::ranges::copy(frameData | ranges::views::transform([](auto val) { return std::byte{val}; }),
                      std::back_inserter(audioData.data));

    av_freep(&buffer);

    av_frame_unref(frame.get());
  }


  // success
  return audioData;
}
}  // namespace pf