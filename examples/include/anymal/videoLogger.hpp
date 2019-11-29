//
// Created by jemin on 11/9/19.
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

#ifndef _RAISIM_GYM_ANYMAL_VIDEOLOGGER_HPP
#define _RAISIM_GYM_ANYMAL_VIDEOLOGGER_HPP
#include "raisim/imgui_plot.h"
#include "font.hpp"

namespace raisim {
namespace anymal_gui {
namespace video {

static std::string fileSavePath;
static int numberOfVidoes = 0;

void callback() {
  ImGui::PushFont(fontBig);
  ImGui::Text("Joint speed and torque");
  ImGui::Spacing();
  ImGui::PopFont();

  auto vis = raisim::OgreVis::get();
  if (vis->isRecording()) {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.8f, 0.2f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.9f, 0.3f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.9f, 0.5f, 1.f));

    if (ImGui::Button("Stop Recording ")) {
      RSINFO("Stop recording")
      raisim::OgreVis::get()->stopRecordingVideoAndSave();
    }

    ImGui::PopStyleColor(3);
  } else {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.9f, 0.5f, 0.5f, 1.f));

    if (ImGui::Button("Record ")) {
      RSINFO("Start recording")
      raisim::OgreVis::get()->startRecordingVideo(
          fileSavePath + "/anymal_gui_video" + std::to_string(numberOfVidoes++) + ".mp4");
    }

    ImGui::PopStyleColor(3);
  }
}

std::function<void()> init(const std::string& savePath) {
  fileSavePath = savePath;
  return callback;
}

}
}
}

#endif //_RAISIM_GYM_ANYMAL_VIDEOLOGGER_HPP
