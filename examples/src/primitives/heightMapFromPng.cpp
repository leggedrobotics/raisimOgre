/*-------------------------------------------------------------------------
This source file is a part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/


Copyright (c) 2000-2013 Torus Knot Software Ltd
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:


The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.


THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE
-------------------------------------------------------------------------*/

#include <raisim/OgreVis.hpp>
#include "raisimBasicImguiPanel.hpp"
#include "helper.hpp"

void setupCallback() {
  auto vis = raisim::OgreVis::get();

  /// light
  vis->getLight()->setDiffuseColour(1, 1, 1);
  vis->getLight()->setCastShadows(true);
  Ogre::Vector3 lightdir(1., 1, -1);
  lightdir.normalise();
  vis->getLight()->setDirection(lightdir);

  vis->setCameraSpeed(300);
  vis->setContactVisObjectSize(0.04, 0.3);

  /// shadow setting
  vis->getSceneManager()->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_ADDITIVE);
  vis->getSceneManager()->setShadowTextureSettings(2048, 3);

  // beyond this distance, shadow disappears
  vis->getSceneManager()->setShadowFarDistance(10);
  // size of contact points and contact forces
  vis->setContactVisObjectSize(0.025, 0.4);
  // speed of camera motion in freelook mode
  vis->getCameraMan()->setTopSpeed(5);

  /// skybox
  Ogre::Quaternion quat;
  quat.FromAngleAxis(Ogre::Radian(M_PI_2), {1., 0., 0.});
  vis->getSceneManager()->setSkyBox(true,
                                    "Examples/StormySkyBox",
                                    500,
                                    true,
                                    quat,
                                    Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
}

int main(int argc, char **argv) {
  /// create raisim world
  raisim::World world;
  world.setTimeStep(0.002);
  world.setERP(0.003, 0.);
  auto vis = raisim::OgreVis::get();

  /// these method must be called before initApp
  vis->setWorld(&world);
  vis->setWindowSize(1200, 600);
  vis->setImguiSetupCallback(imguiSetupCallback);
  vis->setImguiRenderCallback(imguiRenderCallBack);
  vis->setSetUpCallback(setupCallback);
  vis->setAntiAliasing(8);

  /// starts visualizer thread
  vis->initApp();

  /// create raisim objects
  auto sphere1 = world.addSphere(0.1, 10);
  auto sphere2 = world.addSphere(0.1, 10);

  // the floating base orientation is initialized to identity and all joints and positions are initialized to 1
  auto anymal = world.addArticulatedSystem(raisim::loadResource("anymal/anymal.urdf"));

  // loading heightmap from a png file
  auto heightMap = world.addHeightMap(raisim::loadResource("heightMap/zurichHeightMap.png"), 0, 0, 100, 100, 0.0005, -10);

  vis->createGraphicalObject(sphere1, "sphere1", "gravel");
  vis->createGraphicalObject(sphere2, "sphere2", "default");
  vis->createGraphicalObject(heightMap, "floor", "default");
  auto* anymal_visual = vis->createGraphicalObject(anymal, "anymal");

  sphere1->setPosition(0, 0, 5);
  sphere2->setPosition(0.5, 0, 3);

  /// set camera
  vis->select(anymal_visual->at(0));
  vis->getCameraMan()->setYawPitchDist(Ogre::Radian(0.), Ogre::Radian(-1.), 3);

  /// run the app
  vis->run();

  /// terminate
  vis->closeApp();

  return 0;
}
