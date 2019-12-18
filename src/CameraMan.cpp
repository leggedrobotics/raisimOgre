#include <iostream>
#include <raisim/math.hpp>
#include "raisim/CameraMan.hpp"
#include "OgreSceneManager.h"

namespace raisim {

CameraMan::CameraMan(Ogre::SceneNode *cam)
    : mYawSpace(Ogre::Node::TS_PARENT)
    , mCamera(0)
    , mStyle(CS_MANUAL)
    , mTarget(0)
    , mOrbiting(true)
    , mMoving(false)
    , mTopSpeed(10.0f)
    , mVelocity(Ogre::Vector3::ZERO)
    , mGoingForward(false)
    , mGoingBack(false)
    , mGoingLeft(false)
    , mGoingRight(false)
    , mGoingUp(false)
    , mGoingDown(false)
    , mFastMove(false)
    , mOffset(0, 0, 0)
{

  setCamera(cam);
  setStyle(CS_FREELOOK);
}

void CameraMan::setCamera(Ogre::SceneNode *cam)
{
  mCamera = cam;
}

void CameraMan::setTarget(Ogre::SceneNode *target)
{
  if (target == mTarget)
    return;

  mTarget = target;
}

void CameraMan::setYawPitchDist(Ogre::Radian yaw, Ogre::Radian pitch, Ogre::Real dist, bool trackObjectsYaw)
{
  OgreAssert(mTarget, "no target set");
  yaw_ = yaw;
  pitch_ = pitch;
  dist_ = dist;

  mOffset = Ogre::Vector3::ZERO;
  mCamera->setPosition(mTarget->_getDerivedPosition());
  mCamera->setOrientation(Ogre::Quaternion{1.,0.,0.,0.});
  if(trackObjectsYaw)
    mCamera->yaw(mTarget->_getDerivedOrientation().getRoll());
  mCamera->yaw(-yaw_);
  mCamera->pitch(-pitch_);
  mCamera->translate(Ogre::Vector3(0, 0, dist_), Ogre::Node::TS_LOCAL);
}

void CameraMan::setStyle(CameraStyle style)
{
  if (mStyle != CS_ORBIT && style == CS_ORBIT)
  {
    setTarget(mTarget ? mTarget : mCamera->getCreator()->getRootSceneNode());
    auto targetPos = mTarget->getPosition();
    auto cameraPos = mCamera->getPosition();

    auto diff = targetPos - cameraPos;
    yaw_ = mCamera->getOrientation().getYaw();
    pitch_ = mCamera->getOrientation().getPitch();
    dist_ = diff.normalise();
    setYawPitchDist(-yaw_, -pitch_, dist_);

    manualStop();

    // try to replicate the camera configuration
//    Ogre::Real dist = getDistToTarget();
//    const Ogre::Quaternion& q = mCamera->getOrientation();
//    setYawPitchDist(q.getYaw(), q.getPitch(), dist == 0 ? 150 : dist); // enforce some distance
  }
  else if (mStyle != CS_FREELOOK && style == CS_FREELOOK)
  {
    mCamera->setFixedYawAxis(true, Ogre::Vector3::UNIT_Z); // also fix axis with lookAt calls
  }
  else if (mStyle != CS_MANUAL && style == CS_MANUAL)
  {
    manualStop();
  }
  mStyle = style;
  mCamera->setAutoTracking(true);
}

void CameraMan::manualStop()
{
  if (mStyle == CS_FREELOOK)
  {
    mGoingForward = false;
    mGoingBack = false;
    mGoingLeft = false;
    mGoingRight = false;
    mGoingUp = false;
    mGoingDown = false;
    mVelocity = Ogre::Vector3::ZERO;
  }
}

void CameraMan::frameRendered(const Ogre::FrameEvent &evt)
{

  update();
  if (mStyle == CS_FREELOOK)
#include "OgreBitesPrerequisites.h"
#include "OgreCamera.h"
#include "OgreSceneNode.h"
#include "OgreFrameListener.h"

#include "OgreInput.h"

/** \addtogroup Optional
*  @{
*/
/** \addtogroup Bites
*  @{
*/
  {
    enum CameraStyle   /// enumerator values for different styles of camera movement
    {
      CS_FREELOOK,
      CS_ORBIT,
      CS_MANUAL
    };

    /**
    Utility class for controlling the camera in samples.
    */
    class _OgreBitesExport CameraMan : public InputListener
    {
     public:
      CameraMan(Ogre::SceneNode* cam);

      /**
      Swaps the camera on our camera man for another camera.
      */
      void setCamera(Ogre::SceneNode* cam);

      Ogre::SceneNode* getCamera()
      {
        return mCamera;
      }

      /**
      Sets the target we will revolve around. Only applies for orbit style.
      */
      virtual void setTarget(Ogre::SceneNode* target);

      Ogre::SceneNode* getTarget()
      {
        return mTarget;
      }

      /**
      Sets the spatial offset from the target. Only applies for orbit style.
      */
      void setYawPitchDist(Ogre::Radian yaw, Ogre::Radian pitch, Ogre::Real dist, bool trackObjectsYaw);

      /**
      Sets the camera's top speed. Only applies for free-look style.
      */
      void setTopSpeed(Ogre::Real topSpeed)
      {
        mTopSpeed = topSpeed;
      }

      Ogre::Real getTopSpeed()
      {
        return mTopSpeed;
      }

      /**
      Sets the movement style of our camera man.
      */
      virtual void setStyle(CameraStyle style);

      CameraStyle getStyle()
      {
        return mStyle;
      }

      /**
      Manually stops the camera when in free-look mode.
      */
      void manualStop();

      void frameRendered(const Ogre::FrameEvent& evt);

      /**
      Processes key presses for free-look style movement.
      */
      bool keyPressed(const KeyboardEvent& evt);

      /**
      Processes key releases for free-look style movement.
      */
      bool keyReleased(const KeyboardEvent& evt);

      /**
      Processes mouse movement differently for each style.
      */
      bool mouseMoved(const MouseMotionEvent& evt);

      bool mouseWheelRolled(const MouseWheelEvent& evt);

      /**
      Processes mouse presses. Only applies for orbit style.
      Left button is for orbiting, and right button is for zooming.
      */
      bool mousePressed(const MouseButtonEvent& evt);

      /**
      Processes mouse releases. Only applies for orbit style.
      Left button is for orbiting, and right button is for zooming.
      */
      bool mouseReleased(const MouseButtonEvent& evt);

      /**
       * fix the yaw axis to be Vector3::UNIT_Y of the parent node (tabletop mode)
       *
       * otherwise the yaw axis can change freely
       */
      void setFixedYaw(bool fixed)
      {
        mYawSpace = fixed ? Ogre::Node::TS_PARENT : Ogre::Node::TS_LOCAL;
      }

      void setPivotOffset(const Ogre::Vector3& offset);
     protected:
      Ogre::Real getDistToTarget();
      Ogre::Node::TransformSpace mYawSpace;
      Ogre::SceneNode* mCamera;
      CameraStyle mStyle;
      Ogre::SceneNode* mTarget;
      bool mOrbiting;
      bool mMoving;
      Ogre::Real mTopSpeed;
      Ogre::Vector3 mVelocity;
      bool mGoingForward;
      bool mGoingBack;
      bool mGoingLeft;
      bool mGoingRight;
      bool mGoingUp;
      bool mGoingDown;
      bool mFastMove;
      Ogre::Vector3 mOffset;
    };
  }
/** @} */
/** @} */
  {
    // build our acceleration vector based on keyboard input composite
    Ogre::Vector3 accel = Ogre::Vector3::ZERO;
    Ogre::Matrix3 axes = mCamera->getLocalAxes();
    if (mGoingForward) accel -= axes.GetColumn(2);
    if (mGoingBack) accel += axes.GetColumn(2);
    if (mGoingRight) accel += axes.GetColumn(0);
    if (mGoingLeft) accel -= axes.GetColumn(0);
    if (mGoingUp) accel += axes.GetColumn(1);
    if (mGoingDown) accel -= axes.GetColumn(1);

    // if accelerating, try to reach top speed in a certain time
    Ogre::Real topSpeed = mFastMove ? mTopSpeed * 20 : mTopSpeed;
    if (accel.squaredLength() != 0)
    {
      accel.normalise();
      mVelocity += accel * topSpeed * evt.timeSinceLastFrame * 10;
    }
      // if not accelerating, try to stop in a certain time
    else mVelocity -= mVelocity * evt.timeSinceLastFrame * 10;

    Ogre::Real tooSmall = std::numeric_limits<Ogre::Real>::epsilon();

    // keep camera velocity below top speed and above epsilon
    if (mVelocity.squaredLength() > topSpeed * topSpeed)
    {
      mVelocity.normalise();
      mVelocity *= topSpeed;
    }
    else if (mVelocity.squaredLength() < tooSmall * tooSmall)
      mVelocity = Ogre::Vector3::ZERO;

    if (mVelocity != Ogre::Vector3::ZERO) mCamera->translate(mVelocity * evt.timeSinceLastFrame);
  }
}

bool CameraMan::keyPressed(const KeyboardEvent &evt)
{
  if (mStyle == CS_FREELOOK)
  {
    Keycode key = evt.keysym.sym;
    if (key == 'w' || key == SDLK_UP) mGoingForward = true;
    else if (key == 's' || key == SDLK_DOWN) mGoingBack = true;
    else if (key == 'a' || key == SDLK_LEFT) mGoingLeft = true;
    else if (key == 'd' || key == SDLK_RIGHT) mGoingRight = true;
    else if (key == SDLK_PAGEUP) mGoingUp = true;
    else if (key == SDLK_PAGEDOWN) mGoingDown = true;
    else if (key == SDLK_LSHIFT) mFastMove = true;
  }

  return InputListener::keyPressed(evt);
}

bool CameraMan::keyReleased(const KeyboardEvent &evt)
{
  if (mStyle == CS_FREELOOK)
  {
    Keycode key = evt.keysym.sym;
    if (key == 'w' || key == SDLK_UP) mGoingForward = false;
    else if (key == 's' || key == SDLK_DOWN) mGoingBack = false;
    else if (key == 'a' || key == SDLK_LEFT) mGoingLeft = false;
    else if (key == 'd' || key == SDLK_RIGHT) mGoingRight = false;
    else if (key == SDLK_PAGEUP) mGoingUp = false;
    else if (key == SDLK_PAGEDOWN) mGoingDown = false;
    else if (key == SDLK_LSHIFT) mFastMove = false;
  }

  return InputListener::keyReleased(evt);
}

Ogre::Real CameraMan::getDistToTarget()
{
  Ogre::Vector3 offset = mCamera->getPosition() - mTarget->_getDerivedPosition() - mOffset;
  return offset.length();
}

void CameraMan::update() {

  if (mOrbiting && mStyle == CS_ORBIT)   // yaw around the target, and pitch locally
  {
    auto oldRot = mCamera->getOrientation();
    mCamera->setPosition(mTarget->_getDerivedPosition());
    mCamera->translate(Ogre::Vector3(0, 0, dist_), Ogre::Node::TS_LOCAL);
  }
}

void CameraMan::setPivotOffset(const Ogre::Vector3& pivot)
{
  Ogre::Real dist = getDistToTarget();
  mOffset = pivot;
  mCamera->setPosition(mTarget->_getDerivedPosition() + mOffset);
  mCamera->translate(Ogre::Vector3(0, 0, dist), Ogre::Node::TS_LOCAL);
}

bool CameraMan::mouseMoved(const MouseMotionEvent &evt, bool midMousePressed)
{
  if (mStyle == CS_ORBIT)
  {
    if (mOrbiting)   // yaw around the target, and pitch locally
    {
      mCamera->setPosition(mTarget->_getDerivedPosition() + mOffset);
      yaw_ += Ogre::Degree(evt.xrel * 0.25f);
      pitch_ += Ogre::Degree(evt.yrel * 0.25f);

      setYawPitchDist(yaw_, pitch_, dist_);
      // don't let the camera go over the top or around the bottom of the target
    }
  }
  else if (mStyle == CS_FREELOOK)
  {
    if(!midMousePressed) {
      mCamera->yaw(Ogre::Degree(-evt.xrel * 0.15f), Ogre::Node::TS_PARENT);
      mCamera->pitch(Ogre::Degree(-evt.yrel * 0.15f));
    } else {
      mCamera->translate(Ogre::Vector3(evt.xrel*0.005f*getTopSpeed(), -evt.yrel*0.005f*getTopSpeed(), 0), Ogre::Node::TS_LOCAL);
    }

  }

  return InputListener::mouseMoved(evt);
}

bool CameraMan::mouseWheelRolled(const MouseWheelEvent &evt) {
  if (mStyle == CS_ORBIT && evt.y != 0)
  {
    if(evt.y > 0)
      dist_ /= 1.08;
    else
      dist_ *= 1.08;
    setYawPitchDist(yaw_, pitch_, dist_);
  }

  return InputListener::mouseWheelRolled(evt);
}

bool CameraMan::mousePressed(const MouseButtonEvent &evt)
{
  if (mStyle == CS_ORBIT)
  {
    if (evt.button == BUTTON_LEFT) mOrbiting = true;
    else if (evt.button == BUTTON_RIGHT) mMoving = true;
  }

  return InputListener::mousePressed(evt);
}

bool CameraMan::mouseReleased(const MouseButtonEvent &evt)
{
  if (mStyle == CS_ORBIT)
  {
    if (evt.button == BUTTON_LEFT) mOrbiting = true;
    else if (evt.button == BUTTON_RIGHT) mMoving = false;
  }

  return InputListener::mouseReleased(evt);
}

}
