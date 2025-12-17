/**************************************************************************

Filename    :   D3D9_Shader.cpp
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render/D3D9/D3D9_Shader.h"
#include "Render/D3D9/D3D9_HAL.h"
#include "Kernel/SF_Debug.h"

#include "Render/D3D9/D3D9_ShaderDescs.h"
#if SF_CC_MSVC < 1700
#include <d3dx9shader.h>
#endif

namespace Scaleform { namespace Render { namespace D3D9 {

extern const char* ShaderUniformNames[Uniform::SU_Count];

bool VertexShader::Init(Render::HAL* phal, const VertexShaderDesc* pd)
{
    if ( !phal || !pd )
        return false;
    IDirect3DDevice9* pdevice = ((D3D9::HAL*)phal)->pDevice;

    pDesc = pd;

    if ( FAILED( pdevice->CreateVertexShader((const DWORD*)pDesc->pBinary, &pProg.GetRawRef())))
        return false;

    // The following block uses D3DX9 reflection to determine if the shader inputs match the descriptors.
    // D3DX is not available in VC2011+, so just assume that they are in the correct positions. Also, only do this in debug.
#if SF_CC_MSVC < 1700 && defined(SF_BUILD_DEBUG)
    Ptr<ID3DXConstantTable> constantTable = 0;
    if ( FAILED( D3DXGetShaderConstantTableEx((const DWORD*)pDesc->pBinary, D3DXCONSTTABLE_LARGEADDRESSAWARE, &constantTable.GetRawRef())))
        return false;

    for (unsigned i = 0; i < Uniform::SU_Count; i++)
    {
        const UINT MaxDescs = 8;
        D3DXCONSTANT_DESC descs[MaxDescs];
        if (!(pDesc->Uniforms[i].Location >= 0))
            continue;

        D3DXHANDLE hconstant = constantTable->GetConstantByName(0, ShaderUniformNames[i]);        
        UINT descCount = MaxDescs;
        if ( !hconstant || FAILED( constantTable->GetConstantDesc(hconstant, descs, &descCount) ))
        {
            SF_DEBUG_MESSAGE1(1, "Shader uniform: %s expected, but not found\n", ShaderUniformNames[i]);
            return false;
        }

        SF_ASSERT((int)descs[0].RegisterIndex == pDesc->Uniforms[i].Location);
    }
#endif

    return true;
}

void VertexShader::Shutdown( )
{
    pProg = 0;
}

bool FragShader::Init(Render::HAL* phal, const FragShaderDesc* pd)
{
    if ( !phal || !pd )
        return false;
    IDirect3DDevice9* pdevice = ((D3D9::HAL*)phal)->pDevice;

    pDesc = pd;

    if ( FAILED( pdevice->CreatePixelShader((const DWORD*)pDesc->pBinary, &pProg.GetRawRef())))
        return false;

    // The following block uses D3DX9 reflection to determine if the shader inputs match the descriptors.
    // D3DX is not available in VC2011+, so just assume that they are in the correct positions. Also, only do this in debug.
#if SF_CC_MSVC < 1700 && defined(SF_BUILD_DEBUG)
    Ptr<ID3DXConstantTable> constantTable = 0;
    if ( FAILED( D3DXGetShaderConstantTableEx((const DWORD*)pDesc->pBinary, D3DXCONSTTABLE_LARGEADDRESSAWARE, &constantTable.GetRawRef())))
        return false;

    for (unsigned i = 0; i < Uniform::SU_Count; i++)
    {
        const UINT MaxDescs = 8;
        D3DXCONSTANT_DESC descs[MaxDescs];
        if (!(pDesc->Uniforms[i].Location >= 0))
            continue;

        D3DXHANDLE hconstant = constantTable->GetConstantByName(0, ShaderUniformNames[i]);
        UINT descCount = MaxDescs;
        if ( !hconstant || FAILED( constantTable->GetConstantDesc(hconstant, descs, &descCount) ))
        {
            SF_DEBUG_MESSAGE1(1, "Shader uniform: %s expected, but not found\n", ShaderUniformNames[i]);
            D3DXCONSTANTTABLE_DESC csDesc;
            if (SUCCEEDED(constantTable->GetDesc(&csDesc)))
            {
                SF_DEBUG_MESSAGE1(1, "Shader constant table (%d constants)\n", csDesc.Constants);
                for ( unsigned cs = 0; cs < csDesc.Constants; ++cs )
                {
                    D3DXCONSTANT_DESC constDesc;
                    UINT count = 1;
                    hconstant = constantTable->GetConstant(0, cs);
                    if ( FAILED(constantTable->GetConstantDesc(hconstant, &constDesc, &count )))
                        continue;
                    SF_DEBUG_MESSAGE1(1, "\tname = %s", constDesc.Name);
                }
            }
            constantTable->GetConstant(0, 0);
            return false;
        }

        SF_ASSERT((int)descs[0].RegisterIndex == pDesc->Uniforms[i].Location);
    }
#endif

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
    WORD                    Size[2], Count, Unused;
    const VertexShaderDesc* VertexDesc;


    D3DVertexDeclBuilder(const VertexShaderDesc* vdesc) : Count(0), Unused(0), VertexDesc(vdesc)
    { 
        Size[0] = Size[1] = 0; 
    }

    void    Add(int, unsigned attr, int, int)
    {
        // Find the matching vertex attribute within the vertex desc.
        int attrIndex;
        for ( attrIndex = 0; attrIndex < VertexDesc->NumAttribs; ++attrIndex )
        {
            if ( (VertexDesc->Attributes[attrIndex].Attr&(VET_Usage_Mask|VET_Index_Mask)) == (attr&(VET_Usage_Mask|VET_Index_Mask)))
                break;
        }

        // Allow missing vertex attributes
        unsigned usage  = D3DDECLUSAGE_TEXCOORD;
        BYTE usageIndex = (BYTE)(7 - Unused);
        if (attrIndex < VertexDesc->NumAttribs)
        {
            usage = VertexDesc->Attributes[attrIndex].Usage;
            usageIndex = (BYTE)VertexDesc->Attributes[attrIndex].UsageIndex;
        }
        else
            Unused++;

        unsigned type = D3DDECLTYPE_UNUSED;
        WORD byteSize = 0;
        WORD stream = 0;
        getD3DType(attr, type, byteSize, stream);

        AddD3DElement(stream, type, byteSize, usage, usageIndex );
    }


    void    AddD3DElement(WORD stream, unsigned type, WORD size,
                unsigned usage, BYTE usageIndex = 0)
    {
        SF_ASSERT(stream< MaxElements);

        D3DVERTEXELEMENT9 &e = Elements[Count++];
        e.Stream    = stream;
        e.Offset    = Size[stream];
        e.Type      = (BYTE)type;
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

    void getD3DType( unsigned attr, unsigned &type, WORD &byteSize, WORD& stream) 
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
            stream = 1; // fallthough.
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
    VertexShaderDesc::ShaderIndex vsIndex = VertexShaderDesc::GetShaderIndex(shader, pHal->SManager.ShaderModel);
    FragShaderDesc::ShaderIndex psIndex = FragShaderDesc::GetShaderIndex(shader, pHal->SManager.ShaderModel);

    CurShaders.pVFormat = pformat;
    CurShaders.pVS      = &pHal->SManager.StaticVShaders[vsIndex];
    CurShaders.pVDesc   = CurShaders.pVS->pDesc;
    CurShaders.pFS      = &pHal->SManager.StaticFShaders[psIndex];
    CurShaders.pFDesc   = CurShaders.pFS->pDesc;

    if ( pformat && !pformat->pSysFormat )
    {
        (const_cast<VertexFormat*>(pformat))->pSysFormat = *SF_NEW SysVertexFormat(pHal->GetDevice(), 
            pformat, CurShaders.pVS->pDesc);
    }

    return (bool)CurShaders;
}

void ShaderInterface::SetTexture(Shader, unsigned var, Render::Texture* ptexture, ImageFillMode fm, unsigned index)
{
    SF_ASSERT(CurShaders.pFDesc->Uniforms[var].Location >= 0 );
    SF_ASSERT(CurShaders.pFDesc->Uniforms[var].Location + CurShaders.pFDesc->Uniforms[var].Size >= 
        (short)(index + ptexture->TextureCount));
    ptexture->ApplyTexture(CurShaders.pFDesc->Uniforms[var].Location + index, fm);
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
        Max = Alg::Max(Max, (location + size + 3)/4); // 4 elements per register.
        return true;
    }
    int Min, Max, ShadowLocation;
    float* UniformData;
    IDirect3DDevice9* pDevice;
};

void ShaderInterface::Finish(unsigned meshCount)
{
    ShaderInterfaceBase::Finish(meshCount);

    IDirect3DDevice9* pdevice = pHal->GetDevice();
    ShaderConstantRange shaderConstantRangeFS(pdevice, UniformData);
    ShaderConstantRange shaderConstantRangeVS(pdevice, UniformData);
    for (int i = 0; i < Uniform::SU_Count; i++)
    {
        if (UniformSet[i])
        {
            if (CurShaders.pFDesc->Uniforms[i].Location >= 0)
            {
                SF_ASSERT(CurShaders.pFDesc->Uniforms[i].Location >= 0);
                shaderConstantRangeFS.Update( CurShaders.pFDesc->Uniforms[i].Location,
                                               CurShaders.pFDesc->Uniforms[i].Size,
                                               CurShaders.pFDesc->Uniforms[i].ShadowOffset,
                                               true );
            }
            else if (CurShaders->pVDesc->Uniforms[i].Location >= 0 )
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
    return InstancingSupport;
}

bool ShaderManager::HasDynamicLoopingSupport() const
{
    return DynamicLoopingSupport;
}

bool ShaderManager::Initialize(HAL* phal)
{
    pDevice = phal->GetDevice();
    D3DCAPSx caps;
    pDevice->GetDeviceCaps(&caps);

    // Check the overriding of shader model to 2.0
    DWORD shaderVersion = caps.PixelShaderVersion;
    if (phal->GetConfigFlags() & HALConfig_ShaderModel20)
        shaderVersion = D3DPS_VERSION(2,0);

    // Detect which version of shader to use (3.0 if available, otherwise 2.0)
    if (shaderVersion >= D3DPS_VERSION(3,0))
        ShaderModel = ShaderDesc::ShaderVersion_D3D9SM30;
    else if (shaderVersion >= D3DPS_VERSION(2,0))
        ShaderModel = ShaderDesc::ShaderVersion_D3D9SM20;
	else
	{
        SF_DEBUG_MESSAGE(1, "ShaderModel 2.0+ is required for D3D9. Failing.");
		return false;
	}

    if (ShaderModel == ShaderDesc::ShaderVersion_D3D9SM30 && !ShaderDesc::IsShaderVersionSupported(ShaderModel))
    {
        SF_DEBUG_MESSAGE(1, "Support for ShaderModel 3.0 was not included when running GFxShaderMaker. Trying ShaderModel 2.0.");
        ShaderModel = ShaderDesc::ShaderVersion_D3D9SM20;
    }
    if (ShaderModel == ShaderDesc::ShaderVersion_D3D9SM20 && !ShaderDesc::IsShaderVersionSupported(ShaderModel))
    {
        SF_DEBUG_MESSAGE(1, "Support for ShaderModel 2.0 was not included when running GFxShaderMaker. Failing.");
        return false;
    }

    // If we are running ShaderModel 3.0, we can have instancing and dynamic loops.
    if (ShaderModel == ShaderDesc::ShaderVersion_D3D9SM30)
    {
        InstancingSupport = true;
        DynamicLoopingSupport = true;
    }

    // Now initialize the shaders
    for (unsigned i = 0; i < VertexShaderDesc::VSI_Count; i++)
    {
        const VertexShaderDesc* desc = VertexShaderDesc::Descs[i];
        if ( desc && desc->Version == ShaderModel && !StaticVShaders[i].Init(phal, desc) )
        {
            SF_DEBUG_WARNING1(1, "VertexShader (index=%d) failed to initialize.\n", i);
            return false;
        }
    }

    for (unsigned i = 0; i < FragShaderDesc::FSI_Count; i++)
    {
        const FragShaderDesc* desc = FragShaderDesc::Descs[i];
        if ( desc && desc->Version == ShaderModel && !StaticFShaders[i].Init(phal, desc) )
        {
            SF_DEBUG_WARNING1(1, "FragShader (index=%d) failed to initialize.\n", i);
            return false;
        }
    }

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

    for (unsigned i = 0; i < VertexShaderDesc::VSI_Count; i++)
        StaticVShaders[i].Shutdown();

    for (unsigned i = 0; i < FragShaderDesc::FSI_Count; i++)
        StaticFShaders[i].Shutdown();

    VertexFormatComputedHash.Clear();
    VFormats.Clear();
    pDevice = 0;
}

}}}
