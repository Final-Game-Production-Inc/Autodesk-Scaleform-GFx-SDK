/**********************************************************************

Filename    :   D3D9_Shader.h
Content     :   
Created     :   Jun 2011
Authors     :   Bart Muzzin

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#ifndef INC_SF_D3D9_SHADER_H
#define INC_SF_D3D9_SHADER_H
#pragma once

#include "Render/D3D9/D3D9_ShaderDescs.h"
#include "Render/Render_Shader.h"
#include <d3d9.h>

namespace Scaleform { namespace Render { namespace D3D9 {

class HAL;
class Texture;

struct VertexShader
{
    const VertexShaderDesc*             pDesc;
    Ptr<IDirect3DVertexShader9>         pProg;

    VertexShader() : pDesc(0), pProg(0) {}
    ~VertexShader() { Shutdown(); }

    bool Init(Render::HAL* phal, const VertexShaderDesc* pd);
    void Shutdown();
};

struct FragShader
{
    const FragShaderDesc*       pDesc;
    Ptr<IDirect3DPixelShader9>  pProg;

    FragShader() { pDesc = 0; pProg = 0; }
    ~FragShader() { Shutdown(); };

    bool Init(Render::HAL* phal, const FragShaderDesc* pd);
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
    Ptr<IDirect3DVertexDeclaration9>    pVDecl;
    D3DVERTEXELEMENT9                   VertexElements[8];

    SysVertexFormat(IDirect3DDevice9* pdevice, const VertexFormat* vf, const VertexShaderDesc* vdesc);
};

class ShaderInterface : public ShaderInterfaceBase<Uniform,ShaderPair>
{
    friend class ShaderManager;

    HAL*                                    pHal;
    ShaderPair                              CurShaders;     // Currently used set of VS/FS.
    IDirect3DVertexShader9*                 pLastVS;        // Last used Vertex Shader (for removing redundant changes)
    IDirect3DPixelShader9*                  pLastFS;        // Last used Fragment Shader (for removing redundant changes)
    IDirect3DVertexDeclaration9*            pLastDecl;      // Last used VertexDeclaration (for removing redundant changes)

    inline  HAL* GetHAL() const { return pHal; }

public:
    typedef const ShaderPair Shader;

    ShaderInterface(Render::HAL* phal): pHal((HAL*)phal), pLastVS(0), pLastDecl(0), pLastFS(0) { }

    void                BeginScene()
    {
        pLastVS = 0;
        pLastDecl = 0;
        pLastFS = 0;
    }

    const Shader&       GetCurrentShaders() const { return CurShaders; }
    bool                SetStaticShader(ShaderDesc::ShaderType shader, const VertexFormat* pvf);

    void                SetTexture(Shader, unsigned var, Render::Texture* ptexture, ImageFillMode fm, unsigned index = 0);

    void                Finish(unsigned meshCount);
};

class ShaderManager : public StaticShaderManager<ShaderDesc, VertexShaderDesc, Uniform, ShaderInterface, Texture>
{
    friend class ShaderInterface;
public:
    typedef StaticShaderManager<ShaderDesc, VertexShaderDesc, Uniform, ShaderInterface, Texture> Base;
    typedef Uniform UniformType;
    typedef ShaderDesc ShaderDescType;

    ShaderManager(ProfileViews* prof) : 
        StaticShaderManager(prof), pDevice(0), 
            InstancingSupport(false), DynamicLoopingSupport(false),
            ShaderModel(ShaderDesc::ShaderVersion_Default) { }

    // *** StaticShaderManager
    virtual ShaderDesc::ShaderVersion  GetShaderVersion() const { return ShaderModel; }
    bool                               HasInstancingSupport() const;
    bool                               HasDynamicLoopingSupport() const;

    // D3D9 Specific.

    bool    Initialize(HAL* phal);
    void    Reset();

    void    BeginScene();
    void    EndScene();

    static unsigned GetDrawableImageFlags() { return ShaderManager::CPF_HalfPixelOffset; }

private:
    Ptr<IDirect3DDevice9>         pDevice;
    FragShader                    StaticFShaders[FragShaderDesc::FSI_Count];
    VertexShader                  StaticVShaders[VertexShaderDesc::VSI_Count];
    bool                          InstancingSupport;
    bool                          DynamicLoopingSupport;
    ShaderDesc::ShaderVersion     ShaderModel;
};

}}}


#endif // INC_SF_D3D9_SHADER_H