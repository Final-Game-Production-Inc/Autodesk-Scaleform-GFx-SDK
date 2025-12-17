/**************************************************************************

Filename    :   D3D9_HAL.cpp
Content     :   D3D9 Renderer HAL Prototype implementation.
Created     :   May 2009
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

// This removes the need to link with dxguid.lib.
#define DIRECT3D_VERSION 0x0900
#include <InitGuid.h>

#include "Kernel/SF_Debug.h"
#include "Kernel/SF_Random.h"
#include "Render/D3D9/D3D9_HAL.h"
#include "Render/Render_BufferGeneric.h"
#include "Kernel/SF_HeapNew.h"
#include "Render/D3D9/D3D9_Events.h"
#if SF_CC_MSVC < 1700
#include <d3dx9.h>
#endif

#include <stdio.h>

#if SF_CC_MSVC >= 1600 // MSVC 2010
#include <crtdefs.h>
#endif // SF_CC_MSVC >= 1600 // MSVC 2010

namespace Scaleform { namespace Render { namespace D3D9 {

// Defines all D3Dx color channels for D3DRS_COLORWRITEENABLE.
#define D3DCOLORWRITEENABLE_ALL D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_RED | \
    D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_GREEN


// ***** HAL_D3D9

HAL::HAL(ThreadCommandQueue* commandQueue)
:   Render::ShaderHAL<ShaderManager, ShaderInterface>(commandQueue),
    pDevice(0),
    Cache(Memory::GetGlobalHeap(), MeshCacheParams::PC_Defaults),
    PrevBatchType(PrimitiveBatch::DP_None),
    StencilOpInc(D3DSTENCILOP_REPLACE)
{
}

HAL::~HAL()
{
    ShutdownHAL();
}

// *** RenderHAL_D3D9 Implementation
bool HAL::InitHAL(const D3D9::HALInitParams& params)
{
    ScopedRenderEvent GPUEvent(GetEvent(Event_InitHAL), __FUNCTION__);

    if ( !initHAL(params))
        return false;

    if (!params.pD3DDevice)
        return 0; 

    pDevice = params.pD3DDevice;
    pDevice->AddRef();

    // Detect shader level features.
    D3DCAPSx caps;
    D3DDEVICE_CREATION_PARAMETERS cparams;
    pDevice->GetDeviceCaps(&caps);
    pDevice->GetCreationParameters(&cparams);

    // If SetDevice fails, it means that creating queries failed, fallback to using static meshes.
    bool dynamicMeshes = !(params.ConfigFlags&HALConfig_StaticBuffers);
    if ( !RSync.SetDevice(pDevice))
    {
        SF_DEBUG_WARNING(1, "RenderSync initialization failed. Using static buffers in MeshCache.");
        dynamicMeshes = false;
    }

    if (!Cache.Initialize(pDevice, dynamicMeshes) ||
        !SManager.Initialize(this))
    {
        ShutdownHAL();
        return false;
    }

    // Create Texture manager if needed.
    if (params.pTextureManager)
        pTextureManager = params.GetTextureManager();
    else
    {
        D3DCapFlags ourCaps;
        ourCaps.InitFromHWCaps(caps, cparams);

        // Determine D3D9Ex usage.
        IDirect3DDevice9Ex* d3d9exPtr;
        if ( SUCCEEDED(pDevice->QueryInterface(IID_IDirect3DDevice9Ex, (void**)&d3d9exPtr)) && d3d9exPtr)
        {
            d3d9exPtr->Release();
            ourCaps.Flags |= D3DCapFlags::Cap_D3D9Ex;
        }

        pTextureManager = 
            *SF_HEAP_AUTO_NEW(this) TextureManager(pDevice, ourCaps,
                                                   params.RenderThreadId, pRTCommandQueue);
        if (!pTextureManager)
        {
            ShutdownHAL();
            return false;
        }
    }

    // Create RenderBufferManager if needed.
    if (params.pRenderBufferManager)
        pRenderBufferManager = params.pRenderBufferManager;
    else
    {
        // Create the default render target, and manager.
        pRenderBufferManager = *SF_HEAP_AUTO_NEW(this) RenderBufferManagerGeneric(RBGenericImpl::DSSM_EqualOrBigger);
        if ( !pRenderBufferManager || !createDefaultRenderBuffer())
        {
            ShutdownHAL();
            return false;
        }
    }

    // Detect stencil op.
    if (caps.StencilCaps & D3DSTENCILCAPS_INCR)
    {
        StencilOpInc = D3DSTENCILOP_INCR;
    }
    else if (caps.StencilCaps & D3DSTENCILCAPS_INCRSAT)
    {
        StencilOpInc = D3DSTENCILOP_INCRSAT;
    }
    else
    {   // Stencil ops not available.
        StencilOpInc = D3DSTENCILOP_REPLACE;
    }

    memcpy(&PresentParams, &params.PresentParams, sizeof(D3DPRESENT_PARAMETERS));
    HALState|= HS_ModeSet;
    notifyHandlers(HAL_Initialize);
    return true;
}

// Returns back to original mode (cleanup)
bool HAL::ShutdownHAL()
{
    if (!(HALState & HS_ModeSet))
        return true;

    if (!shutdownHAL())
        return false;

    destroyRenderBuffers();
    pRenderBufferManager.Clear();

    // This must happen before destroying buffers. If the device is lost, then the WaitFence implementation
    // depends on the RenderSync to be reset before any meshes are destroy. Otherwise and infinite wait may occur.
    RSync.SetDevice(0);

    // Do TextureManager::Reset to ensure shutdown on the current thread.
    if ( pTextureManager )
        pTextureManager->Reset();
    pTextureManager.Clear();
    SManager.Reset();
    Cache.Reset();
     
    if (pDevice)
        pDevice->Release();
    pDevice = 0;

    return true;
}

void HAL::PrepareForReset()
{
    SF_ASSERT(HALState & HS_ModeSet);
    if (HALState & HS_ReadyForReset)
        return;

    // This must happen before destroying buffers. If the device is lost, then the WaitFence implementation
    // depends on the RenderSync to be reset before any meshes are destroy. Otherwise and infinite wait may occur.
    RSync.SetDevice(0);

    notifyHandlers(HAL_PrepareForReset);

    destroyRenderBuffers();

    pRenderBufferManager->Reset();
    pTextureManager->PrepareForReset();
    Cache.Reset();    

    HALState |= HS_ReadyForReset;
}

// - RestoreAfterReset called after reset to restore needed variables.
bool HAL::RestoreAfterReset()
{
    if (!IsInitialized())
        return false;
    if (!(HALState & HS_ReadyForReset))
        return true;

    // If SetDevice fails, it means that creating queries failed, fallback to using static meshes.
    bool dynamicMeshes = Cache.UsesDynamicMeshes();
    if ( !RSync.SetDevice(pDevice))
    {
        SF_DEBUG_WARNING(1, "RenderSync initialization failed. Using static buffers in MeshCache.");
        dynamicMeshes = false;
    }

    if (!Cache.Initialize(pDevice, dynamicMeshes))
        return false;

    pTextureManager->RestoreAfterReset();

    if (!createDefaultRenderBuffer())
        return false;

     notifyHandlers(HAL_RestoreAfterReset);

     HALState &= ~HS_ReadyForReset;
     return true;
}

    
// ***** Rendering

bool HAL::BeginScene()
{
    ScopedRenderEvent GPUEvent(GetEvent(Event_BeginScene), __FUNCTION__ "-SetState");
    if ( !BaseHAL::BeginScene())
        return false;

    // Not necessary of not alpha testing:
    pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);  // Important!

    pDevice->SetRenderState(D3DRS_SCISSORTESTENABLE,FALSE);

    // Textures off by default.
    pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);

    // Reset the Depth/Stencil states.
    pDevice->SetRenderState(D3DRS_STENCILMASK, 0xFF);

    // Turn off back-face culling.
    pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    pDevice->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, FALSE);

    // Disable fog.
    pDevice->SetRenderState(D3DRS_FOGENABLE, FALSE); 

    // Disable sRGB writes.
    pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, FALSE);

    // Start the scene
    if (!(VMCFlags&HALConfig_NoSceneCalls))
        pDevice->BeginScene();

    return true;
}

bool HAL::EndScene()
{
    if ( !BaseHAL::EndScene())
        return false;

    // 'undo' instancing setup, if it was set.
    pDevice->SetStreamSourceFreq(0, 1);
    pDevice->SetStreamSourceFreq(1, 1);
    PrevBatchType = PrimitiveBatch::DP_None;

    if (!(VMCFlags&HALConfig_NoSceneCalls))
        pDevice->EndScene();

    return true;
}

// Updates D3D HW Viewport and ViewportMatrix based on provided viewport
// and view rectangle.
void HAL::updateViewport()
{
    D3DVIEWPORTx vp;
    Rect<int>    vpRect;

    if (HALState & HS_ViewValid)
    {
        int dx = ViewRect.x1 - VP.Left,
            dy = ViewRect.y1 - VP.Top;
        
        // Modify HW matrix and viewport to clip.
        CalcHWViewMatrix(Viewport::View_HalfPixelOffset, &Matrices->View2D, ViewRect, dx, dy);
        Matrices->SetUserMatrix(Matrices->User);
        Matrices->ViewRect    = ViewRect;
        Matrices->UVPOChanged = 1;

        /*
        // TBD: Prepend UserMatrix here is incorrect for nested viewport-based
        // mask clipping; what's needed is a complex combination of viewport and
        // coordinate adjustment. Until this is done, mask viewport clipping will be
        // in the wrong place for UserMatrix.
        if (UserMatrix != Matrix2F::Identity)
        {
            Rect<int> viewportRect;
            Rect<int> userViewRect(
                ViewRect.x1 + (int)UserMatrix.Tx(),
                ViewRect.y1 + (int)UserMatrix.Ty(),
                Size<int>((int)(UserMatrix.Sx() * (float)ViewRect.Width()),
                          (int)(UserMatrix.Sy() * (float)ViewRect.Height())));

            VP.GetClippedRect(&viewportRect);
            viewportRect.IntersectRect(&vpRect, userViewRect);
        }
        */

        if ( !(HALState & HS_InRenderTarget) )
		{
			Viewport vpHal = VP;
            vpHal.Left     = ViewRect.x1;
            vpHal.Top      = ViewRect.y1;
            vpHal.Width    = ViewRect.Width();
            vpHal.Height   = ViewRect.Height();
            vpHal.SetStereoViewport(Matrices->S3DDisplay);
			vpRect.SetRect(vpHal.Left, vpHal.Top, vpHal.Left + vpHal.Width, vpHal.Top + vpHal.Height);
		}
        else
            vpRect.SetRect(VP.Left, VP.Top, VP.Left + VP.Width, VP.Top + VP.Height);
    }

    vp.X        = vpRect.x1;
    vp.Y        = vpRect.y1;

    // DX9 can't handle a vp with zero area.
    vp.Width    = (DWORD)Alg::Max<int>(vpRect.Width(), 1);
    vp.Height   = (DWORD)Alg::Max<int>(vpRect.Height(), 1);

    vp.MinZ     = 0.0f;
    vp.MaxZ     = 0.0f;
    pDevice->SetViewport(&vp);
}

UPInt HAL::setVertexArray(PrimitiveBatch* pbatch, Render::MeshCacheItem* pmeshBase)
{
    D3D9::MeshCacheItem* pmesh = reinterpret_cast<D3D9::MeshCacheItem*>(pmeshBase);

                pDevice->SetIndices(pmesh->pIndexBuffer->GetHWBuffer());
                pDevice->SetStreamSource(0, pmesh->pVertexBuffer->GetHWBuffer(),
                    (UINT)pmesh->VBAllocOffset, pbatch->pFormat->Size);               
     
    return pmesh->IBAllocOffset / sizeof (IndexType);
}

UPInt HAL::setVertexArray(const ComplexMesh::FillRecord& fr, unsigned formatIndex, Render::MeshCacheItem* pmeshBase)
{    
    D3D9::MeshCacheItem* pmesh = reinterpret_cast<D3D9::MeshCacheItem*>(pmeshBase);
    const Render::VertexFormat* pformat = fr.pFormats[formatIndex];
    pDevice->SetIndices(pmesh->pIndexBuffer->GetHWBuffer());
    pDevice->SetStreamSource(0, pmesh->pVertexBuffer->GetHWBuffer(), (UINT)(fr.VertexByteOffset + pmesh->VBAllocOffset),
        pformat->Size);
    return (unsigned)(pmesh->IBAllocOffset / sizeof(IndexType));
}

void HAL::setLinearStreamSource( PrimitiveBatch::BatchType type )
{
    if (PrevBatchType >= PrimitiveBatch::DP_Instanced)
        {
        pDevice->SetStreamSource(1, NULL, 0, 0);
        pDevice->SetStreamSourceFreq(0, 1);
        pDevice->SetStreamSourceFreq(1, 1);
            }
    PrevBatchType = type;
}


void HAL::setInstancedStreamSource( UPInt instanceCount, UPInt )
{
    SF_ASSERT(SManager.HasInstancingSupport());
    if (PrevBatchType != PrimitiveBatch::DP_Instanced)
        {
        pDevice->SetStreamSource(1, Cache.pInstancingVertexBuffer.GetPtr(),
            0, sizeof(Render_InstanceData));
        pDevice->SetStreamSourceFreq(1, (UINT) D3DSTREAMSOURCE_INSTANCEDATA | 1);
        PrevBatchType = PrimitiveBatch::DP_Instanced;
        }
    pDevice->SetStreamSourceFreq(0, D3DSTREAMSOURCE_INDEXEDDATA | (UINT)instanceCount);
}

void HAL::applyRasterModeImpl(RasterModeType mode)
{
    D3DFILLMODE fillMode;
    switch(mode)
    {
        default:
        case RasterMode_Solid:      fillMode = D3DFILL_SOLID; break;
        case RasterMode_Wireframe:  fillMode = D3DFILL_WIREFRAME; break;
        case RasterMode_Point:      fillMode = D3DFILL_POINT; break;
    }
    pDevice->SetRenderState(D3DRS_FILLMODE, fillMode);
}

//--------------------------------------------------------------------
// *** Mask / Stencil support
//--------------------------------------------------------------------

void HAL::applyDepthStencilMode(DepthStencilMode mode, unsigned stencilRef)
{
    ScopedRenderEvent GPUEvent(GetEvent(Event_ApplyDepthStencil), __FUNCTION__);
    static D3DCMPFUNC DepthStencilCompareFunctions[DepthStencilFunction_Count] =
    {
        D3DCMP_NEVER,           // Ignore
        D3DCMP_NEVER,           // Never
        D3DCMP_LESS,            // Less
        D3DCMP_EQUAL,           // Equal
        D3DCMP_LESSEQUAL,       // LessEqual
        D3DCMP_GREATER,         // Greater
        D3DCMP_NOTEQUAL,        // NotEqual
        D3DCMP_GREATEREQUAL,    // GreaterEqual
        D3DCMP_ALWAYS,          // Always
    };
    static D3DSTENCILOP StencilOps[StencilOp_Count] =
    {
        D3DSTENCILOP_KEEP,      // Ignore
        D3DSTENCILOP_KEEP,      // Keep
        D3DSTENCILOP_REPLACE,   // Replace
        D3DSTENCILOP_INCR,      // Increment        
    };

    const HALDepthStencilDescriptor& oldState = DepthStencilModeTable[CurrentDepthStencilState];
    const HALDepthStencilDescriptor& newState = DepthStencilModeTable[mode];

    // Apply the modes now.
    if (oldState.ColorWriteEnable != newState.ColorWriteEnable)
        pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, newState.ColorWriteEnable ? D3DCOLORWRITEENABLE_ALL : 0);

    if (oldState.StencilEnable != newState.StencilEnable)
        pDevice->SetRenderState(D3DRS_STENCILENABLE, newState.StencilEnable ? TRUE : FALSE);

    // Only need to set stencil pass/fail ops if stenciling is actually enabled.
    if (newState.StencilEnable)
    {
        if (CurrentStencilRef != stencilRef)
        {
            pDevice->SetRenderState(D3DRS_STENCILREF, stencilRef);
            CurrentStencilRef = stencilRef;
        }

        if (oldState.StencilFunction != newState.StencilFunction &&
            newState.StencilFunction != HAL::DepthStencilFunction_Ignore)
        {
            pDevice->SetRenderState(D3DRS_STENCILFUNC, DepthStencilCompareFunctions[newState.StencilFunction]);
        }
        if (oldState.StencilFailOp != newState.StencilFailOp &&
            newState.StencilFailOp != HAL::StencilOp_Ignore)
        {
            pDevice->SetRenderState(D3DRS_STENCILFAIL, StencilOps[newState.StencilFailOp]);
        }
        if (oldState.StencilPassOp != newState.StencilPassOp &&
            newState.StencilPassOp!= HAL::StencilOp_Ignore)
    {
            pDevice->SetRenderState(D3DRS_STENCILPASS, StencilOps[newState.StencilPassOp]);
        }
        if (oldState.StencilZFailOp != newState.StencilZFailOp &&
            newState.StencilZFailOp != HAL::StencilOp_Ignore)
        {
            pDevice->SetRenderState(D3DRS_STENCILZFAIL, StencilOps[newState.StencilZFailOp]);
            }
        }

    // If the value of depth test/write change, we may have to change the value of ZEnable.
    if ((oldState.DepthTestEnable || oldState.DepthWriteEnable) != 
        (newState.DepthTestEnable || newState.DepthWriteEnable))
    {
        pDevice->SetRenderState(D3DRS_ZENABLE, newState.DepthTestEnable || newState.DepthWriteEnable ? D3DZB_TRUE : D3DZB_FALSE);

        // Only need to set the function, if depth testing is enabled.
        if (newState.DepthTestEnable)
        {
            if (oldState.DepthFunction != newState.DepthFunction &&
                newState.DepthFunction != HAL::DepthStencilFunction_Ignore)
        {
                pDevice->SetRenderState(D3DRS_ZFUNC, DepthStencilCompareFunctions[newState.StencilFunction]);
        }
    }
        }

    if (oldState.DepthWriteEnable != newState.DepthWriteEnable)
        pDevice->SetRenderState(D3DRS_ZWRITEENABLE, newState.DepthWriteEnable || newState.DepthWriteEnable ? TRUE : FALSE);

    CurrentDepthStencilState = mode;
}

bool HAL::checkDepthStencilBufferCaps()
{
    if (!StencilChecked)
    {
        // Test for depth-stencil presence.
        IDirect3DSurfaceX *pdepthStencilSurface = 0;
        pDevice->GetDepthStencilSurface(&pdepthStencilSurface);
        if (pdepthStencilSurface)
        {
            D3DSURFACE_DESC sd;
            pdepthStencilSurface->GetDesc(&sd);

            static const D3DFORMAT D3DFMT_INTZ = (D3DFORMAT)MAKEFOURCC('I','N','T','Z');
            static const D3DFORMAT D3DFMT_RAWZ = (D3DFORMAT)MAKEFOURCC('R','A','W','Z');
            switch((DWORD)sd.Format)
            {
            case D3DFMT_D24S8:
            case D3DFMT_D24X4S4:
            case D3DFMT_D24FS8:
            case D3DFMT_INTZ:
                MultiBitStencil = 1;
            case D3DFMT_D15S1:
                StencilAvailable = 1;
                break;
            case D3DFMT_RAWZ:   // RAWZ does not have stencil bits.
                break;
            default:
                SF_DEBUG_ASSERT1(1, "Unexpected DepthStencil format: 0x%08x\n", sd.Format);
                break;
            }

            // Report single-bit stenciling if we cannot increment.
            MultiBitStencil &= (StencilOpInc == D3DSTENCILOP_INCR || StencilOpInc == D3DSTENCILOP_INCRSAT);

            pdepthStencilSurface->Release();
            pdepthStencilSurface = 0;
            DepthBufferAvailable = 1;
        }
        else
            StencilAvailable = 0;

        StencilChecked = 1;
    }

    SF_DEBUG_WARNONCE(!StencilAvailable && !DepthBufferAvailable, 
        "RendererHAL::PushMask_BeginSubmit used, but neither stencil or depth buffer is available");
    if (!StencilAvailable && !DepthBufferAvailable)
        return false;

    return true;
}

//--------------------------------------------------------------------
// Background clear helper, expects viewport coordinates.
void HAL::clearSolidRectangle(const Rect<int>& r, Color color, bool blend)
{
    if ((!blend || color.GetAlpha() == 0xFF) && !(VP.Flags & Viewport::View_Stereo_AnySplit))
    {
        ScopedRenderEvent GPUEvent(GetEvent(Event_Clear), "HAL::clearSolidRectangle"); // NOTE: inside scope, base impl has its own profile.

        // Do more efficient HW clear. Device::Clear expects render-target coordinates.
        D3DRECT  cr = { r.x1 + VP.Left, r.y1 + VP.Top,
                        r.x2 + VP.Left, r.y2 + VP.Top };
        D3DCOLOR d3dc = D3DCOLOR_ARGB(color.GetAlpha(), color.GetRed(), color.GetGreen(), color.GetBlue());
        pDevice->Clear(1, &cr, D3DCLEAR_TARGET, d3dc, 1.0f,  0);
    }
    else
    {
        BaseHAL::clearSolidRectangle(r, color, blend);
    }
}



//--------------------------------------------------------------------
// *** BlendMode Stack support
//--------------------------------------------------------------------

void HAL::applyBlendModeImpl(BlendMode mode, bool sourceAc, bool forceAc )
{    
    static const UInt32 BlendOps[BlendOp_Count] = 
    {
        D3DBLENDOP_ADD,         // BlendOp_ADD
        D3DBLENDOP_MAX,         // BlendOp_MAX
        D3DBLENDOP_MIN,         // BlendOp_MIN
        D3DBLENDOP_REVSUBTRACT, // BlendOp_REVSUBTRACT
    };

    static const UInt32 BlendFactors[BlendFactor_Count] = 
    {
        D3DBLEND_ZERO,          // BlendFactor_ZERO
        D3DBLEND_ONE,           // BlendFactor_ONE
        D3DBLEND_SRCALPHA,      // BlendFactor_SRCALPHA
        D3DBLEND_INVSRCALPHA,   // BlendFactor_INVSRCALPHA
        D3DBLEND_DESTCOLOR,     // BlendFactor_DESTCOLOR
        D3DBLEND_INVDESTCOLOR,  // BlendFactor_INVDESTCOLOR
    };

    if (!pDevice)
        return;


    pDevice->SetRenderState(D3DRS_BLENDOP, BlendOps[BlendModeTable[mode].Operator]);
    pDevice->SetRenderState(D3DRS_DESTBLEND, BlendFactors[BlendModeTable[mode].DestColor]);
    if ( sourceAc && BlendFactors[BlendModeTable[mode].SourceColor] == D3DBLEND_SRCALPHA )
        pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
    else
        pDevice->SetRenderState(D3DRS_SRCBLEND, BlendFactors[BlendModeTable[mode].SourceColor]);        

    if (VP.Flags & Viewport::View_AlphaComposite || forceAc)
    {
        pDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
        pDevice->SetRenderState(D3DRS_BLENDOPALPHA, BlendOps[BlendModeTable[mode].AlphaOperator]);
        pDevice->SetRenderState(D3DRS_SRCBLENDALPHA, BlendFactors[BlendModeTable[mode].SourceAlpha]);
        pDevice->SetRenderState(D3DRS_DESTBLENDALPHA, BlendFactors[BlendModeTable[mode].DestAlpha]);
    }
    else
    {
        pDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
    }
}

RenderTarget* HAL::CreateRenderTarget(IDirect3DSurface9* pcolor, IDirect3DSurface9* pdepth)
{
    D3DSURFACE_DESC sdesc;
    ImageSize dsSize(0), rtSize(0);

    if (SUCCEEDED(pcolor->GetDesc(&sdesc)))
        rtSize.SetSize(sdesc.Width, sdesc.Height);

    RenderTarget* ptarget = pRenderBufferManager->CreateRenderTarget(rtSize, RBuffer_User, Image_R8G8B8A8, 0);

    Ptr<DepthStencilBuffer> pdsb = 0;
    if (pdepth && SUCCEEDED(pdepth->GetDesc(&sdesc)))
    {
        dsSize.SetSize(sdesc.Width, sdesc.Height);
        pdsb = *SF_HEAP_AUTO_NEW(this) DepthStencilBuffer(0, dsSize);
    }

    RenderTargetData::UpdateData(ptarget, pcolor, pdsb, pdepth);
    return ptarget;
}

void HAL::applyBlendModeEnableImpl(bool enabled)
{
    pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, enabled ? TRUE : FALSE);    
}

RenderTarget* HAL::CreateRenderTarget(Render::Texture* texture, bool needsStencil)
{
    D3D9::Texture* pt = (D3D9::Texture*)texture;

    // Cannot render to textures which have multiple HW representations.
    if ( !pt || pt->TextureCount != 1 )
        return 0;

    IDirect3DSurface9* prenderSurface = 0, *pdsSurface = 0;
    RenderTarget* prt = pRenderBufferManager->CreateRenderTarget(
        texture->GetSize(), RBuffer_Texture, texture->GetFormat(), texture);
    if ( !prt )
        return 0;
    pt->pTextures[0].pTexture->GetSurfaceLevel(0,&prenderSurface);

    Ptr<DepthStencilBuffer> pdsb;
    if ( needsStencil )
    {
        pdsb = *pRenderBufferManager->CreateDepthStencilBuffer(prt->GetTexture()->GetSize(), false); // false == not temporary
        if ( pdsb )
        {
            DepthStencilSurface* surf = (D3D9::DepthStencilSurface*)pdsb->GetSurface();
            if ( surf )
                pdsSurface = surf->pDepthStencilSurface;
        }
    }
    RenderTargetData::UpdateData(prt, prenderSurface, pdsb, pdsSurface );
    prenderSurface->Release();

    return prt;
}

RenderTarget* HAL::CreateTempRenderTarget(const ImageSize& size, bool needsStencil)
{
    RenderTarget* prt = pRenderBufferManager->CreateTempRenderTarget(size);
    if ( !prt )
        return 0;

    RenderTargetData* phd = (RenderTargetData*)prt->GetRenderTargetData();
    if ( phd && (!needsStencil || phd->pDSSurface))
        return prt;

    Ptr<IDirect3DSurface9> prenderSurface = 0;
    IDirect3DSurface9* pdsSurface = 0;
    D3D9::Texture* pt = (D3D9::Texture*)prt->GetTexture();

    // Cannot render to textures which have multiple HW representations.
    SF_ASSERT(pt->TextureCount == 1); 
    pt->pTextures[0].pTexture->GetSurfaceLevel(0,&prenderSurface.GetRawRef());

    Ptr<DepthStencilBuffer> pdsb;
    if ( needsStencil )
    {
        pdsb = *pRenderBufferManager->CreateDepthStencilBuffer(prt->GetTexture()->GetSize());
        if ( pdsb )
        {
            DepthStencilSurface* surf = (D3D9::DepthStencilSurface*)pdsb->GetSurface();
            if ( surf )
                pdsSurface = surf->pDepthStencilSurface;
        }
    }

    RenderTargetData::UpdateData(prt, prenderSurface, pdsb, pdsSurface);
    return prt;
}

bool HAL::SetRenderTarget(RenderTarget* ptarget, bool setState)
{
    // When changing the render target while in a scene, we must flush all drawing.
    if ( HALState & HS_InScene)
        Flush();

    // Cannot set the bottom level render target if already in display.
    if ( HALState & HS_InDisplay )
        return false;

    D3DSURFACE_DESC rtDesc;
    RenderTargetEntry entry;
    RenderTargetData* prtdata = (D3D9::RenderTargetData*)ptarget->GetRenderTargetData();
    IDirect3DSurface9* pd3dsurface= prtdata->pRenderSurface;
    if ( FAILED(pd3dsurface->GetDesc(&rtDesc)) )
        return false;

    if ( setState )
    {
        pDevice->SetRenderTarget(0, pd3dsurface);
        if ( prtdata->pDSSurface )
            pDevice->SetDepthStencilSurface(prtdata->pDSSurface);
    }
    entry.pRenderTarget = ptarget;

    // Replace the stack entry at the bottom, or if the stack is empty, add one.
    if ( RenderTargetStack.GetSize() > 0 )
        RenderTargetStack[0] = entry;
    else
        RenderTargetStack.PushBack(entry);
    return true;
}

void HAL::PushRenderTarget(const RectF& frameRect, RenderTarget* prt, unsigned flags, Color clearColor)
{
    GetEvent(Event_RenderTarget).Begin(__FUNCTION__);
    HALState |= HS_InRenderTarget;
    RenderTargetEntry entry;
    entry.pRenderTarget = prt;
    entry.OldViewport = VP;
    entry.OldViewRect = ViewRect;
    entry.OldMatrixState.CopyFrom(Matrices);
    Matrices->SetUserMatrix(Matrix2F::Identity);

    // Setup the render target/depth stencil on the device.
    if ( !prt )
    {
        SF_DEBUG_WARNING(1, __FUNCTION__ " - invalid render target.");
        RenderTargetStack.PushBack(entry);
        return;
    }

    // Unbind the texture if it is bound (can't have a texture bound as an input and target).
    pTextureManager->SetSamplerState(0, 0);
    pTextureManager->SetSamplerState(1, 0);

    RenderTargetData* phd = (D3D9::RenderTargetData*)prt->GetRenderTargetData();
    pDevice->SetRenderTarget(0, phd->pRenderSurface);
    if ( phd->pDSSurface )
        pDevice->SetDepthStencilSurface(phd->pDSSurface);
    else
        pDevice->SetDepthStencilSurface(0);

    StencilChecked = false;
    ++AccumulatedStats.RTChanges;

    // Clear, if not specifically excluded
    if ( (flags & PRT_NoClear) == 0 )
    {    
        D3DCOLOR d3dclearColor = D3DCOLOR_ARGB(clearColor.GetAlpha(),clearColor.GetRed(),clearColor.GetGreen(),clearColor.GetBlue());
        pDevice->Clear(0, 0, D3DCLEAR_TARGET, d3dclearColor, 1.0f,  0);
    }

    // Setup viewport.
    Rect<int> viewRect = prt->GetRect(); // On the render texture, might not be the entire surface.
    const ImageSize& bs = prt->GetBufferSize();
    VP = Viewport(bs.Width, bs.Height, viewRect.x1, viewRect.y1, viewRect.Width(), viewRect.Height());    
    ViewRect.x1 = (int)frameRect.x1;
    ViewRect.y1 = (int)frameRect.y1;
    ViewRect.x2 = (int)frameRect.x2;
    ViewRect.y2 = (int)frameRect.y2;

    // Must offset the 'original' viewrect, otherwise the 3D compensation matrix will be offset.
    Matrices->ViewRectOriginal.Offset(-entry.OldViewport.Left, -entry.OldViewport.Top);
    Matrices->UVPOChanged = true;
    HALState |= HS_ViewValid;
    updateViewport();

    RenderTargetStack.PushBack(entry);
}

void HAL::PopRenderTarget(unsigned flags)
{
    ScopedRenderEvent GPUEvent(GetEvent(Event_RenderTarget), __FUNCTION__, false);

    RenderTargetEntry& entry = RenderTargetStack.Back();
    RenderTarget* prt = entry.pRenderTarget;
    if ( prt->GetType() == RBuffer_Temporary )
    {
        // Strip off the depth stencil surface/buffer from temporary targets.
        D3D9::RenderTargetData* plasthd = (D3D9::RenderTargetData*)prt->GetRenderTargetData();
        if ( plasthd->pDSSurface )
        {
            plasthd->pDSSurface->Release();
            plasthd->pDSSurface = 0;
        }
        plasthd->pDepthStencilBuffer = 0;
    }

    Matrices->CopyFrom(&entry.OldMatrixState);
    ViewRect = entry.OldViewRect;
    VP = entry.OldViewport;

    RenderTargetStack.PopBack();
    RenderTargetData* phd = 0;
    if ( RenderTargetStack.GetSize() > 0 )
    {
        RenderTargetEntry& back = RenderTargetStack.Back();
        phd = (D3D9::RenderTargetData*)back.pRenderTarget->GetRenderTargetData();
    }

    if ( RenderTargetStack.GetSize() == 1 )
        HALState &= ~HS_InRenderTarget;

    // Unbind the texture if it is bound (can't have a texture bound as an input and target).
    pTextureManager->SetSamplerState(0, 0);
    pTextureManager->SetSamplerState(1, 0);

    // Restore the old render target on the device.
    if ((flags & PRT_NoSet) == 0)
    {
        pDevice->SetRenderTarget(0, phd->pRenderSurface);
        if ( phd->pDSSurface )
            pDevice->SetDepthStencilSurface(phd->pDSSurface);
        else
            pDevice->SetDepthStencilSurface(0);

        ++AccumulatedStats.RTChanges;

        // Reset the viewport to the last render target on the stack.
        HALState |= HS_ViewValid;
        updateViewport();
    }
}

bool HAL::createDefaultRenderBuffer()
{
    ImageSize rtSize;

    if ( GetDefaultRenderTarget() )
    {
        RenderTarget* prt = GetDefaultRenderTarget();
        rtSize = prt->GetSize();
    }
    else
    {
        IDirect3DSurface9* prenderTarget = 0;
        IDirect3DSurface9* pdsTarget = 0;
        D3DSURFACE_DESC rtDesc;
        if (FAILED(pDevice->GetRenderTarget(0, &prenderTarget)) ||             
            FAILED(prenderTarget->GetDesc(&rtDesc)) )
        {
            return false;
        }
        rtSize = ImageSize(rtDesc.Width, rtDesc.Height);

        Ptr<DepthStencilBuffer> pdsb = 0;
        D3DSURFACE_DESC dsDesc;
        ImageSize dsSize(0);
        if ( SUCCEEDED(pDevice->GetDepthStencilSurface(&pdsTarget)) &&
            SUCCEEDED(pdsTarget->GetDesc(&dsDesc)))
        {
            dsSize.SetSize(dsDesc.Width, dsDesc.Height);
            pdsb = *SF_HEAP_AUTO_NEW(this) DepthStencilBuffer(0, dsSize);
        }
        Ptr<RenderTarget> ptarget = *SF_HEAP_AUTO_NEW(this) RenderTarget(0, RBuffer_Default, rtSize );
        RenderTargetData::UpdateData(ptarget, prenderTarget, pdsb, pdsTarget );

        if ( pdsTarget )
            pdsTarget->Release();
        prenderTarget->Release();

        if ( !SetRenderTarget(ptarget))
            return false;
    }

    return pRenderBufferManager->Initialize(pTextureManager, Image_B8G8R8A8, rtSize);
}

void HAL::setBatchUnitSquareVertexStream()
{
    setLinearStreamSource(PrimitiveBatch::DP_Batch);
    pDevice->SetStreamSource(0, Cache.pMaskEraseBatchVertexBuffer.GetPtr(),
        0, sizeof(VertexXY16iAlpha));
}

void HAL::drawPrimitive(unsigned indexCount, unsigned meshCount)
{
    pDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, indexCount / 3);

    SF_UNUSED(meshCount);
#if !defined(SF_BUILD_SHIPPING)
    AccumulatedStats.Meshes += meshCount;
    AccumulatedStats.Triangles += indexCount / 3;
    AccumulatedStats.Primitives++;
#endif
}

void HAL::drawIndexedPrimitive( unsigned indexCount, unsigned vertexCount, unsigned meshCount, UPInt indexOffset, UPInt vertexBaseIndex)
{
    pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, (INT)vertexBaseIndex, 0, vertexCount, (UINT)indexOffset, indexCount / 3 );

    SF_UNUSED(meshCount);
#if !defined(SF_BUILD_SHIPPING)
    AccumulatedStats.Meshes += meshCount;
    AccumulatedStats.Triangles += indexCount / 3;
    AccumulatedStats.Primitives++;
#endif
}

void HAL::drawIndexedInstanced( unsigned indexCount, unsigned vertexCount, unsigned meshCount, UPInt indexOffset, UPInt vertexBaseIndex)
{
    pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, (INT)vertexBaseIndex, 0, vertexCount, (UINT)indexOffset, indexCount / 3 );

    SF_UNUSED(meshCount);
#if !defined(SF_BUILD_SHIPPING)
    AccumulatedStats.Meshes += meshCount;
    AccumulatedStats.Triangles += (indexCount / 3) * meshCount;
    AccumulatedStats.Primitives++;
#endif
}


bool HAL::shouldRenderFilters(const FilterPrimitive* prim) const
{
    if ( SManager.HasDynamicLoopingSupport() )
        return true;

    // If the profile doesn't support dynamic loops, check to see if there are any ColorMatrix
    // filters, which can still be rendered. If there are, allow filtering still.
    const FilterSet* filters = prim->GetFilters();
    unsigned filterCount = filters->GetFilterCount();
    for ( unsigned f = 0; f < filterCount; ++f )
    {
        const Filter* filter = filters->GetFilter(f);
        if ( filter->GetFilterType() != Filter_ColorMatrix )
            return false;
    }
    return true;
}

void HAL::initMatrices()
{
    Matrices = *SF_HEAP_AUTO_NEW(this) D3D9::MatrixState(this);
}

Render::RenderEvent& HAL::GetEvent(EventType eventType)
{
#if !defined(SF_BUILD_SHIPPING)
    static D3D9::RenderEvent D3D9Events[Event_Count];
    return D3D9Events[eventType];
#else
    // Shipping builds just return a default event, which does nothing.
    return Render::HAL::GetEvent(eventType);
#endif
}

void HAL::drawScreenQuad()
{
    // Set the vertices, and Draw
    setBatchUnitSquareVertexStream();
	drawPrimitive(6,1);
}

}}} // Scaleform::Render::D3D9

