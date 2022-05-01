//
// Created by xflajs00 on 01.05.2022.
//

#ifndef PF_RENDERING_PLAYGROUND_GLSLTOSPIRV_H
#define PF_RENDERING_PLAYGROUND_GLSLTOSPIRV_H

#include <tl/expected.hpp>
#include <vector>
#include <optional>
#include <range/v3/view/split.hpp>
#include <range/v3/view/transform.hpp>

namespace pf {

struct SpirvCompilationResult {
  std::vector<unsigned int> spirvData;
  std::string messages;
};

struct SpirvErrorRecord {
  enum class Type {
    Warning, Error
  };

  Type type;
  std::optional<std::size_t> line;
};

struct SpirvCompilationError {
  std::string info;
  std::string debugInfo;

  [[nodiscard]] auto getInfoRecords() const {
    return info | ranges::view::split('\n') | ranges::view::transform([] (const auto &&line) -> std::optional<SpirvErrorRecord> {
             auto lineView = std::string_view{line.begin(), line.end()};
             auto result = SpirvErrorRecord{};
             if (lineView.starts_with("ERROR")) {
               result.type = SpirvErrorRecord::Type::Error;
             } else if (lineView.starts_with("WARNING")) {
               result.type = SpirvErrorRecord::Type::Warning;
             } else {
               return std::nullopt;
             }
             return result;
           });
  }
  [[nodiscard]] auto getDebugInfoRecords() {

  }
};

[[nodiscard]] tl::expected<SpirvCompilationResult, SpirvCompilationError>
glslSourceToSpirv(const std::string &glslSource);



}  // namespace pf
#endif  //PF_RENDERING_PLAYGROUND_GLSLTOSPIRV_H
