/**************************************************************************

Filename    :   Vulkan_Shader.cpp
Content     :   Vulkan shader implementation.
Created     :   2026
Authors     :   Scaleform Vulkan Backend

**************************************************************************/

#include "Render/Vulkan/Vulkan_Shader.h"
#include "Render/Vulkan/Vulkan_HAL.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render { namespace Vulkan {

extern const char* ShaderUniformNames[Uniform::SU_Count];

// *** VertexShader

bool VertexShader::Init(VkDevice device, const VertexShaderDesc* pd)
{
    pDesc = pd;
    pDevice = device;

    VkShaderModuleCreateInfo ci = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
    ci.codeSize = pDesc->BinarySize;
    ci.pCode    = (const uint32_t*)pDesc->pBinary;

    VkResult r = vkCreateShaderModule(device, &ci, nullptr, &pProg);
    if (r != VK_SUCCESS)
    {
        SF_DEBUG_WARNING1(1, "Vulkan: Failed to create vertex shader module, VkResult=%d", (int)r);
        return false;
    }
    return true;
}

void VertexShader::Shutdown()
{
    if (pProg != VK_NULL_HANDLE && pDevice != VK_NULL_HANDLE)
        vkDestroyShaderModule(pDevice, pProg, nullptr);
    pProg = VK_NULL_HANDLE;
    pDevice = VK_NULL_HANDLE;
}

// *** FragShader

bool FragShader::Init(VkDevice device, const FragShaderDesc* pd)
{
    pDesc = pd;
    pDevice = device;

    VkShaderModuleCreateInfo ci = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
    ci.codeSize = pDesc->BinarySize;
    ci.pCode    = (const uint32_t*)pDesc->pBinary;

    VkResult r = vkCreateShaderModule(device, &ci, nullptr, &pProg);
    if (r != VK_SUCCESS)
    {
        SF_DEBUG_WARNING1(1, "Vulkan: Failed to create fragment shader module, VkResult=%d", (int)r);
        return false;
    }
    return true;
}

void FragShader::Shutdown()
{
    if (pProg != VK_NULL_HANDLE && pDevice != VK_NULL_HANDLE)
        vkDestroyShaderModule(pDevice, pProg, nullptr);
    pProg = VK_NULL_HANDLE;
    pDevice = VK_NULL_HANDLE;
}


// *** SysVertexFormat

static VkFormat VetToVkFormat(unsigned attr)
{
    unsigned comp = (attr & VET_CompType_Mask);
    unsigned count = (attr & VET_Components_Mask);
    if (count == 0) count = 1;

    switch (comp)
    {
    case VET_F32:
        switch (count) {
        case 1: return VK_FORMAT_R32_SFLOAT;
        case 2: return VK_FORMAT_R32G32_SFLOAT;
        case 3: return VK_FORMAT_R32G32B32_SFLOAT;
        case 4: return VK_FORMAT_R32G32B32A32_SFLOAT;
        }
        break;
    case VET_S16:
        switch (count) {
        case 2: return VK_FORMAT_R16G16_SSCALED;
        case 4: return VK_FORMAT_R16G16B16A16_SSCALED;
        }
        break;
    case VET_U8:
        switch (count) {
        case 1: return VK_FORMAT_R8_UINT;
        case 2: return VK_FORMAT_R8G8_UINT;
        case 4: return VK_FORMAT_R8G8B8A8_UINT;
        }
        break;
    case VET_U8N:
        switch (count) {
        case 1: return VK_FORMAT_R8_UNORM;
        case 2: return VK_FORMAT_R8G8_UNORM;
        case 4: return VK_FORMAT_R8G8B8A8_UNORM;
        }
        break;
    case VET_I8:
        switch (count) {
        case 1: return VK_FORMAT_R8_SSCALED;
        case 2: return VK_FORMAT_R8G8_SSCALED;
        case 4: return VK_FORMAT_R8G8B8A8_SSCALED;
        }
        break;
    }
    return VK_FORMAT_R32G32B32A32_SFLOAT;
}

SysVertexFormat::SysVertexFormat(const VertexFormat* vf, const VertexShaderDesc* pvdesc)
: AttributeCount(0)
{
    memset(&BindingDesc, 0, sizeof(BindingDesc));
    memset(Attributes, 0, sizeof(Attributes));

    BindingDesc.binding   = 0;
    BindingDesc.stride    = vf->Size;
    BindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    const VertexShaderDesc& vdesc = *pvdesc;

    for (int attr = 0; attr < vdesc.NumAttribs; attr++)
    {
        VkVertexInputAttributeDescription& a = Attributes[AttributeCount++];
        a.location = attr;
        a.binding  = 0;

        unsigned vetAttr = vdesc.Attributes[attr].Attr;
        unsigned shaderUsage = vetAttr & VET_Usage_Mask;
        unsigned shaderIndex = vetAttr & VET_Index_Mask;
        unsigned shaderCount = vetAttr & VET_Components_Mask;

        // CalcFormat splits the 4-component factor color (VET_Color, index 1) into
        // separate 1-byte elements: VET_T0Weight8 (index 2) at start, then batch
        // index, then VET_FactorAlpha8 (index 1). The shader reads all 4 bytes as
        // vec4 (using .b for batch index, .a for alpha, .r for weight).
        // Map the full 4-byte block starting at T0Weight8's offset.
        if (shaderUsage == VET_Color && shaderIndex == (1 << VET_Index_Shift) && shaderCount == 4)
        {
            const VertexElement* veWeight = vf->GetElement(
                VET_Color | (2 << VET_Index_Shift), VET_Usage_Mask | VET_Index_Mask);
            if (veWeight)
            {
                a.format = VK_FORMAT_R8G8B8A8_UNORM;
                a.offset = veWeight->Offset;
                continue;
            }
        }

        // Match shader attribute to actual VertexFormat element by usage+index,
        // same approach as StaticShaderManager::CalcFormat in Render_Shader.h.
        const VertexElement* ve = vf->GetElement(
            vetAttr & (VET_Usage_Mask | VET_Index_Mask),
            VET_Usage_Mask | VET_Index_Mask);

        if (ve)
        {
            a.format = VetToVkFormat(ve->Attribute);
            a.offset = ve->Offset;
        }
        else
        {
            a.format = VetToVkFormat(vetAttr);
            a.offset = 0;
        }
    }

}


// *** ShaderInterface

ShaderInterface::ShaderInterface(Render::HAL* phal)
: pHal((HAL*)phal), pLastVS(VK_NULL_HANDLE), pLastFS(VK_NULL_HANDLE)
{
}

void ShaderInterface::BeginScene()
{
    pLastVS  = VK_NULL_HANDLE;
    pLastFS  = VK_NULL_HANDLE;
}

bool ShaderInterface::SetStaticShader(ShaderDesc::ShaderType shader, const VertexFormat* pformat)
{
    CurShaders.pVFormat = pformat;
    unsigned vsIndex = VertexShaderDesc::GetShaderIndex(shader, pHal->SManager.ShaderModel);
    unsigned fsIndex = FragShaderDesc::GetShaderIndex(shader, pHal->SManager.ShaderModel);
    CurShaders.pVS    = &pHal->SManager.StaticVShaders[vsIndex];
    CurShaders.pVDesc = CurShaders.pVS->pDesc;
    CurShaders.pFS    = &pHal->SManager.StaticFShaders[fsIndex];
    CurShaders.pFDesc = CurShaders.pFS->pDesc;

    if (pformat)
    {
        // Cache SysVertexFormat per (VertexFormat*, vsIndex) pair.  Different shaders
        // assign different attribute locations (e.g. pos@loc0 vs pos@loc1), so a single
        // cached SysVertexFormat per VertexFormat is not safe.
        UPInt cacheKey = ((UPInt)pformat * 2654435761u) ^ ((UPInt)vsIndex * 2246822519u);
        Ptr<SysVertexFormat>* ppCached = pHal->SManager.SysVFCache.Get(cacheKey);
        if (ppCached)
        {
            (const_cast<VertexFormat*>(pformat))->pSysFormat = *ppCached;
        }
        else
        {
            Ptr<SysVertexFormat> svf = *SF_NEW SysVertexFormat(pformat, CurShaders.pVS->pDesc);
            pHal->SManager.SysVFCache.Set(cacheKey, svf);
            (const_cast<VertexFormat*>(pformat))->pSysFormat = svf;
        }
    }

    return (bool)CurShaders;
}

void ShaderInterface::SetTexture(Shader, unsigned var, Render::Texture* ptexture, ImageFillMode fm, unsigned index)
{
    Vulkan::Texture* pvkTexture = (Vulkan::Texture*)ptexture;
    SF_ASSERT(CurShaders.pFDesc->Uniforms[var].Location >= 0);
    pvkTexture->ApplyTexture(CurShaders.pFDesc->Uniforms[var].Location + index, fm);
}

void ShaderInterface::Finish(unsigned meshCount)
{
    SF_UNUSED(meshCount);

    HAL* hal = pHal;

    // Copy vertex uniform data to UBO ring
    UPInt vsSize = Uniform::SU_VertexSize * sizeof(float);
    UPInt fsSize = Uniform::SU_FragSize * sizeof(float);

    // Align vsSize so the fragment UBO offset also meets minUniformBufferOffsetAlignment
    UPInt vsAligned = (vsSize + 255) & ~(UPInt)255;
    UPInt totalSize = vsAligned + fsSize;

    UPInt uboOffset = hal->allocUBOSpace(totalSize);
    UByte* pDst = hal->pUBORingData + uboOffset;

    // Copy vertex uniforms using locations from the shader descriptors
    for (int i = 0; i < Uniform::SU_Count; i++)
    {
        if (UniformSet[i])
        {
            if (CurShaders.pVDesc->Uniforms[i].Location >= 0)
            {
                int dstOff = CurShaders.pVDesc->Uniforms[i].Location * 16;
                int srcOff = CurShaders.pVDesc->Uniforms[i].ShadowOffset;
                int sz     = CurShaders.pVDesc->Uniforms[i].Size;
                memcpy(pDst + dstOff, (float*)UniformData + srcOff, sz * sizeof(float));
            }
        }
    }

    // Copy fragment uniforms after vertex uniforms (at aligned offset)
    UByte* pFsDst = pDst + vsAligned;
    for (int i = 0; i < Uniform::SU_Count; i++)
    {
        if (UniformSet[i])
        {
            if (CurShaders.pFDesc->Uniforms[i].Location >= 0)
            {
                int dstOff = CurShaders.pFDesc->Uniforms[i].Location * 16;
                int srcOff = CurShaders.pFDesc->Uniforms[i].ShadowOffset;
                int sz     = CurShaders.pFDesc->Uniforms[i].Size;
                memcpy(pFsDst + dstOff, (float*)UniformData + srcOff, sz * sizeof(float));
            }
        }
    }

    memset(UniformSet, 0, sizeof(UniformSet));

    // Allocate descriptor set from per-frame pool
    VkDescriptorSet descSet;
    VkDescriptorSetAllocateInfo dsAI = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    dsAI.descriptorPool     = hal->FrameDescriptorPools[hal->CurrentFrame % SF_VK_MAX_FRAMES_IN_FLIGHT];
    dsAI.descriptorSetCount = 1;
    dsAI.pSetLayouts        = &hal->pDescriptorSetLayout;

    VkResult r = vkAllocateDescriptorSets(hal->pDevice, &dsAI, &descSet);
    if (r != VK_SUCCESS)
    {
        SF_DEBUG_WARNING(1, "Vulkan: Failed to allocate descriptor set");
        return;
    }

    // Write UBO descriptor
    VkDescriptorBufferInfo bufInfo[2] = {};
    bufInfo[0].buffer = hal->UBORingBuffer;
    bufInfo[0].offset = uboOffset;
    bufInfo[0].range  = vsSize;
    bufInfo[1].buffer = hal->UBORingBuffer;
    bufInfo[1].offset = uboOffset + vsAligned;
    bufInfo[1].range  = fsSize;

    // Write texture descriptors
    VkDescriptorImageInfo imageInfos[SF_VK_MAX_TEXTURE_STAGES] = {};
    unsigned texWriteCount = 0;

    TextureManager* pmgr = (TextureManager*)hal->GetTextureManager();
    VkImageView dummyView    = pmgr->GetDummyImageView();
    VkSampler   dummySampler = pmgr->GetDummySampler();
    for (unsigned s = 0; s < SF_VK_MAX_TEXTURE_STAGES; s++)
    {
        if (pmgr->CurrentTextures[s] != VK_NULL_HANDLE)
        {
            imageInfos[s].imageView   = pmgr->CurrentTextures[s];
            imageInfos[s].sampler     = pmgr->CurrentSamplers[s];
            imageInfos[s].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            texWriteCount = s + 1;
        }
        else
        {
            imageInfos[s].imageView   = dummyView;
            imageInfos[s].sampler     = dummySampler;
            imageInfos[s].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
    }

    // 2 UBOs + 1 array write at binding 2 + (MAX_STAGES-1) individual writes at bindings 3+
    VkWriteDescriptorSet writes[3 + SF_VK_MAX_TEXTURE_STAGES] = {};
    unsigned writeCount = 0;

    // Vertex UBO
    writes[writeCount] = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
    writes[writeCount].dstSet          = descSet;
    writes[writeCount].dstBinding      = 0;
    writes[writeCount].descriptorCount = 1;
    writes[writeCount].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writes[writeCount].pBufferInfo     = &bufInfo[0];
    writeCount++;

    // Fragment UBO
    writes[writeCount] = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
    writes[writeCount].dstSet          = descSet;
    writes[writeCount].dstBinding      = 1;
    writes[writeCount].descriptorCount = 1;
    writes[writeCount].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writes[writeCount].pBufferInfo     = &bufInfo[1];
    writeCount++;

    // Binding 2: array of all texture stages (for array-style shaders like TexTGTexTG, YUV)
    writes[writeCount] = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
    writes[writeCount].dstSet          = descSet;
    writes[writeCount].dstBinding      = 2;
    writes[writeCount].descriptorCount = SF_VK_MAX_TEXTURE_STAGES;
    writes[writeCount].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[writeCount].pImageInfo      = imageInfos;
    writeCount++;

    // Bindings 3+: individual texture stages (for individual-binding-style filter shaders)
    for (unsigned s = 1; s < SF_VK_MAX_TEXTURE_STAGES; s++)
    {
        writes[writeCount] = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
        writes[writeCount].dstSet          = descSet;
        writes[writeCount].dstBinding      = 2 + s;
        writes[writeCount].descriptorCount = 1;
        writes[writeCount].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writes[writeCount].pImageInfo      = &imageInfos[s];
        writeCount++;
    }

    vkUpdateDescriptorSets(hal->pDevice, writeCount, writes, 0, nullptr);
    vkCmdBindDescriptorSets(hal->pCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            hal->pPipelineLayout, 0, 1, &descSet, 0, nullptr);

    // Bind the Vulkan graphics pipeline matching the current shader/blend/stencil state.
    // In Vulkan, unlike D3D11, the pipeline must be explicitly bound before each draw call.
    if (hal->InRenderPass && CurShaders.pVDesc && CurShaders.pFDesc)
    {
        SysVertexFormat* pvf = 0;
        if (CurShaders.pVFormat)
            pvf = (SysVertexFormat*)CurShaders.pVFormat->pSysFormat.GetPtr();
        VkPipeline pipeline = hal->GetPipeline(
            CurShaders.pVDesc->Type, CurShaders.pFDesc->Type,
            hal->CurrentBlendType, hal->CurrentStencilMode, pvf);
        if (pipeline != VK_NULL_HANDLE)
            vkCmdBindPipeline(hal->pCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    }

    memset(pmgr->CurrentTextures, 0, sizeof(pmgr->CurrentTextures));
    memset(pmgr->CurrentSamplers, 0, sizeof(pmgr->CurrentSamplers));

    ShaderInterfaceBase<Uniform, ShaderPair>::Finish(meshCount);
}


// *** ShaderManager

ShaderManager::ShaderManager(ProfileViews* prof)
: StaticShaderManager(prof), pDevice(VK_NULL_HANDLE),
  ShaderModel(ShaderDesc::ShaderVersion_Default)
{
    memset(StaticVShaders, 0, sizeof(StaticVShaders));
    memset(StaticFShaders, 0, sizeof(StaticFShaders));
}

void ShaderManager::MapVertexFormat(PrimitiveFillType fill, const VertexFormat* sourceFormat,
                                    const VertexFormat** single, const VertexFormat** batch,
                                    const VertexFormat** instanced)
{
    // Vulkan uses SSCALED vertex formats which convert S16 integers to float
    // on the GPU, so no S16→F32 conversion is needed here.  Keeping the source
    // format's native types ensures the mapped format stride matches the actual
    // vertex buffer layout (important for the unit-square VB used by filters
    // and mask clears, which is populated with VertexXY16iAlpha data).
    Base::MapVertexFormat(fill, sourceFormat, single, batch, instanced,
                          MVF_HasInstancing | MVF_Align);
}

bool ShaderManager::Initialize(HAL* phal)
{
    pDevice = phal->GetDevice();
    ShaderModel = ShaderDesc::ShaderVersion_Default;

    if (!ShaderDesc::IsShaderVersionSupported(ShaderModel))
    {
        SF_DEBUG_MESSAGE(1, "Vulkan: No supported shader version found.");
        return false;
    }

    for (unsigned i = 0; i < VertexShaderDesc::VSI_Count; i++)
    {
        const VertexShaderDesc* desc = VertexShaderDesc::Descs[i];
        if (desc && desc->Version == ShaderModel && desc->pBinary)
        {
            if (!StaticVShaders[i].Init(pDevice, desc))
                return false;
        }
    }

    for (unsigned i = 0; i < FragShaderDesc::FSI_Count; i++)
    {
        const FragShaderDesc* desc = FragShaderDesc::Descs[i];
        if (desc && desc->Version == ShaderModel && desc->pBinary)
        {
            if (!StaticFShaders[i].Init(pDevice, desc))
                return false;
        }
    }

    return true;
}

void ShaderManager::Reset()
{
    for (unsigned i = 0; i < VertexShaderDesc::VSI_Count; i++)
        StaticVShaders[i].Shutdown();
    for (unsigned i = 0; i < FragShaderDesc::FSI_Count; i++)
        StaticFShaders[i].Shutdown();
    pDevice = VK_NULL_HANDLE;
    VFormats.Clear();
}

void ShaderManager::BeginScene() {}
void ShaderManager::EndScene() {}

}}} // Scaleform::Render::Vulkan
