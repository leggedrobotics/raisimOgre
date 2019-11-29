
#ifndef _GAIT_LOGGER_HPP
#define _GAIT_LOGGER_HPP
#include "raisim/imgui_plot.h"
#include "font.hpp"
#include <unordered_map>
#include <vector>
#include <Eigen/Core>
#include "raisim/SlidingMemory.hpp"

namespace raisim {
namespace anymal_gui {
namespace gait {

static std::vector<SlidingMemory> contactStates;

void clear() {
  for (size_t i = 0; i < 4; i++)
    contactStates[i].clear();
}

void push_back(std::array<bool, 4> &in) {
  for (size_t i = 0; i < 4; i++) {
    contactStates[i].push_back((float) in[i]);
  }
}

void callback() {
  ImGui::PushFont(fontBig);
  ImGui::Text("Gait");
  ImGui::Spacing();
  ImGui::PopFont();

  ImGui::Separator();
  int size = contactStates[0].size();
  ImGui::PushItemWidth(-1);
  ImGui::PushFont(fontMid);

  ImGui::PlotHistogram("", contactStates[0].data(), size, 0, "LF", 0, 1);
  ImGui::PlotHistogram("", contactStates[1].data(), size, 0, "RF", 0, 1);
  ImGui::PlotHistogram("", contactStates[2].data(), size, 0, "LH", 0, 1);
  ImGui::PlotHistogram("", contactStates[3].data(), size, 0, "RH", 0, 1);

  ImGui::PopFont();
  ImGui::PopItemWidth();
}

std::function<void()> init(int window_size = 0) {
  contactStates.resize(4, SlidingMemory(window_size, 0.));
  return callback;
}

}
}

};
#endif //_RAISIM_GYM_REWARDLOGGER_HPP