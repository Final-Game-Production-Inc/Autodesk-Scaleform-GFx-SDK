/**************************************************************************

Filename    :   GameUIRuntimeAtlas.cpp
Content     :   
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GameUIRuntimeAtlas.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_SysFile.h"
#include "GFx/GFx_ImageResource.h"

// #define ENABLE_RUNTIME_ATLAS_DEBUG
// #define DISABLE_ATLASING

class GameUIRuntimeAtlasSubImage : public Render::SubImage
{
public:
    GameUIRuntimeAtlasSubImage(GameUIRuntimeAtlasManager::AtlasRecord* record) 
        : Render::SubImage(record->Atlas->GetImage()->GetAsImage(), record->Rect), Record(record) 
    {

    }
    virtual ~GameUIRuntimeAtlasSubImage()
    {
        Record->notifySubImageRelease();
    }
protected:
    GameUIRuntimeAtlasManager::AtlasRecord*     Record;
};

//////////////////////////////////////////////////////////////////////////

class GameUIAtlasSubImageUpdate : public Render::ImageUpdate
{
public:
    GameUIAtlasSubImageUpdate(Render::Image* atlas, const Render::ImageRect& rect, Render::Image* subImg) 
        : Render::ImageUpdate(atlas), Rect(rect), SubImage(subImg) {}

    virtual void HandleUpdate(Render::TextureManager* pmanager) 
    { 
        // This should be running in the render thread

        // PPS: The atlas image data will be updated with the sub image contents.

        Render::RawImage* atlas = (Render::RawImage*)GetImage();
        Render::Texture* texture = atlas->GetTexture(pmanager);
        Render::Texture::UpdateDesc updateDesc;
        updateDesc.PlaneIndex = 0;
        updateDesc.DestRect = Rect;

        // Setup a plane corresponding to the sub image slot in the atlas
        Render::ImageData data;
        atlas->GetImageData(&data);
        data.GetPlane(0, &updateDesc.SourcePlane);
        updateDesc.SourcePlane.pData += (updateDesc.SourcePlane.Pitch * Rect.Y1()) + (Rect.X1() * (data.GetBitsPerPixel() / 8));
        updateDesc.SourcePlane.Width = SubImage->GetRect().Width();
        updateDesc.SourcePlane.Height = SubImage->GetRect().Height();

        // Copy the sub image data into the atlas
        Render::ImageData atlasSlotData(atlas->GetFormat());
        atlasSlotData.SetPlane(0, updateDesc.SourcePlane);
        SubImage->Decode(&atlasSlotData);

        // Update the atlas texture (partial update)
        texture->Update(&updateDesc);
    }

protected:
    Render::ImageRect   Rect;
    Ptr<Render::Image>  SubImage;
};

//////////////////////////////////////////////////////////////////////////

void    GameUIRuntimeAtlasManager::AtlasRecord::SetContent(Render::Image* image)
{
    // Notify renderer to update texture contents
    Ptr<GameUIAtlasSubImageUpdate> pupdate = *SF_NEW GameUIAtlasSubImageUpdate(Atlas->GetImage()->GetAsImage(), Rect, image);
    Atlas->GetImage()->GetAsImage()->Update(pupdate);
}

Render::Image*  GameUIRuntimeAtlasManager::AtlasRecord::CreateSubImage()
{
    GenCount++;
#ifdef ENABLE_RUNTIME_ATLAS_DEBUG
    SF_DEBUG_MESSAGE1(1, "GENCOUNT : %d\n", GenCount);
#endif
    return SF_NEW GameUIRuntimeAtlasSubImage(this);
}

void    GameUIRuntimeAtlasManager::AtlasRecord::notifySubImageRelease()
{
    GenCount--;
#ifdef ENABLE_RUNTIME_ATLAS_DEBUG
    SF_DEBUG_MESSAGE1(1, "GENCOUNT : %d\n", GenCount);
#endif
    if (GenCount == 0)
        Manager->cleanUpRecord(this);
}

//////////////////////////////////////////////////////////////////////////

GameUIRuntimeAtlasManager::AtlasRecordList::~AtlasRecordList()
{
    AtlasRecord* record = mList.GetFirst();
    while (!mList.IsNull(record))
    {
        AtlasRecord* next = mList.GetNext(record);
        mList.Remove(record);
        delete record;
        record = next;
    }
}

//////////////////////////////////////////////////////////////////////////

GameUIRuntimeAtlasManager::~GameUIRuntimeAtlasManager()
{
    for (RecordBucketLookupType::Iterator iter = FreeAtlasRecordLookup.Begin(); 
        iter != FreeAtlasRecordLookup.End(); ++iter)
    {
        AtlasRecordList* list = iter->Second;
        delete list;
    }
}

Render::Image*  GameUIRuntimeAtlasManager::LoadProtocolImage(const GFx::ImageCreateInfo& info, const String& url)
{
    // Not an icon load?
    if (SFstrncmp("img://icon:", url.ToCStr(), 11)) 
    {
        SF_ASSERT(0);   // Shouldn't be here
        return NULL;
    }
    String iconPath(url.ToCStr() + 11);

#ifdef DISABLE_ATLASING

    return GFx::ImageCreator::LoadImageFile(info, iconPath);

#else

    AtlasRecord* record = NULL;
    // Check if the image is already loaded and return that record if found
    AtlasRecord** pr = LoadedImageRecords.Get(iconPath);
    if (pr)
    {
        record = *pr;
    }
    // Else, create an atlas record for the icon
    else
    {
        record = createRecord(info, iconPath);
        LoadedImageRecords.Add(iconPath, record);
    }
    return record->CreateSubImage();

#endif
}

GameUIRuntimeAtlasManager::AtlasRecord*     GameUIRuntimeAtlasManager::createRecord(const GFx::ImageCreateInfo& info, const String& url)
{
    // Load the icon image. We don't call LoadImage directly, because it will attempt to make a TextureImage
    // because the ImageCreator has an associated TextureManager. This way, we are guaranteed to only create a
    // system memory copy of the image data, which will be updated into the atlas texture.
    Render::ImageFileHandlerRegistry* registry = info.GetImageFileHandlerRegistry();
    if (!registry)
        return 0;

    Ptr<File> file = *info.GetFileOpener()->OpenFile(url);

    Ptr<Render::Image> image = *registry->ReadImage(file);
    SF_ASSERT(image);

    // Find a free atlas record
    AtlasRecord* record = getFreeRecord(image->GetSize());
    // Copy contents into record
    record->SetContent(image);
    // Add to used list
    UsedAtlasRecords.PushBack(record);

    file->Close();

    // Return!
    return record;
}

GameUIRuntimeAtlasManager::AtlasRecord*    GameUIRuntimeAtlasManager::getFreeRecord(const Render::ImageSize& sz)
{    
    // Do we have a free atlas record list for the image size?
    AtlasRecordList* recordList = NULL;
    AtlasRecordList** pr = FreeAtlasRecordLookup.Get(sz);
    if (!pr)
    {
        recordList = SF_NEW AtlasRecordList;
        FreeAtlasRecordLookup.Add(sz, recordList);
    }
    else
        recordList = *pr;
    
    // Do we have free atlas records for the image size?
    if (recordList->IsEmpty())
    {
        // No? Create more records!
        createFreeRecords(recordList, sz);
    }
    
    // Return a free record!
    AtlasRecord* record = recordList->PopFront();
    SF_ASSERT(record);
    return record;
}

void GameUIRuntimeAtlasManager::createFreeRecords(AtlasRecordList* list, const Render::ImageSize& sz)
{
    const unsigned atlasWidth = 256, atlasHeight = 256;
    const unsigned padding = 1;
    const unsigned padding2 = padding * 2;

    // Create a new texture for the atlas
    Render::ImageSize atlasSz(atlasWidth, atlasHeight);
    Ptr<Render::Image> img = *Render::RawImage::Create(Render::Image_R8G8B8A8, 1, atlasSz,
                                                       Render::ImageUse_Update | Render::ImageUse_PartialUpdate, 
                                                       0, GetTextureManager());
    Ptr<GFx::ImageResource> atlas = *SF_NEW GFx::ImageResource(img.GetPtr());

#ifdef ENABLE_RUNTIME_ATLAS_DEBUG
	SF_DEBUG_MESSAGE(1, "CREATED NEW ATLAS TEXTURE");
#endif

    // Create new records to fill the atlas texture
    unsigned slotW = sz.Width + padding2, slotH = sz.Height + padding2;
    for (unsigned y = 0; y < (atlasHeight - slotH); y += slotH)
    {
        for (unsigned x = 0; x < (atlasWidth - slotW); x += slotW)
        {
            AtlasRecord* record = SF_NEW AtlasRecord(atlas, Render::ImageRect(x + padding, 
                                                                              y + padding, 
                                                                              x + padding + sz.Width, 
                                                                              y + padding + sz.Height), this);
            list->PushBack(record);
        }
    }
}

void GameUIRuntimeAtlasManager::cleanUpRecord(AtlasRecord* record)
{
    AtlasRecordList* recordList = NULL;
    AtlasRecordList** pr = FreeAtlasRecordLookup.Get(record->GetRect().GetSize());
    SF_ASSERT(pr);
    recordList = *pr;
    UsedAtlasRecords.Remove(record);
    recordList->PushBack(record);
}
