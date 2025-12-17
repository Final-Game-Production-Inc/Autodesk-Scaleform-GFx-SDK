/**************************************************************************

Filename    :   GL_HAL.h
Content     :   Renderer HAL Prototype header.
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_GL_HAL_H
#define INC_SF_Render_GL_HAL_H

#include "Render/Render_HAL.h"
#include "Render/GL/GL_MeshCache.h"
#include "Render/GL/GL_Shader.h"
#include "Render/GL/GL_Texture.h"
#include "Render/GL/GL_Sync.h"
#include "Render/Render_ShaderHAL.h"    // Must be included after platform specific shader includes.

namespace Scaleform { namespace Render { namespace GL {

// HALConfigFlags enumeration defines system-specific HAL configuration
// flags passed into InitHAL though HALInitParams.
enum HALConfigFlags
{
    // Only compile shaders when they are actually used. This can reduce startup-times,
    // however, compiling shaders dynamically can cause performance spikes during playback.
    // Note that if binary shaders are available in the OpenGL/ES implementation and not explicitly
    // disabled by using HALConfig_DisableBinaryShaders, this flag is ignored, and all shaders are
    // loaded at startup. They subsequently will be loaded from disk, which is much faster.
    HALConfig_DynamicShaderCompile      = 0x00000001,

    // Disables the use of binary shaders (loading and saving), even if the OpenGL/ES implementation
    // has the required support for it. 
    HALConfig_DisableBinaryShaders      = 0x00000002,

    // Instead of storing all binary shaders in a single file, which is only saved on shutdown, save each shader
    // in an individual file. This enables saving of binary shaders when they are dynamically initialized.
    HALConfig_MultipleShaderCacheFiles  = 0x00000004,

    // Disables the usage of separable shader program pipelines (eg. GL_EXT_separate_shader_objects),
    // even if the OpenGL/ES implementation has the required support for it.
    HALConfig_DisableShaderPipelines    = 0x00000008,

	// Enables debugging output via the glDebugMessageCallback. This can be useful to get driver specific
	// error messages. Note that a GL context with the debug-bit enabled is required for these to output.
	HALConfig_DebugMessages				= 0x00000010,
};

// GL::HALInitParams provides OpenGL-specific rendering initialization
// parameters for HAL::InitHAL.

struct HALInitParams : public Render::HALInitParams
{
    String BinaryShaderPath;
    bool NoVAO;
    bool NoInstancing;

    HALInitParams(UInt32 halConfigFlags = 0,
                  ThreadId renderThreadId = ThreadId(),
                  const String& binaryShaderPath = String(),
                  bool noVao = false,
                  bool noInstancing = false) : 
        Render::HALInitParams(0, halConfigFlags, renderThreadId),
        BinaryShaderPath(binaryShaderPath),
        NoVAO(noVao),
        NoInstancing(noInstancing)
    { }

    // GL::TextureManager accessors for correct type.
    void            SetTextureManager(TextureManager* manager) { pTextureManager = manager; }
    TextureManager* GetTextureManager() const       { return (TextureManager*) pTextureManager.GetPtr(); }
};

enum CapFlags
{
    Cap_Align           = MVF_Align,

    Cap_NoBatching      = 0x0010,   // Not capable of doing batching.

    // Caps for buffers in mesh cache. Client buffers will be used if none of these caps are set (eg. Caps & Cap_UseMeshBuffer == 0).
    Cap_MapBuffer       = 0x0020,    // glMapBuffer is available.
    Cap_MapBufferRange  = 0x0040,    // glMapBufferRange is available.
    Cap_BufferUpdate    = 0x0080,	 // glBufferData/glBufferSubData is available.
    Cap_UseMeshBuffers  = Cap_MapBuffer | Cap_MapBufferRange | Cap_BufferUpdate,

    // Caps for shaders. 
    Cap_NoDynamicLoops  = 0x0100,    // Profile does not support dynamic looping.
    Cap_BinaryShaders   = 0x0200,    // Profile supports loading binary shaders
    Cap_NoDerivatives   = 0x0400,    // Profile does not support use of derivatives in fragment shaders (dFdx/dFdy).

    // Caps for drawing.
    Cap_Instancing      = 0x0800,    // Profile supports instancing (DrawArraysInstanced, and GLSL1.4+).
    
    Cap_NoVAO           = 0x1000,    // Not capable of using VAOs
    
    Cap_Sync            = 0x2000,    // Profile supports fence objects (GL_ARB_sync or GL_APPLE_sync).
    
    // GL_MAX_VERTEX_UNIFORM_VECTORS, or a different value on certain devices
    Cap_MaxUniforms       = 0xffff0000,
    Cap_MaxUniforms_Shift = 16,
};

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
    virtual void        recalculateUVPOC() const;
};

class HAL : public Render::ShaderHAL<ShaderManager, ShaderInterface>
#ifdef SF_GL_RUNTIME_LINK
    , public GL::Extensions
#endif
{
    typedef Render::ShaderHAL<ShaderManager, ShaderInterface> BaseHAL;

public:
    int                  EnabledVertexArrays;
    bool                 FilterVertexBufferSet;
    int                  BlendEnable;

    MeshCache            Cache;
    RenderSync               RSync;

    Ptr<TextureManager>      pTextureManager;
    
    // Previous batching mode
    PrimitiveBatch::BatchType PrevBatchType;

    unsigned&                Caps;

    // Self-accessor used to avoid constructor warning.
    HAL*      GetHAL() { return this; }

public:    
    

    HAL(ThreadCommandQueue* commandQueue);
    virtual ~HAL();   

    // *** HAL Initialization and Shutdown

    // Initializes HAL for rendering.
    virtual bool        InitHAL(const GL::HALInitParams& params);

    // ShutdownHAL shuts down rendering, releasing resources allocated in InitHAL.
    virtual bool        ShutdownHAL();


    // Used when the current gl context is lost (GLES on some platforms)
    // or becomes incompatible with the new video mode
    virtual bool        ResetContext();

    // *** Rendering

    virtual bool        BeginFrame();
    virtual void        FinishFrame();
    virtual bool        BeginScene();
    virtual bool        EndScene();

    // Bracket the displaying of a frame from a movie.
    // Fill the background color, and set up default transforms, etc.
    virtual void        beginDisplay(BeginDisplayData* data);

    // Updates HW Viewport and ViewportMatrix based on the current
    // values of VP, ViewRect and ViewportValid.
    virtual void        updateViewport();


    // *** Mask Support
    virtual void        applyDepthStencilMode(DepthStencilMode mode, unsigned stencilRef);
    virtual bool        checkDepthStencilBufferCaps();

    // *** Rasterization
    virtual bool        IsRasterModeSupported(RasterModeType mode) const;
    virtual void        applyRasterModeImpl(RasterModeType mode);

    virtual void    clearSolidRectangle(const Rect<int>& r, Color color, bool blend);

    virtual UPInt setVertexArray(PrimitiveBatch* pbatch, Render::MeshCacheItem* pmesh);
    virtual UPInt setVertexArray(const ComplexMesh::FillRecord& fr, unsigned formatIndex, Render::MeshCacheItem* pmesh);
    UPInt         setVertexArray(const VertexFormat* pFormat, Render::MeshCacheItem* pmesh, UPInt vboffset);
    void          setVertexArray(const VertexFormat* pFormat, GLuint buffer, GLuint vao);


    // *** BlendMode
    virtual void       applyBlendModeImpl(BlendMode mode, bool sourceAc = false, bool forceAc = false);
    virtual void       applyBlendModeEnableImpl(bool enabled);



    virtual Render::TextureManager* GetTextureManager() const
    {
        return pTextureManager.GetPtr();
    }

    virtual RenderTarget*   CreateRenderTarget(GLuint fbo);
    virtual RenderTarget*   CreateRenderTarget(Render::Texture* texture, bool needsStencil);
    virtual RenderTarget*   CreateTempRenderTarget(const ImageSize& size, bool needsStencil);
    virtual bool            SetRenderTarget(RenderTarget* target, bool setState = 1);
    virtual void            PushRenderTarget(const RectF& frameRect, RenderTarget* prt, unsigned flags=0, Color clearColor=0);
    virtual void            PopRenderTarget(unsigned flags = 0);

    virtual bool            createDefaultRenderBuffer();

    // *** Filters
    virtual void            drawUncachedFilter(const FilterStackEntry& e);
    virtual void            drawCachedFilter(FilterPrimitive* primitive);
    virtual void            drawFilter(const Matrix2F& mvp, const Cxform & cx, const Filter* filter, Ptr<RenderTarget> * targets, 
                                       unsigned* shaders, unsigned pass, unsigned passCount, const VertexFormat* pvf, 
                                       BlurFilterState& leBlur);

    virtual class MeshCache&       GetMeshCache()        { return Cache; }
    virtual Render::RenderSync*    GetRenderSync();

    virtual float         GetViewportScaling() const { return 1.0f; }

    virtual void    MapVertexFormat(PrimitiveFillType fill, const VertexFormat* sourceFormat,
                                    const VertexFormat** single,
                                    const VertexFormat** batch, const VertexFormat** instanced, 
                                    unsigned meshType = MeshCacheItem::Mesh_Regular);

    const ShaderObject* GetStaticShader(ShaderDesc::ShaderType shaderType);

    // Check whether the given extension exists in the current profile.
    bool                CheckExtension(const char *name);

    // Check whether the input major/minor version pair is greater or equal to the current profile version.
    bool                CheckGLVersion(unsigned reqMajor, unsigned reqMinor);

    // Returns whether the HAL should use Vertex Array Objects.
    bool                ShouldUseVAOs();


protected:
    ImageSize           getFboInfo(GLint fbo, GLint& currentFBO, bool useCurrent);
    DepthStencilBuffer* createCompatibleDepthStencil(const ImageSize& size, bool temporary);

    // Returns whether the profile can render any of the filters contained in the FilterPrimitive.
    // If a profile does not support dynamic looping (Cap_NoDynamicLoops), no blur/shadow type filters
    // can be rendered, in which case this may return false, however, ColorMatrix filters can still be rendered.
    bool                shouldRenderFilters(const FilterPrimitive* prim) const;

    virtual void        setBatchUnitSquareVertexStream();
    virtual void        drawPrimitive(unsigned indexCount, unsigned meshCount);
    virtual void        drawIndexedPrimitive(unsigned indexCount, unsigned vertexCount, unsigned meshCount, UPInt indexPtr, UPInt vertexOffset );
    virtual void        drawIndexedInstanced(unsigned indexCount, unsigned vertexCount, unsigned meshCount, UPInt indexPtr, UPInt vertexOffset );

    virtual void        initMatrices();

    // Simply sets a quad vertex buffer and draws.
    virtual void        drawScreenQuad();

    // *** Events
    virtual Render::RenderEvent& GetEvent(EventType eventType);

    // Cached versions read from GL_VERSION string.
    unsigned            MajorVersion, MinorVersion;
    // Cached GL_EXTENSIONS string.
    String              Extensions;
    
    // Tracks whether a compatible depth stencil format has been created.
    bool                DeterminedDepthStencilFormat;
};

// Use this HAL if you want to use profile modes.
class ProfilerHAL : public Render::ProfilerHAL<HAL>
{
public:
    ProfilerHAL(ThreadCommandQueue* commandQueue) : Render::ProfilerHAL<HAL>(commandQueue)
    {
    }
};

//--------------------------------------------------------------------
// RenderTargetData, used for both RenderTargets and DepthStencilSurface implementations.
class RenderTargetData : public RenderBuffer::RenderTargetData
{
public:
    friend class HAL;

    HAL*                    pHAL;
    GLuint                  FBOID;                  // Framebuffer object's ID.

    static void UpdateData( RenderBuffer* buffer, HAL* phal, GLuint fboID, DepthStencilBuffer* pdsb)
    {
        if ( !buffer )
            return;

        RenderTargetData* poldHD = (GL::RenderTargetData*)buffer->GetRenderTargetData();
        if ( !poldHD )
        {
            poldHD = SF_NEW RenderTargetData(buffer, phal, fboID, pdsb);
            buffer->SetRenderTargetData(poldHD);
            return;
        }
        poldHD->pDepthStencilBuffer = pdsb;
    }

    ~RenderTargetData();
    HAL* GetHAL() const     { return pHAL; }

private:
    RenderTargetData( RenderBuffer* buffer, HAL* hal, GLuint fboID, DepthStencilBuffer* pdsb ) : 
       RenderBuffer::RenderTargetData(buffer, pdsb), pHAL(hal), FBOID(fboID)
    { }
};

// This function is used with the GL_ARB_debug_output extension. Use HALConfig_DebugMessages in the InitHALParams to activate.
#if defined(SF_CC_GNU)
void GL_APIENTRY DebugMessageCallback(unsigned source,
                                   unsigned type,
                                   unsigned id,
                                   unsigned severity,
                                   int length,
                                   const GLchar* message,
                                   const void* userParam);
#else
void GL_APIENTRY DebugMessageCallback(GLenum source,
                                      GLenum type,
                                      GLuint id,
                                      GLenum severity,
                                      GLsizei length,
                                      const GLchar* message,
                                      GLvoid* userParam);
#endif

}}} // Scaleform::Render::GL

#endif
