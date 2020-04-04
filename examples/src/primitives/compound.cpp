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
  vis->setContactVisObjectSize(0.1, 3.0);
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

  /// init
  vis->initApp();

  /// create raisim objects
  auto ground = world.addGround();
  std::vector<raisim::Compound::CompoundObjectChild> children;

  /// just to get random motions of anymal
  std::default_random_engine generator;
  std::normal_distribution<double> distribution(0.0, 0.6);
  std::srand(std::time(nullptr));

  for(int i=0; i<20; i++) {
    raisim::Compound::CompoundObjectChild child;
    child.objectType = raisim::ObjectType::CAPSULE;
    child.objectParam[0] = 0.1; // radius
    child.objectParam[1] = 0.1; // height (center-to-center distance)
    child.trans.pos[0] = distribution(generator);
    child.trans.pos[1] = distribution(generator);
    child.trans.pos[2] = distribution(generator);
    raisim::Vec<4> quat;
    quat[0] = distribution(generator);
    quat[1] = distribution(generator);
    quat[2] = distribution(generator);
    quat[3] = distribution(generator);
    quat /= quat.norm();
    raisim::quatToRotMat(quat, child.trans.rot);
    children.push_back(child);
  }

  auto compound = world.addCompound(children);

  /// create visualizer objects
  vis->createGraphicalObject(ground, 20, "floor", "default");
  vis->createGraphicalObject(compound, "compound");

  /// set camera
  vis->getCameraMan()->getCamera()->setPosition(0,-N*3.5,N*1.5);
  vis->getCameraMan()->getCamera()->pitch(Ogre::Radian(1.2));

  /// run the app
  vis->run();

  /// terminate
  vis->closeApp();

  return 0;
}