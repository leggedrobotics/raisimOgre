//
// Created by Jemin Hwangbo on 10/11/19.
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
#include "helper.hpp"

void setupCallback() {
  auto vis = raisim::OgreVis::get();

  /// light
  vis->getLight()->setDiffuseColour(1, 1, 1);
  vis->getLight()->setCastShadows(true);
  vis->getLightNode()->setPosition(3, 3, 3);

  /// load textures
  vis->addResourceDirectory(vis->getResourceDir() + "/material/checkerboard");
  vis->loadMaterialFile("checkerboard.material");

  /// shdow setting
  vis->getSceneManager()->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_ADDITIVE);
  vis->getSceneManager()->setShadowTextureSettings(2048, 3);

  /// scale related settings!! Please adapt it depending on your map size
  // beyond this distance, shadow disappears
  vis->getSceneManager()->setShadowFarDistance(10);
  // size of contact points and contact forces
  vis->setContactVisObjectSize(0.03, 0.2);
  // speed of camera motion in freelook mode
  vis->getCameraMan()->setTopSpeed(5);
}

int main(int argc, char **argv) {
  /// create raisim world
  raisim::World world;
  world.setTimeStep(0.002);

  auto vis = raisim::OgreVis::get();

  /// these method must be called before initApp
  vis->setWorld(&world);
  vis->setWindowSize(1800, 1200);
  vis->setImguiSetupCallback(imguiSetupCallback);
  vis->setImguiRenderCallback(imguiRenderCallBack);
  vis->setSetUpCallback(setupCallback);

  /// starts visualizer thread
  vis->initApp();

  /// create raisim objects
  auto ground = world.addGround();
  vis->createGraphicalObject(ground, 10, "floor", "checkerboard_green");

  vis->getCameraMan()->setStyle(raisim::CameraStyle::CS_FREELOOK);
  vis->getCameraMan()->getCamera()->setPosition(0, -5, 5);
  vis->getCameraMan()->getCamera()->setOrientation(1, 0, 0, 0);
  vis->getCameraMan()->getCamera()->pitch(Ogre::Radian(1.2));

  /// add visual objects
  vis->addVisualObject("random_arrow1", "arrowMesh", "red", {0.1, 0.1, 0.2}, false, raisim::OgreVis::RAISIM_OBJECT_GROUP);
  vis->addVisualObject("random_arrow2", "arrowMesh", "blue", {0.1, 0.1, 0.2}, false, raisim::OgreVis::RAISIM_OBJECT_GROUP);
  vis->addVisualObject("sphere1", "sphereMesh", "orange", {0.1, 0.1, 0.2}, false, raisim::OgreVis::RAISIM_OBJECT_GROUP);
  vis->addVisualObject("cylinder1", "cylinderMesh", "yellow", {0.1, 0.1, 0.2}, false, raisim::OgreVis::RAISIM_OBJECT_GROUP);

  auto& list = raisim::OgreVis::get()->getVisualObjectList();
  raisim::Mat<3,3> rot1, rot2;
  raisim::Vec<3> arrowDirection1, arrowDirection2;
  arrowDirection1 = {1,1,1};
  arrowDirection1 /= arrowDirection1.norm();

  arrowDirection2 = {1,1,-1};
  arrowDirection2 /= arrowDirection1.norm();

  raisim::zaxisToRotMat(arrowDirection1, rot1);
  raisim::zaxisToRotMat(arrowDirection2, rot2);

  list["random_arrow1"].offset = {0,0,3};
  list["random_arrow1"].scale = {1,1,3};
  list["random_arrow1"].rotationOffset = rot1;

  list["random_arrow2"].offset = {0,0,3};
  list["random_arrow2"].scale = {0.5,0.5,0.5};
  list["random_arrow2"].rotationOffset = rot2;

  list["sphere1"].offset = {0,2,3};

  list["cylinder1"].offset = {1,2,3};

  /// run the app
  vis->run();

  /// terminate
  vis->closeApp();

  return 0;
}
