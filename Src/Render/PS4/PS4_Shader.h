/**********************************************************************

PublicHeader:   Render
Filename    :   Orbis_Shader.h
Content     :   
Created     :   2012/09/20
Authors     :   Bart Muzzin

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#ifndef INC_SF_PS4_Shader_H
#define INC_SF_PS4_Shader_H

#include "Render/PS4/PS4_Config.h"
#include "Render/PS4/PS4_ShaderDescs.h"
#include "Render/Render_Shader.h"

#include <shader.h>
#include <gnmx/shader_parser.h>

namespace Scaleform { namespace Render { namespace PS4 {

class HAL;
class Texture;

typedef Hash<const int*, sce::Gnmx::ShaderInfo> ShaderInfoHashType;

template<class ShaderDesc, sce::Gnmx::ShaderType ShaderType, sce::Gnm::ShaderStage ShaderStage>
struct BaseShader
{
protected:
    const ShaderDesc*         pDesc;
    sce::Gnmx::ShaderInfo     ShaderInfo;
    void*                     ShaderGPUAddress;
    bool                      IsPatched;
#ifdef SF_PS4_USE_LCUE
    sceGnmxSROType            SROTable;               // The shader resource bindings.
#endif

    BaseShader();
    ~BaseShader()
    {
        SF_DEBUG_ASSERT(ShaderGPUAddress == 0, "Shader still has GPU address pointer (possible leak). Make sure Shutdown was called.");
    }


    bool Init(ShaderInfoHashType& shaderInfoHash, Render::MemoryManager& mgr, sceGnmxContextType& context, const ShaderDesc* pd);
    void Shutdown(ShaderInfoHashType& shaderInfoHash, Render::MemoryManager& mgr);
};

struct VertexShader : public BaseShader<VertexShaderDesc, sce::Gnmx::ShaderType::kVertexShader, sce::Gnm::ShaderStage::kShaderStageVs>
{
    friend class ShaderManager;
    friend class ShaderInterface;
protected:
    uint32_t*                               FetchShaderGPUAddress;  // The GPU address of the generated fetch shader for this vertex shader.
    UPInt                                   FetchShaderSize;        // The size of the fetch shader.
    uint32_t                                ShaderModifier;         // The shader modifier value

    VertexShader();
    bool Init(ShaderInfoHashType& shaderInfoHash, Render::MemoryManager& mgr, sceGnmxContextType& context, const VertexShaderDesc* pd);
    void Shutdown(ShaderInfoHashType& shaderInfoHash, Render::MemoryManager& mgr);
};

struct FragShader : public BaseShader<FragShaderDesc, sce::Gnmx::ShaderType::kPixelShader, sce::Gnm::ShaderStage::kShaderStagePs>
{
protected:
    friend class ShaderManager;
    friend class ShaderInterface;
    bool Init(ShaderInfoHashType& shaderInfoHash, Render::MemoryManager& mgr, sceGnmxContextType& context, const FragShaderDesc* pd);
};

struct ShaderPair
{
    const VertexShader*     pVS;
    const VertexShaderDesc* pVDesc;
    const FragShader*       pFS;
    const FragShaderDesc*   pFDesc;
    const VertexFormat*     pVFormat;

    ShaderPair() : pVS(0), pVDesc(0), pFS(0), pFDesc(0), pVFormat(0) {}

    const ShaderPair* operator->() const { return this; }
    operator bool() const { return pVS && pFS && pVFormat; }
};

class ShaderInterface : public ShaderInterfaceBase<Uniform,ShaderPair>
{
    friend class ShaderManager;
public:
    typedef const ShaderPair Shader;

    ShaderInterface(Render::HAL* phal): pHal(reinterpret_cast<PS4::HAL*>(phal)), pLastVS(0), pLastFS(0) { }

    void                BeginScene();

    const Shader&       GetCurrentShaders() const { return CurShaders; }
    bool                SetStaticShader(ShaderDesc::ShaderType shader, const VertexFormat* pvf);

    void                SetTexture(Shader, unsigned stage, Render::Texture* ptexture, ImageFillMode fm, unsigned index = 0);

    void                BeginPrimitive();
    void                Finish(unsigned meshCount);

protected:
    void                finishApplyShaders(sceGnmxContextType& gnmxctx);

    HAL*                            pHal;
    ShaderPair                      CurShaders;     // Currently used set of VS/FS.
    sce::Gnmx::VsShader*            pLastVS;        // Last used Vertex Shader (for removing redundant changes)
    sce::Gnmx::PsShader*            pLastFS;        // Last used Fragment Shader (for removing redundant changes)

    // Holds textures to be applied. Note that, under the LCUE, textures must be set after the shader has been set. So,
    // they must be accumulated until ShaderInteface::Finish is called. This is now also done when using CUE as well.
    struct TextureEntry
    {
        Render::Texture*    pTexture;
        ImageFillMode       FillMode;
    };
    TextureEntry    TextureEntries[FragShaderDesc::MaxTextureSamplers];
};

class ShaderManager : public StaticShaderManager<ShaderDesc, VertexShaderDesc, Uniform, ShaderInterface, Texture>
{
    friend class ShaderInterface;
public:
    typedef StaticShaderManager<ShaderDesc, VertexShaderDesc, Uniform, ShaderInterface, Texture> Base;
    typedef Uniform UniformType;
    typedef ShaderInterface::Shader        Shader;

    ShaderManager(ProfileViews* prof);

    static unsigned GetDrawableImageFlags() { return 0; }

    // *** StaticShaderManager
    void    MapVertexFormat(PrimitiveFillType fill, const VertexFormat* sourceFormat,
                            const VertexFormat** single, const VertexFormat** batch, const VertexFormat** instanced);

    // Orbis Specific.
    bool    HasInstancingSupport() const;

    bool    Initialize(HAL* phal, Render::MemoryManager& pmemMgr);
    void    Reset(Render::MemoryManager& pmemMgr);

    void    BeginScene();
    void    EndScene();

private:
    FragShader                      StaticFShaders[FragShaderDesc::FSI_Count];
    VertexShader                    StaticVShaders[VertexShaderDesc::VSI_Count];

    ShaderInfoHashType              ShaderInfoHash[ShaderStage_Count];

    sceGnmxContextType*             GnmxCtx;        // Pointer to the Gnmx GfxContext.
};

}}}


#endif // INC_SF_D3D1x_SHADER_H
