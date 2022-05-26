//
// Created by Petr on 26/05/2022.
//

#include "GlslEditorModel.h"

namespace pf {

GlslEditorModel::GlslEditorModel(bool autoCompile, std::chrono::milliseconds autoCompilePeriod, bool timePaused,
                                 std::string code)
    : autoCompile(autoCompile), autoCompilePeriod(autoCompilePeriod), timePaused(timePaused), code(code) {}

toml::table GlslEditorModel::toToml() const {
  return toml::table{{"autoCompile", *autoCompile},
                     {"autoCompilePeriod", autoCompilePeriod->count()},
                     {"timePaused", *timePaused},
                     {"code", *code}};
}

void GlslEditorModel::setFromToml(const toml::table &src) {
  if (const auto iter = src.find("autoCompile"); iter != src.end()) {
    if (const auto autoCompileVal = iter->second.as_boolean(); autoCompileVal != nullptr) {
      *autoCompile.modify() = autoCompileVal->get();
    }
  }
  if (const auto iter = src.find("autoCompilePeriod"); iter != src.end()) {
    if (const auto autoCompilePeriodVal = iter->second.as_integer(); autoCompilePeriodVal != nullptr) {
      *autoCompilePeriod.modify() = std::chrono::milliseconds{autoCompilePeriodVal->get()};
    }
  }
  if (const auto iter = src.find("timePaused"); iter != src.end()) {
    if (const auto timePausedVal = iter->second.as_boolean(); timePausedVal != nullptr) {
      *timePaused.modify() = timePausedVal->get();
    }
  }
  if (const auto iter = src.find("code"); iter != src.end()) {
    if (const auto codeVal = iter->second.as_string(); codeVal != nullptr) { *code.modify() = codeVal->get(); }
  }
}

}  // namespace pf