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

  vis->addResourceDirectory(vis->getResourceDir() + "/material/skybox/violentdays");
  vis->loadMaterialFile("violentdays.material");

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
  quat.FromAngleAxis(Ogre::Radian(M_PI_2), {1., 0, 0});
  vis->getSceneManager()->setSkyBox(true,
                                    "skybox/violentdays",
                                    500,
                                    true,
                                    quat,
                                    Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
}

using namespace raisim;

int main(int argc, char **argv) {
  /// create raisim world
  World world;
  world.setTimeStep(0.002);

  auto vis = OgreVis::get();

  /// these method must be called before initApp
  vis->setWorld(&world);
  vis->setWindowSize(1800, 1200);
  vis->setImguiSetupCallback(imguiSetupCallback);
  vis->setImguiRenderCallback(imguiRenderCallBack);
  vis->setSetUpCallback(setupCallback);
  vis->setAntiAliasing(8);

  /// starts visualizer thread
  vis->initApp();

  /// create raisim objects
  auto ground = world.addGround();

  auto floatingJoint = Joint::getFloatingBaseJoint();

  Body base(1.0, Mat<3, 3>::getIdentity(), Vec<3>::getZeros());
  Body firstLink = base;

  Child root(base, floatingJoint, "root");

  Mat<3, 3> rot;
  rot.setIdentity();

  root.name = "test";

  CollisionBody box(Shape::Type::Box,
                    {1., 1., 1.},
                    {0., 0., 0.},
                    Mat<3, 3>::getIdentity(),
                    "box", "", "");

  VisObject visBox(Shape::Box,
                   {1., 1., 1.},
                   {0, 0, 0},
                   Mat<3, 3>::getIdentity(),
                   {1,1,1},
                   "vis_box",
                   "red");

  root.body.colObj.push_back(box);
  root.body.visObj.push_back(visBox);

  firstLink.colObj.push_back(box);
  firstLink.visObj.push_back(visBox);

  Joint revoluteJoint({1., 0., 0.},
                      {0., 2., 2.},
                      Mat<3, 3>::getIdentity(),
                      {0., 0.},
                      Joint::Type::REVOLUTE,
                      "revolute joint");

  Child firstChild(firstLink, revoluteJoint, "first_child");

  root.addChild(firstChild);

  auto* someRobot = world.addArticulatedSystem(root);

  Eigen::VectorXd gc(someRobot->getGeneralizedCoordinateDim());
  gc.setZero();
  gc.segment<7>(0) << 0, 0, 2, 1, 0, 0, 0;
  someRobot->setGeneralizedCoordinate(gc);

  /// create visualizer objects
  vis->createGraphicalObject(ground, 10, "floor", "checkerboard_green_transparent");
  auto robot_visual = vis->createGraphicalObject(someRobot, "someRobot");

  vis->select(robot_visual->at(0));
  vis->getCameraMan()->setYawPitchDist(Ogre::Radian(0), Ogre::Radian(-1.f), 10);

  /// run the app
  vis->run();

  /// terminate
  vis->closeApp();

  return 0;
}

