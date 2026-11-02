/**************************************************************************

    Filename    :   D3D12_Shader.h
    Content     :   D3D12 shader manager and interface.
                    Static shader programs are described by the generated
                    D3D12_ShaderDescs. Pipeline state objects are created
                    lazily by the HAL (keyed by shader pair, blend state,
                    depth/stencil mode and raster mode), which is why the
                    shader objects here only retain the compiled bytecode.
    Created     :   2026/10/20
    Authors     :   Final Game Production Inc.

    Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.
                         Copyright 2026 Final Game Production Inc. All Rights Reserved.

    Use of this software is subject to the terms of the Autodesk license
    agreement provided at the time of installation or download, or which
    otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_D3D12_SHADER_H
#define INC_SF_D3D12_SHADER_H
#pragma once

#include "Render/D3D12/D3D12_Config.h"
#include "Render/D3D12/D3D12_ShaderDescs.h"
#include "Render/Render_Shader.h"

namespace Scaleform { namespace Render { namespace D3D12 {

class HAL;
class Texture;

// Vertex shader: retains the descriptor and the compiled bytecode.
struct VertexShader
{
    const VertexShaderDesc*     pDesc;
    ID3D12Device*               pDevice;
    D3D12_SHADER_BYTECODE       Bytecode;

    VertexShader() : pDesc(0), pDevice(0) { Bytecode.pShaderBytecode = 0; Bytecode.BytecodeLength = 0; }
    ~VertexShader() { Shutdown(); }

    bool Init(ID3D12Device* pdevice, const VertexShaderDesc* pd);
    void Shutdown();
};

// Fragment shader: retains the descriptor and the compiled bytecode.
struct FragShader
{
    const FragShaderDesc*       pDesc;
    ID3D12Device*               pDevice;
    D3D12_SHADER_BYTECODE       Bytecode;
    UPInt                       Offset;

    FragShader() : pDesc(0), pDevice(0), Offset(0)
    { Bytecode.pShaderBytecode = 0; Bytecode.BytecodeLength = 0; }
    ~FragShader() { Shutdown(); }

    bool Init(ID3D12Device* pdevice, const FragShaderDesc* pd);
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
    operator bool() const { return pVS && pFS && pVS->pDesc && pFS->pDesc && pVFormat; }
};

// System vertex format. The D3D12 input layout is already baked into the
// vertex shader descriptor (D3D12Attributes), so this object only needs to
// retain a copy of those elements for use in pipeline state creation.
class SysVertexFormat : public Render::SystemVertexFormat
{
public:
    static const unsigned       MaxVertexElements = 8;
    D3D12_INPUT_ELEMENT_DESC    VertexElements[MaxVertexElements];
    unsigned                    VertexElementCount;

    SysVertexFormat(const VertexFormat* vf, const VertexShaderDesc* vdesc);
};

class ShaderInterface : public ShaderInterfaceBase<Uniform, ShaderPair>
{
    friend class ShaderManager;

    HAL*                            pHal;
    ShaderPair                      CurShaders;     // Currently used set of VS/FS.

    inline  HAL* GetHAL() const { return pHal; }

public:
    typedef const ShaderPair Shader;

    ShaderInterface(Render::HAL* phal) : pHal((HAL*)phal) { }

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
    // Prototype constructor; equivalent to the ProfileViews constructor.
    ShaderManager(Render::HAL* hal);

    // *** StaticShaderManager
    void    MapVertexFormat(PrimitiveFillType fill, const VertexFormat* sourceFormat,
                            const VertexFormat** single, const VertexFormat** batch, const VertexFormat** instanced);

    // D3D12 Specific.
    bool    HasInstancingSupport() const { return false; }

    // D3D12 cannot use the generic multi-pass Box blur path: the intermediate
    // passes render into a temporary target and immediately sample it back for
    // the next pass, and on this driver that read/write ping-pong hangs the GPU
    // (DXGI_ERROR_DEVICE_HUNG) about 2/3 of the time when the 9-1-B feedback
    // banner is rebuilt. The final shadow/glow shader performs the same blur in
    // a single pass (the low-end platforms do this as well). Set
    // GFX_D3D12_MULTIPASS_BLUR=1 to re-enable the multi-pass path for debugging.
    unsigned SetupFilter(const Filter* filter, unsigned fillFlags, unsigned* passes, BlurFilterState& leBlur) const;

    bool    Initialize(HAL* phal, unsigned configFlags);
    void    Reset();

    // Stores the command list used for pipeline binding / descriptor updates.
    void    SetCommandList(ID3D12GraphicsCommandList* commandList)
    {
        pCommandList = commandList;
    }

    virtual void OnHALEvent(HALNotifyType type);

    void    BeginScene();
    void    EndScene();

    ShaderDesc::ShaderVersion GetShaderVersion() const { return ShaderModel; }
    static unsigned GetDrawableImageFlags() { return 0; }

    // Static shader program storage (accessed by the HAL for pipeline creation).
    FragShader                      StaticFShaders[FragShaderDesc::FSI_Count];
    VertexShader                    StaticVShaders[VertexShaderDesc::VSI_Count];

private:
    Ptr<ID3D12Device>               pDevice;        // Pointer to the D3D12 device.
    Ptr<ID3D12GraphicsCommandList>  pCommandList;   // Command list for pipeline binding.
    ShaderDesc::ShaderVersion       ShaderModel;    // Holds which ShaderModel version should be used.
};

}}} // Scaleform::Render::D3D12

#endif // INC_SF_D3D12_SHADER_H
