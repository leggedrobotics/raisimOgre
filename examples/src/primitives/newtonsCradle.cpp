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
#include "raisimBasicImguiPanel.hpp"
#include "raisimKeyboardCallback.hpp"

void setupCallback() {
  auto vis = raisim::OgreVis::get();

  /// light
  vis->getLight()->setDiffuseColour(1, 1, 1);
  vis->getLight()->setCastShadows(true);
  vis->getLightNode()->setPosition(3, 3, 3);

  /// load  textures
  vis->addResourceDirectory(vis->getResourceDir() + "/material/gravel");
  vis->loadMaterialFile("gravel.material");

  vis->addResourceDirectory(vis->getResourceDir() + "/material/checkerboard");
  vis->loadMaterialFile("checkerboard.material");

  /// shdow setting
  vis->getSceneManager()->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_ADDITIVE);
  vis->getSceneManager()->setShadowTextureSettings(2048, 3);

  /// scale related settings!! Please adapt it depending on your map size
  // beyond this distance, shadow disappears
  vis->getSceneManager()->setShadowFarDistance(60);
  // size of contact points and contact forces
  vis->setContactVisObjectSize(0.03, 1.0);
  // speed of camera motion in freelook mode
  vis->getCameraMan()->setTopSpeed(10);

  /// background
  Ogre::Quaternion quat;
  quat.FromAngleAxis(Ogre::Radian(M_PI_2), {1., 0, 0});
  vis->getSceneManager()->setSkyBox(true,
                                    "white",
                                    500,
                                    true,
                                    quat,
                                    Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
}

int main(int argc, char **argv) {
  /// create raisim world
  raisim::World world;
  world.setTimeStep(0.001);
  world.setERP(0.5, 0.);

  /// starts visualizer thread

  /// these method must be called before initApp
  auto vis = raisim::OgreVis::get();
  vis->setWorld(&world);
  vis->setWindowSize(1800, 1000);
  vis->setImguiSetupCallback(imguiSetupCallback);
  vis->setImguiRenderCallback(imguiRenderCallBack);
  vis->setSetUpCallback(setupCallback);
  vis->setKeyboardCallback(raisimKeyboardCallback);
  vis->setAntiAliasing(2);
  raisim::gui::manualStepping = true;

  /// init
  vis->initApp();

  /// create raisim objects
  auto ground = world.addGround();

  // simulator
  world.setMaterialPairProp("steel", "steel", 0.1, 1.0, 0.0);

  auto pin1 = world.addSphere(0.1, 0.8);
  pin1->setPosition(0.0, 0.0, 3.0);
  pin1->setBodyType(raisim::BodyType::STATIC);
  auto pin1visual = vis->createGraphicalObject(pin1, "pin1", "lawn_green");

  auto pin2 = world.addSphere(0.1, 0.8);
  pin2->setPosition(0.3, 0.0, 3.0);
  pin2->setBodyType(raisim::BodyType::STATIC);
  vis->createGraphicalObject(pin2, "pin2", "lawn_green");

  auto pin3 = world.addSphere(0.1, 0.8);
  pin3->setPosition(0.6, 0.0, 3.0);
  pin3->setBodyType(raisim::BodyType::STATIC);
  vis->createGraphicalObject(pin3, "pin3", "lawn_green");

  auto pin4 = world.addSphere(0.1, 0.8);
  pin4->setPosition(0.9, 0.0, 3.0);
  pin4->setBodyType(raisim::BodyType::STATIC);
  vis->createGraphicalObject(pin4, "pin4", "lawn_green");

  auto pin5 = world.addSphere(0.1, 0.8);
  pin5->setPosition(0.9, 0.0, 6.0);
  pin5->setBodyType(raisim::BodyType::STATIC);
  vis->createGraphicalObject(pin5, "pin5", "lawn_green");

  auto ball1 = world.addSphere(0.1498, 0.8, "steel");
  ball1->setPosition(0, 0.0, 1.0);
  vis->createGraphicalObject(ball1, "ball1", "dark_red");

  auto ball2 = world.addSphere(0.1498, 0.8, "steel");
  ball2->setPosition(0.3, 0.0, 1.0);
  vis->createGraphicalObject(ball2, "ball2", "gray");

  auto ball3 = world.addSphere(0.1498, 0.8, "steel");
  ball3->setPosition(0.6, 0.0, 1.0);
  vis->createGraphicalObject(ball3, "ball3", "green");

  auto ball4 = world.addSphere(0.1498, 0.8, "steel");
  ball4->setPosition(2.9, 0.0, 3.0);
  vis->createGraphicalObject(ball4, "ball4", "orange");

  auto box = world.addBox(.1, .1, .1, 1);
  box->setPosition(0.9, 0.0, 4.0);
  vis->createGraphicalObject(box, "box", "red");

  auto wire1 = world.addStiffWire(pin1, 0, {0,0,0}, ball1, 0, {0,0,0}, 2.0);
  vis->createGraphicalObject(wire1, "wire1", "red");
  auto wire2 = world.addStiffWire(pin2, 0, {0,0,0}, ball2, 0, {0,0,0}, 2.0);
  vis->createGraphicalObject(wire2, "wire2", "red");
  auto wire3 = world.addStiffWire(pin3, 0, {0,0,0}, ball3, 0, {0,0,0}, 2.0);
  vis->createGraphicalObject(wire3, "wire3", "red");
  auto wire4 = world.addStiffWire(pin4, 0, {0,0,0}, ball4, 0, {0,0,0}, 2.0);
  vis->createGraphicalObject(wire4, "wire4", "red");

  auto wire5 = world.addCompliantWire(pin5, 0, {0,0,0}, box, 0, {0., 0, 0}, 2.0, 200);
  vis->createGraphicalObject(wire5, "wire5", "red");

  /// create visualizer objects
  vis->createGraphicalObject(ground, 20, "floor", "checkerboard_green");

  /// set camera
  vis->getCameraMan()->getCamera()->setPosition(0, 3.5, 1.5);
  vis->getCameraMan()->getCamera()->pitch(Ogre::Radian(1.2));
  vis->select(pin1visual->at(0));
  vis->getCameraMan()->setYawPitchDist(Ogre::Radian(0.), Ogre::Radian(-1.5), 5);

  /// run the app
  vis->run();

  /// terminate
  vis->closeApp();

  return 0;
}