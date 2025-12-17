/**********************************************************************

Filename    :   D3D1x_Shader.h
Content     :   
Created     :   Jun 2011
Authors     :   Bart Muzzin

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#ifndef INC_SF_D3D1x_SHADER_H
#define INC_SF_D3D1x_SHADER_H
#pragma once

#include "Render/D3D1x/D3D1x_Config.h"
#if defined(_DURANGO)
    #include "Render/D3D1x/XboxOne_ShaderDescs.h"
#else
    #include "Render/D3D1x/D3D1x_ShaderDescs.h"
#endif
#include "Render/Render_Shader.h"

namespace Scaleform { namespace Render { namespace D3D1x {

class HAL;
class Texture;

struct VertexShader
{
    const VertexShaderDesc*             pDesc;
    Ptr<ID3D1x(VertexShader)>           pProg;
    int                                 UniformOffset[Uniform::SU_Count];

    VertexShader() : pDesc(0), pProg(0) {}
    ~VertexShader() { Shutdown(); }

    bool Init(ID3D1x(Device)* pdevice, const VertexShaderDesc* pd);
    void Shutdown();
};

struct FragShader
{
    const FragShaderDesc*       pDesc;
    Ptr<ID3D1x(PixelShader)>    pProg;
    UPInt                       Offset;
    int                         UniformOffset[Uniform::SU_Count];

    FragShader() { pProg = 0; Offset = 0; }
    ~FragShader() { Shutdown(); };

    bool Init(ID3D1x(Device)* pdevice, const FragShaderDesc* pd);
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
    Ptr<ID3D1x(InputLayout)>    pVDecl;
    static const unsigned       MaxVertexElements = 8;
    D3D1x(INPUT_ELEMENT_DESC)   VertexElements[MaxVertexElements];
    unsigned                    VertexElementCount;

    SysVertexFormat(ID3D1x(Device)* pdevice, const VertexFormat* vf, const VertexShaderDesc* vdesc);
};

class ShaderInterface : public ShaderInterfaceBase<Uniform,ShaderPair>
{
    friend class ShaderManager;

    HAL*                            pHal;
    ShaderPair                      CurShaders;     // Currently used set of VS/FS.
    ID3D1x(VertexShader)*           pLastVS;        // Last used Vertex Shader (for removing redundant changes)
    ID3D1x(PixelShader)*            pLastFS;        // Last used Fragment Shader (for removing redundant changes)
    ID3D1x(InputLayout)*            pLastDecl;      // Last used VertexDeclaration (for removing redundant changes)

    inline  HAL* GetHAL() const { return pHal; }

public:
    typedef const ShaderPair Shader;

    ShaderInterface(Render::HAL* phal): pHal((HAL*)phal), pLastVS(0), pLastDecl(0), pLastFS(0) { }

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

    // *** StaticShaderManager
    void    MapVertexFormat(PrimitiveFillType fill, const VertexFormat* sourceFormat,
                            const VertexFormat** single, const VertexFormat** batch, const VertexFormat** instanced);

    // D3D1x Specific.
    bool    HasInstancingSupport() const;

    bool    Initialize(HAL* phal);
    void    Reset();

    void    BeginScene();
    void    EndScene();

    ShaderDesc::ShaderVersion GetShaderVersion() const { return ShaderModel; }
    static unsigned GetDrawableImageFlags() { return 0; }

private:
    FragShader                      StaticFShaders[FragShaderDesc::FSI_Count];
    VertexShader                    StaticVShaders[VertexShaderDesc::VSI_Count];

    Ptr<ID3D1x(Device)>             pDevice;        // Pointer to the D3D device.
    ShaderDesc::ShaderVersion       ShaderModel;    // Holds which ShaderModel version should be used.
};

}}}


#endif // INC_SF_D3D1x_SHADER_H