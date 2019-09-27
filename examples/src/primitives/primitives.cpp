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

  std::vector<raisim::Box*> cubes;
  std::vector<raisim::Sphere*> spheres;
  std::vector<raisim::Capsule*> capsules;
  std::vector<raisim::Cylinder*> cylinders;



  static const int N=6;

  for(size_t i=0; i<N; i++) {
    for(size_t j=0; j<N; j++) {
      for (size_t k = 0; k < N; k++) {
        std::string number = std::to_string(i) + std::to_string(j) + std::to_string(k);
        raisim::SingleBodyObject *ob = nullptr;
        switch ((i + j + k) % 4) {
          case 0:
            cubes.push_back(world.addBox(1, 1, 1, 1));
            vis->createGraphicalObject(cubes.back(), "cubes" + number, "red");
            ob = cubes.back();
            break;
          case 1:
            spheres.push_back(world.addSphere(0.5, 1));
            vis->createGraphicalObject(spheres.back(), "sphere" + number, "green");
            ob = spheres.back();
            break;
          case 2:
            capsules.push_back(world.addCapsule(0.5, 1., 1));
            vis->createGraphicalObject(capsules.back(), "capsules" + number, "blue");
            ob = capsules.back();
            break;
          case 3:
            cylinders.push_back(world.addCylinder(0.5, 0.5, 1));
            vis->createGraphicalObject(cylinders.back(), "cylinders" + number, "default");
            ob = cylinders.back();
            break;
        }
        ob->setPosition(-N + 2.*i, -N + 2.*j, N*2. + 2.*k);
      }
    }
  }



  /// create visualizer objects
  vis->createGraphicalObject(ground, 20, "floor", "default");

  /// set camera
  vis->getCameraMan()->getCamera()->setPosition(0,-N*3.5,N*1.5);
  vis->getCameraMan()->getCamera()->pitch(Ogre::Radian(1.2));

  /// run the app
  vis->run();

  /// terminate
  vis->closeApp();

  return 0;
}