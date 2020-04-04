//
// Created by Jemin Hwangbo on 2/28/19.
// MIT License
//
// Copyright (c) 2019-2020 Jemin Hwangbo
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
  vis->setContactVisObjectSize(0.02, 3.0);
  // speed of camera motion in freelook mode
  vis->getCameraMan()->setTopSpeed(10);
}

int main(int argc, char **argv) {
  /// create raisim world
  raisim::World world;
  world.setTimeStep(0.003);

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

  auto box = world.addBox(1, 1, 1, 2);
  box->setPosition(0,0,0.5);
  box->setBodyType(raisim::BodyType::STATIC);
  auto capsule = world.addCapsule(0.1, 0.2, 1);
  capsule->setPosition(0.5,0,1.1);
  capsule->setOrientation(0.5,0,0.7,0);
  vis->createGraphicalObject(capsule, "capsule", "blue");
  vis->createGraphicalObject(box, "box", "blue");

  /// set camera
  vis->getCameraMan()->getCamera()->setPosition(0,-3, 5);
  vis->getCameraMan()->getCamera()->pitch(Ogre::Radian(1.2));

  /// run the app
  vis->run();

  /// terminate
  vis->closeApp();

  return 0;
}