/**********************************************************************

PublicHeader:   Render
Filename    :   Orbis_Shader.cpp
Content     :   
Created     :   2012/09/20
Authors     :   Bart Muzzin

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#include "Render/PS4/PS4_Shader.h"
#include "Render/PS4/PS4_HAL.h"
#include "Kernel/SF_Debug.h"

namespace Scaleform { namespace Render { namespace PS4 {

extern const char* ShaderUniformNames[Uniform::SU_Count];

template<class ShaderDesc, sce::Gnmx::ShaderType ShaderType, sce::Gnm::ShaderStage ShaderStage>
BaseShader<ShaderDesc, ShaderType, ShaderStage>::BaseShader() : pDesc(0), ShaderGPUAddress(0), IsPatched(false)
{
#ifdef SF_PS4_USE_LCUE
    memset(&SROTable, 0, sizeof(SROTable));
#endif
}

template<class ShaderDesc, sce::Gnmx::ShaderType ShaderType, sce::Gnm::ShaderStage ShaderStage>
bool BaseShader<ShaderDesc, ShaderType, ShaderStage>::Init(ShaderInfoHashType& shaderInfoHash, Render::MemoryManager& mgr, sceGnmxContextType& context, const ShaderDesc* pd)
{
    pDesc = pd;

    // Attempt to find the ShaderInfo in our hash.
    if (shaderInfoHash.Get(pd->pBinary, &ShaderInfo))
    {
        IsPatched = true;

#ifdef SF_PS4_USE_LCUE
        // Still must create the ShaderOffsetResource structure from the shader code.
        sceGnmxGenerateSROTable(&SROTable, ShaderStage, ShaderInfo.m_shaderStruct, ShaderInfo.m_gpuShaderCode, ShaderInfo.m_gpuShaderCodeSize, false, false);
#endif

        return true;
    }

    // Now copy it to GPU accessible memory.
    sce::Gnmx::parseShader(&ShaderInfo, pd->pBinary);

#ifdef SF_PS4_USE_LCUE
    // Create the ShaderOffsetResource structure from the shader code before patching.
    sceGnmxGenerateSROTable(&SROTable, ShaderStage, ShaderInfo.m_shaderStruct, ShaderInfo.m_gpuShaderCode, ShaderInfo.m_gpuShaderCodeSize, false, false);
#endif

    ShaderGPUAddress = mgr.Alloc(ShaderInfo.m_gpuShaderCodeSize, sce::Gnm::kAlignmentOfShaderInBytes, Memory_Orbis_UC_GARLIC_NONVOLATILE);
    memcpy(ShaderGPUAddress, ShaderInfo.m_gpuShaderCode, ShaderInfo.m_gpuShaderCodeSize );
    
    // Put the shader in the hash, so we don't reload and reinitialize it.
    shaderInfoHash.Add(pd->pBinary, ShaderInfo);
    return true;
}

template<class ShaderDesc, sce::Gnmx::ShaderType ShaderType, sce::Gnm::ShaderStage ShaderStage>
void BaseShader<ShaderDesc, ShaderType, ShaderStage>::Shutdown(ShaderInfoHashType& shaderInfoHash, Render::MemoryManager& mgr)
{
    pDesc            = 0;
    ShaderGPUAddress = 0;
    IsPatched        = false;
}

VertexShader::VertexShader() : 
    BaseShader(),
    FetchShaderGPUAddress(0),
    FetchShaderSize(0)
{

}

bool VertexShader::Init(ShaderInfoHashType& shaderInfoHash, Render::MemoryManager& mgr, sceGnmxContextType& context, const VertexShaderDesc* pd)
{
    if (!BaseShader::Init(shaderInfoHash, mgr, context, pd))
        return false;

    // Check if the shader has already been initialized and patched.
    if (!IsPatched)
    {
        ShaderInfo.m_vsShader->patchShaderGpuAddress(ShaderGPUAddress);
        IsPatched = true;
    }

    // Create the fetch shader.
    FetchShaderSize = sce::Gnmx::computeVsFetchShaderSize(ShaderInfo.m_vsShader);
    FetchShaderGPUAddress = reinterpret_cast<uint32_t*>(mgr.Alloc(FetchShaderSize, sce::Gnm::kAlignmentOfFetchShaderInBytes, Memory_Orbis_UC_GARLIC_NONVOLATILE));

    // NOTE: we do not pass any instancing parameters, even if it is an instanced shader. We are currently
    // inserting instancing data in the command buffer, and indexing it via S_INSTANCE_ID (same method used
    // on most other platforms).
    uint32_t shaderModifier;
    sce::Gnmx::generateVsFetchShader(FetchShaderGPUAddress, &shaderModifier, ShaderInfo.m_vsShader, 0); 
    ShaderInfo.m_vsShader->applyFetchShaderModifier(shaderModifier);

    sce::Gnmx::generateVsFetchShader(FetchShaderGPUAddress, &ShaderModifier, ShaderInfo.m_vsShader, 0); 
    return true;
}

void VertexShader::Shutdown(ShaderInfoHashType& shaderInfoHash, Render::MemoryManager& mgr)
{
    if (FetchShaderGPUAddress && pDesc)
    {
        if (shaderInfoHash.Get(pDesc->pBinary))
        {
            // Remove it, so if some other shader that uses the same binary calls Shutdown, it won't be done twice.
            mgr.Free(reinterpret_cast<void*>(FetchShaderGPUAddress), FetchShaderSize);
        }
        FetchShaderGPUAddress = 0;
        FetchShaderSize = 0;
    }

    // Now call the base shutdown (must come second, because it will remove this object from the hash).
    // See the shader is still in the hash. If it is, that means it hasn't been shutdown yet.
    BaseShader::Shutdown(shaderInfoHash, mgr);
}

bool FragShader::Init(ShaderInfoHashType& shaderInfoHash, Render::MemoryManager& mgr, sceGnmxContextType& context, const FragShaderDesc* pd)
{
    if (!BaseShader::Init(shaderInfoHash, mgr, context, pd))
        return false;

    // Check if the shader has already been initialized and patched.
    if (IsPatched)
        return true;

    ShaderInfo.m_psShader->patchShaderGpuAddress(ShaderGPUAddress);
    IsPatched = true;
    return true;
}

bool ShaderInterface::SetStaticShader(ShaderDesc::ShaderType shader, const VertexFormat* pformat)
{
    CurShaders.pVFormat = pformat;
    CurShaders.pVS      = &pHal->SManager.StaticVShaders[VertexShaderDesc::GetShaderIndex(shader)];
    CurShaders.pVDesc   = CurShaders.pVS->pDesc;
    CurShaders.pFS      = &pHal->SManager.StaticFShaders[FragShaderDesc::GetShaderIndex(shader)];
    CurShaders.pFDesc   = CurShaders.pFS->pDesc;

    // required?
    //if ( pformat && !pformat->pSysFormat )
    //    (const_cast<VertexFormat*>(pformat))->pSysFormat = *SF_NEW SysVertexFormat(pHal->GetDevice(), pformat, CurShaders.pVS->pDesc);

    return (bool)CurShaders;
}

void ShaderInterface::SetTexture(Shader, unsigned var, Render::Texture* ptexture, ImageFillMode fm, unsigned index)
{
    SF_ASSERT(CurShaders.pFDesc->Uniforms[var].Location >= 0 );
    SF_ASSERT(CurShaders.pFDesc->Uniforms[var].Location + CurShaders.pFDesc->Uniforms[var].Size >= 
        (short)(index + ptexture->TextureCount));

    // Save the texture, and apply it in ShaderInterface::Finish.
    TextureEntries[CurShaders.pFDesc->Uniforms[var].Location + index] = {ptexture, fm};
}

// Record the min/max shader constant registers, and just set the entire buffer in one call,
// instead of doing them individually. This could result in some uninitialized data to be sent,
// but it should be unreferenced by the shaders, and should be more efficient.
struct ShaderConstantRange
{
    ShaderConstantRange(float * data, sce::Gnm::ShaderStage shaderType) : Min(SF_MAX_SINT), Max(-1), ShadowLocation(0), UniformData(data), ShaderType(shaderType) { };
    bool Update( int location, int size, int shadowLocation )
    {
        location *= 4; // 4 elements per register.
        if ( (shadowLocation - location) != ShadowLocation && Max >= 0 )
        {
            // This shouldn't be the case, but if the storage is disjoint for some reason
            // we will need to upload the constants in separate steps.
            SF_DEBUG_ASSERT(0, "Unexpected non-contiguous shader constant data.");
            return true;
        }
        ShadowLocation = shadowLocation - location;
        Min = Alg::Min(Min, location/4);
        Max = Alg::Max(Max, (location + size + 3)/4); // 4 elements per register.
        return true;
    }
    unsigned GetSizeInBytes() const
    {
        return (Max-Min) * 4 * sizeof(float); // 4 elements per register
    }

#ifdef SF_PS4_USE_LCUE
    void Apply(sceGnmxContextType& gnmxctx)
    {
        unsigned sizeInBytes = GetSizeInBytes();

        // If there were no constants set, don't do anything. This might be valid, all data could come through varyings.
        if (sizeInBytes == 0)
            return;
        void * constantBuffer = gnmxctx.embedData(UniformData + ShadowLocation, sizeInBytes / sizeof(uint32_t), sce::Gnm::kEmbeddedDataAlignment4);
        sce::Gnm::Buffer buffer;
        buffer.initAsConstantBuffer(constantBuffer, sizeInBytes);
        gnmxctx.setConstantBuffers(ShaderType, 0, 1, &buffer);
    }
#else
	void Apply(sce::Gnmx::GfxContext& gnmxctx)
	{
		// Check to see if any constants at all were set.
		if (Max <= 0)
			return;

		unsigned sizeInBytes = GetSizeInBytes();
		SF_DEBUG_ASSERT(sizeInBytes < 64*1024, "Error, size in bytes should not be larger than 64k (packet size)");
		void * constantBuffer = gnmxctx.allocateFromCommandBuffer(sizeInBytes, sce::Gnm::kEmbeddedDataAlignment4);
		SF_DEBUG_ASSERT(constantBuffer, "Error allocating from GfxContext buffer");
		memcpy(constantBuffer, UniformData + ShadowLocation, sizeInBytes);

		sce::Gnm::Buffer buffer;
		buffer.initAsConstantBuffer(constantBuffer, sizeInBytes);
		gnmxctx.setConstantBuffers(ShaderType, 0, 1, &buffer);
	}
#endif

private:
    int Min, Max, ShadowLocation;
    float* UniformData;
    sce::Gnm::ShaderStage ShaderType;
};

void ShaderInterface::BeginPrimitive()
{
    ShaderInterfaceBase::BeginPrimitive();
    memset(TextureEntries, 0, sizeof(TextureEntries));
}

void ShaderInterface::Finish(unsigned meshCount)
{
    // Must apply shaders first, for LCUE implementation.
    finishApplyShaders(*pHal->GnmxCtx);

    ShaderInterfaceBase::Finish(meshCount);

    ShaderConstantRange shaderConstantRangeFS(UniformData, sce::Gnm::kShaderStagePs);
    ShaderConstantRange shaderConstantRangeVS(UniformData, sce::Gnm::kShaderStageVs);

    for (int i = 0; i < Uniform::SU_Count; i++)
    {
        if (UniformSet[i])
        {
            if (CurShaders.pFDesc->Uniforms[i].Location >= 0)
            {
                shaderConstantRangeFS.Update( CurShaders.pFDesc->Uniforms[i].Location,
                    CurShaders.pFDesc->Uniforms[i].Size,
                    CurShaders.pFDesc->Uniforms[i].ShadowOffset);
            }
            else if (CurShaders->pVDesc->Uniforms[i].Location >= 0 )
            {
                shaderConstantRangeVS.Update( CurShaders.pVDesc->Uniforms[i].Location,
                    CurShaders.pVDesc->Uniforms[i].Size,
                    CurShaders.pVDesc->Uniforms[i].ShadowOffset);
            }
        }
    }

    shaderConstantRangeVS.Apply(*pHal->GnmxCtx);
    shaderConstantRangeFS.Apply(*pHal->GnmxCtx);

    // Apply textures resources.
    for (unsigned texture = 0; texture < sizeof(Textures)/sizeof(Render::Texture*); ++texture)
    {
        if (TextureEntries[texture].pTexture)
        {
            PS4::Texture *ps4Texture= reinterpret_cast< PS4::Texture*>(TextureEntries[texture].pTexture);
            ps4Texture->ApplyTexture(texture, TextureEntries[texture].FillMode);
        }
    }



    memset(UniformSet, 0, Uniform::SU_Count);
}

#ifndef SF_PS4_USE_LCUE
void ShaderInterface::finishApplyShaders(sce::Gnmx::GfxContext& gnmxctx)
{
    // With GfxContext, we can do redundancy checking.
    if (pLastVS != CurShaders.pVS->ShaderInfo.m_vsShader)
    {
        gnmxctx.setVsShader(CurShaders.pVS->ShaderInfo.m_vsShader, CurShaders->pVS->ShaderModifier, CurShaders->pVS->FetchShaderGPUAddress);
        pLastVS = CurShaders.pVS->ShaderInfo.m_vsShader;
    }

    if (pLastFS != CurShaders.pFS->ShaderInfo.m_psShader)
    {
        gnmxctx.setPsShader(CurShaders->pFS->ShaderInfo.m_psShader);
        pLastFS = CurShaders.pFS->ShaderInfo.m_psShader;
    }
}
#else
void ShaderInterface::finishApplyShaders(sceGnmxContextType& gnmxctx)
{
    // With LCUE, we must always set the shaders.
	gnmxctx.setBoundShaderResourceOffsets( sce::Gnm::kShaderStageVs, &CurShaders->pVS->SROTable );
    gnmxctx.setVsShader(CurShaders.pVS->ShaderInfo.m_vsShader, CurShaders->pVS->ShaderModifier, CurShaders->pVS->FetchShaderGPUAddress );

	gnmxctx.setBoundShaderResourceOffsets( sce::Gnm::kShaderStagePs, &CurShaders->pFS->SROTable );
    gnmxctx.setPsShader(CurShaders->pFS->ShaderInfo.m_psShader);
}
#endif

void ShaderInterface::BeginScene()
{
    // Disable compute, domain, geometry and hull pipeline stages.
    pHal->GnmxCtx->setActiveShaderStages(sce::Gnm::kActiveShaderStagesVsPs);
    pHal->GnmxCtx->setShaderType(sce::Gnm::kShaderTypeGraphics);

    pLastVS = 0;
    pLastFS = 0;

    // TODOBM: can vertex buffer objects be cached somehow?
    //pLastDecl = 0;
}

// *** ShaderManager
ShaderManager::ShaderManager( ProfileViews* prof ) : 
    StaticShaderManager(prof), GnmxCtx(0)
{
    memset(StaticVShaders, 0, sizeof(StaticVShaders));
    memset(StaticFShaders, 0, sizeof(StaticFShaders));
}

bool ShaderManager::HasInstancingSupport() const
{
    // TODOBM: Instancing was broken with the SDK 0.930 update. Disabling it until it is fixed.
    return false;
    //return true;

}

void ShaderManager::MapVertexFormat(PrimitiveFillType fill, const VertexFormat* sourceFormat,
                                    const VertexFormat** single, const VertexFormat** batch, 
                                    const VertexFormat** instanced)
{
    // PS4's shader input types need to match their input layouts. So, just convert all position data
    // to float (instead of using short). Extra shaders could be generated to support both, however, that
    // would require double the number of vertex shaders.
    VertexElement floatPositionElements[8];
    VertexFormat floatPositionFormat;
    floatPositionFormat.pElements = floatPositionElements;
    floatPositionFormat.pSysFormat = 0;
    unsigned offset = 0;
    unsigned i;
    for ( i = 0; sourceFormat->pElements[i].Attribute != VET_None; ++i )
    {
        floatPositionElements[i].Attribute = sourceFormat->pElements[i].Attribute;
        if ( (floatPositionElements[i].Attribute&VET_Usage_Mask) == VET_Pos)
        {
            floatPositionElements[i].Attribute &= ~VET_CompType_Mask;
            floatPositionElements[i].Attribute |= VET_F32;
            floatPositionElements[i].Offset = sourceFormat->pElements[i].Offset + offset;
            offset += (floatPositionElements[i].CompSize() - sourceFormat->pElements[i].CompSize()) * (floatPositionElements[i].Attribute & VET_Components_Mask);
        }
        else
        {
            floatPositionElements[i].Offset = sourceFormat->pElements[i].Offset + offset;
        }
    }
    floatPositionElements[i].Attribute = VET_None;
    floatPositionElements[i].Offset    = 0;

    // Now let the base class actually do the mapping.
    StaticShaderManager::MapVertexFormat(fill, &floatPositionFormat, single, batch, instanced, 
        (HasInstancingSupport() ? MVF_HasInstancing : 0) | MVF_Align);
}

bool ShaderManager::Initialize(HAL* phal, Render::MemoryManager& pmemMgr)
{
    // Now, initialize all the shaders
    for (unsigned i = 0; i < VertexShaderDesc::VSI_Count; i++)
    {
        const VertexShaderDesc* desc = VertexShaderDesc::Descs[i];
        if ( desc && desc->pBinary)
        {
            if (!StaticVShaders[i].Init(ShaderInfoHash[ShaderStage_Vertex], pmemMgr, *GnmxCtx, desc))
                return false;
        }
    }

    for (unsigned i = 0; i < FragShaderDesc::FSI_Count; i++)
    {
        const FragShaderDesc* desc = FragShaderDesc::Descs[i];
        if (desc && desc->pBinary)
        {
            if ( !StaticFShaders[i].Init(ShaderInfoHash[ShaderStage_Frag], pmemMgr, *GnmxCtx, desc) )
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

void ShaderManager::Reset(Render::MemoryManager& memMgr)
{
    for (unsigned i = 0; i < VertexShaderDesc::VSI_Count; i++)
    {
        const VertexShaderDesc* desc = VertexShaderDesc::Descs[i];
        if ( desc && desc->pBinary)
            StaticVShaders[i].Shutdown(ShaderInfoHash[ShaderStage_Vertex], memMgr);
    }

    for (unsigned i = 0; i < FragShaderDesc::FSI_Count; i++)
    {
        const FragShaderDesc* desc = FragShaderDesc::Descs[i];
        if (desc && desc->pBinary)
            StaticFShaders[i].Shutdown(ShaderInfoHash[ShaderStage_Frag], memMgr);
    }

    GnmxCtx = 0;
}

}}}
