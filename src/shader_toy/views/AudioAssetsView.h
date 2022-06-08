//
// Created by Petr on 6/5/2022.
//

#pragma once

#include "mvc/View.h"
#include <pf_imgui/elements/Button.h>
#include <pf_imgui/elements/Separator.h>
#include <pf_imgui/interface/Element.h>
#include <pf_imgui/layouts/HorizontalLayout.h>
#include <pf_imgui/layouts/VerticalLayout.h>
#include <pf_imgui/layouts/WrapLayout.h>

namespace pf {

class AudioAssetRecordTile : public ui::ig::Element, public ui::ig::Labellable, public ui::ig::Resizable {
 public:
  AudioAssetRecordTile(const std::string &name, const std::string &label, ui::ig::Size size, const std::string &format,
                       bool play, std::chrono::seconds length);

  [[nodiscard]] const std::string &getFormat() const;
  void setFormat(const std::string &newFormat);
  [[nodiscard]] bool isPlay() const;
  void setPlay(bool newPlay);
  [[nodiscard]] const std::chrono::seconds &getLength() const;
  void setLength(std::chrono::seconds newLength);

  Subscription addRemoveClickListener(std::invocable auto &&listener) {
    return removeObservableImpl.addListener(std::forward<decltype(listener)>(listener));
  }

  Subscription addPlayListener(std::invocable<bool> auto &&listener) {
    return playObservableImpl.addListener(std::forward<decltype(listener)>(listener));
  }

 protected:
  void renderImpl() override;

 private:
  std::string format;
  bool play;
  std::chrono::seconds length;

  ui::ig::Observable_impl<> removeObservableImpl;
  ui::ig::Observable_impl<bool> playObservableImpl;
};

class AudioAssetsView : public UIViewWindow {
 public:
  AudioAssetsView(const std::shared_ptr<ui::ig::ImGuiInterface> &imguiInterface, const std::string_view &windowName,
                  const std::string_view &windowTitle);

  AudioAssetRecordTile &addAssetElement(const std::string &label, const std::string &format, bool play,
                                        std::chrono::seconds length);

  // clang-format off
  ui::ig::HorizontalLayout *controlsLayout;
    ui::ig::Button *addButton;
    ui::ig::Separator *controlsSep;
    ui::ig::InputText *searchTextInput;
  ui::ig::WrapLayout *audioLayout;
    std::vector<AudioAssetRecordTile*> audioTiles;
  // clang-format on

 private:
  void createTooltips();
  ui::ig::Size tileSize{220, 120};
};

}  // namespace pf
