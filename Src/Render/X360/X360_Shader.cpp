/**************************************************************************

Filename    :   X360_Shader.cpp
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render/X360/X360_Shader.h"
#include "Render/X360/X360_HAL.h"
#include "Kernel/SF_Debug.h"

namespace Scaleform { namespace Render { namespace X360 {

extern const char* ShaderUniformNames[Uniform::SU_Count];

bool VertexShader::Init(IDirect3DDeviceX* pdevice, const VertexShaderDesc* pd)
{
    pDesc = pd;

    if ( FAILED( pdevice->CreateVertexShader((const DWORD*)pDesc->pBinary, &pProg.GetRawRef())))
        return false;

    // The following code will determine if the shader generator has put the constants in the
    // table locations that we expect. If they are, this shouldn't be necessary. If they aren't,
    // the shader generator needs to be fixed.
    Ptr<ID3DXConstantTable> constantTable = 0;
    if ( FAILED( D3DXGetShaderConstantTable((const DWORD*)pDesc->pBinary, &constantTable.GetRawRef())))
        return false;

    for (unsigned i = 0; i < Uniform::SU_Count; i++)
    {
        const UINT MaxDescs = 8;
        D3DXCONSTANT_DESC descs[MaxDescs];
        if (!(pDesc->Uniforms[i].Location >= 0))
        {
            Uniforms[i] = -1;
            continue;
        }

        D3DXHANDLE hconstant = constantTable->GetConstantByName(0, ShaderUniformNames[i]);
        UINT descCount = MaxDescs;
        if ( hconstant == 0 || FAILED( constantTable->GetConstantDesc(hconstant, descs, &descCount) ))
        {
            SF_DEBUG_MESSAGE1(1, "Shader uniform: %s expected, but not found\n", ShaderUniformNames[i]);
            return false;
        }

        SF_ASSERT((int)descs[0].RegisterIndex == pDesc->Uniforms[i].Location);
        Uniforms[i] = descs[0].RegisterIndex;
        SF_ASSERT(Uniforms[i] >= 0);
    }

    return true;
}

void VertexShader::Shutdown( )
{
    pProg = 0;
}

bool FragShader::Init(IDirect3DDevice9* pdevice, const FragShaderDesc* pd)
{
    pDesc = pd;

    if ( FAILED( pdevice->CreatePixelShader((const DWORD*)pDesc->pBinary, &pProg.GetRawRef())))
        return false;

    // The following code will determine if the shader generator has put the constants in the
    // table locations that we expect. If they are, this shouldn't be necessary. If they aren't,
    // the shader generator needs to be fixed.
    Ptr<ID3DXConstantTable> constantTable = 0;
    if ( FAILED( D3DXGetShaderConstantTable((const DWORD*)pDesc->pBinary, &constantTable.GetRawRef())))
        return false;

    for (unsigned i = 0; i < Uniform::SU_Count; i++)
    {
        const UINT MaxDescs = 8;
        D3DXCONSTANT_DESC descs[MaxDescs];
        if (!(pDesc->Uniforms[i].Location >= 0))
        {
            Uniforms[i] = -1;
            continue;
        }

        D3DXHANDLE hconstant = constantTable->GetConstantByName(0, ShaderUniformNames[i]);
        UINT descCount = MaxDescs;
        if ( hconstant == 0 || FAILED( constantTable->GetConstantDesc(hconstant, descs, &descCount) ))
        {
            // Some frag shaders contain scolor2, but only reference it during Highlight, so it
            // is sometimes compiled out of the constant table. This should be fixed, but is not crucial.
            if ( i != Uniform::SU_scolor2 )
            {
                SF_DEBUG_MESSAGE1(1, "Shader uniform: %s expected, but not found\n", ShaderUniformNames[i]);
                return false;
            }
            else
            {
                Uniforms[i] = -1;
                continue;
            }
        }

        SF_ASSERT((int)descs[0].RegisterIndex == pDesc->Uniforms[i].Location);
        Uniforms[i] = descs[0].RegisterIndex;
        SF_ASSERT(Uniforms[i] >= 0);
    }

    return true;
}

void FragShader::Shutdown( )
{
    pProg = 0;
}

struct D3DVertexDeclBuilder
{
    static const unsigned   MaxElements = 8;
    D3DVERTEXELEMENT9       Elements[MaxElements];
    WORD                    Size[2], Count;
    const VertexShaderDesc* VertexDesc;


    D3DVertexDeclBuilder(const VertexShaderDesc* vdesc) : Count(0), VertexDesc(vdesc)
    { Size[0] = Size[1] = 0; }

    void    Add(int, unsigned attr, int, int)
    {
        // Find the matching vertex attribute within the vertex desc.
        int attrIndex;
        for ( attrIndex = 0; attrIndex < VertexDesc->NumAttribs; ++attrIndex )
        {
            if ( (VertexDesc->Attributes[attrIndex].Attr&(VET_Usage_Mask|VET_Index_Mask)) == (attr&(VET_Usage_Mask|VET_Index_Mask)))
                break;
            if ( (VertexDesc->Attributes[attrIndex].Attr&VET_Index_Mask) && (attr&(VET_Index_Mask)) )
                break;
        }
        SF_DEBUG_ASSERT(attrIndex < VertexDesc->NumAttribs, "Unexpected attribute");

        D3DDECLTYPE type = D3DDECLTYPE_UNUSED;
        WORD byteSize = 0;
        WORD stream = 0;
        getD3DType(attr, type, byteSize, stream);

        AddD3DElement(stream, type, byteSize, VertexDesc->Attributes[attrIndex].Usage, (BYTE)VertexDesc->Attributes[attrIndex].UsageIndex );
    }


    void    AddD3DElement(WORD stream, D3DDECLTYPE type, WORD size,
        unsigned usage, BYTE usageIndex = 0)
    {
        SF_ASSERT(stream< MaxElements);

        D3DVERTEXELEMENT9 &e = Elements[Count++];
        e.Stream    = stream;
        e.Offset    = Size[stream];
        e.Type      = type;
        e.Method    = D3DDECLMETHOD_DEFAULT;
        e.Usage     = (BYTE)usage;
        e.UsageIndex= usageIndex;
        Size[stream]= Size[stream] + size;
        SF_ASSERT(Count < MaxElements);
    }

    void    Finish(int)
    {
        D3DVERTEXELEMENT9 e = D3DDECL_END();
        Elements[Count++] = e;
    }

    void getD3DType( unsigned attr, D3DDECLTYPE &type, WORD &byteSize, WORD& stream) 
    {
        stream = 0;
        switch( attr & (VET_CompType_Mask|VET_Components_Mask) )
        {
        case VET_U8N|1:
        case VET_U8N|2:
        case VET_U8N|4:
            type = D3DDECLTYPE_UBYTE4N; 
            byteSize = 4*sizeof(UByte);
            break;
        case VET_I8|1:
            stream = 1; 
            byteSize = 2*sizeof(short);
            type = D3DDECLTYPE_SHORT2;
            break;
        case VET_U8|1:
        case VET_U8|2:
        case VET_U8|4:
            type = D3DDECLTYPE_UBYTE4; 
            byteSize = 4*sizeof(UByte);
            break;
        case VET_U32|1:
            type = D3DDECLTYPE_D3DCOLOR; 
            byteSize = 4*sizeof(UByte);
            break;
        case VET_S16|2:
            type = D3DDECLTYPE_SHORT2; 
            byteSize = 2*sizeof(short);
            break;
        case VET_F32|2:
            type = D3DDECLTYPE_FLOAT2; 
            byteSize = 2*sizeof(float);
            break;

        default: SF_DEBUG_ASSERT(0, "Unsupported vertex format.");
        }
    }
};

SysVertexFormat::SysVertexFormat(IDirect3DDevice9* pdevice, const VertexFormat* vf, const VertexShaderDesc* pvdesc) : pVDecl(0)
{
    D3DVertexDeclBuilder builder(pvdesc);
    BuildVertexArray(vf, builder);

    // TODO: hash vertex decls within this class.
    if ( FAILED( pdevice->CreateVertexDeclaration(builder.Elements, &pVDecl.GetRawRef())))
    {
        SF_ASSERT(0);
        pVDecl = 0;
    }
    memcpy(VertexElements, builder.Elements, sizeof VertexElements);
}

bool ShaderInterface::SetStaticShader(ShaderDesc::ShaderType shader, const VertexFormat* pformat)
{
    VertexShaderDesc::ShaderIndex vsIndex = VertexShaderDesc::GetShaderIndex(shader);
    FragShaderDesc::ShaderIndex psIndex = FragShaderDesc::GetShaderIndex(shader);

    CurShaders.pVFormat = pformat;
    CurShaders.pVS      = &pHal->StaticVShaders[vsIndex];
    CurShaders.pVDesc   = CurShaders.pVS->pDesc;
    CurShaders.pFS      = &pHal->StaticFShaders[psIndex];
    CurShaders.pFDesc   = CurShaders.pFS->pDesc;

    if ( pformat && !pformat->pSysFormat )
        (const_cast<VertexFormat*>(pformat))->pSysFormat = *SF_NEW SysVertexFormat(pHal->GetDevice(), pformat, CurShaders.pVS->pDesc);

    return (bool)CurShaders;
}

void ShaderInterface::SetTexture(Shader, unsigned var, Render::Texture* ptexture, ImageFillMode fm, unsigned index)
{
    X360::Texture *pd3dTexture = (X360::Texture*)ptexture;
    SF_ASSERT(CurShaders.pFDesc->Uniforms[var].Location >= 0 );
    SF_ASSERT(CurShaders.pFDesc->Uniforms[var].Location + CurShaders.pFDesc->Uniforms[var].Size >= 
        (short)(index + ptexture->TextureCount));
    pd3dTexture->ApplyTexture(CurShaders.pFDesc->Uniforms[var].Location + index, fm);
}

// Record the min/max shader constant registers, and just set the entire buffer in one call,
// instead of doing them individually. This could result in some uninitialized data to be sent,
// but it should be unreferenced by the shaders, and should be more efficient.
struct ShaderConstantRange
{
    ShaderConstantRange(IDirect3DDevice9* pdevice, float * data) : Min(SF_MAX_SINT), Max(-1), ShadowLocation(0), UniformData(data), pDevice(pdevice) { };
    bool Update( int location, int size, int shadowLocation, bool fragment )
    {
        location *= 4; // 4 elements per register.
        if ( (shadowLocation - location) != ShadowLocation && Max >= 0 )
        {
            // This shouldn't be the case, but if the storage is disjoint for some reason
            // we will need to upload the constants in separate steps.
            SF_ASSERT(0);
            if ( fragment )
                pDevice->SetPixelShaderConstantF(Min, UniformData + ShadowLocation, Max - Min);
            else
                pDevice->SetVertexShaderConstantF(Min, UniformData + ShadowLocation, Max - Min);
            ShadowLocation = shadowLocation - location;
            Min = SF_MAX_SINT;
            Max = -1;
            return true;
        }
        ShadowLocation = shadowLocation - location;
        Min = Alg::Min(Min, location/4);
        Max = Alg::Max(Max, (location + size)/4); // 4 elements per register.
        return true;
    }
    int Min, Max, ShadowLocation;
    float* UniformData;
    IDirect3DDevice9* pDevice;
};

void ShaderInterface::Finish(unsigned meshCount)
{
    // Mesh count is unneeded - the constant registers that have been updated are tracked,
    // which includes any registers set for batching.
    SF_UNUSED(meshCount);

    IDirect3DDevice9* pdevice = pHal->GetDevice();
    ShaderConstantRange shaderConstantRangeFS(pdevice, UniformData);
    ShaderConstantRange shaderConstantRangeVS(pdevice, UniformData);
    for (int i = 0; i < Uniform::SU_Count; i++)
    {
        if (UniformSet[i])
        {
            if (CurShaders.pFS->Uniforms[i] >= 0)
            {
                SF_ASSERT(CurShaders.pFDesc->Uniforms[i].Location >= 0);
                shaderConstantRangeFS.Update( CurShaders.pFDesc->Uniforms[i].Location,
                                               CurShaders.pFDesc->Uniforms[i].Size,
                                               CurShaders.pFDesc->Uniforms[i].ShadowOffset,
                                               true );
            }
            else if (CurShaders->pVS->Uniforms[i] >= 0 )
            {
                SF_ASSERT(CurShaders.pVDesc->Uniforms[i].Location >= 0);
                shaderConstantRangeVS.Update( CurShaders.pVDesc->Uniforms[i].Location,
                                               CurShaders.pVDesc->Uniforms[i].Size,
                                               CurShaders.pVDesc->Uniforms[i].ShadowOffset,
                                               false );
            }
        }
    }

    if ( shaderConstantRangeVS.Max >= 0 )
    {
        pdevice->SetVertexShaderConstantF(
            shaderConstantRangeVS.Min, 
            UniformData + shaderConstantRangeVS.ShadowLocation, 
            shaderConstantRangeVS.Max - shaderConstantRangeVS.Min);
    }
    if ( shaderConstantRangeFS.Max >= 0 )
    {
        pdevice->SetPixelShaderConstantF(
            shaderConstantRangeFS.Min, 
            UniformData  + shaderConstantRangeFS.ShadowLocation, 
            shaderConstantRangeFS.Max - shaderConstantRangeFS.Min);
    }
    memset(UniformSet, 0, sizeof(UniformSet));

    if (pLastVS != CurShaders.pVS->pProg)
    {
        pdevice->SetVertexShader(CurShaders.pVS->pProg);
        pLastVS = CurShaders.pVS->pProg;
    }

    IDirect3DVertexDeclaration9* pdecl = ((SysVertexFormat*)(CurShaders.pVFormat->pSysFormat.GetPtr()))->pVDecl;
    if (pLastDecl != pdecl)
    {
        pdevice->SetVertexDeclaration(pdecl);
        pLastDecl = pdecl;
    }

    if (pLastFS != CurShaders.pFS->pProg)
    {
        pdevice->SetPixelShader(CurShaders.pFS->pProg);
        pLastFS = CurShaders.pFS->pProg;
    }
}

// *** ShaderManager
bool ShaderManager::HasInstancingSupport() const
{
    return true;
}

bool ShaderManager::Initialize(HAL* phal)
{
    pDevice = phal->GetDevice();
    return true;
}

void ShaderManager::BeginScene()
{
    
}

void ShaderManager::EndScene()
{

}

void ShaderManager::Reset()
{
    pDevice = 0;
}

}}}
