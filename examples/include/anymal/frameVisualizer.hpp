//
// Created by jemin on 12/21/19.
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

#ifndef RAISIMOGRE_FRAMEVISUALIZER_HPP
#define RAISIMOGRE_FRAMEVISUALIZER_HPP

#include "raisim/imgui.h"
#include "raisim/World.hpp"
#include "font.hpp"
#include <unordered_map>
#include <vector>
#include "raisim/OgreVis.hpp"

namespace raisim {
namespace anymal_gui {
namespace frame {

raisim::ArticulatedSystem *system = nullptr;
std::vector<int8_t> checkBoxes;

void callback() {
  if (!system) return;

  ImGui::PushFont(fontBig);
  ImGui::Text("Gait");
  ImGui::Spacing();
  ImGui::PopFont();

  /// visualizing axes
  auto vis = raisim::OgreVis::get();
  for (int i = 0; i < system->getFrames().size(); i++) {
    auto &frame = system->getFrames()[i];

    ImGui::Checkbox(frame.name.c_str(), (bool *) (&checkBoxes[i]));

    auto &xAxis = vis->getVisualObjectList()[frame.name + "_x"];
    auto &yAxis = vis->getVisualObjectList()[frame.name + "_y"];
    auto &zAxis = vis->getVisualObjectList()[frame.name + "_z"];

    xAxis.graphics->setVisible(checkBoxes[i]);
    yAxis.graphics->setVisible(checkBoxes[i]);
    zAxis.graphics->setVisible(checkBoxes[i]);

    Mat<3, 3> xAxisOri_B, yAxisOri_B, zAxisOri_B;
    raisim::zaxisToRotMat({1, 0, 0}, xAxisOri_B);
    raisim::zaxisToRotMat({0, 1, 0}, yAxisOri_B);
    raisim::zaxisToRotMat({0, 0, 1}, zAxisOri_B);

    Mat<3, 3> xAxisOri_W, yAxisOri_W, zAxisOri_W, frameOri;
    system->getFrameOrientation(frame, frameOri);
    matmul(frameOri, xAxisOri_B, xAxisOri_W);
    matmul(frameOri, yAxisOri_B, yAxisOri_W);
    matmul(frameOri, zAxisOri_B, zAxisOri_W);

    Vec<3> framePosi;
    system->getFramePosition(frame, framePosi);

    xAxis.setPosition(framePosi);
    yAxis.setPosition(framePosi);
    zAxis.setPosition(framePosi);

    xAxis.setOrientation(xAxisOri_W);
    yAxis.setOrientation(yAxisOri_W);
    zAxis.setOrientation(zAxisOri_W);
  }
}

std::function<void()> init() {
  return callback;
}

void setArticulatedSystem(raisim::ArticulatedSystem* robot, double scale) {
  system = robot;

  /// for displaying different frames
  checkBoxes.resize(system->getFrames().size(), 0);
  auto vis = raisim::OgreVis::get();

  for (auto &frame : system->getFrames()) {
    vis->addVisualObject(frame.name + "_x", "arrowMesh", "red", {0.3 * scale, 0.3 * scale, scale});
    vis->addVisualObject(frame.name + "_y", "arrowMesh", "green", {0.3 * scale, 0.3 * scale, scale});
    vis->addVisualObject(frame.name + "_z", "arrowMesh", "blue", {0.3 * scale, 0.3 * scale, scale});
  }
}

}
}
}

#endif //RAISIMOGRE_FRAMEVISUALIZER_HPP
