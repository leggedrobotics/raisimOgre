//
// Created by Jemin Hwangbo on 3/3/19.
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

#include "raisim/RaisimServer.hpp"
#include <raisim/OgreVis.hpp>
#include "raisimBasicImguiPanel.hpp"
#include "deserializer.hpp"
#include "helper.hpp"


void setupCallback() {
  auto vis = raisim::OgreVis::get();

  /// light
  vis->getLight()->setDiffuseColour(1, 1, 1);
  vis->getLight()->setCastShadows(true);
  vis->getLightNode()->setPosition(3, 3, 3);

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
  vis->setContactVisObjectSize(0.03, 0.2);
  // speed of camera motion in freelook mode
  vis->getCameraMan()->setTopSpeed(5);

  /// skybox
  Ogre::Quaternion quat;
  quat.FromAngleAxis(Ogre::Radian(M_PI_2), {1, 0, 0});
  vis->getSceneManager()->setSkyBox(true,
                                    "Examples/StormySkyBox",
                                    500,
                                    true,
                                    quat,
                                    Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
}

int main (int argc, char **argv) {

  auto vis = raisim::OgreVis::get();

  /// these method must be called before initApp
  vis->setWindowSize(1800, 1200);
  vis->setSetUpCallback(setupCallback);
  vis->setAntiAliasing(1);
  vis->setRemoteMode(true);
  vis->setAntiAliasing(2);

  /// starts visualizer thread
  vis->initApp();

  /// connection
  std::string resDir(argv[0]);
  raisim::Deserializer deserializer(raisim::loadResource(""));

  vis->getCameraMan()->getCamera()->setPosition(0, -4, 5.5);
  vis->getCameraMan()->getCamera()->pitch(Ogre::Radian(1.));
  vis->setDesiredFPS(60);

  deserializer.estabilishConnection();

  RSFATAL_IF(!deserializer.init(), "Server is terminating")

  while(!vis->getRoot()->endRenderingQueued()) {
    if(deserializer.updatePosition()){
    } else {
      deserializer.closeSocket();
      deserializer.estabilishConnection();
      deserializer.init();
    }
    vis->renderOneFrame();
  }

  /// terminate
  vis->closeApp();

};
