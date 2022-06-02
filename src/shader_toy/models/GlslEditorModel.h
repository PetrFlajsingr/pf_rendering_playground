//
// Created by Petr on 26/05/2022.
//

#pragma once

#include "mvc/Model.h"
#include "mvc/reactive.h"
#include <chrono>

namespace pf {

class GlslEditorModel : public SavableModel {
  template<typename... Args>
  using Event = PublicEvent<Args...>;

  using CompilationRequestedEvent = Event<>;
  using RestartRequestedEvent = Event<>;
  using CopyCodeToClipboardRequestedEvent = Event<>;

 public:
  GlslEditorModel(bool autoCompile, std::chrono::milliseconds autoCompilePeriod, bool timePaused, std::string code);

  Observable<bool> autoCompile;
  Observable<std::chrono::milliseconds> autoCompilePeriod;
  Observable<bool> timePaused;
  // TODO: change this so it doesn't own the string
  Observable<std::string> code;
  Observable<bool> compiling{false};

  CompilationRequestedEvent compilationRequested;
  RestartRequestedEvent restartRequested;
  CopyCodeToClipboardRequestedEvent copyCodeToClipboardRequested;

  [[nodiscard]] toml::table toToml() const override;
  void setFromToml(const toml::table &src) override;

  [[nodiscard]] std::string getDebugString() const override;
};

}  // namespace pf
