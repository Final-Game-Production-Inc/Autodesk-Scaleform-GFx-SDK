/**************************************************************************

Filename    :   GFx_SoundResource.cpp
Content     :   GFx audio support
Created     :   September, 2008
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFxConfig.h"
#ifdef GFX_ENABLE_SOUND

#include "GFx/Audio/GFx_SoundResource.h"
#include "GFx/GFx_LoadProcess.h"

namespace Scaleform { namespace GFx {

//////////////////////////////////////////////////////////////////////////
//

class SoundFileInfoKeyData : public RefCountBase<SoundFileInfoKeyData,StatMV_Other_Mem>
{
    // Image States.  
    Ptr<FileOpener> pFileOpener;

public:
    // Key File Info - provides file name and image dimensions.
    // Note that this key data is potentially different from the ResourceFileInfo
    // specified in the export because its filename could have been translated
    // and/or adjusted to have a different relative/absolute path.
    Ptr<SoundFileInfo> pFileInfo;

    SoundFileInfoKeyData(SoundFileInfo* pfileInfo, FileOpener* pfileOpener)
    {        
        pFileInfo   = pfileInfo;
        pFileOpener = pfileOpener;
    }

    bool operator == (SoundFileInfoKeyData& other) const
    {
        return (pFileOpener == other.pFileOpener &&
                pFileInfo->FileName == other.pFileInfo->FileName);
    }
    bool operator != (SoundFileInfoKeyData& other) const
    {
        return !operator == (other);
    }

    UPInt GetHashCode() const
    {
        return pFileInfo->GetHashCode() ^
            ((UPInt)pFileOpener.GetPtr()) ^ (((UPInt)pFileOpener.GetPtr()) >> 7);
    }
};

//////////////////////////////////////////////////////////////////////////
//

class SoundFileKeyInterface : public ResourceKey::KeyInterface
{
public:
    typedef ResourceKey::KeyHandle KeyHandle;    

    // ResourceKey::KeyInterface implementation.    
    virtual void AddRef(KeyHandle hdata);  
    virtual void Release(KeyHandle hdata);
    virtual ResourceKey::KeyType GetKeyType(KeyHandle hdata) const;
    virtual UPInt GetHashCode(KeyHandle hdata) const;
    virtual bool KeyEquals(KeyHandle hdata, const ResourceKey& other);

    const char* GetFileURL(KeyHandle hdata) const;
};

static SoundFileKeyInterface SoundFileKeyInterface_Instance;

// Reference counting on the data object, if necessary.
void SoundFileKeyInterface::AddRef(KeyHandle hdata)
{
    SoundFileInfoKeyData* pdata = (SoundFileInfoKeyData*) hdata;
    SF_ASSERT(pdata);
    pdata->AddRef();
}

void SoundFileKeyInterface::Release(KeyHandle hdata)
{
    SoundFileInfoKeyData* pdata = (SoundFileInfoKeyData*) hdata;
    SF_ASSERT(pdata);
    pdata->Release();
}

// Key/Hash code implementation.
ResourceKey::KeyType SoundFileKeyInterface::GetKeyType(KeyHandle hdata) const
{
    SF_UNUSED(hdata);
    return ResourceKey::Key_File;
}

UPInt SoundFileKeyInterface::GetHashCode(KeyHandle hdata) const
{
    SoundFileInfoKeyData* pdata = (SoundFileInfoKeyData*) hdata;
    return pdata->GetHashCode();    
}

bool SoundFileKeyInterface::KeyEquals(KeyHandle hdata, const ResourceKey& other)
{
    if (this != other.GetKeyInterface())
        return 0;

    SoundFileInfoKeyData* pthisData  = (SoundFileInfoKeyData*) hdata;
    SoundFileInfoKeyData* potherData = (SoundFileInfoKeyData*) other.GetKeyData();
    SF_ASSERT(pthisData);
    SF_ASSERT(potherData);
    return (*pthisData == *potherData);
}

const char* SoundFileKeyInterface::GetFileURL(KeyHandle hdata) const
{
    SoundFileInfoKeyData* pdata = (SoundFileInfoKeyData*) hdata;
    SF_ASSERT(pdata);
    return pdata->pFileInfo->FileName.ToCStr();
}

ResourceKey SoundResource::CreateSoundFileKey(SoundFileInfo* pfileInfo, FileOpener* pfileOpener)
{
    Ptr<SoundFileInfoKeyData> pdata = *SF_NEW SoundFileInfoKeyData(pfileInfo, pfileOpener);

    return ResourceKey(&SoundFileKeyInterface_Instance, (ResourceKey::KeyHandle)pdata.GetPtr() );
}

//////////////////////////////////////////////////////////////////////////
//

// Creates/Loads resource based on SoundData during binding process
bool SoundResourceCreator::CreateResource(
    DataHandle hdata, ResourceBindData *pbindData, LoadStates *pls, MemoryHeap *pbindHeap) const
{
    SF_UNUSED(pls);
    SF_UNUSED(pbindHeap); // Don't use bindHeap for shared resources.

    Sound::SoundData* psound = (Sound::SoundData*) hdata;
    SF_ASSERT(psound);
    if (!pbindData->pResource)
    {
        // Our MovieDefImpl's pBinding should have been provided by caller.
        Ptr<Sound::SoundInfo> sinfo = *SF_NEW Sound::SoundInfo(psound);
        pbindData->pResource = *SF_NEW SoundResource(sinfo);
    }

    return pbindData->pResource ? 1 : 0;
}

//static
ResourceData SoundResourceCreator::CreateSoundResourceData(Sound::SoundData* psound)
{
    static SoundResourceCreator static_inst;
    return ResourceData(&static_inst, psound);
}

// Creates/Loads resource based on data and loading process
bool SoundFileResourceCreator::CreateResource(
    DataHandle hdata, ResourceBindData *pbindData, LoadStates *pls, MemoryHeap *pbindHeap) const
{
    SoundFileInfo *prfi = (SoundFileInfo*) hdata;
    SF_ASSERT(prfi);
    SF_UNUSED(pbindHeap); // Don't use bindHeap for shared resources.

    // Make a new fileInfo so that it can have a modified filename.   
    Ptr<SoundFileInfo> psoundFileInfo = *SF_NEW SoundFileInfo(*prfi);

    // Translate filename.
    URLBuilder::LocationInfo loc(
        URLBuilder::File_Sound, prfi->FileName, pls->GetRelativePath());
    pls->BuildURL(&psoundFileInfo->FileName, loc);

    // Now that we have a correct file object we need to match it against
    // the library so as to check if is is already loaded
    ResourceKey soundKey = SoundResource::CreateSoundFileKey(psoundFileInfo, pls->GetFileOpener());

    String errorMessage;
    ResourceLib::BindHandle bh;
    Ptr<SoundResource> psoundRes = 0;

    if (pls->GetLib()->BindResourceKey(&bh, soundKey) == ResourceLib::RS_NeedsResolve)
    {
        Ptr<File> pf = *pls->GetFileOpener()->OpenFile(psoundFileInfo->FileName);
        if (pf && pf->IsValid())
        {
            Ptr<Sound::SoundFile> psoundData = *SF_NEW Sound::SoundFile(
                psoundFileInfo->FileName,prfi->SampleRate, prfi->SampleCount,false);
            psoundData->SetSeekSample(prfi->SeekSample);
            Ptr<Sound::SoundInfo> sinfo = *SF_NEW Sound::SoundInfo(psoundData);
            psoundRes = *SF_NEW SoundResource(sinfo, soundKey);
        }

        if (psoundRes)
            bh.ResolveResource(psoundRes);
        else
        {
            errorMessage = "Failed to load sound '";
            errorMessage += psoundFileInfo->FileName;
            errorMessage += "'";

            bh.CancelResolve(errorMessage.ToCStr());
        }
    }
    else
    {
        // If Available and Waiting resources will be resolved here.
        if ((psoundRes = *(SoundResource*)bh.WaitForResolve()).GetPtr() == 0)
        {
            errorMessage = bh.GetResolveError();
        }
    }

    // If there was an error, display it
    if (!psoundRes)
    {
        pls->pLog->LogError("%s", errorMessage.ToCStr());
        return 0;
    }

    // Pass resource ownership to BindData.
    pbindData->pResource = psoundRes;
    return 1;    
}

//static 
ResourceData SoundFileResourceCreator::CreateSoundFileResourceData(SoundFileInfo * prfi)
{
    static SoundFileResourceCreator stat_inst;
    return ResourceData(&stat_inst, prfi);
}

}} // namespace Scaleform::GFx

#endif // GFX_ENABLE_SOUND
