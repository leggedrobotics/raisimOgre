#include <raisim/imgui.h>
#ifdef USE_FREETYPE
#include <imgui_freetype.h>
#endif

#include "raisim/ImguiRenderable.h"
#include "raisim/ImguiManager.h"

#include <OgreMaterialManager.h>
#include <OgreMesh.h>
#include <OgreMeshManager.h>
#include <OgreSubMesh.h>
#include <OgreTexture.h>
#include <OgreTextureManager.h>
#include <OgreString.h>
#include <OgreStringConverter.h>
#include <OgreViewport.h>
#include <OgreHighLevelGpuProgramManager.h>
#include <OgreHighLevelGpuProgram.h>
#include <OgreUnifiedHighLevelGpuProgram.h>
#include <OgreRoot.h>
#include <OgreTechnique.h>
#include <OgreTextureUnitState.h>
#include <OgreViewport.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreRenderTarget.h>

#ifdef OGRE_BUILD_COMPONENT_OVERLAY
#include <OgreFontManager.h>
#endif

using namespace Ogre;


template<> ImguiManager* Singleton<ImguiManager>::msSingleton = 0;

void ImguiManager::createSingleton()
{
    if(!msSingleton)
    {
        msSingleton = new ImguiManager();
    }
}
ImguiManager* ImguiManager::getSingletonPtr(void)
{
    createSingleton();
    return msSingleton;
}
ImguiManager& ImguiManager::getSingleton(void)
{  
    createSingleton();
    return ( *msSingleton );  
}

ImguiManager::ImguiManager()
:mSceneMgr(0)
,mLastRenderedFrame(-1)
{
    ImGui::CreateContext();
}
ImguiManager::~ImguiManager()
{
    ImGui::DestroyContext();
    mSceneMgr->removeRenderQueueListener(this);
}

// SDL2 keycode to scancode
static int kc2sc(int kc)
{
    return kc & ~(1 << 30);
}

void ImguiManager::init(Ogre::SceneManager * mgr)
{
    using namespace OgreBites;

    mSceneMgr  = mgr;

    mSceneMgr->addRenderQueueListener(this);
    ImGuiIO& io = ImGui::GetIO();

    // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array that we will update during the application lifetime.
    io.KeyMap[ImGuiKey_Tab] = '\t';
    io.KeyMap[ImGuiKey_LeftArrow] = kc2sc(SDLK_LEFT);
    io.KeyMap[ImGuiKey_RightArrow] = kc2sc(SDLK_RIGHT);
    io.KeyMap[ImGuiKey_UpArrow] = kc2sc(SDLK_UP);
    io.KeyMap[ImGuiKey_DownArrow] = kc2sc(SDLK_DOWN);
    io.KeyMap[ImGuiKey_PageUp] = kc2sc(SDLK_PAGEUP);
    io.KeyMap[ImGuiKey_PageDown] = kc2sc(SDLK_PAGEDOWN);
    io.KeyMap[ImGuiKey_Home] = -1;
    io.KeyMap[ImGuiKey_End] = -1;
    io.KeyMap[ImGuiKey_Delete] = -1;
    io.KeyMap[ImGuiKey_Backspace] = '\b';
    io.KeyMap[ImGuiKey_Enter] = '\r';
    io.KeyMap[ImGuiKey_Escape] = '\033';
    io.KeyMap[ImGuiKey_Space] = ' ';
    io.KeyMap[ImGuiKey_A] = 'a';
    io.KeyMap[ImGuiKey_C] = 'c';
    io.KeyMap[ImGuiKey_V] = 'v';
    io.KeyMap[ImGuiKey_X] = 'x';
    io.KeyMap[ImGuiKey_Y] = 'y';
    io.KeyMap[ImGuiKey_Z] = 'z';

    createFontTexture();
    createMaterial();
}

bool ImguiManager::mouseWheelRolled(const OgreBites::MouseWheelEvent& arg)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheel = Ogre::Math::Sign(arg.y);
    return io.WantCaptureMouse;
}

bool ImguiManager::mouseMoved( const OgreBites::MouseMotionEvent &arg )
{

    ImGuiIO& io = ImGui::GetIO();

    io.MousePos.x = arg.x;
    io.MousePos.y = arg.y;

    return io.WantCaptureMouse;
}

// map sdl2 mouse buttons to imgui
static int sdl2imgui(int b)
{
    switch(b) {
    case 2:
        return 2;
    case 3:
        return 1;
    default:
        return b - 1;
    }
}

bool ImguiManager::mousePressed( const OgreBites::MouseButtonEvent &arg)
{
    ImGuiIO& io = ImGui::GetIO();
    int b = sdl2imgui(arg.button);
    if(b<5)
    {
        io.MouseDown[b] = true;
    }
    return io.WantCaptureMouse;
}
bool ImguiManager::mouseReleased( const OgreBites::MouseButtonEvent &arg)
{
    ImGuiIO& io = ImGui::GetIO();
    int b = sdl2imgui(arg.button);
    if(b<5)
    {
        io.MouseDown[b] = false;
    }
    return io.WantCaptureMouse;
}
bool ImguiManager::keyPressed( const OgreBites::KeyboardEvent &arg )
{
    using namespace OgreBites;

    ImGuiIO& io = ImGui::GetIO();
    
    io.KeyCtrl = arg.keysym.mod & KMOD_CTRL;
    io.KeyShift = arg.keysym.sym == SDLK_LSHIFT;

    int key = kc2sc(arg.keysym.sym);

    if(key > 0 && key < 512)
    {
        io.KeysDown[key] = true;
        io.AddInputCharacter((unsigned short)arg.keysym.sym);
    }

    return io.WantCaptureKeyboard;
}
bool ImguiManager::keyReleased( const OgreBites::KeyboardEvent &arg )
{
    int key = kc2sc(arg.keysym.sym);
    if(key < 0 || key >= 512)
        return true;

    ImGuiIO& io = ImGui::GetIO();
    io.KeysDown[key] = false;
    return io.WantCaptureKeyboard;
}
//-----------------------------------------------------------------------------------
void ImguiManager::renderQueueEnded(uint8 queueGroupId, const String& invocation,bool& repeatThisInvocation)
{
    if((queueGroupId != Ogre::RENDER_QUEUE_OVERLAY) || (invocation == "SHADOWS"))
    {
        return;
    }
    Ogre::RenderSystem* renderSys = Ogre::Root::getSingletonPtr()->getRenderSystem();
    Ogre::Viewport* vp = renderSys->_getViewport();
    
    if (!vp || (!vp->getTarget()->isPrimary()) || mFrameEnded)
    {
        return;
    }
    
    mFrameEnded = true;
    ImGuiIO& io = ImGui::GetIO();
    
    // Construct projection matrix, taking texel offset corrections in account (important for DirectX9)
    // See also:
    //     - OGRE-API specific hint: http://www.ogre3d.org/forums/viewtopic.php?f=5&p=536881#p536881
    //     - IMGUI Dx9 demo solution: https://github.com/ocornut/imgui/blob/master/examples/directx9_example/imgui_impl_dx9.cpp#L127-L138
    const float texelOffsetX = renderSys->getHorizontalTexelOffset();
    const float texelOffsetY = renderSys->getVerticalTexelOffset();
    const float L = texelOffsetX;
    const float R = io.DisplaySize.x + texelOffsetX;
    const float T = texelOffsetY;
    const float B = io.DisplaySize.y + texelOffsetY;
    
    mRenderable.mXform = Matrix4(   2.0f/(R-L),    0.0f,         0.0f,       (L+R)/(L-R),
                                    0.0f,         -2.0f/(B-T),   0.0f,       (T+B)/(B-T),
                                    0.0f,          0.0f,        -1.0f,       0.0f,
                                    0.0f,          0.0f,         0.0f,       1.0f);
    
    // Instruct ImGui to Render() and process the resulting CmdList-s
    /// Adopted from https://bitbucket.org/ChaosCreator/imgui-ogre2.1-binding
    /// ... Commentary on OGRE forums: http://www.ogre3d.org/forums/viewtopic.php?f=5&t=89081#p531059
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    int vpWidth  = vp->getActualWidth();
    int vpHeight = vp->getActualHeight();
    for (int i = 0; i < draw_data->CmdListsCount; ++i)
    {
        const ImDrawList* draw_list = draw_data->CmdLists[i];
        unsigned int startIdx = 0;

        for (int j = 0; j < draw_list->CmdBuffer.Size; ++j)
        {
            // Create a renderable and fill it's buffers
            const ImDrawCmd *drawCmd = &draw_list->CmdBuffer[j];
            mRenderable.updateVertexData(draw_list->VtxBuffer.Data, &draw_list->IdxBuffer.Data[startIdx], draw_list->VtxBuffer.Size, drawCmd->ElemCount);

            // Set scissoring
            int scLeft   = static_cast<int>(drawCmd->ClipRect.x); // Obtain bounds
            int scTop    = static_cast<int>(drawCmd->ClipRect.y);
            int scRight  = static_cast<int>(drawCmd->ClipRect.z);
            int scBottom = static_cast<int>(drawCmd->ClipRect.w);

            scLeft   = scLeft   < 0 ? 0 : (scLeft  > vpWidth ? vpWidth : scLeft); // Clamp bounds to viewport dimensions
            scRight  = scRight  < 0 ? 0 : (scRight > vpWidth ? vpWidth : scRight);
            scTop    = scTop    < 0 ? 0 : (scTop    > vpHeight ? vpHeight : scTop);
            scBottom = scBottom < 0 ? 0 : (scBottom > vpHeight ? vpHeight : scBottom);

            Pass * pass = mRenderable.mMaterial->getBestTechnique()->getPass(0);
            TextureUnitState * st = pass->getTextureUnitState(0);
            if (drawCmd->TextureId != 0)
            {
                Ogre::ResourceHandle handle = (Ogre::ResourceHandle)drawCmd->TextureId;
                Ogre::TexturePtr tex = Ogre::static_pointer_cast<Ogre::Texture>(
                    Ogre::TextureManager::getSingleton().getByHandle(handle));
                if (tex)
                {
                    st->setTexture(tex);
                    st->setTextureFiltering(Ogre::TFO_TRILINEAR);
                }
            }
            else
            {
                st->setTexture(mFontTex);
                st->setTextureFiltering(Ogre::TFO_NONE);
            }
            renderSys->setScissorTest(true, scLeft, scTop, scRight, scBottom);

            // Render!
            mSceneMgr->_injectRenderWithPass(pass,
                                             &mRenderable, false);

            // Update counts
            startIdx += drawCmd->ElemCount;
        }
    }
    renderSys->setScissorTest(false);
}
//-----------------------------------------------------------------------------------
void ImguiManager::createMaterial()
{
    mRenderable.mMaterial = dynamic_pointer_cast<Material>(
        MaterialManager::getSingleton()
            .createOrRetrieve("imgui/material", ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME)
            .first);
    Pass* mPass = mRenderable.mMaterial->getTechnique(0)->getPass(0);
    mPass->setCullingMode(CULL_NONE);
    mPass->setDepthFunction(Ogre::CMPF_ALWAYS_PASS);
    mPass->setLightingEnabled(false);
    mPass->setVertexColourTracking(TVC_DIFFUSE);
    mPass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
    mPass->setSeparateSceneBlendingOperation(Ogre::SBO_ADD,Ogre::SBO_ADD);
    mPass->setSeparateSceneBlending(Ogre::SBF_SOURCE_ALPHA,Ogre::SBF_ONE_MINUS_SOURCE_ALPHA,Ogre::SBF_ONE_MINUS_SOURCE_ALPHA,Ogre::SBF_ZERO);
        
    mTexUnit =  mPass->createTextureUnitState();
    mTexUnit->setTexture(mFontTex);
    mTexUnit->setTextureFiltering(Ogre::TFO_NONE);

    mRenderable.mMaterial->load();
}

ImFont* ImguiManager::addFont(const String& name, const String& group)
{
#ifdef OGRE_BUILD_COMPONENT_OVERLAY
    FontPtr font = FontManager::getSingleton().getByName(name, group);
    OgreAssert(font, "font does not exist");
    OgreAssert(font->getType() == FT_TRUETYPE, "font must be of FT_TRUETYPE");
    DataStreamPtr dataStreamPtr =
        ResourceGroupManager::getSingleton().openResource(font->getSource(), font->getGroup());
    MemoryDataStream ttfchunk(dataStreamPtr, false); // transfer ownership to imgui

    ImGuiIO& io = ImGui::GetIO();
    return io.Fonts->AddFontFromMemoryTTF(ttfchunk.getPtr(), ttfchunk.size(), font->getTrueTypeSize());
#else
    OGRE_EXCEPT(Exception::ERR_INVALID_CALL, "Ogre Overlay Component required");
    return NULL;
#endif
}

void ImguiManager::createFontTexture()
{
    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();
    if(io.Fonts->Fonts.empty())
        io.Fonts->AddFontDefault();
#ifdef USE_FREETYPE
    ImGuiFreeType::BuildFontAtlas(io.Fonts, 0);
#endif

    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    mFontTex = TextureManager::getSingleton().createManual(
        "ImguiFontTex", Ogre::ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME, TEX_TYPE_2D,
        width, height, 1, 1, PF_BYTE_RGBA);

    mFontTex->getBuffer()->blitFromMemory(PixelBox(Box(0, 0, width, height), PF_BYTE_RGBA, pixels));    
}
void ImguiManager::newFrame(float deltaTime,const Ogre::Rect & windowRect)
{
    mFrameEnded=false;
    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = deltaTime;

     // Read keyboard modifiers inputs
    io.KeyAlt = false;// mKeyInput->isKeyDown(OIS::KC_LMENU);
    io.KeySuper = false;

    // Setup display size (every frame to accommodate for window resizing)
     io.DisplaySize = ImVec2((float)(windowRect.right - windowRect.left), (float)(windowRect.bottom - windowRect.top));


    // Start the frame
    ImGui::NewFrame();
}
