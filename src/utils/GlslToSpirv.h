//
// Created by xflajs00 on 01.05.2022.
//

#ifndef PF_RENDERING_PLAYGROUND_GLSLTOSPIRV_H
#define PF_RENDERING_PLAYGROUND_GLSLTOSPIRV_H

#include <charconv>
#include <optional>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/split.hpp>
#include <range/v3/view/transform.hpp>
#include <tl/expected.hpp>
#include <utils/algorithms.h>
#include <vector>

namespace pf {

struct SpirvCompilationResult {
  std::vector<unsigned int> spirvData;
  std::string messages;
};

struct SpirvErrorRecord {
  enum class Type { Warning, Error };

  Type type;
  std::optional<std::size_t> line;
  std::string error;
  std::string errorDesc;
};

struct SpirvCompilationError {
  std::string info;
  std::string debugInfo;

  // Returns view to SpirvErrorRecords
  [[nodiscard]] auto getInfoRecords() const;
  [[nodiscard]] auto getDebugInfoRecords() {}
};

[[nodiscard]] tl::expected<SpirvCompilationResult, SpirvCompilationError>
glslComputeShaderSourceToSpirv(const std::string &glslSource);


auto SpirvCompilationError::getInfoRecords() const {
  return info | ranges::view::split('\n')
      | ranges::view::transform([](auto &&line) -> std::optional<SpirvErrorRecord> {
           auto lineView =
               std::string_view{&*line.begin(), static_cast<std::string_view::size_type>(ranges::distance(line))};
           auto result = SpirvErrorRecord{};
           if (lineView.starts_with("ERROR")) {
             result.type = SpirvErrorRecord::Type::Error;
           } else if (lineView.starts_with("WARNING")) {
             result.type = SpirvErrorRecord::Type::Warning;
           } else {
             return std::nullopt;
           }

           const auto parts = splitByDelimiter(lineView, ':');
           if (parts.size() < 3) { return std::nullopt; }
           int lineNumber;
           const auto [ptr, ec] = std::from_chars(parts[2].c_str(), parts[2].c_str() + parts[2].size(), lineNumber);
           if (ec == std::errc()) { result.line = lineNumber; }

           if (parts.size() >= 4) { result.error = parts[3]; }
           if (parts.size() >= 5) { result.errorDesc = parts[4]; }
           return result;
         })
      | ranges::view::filter(&std::optional<SpirvErrorRecord>::has_value)
      | ranges::view::transform([](const auto &val) { return val.value(); });
}

}  // namespace pf
#endif  //PF_RENDERING_PLAYGROUND_GLSLTOSPIRV_H
