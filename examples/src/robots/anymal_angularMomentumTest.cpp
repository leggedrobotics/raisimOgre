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

#include "raisimBasicImguiPanel.hpp"
#include "raisimKeyboardCallback.hpp"

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
  auto vis = raisim::OgreVis::get();

  /// these method must be called before initApp
  vis->setWorld(&world);
  vis->setWindowSize(1800, 1200);
  vis->setImguiSetupCallback(imguiSetupCallback);
  vis->setImguiRenderCallback(imguiRenderCallBack);
  vis->setKeyboardCallback(raisimKeyboardCallback);
  vis->setSetUpCallback(setupCallback);
  vis->setAntiAliasing(2);

  /// starts visualizer thread
  vis->initApp();

  world.setTimeStep(0.0001);

  world.setERP(0,0);
  world.setGravity({0, 0, 0});

  auto anymal = world.addArticulatedSystem(raisim::loadResource("anymal/anymal_no_col.urdf"));
  auto anymal_graphics = vis->createGraphicalObject(anymal, "ANYmal"); // this is the name assigned for raisimOgre. It is displayed using this name

  Eigen::VectorXd referenceConfig(19), targetConfig(19), targetVel(18);
  Eigen::VectorXd jointPgain(18), jointDgain(18);

  targetVel.setZero();
  jointPgain.setZero();
  jointDgain.setZero();
  jointPgain.tail(12).setConstant(200.0);
  jointDgain.tail(12).setConstant(10.0);
  anymal->setPdGains(jointPgain, jointDgain);

  referenceConfig <<
      0, 0, 10,
      1.0, 0.0, 0.0, 0.0,
      0.03, 0.4, -0.8,
      -0.03, 0.4, -0.8,
      0.03, -0.4, 0.8,
      -0.03, -0.4, 0.8;
  targetConfig = referenceConfig;
  anymal->setGeneralizedCoordinate(referenceConfig);
  anymal->setGeneralizedForce(Eigen::VectorXd::Zero(anymal->getDOF()));
  anymal->updateMassInfo();

  anymal->setControlMode(raisim::ControlMode::PD_PLUS_FEEDFORWARD_TORQUE);

  // random
  std::default_random_engine generator;
  std::uniform_real_distribution<double> uniDist(-1., 1.);
  generator.seed(42);

  raisim::Vec<3> angularMomentum, referencePoint;
  referencePoint.setZero();

  anymal->getAngularMomentum(referencePoint, angularMomentum);

  /// lambda function for the controller
  auto controller = [anymal,
                     &generator,
                     &uniDist,
                     &referenceConfig,
                     &targetConfig,
                     &targetVel,
                     &world]() {
    static size_t controlDecimation = 0;

    if(controlDecimation++ % 10 == 0) {
      for(int j = 7; j < 19; j++)
        targetConfig[j] = referenceConfig[j] + 0.5 * uniDist(generator);

      anymal->setPdTarget(targetConfig, targetVel);
    }

    raisim::Vec<3> angularMomentum, referencePoint;
    referencePoint.setZero();

    world.integrate1();
    anymal->getAngularMomentum(referencePoint, angularMomentum);

    std::cout<<"angular momentum "<<angularMomentum.norm()<<std::endl;

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
