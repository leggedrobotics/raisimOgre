//
// Created by Myank Mittal and Jemin Hwangbo on 6/18/19.
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
// C/C++
#include <chrono>

// raiSim
#include <raisim/World.hpp>
#include <raisim/OgreVis.hpp>
#include <guiState.hpp>
#include "helper.hpp"
#include "raisimBasicImguiPanel.hpp"

#define DEFAULT_JOINT_CONFIG 0.0, 2.76, -1.57, 0.0, 2.0, 0.0, 0.0, 0.0, 0.0

constexpr size_t JointsDim = 9;
using JointPositions = Eigen::Matrix<double, JointsDim, 1>;

void setupCallback() {
  auto vis = raisim::OgreVis::get();

  /// light
  vis->getLight()->setDiffuseColour(1, 1, 1);
  vis->getLight()->setCastShadows(true);
  vis->getLightNode()->setPosition(3, 3, 3);

  /// load  textures
  vis->addResourceDirectory(vis->getResourceDir() + "/material/checkerboard");
  vis->loadMaterialFile("checkerboard.material");

  /// scale related settings!! Please adapt it depending on your map size
  // beyond this distance, shadow disappears
  vis->getSceneManager()->setShadowFarDistance(10);
  // size of contact points and contact forces
  vis->setContactVisObjectSize(0.03, 0.2);
  // speed of camera motion in freelook mode
  vis->getCameraMan()->setTopSpeed(2);

  /// skybox
  Ogre::Quaternion quat;
  quat.FromAngleAxis(Ogre::Radian(M_PI_2), {1, 0, 0});
  vis->getSceneManager()->setSkyBox(true,
                                    "white",
                                    500,
                                    true,
                                    quat,
                                    Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
}


int main() {
  // PD control gains
  JointPositions jointPgain, jointDgain;
  jointPgain << 40.0, 40.0, 40.0, 15.0, 15.0, 15.0, 1.2, 1.2, 1.2;
  jointDgain << 1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 0.01, 0.01, 0.01;
  // max torque
  JointPositions maxTorque;
  maxTorque << 30.5, 30.5, 30.5, 6.8, 6.8, 6.8, 1.75, 1.75, 1.75;
  // Desired position and input torque
  JointPositions tau;
  std::vector<JointPositions> jointConfigTarget(100);
  JointPositions jointVelocityTarget; jointVelocityTarget.setZero();

  tau.setZero();
  // Error for PD controller
  JointPositions jointConfigError, jointVelError;
  // state
  JointPositions jointConfig, jointVel, jointConfigTemp;

  // create simulation handle
  raisim::World sim;
  sim.setTimeStep(0.002);
  std::vector<raisim::ArticulatedSystem *> kinovas;
  constexpr int kinoRows = 1;

  for(int i=0; i< kinoRows*kinoRows; i++) {
    kinovas.push_back(sim.addArticulatedSystem(raisim::loadResource("kinova/urdf/kinova_stand.urdf")));
    kinovas.back()->setControlMode(raisim::ControlMode::PD_PLUS_FEEDFORWARD_TORQUE);
  }

  auto table = sim.addBox(0.6, 0.6, 0.6, 100.);
  // auto cylinder = sim.addCylinder(0.03, 0.1, 1.);
  // auto box = sim.addBox(0.1, 0.1, 0.1, 1.0);
  // auto sphere = sim.addSphere(0.06, 1.);
  auto capsule = sim.addCapsule(0.03, 0.1, 1.);

  table->setPosition(1, 0, 0.31);
  // cylinder->setPosition(0.8, 0, 0.65);
  // box->setPosition(0.9, 0.1, 0.65);
  // sphere->setPosition(1.0, -0.1, 0.65);
  capsule->setPosition(1.1, 0, 0.75);

  // visualizer
  auto real_time_factor = 1.0;
  auto fps = 60.0;
  std::vector<::raisim::GraphicObject>* kinovaGraphic{nullptr};
  std::vector<::raisim::GraphicObject>* groundGraphic{nullptr};
  // Get local handle to visualizer singleton
  auto vis = ::raisim::OgreVis::get();
  vis->setWorld(&sim);
  vis->setDesiredFPS(fps);
  vis->setWindowSize(1900, 1020);
  vis->setSetUpCallback(setupCallback);
  vis->setImguiSetupCallback(imguiSetupCallback);
  vis->setImguiRenderCallback(imguiRenderCallBack);
  raisim::gui::manualStepping = true;

  vis->initApp();
  // Configure general properties
  vis->setContactVisObjectSize(0.025, 0.01);

  // Create the robot and ground visuals
  auto checkerBoard = sim.addGround();
  groundGraphic = vis->createGraphicalObject(checkerBoard, 20, "floor", "checkerboard_green");
  auto tableGraphic = vis->createGraphicalObject(table, "table", "gray");
  // auto cylinderGraphic = vis->createGraphicalObject(cylinder, "cylinder", "red");
  // auto boxGraphic = vis->createGraphicalObject(box, "box", "green");
  // auto sphereGraphic = vis->createGraphicalObject(sphere, "sphere", "orange");
  auto capsuleGraphic = vis->createGraphicalObject(capsule, "capsule", "brown");

  for(int i=0; i< kinoRows*kinoRows; i++)
    kinovaGraphic = vis->createGraphicalObject(kinovas[i], "kinova_" + std::to_string(i));

  vis->select(kinovaGraphic->at(0));
  vis->getCameraMan()->setYawPitchDist(Ogre::Radian(0.), Ogre::Radian(-1.), 3);
  std::default_random_engine generator;
  std::normal_distribution<double> distribution(0.0, 0.6);
  generator.seed(4);

  /*
   * Basic Operation
   */
  // Set initial state of the robot
  double time = 0;
  jointConfig << DEFAULT_JOINT_CONFIG;
  jointVel.setZero();
  tau.setZero();
  for(auto kinova : kinovas) {
    kinova->setState(jointConfig, jointVel);
    kinova->setGeneralizedForce(tau);
  }

  for(int i=0; i<kinoRows; i++) {
    for(int j=0; j<kinoRows; j++) {
      kinovas[i*kinoRows+j]->setBasePos({i*1.5, j*1.5, 0.1});
      jointConfigTarget[i*kinoRows+j] << DEFAULT_JOINT_CONFIG;
      for(int k=0; k<9; k++) {
        jointConfigTarget[i*kinoRows+j][k] += distribution(generator);
        jointConfigTarget[i*kinoRows+j][k]  = 0;
      }

      kinovas[i*kinoRows+j]->setPdTarget(jointConfigTarget[i*kinoRows+j], jointVelocityTarget);
      kinovas[i*kinoRows+j]->setPdGains(jointPgain, jointDgain);
    }
  }

  /// run the app
  vis->run();

  /// terminate
  vis->closeApp();

  return 0;
}