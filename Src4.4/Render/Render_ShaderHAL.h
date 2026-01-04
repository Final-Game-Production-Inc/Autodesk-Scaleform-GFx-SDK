/**********************************************************************

PublicHeader:   Render
Filename    :   Render_ShaderHAL.h
Content     :   Provides common functionality for HALs which have ShaderManagers
                as their primary rendering pipelines
Created     :   2012/09/18
Authors     :   Bart Muzzin

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#ifndef INC_SF_Render_ShaderHAL_H
#define INC_SF_Render_ShaderHAL_H

#include "Render/Render_HAL.h"

namespace Scaleform { namespace Render {

template <class ShaderManagerType, class ShaderInterfaceType>
class ShaderHAL : public HAL
{
public:
    typedef typename ShaderManagerType::Shader Shader;

    ShaderHAL(ThreadCommandQueue *commandQueue);

    // *** Initialization/Shutdown
    virtual bool InitHAL(const HALInitParams& params);
    virtual bool ShutdownHAL();

    // ** Scene Management
    virtual bool BeginScene();
    virtual bool EndScene();

    // ** General
    virtual unsigned GetMaximumBatchCount(Primitive* prim);

    // *** BlendMode
    virtual void PushBlendMode(BlendPrimitive* prim);
    virtual void PopBlendMode();
    // *** Drawing
    virtual void ClearSolidRectangle(const Rect<int>& r, Color color, bool blend);
    virtual void DrawProcessedPrimitive(Primitive* pprimitive, PrimitiveBatch* pstart, PrimitiveBatch *pend);
    virtual void DrawProcessedComplexMeshes(ComplexMesh* p, const StrideArray<HMatrix> &matrices);

    // *** DrawableImage
    virtual void DrawableCxform( Render::Texture** tex, const Matrix2F* texgen, const Cxform* cx);
    virtual void DrawableCompare( Render::Texture** tex, const Matrix2F* texgen );
    virtual void DrawableCopyChannel( Render::Texture** tex, const Matrix2F* texgen, const Matrix4F* cxmul ) 
    { DrawableMerge(tex, texgen, cxmul); }
    virtual void DrawableMerge( Render::Texture** tex, const Matrix2F* texgen, const Matrix4F* cxmul );
    virtual void DrawableCopyPixels( Render::Texture** tex, const Matrix2F* texgen, const Matrix2F& mvp, bool mergeAlpha, bool destAlpha );
    virtual void DrawablePaletteMap( Render::Texture** tex, const Matrix2F* texgen, const Matrix2F& mvp, unsigned channelMask, const UInt32* values);
    virtual void DrawableCopyback( Render::Texture* tex, const Matrix2F& mvp, const Matrix2F& texgen, unsigned flagMask = 0xFFFFFFFF );

    ShaderManagerType            SManager;      // Holds shader code, responsible applying ShaderInterace data to the pipeline.
    ShaderInterfaceType          ShaderData;    // Holds data about currently executing pipeline (shaders, uniforms, etc).

protected:

    // NOTE: 'PerDraw' functions do nothing, except on ORBIS LCUE, because the vertex streams need to be set everytime a shader is set,
    // however, on other platforms this would be inefficient. setVertexArrayPerDraw assumes that the index buffer offset will not change
    // from the original setVertexArray call, and thus does not need to be returned.
    virtual void  setBatchUnitSquareVertexStream() = 0;
    virtual void  setBatchUnitSquareVertexStreamPerDraw() { }
    virtual UPInt setVertexArray(PrimitiveBatch* pbatch, MeshCacheItem* pmesh) = 0;
    virtual UPInt setVertexArray(const ComplexMesh::FillRecord& fr, unsigned formatIndex, MeshCacheItem* pmesh) = 0;
    virtual void  setVertexArrayPerDraw(PrimitiveBatch* pbatch, MeshCacheItem* pmesh) { SF_UNUSED2(pbatch, pmesh); }
    virtual void  setVertexArrayPerDraw(const ComplexMesh::FillRecord& fr, unsigned formatIndex, MeshCacheItem* pmesh) { SF_UNUSED3(fr, formatIndex, pmesh); }
    virtual void  setLinearStreamSource(PrimitiveBatch::BatchType ) { }
    virtual void  setInstancedStreamSource(UPInt, UPInt) { }

    virtual void drawPrimitive(unsigned indexCount, unsigned meshCount) = 0;
    virtual void drawIndexedPrimitive(unsigned indexCount, unsigned vertexCount, unsigned meshCount, UPInt indexPtr, UPInt vertexOffset ) = 0;
    virtual void drawIndexedInstanced(unsigned indexCount, unsigned vertexCount, unsigned meshCount, UPInt indexPtr, UPInt vertexOffset ) = 0;
    virtual void drawScreenQuad();

    virtual void drawMaskClearRectangles(const Matrix2F* matrices, UPInt count);
    virtual void drawFilter(const Matrix2F& mvp, const Cxform & cx, const Filter* filter, Ptr<RenderTarget> * targets, 
                            unsigned* shaders, unsigned pass, unsigned passCount, VertexFormat* pvf);

    // *** Filters
    virtual void drawUncachedFilter(const FilterStackEntry&);
    virtual void drawCachedFilter(FilterPrimitive* primitive);
    virtual void drawBlendPrimitive(BlendPrimitive* prim, Render::Texture* blendSource, Render::Texture* blendDest, Render::Texture* blendAlpha);

    // *** Profiler
    virtual void profilerDrawCacheablePrimArea(const CacheablePrimitive* prim);
    virtual void profilerApplyUniform(ProfilerUniform uniform, unsigned components, float* values);

    // Cached mappings of VertexXY16iAlpha vertex format.
    VertexFormat* MappedXY16iAlphaTexture[PrimitiveBatch::DP_DrawableCount];
    VertexFormat* MappedXY16iAlphaSolid[PrimitiveBatch::DP_DrawableCount];
};

//------------------------------------------------------------------------------------------------------------
template<class ShaderManagerType, class ShaderInterfaceType>
inline ShaderHAL<ShaderManagerType, ShaderInterfaceType>::ShaderHAL(ThreadCommandQueue *commandQueue) : 
    HAL(commandQueue),
    SManager(&Profiler),
    ShaderData(getThis()) 
{
    memset(MappedXY16iAlphaTexture, 0, sizeof(MappedXY16iAlphaTexture));
    memset(MappedXY16iAlphaSolid, 0, sizeof(MappedXY16iAlphaSolid));
}

template<class ShaderManagerType, class ShaderInterfaceType>
inline bool ShaderHAL<ShaderManagerType, ShaderInterfaceType>::InitHAL(const HALInitParams& params)
{
    // Map the vertex format that we use internally for DrawableImage/Filters/Clears
    MapVertexFormat(PrimFill_Texture, &VertexXY16iAlpha::Format, 
        const_cast<const VertexFormat**>(&MappedXY16iAlphaTexture[PrimitiveBatch::DP_Single]),
        const_cast<const VertexFormat**>(&MappedXY16iAlphaTexture[PrimitiveBatch::DP_Batch]), 
        const_cast<const VertexFormat**>(&MappedXY16iAlphaTexture[PrimitiveBatch::DP_Instanced]),
        MeshCacheItem::Mesh_Regular);
    MapVertexFormat(PrimFill_SolidColor, &VertexXY16iAlpha::Format, 
        const_cast<const VertexFormat**>(&MappedXY16iAlphaSolid[PrimitiveBatch::DP_Single]),
        const_cast<const VertexFormat**>(&MappedXY16iAlphaSolid[PrimitiveBatch::DP_Batch]), 
        const_cast<const VertexFormat**>(&MappedXY16iAlphaSolid[PrimitiveBatch::DP_Instanced]),
        MeshCacheItem::Mesh_Regular);

    return HAL::InitHAL(params);
}

template<class ShaderManagerType, class ShaderInterfaceType>
inline bool ShaderHAL<ShaderManagerType, ShaderInterfaceType>::ShutdownHAL()
{
    if (!Render::HAL::ShutdownHAL())
        return false;

    for (unsigned i = 0; i < PrimitiveBatch::DP_DrawableCount; ++i )
    {
        if (MappedXY16iAlphaTexture[i])
            MappedXY16iAlphaTexture[i]->pSysFormat = 0;
        MappedXY16iAlphaTexture[i] = 0;
        if (MappedXY16iAlphaSolid[i])
            MappedXY16iAlphaSolid[i]->pSysFormat = 0;
        MappedXY16iAlphaSolid[i] = 0;
    }
    return true;
}


template<class ShaderManagerType, class ShaderInterfaceType>
inline bool ShaderHAL<ShaderManagerType, ShaderInterfaceType>::BeginScene()
{
    if (!HAL::BeginScene())
        return false;

    SManager.BeginScene();
    ShaderData.BeginScene();
    return true;
}

template<class ShaderManagerType, class ShaderInterfaceType>
inline bool ShaderHAL<ShaderManagerType, ShaderInterfaceType>::EndScene()
{
    if (!HAL::EndScene())
        return false;

    SManager.EndScene();
    return true;
}

template<class ShaderManagerType, class ShaderInterfaceType>
inline unsigned ShaderHAL<ShaderManagerType, ShaderInterfaceType>::GetMaximumBatchCount(Primitive* prim)
{
    if (!prim)
    {
        SF_DEBUG_WARNONCE(1,"GetMaximumBatchCount - prim is NULL. Disabling batching for this primitive.");
        return 1;
    }

    // The uniforms that could possibly be used by batching shaders are as follows:
    // mvp          - always present, could be either 2 or 4, depending on whether the Primitive is 3D.
    // texgen       - optional, 2 uniforms per texture in the fill (up to a maximum of 4 uniforms, for 2 textures)
    // cxadd/cxmul  - optional, if the primitive has a ColorTransform (2 uniforms). However, this is difficult to
    //                calculate up front, because it may depend on the exact shader chosen. So, just assume it will be
    //                present.

    unsigned uniformsPerBatch = 4; // 2D MVP + Cxform.
    PrimitiveFill* pfill = prim->GetFill();

    // Assumption: if the first mesh in the primitive is a 3D one, they are all 3D.
    if (prim->Meshes[0].M.Has3D())
        uniformsPerBatch += 2;

    // Add in the texgen matrices.
    if (pfill)
        uniformsPerBatch += 2 * pfill->GetTextureMatrixCount();

    // Calculate the number of batch instances. Note, that this should not exceed SF_RENDER_MAX_BATCHES,
    // as this is the shader maker's limit on the number of batches it creates the batch shaders with.
    return Alg::Min<unsigned>(SF_RENDER_MAX_BATCHES, SManager.GetNumberOfUniforms() / uniformsPerBatch);
}

template<class ShaderManagerType, class ShaderInterfaceType>
inline void ShaderHAL<ShaderManagerType, ShaderInterfaceType>::drawUncachedFilter(const FilterStackEntry& e)
{
    const FilterSet* filters = e.pPrimitive->GetFilters();
    unsigned filterCount = filters->GetFilterCount();
    const Filter* filter = 0;
    unsigned pass = 0, passes = 0;

    // Invalid primitive or rendertarget.
    if ( !e.pPrimitive || !e.pRenderTarget )
        return;

    // Bind the render target.
    SF_DEBUG_ASSERT(RenderTargetStack.Back().pRenderTarget == e.pRenderTarget, "Expected filter target to be the last on the stack.");
    Ptr<RenderTarget> temporaryTextures[Target_Count];
    memset(temporaryTextures, 0, sizeof temporaryTextures);

    // Fill our the source target.
    ImageSize size = e.pRenderTarget->GetSize();
    temporaryTextures[Target_Source] = e.pRenderTarget;

    // Set common render states.
    setBatchUnitSquareVertexStream();
    applyDepthStencilMode(DepthStencil_Disabled, 0);
    ApplyBlendMode(Blend_FilterBlend, true, false);
    applyBlendModeEnable(true);

    // Render filter(s).
    unsigned shaders[ShaderManagerType::MaximumFilterPasses];
    for ( unsigned i = 0; i < filterCount; ++i )
    {
        filter = filters->GetFilter(i);
        passes = SManager.SetupFilter(filter, FillFlags, shaders);

        // All shadows (except those hiding the object) need the original texture.
        bool requireSource = false;
        if ( filter->GetFilterType() >= Filter_Shadow &&
             filter->GetFilterType() <= Filter_Blur_End )
        {
            // If the Original texture was stored from a previous pass, it can be released
            // as it should be used already.
            if (temporaryTextures[Target_Original])
            {
                SF_DEBUG_ASSERT(i > 0, "Unexpected non-null original target on first pass.");
                temporaryTextures[Target_Original]->SetInUse(RTUse_Unused);
            }

            temporaryTextures[Target_Original] = temporaryTextures[Target_Source];
            requireSource = true;
        }

        // Now actually render the filter.
        for (pass = 0; pass < passes; ++pass)
        {
            // Render the final pass directly to the target surface.
            bool lastPass = (pass == passes-1 && i == filterCount-1);
            if (lastPass)
                break;

            // Create a destination texture if required.
            if ( !temporaryTextures[Target_Destination] )
            {
                temporaryTextures[Target_Destination] = *CreateTempRenderTarget(size, false);

                // Failed allocation. Try and quit gracefully.
                if ( !temporaryTextures[Target_Destination] )
                {
                    SF_DEBUG_WARNONCE(1, "Failed CreateTempRenderTarget. Filter rendering will be incorrect.");
                    temporaryTextures[Target_Source]->SetInUse(RTUse_Unused);
                    temporaryTextures[Target_Source] = 0;
                    temporaryTextures[Target_Source] = e.pRenderTarget;
                    i = filterCount;
                    break;
                }
            }

            // Push the next filter target on the stack.
            RenderTarget* prt = temporaryTextures[Target_Destination];
            const Rect<int>& viewRect = prt->GetRect(); // On the render texture, might not be the entire surface.
            PushRenderTarget(prt, PRT_Subscene, viewRect);

            Matrix2F mvp = Matrices->GetFullViewportMatrix(prt->GetBufferSize());
            drawFilter(mvp, Cxform::Identity, filter, temporaryTextures, shaders, pass, passes, 
                MappedXY16iAlphaTexture[PrimitiveBatch::DP_Batch]);

            // If we require the original source, create a new target for the source.
            bool nextPassIsLast = (pass+1 == passes-1 && i == filterCount-1);
            if ( requireSource && pass == 0 && !nextPassIsLast)
            {
                temporaryTextures[Target_Source] = *CreateTempRenderTarget(size, false);
                
                // Failed allocation. Try and quit gracefully.
                if ( !temporaryTextures[Target_Source] )
                {
                    SF_DEBUG_WARNONCE(1, "Failed CreateTempRenderTarget. Filter rendering will be incorrect.");
                    temporaryTextures[Target_Destination]->SetInUse(RTUse_Unused);
                    temporaryTextures[Target_Destination] = 0;
                    temporaryTextures[Target_Source] = e.pRenderTarget;
                    PopRenderTarget(PRT_NoSet);
                    i = filterCount;
                    break;
                }
            }

            // Pop the render target, but do not set the old one on the device (optimization). Note, this must
            // come after allocating the render target above, because it will set the render target to be 'not-in-use',
            // which means that the allocation above 
            PopRenderTarget(PRT_NoSet);

            // Setup for the next pass.
            Alg::Swap(temporaryTextures[Target_Source], temporaryTextures[Target_Destination]);
        }
    }

    // If there were no passes, assume we were doing a cacheAsBitmap.
    bool cacheAsBitmap = passes == 0;
    SF_DEBUG_ASSERT(!cacheAsBitmap || filterCount == 1, "Expected exactly one cacheAsBitmap filter.");

    // Cache the 2nd last step so it might be available as a cached filter next time.
    if (temporaryTextures[Target_Source] && (GetProfiler().IsFilterCachingEnabled() || cacheAsBitmap))
    {
        // If there were no passes, assume we were doing a cacheAsBitmap.
        RenderTarget* cacheResults[2] = { temporaryTextures[Target_Source], temporaryTextures[Target_Original] };        
        e.pPrimitive->SetCacheResults(cacheAsBitmap ? FilterPrimitive::Cache_Target : FilterPrimitive::Cache_PreTarget, cacheResults, cacheAsBitmap ? 1 : 2);
        cacheResults[0]->GetRenderTargetData()->CacheID = reinterpret_cast<UPInt>(e.pPrimitive.GetPtr());
        if ( cacheResults[1] )
            cacheResults[1]->GetRenderTargetData()->CacheID = reinterpret_cast<UPInt>(e.pPrimitive.GetPtr());
    }
    else
    {
        // This is required, or else disabling filter caching may produce incorrect results.
        e.pPrimitive->SetCacheResults(FilterPrimitive::Cache_Uncached, 0, 0);
    }
    

    // Pop the temporary target, begin rendering to the previous surface.
    PopRenderTarget();

    // Re-[en/dis]able masking from previous target, if available.
    if ( MaskStackTop != 0 )
    {
        bool drawingMask = (HALState & HS_DrawingMask) != 0;
        unsigned stencilRef = drawingMask ? MaskStackTop-1 : MaskStackTop;
        if (RenderTargetStack.Back().StencilAvailable)
            applyDepthStencilMode(drawingMask ? DepthStencil_StencilIncrementEqual : DepthStencil_StencilTestLessEqual, stencilRef);
        else if (RenderTargetStack.Back().DepthBufferAvailable)
            applyDepthStencilMode(drawingMask ? DepthStencil_DepthWrite : DepthStencil_DepthTestEqual, stencilRef);
    }

    // Reapply the current rasterization mode, as it may have been switched in PushFilters.
    applyRasterModeImpl(CurrentSceneRasterMode);

    // Now actually draw the filtered sub-scene to the target below.
    if (passes != 0)
    {
        // 'Real' filter.
        const Matrix2F& mvp = Matrices->UserView * e.pPrimitive->GetAreaMatrix().GetMatrix2D();
        const Cxform&   cx  = e.pPrimitive->GetAreaMatrix().GetCxform();
        ApplyBlendMode(getLastBlendModeOrDefault(), true, true);
        drawFilter(mvp, cx, filter, temporaryTextures, shaders, pass, passes, MappedXY16iAlphaTexture[PrimitiveBatch::DP_Batch]);
        ApplyBlendMode(getLastBlendModeOrDefault(), false, (HALState&HS_InRenderTarget) != 0);
    }
    else
    {
        // CacheAsBitmap
        drawCachedFilter(e.pPrimitive);
    }

    // Cleanup.
    for ( unsigned i = 0; i < Target_Count; ++i )
    {
        // pTextureManager->SetSamplerState
        if (temporaryTextures[i])
            temporaryTextures[i]->SetInUse(RTUse_Unused);
    }
    AccumulatedStats.Filters += filters->GetFilterCount();
}

template<class ShaderManagerType, class ShaderInterfaceType>
inline void ShaderHAL<ShaderManagerType, ShaderInterfaceType>::drawCachedFilter(FilterPrimitive* primitive)
{
    applyBlendModeEnable(true);
    
    switch(primitive->GetCacheState())
    {
        // We have one-step from final target. Render it to a final target now.
        case FilterPrimitive::Cache_PreTarget:
        {
            const FilterSet* filters = primitive->GetFilters();
            UPInt filterIndex = filters->GetFilterCount()-1;
            const Filter* filter = filters->GetFilter(filterIndex);
            unsigned shaders[ShaderManagerType::MaximumFilterPasses];
            unsigned passes = SManager.SetupFilter(filter, FillFlags, shaders);

            // Fill out the temporary textures from the cached results.
            Ptr<RenderTarget> temporaryTextures[Target_Count];
            memset(temporaryTextures, 0, sizeof temporaryTextures);
            RenderTarget* results[2];
            primitive->GetCacheResults(results, 2);
            temporaryTextures[Target_Source] = results[0];
            ImageSize size = temporaryTextures[Target_Source]->GetSize();
            temporaryTextures[Target_Original] = results[1];

            temporaryTextures[Target_Destination] = *CreateTempRenderTarget(size, false);
            SF_DEBUG_WARNONCE(!temporaryTextures[Target_Destination], "Failed CreateTempRenderTarget. Filter rendering will be incorrect.");
            if (!temporaryTextures[Target_Destination])
            {
                for ( unsigned i = 0; i < Target_Count; ++i )
                {
                    if (temporaryTextures[i])
                        temporaryTextures[i]->SetInUse(RTUse_Unused);
                }
                primitive->SetCacheResults(FilterPrimitive::Cache_Uncached, 0, 0);
                return;
            }

            PushRenderTarget(temporaryTextures[Target_Destination], PRT_Subscene, Rect<int>(size.Width,size.Height));

            // Render to the target.
            Matrix2F mvp = Matrices->GetFullViewportMatrix(size);
            ApplyBlendMode(Blend_Normal, true, true);
            drawFilter(mvp, Cxform::Identity, filter, temporaryTextures, shaders, passes-1, passes, 
                MappedXY16iAlphaTexture[PrimitiveBatch::DP_Batch]);
            PopRenderTarget();

            // Set this as the final cache result, and then render it.
            RenderTarget* prt = temporaryTextures[1];
            primitive->SetCacheResults(FilterPrimitive::Cache_Target, &prt, 1);
            prt->GetRenderTargetData()->CacheID = reinterpret_cast<UPInt>(primitive);
            drawCachedFilter(primitive);

            // Cleanup.
            for ( unsigned i = 0; i < Target_Count; ++i )
            {
                if (temporaryTextures[i])
                    temporaryTextures[i]->SetInUse(RTUse_Unused);
            }
            break;
        }

        // We have a final filtered texture. Just apply it to a screen quad.
        case FilterPrimitive::Cache_Target:
        {
            unsigned fillFlags = (FillFlags|FF_Cxform|FF_AlphaWrite);
            const typename ShaderManagerType::Shader& pso = SManager.SetFill(PrimFill_Texture, fillFlags, PrimitiveBatch::DP_Batch, 
                MappedXY16iAlphaTexture[PrimitiveBatch::DP_Batch], &ShaderData);

            RenderTarget* results;
            primitive->GetCacheResults(&results, 1);
            Render::Texture* ptexture = results->GetTexture();
            const Matrix2F& mvp = Matrices->UserView * primitive->GetAreaMatrix().GetMatrix2D();
            const Rect<int>& srect = results->GetRect();
            Matrix2F texgen;
            texgen.AppendTranslation((float)srect.x1, (float)srect.y1);
            texgen.AppendScaling((float)srect.Width() / ptexture->GetSize().Width, (float)srect.Height() / ptexture->GetSize().Height);

            const Cxform & cx = primitive->GetAreaMatrix().GetCxform();
            ShaderData.SetCxform(pso, cx);
            ShaderData.SetUniform(pso, ShaderManagerType::UniformType::SU_mvp, &mvp.M[0][0], 8 );
            ShaderData.SetUniform(pso, ShaderManagerType::UniformType::SU_texgen, &texgen.M[0][0], 8 );
            ShaderData.SetTexture(pso, ShaderManagerType::UniformType::SU_tex, ptexture, ImageFillMode(Wrap_Clamp, Sample_Linear));
            ShaderData.Finish(1);

            // Reapply the current rasterization mode, as it may have been switched in PushFilters.
            applyRasterModeImpl(CurrentSceneRasterMode);

            // Re-enable masking from previous target.
            if ( MaskStackTop != 0 )
            {
                RenderTargetEntry& rte = RenderTargetStack.Back();
                bool drawingMask = (HALState & HS_DrawingMask) != 0;
                unsigned stencilRef = drawingMask ? MaskStackTop-1 : MaskStackTop;
                if (rte.StencilAvailable)
                    applyDepthStencilMode(drawingMask ? DepthStencil_StencilIncrementEqual : DepthStencil_StencilTestLessEqual, stencilRef);
                else if (rte.DepthBufferAvailable)
                    applyDepthStencilMode(drawingMask ? DepthStencil_DepthWrite : DepthStencil_DepthTestEqual, stencilRef);
            }

            ApplyBlendMode(getLastBlendModeOrDefault(), true, true);
            setBatchUnitSquareVertexStream();
            drawPrimitive(6,1);
            ApplyBlendMode(getLastBlendModeOrDefault(), false, (HALState&HS_InRenderTarget)!=0);

            // Notify the RenderSync that a draw has been submitted.
            if (GetRenderSync())
                GetRenderSync()->NotifySubmission(RenderSync::SubmissionType_DrawCall);

            // Cleanup.
            results->SetInUse(RTUse_Unused_Cacheable);
            if ( !GetProfiler().IsFilterCachingEnabled() )
                primitive->SetCacheResults(FilterPrimitive::Cache_Uncached, 0, 0);
            break;
        }

        // Should have been one of the other two caching types.
        default: 
            SF_DEBUG_ASSERT(0, "HAL::drawCachedFilter - expected status to be Cache_PreTarget or Cache_Target."); 
            break;
    }
}

template<class ShaderManagerType, class ShaderInterfaceType>
inline void ShaderHAL<ShaderManagerType, ShaderInterfaceType>::drawFilter(const Matrix2F& mvp, const Cxform & cx, 
                                                                          const Filter* filter, Ptr<RenderTarget> * targets, 
                                                                          unsigned* shaders, unsigned pass, unsigned passCount, 
                                                                          VertexFormat* pvf)
{
    // Check if filter scaling is disabled. If so, just pass an identity matrix as the view matrix.
    const Matrix2F& viewportMatrix = (GetConfigFlags() & HALConfig_DisableFilterScaling) ? Matrix2F::Identity : ViewportMatrix;
    SManager.SetFilterFill(mvp, cx, viewportMatrix, filter, targets, shaders, pass, passCount, pvf, &ShaderData);
    setBatchUnitSquareVertexStream();
    drawPrimitive(6,1);

    // Notify the RenderSync that a draw has been submitted.
    if (GetRenderSync())
        GetRenderSync()->NotifySubmission(RenderSync::SubmissionType_DrawCall);
}

template<class ShaderManagerType, class ShaderInterfaceType>
inline void ShaderHAL<ShaderManagerType, ShaderInterfaceType>::PushBlendMode(BlendPrimitive* prim)
{
    if (!checkState(HS_InDisplay, __FUNCTION__))
        return;

    GetEvents().Begin(Event_BlendMode, "HAL::PushBlendMode");

    BlendMode mode = prim->GetBlendMode();
    BlendModeStackEntry entry = { prim, 0, 0, false, false };
    SF_DEBUG_ASSERT(prim != 0, "Unexpected NULL BlendPrimitive in HAL::PushBlendMode.");
    BlendModeStack.PushBack(entry);

    // If we are already in a cached blend target, just quit now.
    if (HALState & HS_InCachedTarget)
        return;

    BlendModeStackEntry& e = BlendModeStack.Back();
    if (BlendState::IsTargetAllocationNeededForBlendMode(prim->GetBlendMode()))
    {
        if (prim->GetCacheState() == CacheablePrimitive::Cache_Uncached)
        {
            // Create a render target.
            const Matrix2F& m = e.pPrimitive->GetAreaMatrix().GetMatrix2D();
            e.pRenderTarget = *CreateTempRenderTarget(ImageSize((UInt32)m.Sx(), (UInt32)m.Sy()), prim->GetMaskPresent());

            // Check for RT allocation failure.
            SF_DEBUG_WARNING(!e.pRenderTarget, "Failed CreateTempRenderTarget. Target BlendMode will be incorrect.");
            if (e.pRenderTarget)
            {
                RectF frameRect(m.Tx(), m.Ty(), m.Tx() + m.Sx(), m.Ty() + m.Sy());
                PushRenderTarget(e.pRenderTarget, PRT_Subscene, frameRect);
                // TODO: what if immediately next blend mode is alpha/erase? this would be inefficient.
            }

            // For layer blends, we need to allocate a single-channel render target, which holds the layer alpha values.
            if (prim->GetBlendMode() == Blend_Layer)
            {
                e.pLayerAlpha = *CreateTempRenderTarget(ImageSize((UInt32)m.Sx(), (UInt32)m.Sy()), false); //, true); TODO: single channel RBM allocation
                // Check for RT allocation failure.
                SF_DEBUG_WARNING(!e.pLayerAlpha, "Failed CreateTempRenderTarget. Target BlendMode will be incorrect.");
            }

            // Disable masking from previous target, if this blend primitive doesn't have any masking.
            if ( MaskStackTop != 0 && !prim->GetMaskPresent() && prim->GetCacheState() != CacheablePrimitive::Cache_Target )
                applyDepthStencilMode(HAL::DepthStencil_Disabled, MaskStackTop);

            // If this primitive has masking, then clear the entire area to the current mask level, because 
            // the depth stencil target may be different, and thus does not contain the previously written values.
            if (prim->GetMaskPresent() && checkDepthStencilBufferCaps())
            {
                RenderTargetEntry& rte = RenderTargetStack.Back();
                if (rte.StencilAvailable)
                    applyDepthStencilMode(HAL::DepthStencil_StencilClear, MaskStackTop);
                else if (rte.DepthBufferAvailable)
                    applyDepthStencilMode(HAL::DepthStencil_DepthWrite, MaskStackTop);

                const Matrix2F& viewMtx = GetMatrices()->GetFullViewportMatrix(e.pRenderTarget->GetBufferSize());
                drawMaskClearRectangles(&viewMtx, 1);


                // Although we cleared the mask, we might not be rendering it immediately, so put depth/stencil back to normal.
                applyDepthStencilMode(HAL::DepthStencil_Disabled, MaskStackTop);
            }

            // If we require a RT, simply do 'normal' blending when we start using it (the blend shader will
            // apply the real blending function).
            mode = Blend_Normal; 
        }
        else
        {
            // Drawing a cached filter, ignore all draw calls until the corresponding PopBlendMode.
            // Keep track of the level at which we need to draw the cached blend target, by adding entries to the stack.
            HALState |= HS_InCachedBlend;
            CacheableIndex = (int)BlendModeStack.GetSize()-1;
            GetRQProcessor().SetQueueEmitFilter(RenderQueueProcessor::QPF_CacheableOnly);
            return; // Don't actually apply the blend mode - it won't be used.
        }
    }
    else if(prim->GetBlendMode() == Blend_Alpha || prim->GetBlendMode() == Blend_Erase)
    {
        // For alpha and erase, we want to apply the results to the most immediate layer alpha.
        int parentLayerIndex = (int)(BlendModeStack.GetSize()-1);
        while (parentLayerIndex >= 0 && BlendModeStack[parentLayerIndex].pPrimitive->GetBlendMode() != Blend_Layer)
            parentLayerIndex--;

        SF_DEBUG_WARNONCE(parentLayerIndex < 0, "Could not find parent with BlendMode = 'Layer' for 'Alpha' or 'Erase' BlendMode.");
        if (parentLayerIndex >= 0 )
        {
            BlendModeStackEntry& parent = BlendModeStack[parentLayerIndex];
            SF_DEBUG_ASSERT(BlendModeStack[parentLayerIndex].pLayerAlpha, "Found parent with BlendMode = 'Layer' for 'Alpha' or 'Erase' BlendMode, "
                "but its LayerAlpha did not exist.");
            const Matrix2F& m = parent.pPrimitive->GetAreaMatrix().GetMatrix2D();
            RectF frameRect(m.Tx(), m.Ty(), m.Tx() + m.Sx(), m.Ty() + m.Sy());

            // Note: if there is a mask present, we clear to zero alpha, because anything outside the mask area should not have any alpha.
            PushRenderTarget(parent.pLayerAlpha, PRT_Subscene | (parent.LayerAlphaCleared ? PRT_NoClear : 0), frameRect,
                prim->GetMaskPresent() ? (UInt32)Color::Alpha0 : (UInt32)Color::Alpha100);

            // Only clear the parent layer alpha the first time. Otherwise, previous alpha/erase operations will be overwritten.
            parent.LayerAlphaCleared = true;
        }
        else
        {
            // switch the mode to 'Ignore', and record the situation.
            mode = Blend_Ignore;
            e.NoLayerParent = true;
        }
    }

    // Apply the blend mode
    ApplyBlendMode(mode, false, (HALState& HS_InRenderTarget) != 0 );
}

template<class ShaderManagerType, class ShaderInterfaceType>
inline void ShaderHAL<ShaderManagerType, ShaderInterfaceType>::PopBlendMode()
{
    if (!checkState(HS_InDisplay, __FUNCTION__))
        return;

    ScopedRenderEvent(GetEvents(), Event_BlendMode, 0, false);

    SF_DEBUG_ASSERT(BlendModeStack.GetSize() != 0, "Blend mode stack was unexpectedly empty during PopBlendMode.");
    BlendModeStackEntry e = BlendModeStack.Back();
    BlendModeStack.PopBack();

    // If doing a cached blendmode, and haven't reached the level at which it will be displayed, ignore the pop.
    if ( (HALState&HS_InCachedFilter) ||
         ((HALState&HS_InCachedTarget) && (CacheableIndex < (int)BlendModeStack.GetSize())) )
        return;

    BlendMode nextBlendMode = getLastBlendModeOrDefault();
    bool wasCached = (HALState & HS_InCachedTarget) != 0;
    CacheableIndex = -1;

    // Finish with the render target, and then use it to draw into the parent target.
    BlendMode mode = e.pPrimitive->GetBlendMode();
    if (BlendState::IsTargetAllocationNeededForBlendMode(mode))
    {
        Ptr<RenderTarget> blendSource  = 0;
        Ptr<RenderTarget> blendTarget  = 0;
        Ptr<RenderTarget> blendAlpha   = 0;

        // Ensure that blending is actually on.
        applyBlendModeEnable(true);

        if (!wasCached)
        {
            // Not cached, so we actually need to render the blend.
            blendSource  = RenderTargetStack.Back().pRenderTarget;  // Contains 'unblended' content
            blendAlpha   = e.pLayerAlpha;                           // Contains layer alpha information (for Blend_Layer only, otherwise null).
            blendTarget  = blendSource;

            SF_DEBUG_ASSERT(blendSource == e.pRenderTarget || e.pRenderTarget == 0, 
                "Blend target is not the last on the render target stack.");

            // if CreateRenderTarget failed in PushBlendMode, pRenderTarget and/or pLayerAlpha will be
            // NULL (latter for Blend_Layer only).
            if (e.pRenderTarget == 0 ||
                (mode == Blend_Layer && e.pLayerAlpha == 0))
            {
                ApplyBlendMode(nextBlendMode, false, (HALState& HS_InRenderTarget) != 0 );
                return;
            }
            
            // Rasterizing to a temporary target can be avoided if, there are no alpha/erase operations
            // on a layer target, which doesn't have a Cxform applied to it.
            bool unusedLayer = mode == Blend_Layer && !e.LayerAlphaCleared && 
                e.pPrimitive->GetAreaMatrix().GetCxform() == Cxform::Identity;

            PopRenderTarget(unusedLayer ? 0 : PRT_NoSet);

            if (!unusedLayer)
            {
                // Allocate the target which will hold the final blend.
                Ptr<RenderTarget> blendDest    = RenderTargetStack.Back().pRenderTarget;      // The 'backbuffer'
                blendTarget  = *CreateTempRenderTarget(blendSource->GetBufferSize(), false);  

                // Render Target allocation failure, just bail.
                SF_DEBUG_WARNING(!blendTarget, "Failed CreateTempRenderTarget. Target BlendMode will be incorrect.");
                if (!blendTarget)
                {
                    ApplyBlendMode(nextBlendMode, false, (HALState& HS_InRenderTarget) != 0 );
                    return;
                }
                PushRenderTarget(blendTarget, PRT_Subscene, Rect<int>(blendTarget->GetSize()));

                SF_DEBUG_WARNONCE(mode != Blend_Layer && (!blendDest || !blendDest->GetTexture()), 
                    "BlendMode destination did not have an associated Texture. Likely this is caused "
                    "by the lack of a backbuffer texture to sample. To support this, the backbuffer must be set using "
                    "HAL::SetRenderTarget, and the RenderTarget must have an associated texture.");

                // Now draw the blend primitive to the final blend target.
                if ((blendDest && blendDest->GetTexture()) ||
                    (blendAlpha && blendAlpha->GetTexture()))
                {
                    ApplyBlendMode(Blend_OverwriteAll, false, true);
                    drawBlendPrimitive(e.pPrimitive, 
                        blendSource->GetTexture(), 
                        mode == Blend_Layer ? 0 : blendDest->GetTexture(),                  // 'dest' is not required with Blend_Layer, it doesn't sample the parent.
                        blendAlpha && e.LayerAlphaCleared ? blendAlpha->GetTexture() : 0);  // If alpha has not been cleared, do not provide it, as it should be full.
                }
                PopRenderTarget();
            }
        }
        else
        {
            // Cached, just get the cache results.
            RenderTarget* cacheResults[2];
            e.pPrimitive->GetCacheResults(cacheResults, 2);
            blendTarget = cacheResults[0];
            blendAlpha  = cacheResults[1]; // layer alpha.
        }

        // Calculate the mvp and texgen for the incoming blend target.
        Matrix2F mvp = Matrices->UserView * e.pPrimitive->GetAreaMatrix().GetMatrix2D();
        const Rect<int>& srect = blendTarget->GetRect();
        Matrix2F texgen;
        texgen.AppendTranslation((float)srect.x1, (float)srect.y1);
        texgen.AppendScaling((float)srect.Width() / blendTarget->GetBufferSize().Width, (float)srect.Height() / blendTarget->GetBufferSize().Height);

        // Re-enable masking from previous target.
        if ( MaskStackTop != 0 )
        {
            RenderTargetEntry& rte = RenderTargetStack.Back();
            bool drawingMask = (HALState & HS_DrawingMask) != 0;
            unsigned stencilRef = drawingMask ? MaskStackTop-1 : MaskStackTop;
            if (rte.StencilAvailable)
                applyDepthStencilMode(drawingMask ? DepthStencil_StencilIncrementEqual : DepthStencil_StencilTestLessEqual, stencilRef);
            else if (rte.DepthBufferAvailable)
                applyDepthStencilMode(drawingMask ? DepthStencil_DepthWrite : DepthStencil_DepthTestEqual, stencilRef);
        }

        // Now draw the final blend into the backbuffer (just use DrawableCopyback, as it should do everything we need).
        // Note that we disable the 1/2 pixel offset in DrawableCopyback, it is already in UserView matrix.
        ApplyBlendMode(mode == Blend_Layer ? Blend_Layer : Blend_OverwriteAll, true, (HALState&HS_InRenderTarget) != 0);
        DrawableCopyback(blendTarget->GetTexture(), mvp, texgen, 0);

        // If we weren't cached, try to cache now.
        if (!wasCached && GetProfiler().IsBlendCachingEnabled())
        {
            // The source is no longer useful, but the target can be cached.
            Render::RenderTargetData* prt = blendTarget->GetRenderTargetData();
            RenderTarget* cacheResults[1] = {blendTarget};
            e.pPrimitive->SetCacheResults(CacheablePrimitive::Cache_Target, cacheResults, 1);
            prt->CacheID = reinterpret_cast<UPInt>(e.pPrimitive.GetPtr());
        }
        else if (!GetProfiler().IsBlendCachingEnabled())
        {
            e.pPrimitive->SetCacheResults(CacheablePrimitive::Cache_Uncached, 0, 0);
        }

        // Source isn't necessary, it contains the unblended content.
        if (blendSource)
            blendSource->SetInUse(RTUse_Unused);        
        if (blendAlpha)
            blendAlpha->SetInUse(RTUse_Unused);

        // The final target is cacheable (if nothing inside it changes).
        blendTarget->SetInUse(RTUse_Unused_Cacheable);
    }
    else if (mode == Blend_Alpha || mode == Blend_Erase)
    {
        // If the rendering was taking place in an alpha or erase mode, we need to put the color target
        // back on the top of the stack. This is done by simply popping the parent's pLayerAlpha.
        if (!e.NoLayerParent)
            PopRenderTarget();
        else
            nextBlendMode = Blend_Normal;
    }

    // If we were drawing a cached blend target, begin emitting all primitives again.
    if ( wasCached )
    {
        GetRQProcessor().SetQueueEmitFilter(RenderQueueProcessor::QPF_All);
        HALState &= ~HS_InCachedTarget;
    }

    ApplyBlendMode(nextBlendMode, false, (HALState& HS_InRenderTarget) != 0 );
}

template<class ShaderManagerType, class ShaderInterfaceType>
inline void ShaderHAL<ShaderManagerType, ShaderInterfaceType>::drawBlendPrimitive(BlendPrimitive* prim, Render::Texture* blendSource, 
                                                                                  Render::Texture* blendDest, Render::Texture* blendAlpha)
{
    Matrix2F mvp = GetMatrices()->GetFullViewportMatrix(RenderTargetStack.Back().pRenderTarget->GetSize());
    Cxform cx    = prim->GetAreaMatrix().GetCxform();
    Matrix2F texgen[Target_Count];
    Render::Texture* ptextures[Target_Count];
    const Matrix2F& primMtx = prim->GetAreaMatrix().GetMatrix2D();
    Size<float> primSize(primMtx.Sx(), primMtx.Sy());

    ptextures[Target_Source] = blendSource;
    ptextures[Target_Destination] = blendDest;
    ptextures[Target_Alpha] = blendAlpha;
    texgen[Target_Source].AppendScaling(primSize / blendSource->GetSize());

    // The alpha and source may be different sizes, depending on the temporary texture allocations. If this is the case,
    // modify the texture coordinates of the alpha to match the sampling of the color source.
    if (blendAlpha)
        texgen[Target_Alpha].AppendScaling(primSize / blendAlpha->GetSize());

    // 'dest' may be NULL in the case of Blend_Layer.
    if (blendDest)
    {
        texgen[Target_Destination].AppendScaling(primSize / blendDest->GetSize());
        RenderTargetEntry e = RenderTargetStack.Back();
        texgen[Target_Destination].AppendTranslation((primMtx.Tx() -e.OldViewRect.x1) / blendDest->GetSize().Width, 
            (primMtx.Ty() - e.OldViewRect.y1) / blendDest->GetSize().Height);
    }

    SManager.SetBlendFill(prim->GetBlendMode(), mvp, cx, ptextures, texgen, MappedXY16iAlphaTexture[PrimitiveBatch::DP_Batch], &ShaderData);
    drawScreenQuad();
}

// Draws a range of pre-cached and preprocessed primitives
template<class ShaderManagerType, class ShaderInterfaceType>
inline void ShaderHAL<ShaderManagerType, ShaderInterfaceType>::DrawProcessedPrimitive(Primitive* pprimitive,
                                                                                      PrimitiveBatch* pstart, 
                                                                                      PrimitiveBatch *pend)
{
    SF_AMP_SCOPE_RENDER_TIMER("HAL::DrawProcessedPrimitive", Amp_Profile_Level_High);
    ScopedRenderEvent GPUEvent(GetEvents(), Event_DrawPrimitive, "HAL::DrawProcessedPrimitive");

    if (!checkState(HS_InDisplay, "HAL::DrawProcessedPrimitive") ||
        !pprimitive->GetMeshCount() )
        return;

    SF_DEBUG_ASSERT(pend != 0, "No batch end to the given primitive.");

    PrimitiveBatch* pbatch = pstart ? pstart : pprimitive->Batches.GetFirst();

    unsigned bidx = 0;
    while (pbatch != pend)
    {        
        // pBatchMesh can be null in case of error, such as VB/IB lock failure.
        MeshCacheItem* pmesh = (MeshCacheItem*)pbatch->GetCacheItem();
        unsigned       meshIndex = pbatch->GetMeshIndex();
        unsigned       batchMeshCount = pbatch->GetMeshCount();

        if (pmesh)
        {
            unsigned fillFlags = FillFlags;
            if ( batchMeshCount > 0 )
                fillFlags |= pprimitive->Meshes[0].M.Has3D() ? FF_3DProjection : 0;

            ShaderData.BeginPrimitive();

            const typename ShaderManagerType::Shader& pShader =
                SManager.SetPrimitiveFill(pprimitive->pFill, fillFlags, pbatch->Type, pbatch->pFormat, 
                batchMeshCount, Matrices, &pprimitive->Meshes[meshIndex], &ShaderData);

            GetProfiler().SetBatch(this, pprimitive, bidx);

            // Must set the stream source before 'Finish', as it may set uniforms.
            if (pbatch->Type != PrimitiveBatch::DP_Instanced)
                setLinearStreamSource(pbatch->Type);
            else
                setInstancedStreamSource(pbatch->GetMeshCount(), pmesh->IndexCount);

            ShaderData.Finish(batchMeshCount);

            if ((HALState & HS_ViewValid) && pShader) 
            {
                SF_DEBUG_ASSERT((pbatch->Type != PrimitiveBatch::DP_Failed) &&
                    (pbatch->Type != PrimitiveBatch::DP_Virtual),
                    "Cannot render failed or virtual batches.");

                // Make sure the blending state is correct.
                bool blend = (fillFlags & FF_Blending) != 0;
                applyBlendModeEnable(blend);

                // Draw the object with cached mesh.
                UPInt   indexOffset = setVertexArray(pbatch, pmesh);

                if (pbatch->Type != PrimitiveBatch::DP_Instanced)
                    drawIndexedPrimitive(pmesh->IndexCount, pmesh->VertexCount, pmesh->MeshCount, indexOffset, 0 );
                else
                    drawIndexedInstanced(pmesh->IndexCount, pmesh->VertexCount, pbatch->GetMeshCount(), indexOffset, 0);
            }

            if (GetRenderSync())
                GetRenderSync()->AddFenceResource(pmesh);

            pmesh->MoveToCacheListFront(MCL_ThisFrame);
        }

        pbatch = pbatch->GetNext();
        bidx++;
    }

    // Notify the RenderSync that a draw has been submitted.
    if (GetRenderSync())
        GetRenderSync()->NotifySubmission(RenderSync::SubmissionType_DrawCall);
}

template<class ShaderManagerType, class ShaderInterfaceType>
inline void ShaderHAL<ShaderManagerType, ShaderInterfaceType>::DrawProcessedComplexMeshes(ComplexMesh* complexMesh,
                                                                                          const StrideArray<HMatrix>& matrices)
{    
    SF_AMP_SCOPE_RENDER_TIMER("HAL::DrawProcessedComplexMeshes", Amp_Profile_Level_High);
    ScopedRenderEvent GPUEvent(GetEvents(), Event_DrawComplex, "HAL::DrawProcessedComplexMeshes");

    typedef ComplexMesh::FillRecord   FillRecord;
    typedef PrimitiveBatch::BatchType BatchType;

    MeshCacheItem* pmesh = (MeshCacheItem*)complexMesh->GetCacheItem();
    if (!checkState(HS_InDisplay, "HAL::DrawProcessedComplexMeshes") || !pmesh)
        return;

    const FillRecord* fillRecords = complexMesh->GetFillRecords();
    unsigned    fillCount     = complexMesh->GetFillRecordCount();
    unsigned    instanceCount = (unsigned)matrices.GetSize();
    BatchType   batchType = PrimitiveBatch::DP_Single;
    unsigned    formatIndex;
    unsigned    maxDrawCount = 1;
    unsigned    vertexBaseIndex = 0;

    const Matrix2F* textureMatrices = complexMesh->GetFillMatrixCache();
    for (unsigned fillIndex = 0; fillIndex < fillCount; fillIndex++)
    {
        const FillRecord& fr = fillRecords[fillIndex];

        if (instanceCount > 1 && SManager.HasInstancingSupport() && 
            GetProfiler().GetProfileFlag(ProfileFlag_NoInstancing) == 0 && fr.pFormats[1] != 0)
        {
            maxDrawCount = Alg::Min(instanceCount, GetMeshCache().GetParams().MaxBatchInstances);
            batchType = PrimitiveBatch::DP_Instanced;
            formatIndex = 1;
        }
        else
        {
            batchType = PrimitiveBatch::DP_Single;
            formatIndex = 0;
            setLinearStreamSource(batchType);
        }

        unsigned fillFlags = FillFlags;
        unsigned startIndex = 0;
        if ( instanceCount > 0 )
        {
            const HMatrix& hm = matrices[0];
            fillFlags |= hm.Has3D() ? FF_3DProjection : 0;

            for (unsigned i = 0; i < instanceCount; i++)
            {
                const HMatrix& hm = matrices[startIndex + i];
                Cxform finalCx = GetProfiler().GetCxform(hm.GetCxform());
                if (!(finalCx == Cxform::Identity))
                    fillFlags |= FF_Cxform;
                if (finalCx.RequiresBlend())
                    fillFlags |= FF_Blending;
            }
        }

        // Apply fill.
        PrimitiveFillType fillType = Profiler.GetFillType(fr.pFill->GetType());
        const typename ShaderManagerType::Shader& pso = SManager.SetFill(fr.pFill, fillFlags, batchType, fr.pFormats[formatIndex], &ShaderData);

        GetProfiler().SetBatch(this, complexMesh, fillIndex);

        UPInt indexBufferOffset = 0;
        indexBufferOffset = setVertexArray(fr, formatIndex, pmesh);

        bool blend = ((fillFlags & FF_Blending) != 0 || fr.pFill->RequiresBlend());
        applyBlendModeEnable(blend);
        GetProfiler().SetFillFlags(fillFlags);

        UByte textureCount = fr.pFill->GetTextureCount();
        bool solid = PrimitiveFill::IsSolid(fillType);

        for (unsigned i = 0; i < instanceCount; i++)
        {            
            const HMatrix& hm = matrices[startIndex + i];

            ShaderData.SetMatrix(pso, ShaderManagerType::UniformType::SU_mvp, complexMesh->GetVertexMatrix(), hm, Matrices, 0, i%maxDrawCount);
            if (solid)
            {
                ShaderData.SetColor(pso, ShaderManagerType::UniformType::SU_cxmul, GetProfiler().GetColor(fr.pFill->GetSolidColor()), 0, i%maxDrawCount);
                textureCount = 0;
            }
            else if (fillFlags & FF_Cxform)
                ShaderData.SetCxform(pso, GetProfiler().GetCxform(hm.GetCxform()), 0, i%maxDrawCount);

            for (unsigned tm = 0, stage = 0; tm < textureCount; tm++)
            {
                ShaderData.SetMatrix(pso, ShaderManagerType::UniformType::SU_texgen, textureMatrices[fr.FillMatrixIndex[tm]], tm, i%maxDrawCount);
                Texture* ptex = (Texture*)fr.pFill->GetTexture(tm);
                ShaderData.SetTexture(pso, ShaderManagerType::UniformType::SU_tex, ptex, fr.pFill->GetFillMode(tm), stage);
                stage += ptex->GetTextureStageCount();
            }

            bool lastPrimitive = (i == instanceCount-1);
            if ( batchType != PrimitiveBatch::DP_Instanced )
            {
                ShaderData.Finish(1);
                setVertexArrayPerDraw(fr, formatIndex, pmesh);
                drawIndexedPrimitive(fr.IndexCount, fr.VertexCount, 1, fr.IndexOffset + indexBufferOffset, vertexBaseIndex);
                AccumulatedStats.Primitives++;
                if ( !lastPrimitive )
                    ShaderData.BeginPrimitive();
            }
            else if (( (i+1) % maxDrawCount == 0 && i != 0) || lastPrimitive )
            {
                unsigned drawCount = maxDrawCount;
                if ( lastPrimitive && (i+1) % maxDrawCount != 0)
                    drawCount = (i+1) % maxDrawCount;
                setInstancedStreamSource(drawCount, fr.IndexCount);
                ShaderData.Finish(drawCount);
                setVertexArrayPerDraw(fr, formatIndex, pmesh);
                drawIndexedInstanced(fr.IndexCount, fr.VertexCount, drawCount, fr.IndexOffset + indexBufferOffset, vertexBaseIndex);
                AccumulatedStats.Primitives++;
                if ( !lastPrimitive )
                    ShaderData.BeginPrimitive();
            }
        }
        //vertexBaseIndex += fr.VertexCount;

    } // for (fill record)

    if (GetRenderSync())
        GetRenderSync()->AddFenceResource(pmesh);

    // Notify the RenderSync that a draw has been submitted.
    if (GetRenderSync())
        GetRenderSync()->NotifySubmission(RenderSync::SubmissionType_DrawCall);

    pmesh->MoveToCacheListFront(MCL_ThisFrame);
}

template<class ShaderManagerType, class ShaderInterfaceType>
inline void ShaderHAL<ShaderManagerType, ShaderInterfaceType>::DrawableCxform( Render::Texture** tex, const Matrix2F* texgen, const Cxform* cx)
{
    ScopedRenderEvent GPUEvent(GetEvents(), Event_DICxform, "HAL::DrawableCxform");
    SManager.SetDrawableCxform(tex, texgen, RenderTargetStack.Back().pRenderTarget->GetSize(), cx, 
        MappedXY16iAlphaTexture[PrimitiveBatch::DP_Batch], &ShaderData, ShaderManagerType::GetDrawableImageFlags() );
    drawScreenQuad();
}

template<class ShaderManagerType, class ShaderInterfaceType>
inline void ShaderHAL<ShaderManagerType, ShaderInterfaceType>::DrawableCompare( Render::Texture** tex, const Matrix2F* texgen )
{
    ScopedRenderEvent GPUEvent(GetEvents(), Event_DICompare, "HAL::DrawableCompare");
    SManager.SetDrawableCompare(tex, texgen, RenderTargetStack.Back().pRenderTarget->GetSize(), 
        MappedXY16iAlphaTexture[PrimitiveBatch::DP_Batch], &ShaderData, ShaderManagerType::GetDrawableImageFlags() );
    drawScreenQuad();
}

template<class ShaderManagerType, class ShaderInterfaceType>
inline void ShaderHAL<ShaderManagerType, ShaderInterfaceType>::DrawableMerge( Render::Texture** tex, const Matrix2F* texgen, const Matrix4F* cxmul )
{
    ScopedRenderEvent GPUEvent(GetEvents(), Event_DIMerge, "HAL::DrawableMerge");
    SManager.SetDrawableMergeFill(tex, texgen, RenderTargetStack.Back().pRenderTarget->GetSize(), 
        cxmul, MappedXY16iAlphaTexture[PrimitiveBatch::DP_Batch], &ShaderData, ShaderManagerType::GetDrawableImageFlags() );

    drawScreenQuad();
}

template<class ShaderManagerType, class ShaderInterfaceType>
inline void ShaderHAL<ShaderManagerType, ShaderInterfaceType>::DrawableCopyPixels( Render::Texture** tex, const Matrix2F* texgen, const Matrix2F& mvp, bool mergeAlpha, bool destAlpha )
{
    ScopedRenderEvent GPUEvent(GetEvents(), Event_DICopyPixels, "HAL::DrawableCopyPixels");
    SManager.SetDrawableCopyPixelsFill(tex, texgen, RenderTargetStack.Back().pRenderTarget->GetSize(), mvp,
        mergeAlpha, destAlpha, MappedXY16iAlphaTexture[PrimitiveBatch::DP_Batch], &ShaderData, 
        ShaderManagerType::GetDrawableImageFlags() );

    drawScreenQuad();
}

template<class ShaderManagerType, class ShaderInterfaceType>
inline void ShaderHAL<ShaderManagerType, ShaderInterfaceType>::DrawablePaletteMap( Render::Texture** tex, const Matrix2F* texgen, const Matrix2F& mvp, unsigned channelMask, const UInt32* values)
{
    ScopedRenderEvent GPUEvent(GetEvents(), Event_DIPaletteMap, "HAL::DrawablePaletteMap");

    // Create a temporary texture with the palette map. There may be a more efficient way to do this; however, using
    // uniforms seems unworkable, due to shader constant slot constraints.
    ImageData data;
    Render::TextureManager* mgr = GetTextureManager();
    Ptr<Render::Texture> ptex = *mgr->CreateTexture(mgr->GetDrawableImageFormat(), 1, ImageSize(256, 4), ImageUse_Map_Mask, 0);
    if ( !ptex || !ptex->Map(&data, 0, 1) )
        return;
    for ( int channel = 0; channel < 4; ++channel )
    {
        UByte* dataPtr = data.GetScanline(channel);
        if ( channelMask & (1<<channel))
        {
            const UInt32* value = values + channel*256;
            for (unsigned ival = 0; ival < 256; ++ival)
                data.SetPixelInScanline(dataPtr, ival, value[ival]);
        }
        else
        {
            // Channel was not provided, just do a straight mapping.
            for (unsigned ival = 0; ival < 256; ++ival)
                data.SetPixelInScanline(dataPtr, ival, (ival << (channel*8)));
        }
    }
    if (!ptex->Unmap())
        return;

    // First pass overwrites everything.
    ApplyBlendMode(Blend_OverwriteAll, true, true);
    SManager.SetDrawablePaletteMap(tex, texgen, RenderTargetStack.Back().pRenderTarget->GetSize(), mvp,
        ptex, MappedXY16iAlphaTexture[PrimitiveBatch::DP_Batch], &ShaderData, ShaderManagerType::GetDrawableImageFlags() );

    drawScreenQuad();
}

template<class ShaderManagerType, class ShaderInterfaceType>
inline void ShaderHAL<ShaderManagerType, ShaderInterfaceType>::DrawableCopyback( Render::Texture* source, const Matrix2F& mvpOriginal, 
                                                                                 const Matrix2F& texgen, unsigned flagMask )
{
    ScopedRenderEvent GPUEvent(GetEvents(), Event_DICopyback, "HAL::DrawableCopyback");

    // Set shader constants.
    unsigned fillFlags = 0;
    const typename ShaderManagerType::Shader& pso = SManager.SetFill(PrimFill_Texture, fillFlags, PrimitiveBatch::DP_Batch, 
        MappedXY16iAlphaTexture[PrimitiveBatch::DP_Batch], &ShaderData);    

    Matrix2F mvp = mvpOriginal;
    if ((ShaderManagerType::GetDrawableImageFlags() & flagMask) & ShaderManagerType::Base::CPF_HalfPixelOffset)
    {
        // D3D9 1/2 pixel center offset
        mvp.Tx() -= 1.0f/RenderTargetStack.Back().pRenderTarget->GetSize().Width;   
        mvp.Ty() += 1.0f/RenderTargetStack.Back().pRenderTarget->GetSize().Height;
    }

    ShaderData.SetMatrix(pso,  ShaderManagerType::UniformType::SU_mvp,    mvp);
    ShaderData.SetMatrix(pso,  ShaderManagerType::UniformType::SU_texgen, texgen);
    ShaderData.SetTexture(pso, ShaderManagerType::UniformType::SU_tex,    source, ImageFillMode(Wrap_Clamp, Sample_Point));
    ShaderData.Finish(1);

    drawScreenQuad();
}

template<class ShaderManagerType, class ShaderInterfaceType>
inline UPInt ShaderHAL<ShaderManagerType, ShaderInterfaceType>::setVertexArray(PrimitiveBatch* pbatch, MeshCacheItem* pmesh)
{
    SF_UNUSED(pbatch);
    if (GetRenderSync())
        GetRenderSync()->AddFenceResource(pmesh);

    return 0; // Return value should be ignored.
}

template<class ShaderManagerType, class ShaderInterfaceType>
inline UPInt ShaderHAL<ShaderManagerType, ShaderInterfaceType>::setVertexArray(const ComplexMesh::FillRecord& fr, unsigned formatIndex, MeshCacheItem* pmesh)
{
    SF_UNUSED2(fr, formatIndex);
    if (GetRenderSync())
        GetRenderSync()->AddFenceResource(pmesh);

    return 0; // Return value should be ignored.
}

template<class ShaderManagerType, class ShaderInterfaceType>
inline void ShaderHAL<ShaderManagerType, ShaderInterfaceType>::drawScreenQuad()
{
    // Set the vertices, and Draw
    setBatchUnitSquareVertexStream();
    drawPrimitive(6,1);

    // Notify the RenderSync that a draw has been submitted.
    if (GetRenderSync())
        GetRenderSync()->NotifySubmission(RenderSync::SubmissionType_DrawCall);
}

template<class ShaderManagerType, class ShaderInterfaceType>
inline void ShaderHAL<ShaderManagerType, ShaderInterfaceType>::drawMaskClearRectangles(const Matrix2F* matrices, UPInt count)
{
    ScopedRenderEvent GPUEvent(GetEvents(), Event_MaskClear, "HAL::drawMaskClearRectangles");

    // This operation is used to clear bounds for masks.
    // Potential issue: Since our bounds are exact, right/bottom pixels may not
    // be drawn due to HW fill rules.
    //  - This shouldn't matter if a mask is tessellated within bounds of those
    //    coordinates, since same rules are applied to those render shapes.
    //  - EdgeAA must be turned off for masks, as that would extrude the bounds.

    unsigned fillflags = 0;

    const typename ShaderManagerType::Shader& pso = SManager.SetFill(PrimFill_SolidColor, fillflags, PrimitiveBatch::DP_Batch, 
        MappedXY16iAlphaSolid[PrimitiveBatch::DP_Batch], &ShaderData);
    unsigned drawRangeCount = 0;

    setBatchUnitSquareVertexStream();

    for (UPInt i = 0; i < count; i+= (UPInt)drawRangeCount)
    {
        drawRangeCount = Alg::Min<unsigned>((unsigned)count, SF_RENDER_MAX_BATCHES);

        // NOTE: If this is not the first pass, then we must call BeginPrimitive, which is called in the
        // first pass inside SManager.SetFill. Otherwise, you will never being another shader uniform set.
        // However, it's extremely rare that you will reach a situation when you have more than one batch
        // worth of clear rectangles.
        if (i != 0)
            ShaderData.BeginPrimitive();

        for (unsigned j = 0; j < drawRangeCount; j++)
        {
            ShaderData.SetMatrix(pso, ShaderManagerType::UniformType::SU_mvp, matrices[i+j], 0, j);

            // Color writes should be disabled, so although the shader does have a cxmul uniform, we don't really need to set it.
            float colorf[4];
            Color c = GetProfiler().GetClearColor(0xFF00007F);
            c.GetRGBAFloat(colorf);
            ShaderData.SetUniform(pso, ShaderManagerType::UniformType::SU_cxmul, colorf, 4);
        }
        ShaderData.Finish(drawRangeCount);
        setBatchUnitSquareVertexStreamPerDraw();
        drawPrimitive(drawRangeCount * 6, drawRangeCount);
    }

    // Notify the RenderSync that a draw has been submitted.
    if (GetRenderSync())
        GetRenderSync()->NotifySubmission(RenderSync::SubmissionType_DrawCall);
}

// NOTE: This implementation is for platforms which do not have a device clear function. Platforms which have a
// hardware clear should override this method, and do the hardware clear instead, as it is generally more efficient.
template<class ShaderManagerType, class ShaderInterfaceType>
inline void ShaderHAL<ShaderManagerType, ShaderInterfaceType>::ClearSolidRectangle(const Rect<int>& r, Color color, bool blend)
{
    ScopedRenderEvent GPUEvent(GetEvents(), Event_Clear, "HAL::ClearSolidRectangle");

    // Don't draw clears in wireframe, always draw them in solid.
    applyRasterMode(RasterMode_Solid);

    HALBlendState previousState = CurrentBlendState;
    if (blend)
    {
        // Apply 'Normal', so that the 'clear' is blended over the current buffer contents.
        ApplyBlendMode(Blend_Normal, false, true);
    }
    else
    {
        // Apply 'OverwriteAll', so that the buffer is cleared to the exact color.
        ApplyBlendMode(Blend_OverwriteAll);
    }

    color = GetProfiler().GetClearColor(color);

    // Note: cannot do HW clears in Orbis, because no hardware clear exists.
    float colorf[4];
    color.GetRGBAFloat(colorf);
    Matrix2F m((float)r.Width(), 0.0f, (float)r.x1,
        0.0f, (float)r.Height(), (float)r.y1);
    Matrix2F  mvp(m, Matrices->UserView);

    unsigned fillflags = 0;
    const typename ShaderManagerType::Shader& pso = SManager.SetFill(PrimFill_SolidColor, fillflags, PrimitiveBatch::DP_Batch, 
        MappedXY16iAlphaSolid[PrimitiveBatch::DP_Batch], &ShaderData);
    ShaderData.SetMatrix(pso, ShaderManagerType::UniformType::SU_mvp, mvp);
    ShaderData.SetUniform(pso, ShaderManagerType::UniformType::SU_cxmul, colorf, 4);
    ShaderData.Finish(1);

    drawScreenQuad();

    // Restore previous blending and raster modes
    applyBlendMode(previousState);
    applyRasterMode(CurrentSceneRasterMode);
}

template<class ShaderManagerType, class ShaderInterfaceType>
inline void ShaderHAL<ShaderManagerType, ShaderInterfaceType>::profilerDrawCacheablePrimArea(const CacheablePrimitive* prim)
{
    GetProfiler().SetDrawMode(DrawMode_CachedPrim);
    unsigned fillflags = 0;
    float colorf[4];
    Profiler.GetColor(0xFFFFFFFF).GetRGBAFloat(colorf);
    const typename ShaderManagerType::Shader& pso = SManager.SetFill(PrimFill_SolidColor, fillflags,
        PrimitiveBatch::DP_Batch,  MappedXY16iAlphaSolid[PrimitiveBatch::DP_Batch], &ShaderData);
    Matrix2F mvp(prim->GetAreaMatrix().GetMatrix2D(), GetMatrices()->UserView);
    ShaderData.SetMatrix(pso, ShaderManagerType::UniformType::SU_mvp, mvp);
    ShaderData.SetUniform(pso, ShaderManagerType::UniformType::SU_cxmul, colorf, 4);
    ShaderData.Finish(1);
    setBatchUnitSquareVertexStream();
    drawPrimitive(6,1);

    // Notify the RenderSync that a draw has been submitted.
    if (GetRenderSync())
        GetRenderSync()->NotifySubmission(RenderSync::SubmissionType_DrawCall);
}

template<class ShaderManagerType, class ShaderInterfaceType>
inline void ShaderHAL<ShaderManagerType, ShaderInterfaceType>::profilerApplyUniform(ProfilerUniform uniform, 
                                                                                    unsigned components, 
                                                                                    float* values)
{
    const typename ShaderManagerType::Shader& pso = ShaderData.GetCurrentShaders();
    switch(uniform)
    {
    case ProfileUniform_TextureSize:
        ShaderData.SetUniform(pso, ShaderManagerType::UniformType::SU_textureDims, values, components );
        break;
    case ProfileUniform_TextureMips:
        ShaderData.SetUniform(pso, ShaderManagerType::UniformType::SU_mipLevels, values, components );
        break;
    default:
        SF_DEBUG_ASSERT1(0, "Unknown profiler uniform: %d\n", uniform);
        break;
    }
}

}}; // Scaleform::Render

#endif // INC_SF_Render_ShaderHAL_H
