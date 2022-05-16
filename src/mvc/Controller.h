//
// Created by xflajs00 on 15.05.2022.
//

#pragma once

#include "View.h"
#include "reactive.h"

namespace pf{

template<std::derived_from<UIViewBase> UIView, typename Model>
class Controller {
 public:
  Controller(std::unique_ptr<UIView> uiView, std::shared_ptr<Model> mod)
      : view(std::move(uiView)), model(std::move(mod)) {}
  [[nodiscard]] const std::shared_ptr<Model> &getModel() { return model; }
  [[nodiscard]] std::shared_ptr<const Model> getModel() const { return model; }
  [[nodiscard]] UIView &getView() { return *view; }
  [[nodiscard]] const UIView &getView() const { return *view; }

 protected:
  template<typename ...Args>
  using Event = Event<Controller, Args...>;

  void notifyEvent(auto &event) {
    event.notify();
  }

  std::unique_ptr<UIView> view;
  std::shared_ptr<Model> model;
};

}
