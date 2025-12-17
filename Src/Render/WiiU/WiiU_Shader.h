/**************************************************************************

Filename    :   WiiU_Shader.h
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_WiiU_Shader_H
#define INC_SF_WiiU_Shader_H

#include <cafe/gx2.h>

#include "Render/WiiU/WiiU_ShaderDescs.h"
#include "Render/Render_Shader.h"


namespace Scaleform { namespace Render { namespace WiiU {

class HAL;
class Texture;
class MemoryManager;

struct VertexShader
{
    const VertexShaderDesc* pDesc;
    const GX2VertexShader*  pShader;
    int                     Uniforms[WiiU::Uniform::SU_Count];

    bool Init(const VertexShaderDesc* pd, const GX2VertexShader* ps);
    void Shutdown() { }
};

struct FragShader
{
    const FragShaderDesc*   pDesc;
    MemoryManager*          pMemManager;
    const GX2PixelShader*   pShader;
    int                     Uniforms[WiiU::Uniform::SU_Count];

    bool Init(const FragShaderDesc* pd, const GX2PixelShader* ps);
};

struct ShaderPair
{
    const VertexShader*           pVS;
    const VertexShaderDesc*       pVDesc;
    const FragShader*             pFS;
    const FragShaderDesc*         pFDesc;
    Ptr<SystemVertexFormat>       pVF;
    const GX2FetchShader*         pVFS;

    ShaderPair() : pVS(0), pVDesc(0), pFS(0), pFDesc(0), pVFS(0) {}

    const ShaderPair* operator->() const { return this; }
};

class SysVertexFormat : public Render::SystemVertexFormat
{
public:
    Render::MemoryManager*      pMemManager;
    GX2FetchShader              FetchShader;
    void*                       pShaderCode;
    UPInt                       CodeSize;

    SysVertexFormat(Render::MemoryManager* pmemmgr, const VertexFormat* vf);
    ~SysVertexFormat();
};

class ShaderInterface : public ShaderInterfaceBase<Uniform,ShaderPair>
{
    Render::HAL*               pHal;
    Render::MemoryManager*     pMemManager;
    FragShader                 StaticFShaders[FragShaderDesc::FSI_Count];
    VertexShader               StaticVShaders[VertexShaderDesc::VSI_Count];

    ShaderPair          CurShaders;
    const FragShader*   pLastFS;
    const VertexShader* pLastVS;

public:
    typedef ShaderPair Shader;

    ShaderInterface(Render::HAL* phal) : pHal(phal), pLastFS(0), pLastVS(0) {}

    bool                Initialize(Render::MemoryManager* pmemmgr);
    void                BeginFrame() { pLastVS = 0; pLastFS = 0; }

    const ShaderPair&   GetCurrentShaders() const { return CurShaders; }
    bool                SetStaticShader(ShaderDesc::ShaderType shader, const VertexFormat* pvf);

    void                SetTexture(ShaderPair sp, unsigned var, Render::Texture* ptexture, ImageFillMode fm, unsigned index = 0);

    void                Finish(unsigned meshCount =0);
};

class ShaderManager : public StaticShaderManager<ShaderDesc, VertexShaderDesc, Uniform, ShaderInterface, Texture>
{
public:
    typedef StaticShaderManager<ShaderDesc, VertexShaderDesc, Uniform, ShaderInterface, Texture> Base;
    typedef Uniform UniformType;
    typedef ShaderInterface::Shader        Shader;

    ShaderManager(ProfileViews* prof) : Base(prof) {}

    static unsigned GetDrawableImageFlags() { return 0; }
};

}}}

#endif
