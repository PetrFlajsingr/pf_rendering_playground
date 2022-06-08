//
// Created by Petr on 6/5/2022.
//

#include "AudioAssetsView.h"
#include <fmt/chrono.h>

namespace pf {

namespace gui = ui::ig;

AudioAssetRecordTile::AudioAssetRecordTile(const std::string &name, const std::string &label, gui::Size size,
                                           const std::string &format, bool play, std::chrono::seconds length)
    : Element(name), Labellable(label), Resizable(size), format(format), play(play), length(length) {}

void AudioAssetRecordTile::renderImpl() {
  ImGui::BeginChild(getName().c_str(), static_cast<ImVec2>(getSize()), true);
  ImGui::BeginVertical("rec_vert");
  ImGui::Text(getLabel().c_str());
  ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(91, 142, 34, 255));
  ImGui::Text(format.c_str());
  ImGui::PopStyleColor();
  ImGui::Text(fmt::format("Length: {}", length).c_str());
  if (ImGui::Checkbox("Play through speakers", &play)) { playObservableImpl.notify(play); }
  ImGui::Spring(1.f);
  ImGui::BeginHorizontal("rec_mid_rm");
  ImGui::Spring(1.f);
  if (ImGui::Button("Remove")) { removeObservableImpl.notify(); }
  ImGui::Spring(1.f);
  ImGui::EndHorizontal();
  ImGui::EndVertical();
  ImGui::EndChild();
}

const std::string &AudioAssetRecordTile::getFormat() const { return format; }

void AudioAssetRecordTile::setFormat(const std::string &newFormat) { format = newFormat; }

bool AudioAssetRecordTile::isPlay() const { return play; }

void AudioAssetRecordTile::setPlay(bool newPlay) { play = newPlay; }

const std::chrono::seconds &AudioAssetRecordTile::getLength() const { return length; }

void AudioAssetRecordTile::setLength(std::chrono::seconds newLength) {  //-V813
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
  audioLayout = &window->createChild(gui::WrapLayout::Config{.name = "records_layout",
                                                             .layoutDirection = gui::LayoutDirection::LeftToRight,
                                                             .size = gui::Size::Auto()});
  audioLayout->setScrollable(true);

  createTooltips();
}

void AudioAssetsView::createTooltips() {
  addButton->setTooltip("Add a new audio asset");
  searchTextInput->setTooltip("Filter assets by name");
}

AudioAssetRecordTile &AudioAssetsView::addAssetElement(const std::string &label, const std::string &format, bool play,
                                                       std::chrono::seconds length) {
  auto &newRecord = audioLayout->createChild<AudioAssetRecordTile>(std::string{label}, std::string{label}, tileSize,
                                                                   format, play, length);
  audioTiles.emplace_back(&newRecord);
  return newRecord;
}

}  // namespace pf