/**************************************************************************

Filename    :   GLES11_HAL.h
Content     :   GLES 1.1 Renderer HAL implementation.
Created     :   Feb 2012
Authors     :   Bart Muzzin

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_GLES11_HAL_H
#define INC_SF_Render_GLES11_HAL_H

#include "Render/Render_HAL.h"
#include "Render/Render_Queue.h"
#include "Render/GL/GL_Common.h"
#include "Render/GL/GL_MeshCache.h"
#include "Render/GL/GL_Texture.h"

namespace Scaleform { namespace Render { namespace GL {

// HALConfigFlags enumeration defines system-specific HAL configuration
// flags passed into InitHAL though HALInitParams.
enum HALConfigFlags
{
    // Only compile shaders when they are actually used. This can reduce startup-times,
    // however, compiling shaders dynamically can cause performance spikes during playback.
    HALConfig_DynamicShaderCompile  = 0x00000001,
};

// GL::HALInitParams provides OpenGL-specific rendering initialization
// parameters for HAL::InitHAL.

struct HALInitParams : public Render::HALInitParams
{
    HALInitParams(UInt32 halConfigFlags = 0,
        ThreadId renderThreadId = ThreadId())
        : Render::HALInitParams(0, halConfigFlags, renderThreadId)
    { }

    // GL::TextureManager accessors for correct type.
    void            SetTextureManager(TextureManager* manager) { pTextureManager = manager; }
    TextureManager* GetTextureManager() const       { return (TextureManager*) pTextureManager.GetPtr(); }
};

enum CapFlags
{
    Cap_NoBatching      = 0x10,

    // Caps for buffers in mesh cache. Static buffers are still used even if none of these caps are set.
    Cap_MapBuffer       = 0x20,
    Cap_BufferUpdate    = 0x40,
    Cap_MapBufferRange  = 0x80, // GLES 1.1 does not support this extension, it is here to make code cross-compilable.
    Cap_UseMeshBuffers  = Cap_MapBuffer | Cap_BufferUpdate | Cap_MapBufferRange,

    Caps_Standard       = Cap_MapBuffer
};

class HAL : public Render::HAL
#ifdef SF_GL_RUNTIME_LINK
    , public GL::Extensions
#endif
{
public:

    MemoryHeap*         pHeap;

    bool                MultiBitStencil;
    int                 EnabledVertexArrays;
    unsigned            ActiveTextures;         // Bit-flag for enabled textures.
    
    int                 MaxTexUnits;            // Number of TUs reported by GL_MAX_TEXTURE_UNITS
    
    MeshCache           Cache;
    
    Ptr<TextureManager> pTextureManager;

    unsigned                Caps;

    // Self-accessor used to avoid constructor warning.
    HAL*      getThis() { return this; }

public:    

    HAL(ThreadCommandQueue* commandQueue);
    virtual ~HAL();

    // *** Implement Dependent Video Mode configuration

    virtual bool        InitHAL(const GL::HALInitParams& params);

    // Returns back to original mode (cleanup)
    virtual bool        ShutdownHAL();

    // Used when the current gl context is lost (GLES on some platforms)
    // or becomes incompatible with the new video mode
    virtual bool        ResetContext();

    // *** Rendering

    virtual bool        BeginFrame();
    virtual void        FinishFrame();

    virtual bool        BeginScene();

    // Bracket the displaying of a frame from a movie.
    // Fill the background color, and set up default transforms, etc.
    virtual void        beginDisplay(BeginDisplayData* data);
    virtual void        endDisplay();

    virtual void        GetHWViewMatrix(Matrix* pmatrix, const Viewport& vp);

    // Updates HW Viewport and ViewportMatrix based on the current
    // values of VP, ViewRect and ViewportValid.
    void                updateViewport();


    // GLES 1.1 does not support VAOs, this is here to make the code cross-compilable.
    bool                ShouldUseVAOs() const { return false; }

    virtual void        DrawProcessedPrimitive(Primitive* pprimitive,
                                               PrimitiveBatch* pstart, PrimitiveBatch *pend);

    virtual void        DrawProcessedComplexMeshes(ComplexMesh* p,
                                                   const StrideArray<HMatrix>& matrices);
    
    // *** Mask Support

    // Mask support is implemented as a stack, with three possible operations doing
    // the following:
    //  PushMask_BeginSubmit - pushes a new mask on stack and begins "submit mask" rendering.
    //  EndMaskSubmit - Ends submit mask and begins content rendering, clipped by the mask.
    //  PopMask - pops the mask from stack; further rendering will use previous masks, if any.
    virtual void    PushMask_BeginSubmit(MaskPrimitive* primitive);    
    virtual void    EndMaskSubmit();
    virtual void    PopMask();

    void    drawMaskClearRectangles(const HMatrix* matrices, UPInt count);
    void    clearSolidRectangle(const Rect<int>& r, Color color, bool blend);
    
    // This flag indicates whether to use stencil-based masking or Z-based masking. This is
    // implemented more as a hedge in case we want to run on ES 1.1 implementations that actually
    // do report that they have bits available for stencil, as PowerVR MBX in iPhone <3GS does not.
    bool            StencilAvailable;
    
    // *** Filters (disabled)
    virtual void        PrepareFilters(FilterPrimitive*) { };
    virtual void        PushFilters(FilterPrimitive*)    { };
    virtual void        PopFilters()                     { };

    // *** Render target state management - all disabled.
    virtual Render::RenderBufferManager* GetRenderBufferManager() const { return pRenderBufferManager; };
    virtual Render::RenderTarget* GetDefaultRenderTarget() { return 0; };
    virtual Render::RenderTarget* CreateRenderTarget(Render::Texture*, bool) { return 0; };
    virtual Render::RenderTarget* CreateTempRenderTarget(const ImageSize&, bool) { return 0; };
    virtual bool    SetRenderTarget(Render::RenderTarget*, bool setState = 1) { SF_UNUSED(setState); return false; };
    virtual void    PushRenderTarget(const RectF&, Render::RenderTarget* ) { };
    virtual void    PopRenderTarget(unsigned flags = 0) { };
    virtual bool    createDefaultRenderBuffer() { return false; };
    virtual void    destroyRenderBuffers() { };

    bool SetVertexArray(PrimitiveFillType fillType, const VertexFormat* pFormat, GLuint buffer, UByte* vertexOffset, int textureBase = 0);
    void SetFactorArray(const VertexFormat* pFormat, GLuint buffer, UByte* vertexOffset, int factorTexture);


    // *** BlendMode
    virtual void    applyBlendModeImpl(BlendMode mode, bool sourceAc = false, bool forceAc = false);

    virtual Render::TextureManager* GetTextureManager() const
    {
        return pTextureManager.GetPtr();
    }

    virtual class MeshCache&              GetMeshCache()
    {
        return Cache;
    }
        
    virtual void    MapVertexFormat(PrimitiveFillType fill, const VertexFormat* sourceFormat,
                                    const VertexFormat** single,
                                    const VertexFormat** batch, const VertexFormat** instanced,
                                    unsigned flags);

    // Check whether the given extension exists in the current profile.
    bool                CheckExtension(const char *name);

    // Check whether the input major/minor version pair is greater or equal to the current profile version.
    bool                CheckGLVersion(unsigned reqMajor, unsigned reqMinor);

    virtual RQCacheInterface& GetRQCacheInterface()
    {
        return QueueProcessor.GetQueueCachesRef();
    }

    enum MatrixType
    {
        MT_MVP,
        MT_Texture,
        MT_Cxform,
        MT_Count
    };

    int SetPrimitiveFill(PrimitiveFill* pfill, UInt32 fillFlags, PrimitiveBatch::BatchType btype, const VertexFormat* pformat,
        unsigned batchCount, const MatrixState* mstate, const Primitive::MeshEntry* pmeshes, int *textureBase);
    void SetFill(PrimitiveFillType fillType, unsigned fillFlags, const VertexFormat* vf );
    void SetMatrix(MatrixType type, const Matrix2F &m1, const HMatrix &m2, const MatrixState* Matrices, unsigned index = 0, unsigned batch = 0);
    void SetMatrix(MatrixType type, const Matrix2F &m, unsigned index = 0, unsigned batch = 0);
    void SetCxform(unsigned stage, const Cxform & cx, unsigned index = 0, unsigned batch = 0);
    int SetComplexCombiners(bool vertexColor, bool solidColor, bool uvTexture, int textureCount, PrimitiveFill* pfill, const Cxform &cx);
    void SetColor(unsigned stage, Color c, unsigned index = 0, unsigned batch = 0);
    void SetVertexColors(unsigned stage);
    void SetTexture(PrimitiveFillType fillType, unsigned stage, Texture* ptex, ImageFillMode mode, bool deferredCombine, bool uvTexture, bool vcolor);
    void DisableExtraStages(unsigned stage);

    MultiKeyCollection<VertexElement, VertexFormat, 32>         VFormats;
    
    unsigned DummyTextureID;
    
    unsigned AddTextureID[256];
    unsigned EdgeAAID;

protected:

    virtual void        initMatrices();
    
    void                drawPrimitive(unsigned indexCount, unsigned meshCount);
    void                drawIndexedPrimitive( unsigned indexCount, unsigned meshCount, UByte* indexPtr);

    // Cached versions read from GL_VERSION string.
    unsigned            MajorVersion, MinorVersion;
    // Cached GL_EXTENSIONS string.
    String              Extensions;
};


}}} // Scaleform::Render::GL

#endif
