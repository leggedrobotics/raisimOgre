#pragma once

#include <raisim/imgui.h>
#include <OgreCommon.h>

#include <OgreRenderQueueListener.h>
#include <OgreSingleton.h>
#include <OgreTexture.h>
#include <OgreInput.h>
#include <OgreResourceGroupManager.h>

#include "ImguiRenderable.h"

namespace Ogre
{
    class SceneManager;
    class TextureUnitState;

    class ImguiManager : public RenderQueueListener,public OgreBites::InputListener, public Singleton<ImguiManager>
    {
    public:
        static void createSingleton();

        ImguiManager();
        ~ImguiManager();

        /// add font from ogre .fontdef file
        /// must be called before init()
        ImFont* addFont(const String& name, const String& group OGRE_RESOURCE_GROUP_INIT);

        virtual void init(Ogre::SceneManager* mgr);

        virtual void newFrame(float deltaTime,const Ogre::Rect & windowRect);

        //inherited from RenderQueueListener
        virtual void renderQueueEnded(uint8 queueGroupId, const String& invocation,bool& repeatThisInvocation);

        virtual bool mouseWheelRolled(const OgreBites::MouseWheelEvent& arg);
        virtual bool mouseMoved( const OgreBites::MouseMotionEvent &arg );
		virtual bool mousePressed( const OgreBites::MouseButtonEvent &arg);
		virtual bool mouseReleased( const OgreBites::MouseButtonEvent &arg);
		virtual bool keyPressed( const OgreBites::KeyboardEvent &arg );
		virtual bool keyReleased( const OgreBites::KeyboardEvent &arg );

        /** Override standard Singleton retrieval.
        @remarks
        Why do we do this? Well, it's because the Singleton
        implementation is in a .h file, which means it gets compiled
        into anybody who includes it. This is needed for the
        Singleton template to work, but we actually only want it
        compiled into the implementation of the class based on the
        Singleton, not all of them. If we don't change this, we get
        link errors when trying to use the Singleton-based class from
        an outside dll.
        @par
        This method just delegates to the template version anyway,
        but the implementation stays in this single compilation unit,
        preventing link errors.
        */
        static ImguiManager& getSingleton(void);
        /** Override standard Singleton retrieval.
        @remarks
        Why do we do this? Well, it's because the Singleton
        implementation is in a .h file, which means it gets compiled
        into anybody who includes it. This is needed for the
        Singleton template to work, but we actually only want it
        compiled into the implementation of the class based on the
        Singleton, not all of them. If we don't change this, we get
        link errors when trying to use the Singleton-based class from
        an outside dll.
        @par
        This method just delegates to the template version anyway,
        but the implementation stays in this single compilation unit,
        preventing link errors.
        */
        static ImguiManager* getSingletonPtr(void);

    protected:

        void createFontTexture();
        void createMaterial();

        SceneManager*				mSceneMgr;
        TextureUnitState*           mTexUnit;
        int                         mLastRenderedFrame;

        ImGUIRenderable             mRenderable;
        TexturePtr                  mFontTex;

        bool                        mFrameEnded;
    };
}
