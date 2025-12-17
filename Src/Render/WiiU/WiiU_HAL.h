/**************************************************************************

Filename    :   WiiU_HAL.h
Content     :   Renderer HAL Prototype header.
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_WiiU_HAL_H
#define INC_SF_Render_WiiU_HAL_H

#include "Render/Render_HAL.h"
#include "WiiU_MeshCache.h"
#include "WiiU_Shader.h"
#include "WiiU_Texture.h"
#include "WiiU_Sync.h"
#include "Render/Render_ShaderHAL.h"    // Must be included after platform specific shader includes.

namespace Scaleform { namespace Render { namespace WiiU {

// WiiU::HALInitParams provides WiiU-specific rendering initialization
// parameters for HAL::InitHAL.

struct HALInitParams : public Render::HALInitParams
{
    GX2ContextState*        pContext;

    // HALInitParams constructs initialization 
    HALInitParams(GX2ContextState* pcs,
                  Render::MemoryManager* memManager,
                  UInt32 halConfigFlags = 0,
                  ThreadId renderThreadId = ThreadId())
        : Render::HALInitParams(memManager, halConfigFlags, renderThreadId),
        pContext(pcs)
    { }

    // WiiU::TextureManager accessors for correct type.
    void            SetTextureManager(TextureManager* manager) { pTextureManager = manager; }
    TextureManager* GetTextureManager() const       { return (TextureManager*) pTextureManager.GetPtr(); }
};

class MemoryManager : public Render::MemoryManager
{
public:
    virtual void*   Alloc(UPInt size, UPInt align, MemoryType type, unsigned arena = 0);
    virtual void    Free(void* pmem, UPInt size);
};

class HAL : public Render::ShaderHAL<ShaderManager, ShaderInterface>
{
public:
    struct StencilState
    {
        GX2Boolean         StencilEnable;
        GX2CompareFunction StencilFunc;
        GX2StencilFunction Pass, Fail;
        UByte              Ref;

        StencilState(GX2Boolean enable, UByte ref, GX2CompareFunction func = GX2_COMPARE_ALWAYS,
                     GX2StencilFunction pass = GX2_STENCIL_KEEP, GX2StencilFunction fail = GX2_STENCIL_KEEP)
            : StencilEnable(enable), StencilFunc(func), Pass(pass), Fail(fail), Ref(ref) {}

        StencilState() : StencilEnable(GX2_DISABLE), StencilFunc(GX2_COMPARE_ALWAYS), Pass(GX2_STENCIL_KEEP), Fail(GX2_STENCIL_KEEP) {}

        void Set() const
        {
            GX2SetDepthStencilControl(GX2_DISABLE, GX2_DISABLE, GX2_COMPARE_ALWAYS, StencilEnable, GX2_DISABLE,
                                      StencilFunc, Pass, Fail, Fail, StencilFunc, Pass, Fail, Fail);
            if (StencilEnable)
                GX2SetStencilMask(0xff, 0xff, Ref, 0xff, 0xff, Ref);
        }
    };

    GX2ContextState*            pContext;

    RenderSync                  RSync;
    MeshCache                   Cache;

    Ptr<TextureManager>         pTextureManager;
    Render::MemoryManager*      pMemManager;
    
    // Previous batching mode
    PrimitiveBatch::BatchType   PrevBatchType;

    StencilState                CurStencilState;

#if defined(SF_USE_GPU_PROFILER)
    unsigned                    CaptureIndex;
#endif
    // Self-accessor used to avoid constructor warning.
    HAL*      getThis() { return this; }

public:    
    

    HAL(ThreadCommandQueue* commandQueue = 0);
    virtual ~HAL();   

    // *** HAL Initialization and Shutdown

    // Initializes HAL for rendering.
    virtual bool        InitHAL(const WiiU::HALInitParams& params);

    // ShutdownHAL shuts down rendering, releasing resources allocated in InitHAL.
    virtual bool        ShutdownHAL();

    // *** Rendering
    virtual bool        BeginFrame();
    virtual bool        BeginScene();
    
    // Bracket the displaying of a frame from a movie.
    // Fill the background color, and set up default transforms, etc.
    virtual void        beginDisplay(BeginDisplayData* data);
#if defined(SF_USE_GPU_PROFILER)
    virtual void        endDisplay();
#endif

    // Updates HW Viewport and ViewportMatrix based on the current
    // values of VP, ViewRect and ViewportValid.
    virtual void        updateViewport();

    virtual void        DrawProcessedPrimitive(Primitive* pprimitive,
                                               PrimitiveBatch* pstart, PrimitiveBatch *pend);

    virtual void        DrawProcessedComplexMeshes(ComplexMesh* p,
                                                   const StrideArray<HMatrix>& matrices);
    
    // *** Mask Support
    virtual void    PushMask_BeginSubmit(MaskPrimitive* primitive);    
    virtual void    EndMaskSubmit();
    virtual void    PopMask();

    virtual void    clearSolidRectangle(const Rect<int>& r, Color color);

    // *** BlendMode
    virtual void       applyBlendModeImpl(BlendMode mode, bool sourceAc = false, bool forceAc = false);



    virtual Render::TextureManager* GetTextureManager() const
    {
        return pTextureManager.GetPtr();
    }

    virtual RenderTarget*   CreateRenderTarget(const GX2ColorBuffer& color, const GX2DepthBuffer& depth);
    virtual RenderTarget*   CreateRenderTarget(Render::Texture* texture, bool needsStencil);
    virtual RenderTarget*   CreateTempRenderTarget(const ImageSize& size, bool needsStencil);
    virtual bool            SetRenderTarget(RenderTarget* target, bool setState = 1);
    virtual void            PushRenderTarget(const RectF& frameRect, RenderTarget* prt, unsigned flags=0);
    virtual void            PopRenderTarget(unsigned flags = 0);

    virtual bool            createDefaultRenderBuffer() { return false; }

    // *** Filters
    virtual void          PushFilters(FilterPrimitive* prim);
    virtual void          drawUncachedFilter(const FilterStackEntry& e);
    virtual void          drawCachedFilter(FilterPrimitive* primitive);

    // *** DrawableImage
    virtual void            DrawablePaletteMap( Render::Texture** tex, const Matrix2F* texgen, const Matrix2F& mvp, 
                                                unsigned channelMask, const UInt32* values);

    virtual class MeshCache&        GetMeshCache()        { return Cache; }
    virtual Render::RenderSync*     GetRenderSync() const { return (Render::RenderSync*)&RSync; }
        
    virtual void    MapVertexFormat(PrimitiveFillType fill, const VertexFormat* sourceFormat,
                                    const VertexFormat** single,
                                    const VertexFormat** batch, const VertexFormat** instanced, unsigned meshType);

protected:
    virtual void setBatchUnitSquareVertexStream();
    virtual void drawPrimitive(unsigned indexCount, unsigned meshCount);
    void drawIndexedPrimitive( unsigned indexCount, unsigned meshCount, UByte* indexPtr);

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

};

//--------------------------------------------------------------------
// RenderTargetData, used for both RenderTargets and DepthStencilSurface implementations.
class RenderTargetData : public RenderBuffer::RenderTargetData
{
public:
    friend class HAL;

    GX2ColorBuffer ColorBuffer;
    GX2DepthBuffer DepthBuffer;

    static void UpdateData(RenderBuffer* buffer, const GX2ColorBuffer& cb, DepthStencilBuffer* pdsb)
    {
        if ( !buffer )
            return;

        RenderTargetData* poldHD = (WiiU::RenderTargetData*)buffer->GetRenderTargetData();
        if ( !poldHD )
        {
            poldHD = SF_NEW RenderTargetData(buffer, cb, pdsb);
            buffer->SetRenderTargetData(poldHD);
            return;
        }
        poldHD->ColorBuffer = cb;
        poldHD->pDepthStencilBuffer = pdsb;
        memset(&poldHD->DepthBuffer, 0, sizeof(poldHD->DepthBuffer));
    }
    static void UpdateData(RenderBuffer* buffer, const GX2ColorBuffer& cb, const GX2DepthBuffer& db)
    {
        if ( !buffer )
            return;

        RenderTargetData* poldHD = (WiiU::RenderTargetData*)buffer->GetRenderTargetData();
        if ( !poldHD )
        {
            poldHD = SF_NEW RenderTargetData(buffer, cb, db);
            buffer->SetRenderTargetData(poldHD);
            return;
        }
        poldHD->ColorBuffer = cb;
        poldHD->DepthBuffer = db;
        poldHD->pDepthStencilBuffer = 0;
    }

private:
    RenderTargetData( RenderBuffer* buffer, const GX2ColorBuffer& cb, DepthStencilBuffer* pdsb ) : 
       RenderBuffer::RenderTargetData(buffer, pdsb), ColorBuffer(cb)
    {
        memset(&DepthBuffer, 0, sizeof(DepthBuffer));
    }

   RenderTargetData( RenderBuffer* buffer, const GX2ColorBuffer& cb, const GX2DepthBuffer& db) : 
       RenderBuffer::RenderTargetData(buffer, 0), ColorBuffer(cb), DepthBuffer(db)
   { }
};


}}} // Scaleform::Render::WiiU

#endif
