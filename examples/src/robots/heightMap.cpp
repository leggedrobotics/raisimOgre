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
  vis->setContactVisObjectSize(0.04, 0.3);

  /// load  textures
  vis->addResourceDirectory(vis->getResourceDir() + "/material/gravel");
  vis->loadMaterialFile("gravel.material");

  /// shadow setting
  vis->getSceneManager()->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_ADDITIVE);
  vis->getSceneManager()->setShadowTextureSettings(2048, 3);

  // beyond this distance, shadow disappears
  vis->getSceneManager()->setShadowFarDistance(10);
  // size of contact points and contact forces
  vis->setContactVisObjectSize(0.01, 0.4);
  // speed of camera motion in freelook mode
  vis->getCameraMan()->setTopSpeed(5);
}

int main(int argc, char **argv) {
  /// create raisim world
  raisim::World world;
  world.setTimeStep(0.0001);
  auto vis = raisim::OgreVis::get();
  world.setERP(0,0);

  /// these method must be called before initApp
  vis->setWorld(&world);
  vis->setWindowSize(1800, 1200);
  vis->setImguiSetupCallback(imguiSetupCallback);
  vis->setImguiRenderCallback(imguiRenderCallBack);
  vis->setSetUpCallback(setupCallback);
  vis->setAntiAliasing(8);
  raisim::gui::manualStepping = true;

  /// starts visualizer thread
  vis->initApp();

  /// create raisim objects
  auto sphere1 = world.addSphere(0.1, 1);
  auto sphere2 = world.addSphere(0.1, 1);
  auto anymal = world.addArticulatedSystem(raisim::loadResource("anymal/anymal.urdf"));
  sphere1->setPosition(0, 0, 5);
  sphere2->setPosition(0.5, 0, 3);

  /// create heightmap
  raisim::TerrainProperties terrainProperties;
  terrainProperties.frequency = 0.2;
  terrainProperties.zScale = 3.0;
  terrainProperties.xSize = 20.0;
  terrainProperties.ySize = 20.0;
  terrainProperties.xSamples = 50;
  terrainProperties.ySamples = 50;
  terrainProperties.fractalOctaves = 3;
  terrainProperties.fractalLacunarity = 2.0;
  terrainProperties.fractalGain = 0.25;

  /// comment one of the following two heightmap methods

  /// using terrain properties
  auto hm = world.addHeightMap(0.0, 0.0, terrainProperties);

  /// using raisim text file
//  auto hm = world.addHeightMap(raisim::loadResource("heightMap/heightMapExample.txt"), 0, 0);

  /// create visualizer objects
  vis->createGraphicalObject(hm, "terrain", "default");
  vis->createGraphicalObject(sphere1, "sphere1", "gravel");
  vis->createGraphicalObject(sphere2, "sphere2", "default");

  /// ANYmal joint PD controller
  Eigen::VectorXd jointNominalConfig(19), jointVelocityTarget(18);
  Eigen::VectorXd jointState(18), jointVel(18), jointForce(18), jointPgain(18), jointDgain(18);
  jointPgain.setZero();
  jointDgain.setZero();
  jointVelocityTarget.setZero();
  jointPgain.tail(12).setConstant(200.0);
  jointDgain.tail(12).setConstant(10.0);

  jointNominalConfig << 0, 0, 0, 0, 0, 0, 0, 0.03, 0.4, -0.8, -0.03, 0.4, -0.8, 0.03, -0.4, 0.8, -0.03, -0.4, 0.8;
  jointVel << 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0;

  auto anymal_graphics = vis->createGraphicalObject(anymal, "ANYmal");

  anymal->setGeneralizedCoordinate({0, 0, 3.54, 1.0, 0.0, 0.0, 0.0, 0.03, 0.4,
                                    -0.8, -0.03, 0.4, -0.8, 0.03, -0.4, 0.8, -0.03, -0.4, 0.8});
  Eigen::Vector4d quat; quat <<1., 0., 0., 0.;
  hm->setPosition(0.1, 0.2, -0.3);

  vis->select(anymal_graphics->at(0), false);
  vis->getCameraMan()->setYawPitchDist(Ogre::Radian(0), -Ogre::Radian(M_PI_4), 2);
  anymal->setGeneralizedForce(Eigen::VectorXd::Zero(anymal->getDOF()));
  anymal->setControlMode(raisim::ControlMode::PD_PLUS_FEEDFORWARD_TORQUE);
  anymal->setPdGains(jointPgain, jointDgain);
  anymal->setPdTarget(jointNominalConfig, jointVelocityTarget);

  /// run the app
  vis->run();

  /// terminate
  vis->closeApp();

  return 0;
}
