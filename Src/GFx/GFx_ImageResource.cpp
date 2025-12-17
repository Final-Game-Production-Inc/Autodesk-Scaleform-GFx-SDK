/**************************************************************************

Filename    :   GFx_ImageResource.cpp
Content     :   Image resource representation for GFxPlayer
Created     :   January 30, 2007
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/GFx_ImageResource.h"
#include "Kernel/SF_HeapNew.h"

#include "GFx/GFx_ImageCreator.h"

namespace Scaleform { namespace GFx {

// ***** GFxImageFileKeyData implementation

class ImageFileInfoKeyData : public RefCountBase<ImageFileInfoKeyData, Stat_Default_Mem>
{
    // Image States.  
    Ptr<FileOpener>     pFileOpener;
    Ptr<ImageCreator>   pImageCreator;
    // We store image heap as a part of the key to make sure that images
    // in different heaps don't match accidentally, causing incorrect lifetime
    // crashes. Note that right now, this probably won't happen since ResourceLib
    // is only accessed internally..
    MemoryHeap*         pImageHeap;

public:
    // Key File Info - provides file name and image dimensions.
    // Note that this key data is potentially different from the ResourceFileInfo
    // specified in the export because its filename could have been translated
    // and/or adjusted to have a different relative/absolute path.
    Ptr<ImageFileInfo>  pFileInfo;

    ImageFileInfoKeyData(ImageFileInfo* pfileInfo,
                         FileOpener* pfileOpener,
                         ImageCreator* pimageCreator,
                         MemoryHeap *pimageHeap)
    {        
        pFileInfo   = pfileInfo;
        pFileOpener = pfileOpener;
        pImageCreator = pimageCreator;
        pImageHeap  = pimageHeap;
    }

    bool operator == (ImageFileInfoKeyData& other) const
    {
        return (pFileOpener == other.pFileOpener &&
                pImageCreator == other.pImageCreator &&
                pImageHeap == other.pImageHeap &&
                pFileInfo->FileName == other.pFileInfo->FileName);
    }
    bool operator != (ImageFileInfoKeyData& other) const
    {
        return !operator == (other);
    }

    UPInt  GetHashCode() const
    {
        return pFileInfo->GetHashCode() ^
            ((UPInt)pFileOpener.GetPtr()) ^ (((UPInt)pFileOpener.GetPtr()) >> 7) ^
            ((UPInt)pImageCreator.GetPtr()) ^ (((UPInt)pImageCreator.GetPtr()) >> 7) ^
            ((UPInt)pImageHeap) ^ (((UPInt)pImageHeap) >> 7);
    }
};

class ImageFileKeyInterface : public ResourceKey::KeyInterface
{
public:
    typedef ResourceKey::KeyHandle KeyHandle;    

    // ResourceKey::KeyInterface implementation.    
    virtual void                AddRef(KeyHandle hdata);  
    virtual void                Release(KeyHandle hdata);
    virtual ResourceKey::KeyType GetKeyType(KeyHandle hdata) const;
    virtual UPInt               GetHashCode(KeyHandle hdata) const;
    virtual bool                KeyEquals(KeyHandle hdata, const ResourceKey& other);
   // const  ResourceFileInfo* GetFileInfo(KeyHandle hdata) const;

     const  char* GetFileURL(KeyHandle hdata) const;
};

static ImageFileKeyInterface ImageFileKeyInterface_Instance;


// Reference counting on the data object, if necessary.
void    ImageFileKeyInterface::AddRef(KeyHandle hdata)
{
    ImageFileInfoKeyData* pdata = (ImageFileInfoKeyData*) hdata;
    SF_ASSERT(pdata);
    pdata->AddRef();
}

void    ImageFileKeyInterface::Release(KeyHandle hdata)
{
    ImageFileInfoKeyData* pdata = (ImageFileInfoKeyData*) hdata;
    SF_ASSERT(pdata);
    pdata->Release();
}

// Key/Hash code implementation.
ResourceKey::KeyType ImageFileKeyInterface::GetKeyType(KeyHandle hdata) const
{
    SF_UNUSED(hdata);
    return ResourceKey::Key_File;
}

UPInt  ImageFileKeyInterface::GetHashCode(KeyHandle hdata) const
{
    ImageFileInfoKeyData* pdata = (ImageFileInfoKeyData*) hdata;
    return pdata->GetHashCode();    
}

bool    ImageFileKeyInterface::KeyEquals(KeyHandle hdata, const ResourceKey& other)
{
    if (this != other.GetKeyInterface())
        return 0;

    ImageFileInfoKeyData* pthisData = (ImageFileInfoKeyData*) hdata;
    ImageFileInfoKeyData* potherData = (ImageFileInfoKeyData*) other.GetKeyData();
    SF_ASSERT(pthisData);
    SF_ASSERT(potherData);
    return (*pthisData == *potherData);
}

// Get file info about the resource, potentially available with Key_File.
/*
const ResourceFileInfo* ImageFileKeyInterface::GetFileInfo(KeyHandle hdata) const
{
    ImageFileInfoKeyData* pdata = (ImageFileInfoKeyData*) hdata;
    SF_ASSERT(pdata);
    return pdata->pFileInfo.GetPtr();
}
*/

const char* ImageFileKeyInterface::GetFileURL(KeyHandle hdata) const
{
    ImageFileInfoKeyData* pdata = (ImageFileInfoKeyData*) hdata;
    SF_ASSERT(pdata);
    return pdata->pFileInfo->FileName.ToCStr();
}


// ***** ImageResource

ResourceKey  ImageResource::CreateImageFileKey(ImageFileInfo* pfileInfo,
                                               FileOpener* pfileOpener,
                                               ImageCreator* pimageCreator,
                                               MemoryHeap* pimageHeap)
{
    MemoryHeap* pheap = pimageHeap ? pimageHeap : Memory::GetGlobalHeap();
    Ptr<ImageFileInfoKeyData> pdata =
        *SF_HEAP_NEW(pheap) ImageFileInfoKeyData(pfileInfo, pfileOpener,
                                                 pimageCreator, pimageHeap);

    return ResourceKey(&ImageFileKeyInterface_Instance,
                       (ResourceKey::KeyHandle)pdata.GetPtr() );
}

}} // namespace Scaleform { namespace GFx {

