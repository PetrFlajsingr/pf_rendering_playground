//
// Created by Petr on 03/06/2022.
//

#pragma once

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "utils/files.h"
#include <memory>

namespace pf::log {
static inline std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> stdout_color_sink =
    std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

inline std::shared_ptr<spdlog::sinks::basic_file_sink_mt> createFileSink(std::string_view filename) {
  return std::make_shared<spdlog::sinks::basic_file_sink_mt>((getExeFolder() / filename).string());
}

}  // namespace pf::log