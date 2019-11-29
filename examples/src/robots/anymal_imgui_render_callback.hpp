//
// Created by jemin on 11/4/19.
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

#ifndef RAISIMOGRE_ANYMAL_IMGUI_RENDER_CALLBACK_HPP
#define RAISIMOGRE_ANYMAL_IMGUI_RENDER_CALLBACK_HPP

#include "guiState.hpp"
#include "raisim/imgui_plot.h"

namespace raisim {
namespace anymal_gui {

ImFont* fontBig;
ImFont* fontMid;
ImFont* fontSmall;

constexpr int windowSize = 200;
extern std::vector<std::vector<float>> jointSpeed, jointTorque;
extern std::vector<float> time;

int data_size = 0;
const float* speed_data[12], *torque_data[12];

void init() {
  data_size = 0;
  jointSpeed.resize(12);
  jointTorque.resize(12);
  time.resize(windowSize);

  for (auto &joint: raisim::anymal_gui::jointSpeed)
    joint.resize(windowSize, 0.f);

  for (auto &joint: raisim::anymal_gui::jointTorque)
    joint.resize(windowSize, 0.f);

  for (int i=0; i<12; i++)
    speed_data[i] = &jointSpeed[i][0];

  for (int i=0; i<12; i++)
    torque_data[i] = &jointTorque[i][0];
}

void push_back(double time_point, const Eigen::VectorXd& joint_speed, const Eigen::VectorXd& joint_torque) {
  if(data_size != windowSize) {
    for(int i=0; i<12; i++) {
      jointSpeed[i][data_size] = float(joint_speed[i]);
      jointTorque[i][data_size] = float(joint_torque[i]);
    }
    time[data_size] = time_point;
    data_size++;
  } else {
    for(int i=0; i<12; i++) {
      for(int j=0; j<windowSize-1; j++) {
        jointSpeed[i][j] = jointSpeed[i][j+1];
        jointTorque[i][j] = jointTorque[i][j+1];
      }
      jointSpeed[i][data_size-1] = joint_speed[i];
      jointTorque[i][data_size-1] = joint_torque[i];
    }

    for(int j=0; j<windowSize-1; j++) {
      time[j] = time[j+1];
    }
    time[data_size-1] = time_point;
  }
}

void clear() {
  data_size = 0;
}

void anymalImguiRenderCallBack() {
  static ImU32 colors[12] = {ImColor(114,229,239),
                             ImColor(52,115,131),
                             ImColor(111,239,112),
                             ImColor(30,123,32),
                             ImColor(201,221,135),
                             ImColor(137,151,91),
                             ImColor(233,173,111),
                             ImColor(159,88,39),
                             ImColor(214,68,5),
                             ImColor(235,62,134),
                             ImColor(142,0,73),
                             ImColor(191,214,250)};

  static uint32_t selection_start = 0, selection_length = 0;
  ImGui::SetNextWindowPos({0, 0});
  ImGui::Begin("Example plot", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

  // Draw first plot with multiple sources
  ImGui::PlotConfig conf;
  conf.values.xs = &raisim::anymal_gui::time[0];
  conf.values.count = raisim::anymal_gui::data_size;
  conf.values.ys_list = &raisim::anymal_gui::speed_data[0]; // use ys_list to draw several lines simultaneously
  conf.values.ys_count = 12;
  conf.values.colors = colors;
  conf.scale.min = -10;
  conf.scale.max = 10;
  conf.grid_x.show = true;
  conf.grid_x.size = 10;
  conf.grid_x.subticks = 4;
  conf.grid_y.show = true;
  conf.grid_y.size = 0.5f;
  conf.grid_y.subticks = 5;
  conf.selection.show = true;
  conf.selection.start = &selection_start;
  conf.selection.length = &selection_length;
  conf.frame_size = ImVec2(500, 300);
  ImGui::PushFont(fontBig);
  ImGui::Text("Joint speed");
  ImGui::Spacing();
  ImGui::PopFont();

  ImGui::Plot("plot1", conf);

  // Draw second plot with the selection
  // reset previous values
  conf.values.ys_list = &raisim::anymal_gui::torque_data[0];
  conf.selection.show = false;
  conf.scale.min = -80;
  conf.scale.max = 80;
  ImGui::PushFont(fontBig);
  ImGui::Text("Joint torque");
  ImGui::Spacing();
  ImGui::PopFont();
  ImGui::Plot("plot2", conf);

  if (ImGui::CollapsingHeader("Video recording")) {
    auto vis = raisim::OgreVis::get();
    if(vis->isRecording()) {
      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.8f, 0.2f, 1.f));
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.9f, 0.3f, 1.f));
      ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.9f, 0.5f, 1.f));

      if(ImGui::Button("Stop Recording ")) {
        RSINFO("Stop recording")
        raisim::OgreVis::get()->stopRecordingVideoAndSave();
      }

      ImGui::PopStyleColor(3);
    } else {
      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.f));
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.f));
      ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.9f, 0.5f, 0.5f, 1.f));

      if(ImGui::Button("Record ")){
        RSINFO("Start recording")
        raisim::OgreVis::get()->startRecordingVideo(raisim::OgreVis::get()->getResourceDir() + "/test.mp4");
      }

      ImGui::PopStyleColor(3);
    }
  }

  ImGui::End();
}


void imguiSetupCallback() {

#define HI(v)   ImVec4(0.502f, 0.075f, 0.256f, v)
#define MED(v)  ImVec4(0.455f, 0.198f, 0.301f, v)
#define LOW(v)  ImVec4(0.232f, 0.201f, 0.271f, v)
  // backgrounds (@todo: complete with BG_MED, BG_LOW)
#define BG(v)   ImVec4(0.200f, 0.220f, 0.270f, v)
  // text
#define TEXT(v) ImVec4(0.860f, 0.930f, 0.890f, v)

  auto &style = ImGui::GetStyle();
  style.Alpha = 0.8;
  style.Colors[ImGuiCol_Text]                  = TEXT(0.78f);
  style.Colors[ImGuiCol_TextDisabled]          = TEXT(0.28f);
  style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.13f, 0.14f, 0.17f, 1.00f);
  style.Colors[ImGuiCol_ChildWindowBg]         = BG( 0.58f);
  style.Colors[ImGuiCol_PopupBg]               = BG( 0.9f);
  style.Colors[ImGuiCol_Border]                = ImVec4(0.31f, 0.31f, 1.00f, 0.00f);
  style.Colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  style.Colors[ImGuiCol_FrameBg]               = BG( 1.00f);
  style.Colors[ImGuiCol_FrameBgHovered]        = MED( 0.78f);
  style.Colors[ImGuiCol_FrameBgActive]         = MED( 1.00f);
  style.Colors[ImGuiCol_TitleBg]               = LOW( 1.00f);
  style.Colors[ImGuiCol_TitleBgActive]         = HI( 1.00f);
  style.Colors[ImGuiCol_TitleBgCollapsed]      = BG( 0.75f);
  style.Colors[ImGuiCol_MenuBarBg]             = BG( 0.47f);
  style.Colors[ImGuiCol_ScrollbarBg]           = BG( 1.00f);
  style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.09f, 0.15f, 0.16f, 1.00f);
  style.Colors[ImGuiCol_ScrollbarGrabHovered]  = MED( 0.78f);
  style.Colors[ImGuiCol_ScrollbarGrabActive]   = MED( 1.00f);
  style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.71f, 0.22f, 0.27f, 1.00f);
  style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.47f, 0.77f, 0.83f, 0.14f);
  style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.71f, 0.22f, 0.27f, 1.00f);
  style.Colors[ImGuiCol_Button]                = ImVec4(0.47f, 0.77f, 0.83f, 0.14f);
  style.Colors[ImGuiCol_ButtonHovered]         = MED( 0.86f);
  style.Colors[ImGuiCol_ButtonActive]          = MED( 1.00f);
  style.Colors[ImGuiCol_Header]                = MED( 0.76f);
  style.Colors[ImGuiCol_HeaderHovered]         = MED( 0.86f);
  style.Colors[ImGuiCol_HeaderActive]          = HI( 1.00f);
  style.Colors[ImGuiCol_Column]                = ImVec4(0.14f, 0.16f, 0.19f, 1.00f);
  style.Colors[ImGuiCol_ColumnHovered]         = MED( 0.78f);
  style.Colors[ImGuiCol_ColumnActive]          = MED( 1.00f);
  style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(0.47f, 0.77f, 0.83f, 0.04f);
  style.Colors[ImGuiCol_ResizeGripHovered]     = MED( 0.78f);
  style.Colors[ImGuiCol_ResizeGripActive]      = MED( 1.00f);
  style.Colors[ImGuiCol_PlotLines]             = TEXT(0.63f);
  style.Colors[ImGuiCol_PlotLinesHovered]      = MED( 1.00f);
  style.Colors[ImGuiCol_PlotHistogram]         = TEXT(0.63f);
  style.Colors[ImGuiCol_PlotHistogramHovered]  = MED( 1.00f);
  style.Colors[ImGuiCol_TextSelectedBg]        = MED( 0.43f);
  // [...]
  style.Colors[ImGuiCol_ModalWindowDarkening]  = BG( 0.73f);

  style.WindowPadding            = ImVec2(6, 4);
  style.WindowRounding           = 0.0f;
  style.FramePadding             = ImVec2(5, 2);
  style.FrameRounding            = 3.0f;
  style.ItemSpacing              = ImVec2(7, 1);
  style.ItemInnerSpacing         = ImVec2(1, 1);
  style.TouchExtraPadding        = ImVec2(0, 0);
  style.IndentSpacing            = 6.0f;
  style.ScrollbarSize            = 12.0f;
  style.ScrollbarRounding        = 16.0f;
  style.GrabMinSize              = 20.0f;
  style.GrabRounding             = 2.0f;

  style.WindowTitleAlign.x = 0.50f;

  style.Colors[ImGuiCol_Border] = ImVec4(0.539f, 0.479f, 0.255f, 0.162f);
  style.FrameBorderSize = 0.0f;
  style.WindowBorderSize = 1.0f;

  ImGuiIO &io = ImGui::GetIO();
  fontBig = io.Fonts->AddFontFromFileTTF((raisim::OgreVis::get()->getResourceDir() + "/font/DroidSans.ttf").c_str(), 25.0f);
  fontMid = io.Fonts->AddFontFromFileTTF((raisim::OgreVis::get()->getResourceDir() + "/font/DroidSans.ttf").c_str(), 22.0f);
  fontSmall = io.Fonts->AddFontFromFileTTF((raisim::OgreVis::get()->getResourceDir() + "/font/DroidSans.ttf").c_str(), 16.0f);
}

}
}


#endif //RAISIMOGRE_ANYMAL_IMGUI_RENDER_CALLBACK_HPP
