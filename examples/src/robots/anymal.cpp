//
// Created by Jemin Hwangbo on 2/28/19.
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


#include <raisim/OgreVis.hpp>
#include <helper.hpp>

#include "anymal/anymal_imgui_render_callback.hpp"
#include "anymal/gaitLogger.hpp"
#include "anymal/jointSpeedTorqueLogger.hpp"
#include "anymal/rewardLogger.hpp"
#include "anymal/videoLogger.hpp"
#include "anymal/frameVisualizer.hpp"

using namespace raisim;

void setupCallback() {
  auto vis = raisim::OgreVis::get();

  /// light
  vis->getLight()->setDiffuseColour(1, 1, 1);
  vis->getLight()->setCastShadows(true);
  Ogre::Vector3 lightdir(-3,-3,-0.5);
  lightdir.normalise();
  vis->getLightNode()->setDirection({lightdir});

  /// load  textures
  vis->addResourceDirectory(vis->getResourceDir() + "/material/checkerboard");
  vis->loadMaterialFile("checkerboard.material");

  /// shdow setting
  vis->getSceneManager()->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_ADDITIVE);
  vis->getSceneManager()->setShadowTextureSettings(2048, 3);

  /// scale related settings!! Please adapt it depending on your map size
  // beyond this distance, shadow disappears
  vis->getSceneManager()->setShadowFarDistance(10);
  // size of contact points and contact forces
  vis->setContactVisObjectSize(0.03, 0.6);
  // speed of camera motion in freelook mode
  vis->getCameraMan()->setTopSpeed(5);

  /// skybox
  Ogre::Quaternion quat;
  quat.FromAngleAxis(Ogre::Radian(M_PI_2), {1., 0, 0});
  vis->getSceneManager()->setSkyBox(true,
                                    "Examples/StormySkyBox",
                                    500,
                                    true,
                                    quat);
}

int main(int argc, char **argv) {
  /// create raisim world
  raisim::World world;
  world.setTimeStep(0.0025);

  /// just a shortcut
  auto vis = raisim::OgreVis::get();

  /// gui
  anymal_gui::init({anymal_gui::video::init(vis->getResourceDir()),
                    anymal_gui::joint_speed_and_torque::init(100),
                    anymal_gui::gait::init(100),
                    anymal_gui::reward::init({"commandTracking", "torque"}),
                    anymal_gui::frame::init()});

  /// these method must be called before initApp
  vis->setWorld(&world);
  vis->setWindowSize(1800, 1200);
  vis->setImguiSetupCallback(raisim::anymal_gui::imguiSetupCallback);
  vis->setImguiRenderCallback(raisim::anymal_gui::anymalImguiRenderCallBack);
  vis->setSetUpCallback(setupCallback);
  vis->setAntiAliasing(2);

  /// starts visualizer thread
  vis->initApp();

  /// create raisim objects
  auto anymal = world.addArticulatedSystem(raisim::loadResource("anymal/anymal.urdf"));
  auto ground = world.addGround();
  ground->setName("checkerboard"); /// not necessary here but once you set name, you can later retrieve it using raisim::World::getObject()

  /// create visualizer objects
  vis->createGraphicalObject(ground, 20, "floor", "checkerboard_green");
  auto anymal_graphics = vis->createGraphicalObject(anymal, "ANYmal"); // this is the name assigned for raisimOgre. It is displayed using this name
  anymal_gui::frame::setArticulatedSystem(anymal, 0.3); // to visualize frames

  /// ANYmal joint PD controller
  Eigen::VectorXd jointNominalConfig(19), jointVelocityTarget(18);
  Eigen::VectorXd jointState(18), jointForce(18), jointPgain(18), jointDgain(18);
  Eigen::VectorXd jointTorque(12), jointSpeed(12);

  jointPgain.setZero();
  jointDgain.setZero();
  jointVelocityTarget.setZero();
  jointPgain.tail(12).setConstant(200.0);
  jointDgain.tail(12).setConstant(10.0);

  /// set anymal properties
  anymal->setGeneralizedCoordinate({0, 0, 0.54, 1.0, 0.0, 0.0, 0.0, 0.03, 0.4,
                                    -0.8, -0.03, 0.4, -0.8, 0.03, -0.4, 0.8, -0.03, -0.4, 0.8});
  anymal->setGeneralizedForce(Eigen::VectorXd::Zero(anymal->getDOF()));
  anymal->setControlMode(raisim::ControlMode::PD_PLUS_FEEDFORWARD_TORQUE);
  anymal->setPdGains(jointPgain, jointDgain);
  anymal->setName("anymal"); // this is the name assigned for raisim. Not used in this example

  /// contacts
  std::vector<size_t> footIndices;
  std::array<bool, 4> footContactState;
  footIndices.push_back(anymal->getBodyIdx("LF_SHANK"));
  footIndices.push_back(anymal->getBodyIdx("RF_SHANK"));
  footIndices.push_back(anymal->getBodyIdx("LH_SHANK"));
  footIndices.push_back(anymal->getBodyIdx("RH_SHANK"));

  /// just to get random motions of anymal
  std::default_random_engine generator;
  std::normal_distribution<double> distribution(0.0, 0.6);
  std::srand(std::time(nullptr));
  double time=0.;

  /// lambda function for the controller
  auto controller = [anymal,
                     &generator,
                     &distribution,
                     &jointTorque,
                     &jointSpeed,
                     &time,
                     &world,
                     &footIndices,
                     &footContactState]() {
    static size_t controlDecimation = 0;
    time += world.getTimeStep();

    if(controlDecimation++ % 2500 == 0) {
      anymal->setGeneralizedCoordinate({0, 0, 0.54, 1.0, 0.0, 0.0, 0.0, 0.03, 0.4,
                                        -0.8, -0.03, 0.4, -0.8, 0.03, -0.4, 0.8, -0.03, -0.4, 0.8});
      raisim::anymal_gui::reward::clear();
      raisim::anymal_gui::gait::clear();
      raisim::anymal_gui::joint_speed_and_torque::clear();

      time = 0.;
    }

    jointTorque = anymal->getGeneralizedForce().e().tail(12);
    jointSpeed = anymal->getGeneralizedVelocity().e().tail(12);

    if(controlDecimation % 50 != 0)
      return;

    /// ANYmal joint PD controller
    Eigen::VectorXd jointNominalConfig(19), jointVelocityTarget(18);
    jointVelocityTarget.setZero();
    jointNominalConfig << 0, 0, 0, 0, 0, 0, 0, 0.03, 0.3, -.6, -0.03, 0.3, -.6, 0.03, -0.3, .6, -0.03, -0.3, .6;

    for (size_t k = 0; k < anymal->getGeneralizedCoordinateDim(); k++)
      jointNominalConfig(k) += distribution(generator);

    anymal->setPdTarget(jointNominalConfig, jointVelocityTarget);

    /// check if the feet are in contact with the ground
    for(auto& fs: footContactState) fs = false;

    for(auto& contact: anymal->getContacts()) {
      auto it = std::find(footIndices.begin(), footIndices.end(), contact.getlocalBodyIndex());
      size_t index = it - footIndices.begin();
      if (index < 4)
        footContactState[index] = true;
    }

    /// torque, speed and contact state
    anymal_gui::joint_speed_and_torque::push_back(time, jointSpeed, jointTorque);
    anymal_gui::gait::push_back(footContactState);

    /// just displaying random numbers since we are not doing any training here
    anymal_gui::reward::log("torque", distribution(generator));
    anymal_gui::reward::log("commandTracking", distribution(generator));
  };

  vis->setControlCallback(controller);

  /// set camera
  vis->select(anymal_graphics->at(0), false);
  vis->getCameraMan()->setYawPitchDist(Ogre::Radian(0), -Ogre::Radian(M_PI_4), 2);

  /// run the app
  vis->run();

  /// terminate
  vis->closeApp();

  return 0;
}
