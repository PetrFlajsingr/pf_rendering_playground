//
// Created by Petr on 04/06/2022.
//

#include "AudioLoader.h"
#include "fmt/format.h"
#include "pf_common/RAII.h"
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
}
#include <span>
#include <ztd/out_ptr.hpp>

namespace pf {

AVAudioLoader::~AVAudioLoader() = default;

tl::expected<AudioData, std::string> AVAudioLoader::loadAudioFile(const std::filesystem::path &path) {
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
  // TODO: loading with resampling
  // prepare resampler
  const auto swrDeleter = [](auto swr) { swr_free(&swr); };
  auto swr = std::unique_ptr<struct SwrContext, decltype(swrDeleter)>(swr_alloc(), swrDeleter);
  av_opt_set_int(swr.get(), "in_channel_count", codecContext->channels, 0);
  av_opt_set_int(swr.get(), "out_channel_count", 1, 0);
  av_opt_set_int(swr.get(), "in_channel_layout", static_cast<std::int64_t>(codecContext->channel_layout), 0);
  av_opt_set_int(swr.get(), "out_channel_layout", AV_CH_LAYOUT_MONO, 0);
  av_opt_set_int(swr.get(), "in_sample_rate", codecContext->sample_rate, 0);
  av_opt_set_int(swr.get(), "out_sample_rate", codecContext->sample_rate, 0);
  av_opt_set_sample_fmt(swr.get(), "in_sample_fmt", codecContext->sample_fmt, 0);
  av_opt_set_sample_fmt(swr.get(), "out_sample_fmt", AV_SAMPLE_FMT_DBL, 0);
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
    auto result = avcodec_send_packet(codecContext.get(), packet.get());
    if (result < 0) { return tl::make_unexpected("Error sending packet to decoder"); }
    while (!result) {
      result = avcodec_receive_frame(codecContext.get(), frame.get());
      if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
        break;
      } else if (result < 0) {
        return tl::make_unexpected("Error during decoding");
      }
    }

    // resample frames
    double *buffer;
    av_samples_alloc((uint8_t **) &buffer, nullptr, 1, frame->nb_samples, AV_SAMPLE_FMT_DBL, 0);
    int frame_count = swr_convert(swr.get(), (uint8_t **) &buffer, frame->nb_samples, (const uint8_t **) frame->data,
                                  frame->nb_samples);
    // append resampled frames to data
    auto frameData = std::span{buffer, static_cast<std::size_t>(frame_count)};

    audioData.data.reserve(audioData.data.size() + frameData.size());
    std::ranges::copy(frameData, std::back_inserter(audioData.data));

    av_freep(buffer);

    av_frame_unref(frame.get());
  }

  // success
  return audioData;
}

}  // namespace pf