//
// Created by Petr on 6/5/2022.
//

#include "AudioAssetsView.h"
#include <fmt/chrono.h>

namespace pf {

namespace gui = ui::ig;

AudioAssetRecordElement::AudioAssetRecordElement(const std::string &name, const std::string &label,
                                                 const std::string &format, bool play, std::chrono::seconds length)
    : Element(name), Labellable(label), format(format), play(play), length(length) {}

void AudioAssetRecordElement::renderImpl() {
  ImGui::BeginVertical("rec_vert");
  ImGui::BeginHorizontal("rec_hor");
  ImGui::Text(getLabel().c_str());
  ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(91, 142, 34, 255));
  ImGui::Text(format.c_str());
  ImGui::PopStyleColor();
  ImGui::Text(fmt::format("Length: {}", length).c_str());
  if (ImGui::Checkbox("Play through speakers", &play)) { playObservableImpl.notify(play); }
  ImGui::EndHorizontal();

  ImGui::BeginHorizontal("rec_mid_rm");
  ImGui::Spring(1.f);
  if (ImGui::Button("Remove")) { removeObservableImpl.notify(); }
  ImGui::Spring(1.f);
  ImGui::EndHorizontal();
  ImGui::EndVertical();
}

const std::string &AudioAssetRecordElement::getFormat() const { return format; }

void AudioAssetRecordElement::setFormat(const std::string &newFormat) { format = newFormat; }

bool AudioAssetRecordElement::isPlay() const { return play; }

void AudioAssetRecordElement::setPlay(bool newPlay) { play = newPlay; }

const std::chrono::seconds &AudioAssetRecordElement::getLength() const { return length; }

void AudioAssetRecordElement::setLength(std::chrono::seconds newLength) { //-V813
  length = newLength;
}

AudioAssetsView::AudioAssetsView(const std::shared_ptr<ui::ig::ImGuiInterface> &imguiInterface,
                                 const std::string_view &windowName, const std::string_view &windowTitle)
    : UIViewWindow(imguiInterface, windowName, windowTitle) {
  window->setIsDockable(true);
  controlsLayout = &window->createChild(
      gui::HorizontalLayout::Config{.name = "layout", .size = gui::Size{gui::Width::Auto(), 35}, .showBorder = true});
  addButton = &controlsLayout->createChild<gui::Button>("add_var_btn", "Add audio");
  controlsSep = &controlsLayout->createChild<gui::Separator>("controls_separator");
  searchTextInput = &controlsLayout->createChild(gui::InputText::Config{
      .name = "search_input",
      .label = "Search",
  });
  recordsLayout = &window->createChild(
      gui::VerticalLayout::Config{.name = "records_layout", .size = gui::Size::Auto(), .showBorder = false});
  recordsLayout->setScrollable(true);

  createTooltips();
}

void AudioAssetsView::createTooltips() {
  addButton->setTooltip("Add a new audio asset");
  searchTextInput->setTooltip("Filter assets by name");
}

AudioAssetRecordElement &AudioAssetsView::addAssetElement(const std::string &label, const std::string &format,
                                                          bool play, std::chrono::seconds length) {
  auto &newRecord =
      recordsLayout->createChild<AudioAssetRecordElement>(std::string{label}, std::string{label}, format, play, length);
  records.emplace_back(&newRecord);
  return newRecord;
}

}  // namespace pf