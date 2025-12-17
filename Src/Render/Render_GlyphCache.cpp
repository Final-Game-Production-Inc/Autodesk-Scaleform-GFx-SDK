/**************************************************************************

Filename    :   Render_GlyphCache.cpp
Content     :   
Created     :   
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render_GlyphCache.h"
#include "Render_HAL.h"
#include "Render_Image.h"
#include "Render_FontCacheHandle.h"
#include "Render_BoundBox.h"
#include "Render_TessCurves.h"
#include "Render_ResizeImage.h"
#include "ImageFiles/Render_ImageFile.h"

#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_AmpInterface.h"

namespace Scaleform { namespace Render {


//static unsigned FontSizeRamp[] = { 0,1,2,4,8,16,32,64,128,256,0};
//static unsigned FontSizeRamp[] = { 0,1,2,4,6,8,11,16,22,32,40,50,64,76,90,108,128,147,168,194,222,256,0};
static unsigned FontSizeRamp[] = { 0,0,2,4,6,8,12,16,18,22,26,32,36,40,45,50,56,64,72,80,90,108,128,147,168,194,222,256,0};


//------------------------------------------------------------------------
VertexElement RasterGlyphVertex::Elements[] =
{
    { 0,  {VET_XY32f} },
    { 8,  {VET_ColorARGB8} },
    { 12, {VET_UV32f} },
    { 0,  {VET_None} }
};
VertexFormat RasterGlyphVertex::Format = { sizeof(RasterGlyphVertex), RasterGlyphVertex::Elements };


//------------------------------------------------------------------------
GlyphTextureImage::GlyphTextureImage(GlyphCache* cache, unsigned textureId, 
                                     const ImageSize& size, unsigned use) : 
    TextureImage(Image_A8, size, use), pCache(cache), TextureId(textureId)
{}



//------------------------------------------------------------------------
GlyphTextureImage* GlyphTextureImage::Create(MemoryHeap* heap, TextureManager* texMan, GlyphCache* cache, 
                                             unsigned textureId, const ImageSize& size, unsigned use)
{
    GlyphTextureImage* img = SF_HEAP_NEW(heap) GlyphTextureImage(cache, textureId, size, use);
    Texture* tex = texMan->CreateTexture(Image_A8, 1, size, use, img);
    if (!tex)
    {
        img->Release();
        return 0;
    }
    img->initTexture_NoAddRef(tex);
    return img;
}


//------------------------------------------------------------------------
void GlyphTextureImage::TextureLost(TextureLossReason reason)
{
    TextureImage::TextureLost(reason);
    pCache->TextureLost(TextureId, reason);
}


//------------------------------------------------------------------------
bool GlyphTextureMapper::Create(unsigned method, MemoryHeap* heap,
                                TextureManager* texMan, PrimitiveFillManager* fillMan, GlyphCache* cache, 
                                unsigned textureId, const ImageSize& size)
{
    SF_AMP_SCOPE_RENDER_TIMER_ID("GlyphTextureMapper::Create", Amp_Native_Function_Id_GlyphTextureMapper_Create);

    pTexMan = texMan;
    unsigned use = 0;
    bool ret = false;
    Method = method;
    if (method == GlyphCache::TU_WholeImage)
    {
        pTexImg = 0;
        use |= ImageUse_Update;
        pRawImg = *RawImage::Create(Image_A8, 1, size, use, heap);
        ret = pRawImg.GetPtr() != 0;
        if (ret)
        {
            PrimitiveFillData fillData(PrimFill_UVTextureAlpha_VColor, &RasterGlyphVertex::Format, 
                                       pRawImg->GetTexture(texMan), ImageFillMode(Wrap_Clamp, Sample_Linear));
            pFill = *fillMan->CreateFill(fillData);
        }
    }
    else
    {
        pRawImg = 0;
        if (method == GlyphCache::TU_MultipleUpdate)
            use |= ImageUse_PartialUpdate;
        else
            use |= ImageUse_MapSimThread | ImageUse_MapRenderThread;  // TO DO: Verify ImageUse_MapSimThread!

        pTexImg = *GlyphTextureImage::Create(heap, texMan, cache, textureId, size, use);
        ret = pTexImg.GetPtr() != 0;
        if (ret)
        {
            PrimitiveFillData fillData(PrimFill_UVTextureAlpha_VColor, &RasterGlyphVertex::Format, 
                                       pTexImg->GetTexture(texMan), ImageFillMode(Wrap_Clamp, Sample_Linear));
            pFill = *fillMan->CreateFill(fillData);
        }
    }
    return Valid = ret;
}


//------------------------------------------------------------------------
ImagePlane* GlyphTextureMapper::Map()
{
    SF_AMP_CODE(LockSafe::Locker lock(&ImageLock);)
    ImagePlane* plane = 0;
    switch(Method)
    {
    case GlyphCache::TU_DirectMap:
        SF_ASSERT(pTexImg.GetPtr() != 0);
        if (!Mapped)
        {
            if (!pTexImg->Map(&Data, 0, 1))
                break;
            Mapped = true;
        }
        plane = Data.pPlanes;
        break;

    case GlyphCache::TU_MultipleUpdate:
        break;

    case GlyphCache::TU_WholeImage:
        SF_ASSERT(pRawImg.GetPtr() != 0);
        if (!Mapped)
        {
            if (!pRawImg->Map(&Data))
                break;
            Mapped = true;
        }
        plane = Data.pPlanes;
        break;
    }
    return plane;
}


//------------------------------------------------------------------------
bool GlyphTextureMapper::Unmap()
{
    SF_AMP_CODE(LockSafe::Locker lock(&ImageLock);)
    bool ret = false;
    switch(Method)
    {
    case GlyphCache::TU_DirectMap:
        SF_ASSERT(pTexImg.GetPtr() != 0);
        if (Mapped)
        {
            ret = pTexImg->Unmap();
            Mapped = false;
        }
        break;

    case GlyphCache::TU_MultipleUpdate:
        SF_ASSERT(pTexImg.GetPtr() != 0);
        break;

    case GlyphCache::TU_WholeImage:
        SF_ASSERT(pRawImg.GetPtr() != 0);
        if (Mapped)
        {
            ret = pRawImg->Unmap();
            Texture* tex = pRawImg->GetTexture(pTexMan);
            tex->Update();
            Mapped = false;
        }
        break;
    }
    return ret;
}


//------------------------------------------------------------------------
bool GlyphTextureMapper::Update(const Texture::UpdateDesc* updates, unsigned count)
{
    SF_AMP_CODE(LockSafe::Locker lock(&ImageLock);)
    bool ret = false;
    if (Method == GlyphCache::TU_MultipleUpdate)
    {
        SF_ASSERT(pTexImg.GetPtr() != 0);
        Texture* tex = pTexImg->GetTexture(pTexMan);
        if (tex)
        {
            ret = tex->Update(updates, count);
        }
    }
    return ret;
}

Image* GlyphTextureMapper::GetImage()
{ 
    SF_AMP_CODE(SF_DEBUG_ASSERT(ImageLock.IsLocked(), "Font texture not locked - possible race condition with AMP");)
    return pRawImg.GetPtr() ? (Image*)pRawImg : (Image*)pTexImg;
}


//------------------------------------------------------------------------
GlyphCache::GlyphCache(MemoryHeap* heap) : 
    pHeap(heap),
    pRenderer(0),
    pFillMan(0),
    pTexMan(0),
    TextureWidth(0),
    TextureHeight(0),
    MaxNumTextures(0),
    MaxSlotHeight(0),
    SlotPadding(0),
    ScaleU(0),
    ScaleV(0),
    ShadowQuality(1),
    Method(TU_DirectMap),
    UpdatePacker(0,0),
    pRQCaches(0),
    Fitter(heap),
    Ras(heap),
    ScanlineFilter(8.0f/9.0f, 2.0f/9.0f, 0.2f/9.0f),
    RasterPitch(0),
    LHeap1(heap),
    LHeap2(heap),
    mStroker(&LHeap2),
    TmpPath1(&LHeap1),
    TmpPath2(&LHeap2),
    RasterizationCount(0),
    RasterCacheWarning(true),
    RasterTooBigWarning(true)
{
    Notifier.pCache = this;

    unsigned i;
    unsigned ramp = 0;
    for (i = 0; i < 256; ++i)
    {
        if (i > FontSizeRamp[ramp + 1])
            ++ramp;
        FontSizeMap[i] = (UByte)ramp;
    }

    pLog = Log::GetGlobalLog();
    if (pLog == 0)
        pLog = Log::GetDefaultLog();
}

//------------------------------------------------------------------------
GlyphCache::~GlyphCache()
{
    Destroy();
}

//------------------------------------------------------------------------
void GlyphCache::Destroy()
{
    UnpinAllSlots();
    Queue.Clear();
    releaseAllTextures();
    UpdatePacker.Clear();
    GlyphsToUpdate.Clear();
    RectsToUpdate.Clear();

    VectorGlyphShapeList.Clear();
    VectorGlyphCache.Clear();
    //pRenderer = 0;
    //pFillMan  = 0;
    //pTexMan   = 0;
    if (pFontHandleManager)
    {
        pFontHandleManager->DestroyAllFonts();
        pFontHandleManager.Clear();
    }
    if (pRQCaches)
    {
        pRQCaches->SetCache(Cache_Glyph, 0);
        pRQCaches = 0;
    }
}

//------------------------------------------------------------------------
void GlyphCache::ClearCache()
{
    UnpinAllSlots();
    Queue.Clear();
    UpdatePacker.Clear();
    GlyphsToUpdate.Clear();
    RectsToUpdate.Clear();

    VectorGlyphShapeList.Clear();
    VectorGlyphCache.Clear();
}

bool GlyphCache::SetParams(const GlyphCacheParams& params)
{
    Param = params;
    if (pRenderer && pRenderer->IsInitialized())
        initialize();
    return true;
}


//------------------------------------------------------------------------
void GlyphCache::PrintMemStats()
{
    Queue.PrintMemStats();
}

unsigned GlyphCache::GetNumTextures() const
{
    unsigned numTextures = 0;
    for (unsigned i = 0; i < MaxNumTextures; ++i)
    {
        if (Textures[i].IsValid())
        {
            ++numTextures;
        }
    }
    return numTextures;

}

// Initialize or re-initialize the cache. The physical structure of the cache 
// slots is shown above. The function also clears the cache.
//
//------------------------------------------------------------------------
void GlyphCache::initialize()
{
    Destroy();

    pTexMan = pRenderer->GetTextureManager();
    SF_ASSERT( pTexMan != NULL );

    unsigned texWidth        = Param.TextureWidth;
    unsigned texHeight       = Param.TextureHeight;
    unsigned maxNumTextures  = Param.NumTextures;
    unsigned maxSlotHeight   = Param.MaxSlotHeight;
    unsigned slotPadding     = Param.SlotPadding;
    unsigned texUpdWidth     = Param.TexUpdWidth;
    unsigned texUpdHeight    = Param.TexUpdHeight;
    bool     fenceWaitOnFull = Param.FenceWaitOnFullCache;

    unsigned w  = (texWidth  < 64) ? 63 : texWidth  - 1;
    unsigned h  = (texHeight < 64) ? 63 : texHeight - 1;
    unsigned sw = 0;
    unsigned sh = 0;

    while(w) { ++sw; w >>= 1; }
    while(h) { ++sh; h >>= 1; }

    if (maxNumTextures > TexturePoolSize)
        maxNumTextures = TexturePoolSize;

    TextureWidth         = 1 << sw;
    TextureHeight        = 1 << sh;
    MaxNumTextures       = maxNumTextures;
    MaxSlotHeight        = maxSlotHeight;
    SlotPadding          = slotPadding;
    ScaleU               = 1.0f / (float)TextureWidth;
    ScaleV               = 1.0f / (float)TextureHeight;
    ShadowQuality        = Param.ShadowQuality;

    if (MaxNumTextures)
    {
        Queue.Init(&Notifier, 0, maxNumTextures, texWidth, texHeight, maxSlotHeight, fenceWaitOnFull);
        unsigned caps = pTexMan->GetTextureUseCaps(Image_A8);

        if (caps & ImageUse_MapRenderThread)
        {
            Method = TU_DirectMap;
        }
        else
        if (caps & ImageUse_PartialUpdate)
        {
            Method = TU_MultipleUpdate;
            UpdatePacker = TextureUpdatePacker(texUpdWidth, texUpdHeight);
            UpdateBuffer = *RawImage::Create(Image_A8, 1, ImageSize(texUpdWidth, texUpdHeight), 0, pHeap);
        }
        else
        {
            Method = TU_WholeImage;
        }

        unsigned i;
        for(i = 0; i < MaxNumTextures; ++i)
        {
            Textures[i].Create(Method, pHeap, pTexMan, pFillMan, this, i, 
                               ImageSize(TextureWidth, TextureHeight));
        }
    }

    // Must be created in Global Heap
    if (!pFontHandleManager) // TO DO: Revise
        pFontHandleManager = *SF_NEW FontCacheHandleManager(pHeap, this);

    PrimitiveFillData fillDataSolid(PrimFill_VColor_EAlpha, &VertexXY16iCF32::Format);
    PrimitiveFillData fillDataMask(PrimFill_Mask, &VertexXY16i::Format);
    pSolidFill = *pFillMan->CreateFill(fillDataSolid);
    pMaskFill  = *pFillMan->CreateFill(fillDataMask);

    pRQCaches  = &pRenderer->GetRQCacheInterface();
    pRQCaches->SetCache(Cache_Glyph, this);
}

//------------------------------------------------------------------------
void GlyphCache::Initialize(HAL* ren, PrimitiveFillManager* fillMan)
{
    pRenderer = ren;
    pFillMan  = fillMan;

    // Only initialize the GlyphCache if the HAL is also initialized.
    if (pRenderer->IsInitialized())
        initialize();
}


//------------------------------------------------------------------------
void GlyphCache::OnBeginFrame()
{
    if ( checkInitialization() )
        pFontHandleManager->ProcessKillList();
}

//------------------------------------------------------------------------
void GlyphCache::MergeCacheSlots()
{
    Queue.MergeEmptySlots();
}

//------------------------------------------------------------------------
void GlyphCache::OnEndFrame()
{
}

//------------------------------------------------------------------------
void GlyphCache::CleanUpFont(FontCacheHandle* font)
{
    SF_ASSERT(font->pFont == 0);
    ApplyInUseList();
    UpdatePinList();
    Queue.CleanUpFont(font);

    VectorGlyphShape* sh = VectorGlyphShapeList.GetFirst();
    while(!VectorGlyphShapeList.IsNull(sh))
    {
        VectorGlyphShape* next = sh->pNext;
        if (sh->Key.pFont == font)
        {
            VectorGlyphShapeList.Remove(sh);
            VectorGlyphCache.Remove(sh);
        }
        sh = next;
    }
}

//------------------------------------------------------------------------
PrimitiveFill* GlyphCache::GetFill(TextLayerType type, unsigned textureId)
{
    switch(type)
    {
    case TextLayer_Background:
    case TextLayer_Selection:
        return pSolidFill;

    case TextLayer_Shadow:
    case TextLayer_ShadowText:
    case TextLayer_RasterText:
        return Textures[textureId].GetFill();

    case TextLayer_PackedText:
    case TextLayer_PackedDFAText:
    case TextLayer_Images:
        return 0;

    case TextLayer_Shapes:
    case TextLayer_Underline:
    case TextLayer_Cursor:
        return pSolidFill;

    case TextLayer_Mask:
        return pMaskFill;

    case TextLayer_Shapes_Masked:
    case TextLayer_Underline_Masked:
        return pSolidFill;
    }
    return 0;
}


//------------------------------------------------------------------------
Image* GlyphCache::GetImage(unsigned textureId)
{
    SF_ASSERT(textureId < MaxNumTextures);
    return Textures[textureId].GetImage();
}


//------------------------------------------------------------------------
void GlyphCache::partialUpdateTextures()
{
    SF_ASSERT(UpdateBuffer.GetPtr() != 0);

    ImageData d;
    UpdateBuffer->GetImageData(&d);

    unsigned i, j;
    for(i = 0; i < MaxNumTextures; ++i)
    {
        GlyphTextureMapper& tex = Textures[i];
        if (tex.NumGlyphsToUpdate)
        {
            RectsToUpdate.Resize(tex.NumGlyphsToUpdate, 32);
            unsigned numRects = 0;
            for (j = 0; j < (unsigned)GlyphsToUpdate.GetSize(); j++)
            {
                if (GlyphsToUpdate[j].TextureId == i)
                {
                    SF_ASSERT(numRects < (unsigned)RectsToUpdate.GetSize());
                    const UpdateRect& r = GlyphsToUpdate[j];
                    UpdateDesc& desc = RectsToUpdate[numRects++];
                    ImageData data;
                    UpdateBuffer->GetImageData(&data);
                    desc.DestRect.x1 = r.DstX;
                    desc.DestRect.y1 = r.DstY;
                    desc.DestRect.x2 = r.DstX + r.w;
                    desc.DestRect.y2 = r.DstY + r.h;
                    desc.SourcePlane = d.pPlanes[0];
                    desc.SourcePlane.pData = data.pPlanes[0].GetScanline(r.SrcY) + r.SrcX;
                    desc.PlaneIndex = 0;
                }
            }
            tex.Update(&RectsToUpdate[0], numRects);
// DBG
//printf("\nUpd %d Packed %d\n", numRects, SlotQueue.GetNumPacked());
            tex.NumGlyphsToUpdate = 0;
        }
    }
    GlyphsToUpdate.Clear();
    UpdatePacker.Clear();
}



//------------------------------------------------------------------------
void GlyphCache::copyImageData(ImagePlane* pl, const UByte* data, unsigned pitch, 
                               unsigned dstX, unsigned dstY, unsigned w, unsigned h)
{
    unsigned i;
    for (i = 0; i < h; ++i)
    {
        UByte* p = pl->GetScanline(dstY+i);
        memcpy(p+dstX, data, w);
        data += pitch;
    }
}

//------------------------------------------------------------------------
bool GlyphCache::updateTextureGlyph(const GlyphNode* node)
{
    SF_AMP_SCOPE_RENDER_TIMER("GlyphCache::UpdateTextureGlyph", Amp_Profile_Level_Medium);

    unsigned textureId = GetTextureId(node);
    const UByte* data  = &RasterData[0];
    unsigned     pitch = RasterPitch;
    unsigned     dstX  = node->mRect.x;
    unsigned     dstY  = node->mRect.y;
    unsigned     w     = node->mRect.w;
    unsigned     h     = node->mRect.h;

    if (!Textures[textureId].IsValid())
    {
        Textures[textureId].Create(Method, pHeap, pTexMan, pFillMan, this, textureId, 
                                   ImageSize(TextureWidth, TextureHeight));
    }

    pRQCaches->SetCacheLocked(Cache_Glyph);
    if (Method == TU_MultipleUpdate)
    {
        unsigned updX, updY;
        if (!UpdatePacker.Allocate(w, h, &updX, &updY))
        {
            partialUpdateTextures();
            if (!UpdatePacker.Allocate(w, h, &updX, &updY))
            {
                return false;
            }
        }
        ImageData d;
        UpdateBuffer->GetImageData(&d);
        copyImageData(d.pPlanes, data, pitch, updX, updY, w, h);
        Textures[textureId].NumGlyphsToUpdate++;
        UpdateRect r;
        r.SrcX = updX;
        r.SrcY = updY;
        r.DstX = dstX;
        r.DstY = dstY;
        r.w = w;
        r.h = h;
        r.TextureId = textureId;
        GlyphsToUpdate.PushBack(r);
        return true;
    }
    else
    {
        ImagePlane* p = Textures[textureId].Map();
        if (p)
        {
            copyImageData(p, data, pitch, dstX, dstY, w, h);
            return true;
        }
    }
    return false;
}

#ifdef SF_AMP_SERVER

class AmpFileWriter : public ImageFileWriter_Mixin<AmpFileWriter>
{
public:
    AmpFileWriter() : Version(0) { }
    void SetVersion(UInt32 version) { Version = version; }
    virtual bool Write(File* file, const ImageData& imageData, const ImageWriteArgs* args = 0) const
    {
        SF_UNUSED(args);
        imageData.Write(*file, Version);
        return true;
    }

    // Instance singleton.
    static AmpFileWriter Instance;
private:
    UInt32 Version;
};

AmpFileWriter AmpFileWriter::Instance;

// returns the number of textures serialized to the file
int GlyphCache::GetTextureData(File* dataFile, UInt32 version)
{
    int numTextures = 0;
    AmpFileWriter::Instance.SetVersion(version);
    pRQCaches->SetCacheLocked(Cache_Glyph);
    for (unsigned i = 0; i < MaxNumTextures; ++i)
    {
        if (Textures[i].IsValid())
        {
            Textures[i].LockImage();
            if (AmpFileWriter::WriteImage(dataFile, Textures[i].GetImage()))
            {
                ++numTextures;
            }
            Textures[i].UnlockImage();
        }
    }
    pRQCaches->ClearCacheLocked(Cache_Glyph);
    return numTextures;
}
#endif



//------------------------------------------------------------------------
void GlyphCache::UnlockBuffers()
{
    unsigned i;
    if (Method == TU_MultipleUpdate)
    {
        partialUpdateTextures();
    }
    for(i = 0; i < MaxNumTextures; ++i)
        Textures[i].Unmap();
    pRQCaches->ClearCacheLocked(Cache_Glyph);
}


//------------------------------------------------------------------------
void GlyphCache::releaseAllTextures()
{
    for(unsigned i = 0; i < MaxNumTextures; ++i)
        Textures[i].Invalidate();
    GlyphsToUpdate.Clear();
}


//------------------------------------------------------------------------
void GlyphCache::TextureLost(unsigned textureId, unsigned reason)
{
    SF_UNUSED(reason);
    Queue.CleanUpTexture(textureId);
    UPInt i, j;
    for(i = j = 0; i < GlyphsToUpdate.GetSize(); i++)
    {
        if (GlyphsToUpdate[i].TextureId != textureId)
            GlyphsToUpdate[j++] = GlyphsToUpdate[i];
    }
    GlyphsToUpdate.CutAt(j);
    Textures[textureId].Invalidate();
}



//------------------------------------------------------------------------
FontCacheHandle* GlyphCache::RegisterFont(Font* font)
{
    if ( checkInitialization() )
        return pFontHandleManager->RegisterFont(font);
    else
        return 0;
}


//------------------------------------------------------------------------
GlyphNode* GlyphCache::FindGlyph(TextMeshProvider* tm, const GlyphParam& gp)
{
    GlyphNode* node = Queue.FindGlyph(gp);
    if (node)
    {
        tm->AddNotifier(Queue.CreateNotifier(node, tm));
    }
    return node;
}

//------------------------------------------------------------------------
GlyphNode* GlyphCache::allocateGlyph(TextMeshProvider* tm, const GlyphParam& gp, unsigned w, unsigned h)
{
    GlyphNode* node = Queue.AllocateGlyph(gp, w, h);
    if (node)
    {
        tm->AddNotifier(Queue.CreateNotifier(node, tm));
    }
    else
    {
        SF_AMP_CODE(AmpServer::GetInstance().IncrementFontFailures();)
    }
    return node;
}


//------------------------------------------------------------------------
// The function can be called from EndFrame, when all the batches are processed 
// for sure. In average it should be more efficient than calling UnpinSlots()
// for each TextMeshProvider.
void GlyphCache::UnpinAllSlots()
{
    TextMeshProvider* tm = TextInUse.GetFirst();
    while(!TextInUse.IsNull(tm))
    {
        tm->ClearInList();
        tm = tm->pNext;
    }
    tm = TextInPin.GetFirst();
    while(!TextInPin.IsNull(tm))
    {
        tm->ClearInList();
        tm = tm->pNext;
    }
    TextInUse.Clear();
    TextInPin.Clear();
    Queue.UnpinAllSlots();
}

//------------------------------------------------------------------------
// Call-back from GlyphQueue. Occurs when the slot releases. The function
// is called for each TextMeshProvider associated with a particular slot in GlyphQueue.
void GlyphCache::EvictText(TextMeshProvider* tm)
{
    tm->OnEvictSlots();
}

//------------------------------------------------------------------------
// Call-back from GlyphQueue. Occurs before eviction to apply and pin all slots 
// in batches that are in process.
void GlyphCache::ApplyInUseList()
{
    TextMeshProvider* tm = TextInUse.GetFirst();
    while(!TextInUse.IsNull(tm))
    {
        SF_ASSERT(tm->IsInUseList());
        TextMeshProvider* next = tm->pNext;
        tm->SetInUseList(false);
        tm->PinSlots();
        TextInPin.PushBack(tm);
        tm->SetInPinList(true);
        tm = next;
    }
    TextInUse.Clear();
}

//------------------------------------------------------------------------
// Call-back from GlyphQueue. Occurs when an attempt to evict a slot
// failed. This function iterates through the "pin list", verifies if the 
// batch is in process, and if not, unpins the slots, and removes the batch
// from the list. Then GlyphQueue makes a second attempt to allocate a slot.
bool GlyphCache::UpdatePinList()
{
    bool ret = false;
    TextMeshProvider* tm = TextInPin.GetFirst();
    while(!TextInPin.IsNull(tm))
    {
        SF_ASSERT(tm->IsInPinList());
        TextMeshProvider* next = tm->pNext;
        if (!tm->IsInUse())
        {
            tm->SetInPinList(false);
            tm->UnpinSlots();
            tm->RemoveNode();
            ret = true;
        }
        tm = next;
    }
    return ret;
}




//-----------------------------------------------------------------------
void GlyphCache::getGlyphBounds(VectorGlyphShape* glyphShape, const ShapeDataInterface* shapeData) const
{
    if (glyphShape->Key.HintedVector == 0)
    {
        glyphShape->Key.pFont->pFont->GetGlyphBounds(glyphShape->Key.GlyphIndex, &glyphShape->Bounds);
        if (glyphShape->Bounds.x1 >= glyphShape->Bounds.x2 || glyphShape->Bounds.y1 >= glyphShape->Bounds.y2)
        {
            if (!shapeData->IsEmpty())
                glyphShape->Bounds = ComputeBoundsFill(*shapeData, Matrix2F(), Bound_AllEdges);
        }
    }
    else
    {
        if (!shapeData->IsEmpty())
            glyphShape->Bounds = ComputeBoundsFill(*shapeData, Matrix2F(), Bound_AllEdges);
        else
            glyphShape->Key.pFont->pFont->GetGlyphBounds(glyphShape->Key.GlyphIndex, &glyphShape->Bounds);
    }
}


//------------------------------------------------------------------------
bool GlyphCache::isOuterContourCW(const ShapeDataInterface* shape) const
{
    float minX1 =  1e10f;
    float minY1 =  1e10f;
    float maxX1 = -1e10f;
    float maxY1 = -1e10f;
    float minX2 =  1e10f;
    float minY2 =  1e10f;
    float maxX2 = -1e10f;
    float maxY2 = -1e10f;
    bool cw = true;

    ShapePosInfo srcPos(shape->GetStartingPos());
    ShapePathType pathType;
    PathEdgeType pathEdge;
    float coord[Edge_MaxCoord];
    unsigned styles[3];
    bool first = true;
    while((pathType = shape->ReadPathInfo(&srcPos, coord, styles)) != Shape_EndShape)
    {
        if (!first && pathType == Shape_NewLayer)
            break;
        first = false;

        if (styles[0] != styles[1])
        {
            float sum = 0;
            float firstX = coord[0];
            float firstY = coord[1];
            float prevX = coord[0];
            float prevY = coord[1];
            while((pathEdge = shape->ReadEdge(&srcPos, coord)) != Edge_EndPath)
            {
                if(coord[0] < minX1) minX1 = coord[0];
                if(coord[1] < minY1) minY1 = coord[1];
                if(coord[0] > maxX1) maxX1 = coord[0];
                if(coord[1] > maxY1) maxY1 = coord[1];
                sum += prevX * coord[1] - prevY * coord[0];
                prevX = coord[0];
                prevY = coord[1];

                if (pathEdge == Edge_QuadTo)
                {
                    // TO DO: Possibly process curves it in a more intelligent way, but most probably
                    // this simplest method is good enough in practice.
                    if(coord[2] < minX1) minX1 = coord[2];
                    if(coord[3] < minY1) minY1 = coord[3];
                    if(coord[2] > maxX1) maxX1 = coord[2];
                    if(coord[3] > maxY1) maxY1 = coord[3];
                    sum += prevX * coord[3] - prevY * coord[2];
                    prevX = coord[2];
                    prevY = coord[3];
                }
            }

            if (prevX != firstX || prevY != firstY)
            {
                if(firstX < minX1) minX1 = firstX;
                if(firstY < minY1) minY1 = firstY;
                if(firstX > maxX1) maxX1 = firstX;
                if(firstY > maxY1) maxY1 = firstY;
                sum += prevX * firstY - prevY * firstX;
            }

            if(minX1 < minX2 || minY1 < minY2 || maxX1 > maxX2 || maxY1 > maxY2)
            {
                minX2 = minX1;
                minY2 = minY1;
                maxX2 = maxX1;
                maxY2 = maxY1;
                cw = sum > 0;
            }
        }
        else
        {
            shape->SkipPathData(&srcPos);
        }
    }
    return cw;
}


//-----------------------------------------------------------------------
void GlyphCache::copyAndTransformShape(VectorGlyphShape* glyphShape, const ShapeDataInterface* srcShape, 
                                       bool fauxBold, bool fauxItalic, unsigned outline, 
                                       float italicOffset, float nominalSize)
{
    if (srcShape->IsEmpty())
        return;

    float tolerance = nominalSize / 64;

    // TO DO: Get from Renderer2D and possibly tweak.
    ToleranceParams param;
    param.CurveTolerance *= tolerance;
    param.CollinearityTolerance *= tolerance;

    Matrix2F italic; 
    if (fauxItalic)
    {
        italic.AppendTranslation(0, +italicOffset);
        italic.AppendShearing(0, -Param.FauxItalicAngle);
        italic.AppendTranslation(0, -italicOffset);
    }

    ShapePosInfo srcPos(srcShape->GetStartingPos());
    ShapePosInfo dstPos(0);
    ShapePathType pathType;
    PathEdgeType pathEdge;
    float coord[Edge_MaxCoord];
    unsigned styles[3];
    bool first = true;

    if (fauxBold || outline)
    {
        const float verScale = 1000;
        bool cw = isOuterContourCW(srcShape);
        mStroker.Clear();
        TmpPath1.Clear();
        float fauxBoldWidth = fauxBold ? Param.FauxBoldRatio * nominalSize : 0.0f;
        mStroker.SetWidth(fauxBoldWidth);
        StrokeScaler scaler(mStroker, 1, verScale);
        while((pathType = srcShape->ReadPathInfo(&srcPos, coord, styles)) != Shape_EndShape)
        {
            if (!first && pathType == Shape_NewLayer)
                break;
            first = false;

            if (styles[0] != styles[1])
            {
                italic.Transform(&coord[0], &coord[1]);
                scaler.AddVertex(coord[0], coord[1]);
                while((pathEdge = srcShape->ReadEdge(&srcPos, coord)) != Edge_EndPath)
                {
                    if (pathEdge == Edge_LineTo)
                    {
                        italic.Transform(&coord[0], &coord[1]);
                        scaler.AddVertex(coord[0], coord[1]);
                    }
                    else
                    {
                        italic.Transform(&coord[0], &coord[1]);
                        italic.Transform(&coord[2], &coord[3]);
                        TessellateQuadCurve(&scaler, param, coord[0], coord[1], coord[2], coord[3]);
                    }
                }
                mStroker.ClosePath();
                mStroker.CalcEquidistant(&TmpPath1, cw ? Stroker::DirForward : Stroker::DirBackward);
            }
            else
            {
                srcShape->SkipPathData(&srcPos);
            }
        }
        TmpPath1.Scale(1, 1/verScale);
        const VertexPath* vp = &TmpPath1;

        unsigned i, j;
        if (outline)
        {
            TmpPath2.Clear();
            mStroker.Clear();
            mStroker.SetWidth(float(outline) * Param.OutlineRatio * nominalSize);
            mStroker.SetLineJoin(StrokerTypes::MiterJoin);

            for (i = 0; i < TmpPath1.GetNumPaths(); ++i)
            {
                const PathBasic& path = TmpPath1.GetPath(i);
                if (path.Count > 2)
                {
                    for (j = 0; j < path.Count; ++j)
                    {
                        const VertexBasic& v2 = TmpPath1.GetVertex(path.Start + j);
                        mStroker.AddVertex(v2.x, v2.y);
                    }
                    mStroker.ClosePath();
                    mStroker.GenerateStroke(&TmpPath2);
                }
            }
            vp = &TmpPath2;
        }

        unsigned np = 0;
        for (i = 0; i < vp->GetNumPaths(); ++i)
        {
            const PathBasic& path = vp->GetPath(i);
            if (path.Count > 2)
            {
                const VertexBasic& v1 = vp->GetVertex(path.Start);
                glyphShape->pShape->StartPath(&dstPos, (np == 0) ? Shape_NewLayer : Shape_NewPath, 1, 0, 0, v1.x, v1.y);
                for (j = 1; j < path.Count; ++j)
                {
                    const VertexBasic& v2 = vp->GetVertex(path.Start + j);
                    glyphShape->pShape->LineTo(&dstPos, v2.x, v2.y);
                }
                glyphShape->pShape->ClosePath(&dstPos);
                glyphShape->pShape->EndPath();
                ++np;
            }
        }
        glyphShape->pShape->EndShape();
        mStroker.Clear();
        TmpPath1.Clear();
        TmpPath2.Clear();
        LHeap1.ClearAndRelease();
        LHeap2.ClearAndRelease();
        return;
    }

    while((pathType = srcShape->ReadPathInfo(&srcPos, coord, styles)) != Shape_EndShape)
    {
        if (!first && pathType == Shape_NewLayer)
            break;
        first = false;

        if (styles[0] != styles[1])
        {
            italic.Transform(&coord[0], &coord[1]);
            glyphShape->pShape->StartPath(&dstPos, pathType, styles[0], styles[1], styles[2], coord[0], coord[1]);
            while((pathEdge = srcShape->ReadEdge(&srcPos, coord)) != Edge_EndPath)
            {
                if (pathEdge == Edge_LineTo)
                {
                    italic.Transform(&coord[0], &coord[1]);
                    glyphShape->pShape->LineTo(&dstPos, coord[0], coord[1]);
                }
                else
                {
                    italic.Transform(&coord[0], &coord[1]);
                    italic.Transform(&coord[2], &coord[3]);
                    glyphShape->pShape->QuadTo(&dstPos, coord[0], coord[1], coord[2], coord[3]);
                }
            }
            glyphShape->pShape->ClosePath(&dstPos);
            glyphShape->pShape->EndPath();
        }
        else
        {
            srcShape->SkipPathData(&srcPos);
        }
    }
    glyphShape->pShape->EndShape();
}


//-----------------------------------------------------------------------
VectorGlyphShape* GlyphCache::CreateGlyphShape(GlyphRunData& data, unsigned glyphIndex, float screenSize, 
                                               bool fauxBold, bool fauxItalic, unsigned outline, 
                                               bool needsVectorShape)
{
    Font* font = data.pFont;
    data.VectorSize = 0;
    data.RasterSize = 0;
    data.pShape     = 0;
    data.pRaster    = 0;
    data.GlyphBounds= RectF(0,0,0,0);
    data.NomHeight  = font->GetNominalGlyphHeight();
    data.NomWidth   = font->GetNominalGlyphWidth();

    if (Font::IsMissingGlyph(glyphIndex))
        return 0;

    if (!font->HasNativeHinting())
        screenSize = 0;

    unsigned hintedVector = 0;
    unsigned hintedRaster = 0;
    if (screenSize)
    {
        if (font->IsHintedRasterGlyph(glyphIndex, unsigned(screenSize)))
            hintedRaster = unsigned(screenSize);

        if (font->IsHintedVectorGlyph(glyphIndex, unsigned(screenSize)))
            hintedVector = unsigned(screenSize);
    }

    unsigned flags = (fauxBold   ? Font::FF_Bold : 0) |
                     (fauxItalic ? Font::FF_Italic : 0);

    if (flags == 0 && hintedVector == 0 && hintedRaster == 0 && outline == 0 && !needsVectorShape)
    {
        data.pShape = font->GetPermanentGlyphShape(glyphIndex);
        if (data.pShape)
        {
            font->GetGlyphBounds(glyphIndex, &data.GlyphBounds);
            return 0;
        }
    }

    Ptr<VectorGlyphShape>* shapePtr = 
        VectorGlyphCache.GetAlt(VectorGlyphKey(data.pFontHandle, glyphIndex, 
                                               hintedVector, hintedRaster, 
                                               flags, outline));
    if (shapePtr)
    {
        VectorGlyphShapeList.SendToBack(*shapePtr);
        data.VectorSize = (*shapePtr)->Key.HintedVector;
        data.RasterSize = (*shapePtr)->Key.HintedRaster;
        data.pShape     = (*shapePtr)->pShape;
        data.pRaster    = (*shapePtr)->pRaster;
        data.GlyphBounds= (*shapePtr)->Bounds;
        (*shapePtr)->AddRef();
        return *shapePtr;
    }

    VectorGlyphShape* shape = 0;

    if (VectorGlyphCache.GetSize() > Param.MaxVectorCacheSize)
    {
        shape = VectorGlyphShapeList.GetFirst();
        unsigned n = (unsigned)VectorGlyphCache.GetSize() - Param.MaxVectorCacheSize;
        if (n > Param.MaxVectorCacheSize)
            n = Param.MaxVectorCacheSize;
        for(unsigned i = 0; i < n && !VectorGlyphShapeList.IsNull(shape); ++i)
        {
            VectorGlyphShape* next = shape->pNext;
            if (shape->IsEmpty())
            {
                VectorGlyphShapeList.Remove(shape);
                VectorGlyphCache.Remove(shape);
            }
            shape = next;
        }
    }


    shape = SF_HEAP_NEW(pHeap) VectorGlyphShape(this);

    shape->Key.pFont        = data.pFontHandle;
    shape->Key.GlyphIndex   = (UInt16)glyphIndex;
    shape->Key.HintedVector = (UInt8)hintedVector;
    shape->Key.HintedRaster = (UInt8)hintedRaster;
    shape->Key.Flags        = (UInt8)flags;
    shape->Key.Outline      = (UInt8)outline;
    shape->pShape           = *SF_HEAP_NEW(pHeap) GlyphShape;
    if (hintedRaster)
        shape->pRaster      = *SF_HEAP_NEW(pHeap) GlyphRaster;

    const ShapeDataInterface* srcShape = 0;
    if (hintedVector == 0)
        srcShape = font->GetPermanentGlyphShape(glyphIndex);

    float italicOffset = font->GetDescent();
    if (srcShape)
    {
        font->GetGlyphBounds(glyphIndex, &shape->Bounds);
        copyAndTransformShape(shape, srcShape, fauxBold, fauxItalic, outline, italicOffset, data.NomHeight);
    }
    else
    {
        if (fauxBold == 0 && fauxItalic == 0 && outline == 0)
        {
            shape->pShape->Clear();
            font->GetTemporaryGlyphShape(glyphIndex, hintedVector, shape->pShape);
            getGlyphBounds(shape, shape->pShape);
        }
        else
        {
            Ptr<GlyphShape> tmpShape = *SF_HEAP_NEW(pHeap) GlyphShape;
            font->GetTemporaryGlyphShape(glyphIndex, hintedVector, tmpShape);
            getGlyphBounds(shape, tmpShape);
            copyAndTransformShape(shape, tmpShape, fauxBold, fauxItalic, outline, italicOffset, data.NomHeight);
        }
    }


    if (hintedRaster)
    {
        SF_ASSERT(shape->pRaster.GetPtr());
        font->GetGlyphRaster(glyphIndex, hintedRaster, shape->pRaster);
        shape->pRaster->HintedSize = hintedRaster;
    }

    data.GlyphBounds= shape->Bounds;
    data.VectorSize = hintedVector;
    data.RasterSize = hintedRaster;
    data.pShape     = shape->pShape;
    data.pRaster    = shape->pRaster;
    data.HintedNomHeight = shape->pShape->GetHintedSize();
    VectorGlyphShapeList.PushBack(shape);
    VectorGlyphCache.Add(shape);
    return shape;
}






//------------------------------------------------------------------------
class ImgBlurWrapperX 
{
public:
    ImgBlurWrapperX(UByte* img, unsigned pitch, unsigned x, unsigned y, unsigned w, unsigned h) :
        Img(img), Pitch(pitch), Sx(x), Sy(y), W(w), H(h) {}

    unsigned GetWidth()  const { return W; }
    unsigned GetHeight() const { return H; }
    UInt8 GetPixel(unsigned x, unsigned y) const { return Img[Pitch*(Sy+y) + Sx+x]; }

    void CopySpanTo(unsigned x, unsigned y, unsigned len, const UInt8* buf)
    {
        memcpy(Img + Pitch*(Sy+y) + Sx+x, buf, len);
    }

private:
    ImgBlurWrapperX(const ImgBlurWrapperX&);
    const ImgBlurWrapperX& operator = (const ImgBlurWrapperX&);
    UByte* Img;
    unsigned Pitch;
    unsigned Sx, Sy, W, H;
};


//------------------------------------------------------------------------
class ImgBlurWrapperY
{
public:
    ImgBlurWrapperY(UByte* img, unsigned pitch, unsigned x, unsigned y, unsigned w, unsigned h) :
        Img(img), Pitch(pitch), Sx(x), Sy(y), W(w), H(h) {}

    unsigned  GetWidth()  const { return H; }
    unsigned  GetHeight() const { return W; }
    UInt8 GetPixel(unsigned x, unsigned y) const { return Img[Pitch*(Sy+x) + Sx+y]; }

    void CopySpanTo(unsigned x, unsigned y, unsigned len, const UInt8* buf)
    {
        UInt8* p = Img + Pitch*(Sy+x) + Sx+y;
        do
        {
            *p  = *buf++;
             p += Pitch;
        }
        while(--len);
    }

private:
    ImgBlurWrapperY(const ImgBlurWrapperY&);
    const ImgBlurWrapperY& operator = (const ImgBlurWrapperY&);
    UByte* Img;
    unsigned Pitch;
    unsigned Sx, Sy, W, H;
};


//------------------------------------------------------------------------
template<class Img, class SumBuf, class ColorBuf>
void RecursiveBlur(Img& img, float radius, SumBuf& sum, ColorBuf& buf)
{
    if (radius < 0.62f) 
        radius = 0.62f;

    int w   = img.GetWidth();
    int h   = img.GetHeight();
    int pad = int(ceilf(radius)) + 3;
    int x, y;

    float s = radius * 0.5f;
    float q = (s < 2.5f) ?
               3.97156f - 4.14554f * sqrtf(1 - 0.26891f * s) :
               0.98711f * s - 0.96330f;

    float q2 = q  * q;
    float q3 = q2 * q;

    float b0 =  1.0f / (1.578250f + 2.444130f * q + 1.428100f * q2 + 0.422205f * q3);
    float b2 =  2.44413f * q + 2.85619f * q2 + 1.26661f * q3;
    float b3 = -1.42810f * q2 + -1.26661f * q3;
    float b4 =  0.422205f * q3;

    float b1  = 1.0f - (b2 + b3 + b4) * b0;

    b2 *= b0;
    b3 *= b0;
    b4 *= b0;

    sum.Resize(w + 2*pad);
    buf.Resize(w + 2*pad);

    for(y = 0; y < h; y++)
    {
        // C++ 17 remove register
        // register float a1=0, a2=0, a3=0, a4=0;
        float a1 = 0, a2 = 0, a3 = 0, a4 = 0;

        for(x = 0; x < pad; ++x)
            sum[x] = 0;

        for(x = 0; x < w; ++x)
        {
            a1 = sum[x+pad] = b1*img.GetPixel(x, y) + b2*a2 + b3*a3 + b4*a4;
            a4 = a3;
            a3 = a2;
            a2 = a1;
        }

        for(x = 0; x < pad; ++x)
        {
            a1 = sum[x+w+pad] = b2*a2 + b3*a3 + b4*a4;
            a4 = a3;
            a3 = a2;
            a2 = a1;
        }

        a1 = a2 = a3 = a4 = 0;

        for(x = w+pad+pad-1; x >= pad; --x)
        {
            a1 = b1*sum[x] + b2*a2 + b3*a3 + b4*a4;
            a4 = a3;
            a3 = a2;
            a2 = a1;
            buf[x] = UInt8(a1 + 0.5f);
        }
        img.CopySpanTo(0, y, w, &buf[pad]);
    }
}



//------------------------------------------------------------------------
void GlyphCache::recursiveBlur(UByte* img, unsigned pitch,
                               unsigned  sx, unsigned  sy, 
                               unsigned  w,  unsigned  h, 
                               float rx, float ry)
{
    ImgBlurWrapperX imgWX(img, pitch, sx, sy, w, h); 
    RecursiveBlur(imgWX, rx, BlurSum, BlurStack);
    ImgBlurWrapperY imgWY(img, pitch, sx, sy, w, h); 
    RecursiveBlur(imgWY, ry, BlurSum, BlurStack);
}


//------------------------------------------------------------------------
void GlyphCache::strengthenImage(UByte* img, unsigned pitch, 
                                 unsigned  sx, unsigned  sy, 
                                 unsigned  w,  unsigned  h, 
                                 float ratio,
                                 int  bias)
{
    if (ratio != 1.0f)
    {
        unsigned x, y;
        for(y = 0; y < h; ++y)
        {
            UInt8* p = img + (sy + y)*pitch + sx;
            for(x = 0; x < w; ++x)
            {
                int v = int((int(*p) - bias) * ratio + 0.5f) + bias;
                if (v <   0) v = 0;
                if (v > 255) v = 255;
                *p++ = UInt8(v);
            }
        }
    }
}

//------------------------------------------------------------------------
void GlyphCache::knockOut(UByte* raster)
{
    const UInt8* src = &KnockOutCopy[0];
          UInt8* dst = raster;

    for (UPInt i = 0; i < KnockOutCopy.GetSize(); ++i)
    {
        unsigned s = 255 - *src++;
        unsigned d = *dst;
        *dst++ = UInt8((s * d + 255) >> 8);
    }
}



//-----------------------------------------------------------------------
void GlyphCache::addShapeToRasterizer(const ShapeDataInterface* shape, float scaleX, float scaleY)
{
    if (shape->IsEmpty())
        return;

    // TO DO: Get from Renderer2D and possibly tweak.
    ToleranceParams param;

    ShapePosInfo pos(shape->GetStartingPos());
    ShapePathType pathType;
    PathEdgeType pathEdge;
    float coord[Edge_MaxCoord];
    unsigned styles[3];
    bool first = true;
    while((pathType = shape->ReadPathInfo(&pos, coord, styles)) != Shape_EndShape)
    {
        if (!first && pathType == Shape_NewLayer)
            break;
        first = false;

        if (styles[0] != styles[1])
        {
            coord[0] *= scaleX;
            coord[1] *= scaleY;
            Ras.MoveTo(coord[0], coord[1]);
            while((pathEdge = shape->ReadEdge(&pos, coord)) != Edge_EndPath)
            {
                if (pathEdge == Edge_LineTo)
                {
                    coord[0] *= scaleX;
                    coord[1] *= scaleY;
                    Ras.LineTo(coord[0], coord[1]);
                }
                else
                {
                    coord[0] *= scaleX;
                    coord[1] *= scaleY;
                    coord[2] *= scaleX;
                    coord[3] *= scaleY;
                    TessellateQuadCurve(&Ras, param, coord[0], coord[1], coord[2], coord[3]);
                }
            }
            Ras.ClosePath();
        }
        else
        {
            shape->SkipPathData(&pos);
        }
    }
}



//-----------------------------------------------------------------------
void GlyphCache::addShapeAutoFit(const ShapeDataInterface* shape, unsigned nomHeight, 
                                 int lowerCaseTop, int upperCaseTop,
                                 float screenSize, float stretch)
{
    Fitter.Clear();

    if (shape->IsEmpty())
        return;

    int nominalSize = int(screenSize * 64);
    if (nominalSize > 2048)
        nominalSize = 2048;

    Fitter.SetNominalFontHeight(nominalSize);
    float scale = float(nominalSize) / float(nomHeight);

    float tolerance = 0.5f * float(nominalSize) / screenSize;

    // TO DO: Get from Renderer2D and possibly tweak.
    ToleranceParams param;
    param.CurveTolerance *= tolerance;
    param.CollinearityTolerance *= tolerance;

    ShapePosInfo pos(shape->GetStartingPos());
    ShapePathType pathType;
    PathEdgeType pathEdge;
    float coord[Edge_MaxCoord];
    unsigned styles[3];
    bool first = true;
    while((pathType = shape->ReadPathInfo(&pos, coord, styles)) != Shape_EndShape)
    {
        if (!first && pathType == Shape_NewLayer)
            break;
        first = false;

        if (styles[0] != styles[1])
        {
            coord[0] *=  scale;
            coord[1] *= -scale;
            Fitter.MoveTo(coord[0], coord[1]);
            while((pathEdge = shape->ReadEdge(&pos, coord)) != Edge_EndPath)
            {
                if (pathEdge == Edge_LineTo)
                {
                    coord[0] *=  scale;
                    coord[1] *= -scale;
                    Fitter.LineTo(coord[0], coord[1]);
                }
                else
                {
                    coord[0] *=  scale;
                    coord[1] *= -scale;
                    coord[2] *=  scale;
                    coord[3] *= -scale;
                    TessellateQuadCurve(&Fitter, param, coord[0], coord[1], coord[2], coord[3]);
                }
            }
            Fitter.ClosePath();
        }
        else
        {
            shape->SkipPathData(&pos);
        }
    }

    Fitter.FitGlyph(int(screenSize), 0, int(lowerCaseTop * scale), int(upperCaseTop * scale));

    scale = 1.0f / Fitter.GetUnitsPerPixelY();

    unsigned i, j;
    for(i = 0; i < Fitter.GetNumContours(); ++i)
    {
        const GlyphFitter::ContourType& c = Fitter.GetContour(i);
        if(c.NumVertices > 2)
        {
            GlyphFitter::VertexType v = Fitter.GetVertex(c, 0);
            Fitter.SnapVertex(v);

            Ras.MoveTo(v.x * scale * stretch, -v.y * scale);
            for(j = 1; j < c.NumVertices; ++j)
            {
                v = Fitter.GetVertex(c, j);
                Fitter.SnapVertex(v);
                Ras.LineTo(v.x * scale * stretch, -v.y * scale);
            }
            Ras.ClosePath();
        }
    }
    Fitter.Clear();
}



//-----------------------------------------------------------------------
float GlyphCache::SnapFontSizeToRamp(float screenSize) const
{
    unsigned size = unsigned(floor(screenSize));
    size = size + ((size + 3) >> 2);
    size =(size <= 255) ? FontSizeRamp[FontSizeMap[size] + 1] : 255;
    return (float)size;
}

//-----------------------------------------------------------------------
float GlyphCache::SnapShadowSizeToRamp(float screenSize) const
{
    unsigned size = unsigned(floor(screenSize));
    size = (size <= 255) ? FontSizeRamp[FontSizeMap[size] + 1] : 255;
    return (float)size;
}

//-----------------------------------------------------------------------
bool GlyphCache::GlyphFits(unsigned screenHeight) const
{
    return screenHeight < MaxSlotHeight * Param.MaxRasterScale;
}

//-----------------------------------------------------------------------
float GlyphCache::GetCachedFontSize(const GlyphParam& gp, float screenSize, bool exactFit) const
{
    if (exactFit || gp.IsOptRead())
    {
        // Snap the font size to 1/snapTo of a pixel. It results in less cache load, 
        // with a fair glyph sharpness.
        if (!exactFit)
        {
            const float snapTo = 0.25f;
            screenSize = (float)(floor(screenSize/snapTo + 0.5f)) * snapTo;
        }
    }
    else
    {
        screenSize = SnapFontSizeToRamp(screenSize);
    }

    float maxHeight = float(MaxSlotHeight-2*SlotPadding);
    if (screenSize <= maxHeight * Param.MaxRasterScale)
    {
        return (screenSize <= maxHeight) ? screenSize : maxHeight;
    }
    return screenSize;
}


//-----------------------------------------------------------------------
float GlyphCache::GetCachedShadowSize(float screenSize, const GlyphRaster* ras) const
{
    if (ras)
        return float(ras->HintedSize);

    screenSize = SnapShadowSizeToRamp(screenSize);
    return (screenSize <= float(MaxSlotHeight-2*SlotPadding)) ? screenSize : float(MaxSlotHeight-2*SlotPadding); // TO DO: Revise. Temporary solution for big glyphs
}

//-----------------------------------------------------------------------
GlyphNode* GlyphCache::getPrerasterizedGlyph(GlyphRunData& data, TextMeshProvider* tm, const GlyphParam& gp)
{
    SF_ASSERT(data.pRaster);

    const GlyphRaster* raster = data.pRaster;

    int y1 = -raster->OriginY;
    int y2 = y1 + raster->Height;
    unsigned h = unsigned(y2 - y1) + 2*SlotPadding;

    if (h >= MaxSlotHeight && !data.pShape->IsEmpty())
    {
        Result = Res_ShapeIsTooBig;
        return 0;
    }

    int padX = SlotPadding;
    int padY = SlotPadding;

    int imgX1 = -raster->OriginX - padX;
    int imgX2 = -raster->OriginX + raster->Width + padX;
    int imgY1 = -raster->OriginY - padY;
    int imgY2 = -raster->OriginY + raster->Height + padY;

    unsigned imgW  = imgX2 - imgX1 + 1;
    unsigned imgH  = imgY2 - imgY1 + 1;

    if (imgH > MaxSlotHeight)
    {
        imgH = MaxSlotHeight;
        rasterTooBigWarning();
    }

    GlyphNode* node = allocateGlyph(tm, gp, imgW, imgH);
    if (node == 0)
    {
        Result = Res_CacheFull;
        cacheFullWarning();
        return 0;
    }

    node->Origin.x = SInt16(imgX1 * 16);
    node->Origin.y = SInt16(imgY1 * 16);
    node->Scale = 1;

    RasterData.Resize(imgW * imgH);
    RasterPitch = imgW;

    memset(&RasterData[0], 0, imgW * imgH);

    unsigned i;
    unsigned numSl = 0;
    for(i = 0; i < raster->Height && (padY+i) < imgH; ++i)
    {
        UByte* sl = &RasterData[(padY+i)*RasterPitch + padX];
        memcpy(sl, &raster->Raster[i * raster->Width], raster->Width);
        ++numSl;
    }

    updateTextureGlyph(node); 

    ++RasterizationCount;
    return node;
}



//-----------------------------------------------------------------------
void GlyphCache::filterScanline(UByte* sl, unsigned w) const
{
    unsigned i;
    UByte buf[256];
    UByte* src = sl;
    UByte* dst = buf;
    if (w > 256) w = 256;
    memset(buf, 0, w);
    for(i = 2; i+2 < w; ++i)
    {
        ScanlineFilter.Filter(src++, dst++);
    }
    memcpy(sl, buf, w);
}


//-----------------------------------------------------------------------
GlyphNode* GlyphCache::RasterizeGlyph(GlyphRunData& data, TextMeshProvider* tm, const GlyphParam& gp)
{
    if (MaxNumTextures == 0)
    {
        Result = Res_NoRasterCache;
        return 0;
    }

    if (data.RasterSize)
    {
        return getPrerasterizedGlyph(data, tm, gp);
    }

    if (data.pShape == 0)
    {
        Result = Res_ShapeNotFound;
        return 0;
    }

    int   lowerCaseTop = 0;
    int   upperCaseTop = 0;
    bool  autoFit = Param.UseAutoFit && gp.IsAutoFit();

    float stretch = gp.GetStretch(); 

    if (autoFit)
    {
        lowerCaseTop = gp.pFont->pFont->GetLowerCaseTop(this);
        upperCaseTop = gp.pFont->pFont->GetUpperCaseTop(this);
        if (lowerCaseTop == 0 || upperCaseTop == 0)
            autoFit = false;
    }

    float nomHeight = data.NomHeight;
    if (data.HintedNomHeight)
    {
        nomHeight = (float)data.HintedNomHeight;
        autoFit = false;
    }

    float scale = gp.GetFontSize() / nomHeight;
    float y1 = floorf(data.GlyphBounds.y1) * scale;
    float y2 =  ceilf(data.GlyphBounds.y2) * scale;

    if (y1 >= y2)
        y1 = y2 = 0;

    unsigned h = unsigned(y2 - y1) + 2*SlotPadding;

    if (h >= MaxSlotHeight)
    {
        Result = Res_ShapeIsTooBig;
        return 0;
    }

    GlyphNode* node = 0;

    // Rasterization
    Ras.Clear();
    if (autoFit)
        addShapeAutoFit(data.pShape, (unsigned)data.NomHeight, lowerCaseTop, upperCaseTop, gp.GetFontSize(), stretch);
    else
        addShapeToRasterizer(data.pShape, scale*stretch, scale);

    int padX = SlotPadding;
    int padY = SlotPadding;

    int imgX1 = 0;
    int imgX2 = 0;
    int imgY1 = 0;
    int imgY2 = 0;

    if (Ras.SortCells())
    {
        imgX1 = Ras.GetMinX() - padX;
        imgX2 = Ras.GetMaxX() + padX;
        imgY1 = Ras.GetMinY() - padY;
        imgY2 = Ras.GetMaxY() + padY;
    }

    unsigned imgW  = imgX2 - imgX1 + 1;
    unsigned imgH  = imgY2 - imgY1 + 1;

    if (imgH > MaxSlotHeight)
        imgH = MaxSlotHeight;
    //SF_ASSERT(imgH <= MaxSlotHeight);

    node = allocateGlyph(tm, gp, imgW, imgH);
    if (node == 0)
    {
        Result = Res_CacheFull;
        cacheFullWarning();
        return 0;
    }

    node->Origin.x = SInt16(imgX1 * 16);
    node->Origin.y = SInt16(imgY1 * 16);
    node->Scale = 1;

    RasterData.Resize(imgW * imgH);
    RasterPitch = imgW;

    memset(&RasterData[0], 0, imgW * imgH);

// DBG
//memset(&RasterData[(padY + 0)     *RasterPitch] + padX, 0xFF, imgW - padX*2);
//memset(&RasterData[(imgH - padY*2)*RasterPitch] + padX, 0xFF, imgW - padX*2);
//for(int i = padY; i < imgH-padY*2; ++i)
//{
//    UByte* sl = &RasterData[i*RasterPitch];
//    sl[padX] = 0xFF;
//    sl[imgW-padX] = 0xFF;
//}

    if (Ras.GetGamma1() != 1.0f)
        Ras.SetGamma1(1.0f);

    bool filter = imgW >= 5 && stretch > 1;

    unsigned i;
    unsigned numSl = 0;
    for(i = 0; i < Ras.GetNumScanlines() && (padY+i) < imgH; ++i)
    {
        UByte* sl = &RasterData[(padY+i)*RasterPitch];
        Ras.SweepScanline(i, sl+padX, 1, 0);
        if (filter)
            filterScanline(sl, imgW);
        ++numSl;
    }

    updateTextureGlyph(node); 
    ++RasterizationCount;


    Ras.Clear();
    return node;
}



//-----------------------------------------------------------------------
GlyphNode* GlyphCache::createShadowFromRaster(GlyphRunData& data, TextMeshProvider* tm, const GlyphParam& gp, float screenSize, const GlyphRaster* ras)
{
    float k = gp.GetFontSize() / screenSize;
    float blurX = gp.GetBlurX() * k * data.HeightRatio;
    float blurY = gp.GetBlurY() * k * data.HeightRatio;

    SF_ASSERT(ShadowQuality <= 1.0f && ShadowQuality >= 0.25f);
    float maxH = float(MaxSlotHeight) * ShadowQuality - float(2*SlotPadding);

    float rasterScale = 1;
    float h = float(ras->Height + 2*blurY);

    if (h >= maxH)
    {
        k = maxH / h;
        blurX *= k;
        blurY *= k;
        rasterScale = k;
    }

    int intBlurX = int(ceil(blurX));
    int intBlurY = int(ceil(blurY));

    int padX = SlotPadding + intBlurX;
    int padY = SlotPadding + intBlurY;

    unsigned imgW = ras->Width  + 2*padX + 1;
    unsigned imgH = ras->Height + 2*padY + 1;

    unsigned imgWs = unsigned(ceilf(float(imgW) * rasterScale));
    unsigned imgHs = unsigned(ceilf(float(imgH) * rasterScale));

    if (imgHs > MaxSlotHeight)
        imgHs = MaxSlotHeight;

    GlyphNode* node = allocateGlyph(tm, gp, imgWs, imgHs);
    if (node == 0)
    {
        Result = Res_CacheFull;
        cacheFullWarning();
        return 0;
    }

    node->Origin.x = SInt16(float(-ras->OriginX*16 - padX*16) * imgWs / imgW);
    node->Origin.y = SInt16(float(-ras->OriginY*16 - padY*16) * imgHs / imgH);
    node->Scale = (float(imgH) / float(imgHs) + float(imgW) / float(imgWs)) / 2;

    RasterDataSrc.Resize(imgW * imgH);

    RasterData.Resize(imgWs * imgHs);
    RasterPitch = imgWs;

    memset(&RasterDataSrc[0], 0, imgW * imgH);

    if (imgW > 1 && imgH > 1)
    {
        unsigned y;
        for(y = 0; y < ras->Height && y+padY < imgH; ++y)
        {
            memcpy(&RasterDataSrc[(y+padY)*imgW + padX], &ras->Raster[y*ras->Width], ras->Width);
        }

        int bias = 0;
        int imgX = 0;
        int imgY = 0;
        if (gp.IsKnockOut())
            KnockOutCopy = RasterDataSrc;

        if (blurX > 0 || blurY > 0)
        {
            recursiveBlur(&RasterDataSrc[0], imgW, imgX, imgY, imgW, imgH, blurX, blurY);
            bias = 8;
        }

        strengthenImage(&RasterDataSrc[0], imgW, imgX, imgY, imgW, imgH, 
                        gp.GetBlurStrength(), (gp.GetBlurStrength() <= 1) ? 0 : bias);

        if (gp.IsKnockOut())
            knockOut(&RasterDataSrc[0]);
    }

    if (imgH == imgHs && imgW == imgWs)
    {
        RasterData = RasterDataSrc;
    }
    else
    {
        memset(&RasterData[0], 0, imgWs * imgHs);
        ResizeImageBilinear(&RasterData[0], imgWs, imgHs, imgWs,
                            &RasterDataSrc[0], imgW, imgH, imgW,
                            ResizeGray);
    }

    updateTextureGlyph(node);
    ++RasterizationCount;
    return node;
}

//-----------------------------------------------------------------------
GlyphNode* GlyphCache::RasterizeShadow(GlyphRunData& data, TextMeshProvider* tm, const GlyphParam& gp, float screenSize, const GlyphRaster* ras)
{
    if (MaxNumTextures == 0)
    {
        Result = Res_NoRasterCache;
        return 0;
    }

    GlyphNode* node = 0;
    if (ras)
    {
        node = createShadowFromRaster(data, tm, gp, screenSize, ras);
        if (node)
            return node;
    }

    if (data.pShape == 0)
    {
        Result = Res_ShapeNotFound;
        return 0;
    }

    float k = gp.GetFontSize() / screenSize;
    float blurX = gp.GetBlurX() * k * data.HeightRatio;
    float blurY = gp.GetBlurY() * k * data.HeightRatio;

    SF_ASSERT(ShadowQuality <= 1.0f && ShadowQuality >= 0.25f);
    float maxH = float(MaxSlotHeight) * ShadowQuality - float(2*SlotPadding);
    float rasterScale = 1;
    float vectorScale = gp.GetFontSize() / (data.HintedNomHeight ? (float)data.HintedNomHeight : data.NomHeight);

    float y1 = data.GlyphBounds.y1;
    float y2 = data.GlyphBounds.y2;

    if (y1 >= y2)
        y1 = y2 = 0;

    y1 = y1 * vectorScale - blurY;
    y2 = y2 * vectorScale + blurY;
    float h = y2 - y1;

    if (h >= maxH)
    {
        k = maxH / h;
        vectorScale *= k;
        blurX *= k;
        blurY *= k;
        rasterScale = 1/k;
    }

    int intBlurX = int(ceil(blurX));
    int intBlurY = int(ceil(blurY));

    // Rasterization
    Ras.Clear();
    addShapeToRasterizer(data.pShape, vectorScale, vectorScale);

    int padX = SlotPadding + intBlurX;
    int padY = SlotPadding + intBlurY;

    int imgX1 = 0;
    int imgX2 = 0;
    int imgY1 = 0;
    int imgY2 = 0;

    if (Ras.SortCells())
    {
        imgX1 = Ras.GetMinX() - padX;
        imgX2 = Ras.GetMaxX() + padX;
        imgY1 = Ras.GetMinY() - padY;
        imgY2 = Ras.GetMaxY() + padY;
    }

    unsigned imgW  = imgX2 - imgX1 + 1;
    unsigned imgH  = imgY2 - imgY1 + 1;

    if (imgH > MaxSlotHeight)
        imgH = MaxSlotHeight;

    node = allocateGlyph(tm, gp, imgW, imgH);
    if (node == 0)
    {
        Result = Res_CacheFull;
        cacheFullWarning();
        return 0;
    }

    node->Origin.x = SInt16(imgX1 * 16);
    node->Origin.y = SInt16(imgY1 * 16);
    node->Scale = rasterScale;

    RasterData.Resize(imgW * imgH);
    RasterPitch = imgW;

    memset(&RasterData[0], 0, imgW * imgH);
    if (imgW > 1 && imgH > 1)
    {
        float gamma = 1.0f;
        if (gp.BlurX || gp.BlurY)
        {
            // Gamma correction improves the "strength" of glow and shadow
            // TO DO: Think of smarter control of it.
            gamma = 0.4f;
        }
        if (gamma != Ras.GetGamma2())
            Ras.SetGamma2(gamma);

        unsigned i;
        for(i = 0; i < Ras.GetNumScanlines() && (padY+i) < imgH; ++i)
        {
            UByte* sl = &RasterData[(padY+i)*RasterPitch];
            Ras.SweepScanline(i, sl+padX, 1, 1);
        }

        int bias = 0;
        int imgX = 0;
        int imgY = 0;
        if (gp.IsKnockOut())
            KnockOutCopy = RasterData;

        if (blurX > 0 || blurY > 0)
        {
            recursiveBlur(&RasterData[0], RasterPitch, imgX, imgY, imgW, imgH, blurX, blurY);
            bias = 8;
        }

        strengthenImage(&RasterData[0], RasterPitch, imgX, imgY, imgW, imgH, 
                        gp.GetBlurStrength(), (gp.GetBlurStrength() <= 1) ? 0 : bias);

        if (gp.IsKnockOut())
            knockOut(&RasterData[0]);
    }

    updateTextureGlyph(node);
    ++RasterizationCount;
    Ras.Clear();
    return node;
}



void GlyphCache::LogWarning(const char* fmt, ...)
{
    if (pLog)
    {
        va_list argList; va_start(argList, fmt);
        pLog->LogMessageVarg(LogChannel_Render | LogMessage_Warning, fmt, argList);
        va_end(argList);
    }
}

void GlyphCache::LogError(const char* fmt, ...)
{
    if (pLog)
    {
        va_list argList; va_start(argList, fmt);
        pLog->LogMessageVarg(LogChannel_Render | LogMessage_Error, fmt, argList);
        va_end(argList);
    }
}

void GlyphCache::cacheFullWarning()
{
    if (RasterCacheWarning)
    {
        LogWarning("Warning: Increase raster glyph cache capacity - see GlyphCacheParams");
        RasterCacheWarning = false;
    }
}

void GlyphCache::rasterTooBigWarning()
{
    if (RasterTooBigWarning)
    {
        LogWarning("Warning: Raster glyph is too big - increase GlyphCacheParams.MaxSlotHeight");
        RasterTooBigWarning = false;
    }
}


bool GlyphCache::checkInitialization()
{
    if (!pFontHandleManager)
    {
        if ( !pRenderer || !pRenderer->IsInitialized())
            return false;
        initialize();
    }
    return pFontHandleManager != 0;
}



}} // Scaleform::Render

