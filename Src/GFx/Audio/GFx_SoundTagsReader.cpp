/**************************************************************************

Filename    :   GFx_SoundTagsReader.cpp
Content     :   GFx audio support
Created     :   February, 2009
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFxConfig.h"
#ifdef GFX_ENABLE_SOUND

#include "GFx/Audio/GFx_SoundTagsReader.h"
#include "GFx/GFx_LoadProcess.h"
#include "GFx/GFx_Button.h"
#include "GFx/GFx_Sprite.h"

namespace Scaleform { namespace GFx {

//////////////////////////////////////////////////////////////////////////
//

ButtonSoundDef* SoundTagsReader::ReadButtonSoundDef(LoadProcess* p)
{
    ButtonSoundDefImpl* psound = SF_HEAP_NEW(p->GetLoadHeap()) ButtonSoundDefImpl;
    p->LogParse("button sound options:\n");
    for (int i = 0; i < 4; i++)
    {
        SoundInfo& bs = psound->ButtonSounds[i];
        bs.SoundId = ResourceId(p->ReadU16());
        if (bs.SoundId == 0)
            continue;
        ResourceHandle rh;
        if (p->GetResourceHandle(&rh, bs.SoundId))
        {
            bs.pSample.SetFromHandle(rh);
        }
        else
        {
            p->LogParse("sound tag not found, SoundId=%d, button state #=%i", (int)bs.SoundId.GetIdValue(), i);
        }
        p->LogParse("\n  SoundId = %d\n", (int)bs.SoundId.GetIdValue());
        bs.Style.Read(p->GetStream());
    }
    return psound;
}

void SoundTagsReader::ReadDefineSoundTag(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED(tagInfo); 
    SF_ASSERT(tagInfo.TagType == Tag_DefineSound);

    Sound::SoundRenderer* pSoundRenderer = p->GetLoadStates()->GetAudio()->GetRenderer();
    UInt32 caps = 0;
    if (pSoundRenderer)
        pSoundRenderer->GetRenderCaps(&caps);

    Stream* in = p->GetStream();
    UInt16  characterId = in->ReadU16();
    Ptr<Sound::SoundData> psound = *GFx_ReadSoundData(p, characterId);

    if (pSoundRenderer && (psound->GetFormat() & Sound::SoundDataBase::Sample_MP3)
                       && (caps & Sound::SoundRenderer::Cap_NoMP3))
        return;

    ResourceData resData = SoundResourceCreator::CreateSoundResourceData(psound);
    p->AddDataResource(ResourceId(characterId), resData);
}

class StartSoundTag : public ExecuteTag
{
public:
    void Read(LoadProcess* p, const TagInfo& tagInfo, UInt16 soundID, ResourceHandle rh)      
    {
        SF_UNUSED(tagInfo);    
        SF_ASSERT(tagInfo.TagType == Tag_StartSound);

        Info.SoundId = ResourceId(soundID);
        Info.Style.Read(p->GetStream());
        Info.pSample.SetFromHandle(rh);
    }

    void Execute(DisplayObjContainer* m)
    {
        Info.Play(m);
    }

    SoundInfo Info;
};

void SoundTagsReader::ReadStartSoundTag(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED(tagInfo);    
    SF_ASSERT(tagInfo.TagType == Tag_StartSound);

    UInt16 soundId = p->ReadU16();

    ResourceHandle rh;
    if (p->GetResourceHandle(&rh, ResourceId(soundId)))
    {
        StartSoundTag* sst = p->AllocTag<StartSoundTag>();
        sst->Read(p, tagInfo, soundId, rh);

        p->LogParse("StartSound tag: id=%d, stop = %d, loop ct = %d\n",
                    soundId, int(sst->Info.Style.StopPlayback), sst->Info.Style.LoopCount);
        p->AddExecuteTag(sst);
    }
    else
    {
        p->LogError("StartSoundLoader - SoundId %d is not defined", soundId);
    }
}

void SoundTagsReader::ReadButtonSoundTag(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_ASSERT(tagInfo.TagType == Tag_ButtonSound);

    int buttonCharacterId = p->ReadU16();
    ResourceHandle rh;
    Resource* pres = 0;

    if (p->GetResourceHandle(&rh, ResourceId(buttonCharacterId)))
        pres = rh.GetResourcePtr();

    if (pres)
    {
        if (pres->GetResourceType() == Resource::RT_ButtonDef)
        {
            ButtonDef* ch = (ButtonDef*) pres;
            SF_ASSERT(ch != NULL);
            ch->Read(p, tagInfo.TagType);
        }
    }
    else
    {
        p->LogError("ButtonDef %d referenced in ButtonSound tag not found", buttonCharacterId);
    }
}

void SoundTagsReader::ReadDefineExternalSoundTag(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED(tagInfo);
    SF_ASSERT(tagInfo.TagType == Tag_DefineExternalSound);

    // Utilizes the tag 1006 (unused in normal SWF): the format is as follows:
    // Header           RECORDHEADER    1006
    // CharacterID      UI16
    // SoundFormat      UI16            0 - WAV
    // Bits             UI16
    // Channels         UI16
    // SampleRate       UI32
    // SampleCount      UI32
    // SeekSample       UI32
    // ExportNameLen    UI8
    // ExportName       UI8[ExportNameLen]
    // FileNameLen      UI8             with extension
    // FileName         UI8[FileNameLen]

    Stream* pin = p->GetStream();

    unsigned soundResourceId = p->ReadU16();
    unsigned soundFormat     = p->ReadU16();
    unsigned bits            = p->ReadU16();
    unsigned channels        = p->ReadU16();
    unsigned sample_rate     = p->ReadU32();
    unsigned sample_count    = p->ReadU32();
    unsigned seek_sample     = p->ReadU32();
    String soundExportName, soundFileName;
    pin->ReadStringWithLength(&soundExportName);
    pin->ReadStringWithLength(&soundFileName);

    pin->LogParse("  DefineExternalSound: id = 0x%X, fmt = %d, name = '%s', exp = '%s'\n",
        soundResourceId,
        soundFormat,
        soundFileName.ToCStr(),
        soundExportName.ToCStr());

    Ptr<SoundFileInfo> pfi = *SF_NEW SoundFileInfo;
    if (pfi)
    {
        pfi->FileName       = soundFileName;
        pfi->ExportName     = soundExportName;
        pfi->pExporterInfo  = p->GetExporterInfo();
        pfi->Format         = (FileTypeConstants::FileFormatType)soundFormat;
        pfi->Bits           = bits;
        pfi->Channels       = channels;
        pfi->SampleRate     = sample_rate;
        pfi->SampleCount    = sample_count;
        pfi->SeekSample     = seek_sample;
        pfi->Use            = Resource::Use_SoundSample;
   
        p->AddDataResource(ResourceId(soundResourceId),
            SoundFileResourceCreator::CreateSoundFileResourceData(pfi));
    }
}

void SoundTagsReader::ReadDefineExternalStreamSoundTag(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED(tagInfo);
    SF_ASSERT(tagInfo.TagType == Tag_DefineExternalStreamSound);

    // utilizes the tag 1007 (unused in normal SWF): the format is as follows:
    // Header           RECORDHEADER    1007
    // SoundFormat      UI16            0 - WAV
    // Bits             UI16
    // Channels         UI16
    // SampleRate       UI32
    // SampleCount      UI32
    // SeekSample       UI32
    // StartFrame       UI32
    // LastFrame        UI32
    // FileNameLen      UI8             with extension
    // FileName         UI8[FileNameLen]

    Stream* pin = p->GetStream();
    Ptr<SoundStreamDefImpl> pdef = *SF_NEW SoundStreamDefImpl;

    unsigned soundFormat    = p->ReadU16();
    unsigned bits           = p->ReadU16();
    unsigned channels       = p->ReadU16();
    pdef->StreamSoundRate   = p->ReadU32();
    pdef->StreamSampleCount = p->ReadU32();
    pdef->LatencySeek       = p->ReadU32();
    pdef->StartFrame        = p->ReadU32();
    pdef->LastFrame         = p->ReadU32();
    SF_UNUSED2(bits,channels);
    String soundFileName;
    pin->ReadStringWithLength(&soundFileName);

    pin->LogParse("  DefineExternalStreamSound: fmt = %d, name = '%s'\n",
        soundFormat,
        soundFileName.ToCStr());

    String fname;
    URLBuilder::LocationInfo loc(
        URLBuilder::File_Sound, soundFileName, p->GetLoadStates()->GetRelativePath());
    p->GetLoadStates()->BuildURL(&fname, loc);

    Ptr<Sound::SoundFile> sound = *SF_NEW Sound::SoundFile(
        fname.ToCStr(),pdef->StreamSoundRate, pdef->StreamSampleCount, true);
    sound->SetSeekSample(pdef->LatencySeek);
    pdef->pSoundInfo = *SF_NEW Sound::SoundInfo(sound);
    p->SetSoundStream(pdef);

}

void SoundTagsReader::ReadSoundStreamHeadTag(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED(tagInfo);    
    SF_ASSERT(tagInfo.TagType == Tag_SoundStreamHead || tagInfo.TagType == Tag_SoundStreamHead2);

    p->LogParse("SoundStreamHead%c\n", (tagInfo.TagType == Tag_SoundStreamHead? ' ' : '2'));

    Sound::SoundRenderer* pSoundRenderer = p->GetLoadStates()->GetAudio()->GetRenderer();
    UInt32 caps = 0;
    if (pSoundRenderer)
        pSoundRenderer->GetRenderCaps(&caps);

    if (pSoundRenderer && (caps & Sound::SoundRenderer::Cap_NoStreaming))
        return;

    Stream* in = p->GetStream();
    Ptr<SoundStreamDefImpl> pdef = *SF_NEW SoundStreamDefImpl;
    pdef->ReadHeadTag(in);

    if (pdef->StreamSampleCount > 0)
        p->SetSoundStream(pdef);
}

void SoundTagsReader::ReadSoundStreamBlockTag(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED(tagInfo);
    SF_ASSERT(tagInfo.TagType == Tag_SoundStreamBlock);

    Sound::SoundRenderer* pSoundRenderer = p->GetLoadStates()->GetAudio()->GetRenderer();
    UInt32 caps = 0;
    if (pSoundRenderer)
        pSoundRenderer->GetRenderCaps(&caps);

    if (pSoundRenderer && (caps & Sound::SoundRenderer::Cap_NoStreaming))
        return;

    SoundStreamDefImpl* pdef = (SoundStreamDefImpl*)p->GetSoundStream();
    SF_ASSERT(pdef);

    if(pSoundRenderer && pdef->SoundCompression == 2 && (caps & Sound::SoundRenderer::Cap_NoMP3))
        return;

    pdef->ReadBlockTag(p);
}

}} // namespace Scaleform::GFx

#endif // GFX_ENABLE_SOUND
