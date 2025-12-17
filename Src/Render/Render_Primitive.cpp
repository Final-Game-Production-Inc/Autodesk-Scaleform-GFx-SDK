/**************************************************************************

Filename    :   Render_Primitive.cpp
Content     :   Renderer HAL Mesh and Primitive objects implementation.
Created     :   May 2009
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render/Render_Primitive.h"
#include "Render/Render_HAL.h"
#include "Render/Render_MeshCache.h"
#include "Render/Render_Queue.h"
#include "Render/Render_PrimitiveProcessor.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"
#include "Render/Renderer2DImpl.h"

//#include <stdio.h> // printf

namespace Scaleform { namespace Render {


// ***** Mesh Implementation

Mesh::Mesh(Renderer2DImpl* prenderer, MeshProvider *provider,
           const Matrix2F& viewMatrix, float morphRatio, 
           unsigned layer, unsigned meshGenFlags)
: MeshBase(prenderer, provider, viewMatrix, morphRatio, layer, meshGenFlags), LargeMesh(false)
{
}

Mesh::~Mesh()
{
 //   if (Data.pInterface)
 //       Data.pInterface->ReleaseMeshData(Data.hMesh);

    // Go through CacheItems and release them.
    UPInt           count = CacheItems.GetSize();
    MeshCacheItem** pdata = CacheItems.GetBuffer();
    while(count--)
    {
        if (*pdata)
            (*pdata)->NotifyMeshRelease(this);
        pdata++;
    }

    // Remove us from the staging buffer.
    if (StagingBufferSize)
    {
        SF_ASSERT(PinCount == 0);
        RemoveNode();
    }
}

MeshUseStatus Mesh::GetUseStatus() const
{
    MeshUseStatus   status = MUS_Uncached;
    UPInt           count = CacheItems.GetSize();
    MeshCacheItem* const * pdata = CacheItems.GetBuffer();
    for(UPInt i = 0; i < count; i++)
    {
        MeshUseStatus s = pdata[i]->GetUseStatus();
        if (s > status)
            status = s;
    }
    return status;
}




// ***** PrimitiveFillData

// Check fillType dependencies
const char* PrimitiveFill_FormatCheck[PrimFill_Type_Count] = 
{
    "",     // PrimFill_None,
    "",     // PrimFill_SolidColor,                { XY }
    "",     // PrimFill_Mask,                      { XY }
    "C",    // PrimFill_VColor,                    { XY, Color }
    "CE",   // PrimFill_VColor_EAlpha,             { XY, Color, EAlpha }
    "",     // PrimFill_Texture,                   { XY }
    "E",    // PrimFill_Texture_EAlpha,            { XY, EAlpha }
    "CW",   // PrimFill_Texture_VColor,            { XY, Color, Weight1 }
    "CWE",  // PrimFill_Texture_VColor_EAlpha,     { XY, Color, Weight1, EAlpha }
    "W",    // PrimFill_2Texture,                  { XY, Weight1 }
    "EW",   // PrimFill_2Texture_EAlpha,           { XY, Weight1, EAlpha  }
    "U",    // PrimFill_UVTexture,                 { XY, UV }
    "UC",   // PrimFill_UVTextureAlpha_VColor,     { XY, UV, Color }
    "UC",   // PrimFill_UVTextureDFAlpha_VColor,   { XY, UV, Color }
};

bool    PrimitiveFillData::CheckVertexFormat(PrimitiveFillType fill, const VertexFormat* format)
{        
    SF_DEBUG_ASSERT(fill < PrimFill_Type_Count, "Invalid fill type.");

    if (!format->HasUsage(VET_Pos))
    {
        SF_DEBUG_ERROR(1, "D3D9::ShaderManager - VertexFormat missing Pos attribute");
        return false;
    }

    const char* pattrs;   
    for (pattrs = PrimitiveFill_FormatCheck[fill]; *pattrs; pattrs++)
    {
        const char* notFound = 0;

        switch(*pattrs)
        {
        case 'C':
            if (!format->HasUsage(VET_Color))
                notFound = "VColor";
            break;
        case 'E':
            if (!format->GetElement(VET_FactorAlpha8, VET_Type_Mask))
                notFound = "FactorAlpha8";
            break;
        case 'W':
            if (!format->GetElement(VET_T0Weight8, VET_Type_Mask))
                notFound = "T0Weight8";
            break;
        case 'U':
            if (!format->HasUsage(VET_TexCoord))
                notFound = "TexCoord";
            break;
        }
        if (notFound)
        {
            SF_DEBUG_ERROR1(1, 
                "Render::Primitive - VertexFormat missing %s attribute", notFound);
            return false;
        }
    }
    return true;
}



// ***** PrimitiveFill 

PrimitiveFill::~PrimitiveFill()
{
    if (pManager)
        pManager->removeFill(this);
}

// Utility function, which determines whether the fill type is a 'solid' fill color, from the
// point of view of setting textures and color streams.
bool PrimitiveFill::IsSolid(PrimitiveFillType type)
{
    switch(type)
    {
    case PrimFill_None:
    case PrimFill_Mask:
    case PrimFill_SolidColor:
        return true;
    default:
        return false;
    }
}

// Utility function, which determines which fill types use textures.
bool PrimitiveFill::HasTexture(PrimitiveFillType type)
{
    return type >= PrimFill_Texture;
}

void PrimitiveFillManager::removeFill(PrimitiveFill* fill)
{
    FillSet.Remove(fill);
}

void PrimitiveFillManager::removeGradient(GradientImage* img)
{
    Gradients.Remove(img);
}
// Creates and/or registers a gradient image; caller must store reference.
Image*  PrimitiveFillManager::createGradientImage(GradientData* data, float morphRatio)
{
    GradientImage* found = 0;
    if (Gradients.GetAlt(GradientKey(data, morphRatio), &found))
    {
        found->AddRef();
    }
    else
    {
        // TO DO: Revise Heap, SID, texArena
        found = SF_HEAP_AUTO_NEW(this) GradientImage(this, data, morphRatio);
        Gradients.Add(found);
    }
    return found;
}


PrimitiveFill* PrimitiveFillManager::CreateFill(const PrimitiveFillData& initdata)
{
    PrimitiveFill* fill = 0;
    
    if (FillSet.GetAlt(initdata, &fill))
        fill->AddRef();
    else
    {
        fill = pHAL->CreatePrimitiveFill(initdata);
        if (!fill)
            return 0;
        FillSet.Add(fill);
        fill->SetManager(this);
    }
    return fill;
}


PrimitiveFill* PrimitiveFillManager::CreateFill(const FillData& initdata, Ptr<Image>* gradientImg, 
                                                TextureManager* mng, float morphRatio)
{
    switch(initdata.Type)
    {    
    case Fill_Mask:
        {
            PrimitiveFillData data(PrimFill_Mask, initdata.pVFormat);
            return CreateFill(data);
        }
        break;
    case Fill_SolidColor:
        {
            PrimitiveFillData data(PrimFill_SolidColor, initdata.pVFormat, initdata.Color);
            return CreateFill(data);
        }        
        break;

    case Fill_VColor:
    case Fill_VColor_TestKey:
        {
            PrimitiveFillData data(initdata.PrimFill, initdata.pVFormat);
            if (initdata.Type == Fill_VColor_TestKey)
            {
                // MA: We assign color to use it as a unique key for batching,
                // as needed by SortedRender2D and SortedRenderTree tests.
                data.SolidColor = initdata.Color;
            }            
            return CreateFill(data);
        }
        break;

    case Fill_Image:
        {
        	if ( initdata.pImage )
        	{
                Texture* fillTexture = initdata.pImage->GetTexture(mng);
                if ( fillTexture )
                {
	                PrimitiveFillData data(initdata.PrimFill,
	                                       initdata.pVFormat, 
	                                       fillTexture, 
	                                       initdata.FillMode);
	                return CreateFill(data);
                }
        	}

            // If for some reason the image doesn't exist or the texture can't be created, create a bogus fill.
            PrimitiveFillData data(PrimFill_SolidColor, initdata.pVFormat, Color(0xFFFF0000) );
    		return CreateFill(data);
        }
        break;

    case Fill_Gradient:
        {
#ifdef SF_AMP_SERVER
            SF_AMP_SCOPE_RENDER_TIMER_ID("PrimitiveFillManager::CreateFill", Amp_Native_Function_Id_GradientFill);
#endif
            // Assign ownership externally.
            *gradientImg = *createGradientImage(initdata.pGradient, morphRatio);
            if (!*gradientImg)
                break;
            PrimitiveFillData data(initdata.PrimFill,
                                   initdata.pVFormat, 
                                   (*gradientImg)->GetTexture(mng), 
                                   initdata.FillMode);
            return CreateFill(data);
        }
        break;

    case Fill_None:
        break;
    }
    return 0;
}


// FillTypeMergeTable maps {fill0, fill1, mergeFlags} data to PrimitiveFillType result.
// Not all combinations are supported, so some cases will be missed.
// In particular, we expect fills to be normalized by tessellation, allowing
// only {Image, VColor} pair fill ordering and nor {VColor, Image}.
// If we choose to allow the later, we'd need to swap fills and invert Weight
// channel in vertices.

struct FillTypeMergeInfo
{
    UByte F0, F1;    
    UByte MergeFlags;
    UByte Result;
};

static FillTypeMergeInfo FillTypeMergeTable[] =
{
    // VColor
    { Fill_VColor,      Fill_None,      FillMerge_None,             PrimFill_VColor},
    { Fill_VColor,      Fill_None,      FillMerge_EAlpha,           PrimFill_VColor_EAlpha},
    // Image
    { Fill_Image,       Fill_None,      FillMerge_None,             PrimFill_Texture},
    { Fill_Image,       Fill_None,      FillMerge_EAlpha,           PrimFill_Texture_EAlpha},
    { Fill_Image,       Fill_VColor,    FillMerge_Weight,           PrimFill_Texture_VColor},
    { Fill_Image,       Fill_VColor,    FillMerge_EAlpha_Weight,    PrimFill_Texture_VColor_EAlpha},
    { Fill_Image,       Fill_Image,     FillMerge_Weight,           PrimFill_2Texture},
    { Fill_Image,       Fill_Image,     FillMerge_EAlpha_Weight,    PrimFill_2Texture_EAlpha},
    // Solid
    { Fill_SolidColor,  Fill_None,      FillMerge_None,             PrimFill_SolidColor},
    { Fill_Mask,        Fill_None,      FillMerge_None,             PrimFill_Mask},
    // End list
    { Fill_None,        Fill_None,      FillMerge_None,             PrimFill_None},
};

PrimitiveFillType GetMergedFillType(FillType ft0, FillType ft1, unsigned mergeFlags)
{
    FillTypeMergeInfo* mergeInfo = FillTypeMergeTable;

    // These types should've been masked out by caller.
    SF_ASSERT((ft0 != Fill_Gradient) && (ft0 != Fill_VColor_TestKey));
    SF_ASSERT((ft1 != Fill_Gradient) && (ft1 != Fill_VColor_TestKey));
    
    // Exhaustive search for matching target type.
    // This can be optimized if it becomes an issue.
    do {
        if ((mergeInfo->F0 == ft0) && (mergeFlags == mergeInfo->MergeFlags))
            if ((mergeInfo->F1 == ft1) || (!(mergeFlags & FillMerge_Weight)))
                break;
        mergeInfo++;
    } while(mergeInfo->F0 != Fill_None);

    return (PrimitiveFillType) mergeInfo->Result;
}


// Creates a complex merged fill built out of one or more regular fills.
PrimitiveFill* PrimitiveFillManager::CreateMergedFill(
    unsigned mergeFlags, const VertexFormat* vformat,
    const FillData* fd0, const FillData* fd1,    
    Ptr<Image>* gradientImg0, Ptr<Image>* gradientImg1, TextureManager* textureManager, 
    float morphRatio)
{
    bool merge = (mergeFlags & FillMerge_Weight) != 0;
    SF_UNUSED(merge); // debug only

    // Convert gradient types to image
    FillType ft0  = fd0->Type;
    FillType ft1  = fd1->Type;
    Image*   img0 = fd0->pImage; // Warning: May be invalid.
    Image*   img1 = fd1->pImage; // Warning: May be invalid.

    // Convert gradients to images.
    if (ft0 == Fill_Gradient)
    {
        *gradientImg0 = *createGradientImage(fd0->pGradient, morphRatio);
        img0 = *gradientImg0;
        ft0 = Fill_Image;
    }    
    if (ft1 == Fill_Gradient)
    {
        SF_ASSERT(merge);
        *gradientImg1 = *createGradientImage(fd1->pGradient, morphRatio);
        img1 = *gradientImg1;
        ft1 = Fill_Image;
    }
    
    // TBD: Remove Fill_VColor_TestKey once not needed
    if (ft0 == Fill_VColor_TestKey)
        ft0 = Fill_VColor;
    if (ft1 == Fill_VColor_TestKey)
        ft1 = Fill_VColor;

    PrimitiveFillType mergedFillType = GetMergedFillType(ft0, ft1, mergeFlags);
    PrimitiveFillData data(mergedFillType, vformat);

    switch(mergedFillType)
    {
        case PrimFill_Mask:
            break;
        case PrimFill_SolidColor:
            data.SolidColor = fd0->Color;
            break;

        case PrimFill_VColor:
        case PrimFill_VColor_EAlpha:            
            if (fd0->Type== Fill_VColor_TestKey)
            {
                // MA: We assign color to use it as a unique key for batching,
                // as needed by SortedRender2D and SortedRenderTree tests.
                data.SolidColor = fd0->Color;
            }
            break;

        case PrimFill_2Texture:
        case PrimFill_2Texture_EAlpha:
            data.Textures[1] = img1->GetTexture(textureManager);
            data.FillModes[1]= fd1->FillMode;
            // Fall through to set fill 0.
        case PrimFill_Texture:
        case PrimFill_Texture_VColor:
        case PrimFill_Texture_EAlpha:
        case PrimFill_Texture_VColor_EAlpha:
            data.Textures[0] = img0->GetTexture(textureManager);
            data.FillModes[0]= fd0->FillMode;
            break;

        default:
            // This shouldn't happen, as it typically means that proper 
            // merged fill type wasn't found.
            SF_DEBUG_WARNING(1, "PrimitiveFillManager::CreateMergedFill failed");
            return 0;
    }

    return CreateFill(data);
}


// TO DO: Determine vertex format in TreeCacheShape or ShapeMeshProvider
FillData::FillData(FillType type)
:   Type(type), 
    Color(0), 
    PrimFill(PrimFill_VColor_EAlpha),
    FillMode(),
    pVFormat(&VertexXY16iCF32::Format)
{
    if (type != Fill_VColor)
    {
        if (type == Fill_Mask)
        {
            PrimFill = PrimFill_Mask;
            pVFormat = &VertexXY16i::Format;
        }
        else
        {
            SF_ASSERT(1);
        }
    }
}

FillData::FillData(UInt32 color) : 
    Type(Fill_SolidColor), 
    Color(color), 
    PrimFill(PrimFill_SolidColor),
    FillMode(),
    pVFormat(&VertexXY16i::Format)
{}

FillData::FillData(Image* p, ImageFillMode fm) : 
    Type(Fill_Image), 
    pImage(p),
    PrimFill(PrimFill_Texture_EAlpha),
    FillMode(fm),
    pVFormat(&VertexXY16iCF32::Format)
{}

FillData::FillData(GradientData* p) : 
    Type(Fill_Gradient), 
    pGradient(p),
    PrimFill(PrimFill_Texture_EAlpha),
    FillMode(ImageFillMode(Wrap_Clamp, Sample_Linear)),
    pVFormat(&VertexXY16iCF32::Format)
{}


//--------------------------------------------------------------------
// ***** Primitive implementation

static unsigned Primitive_CreateCount = 0;
static unsigned Primitive_Total = 0;
static unsigned Primitive_Insert = 0;

Primitive::Primitive(HAL* phal, PrimitiveFill* pfill)
: pHAL(phal), pFill(pfill), 
  MatricesPerMesh(pfill->GetTextureMatrixCount()+1), ModifyIndex(0)
{
    Primitive_CreateCount++;
    Primitive_Total++;
//    printf("\nPrimitive create %d - total: %d", Primitive_CreateCount, Primitive_Total);
}

Primitive::~Primitive()
{
    Primitive_Total--;

    while(!Batches.IsEmpty())
        Batches.GetFirst()->RemoveAndFree();
}


bool Primitive::Insert(unsigned index, Mesh* pmesh, const HMatrix& m)
{
    SF_ASSERT(index <= GetMeshCount());

    PrimitiveBatch *pprevBatch = 0;
    PrimitiveBatch *pbatch = Batches.GetFirst();
    unsigned        i = 0;

    const MeshCacheParams& params = pHAL->GetMeshCache().GetParams();

    // Verify/assign vertex format. All meshes withing a primitive
    // should share the same vertex format.  
    /*
    if (pVertexFormat == 0)    
        pVertexFormat = pmesh->GetVertexFormat();
    else
    {
        SF_ASSERT(*pVertexFormat == *pmesh->GetVertexFormat());
    }   */

    while(1)
    {
        // If we are at the index, index fell between the blocks
        if (i == index)
        {
            if (pprevBatch && pprevBatch->IsVirtual())
            {
                // InsertMeshes.
                pprevBatch->MeshCount++;
                break;
            }
            // Batches.IsNull(pbatch) can hold for an empty list
            else if (Batches.IsNull(pbatch) || !pbatch->IsVirtual())
            {
                PrimitiveBatch* pnewBatch =
                    PrimitiveBatch::Create(this, PrimitiveBatch::DP_Virtual, i, 1);
                SF_ASSERT(pnewBatch);
                if (!pnewBatch)
                    return false;
                // Insert a new chunk here; this works even if Batches list is empty.
                pbatch->InsertNodeBefore(pnewBatch);
                break;
            }  
        }

        // Terminate loop after we reached the end. This check is here instead of
        // top/bottom of loop to ensure that the last item index is properly handled.
        if (Batches.IsNull(pbatch))
            break;

        // If we land within this chunk, insert us into this primitive.
        if ((index - i) < pbatch->MeshCount)
        {
            // Insert meshes.
            pbatch->MeshCount++;
            if (!pbatch->IsInstanced() || (Meshes[i].pMesh != pmesh) ||
                (pbatch->MeshCount > params.MaxBatchInstances))
            {
                pbatch->Type = PrimitiveBatch::DP_Virtual;
                pbatch->ClearCacheItem();
            }
            break;
        }

        i += pbatch->MeshCount;
        pprevBatch = pbatch;
        pbatch     = pbatch->GetNext();
    }

    // Resize arrays.
    MeshEntry e = { m, pmesh };
    Meshes.InsertAt(index, e); 
    if (index < ModifyIndex)
        ModifyIndex = index;

    Primitive_Insert++;
    //  printf(" I%d", Primitive_Insert);

    return true;
}


void Primitive::Remove(unsigned index, unsigned count)
{
    // When we remove, we can modify up to two batches and remove even more from the middle.
    SF_ASSERT((index + count) <= GetMeshCount());

    PrimitiveBatch *pbatch = Batches.GetFirst();
    unsigned        i = 0;
    unsigned        leftToRemove = count;

    do {

        if ((index - i) < pbatch->MeshCount)
        {
            if (index == i)
            {
                if (leftToRemove >= pbatch->MeshCount)
                {
                   PrimitiveBatch* poldBatch = pbatch;

                    // Remove All.  Note that 'i' doesn't change.
                    leftToRemove -= pbatch->MeshCount;
                    pbatch = pbatch->GetNext();
                    poldBatch->RemoveAndFree();
                    continue;
                }
                else
                {
                    // RemoveMeshes() from front.
                    pbatch->MeshCount -= leftToRemove;
                    if (!pbatch->IsInstanced())
                    {
                        pbatch->Type = PrimitiveBatch::DP_Virtual;
                        pbatch->ClearCacheItem();
                    }
                    leftToRemove = 0;
                    break;
                }
            }
            else
            {
                // Remove Tail
                unsigned removeCount = (leftToRemove > (pbatch->MeshCount - (index - i))) ?
                                       pbatch->MeshCount - (index - i) : leftToRemove;

                // RemoveMeshes():
                pbatch->MeshCount -= removeCount;
                if (!pbatch->IsInstanced())
                {
                    pbatch->Type = PrimitiveBatch::DP_Virtual;
                    pbatch->ClearCacheItem();
                }

                leftToRemove -= removeCount;
                // Advance index by leftover mesh count.
                i += pbatch->MeshCount;
            }
        }
        else
        {
            i += pbatch->MeshCount;
        }
        
        pbatch = pbatch->GetNext();

    } while(leftToRemove && !Batches.IsNull(pbatch));


    // Remove from arrays.
    Meshes.RemoveMultipleAt(index, count);
    if (index < ModifyIndex)
        ModifyIndex = index;
}


void Primitive::SetMesh(unsigned index, Mesh* pmesh)
{
    if (Meshes[index].pMesh == pmesh)
        return;
    
    Meshes[index].pMesh = pmesh;
        
    // Find PrimitiveBatch and mark it as Virtual.
    PrimitiveBatch *pbatch = Batches.GetFirst();
    unsigned        i = 0;

    while(index >= (i + pbatch->MeshCount))
    {
        i += pbatch->MeshCount;
        pbatch = pbatch->GetNext();
        SF_ASSERT(!Batches.IsNull(pbatch));
    }
     
    pbatch->Type = PrimitiveBatch::DP_Virtual;
    pbatch->ClearCacheItem();
}

#ifdef SF_BUILD_DEBUG
void Primitive::VerifyMeshIndices()
{
    // Find PrimitiveBatch and mark it as Virtual.
    PrimitiveBatch *pbatch = Batches.GetFirst();
    unsigned        i = 0;

    while(!Batches.IsNull(pbatch))
    {
        SF_ASSERT(pbatch->MeshIndex == i);
        i += pbatch->MeshCount;
        pbatch = pbatch->GetNext();
    }
}
#endif


void Primitive::updateMeshIndicies_Impl()
{    
    if (ModifyIndex >= GetMeshCount())
        return;
    
    // Walk backwards and re-assign indices.
    // This is more efficient then walking forward,
    // since front nodes don't need update.
    PrimitiveBatch *pbatch = Batches.GetLast();
    unsigned        i = GetMeshCount();

    while(!Batches.IsNull(pbatch))
    {
        i -= pbatch->MeshCount;
        pbatch->MeshIndex = i;
        if (i < ModifyIndex)
            break;
        pbatch = pbatch->GetPrev();
    }

    ModifyIndex = GetMeshCount();
}


//--------------------------------------------------------------------
// *** PrimitiveBatch implementation

PrimitiveBatch* PrimitiveBatch::Create(Primitive* p, BatchType type,
                                       unsigned meshIndex, unsigned meshCount)
{
    PrimitiveBatch* pbatch = 
        (PrimitiveBatch*)SF_HEAP_AUTO_ALLOC(p, sizeof(PrimitiveBatch));
    pbatch->Init(p, type, meshIndex, meshCount);
    return pbatch;
}

void PrimitiveBatch::RemoveAndFree()
{
    RemoveNode();
    ClearCacheItem();
    SF_FREE(this);
}

// Simple helper that computes the number of vertices and indices (total) in the batch.
void PrimitiveBatch::CalcMeshSizes(unsigned* ptotalVertices, unsigned *ptotalIndices)
{
    pPrimitive->UpdateMeshIndicies();

    unsigned arrayMeshCount   = IsInstanced() ? 1 : MeshCount; 
    unsigned totalVertexCount = 0,
             totalIndexCount  = 0;
    for (unsigned imesh = 0; imesh < arrayMeshCount; imesh++)
    {
        Mesh* pmesh = pPrimitive->Meshes[MeshIndex + imesh].pMesh;
        totalVertexCount += pmesh->VertexCount;
        totalIndexCount  += pmesh->IndexCount;
    }
    *ptotalVertices = totalVertexCount;
    *ptotalIndices  = totalIndexCount;
}


//--------------------------------------------------------------------
// ***** ComplexMesh & Complex Primitive

ComplexMesh::ComplexMesh(Renderer2DImpl* renderer, MeshProvider *meshProvider,
                         PrimitiveFillManager *fillManager,
                         const Matrix2F& viewMatrix, float morphRatio, 
                         unsigned layer, unsigned meshGenFlags)
: MeshBase(renderer, meshProvider, viewMatrix, morphRatio, layer, meshGenFlags),  
  pFillManager(fillManager), pCacheMeshItem(0), AllocTooBig(false)
{
}
ComplexMesh::~ComplexMesh()
{
    if (UpdateListNode.pPrev)
    {
        UpdateListNode.RemoveNode();
    }

    //unsigned warn_unfinished;    
    if (pCacheMeshItem)
        pCacheMeshItem->NotifyMeshRelease(this);

    // Staging buffer not used for complex mesh.
    SF_ASSERT(StagingBufferSize == 0); 
}

bool ComplexMesh::InitFillRecords(const VertexOutput::Fill* fills, unsigned fillRecordCount,
                                  const Matrix2F& vertexMatrix, HAL *hal,
                                  UPInt *vbSize, unsigned *vertexCount, unsigned *indexCount)
{    
    FillRecords.Resize(fillRecordCount);
    if (FillRecords.GetSize() != fillRecordCount)
        return false;

    TextureManager* textureManager = hal->GetTextureManager();

    if (UpdateListNode.pPrev)
    {
        UpdateListNode.RemoveNode();
        UpdateListNode.pPrev = UpdateListNode.pNext = NULL;
    }

    // Initialize rendering fill records by
    //  - Creating PrimitiveFills, potentially merged from two FillDatas for EdgeAA.
    //  - Mapping vertex formats to hardware version.
    //  - Assigning buffer offsets.

    *vbSize      = 0;
    *vertexCount = *indexCount = 0;  
    
    for (unsigned i = 0; i < fillRecordCount; i++)
    {
        const VertexOutput::Fill& src = fills[i];
        FillRecord &              fr  = FillRecords[i];
        
        // 1. Create primitive fill
        FillData    fd[2];
        Ptr<Image>  gi[2];
        GetProvider()->GetFillData(&fd[0], GetLayer(), src.FillIndex0, GetMeshGenFlags());
        if (src.MergeFlags & FillMerge_Weight)
            GetProvider()->GetFillData(&fd[1], GetLayer(), src.FillIndex1, GetMeshGenFlags());

        fr.pFill = *pFillManager->CreateMergedFill(src.MergeFlags, src.pFormat,
                                                   &fd[0], &fd[1],
                                                   &gi[0], &gi[1], textureManager, GetMorphRatio());
        if (!fr.pFill)
        {
            // Error... either fail or skip it in list. Fail for now.
            return false;
        }
        
        // If pFill is an image fill - add the ComplexMesh into the list
        if (!UpdateListNode.pPrev && (fd[0].Type == Fill_Image || fd[1].Type == Fill_Image))
        {
            GetRenderer()->AddComplexMeshToUpdateList(UpdateListNode);
        }

        fr.IndexOffset        = *indexCount;
        fr.IndexCount         = src.IndexCount;
        fr.VertexByteOffset   = *vbSize;
        fr.VertexCount        = src.VertexCount;
        fr.FillMatrixIndex[0] = src.FillIndex0;
        fr.FillMatrixIndex[1] = src.FillIndex1;
        fr.MergeFlags         = src.MergeFlags;

        // Keep gradient image references, if any, so that it doesn't die,
        // in case texture needs to be re-generated.
        if (gi[0]) GradientImages.PushBack(gi[0]);
        if (gi[1]) GradientImages.PushBack(gi[1]);

        const VertexFormat* tempBatchVF;
        hal->MapVertexFormat(fr.pFill->GetType(), fr.pFill->GetVertexFormat(),
                             &fr.pFormats[0], &tempBatchVF, &fr.pFormats[1], MeshCacheItem::Mesh_Complex);

        *vbSize += fr.pFormats[0]->Size * src.VertexCount;
        *vertexCount += src.VertexCount;
        *indexCount += src.IndexCount;
        
        #ifdef SF_OS_XBOX360
        // On X360, the index buffer is used as a stream in instancing, and thus the number of indices in a 
        // fill record must be a multiple of 2, so that they are 4 byte aligned, which is a requirement of
        // SetStreamSource on X360.
        if ( *indexCount & 1 )
            (*indexCount)++;
        #endif
    }

    VertexMatrix = vertexMatrix;
    updateFillMatrixCache(vertexMatrix);
    return true;
}

void ComplexMesh::updateFillMatrixCache(const Matrix2F& vertexMatrix)
{
    unsigned fillCount = GetFillCount(GetLayer(), GetMeshGenFlags());
    FillMatrixCache.Resize(fillCount);
    if (FillMatrixCache.GetSize() != fillCount)
        return;
    for (unsigned i=0; i< fillCount; i++)
    {            
        GetProvider()->GetFillMatrix(this, &FillMatrixCache[i], GetLayer(),
                                     i, GetMeshGenFlags());
        FillMatrixCache[i].Prepend(vertexMatrix);
    }
}

bool ComplexMesh::updateFills()
{
    if (!GetProvider()->IsValid())
        return false;
    TextureManager* textureManager = GetRenderer()->GetHAL()->GetTextureManager();

    // Initialize rendering fill records by
    //  - Creating PrimitiveFills, potentially merged from two FillDatas for EdgeAA.
    //  - Mapping vertex formats to hardware version.
    //  - Assigning buffer offsets.

    for (unsigned i = 0; i < FillRecords.GetSize(); i++)
    {
        FillRecord& fr  = FillRecords[i];

        if (fr.pFill)
        {
            // 1. Create primitive fill
            FillData    fd[2];
            Ptr<Image>  gi[2];
            GetProvider()->GetFillData(&fd[0], GetLayer(), fr.FillMatrixIndex[0], GetMeshGenFlags());
            
            GetProvider()->GetFillData(&fd[1], GetLayer(), fr.FillMatrixIndex[1], GetMeshGenFlags());

            Ptr<PrimitiveFill> mergedFill = *pFillManager->CreateMergedFill(fr.MergeFlags, 
                fr.pFill->GetVertexFormat(),
                &fd[0], &fd[1],
                &gi[0], &gi[1], textureManager, GetMorphRatio());
            if (!mergedFill)
            {
                // Error... either fail or skip it in list. Skip for now.
                continue;
            }
            fr.pFill = mergedFill;
        }
    }
    updateFillMatrixCache(VertexMatrix);
    return true;
}

//--------------------------------------------------------------------
// ***** MaskPrimitive

void MaskPrimitive::Insert(UPInt index, const HMatrix& m)
{
    MaskAreas.InsertAt(index, m);   
}
void MaskPrimitive::Remove(UPInt index, UPInt count)
{
    MaskAreas.RemoveMultipleAt(index, count);
}

// RenderQueueItem::Interface impl
void MaskPrimitive::EmitToHAL(RenderQueueItem&, RenderQueueProcessor& qp)
{
    if ( qp.GetQueueEmitFilter() != RenderQueueProcessor::QPF_All )
        return;
    qp.GetHAL()->PushMask_BeginSubmit(this);
}

//--------------------------------------------------------------------
// ***** CacheablePrimitive
CacheablePrimitive::CacheablePrimitive(bool masksPresent) :
    MaskPresent(masksPresent)
{
    SetCacheResults(Cache_Uncached, 0, 0);
}

CacheablePrimitive::~CacheablePrimitive()
{
    for ( unsigned i =0; i < MaximumCachedResults; ++i )
        CacheResults[i] = 0;
}

void CacheablePrimitive::GetCacheResults( RenderTarget** results, unsigned count )
{
    for ( unsigned i =0; i < count; ++i )
    {
        if ( i < count )
            results[i] = CacheResults[i];
        else
            results[i] = 0;
    }
}

void CacheablePrimitive::SetCacheResults( CacheState state, RenderTarget** results, unsigned count )
{
    Caching = state;
    for ( unsigned i = 0; i < MaximumCachedResults; ++i)
    {
        if ( i < count && results != 0)
            CacheResults[i] = results[i];
        else
            CacheResults[i] = 0;
    }

    // If we become uncached, we need to make sure that we notify the derived class.
    if (state == Cache_Uncached )
        uncachePrimitive();
}

void CacheablePrimitive::Insert(UPInt index, const HMatrix& m)
{
    SF_UNUSED(index);
    //SF_DEBUG_ASSERT(index == 0, "Currently can't batch filters, should only ever be one filter.");
    PrimitiveArea.InsertAt(index, m);   
}
void CacheablePrimitive::Remove(UPInt index, UPInt count)
{
    SF_UNUSED2(index, count);
    //SF_DEBUG_ASSERT(index == 0 && count == 1, "Currently can't batch filters, should only ever be one filter.");
    for (UPInt i = index; i < count; ++i)
        uncachePrimitive(i);

    PrimitiveArea.RemoveMultipleAt(index, count);
}


//--------------------------------------------------------------------
// ***** FilterPrimitive

void FilterPrimitive::EmitToHAL(RenderQueueItem& item, RenderQueueProcessor& qp)
{
    // qp.GetQueueFilter() - if check if more modes are added.
    PrimitiveEmitBuffer*    emitBuffer    = (PrimitiveEmitBuffer*)qp.GetEmitItemBuffer();
    if ( !emitBuffer->IsEmitting(&item) )
    {
        emitBuffer->StartEmitting(&item, 0);
        if (pFilters)
            qp.GetHAL()->PushFilters(this);
        else
            qp.GetHAL()->PopFilters();
    }
}

RenderQueueItem::QIPrepareResult FilterPrimitive::Prepare(RenderQueueItem&, RenderQueueProcessor& qp, bool)
{
    // Only prepare the filters, if they are actually going to be rendered.
    // NOTE: Cacheable preparation may change the RenderQueueProcessor's filter, which will affect 
    // subsequent items Prepare processing.
    if (qp.GetHAL()->shouldRenderFilters(this))
        qp.GetHAL()->PrepareCacheable(this, GetFilters() == 0);

    return RenderQueueItem::QIP_Done;
}

// Reverts the FilterArea matrix to its uncached state. This is required, because the cached
// matrix and uncached matrices may be different, if we are caching across filter transformations.
void FilterPrimitive::uncachePrimitive(UPInt index)
{
    if (!PrimitiveArea[index].IsNull())
    {
        if(PrimitiveArea[index].HasTextureMatrix(0))
        {
            const Matrix2F& m = PrimitiveArea[index].GetTextureMatrix(0);
            PrimitiveArea[index].SetMatrix2D(m);
        }
        Matrix2F zero;
        zero.SetZero();
        PrimitiveArea[index].SetTextureMatrix(zero, 1);
    }
}

//--------------------------------------------------------------------
// ***** BlendPrimitive

void BlendPrimitive::EmitToHAL(RenderQueueItem& item, RenderQueueProcessor& qp)
{
    // qp.GetQueueFilter() - if check if more modes are added.
    PrimitiveEmitBuffer*    emitBuffer    = (PrimitiveEmitBuffer*)qp.GetEmitItemBuffer();
    if ( !emitBuffer->IsEmitting(&item) )
    {
        emitBuffer->StartEmitting(&item, 0);
        if (BlendModeValue < Blend_Count)
            qp.GetHAL()->PushBlendMode(this);
        else
            qp.GetHAL()->PopBlendMode();
    }
}

RenderQueueItem::QIPrepareResult BlendPrimitive::Prepare(RenderQueueItem&, RenderQueueProcessor& qp, bool)
{
    // NOTE: Cacheable preparation may change the RenderQueueProcessor's filter, which will affect 
    // subsequent items Prepare processing. Note: Blend_Invalid signifies the end of a mode that required
    // a target (Blend_Count is the end of a blend mode not requiring a target)
    if (BlendModeValue == Blend_Invalid)
        qp.GetHAL()->PrepareCacheable(this, true);
    else if (BlendState::IsTargetAllocationNeededForBlendMode(BlendModeValue))
        qp.GetHAL()->PrepareCacheable(this, false);
    return RenderQueueItem::QIP_Done;
}

//--------------------------------------------------------------------
// ***** ViewMatrix3DPrimitive

void ViewMatrix3DPrimitive::EmitToHAL(RenderQueueItem&, RenderQueueProcessor& qp)
{
    if (HasViewMatrix3D())
        qp.GetHAL()->PushView3D(GetViewMatrix3D());
    else
        qp.GetHAL()->PopView3D();
}

//--------------------------------------------------------------------
// ***** ProjectionMatrix3DPrimitive

void ProjectionMatrix3DPrimitive::EmitToHAL(RenderQueueItem&, RenderQueueProcessor& qp)
{
    if (HasProjectionMatrix3D())
        qp.GetHAL()->PushProj3D(GetProjectionMatrix3D());
    else
        qp.GetHAL()->PopProj3D();
}

//--------------------------------------------------------------------
// ***** UserDataPrimitive

void UserDataPrimitive::EmitToHAL(RenderQueueItem&, RenderQueueProcessor& qp)
{
    if (pUserData)
        qp.GetHAL()->PushUserData(pUserData);
    else
        qp.GetHAL()->PopUserData();
}

}} // Scaleform::Render

