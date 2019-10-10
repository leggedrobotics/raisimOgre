//
// Created by jhwangbo on 17.01.19.
//

#ifndef RAISIM_OGRE_VISUALIZER_HPP
#define RAISIM_OGRE_VISUALIZER_HPP
#define OGREVIS_MAKE_STR(x) _OGREVIS_MAKE_STR(x)
#define _OGREVIS_MAKE_STR(x) #x

#include "Ogre.h"
#include "OgreApplicationContext.h"
#include "OgreInput.h"
#include "OgreRTShaderSystem.h"
#include "OgreApplicationContext.h"
#include <iostream>
#include "raisim/RaiCameraMan.hpp"
#include <OgreTrays.h>
#include "ImguiManager.h"
#include "AssimpLoader.h"
#include "interfaceClasses.hpp"
#include "SDL2/SDL.h"

namespace raisim {

class OgreVis :
    public ApplicationContext, public InputListener, public TrayListener {

 public:

  enum VisualizationGroup :
      unsigned long {
    RAISIM_OBJECT_GROUP = 1ul << 0,
    RAISIM_COLLISION_BODY_GROUP = 1ul << 1,
    RAISIM_CONTACT_POINT_GROUP = 1ul << 2,
    RAISIM_CONTACT_FORCE_GROUP = 1ul << 3
  };

  /** return a pointer of the singleton**/
  static OgreVis *get() {
    static OgreVis singleton;
    return &singleton;
  }

  using ImGuiRenderCallback = std::function<void()>;
  using ImGuiSetupCallback = std::function<void()>;
  using KeyboardCallback = std::function<bool(const KeyboardEvent &)>;
  using SetUpCallback = std::function<void()>;
  using ControlCallback = std::function<void()>;

  /** set imgui render callback. This callback is called for every frame. */
  void setImguiRenderCallback(ImGuiRenderCallback callback) { imGuiRenderCallback_ = callback; }

  /** set imgui setup callback. This callback is called only once in setup */
  void setImguiSetupCallback(ImGuiSetupCallback callback) { imGuiSetupCallback_ = callback; }

  /** set keyboard callback. This callback is called for every keyboard event */
  void setKeyboardCallback(KeyboardCallback callback) { keyboardCallback_ = callback; }

  /** set up callbacks. Load custom meshes. Load materials. anything necessary for setup */
  void setSetUpCallback(SetUpCallback callback) { setUpCallback_ = callback; }

  /** set up callbacks. Load custom meshes. Load materials. anything necessary for setup */
  void setControlCallback(ControlCallback callback) { controlCallback_ = callback; }

  /** add resource directory for materials */
  void addResourceDirectory(const std::string &dir);

  /** once the directory is added, load the material file in the directory**/
  void loadMaterialFile(const std::string &filename);

  /** loading mesh file using assimp. Ogre only reads .mesh file. If it something else is given,
   * it creates .mesh file inside the directory. file can be the whole path to the meshfile, or from
   * the memory**/
  void loadMeshFile(const std::string &file, const std::string &meshName, bool fromMemory = false);

  /** get resource directory of RaisimOgreVisualizer**/
  const std::string &getResourceDir() { return resourceDir_; }

  /** get Ogre::SceneManager owned by this class**/
  Ogre::SceneManager *getSceneManager() { return scnMgr_; }

  /** get CameraMan**/
  CameraMan *getCameraMan() { return cameraMan_; }

  /** get default light for adjusting it**/
  Ogre::Light *getLight() { return light_; }

  /** get LightNode for moving the main light around**/
  Ogre::SceneNode *getLightNode() { return lightNode_; }

  /** get main Ogre::Viewport**/
  Ogre::Viewport *getViewPort() { return viewport_; }

  /** set raisim world. Must be called before simulation */
  void setWorld(raisim::World *world) {
    world_ = world;
    objectSet_.setWorld(world);
  }

  /** get raisim world.*/
  const raisim::World *getWorld() { return world_; }

  /** run simulation and visualization**/
  void run();

  /** start rendering loop without updating simulation*/
  void startRendering() { while (!getRoot()->endRenderingQueued()) { getRoot()->renderOneFrame(); }}

  /** renders a single frame without updating simulation*/
  void renderOneFrame();

  /** set camera speed for free motion*/
  void setCameraSpeed(float speed) { cameraSpeed_ = speed; }

  /** register a pair of raisim object and graphic object manually. */
  std::vector<GraphicObject> *registerSet(const std::string &name,
                                          raisim::Object *ob,
                                          std::vector<GraphicObject> &&graphics);

  /**
   * @param sphere raisim sphere object
   * @param name unique identifier of the object
   * @param material for visualization */
  std::vector<GraphicObject> *createGraphicalObject(raisim::Sphere *sphere,
                                                    const std::string &name,
                                                    const std::string &material);

  /**
  * @param ground raisim ground object
  * @param name unique identifier of the object
  * @param material for visualization */
  std::vector<GraphicObject> *createGraphicalObject(raisim::Ground *ground,
                                                    double planeDim,
                                                    const std::string &name,
                                                    const std::string &material);

  /**
  * @param box raisim box object
  * @param name unique identifier of the object
  * @param material for visualization */
  std::vector<GraphicObject> *createGraphicalObject(raisim::Box *box,
                                                    const std::string &name,
                                                    const std::string &material);
  /**
  * @param capsule raisim capsule object
  * @param name unique identifier of the object
  * @param material for visualization */
  std::vector<GraphicObject> *createGraphicalObject(raisim::Cylinder *capsule,
                                                    const std::string &name,
                                                    const std::string &material);

  /**
  * @param raisim wire object
  * @param name unique identifier of the object
  * @param material for visualization */
  raisim::VisualObject *createGraphicalObject(raisim::Wire *wire,
                                              const std::string &name,
                                              const std::string &material = "default");

  /**
  * @param capsule raisim capsule object
  * @param name unique identifier of the object
  * @param material for visualization */
  std::vector<GraphicObject> *createGraphicalObject(raisim::Capsule *capsule,
                                                    const std::string &name,
                                                    const std::string &material);

  /**
  * @param as raisim articulated system object
  * @param name unique identifier of the object */
  std::vector<GraphicObject> *createGraphicalObject(raisim::ArticulatedSystem *as,
                                                    const std::string &name);

  /**
  * @param hm raisim heightmap object
  * @param name unique identifier of the object */
  std::vector<GraphicObject> *createGraphicalObject(raisim::HeightMap *hm,
                                                    const std::string &name,
                                                    const std::string &material = "default",
                                                    int sampleEveryN = 1);

  /**
  * @param as raisim articulated system object
  * @param name unique identifier of the object */
  std::vector<GraphicObject> *createGraphicalObject(raisim::Mesh *mesh,
                                                    const std::string &name,
                                                    const std::string &material = "default");

  /** sync raisim and ogre*/
  void sync();

  /** **/
  bool &getPaused() { return paused_; }

  /** remove object */
  void remove(raisim::Object *ob);

  /** remove object */
  void remove(const std::string &name);

  void setWindowSize(uint32_t windowX, uint32_t windowY) {
    initialWindowSizeX_ = windowX;
    initialWindowSizeY_ = windowY;
  }

  /** return the current selected item **/
  std::pair<Object *, size_t> getSelected() { return objectSet_[selected_]; }

  /** return the current selected item **/
  GraphicObject *getSelectedGraphicalObject() { return objectSet_.getGraphicObject(selected_); }

  /** select given object **/
  void select(const GraphicObject &ob, bool highlight=true);
  void deselect();

  /** get the raisim object **/
  std::pair<raisim::Object *, size_t> getRaisimObject(Ogre::SceneNode *ob);

  /** get the raw resource storage **/
  SimAndGraphicsObjectPool &getObjectSet();

  GraphicObject *getGraphicObject(Ogre::SceneNode *node) { return objectSet_.getGraphicObject(node); }

  /** is visualizer recording frames for a video **/
  bool isRecording() { return isVideoRecording_; }

  /** initiate a video recording session **/
  void startRecordingVideo(const std::string &filename);

  /** stop recording frames and save to a file**/
  void stopRecordingVideoAndSave();

  void setDesiredFPS(double fps) { desiredFPS_ = fps; }

  /** anti-aliasing. Must be set before init **/
  void setAntiAliasing(int fsaa) {
    RSFATAL_IF(fsaa != 1 && fsaa != 2 && fsaa != 4 && fsaa != 8, "fsaa should be set to one of (1,2,4,8)")
    fsaa_ = fsaa;
  }

  /** visibility mask is a bitfield **/
  void setVisibilityMask(unsigned long int mask) { mask_ = mask; }

  /** return visual objects **/
  std::map<std::string, VisualObject> &getVisualObjectList() { return visObject_; }

  /** the force size is the size corresponding to the maximum impulse **/
  void setContactVisObjectSize(double pointSize, double forceArrowLength) {
    contactPointSphereSize_ = pointSize;
    contactForceArrowLength_ = forceArrowLength;
  }

  float &getRealTimeFactorReference() { return realTimeFactor_; }

  int &getTakeNSteps() { return takeNSteps_; }

  void setRemoteMode(bool remoteMode) { remoteMode_ = remoteMode; }

  void remoteRun();

  void addVisualObject(const std::string &name,
                       const std::string &meshName,
                       const std::string &material,
                       const raisim::Vec<3> &scale,
                       bool castShadow = true,
                       unsigned long int group = RAISIM_OBJECT_GROUP | RAISIM_COLLISION_BODY_GROUP);

  void clearVisualObject();

  void buildHeightMap(const std::string &name,
                      size_t xSamples,
                      float xSize,
                      float centerX,
                      size_t ySamples,
                      float ySize,
                      float centerY,
                      const std::vector<float> &height);

  /// this method is not meant to be inerited but it solves python binding issue in raisimpy
  void closeApp();

  void hideWindow() { SDL_HideWindow(windowPair_.native); }
  void showWindow() { SDL_ShowWindow(windowPair_.native);}

  /// use this method to manually assign a mesh to an object
  GraphicObject createSingleGraphicalObject(const std::string &name,
                                            const std::string &meshName,
                                            const std::string &material,
                                            const raisim::Vec<3> &scale,
                                            const Vec<3> &offset,
                                            const Mat<3, 3> &rot,
                                            size_t localIdx,
                                            bool castShadow = true,
                                            bool selectable = true,
                                            unsigned long int group = RAISIM_OBJECT_GROUP | RAISIM_COLLISION_BODY_GROUP);

 private:
  OgreVis()
      : ApplicationContext("RaisimDemoApp") {
    resourceDir_ = std::string(OGREVIS_MAKE_STR(RAISIM_OGRE_RESOURCE_DIR));
    mFSLayer->setHomePath(std::string(OGREVIS_MAKE_STR(OGRE_CONFIG_DIR)));

    RSINFO("Loading RaisimOgre Resource from: " + resourceDir_)
    RSINFO("Loading Ogre Configuration from: " + std::string(OGREVIS_MAKE_STR(OGRE_CONFIG_DIR)))

    lm_ = std::make_unique<Ogre::LogManager>();
    lm_->createLog("", true, false, false); //TODO: redirect to our own logger.

    start = std::chrono::system_clock::now();
  }

  ~OgreVis() final;
  void setup() final;
  void shutdown() final;
  bool keyPressed(const KeyboardEvent &evt) final;
  bool keyReleased(const KeyboardEvent &evt) final;
  bool mousePressed(const MouseButtonEvent &evt) final;
  bool mouseReleased(const MouseButtonEvent &evt) final;
  bool mouseMoved(const MouseMotionEvent &evt) final;
  bool mouseWheelRolled(const MouseWheelEvent &evt) final;
  bool frameRenderingQueued(const Ogre::FrameEvent &evt) final;
  void frameRendered(const Ogre::FrameEvent &evt) final;
  bool frameStarted(const Ogre::FrameEvent &evt) final;
  bool frameEnded(const Ogre::FrameEvent &evt) final;
  void videoThread();

  void registerRaisimGraphicalObjects(raisim::VisObject &vo,
                                      std::vector<GraphicObject> &graphics,
                                      raisim::ArticulatedSystem *as,
                                      const std::string &name,
                                      unsigned long int group);

  void createAndAppendVisualObject(const std::string &name,
                                   const std::string &meshName,
                                   const std::string &mat,
                                   std::vector<raisim::VisualObject> &vec);

  void updateVisualizationObject(raisim::VisualObject &vo);

  void createMesh(const std::string& name,
                  const std::vector<float>& vertex,
                  const std::vector<float>& normal,
                  const std::vector<float>& uv,
                  const std::vector<unsigned long>& indices);

  ImGuiRenderCallback imGuiRenderCallback_ = nullptr;
  ImGuiSetupCallback imGuiSetupCallback_ = nullptr;
  KeyboardCallback keyboardCallback_ = nullptr;
  SetUpCallback setUpCallback_ = nullptr;
  ControlCallback controlCallback_ = nullptr;
  NativeWindowPair windowPair_;
  Ogre::SceneNode *camNode_ = nullptr;
  Ogre::Camera *mainCamera_ = nullptr;
  raisim::CameraMan *cameraMan_;
  Ogre::RaySceneQuery *raySceneQuery_ = nullptr;
  float cameraSpeed_;
  bool leftMouseButtonPressed_ = false;
  bool rightMouseButtonPressed_ = false;
  bool leftShiftButtonPressed_ = false;
  int mouseX_ = 0, mouseY_ = 0;
  std::string resourceDir_;
  Ogre::SceneNode *selected_ = nullptr, *hovered_ = nullptr;
  std::vector<Ogre::Entity> *highlightEntity_;
  Ogre::SceneManager *scnMgr_;
  Ogre::Light *light_;
  Ogre::SceneNode *lightNode_;
  Ogre::Viewport *viewport_;
  Ogre::GpuProgramParametersSharedPtr mParams;
  raisim::World *world_ = nullptr;
  SimAndGraphicsObjectPool objectSet_;
  std::map<std::string, VisualObject> visObject_;
  std::map<std::string, VisualObject> wires_;
  uint32_t initialWindowSizeX_ = 400, initialWindowSizeY_ = 300;
  std::map<std::string, size_t> meshUsageCount_;
  std::set<std::string> primitiveMeshNames_;
  double desiredFPS_ = 30.;
  int fsaa_ = 8;
  float realTimeFactor_ = 1.0;
  bool remoteMode_ = false;
  double wireThickness_ = 0.02;

  /// visualizing simulation
  std::vector<raisim::VisualObject> contactPoints_;
  double contactPointSphereSize_ = 0.03;
  std::vector<raisim::VisualObject> contactForces_;
  double contactForceArrowRadius_ = 0.03;
  double contactForceArrowLength_ = 0.1;
  raisim::VisualObject externalForceArrow_;
  std::unique_ptr<Ogre::LogManager> lm_;

  /// video recording related
  std::string currentVideoFile_;
  bool initiateVideoRecording_;
  bool stopVideoRecording_;
  bool isVideoRecording_;
  bool newFrameAvailable_ = false;
  std::mutex videoFrameMutext_, videoInitMutex_;

  std::unique_ptr<Ogre::PixelBox> videoPixelBox_;
  std::unique_ptr<Ogre::uchar> videoBuffer_;
  FILE *ffmpeg;
  std::unique_ptr<std::thread> videoThread_;
  int imageCounter, imageBufferSize_;
  int takeNSteps_ = 0; // negative means run infinitely
  bool paused_ = false;
  unsigned long int mask_ = 1ul;
  std::chrono::system_clock::time_point start, end;

};
}

#endif //OGRETEST_RAISIMVISUALIZER_HPP
