//
// Created by jhwangbo on 17.01.19.
//

#include <raisim/misc.hpp>
#include "raisim/OgreVis.hpp"
#include <chrono>
#include "OgreTangentSpaceCalc.h"

namespace raisim {

OgreVis::~OgreVis() {
  if (videoThread_ ) videoThread_->join();
}

bool OgreVis::mouseMoved(const MouseMotionEvent &evt) {
  mouseX_ = evt.x;
  mouseY_ = evt.y;

  if (Ogre::ImguiManager::getSingleton().mouseMoved(evt)) return true;

  if ((cameraMan_->getStyle() == CameraStyle::CS_FREELOOK && rightMouseButtonPressed_)
      || (cameraMan_->getStyle() == CameraStyle::CS_ORBIT && leftMouseButtonPressed_))
    if (cameraMan_->mouseMoved(evt, leftShiftButtonPressed_))
      return true;

  return true;
}

void OgreVis::addResourceDirectory(const std::string &dir) {
  Ogre::ResourceGroupManager::getSingleton().addResourceLocation(dir,
                                                                 "FileSystem",
                                                                 Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                                 false,
                                                                 true);
  // Initialise the resource groups:
  Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}

void OgreVis::loadMaterialFile(const std::string &filename) {
  Ogre::DataStreamPtr ds = Ogre::ResourceGroupManager::getSingleton().openResource(filename,
                                                                                   Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
  Ogre::MaterialManager::getSingleton().parseScript(ds, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
}

void OgreVis::loadMeshFile(const std::string &file, const std::string &meshName, bool fromMemory) {
  AssimpLoader::AssOptions opts;
  if (meshUsageCount_.find(meshName) != meshUsageCount_.end())
    meshUsageCount_[meshName]++;
  else
    meshUsageCount_[meshName] = 1;

  if (Ogre::MeshManager::getSingleton().getByName(meshName)) return;

  if (!fromMemory) {

    RSFATAL_IF(!fileExists(file), "File " + file + " not found.")

    std::string meshFilename;
    std::string extension = file.substr(file.find_last_of('.') + 1);
    std::string baseFilename = file.substr(file.find_last_of(separator()) + 1,
                                           file.size() - file.find_last_of(separator()) - extension.size() - 2);
    std::string path = raisim::getPathName(file);
    meshFilename = path + separator() + baseFilename + "_" + extension + ".mesh";
    if (extension == "mesh")
      return;

    opts.source = file;

    if (extension == "dae") {
      std::ifstream fstream;
      fstream.open(file);
      std::string cadFileString((std::istreambuf_iterator<char>(fstream)),
                                std::istreambuf_iterator<char>());

      auto itr = cadFileString.find("<up_axis>Z_UP</up_axis>");
      if (itr != std::string::npos)
        opts.flipYZ = true;
    }
  } else {
    opts.useMemory = true;
    opts.source = file;
  }

  opts.quietMode = true;
  opts.logFile = "ass.log";
  opts.customAnimationName = "";
  opts.dest = "";
  opts.animationSpeedModifier = 1.0;
  opts.lodValue = 250000;
  opts.lodFixed = 0;
  opts.lodPercent = 20;
  opts.numLods = 0;
  opts.usePercent = true;
  opts.params = AssimpLoader::LP_GENERATE_SINGLE_MESH | AssimpLoader::LP_QUIET_MODE;

  AssimpLoader loader;
  if (!loader.convert(opts, meshName))
    OGRE_EXCEPT(Ogre::Exception::ERR_FILE_NOT_FOUND, "file conversion failed", "Load mesh file" + opts.source);
}

bool OgreVis::mouseWheelRolled(const MouseWheelEvent &evt) {
  if (Ogre::ImguiManager::getSingleton().mouseWheelRolled(evt)) return true;
  if (cameraMan_->mouseWheelRolled(evt)) return true;

  return true;
}

bool OgreVis::mousePressed(const MouseButtonEvent &evt) {
  if (Ogre::ImguiManager::getSingleton().mousePressed(evt)) return true;

  switch (evt.button) {
    case BUTTON_LEFT:leftMouseButtonPressed_ = true;
      if (hovered_) {
        if (selected_ == hovered_) break;
        if (selected_) {
          selected_ = nullptr;
        }
        selected_ = hovered_;
        auto *newSelEn = dynamic_cast<Ogre::Entity *>(selected_->getAttachedObject(0));
        cameraMan_->setStyle(CS_ORBIT);
        cameraMan_->setTarget(selected_);
      } else {
      }
      break;
    case BUTTON_RIGHT:
      deselect();
      rightMouseButtonPressed_ = true;
      break;
    default:break;
  }
  if (cameraMan_->mousePressed(evt)) return true;

  return true;
}

bool OgreVis::mouseReleased(const MouseButtonEvent &evt) {
  if (Ogre::ImguiManager::getSingleton().mouseReleased(evt)) return true;

  switch (evt.button) {
    case BUTTON_LEFT:leftMouseButtonPressed_ = false;
      break;
    case BUTTON_RIGHT:rightMouseButtonPressed_ = false;
      break;
    default:break;
  }

  if (cameraMan_->mouseReleased(evt)) return true;
  return true;
}

bool OgreVis::frameRenderingQueued(const Ogre::FrameEvent &evt) {
  cameraMan_->frameRendered(evt);   // if dialog isn't up, then update the camera
  /// ray tracing for item selection
  Ogre::Vector3 camPos = mainCamera_->getRealPosition();
  Ogre::Ray cameraRay = mainCamera_->getCameraToViewportRay(mouseX_ / float(getRenderWindow()->getWidth()),
                                                            mouseY_ / float(getRenderWindow()->getHeight()));
  raySceneQuery_->setRay(cameraRay);
  raySceneQuery_->setSortByDistance(true);
  Ogre::RaySceneQueryResult &result = raySceneQuery_->execute();
  auto iter = result.begin();
  hovered_ = nullptr;

  while (iter != result.end()) {
    hovered_ = iter->movable->getParentSceneNode();
    if (getGraphicObject(hovered_) == nullptr)
      hovered_ = nullptr;
    else if (!getGraphicObject(hovered_)->selectable_ || !(getGraphicObject(hovered_)->group & mask_))
      hovered_ = nullptr;

    if (hovered_) break;
    iter++;
  }
  return true;
}

void OgreVis::videoThread() {
  imageCounter = 0;
  auto w = getRenderWindow()->getViewport(0)->getActualWidth();
  auto h = getRenderWindow()->getViewport(0)->getActualHeight();
  std::string command =
      "ffmpeg -r " + std::to_string(desiredFPS_) + " -f rawvideo -pix_fmt rgb24 -s " + std::to_string(w) + "x"
          + std::to_string(h) +
          " -i - -threads 0 -preset fast -y -crf 21 " + currentVideoFile_;
  const char *cmd = command.c_str();
  ffmpeg = popen(cmd, "w");
  RSFATAL_IF(!ffmpeg, "a pipe cannot be initiated for video recording. Maybe missing ffmpeg?")

  Ogre::PixelFormat pf = getRenderWindow()->suggestPixelFormat();
  videoBuffer_.reset(OGRE_ALLOC_T(Ogre::uchar, w * h * Ogre::PixelUtil::getNumElemBytes(pf), MEMCATEGORY_RENDERSYS));
  videoPixelBox_ = std::make_unique<Ogre::PixelBox>(w, h, 1, pf, videoBuffer_.get());
  videoInitMutex_.unlock();

  while (true) {
    if (stopVideoRecording_) {
      fflush(ffmpeg);
      pclose(ffmpeg);
      imageCounter = 0;
      break;
    } else if (newFrameAvailable_) {
      std::lock_guard<std::mutex> lock(videoFrameMutext_);
      newFrameAvailable_ = false;
      fwrite(videoPixelBox_->data, Ogre::PixelUtil::getNumElemBytes(pf) * w * h, 1, ffmpeg);
    } else {
      usleep(1e4);
    }
  }

  isVideoRecording_ = false;
  stopVideoRecording_ = false;
  newFrameAvailable_ = false;
}

void OgreVis::frameRendered(const Ogre::FrameEvent &evt) {
  if (isVideoRecording_) return;
  Ogre::ImguiManager::getSingleton().frameRendered(evt);
  cameraMan_->frameRendered(evt);
}

bool OgreVis::frameStarted(const Ogre::FrameEvent &evt) {
  ApplicationContext::frameStarted(evt);
  Ogre::ImguiManager::getSingleton().newFrame(
      evt.timeSinceLastFrame,
      Ogre::Rect(0, 0, getRenderWindow()->getWidth(), getRenderWindow()->getHeight()));
  if (imGuiRenderCallback_) imGuiRenderCallback_();

  /// video recording
  if (!isVideoRecording_ && initiateVideoRecording_) {
    videoInitMutex_.lock();
    isVideoRecording_ = true;
    initiateVideoRecording_ = false;
    if (videoThread_ && videoThread_->joinable()) videoThread_->join();
    videoThread_ = std::make_unique<std::thread>(&OgreVis::videoThread, this);
  }

  return true;
}

bool OgreVis::frameEnded(const Ogre::FrameEvent &evt) {
  if (isVideoRecording_ && stopVideoRecording_) {
    videoThread_->join();
  } else if (isVideoRecording_ && !stopVideoRecording_) {

    /// wait until the video thread processes the previous frame
    while (newFrameAvailable_)
      usleep(1e4);

    std::lock_guard<std::mutex> lock(videoFrameMutext_);
    std::lock_guard<std::mutex> lock2(videoInitMutex_);

    imageCounter++;

//    Ogre::uchar *data = OGRE_ALLOC_T(Ogre::uchar, w * h * Ogre::PixelUtil::getNumElemBytes(pf), MEMCATEGORY_RENDERSYS);
    getRenderWindow()->copyContentsToMemory(*videoPixelBox_, *videoPixelBox_);
//    OGRE_FREE(data, MEMCATEGORY_RENDERSYS);
    newFrameAvailable_ = true;
//    if(imageCounter > imageBufferSize_)
//      stopVideoRecording_ = true;
  }

  // reset the visibility of the objects
  for (auto &ele : objectSet_.set)
    for (auto &grp : ele.second)
      grp.graphics->setVisible(bool(mask_ & grp.group));

  // reset the visibility of the objects
  for (auto &ele : visObject_)
    ele.second.graphics->setVisible(bool(mask_ & ele.second.group));

  return true;
}

bool OgreVis::keyPressed(const KeyboardEvent &evt) {
  auto &key = evt.keysym.sym;
  // termination gets the highest priority
  switch (key) {
    case OgreBites::SDLK_ESCAPE:
      getRoot()->queueEndRendering();
      break;
    case OgreBites::SDLK_LSHIFT:
      leftShiftButtonPressed_ = true;
      break;
    case OgreBites::SDLK_SPACE:
      paused_ = !paused_;
      break;
    default:break;
  }

  if (keyboardCallback_)
    if (keyboardCallback_(evt))
      return true;

  if (!leftShiftButtonPressed_)
    if (cameraMan_->keyPressed(evt))
      return true;

  if (Ogre::ImguiManager::getSingleton().keyPressed(evt)) return true;

  return true;
}

bool OgreVis::keyReleased(const KeyboardEvent &evt) {
  if (Ogre::ImguiManager::getSingleton().keyReleased(evt)) return true;
  auto &key = evt.keysym.sym;

  switch (key) {
    case OgreBites::SDLK_LSHIFT:
      leftShiftButtonPressed_ = false;
    default:
      break;
  }

  if (cameraMan_->keyReleased(evt)) return true;
  return true;
}

void OgreVis::shutdown() {
  delete cameraMan_;
}

void OgreVis::setup() {
  // do not forget to call the base first
  mRoot->initialise(false);
  std::map<std::string, std::string> param;
  param["FSAA"] = std::to_string(fsaa_);
  param["vsync"] = "true";

  windowPair_ = createWindow(mAppName, initialWindowSizeX_, initialWindowSizeY_, param);

  locateResources();
  initialiseRTShaderSystem();
  loadResources();

  // adds context as listener to process context-level (above the sample level) events
  mRoot->addFrameListener(this);
  addInputListener(this);

  // imgui
  Ogre::ImguiManager::createSingleton();
  addInputListener(Ogre::ImguiManager::getSingletonPtr());
  if (imGuiSetupCallback_) imGuiSetupCallback_();

  // get a pointer to the already created root
  Ogre::Root *root = getRoot();
  scnMgr_ = root->createSceneManager();
  Ogre::ImguiManager::getSingleton().init(scnMgr_);
  scnMgr_->addRenderQueueListener(Ogre::ImguiManager::getSingletonPtr());

  // register our scene with the RTSS
  Ogre::RTShader::ShaderGenerator *shadergen = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
  shadergen->addSceneManager(scnMgr_);

  // -- tutorial section start --
  scnMgr_->setAmbientLight(Ogre::ColourValue(0.3, 0.3, 0.3));
  raySceneQuery_ = scnMgr_->createRayQuery(Ogre::Ray());

  light_ = scnMgr_->createLight("mainLight");

  Ogre::Vector3 lightdir(1., 0, -2);
  lightdir.normalise();

  light_->setType(Ogre::Light::LT_DIRECTIONAL);
  light_->setDirection(lightdir);
  light_->setDiffuseColour(Ogre::ColourValue(0.3f, 0.3f, 0.3f));
  light_->setSpecularColour(Ogre::ColourValue(0.3f, 0.3f, 0.3f));
  light_->setCastShadows(true);
  lightNode_ = scnMgr_->getRootSceneNode()->createChildSceneNode();
  lightNode_->attachObject(light_);

  scnMgr_->setShadowFarDistance(20); // Try it with different values, as that can also cause shadows to fade out
  scnMgr_->setShadowDirLightTextureOffset(0);
  auto *camSetup = new Ogre::FocusedShadowCameraSetup();
  scnMgr_->setShadowCameraSetup(Ogre::ShadowCameraSetupPtr(camSetup));

  // create the camera
  mainCamera_ = scnMgr_->createCamera("myCam");
  mainCamera_->setNearClipDistance(0.1);
  mainCamera_->setAutoAspectRatio(true);
  mainCamera_->setFarClipDistance(1000);
  mainCamera_->setFocalLength(5);
  camNode_ = scnMgr_->getRootSceneNode()->createChildSceneNode();
  camNode_->attachObject(mainCamera_);
  camNode_->setPosition(10, 10, 10);
  camNode_->setOrientation(1, 0, 0, 0);
  cameraMan_ = new CameraMan(camNode_);   // create a default camera controller
  cameraMan_->setStyle(CS_FREELOOK);

  // and tell it to render into the main window
  viewport_ = getRenderWindow()->addViewport(mainCamera_);

  /// add default resource directory
  addResourceDirectory(resourceDir_);

  /// add what meshes to be used
  std::string sphereFile = raisim::OgreVis::getResourceDir() + "/model/primitives/sphere.obj";
  std::string cubeFile = raisim::OgreVis::getResourceDir() + "/model/primitives/cube.obj";
  std::string cylinderFile = raisim::OgreVis::getResourceDir() + "/model/primitives/cylinder.obj";
  std::string planeFile = raisim::OgreVis::getResourceDir() + "/model/primitives/plane.obj";
  std::string capsuleFile = raisim::OgreVis::getResourceDir() + "/model/primitives/capsule.obj";
  std::string arrowFile = raisim::OgreVis::getResourceDir() + "/model/primitives/arrow.obj";

  raisim::OgreVis::loadMeshFile(sphereFile, "sphereMesh");
  raisim::OgreVis::loadMeshFile(cubeFile, "cubeMesh");
  raisim::OgreVis::loadMeshFile(cylinderFile, "cylinderMesh");
  raisim::OgreVis::loadMeshFile(planeFile, "planeMesh");
  raisim::OgreVis::loadMeshFile(capsuleFile, "capsuleMesh");
  raisim::OgreVis::loadMeshFile(arrowFile, "arrowMesh");

  primitiveMeshNames_.insert("sphereMesh");
  primitiveMeshNames_.insert("cubeMesh");
  primitiveMeshNames_.insert("cylinderMesh");
  primitiveMeshNames_.insert("planeMesh");
  primitiveMeshNames_.insert("capsuleMesh");
  primitiveMeshNames_.insert("arrowMesh");

  raisim::OgreVis::addResourceDirectory(raisim::OgreVis::getResourceDir() + "/material/selection");
  raisim::OgreVis::loadMaterialFile("selection.material");

  raisim::OgreVis::addResourceDirectory(raisim::OgreVis::getResourceDir() + "/material/default");
  raisim::OgreVis::loadMaterialFile("default.material");

  if (setUpCallback_) setUpCallback_();
}

std::vector<GraphicObject> *OgreVis::registerSet(const std::string &name,
                                                 raisim::Object *ob,
                                                 std::vector<GraphicObject> &&graphics) {
  // check if it exists. if not create a new one
  objectSet_.insert(name, ob, std::move(graphics));
  return objectSet_[ob].first;
}

void OgreVis::sync() {
  objectSet_.sync();
}

void OgreVis::remove(raisim::Object *ob) {
  auto set = objectSet_[ob];

  for (auto &go : *set.first) {
    if (primitiveMeshNames_.find(go.meshName) == primitiveMeshNames_.end() && meshUsageCount_[go.meshName] == 1) {
      Ogre::MeshManager::getSingleton().unload(go.meshName);
      Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().getByName(go.meshName);
      Ogre::MeshManager::getSingleton().remove(mesh);
      meshUsageCount_[go.meshName]--;
    }
    this->getSceneManager()->destroyEntity(go.name);
    this->getSceneManager()->getRootSceneNode()->removeAndDestroyChild(go.graphics);
  }

  objectSet_.erase(ob);
}

void OgreVis::remove(const std::string &name) {
  remove(objectSet_.ref[name]);
}

void OgreVis::run() {
  RSFATAL_IF(!world_, "world is not set. use remoteRun() if you intend to visualize remotely")

  double time = 0;

  /// inverse of visualization frequency
  double visTime = 1. / desiredFPS_;

  while (!getRoot()->endRenderingQueued()) {
    while (time < 0 && (!(takeNSteps_ == 0 && paused_) || !paused_)) {

      /// control and simulation
      if (controlCallback_) controlCallback_();
      world_->integrate1();
      world_->integrate2();
      time += world_->getTimeStep();

      /// count the allowed number of sim steps
      if (takeNSteps_ > 0) takeNSteps_--;
    }

    /// compute how much sim is ahead
    if (time > -visTime * realTimeFactor_ * .1)
      time -= visTime * realTimeFactor_;

    /// do actual rendering
    renderOneFrame();
  }
}

void OgreVis::remoteRun() {
  while (!getRoot()->endRenderingQueued()) {
    auto start = std::chrono::system_clock::now();
    renderOneFrame();
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    auto diff = 1. / desiredFPS_ - elapsed.count();
    if (diff > 0) MSLEEP(diff * 1000.);
  }
}

GraphicObject OgreVis::createSingleGraphicalObject(const std::string &name,
                                                   const std::string &meshName,
                                                   const std::string &material,
                                                   const raisim::Vec<3> &scale,
                                                   const Vec<3> &offset,
                                                   const Mat<3, 3> &rot,
                                                   size_t localIdx,
                                                   bool castShadow,
                                                   bool selectable,
                                                   unsigned long int group) {
  auto *ent = raisim::OgreVis::getSceneManager()->createEntity(name,
                                                               meshName,
                                                               Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
  ent->setCastShadows(castShadow);
  if (!material.empty())
    ent->setMaterialName(material);
  /// hack to check if texture coordinates exist
  GraphicObject obj;
  obj.graphics = this->getSceneManager()->getRootSceneNode()->createChildSceneNode(name);
  obj.graphics->attachObject(ent);
  obj.scale = scale;
  obj.graphics->scale(float(obj.scale[0]), float(obj.scale[1]), float(obj.scale[2]));
  obj.selectable_ = selectable;
  obj.localId = localIdx;
  obj.offset = offset;
  obj.group = group;
  obj.rotationOffset = rot;
  obj.name = name;
  obj.meshName = meshName;

  return obj;
}

void OgreVis::addVisualObject(const std::string &name,
                              const std::string &meshName,
                              const std::string &material,
                              const raisim::Vec<3> &scale,
                              bool castShadow,
                              unsigned long int group) {
  auto *ent = raisim::OgreVis::getSceneManager()->createEntity(name,
                                                               meshName,
                                                               Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

  visObject_[name] = VisualObject();

  ent->setCastShadows(castShadow);

  if (!material.empty())
    ent->setMaterialName(material);
  /// hack to check if texture coordinates exist
  VisualObject &obj = visObject_[name];
  obj.graphics = getSceneManager()->getRootSceneNode()->createChildSceneNode(name);
  obj.graphics->attachObject(ent);
  obj.scale = scale;
  obj.graphics->scale(float(obj.scale[0]), float(obj.scale[1]), float(obj.scale[2]));
  obj.group = group;
  obj.name = name;
}

void OgreVis::clearVisualObject() {
  for (auto &vo: visObject_) {
    Ogre::Entity *ent = dynamic_cast<Ogre::Entity *>(vo.second.graphics->getAttachedObject(0));
    vo.second.graphics->detachObject((short unsigned int) 0);
    scnMgr_->destroyEntity(ent);
    scnMgr_->destroySceneNode(vo.second.graphics);
  }
  visObject_.clear();
}

std::vector<GraphicObject> *OgreVis::createGraphicalObject(raisim::Sphere *sphere,
                                                           const std::string &name,
                                                           const std::string &material) {
  auto rad = sphere->getRadius();
  raisim::Mat<3, 3> rot;
  rot.setIdentity();
  sphere->setName(name);
  auto graphicalObj = createSingleGraphicalObject(name, "sphereMesh", material, {rad, rad, rad}, {0, 0, 0}, rot, 0);
  return registerSet(name, sphere, {graphicalObj});
}

std::vector<GraphicObject> *OgreVis::createGraphicalObject(raisim::Ground *ground,
                                                           double planeDim,
                                                           const std::string &name,
                                                           const std::string &material) {
  raisim::Mat<3, 3> rot;
  rot.setIdentity();
  ground->setName(name);
  return registerSet(name, ground, {createSingleGraphicalObject(name,
                                                                "planeMesh",
                                                                material,
                                                                {planeDim, planeDim, planeDim},
                                                                {0, 0, ground->getHeight()},
                                                                rot,
                                                                1 << 0,
                                                                true,
                                                                false,
                                                                RAISIM_OBJECT_GROUP | RAISIM_COLLISION_BODY_GROUP)});
}

std::vector<GraphicObject> *OgreVis::createGraphicalObject(raisim::Box *box,
                                                           const std::string &name,
                                                           const std::string &material) {
  raisim::Mat<3, 3> rot;
  rot.setIdentity();
  box->setName(name);
  return registerSet(name,
                     box,
                     {createSingleGraphicalObject(name, "cubeMesh", material, box->getDim(), {0, 0, 0}, rot, 0)});
}

std::vector<GraphicObject> *OgreVis::createGraphicalObject(raisim::Cylinder *capsule,
                                                           const std::string &name,
                                                           const std::string &material) {
  auto rad = capsule->getRadius();
  auto h = capsule->getHeight();
  raisim::Mat<3, 3> rot;
  rot.setIdentity();
  capsule->setName(name);
  return registerSet(name,
                     capsule,
                     {createSingleGraphicalObject(name, "cylinderMesh", material, {rad, rad, h}, {0, 0, 0}, rot, 0)});
}

raisim::VisualObject *OgreVis::createGraphicalObject(raisim::Wire *wire,
                                                     const std::string &name,
                                                     const std::string &material) {
  wire->setName(name);

  auto *ent = raisim::OgreVis::getSceneManager()->createEntity(name,
                                                               "cylinderMesh",
                                                               Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

  wires_[name] = VisualObject();

  ent->setCastShadows(true);

  if (!material.empty())
    ent->setMaterialName(material);
  /// hack to check if texture coordinates exist
  VisualObject &obj = wires_[name];
  obj.graphics = getSceneManager()->getRootSceneNode()->createChildSceneNode(name);
  obj.graphics->attachObject(ent);
  obj.scale = {1., 1., 1.};
  obj.graphics->scale(float(obj.scale[0]), float(obj.scale[1]), float(obj.scale[2]));
  obj.group = RAISIM_OBJECT_GROUP | RAISIM_COLLISION_BODY_GROUP;
  obj.name = name;

  return &wires_[name];
}

std::vector<GraphicObject> *OgreVis::createGraphicalObject(raisim::Capsule *capsule,
                                                           const std::string &name,
                                                           const std::string &material) {
  auto rad = capsule->getRadius();
  auto h = capsule->getHeight();
  capsule->setName(name);
  raisim::Mat<3, 3> rot;
  rot.setIdentity();
  return registerSet(name,
                     capsule,
                     {createSingleGraphicalObject(name + "_cyl",
                                                  "cylinderMesh",
                                                  material,
                                                  {rad, rad, h},
                                                  {0, 0, 0},
                                                  rot,
                                                  0),
                      createSingleGraphicalObject(name + "_sph1",
                                                  "sphereMesh",
                                                  material,
                                                  {rad, rad, rad},
                                                  {0, 0, 0.5},
                                                  rot,
                                                  0),
                      createSingleGraphicalObject(name + "_sph2",
                                                  "sphereMesh",
                                                  material,
                                                  {rad, rad, rad},
                                                  {0, 0, -0.5},
                                                  rot,
                                                  0)});
}

raisim::SimAndGraphicsObjectPool &OgreVis::getObjectSet() {
  return objectSet_;
}

std::vector<GraphicObject> *OgreVis::createGraphicalObject(raisim::ArticulatedSystem *as,
                                                           const std::string &name) {
  std::vector<GraphicObject> graphics;

  int itemId = 0;

  for (auto &vo: as->getVisOb())
    registerRaisimGraphicalObjects(vo, graphics, as, name + "_" + std::to_string(itemId++), RAISIM_OBJECT_GROUP);

  itemId = 0;

  for (auto &vo: as->getVisColOb())
    registerRaisimGraphicalObjects(vo,
                                   graphics,
                                   as,
                                   name + "_collisionBody_" + std::to_string(itemId++),
                                   RAISIM_COLLISION_BODY_GROUP);

  return registerSet(name, as, std::move(graphics));
}

void OgreVis::registerRaisimGraphicalObjects(raisim::VisObject &vo,
                                             std::vector<GraphicObject> &graphics,
                                             raisim::ArticulatedSystem *as,
                                             const std::string &name,
                                             unsigned long int group) {
  if (vo.shape == raisim::Shape::Mesh) {
    std::string fullFilePath = as->getResourceDir() + separator() + vo.fileName;
    if (!Ogre::MeshManager::getSingleton().getByName(fullFilePath))
      raisim::OgreVis::loadMeshFile(fullFilePath, fullFilePath);
    auto visname = name + "_" + as->getBodyNames()[vo.localIdx] + "_" + getBaseFileName(vo.fileName);
    graphics.push_back(createSingleGraphicalObject(visname,
                                                   fullFilePath,
                                                   "",
                                                   vo.scale,
                                                   vo.offset,
                                                   vo.rot,
                                                   vo.localIdx,
                                                   true,
                                                   true,
                                                   group));
  } else {
    auto visname = name + "_" + as->getBodyNames()[vo.localIdx] + "_";
    raisim::Vec<3> dim;
    std::string meshName;
    switch (vo.shape) {
      case raisim::Shape::Box :
        meshName = "cubeMesh";
        dim = {vo.visShapeParam[0], vo.visShapeParam[1], vo.visShapeParam[2]};
        break;
      case raisim::Shape::Sphere :
        meshName = "sphereMesh";
        dim = {vo.visShapeParam[0], vo.visShapeParam[0], vo.visShapeParam[0]};
        break;
      case raisim::Shape::Cylinder :
        meshName = "cylinderMesh";
        dim = {vo.visShapeParam[0], vo.visShapeParam[0], vo.visShapeParam[1]};
        break;
      case raisim::Shape::Capsule :
        meshName = "capsuleMesh";
        dim = {vo.visShapeParam[0], vo.visShapeParam[0], vo.visShapeParam[1]};
        break;
      default:
        RSFATAL("unsupported visual shape of " << name << " of " << as->getRobotDescriptionfFileName())
    }
    graphics.push_back(createSingleGraphicalObject(visname + meshName,
                                                   meshName,
                                                   "",
                                                   dim,
                                                   vo.offset,
                                                   vo.rot,
                                                   vo.localIdx,
                                                   true,
                                                   true,
                                                   group));
  }
  graphics.back().rotationOffset = vo.rot;
}

void OgreVis::select(const GraphicObject &ob, bool highlight) {
  auto node = ob.graphics;
  auto *newSelEn = dynamic_cast<Ogre::Entity *>(node->getAttachedObject(0));

  cameraMan_->setStyle(CS_ORBIT);
  cameraMan_->setTarget(node);
  selected_ = node;
}

std::pair<raisim::Object *, size_t> OgreVis::getRaisimObject(Ogre::SceneNode *ob) {
  return objectSet_[ob];
}

void OgreVis::startRecordingVideo(const std::string &filename) {
  initiateVideoRecording_ = true;
  stopVideoRecording_ = false;
  isVideoRecording_ = false; // since we did not init
  currentVideoFile_ = filename;
}

void OgreVis::stopRecordingVideoAndSave() {
  stopVideoRecording_ = true;
}

void OgreVis::buildHeightMap(const std::string &name,
                             size_t xSamples,
                             float xSize,
                             float centerX,
                             size_t ySamples,
                             float ySize,
                             float centerY,
                             const std::vector<float> &height) {
  if (Ogre::MeshManager::getSingleton().getByName(name))
    return;
  std::vector<float> vertex;
  std::vector<float> normal;
  std::vector<float> uv;
  std::vector<unsigned long> indices;

  vertex.reserve(xSamples * ySamples * 18);
  normal.reserve(xSamples * ySamples * 18);
  uv.reserve(xSamples * ySamples * 12);
  indices.reserve(xSamples * ySamples * 18);

  auto gridSizeX = float(xSize / (xSamples - 1));
  auto gridSizeY = float(ySize / (ySamples - 1));
  auto gridStartX = float(centerX - xSize / 2.0);
  auto gridStartY = float(centerY - ySize / 2.0);

  /// HACK: all verticies are copied 6 times to create flat shading
  for (int i = 0; i < ySamples - 1; i++) {
    for (int j = 0; j < xSamples - 1; j++) {
      vertex.push_back(gridStartX + j * gridSizeX);
      vertex.push_back(gridStartY + i * gridSizeY);
      vertex.push_back(float(height[i * xSamples + j]));

      vertex.push_back(gridStartX + (j + 1) * gridSizeX);
      vertex.push_back(gridStartY + i * gridSizeY);
      vertex.push_back(float(height[i * xSamples + (j + 1)]));

      vertex.push_back(gridStartX + (j + 1) * gridSizeX);
      vertex.push_back(gridStartY + (i + 1) * gridSizeY);
      vertex.push_back(float(height[(i + 1) * xSamples + (j + 1)]));

      vertex.push_back(gridStartX + j * gridSizeX);
      vertex.push_back(gridStartY + i * gridSizeY);
      vertex.push_back(float(height[i * xSamples + j]));

      vertex.push_back(gridStartX + (j + 1) * gridSizeX);
      vertex.push_back(gridStartY + (i + 1) * gridSizeY);
      vertex.push_back(float(height[(i + 1) * xSamples + (j + 1)]));

      vertex.push_back(gridStartX + j * gridSizeX);
      vertex.push_back(gridStartY + (i + 1) * gridSizeY);
      vertex.push_back(float(height[(i + 1) * xSamples + j]));
    }
  }

  for (unsigned long i = 0; i < vertex.size() / 3; i++) {
    indices.push_back(i);
  }

  for (unsigned long i = 0; i < vertex.size(); i += 9) {
    raisim::Vec<3> point1, point2, point3, norm, diff1, diff2;
    point1 = {vertex[i], vertex[i + 1], vertex[i + 2]};
    point2 = {vertex[i + 3], vertex[i + 4], vertex[i + 5]};
    point3 = {vertex[i + 6], vertex[i + 7], vertex[i + 8]};

    vecsub(point2, point1, diff1);
    vecsub(point3, point2, diff2);
    cross(diff1, diff2, norm);
    norm /= norm.norm();

    normal.push_back(norm[0]);
    normal.push_back(norm[1]);
    normal.push_back(norm[2]);

    normal.push_back(norm[0]);
    normal.push_back(norm[1]);
    normal.push_back(norm[2]);

    normal.push_back(norm[0]);
    normal.push_back(norm[1]);
    normal.push_back(norm[2]);
  }

  for (int i = 0; i < ySamples - 1; i++) {
    for (int j = 0; j < xSamples - 1; j++) {
      uv.push_back(float(j) / float(xSamples));
      uv.push_back(float(i) / float(ySamples));
      uv.push_back(float(j + 1) / float(xSamples));
      uv.push_back(float(i) / float(ySamples));
      uv.push_back(float(j + 1) / float(xSamples));
      uv.push_back(float(i + 1) / float(ySamples));

      uv.push_back(float(j) / float(xSamples));
      uv.push_back(float(i) / float(ySamples));
      uv.push_back(float(j + 1) / float(xSamples));
      uv.push_back(float(i + 1) / float(ySamples));
      uv.push_back(float(j) / float(xSamples));
      uv.push_back(float(i + 1) / float(ySamples));
    }
  }

  createMesh(name, vertex, normal, uv, indices);
}

void OgreVis::createMesh(const std::string& name,
                         const std::vector<float>& vertex,
                         const std::vector<float>& normal,
                         const std::vector<float>& uv,
                         const std::vector<unsigned long>& indices) {
  // now begin the object definition
  // We create a submesh per material
  Ogre::MeshPtr mMesh =
      Ogre::MeshManager::getSingleton().createManual(name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

  RSFATAL_IF(meshUsageCount_.find(name) != meshUsageCount_.end() && meshUsageCount_[name] != 0,
             "Destroy the existing terrain before creating one")
  meshUsageCount_[name] = 1;

  Ogre::SubMesh *submesh = mMesh->createSubMesh(name + "_submesh");
  Ogre::AxisAlignedBox bounds;

  // We must create the vertex data, indicating how many vertices there will be
  submesh->useSharedVertices = false;
  submesh->vertexData = new Ogre::VertexData();
  submesh->vertexData->vertexStart = 0;
  submesh->vertexData->vertexCount = vertex.size() / 3;

  // We must now declare what the vertex data contains
  Ogre::VertexDeclaration *declaration = submesh->vertexData->vertexDeclaration;
  static const unsigned short source = 0;
  size_t offset = 0;
  offset += declaration->addElement(source, offset, Ogre::VET_FLOAT3, Ogre::VES_POSITION).getSize();
  offset += declaration->addElement(source, offset, Ogre::VET_FLOAT3, Ogre::VES_NORMAL).getSize();
  if(uv.size()!=0) offset += declaration->addElement(source, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES).getSize();

  // We create the hardware vertex buffer
  Ogre::HardwareVertexBufferSharedPtr vbuffer =
      Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(declaration->getVertexSize(source), // == offset
                                                                     submesh->vertexData->vertexCount,   // == nbVertices
                                                                     Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);

  // Now we get access to the buffer to fill it.  During so we record the bounding box.
  auto *vdata = static_cast<float *>(vbuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD));

  auto *vect = &vertex[0];
  auto *uvco = &uv[0];
  auto *norm = &normal[0];

  for (size_t i = 0; i < vertex.size() / 3; ++i) {
    *vdata++ = *vect;
    *vdata++ = *(vect + 1);
    *vdata++ = *(vect + 2);
    Ogre::Vector3 position(*vect, *(vect + 1), *(vect + 2));
    bounds.merge(position);
    vect += 3;
    *vdata++ = *norm++;
    *vdata++ = *norm++;
    *vdata++ = *norm++;
    if(uv.size()!=0) {
      *vdata++ = *uvco++;
      *vdata++ = *uvco++;
    }
  }

  vbuffer->unlock();
  submesh->vertexData->vertexBufferBinding->setBinding(source, vbuffer);

  // Creates the index data
  submesh->indexData->indexStart = 0;
  submesh->indexData->indexCount = indices.size();

  if (submesh->indexData->indexCount >= 65536) // 32 bit index buffer
  {
    submesh->indexData->indexBuffer = Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
        Ogre::HardwareIndexBuffer::IT_32BIT,
        submesh->indexData->indexCount,
        Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);

    auto *indexData =
        static_cast<Ogre::uint32 *>(submesh->indexData->indexBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD));

    auto *ind = &indices[0];
    for (size_t i = 0; i < indices.size(); ++i)
      *indexData++ = *ind++;

  } else // 16 bit index buffer
  {
    submesh->indexData->indexBuffer = Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
        Ogre::HardwareIndexBuffer::IT_16BIT,
        submesh->indexData->indexCount,
        Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);

    auto *indexData =
        static_cast<Ogre::uint16 *>(submesh->indexData->indexBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD));

    auto *ind = &indices[0];
    for (size_t i = 0; i < indices.size(); ++i)
      *indexData++ = *ind++;
  }

  submesh->indexData->indexBuffer->unlock();

  mMesh->_setBounds(bounds);
  mMesh->_setBoundingSphereRadius((bounds.getMaximum() - bounds.getMinimum()).length() / 2);
}

std::vector<GraphicObject> *OgreVis::createGraphicalObject(raisim::HeightMap *hm,
                                                           const std::string &name,
                                                           const std::string &material,
                                                           int sampleEveryN) {

  auto xSamples = hm->getXSamples();
  auto xSize = hm->getXSize();
  auto centerX = hm->getCenterX();
  hm->setName(name);
  auto ySamples = hm->getYSamples();
  auto ySize = hm->getYSize();
  auto centerY = hm->getCenterY();
  auto &height = hm->getHeightVector();
  std::vector<float> heightFloat;
  heightFloat.clear();

  for (size_t x=0; x < xSamples; x+=sampleEveryN) {
    for (size_t y=0; y < ySamples; y+=sampleEveryN) {
      heightFloat.push_back(height[x*ySamples+y]);
    }
  }

  buildHeightMap(name, (xSamples-1)/sampleEveryN+1, xSize, centerX, (ySamples-1)/sampleEveryN+1, ySize, centerY, heightFloat);
  raisim::Mat<3, 3> rot;
  rot.setIdentity();

  return registerSet(name,
                     hm,
                     {createSingleGraphicalObject("terrain",
                                                  name,
                                                  material,
                                                  {1, 1, 1},
                                                  {0, 0, 0},
                                                  rot,
                                                  0,
                                                  false,
                                                  false,
                                                  RAISIM_COLLISION_BODY_GROUP | RAISIM_OBJECT_GROUP)});
}

std::vector<GraphicObject> *OgreVis::createGraphicalObject(raisim::Mesh *mesh,
                                                           const std::string &name,
                                                           const std::string &material) {

  Mat<3, 3> rot;
  rot.setIdentity();
  const std::string &meshName = mesh->getMeshFileName();

  raisim::OgreVis::loadMeshFile(meshName, meshName);
  return registerSet(name, mesh, {
          createSingleGraphicalObject(name,
                                      meshName,
                                      material,
                                      {1, 1, 1},
                                      {0, 0, 0},
                                      rot,
                                      0,
                                      true,
                                      true,
                                      RAISIM_COLLISION_BODY_GROUP | RAISIM_OBJECT_GROUP)
  });
}

void OgreVis::createAndAppendVisualObject(const std::string &name,
                                          const std::string &meshName,
                                          const std::string &mat,
                                          std::vector<raisim::VisualObject> &vec) {
  auto *ent = this->getSceneManager()->createEntity(name, meshName);
  ent->setMaterialName(mat);
  auto *node = this->getSceneManager()->getRootSceneNode()->createChildSceneNode(name);
  node->attachObject(ent);
  vec.emplace_back(node);
}

void OgreVis::updateVisualizationObject(raisim::VisualObject &vo) {
  Vec<4> quat;
  vo.graphics->setPosition(float(vo.offset[0]), float(vo.offset[1]), float(vo.offset[2]));
  raisim::rotMatToQuat(vo.rotationOffset, quat);
  vo.graphics->setOrientation(float(quat[0]), float(quat[1]), float(quat[2]), float(quat[3]));
  vo.graphics->setScale(vo.scale[0], vo.scale[1], vo.scale[2]);
}

void OgreVis::renderOneFrame() {
  /// graphical objects are synced with simulation objects
  if (!remoteMode_) {

    auto *contactProblem = world_->getContactProblem();
    auto nContact = contactProblem->size();

    sync();

    /// initially set everything to false
    for (auto &con : contactPoints_)
      con.graphics->setVisible(false);

    for (auto &con : contactForces_)
      con.graphics->setVisible(false);

    /// add more if necessary
    if (mask_ & RAISIM_CONTACT_POINT_GROUP)
      for (size_t i = contactPoints_.size(); i < nContact; i++) {
        createAndAppendVisualObject("cp" + std::to_string(i), "sphereMesh", "redEmit", contactPoints_);
        contactPoints_.back().scale = {contactPointSphereSize_, contactPointSphereSize_, contactPointSphereSize_};
      }

    if (mask_ & RAISIM_CONTACT_FORCE_GROUP)
      for (size_t i = contactForces_.size(); i < nContact; i++) {
        createAndAppendVisualObject("cf" + std::to_string(i), "arrowMesh", "blueEmit", contactForces_);
      }

    /// now actually update the position and orientation
    if (mask_ & RAISIM_CONTACT_POINT_GROUP)
      for (size_t i = 0; i < nContact; i++) {
        contactPoints_[i].offset = contactProblem->at(i).position_W;
        contactPoints_[i].graphics->setVisible(true);
        contactPoints_[i].group = RAISIM_CONTACT_POINT_GROUP;
        contactPoints_[i].scale.setConstant(contactPointSphereSize_);
        updateVisualizationObject(contactPoints_[i]);
      }

    if (mask_ & RAISIM_CONTACT_FORCE_GROUP) {
      double maxNorm = 0;
      for (size_t i = 0; i < nContact; i++)
        maxNorm = std::max(maxNorm, contactProblem->at(i).imp_i.norm());

      size_t contactIdx = 0;

      for (auto *obj: world_->getObjList()) {
        for (auto &contact: obj->getContacts()) {
          if (!contact.isObjectA() && contact.getPairObjectBodyType() != raisim::BodyType::STATIC) continue;
          contactForces_[contactIdx].offset = contact.getPosition();
          raisim::Vec<3> zaxis = *contact.getImpulse();
          raisim::Mat<3, 3> rot;
          double norm = zaxis.norm();
          if (norm == 0) {
            contactForces_[contactIdx].graphics->setVisible(false);
            continue;
          }

          raisim::transpose(contact.getContactFrame(), contactForces_[contactIdx].rotationOffset);

          zaxis /= norm;

          contactForces_[contactIdx].scale = {0.3 * norm / maxNorm * contactForceArrowLength_,
                                              0.3 * norm / maxNorm * contactForceArrowLength_,
                                              norm / maxNorm * contactForceArrowLength_};

          raisim::zaxisToRotMat(zaxis, rot);

          contactForces_[contactIdx].rotationOffset = rot;
          contactForces_[contactIdx].graphics->setVisible(true);
          contactForces_[contactIdx].group = RAISIM_CONTACT_FORCE_GROUP;
          updateVisualizationObject(contactForces_[contactIdx]);
          contactIdx++;
        }
      }
    }
  }

  for (auto &vob: visObject_) {
    vob.second.graphics->setVisible(vob.second.group & mask_);
    updateVisualizationObject(vob.second);
  }

  for (auto &wire: wires_) {
    wire.second.graphics->setVisible(wire.second.group & mask_);
    raisim::Vec<3> mid, p1, p2, norm;
    raisim::Mat<3, 3> rot;
    auto wobj = world_->getWire(wire.first);
    wobj->update();
    p1 = wobj->getP1();
    p2 = wobj->getP2();
    raisim::vecadd(p1, p2, mid);
    mid *= 0.5;
    norm = wobj->getNorm();
    raisim::zaxisToRotMat(norm, rot);

    wire.second.offset = mid;
    wire.second.rotationOffset = rot;
    wire.second.scale = {wireThickness_, wireThickness_, wobj->getLength()};
    updateVisualizationObject(wire.second);
  }

  getRoot()->renderOneFrame();

  /// meausre time for sim and rendering
  end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed = end - start;
  start = std::chrono::system_clock::now();

  /// compute how much you have to wait and wait
  auto diff = 1. / desiredFPS_ - elapsed.count();
  if (diff > 0) {
    MSLEEP(diff * 1000.);
    start = std::chrono::system_clock::now();
  }
}

void OgreVis::deselect() {
  cameraMan_->setStyle(CS_FREELOOK);
  selected_ = nullptr;
}

void OgreVis::closeApp() {
  ApplicationContext::closeApp();
  imGuiRenderCallback_ = nullptr;
  imGuiSetupCallback_ = nullptr;
  keyboardCallback_ = nullptr;
  setUpCallback_ = nullptr;
  controlCallback_ = nullptr;
}

}
