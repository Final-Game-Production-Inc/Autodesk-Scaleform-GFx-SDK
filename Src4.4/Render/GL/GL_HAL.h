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
#include "Render/GL/GL_Common.h"
#include "Render/GL/GL_MeshCache.h"
#include "Render/GL/GL_Shader.h"
#include "Render/GL/GL_Texture.h"
#include "Render/GL/GL_Sync.h"
#include "Render/GL/GL_Events.h"
#include "Render/GL/GL_GraphicsDevice.h"
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
    HALConfig_DynamicShaderCompile          = HALConfig_DynamicShaderInit, // Alias.

    // Disables the use of binary shaders (loading and saving), even if the OpenGL/ES implementation
    // has the required support for it. 
    HALConfig_DisableBinaryShaders          = 0x00000002,

    // Instead of storing all binary shaders in a single file, which is only saved on shutdown, save each shader
    // in an individual file. This enables saving of binary shaders when they are dynamically initialized.
    HALConfig_MultipleShaderCacheFiles      = 0x00000004,

    // Disables the usage of separable shader program pipelines (eg. GL_EXT_separate_shader_objects),
    // even if the OpenGL/ES implementation has the required support for it.
    HALConfig_DisableShaderPipelines        = 0x00000008,

    // Shaders are compiled in passes by default (compile all, then check link status). This allows 
    // some drivers to thread the compilation, and can reduce startup times. This is ignored when 
    // using HALConfig_DynamicShaderCompile. 
    HALConfig_DisableMultipassShaderCompile = 0x00000010,

	// Enables debugging output via the glDebugMessageCallback. This can be useful to get driver specific
	// error messages. Note that a GL context with the debug-bit enabled is required for these to output.
	HALConfig_DebugMessages				    = 0x00000020,


    // If set, Vertex Array Objects will not be used. Note that on some GL versions (specifically GL 3.x+ and
    // GLES 3.0), VAO usage is required, and this flag will be ignored.
    HALConfig_NoVAO                         = 0x00000040,

    // If set, all GL commands and their parameters will be logged to the output.
    HALConfig_TraceGLExecution              = 0x00000080,
};

// GL::HALInitParams provides OpenGL-specific rendering initialization
// parameters for HAL::InitHAL.

struct HALInitParams : public Render::HALInitParams
{
    String BinaryShaderPath;

    HALInitParams(UInt32 halConfigFlags = 0,
                  ThreadId renderThreadId = ThreadId(),
                  const String& binaryShaderPath = String()) : 
        Render::HALInitParams(0, halConfigFlags, renderThreadId),
        BinaryShaderPath(binaryShaderPath)
    { }

    // GL::TextureManager accessors for correct type.
    void            SetTextureManager(TextureManager* manager) { pTextureManager = manager; }
    TextureManager* GetTextureManager() const       { return (TextureManager*) pTextureManager.GetPtr(); }
};

class HAL : public Render::ShaderHAL<ShaderManager, ShaderInterface>
{
    typedef Render::ShaderHAL<ShaderManager, ShaderInterface> BaseHAL;

    friend class  ScopedImmediateDeviceUsage;
    friend class  VertexBuilder_Legacy;
    friend class  ShaderObject;
    friend class  GL::MeshCache;
    friend class  GL::Texture;
public:    
    

    HAL(ThreadCommandQueue* commandQueue);
    virtual ~HAL();   

    virtual RenderTarget*           CreateRenderTarget(GLuint fbo);
    virtual RenderTarget*           CreateRenderTarget(Render::Texture* texture, bool needsStencil);
    virtual RenderTarget*           CreateTempRenderTarget(const ImageSize& size, bool needsStencil);


    virtual bool                    InitHAL(const Render::HALInitParams& params);
    virtual bool                    ShutdownHAL();

    virtual bool                    PrepareForReset();
    virtual bool                    RestoreAfterReset();

    virtual bool                    BeginScene();
    virtual bool                    EndScene();
    virtual bool                    Submit();

    virtual void                    ClearSolidRectangle(const Rect<int>& r, Color color, bool blend);
    virtual bool                    IsRasterModeSupported(RasterModeType mode) const;
    virtual float                   GetViewportScaling() const      { return 1.0f; }

    virtual Render::RenderSync*     GetRenderSync();
    virtual Render::RenderEvents&   GetEvents();
    virtual Render::TextureManager* GetTextureManager()             { return pTextureManager.GetPtr(); }
    virtual class MeshCache&        GetMeshCache()                  { return Cache; }
    virtual GL::GraphicsDevice*     GetGraphicsDevice()             { return pDevice; }
    virtual Render::GraphicsDevice* GetGraphicsDeviceBase()         { return pDevice; }

    virtual void                    MapVertexFormat(PrimitiveFillType fill, const VertexFormat* sourceFormat,
                                                    const VertexFormat** single,
                                                    const VertexFormat** batch, const VertexFormat** instanced, 
                                                    unsigned meshType = MeshCacheItem::Mesh_Regular);

    ShaderObject*                   GetStaticShader(ShaderDesc::ShaderType shaderType);
    bool                            ShouldUseVAOs();                        // Returns whether the HAL should use Vertex Array Objects.
    GL::HAL*                        GetHAL()            { return this; }    // Provided to be compatible with extension macros.

    bool                            CheckGLVersion(unsigned reqMajor, unsigned reqMinor);
    bool                            CheckExtension(GLExtensionType type);
    unsigned                        GetCaps() const;

protected:

    virtual void        beginDisplay(BeginDisplayData* data);
    virtual void        updateViewport();
    virtual bool        createDefaultRenderBuffer();
    virtual void        setRenderTargetImpl(Render::RenderTargetData* data, unsigned flags, const Color &clearColor); 

    virtual bool        checkDepthStencilBufferCaps();
    virtual void        applyDepthStencilMode(DepthStencilMode mode, unsigned stencilRef);
    virtual void        applyRasterModeImpl(RasterModeType mode);
    virtual void        applyBlendModeImpl(BlendMode mode, bool sourceAc = false, bool forceAc = false);
    virtual void        applyBlendModeEnableImpl(bool enabled);

    virtual void        setBatchUnitSquareVertexStream();
    virtual UPInt       setVertexArray(PrimitiveBatch* pbatch, Render::MeshCacheItem* pmesh);
    virtual UPInt       setVertexArray(const ComplexMesh::FillRecord& fr, unsigned formatIndex, Render::MeshCacheItem* pmesh);
    UPInt               setVertexArray(const VertexFormat* pFormat, Render::MeshCacheItem* pmesh, UPInt vboffset);
    void                setVertexArray(const VertexFormat* pFormat, HALGLBuffer* buffer, HALGLVertexArray* vao);
    virtual void        drawPrimitive(unsigned indexCount, unsigned meshCount);
    virtual void        drawIndexedPrimitive(unsigned indexCount, unsigned vertexCount, unsigned meshCount, UPInt indexPtr, UPInt vertexOffset );
    virtual void        drawIndexedInstanced(unsigned indexCount, unsigned vertexCount, unsigned meshCount, UPInt indexPtr, UPInt vertexOffset );

    ImageSize           getFboInfo(HALGLFramebuffer* fbo, HALGLFramebuffer*& currentFBO, bool useCurrent);
    DepthStencilBuffer* createCompatibleDepthStencil(const ImageSize& size, bool temporary);

    
    int                                 EnabledVertexArrays;
    GLint                               MaxVertexAttributes;
    bool                                FilterVertexBufferSet;
    bool                                DeterminedDepthStencilFormat;   // Tracks whether a compatible depth stencil format has been created.
    GL::MeshCache                       Cache;
    GL::RenderSync                      RSync;
    Ptr<GL::TextureManager>             pTextureManager;
    GL::RenderEvents                    Events;
    GL::GraphicsDeviceImmediate         ImmediateDevice;
    GL::GraphicsDevice*                 pDevice;
    Ptr<GL::GraphicsDeviceRecorder>     pRecordingDevice;               // Rendering delegate, records D3D9 API calls into a buffer. When Submit is called, they are executed.
    PrimitiveBatch::BatchType           PrevBatchType;                  // Previous batching mode
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
class RenderTargetData : public Render::RenderTargetData
{
public:
    friend class HAL;

    HAL*                    pHAL;
    Ptr<HALGLFramebuffer>   FBOID;                  // Framebuffer object's ID.

    static void UpdateData( RenderBuffer* buffer, HAL* phal, GLuint fboID, DepthStencilBuffer* pdsb);
    static void UpdateData( RenderBuffer* buffer, HAL* phal, HALGLFramebuffer* fboID, DepthStencilBuffer* pdsb);

    virtual ~RenderTargetData();
    virtual void StripDepthStencilTarget();


    HAL* GetHAL() const     { return pHAL; }

private:
    RenderTargetData( RenderBuffer* buffer, HAL* hal, HALGLFramebuffer* fboID, DepthStencilBuffer* pdsb ) : 
       Render::RenderTargetData(buffer, pdsb), pHAL(hal), FBOID(fboID)
    { }
};

}}} // Scaleform::Render::GL

#endif
