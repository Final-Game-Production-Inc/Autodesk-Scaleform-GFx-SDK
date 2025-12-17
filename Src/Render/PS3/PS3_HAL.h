/**************************************************************************

Filename    :   PS3_HAL.h
Content     :   Renderer HAL Prototype header.
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_PS3_HAL_H
#define INC_SF_Render_PS3_HAL_H

#include "Render/Render_HAL.h"
#include "Render/PS3/PS3_MeshCache.h"
#include "Render/PS3/PS3_Shader.h"
#include "Render/PS3/PS3_Texture.h"
#include "Render/PS3/PS3_Sync.h"
#include "Render/Render_ShaderHAL.h"    // Must be included after platform specific shader includes.

#include <cell/gcm.h>

namespace Scaleform { namespace Render { namespace PS3 {

//------------------------------------------------------------------------

// HALConfigFlags enumeration defines system-specific HAL configuration
// flags passed into InitHAL though HALInitParams.
enum HALConfigFlags
{
    // No-wait texture updates. Application must call BeginFrame/EndFrame.
    // Glyph cache must be large enough for entire frame's usage.
    HALConfig_FastTexUpdate = 0x00000001,
};

// PS3::HALInitParems provides PS3-specific rendering initialization
// parameters for HAL::InitHAL.

struct HALInitParams : public Render::HALInitParams
{
    CellGcmContextData*     pGcm;
    UInt32                  Label;

    // HALInitParams constructs initialization 
    HALInitParams(CellGcmContextData* gcm,
                  Render::MemoryManager* memManager,
                  UInt32 halConfigFlags = 0,
                  ThreadId renderThreadId = ThreadId())
    : Render::HALInitParams(memManager, halConfigFlags, renderThreadId),
      pGcm(gcm), Label(250)
    { }

    // PS3::TextureManager accessors for correct type.
    void SetTextureManager(TextureManager* manager) { pTextureManager = manager; }
    TextureManager* GetTextureManager() const       { return (TextureManager*) pTextureManager.GetPtr(); }
};


//------------------------------------------------------------------------

class HAL : public Render::ShaderHAL<ShaderManager, ShaderInterface>
{
public:

    CellGcmContextData*      GcmCtx;
    unsigned                 VertexArrayUsed;

    RenderSync               RSync;
    MeshCache                Cache;

    FragShader               StaticFShaders[FragShaderDesc::FSI_Count];
    VertexShader             StaticVShaders[VertexShaderDesc::VSI_Count];
    Ptr<TextureManager>      pTextureManager;

    // Self-accessor used to avoid constructor warning.
    HAL*      getThis() { return this; }

public:    
    

    HAL(ThreadCommandQueue* commandQueue = 0);
    virtual ~HAL();
    
    // *** HAL Initialization / Shutdown Logic

    // Initializes HAL for rendering.
    virtual bool        InitHAL(const PS3::HALInitParams& params);
    // ShutdownHAL shuts down rendering, releasing resources allocated in InitHAL.
    virtual bool        ShutdownHAL();

    // *** Rendering

    virtual bool        BeginFrame();
    virtual bool        BeginScene();

    // Bracket the displaying of a frame from a movie.
    // Fill the background color, and set up default transforms, etc.
    virtual void        beginDisplay(BeginDisplayData* data);

    virtual void        SetUserMatrix(const Matrix& userMatrix);

    virtual void        updateViewport();
    void                setVertexArray (const VertexShader* pvs, const VertexFormat* pvf, int vertexLoc, int vertexBase);

    virtual void        DrawProcessedPrimitive(Primitive* pprimitive, PrimitiveBatch* pstart, PrimitiveBatch *pend);

    virtual void        DrawProcessedComplexMeshes(ComplexMesh* p,
        const StrideArray<HMatrix>& matrices);

    // *** Mask Support
    virtual void    PushMask_BeginSubmit(MaskPrimitive* primitive);    
    virtual void    EndMaskSubmit();
    virtual void    PopMask();

    virtual void    clearSolidRectangle(const Rect<int>& r, Color color);

    // *** BlendMode
    virtual void    applyBlendModeImpl(BlendMode mode, bool sourceAc = false, bool forceAc = false);



    virtual Render::TextureManager* GetTextureManager() const
    {
        return pTextureManager.GetPtr();
    }

    virtual RenderTarget*   CreateRenderTarget(CellGcmSurface surface, unsigned frameHeightIfStereo);
    virtual RenderTarget*   CreateRenderTarget(Render::Texture* texture, bool needsStencil);
    virtual RenderTarget*   CreateTempRenderTarget(const ImageSize& size, bool needsStencil);
    virtual bool            SetRenderTarget(RenderTarget* target, bool setState = 1);
    virtual void            PushRenderTarget(const RectF& frameRect, RenderTarget* prt, unsigned flags=0);
    virtual void            PopRenderTarget(unsigned flags = 0);

    virtual bool            createDefaultRenderBuffer();

    virtual void            SetStereoDisplay(StereoDisplay sDisplay, bool setstate = 0);

    // *** Filters
    virtual void          PushFilters(FilterPrimitive* primitive);
    virtual void          drawUncachedFilter(const FilterStackEntry& e);
    virtual void          drawCachedFilter(FilterPrimitive* primitive);

    virtual void          DrawablePaletteMap( Render::Texture** tex, const Matrix2F* texgen, const Matrix2F& mvp, 
                                              unsigned channelMask, const UInt32* values);

    virtual class MeshCache&    GetMeshCache()        { return Cache; }
    virtual Render::RenderSync* GetRenderSync() const { return (Render::RenderSync*)&RSync; }

    virtual void    MapVertexFormat(PrimitiveFillType fill, const VertexFormat* sourceFormat,
        const VertexFormat** single,
        const VertexFormat** batch, const VertexFormat** instanced, unsigned meshType);

protected:
    virtual void setBatchUnitSquareVertexStream();
    virtual void drawPrimitive(unsigned indexCount, unsigned meshCount);
    void drawIndexedPrimitive( unsigned indexCount, unsigned meshCount, UPInt indexOffset);

    // Simply sets a quad vertex buffer and draws.
    virtual void        drawScreenQuad();

    struct TemporaryTexture{  Ptr<Render::Texture> Tex; };
    typedef ArrayLH<TemporaryTexture, Stat_Default_Mem, NeverShrinkPolicy> TemporaryTextureListType;
    static const int            MaximumTempTextures = 32;   // Maximum number of temporary textures allowed before we wait on their destruction.
    TemporaryTextureListType    TemporaryTextureList;       // Holds a list of temporary textures (used in DrawablePaletteMap).
    Ptr<Fence>                  TemporaryTextureFence;      // Fence for the last usage of any temporary textures.

    // Destroy any temporary textures that are used. If there are too many temporary textures,
    // wait until they are used and destroy them.
    void                        destroyTemporaryTextures();

    // *** Events
    virtual Render::RenderEvent& GetEvent(EventType eventType);
};

//--------------------------------------------------------------------
// RenderTargetData, used for both RenderTargets and DepthStencilSurface implementations.
class RenderTargetData : public RenderBuffer::RenderTargetData
{
public:
    friend class HAL;

    CellGcmSurface          RenderSurface;
    UPInt                   ColorOffset;
    UPInt                   DepthOffset;
    unsigned                FrameHeightIfStereo;

    static void UpdateData(RenderBuffer* buffer, const CellGcmSurface& rt, DepthStencilBuffer* pdsb = 0, unsigned height = 0)
    {
        if ( !buffer )
            return;
        RenderTargetData* poldHD = (PS3::RenderTargetData*)buffer->GetRenderTargetData();
        if ( !poldHD )
        {
            poldHD = SF_NEW RenderTargetData(buffer, rt, pdsb, height);
            buffer->SetRenderTargetData(poldHD);
            return;
        }

        poldHD->pDepthStencilBuffer  = pdsb;
        poldHD->RenderSurface        = rt;
        poldHD->ColorOffset          = rt.colorOffset[0];
        poldHD->DepthOffset          = rt.depthOffset;
        poldHD->FrameHeightIfStereo  = height;
        poldHD->CacheID              = 0;
    }

private:

    RenderTargetData( RenderBuffer* buffer, const CellGcmSurface& rt, DepthStencilBuffer* pdsb = 0, unsigned height = 0) : 
        RenderBuffer::RenderTargetData(buffer, pdsb), RenderSurface(rt), FrameHeightIfStereo(height)
    {
        ColorOffset = rt.colorOffset[0];
        DepthOffset = rt.depthOffset;
    }
};


}}} // Scaleform::Render::PS3

#endif
