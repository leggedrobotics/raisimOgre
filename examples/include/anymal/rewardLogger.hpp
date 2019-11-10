//
// Created by jemin on 5/18/19.
// MIT License
//
// Copyright (c) 2019-2019 Robotic Systems Lab, ETH Zurich
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef _RAISIM_GYM_REWARDLOGGER_HPP
#define _RAISIM_GYM_REWARDLOGGER_HPP

#include <unordered_map>
#include "font.hpp"

namespace raisim {
namespace anymal_gui {
namespace reward {

class RewardTerm {
 public:
  RewardTerm() = default;

  void clean() {
    sum = 0.;
    count = 0;
    values.clear();
  }

  void log(double value) {
    values.push_back(value);
    sum += value;
    count++;
  }

  double sum = 0.;
  int count = 0;
  std::vector<double> values;
};

static std::unordered_map<std::string, RewardTerm> rewardTerms;

void log(const std::string &termName, double value) {
  rewardTerms[termName].log(value);
}

void clear() {
  for (auto &item : rewardTerms)
    item.second.clean();
}

void callback() {
  ImGui::PushFont(fontBig);
  ImGui::Text("Reward logger");
  ImGui::Spacing();
  ImGui::PopFont();

  ImGui::Separator();
  double max = 0.;
  for (auto &item: rewardTerms)
    max = std::max(max, fabs(item.second.sum / double(item.second.count)));

  for (auto &item: rewardTerms) {
    double avg = item.second.sum / double(item.second.count);
    float percentage = fabs(avg) / max;
    char buf[32];
    sprintf(buf, "%f", avg);
    ImGui::ProgressBar(percentage, ImVec2(0.f, 0.f), buf);
    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::Text("  %s", item.first.c_str());
  }
}

std::function<void()> init(std::initializer_list<std::string> rewardTermNames) {
  for (auto &item : rewardTermNames)
    rewardTerms[item] = RewardTerm();
  return callback;
}

}
}

}
#endif //_RAISIM_GYM_REWARDLOGGER_HPP
