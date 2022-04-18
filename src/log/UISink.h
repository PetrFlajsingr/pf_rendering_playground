//
// Created by xflajs00 on 18.04.2022.
//

#ifndef PF_RENDERING_PLAYGROUND_UISINK_H
#define PF_RENDERING_PLAYGROUND_UISINK_H

#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>
#include <pf_imgui/elements/LogPanel.h>

template<typename Mutex>
class PfImguiLogSink : public spdlog::sinks::base_sink<Mutex> {
 public:
  template<std::size_t LogLimit>
  explicit PfImguiLogSink(pf::ui::ig::LogPanel<spdlog::level::level_enum, LogLimit> &logPanel)
      : spdlog::sinks::base_sink<Mutex>(),
        addRecord([this, &logPanel](auto level, auto message) { logPanel.addRecord(level, message); }) {
    logPanel.addDestroyListener([this] { panelValid = false; });
  }
  template<std::size_t LogLimit>
  PfImguiLogSink(pf::ui::ig::LogPanel<spdlog::level::level_enum, LogLimit> &logPanel,
                 const std::unique_ptr<spdlog::formatter> &formatter)
      : spdlog::sinks::base_sink<Mutex>(formatter),
        addRecord([this, &logPanel](auto level, auto message) { logPanel.addRecord(level, message); }) {
    logPanel.addDestroyListener([this] { panelValid = false; });
  }

 protected:
  void sink_it_(const spdlog::details::log_msg &msg) override {
    if (!panelValid) { return; }
    spdlog::memory_buf_t formatted;
    spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
    addRecord(msg.level, std::string_view{formatted.begin(), formatted.end()});
  }
  void flush_() override {}

 private:
  bool panelValid = true;
  std::function<void(spdlog::level::level_enum, std::string_view)> addRecord;
};
using PfImguiLogSink_mt = PfImguiLogSink<std::mutex>;
using PfImguiLogSink_st = PfImguiLogSink<spdlog::details::null_mutex>;


#endif//PF_RENDERING_PLAYGROUND_UISINK_H
