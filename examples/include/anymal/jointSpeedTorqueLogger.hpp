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

#ifndef _RAISIM_GYM_ANYMAL_JOINTSPEEDTORQUELOGGER_HPP
#define _RAISIM_GYM_ANYMAL_JOINTSPEEDTORQUELOGGER_HPP
#include "raisim/imgui_plot.h"
#include "font.hpp"
#include "raisim/SlidingMemory.hpp"

namespace raisim {
namespace anymal_gui {
namespace joint_speed_and_torque {

static constexpr int numberOfJoints = 12;
static std::vector<SlidingMemory> jointSpeed, jointTorque;
static std::unique_ptr<SlidingMemory> time;
static const float *speed_data[numberOfJoints], *torque_data[numberOfJoints];

void push_back(double time_point,
               const Eigen::VectorXd &joint_speed,
               const Eigen::VectorXd &joint_torque) {
  time->push_back(float(time_point));

  for (int i = 0; i < numberOfJoints; i++) {
    jointSpeed[i].push_back(float(joint_speed[i]));
    jointTorque[i].push_back(float(joint_torque[i]));
  }
}

void clear() {
  time->clear();

  for (int i = 0; i < numberOfJoints; i++) {
    jointSpeed[i].clear();
    jointTorque[i].clear();
  }
}

void callback() {
  // ADD MORE COLORS FOR DIFFERENT A NUMBER OF LEGS //
  static ImU32 colors[numberOfJoints] = {ImColor(114, 229, 239),
                                         ImColor(52, 115, 131),
                                         ImColor(111, 239, 112),
                                         ImColor(30, 123, 32),
                                         ImColor(201, 221, 135),
                                         ImColor(137, 151, 91),
                                         ImColor(233, 173, 111),
                                         ImColor(159, 88, 39),
                                         ImColor(214, 68, 5),
                                         ImColor(235, 62, 134),
                                         ImColor(142, 0, 73),
                                         ImColor(191, 214, 250)};

  static uint32_t selection_start = 0, selection_length = 0;

  for (int i = 0; i < numberOfJoints; i++) {
    speed_data[i] = jointSpeed[i].data();
    torque_data[i] = jointTorque[i].data();
  }

  // Draw first plot with multiple sources
  ImGui::PlotConfig conf;
  conf.values.xs = time->data();
  conf.values.count = jointSpeed[0].size();
  conf.values.ys_list = &speed_data[0];
  conf.values.ys_count = numberOfJoints;
  conf.values.colors = colors;
  conf.scale.min = -15;
  conf.scale.max = 15;
  conf.grid_x.show = false;
  conf.grid_x.size = 5;
  conf.grid_x.subticks = 2;
  conf.selection.show = true;
  conf.selection.start = &selection_start;
  conf.selection.length = &selection_length;
  conf.grid_y.show = false;
  conf.grid_y.size = 10.0f;
  conf.grid_y.subticks = 2;
  conf.selection.show = true;
  conf.frame_size = ImVec2(560, 200);
  ImGui::PushFont(fontBig);
  ImGui::Text("Joint speed (max 15, min -15 rad/s)");
  ImGui::PopFont();

  ImGui::Plot("plot1", conf);

  // Draw second plot with the selection
  // reset previous values
  conf.values.ys_list = &torque_data[0];
  conf.selection.show = false;
  conf.scale.min = -80;
  conf.scale.max = 80;
  conf.grid_y.size = 20.0f;
  ImGui::PushFont(fontBig);
  ImGui::Text("Joint torque (max 80, min -80)");
  ImGui::PopFont();
  ImGui::Plot("plot2", conf);
}

std::function<void()> init(int buffer_size) {
  jointSpeed.resize(numberOfJoints, SlidingMemory(buffer_size, 0.));
  jointTorque.resize(numberOfJoints, SlidingMemory(buffer_size, 0.));
  time = std::make_unique<SlidingMemory>(buffer_size, 0.);

  return callback;
}

}

}
}

#endif //_RAISIM_GYM_ANYMAL_JOINTSPEEDTORQUELOGGER_HPP
