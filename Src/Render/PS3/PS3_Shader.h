/**************************************************************************

Filename    :   PS3_Shader.h
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_PS3_Shader_H
#define INC_SF_PS3_Shader_H

#include <cell/gcm.h>
#include "Render/PS3/PS3_ShaderDescs.h"
#include "Render/Render_MemoryManager.h"
#include "Render/Render_Shader.h"


namespace Scaleform { namespace Render { namespace PS3 {

class HAL;
class Texture;
class MemoryManager;

struct VertexShader
{
    const VertexShaderDesc* pDesc;
    CGprogram               Prog;
    void *                  UCode;
    int                     Uniforms[PS3::Uniform::SU_Count];
    int                     Attributes[PS3::VertexShaderDesc::MaxVertexAttributes];

    bool Init(const VertexShaderDesc* pd);
    void Shutdown() { }
};

struct FragShader
{
    const FragShaderDesc*   pDesc;
    MemoryManager*          pMemManager;
    CGprogram               Prog;
    void *                  UCode;
    size_t                  UCodeSize;
    UPInt                   Offset;
    CGparameter             Uniforms[PS3::Uniform::SU_Count];

    FragShader() { Offset = 0; pMemManager = 0; UCode = 0; UCodeSize = 0; }
    ~FragShader();

    bool Init(const FragShaderDesc* pd, MemoryManager* pmm);
    void Shutdown();
};

struct ShaderPair
{
    const VertexShader*     pVS;
    const VertexShaderDesc* pVDesc;
    const FragShader*       pFS;
    const FragShaderDesc*   pFDesc;

    ShaderPair() : pVS(0), pVDesc(0), pFS(0), pFDesc(0) {}

    const ShaderPair* operator->() const { return this; }
};

class ShaderInterface : public ShaderInterfaceBase<Uniform,ShaderPair>
{
    HAL*                pHal;
    ShaderPair          CurShaders;
    const FragShader*   pLastFS;
    const VertexShader* pLastVS;

public:
    typedef ShaderPair Shader;

    ShaderInterface(Render::HAL* phal) : pHal((HAL*)phal), pLastFS(0), pLastVS(0) {}

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

    ShaderManager(ProfileViews* prof) : Base(prof) { }
    static unsigned GetDrawableImageFlags() { return 0; }
};

}}}

#endif
