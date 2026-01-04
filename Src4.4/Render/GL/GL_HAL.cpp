/**************************************************************************

Filename    :   GL_HAL.cpp
Content     :   GL Renderer HAL Prototype implementation.
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Kernel/SF_Debug.h"
#include "Kernel/SF_Random.h"
#include "Kernel/SF_HeapNew.h"  
#include "Kernel/SF_Debug.h"

#include "Render/Render_TextureCacheGeneric.h"
#include "Render/Render_BufferGeneric.h"
#include "Render/GL/GL_HAL.h"
#include "Render/GL/GL_Events.h"

#include "Render/GL/GL_ExtensionMacros.h"

namespace Scaleform { namespace Render { namespace GL {

class MatrixState : public Render::MatrixState
{
public:
    MatrixState(HAL* phal) : Render::MatrixState((Render::HAL*)phal)
    {
        // GL's full viewport quad is different from other platforms (upside down).
        FullViewportMVP = Matrix2F::Scaling(2,2) * Matrix2F::Translation(-0.5f, -0.5f);
    }

    MatrixState() : Render::MatrixState()
    {
        FullViewportMVP = Matrix2F::Scaling(2,2) * Matrix2F::Translation(-0.5f, -0.5f);
    }

protected:
    virtual void        recalculateUVPOC() const
    {
        if (UVPOChanged)
        {
            // Recalculated the view compensation matrix.
            if ( ViewRect != ViewRectOriginal && !ViewRectOriginal.IsNull())
            {
                Point<int> dc = ViewRect.Center() - ViewRectOriginal.Center();
                float      dx = ((float)ViewRectOriginal.Width()) / ViewRect.Width();
                float      dy = ((float)ViewRectOriginal.Height()) / ViewRect.Height();
                float      ox = 2.0f * dc.x / ViewRect.Width();
                float      oy = 2.0f * dc.y / ViewRect.Height();
                ViewRectCompensated3D.MultiplyMatrix(Matrix4F::Translation(-ox, oy, 0), Matrix4F::Scaling(dx, dy, 1));
            }
            else
            {
                ViewRectCompensated3D = Matrix4F::Identity;
            }

            const Matrix4F& Projection = updateStereoProjection();

            Matrix4F flipmat;
            flipmat.SetIdentity();
            if(pHAL && (pHAL->GetHALState() & HAL::HS_InRenderTarget))
            {
                flipmat.Append(Matrix4F::Scaling(1.0f, -1.0f, 1.0f));
            }

            Matrix4F FV(flipmat, ViewRectCompensated3D);
            Matrix4F UO(User3D, FV);
            Matrix4F VRP(Orient3D, Projection);
            UVPO = Matrix4F(Matrix4F(UO, VRP), View3D);
            UVPOChanged = 0;
        }
    }
};

class MatrixStateFactory : public Render::MatrixStateFactory
{
public:
    MatrixStateFactory(GL::HAL* hal, MemoryHeap* heap) :
        Render::MatrixStateFactory(heap),
        pHAL(hal)
    { }

    virtual MatrixState*       CreateMatrixState() const
    {
        return SF_HEAP_NEW(pHeap) GL::MatrixState(pHAL);
    }
private:
    GL::HAL*    pHAL;
};


// ***** RenderHAL_GL

HAL::HAL(ThreadCommandQueue* commandQueue) :   
    Render::ShaderHAL<ShaderManager, ShaderInterface>(commandQueue),
    EnabledVertexArrays(-1),
    MaxVertexAttributes(0),
    FilterVertexBufferSet(false),
    DeterminedDepthStencilFormat(false),
    Cache(Memory::GetGlobalHeap(), MeshCacheParams::PC_Defaults),
    RSync(),
    Events(GetHAL()),
    pDevice(0),
    pRecordingDevice(0),
    PrevBatchType(PrimitiveBatch::DP_None)
{
}

HAL::~HAL()
{
    ShutdownHAL();
}

RenderTarget* HAL::CreateRenderTarget(GLuint fbo)
{
    Ptr<HALGLFramebuffer> halfbo = *SF_NEW HALGLFramebuffer(fbo, GL_FRAMEBUFFER, ImmediateDevice);
    HALGLFramebuffer* currentFbo = 0;
    RenderTarget* prt = pRenderBufferManager->CreateRenderTarget(getFboInfo(halfbo, currentFbo, false), RBuffer_User, Image_R8G8B8A8, 0);
    if ( !prt )
        return 0;
    if ( prt->GetRenderTargetData() != 0 )
        return prt;

    RenderTargetData::UpdateData(prt, this, fbo, 0);

    // Set the FBO back to the top level of the RenderTargetStack.
    if (RenderTargetStack.GetSize() > 0 && RenderTargetStack.Back().pRenderTarget)
    {
        GL::RenderTargetData* lasthd = reinterpret_cast<GL::RenderTargetData*>(RenderTargetStack.Back().pRenderTarget->GetRenderTargetData());
        if (lasthd)
            glBindFramebuffer(GL_FRAMEBUFFER, lasthd->FBOID);
    }

    return prt;
}

RenderTarget* HAL::CreateRenderTarget(Render::Texture* texture, bool needsStencil)
{
    GL::Texture* pt = (GL::Texture*)texture;

    if ( !pt || pt->TextureCount != 1 )
        return 0;

    RenderTarget* prt = pRenderBufferManager->CreateRenderTarget(
        texture->GetSize(), RBuffer_Texture, texture->GetFormat(), texture);
    if ( !prt )
        return 0;
    Ptr<DepthStencilBuffer> pdsb;

    // Cannot render to textures which have multiple HW representations.
    SF_ASSERT(pt->TextureCount == 1); 
    Ptr<HALGLTexture> colorID = pt->pTextures[0].TexId;
    Ptr<HALGLFramebuffer> fboID = *SF_NEW HALGLFramebuffer();

    glGenFramebuffers(1, &fboID.GetRawRef());
    glBindFramebuffer(GL_FRAMEBUFFER, fboID);
    ++AccumulatedStats.RTChanges;

#if defined(SF_RENDER_GLES)
    // If on GLES2, and it has NPOT limitations, then we need to ensure that the texture
    // uses clamping mode without mipmapping, otherwise the glCheckFramebufferStatus will 
    // return that the target is unsupported.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
#endif

    // Bind the color buffer.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorID, 0);

    // Create (and bind) the depth/stencil buffers if required.
    if ( needsStencil )
        pdsb = *createCompatibleDepthStencil(texture->GetSize(), false);

    RenderTargetData::UpdateData(prt, this, fboID, pdsb);

    // Set the FBO back to the top level of the RenderTargetStack.
    if (RenderTargetStack.GetSize() > 0 && RenderTargetStack.Back().pRenderTarget)
    {
        GL::RenderTargetData* lasthd = reinterpret_cast<GL::RenderTargetData*>(RenderTargetStack.Back().pRenderTarget->GetRenderTargetData());
        if (lasthd)
            glBindFramebuffer(GL_FRAMEBUFFER, lasthd->FBOID);
    }
    return prt;
}

RenderTarget* HAL::CreateTempRenderTarget(const ImageSize& size, bool needsStencil)
{
    RenderTarget* prt = pRenderBufferManager->CreateTempRenderTarget(size);
    if ( !prt )
        return 0;
    Texture* pt = (Texture*)prt->GetTexture();
    if ( !pt )
        return 0;

    RenderTargetData* phd = (RenderTargetData*)prt->GetRenderTargetData();
    if ( phd && (!needsStencil || phd->pDepthStencilBuffer != 0 ))
        return prt;

    // If only a new depth stencil is required.
    Ptr<HALGLFramebuffer> fboID;
    Ptr<HALGLTexture> colorID = pt->pTextures[0].TexId;

    if ( phd )
    {
        fboID = phd->FBOID;
    }
    else
    {
        fboID = *SF_NEW HALGLFramebuffer();
        glGenFramebuffers(1, &fboID.GetRawRef());
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fboID);
    ++AccumulatedStats.RTChanges;

#if defined(SF_RENDER_GLES)
    // If on GLES2, and it has NPOT limitations, then we need to ensure that the texture
    // uses clamping mode without mipmapping, otherwise the glCheckFramebufferStatus will 
    // return that the target is unsupported.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
#endif

    // Bind the color buffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorID, 0);

    // Create (and bind) the depth/stencil buffers if required.
    Ptr<DepthStencilBuffer> pdsb = 0;
    if ( needsStencil )
        pdsb = *createCompatibleDepthStencil(size, true);

    RenderTargetData::UpdateData(prt, this, fboID, pdsb);

    // Set the FBO back to the top level of the RenderTargetStack.
    if (RenderTargetStack.GetSize() > 0 && RenderTargetStack.Back().pRenderTarget)
    {
        GL::RenderTargetData* lasthd = reinterpret_cast<GL::RenderTargetData*>(RenderTargetStack.Back().pRenderTarget->GetRenderTargetData());
        if (lasthd)
            glBindFramebuffer(GL_FRAMEBUFFER, lasthd->FBOID);
    }

    return prt;
}

// *** RenderHAL_GL Implementation

bool HAL::InitHAL(const Render::HALInitParams& paramsIn)
{
    const GL::HALInitParams& params = reinterpret_cast<const GL::HALInitParams&>(paramsIn);

    // Initialize the device, and set the 'current' device to the immediate, for initialization.
    ImmediateDevice.Initialize(params.ConfigFlags);
    pDevice = &ImmediateDevice;

    // Disable the usage of texture density profile mode, if derivatives are not available.
    if (pDevice->GetCaps() & Cap_NoDerivatives)
        GetProfiler().SetModeAvailability((unsigned)(Profile_All & ~Profile_TextureDensity));

    SManager.SetBinaryShaderPath(params.BinaryShaderPath);

    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &MaxVertexAttributes);

    SF_DEBUG_MESSAGE1(1, "GL_VENDOR                   = %s\n", (const char*)glGetString(GL_VENDOR));
    SF_DEBUG_MESSAGE1(1, "GL_VERSION                  = %s\n", (const char*)glGetString(GL_VERSION));
    SF_DEBUG_MESSAGE1(1, "GL_RENDERER                 = %s\n", (const char*)glGetString(GL_RENDERER));
    SF_DEBUG_MESSAGE1(1, "GL_SHADING_LANGUAGE_VERSION = %s\n", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
    SF_DEBUG_MESSAGE1(1, "GL_MAX_VERTEX_ATTRIBS       = %d\n", MaxVertexAttributes);

    GLint maxTextureSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    SF_DEBUG_MESSAGE1(1, "GL_MAX_TEXTURE_SIZE         = %d\n", maxTextureSize);

    if (CheckGLVersion(3,0))
    {
        String extensions;
        GLint extCount;
        glGetIntegerv(GL_NUM_EXTENSIONS, &extCount);
        for (unsigned extIndex = 0; extIndex < (unsigned)extCount; ++extIndex)
        {
            extensions += (const char*)glGetStringi(GL_EXTENSIONS, extIndex);
            extensions += " ";
            if (extensions.GetLength() > 1024)
            {
                SF_DEBUG_MESSAGE1(1, "GL_EXTENSIONS               = %s\n", extensions.ToCStr());
                extensions = "";
            }
        }
        SF_DEBUG_MESSAGE1(1, "GL_EXTENSIONS               = %s\n", extensions.ToCStr()); 
    }
    else
    {
        SF_DEBUG_MESSAGE1(1, "GL_EXTENSIONS               = %s\n", (const char*)glGetString(GL_EXTENSIONS));
    }

    SF_DEBUG_MESSAGE1(1, "GL_CAPS                     = 0x%x\n", pDevice->GetCaps());

#if defined(SF_RENDER_GLES)
    GLint rgbaBits[4], stencilBits, depthBits;
    glGetIntegerv(GL_RED_BITS, &rgbaBits[0]);
    glGetIntegerv(GL_GREEN_BITS, &rgbaBits[1]);
    glGetIntegerv(GL_BLUE_BITS, &rgbaBits[2]);
    glGetIntegerv(GL_ALPHA_BITS, &rgbaBits[3]);
    glGetIntegerv(GL_STENCIL_BITS, &stencilBits);
    glGetIntegerv(GL_DEPTH_BITS, &depthBits);
    SF_DEBUG_MESSAGE6(1, "GL_x_BITS                   = R%dG%dB%dA%d, D%dS%d\n", rgbaBits[0], rgbaBits[1], rgbaBits[2], rgbaBits[3], depthBits, stencilBits);
#endif

    RSync.SetContext(this);

    pTextureManager = params.GetTextureManager();
    if (!pTextureManager)
    {
        Ptr<TextureCacheGeneric> textureCache = 0;

        // On GLES, create a texture cache, with the default size. Otherwise, do not use texture caching.
#if defined(SF_RENDER_GLES)
        textureCache = *SF_NEW TextureCacheGeneric();
#endif
        pTextureManager = 
            *SF_HEAP_AUTO_NEW(this) TextureManager(params.RenderThreadId, pRTCommandQueue, textureCache);
    }
    pTextureManager->Initialize(this);

    pRenderBufferManager = params.pRenderBufferManager;
    if (!pRenderBufferManager)
    {
        pRenderBufferManager = *SF_HEAP_AUTO_NEW(this) RenderBufferManagerGeneric(RBGenericImpl::DSSM_None);
        if ( !pRenderBufferManager || !pRenderBufferManager->Initialize(pTextureManager))
        {
            ShutdownHAL();
            return false;
        }
    }

    if (!SManager.Initialize(this, params.ConfigFlags) ||
        !Cache.Initialize(this))
        return false;

    // Create a framebuffer binding for the current FBO.
    GLint currentFBOName;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBOName);
    Ptr<HALGLFramebuffer> currentFBO = 0;
    if (currentFBOName != 0)
    {
        currentFBO = *SF_NEW HALGLFramebuffer(currentFBOName, GL_FRAMEBUFFER, ImmediateDevice);
        glBindFramebuffer(GL_FRAMEBUFFER, currentFBO);
    }

    // Create a temporary render target while the immediate device is in use, to determine
    // a compatible depth stencil format. Because this requires querying GL state, this is difficult to do in a deferred state.
    if (params.ConfigFlags & HALConfig_SoftwareDeferredContext)
    {
        Ptr<RenderTarget> tempRenderTarget = *CreateTempRenderTarget(ImageSize(64,64), true);
        tempRenderTarget.Clear();
    }

    // Create GL specific MatrixStateFactory.
    pMatrixFactory = SF_HEAP_NEW(pHeap) GL::MatrixStateFactory(this, pHeap);

    // Call the base-initialization.
    if ( !BaseHAL::InitHAL(params))
        return false;

    // Now, setup deferred context, if requested.
    if (params.ConfigFlags & HALConfig_SoftwareDeferredContext)
    {
        pRecordingDevice = *SF_NEW GL::GraphicsDeviceRecorder(ImmediateDevice);
        pDevice = pRecordingDevice;
    }
    else
    {
        pDevice = &ImmediateDevice;
    }

    return true;
}

// Handles switching from the GL::GraphicsDeviceRecorder to the GL::GraphicsDeviceImmediate, and back. 
// This is used during certain HAL events which must be executed on the render thread, such as ShutdownHAL,
// PrepareForReset and RestoreAfterReset.
class ScopedImmediateDeviceUsage
{
public:
    ScopedImmediateDeviceUsage(GL::HAL* phal, bool flush = true) : pHAL(phal), pPreviousGraphicsDevice(pHAL->pDevice)
    {
        // Ensure that this code is executing on the immediate render thread.
        SF_DEBUG_ASSERT(pHAL, "Unexpected NULL HAL.");
        SF_DEBUG_ASSERT2(GetCurrentThreadId() == pHAL->RenderThreadID, 
            "Immediate device can only be called from the render thread (RenderThreadID=" THREAD_FORMAT ", CurrentThreadID=" THREAD_FORMAT ")", 
            pHAL->RenderThreadID, GetCurrentThreadId());

        // Flush any recorded commands.
        if (pHAL->pDevice == pHAL->pRecordingDevice && flush)
        {
            pHAL->pRecordingDevice->End();
            pHAL->pRecordingDevice->ExecuteRecording(pHAL->ImmediateDevice);
        }
        pHAL->pDevice->clearCachedBindings();
        pHAL->pDevice = &pHAL->ImmediateDevice;       
    }
    ~ScopedImmediateDeviceUsage()
    {
        pHAL->pDevice->clearCachedBindings();
        pHAL->pDevice = pPreviousGraphicsDevice;
    }
private:
    GL::HAL* pHAL;
    GL::GraphicsDevice* pPreviousGraphicsDevice;
};

// Returns back to original mode (cleanup)
bool HAL::ShutdownHAL()
{
    if (!(HALState & HS_Initialized))
        return true;

    // Switch to using the immediate device, and ensure that this is called from the thread that owns the GL context.
    ScopedImmediateDeviceUsage scope(this);

    if (!BaseHAL::ShutdownHAL())
        return false;

    destroyDefaultRenderBuffer();
    pRenderBufferManager.Clear();
    pTextureManager->Reset();
    pTextureManager.Clear();
    Cache.Reset();
    SManager.Reset();

    return true;
}

bool HAL::PrepareForReset()
{
    // Switch to using the immediate device, and ensure that this is called from the thread that owns the GL context.
    ScopedImmediateDeviceUsage scope(this);

    // NOTE: The RenderSync must be cleared before other systems, as they may depend on fences that are no longer valid objects.
    // This includes notifyHandlers, because in the general case, it notifies the GlyphCache, which will destroy text meshes,
    // which may reference fence objects.
    RSync.SetContext(0);

    if (!BaseHAL::PrepareForReset())
        return false;

    pTextureManager->NotifyLostContext();
    if (pRenderBufferManager)
        pRenderBufferManager->Reset();
    Cache.Reset(true);
    SManager.Reset(true);
    ShaderData.ResetContext();
    return true;
}

bool HAL::RestoreAfterReset()
{
    // Switch to using the immediate device, and ensure that this is called from the thread that owns the GL context.
    ScopedImmediateDeviceUsage scope(this);

    // Must initialize RenderSync before anything else. Other systems may try to InsertFence.
    RSync.SetContext(this);

    if (!BaseHAL::RestoreAfterReset())
        return false;

    pTextureManager->Initialize(this);
    if (!SManager.Initialize(this, ConfigFlags))
        return false;
    if (!Cache.Initialize(this))
        return false;

    return true;
}

// Set states not changed in our rendering, or that are reset after changes
bool HAL::BeginScene()
{
    if ( !BaseHAL::BeginScene())
        return false;

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    glStencilMask(0xffffffff);
    if (CheckExtension(SF_GL_EXT_stencil_two_side))
        glDisable(GL_STENCIL_TEST_TWO_SIDE);

#if defined(SF_RENDER_OPENGL)
    if (!CheckGLVersion(3,0))
        glDisable(GL_ALPHA_TEST);
#endif

    if (!ShouldUseVAOs())
    {
        // Reset vertex array usage (in case it changed between frames).
        EnabledVertexArrays = -1;
        for (int i = 0; i < MaxVertexAttributes; i++)
            glDisableVertexAttribArray(i);
    }
    return true;
}

bool HAL::EndScene()
{
    if ( !BaseHAL::EndScene())
        return false;

    // Unbind the current VAO, so it doesn't get modified if this is an index buffer.
    if (ShouldUseVAOs())
    {
        glBindVertexArray(0);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glUseProgram(0);

    // End the recording (if applicable). If in the render thread, also submit the buffer immediately.
    if (GetGraphicsDeviceBase())
        GetGraphicsDeviceBase()->End();
    if ((GetConfigFlags() & HALConfig_DisableImplicitSubmit) == 0 && GetCurrentThreadId() == RenderThreadID)
        Submit();

    return true;
}

bool HAL::Submit()
{
    if (!BaseHAL::Submit())
        return false;

    // If the recording device is NULL, assume that execution is happening in immediate mode.
    if (!pRecordingDevice)
    {
        SF_DEBUG_ASSERT((GetConfigFlags() & HALConfig_SoftwareDeferredContext) == 0,  
            "HALConfig_SoftwareDeferredContext was set, but recording device is NULL.");

        // NOTE: Must perform shader validation, otherwise binary shader will not be saved.
        SManager.PerformShaderValidation();

        return (GetConfigFlags() & HALConfig_SoftwareDeferredContext) == 0;
    }

    // If using a deferred context, reset cached values within the graphics device immediately.
    ImmediateDevice.Begin();
    pRecordingDevice->ExecuteRecording(ImmediateDevice);


    // Switch to the immediate device, to validate shaders. Note: do not flush commands, as,
    // that would execute the next scene's recording.
    {
        ScopedImmediateDeviceUsage scope(this, false);
        SManager.PerformShaderValidation();
    }    
    return true;
}

//--------------------------------------------------------------------
// Background clear helper, expects viewport coordinates.
void HAL::ClearSolidRectangle(const Rect<int>& r, Color color, bool blend)
{
    if ((!blend || color.GetAlpha() == 0xFF) && !(VP.Flags & Viewport::View_Stereo_AnySplit))
    {
        ScopedRenderEvent GPUEvent(GetEvents(), Event_Clear, "HAL::clearSolidRectangle"); // NOTE: inside scope, base impl has its own profile.

        glEnable(GL_SCISSOR_TEST);

        PointF tl((float)(VP.Left + r.x1), (float)(VP.Top + r.y1));
        PointF br((float)(VP.Left + r.x2), (float)(VP.Top + r.y2));
        tl = Matrices->Orient2D * tl;
        br = Matrices->Orient2D * br;
        Rect<int> scissor((int)Alg::Min(tl.x, br.x), (int)Alg::Min(tl.y,br.y), (int)Alg::Max(tl.x,br.x), (int)Alg::Max(tl.y,br.y));
        glScissor(scissor.x1, scissor.y1, scissor.Width(), scissor.Height());
        glClearColor(color.GetRed() * 1.f/255.f, color.GetGreen() * 1.f/255.f, color.GetBlue() * 1.f/255.f, color.GetAlpha() * 1.f/255.f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (VP.Flags & Viewport::View_UseScissorRect)
        {
            glEnable(GL_SCISSOR_TEST);
            glScissor(VP.ScissorLeft, VP.BufferHeight-VP.ScissorTop-VP.ScissorHeight, VP.ScissorWidth, VP.ScissorHeight);
        }
        else
        {
            glDisable(GL_SCISSOR_TEST);
        }
    }
    else
    {
        BaseHAL::ClearSolidRectangle(r, color, blend);
    }
}

bool HAL::IsRasterModeSupported(RasterModeType mode) const
{
    SF_UNUSED(mode);
#if defined(SF_RENDER_OPENGL)
    // OpenGL supports all.
    return true;
#else
    // GLES supports none
    return false;
#endif
}

Render::RenderSync* HAL::GetRenderSync()
{
    // Only actually return the render sync object, if the MeshCache is using unsynchronized buffer updates. 
    // Fencing is not useful otherwise, but it might have performance implications if used regardless. Returning 
    // NULL from this function will act as though fencing is not supported.
    if (Cache.GetBufferUpdateType() == MeshCache::BufferUpdate_MapBufferUnsynchronized)
        return (Render::RenderSync*)(&RSync);
    else
        return 0;
}

void   HAL::MapVertexFormat(PrimitiveFillType fill, const VertexFormat* sourceFormat,
                            const VertexFormat** single,
                            const VertexFormat** batch, const VertexFormat** instanced, unsigned)
{
    unsigned caps = pDevice->GetCaps();
    unsigned instancingFlag = (caps&Cap_Instancing ? MVF_HasInstancing : 0);
    SManager.MapVertexFormat(fill, sourceFormat, single, batch, instanced, (caps&MVF_Align)| instancingFlag);
    if (caps & Cap_NoBatching)
        *batch = 0;
}

ShaderObject* HAL::GetStaticShader( ShaderDesc::ShaderType shaderType )
{
    unsigned comboIndex = FragShaderDesc::GetShaderComboIndex(shaderType, SManager.GLSLVersion);
    SF_DEBUG_ASSERT(VertexShaderDesc::GetShaderComboIndex(shaderType, SManager.GLSLVersion) == comboIndex,
        "Expected ComboIndex for both vertex and fragment shaders to be equivalent.");
    if ( comboIndex >= UniqueShaderCombinations )
        return 0;

    ShaderObject* shader = &SManager.StaticShaders[comboIndex];

    // Initialize the shader if it hasn't already been initialized.
    if ( (ConfigFlags & HALConfig_DynamicShaderCompile) && !shader->IsInitialized() )
    {
        if ( !shader->Init(this, SManager.GLSLVersion, comboIndex, 
            SManager.UsingSeparateShaderObject(), SManager.CompiledShaderHash ))
        {
            return 0;
        }       
        SManager.addShaderToValidationQueue(comboIndex);
    }
    return shader;
}

bool HAL::ShouldUseVAOs()
{
    // If we are not using VBOs, then we cannot use VAOs.
    if ((GetMeshCache().GetBufferUpdateType() == MeshCache::BufferUpdate_ClientBuffers) )
        return false;

    // If VAOs are specifically disabled, then don't use them.
    if (pDevice->GetCaps() & Cap_NoVAO)
        return false;
    
    // OpenGL/ES 3.0+ should use it, or if the GLES extension exists.
    return CheckGLVersion(3,0) || CheckExtension(SF_GL_OES_vertex_array_object) ||
        CheckExtension(SF_GL_ARB_vertex_array_object);
}

bool HAL::CheckGLVersion(unsigned reqMajor, unsigned reqMinor)
{
    if (!pDevice)
    {
        SF_DEBUG_WARNONCE(1, "Call HAL::InitHAL before querying GL version.");
        return false;
    }
    return pDevice->CheckGLVersion(reqMajor, reqMinor);
}

bool HAL::CheckExtension(GLExtensionType type)
{
    if (!pDevice)
    {
        SF_DEBUG_WARNONCE(1, "Call HAL::InitHAL before querying GL extensions.");
        return false;
    }
    return pDevice->CheckExtension(type);
}

unsigned HAL::GetCaps() const
{
    if (!pDevice)
    {
        SF_DEBUG_WARNONCE(1, "Call HAL::InitHAL before querying Caps.");
        return 0;
    }
    return pDevice->GetCaps();
}

void HAL::beginDisplay(BeginDisplayData* data)
{
    glDisable(GL_STENCIL_TEST);

    BaseHAL::beginDisplay(data);
}

// Updates HW Viewport and ViewportMatrix based on provided viewport
// and view rectangle.
void HAL::updateViewport()
{
    Viewport vp;

    if (HALState & HS_ViewValid)
    {
        int dx = ViewRect.x1 - VP.Left,
            dy = ViewRect.y1 - VP.Top;

        // Modify HW matrix and viewport to clip.
        calcHWViewMatrix(VP.Flags, &Matrices->View2D, ViewRect, dx, dy);
        Matrices->SetUserMatrix(Matrices->User);
        Matrices->ViewRect    = ViewRect;
        Matrices->UVPOChanged = 1;

        if ( HALState & HS_InRenderTarget )
        {
            glViewport(VP.Left, VP.Top, VP.Width, VP.Height);
            glDisable(GL_SCISSOR_TEST);
        }
        else
        {
            vp = VP;
            vp.Left     = ViewRect.x1;
            vp.Top      = ViewRect.y1;
            vp.Width    = ViewRect.Width();
            vp.Height   = ViewRect.Height();
            vp.SetStereoViewport(Matrices->S3DDisplay);
            glViewport(vp.Left, VP.BufferHeight-vp.Top-vp.Height, vp.Width, vp.Height);
            if (VP.Flags & Viewport::View_UseScissorRect)
            {
                glEnable(GL_SCISSOR_TEST);
                glScissor(VP.ScissorLeft, VP.BufferHeight-VP.ScissorTop-VP.ScissorHeight, VP.ScissorWidth, VP.ScissorHeight);
            }
            else
            {
                glDisable(GL_SCISSOR_TEST);
            }
        }
    }
    else
    {
        glViewport(0,0,0,0);
    }

    // Workaround: it appears that when changing FBOs, the Tegra 3 will lose the current shader program
    // binding, and crash when rendering the next primitive. updateViewport is always called when GFx changes
    // FBOs, so, clear the cached shader program, so that next time it is requested, it will actually be set.
    // This should only result in a minimal amount of redundant state-sets.
    ShaderData.BeginScene();
}

bool HAL::createDefaultRenderBuffer()
{
    ImageSize rtSize;

    RenderTargetEntry entry;

    HALGLFramebuffer* currentFBO = 0;
    rtSize = getFboInfo(0, currentFBO, true);

    Ptr<RenderTarget> ptarget = *SF_HEAP_AUTO_NEW(this) RenderTarget(0, RBuffer_Default, rtSize );
    Ptr<DepthStencilBuffer> pdsb = *SF_HEAP_AUTO_NEW(this) DepthStencilBuffer(0, rtSize);
    RenderTargetData::UpdateData(ptarget, this, currentFBO, pdsb);

    SetRenderTarget(ptarget);
    return true;
}

void HAL::setRenderTargetImpl(Render::RenderTargetData* phdinput, unsigned flags, const Color &clearColor)
{
    SF_DEBUG_ASSERT(phdinput != 0, "Unexpected NULL RenderTargetData.");
    RenderTargetData* phd = reinterpret_cast<RenderTargetData*>(phdinput);

    glBindFramebuffer(GL_FRAMEBUFFER, phd->FBOID);
    glDisable(GL_SCISSOR_TEST);

    // Clear, if not specifically excluded
    if (flags & PRT_NoClear)
        return;

    float clear[4];
    clearColor.GetRGBAFloat(clear);
    glClearColor(clear[0], clear[1], clear[2], clear[3]);
    glClear(GL_COLOR_BUFFER_BIT);
}

bool HAL::checkDepthStencilBufferCaps()
{
    RenderTargetEntry& rte = RenderTargetStack.Back();
    if (!rte.StencilChecked)
    {
        Ptr<HALGLFramebuffer> currentFBO = GetGraphicsDevice()->GetBoundFramebuffer(GL_FRAMEBUFFER);
        if (currentFBO != 0)
        {
            // Check for stencil buffer bits.
            GLint stencilType, stencilBits = 0;
            glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &stencilType);
            switch(stencilType)
            {
            case GL_NONE:
                stencilBits = 0;
                break;
            default:
                const HALGLFramebuffer::FramebufferAttachment* stencil = currentFBO->GetAttachment(GL_STENCIL_ATTACHMENT);
                if (stencil)
                {
                    if (stencil->RenderBuffer)
                    {
                        glBindRenderbuffer(GL_RENDERBUFFER, stencil->RenderBuffer);
                        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_STENCIL_SIZE, &stencilBits);
                    }
                    else
                    {
                        // Texture attachment. Assume there are at least 8 bits in the attached texture.
                        stencilBits = 8;
                    }
                }
            }

            if (stencilBits > 0)
            {
                rte.StencilAvailable = true;
                rte.MultiBitStencil = (stencilBits > 1);
            }

            // Check for depth buffer.
            GLint depthType, depthBits = 0;
            glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &depthType);
            switch(depthType)
            {
            case GL_NONE:
                depthBits = 0;
                break;
            default:
                const HALGLFramebuffer::FramebufferAttachment* depth = currentFBO->GetAttachment(GL_DEPTH_ATTACHMENT);
                if (depth)
                {
                    if (depth->RenderBuffer)
                    {
                        glBindRenderbuffer(GL_RENDERBUFFER, depth->RenderBuffer);
                        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_DEPTH_SIZE, &depthBits);
                    }
                    else
                    {
                        // Texture attachment. Assume there are at least 8 bits in the attached texture.
                        depthBits = 8;
                    }
                }
            }
            rte.DepthBufferAvailable = (depthBits >= 1);
        }
        else
        {
            // In GLES, the default framebuffer can be checked for depth and stencil bits.
#if defined(SF_RENDER_GLES)
            GLint stencilBits, depthBits;
            glGetIntegerv(GL_STENCIL_BITS, &stencilBits);
            glGetIntegerv(GL_DEPTH_BITS, &depthBits);
            rte.StencilAvailable = stencilBits != 0;
            rte.MultiBitStencil = stencilBits > 1;
            rte.DepthBufferAvailable = false;
            SF_DEBUG_WARNONCE(!rte.StencilAvailable && depthBits > 0, "Only depth buffer provided, but only stencil masking is available on this platform. Please provide a stencil buffer.");
#else
            // If we are using the default FBO, assume we have everything. TBD: this should be overridable in HALInitParams
            rte.StencilAvailable = true;
            rte.DepthBufferAvailable = true;
            rte.MultiBitStencil = true;
#endif
        }
        rte.StencilChecked = 1;
    }   

    SF_DEBUG_WARNONCE(!rte.StencilAvailable && !rte.DepthBufferAvailable, 
        "RendererHAL::PushMask_BeginSubmit used, but neither stencil or depth buffer is available");
    return (rte.StencilAvailable || rte.DepthBufferAvailable);
}

void HAL::applyDepthStencilMode(DepthStencilMode mode, unsigned stencilRef)
{
    ScopedRenderEvent GPUEvent(GetEvents(), Event_ApplyDepthStencil, "HAL::applyDepthStencilMode");
    static GLenum DepthStencilCompareFunctions[DepthStencilFunction_Count] =
    {
        GL_NEVER,           // Ignore
        GL_NEVER,           // Never
        GL_LESS,            // Less
        GL_EQUAL,           // Equal
        GL_LEQUAL,          // LessEqual
        GL_GREATER,         // Greater
        GL_NOTEQUAL,        // NotEqual
        GL_GEQUAL,          // GreaterEqual
        GL_ALWAYS,          // Always
    };
    static GLenum StencilOps[StencilOp_Count] =
    {
        GL_KEEP,      // Ignore
        GL_KEEP,      // Keep
        GL_REPLACE,   // Replace
        GL_INCR,      // Increment        
    };

    const HALDepthStencilDescriptor& oldState = DepthStencilModeTable[CurrentDepthStencilState];
    const HALDepthStencilDescriptor& newState = DepthStencilModeTable[mode];

    // Apply the modes now.
    if (oldState.ColorWriteEnable != newState.ColorWriteEnable)
    {
        if (newState.ColorWriteEnable)
            glColorMask(1,1,1,1);
        else
            glColorMask(0,0,0,0);
    }

    if (oldState.StencilEnable != newState.StencilEnable)
    {
        if (newState.StencilEnable)
            glEnable(GL_STENCIL_TEST);
        else
            glDisable(GL_STENCIL_TEST);
    }

    // Only need to set stencil pass/fail ops if stenciling is actually enabled.
    if (newState.StencilEnable)
    {
        // No redundancy checking on stencil ref/write mask.
        glStencilFunc(DepthStencilCompareFunctions[newState.StencilFunction], stencilRef, 0XFF);

        if ((oldState.StencilFailOp != newState.StencilFailOp &&
            newState.StencilFailOp != HAL::StencilOp_Ignore) ||
            (oldState.StencilPassOp != newState.StencilPassOp &&
            newState.StencilPassOp!= HAL::StencilOp_Ignore) ||
            (oldState.StencilZFailOp != newState.StencilZFailOp &&
            newState.StencilZFailOp != HAL::StencilOp_Ignore))
        {
            glStencilOp(StencilOps[newState.StencilFailOp], StencilOps[newState.StencilZFailOp], StencilOps[newState.StencilPassOp]);
        }
    }

    // If the value of depth test/write change, we may have to change the value of ZEnable.
    if ((oldState.DepthTestEnable || oldState.DepthWriteEnable) != 
        (newState.DepthTestEnable || newState.DepthWriteEnable))
    {
        if ((newState.DepthTestEnable || newState.DepthWriteEnable))
            glEnable(GL_DEPTH_TEST);
        else
            glDisable(GL_DEPTH_TEST);

        // Only need to set the function, if depth testing is enabled.
        if (newState.DepthTestEnable)
        {
            if (oldState.DepthFunction != newState.DepthFunction &&
                newState.DepthFunction != HAL::DepthStencilFunction_Ignore)
            {
                glDepthFunc(DepthStencilCompareFunctions[newState.DepthFunction]);
            }
        }
    }

    if (oldState.DepthWriteEnable != newState.DepthWriteEnable)
    {
        glDepthMask(newState.DepthWriteEnable ? GL_TRUE : GL_FALSE);
    }

    CurrentDepthStencilState = mode;
}

void HAL::applyRasterModeImpl(RasterModeType mode)
{
#if defined(SF_RENDER_OPENGL)
    GLenum fillMode;
    switch(mode)
    {
        default:
        case RasterMode_Solid:      fillMode = GL_FILL; break;
        case RasterMode_Wireframe:  fillMode = GL_LINE; break;
        case RasterMode_Point:      fillMode = GL_POINT; break;
    }
    glPolygonMode(GL_FRONT_AND_BACK, fillMode);
#else
    SF_UNUSED(mode);
#endif
}



void HAL::applyBlendModeImpl(BlendMode mode, bool sourceAc, bool forceAc)
{    
    static const UInt32 BlendOps[BlendOp_Count] = 
    {
        GL_FUNC_ADD,                // BlendOp_ADD
        GL_MAX,                     // BlendOp_MAX
        GL_MIN,                     // BlendOp_MIN
        GL_FUNC_REVERSE_SUBTRACT,   // BlendOp_REVSUBTRACT
    };

    static const UInt32 BlendFactors[BlendFactor_Count] = 
    {
        GL_ZERO,                // BlendFactor_ZERO
        GL_ONE,                 // BlendFactor_ONE
        GL_SRC_ALPHA,           // BlendFactor_SRCALPHA
        GL_ONE_MINUS_SRC_ALPHA, // BlendFactor_INVSRCALPHA
        GL_DST_COLOR,           // BlendFactor_DESTCOLOR
        GL_ONE_MINUS_DST_COLOR, // BlendFactor_INVDESTCOLOR
    };

    GLenum sourceColor = BlendFactors[BlendModeTable[mode].SourceColor];
    if ( sourceAc && sourceColor == GL_SRC_ALPHA )
        sourceColor = GL_ONE;

    if (VP.Flags & Viewport::View_AlphaComposite || forceAc)
    {
        glBlendFuncSeparate(sourceColor, BlendFactors[BlendModeTable[mode].DestColor], 
            BlendFactors[BlendModeTable[mode].SourceAlpha], BlendFactors[BlendModeTable[mode].DestAlpha]);
        glBlendEquationSeparate(BlendOps[BlendModeTable[mode].Operator], BlendOps[BlendModeTable[mode].AlphaOperator]);
    }
    else
    {
        glBlendFunc(sourceColor, BlendFactors[BlendModeTable[mode].DestColor]);
        glBlendEquation(BlendOps[BlendModeTable[mode].Operator]);
    }
}

void HAL::applyBlendModeEnableImpl(bool enabled)
{
    if (enabled)
        glEnable(GL_BLEND);
    else
        glDisable(GL_BLEND);
}

// Helper function to retrieve the vertex element type (VET) and normalization from a vertex attribute. Returns false if the attribute should be ignored.
bool VertexBuilderVET(unsigned attr, GLenum&vet, bool& norm)
{
    switch (attr & VET_CompType_Mask)
    {
    case VET_U8:  vet = GL_UNSIGNED_BYTE; norm = false; break;
    case VET_U8N: vet = GL_UNSIGNED_BYTE; norm = true; break;
    case VET_U16: vet = GL_UNSIGNED_SHORT; norm = false; break;
    case VET_S16: vet = GL_SHORT; norm = false; break;
    case VET_U32: vet = GL_UNSIGNED_INT; norm = false; break;
    case VET_F32: vet = GL_FLOAT; norm = false;  break;

        // Instance indices are not used in the vertex arrays, so just ignore them.
    case VET_I8:
    case VET_I16:
        return false;

    default: SF_ASSERT(0); vet = GL_FLOAT; norm = false; return false;
    }
    return true;
}

// Uses functions within the GL 2.1- (or GLES 2.0) spec to define vertex attributes. This is not
// compatible with GL 3.0+ (see VertexBuilder_Core30).
class VertexBuilder_Legacy
{
public:
    HAL*            pHal;
    unsigned        Stride;
    UByte*          VertexOffset;

    VertexBuilder_Legacy(HAL* phal, unsigned stride, HALGLBuffer* vbuffer, HALGLBuffer* ibuffer, UByte* vertOffset) : 
        pHal(phal), Stride(stride), VertexOffset(vertOffset)
    {
        // Bind the vertex buffer and the index buffer immediately.
        glBindBuffer(GL_ARRAY_BUFFER, vbuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuffer);
    }

    HAL* GetHAL() { return pHal; }

    void Add(int vi, int attr, int ac, int offset)
    {
        GLenum vet; bool norm;
        if (!VertexBuilderVET(attr, vet, norm))
            return;

        if ( pHal->EnabledVertexArrays < vi )
        {
            glEnableVertexAttribArray(vi);
            pHal->EnabledVertexArrays++;
        }

        // Note: Extend the size of UByte w/1 component to 4 components. On certain drivers, this
        // appears to work incorrectly, but extending it to 4xUByte corrects the issue (even though
        // 3 of the elements are unused).
        if (vet == GL_UNSIGNED_BYTE && ac < 4)
            ac = 4;

        glVertexAttribPointer(vi, ac, vet, norm, Stride, VertexOffset + offset);
    }

    void Finish(int vi)
    {
        int newEnabledCount = vi-1;
        for (int i = vi; i < pHal->EnabledVertexArrays; i++)
        {
            glDisableVertexAttribArray(i);
        }
        pHal->EnabledVertexArrays = newEnabledCount;
    }
};

class VertexBuilder_Core30
{
public:
    HAL*            pHal;
    unsigned        Stride;
    MeshCacheItem*  pMesh;
    bool            NeedsGeneration;    // Set to true if the VAO has not been initialized yet (and should be done by this class).
    UByte*          VertexOffset;

    VertexBuilder_Core30(HAL* phal, const VertexFormat* pformat, MeshCacheItem* pmesh, UPInt vbOffset) : 
        pHal(phal), Stride(pformat->Size), pMesh(pmesh), NeedsGeneration(false), VertexOffset(0)
    {
        // Allocate VAO for this mesh now.
        VertexOffset = pmesh->pVertexBuffer->GetBufferBase() + pmesh->VBAllocOffset + vbOffset;
        if (pMesh->VAOFormat != pformat || pMesh->VAOOffset != VertexOffset || pMesh->VAO == 0)
        {
            if (pMesh->VAO)
                glDeleteVertexArrays(1, &pMesh->VAO.GetRawRef());

            pMesh->VAO = *SF_NEW HALGLVertexArray();
            glGenVertexArrays(1, &pMesh->VAO.GetRawRef());

            // Store the vertex offset, and indicate that we need to generate the contents of the VAO.
            pMesh->VAOOffset = VertexOffset;
            pMesh->VAOFormat = pformat;
            NeedsGeneration = true;
        }

        // Bind the VAO.
        glBindVertexArray(pMesh->VAO);        

        // If need to generate the VAO, bind the VB/IB now
        if (NeedsGeneration)
        {
            Ptr<HALGLBuffer> vbuffer = pmesh->pVertexBuffer->GetBuffer();
            Ptr<HALGLBuffer> ibuffer = pmesh->pIndexBuffer->GetBuffer();
            glBindBuffer(GL_ARRAY_BUFFER, vbuffer);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuffer);
        }
    }

    HAL* GetHAL() { return pHal; }

    void Add(int vi, int attr, int ac, int offset)
    {
        // If we have already generated the VAO, just skip everything.
        if (!NeedsGeneration)
            return;

        GLenum vet; bool norm;
        if (!VertexBuilderVET(attr, vet, norm))
            return;

        // Note: Extend the size of UByte w/1 component to 4 components. On certain drivers, this
        // appears to work incorrectly, but extending it to 4xUByte corrects the issue (even though
        // 3 of the elements are unused).
        if (vet == GL_UNSIGNED_BYTE && ac < 4)
            ac = 4;

        glEnableVertexAttribArray(vi);
        glVertexAttribPointer(vi, ac, vet, norm, Stride, VertexOffset + offset);
    }

    void Finish(int)
    {
    }
};

UPInt HAL::setVertexArray(PrimitiveBatch* pbatch, Render::MeshCacheItem* pmesh)
{
    BaseHAL::setVertexArray(pbatch, pmesh);
    return setVertexArray(pbatch->pFormat, pmesh, 0);
}

UPInt HAL::setVertexArray(const ComplexMesh::FillRecord& fr, unsigned formatIndex, Render::MeshCacheItem* pmesh)
{
    BaseHAL::setVertexArray(fr, formatIndex, pmesh);
    return setVertexArray(fr.pFormats[formatIndex], pmesh, fr.VertexByteOffset);
}

UPInt HAL::setVertexArray(const VertexFormat* pformat, Render::MeshCacheItem* pmeshBase, UPInt vboffset)
{
    GL::MeshCacheItem* pmesh = reinterpret_cast<GL::MeshCacheItem*>(pmeshBase);
    if (ShouldUseVAOs())
    {
        VertexBuilder_Core30 vb (this, pformat, pmesh, vboffset);
        BuildVertexArray(pformat, vb);
    }
    else
    {
        // Legacy and/or GLES path.
        VertexBuilder_Legacy vb (this, pformat->Size, pmesh->pVertexBuffer->GetBuffer(),
            pmesh->pIndexBuffer->GetBuffer(), pmesh->pVertexBuffer->GetBufferBase() + pmesh->VBAllocOffset + vboffset);
        BuildVertexArray(pformat, vb);
    }
    return reinterpret_cast<UPInt>((pmesh->pIndexBuffer->GetBufferBase() + pmesh->IBAllocOffset)) / sizeof(IndexType); 
}

void HAL::setVertexArray(const VertexFormat* pFormat, HALGLBuffer* buffer, HALGLVertexArray* vao)
{
	SF_UNUSED(pFormat);
    SF_UNUSED(buffer);
    if (ShouldUseVAOs())
    {
        // Immediately bind the VAO, it must be constructed already.
        glBindVertexArray(vao);
        return;
    }

    // Legacy and/or GLES path. Assume no buffer offsets.
    VertexBuilder_Legacy vb (this, pFormat->Size, buffer, 0, 0);
    BuildVertexArray(pFormat, vb);
}

void HAL::setBatchUnitSquareVertexStream()
{
    setVertexArray(&VertexXY16iInstance::Format, &Cache.MaskEraseBatchVertexBuffer, &Cache.MaskEraseBatchVAO);
}

void HAL::drawPrimitive(unsigned indexCount, unsigned meshCount)
{
    glDrawArrays(GL_TRIANGLES, 0, indexCount);

    SF_UNUSED(meshCount);
#if !defined(SF_BUILD_SHIPPING)
    AccumulatedStats.Meshes += meshCount;
    AccumulatedStats.Triangles += indexCount / 3;
    AccumulatedStats.Primitives++;
#endif
}

void HAL::drawIndexedPrimitive(unsigned indexCount, unsigned vertexCount, unsigned meshCount, UPInt indexPtr, UPInt vertexOffset )
{
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, reinterpret_cast<const GLvoid*>(indexPtr*sizeof(IndexType)));

    SF_UNUSED3(meshCount, vertexCount, vertexOffset);
#if !defined(SF_BUILD_SHIPPING)
    AccumulatedStats.Meshes += meshCount;
    AccumulatedStats.Triangles += indexCount / 3;
    AccumulatedStats.Primitives++;
#endif
}

void HAL::drawIndexedInstanced(unsigned indexCount, unsigned vertexCount, unsigned meshCount, UPInt indexPtr, UPInt vertexOffset )
{
    SF_UNUSED2(vertexCount, vertexOffset);
    glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, reinterpret_cast<const GLvoid*>(indexPtr*sizeof(IndexType)), meshCount);

#if !defined(SF_BUILD_SHIPPING)
    AccumulatedStats.Meshes += meshCount;
    AccumulatedStats.Triangles += (indexCount / 3) * meshCount;
    AccumulatedStats.Primitives++;
#endif
}

ImageSize HAL::getFboInfo(HALGLFramebuffer* fbo, HALGLFramebuffer*& currentFBO, bool useCurrent)
{
    currentFBO = GetGraphicsDevice()->GetBoundFramebuffer(GL_FRAMEBUFFER);
    if (!useCurrent)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        ++AccumulatedStats.RTChanges;
    }

    bool validFBO = glIsFramebuffer( fbo ) ? true : false;
    GLint width = 0, height = 0;
    GLint type, id;

    if ( validFBO )
    {
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &type );
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &id );
        switch(type)
        {
        case GL_TEXTURE:
            {
#if defined(SF_RENDER_OPENGL)
                // It is possible that the user has attached a slice of a 3D texture to the framebuffer.
                static const GLenum targetTypes[] = { GL_TEXTURE_2D, GL_TEXTURE_3D };
                for (unsigned int i = 0; i < sizeof(targetTypes)/sizeof(GLenum); ++i)
                {
                    GLenum target = targetTypes[i];
                    Ptr<HALGLTexture> tex = *SF_NEW HALGLTexture(id);
                    glBindTexture(target, tex );
                    GLenum err = glGetError();
                    if (err != 0)
                        continue;
                    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_WIDTH, &width );
                    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_HEIGHT, &height );
                    break;
                }
#endif
                break;
            }
        case GL_RENDERBUFFER:
            Ptr<HALGLRenderbuffer> rb = *SF_NEW HALGLRenderbuffer(id, GL_RENDERBUFFER, ImmediateDevice);
            if ( !glIsRenderbuffer( rb ) )
                break;
            glBindRenderbuffer(GL_RENDERBUFFER, rb);
            glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width );
            glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height );
            break;
        }
    }

    if (width == 0 || height == 0)
    {
        // Get the dimensions of the framerect from glViewport.
        GLfloat viewport[4];
        glGetFloatv(GL_VIEWPORT, viewport);
        width = (GLint)viewport[2];
        height = (GLint)viewport[3];
    }

    if (!useCurrent)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, currentFBO);
        ++AccumulatedStats.RTChanges;
    }

    return ImageSize(width, height);
}

DepthStencilBuffer* HAL::createCompatibleDepthStencil(const ImageSize& size, bool temporary)
{
    HALGLRenderbuffer* dsbID = 0;

    // NOTE: until the HAL has successfully created compatible depth stencil buffer, it creates
    // 'user' depth stencil buffers, as these will not be reused. If we happen to create incompatible
    // ones, when trying to locate a compatible format, we don't want them to be destroyed.
    DepthStencilBuffer* pdsb = pRenderBufferManager->CreateDepthStencilBuffer(size, temporary && DeterminedDepthStencilFormat);
    DepthStencilSurface* pdss = (DepthStencilSurface*)pdsb->GetSurface();
    dsbID = pdss->RenderBufferID;

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, dsbID);

    // Some devices require that the depth buffer be attached, even if we don't use it.
    if (GL::DepthStencilSurface::CurrentFormatHasDepth())
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, dsbID);

    // If this check fails, it means that the stencil format and color format are incompatible.
    // In this case, we will need to try another depth stencil format combination.
    GLenum framebufferStatusError;
    while ((framebufferStatusError = glCheckFramebufferStatus(GL_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE)
    {
        // If the format has been previously determined, but the framebuffer couldn't be created, don't try other formats.
        // It may be that the dimensions are invalid, and no formats would satisfy the framebuffer.
        if (DeterminedDepthStencilFormat || !GL::DepthStencilSurface::SetNextGLFormatIndex())
        {
            SF_DEBUG_WARNING2(DeterminedDepthStencilFormat, "Determined depth stencil format could not create a compatible depth stencil buffer (size=%d x %d)\n", size.Width, size.Height);
            SF_DEBUG_WARNING2(!DeterminedDepthStencilFormat, "No compatible depth stencil formats available. Masking in filter will be disabled (size=%d x %d)\n", size.Width, size.Height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
            pdsb = 0;
            break;
        }

        pdsb = pRenderBufferManager->CreateDepthStencilBuffer(size, temporary && DeterminedDepthStencilFormat);
        DepthStencilSurface* pdss = (DepthStencilSurface*)pdsb->GetSurface();
        dsbID = pdss->RenderBufferID;        
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, dsbID);

        // Some devices require that the depth buffer be attached, even if we don't use it. If it was previously attached,
        // and now our format does not have depth, we must remove it.
        if (GL::DepthStencilSurface::CurrentFormatHasDepth())
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, dsbID);
        else
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
    }

    // If a complete framebuffer was found, then indicate that the depth/stencil format has been determined.
    if (framebufferStatusError == GL_FRAMEBUFFER_COMPLETE)
        DeterminedDepthStencilFormat = true;

    // If this was the first DS surface allocated, and it failed, reset the format index, so perhaps next time it will find a correct one.
    if (!DeterminedDepthStencilFormat)
        GL::DepthStencilSurface::ResetGLFormatIndex();

    return pdsb;
}

void GL::RenderEvents::beginImpl( const char* eventName )
{
    if (GetHAL()->CheckExtension(SF_GL_EXT_debug_marker))
        glPushGroupMarker(0, eventName);
    if (GetHAL()->CheckExtension(SF_GL_GREMEDY_string_marker))
        glStringMarker(0, eventName);
}
void GL::RenderEvents::endImpl()
{
    if (GetHAL()->CheckExtension(SF_GL_EXT_debug_marker))
        glPopGroupMarker();
    if (GetHAL()->CheckExtension(SF_GL_GREMEDY_string_marker))
        glStringMarker(0, "End");
}

//--------------------------------------------------------------------

void RenderTargetData::UpdateData( RenderBuffer* buffer, HAL* phal, GLuint fboID, DepthStencilBuffer* pdsb )
{
    Ptr<HALGLFramebuffer> fbo = *SF_NEW HALGLFramebuffer(fboID, GL_FRAMEBUFFER, *phal->GetGraphicsDevice());
    RenderTargetData::UpdateData(buffer, phal, fbo, pdsb);
}

void RenderTargetData::UpdateData( RenderBuffer* buffer, HAL* phal, HALGLFramebuffer* fboID, DepthStencilBuffer* pdsb )
{
    if ( !buffer )
        return;

    RenderTargetData* poldHD = (GL::RenderTargetData*)buffer->GetRenderTargetData();
    if ( !poldHD )
    {
        poldHD = SF_NEW RenderTargetData(buffer, phal, fboID, pdsb);
        buffer->SetRenderTargetData(poldHD);
    }
    else
    {
        poldHD->FBOID = fboID;
        poldHD->pDepthStencilBuffer = pdsb;
    }
}

RenderTargetData::~RenderTargetData()
{
    if (pBuffer->GetType() != RBuffer_Default && pBuffer->GetType() != RBuffer_User)
    {
        GL::TextureManager* pmgr = (GL::TextureManager*)pHAL->GetTextureManager();

        // If the texture manager isn't present, just try deleting it immediately.
        if (!pmgr)
            glDeleteFramebuffers(1, &FBOID.GetRawRef());
        else
            pmgr->DestroyFBO(FBOID);
    }

}

void RenderTargetData::StripDepthStencilTarget()
{
    if ( pDepthStencilBuffer )
    {
        glBindFramebuffer(GL_FRAMEBUFFER, FBOID);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
    }
    Render::RenderTargetData::StripDepthStencilTarget();
}

Render::RenderEvents& HAL::GetEvents()
{
    return Events;
}

}}} // Scaleform::Render::GL
