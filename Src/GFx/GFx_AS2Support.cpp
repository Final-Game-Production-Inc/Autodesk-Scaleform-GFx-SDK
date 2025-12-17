/**************************************************************************

Filename    :   GFx_AS2Support.cpp
Content     :   Implementation of AS2 support state
Created     :   Dec, 2009
Authors     :   Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "GFx/GFx_LoadProcess.h"
#include "GFx/GFx_AS2Support.h"
#include "GFx/GFx_CharacterDef.h"
#include "GFx/GFx_PlayerImpl.h"
#include "GFx/GFx_Button.h"
#include "GFx/GFx_Sprite.h"
#include "GFx/GFx_TextField.h"
#include "GFx/GFx_StaticText.h"
#include "GFx/AS2/AS2_Action.h"
#include "GFx/AS2/AS2_MovieRoot.h"
#include "GFx/AS2/AS2_AvmSprite.h"
#include "GFx/AS2/AS2_AvmButton.h"
#include "GFx/AS2/AS2_AvmTextField.h"
#include "GFx/AS2/IME/AS2_IMEManager.h"

#ifdef GFX_ENABLE_VIDEO
#include "GFx/GFx_VideoBase.h"
#endif

namespace Scaleform { namespace GFx {

MovieImpl* AS2Support::CreateMovie(MemoryContext* memContext)
{
    AS2::MemoryContextImpl* memContextImpl = static_cast<AS2::MemoryContextImpl*>(memContext);
    MemoryHeap* pheap = memContextImpl->Heap;
    MovieImpl* pmovie           = SF_HEAP_NEW(pheap) MovieImpl(pheap);

    Ptr<ASMovieRootBase> pasmr = *SF_HEAP_NEW(pheap) AS2::MovieRoot(memContextImpl, pmovie, this);
    pmovie->SetAcceptAnimMovesWith3D(true);     // Set AS2-only default which allows timeline with 3D
    return pmovie;
}

Ptr<ASIMEManager> AS2Support::CreateASIMEManager()
{
#ifndef SF_NO_IME_SUPPORT
	Ptr<ASIMEManager> pasimemgr = *new AS2::IMEManager();
	return pasimemgr;
#else
    return NULL;
#endif
}

MemoryContext* AS2Support::CreateMemoryContext(const char* heapName, 
                                               const MemoryParams& memParams,
                                               bool debugHeap)
{
    unsigned heapFlags = debugHeap ? MemoryHeap::Heap_UserDebug : 0;
    MemoryHeap::HeapDesc desc = memParams.Desc;
    desc.Flags |= heapFlags;
    desc.Flags |= MemoryHeap::Heap_FastTinyBlocks;
    desc.Flags |= MemoryHeap::Heap_ThreadUnsafe;
    // disable Heap_ThreadUnsafe for AMP, 
    // since we get all memory reports from display thread
    SF_AMP_CODE(desc.Flags &= ~MemoryHeap::Heap_ThreadUnsafe;)
    desc.HeapId = HeapId_MovieView;
    // setting the initial dynamic limit (by default - 128K)
    desc.Limit  = (memParams.InitialDynamicLimit == ~0u) ? 
        AS2::MemoryContextImpl::HeapLimit::INITIAL_DYNAMIC_LIMIT : memParams.InitialDynamicLimit; 

    MemoryHeap* heap = Memory::GetGlobalHeap()->CreateHeap(heapName, desc);
    heap->AssignToCurrentThread();
    AS2::MemoryContextImpl* memContext = SF_HEAP_NEW(heap) AS2::MemoryContextImpl();
    memContext->Heap = heap;
#ifdef GFX_AS_ENABLE_GC
    memContext->ASGC = *SF_HEAP_NEW(heap) AS2::ASRefCountCollector();
    memContext->ASGC->SetParams(memParams.FramesBetweenCollections, memParams.MaxCollectionRoots);
#endif
    memContext->StringMgr = *SF_HEAP_NEW(heap) ASStringManager(heap);

    memContext->LimHandler.UserLevelLimit        = memParams.Desc.Limit;
    memContext->LimHandler.HeapLimitMultiplier   = memParams.HeapLimitMultiplier;

    heap->SetLimitHandler(&memContext->LimHandler);
    heap->ReleaseOnFree(memContext);

    return memContext;
}


DisplayObjectBase* AS2Support::CreateCharacterInstance
    (MovieImpl* proot,
    const CharacterCreateInfo& ccinfo,  
    InteractiveObject* pparent, 
    ResourceId rid, 
    CharacterDef::CharacterDefType type)
{
    //MovieImpl* proot = pparent->GetMovieImpl();    
    switch ((type != CharacterDef::Unknown) ? type : ccinfo.pCharDef->GetType())
    {
    case CharacterDef::Shape:
        return SF_HEAP_AUTO_NEW(pparent) AS2::GenericDisplayObj(ccinfo.pCharDef, proot->pASMovieRoot, pparent, rid);

    case CharacterDef::Sprite:
        {
            UByte* pm = (UByte*) SF_HEAP_ALLOC(proot->GetMovieHeap(), 
                ((sizeof(Sprite) + 3) & ~3) + sizeof(AS2::AvmSprite),
                StatMV_ActionScript_Mem);
            Sprite* pspr = new (pm) 
                Sprite(static_cast<SpriteDef*>(ccinfo.pCharDef), ccinfo.pBindDefImpl, proot->pASMovieRoot, pparent, rid);
            AS2::AvmSprite* avmSpr = new (pm + ((sizeof(Sprite) + 3) & ~3)) AS2::AvmSprite(pspr);
            SF_UNUSED(avmSpr);
            return pspr;
        }
    case CharacterDef::Button:
        {
            UByte* pm = (UByte*) SF_HEAP_ALLOC(proot->GetMovieHeap(), 
                ((sizeof(Button) + 3) & ~3) + sizeof(AS2::AvmButton),
                StatMV_ActionScript_Mem);
            Button* pbtn = new (pm)
                Button(static_cast<ButtonDef*>
                    (ccinfo.pCharDef), ccinfo.pBindDefImpl, proot->pASMovieRoot, pparent, rid);
            AS2::AvmButton* avmBtn = new (pm + ((sizeof(Button) + 3) & ~3)) AS2::AvmButton(pbtn);
            SF_UNUSED(avmBtn);
            return pbtn;
        }
    case GFx::CharacterDef::TextField:
        {
            UByte* pm = (UByte*) SF_HEAP_ALLOC(proot->GetMovieHeap(), 
                ((sizeof(TextField) + 3) & ~3) + sizeof(AS2::AvmTextField),
                StatMV_ActionScript_Mem);
            TextField* ptxt = new (pm)
                TextField(static_cast<TextFieldDef*>
                (ccinfo.pCharDef), ccinfo.pBindDefImpl, proot->pASMovieRoot, pparent, rid);
            AS2::AvmTextField* avmTxt = new (pm + ((sizeof(TextField) + 3) & ~3)) AS2::AvmTextField(ptxt);
            SF_UNUSED(avmTxt);
            return ptxt;
        }
    case GFx::CharacterDef::StaticText:
        {
            return SF_HEAP_NEW(proot->GetMovieHeap()) 
                StaticTextCharacter(static_cast<StaticTextDef*>
                (ccinfo.pCharDef), ccinfo.pBindDefImpl, proot->pASMovieRoot, pparent, rid);
        }
#ifdef GFX_ENABLE_VIDEO
    case GFx::CharacterDef::Video:
        {
            Ptr<Video::VideoBase> pvideo = proot->pStateBag->GetVideo();
            if (pvideo && pvideo->GetAS2Support())
            {
                DisplayObjectBase* pdispobj = pvideo->GetAS2Support()->CreateASCharacter(
                    proot, ccinfo.pCharDef, pparent, rid, ccinfo.pBindDefImpl);
                return pdispobj;
            }       
        }
#endif
    default:;
    }
    SF_ASSERT(0); // unknown type!
    return NULL;

}

void  AS2Support::DoActions()
{

}

namespace AS2
{

// AS2 specific PlaceObject2 tag with embedded event handling support
// (onClipEvent, on)
class PlaceObject2EH : public PlaceObject2Tag
{
public:
    ~PlaceObject2EH()
    {
        GFxPlaceObjectBase::EventArrayType* peh = NULL;
        StreamContext    sc(pData);
        UByte   po2Flags = sc.ReadU8();
        if (po2Flags & PO2_HasActions)
        {
            peh = GetEventHandlersPtr(pData);
            if (peh)
            {
                for (UPInt i = 0, n = peh->GetSize(); i < n; i++)
                {
                    delete (*peh)[i];
                }
                delete peh;
            }
        }
    }
    virtual void        Unpack(GFxPlaceObjectBase::UnpackedData& data) { UnpackBase(data, 6); }

    // *** ExecuteTag implementation
    // Place/move/whatever our object in the given pMovie.
    virtual void        Execute(DisplayObjContainer* m) { ExecuteBase(m, 6); }
    virtual void        Trace(const char* str)          { TraceBase(str, 6); }

    virtual void ProcessEventHandlers
        (UnpackedData& data, StreamContext& sc, const UByte* prawdata, UInt8);
};

class PlaceObject2EHa : public PlaceObject2EH
{
public:
    virtual void        Unpack(GFxPlaceObjectBase::UnpackedData& data) { UnpackBase(data, 3); }

    // *** ExecuteTag implementation
    // Place/move/whatever our object in the given pMovie.
    virtual void        Execute(DisplayObjContainer* m) { ExecuteBase(m, 3); }
    virtual void        Trace(const char* str)          { TraceBase(str, 3); }

    virtual void ProcessEventHandlers
        (UnpackedData& data, StreamContext& sc, const UByte* prawdata, UInt8)
    {
        PlaceObject2EH::ProcessEventHandlers(data, sc, prawdata, 3);
    }
};

// AS2 specific PlaceObject3 tag with embedded event handling support
// (onClipEvent, on)
class PlaceObject3EH : public PlaceObject3Tag
{
public:
    ~PlaceObject3EH() 
    {
        GFxPlaceObjectBase::EventArrayType* peh = NULL;
        StreamContext    sc(pData);
        UByte   po2Flags = sc.ReadU8();
        if (po2Flags & PlaceObject2Tag::PO2_HasActions)
        {
            peh = PlaceObject2Tag::GetEventHandlersPtr(pData);
            if (peh)
            {
                for (UPInt i = 0, n = peh->GetSize(); i < n; i++)
                {
                    delete (*peh)[i];
                }
                delete peh;
            }
        }
    }
    virtual void ProcessEventHandlers(UnpackedData& data, StreamContext& sc, const UByte* prawdata);
};

void PlaceObject2EH::ProcessEventHandlers
    (UnpackedData& data, StreamContext& sc, const UByte* prawdata, UInt8 version)
{
    GFxPlaceObjectBase::EventArrayType* peh = NULL;
    peh = GetEventHandlersPtr(prawdata);
    if (!peh)
    {
        UInt16  reserved = sc.ReadU16();
        SF_UNUSED(reserved);
        // The logical 'or' of all the following handlers.
        // I don't think we care about this...
        bool    u32Flags = (version >= 6);
        UInt32  allFlags = 0;
        if (u32Flags)
            allFlags = sc.ReadU32();
        else
            allFlags = sc.ReadU16();
        SF_UNUSED(allFlags);
        peh = SF_HEAP_NEW_ID(Memory::GetGlobalHeap(), StatMD_Tags_Mem) PlaceObjectTag::EventArrayType;
        // Read SwfEvents.
        for (;;)
        {
            // Read event.
            sc.Align();
            UInt32  thisFlags = 0;
            if (u32Flags)
                thisFlags = sc.ReadU32();
            else                    
                thisFlags = sc.ReadU16();
            if (thisFlags == 0)
            {
                // Done with events.
                break;
            }
            SwfEvent*    ev = SF_HEAP_NEW_ID(Memory::GetGlobalHeap(), StatMD_Tags_Mem) SwfEvent;
            ev->Read(&sc, thisFlags);
            peh->PushBack(ev);
        }
        SetEventHandlersPtr(pData, peh);
    }
    data.pEventHandlers = peh;
}

void PlaceObject3EH::ProcessEventHandlers(UnpackedData& data, StreamContext& sc, const UByte*)
{
    GFxPlaceObjectBase::EventArrayType* peh = NULL;
    peh = PlaceObject2Tag::GetEventHandlersPtr(pData);
    if (!peh)
    {
        UInt16  reserved = sc.ReadU16();
        SF_UNUSED(reserved);
        // The logical 'or' of all the following handlers.
        // I don't think we care about this...
        UInt32 allFlags = sc.ReadU32();
        SF_UNUSED(allFlags);
        peh = SF_HEAP_NEW_ID(Memory::GetGlobalHeap(), StatMD_Tags_Mem) PlaceObjectTag::EventArrayType;
        // Read SwfEvents.
        for (;;)
        {
            // Read event.
            sc.Align();
            UInt32  thisFlags = 0;
            thisFlags = sc.ReadU32();
            if (thisFlags == 0)
            {
                // Done with events.
                break;
            }
            SwfEvent*    ev = SF_HEAP_NEW_ID(Memory::GetGlobalHeap(), StatMD_Tags_Mem) SwfEvent;
            ev->Read(&sc, thisFlags);
            peh->PushBack(ev);
        }
        PlaceObject2Tag::SetEventHandlersPtr(pData, peh);
    }
    data.pEventHandlers = peh;
}

// AS2 specific RemoveObject tag with embedded event handling support
// (onClipEvent, on)
class RemoveObjectEH : public RemoveObjectTag
{
    friend class RemoveObject2EH;
protected:
    static TimelineSnapshot::SnapshotElement* CheckEventHandlers
        (TimelineSnapshot::SnapshotElement* pse, 
        GFxPlaceObjectBase::EventArrayType* pevts) 
    { 
        UPInt sz = pevts->GetSize();
        for (UPInt i = 0; i < sz; ++i)
        {
            const SwfEvent* pevt = pevts->At(i);
            if (pevt->Event.Id & EventId::Event_Unload)
            {
                // found!
                pse->Flags |= TimelineSnapshot::Flags_DeadOnArrival;
                pse = NULL; // prevent from deletion
                break;
            }
        }
        return pse;
    }
    virtual void CheckEventHandlers
        (void** pse, 
         GFxPlaceObjectBase::EventArrayType* pevts) 
    { 
        TimelineSnapshot::SnapshotElement** ppse = (TimelineSnapshot::SnapshotElement**)pse;
        (*ppse) = RemoveObjectEH::CheckEventHandlers(*ppse, pevts);
    }
};

// AS2 specific RemoveObject2 tag with embedded event handling support
// (onClipEvent, on)
class RemoveObject2EH : public RemoveObject2Tag
{
protected:
    virtual void CheckEventHandlers
        (void** pse, 
        GFxPlaceObjectBase::EventArrayType* pevts) 
    { 
        TimelineSnapshot::SnapshotElement** ppse = (TimelineSnapshot::SnapshotElement**)pse;
        (*ppse) = RemoveObjectEH::CheckEventHandlers(*ppse, pevts);
    }
};

//
// ButtonAction
//
ButtonAction::~ButtonAction()
{
//     for (UPInt i = 0, n = Actions.GetSize(); i < n; i++)
//     {
//         Actions[i]->Release();
//     }
    Actions.Resize(0);
}

void    ButtonAction::Read(Stream* pin, TagType tagType, unsigned actionLength)
{
    if (actionLength == 0)
        return;

    // Read condition flags.
    if (tagType == Tag_ButtonCharacter)
    {
        Conditions = OVER_DOWN_TO_OVER_UP;
    }
    else
    {
        SF_ASSERT(tagType == Tag_ButtonCharacter2);
        Conditions = pin->ReadU16();
        actionLength -= 2;
    }
    pin->LogParse("-- action conditions %X\n", Conditions);

    // Read actions.
    pin->LogParseAction("-- actions in button\n"); // @@ need more info about which actions
    Ptr<ActionBufferData> a = *ActionBufferData::CreateNew();
    a->Read(pin, actionLength);
    Actions.PushBack(a);
}

//
// AS2::DoActionTag
//

// Thin wrapper around ActionBuffer.
class DoActionTag : public ExecuteTag
{
public:
    Ptr<ActionBufferData> pBuf;

    void    Read(LoadProcess* p)
    {
        Stream* in = p->GetStream();

        pBuf = *ActionBufferData::CreateNew();
#ifdef SF_AMP_SERVER
        int offset = in->Tell();
        unsigned flags = p->GetProcessInfo().Header.SWFFlags;
        if (flags & MovieInfo::SWF_Stripped)
        {
            p->GetNextCodeOffset(&offset);
        }
        if (flags & MovieInfo::SWF_Compressed)
        {
            // Add the size of the 8-byte header, 
            // since that is not included in the compressed stream
            offset += 8;
        }
        pBuf->SetSWFFileOffset(offset);
        pBuf->SetSwdHandle(p->GetLoadTaskData()->GetSwdHandle());
#endif  // SF_AMP_SERVER
        pBuf->Read(in, (unsigned)(p->GetTagEndPosition() - p->Tell()));
    }

    virtual void    Execute(DisplayObjContainer* m)
    {
        AvmSprite* avm = ToAvmSprite(m);
        const Environment *penv = avm->GetASEnvironment();
        if (pBuf && !pBuf->IsNull())
        {
            Ptr<ActionBuffer> pbuff =
                *SF_HEAP_NEW(penv->GetHeap()) ActionBuffer(penv->GetSC(), pBuf);
            avm->AddActionBuffer(pbuff.GetPtr());
        }
    }

    virtual void    ExecuteWithPriority(DisplayObjContainer* m, ActionPriority::Priority prio) 
    { 
        AvmSprite* avm = ToAvmSprite(m);
        if (pBuf && !pBuf->IsNull())
        {
            const Environment *penv = avm->GetASEnvironment();
            Ptr<ActionBuffer> pbuff =
                *SF_HEAP_NEW(penv->GetHeap()) ActionBuffer(penv->GetSC(), pBuf);
            avm->AddActionBuffer(pbuff.GetPtr(), prio);
        }
    }
    // Don't override because actions should not be replayed when seeking the Sprite.
    //void  ExecuteState(Sprite* m) {}

    // Tell the caller that we are an action tag.
    virtual bool    IsActionTag() const
    {
        return true;
    }

    void Trace(const char* str)
    {
        SF_UNUSED(str);
#ifdef SF_BUILD_DEBUG
        printf("  %s actions, file %s\n", str, pBuf->GetFileName());
#endif
    }
};

class DoInitActionTag : public DoActionTag
{
    DoInitActionTag(const DoInitActionTag&) {} // suppress warning
    DoInitActionTag& operator=(const DoInitActionTag&) { return *this; } // suppress warning
public:
    DoInitActionTag() {}

    virtual void    Execute(DisplayObjContainer* m)
    {
        if (pBuf && !pBuf->IsNull())
        {
            AvmSprite* avm = ToAvmSprite(m);
            const Environment *penv  = avm->GetASEnvironment();
            Ptr<ActionBuffer> pbuff = 
                *SF_HEAP_NEW(penv->GetHeap()) ActionBuffer(penv->GetSC(), pBuf);
            avm->AddActionBuffer(pbuff.GetPtr(), ActionPriority::AP_InitClip);
        }
    }

    // Tell the caller that we are not a regular action tag.
    virtual bool    IsActionTag() const
    {
        return false;
    }
};
} // AS2

void AS2Support::ReadButtonActions
(LoadProcess* p, ButtonDef* pbuttonDef, TagType tagType)
{
    Ptr<AS2::ButtonAction> pact = *SF_HEAP_NEW_ID(p->GetLoadHeap(), StatMD_CharDefs_Mem) AS2::ButtonAction;
    pbuttonDef->AddButtonAction(pact);
    pact->Read(p->GetStream(), tagType, (unsigned)(p->GetTagEndPosition() - p->Tell()));

}

void AS2Support::ReadButton2ActionConditions
(LoadProcess* p, ButtonDef* pbuttonDef, TagType tagType)
{
    // Read Button2ActionConditions
    for (;;)
    {
        unsigned NextActionOffset = p->ReadU16();
        unsigned NextActionPos = p->Tell() + NextActionOffset - 2;

        Ptr<AS2::ButtonAction> pact = *SF_HEAP_NEW_ID(p->GetLoadHeap(), StatMD_CharDefs_Mem) AS2::ButtonAction;
        pbuttonDef->AddButtonAction(pact);
        pact->Read(p->GetStream(), tagType, 
            (NextActionOffset) ? NextActionOffset - 2 : (unsigned)(p->GetTagEndPosition() - p->Tell()));

        if (NextActionOffset == 0
            || p->Tell() >= p->GetTagEndPosition())
        {
            // done.
            break;
        }

        // seek to next action.
        p->SetPosition(NextActionPos);
    }
}

void AS2Support::DoActionLoader(LoadProcess* p, const TagInfo& tagInfo)
{
    p->LogParse("tag %d: DoActionLoader\n", tagInfo.TagType);
    p->LogParseAction("-- actions in frame %d\n", p->GetLoadingFrame());

    SF_ASSERT(p);
    SF_ASSERT(tagInfo.TagType == Tag_DoAction);    

    AS2::DoActionTag* da = p->AllocTag<AS2::DoActionTag>();
    da->Read(p);
    p->AddExecuteTag(da);
}

void AS2Support::DoInitActionLoader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_ASSERT(tagInfo.TagType == Tag_DoInitAction);

    int spriteCharacterId = p->ReadU16();

    p->LogParse("  tag %d: DoInitActionLoader\n", tagInfo.TagType);
    p->LogParseAction("  -- init actions for sprite %d\n", spriteCharacterId);

    AS2::DoActionTag* da = p->AllocTag<AS2::DoInitActionTag>();
    da->Read(p);
    p->AddInitAction(ResourceId(spriteCharacterId), da);
}

PlaceObject2Tag* AS2Support::AllocPlaceObject2Tag(LoadProcess* p, UPInt dataSz, UInt8 swfVer)
{
    AS2::PlaceObject2EH* tag;
    if (swfVer >= 6)
        tag = p->AllocTag<AS2::PlaceObject2EH>(dataSz);
    else
        tag = p->AllocTag<AS2::PlaceObject2EHa>(dataSz);
    return tag;
}

PlaceObject3Tag* AS2Support::AllocPlaceObject3Tag(LoadProcess* p, UPInt dataSz)
{
    return p->AllocTag<AS2::PlaceObject3EH>(dataSz);
}

RemoveObjectTag* AS2Support::AllocRemoveObjectTag(LoadProcess* p)
{
    return p->AllocTag<AS2::RemoveObjectEH>();
}

RemoveObject2Tag* AS2Support::AllocRemoveObject2Tag(LoadProcess* p)
{
    return p->AllocTag<AS2::RemoveObject2EH>();
}


}} // namespace Scaleform { namespace GFx {

