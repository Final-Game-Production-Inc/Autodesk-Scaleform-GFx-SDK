/**************************************************************************

Filename    :   Vulkan_Shader.h
Content     :   Vulkan shader manager and interface.
Created     :   2026
Authors     :   Scaleform Vulkan Backend

**************************************************************************/

#ifndef INC_SF_VULKAN_SHADER_H
#define INC_SF_VULKAN_SHADER_H
#pragma once

#include "Render/Vulkan/Vulkan_Config.h"
#include "Render/Vulkan/Vulkan_ShaderDescs.h"
#include "Render/Render_Shader.h"

namespace Scaleform { namespace Render { namespace Vulkan {

class HAL;
class Texture;

struct VertexShader
{
    const VertexShaderDesc*     pDesc;
    VkShaderModule              pProg;
    VkDevice                    pDevice;

    VertexShader() : pDesc(0), pProg(VK_NULL_HANDLE), pDevice(VK_NULL_HANDLE) {}
    ~VertexShader() { Shutdown(); }

    bool Init(VkDevice device, const VertexShaderDesc* pd);
    void Shutdown();
};

struct FragShader
{
    const FragShaderDesc*       pDesc;
    VkShaderModule              pProg;
    VkDevice                    pDevice;
    UPInt                       Offset;

    FragShader() : pDesc(0), pProg(VK_NULL_HANDLE), pDevice(VK_NULL_HANDLE), Offset(0) {}
    ~FragShader() { Shutdown(); }

    bool Init(VkDevice device, const FragShaderDesc* pd);
    void Shutdown();
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
    operator bool() const { return pVS && pFS && pVS->pProg && pFS->pProg && pVFormat; }
};

class SysVertexFormat : public Render::SystemVertexFormat
{
public:
    static const unsigned MaxVertexElements = 8;
    VkVertexInputBindingDescription     BindingDesc;
    VkVertexInputAttributeDescription   Attributes[MaxVertexElements];
    unsigned                            AttributeCount;

    SysVertexFormat(const VertexFormat* vf, const VertexShaderDesc* pvdesc);
};


class ShaderInterface : public ShaderInterfaceBase<Uniform, ShaderPair>
{
    friend class ShaderManager;

    HAL*                pHal;
    ShaderPair          CurShaders;
    VkShaderModule      pLastVS;
    VkShaderModule      pLastFS;

    inline HAL* GetHAL() const { return pHal; }

public:
    typedef const ShaderPair Shader;

    ShaderInterface(Render::HAL* phal);

    void                BeginScene();
    const Shader&       GetCurrentShaders() const { return CurShaders; }
    bool                SetStaticShader(ShaderDesc::ShaderType shader, const VertexFormat* pvf);
    void                SetTexture(Shader, unsigned stage, Render::Texture* ptexture, ImageFillMode fm, unsigned index = 0);
    void                Finish(unsigned meshCount);
};


class ShaderManager : public StaticShaderManager<ShaderDesc, VertexShaderDesc, Uniform, ShaderInterface, Texture>
{
    friend class ShaderInterface;
public:
    typedef StaticShaderManager<ShaderDesc, VertexShaderDesc, Uniform, ShaderInterface, Texture> Base;
    typedef Uniform UniformType;

    ShaderManager(ProfileViews* prof);

    void    MapVertexFormat(PrimitiveFillType fill, const VertexFormat* sourceFormat,
                            const VertexFormat** single, const VertexFormat** batch, const VertexFormat** instanced);

    bool    HasInstancingSupport() const { return true; }

    bool    Initialize(HAL* phal);
    void    Reset();
    void    BeginScene();
    void    EndScene();

    ShaderDesc::ShaderVersion GetShaderVersion() const { return ShaderModel; }
    static unsigned GetDrawableImageFlags() { return 0; }

    FragShader      StaticFShaders[FragShaderDesc::FSI_Count];
    VertexShader    StaticVShaders[VertexShaderDesc::VSI_Count];
    VkDevice        pDevice;
    ShaderDesc::ShaderVersion ShaderModel;

    // SysVertexFormat cache keyed by (VertexFormat*, vsIndex) so that different
    // shaders using the same VertexFormat get correctly-mapped attribute locations.
    Hash<UPInt, Ptr<SysVertexFormat>> SysVFCache;
};

}}} // Scaleform::Render::Vulkan

#endif // INC_SF_VULKAN_SHADER_H
