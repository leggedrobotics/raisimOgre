/*-------------------------------------------------------------------------
Unless otherwise noted in the directory, this repository is licensed under

MIT License

Copyright (c) 2019-2019, Jemin Hwangbo, ETH Zurich

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
THE SOFTWARE. 
-------------------------------------------------------------------------*/

#include <raisim/OgreVis.hpp>
#include "raisimBasicImguiPanel.hpp"
#include "helper.hpp"
#include "RandomHeightMapGenerator.hpp"

void setupCallback() {
  auto vis = raisim::OgreVis::get();

  /// light
  vis->getLight()->setDiffuseColour(1, 1, 1);
  vis->getLight()->setCastShadows(true);
  Ogre::Vector3 lightdir(1., 1, -1);
  lightdir.normalise();
  vis->getLight()->setDirection(lightdir);
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
  vis->getRenderWindow()->getViewport(0)->setBackgroundColour(Ogre::ColourValue(1.,1.,1.));
}

int main(int argc, char **argv) {
  /// create raisim world
  raisim::World world;
  world.setTimeStep(0.002);
  auto vis = raisim::OgreVis::get();

  /// these method must be called before initApp
  vis->setWorld(&world);
  vis->setWindowSize(1200, 600);
  vis->setSetUpCallback(setupCallback);
  vis->setAntiAliasing(8);

  /// starts visualizer thread
  vis->initApp();

  double cameraAngle = 0.;
  raisim::HeightMap* heightMap = nullptr;
  raisim::RandomHeightMapGenerator hmGenerator;
  std::mt19937 gen(0);  


  for (int i=0; i< 5000; i++) {
    if (i % 500 == 0) {
      if(heightMap) {
        vis->remove(heightMap);
        world.removeObject(heightMap);
      }

      // loading heightmap from a png file
      heightMap = hmGenerator.generateTerrain(&world, raisim::RandomHeightMapGenerator::GroundType((i+100)/500), 1, false, gen);
      auto heightMapVis = vis->createGraphicalObject(heightMap, "floor", "default");
      vis->select(heightMapVis->at(0));
    }
    vis->getCameraMan()->setYawPitchDist(Ogre::Radian(i*0.03), Ogre::Radian(-1.), 12);      
    vis->renderOneFrame();
  }

  vis->closeApp();

  return 0;
}
