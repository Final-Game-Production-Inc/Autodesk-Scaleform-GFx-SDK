/**************************************************************************

Filename    :   D3D1x_HAL.cpp
Content     :   
Created     :   Mar 2011
Authors     :   Bart Muzzin

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Kernel/SF_Debug.h"
#include "Kernel/SF_Random.h"
#include "Render/D3D1x/D3D1x_HAL.h"
#include "Render/Render_BufferGeneric.h"
#include "Kernel/SF_HeapNew.h"
#include "Render/D3D1x/D3D1x_Events.h"
#include "Render/Render_TextureCacheGeneric.h"

#include <stdio.h>

#if SF_CC_MSVC >= 1600 // MSVC 2010
#include <crtdefs.h>
#endif // SF_CC_MSVC >= 1600 // MSVC 2010

namespace Scaleform { namespace Render { namespace D3D1x {

#if !defined(SF_BUILD_SHIPPING) && !defined(__ID3DUserDefinedAnnotation_FWD_DEFINED__) && !defined(SF_DURANGO_MONOLITHIC)
LPD3DPERF_BEGINEVENT D3D1x::RenderEvent::BeginEventFn =0;
LPD3DPERF_ENDEVENT   D3D1x::RenderEvent::EndEventFn   =0;
#endif
ID3D1x(DeviceContext)*     D3D1x::RenderEvent::pContext = 0;

// ***** HAL_D3D1x

HAL::HAL(ThreadCommandQueue* commandQueue)
:   Render::ShaderHAL<ShaderManager, ShaderInterface>(commandQueue),
    pDevice(0),
    pDeviceContext(0),
    pRenderTargetView(0), 
    pDepthStencilView(0),
    Cache(Memory::GetGlobalHeap(), MeshCacheParams::PC_Defaults),
    PrevBatchType(PrimitiveBatch::DP_None),
    CurrentConstantBuffer(0)
{
}

HAL::~HAL()
{
    ShutdownHAL();
}

// *** RenderHAL_D3D1x Implementation
    
bool HAL::InitHAL(const D3D1x::HALInitParams& params)
{
    if ( !initHAL(params))
        return false;

    if (!params.pD3DDevice)
        return 0;

    pDevice = params.pD3DDevice;

    D3D11(
        ID3D1x(DeviceContext)* pd3dDeviceContext = params.pD3DContext;
        if (!pd3dDeviceContext)
            pDevice->GetImmediateContext(&pd3dDeviceContext);
        );

    D3D10_11( pDeviceContext = pDevice, pDeviceContext  = pd3dDeviceContext);

    D3D1x::RenderEvent::InitializeEvents(pDeviceContext);

    pDevice->AddRef();
    pDeviceContext->AddRef();

    // If SetDevice fails, it means that creating queries failed. In D3D1x, we do not support this case.
    if ( !RSync.SetDevice(pDevice, pDeviceContext))
    {
        SF_DEBUG_ASSERT(0, "Failed to create D3D query.");
        ShutdownHAL();
        return false;
    }

    if ( createBlendStates() &&
         createDepthStencilStates() &&
         createRasterStates() &&
         createConstantBuffers() && 
         SManager.Initialize(this) &&
         Cache.Initialize(pDevice, pDeviceContext, &SManager))
    {
        // Create Texture manager if needed.
        if (params.pTextureManager)
            pTextureManager = params.GetTextureManager();
        else
        {
            pTextureManager = 
                *SF_HEAP_AUTO_NEW(this) TextureManager(pDevice, pDeviceContext,
                                                       params.RenderThreadId, pRTCommandQueue);
            if (!pTextureManager)
            {
                Cache.Reset();
                SManager.Reset();
            }
        }

        // Create RenderBufferManager if needed.
        if (params.pRenderBufferManager)
            pRenderBufferManager = params.pRenderBufferManager;
        else
        {
            // Create the default render target, and manager.
            pRenderBufferManager = *SF_HEAP_AUTO_NEW(this) RenderBufferManagerGeneric(RBGenericImpl::DSSM_Exact);
            if ( !pRenderBufferManager || !createDefaultRenderBuffer())
            {
                ShutdownHAL();
                return false;
            }
        }

        // Success.
        if ( pTextureManager && pRenderBufferManager )
        {
            HALState|= HS_ModeSet;
            notifyHandlers(HAL_Initialize);
            return true;
        }
    }

    // Failure.
    pDevice->Release();
    pDeviceContext->Release();
    pDevice = 0;
    pDeviceContext = 0;
    return false;
}

// Returns back to original mode (cleanup)
bool HAL::ShutdownHAL()
{
    if (!(HALState & HS_ModeSet))
        return true;

    if (!shutdownHAL())
        return false;

    D3D1x::RenderEvent::ShutdownEvents();

    // Destroy device states.
    destroyBlendStates();
    destroyDepthStencilStates();
    destroyRasterStates();
    destroyConstantBuffers();

    // Release the views, if they exist.
    pRenderTargetView = 0;
    pDepthStencilView = 0;

    destroyRenderBuffers();
    pRenderBufferManager.Clear();

    // This must happen before destroying mesh buffers. If the device is lost, then the WaitFence implementation
    // depends on the RenderSync to be reset before any meshes are destroy. Otherwise and infinite wait may occur.
    RSync.SetDevice(0, 0);

    // Do TextureManager::Reset to ensure shutdown on the current thread.
    pTextureManager->Reset();
    pTextureManager.Clear();
    SManager.Reset();
    Cache.Reset();

    pDeviceContext->Release();
    pDeviceContext  = 0;

    pDevice->Release();
    pDevice         = 0;
    return true;
}

void HAL::PrepareForReset()
{
    SF_ASSERT(HALState & HS_ModeSet);
    if (HALState & HS_ReadyForReset)
        return;

    // Release the default rendertarget, and depth stencil references.
    if ( RenderTargetStack.GetSize() > 0 )
    {
        RenderTargetEntry& entry = RenderTargetStack[0];
        if ( entry.pRenderTarget )
        {
            SF_ASSERT( entry.pRenderTarget->GetType() == RBuffer_Default);
            D3D1x::RenderTargetData* phd = (D3D1x::RenderTargetData*)entry.pRenderTarget->GetRenderTargetData();
            if ( phd && phd->pRenderSurface )
            {
                phd->pRenderSurface->Release();
                phd->pRenderSurface = 0;
            }
            if ( phd && phd->pDSSurface )
            {
                phd->pDSSurface->Release();
                phd->pDSSurface = 0;
            }
        }
    }
    
    notifyHandlers(HAL_PrepareForReset);
    HALState |= HS_ReadyForReset;
}

bool HAL::RestoreAfterReset()
{
    if (!IsInitialized())
        return false;
    if (!(HALState & HS_ReadyForReset))
        return true;

    // Reobtain the default rendertarget and depthstencil references.
    if ( RenderTargetStack.GetSize() > 0 )
    {
        RenderTargetEntry& entry = RenderTargetStack[0];

        ID3D1x(RenderTargetView)* rtView;
        ID3D1x(DepthStencilView)* dsView;
        pDeviceContext->OMGetRenderTargets(1, &rtView, &dsView);

        RenderTargetData* phd = 0;
        if ( entry.pRenderTarget && rtView)
        {
            SF_ASSERT( entry.pRenderTarget->GetType() == RBuffer_Default);
            phd = (D3D1x::RenderTargetData*)entry.pRenderTarget->GetRenderTargetData();
            SF_ASSERT( phd->pRenderSurface == 0 );
            phd->pRenderSurface = rtView;
            if ( dsView )
                phd->pDSSurface = dsView;
        }

        // Note: No Release called on resource views, it acts as an AddRef on them,
        // which the RenderTargetData class expects.
    }

    notifyHandlers(HAL_RestoreAfterReset);
    HALState &= ~HS_ReadyForReset;
    return true;
}

// ***** Rendering

bool HAL::BeginScene()
{
    // Render event for anything that is done immediately within BeginScene (but not the entire scene).
    ScopedRenderEvent GPUEvent(GetEvent(Event_BeginScene), __FUNCTION__ "-SetState");

    if ( !BaseHAL::BeginScene() )
        return false;

    // Get the rendertarget and depth surfaces we will render the scene to.
    pDeviceContext->OMGetRenderTargets(1, &pRenderTargetView.GetRawRef(), &pDepthStencilView.GetRawRef());

    pDeviceContext->IASetPrimitiveTopology(D3D1x(PRIMITIVE_TOPOLOGY_TRIANGLELIST));
    pDeviceContext->OMSetDepthStencilState(DepthStencilStates[DepthStencil_Disabled], 0);
    CurrentConstantBuffer = 0;

    return true;
}

bool HAL::EndScene()
{
    ScopedRenderEvent GPUEvent(GetEvent(Event_Scene), 0, false);

    if ( !BaseHAL::EndScene())
        return false;

    // Release the views.
    pRenderTargetView = 0;
    pDepthStencilView = 0;

    return true;
}

// Updates D3D HW Viewport and ViewportMatrix based on provided viewport
// and view rectangle.
void HAL::updateViewport()
{
    D3D1x(VIEWPORT) vp;
    Rect<int>      vpRect;

    if (HALState & HS_ViewValid)
    {
        int dx = ViewRect.x1 - VP.Left,
            dy = ViewRect.y1 - VP.Top;
        
        // Modify HW matrix and viewport to clip.
        CalcHWViewMatrix(0, &Matrices->View2D, ViewRect, dx, dy);
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

        if ( !(HALState & HS_InRenderTarget))
            vpRect = ViewRect;
        else
            vpRect.SetRect(VP.Left, VP.Top, VP.Left + VP.Width, VP.Top + VP.Height);
    }

    vp.TopLeftX = D3D10_11((UINT),(FLOAT))vpRect.x1;
    vp.TopLeftY = D3D10_11((UINT),(FLOAT))vpRect.y1;
    vp.Width    = D3D10_11((UINT),(FLOAT))vpRect.Width();
    vp.Height   = D3D10_11((UINT),(FLOAT))vpRect.Height();

    // DX can't handle a vp with zero area.
    vp.Width  = Alg::Max<D3D10_11(UINT,FLOAT)>(vp.Width, 1);
    vp.Height = Alg::Max<D3D10_11(UINT,FLOAT)>(vp.Height, 1);

    vp.MinDepth = 0.0f;
    vp.MaxDepth = 0.0f;
    pDeviceContext->RSSetViewports(1, &vp);
}

UPInt HAL::setVertexArray(PrimitiveBatch* pbatch, Render::MeshCacheItem* pmeshBase)
{
    D3D1x::MeshCacheItem* pmesh = reinterpret_cast<D3D1x::MeshCacheItem*>(pmeshBase);
    ID3D1x(Buffer)* pb          = (pmesh->pVertexBuffer->GetHWBuffer());
    UINT offset                 = (UINT)pmesh->VBAllocOffset;

    pDeviceContext->IASetIndexBuffer(pmesh->pIndexBuffer->GetHWBuffer(), sizeof(IndexType) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0 );
    pDeviceContext->IASetVertexBuffers(0, 1, &pb, &(pbatch->pFormat->Size), &offset );
    return pmesh->IBAllocOffset / sizeof (IndexType);
}

UPInt HAL::setVertexArray(const ComplexMesh::FillRecord& fr, unsigned formatIndex, Render::MeshCacheItem* pmeshBase)
{
    D3D1x::MeshCacheItem* pmesh         = reinterpret_cast<D3D1x::MeshCacheItem*>(pmeshBase);
    ID3D1x(Buffer)* pb                  = (pmesh->pVertexBuffer->GetHWBuffer());
    UINT byteOffset                     = (UINT)(fr.VertexByteOffset + pmesh->VBAllocOffset);
    const Render::VertexFormat* pformat = fr.pFormats[formatIndex];
    UINT  vertexSize                    = pformat->Size;

    pDeviceContext->IASetVertexBuffers(0, 1, &pb, &vertexSize, &byteOffset);
    pDeviceContext->IASetIndexBuffer(pmesh->pIndexBuffer->GetHWBuffer(), sizeof(IndexType) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
    return (unsigned)(pmesh->IBAllocOffset / sizeof(IndexType));
}

//--------------------------------------------------------------------
// *** Mask / Stencil support
//--------------------------------------------------------------------
void HAL::applyDepthStencilMode(DepthStencilMode mode, unsigned stencilRef)
{
    ScopedRenderEvent GPUEvent(GetEvent(Event_ApplyDepthStencil), __FUNCTION__);
    if (mode != CurrentDepthStencilState || CurrentStencilRef != stencilRef)
    {
        pDeviceContext->OMSetDepthStencilState(DepthStencilStates[mode], (UINT)stencilRef);
        CurrentStencilRef = stencilRef;

        if (DepthStencilModeTable[mode].ColorWriteEnable != DepthStencilModeTable[CurrentDepthStencilState].ColorWriteEnable)
        {
            // applyBlendModeImpl will check the CurrentDepthStencilState, and set CWE appropriately, so set it before calling.
            // Note that we are calling the Impl directly, as applyBlendMode may detect this as a duplicate setting of the same state.
            CurrentDepthStencilState = mode;
            applyBlendModeImpl(CurrentBlendState.Mode, CurrentBlendState.SourceAc, CurrentBlendState.ForceAc);
        }
        CurrentDepthStencilState = mode;
    }
}

bool HAL::checkDepthStencilBufferCaps()
{
    if (!StencilChecked)
    {
        StencilAvailable = 0;
        DepthBufferAvailable = 0;

        // Test for depth-stencil presence.
        Ptr<ID3D1x(DepthStencilView)> depthStencilTarget;
        Ptr<ID3D1x(RenderTargetView)> renderTarget; // Note: not strictly necessary, but causes PIX to crash if NULL.
        pDeviceContext->OMGetRenderTargets(1, &renderTarget.GetRawRef(), &depthStencilTarget.GetRawRef() );

        if (depthStencilTarget)
        {
            D3D1x(DEPTH_STENCIL_VIEW_DESC) desc;
            depthStencilTarget->GetDesc(&desc);
            switch(desc.Format)
            {
                case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
                case DXGI_FORMAT_D24_UNORM_S8_UINT:
                    StencilAvailable = 1;
                    MultiBitStencil  = 1;
                    DepthBufferAvailable = 1;
                    break;

                case DXGI_FORMAT_D16_UNORM:
                case DXGI_FORMAT_D32_FLOAT:
                    DepthBufferAvailable = 1;
                    break;
                default:
                    SF_DEBUG_ASSERT1(1, "Unexpected DepthStencil surface format: 0x%08x", desc.Format);
                    break;
            }
        }
        StencilChecked = 1;
    }

    if (!StencilAvailable && !DepthBufferAvailable)
    {
        SF_DEBUG_WARNONCE(1, "RendererHAL::PushMask_BeginSubmit used, but stencil is not available");
        return false;
    }
    return true;
}

bool HAL::IsRasterModeSupported(RasterModeType mode) const
{
    // D3D1x doesn't support point.
    return mode != HAL::RasterMode_Point;
}

void HAL::applyRasterModeImpl(RasterModeType mode)
{
    pDeviceContext->RSSetState(RasterStates[mode]);
}

//--------------------------------------------------------------------
// *** BlendMode Stack support
//--------------------------------------------------------------------

// Structure describing color combines applied for a given blend mode.
struct BlendModeDescAlpha
{
    D3D1x(BLEND_OP)  BlendOp;
    D3D1x(BLEND)     SrcArg, DestArg;
    D3D1x(BLEND)     SrcAlphaArg, DestAlphaArg;
};

bool HAL::createBlendStates()
{
    static const D3D1x(BLEND_OP) BlendOps[BlendOp_Count] = 
    {
        D3D1x(BLEND_OP_ADD),            // BlendOp_ADD
        D3D1x(BLEND_OP_MAX),            // BlendOp_MAX
        D3D1x(BLEND_OP_MIN),            // BlendOp_MIN
        D3D1x(BLEND_OP_REV_SUBTRACT),   // BlendOp_REVSUBTRACT
    };

    static const D3D1x(BLEND) BlendFactors[BlendFactor_Count] = 
    {
        D3D1x(BLEND_ZERO),              // BlendFactor_ZERO
        D3D1x(BLEND_ONE),               // BlendFactor_ONE
        D3D1x(BLEND_SRC_ALPHA),         // BlendFactor_SRCALPHA
        D3D1x(BLEND_INV_SRC_ALPHA),     // BlendFactor_INVSRCALPHA
        D3D1x(BLEND_DEST_COLOR),        // BlendFactor_DESTCOLOR
        D3D1x(BLEND_INV_DEST_COLOR),    // BlendFactor_INVDESTCOLOR
    };

    memset(BlendStates, 0, sizeof BlendStates);
    for (unsigned i = 0; i < BlendTypeCount; ++i )
    {
        D3D1x(BLEND_DESC) desc;

        memset(&desc, 0, sizeof desc);
        unsigned mode = i;
        bool sourceAc = false;

        D3D10_11
        (
        D3D10_BLEND_DESC& rt0 = desc;,
        D3D11_RENDER_TARGET_BLEND_DESC& rt0 = desc.RenderTarget[0];
        );
        
        if ( i >= Blend_Count*2 )
        {
#if (SF_D3D_VERSION == 11)
            // Anything above normal blends has write mask disabled.
            rt0.BlendEnable            = FALSE;
            rt0.RenderTargetWriteMask  = 0;
#else
            // Anything above normal blends has write mask disabled.
			memset(rt0.BlendEnable, FALSE, sizeof(rt0.BlendEnable));
            memset(rt0.RenderTargetWriteMask, 0, sizeof(rt0.RenderTargetWriteMask));
#endif
        }
        else 
        {
#if (SF_D3D_VERSION == 11)
            rt0.BlendEnable           = TRUE;
            rt0.RenderTargetWriteMask = D3D1x(COLOR_WRITE_ENABLE_ALL);
#else
        memset(rt0.BlendEnable, TRUE, sizeof(rt0.BlendEnable));
        memset(rt0.RenderTargetWriteMask, D3D1x(COLOR_WRITE_ENABLE_ALL), sizeof(rt0.RenderTargetWriteMask));
#endif

            if ( i >= Blend_Count )
            {
                mode -= Blend_Count;
                sourceAc = true;
            }
        }
        mode %= Blend_Count;

        rt0.BlendOp        = BlendOps[BlendModeTable[mode].Operator];
        rt0.SrcBlend       = BlendFactors[BlendModeTable[mode].SourceColor];
        rt0.DestBlend      = BlendFactors[BlendModeTable[mode].DestColor];
        rt0.BlendOpAlpha   = BlendOps[BlendModeTable[mode].AlphaOperator];
        rt0.SrcBlendAlpha  = BlendFactors[BlendModeTable[mode].SourceAlpha];
        rt0.DestBlendAlpha = BlendFactors[BlendModeTable[mode].DestAlpha];

        if ( sourceAc && rt0.SrcBlend == D3D1x(BLEND_SRC_ALPHA) )
            rt0.SrcBlend        = D3D1x(BLEND_ONE);

        if ( FAILED(pDevice->CreateBlendState(&desc, &BlendStates[i]) ))
            return false;
    }
    return true;
}

void HAL::destroyBlendStates()
{
    for (unsigned i = 0; i < BlendTypeCount; ++i )
    {
        if ( BlendStates[i] )
            BlendStates[i]->Release();
    }
    memset(BlendStates, 0, sizeof BlendStates);
}

bool HAL::createDepthStencilStates()
{
    memset(DepthStencilStates, 0, sizeof DepthStencilStates);
    for ( unsigned state = 0; state < DepthStencil_Count; ++state)
    {
        D3D1x(DEPTH_STENCIL_DESC) desc;

        // Set (non-zero) default.
        memset(&desc, 0, sizeof desc);

        const HALDepthStencilDescriptor& mode = DepthStencilModeTable[state];

        // Mapping tables.
        static const D3D1x(COMPARISON_FUNC) ComparisonFunctions[DepthStencilFunction_Count] =
        {
            D3D1x(COMPARISON_ALWAYS),           // DepthStencilFunction_Ignore,
            D3D1x(COMPARISON_NEVER),            // DepthStencilFunction_Never,
            D3D1x(COMPARISON_LESS),             // DepthStencilFunction_Less,
            D3D1x(COMPARISON_EQUAL),            // DepthStencilFunction_Equal,
            D3D1x(COMPARISON_LESS_EQUAL),       // DepthStencilFunction_LessEqual,
            D3D1x(COMPARISON_GREATER),          // DepthStencilFunction_Greater,
            D3D1x(COMPARISON_NOT_EQUAL),        // DepthStencilFunction_NotEqual,
            D3D1x(COMPARISON_GREATER_EQUAL),    // DepthStencilFunction_GreaterEqual,
            D3D1x(COMPARISON_ALWAYS),           // DepthStencilFunction_Always
        };

        static const D3D1x(STENCIL_OP) StencilOperations[StencilOp_Count] =
        {
            D3D1x(STENCIL_OP_KEEP),     //StencilOp_Ignore,
            D3D1x(STENCIL_OP_KEEP),     //StencilOp_Keep,
            D3D1x(STENCIL_OP_REPLACE),  //StencilOp_Replace,
            D3D1x(STENCIL_OP_INCR)      //StencilOp_Increment
        };

        desc.StencilEnable                = mode.StencilEnable ? TRUE : FALSE;
        desc.DepthEnable                  = (mode.DepthTestEnable || mode.DepthWriteEnable) ? TRUE: FALSE;
        desc.DepthWriteMask               = mode.DepthWriteEnable ? D3D1x(DEPTH_WRITE_MASK_ALL) : D3D1x(DEPTH_WRITE_MASK_ZERO);
        desc.DepthFunc                    = ComparisonFunctions[mode.DepthFunction];
        desc.FrontFace.StencilFunc        = ComparisonFunctions[mode.StencilFunction];
        desc.FrontFace.StencilPassOp      = StencilOperations[mode.StencilPassOp];
        desc.FrontFace.StencilFailOp      = StencilOperations[mode.StencilFailOp];
        desc.FrontFace.StencilDepthFailOp = StencilOperations[mode.StencilZFailOp];
        desc.StencilReadMask              = 0xFF;
        desc.StencilWriteMask             = 0xFF;

        // Make backface identical to front face.
        memcpy(&desc.BackFace, &desc.FrontFace, sizeof D3D1x(DEPTH_STENCILOP_DESC));

        if (FAILED(pDevice->CreateDepthStencilState(&desc, &DepthStencilStates[state])))
            return false;
    }
    return true;
}

void HAL::destroyDepthStencilStates()
{
    for ( unsigned state = 0; state < DepthStencil_Count; ++state)
    {
        if ( DepthStencilStates[state] )
            DepthStencilStates[state]->Release();
    }
    memset(DepthStencilStates, 0, sizeof DepthStencilStates);
}

bool HAL::createRasterStates()
{
    memset(RasterStates, 0, sizeof RasterStates);
    for ( unsigned state = 0; state < RasterMode_Count; ++state )
    {
        D3D1x(RASTERIZER_DESC) rsdesc;
        memset(&rsdesc, 0, sizeof rsdesc);
        rsdesc.CullMode = D3D1x(CULL_NONE);
		rsdesc.DepthClipEnable = TRUE;
        switch(state)
        {
            case RasterMode_Wireframe: rsdesc.FillMode = D3D1x(FILL_WIREFRAME); break;
            default:                   rsdesc.FillMode = D3D1x(FILL_SOLID); break;
        }              
        if ( FAILED(pDevice->CreateRasterizerState( &rsdesc, &RasterStates[state] )) )
            return false;
    }
    return true;
}

void HAL::destroyRasterStates()
{
    for ( unsigned state = 0; state < RasterMode_Count; ++state)
    {
        if ( RasterStates[state] )
            RasterStates[state]->Release();
    }
    memset(RasterStates, 0, sizeof RasterStates);
}

bool HAL::createConstantBuffers()
{
    memset(ConstantBuffers, 0, sizeof ConstantBuffers);
    D3D1x(BUFFER_DESC) desc;
    memset(&desc, 0, sizeof desc);
    for ( unsigned b = 0; b < ConstantBufferCount; ++b )
    {
        desc.ByteWidth      = Alg::Max(Uniform::SU_VertexSize,Uniform::SU_FragSize) * 4 * sizeof(float);
        desc.Usage          = D3D1x(USAGE_DYNAMIC);
        desc.BindFlags      = D3D1x(BIND_CONSTANT_BUFFER);
        desc.CPUAccessFlags = D3D1x(CPU_ACCESS_WRITE);
        if ( FAILED( pDevice->CreateBuffer( &desc, 0, &ConstantBuffers[b] ) ))
            return false;
    }
    return true;
}

void HAL::destroyConstantBuffers()
{
    for ( unsigned b = 0; b < ConstantBufferCount; ++b )
    {
        if ( ConstantBuffers[b] )
            ConstantBuffers[b]->Release();
    }
    memset(ConstantBuffers, 0, sizeof ConstantBuffers);
}

ID3D1x(Buffer)* HAL::getNextConstantBuffer()
{
    CurrentConstantBuffer++; 
    CurrentConstantBuffer %= ConstantBufferCount; 
    return ConstantBuffers[CurrentConstantBuffer];
}

void HAL::applyBlendModeImpl(BlendMode mode, bool sourceAc, bool forceAc )
{    
    SF_UNUSED(forceAc);
    if (!pDeviceContext)
        return;

    unsigned blendType = GetBlendType(mode, DepthStencilModeTable[CurrentDepthStencilState].ColorWriteEnable ? Write_All : Write_None, sourceAc);
    pDeviceContext->OMSetBlendState(BlendStates[blendType], 0, 0xFFFFFFFF );
}

RenderTarget*   HAL::CreateRenderTarget(ID3D1x(View)* pcolor, ID3D1x(View)* pdepth)
{
    Ptr<ID3D1x(Texture2D)> prenderTarget;
    Ptr<ID3D1x(Texture2D)> pdepthStencilTarget;
    D3D1x(TEXTURE2D_DESC) rtDesc;
    D3D1x(TEXTURE2D_DESC) dsDesc;
    ImageSize dsSize(0), rtSize(0);

    pcolor->GetResource((ID3D1x(Resource)**)(&prenderTarget.GetRawRef()));
    prenderTarget->GetDesc(&rtDesc);
    rtSize.SetSize(rtDesc.Width, rtDesc.Height);

    Ptr<RenderTarget> ptarget = pRenderBufferManager->CreateRenderTarget(rtSize, RBuffer_User, Image_R8G8B8A8, 0);

    Ptr<DepthStencilBuffer> pdsb = 0;
    if (pdepth)
    {
        pdepth->GetResource((ID3D1x(Resource)**)(&pdepthStencilTarget.GetRawRef()));
        pdepthStencilTarget->GetDesc(&dsDesc);
        ImageSize dsImgSize(dsDesc.Width, dsDesc.Height);
        pdsb = *SF_HEAP_AUTO_NEW(this) DepthStencilBuffer(0, dsImgSize);
    }

    RenderTargetData::UpdateData(ptarget, pcolor, pdsb, pdepth);
    return ptarget;
}

RenderTarget* HAL::CreateRenderTarget(Render::Texture* texture, bool needsStencil)
{
    D3D1x::Texture* pt = (D3D1x::Texture*)texture;

    // Cannot render to textures which have multiple HW representations.
    if ( !pt || pt->TextureCount != 1 )
        return 0;
    RenderTarget* prt = pRenderBufferManager->CreateRenderTarget(
        texture->GetSize(), RBuffer_Texture, texture->GetFormat(), texture);   
    if ( !prt )
        return 0;

    Ptr<ID3D1x(RenderTargetView)> prtView;
    ID3D1x(DepthStencilView)* pdsView = 0;
    if ( !prt || FAILED(pDevice->CreateRenderTargetView(pt->pTextures[0].pTexture, 0, &prtView.GetRawRef() )) )
        return 0;
    Ptr<DepthStencilBuffer> pdsb =0;
    if ( needsStencil )
    {
        pdsb = *pRenderBufferManager->CreateDepthStencilBuffer(texture->GetSize(), false); // false == not temporary
        if ( pdsb )
        {
            DepthStencilSurface* surf = (D3D1x::DepthStencilSurface*)pdsb->GetSurface();
            if ( surf )
                pdsView = surf->pDepthStencilSurfaceView;
        }
    }
    RenderTargetData::UpdateData(prt, prtView, pdsb, pdsView);
    return prt;
}

RenderTarget* HAL::CreateTempRenderTarget(const ImageSize& size, bool needsStencil)
{
    RenderTarget* prt = pRenderBufferManager->CreateTempRenderTarget(size);
    if ( !prt )
        return 0;

    RenderTargetData* phd = (RenderTargetData*)prt->GetRenderTargetData();
    if ( phd && (!needsStencil || phd->pDepthStencilBuffer))
        return prt;

    D3D1x::Texture* pt = (D3D1x::Texture*)prt->GetTexture();

    // Cannot render to textures which have multiple HW representations.
    SF_ASSERT(pt->TextureCount == 1); 
    Ptr<ID3D1x(RenderTargetView)> prtView;
    ID3D1x(DepthStencilView)* pdsView = 0;
    if ( FAILED(pDevice->CreateRenderTargetView(pt->pTextures[0].pTexture, 0, &prtView.GetRawRef() )) )
        return 0;

    Ptr<DepthStencilBuffer> pdsb = 0;
    if ( needsStencil )
    {   
        pdsb = *pRenderBufferManager->CreateDepthStencilBuffer(pt->GetSize());
        if ( pdsb )
        {
            DepthStencilSurface* surf = (D3D1x::DepthStencilSurface*)pdsb->GetSurface();
            if ( surf )
                pdsView = surf->pDepthStencilSurfaceView;
        }
    }

    RenderTargetData::UpdateData(prt, prtView, pdsb, pdsView);
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

    RenderTargetEntry entry;
    RenderTargetData* prtdata = (D3D1x::RenderTargetData*)ptarget->GetRenderTargetData();
    ID3D1x(RenderTargetView)* prtView = (ID3D1x(RenderTargetView)*)prtdata->pRenderSurface;
    ID3D1x(DepthStencilView)* pdsView = (ID3D1x(DepthStencilView)*)prtdata->pDSSurface;

    if ( setState )
    {
        pDeviceContext->OMSetRenderTargets(1, &prtView, pdsView);
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
    Matrices->Orient2D.SetIdentity();
    Matrices->Orient3D.SetIdentity();
    Matrices->SetUserMatrix(Matrix2F::Identity);

    // Setup the render target/depth stencil on the device.
    if ( !prt )
    {
        SF_DEBUG_WARNING(1, __FUNCTION__ " - invalid render target.");
        RenderTargetStack.PushBack(entry);
        return;
    }
    RenderTargetData* phd = (D3D1x::RenderTargetData*)prt->GetRenderTargetData();

    // Unbind the texture if it is bound (can't have a texture bound as an input and target).
    ID3D1x(ShaderResourceView)* clearViews[2];
    memset(clearViews, 0, sizeof clearViews);
    pTextureManager->SetSamplerState(0, 2, clearViews);

    ID3D1x(DepthStencilView)* pdsView = (ID3D1x(DepthStencilView)*)phd->pDSSurface;
    pDeviceContext->OMSetRenderTargets(1, (ID3D1x(RenderTargetView)**)&phd->pRenderSurface, pdsView);
    StencilChecked = false;
    ++AccumulatedStats.RTChanges;

    // Clear, if not specifically excluded
    if ( (flags & PRT_NoClear) == 0 )
    {
        float clear[4];
        clearColor.GetRGBAFloat(clear);
        pDeviceContext->ClearRenderTargetView((ID3D1x(RenderTargetView)*)phd->pRenderSurface, clear);
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
    ScopedRenderEvent GPUEvent(GetEvent(Event_RenderTarget), 0, false);

    RenderTargetEntry& entry = RenderTargetStack.Back();
    RenderTarget* prt = entry.pRenderTarget;
    if ( prt->GetType() == RBuffer_Temporary )
    {
        // Strip off the depth stencil surface/buffer from temporary targets.
        D3D1x::RenderTargetData* plasthd = (D3D1x::RenderTargetData*)prt->GetRenderTargetData();
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
    ID3D1x(DepthStencilView)* pds = 0;
    RenderTargetData* phd = 0;
    if ( RenderTargetStack.GetSize() > 0 )
    {
        RenderTargetEntry& back = RenderTargetStack.Back();
        phd = (D3D1x::RenderTargetData*)back.pRenderTarget->GetRenderTargetData();
        pds = (ID3D1x(DepthStencilView)*)phd->pDSSurface;
    }

    if ( RenderTargetStack.GetSize() == 1 )
        HALState &= ~HS_InRenderTarget;

    // Unbind the texture if it is bound (can't have a texture bound as an input and target).
    ID3D1x(ShaderResourceView)* clearViews[2];
    memset(clearViews, 0, sizeof clearViews);
    pTextureManager->SetSamplerState(0, 2, clearViews);

    // Restore the old render target on the device.
    if ((flags & PRT_NoSet) == 0)
    {
        pDeviceContext->OMSetRenderTargets(1, (ID3D1x(RenderTargetView)**)&phd->pRenderSurface, pds );
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
        Ptr<ID3D1x(RenderTargetView)> prtView;
        Ptr<ID3D1x(DepthStencilView)> pdsView;
        Ptr<ID3D1x(Texture2D)> prenderTarget;
        Ptr<ID3D1x(Texture2D)> pdepthStencilTarget;
        D3D1x(TEXTURE2D_DESC) rtDesc;
        D3D1x(TEXTURE2D_DESC) dsDesc;


        pDeviceContext->OMGetRenderTargets(1, &prtView.GetRawRef(), &pdsView.GetRawRef());
        prtView->GetResource((ID3D1x(Resource)**)(&prenderTarget.GetRawRef()));
        prenderTarget->GetDesc(&rtDesc);

        rtSize = ImageSize(rtDesc.Width, rtDesc.Height);
        Ptr<RenderTarget> ptarget = *SF_HEAP_AUTO_NEW(this) RenderTarget(0, RBuffer_Default, rtSize );
        Ptr<DepthStencilBuffer> pdsb = 0;
        if (pdsView)
        {
            prtView->GetResource((ID3D1x(Resource)**)(&pdepthStencilTarget.GetRawRef()));
            pdepthStencilTarget->GetDesc(&dsDesc);
            ImageSize dsSize(dsDesc.Width, dsDesc.Height);
            pdsb = *SF_HEAP_AUTO_NEW(this) DepthStencilBuffer(0, dsSize);
        }
        RenderTargetData::UpdateData(ptarget, prtView, pdsb, pdsView);

        if ( !SetRenderTarget(ptarget))
            return false;
    }

    return pRenderBufferManager->Initialize(pTextureManager, Image_R8G8B8A8, rtSize);
}

void HAL::setBatchUnitSquareVertexStream()
{
    static const UINT stride = sizeof(VertexXY16fAlpha);
    static const UINT offset = 0;
    ID3D1x(Buffer) * pb = Cache.pMaskEraseBatchVertexBuffer.GetPtr();
    pDeviceContext->IASetVertexBuffers(0, 1, &pb, &stride, &offset );
}

void HAL::drawPrimitive(unsigned indexCount, unsigned meshCount)
{
    pDeviceContext->Draw( indexCount, 0 );

    SF_UNUSED(meshCount);
#if !defined(SF_BUILD_SHIPPING)
    AccumulatedStats.Meshes += meshCount;
    AccumulatedStats.Triangles += indexCount / 3;
    AccumulatedStats.Primitives++;
#endif
}

void HAL::drawIndexedPrimitive(unsigned indexCount, unsigned vertexCount, unsigned meshCount, UPInt indexPtr, UPInt vertexOffset )
{
    pDeviceContext->DrawIndexed( indexCount, (UINT)indexPtr, (INT)vertexOffset);

    SF_UNUSED2(meshCount, vertexCount);
#if !defined(SF_BUILD_SHIPPING)
    AccumulatedStats.Meshes += meshCount;
    AccumulatedStats.Triangles += indexCount / 3;
    AccumulatedStats.Primitives++;
#endif
}

void HAL::drawIndexedInstanced(unsigned indexCount, unsigned vertexCount, unsigned meshCount, UPInt indexPtr, UPInt vertexOffset )
{
    pDeviceContext->DrawIndexedInstanced( indexCount, meshCount, (UINT)indexPtr, (INT)vertexOffset, 0 );

    SF_UNUSED2(meshCount, vertexCount);
#if !defined(SF_BUILD_SHIPPING)
    AccumulatedStats.Meshes += meshCount;
    AccumulatedStats.Triangles += (indexCount / 3) * meshCount;
    AccumulatedStats.Primitives++;
#endif
}

bool HAL::shouldRenderFilters(const FilterPrimitive* prim) const
{
#if defined(_DURANGO)
    // Durango can always render all filters.
    return true;
#else
    // FeatureLevel 10.0+ can always render all filters.
    if (SManager.GetShaderVersion() >= ShaderDesc::ShaderVersion_D3D1xFL10X)
        return true;

    // If the profile doesn't support dynamic loops, check to see if there are any ColorMatrix
    // filters, which can still be rendered. If there are, allow filtering still.
    const FilterSet* filters = prim->GetFilters();
    unsigned filterCount = filters->GetFilterCount();
    for ( unsigned f = 0; f < filterCount; ++f )
    {
        const Filter* filter = filters->GetFilter(f);
        if ( filter->GetFilterType() == Filter_ColorMatrix )
            return true;
    }
    return false;
#endif
}

Render::RenderEvent& HAL::GetEvent(EventType eventType)
{
#if !defined(SF_BUILD_SHIPPING) && !defined(SF_OS_WINMETRO)
    static D3D1x::RenderEvent D3D1xEvents[Event_Count];
    return D3D1xEvents[eventType];
#else
    // Shipping/Metro builds just return a default event, which does nothing.
    return Render::HAL::GetEvent(eventType);
#endif
}

void HAL::drawScreenQuad()
{
    ID3D1x(Buffer)* pb = Cache.pMaskEraseBatchVertexBuffer.GetPtr();
    static const unsigned stride = sizeof(VertexXY16fAlpha);
    static const unsigned offset = 0;
    pDeviceContext->IASetVertexBuffers(0, 1, &pb, &stride, &offset );
    drawPrimitive(6,1);
}

}}} // Scaleform::Render::D3D1x

