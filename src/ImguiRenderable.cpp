/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

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
-----------------------------------------------------------------------------
*/
#include <raisim/imgui.h>

#include "raisim/ImguiRenderable.h"

#include "OgreHardwareBufferManager.h"
#include "OgreMaterialManager.h"
#include <OgreSceneManager.h>
#include <OgreHardwareVertexBuffer.h>
#include <OgreHardwareIndexBuffer.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreRoot.h>


namespace Ogre
{
    ImGUIRenderable::ImGUIRenderable():
    mVertexBufferSize(5000)
    ,mIndexBufferSize(10000)
    {
        initImGUIRenderable();

        //By default we want ImGUIRenderables to still work in wireframe mode
        setPolygonModeOverrideable( false );
        
    }
    //-----------------------------------------------------------------------------------
    void ImGUIRenderable::initImGUIRenderable(void)
    {
        // use identity projection and view matrices
        mUseIdentityProjection  = true;
        mUseIdentityView        = true;

        mRenderOp.vertexData = OGRE_NEW VertexData();
        mRenderOp.indexData  = OGRE_NEW IndexData();

        mRenderOp.vertexData->vertexCount   = 0;
        mRenderOp.vertexData->vertexStart   = 0;

        mRenderOp.indexData->indexCount = 0;
        mRenderOp.indexData->indexStart = 0;
        mRenderOp.operationType             = RenderOperation::OT_TRIANGLE_LIST;
        mRenderOp.useIndexes                                    = true; 
        mRenderOp.useGlobalInstancingVertexBufferIsAvailable    = false;

        VertexDeclaration* decl     = mRenderOp.vertexData->vertexDeclaration;
        
        // vertex declaration
        size_t offset = 0;
        decl->addElement(0,offset,Ogre::VET_FLOAT2,Ogre::VES_POSITION);
        offset += VertexElement::getTypeSize( VET_FLOAT2 );
        decl->addElement(0,offset,Ogre::VET_FLOAT2,Ogre::VES_TEXTURE_COORDINATES,0);
        offset += VertexElement::getTypeSize( VET_FLOAT2 );
        decl->addElement(0,offset,Ogre::VET_COLOUR,Ogre::VES_DIFFUSE);
    }
    //-----------------------------------------------------------------------------------
    ImGUIRenderable::~ImGUIRenderable()
    {
        OGRE_DELETE mRenderOp.vertexData;
        OGRE_DELETE mRenderOp.indexData;
    }
    //-----------------------------------------------------------------------------------
    void ImGUIRenderable::updateVertexData(const ImDrawVert* vtxBuf, const ImDrawIdx* idxBuf, unsigned int vtxCount, unsigned int idxCount)
    {
        Ogre::VertexBufferBinding* bind = mRenderOp.vertexData->vertexBufferBinding;

        if (bind->getBindings().empty() || mVertexBufferSize != vtxCount)
        {
	        mVertexBufferSize = vtxCount;

	        bind->setBinding(0, Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(sizeof(ImDrawVert), mVertexBufferSize, Ogre::HardwareBuffer::HBU_WRITE_ONLY));
        }
        if (!mRenderOp.indexData->indexBuffer || mIndexBufferSize != idxCount)
        {
	        mIndexBufferSize = idxCount;
            
	        mRenderOp.indexData->indexBuffer =
		        Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(Ogre::HardwareIndexBuffer::IT_16BIT, mIndexBufferSize, Ogre::HardwareBuffer::HBU_WRITE_ONLY);
        }
      
        // Copy all vertices
        ImDrawVert* vtxDst = (ImDrawVert*)(bind->getBuffer(0)->lock(Ogre::HardwareBuffer::HBL_DISCARD));
        ImDrawIdx* idxDst = (ImDrawIdx*)(mRenderOp.indexData->indexBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD));
          
        memcpy(vtxDst, vtxBuf, mVertexBufferSize * sizeof(ImDrawVert));
        memcpy(idxDst, idxBuf, mIndexBufferSize * sizeof(ImDrawIdx));
         
        mRenderOp.vertexData->vertexStart = 0;
        mRenderOp.vertexData->vertexCount = vtxCount;
        mRenderOp.indexData->indexStart = 0;
        mRenderOp.indexData->indexCount = idxCount;


        bind->getBuffer(0)->unlock();
        mRenderOp.indexData->indexBuffer->unlock();
    }
    //-----------------------------------------------------------------------------------
    const LightList& ImGUIRenderable::getLights(void) const
    {
        static const LightList l;
        return l;
    }
}
