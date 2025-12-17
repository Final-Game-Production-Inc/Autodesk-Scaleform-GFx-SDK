/**************************************************************************

Filename    :   GFx_SpriteDef.cpp
Content     :   Sprite Definition
Created     :   
Authors     :   Artem Bolgar

Notes       :   This file contains class declarations used in
GFxSprite.cpp only. Declarations that need to be
visible by other player files should be placed
in DisplayObjectBase.h.

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/GFx_SpriteDef.h"
#include "GFx/GFx_LoadProcess.h"
#include "GFx/GFx_PlayerImpl.h"

namespace Scaleform { namespace GFx {

//
// ***** SpriteDef
//

SpriteDef::SpriteDef(MovieDataDef* pmd)
:
    pMovieDef(pmd),
    FrameCount(0),
    LoadingFrame(0),
    pScale9Grid(0),
    //pSoundStream(NULL), //@SOUND
    Flags(0)
{   
    SF_ASSERT(pMovieDef);
}

SpriteDef::~SpriteDef()
{
    // Destroy frame data; the actual de-allocation of tag memory
    // takes place in the MovieDataDef's tag block allocator.
    unsigned i;
    for(i=0; i<Playlist.GetSize(); i++)
        Playlist[i].DestroyTags();        
    delete pScale9Grid;
    //#ifdef GFX_ENABLE_SOUND
    //    if (pSoundStream) //@SOUND
    //        pSoundStream->Release();
    //#endif
}


// Labels the frame currently being loaded with the
// given name.  A copy of the name string is made and
// kept in this object.
void    SpriteDef::AddFrameName(const String& name, LogState *plog)
{
    if (!(LoadingFrame >= 0 && LoadingFrame < FrameCount))
    {
        if (plog)
            plog->LogError("AddFrameName(%d, '%s') -- frame is out of range (frameCount = %d; skipping",
            LoadingFrame, name.ToCStr(), FrameCount);
        return;
    }

    unsigned currentlyAssigned = 0;
    if (NamedFrames.GetAlt(name, &currentlyAssigned) == true)
    {
        if (plog)
            plog->LogError("AddFrameName(%d, '%s') -- frame name already assigned to frame %d; overriding",
            LoadingFrame, name.ToCStr(), currentlyAssigned);
    }
    // check if we have predefined special labels, such as "_up", "_down", "_over".
    if (name.GetLength() > 0 && name[0] == '_')
    {
        if (name == "_up")
            Flags |= Flags_Has_Frame_up;
        else if (name == "_down")
            Flags |= Flags_Has_Frame_down;
        else if (name == "_over")
            Flags |= Flags_Has_Frame_over;
    }
    NamedFrames.Set(name, LoadingFrame);   // stores 0-based frame #
}


// Read the sprite info.  Consists of a series of tags.
void    SpriteDef::Read(LoadProcess* p, ResourceId charId)
{
    Stream*  pin     = p->GetStream();
    UInt32      tagEnd = pin->GetTagEndPosition();


    p->EnterSpriteDef(this);

    FrameCount = pin->ReadU16();

    // ALEX: some SWF files have been seen that have 0-frame sprites.
    // The Macromedia player behaves as if they have 1 frame.
    if (FrameCount < 1)    
        FrameCount = 1;    
    Playlist.Resize(FrameCount);    // need a playlist for each frame

    pin->LogParse("  frames = %d\n", FrameCount);

    LoadingFrame = 0;

    while ((UInt32) pin->Tell() < tagEnd)
    {
        TagInfo  tagInfo;
        TagType  tagType = pin->OpenTag(&tagInfo);
        LoaderImpl::LoaderFunction lf = NULL;

#ifdef SF_DEBUG_COUNT_TAGS
        p->CountTag(tagType);
#endif

        p->ReportProgress(p->GetFileURL(), tagInfo, true);

        if (tagType == Tag_EndFrame)
        {
            // NOTE: In very rare cases FrameCount can LIE, containing
            // more frames then was reported in sprite header (wizardy.swf).
            if (LoadingFrame == (int)Playlist.GetSize())
            {
                Playlist.Resize(Playlist.GetSize()+1);
                pin->LogError("An extra frame is found for sprite id = %d, framecnt = %d, actual frames = %d",
                    (int)charId.GetIdIndex(), (int)FrameCount, (int)LoadingFrame+1);
            }

            p->CommitFrameTags();

            // show frame tag -- advance to the next frame.
            pin->LogParse("  ShowFrame (sprite, char id = %d)\n", charId.GetIdIndex());
            LoadingFrame++;
        }
        else if (LoaderImpl::GetTagLoader(tagType, &lf))            
        {
            // call the tag loader.  The tag loader should add
            // characters or tags to the InteractiveObject data structure.
            (*lf)(p, tagInfo);
        }
        else
        {
            // no tag loader for this tag type.
            pin->LogParse("*** no tag loader for type %d\n", tagType);
        }

        pin->CloseTag();
    }

    // In general, we should have EndFrame
    if (p->FrameTagsAvailable())
    {
        // NOTE: In very rare cases FrameCount can LIE, containing
        // more frames then was reported in sprite header (wizardy.swf).
        if (LoadingFrame == (int)Playlist.GetSize())
        {
            Playlist.Resize(Playlist.GetSize()+1);
            pin->LogError("An extra frame is found for sprite id = %d, framecnt = %d, actual frames = %d",
                          (int)charId.GetIdIndex(), (int)FrameCount, (int)LoadingFrame+1);
        }

        p->CommitFrameTags();
        // ??
        // LoadingFrame++;
    }


    p->LeaveSpriteDef();

    pin->LogParse("  -- sprite END, char id = %d --\n", charId.GetIdIndex());
}

// Initialize an empty clip.
void SpriteDef::InitEmptyClipDef()
{
    // Set FrameCount = 1; that is the default for an empty clip.
    FrameCount = 1;
    Playlist.Resize(FrameCount);
}

bool SpriteDef::DefPointTestLocal(const Render::PointF &pt, bool testShape, const DisplayObjectBase* pinst) const  
{ 
    SF_UNUSED3(pt, testShape, pinst);
    return false;
}

// fills array of labels for the passed frame. One frame may have multiple labels.
Array<String>* SpriteDef::GetFrameLabels(unsigned frameNumber, Array<String>* destArr) const
{
    SF_ASSERT(destArr);

    StringHashLH<unsigned>::ConstIterator it = NamedFrames.Begin();
    int i = 0;
    for (; it != NamedFrames.End(); ++it)
    {
        if (frameNumber == it->Second)
        {
            destArr->PushBack(it->First);
            ++i;
        }
    }
    return (i) ? destArr : NULL;
}

#ifdef GFX_ENABLE_SOUND

SoundStreamDef*  SpriteDef::GetSoundStream() const 
{ 
    return pSoundStream; 
}
void SpriteDef::SetSoundStream(SoundStreamDef* psoundStream) 
{ 
    //    if (pSoundStream)
    //        pSoundStream->Release();
    //    if (psoundStream)
    //        psoundStream->AddRef();
    pSoundStream = psoundStream; 
}

#endif // SF_NO_SOUND

//////////////////////////////////////////////////////////////////////////
// Timeline snapshot
//
TimelineSnapshot::SnapshotElement* TimelineSnapshot::Add(int depth)
{
    SnapshotElement* pe = SnapshotHeap.Alloc();
    if (pe)
    {
        SnapshotList.PushBack(pe);
        pe->Depth = depth;

        UPInt i = Alg::UpperBound(SnapshotSortedArray, depth, DepthLess);
        SF_ASSERT(i == SnapshotSortedArray.GetSize() || SnapshotSortedArray[i]->Depth != depth);
        SnapshotSortedArray.InsertAt((UPInt)i, pe);
        return pe;
    }
    return NULL;
}

// Remove the last (upper-bound) element at the specified depth
void TimelineSnapshot::Remove(int depth)
{
    UPInt idx;
    SnapshotElement* pe = FindDepth(depth, &idx);
    SF_ASSERT(pe);
    if (pe)
    {
        SnapshotList.Remove(pe);
        SnapshotSortedArray.RemoveAt(idx);
        SnapshotHeap.Free(pe);
    }
}

void    TimelineSnapshot::MakeSnapshot(TimelineDef* pdef, unsigned startFrame, unsigned endFrame)
{
    for (unsigned f = startFrame; f <= endFrame; ++f)
    {
#ifdef GFX_TRACE_TIMELINE_EXECUTION
        printf("\n\t--- frame: %d ---\n", f);
#endif
        const TimelineDef::Frame& playlist = pdef->GetPlaylist(f);
        for (unsigned i = 0; i < playlist.GetTagCount(); i++)
        {
            ExecuteTag*  e = playlist.GetTag(i);
            e->AddToTimelineSnapshot(this, f);
        }        
    }
#ifdef GFX_TRACE_TIMELINE_EXECUTION
    printf("-------------\n");
#endif
}

void    TimelineSnapshot::ExecuteSnapshot(DisplayObjContainer* pdispObj)
{
    if (!SnapshotList.IsEmpty())
    {
        const TimelineSnapshot::SnapshotElement* pe;
        for(pe = SnapshotList.GetFirst(); ;
            pe = SnapshotList.GetNext(pe))
        {
            const TimelineSnapshot::SnapshotElement& e = *pe;

            // [PPS] Place_Add/Move/Replace are always PlaceObject types?

            switch(e.PlaceType)
            {
            case TimelineSnapshot::Place_Add:
                {
                    GFxPlaceObjectBase::UnpackedData data;
                    e.Tags.Unpack(data);

                    ASStringManager *psm = pdispObj->GetStringManager();
                    ASString       sname((data.Name == NULL) ? 
                        psm->CreateEmptyString() : psm->CreateString(data.Name));
                    UInt32 flags = DisplayList::Flags_PlaceObject;
                    if (e.Flags & TimelineSnapshot::Flags_DeadOnArrival)
                        flags |= DisplayList::Flags_DeadOnArrival;
                    pdispObj->AddDisplayObject(
                        data.Pos, sname, data.pEventHandlers, 0, 
                        e.CreateFrame, flags);
                }
                break;
            case TimelineSnapshot::Place_Move:
                {
                    GFxPlaceObjectBase::UnpackedData data;
                    e.Tags.Unpack(data);

                    pdispObj->MoveDisplayObject(data.Pos);
                }
                break;
            case TimelineSnapshot::Place_Replace:
                {
                    GFxPlaceObjectBase::UnpackedData data;
                    e.Tags.Unpack(data);

                    ASStringManager *psm = pdispObj->GetStringManager();
                    ASString       sname((data.Name == NULL) ? psm->CreateEmptyString() : psm->CreateString(data.Name));
                    DisplayObjectBase*  newChar;
                    pdispObj->CreateAndReplaceDisplayObject(data.Pos, sname, &newChar);
                }
                break;
            case TimelineSnapshot::Place_Remove:
                {
                    //RemoveDisplayObject(e.Depth, pos.CharacterId);
                    pdispObj->RemoveDisplayObject(e.Depth, ResourceId());  // [PPS] RemoveObjectTag/2 does not store char id
                }
                break;
            }

            if (SnapshotList.IsLast(pe))
                break;
        }
    }
}

void TimelineSnapshot::SourceTags::Unpack(GFxPlaceObjectBase::UnpackedData& data) const
{
    SF_ASSERT(pMainTag);
    pMainTag->Unpack(data);

    // Override tag data for Depth, CharId, Matrix, ClassName, Cxform, BlendMode, ClipDepth, Ratio, TextFilter
    GFxPlaceObjectBase::UnpackedData    unpackedData[9];    

    UPInt   dataIdx;
    if (pDepthTag != pMainTag)
    {
        pDepthTag->Unpack(unpackedData[0]);
        data.Pos.Depth = unpackedData[0].Pos.Depth;
        data.Pos.SetDepthFlag();
    }
    if (pCharIdTag != pMainTag)
    {
        if (pDepthTag == pCharIdTag) dataIdx = 0;
        else { pCharIdTag->Unpack(unpackedData[1]); dataIdx = 1; }
        data.Pos.CharacterId = unpackedData[dataIdx].Pos.CharacterId;
        data.Pos.SetCharacterIdFlag();
    }
    if (pMatrixTag != pMainTag)
    {
        if (pDepthTag == pMatrixTag) dataIdx = 0;
        else if (pCharIdTag == pMatrixTag) dataIdx = 1;
        else { pMatrixTag->Unpack(unpackedData[2]); dataIdx = 2; }
        data.Pos.Matrix_1 = unpackedData[dataIdx].Pos.Matrix_1;
        data.Pos.SetMatrixFlag();
    }
    if (pClassNameTag != pMainTag)
    {
        if (pDepthTag == pClassNameTag) dataIdx = 0;
        else if (pCharIdTag == pClassNameTag) dataIdx = 1;
        else if (pMatrixTag == pClassNameTag) dataIdx = 2;
        else { pClassNameTag->Unpack(unpackedData[3]); dataIdx = 3; }
        data.Pos.ClassName = unpackedData[dataIdx].Pos.ClassName;
        data.Pos.SetClassNameFlag();
    }
    if (pCxFormTag != pMainTag)
    {
        if (pDepthTag == pCxFormTag) dataIdx = 0;
        else if (pCharIdTag == pCxFormTag) dataIdx = 1;
        else if (pMatrixTag == pCxFormTag) dataIdx = 2;
        else if (pClassNameTag == pCxFormTag) dataIdx = 3;
        else { pCxFormTag->Unpack(unpackedData[4]); dataIdx = 4; }
        data.Pos.ColorTransform = unpackedData[dataIdx].Pos.ColorTransform;
        data.Pos.SetCxFormFlag();
    }
    if (pBlendModeTag != pMainTag)
    {
        if (pDepthTag == pBlendModeTag) dataIdx = 0;
        else if (pCharIdTag == pBlendModeTag) dataIdx = 1;
        else if (pMatrixTag == pBlendModeTag) dataIdx = 2;
        else if (pClassNameTag == pBlendModeTag) dataIdx = 3;
        else if (pCxFormTag == pBlendModeTag) dataIdx = 4;
        else { pBlendModeTag->Unpack(unpackedData[5]); dataIdx = 5; }
        data.Pos.BlendMode = unpackedData[dataIdx].Pos.BlendMode;
        data.Pos.SetBlendModeFlag();
    }
    if (pClipDepthTag != pMainTag)
    {
        if (pDepthTag == pClipDepthTag) dataIdx = 0;
        else if (pCharIdTag == pClipDepthTag) dataIdx = 1;
        else if (pMatrixTag == pClipDepthTag) dataIdx = 2;
        else if (pClassNameTag == pClipDepthTag) dataIdx = 3;
        else if (pCxFormTag == pClipDepthTag) dataIdx = 4;
        else if (pBlendModeTag == pClipDepthTag) dataIdx = 5;
        else { pClipDepthTag->Unpack(unpackedData[6]); dataIdx = 6; }
        data.Pos.ClipDepth = unpackedData[dataIdx].Pos.ClipDepth;
        data.Pos.SetClipDepthFlag();
    }
    if (pRatioTag != pMainTag)
    {
        if (pDepthTag == pRatioTag) dataIdx = 0;
        else if (pCharIdTag == pRatioTag) dataIdx = 1;
        else if (pMatrixTag == pRatioTag) dataIdx = 2;
        else if (pClassNameTag == pRatioTag) dataIdx = 3;
        else if (pCxFormTag == pRatioTag) dataIdx = 4;
        else if (pBlendModeTag == pRatioTag) dataIdx = 5;
        else if (pClipDepthTag == pRatioTag) dataIdx = 6;
        else { pRatioTag->Unpack(unpackedData[7]); dataIdx = 7; }
        data.Pos.Ratio = unpackedData[dataIdx].Pos.Ratio;
        data.Pos.SetRatioFlag();
    }
    if (pFiltersTag != pMainTag)
    {
        if (pDepthTag == pFiltersTag) dataIdx = 0;
        else if (pCharIdTag == pFiltersTag) dataIdx = 1;
        else if (pMatrixTag == pFiltersTag) dataIdx = 2;
        else if (pClassNameTag == pFiltersTag) dataIdx = 3;
        else if (pCxFormTag == pFiltersTag) dataIdx = 4;
        else if (pBlendModeTag == pFiltersTag) dataIdx = 5;
        else if (pClipDepthTag == pFiltersTag) dataIdx = 6;
        else if (pRatioTag == pFiltersTag) dataIdx = 7;
        else { pFiltersTag->Unpack(unpackedData[8]); dataIdx = 8; }
        data.Pos.pFilters = unpackedData[dataIdx].Pos.pFilters;
        data.Pos.SetFiltersFlag();
    }
}

// Adds the tag info to a timeline snapshot.
void GFxPlaceObjectUnpacked::AddToTimelineSnapshot(TimelineSnapshot* psnapshot, unsigned frame)
{
    Trace("\n");
    TimelineSnapshot::SnapshotElement* pse = psnapshot->FindDepth(Pos.Depth);
    if (pse && pse->Flags & TimelineSnapshot::Flags_DeadOnArrival)
        pse = NULL; // force to add a new SE, if the prev is marked as DOA
    if (!pse)
    {
        pse = psnapshot->Add(Pos.Depth);
        pse->Tags.Assign(this);
        pse->PlaceType   = (UInt8)PlaceObjectTag::Place_Add;
        pse->CreateFrame = frame;
    }
    else
    {
        // found something at the depth
        pse->Tags.Assign(this);
        pse->CreateFrame = frame;
    }
    pse->Flags |= TimelineSnapshot::Flags_NoReplaceAllowed;
}

void GFxPlaceObjectUnpacked::Trace(const char* str)
{
#ifdef GFX_TRACE_TIMELINE_EXECUTION
    printf ("%s", str);
    printf ("PlaceObject(U): \tAdd\n");
    printf("\tPos.Depth: \t\t%d\n", Pos.Depth);
    printf("\tPos.CharacterId: \t%d\n", Pos.CharacterId);
#else
    SF_UNUSED(str);
#endif
}


void PlaceObjectTag::AddToTimelineSnapshot(TimelineSnapshot* psnapshot, unsigned frame)
{
    Trace("\n");
    UInt16 depth = GetDepth();
    SF_DEBUG_ASSERT(depth != 7, "blag");
    TimelineSnapshot::SnapshotElement* pse = psnapshot->FindDepth(depth);
    if (pse && pse->Flags & TimelineSnapshot::Flags_DeadOnArrival)
        pse = NULL; // force to add a new SE, if the prev is marked as DOA
    if (!pse)
    {
        pse = psnapshot->Add(depth);
        pse->PlaceType   = (UInt8)PlaceObjectTag::Place_Add;
        pse->Tags.Assign(this);
        pse->CreateFrame = frame;
    }
    else
    {
        // found something at the depth
        pse->Tags.Assign(this);
        pse->CreateFrame = frame;
    }
    pse->Flags |= TimelineSnapshot::Flags_NoReplaceAllowed;
}

void PlaceObjectTag::Trace(const char* str)
{
#ifdef GFX_TRACE_TIMELINE_EXECUTION
    UnpackedData data;
    Unpack(data);
    printf ("%s", str);
    printf ("PlaceObject: \tAdd\n");
    printf("\tPos.Depth: \t\t%d\n", data.Pos.Depth);
    printf("\tPos.CharacterId: \t%d\n", data.Pos.CharacterId);
#else
    SF_UNUSED(str);
#endif
}


//
// ***** GFxPlaceObjectUnpacked
//


// Place/move/whatever our object in the given pMovie.
void GFxPlaceObjectUnpacked::Execute(DisplayObjContainer* m)
{
    ASStringManager *psm = m->GetStringManager();
    ASString       sname(psm->CreateEmptyString());
    m->AddDisplayObject(Pos, sname, NULL, 0, SF_MAX_UINT, DisplayList::Flags_PlaceObject);
}


//
// ***** PlaceObjectTag
//

PlaceObjectTag::PlaceObjectTag()
: HasCxForm(false) {}

PlaceObjectTag::~PlaceObjectTag() {}

UPInt    PlaceObjectTag::ComputeDataSize(Stream* pin)
{   
    int tagStart = pin->Tell();
    int tagEnd = pin->GetTagEndPosition();
    int len = tagEnd - tagStart;    

#ifdef GFX_VERBOSE_PARSE
    if (pin->IsVerboseParse())
    {
        // Original PlaceObject tag; very simple.
        CharPosInfo pos;
        pos.CharacterId = ResourceId(pin->ReadU16());
        pos.Depth       = pin->ReadU16();
        pin->ReadMatrix(&pos.Matrix_1);
        pin->LogParse("  CharId = %d\n"
            "  depth = %d\n"
            "  mat = \n",
            pos.CharacterId.GetIdIndex(), pos.Depth);
        pin->LogParseClass(pos.Matrix_1);
        if (pin->Tell() < pin->GetTagEndPosition())
        {
            pin->ReadCxformRgb(&pos.ColorTransform);
            pin->LogParse("  cxform:\n");
            pin->LogParseClass(pos.ColorTransform);
        }

        // reset stream position
        pin->SetPosition(tagStart);
    }
#endif

    return (UPInt)len;
}

CharPosInfoFlags PlaceObjectTag::GetFlags() const
{
    return (UInt8) (CharPosInfoFlags::Flags_HasCharacterId | 
        CharPosInfoFlags::Flags_HasDepth | 
        CharPosInfoFlags::Flags_HasMatrix |
        ((HasCxForm) ? CharPosInfoFlags::Flags_HasCxform : 0x0));
}

UInt16 PlaceObjectTag::GetDepth() const 
{
    StreamContext    sc(pData);
    sc.Skip(2);
    return (sc.ReadU16());
}

// Place/move/whatever our object in the given pMovie.
void PlaceObjectTag::Execute(DisplayObjContainer* m)
{
    PlaceObjectTag::UnpackedData data;
    Unpack(data);
    ASStringManager *psm = m->GetStringManager();
    SF_ASSERT(m->IsSprite());
    m->CharToSprite_Unsafe()->AddDisplayObject
        (data.Pos, psm->CreateEmptyString(), NULL, 0, 
         SF_MAX_UINT, DisplayList::Flags_PlaceObject);
}

// Unpack data
void PlaceObjectTag::Unpack(GFxPlaceObjectBase::UnpackedData& data)
{
    StreamContext    sc(pData);
    data.Name           = NULL;
    data.pEventHandlers = NULL;
    data.PlaceType      = Place_Add;
    data.Pos.SetCharacterIdFlag();
    data.Pos.CharacterId= ResourceId(sc.ReadU16());
    data.Pos.SetDepthFlag();
    data.Pos.Depth      = sc.ReadU16();
    data.Pos.SetMatrixFlag();
    sc.ReadMatrix(&data.Pos.Matrix_1);
    if (HasCxForm)
    {
        data.Pos.SetCxFormFlag();
        sc.ReadCxformRgb(&data.Pos.ColorTransform);
    }
}

void PlaceObjectTag::CheckForCxForm(UPInt dataSz)
{
    StreamContext    sc(pData);
    Matrix2F   mat;
    sc.Skip(4);
    sc.ReadMatrix(&mat);
    HasCxForm = (sc.CurByteIndex < (dataSz-1));
}


//
// ***** PlaceObject2Tag
//

PlaceObject2Tag::~PlaceObject2Tag() 
{
}

UPInt    PlaceObject2Tag::ComputeDataSize(Stream* pin, unsigned movieVersion)
{   
    int tagStart = pin->Tell();
    int tagEnd = pin->GetTagEndPosition();
    int len = tagEnd - tagStart;

    SF_UNUSED(movieVersion);

#ifdef GFX_VERBOSE_PARSE
    if (pin->IsVerboseParse())
    {
        UByte   po2Flags = pin->ReadU8();
        UInt32  allFlags = 0;
        char*   pname = NULL;

        CharPosInfo pos;
        pos.Depth = pin->ReadU16();
        if (po2Flags & PO2_HasChar)
        {
            pos.SetCharacterIdFlag();
            pos.CharacterId = ResourceId(pin->ReadU16());
        }
        if (po2Flags & PO2_HasMatrix)
        {
            pos.SetMatrixFlag();
            pin->ReadMatrix(&pos.Matrix_1);
        }
        if (po2Flags & PO2_HasCxform)
        {
            pos.SetCxFormFlag();
            pin->ReadCxformRgba(&pos.ColorTransform);
        }
        if (po2Flags & PO2_HasRatio)
        {
            pos.SetRatioFlag();
            pos.Ratio = (float)pin->ReadU16() / (float)65535;
        }
        if (po2Flags & PO2_HasName)
            pname = pin->ReadString(pin->GetHeap());
        if (po2Flags & PO2_HasClipBracket)
        {
            pos.SetClipDepthFlag();
            pos.ClipDepth = pin->ReadU16(); 
        }
        if (po2Flags & PO2_HasActions)
        {
            UInt16  reserved = pin->ReadU16();
            SF_UNUSED(reserved);
            // The logical 'or' of all the following handlers.
            // I don't think we care about this...
            bool    u32Flags = (movieVersion >= 6);
            if (u32Flags)
                allFlags = pin->ReadU32();
            else
                allFlags = pin->ReadU16();
            SF_UNUSED(allFlags);

            // NOTE: The event handlers are not logged.
        }

        // Reset stream position
        pin->SetPosition(tagStart);

        // Separate parse logging to avoid extra tests/logic during execution.
        // Particularly important because VC++ does not compile out var-args.
        bool    hasChar         = (po2Flags & PO2_HasChar) != 0;
        bool    flagMove        = (po2Flags & PO2_FlagMove) != 0;
        pin->LogParse("  depth = %d\n", pos.Depth);
        if (po2Flags & PO2_HasChar)
            pin->LogParse("  char id = %d\n", pos.CharacterId.GetIdIndex());
        if (po2Flags & PO2_HasMatrix)
        {
            pin->LogParse("  mat:\n");
            pin->LogParseClass(pos.Matrix_1);
        }
        if (po2Flags & PO2_HasCxform)
        {
            pin->LogParse("  cxform:\n");
            pin->LogParseClass(pos.ColorTransform);
        }
        if (po2Flags & PO2_HasRatio)
            pin->LogParse("  ratio: %f\n", pos.Ratio);
        if (po2Flags & PO2_HasName)            
            pin->LogParse("  name = %s\n", pname ? pname : "<null>");
        if (po2Flags & PO2_HasClipBracket)            
            pin->LogParse("  ClipDepth = %d\n", pos.ClipDepth);
        if (po2Flags & PO2_HasActions)
            pin->LogParse("  actions: flags = 0x%X\n", (unsigned int)allFlags);
        if (hasChar && flagMove)
            pin->LogParse("    * (replace)\n");
        else if (!hasChar && flagMove)
            pin->LogParse("    * (move)\n");

        if (pname)
            SF_FREE(pname);

        // reset stream position
        pin->SetPosition(tagStart);
    }
#endif

    return (UPInt)len;
}

CharPosInfoFlags PlaceObject2Tag::GetFlags() const
{
    StreamContext    sc(pData);
    UByte po2Flags = sc.ReadU8();
    return (po2Flags & 0x5F);
}

UInt16 PlaceObject2Tag::GetDepth() const 
{
    StreamContext    sc(pData);
    UByte po2Flags = sc.ReadU8();
    if (po2Flags & PO2_HasActions)
        sc.Skip(sizeof(GFxPlaceObjectBase::EventArrayType*));
    return (sc.ReadU16());
}

GFxPlaceObjectBase::PlaceActionType PlaceObject2Tag::GetPlaceType() const
{
    StreamContext    sc(pData);
    UByte po2Flags = sc.ReadU8();
    bool    hasChar         = (po2Flags & PO2_HasChar) != 0;
    bool    flagMove        = (po2Flags & PO2_FlagMove) != 0;
    if (hasChar && flagMove)
        return PlaceObjectTag::Place_Replace;
    else if (!hasChar && flagMove)
        return PlaceObjectTag::Place_Move;
    return PlaceObjectTag::Place_Add;
}

// Place/move/whatever our object in the given pMovie.
void PlaceObject2Tag::ExecuteBase(DisplayObjContainer* m, UInt8 version)
{
    PlaceObjectTag::UnpackedData data;
    UnpackBase(data, version);
    switch (data.PlaceType)
    {
    case PlaceObjectTag::Place_Add:
        // Original PlaceObject (tag 4) doesn't do replacement
        {
            ASStringManager *psm = m->GetStringManager();
            ASString       sname((data.Name == NULL) ? 
                psm->CreateEmptyString() : psm->CreateString(data.Name));
            m->AddDisplayObject
                (data.Pos, sname, data.pEventHandlers, 0, 
                 SF_MAX_UINT, DisplayList::Flags_PlaceObject);
        }
        break;

    case PlaceObjectTag::Place_Move:
        m->MoveDisplayObject(data.Pos);
        break;

    case PlaceObjectTag::Place_Replace:
        {
            ASStringManager *psm = m->GetStringManager();
            ASString       sname((data.Name == NULL) ? psm->CreateEmptyString() : psm->CreateString(data.Name));
            DisplayObjectBase*  newChar;
            m->CreateAndReplaceDisplayObject(data.Pos, sname, &newChar);
        }        
        break;
    }
}

void PlaceObject2Tag::SetEventHandlersPtr(UByte* pdata, EventArrayType* peh)
{
#if defined(SF_CPU_OTHER) || defined(SF_CPU_MIPS)
    memcpy(pdata+1, &peh, sizeof(GFxPlaceObjectBase::EventArrayType*));
#else
    GFxPlaceObjectBase::EventArrayType** ptr = ((GFxPlaceObjectBase::EventArrayType**)(pdata+1));
    *ptr = peh;
#endif
}

GFxPlaceObjectBase::EventArrayType* PlaceObject2Tag::GetEventHandlersPtr(const UByte* pdata)
{
#if defined(SF_CPU_OTHER) || defined(SF_CPU_MIPS)
    GFxPlaceObjectBase::EventArrayType* ptr;
    memcpy(&ptr, pdata+1, sizeof(GFxPlaceObjectBase::EventArrayType*));
    return ptr;
#else
    return *((GFxPlaceObjectBase::EventArrayType**)(pdata+1));
#endif
}

bool PlaceObject2Tag::HasEventHandlers(Stream* pin)
{
    UByte po2Flags = pin->ReadU8();
    pin->SetPosition(pin->Tell()-1);
    return ((po2Flags & PO2_HasActions) != 0);
}

void PlaceObject2Tag::RestructureForEventHandlers(UByte* pdata)
{
    // Move flags to byte 0
    pdata[0] = pdata[sizeof(GFxPlaceObjectBase::EventArrayType*)];
    // Zero event handler pointer
#if defined(SF_CPU_OTHER) || defined(SF_CPU_MIPS)
    memset(pdata+1, 0, sizeof(GFxPlaceObjectBase::EventArrayType*));
#else
    *((GFxPlaceObjectBase::EventArrayType**)(pdata+1)) = NULL;
#endif
}

GFxPlaceObjectBase::EventArrayType* PlaceObject2Tag::UnpackEventHandlers()
{
    GFxPlaceObjectBase::EventArrayType* peh = NULL;
    StreamContext    sc(pData);
    UByte   po2Flags = sc.ReadU8();
    if (po2Flags & PO2_HasActions)
    {
        peh = GetEventHandlersPtr(pData);
        if (!peh)
        {
            PlaceObjectTag::UnpackedData data;
            Unpack(data); 
            peh = data.pEventHandlers;
        }
    }
    return peh;
}

void PlaceObject2Tag::UnpackBase(PlaceObjectTag::UnpackedData& data, UInt8 version)
{
    StreamContext    sc(pData);
    UByte   po2Flags    = sc.ReadU8();
    if (po2Flags & PO2_HasActions)
        sc.Skip(sizeof(GFxPlaceObjectBase::EventArrayType*));
    data.Pos.Depth       = sc.ReadU16();
    if (po2Flags & PO2_HasChar)
    {
        data.Pos.SetCharacterIdFlag();
        data.Pos.CharacterId = ResourceId(sc.ReadU16());
    }
    if (po2Flags & PO2_HasMatrix)
    {
        data.Pos.SetMatrixFlag();
        sc.ReadMatrix(&data.Pos.Matrix_1);
    }
    if (po2Flags & PO2_HasCxform)
    {
        data.Pos.SetCxFormFlag();
        sc.ReadCxformRgba(&data.Pos.ColorTransform);
    }
    if (po2Flags & PO2_HasRatio)
    {
        data.Pos.SetRatioFlag();
        data.Pos.Ratio = (float)sc.ReadU16() / (float)65535;
    }
    if (po2Flags & PO2_HasName)
    {
        sc.Align();
        data.Name = (const char*)pData + sc.CurByteIndex;
        char c;
        while ( (c=sc.ReadU8()) != 0) {}
    }
    else
        data.Name = NULL;
    if (po2Flags & PO2_HasClipBracket)
    {
        data.Pos.SetClipDepthFlag();
        data.Pos.ClipDepth = sc.ReadU16(); 
    }
    if (po2Flags & PO2_HasActions)
    {   
        ProcessEventHandlers(data, sc, pData, version);
    }
    else
        data.pEventHandlers = NULL;

    bool    hasChar         = (po2Flags & PO2_HasChar) != 0;
    bool    flagMove        = (po2Flags & PO2_FlagMove) != 0;
    data.PlaceType = PlaceObjectTag::Place_Add;
    if (hasChar && flagMove)
    {
        // Remove whatever's at Depth, and put DisplayObjectBase there.
        data.PlaceType = PlaceObjectTag::Place_Replace;
    }
    else if (!hasChar && flagMove)
    {
        // Moves the object at Depth to the new location.
        data.PlaceType = PlaceObjectTag::Place_Move;
    }
}


//
// ***** PlaceObject3Tag
//

PlaceObject3Tag::~PlaceObject3Tag() 
{
}

UPInt    PlaceObject3Tag::ComputeDataSize(Stream* pin)
{   
    int tagStart = pin->Tell();
    int tagEnd = pin->GetTagEndPosition();
    int len = tagEnd - tagStart;

#ifdef GFX_VERBOSE_PARSE
    if (pin->IsVerboseParse())
    {
        UByte   po2Flags = pin->ReadU8();
        UByte   po3Flags = pin->ReadU8();
        UInt32  allFlags = 0;
        char*   pname = NULL;
        char*   classname = NULL;

        CharPosInfo pos;
        pos.Depth = pin->ReadU16();                
        if (po3Flags & PlaceObject3Tag::PO3_HasClassName)
        {
            pos.SetClassNameFlag();
            pos.ClassName = classname = pin->ReadString(pin->GetHeap());
        }
        if (po2Flags & PlaceObject2Tag::PO2_HasChar)
        {
            pos.SetCharacterIdFlag();
            pos.CharacterId = ResourceId(pin->ReadU16());
        }

        if (po2Flags & PlaceObject2Tag::PO2_HasMatrix)
        {
            pos.SetMatrixFlag();
            pin->ReadMatrix(&pos.Matrix_1);
        }
        if (po2Flags & PlaceObject2Tag::PO2_HasCxform)
        {
            pos.SetCxFormFlag();
            pin->ReadCxformRgba(&pos.ColorTransform);
        }

        if (po2Flags & PlaceObject2Tag::PO2_HasRatio)
        {
            pos.SetRatioFlag();
            pos.Ratio = (float)pin->ReadU16() / (float)65535;
        }
        if (po2Flags & PlaceObject2Tag::PO2_HasName)
            pname = pin->ReadString(pin->GetHeap());
        if (po2Flags & PlaceObject2Tag::PO2_HasClipBracket)
        {
            pos.SetClipDepthFlag();
            pos.ClipDepth = pin->ReadU16(); 
        }
        if (po3Flags & PO3_HasFilters)
        {
            pos.SetFiltersFlag();
            LoadFilters(pin, 0);
        }
        if (po3Flags & PO3_HasBlendMode)
        {
            UByte   blendMode = pin->ReadU8();
            if ((blendMode < 1) || (blendMode>14))
            {
                SF_DEBUG_WARNING(1, "PlaceObject3Tag::Read - loaded blend mode out of range");
                blendMode = 1;
            }
            // Assign the mode.
            pos.SetBlendModeFlag();
            pos.BlendMode = (UInt8) blendMode;
        }
        if (po3Flags & PO3_BitmapCaching)
        {
            pin->ReadU8();
        }
        if (po2Flags & PlaceObject2Tag::PO2_HasActions)
        {
            UInt16  reserved = pin->ReadU16();
            SF_UNUSED(reserved);
            allFlags = pin->ReadU32();
            SF_UNUSED(allFlags);

            // NOTE: The event handlers are not logged.
        }

        // Reset stream
        pin->SetPosition(tagStart);

        // Separate parse logging to avoid extra tests/logic during execution.
        // Particularly important because VC++ does not compile out var-args.
        bool    hasChar         = (po2Flags & PlaceObject2Tag::PO2_HasChar) != 0;
        bool    flagMove        = (po2Flags & PlaceObject2Tag::PO2_FlagMove) != 0;
        pin->LogParse("  depth = %d\n", pos.Depth);
        if (po2Flags & PlaceObject2Tag::PO2_HasChar)
            pin->LogParse("  char id = %d\n", pos.CharacterId.GetIdIndex());
        if (po3Flags & PlaceObject3Tag::PO3_HasClassName)
        {
            pin->LogParse("  classname = %s\n", pos.ClassName);
        }
        if (po2Flags & PlaceObject2Tag::PO2_HasMatrix)
        {
            pin->LogParse("  mat:\n");
            pin->LogParseClass(pos.Matrix_1);
        }
        if (po2Flags & PlaceObject2Tag::PO2_HasCxform)
        {
            pin->LogParse("  cxform:\n");
            pin->LogParseClass(pos.ColorTransform);
        }
        if (po2Flags & PlaceObject2Tag::PO2_HasRatio)
            pin->LogParse("  ratio: %f\n", pos.Ratio);
        if (po2Flags & PlaceObject2Tag::PO2_HasName)           
            pin->LogParse("  name = %s\n", pname ? pname : "<null>");
        if (po2Flags & PlaceObject2Tag::PO2_HasClipBracket)            
            pin->LogParse("  ClipDepth = %d\n", pos.ClipDepth);
        if (po3Flags & PO3_HasBlendMode)
            pin->LogParse("  blend mode = %d\n", pos.BlendMode);
        if (po2Flags & PlaceObject2Tag::PO2_HasActions)
            pin->LogParse("  actions: flags = 0x%X\n", (unsigned int)allFlags);
        if (hasChar && flagMove)
            pin->LogParse("    * (replace)\n");
        else if (!hasChar && flagMove)
            pin->LogParse("    * (move)\n");

        if (pname)
            SF_FREE(pname);
        if (classname)
            SF_FREE(classname);

        // reset stream position
        pin->SetPosition(tagStart);
    }
#endif

    return (UPInt)len;
}


CharPosInfoFlags PlaceObject3Tag::GetFlags() const
{
    StreamContext    sc(pData);
    UByte po2Flags = sc.ReadU8();
    if (po2Flags & PlaceObject2Tag::PO2_HasActions)
        sc.Skip(sizeof(GFxPlaceObjectBase::EventArrayType*));
    UByte po3Flags = sc.ReadU8();
    return (UInt8)((po2Flags & 0x5f) | ((po3Flags & PO3_HasFilters) << 5) | ((po3Flags & PO3_HasBlendMode) << 6));
}
UInt16 PlaceObject3Tag::GetDepth() const 
{
    StreamContext    sc(pData);
    UByte po2Flags = sc.ReadU8();
    if (po2Flags & PlaceObject2Tag::PO2_HasActions)
        sc.Skip(sizeof(GFxPlaceObjectBase::EventArrayType*));
    sc.Skip(1);
    return (sc.ReadU16());
}

GFxPlaceObjectBase::PlaceActionType PlaceObject3Tag::GetPlaceType() const
{
    StreamContext    sc(pData);
    UByte po2Flags = sc.ReadU8();
    bool    hasChar         = (po2Flags & PlaceObject2Tag::PO2_HasChar) != 0;
    bool    flagMove        = (po2Flags & PlaceObject2Tag::PO2_FlagMove) != 0;
    if (hasChar && flagMove)
        return PlaceObjectTag::Place_Replace;
    else if (!hasChar && flagMove)
        return PlaceObjectTag::Place_Move;
    return PlaceObjectTag::Place_Add;
}

// Place/move/whatever our object in the given pMovie.
void PlaceObject3Tag::Execute(DisplayObjContainer* m)
{
    PlaceObjectTag::UnpackedData data;
    Unpack(data);
    switch (data.PlaceType)
    {
    case PlaceObjectTag::Place_Add:
        // Original PlaceObject (tag 4) doesn't do replacement
        {
            ASStringManager *psm = m->GetStringManager();
            ASString       sname((data.Name == NULL) ? 
                psm->CreateEmptyString() : psm->CreateString(data.Name));
            m->AddDisplayObject
                (data.Pos, sname, data.pEventHandlers, 0, 
                 SF_MAX_UINT, DisplayList::Flags_PlaceObject);
        }
        break;

    case PlaceObjectTag::Place_Move:
        m->MoveDisplayObject(data.Pos);
        break;

    case PlaceObjectTag::Place_Replace:
        {
            ASStringManager *psm = m->GetStringManager();
            ASString       sname((data.Name == NULL) ? psm->CreateEmptyString() : psm->CreateString(data.Name));
            DisplayObjectBase*  newChar;
            m->CreateAndReplaceDisplayObject(data.Pos, sname, &newChar);
        }        
        break;
    }
}

GFxPlaceObjectBase::EventArrayType* PlaceObject3Tag::UnpackEventHandlers()
{
    GFxPlaceObjectBase::EventArrayType* peh = NULL;
    StreamContext    sc(pData);
    UByte   po2Flags = sc.ReadU8();
    if (po2Flags & PlaceObject2Tag::PO2_HasActions)
    {
        peh = PlaceObject2Tag::GetEventHandlersPtr(pData);
        if (!peh)
        {
            PlaceObjectTag::UnpackedData data;
            Unpack(data);
            peh = data.pEventHandlers;
        }
    }
    return peh;
}

void PlaceObject3Tag::Unpack(PlaceObjectTag::UnpackedData& data)
{
    StreamContext    sc(pData);    
    UByte   po2Flags = sc.ReadU8();
    if (po2Flags & PlaceObject2Tag::PO2_HasActions)
        sc.Skip(sizeof(GFxPlaceObjectBase::EventArrayType*));
    UByte   po3Flags = sc.ReadU8();
    data.Pos.Depth   = sc.ReadU16();
    if (po3Flags & PlaceObject3Tag::PO3_HasClassName)
    {
        data.Pos.SetClassNameFlag();
        sc.Align();
        data.Pos.ClassName = (const char*)pData + sc.CurByteIndex;
        char c;
        while ( (c=sc.ReadU8()) != 0) {}
    }
    if (po2Flags & PlaceObject2Tag::PO2_HasChar)
    {
        data.Pos.SetCharacterIdFlag();
        data.Pos.CharacterId = ResourceId(sc.ReadU16());
    }
    if (po2Flags & PlaceObject2Tag::PO2_HasMatrix)
    {
        data.Pos.SetMatrixFlag();
        sc.ReadMatrix(&data.Pos.Matrix_1);
    }
    if (po2Flags & PlaceObject2Tag::PO2_HasCxform)
    {
        data.Pos.SetCxFormFlag();
        sc.ReadCxformRgba(&data.Pos.ColorTransform);
    }
    if (po2Flags & PlaceObject2Tag::PO2_HasRatio)
    {
        data.Pos.SetRatioFlag();
        data.Pos.Ratio = (float)sc.ReadU16() / (float)65535;
    }
    if (po2Flags & PlaceObject2Tag::PO2_HasName)
    {
        sc.Align();
        data.Name = (const char*)pData + sc.CurByteIndex;
        char c;
        while ( (c=sc.ReadU8()) != 0) {}
    }
    else
        data.Name = NULL;
    if (po2Flags & PlaceObject2Tag::PO2_HasClipBracket)
    {
        data.Pos.ClipDepth = sc.ReadU16(); 
        data.Pos.SetClipDepthFlag();
    }
    if (po3Flags & PO3_HasFilters)
    {
        data.Pos.SetFiltersFlag();
        Ptr<Render::FilterSet> filters =
            *SF_HEAP_NEW_ID(Memory::GetGlobalHeap(), StatMD_Tags_Mem) Render::FilterSet;
        if (LoadFilters(&sc, filters.GetPtr()))
            data.Pos.pFilters = filters;
    }
    if (po3Flags & PO3_HasBlendMode)
    {
        data.Pos.SetBlendModeFlag();
        UByte   blendMode = sc.ReadU8();
        if ((blendMode < 1) || (blendMode > 14))
        {
            SF_DEBUG_WARNING1(1, "PlaceObject3Tag::Read - loaded blend mode out of [0, 14] range : %d", blendMode);
            blendMode = 1;
        }
        // Assign the mode.
        data.Pos.BlendMode = (UInt8) blendMode;
    }
    if (po3Flags & PO3_BitmapCaching)
    {
        UByte cabProperty = sc.ReadU8();
        Ptr<Render::FilterSet> pfilters = data.Pos.pFilters;
        if (cabProperty == 1)
        {
            if (!pfilters)
            {
                pfilters = *SF_HEAP_NEW_ID(Memory::GetGlobalHeap(), StatMD_Tags_Mem) Render::FilterSet;
                data.Pos.pFilters = pfilters;
            }
            pfilters->SetCacheAsBitmap(true);
        }
        else if (pfilters)
        {
            pfilters->SetCacheAsBitmap(false);
        }
    }
    if (po3Flags & PlaceObject3Tag::PO3_Invisible)
    {
        data.Pos.Visible = sc.ReadU8();
        data.Pos.SetVisibilityFlag();
    }
    if (po2Flags & PlaceObject2Tag::PO2_HasActions)
    {        
        ProcessEventHandlers(data, sc, pData);
    }
    else
        data.pEventHandlers = NULL;

    bool    hasChar         = (po2Flags & PlaceObject2Tag::PO2_HasChar) != 0;
    bool    flagMove        = (po2Flags & PlaceObject2Tag::PO2_FlagMove) != 0;
    data.PlaceType = PlaceObjectTag::Place_Add;
    if (hasChar && flagMove)
    {
        // Remove whatever's at Depth, and put DisplayObjectBase there.
        data.PlaceType = PlaceObjectTag::Place_Replace;
    }
    else if (!hasChar && flagMove)
    {
        // Moves the object at Depth to the new location.
        data.PlaceType = PlaceObjectTag::Place_Move;
    }
}

void PlaceObject2Tag::AddToTimelineSnapshot(TimelineSnapshot* psnapshot, unsigned frame)
{
    Trace("\n");
    UInt16 depth = GetDepth();
    UInt8  placeType = (UInt8)GetPlaceType();
    TimelineSnapshot::SnapshotElement* pse = psnapshot->FindDepth(depth);
    if (pse && pse->Flags & TimelineSnapshot::Flags_DeadOnArrival)
        pse = NULL; // force to add a new SE, if the prev is marked as DOA

    if (!pse)
    {
        pse = psnapshot->Add(depth);
        pse->PlaceType   = placeType;
        pse->Tags.Assign(this);
        pse->CreateFrame = frame;
    }
    else
    {
        // found something at the depth
        switch(placeType)
        {
        case PlaceObjectTag::Place_Move:
            pse->Tags.Union(this);
            break;
        case PlaceObjectTag::Place_Replace:
            if (pse->PlaceType != TimelineSnapshot::Place_Add)
                pse->PlaceType = PlaceObjectTag::Place_Replace;
            pse->Tags.Union(this);
            pse->CreateFrame = frame;
            break;
        default:
            pse->Tags.Assign(this);
            pse->CreateFrame = frame;
        }
    }
}

void PlaceObject2Tag::TraceBase(const char* str, UInt8 version)
{
#ifdef GFX_TRACE_TIMELINE_EXECUTION
    PlaceObject2Tag::UnpackedData   data;
    UnpackBase(data, version);
    printf ("%s", str);
    printf ("PlaceObject2Tag: \t%s\n", (data.PlaceType ==Place_Add)?"Add":((data.PlaceType ==Place_Move)?"Move":"Replace"));
    printf ("\tName: \t%s\n", data.Name);
    printf("\tPos.Depth: \t\t%d\n", data.Pos.Depth);
    printf("\tPos.CharacterId: \t%d\n", data.Pos.CharacterId);
    printf("\tPos.ClipDepth: \t\t%d\n", data.Pos.ClipDepth);
    if (data.Pos.HasCxform())
    {
        char buff[2560];
        Format(buff, sizeof(buff), data.Pos.ColorTransform);
        printf ("\tPos.Cxform:\n");
        printf("%s", buff);

    }
    if (data.Pos.HasMatrix())
    {
        char buff[2560];
        Format(buff, sizeof(buff), data.Pos.Matrix_1);
        printf ("\tPos.Matrix:\n");
        printf("%s", buff);
    }
    if (data.Pos.HasBlendMode())
        printf("\tPos.BlendMode: \t%d\n", data.Pos.BlendMode);
    if (data.Pos.TextFilter)
        printf("\tPos.HasTextFilter\n");
    if (data.Pos.HasInvisible())
        printf("\tPos.HasInvisible\n");
#else
    SF_UNUSED2(str, version);
#endif
}

void PlaceObject3Tag::AddToTimelineSnapshot(TimelineSnapshot* psnapshot, unsigned frame)
{
    Trace("\n");
    UInt16 depth = GetDepth();
    UInt8 placeType = (UInt8)GetPlaceType();
    TimelineSnapshot::SnapshotElement* pse = psnapshot->FindDepth(depth);
    if (pse && pse->Flags & TimelineSnapshot::Flags_DeadOnArrival)
        pse = NULL; // force to add a new SE, if the prev is marked as DOA
    if (!pse)
    {
        pse = psnapshot->Add(depth);
        pse->PlaceType   = placeType;
        pse->Tags.Assign(this);
        pse->CreateFrame = frame;
    }
    else
    {
        // found something at the depth
        switch(placeType)
        {
        case PlaceObjectTag::Place_Move:
            pse->Tags.Union(this);
            break;
        case PlaceObjectTag::Place_Replace:
            if (pse->PlaceType != TimelineSnapshot::Place_Add)
                pse->PlaceType = PlaceObject2Tag::Place_Replace;
            pse->Tags.Union(this);
            pse->CreateFrame = frame;
            break;
        default:
            pse->Tags.Assign(this);
            pse->CreateFrame = frame;
        }
    }
}

void PlaceObject3Tag::Trace(const char* str)
{
#ifdef GFX_TRACE_TIMELINE_EXECUTION
    PlaceObject2Tag::UnpackedData   data;
    Unpack(data);
    printf ("%s", str);
    printf ("PlaceObject3Tag: \t%s\n", (data.PlaceType ==PlaceObject2Tag::Place_Add)?"Add":((data.PlaceType ==PlaceObject2Tag::Place_Move)?"Move":"Replace"));
    printf ("\tName: \t%s\n", data.Name);
    printf("\tPos.Depth: \t\t%d\n", data.Pos.Depth);
    printf("\tPos.ClassName: \t\t'%s'\n", data.Pos.ClassName);
    printf("\tPos.CharacterId: \t%d\n", data.Pos.CharacterId);
    printf("\tPos.ClipDepth: \t\t%d\n", data.Pos.ClipDepth);
    if (data.Pos.HasCxform())
    {
        char buff[2560];
        Format(buff, sizeof(buff), data.Pos.ColorTransform);
        printf ("\tPos.Cxform:\n");
        printf("%s", buff);

    }
    if (data.Pos.HasMatrix())
    {
        char buff[2560];
        Format(buff, sizeof(buff), data.Pos.Matrix_1);
        printf ("\tPos.Matrix:\n");
        printf("%s", buff);
    }
    if (data.Pos.HasBlendMode())
        printf("\tPos.BlendMode: \t%d\n", data.Pos.BlendMode);
    if (data.Pos.TextFilter)
        printf("\tPos.HasTextFilter\n");
    if (data.Pos.HasInvisible())
        printf("\tPos.HasInvisible\n");
#else
    SF_UNUSED(str);
#endif
}

//
// ***** RemoveObjectTag
//

void    RemoveObjectTag::Read(LoadProcess* p)
{
    Id = p->ReadU16();
    Depth = p->ReadU16();
}

void    RemoveObjectTag::Execute(DisplayObjContainer* m)
{
    m->RemoveDisplayObject(Depth, ResourceId(Id));
}

void RemoveObjectTag::Trace(const char* str)
{
#ifdef GFX_TRACE_TIMELINE_EXECUTION
    printf ("%s", str);
    printf ("RemoveObject:\n");
    printf("\tDepth: \t%d\n", Depth);
    printf("\tCharacterId: %d\n", Id);
#else
    SF_UNUSED(str);
#endif
}

//
// ***** RemoveObject2Tag
//

void    RemoveObject2Tag::Read(LoadProcess* p)
{
    Depth = p->ReadU16();
}

void    RemoveObject2Tag::Execute(DisplayObjContainer* m)
{
    m->RemoveDisplayObject(Depth, ResourceId());
}

void RemoveObject2Tag::AddToTimelineSnapshot(TimelineSnapshot* psnapshot, unsigned)
{
    Trace("\n");
    UPInt index;
    TimelineSnapshot::SnapshotElement* pse = psnapshot->FindDepth(Depth, &index);
    if (pse && pse->PlaceType != TimelineSnapshot::Place_Add)
    {
        psnapshot->RemoveAtIndex(index);
        pse = NULL;
    }
    if (pse)
    {
        if (pse->Tags.HasMainTag() && psnapshot->Direction == TimelineSnapshot::Direction_Forward)
        {
            // forward direction: check if the event handler array contains onUnload event.
            // If it does, do not remove the element, just set flag Flags_DeadOnArrive.
            // ExecuteSnapshot will create the object, execute onInitialize, onConstruct,
            // onLoad and onUnload events and then kill the object.

            GFxPlaceObjectBase::EventArrayType* pEventHandlers = 
                pse->Tags.pMainTag->UnpackEventHandlers();

            if (pEventHandlers)
            {
                CheckEventHandlers((void**)&pse, pEventHandlers);
            }
        }
        if (pse)
            psnapshot->RemoveAtIndex(index);
    }
    if (!pse)
    {
        if (psnapshot->Direction == TimelineSnapshot::Direction_Forward)
        {
            pse = psnapshot->Add(Depth);
            pse->Depth  = Depth;
            pse->PlaceType  = TimelineSnapshot::Place_Remove;
            // need to keep this SE, don't delete it from Timeline Snapshot
            pse->Flags |= TimelineSnapshot::Flags_DeadOnArrival; 
        }
        else
        {
            // if we are moving backward, then RemoveObject tag should not exist
            // w/o corresponding Add tag. So, at the moment I consider this situation
            // as an error. (!AB)
            SF_ASSERT(0);
        }
    }
}

void RemoveObject2Tag::Trace(const char* str)
{
#ifdef GFX_TRACE_TIMELINE_EXECUTION
    printf ("%s", str);
    printf ("RemoveObject2:\n");
    printf("\tDepth: \t%d\n", Depth);
#else
    SF_UNUSED(str);
#endif
}

//
// ***** SetBackgroundColorTag
//

void    SetBackgroundColorTag::Execute(DisplayObjContainer* m)
{
    //!AB: background should be set only once and only by the _level0 movie
    MovieImpl *pmroot = m->GetMovieImpl();
    if (!pmroot->IsBackgroundSetByTag() /* && pmroot->GetMainMovie() == m*/) //?
    {
        float         alpha  = pmroot->GetBackgroundAlpha();
        Color.SetAlpha( UByte(Alg::IRound(alpha * 255.0f)) );
        pmroot->SetBackgroundColorByTag(Color);
    }
}

void    SetBackgroundColorTag::Read(LoadProcess* p)
{
    p->GetStream()->ReadRgb(&Color);

    p->LogParse("  SetBackgroundColor: (%d %d %d)\n",
        (int)Color.GetRed(), (int)Color.GetGreen(), (int)Color.GetBlue());
}

}} //namespace Scaleform { namespace GFx {
