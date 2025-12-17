/**************************************************************************

Filename    :   Wii_HAL.h
Content     :   Renderer HAL Prototype header.
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_Wii_HAL_H
#define INC_SF_Render_Wii_HAL_H

#include "Render/Render_HAL.h"
#include "Wii_MeshCache.h"
#include "Wii_Shader.h"
#include "Wii_Texture.h"

#include <revolution/gx.h>

namespace Scaleform { namespace Render { namespace Wii {

//------------------------------------------------------------------------
enum HALConfigFlags
{
    // No-wait texture updates. Application must call BeginFrame/EndFrame.
    // Glyph cache must be large enough for entire frame's usage.
    HALConfig_FastTexUpdate = 0x00000001,
};

// Wii::HALInitParems provides Wii-specific rendering initialization
// parameters for HAL::InitHAL.

struct HALInitParams : public Render::HALInitParams
{
    GXRenderModeObj *pRMode;
    RenderSync*      pRSync;    

    // Base class only
    HALInitParams(GXRenderModeObj *prmode,
                  RenderSync* prsync,
                  Render::MemoryManager* pmemManager,
                  UInt32 halConfigFlags = 0,
                  ThreadId renderThreadId = ThreadId())
    : pRMode(prmode), pRSync(prsync),
      Render::HALInitParams(pmemManager, halConfigFlags, renderThreadId)
    { }

    // Wii::TextureManager accessors for correct type.
    void SetTextureManager(TextureManager* manager)
    { pTextureManager = manager; }
    TextureManager* GetTextureManager() const
    { return (TextureManager*) pTextureManager.GetPtr(); }
};

struct RenderTargetParams
{
    GXPixelFmt                  FbMode;
    GXZFmt16                    FbZMode;
    GXRenderModeObj*            pRMode;
};

class RenderTargetData : public RenderBuffer::RenderTargetData
{
public:
    RenderTargetParams  RT;
    Texture*            pTexture;

    static void UpdateData(RenderBuffer* buffer, RenderTargetParams p, Texture* pt = 0)
    {
        if ( !buffer )
            return;
        RenderTargetData* poldHD = (Wii::RenderTargetData*)buffer->GetRenderTargetData();
        if ( !poldHD )
        {
            poldHD = SF_NEW RenderTargetData(buffer, p, pt);
            buffer->SetRenderTargetData(poldHD);
            return;
        }
        poldHD->RT       = p;
        poldHD->pTexture = pt;
        poldHD->CacheID  = 0;
    }

private:
    RenderTargetData(RenderBuffer* buffer, RenderTargetParams p, Texture* pt = 0) : 
       RenderBuffer::RenderTargetData(buffer, 0), RT(p), pTexture(pt) {}
};

class MatrixState : public Render::MatrixState
{
public:
    MatrixState(HAL* phal) : Render::MatrixState((Render::HAL*)phal)
    { }

    MatrixState() : Render::MatrixState()
    { }

protected:
    virtual void        recalculateUVPOC() const;
};

//------------------------------------------------------------------------

class HAL : public Render::HAL
{
    friend class ShaderManager;

public:

    enum
    {
        MaxEraseBatchCount = 10
    };

    // Output size.
    GXRenderModeObj*    pRMode;
    int                 ProjMatrixSet;
    Matrix4F            ProjMatrix2D;

    MeshCache           Cache;

    ShaderManager       SManager;
    Ptr<TextureManager> pTextureManager;

    UByte*              pDummyDList;

    // Previous batching mode
    PrimitiveBatch::BatchType PrevBatchType;

    bool                ZTest, ZWrite;
    GXCompare           ZCompare;

    // Self-accessor used to avoid constructor warning.
    HAL*      GetThis() { return this; }

public:    


    HAL(ThreadCommandQueue* commandQueue = 0);
    virtual ~HAL();

    // *** HAL Initialization and Shutdown

    // Initializes HAL for rendering.
    virtual bool        InitHAL(const Wii::HALInitParams& params);
    // ShutdownHAL shuts down rendering, releasing resources allocated in InitHAL.
    virtual bool        ShutdownHAL();

    // *** Rendering
    virtual bool        BeginScene();

    // Wii requires a prepass rendering step, because there is only one framebuffer available.
    virtual bool        IsPrepassRequired() const { return true; }

    virtual void        SetUserMatrix(const Matrix& userMatrix);

    virtual void        updateViewport();

    // Creates / Destroys mesh and DP data 

    virtual PrimitiveFill*  CreatePrimitiveFill(const PrimitiveFillData& data);    

    virtual void        DrawProcessedPrimitive(Primitive* pprimitive, PrimitiveBatch* pstart, PrimitiveBatch *pend);

    virtual void        DrawProcessedComplexMeshes(ComplexMesh* p,
        const StrideArray<HMatrix>& matrices);

    // *** Mask Support
    virtual void    PushMask_BeginSubmit(MaskPrimitive* primitive);    
    virtual void    EndMaskSubmit();
    virtual void    PopMask();

    void            drawMaskClearRectangles(const HMatrix* matrices, UPInt count);
    virtual void    clearSolidRectangle(const Rect<int>& r, Color color);

    void   setProj2D()
    {
        if (ProjMatrixSet != 2)
        {
            GXSetProjection(ProjMatrix2D.M, GX_ORTHOGRAPHIC);
            ProjMatrixSet = 2;
        }
    }

    int    getMaskZ() const { return -MaskStackTop; }

    // *** BlendMode
    void            applyBlendModeImpl(BlendMode mode, bool forceAc = false, bool sourceAc = false);

    virtual Render::TextureManager* GetTextureManager() const
    {
        return pTextureManager.GetPtr();
    }

    virtual RenderTarget*   CreateRenderTarget(RenderTargetParams params);
    virtual RenderTarget*   CreateRenderTarget(Render::Texture* texture, bool needsStencil);
    virtual RenderTarget*   CreateTempRenderTarget(const ImageSize& size, bool needsStencil);
    virtual bool            SetRenderTarget(RenderTarget* target, bool setState = 1);
    virtual void            PushRenderTarget(const RectF& frameRect, RenderTarget* prt, unsigned flags=0);
    virtual void            PopRenderTarget(unsigned flags = 0);

    virtual bool            createDefaultRenderBuffer();

    void                    copyRTTexture(RenderTarget* prt);
    void                    clearBuffer(RenderTargetData* phd);

    // *** Filters
    virtual void            PushFilters(FilterPrimitive* primitive);
    virtual void            PopFilters();
    virtual void            drawUncachedFilter(const FilterStackEntry& e);
    virtual void            drawCachedFilter(FilterPrimitive* primitive);


    virtual class MeshCache&       GetMeshCache()        { return Cache; }

    virtual void    MapVertexFormat(PrimitiveFillType fill, const VertexFormat* sourceFormat,
        const VertexFormat** single,
        const VertexFormat** batch, const VertexFormat** instanced, unsigned meshType);

protected:
    virtual void drawScreenQuad();

};

inline void ShaderManager::SetMatrix(const Shader& sd, unsigned var, Matrix2F m, unsigned index, unsigned batch)
{
    float rows[3][4];
    m.GetAsFloat2x4(rows);

    if (var != Uniform::SU_mvp)
        GXLoadTexMtxImm(rows, GX_PNMTX1 + index * 2 + batch * sd.BatchMatrixSize, GX_MTX2x4);
    else
    {
        int maskZ = pHal->MaskStackTop;
        rows[2][0] = rows[2][1] = 0;
        rows[2][2] = 0;
        rows[2][3] = (-1-maskZ) * 10;

        SF_ASSERT(index == 0);
        GXLoadPosMtxImm(rows, GX_PNMTX0 + batch * sd.BatchMatrixSize);
    }
}

inline void ShaderManager::SetMatrix(const Shader& sd, unsigned var, Matrix2F m1, HMatrix m2, const Render::MatrixState* halMatrices,
                                     unsigned index, unsigned batch)
{
    const MatrixState* matrices = (const MatrixState*)halMatrices;
    if (m2.Has3D())
    {
        SF_ASSERT(index == 0);
        SF_ASSERT(var == Uniform::SU_mvp);

        Matrix3F m(matrices->View3D, Matrix3F(m2.GetMatrix3D(), m1));
        int maskZ = pHal->MaskStackTop;
        GXLoadPosMtxImm(m.M, GX_PNMTX0 + batch * sd.BatchMatrixSize);

        if (matrices->UVPOChanged || pHal->ProjMatrixSet != 3)
        {
            matrices->GetUVP();
            pHal->ProjMatrixSet = 3;
            matrices->UVPO.M[2][0] = matrices->UVPO.M[2][1] = matrices->UVPO.M[2][3] = 0;
            matrices->UVPO.M[2][2] = - (-1-maskZ) * (10.0f/255.0f);
            GXSetProjection(matrices->GetUVP().M, GX_PERSPECTIVE);
        } 
    }
    else
    {
        Matrix2F  m(m1);
        m.Append(m2.GetMatrix2D());
        m.Append(matrices->UserView);

        SetMatrix(sd, var, m, index, batch);

        if (pHal->ProjMatrixSet != 2)
        {
            pHal->ProjMatrixSet = 2;
            GXSetProjection(pHal->ProjMatrix2D.M, GX_ORTHOGRAPHIC);
        }
    }
}

}}} // Scaleform::Render::Wii

#endif
