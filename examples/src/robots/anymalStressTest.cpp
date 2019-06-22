//
// Created by jemin on 2/28/19.
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
#include "raisimBasicImguiPanel.hpp"
#include <random>

void setupCallback() {
  /// light
  raisim::OgreVis::getLight()->setDiffuseColour(1, 1, 1);
  raisim::OgreVis::getLight()->setCastShadows(true);
  raisim::OgreVis::getLightNode()->setPosition(3, 3, 3);
  raisim::OgreVis::setCameraSpeed(300);

  /// shdow setting
  vis->getSceneManager()->setShadowTechnique(SHADOWTYPE_TEXTURE_ADDITIVE);
  vis->getSceneManager()->setShadowTextureSettings(2048, 3);

  /// skybox
  Ogre::Quaternion quat;
  quat.FromAngleAxis(Ogre::Radian(M_PI_2), {1, 0, 0});
  raisim::OgreVis::getSceneManager()->setSkyBox(true,
                                                "Examples/StormySkyBox",
                                                500,
                                                true,
                                                quat,
                                                ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
}

int main(int argc, char **argv) {
  /// create raisim world
  raisim::World world;
  world.setTimeStep(0.001);
  world.setERP(world.getTimeStep() * .1, world.getTimeStep() * .1);

  auto vis = raisim::OgreVis::get();

  /// these method must be called before initApp
  vis->setWorld(&world);
  vis->setWindowSize(1800, 900);
  vis->setImguiSetupCallback(imguiSetupCallback);
  vis->setImguiRenderCallback(imguiRenderCallBack);
  vis->setSetUpCallback(setupCallback);
  vis->setAntiAliasing(8);

  /// starts visualizer thread
  vis->initApp();

  /// create raisim objects
  auto ground = world.addGround();
  auto anymal = world.addArticulatedSystem(raisim::loadResource("ANYmal_bear2/anymal.urdf"));

  /// create visualizer objects
  vis->createGroundVisualAndRegister(ground, 50, "floor", "default");
  auto anymal_graphics = vis->createArticulatedSystemVisualAndRegister(anymal, "ANYmal");

  /// ANYmal joint PD controller
  Eigen::VectorXd jointState(18), jointVel(18), jointForce(18), jointPgain(18), jointDgain(18);
  jointPgain.setZero();
  jointDgain.setZero();
  jointPgain.tail(12).setConstant(50.0);
  jointDgain.tail(12).setConstant(1.0);
  jointVel.setZero();

  anymal->setGeneralizedCoordinate({0.5, 5 * 0.5, 0.54, 1.0, 0.0, 0.0, 0.0, 0.03, 0.4,
                                    -0.8, -0.03, 0.4, -0.8, 0.03, -0.4, 0.8, -0.03, -0.4, 0.8});

  vis->setSelected(anymal_graphics->at(0));
  vis->getCameraMan()->setYawPitchDist(Ogre::Radian(0), -Ogre::Radian(M_PI_4), 1);
  anymal->setGeneralizedForce(Eigen::VectorXd::Zero(anymal->getDOF()));
  anymal->setControlMode(raisim::ControlMode::PD_PLUS_FEEDFORWARD_TORQUE);
//  anymal->setControlMode(raisim::ControlMode::FORCE_AND_TORQUE);

  anymal->setPdGains(jointPgain, jointDgain);

  std::default_random_engine generator;
  std::normal_distribution<double> distribution(0.0, 1.5);

  std::srand(std::time(nullptr));

  auto controller = [&anymal, &generator, &distribution]() {
    static size_t controlDecimation = 0;

    if(controlDecimation++ % 10 != 0)
      return;

    /// ANYmal joint PD controller
    Eigen::VectorXd jointNominalConfig(19), jointVelocityTarget(18);
    jointVelocityTarget.setZero();
    jointNominalConfig << 0, 0, 0, 0, 0, 0, 0, 0.03, 0.7, -1.4, -0.03, 0.7, -1.4, 0.03, -0.7, 1.4, -0.03, -0.7, 1.4;

    for(size_t i=0; i < anymal->getGeneralizedCoordinateDim(); i++) {
      jointNominalConfig(i) += distribution(generator);
    }

    anymal->setPdTarget(jointNominalConfig, jointVelocityTarget);
  };

  vis->setControlCallback(controller);

  /// run the app
  vis->run();

  /// terminate
  vis->closeApp();

  return 0;
}