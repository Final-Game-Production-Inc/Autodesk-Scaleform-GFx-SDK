/**************************************************************************

Filename    :   AS2_MovieRoot.cpp
Content     :   Implementation of AS2 part of MovieImpl
Created     :   
Authors     :   Artem Bolgar, Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "GFx/GFx_DisplayObjContainer.h"
#include "GFx/AS2/AS2_MovieRoot.h"
#include "GFx/AS2/AS2_ArrayObject.h"
#include "GFx/AS2/AS2_Stage.h"
#include "GFx/AS2/AS2_ExternalInterface.h"
#include "GFx/AS2/AS2_Selection.h"
#include "GFx/AS2/AS2_Mouse.h"
#include "GFx/AS2/AS2_KeyObject.h"
#include "GFx/AS2/AS2_AsFunctionObject.h"
#include "GFx/AS2/AS2_AvmSprite.h"
#include "GFx/AS2/XML/AS2_Xml.h"

namespace Scaleform {
namespace GFx {
namespace AS2 {

// Helper macro to determine the offset of a member. The 16 value is an 
// arbitrary number used as a pointer. The commonly used pointer, NULL,
// produces warnings on some compilers, so 16 is used.
#define OFFSETOF(st, m) ((size_t) ( (char *)&((st *)(16))->m - (char *)16 ))

MovieRoot::MovieRoot(MemoryContextImpl* memContext, MovieImpl* pmovie, ASSupport* pas)
:   ASMovieRootBase(pas),
    pASSupport(pas), 
    MemContext(memContext),
    NumAdvancesSinceCollection(0),
    LastCollectionFrame(0),
    ActionQueue(memContext->Heap),
    BuiltinsMgr(memContext->StringMgr)
{
    SF_ASSERT(memContext->ASGC);
    SF_ASSERT(memContext->StringMgr);

    pMovieImpl         = pmovie;
    pASMouseListener   = 0;
    pInvokeAliases     = NULL;
    pGlobalContext     = NULL;

    pMovieImpl->SetASMovieRoot(this);
    CreateObjectInterface(pMovieImpl);
}

MovieRoot::~MovieRoot()
{
}

void MovieRoot::Shutdown()
{
    SpritesWithHitArea.Clear();

    // release TopNode
    TopNode = NULL;

    // Unregister all classes
    pGlobalContext->UnregisterAllClasses();
    pASMouseListener = NULL;
    
    // Reset invoke aliases set by ExternalInterface.addCallback
    delete pInvokeAliases; pInvokeAliases = NULL;

    // before releasing AVM we need to release all Values.
    ExternalIntfRetVal.SetUndefined();

    pGlobalContext->PreClean(true); // clean with preserving builtin gfxPlayer, gfxArg, gfxLanguage props

    pGlobalContext->DetachMovieRoot();

#ifdef GFX_AS_ENABLE_GC
    // need to release all "collectables" before freeing the RefCount Collector itself
    pGlobalContext->PreClean();
    ActionQueue.Clear();

    MemContext->ASGC->ForceCollect(pMovieImpl->AdvanceStats); // !AB @TODO: make sure this is the right place for it!
#endif // SF_NO_GC
}

void MovieRoot::SetMovie(MovieImpl* pmovie)
{
    ASMovieRootBase::SetMovie(pmovie);
    pGlobalContext     = *SF_HEAP_NEW(pmovie->GetMovieHeap()) GlobalContext(pmovie);
}

bool MovieRoot::Init(MovieDefImpl* pmovieDef)
{
    // Create global variable context
    pGlobalContext      = *SF_HEAP_NEW(GetMovieHeap()) GlobalContext(pMovieImpl);

    // Create a tree container that will contain all levels' nodes
    TopNode = *GetMovieImpl()->GetRenderContext().CreateEntry<Render::TreeContainer>();
    GetMovieImpl()->GetRenderRoot()->Insert(0, TopNode);

    // Note: Log is inherited dynamically from Def, so we don't set it here
    Ptr<Sprite> prootMovie = *CreateSprite(pmovieDef->GetDataDef(), pmovieDef, NULL, ResourceId(), true);
    AvmSprite* pavmRoot     = ToAvmSprite(prootMovie);

    if (!prootMovie)
    {
        return false;
    }

    // Assign level and _level0 name.
    pavmRoot->SetLevel(0);
    SetLevelMovie(0, prootMovie);

    // Register aux AS classes defined in external libraries
    RegisterAuxASClasses();

    // In case permanent variables were assigned.. check them.
    ResolveStickyVariables(prootMovie);

    // Allocate a default PerspectiveDataType object and set fieldOfView.
    prootMovie->SetFOV(DEFAULT_FLASH_FOV);

    return true;
}

void MovieRoot::AdvanceFrame(bool nextFrame)
{
#ifdef GFX_AS_ENABLE_GC
    SF_AMP_SCOPE_TIMER(pMovieImpl->AdvanceStats, "MovieRoot::AdvanceFrame", Amp_Profile_Level_Low);
    if (nextFrame)
        MemContext->ASGC->AdvanceFrame(&NumAdvancesSinceCollection, &LastCollectionFrame, pMovieImpl->AdvanceStats);
#endif // SF_NO_GC
}

// forces garbage collection (if GC is enabled)
void MovieRoot::ForceCollect(unsigned)
{
#ifdef GFX_AS_ENABLE_GC
    MemContext->ASGC->ForceCollect(pMovieImpl->AdvanceStats);
#endif // SF_NO_GC
}

// forces emergency garbage collection (if GC is enabled). This method is called
// when heap is overflown. 
void MovieRoot::ForceEmergencyCollect()
{
#ifdef GFX_AS_ENABLE_GC
    // If garbage collection is used - force collect. Use ForceEmergencyCollect - it
    // guarantees that no new allocations will be made during the collection.
    MemContext->ASGC->ForceEmergencyCollect(pMovieImpl->AdvanceStats);
#endif // SF_NO_GC
}

// Generate button events (onRollOver, onPress, etc)
void MovieRoot::GenerateMouseEvents(unsigned mouseIndex)
{
    Environment* penv = GetAvmLevelMovie(0)->GetASEnvironment();
    UInt8 checkCount = penv->CheckExtensions() ? 16 : 1;

    MouseState& ms = *pMovieImpl->GetMouseState(mouseIndex);
    SF_ASSERT(&ms);
    Ptr<InteractiveObject>    ActiveEntity  = ms.GetActiveEntity();
    Ptr<InteractiveObject>    TopmostEntity = ms.GetTopmostEntity();

    if (ActiveEntity && ActiveEntity->IsUnloaded())
        ActiveEntity = NULL;
    if (TopmostEntity && TopmostEntity->IsUnloaded())
        TopmostEntity = NULL;
    Ptr<InteractiveObject> newActiveEntity = ActiveEntity;

    unsigned changeMask = ms.GetPrevButtonsState() ^ ms.GetButtonsState();
    bool suppressRollOut = false;
    bool miel = ms.IsMouseInsideEntityLast();
    for (UInt8 buttonIdx = 0; buttonIdx < checkCount; ++buttonIdx)   // PPS: 16 is max number of mouse buttons supported
    {
        bool genAuxEvent = buttonIdx != 0;
        unsigned buttonMask = 1 << buttonIdx;

        // Handle button state change
        if ( ((changeMask >> buttonIdx) & 0x1) != 0 )
        {
            // Button was down
            if (ms.GetPrevButtonsState() & buttonMask)
            {
                // Button is now up
                // Handle onRelease, onReleaseOutside
                if ((ms.GetButtonsState() & buttonMask) == 0)
                {
                    if (ActiveEntity)
                    {
                        if (ms.IsMouseInsideEntityLast())
                        {
                            // onRelease
                            ActiveEntity->OnMouseEvent(ButtonEventId(genAuxEvent ? EventId::Event_ReleaseAux : EventId::Event_Release, 
                                mouseIndex, 0, buttonIdx));
                        }
                        else
                        {
                            // onReleaseOutside
                            if (!ActiveEntity->GetTrackAsMenu())
                                ActiveEntity->OnMouseEvent(ButtonEventId(genAuxEvent ? EventId::Event_ReleaseOutsideAux : EventId::Event_ReleaseOutside, 
                                mouseIndex, 0, buttonIdx));                       
                            // Prevent Event_RollOut event below.
                            suppressRollOut = true;
                        }
                    }
                }
            }
            // Button was up
            if ((ms.GetPrevButtonsState() & buttonMask) == 0)
            {
                // Button is now down
                if ((ms.GetButtonsState() & buttonMask))
                {
                    newActiveEntity = TopmostEntity;
                    // onPress
                    if (newActiveEntity)
                        newActiveEntity->OnMouseEvent(ButtonEventId(genAuxEvent ? EventId::Event_PressAux : EventId::Event_Press, mouseIndex, 0, buttonIdx));

                    miel = true;
                }
            }
        }
        else if (ms.GetButtonsState() & buttonMask)
        {
            // Handle buttons-specific continous polling states
            // Handle onDragOut, onDragOver
            if (!ms.IsMouseInsideEntityLast())
            {
                if (TopmostEntity == ActiveEntity)
                {
                    // onDragOver
                    if (ActiveEntity)  
                        ActiveEntity->OnMouseEvent(ButtonEventId
                        (genAuxEvent ? EventId::Event_DragOverAux : EventId::Event_DragOver, 
                        mouseIndex, ActiveEntity->IncrementRollOverCnt(), buttonIdx));

                    miel = true;
                }
            }
            else
            {
                // MouseInsideEntityLast == true
                if (TopmostEntity != ActiveEntity)
                {
                    // onDragOut
                    if (ActiveEntity)
                    {
                        ActiveEntity->OnMouseEvent(ButtonEventId
                            (genAuxEvent ? EventId::Event_DragOutAux : EventId::Event_DragOut, 
                            mouseIndex, ActiveEntity->DecrementRollOverCnt(), buttonIdx));
                    }
                    miel = false;
                }
            }
            // Handle trackAsMenu dragOver
            if (!ActiveEntity
                || ActiveEntity->GetTrackAsMenu())
            {
                if (TopmostEntity
                    && TopmostEntity != ActiveEntity
                    && TopmostEntity->GetTrackAsMenu())
                {
                    // Transfer to topmost entity, dragOver
                    newActiveEntity = TopmostEntity;
                    newActiveEntity->OnMouseEvent(ButtonEventId(genAuxEvent ? EventId::Event_DragOverAux : EventId::Event_DragOver, 
                        mouseIndex, 
                        newActiveEntity->IncrementRollOverCnt(), buttonIdx));
                    miel = true;
                }
            }
        }
    }
    //@DBG
    //printf("btnst = %d, topmost %p, active %p, supr %d\n", ms.GetButtonsState(), 
    //	TopmostEntity.GetPtr(), ActiveEntity.GetPtr(), int(suppressRollOut));

    // Handle non-button specific continuous polling states
    // New active entity is whatever is below the mouse right now.
    //printf("%x\n",ms.GetPrevButtonsState());
    if ((ms.GetPrevButtonsState() & 0x1) == 0 && TopmostEntity != ActiveEntity)
    {
        // onRollOut
        if (!suppressRollOut && ActiveEntity)
        {
            ActiveEntity->OnMouseEvent(ButtonEventId
                (EventId::Event_RollOut, mouseIndex, ActiveEntity->DecrementRollOverCnt()));              
            //printf("   out!\n"); //@DBG
        }

        newActiveEntity = TopmostEntity;

        // onRollOver
        if (newActiveEntity)
		{
            //printf("   over\n"); //@DBG
            newActiveEntity->OnMouseEvent(ButtonEventId
                (EventId::Event_RollOver, mouseIndex, newActiveEntity->IncrementRollOverCnt()));
		}

        miel = true;
    }

    ms.SetMouseInsideEntityLast(miel);

    // Write The (possibly modified) GPtr copies back
    // into the state struct.
    ms.SetActiveEntity(newActiveEntity);
}

void MovieRoot::SetMemoryParams(unsigned frameBetweenCollections, unsigned maxRootCount)
{
#ifdef GFX_AS_ENABLE_GC
    MemContext->ASGC->SetParams(frameBetweenCollections, maxRootCount);
#else
    SF_UNUSED2(frameBetweenCollections, maxRootCount);
#endif // SF_NO_GC
}

Sprite* MovieRoot::CreateSprite(TimelineDef* pdef, 
                                MovieDefImpl* pdefImpl,
                                InteractiveObject* parent,
                                ResourceId id, 
                                bool loadedSeparately)
{
    CharacterCreateInfo ccinfo(pdef, pdefImpl);
    DisplayObjectBase* pdo = GetASSupport()->CreateCharacterInstance
        (GetMovieImpl(), ccinfo, parent, id, CharacterDef::Sprite);
    SF_ASSERT(pdo->IsSprite());
    Sprite* pspr = pdo->CharToSprite_Unsafe();
    pspr->SetLoadedSeparately(loadedSeparately);
    return pspr;
}

void    MovieRoot::DoActions()
{
    SF_AMP_SCOPE_TIMER_ID(pMovieImpl->AdvanceStats, "MovieRoot::DoActions", Amp_Native_Function_Id_DoActions);

    // Execute the actions in the action list, in the given environment.
    // NOTE: Actions can add other actions to the end of the list while running.
    MovieRoot::ActionQueueIterator iter(&ActionQueue);
    const ActionEntry* paction;
    while((paction = iter.getNext()) != NULL)
    {
        paction->Execute(this);
    }
}

void    MovieRoot::DoActionsForSession(unsigned sessionId)
{
    // Execute the actions in the action list, in the given environment.
    // NOTE: Actions can add other actions to the end of the list while running.
    MovieRoot::ActionQueueSessionIterator iter(&ActionQueue, sessionId);
    const ActionEntry* paction;
    while((paction = iter.getNext()) != NULL)
    {
        paction->Execute(this);
    }
}

void MovieRoot::RegisterAuxASClasses()
{
    ASStringContext sc(pGlobalContext, 8);
    
/*    
#ifdef GFX_ENABLE_XML
    // Register the XML and XMLNode classes
    Ptr<XMLSupportBase> xmlstate = pMovieImpl->pStateBag->GetXMLSupport();
    if (xmlstate)
    {
        xmlstate->RegisterASClasses(*pGlobalContext, sc);
    }
#endif
*/
#ifdef GFX_ENABLE_XML
    pGlobalContext->AddBuiltinClassRegistry<ASBuiltin_XML, XmlCtorFunction>(sc, pGlobalContext->pGlobal);
    pGlobalContext->AddBuiltinClassRegistry<ASBuiltin_XMLNode, XmlNodeCtorFunction>(sc, pGlobalContext->pGlobal);
#endif

#ifdef GFX_ENABLE_SOUND
    Ptr<AudioBase> paudioState = pMovieImpl->pStateBag->GetAudio();
    if (paudioState)
        paudioState->RegisterASClasses(*pGlobalContext, sc);
#endif
#ifdef GFX_ENABLE_VIDEO
    Ptr<Video::VideoBase> pvideoState = pMovieImpl->pStateBag->GetVideo();
    if (pvideoState && pvideoState->GetAS2Support())
        pvideoState->GetAS2Support()->RegisterASClasses(*pGlobalContext, sc);
#endif
}


Sprite*  MovieRoot::GetLevelMovie(int level) const
{
    SF_ASSERT(level >= 0);

    // Exhaustive for now; could do binary.
    for (unsigned i = 0; i < pMovieImpl->MovieLevels.GetSize(); i++)
    {
        if (pMovieImpl->MovieLevels[i].Level == level)
            return static_cast<Sprite*>(pMovieImpl->MovieLevels[i].pSprite.GetPtr());
    }   
    return 0;
}

// Sets a movie at level; used for initialization.
bool        MovieRoot::SetLevelMovie(int level, Sprite *psprite)
{
    pMovieImpl->SetLevelMovie(level, psprite);
    return 1;
}

// Helper: parses _levelN tag and returns the end of it.
// Returns level index, of -1 if there is no match.
int MovieRoot::ParseLevelName(const char* pname, const char **ptail, bool caseSensitive)
{
    if ((pname[0] >= '0') && (pname[0] <= '9'))
    {
        char *ptail2 = 0;
        int level = (int)strtol(pname, &ptail2, 10);
        *ptail = ptail2;
        return level;
    }

    if (!pname || pname[0] != '_')
        return -1;

    if (caseSensitive)
    {       
        if ( (pname[1] != 'l') ||
            (pname[2] != 'e') ||
            (pname[3] != 'v') ||
            (pname[4] != 'e') ||
            (pname[5] != 'l') )
            return -1;
    }
    else
    {
        if ( ((pname[1] != 'l') && (pname[1] != 'L')) ||
            ((pname[2] != 'e') && (pname[2] != 'E')) ||
            ((pname[3] != 'v') && (pname[3] != 'V')) ||
            ((pname[4] != 'e') && (pname[4] != 'E')) ||
            ((pname[5] != 'l') && (pname[5] != 'L')) )
            return -1;
    }
    // Number must follow the level.
    if ((pname[6] < '0') || (pname[6] > '9'))
        return -1;

    char *ptail2 = 0;
    int   level = (int)strtol(pname + 6, &ptail2, 10);
    *ptail = ptail2;

    return level;
}


// *** Action List management
MovieRoot::ActionQueueType::ActionQueueType(MemoryHeap *pheap)
: pHeap(pheap)
{
    LastSessionId = 0;
    ModId = 1;
    pFreeEntry = NULL;
    CurrentSessionId = ++LastSessionId;
    FreeEntriesCount = 0;
}

MovieRoot::ActionQueueType::~ActionQueueType()
{
    Clear();

    // free pFreeEntry chain
    for(ActionEntry* pcur = pFreeEntry; pcur; )
    {
        ActionEntry* pnext = pcur->pNextEntry;
        delete pcur;
        pcur = pnext;
    }
}

MovieRoot::ActionEntry*  MovieRoot::ActionQueueType::InsertEntry(ActionPriority::Priority prio)
{
    ActionEntry *p = 0;
    if (pFreeEntry)
    {
        p = pFreeEntry;
        pFreeEntry = pFreeEntry->pNextEntry;
        p->pNextEntry = 0;
        --FreeEntriesCount;
    }
    else
    {
        if ((p = SF_HEAP_NEW(GetMovieHeap()) ActionEntry)==0)
            return 0;
    }

    ActionQueueEntry& actionQueueEntry = Entries[prio];

    // Insert and move insert pointer to us.
    if (actionQueueEntry.pInsertEntry == 0) // insert at very beginning
    {
        p->pNextEntry                   = actionQueueEntry.pActionRoot;
        actionQueueEntry.pActionRoot    = p;
    }
    else
    {
        p->pNextEntry = actionQueueEntry.pInsertEntry->pNextEntry;
        actionQueueEntry.pInsertEntry->pNextEntry = p;
    }
    actionQueueEntry.pInsertEntry   = p;
    if (p->pNextEntry == NULL)
        actionQueueEntry.pLastEntry = p;
    p->SessionId = CurrentSessionId;
    ++ModId;

    // Done
    return p;
}

MovieRoot::ActionEntry* MovieRoot::ActionQueueType::FindEntry
(ActionPriority::Priority prio, const ActionEntry& entry)
{
    ActionQueueEntry& queue = Entries[prio];
    ActionEntry* pentry = queue.pActionRoot;
    for (; pentry != NULL; pentry = pentry->pNextEntry)
    {
        if (*pentry == entry)
            return pentry;
    }
    return NULL;
}

void MovieRoot::ActionQueueType::AddToFreeList(MovieRoot::ActionEntry* pentry)
{
    pentry->ClearAction();
    if (FreeEntriesCount < 50)
    {
        pentry->pNextEntry = pFreeEntry;
        pFreeEntry = pentry;
        ++FreeEntriesCount;
    }
    else
        delete pentry;
}

void    MovieRoot::ActionQueueType::Clear()
{   
    MovieRoot::ActionQueueIterator iter(this);
    while(iter.getNext())
        ;
}

unsigned MovieRoot::ActionQueueType::StartNewSession(unsigned* pprevSessionId) 
{ 
    if (pprevSessionId) *pprevSessionId = CurrentSessionId;
    CurrentSessionId = ++LastSessionId; 
    return CurrentSessionId; 
}

MovieRoot::ActionQueueIterator::ActionQueueIterator(MovieRoot::ActionQueueType* pactionQueue)
{
    SF_ASSERT(pactionQueue);
    pActionQueue    = pactionQueue;
    ModId           = 0;
    CurrentPrio     = 0;
    pLastEntry      = 0;
}

MovieRoot::ActionQueueIterator::~ActionQueueIterator()
{
    if (pLastEntry)
        pActionQueue->AddToFreeList(pLastEntry); // release entry
}

const MovieRoot::ActionEntry* MovieRoot::ActionQueueIterator::getNext()
{
    if (pActionQueue->ModId != ModId)
    {
        // new actions were added - restart
        CurrentPrio = 0;
        ModId = pActionQueue->ModId;
    }
    ActionEntry* pcurEntry = pActionQueue->Entries[CurrentPrio].pActionRoot;
    if (!pcurEntry)
    {
        while(pcurEntry == NULL && ++CurrentPrio < AP_Count)
        {
            pcurEntry = pActionQueue->Entries[CurrentPrio].pActionRoot;
        }
    }
    if (pcurEntry)
    {
        if (pcurEntry == pActionQueue->Entries[CurrentPrio].pInsertEntry)
            pActionQueue->Entries[CurrentPrio].pInsertEntry = pcurEntry->pNextEntry;
        pActionQueue->Entries[CurrentPrio].pActionRoot = pcurEntry->pNextEntry;
        pcurEntry->pNextEntry = NULL;
    }
    if (pActionQueue->Entries[CurrentPrio].pActionRoot == NULL)
    {
        pActionQueue->Entries[CurrentPrio].pInsertEntry = NULL;
        pActionQueue->Entries[CurrentPrio].pLastEntry = NULL;
    }
    if (pLastEntry)
        pActionQueue->AddToFreeList(pLastEntry); // release entry
    pLastEntry = pcurEntry;
    return pcurEntry;
}

MovieRoot::ActionQueueSessionIterator::ActionQueueSessionIterator(struct MovieRoot::ActionQueueType* pactionQueue, unsigned sessionId) :
ActionQueueIterator(pactionQueue), SessionId(sessionId)
{
}

const MovieRoot::ActionEntry* MovieRoot::ActionQueueSessionIterator::getNext()
{
    if (pActionQueue->ModId != ModId)
    {
        // new actions were added - restart
        CurrentPrio = 0;
        ModId = pActionQueue->ModId;
    }
    ActionEntry* pcurEntry = NULL;
    for(;CurrentPrio < AP_Count; ++CurrentPrio)
    {
        pcurEntry = pActionQueue->Entries[CurrentPrio].pActionRoot;
        ActionEntry* pprevEntry = NULL;

        // now search for the node with appropriate session id. Once it is found - 
        // save it and remove from the list
        while(pcurEntry && pcurEntry->SessionId != SessionId)
        {
            pprevEntry = pcurEntry;
            pcurEntry = pcurEntry->pNextEntry;
        }

        if (pcurEntry)
        {
            if (!pprevEntry)
                pActionQueue->Entries[CurrentPrio].pActionRoot = pcurEntry->pNextEntry;
            else
                pprevEntry->pNextEntry = pcurEntry->pNextEntry;

            if (!pcurEntry->pNextEntry)
                pActionQueue->Entries[CurrentPrio].pLastEntry = pprevEntry;

            if (pcurEntry == pActionQueue->Entries[CurrentPrio].pInsertEntry)
            {
                if (!pcurEntry->pNextEntry)
                    pActionQueue->Entries[CurrentPrio].pInsertEntry = pprevEntry;
                else
                    pActionQueue->Entries[CurrentPrio].pInsertEntry = pcurEntry->pNextEntry;
            }

            pcurEntry->pNextEntry = NULL;
            break;
        }
    }
    if (pLastEntry)
        pActionQueue->AddToFreeList(pLastEntry); // release entry
    pLastEntry = pcurEntry;
    return pcurEntry;
}


// Executes actions in this entry
void    MovieRoot::ActionEntry::Execute(MovieRoot *proot) const
{   
    SF_UNUSED(proot);
    // pCharacter might be null, if only InsertEmptyAction was used, without
    // subsequent call to SetAction.
    if (!pCharacter || pCharacter->IsUnloaded())
        return;

    switch(Type)
    {
    case Entry_Buffer:      
        // Sprite is already AddRefd' so it's guaranteed to stick around.
        ToAvmCharacter(pCharacter)->ExecuteBuffer(pActionBuffer);
        break;

    case Entry_Event:       
        ToAvmCharacter(pCharacter)->ExecuteEvent(mEventId);      
        break;

    case Entry_Function:        
        ToAvmCharacter(pCharacter)->ExecuteFunction(Function, FunctionParams);      
        break;

    case Entry_CFunction:        
        ToAvmCharacter(pCharacter)->ExecuteCFunction(CFunction, FunctionParams);      
        break;

    case Entry_None:
        break;
    }
}

MovieDefRootNode* MovieRoot::CreateMovieDefRootNode(MemoryHeap* pheap, MovieDefImpl *pdefImpl, bool importFlag)
{
    return SF_HEAP_NEW(pheap) MovieDefRootNode(pdefImpl, importFlag);
}

void MovieRoot::ClearDisplayList()
{
    // It is important to destroy the sprite before the global context,
    // so that is is not used from OnEvent(unload) in sprite destructor
    // NOTE: We clear the list here first because users can store pointers in _global,
    // which would cause pMovie assignment to not release it early (avoid "aeon.swf" crash).
    UPInt i;

    for (i = pMovieImpl->MovieLevels.GetSize(); i > 0; i--)
        static_cast<Sprite*>(pMovieImpl->MovieLevels[i-1].pSprite.GetPtr())->ClearDisplayList();
    for (i = pMovieImpl->MovieLevels.GetSize(); i > 0; i--)
        static_cast<Sprite*>(pMovieImpl->MovieLevels[i-1].pSprite.GetPtr())->ForceShutdown();
    // Release all refs.
    pMovieImpl->MovieLevels.Clear();
}

// *** Load/Unload movie support
void    MovieRoot::AddLoadQueueEntryMT(LoadQueueEntry* pentry)
{
    LoadQueueEntryMT* pentryMT = NULL;
    if (pentry->Type & GFxAS2LoadQueueEntry::LTF_VarsFlag)
    {
#ifdef GFX_AS2_ENABLE_LOADVARS
        pentryMT = SF_HEAP_NEW(GetMovieHeap()) LoadQueueEntryMT_LoadVars(pentry, pMovieImpl);
#endif
    }
    else if (pentry->Type & GFxAS2LoadQueueEntry::LTF_XMLFlag)
    {
#ifdef GFX_ENABLE_XML
        if (pentry->URL.GetLength())
        {
            pentryMT = SF_HEAP_NEW(GetMovieHeap()) GFxAS2LoadQueueEntryMT_LoadXML(pentry, this);
            GFxAS2LoadQueueEntry* as2entry = static_cast<GFxAS2LoadQueueEntry*>(pentryMT->GetQueueEntry());
            if (pentryMT && as2entry->XMLHolder.ASObj.ToObject(NULL))
            {
                Object* pxmlobj = as2entry->XMLHolder.ASObj.ToObject(NULL);
                LoadQueueEntryMT *ptail = pMovieImpl->pLoadQueueMTHead;
                while (ptail)    
                {
                    GFxAS2LoadQueueEntry* as2tail = static_cast<GFxAS2LoadQueueEntry*>(ptail->GetQueueEntry());
                    as2tail->CancelByXMLASObjPtr(pxmlobj);
                    ptail = ptail->GetNext();
                }
            }
        }
#endif
    } 
    else if (pentry->Type & GFxAS2LoadQueueEntry::LTF_CSSFlag)
    {
#ifdef GFX_ENABLE_CSS
        if (pentry->URL.GetLength())
        {
            pentryMT = SF_HEAP_NEW(GetMovieHeap()) GFxAS2LoadQueueEntryMT_LoadCSS(pentry,this);
            GFxAS2LoadQueueEntry* as2entry = static_cast<GFxAS2LoadQueueEntry*>(pentryMT->GetQueueEntry());
            if (pentryMT && as2entry->CSSHolder.ASObj.ToObject(NULL))
            {
                Object* pxmlobj = as2entry->CSSHolder.ASObj.ToObject(NULL);
                LoadQueueEntryMT *ptail = pMovieImpl->pLoadQueueMTHead;
                while (ptail)    
                {
                    GFxAS2LoadQueueEntry* as2tail = static_cast<GFxAS2LoadQueueEntry*>(ptail->GetQueueEntry());
                    as2tail->CancelByCSSASObjPtr(pxmlobj);
                    ptail = ptail->GetNext();
                }
            }
        }
#endif
    }
    else 
    {
        pentryMT = SF_HEAP_NEW(GetMovieHeap()) GFxAS2LoadQueueEntryMT_LoadMovie(pentry, pMovieImpl);
        if (pentryMT)
        {
            GFxAS2LoadQueueEntry* as2entry = static_cast<GFxAS2LoadQueueEntry*>(pentryMT->GetQueueEntry());
            LoadQueueEntryMT *ptail = pMovieImpl->pLoadQueueMTHead;
            while (ptail)    
            {
                GFxAS2LoadQueueEntry* as2tail = static_cast<GFxAS2LoadQueueEntry*>(ptail->GetQueueEntry());
                if (as2entry->pCharacter)
                    as2tail->CancelByNamePath(as2entry->pCharacter->GetNamePath());
                else 
                    as2tail->CancelByLevel(as2entry->Level);
                ptail = ptail->GetNext();
            }
        }
    }
    if (!pentryMT)
    {
        delete pentry;
        return;
    }
    pMovieImpl->AddLoadQueueEntryMT(pentryMT);
}


// ****************************************************************************
// Adds load queue entry based on parsed url and target path 
// (Load/unload Movie)
//
void    MovieRoot::AddLoadQueueEntry(const char* ptarget, const char* purl, Environment* env,
                                        LoadQueueEntry::LoadMethod method,
                                        MovieClipLoader* pmovieClipLoader)
{
    GFxAS2LoadQueueEntry* pentry        = 0;
    int                   level         = -1;
    InteractiveObject*    ptargetChar   = env ? env->FindTarget(GetStringManager()->CreateString(ptarget)) : 
                    static_cast<InteractiveObject*>(FindTarget(GetStringManager()->CreateString(ptarget)));
    Sprite*          ptargetSprite = ptargetChar ? ptargetChar->CharToSprite() : 0;  

    // If target leads to level, use level loading option.
    if (ptargetSprite)  
    {
        level = ToAvmSprite(ptargetSprite)->GetLevel();
        if (level != -1)
        {
            ptargetSprite = 0;
            ptargetChar   = 0;
        }       
    }

    // Otherwise, we have a real target.
    if (ptargetChar)
    {
        pentry = SF_HEAP_NEW(GetMovieHeap()) GFxAS2LoadQueueEntry(ptargetChar->GetCharacterHandle(), purl, method);
    }
    else
    {
        // It must be a level or bad target. 
        if (level == -1)
        {
            // Decode level name.
            const char* ptail = "";
            level = ParseLevelName(ptarget, &ptail, GetLevelMovie(0)->IsCaseSensitive());
            if (*ptail != 0)
                level = -1; // path must end at _levelN.
        }

        if (level != -1)
        {
            pentry = SF_HEAP_NEW(GetMovieHeap()) GFxAS2LoadQueueEntry(level, purl, method);
        }
    }

    if (pentry) 
    {
        pentry->MovieClipLoaderHolder.SetAsObject(pmovieClipLoader);
        AddMovieLoadQueueEntry(pentry);
    }
}

// ****************************************************************************
// Adds load queue entry based on parsed url and target path 
// (Load Variables)
//
void MovieRoot::AddVarLoadQueueEntry(const char* ptarget, const char* purl,
                                     GFxAS2LoadQueueEntry::LoadMethod method)

{
    GFxAS2LoadQueueEntry* pentry        = 0;
    int                   level         = -1;
    InteractiveObject*    ptargetChar   = 
        static_cast<InteractiveObject*>(FindTarget(GetStringManager()->CreateString(ptarget)));
    Sprite*          ptargetSprite = ptargetChar ? ptargetChar->CharToSprite() : 0;  

    // If target leads to level, use level loading option.
    if (ptargetSprite)  
    {
        level = ToAvmSprite(ptargetSprite)->GetLevel();
        if (level != -1)
        {
            ptargetSprite = 0;
            ptargetChar   = 0;
        }       
    }

    // Otherwise, we have a real target.
    if (ptargetChar)
    {
        pentry = SF_HEAP_NEW(GetMovieHeap()) GFxAS2LoadQueueEntry(ptargetChar->GetCharacterHandle(), purl, method, true);
    }
    else
    {
        // It must be a level or bad target. 
        if (level == -1)
        {
            // Decode level name.
            const char* ptail = "";
            level = ParseLevelName(ptarget, &ptail, GetLevelMovie(0)->IsCaseSensitive());
            if (*ptail != 0)
                level = -1; // path must end at _levelN.
        }

        if (level != -1)
        {
            pentry = SF_HEAP_NEW(GetMovieHeap()) GFxAS2LoadQueueEntry(level, purl, method, true);
        }
    }

    if (pentry) 
    {
        //AddLoadQueueEntry(pentry);
        if (pMovieImpl->GetTaskManager())
            AddLoadQueueEntryMT(pentry);
        else
            AddLoadQueueEntry(pentry);
    }
}

// ****************************************************************************
// Adds load queue entry based on parsed url and target character 
// (Load/unload Movie)
//
void MovieRoot::AddLoadQueueEntry(InteractiveObject* ptargetChar, const char* purl,
                                  GFxAS2LoadQueueEntry::LoadMethod method,
                                  MovieClipLoader* pmovieClipLoader)
{   
    if (!ptargetChar)
        return;

    GFxAS2LoadQueueEntry* pentry  = 0;
    int                   level   = -1;
    Sprite*               ptargetSprite = ptargetChar->CharToSprite();

    // If target leads to level, use level loading option.
    if (ptargetSprite)
    {
        level = ToAvmSprite(ptargetSprite)->GetLevel();
        if (level != -1)
        {
            ptargetSprite = 0;
            ptargetChar   = 0;
        }   
    }

    // Otherwise, we have a real target.
    if (ptargetChar)
    {
        pentry = SF_HEAP_NEW(GetMovieHeap()) GFxAS2LoadQueueEntry(ptargetChar->GetCharacterHandle(), purl, method);
    }
    else if (level != -1)
    {
        pentry = SF_HEAP_NEW(GetMovieHeap()) GFxAS2LoadQueueEntry(level, purl, method);
    }

    if (pentry)
    {
        pentry->MovieClipLoaderHolder.SetAsObject(pmovieClipLoader);
        AddMovieLoadQueueEntry(pentry);
    }
}

void MovieRoot::AddMovieLoadQueueEntry(LoadQueueEntry* pentry)
{
    if(!pentry)
        return;

    bool sync;
    // Check to see if URL is a user image substitute.
    bool userImageProtocol = LoaderImpl::IsProtocolImage(pentry->URL, NULL, &sync);

    if (userImageProtocol && sync)
    {
        // Custom image protocol can be loaded synchronously
        Ptr<LoadStates>  pls = *new LoadStates(GetMovieImpl()->GetLoaderImpl(), 
            GetMovieImpl()->GetStateBagImpl());
        ProcessLoadQueueEntry(pentry, pls);
        delete pentry;
        return;
    }

    if ( pentry->URL.GetLength() > 0 && !userImageProtocol && pMovieImpl->GetTaskManager())
        AddLoadQueueEntryMT(pentry);
    else
        AddLoadQueueEntry(pentry);

}
// ****************************************************************************
// Adds load queue entry based on parsed url and target character 
// (Load Variables)
//
void    MovieRoot::AddVarLoadQueueEntry(InteractiveObject* ptargetChar, const char* purl,
                                              GFxAS2LoadQueueEntry::LoadMethod method)
{
    if (!ptargetChar)
        return;

    GFxAS2LoadQueueEntry* pentry  = 0;
    int                   level   = -1;
    Sprite*               ptargetSprite = ptargetChar->CharToSprite();

    // If target leads to level, use level loading option.
    if (ptargetSprite)
    {
        level = ToAvmSprite(ptargetSprite)->GetLevel();
        if (level != -1)
        {
            ptargetSprite = 0;
            ptargetChar   = 0;
        }   
    }

    // Otherwise, we have a real target.
    if (ptargetChar)
    {
        pentry = SF_HEAP_NEW(GetMovieHeap()) GFxAS2LoadQueueEntry(ptargetChar->GetCharacterHandle(), purl, method, true);
    }
    else if (level != -1)
    {
        pentry = SF_HEAP_NEW(GetMovieHeap()) GFxAS2LoadQueueEntry(level, purl, method, true);
    }

    if (pentry)
    {
        if (pMovieImpl->GetTaskManager())
            AddLoadQueueEntryMT(pentry);
        else
            AddLoadQueueEntry(pentry);
    }
}

// ****************************************************************************
// Adds load queue entry based on parsed url and LoadVars object
// (Load Variables)
//
#ifdef GFX_AS2_ENABLE_LOADVARS
void MovieRoot::AddVarLoadQueueEntry(LoadVarsObject* ploadVars, const char* purl,
                                           GFxAS2LoadQueueEntry::LoadMethod method)
{
    GFxAS2LoadQueueEntry* pentry = SF_HEAP_NEW(GetMovieHeap()) GFxAS2LoadQueueEntry(purl, method, true);
    if (pentry)
    {
        pentry->LoadVarsHolder.SetAsObject(ploadVars);
        if (pMovieImpl->GetTaskManager())
            AddLoadQueueEntryMT(pentry);
        else
            AddLoadQueueEntry(pentry);
    }
}
#endif

// ****************************************************************************
// Adds load queue entry based on parsed url and CSS object
// (Load CSS)
//
#ifdef GFX_ENABLE_CSS
void MovieRoot::AddCssLoadQueueEntry(Object* pobj, ASCSSFileLoader* pLoader, 
                                        const char* purl,GFxAS2LoadQueueEntry::LoadMethod method)
{
    GFxAS2LoadQueueEntry* pentry = SF_HEAP_NEW(GetMovieHeap()) GFxAS2LoadQueueEntry(purl, method);
    if (pentry)
    {
        pentry->Type = GFxAS2LoadQueueEntry::LT_LoadCSS;
        pentry->CSSHolder.ASObj.SetAsObject(pobj);
        pentry->CSSHolder.Loader = pLoader;
        if (pMovieImpl->GetTaskManager())
            AddLoadQueueEntryMT(pentry);
        else
            AddLoadQueueEntry(pentry);
    }
}
#endif


// ****************************************************************************
// Adds load queue entry based on parsed url and XML object
// (Load XML)
//
#ifdef GFX_ENABLE_XML
void MovieRoot::AddXmlLoadQueueEntry(Object* pxmlobj, XMLFileLoader* pxmlLoader, 
                                        const char* purl,GFxAS2LoadQueueEntry::LoadMethod method)
{
    GFxAS2LoadQueueEntry* pentry = SF_HEAP_NEW(GetMovieHeap()) GFxAS2LoadQueueEntry(purl, method);
    if (pentry)
    {
        pentry->Type = GFxAS2LoadQueueEntry::LT_LoadXML;
        pentry->XMLHolder.ASObj.SetAsObject(pxmlobj);
        pentry->XMLHolder.Loader = pxmlLoader;
        if (pMovieImpl->GetTaskManager())
            AddLoadQueueEntryMT(pentry);
        else
            AddLoadQueueEntry(pentry);
    }
}
#endif

// Process load queue entry (single threaded)
void    MovieRoot::ProcessLoadQueueEntry(LoadQueueEntry *pentry, LoadStates* pls)
{
#ifdef GFX_AS2_ENABLE_LOADVARS
    if ((pentry->Type & LoadQueueEntry::LTF_VarsFlag) != 0)
    {
        ProcessLoadVars(pentry, pls);
    }
#ifdef GFX_ENABLE_XML
    else if ((pentry->Type & LoadQueueEntry::LTF_XMLFlag) != 0)
        ProcessLoadXML(pentry, pls);
#endif
#ifdef GFX_ENABLE_CSS
    else if ((pentry->Type & LoadQueueEntry::LTF_CSSFlag) != 0)
        ProcessLoadCSS(pentry, pls);
#endif
    else
        ProcessLoadMovieClip(pentry, pls);
#else
    SF_UNUSED2(pentry, pls);
#endif
}

void    MovieRoot::ProcessLoadMovieClip(LoadQueueEntry *p_entry, LoadStates* pls)
{
    SF_ASSERT(p_entry);
    SF_ASSERT((p_entry->Type & GFxAS2LoadQueueEntry::LTF_VarsFlag) == 0);

    GFxAS2LoadQueueEntry *pentry = static_cast<GFxAS2LoadQueueEntry*>(p_entry);
    // Obtain level0 path before it has a chance to be unloaded below.
    String level0Path;
    GetLevel0Path(&level0Path);

    // *** Handle loading/unloading.

    String              url(pentry->URL), urlStrGfx;
    Ptr<InteractiveObject> poldChar;      
    InteractiveObject*  pparent   = 0;
    ResourceId          newCharId(CharacterDef::CharId_EmptyMovieClip);

    LogState*           plog = pls->GetLogState();
    // We take out load flags from GetMovieDef() so that its options
    // can be inherited into GFxMovieDef.
    unsigned            loadFlags = GetMovieDefImpl()->GetLoadFlags();

    Ptr<MovieDefImpl>   pmovieDef;
    Ptr<Sprite>         pnewChar;

    bool stripped = false;

    if (pentry->pCharacter)
    {
        // This is loadMovie, not level.
        SF_ASSERT(pentry->Level == -1);
        SF_ASSERT(!(pentry->Type & GFxAS2LoadQueueEntry::LTF_LevelFlag));

        // Make sure we have character targets and parents.
        //!AB: We need to get the character by name first time. It is kind of weird but
        // it seems that is how Flash works. For example, we have a movieclip 'mc'
        // and it already contains child 'holder'. Then we create an empty movie clip
        // within 'mc' and call it 'holder' as well. If we resolve string 'mc.holder' to
        // a character when the first (native) 'holder' will be returned. However, if we 
        // use returning value from createEmptyMovieClip we will have a pointer to the new
        // 'holder'. Now, we call loadMovie using the pointer returned from createEmptyMovieClip.
        // We expect, that movie will be loaded into newly created 'holder'. However, it loads
        // it into the native 'holder'. So, looks like the implementation of 'loadMovie' always
        // resolve character by name. Test file: Test\LoadMovie\test_loadmovie_with_two_same_named_clips.swf 
        poldChar = static_cast<InteractiveObject*>(pentry->pCharacter->ForceResolveCharacter(pMovieImpl));
        if (!poldChar)
            return;
        if ((pparent = poldChar->GetParent())==0)
            return;
        stripped = ((poldChar->GetResourceMovieDef()->GetSWFFlags()
            & MovieInfo::SWF_Stripped) != 0);
        //AB: if loading in the character then we need to assign the
        // same Id as the old character had; this is important to avoid
        // re-creation of the original character when timeline rollover 
        // of the parent character occurs.
        newCharId = poldChar->GetId();
    }
    else if (pentry->Level != -1)
    {
        SF_ASSERT(pentry->Type & GFxAS2LoadQueueEntry::LTF_LevelFlag);
        if (GetLevelMovie(pentry->Level))
            stripped = ((GetLevelMovie(pentry->Level)->GetResourceMovieDef()->GetSWFFlags()
            & MovieInfo::SWF_Stripped) != 0);
        else if (GetLevelMovie(0))
            stripped = ((GetLevelMovie(0)->GetResourceMovieDef()->GetSWFFlags()
            & MovieInfo::SWF_Stripped) != 0);

        // Wipe out level, making it ready for load.
        // Note that this could wipe out all levels!
        // Do this only if loading movie, not variables
        pMovieImpl->ReleaseLevelMovie(pentry->Level);
        newCharId = ResourceId(); // Assign invalid id.
    }
    else
    {
        // If not level or target, then it should be a LoadVars object
        SF_ASSERT(0);
        return;
    }

    int filelength = 0;
    // If the file is stripped, its loadMovie may be stripped too,
    // so try to load '.gfx' file first.
    if (stripped)
        urlStrGfx = GetUrlStrGfx(url);

    // Load movie: if there is no URL or it fails, do 'unloadMovie'.
    if (url.GetLength())
    {               
        // Check to see if URL is a user image substitute.
        bool    bilinearImage     = 0;
        bool    userImageProtocol = LoaderImpl::IsProtocolImage(url, &bilinearImage);

        if (userImageProtocol)
        {
            ImageCreator* pimageLoader = pls->GetImageCreator();
            // Create image through image callback
            if (pimageLoader)
            {
                ImageCreateInfo icinfo(ImageCreateInfo::Create_Protocol, GetMovieHeap());
                icinfo.SetStates(pls->GetLog(), pls->GetFileOpener(), pls->GetImageFileHandlerRegistry());
                icinfo.pMovie = GetMovieImpl();
                Ptr<Image> pimg = *pimageLoader->LoadProtocolImage(icinfo, url.ToCStr());
                if (pimg)
                {
                    Ptr<ImageResource> pimageRes = 
                        *SF_HEAP_NEW(GetMovieHeap()) ImageResource(pimg, Resource::Use_Bitmap);
                    if (pimageRes)
                    {
                        pmovieDef = *pMovieImpl->CreateImageMovieDef(pimageRes, bilinearImage,
                            url.ToCStr(), pls);
                    }
                }
                else if (plog)
                {
                    plog->LogScriptWarning(
                        "ImageCreator::LoadProtocolImage failed to load image \"%s\"", url.ToCStr());
                }
            }
            else if (plog)
            {
                plog->LogScriptWarning(
                    "ImageCreator is not installed,failed to load image \"%s\"", url.ToCStr());
            }
        }
        else
        {
            // Load the movie, first trying the .gfx file if necessary.
            // Our file loading can automatically detect and load images as well,
            // so we take advantage of that by passing the Loader::LoadImageFiles flag.

            // TBD: We will need to do something with delayed loading here in the future;
            // i.e. if the CreateMovie is executing in a different thread it will need to
            // post messages when it is done so that the handlers can be called.


            unsigned lf = loadFlags | Loader::LoadImageFiles | Loader::LoadWaitCompletion;
            if (pentry->QuietOpen)
                lf |= Loader::LoadQuietOpen;

            if (urlStrGfx.GetLength() > 0)
            {
                URLBuilder::LocationInfo loc(URLBuilder::File_LoadMovie,
                    urlStrGfx, level0Path);

                pmovieDef = *LoaderImpl::CreateMovie_LoadState(pls, loc, lf);
            }
            if (!pmovieDef)
            {
                URLBuilder::LocationInfo loc(URLBuilder::File_LoadMovie,
                    url, level0Path);

                pmovieDef = *LoaderImpl::CreateMovie_LoadState(pls, loc, lf);
            }

            if (pmovieDef)
            {
                // Record file length for progress report below. Once we do
                // threaded loading this will change.
                filelength = pmovieDef->GetFileBytes();
            }
            else if (plog && !pentry->QuietOpen)
            {
                plog->LogScriptWarning("Failed loading URL \"%s\"", url.ToCStr());
            }
        }   
    } 
    else
    {
        // if we unloading a movieclip we need to go over the list of loading queue which 
        // and cancel all items which are going to load into the same level or target
        // Note, we need to cancel ONLY entries, inserted BEFORE the "unloadMovie" entry!
        LoadQueueEntryMT *plentry = pMovieImpl->pLoadQueueMTHead;
        while(plentry)
        {
            if (plentry->GetQueueEntry()->EntryTime < pentry->EntryTime)
            {
                GFxAS2LoadQueueEntry* plqentry = static_cast<GFxAS2LoadQueueEntry*>(plentry->GetQueueEntry());
                if (pentry->Level != -1 && plqentry->Level != -1) 
                {
                    if (pentry->Level == plqentry->Level)
                        plqentry->Canceled = true;
                }
                else if (pentry->pCharacter && plqentry->pCharacter && pentry->pCharacter == plqentry->pCharacter)
                    plqentry->Canceled = true;
            }

            plentry = plentry->GetNext();
        }
    } // end if (url)

    // Create a new sprite of desired type.
    if (pmovieDef)
    {
        if (pmovieDef->GetVersion() != ~0u && (pmovieDef->GetVersion() >= 9 && pmovieDef->GetDataDef()->GetASVersion() > 2))
        {
            if (plog && !pentry->QuietOpen)
            {
                plog->LogScriptWarning("Failed loading SWF \"%s\": ActionScript version mismatch", 
                    url.ToCStr());
            }
            if (poldChar)
            {
                Environment* penv = ToAvmCharacter(poldChar)->GetASEnvironment();
                SF_ASSERT(penv);
                MovieClipLoader* pmovieClipLoader = 
                    static_cast<MovieClipLoader*>(pentry->MovieClipLoaderHolder.ToObject(penv));
                if (pmovieClipLoader)
                    pmovieClipLoader->NotifyOnLoadError(penv, poldChar, "ActionScriptMismatch", 0);
            }
            return;
        }
        else
        {
            // CharId of loaded clip is the same as the old one
            // Also mark clip as loadedSeparately, so that _lockroot works.
            CharacterCreateInfo ccinfo(pmovieDef->GetDataDef(), pmovieDef);
            pnewChar = *static_cast<Sprite*>(GetASSupport()->CreateCharacterInstance(
                GetMovieImpl(), ccinfo, pparent, newCharId, CharacterDef::Sprite));
            pnewChar->SetLoadedSeparately();
        }
    }
    bool charIsLoadedSuccessfully = (pnewChar.GetPtr() != NULL);

    if (pentry->Level == -1)
    {
        // Nested clip loading; replace poldChar.
        SF_ASSERT(poldChar);

        // If that failed, create an empty sprite. This also handles unloadMovie().
        if (!pnewChar)
        {
            CharacterCreateInfo ccinfo =
                pparent->GetResourceMovieDef()->GetCharacterCreateInfo
                (ResourceId(CharacterDef::CharId_EmptyMovieClip));
            pnewChar = *(Sprite*)pASSupport->CreateCharacterInstance
                (GetMovieImpl(), ccinfo, pparent, newCharId, CharacterDef::Sprite);
        }
        // And attach a new character in place of an old one.
        if (pnewChar)
        {
            pnewChar->AddToPlayList();
            //AB: we need to replicate the same CreateFrame, Depth and the Name.
            // Along with replicated Id this is important to maintain the loaded
            // movie on the stage. Otherwise, once the parent clip reaches the end
            // of its timeline the original movieclip will be recreated. Flash
            // doesn't recreate the original movie clip in this case, and it also
            // doesn't stop timeline animation for the target movieclip. The only way
            // to achieve both of these features is to copy Id, CreateFrame, Depth and
            // Name from the old character to the new one.
            // See last_to_first_frame_mc_no_reloadX.fla and 
            // test_timeline_anim_after_loadmovie.swf
            pnewChar->SetCreateFrame(poldChar->GetCreateFrame());
            pnewChar->SetDepth(poldChar->GetDepth());
            if (!poldChar->HasInstanceBasedName())
                pnewChar->SetName(poldChar->GetName());

            ToAvmCharacter(pparent)->ReplaceChildCharacter(poldChar, pnewChar);
            poldChar->SetParent(NULL);
            // In case permanent variables were assigned.. check them.
            ResolveStickyVariables(pnewChar);

            // need to update the optimized play list
            pnewChar->ModifyOptimizedPlayListLocal<Sprite>();
        }
    }
    else
    {
        // Level loading.
        SF_ASSERT(pentry->Type & GFxAS2LoadQueueEntry::LTF_LevelFlag);           
        if (pnewChar)
        {
            ToAvmSprite(pnewChar)->SetLevel(pentry->Level);
            SetLevelMovie(pentry->Level, pnewChar);
            // we will execute action script for frame 0 ourself here
            // this is needed because this script has to be executed 
            // before calling OnLoadInit
            G_SetFlag<MovieImpl::Flag_LevelClipsChanged>(pMovieImpl->Flags, false);
            // Check if permanent/sticky variables were assigned to level.
            ResolveStickyVariables(pnewChar);
        }

        if (!GetLevelMovie(0) && plog)
        {
            plog->LogScriptWarning("_level0 unloaded - no further playback possible");
            return;
        }
    }

    SF_ASSERT(GetLevelMovie(0));
    Environment* penv = GetAvmLevelMovie(0)->GetASEnvironment();
    SF_ASSERT(penv);
    MovieClipLoader* pmovieClipLoader = 
        static_cast<MovieClipLoader*>(pentry->MovieClipLoaderHolder.ToObject(penv));


    // Notify MovieClipLoader listeners. TODO: should be done in right places in right time.
    if (charIsLoadedSuccessfully)
    {
        if (pmovieClipLoader)
        {
            pmovieClipLoader->NotifyOnLoadStart(penv, pnewChar);
            pmovieClipLoader->NotifyOnLoadProgress(penv, pnewChar, filelength, filelength);
            pmovieClipLoader->NotifyOnLoadComplete(penv, pnewChar, 0);
        }
        // Run actions on Frame 0. This should be done before calling OnLoadInit
        //!AB: note, we can't use OnEventLoad here, need to use ExecuteFrame0Events. 
        // ExecuteFrame0Events executes onLoad event AFTER the first frame and Flash
        // does exactly the same.
        //pnewChar->OnEventLoad();
        pnewChar->ExecuteFrame0Events();
        DoActions();
        if (pmovieClipLoader)
            pmovieClipLoader->NotifyOnLoadInit(penv, pnewChar);
    } 
    else 
    {
        if (pmovieClipLoader)
        {
            if (url.GetLength())
            {
                // No new char is loaded - assuming the error occurs
                pmovieClipLoader->NotifyOnLoadError(penv, pnewChar, "URLNotFound", 0);
                // are there any other error codes? (AB)
            }
            else 
            {
                pmovieClipLoader->NotifyOnLoadError(penv, pnewChar, "Unknown error", 0);
            }
        }
    }
    if (url.GetLength() == 0)
    {
        // if unloadMovie then force oldChar to be released now and
        // force collect to flush text format cache and invoke garbage collection.
        poldChar = NULL;
        MemContext->LimHandler.Collect(GetMovieHeap());
    }
}


#ifdef GFX_ENABLE_XML
void    MovieRoot::ProcessLoadXML(LoadQueueEntry *pentry, LoadStates* pls)
{
    SF_ASSERT(pentry);
    SF_ASSERT((pentry->Type & GFxAS2LoadQueueEntry::LTF_XMLFlag) != 0);

    // Obtain level0 path before it has a chance to be unloaded below.
    String level0Path;
    GetLevel0Path(&level0Path);

    if (pentry->URL.GetLength())
    {
        URLBuilder::LocationInfo loc(URLBuilder::File_LoadXML, pentry->URL, level0Path);
        String                   fileName;
        pls->BuildURL(&fileName, loc);
        GFxAS2LoadQueueEntry* as2entry = static_cast<GFxAS2LoadQueueEntry*>(pentry);
        as2entry->XMLHolder.Loader->Load(fileName.ToCStr(), pls->GetFileOpener());
        Environment* penv = GetAvmLevelMovie(0)->GetASEnvironment();
        as2entry->XMLHolder.Loader->InitASXml(penv, as2entry->XMLHolder.ASObj.ToObject(penv));
    }
}
#endif


#ifdef GFX_ENABLE_CSS
void    MovieRoot::ProcessLoadCSS(LoadQueueEntry *pentry, LoadStates* pls)
{
    SF_ASSERT(pentry);
    SF_ASSERT((pentry->Type & GFxAS2LoadQueueEntry::LTF_CSSFlag) != 0);

    // Obtain level0 path before it has a chance to be unloaded below.
    String level0Path;
    GetLevel0Path(&level0Path);

    if (pentry->URL.GetLength())
    {
        URLBuilder::LocationInfo loc(URLBuilder::File_LoadCSS, pentry->URL, level0Path);
        String                   fileName;
        pls->BuildURL(&fileName, loc);
        GFxAS2LoadQueueEntry* as2entry = static_cast<GFxAS2LoadQueueEntry*>(pentry);
        as2entry->CSSHolder.Loader->Load(fileName.ToCStr(), pls->GetFileOpener());
        Environment* penv = GetAvmLevelMovie(0)->GetASEnvironment();
        as2entry->CSSHolder.Loader->Init(penv, as2entry->CSSHolder.ASObj.ToObject(penv));
    }
}
#endif

void MovieRoot::ProcessLoadVarsMT
(LoadQueueEntry *p_entry, LoadStates* pls, const String& data, UPInt fileLen, bool succeeded)
{
#ifdef GFX_AS2_ENABLE_LOADVARS
    // data read by LoadVarsTask::Execute() IS NOT unescaped yet. Do it now.
    String decodedData;
    ASUtils::Unescape(data.ToCStr(), data.GetSize(), &decodedData);

    DoProcessLoadVars(p_entry, pls, decodedData, fileLen, succeeded);
#else
    SF_UNUSED4(p_entry, pls, data, fileLen);
#endif
}

#ifdef GFX_AS2_ENABLE_LOADVARS
void MovieRoot::DoProcessLoadVars
    (LoadQueueEntry *p_entry, LoadStates* pls, const String& data, UPInt fileLen, bool /*succeeded*/)
{
    GFxAS2LoadQueueEntry *pentry = static_cast<GFxAS2LoadQueueEntry*>(p_entry);
    //if (!succeeded && pentry->LoadVarsHolder.IsObject())
    //    pentry->LoadVarsHolder.DropRefs();
    if (pentry->LoadVarsHolder.ToObject(NULL))
    {
        SF_ASSERT(pentry->LoadVarsHolder.ToObject(NULL)->GetObjectType() == ObjectInterface::Object_LoadVars);
        LoadVarsObject* ploadVars = 
            static_cast<LoadVarsObject*>(pentry->LoadVarsHolder.ToObject(NULL));
        Environment* penv = GetAvmLevelMovie(0)->GetASEnvironment();
        ASString str = penv->CreateString(data);
        ploadVars->SetLoadedBytes(Number(fileLen));
        ploadVars->NotifyOnData(penv, str);
    }
    else 
    {
        Ptr<InteractiveObject> pchar;
        if (pentry->Level == -1)
        {
            // character loading
            SF_ASSERT(pentry->pCharacter);
            pchar = static_cast<InteractiveObject*>(pentry->pCharacter->ResolveCharacter(pMovieImpl));
            SF_ASSERT(pchar);
        }
        else
        {
            // Level loading
            SF_ASSERT(pentry->Type & GFxAS2LoadQueueEntry::LTF_LevelFlag);           
            // use MovieLevels to get the appropriate level, and
            // add the variables into the level's sprite
            // find the right level movie
            pchar = GetLevelMovie(pentry->Level);

            // If level doesn't exist, create one using an empty MovieDef.
            if (!pchar)
                pchar = *CreateEmptySprite(pls, pentry->Level);
            if (!pchar)
                return;
        }
        //!AB - don't understand this code! @TODO!
        //         if (pchar->GetObjectType() == ObjectInterface::Object_LoadVars)
        //         {
        //             LoadVarsObject* lvobj = (LoadVarsObject*) pchar.GetPtr();
        //             lvobj->SetLoadedBytes(Number(fileLen));
        //         }
        LoadVarsProto::LoadVariables(GetAvmLevelMovie(0)->GetASEnvironment(), ToAvmCharacter(pchar), data);
    }
}

void    MovieRoot::ProcessLoadVars(LoadQueueEntry *p_entry, LoadStates* pls)
{
    SF_ASSERT(p_entry);
    SF_ASSERT((p_entry->Type & GFxAS2LoadQueueEntry::LTF_VarsFlag) != 0);

    GFxAS2LoadQueueEntry *pentry = static_cast<GFxAS2LoadQueueEntry*>(p_entry);
    // Obtain level0 path before it has a chance to be unloaded below.
    String level0Path;
    GetLevel0Path(&level0Path);

    String data;
    int    fileLen = 0;
    if (pentry->URL.GetLength())
    {
        bool userVarsProtocol = 0;

        // @TODO logic to determine if a user protocol was used

        if (userVarsProtocol)
        {
            // @TODO
        }
        else
        {
            URLBuilder::LocationInfo loc(URLBuilder::File_LoadVars, pentry->URL, level0Path);
            String                   fileName;
            pls->BuildURL(&fileName, loc);

            // File loading protocol
            Ptr<File> pfile;
            Array<UByte> bytes;
            if (URLBuilder::IsProtocol(fileName))
            {
#ifdef SF_ENABLE_HTTP_LOADING
                URLBuilder::UrlMethod sendUrlRequest = URLBuilder::Url_Method_None;
                switch (pentry->Method)
                {
                case LoadQueueEntry::LM_Get:
                    sendUrlRequest = URLBuilder::Url_Method_Get;
                    break;
                case LoadQueueEntry::LM_Post:
                    sendUrlRequest = URLBuilder::Url_Method_Post;
                    break;
                case LoadQueueEntry::LM_Put:
                    sendUrlRequest = URLBuilder::Url_Method_Put;
                    break;
                case LoadQueueEntry::LM_Delete:
                    sendUrlRequest = URLBuilder::Url_Method_Delete;
                    break;
                case LoadQueueEntry::LM_None:
                    break;
                default:
                    SF_ASSERT(false); // Unsupported method
                    break;
                }

                if (sendUrlRequest != URLBuilder::Url_Method_None)
                {
                    if (URLBuilder::SendURLRequest(&bytes, fileName, sendUrlRequest)
                        && !bytes.IsEmpty())
                    {
                        pfile = *SF_NEW MemoryFile(fileName, bytes.GetDataPtr(), (int)bytes.GetSize());
                    }
                }
#endif
            }
            else
            {
                pfile = *pls->OpenFile(fileName.ToCStr());
            }
            if (pfile)
            {
                if (pentry->LoadVarsHolder.IsObject())
                {
                    if (!MovieImpl::ReadTextData(&data, pfile, &fileLen, true))
                        pentry->LoadVarsHolder.DropRefs();
                }
                else 
                {
                    MovieImpl::ReadTextData(&data, pfile, &fileLen, true);
                }
            }
        }
    }
    DoProcessLoadVars(pentry, pls, data, fileLen, true);
}

Sprite* MovieRoot::CreateEmptySprite(LoadStates* pls, int level)
{
    // Technically this is not necessary since empty 'DataDefs' are not stored 
    // in the library right now, but the argument must be passed for consistency.
    // TBD: Perhaps we could hash it globally, since there should be only one?
    ResourceKey createKey = MovieDataDef::CreateMovieFileKey("", 0, 0, 0);

    // Create a MovieDataDef containing our image (and an internal corresponding ShapeDef).
    Ptr<MovieDataDef> pemptyDataDef = 
        *MovieDataDef::Create(createKey, MovieDataDef::MT_Empty, "", GetMovieHeap());

    Ptr<MovieDefImpl> pemptyMovieImpl;
    if (pemptyDataDef)
    {
        pemptyDataDef->InitEmptyMovieDef();
        pls->SetRelativePathForDataDef(pemptyDataDef);
        pemptyMovieImpl = 
            *SF_HEAP_NEW(GetMovieHeap()) MovieDefImpl(pemptyDataDef, pls->GetBindStates(), 
            pls->pLoaderImpl, Loader::LoadAll, 
            pMovieImpl->pStateBag->GetDelegate(), GetMovieHeap(), true);
    }
    if (!pemptyMovieImpl)
        return NULL;

    Sprite* pnewChar = CreateSprite(pemptyDataDef, pemptyMovieImpl, NULL, ResourceId(), true);
//     Sprite* pnewChar = SF_HEAP_NEW(GetMovieHeap()) Sprite(pemptyDataDef, pemptyMovieImpl, this,
//         NULL, ResourceId(), true);
    ToAvmSprite(pnewChar)->SetLevel(level);
    SetLevelMovie(level, pnewChar);
    return pnewChar;
}
#endif //#ifdef GFX_AS2_ENABLE_LOADVARS

bool MovieRoot::InvokeAlias(const char* pmethodName, const InvokeAliasInfo& alias,
                            Value *presult, unsigned numArgs)
{
    if (GetLevelMovie(0))
    {
        Ptr<Object> pobj = alias.ThisObject; // make a copy to hold ref on object
        Ptr<InteractiveObject> pchar;
        if (alias.ThisChar)
        {
            pchar = static_cast<InteractiveObject*>(alias.ThisChar->ResolveCharacter(pMovieImpl));
        }
        ObjectInterface* pthis;
        if (pobj) 
            pthis = pobj;
        else
            pthis = ToAvmCharacter(pchar);
        Environment* penv = GetAvmLevelMovie(0)->GetASEnvironment();
        return GAS_Invoke(alias.Function, presult, pthis, penv,
                          numArgs, penv->GetTopIndex(), pmethodName);
    }
    return false;
}

bool MovieRoot::InvokeAliasArgs(const char* pmethodName, const InvokeAliasInfo& alias,
                                Value *presult, const char* methodArgFmt, va_list args)
{
    if (GetLevelMovie(0))
    {
        Ptr<Object> pobj = alias.ThisObject; // make a copy to hold ref on object
        Ptr<InteractiveObject> pchar;
        if (alias.ThisChar)
        {
            pchar = static_cast<InteractiveObject*>(alias.ThisChar->ResolveCharacter(pMovieImpl));
        }
        ObjectInterface* pthis;
        if (pobj) 
            pthis = pobj;
        else
            pthis = ToAvmCharacter(pchar);
        Environment* penv = GetAvmLevelMovie(0)->GetASEnvironment();
        return GAS_InvokeParsed(alias.Function, presult, pthis, penv,
                                methodArgFmt, args, pmethodName);
    }
    return false;
}

void MovieRoot::AddInvokeAlias(const ASString& alias, CharacterHandle* pthisChar, 
                               Object* pthisObj, const FunctionRef& func)
{
    if (!pInvokeAliases)
        pInvokeAliases = new ASStringHash<InvokeAliasInfo>();
    InvokeAliasInfo aliasInfo;
    aliasInfo.ThisObject = pthisObj;
    aliasInfo.ThisChar   = pthisChar;
    aliasInfo.Function   = func;
    pInvokeAliases->Set(alias, aliasInfo);
}

Text::Allocator* MovieRoot::GetTextAllocator()
{
    if (!MemContext->TextAllocator)
    {
        MemContext->TextAllocator = *SF_HEAP_NEW(MemContext->Heap) Text::Allocator(MemContext->Heap);
    }
    return MemContext->TextAllocator;
}


MovieRoot::InvokeAliasInfo* MovieRoot::ResolveInvokeAlias(const char* pstr) const
{
    if (pInvokeAliases && GetLevelMovie(0))
    {
        ASString str = GetAvmLevelMovie(0)->GetASEnvironment()->CreateString(pstr);
        return pInvokeAliases->Get(str);
    }
    return NULL;
}

// Action Script access.

bool MovieRoot::GetVariable(GFx::Value *pval, const char* ppathToVar) const
{
    if (!GetLevelMovie(0) || !pval)
        return 0;

    // Need to restore high precision mode of FPU for X86 CPUs.
    // Direct3D may set the Mantissa Precision Control Bits to 24-bit (by default 53-bits) and this 
    // leads to bad precision of FP arithmetic. For example, the result of 0.0123456789 + 1.0 will 
    // be 1.0123456789 with 53-bit mantissa mode and 1.012345671653 with 24-bit mode.
    DoublePrecisionGuard dpg;

    Environment* penv = GetAvmLevelMovie(0)->GetASEnvironment();
    ASString     path(penv->CreateString(ppathToVar));

    Value retVal;
    if (penv->GetVariable(path, &retVal))
    {
        ASValue2Value(penv, retVal, pval);
        return true;
    }
    return false;
}

bool MovieRoot::SetVariable(const char* ppathToVar, const GFx::Value& value, Movie::SetVarType setType)
{
    if (!GetLevelMovie(0))
        return 0;
    if (!ppathToVar)
    {
        if (GetLog())
            GetLog()->LogError("NULL pathToVar passed to SetVariable/SetDouble()");
        return 0;
    }

    // Need to restore high precision mode of FPU for X86 CPUs.
    // Direct3D may set the Mantissa Precision Control Bits to 24-bit (by default 53-bits) and this 
    // leads to bad precision of FP arithmetic. For example, the result of 0.0123456789 + 1.0 will 
    // be 1.0123456789 with 53-bit mantissa mode and 1.012345671653 with 24-bit mode.
    DoublePrecisionGuard dpg;

    Environment* penv = GetAvmLevelMovie(0)->GetASEnvironment();
    ASString     path(penv->CreateString(ppathToVar));

    Value val;
    Value2ASValue(value, &val);
    bool setResult = GetAvmLevelMovie(0)->GetASEnvironment()->SetVariable(path, val, NULL, 
        (setType == Movie::SV_Normal));

    if ( (!setResult && (setType != Movie::SV_Normal)) ||
        (setType == Movie::SV_Permanent) )
    {
        // Store in sticky hash.
        AddStickyVariable(path, val, setType);
    }
    return setResult;
}

bool MovieRoot::SetVariableArray(Movie::SetArrayType type, const char* ppathToVar,
                                 unsigned index, const void* pdata, unsigned count, Movie::SetVarType setType)
{
    if (!GetLevelMovie(0))
        return false;

    // Need to restore high precision mode of FPU for X86 CPUs.
    // Direct3D may set the Mantissa Precision Control Bits to 24-bit (by default 53-bits) and this 
    // leads to bad precision of FP arithmetic. For example, the result of 0.0123456789 + 1.0 will 
    // be 1.0123456789 with 53-bit mantissa mode and 1.012345671653 with 24-bit mode.
    DoublePrecisionGuard dpg;

    Environment* penv = GetAvmLevelMovie(0)->GetASEnvironment();
    ASString     path(penv->CreateString(ppathToVar));

    Ptr<ArrayObject> parray;
    Value retVal;
    if(penv->GetVariable(path, &retVal) && retVal.IsObject())
    {
        Object* pobj = retVal.ToObject(penv);
        if (pobj && pobj->GetObjectType() == Object::Object_Array)
            parray = static_cast<ArrayObject*>(pobj);
    }

    if (!parray)
    {
        parray = *SF_HEAP_NEW(GetMovieHeap()) ArrayObject(GetAvmLevelMovie(0)->GetASEnvironment());
    }
    if (count + index > unsigned(parray->GetSize()))
        parray->Resize(count + index);

    switch(type)
    {
    case Movie::SA_Int:
        {
            const int* parr = static_cast<const int*>(pdata);
            for (unsigned i = 0; i < count; ++i)
            {
                parray->SetElement(i + index, Value(parr[i]));
            }
            break;
        }
    case Movie::SA_Float:
        {
            const float* parr = static_cast<const float*>(pdata);
            for (unsigned i = 0; i < count; ++i)
            {
                parray->SetElement(i + index, Value(Number(parr[i])));
            }
            break;
        }
    case Movie::SA_Double:
        {
            const Double* parr = static_cast<const Double*>(pdata);
            for (unsigned i = 0; i < count; ++i)
            {
                parray->SetElement(i + index, Value(Number(parr[i])));
            }
            break;
        }
    case Movie::SA_Value:
        {
            const GFx::Value* parr = static_cast<const GFx::Value*>(pdata);
            for (unsigned i = 0; i < count; ++i)
            {
                Value asVal;
                Value2ASValue(parr[i], &asVal);
                parray->SetElement(i + index, asVal);
            }
            break;
        }
    case Movie::SA_String:
        {
            const char* const* parr = static_cast<const char* const*>(pdata);
            for (unsigned i = 0; i < count; ++i)
            {
                parray->SetElement(i + index, Value(GetStringManager()->CreateString(parr[i])));
            }
            break;
        }
    case Movie::SA_StringW:
        {
            const wchar_t* const* parr = static_cast<const wchar_t* const*>(pdata);
            for (unsigned i = 0; i < count; ++i)
            {
                parray->SetElement(i + index, Value(GetStringManager()->CreateString(parr[i])));
            }
            break;
        }
    }

    Value val;
    val.SetAsObject(parray);

    bool setResult = GetAvmLevelMovie(0)->GetASEnvironment()->SetVariable(path, val, NULL, 
        (setType == Movie::SV_Normal));

    if ( (!setResult && (setType != Movie::SV_Normal)) ||
        (setType == Movie::SV_Permanent) )
    {
        // Store in sticky hash.
        AddStickyVariable(path, val, setType);
    }
    return setResult;
}

bool MovieRoot::SetVariableArraySize(const char* ppathToVar, unsigned count, Movie::SetVarType setType)
{
    if (!GetLevelMovie(0))
        return false;

    Environment* penv = GetAvmLevelMovie(0)->GetASEnvironment();
    ASString     path(penv->CreateString(ppathToVar));

    Ptr<ArrayObject> parray;
    Value retVal;
    if(penv->GetVariable(path, &retVal) && retVal.IsObject())
    {
        Object* pobj = retVal.ToObject(penv);
        if (pobj && pobj->GetObjectType() == Object::Object_Array)
        {
            // Array already exists - just resize
            parray = static_cast<ArrayObject*>(pobj);
            if (count != unsigned(parray->GetSize()))
                parray->Resize(count);
            return true;
        }
    }

    // no array found - allocate new one, resize and set
    parray = *SF_HEAP_NEW(GetMovieHeap()) ArrayObject(GetAvmLevelMovie(0)->GetASEnvironment());
    parray->Resize(count);
    Value val;
    val.SetAsObject(parray);

    bool setResult = GetAvmLevelMovie(0)->GetASEnvironment()->SetVariable(path, val, NULL, 
        (setType == Movie::SV_Normal));

    if ( (!setResult && (setType != Movie::SV_Normal)) ||
        (setType == Movie::SV_Permanent) )
    {
        // Store in sticky hash.
        AddStickyVariable(path, val, setType);
    }
    return setResult;
}

unsigned MovieRoot::GetVariableArraySize(const char* ppathToVar)
{
    if (!GetLevelMovie(0))
        return 0;

    Environment* penv = GetAvmLevelMovie(0)->GetASEnvironment();
    ASString     path(penv->CreateString(ppathToVar));

    Value retVal;
    if (penv->GetVariable(path, &retVal) && retVal.IsObject())
    {
        Object* pobj = retVal.ToObject(penv);
        if (pobj && pobj->GetObjectType() == Object::Object_Array)
        {
            ArrayObject* parray = static_cast<ArrayObject*>(pobj);
            return parray->GetSize();
        }
    }
    return 0;
}

bool MovieRoot::GetVariableArray(Movie::SetArrayType type, const char* ppathToVar,
                                 unsigned index, void* pdata, unsigned count)
{
    if (!GetLevelMovie(0))
        return false;

    // Need to restore high precision mode of FPU for X86 CPUs.
    // Direct3D may set the Mantissa Precision Control Bits to 24-bit (by default 53-bits) and this 
    // leads to bad precision of FP arithmetic. For example, the result of 0.0123456789 + 1.0 will 
    // be 1.0123456789 with 53-bit mantissa mode and 1.012345671653 with 24-bit mode.
    DoublePrecisionGuard dpg;

    Environment* penv = GetAvmLevelMovie(0)->GetASEnvironment();
    ASString     path(penv->CreateString(ppathToVar));

    Value retVal;
    if(penv->GetVariable(path, &retVal) && retVal.IsObject())
    {
        Object* pobj = retVal.ToObject(penv);
        if (pobj && pobj->GetObjectType() == Object::Object_Array)
        {
            MovieImpl::ReturnValueHolder* rvHolder = pMovieImpl->GetRetValHolder();
            rvHolder->ResetPos();
            rvHolder->ResizeStringArray(0);

            ArrayObject* parray = static_cast<ArrayObject*>(pobj);
            unsigned arrSize = parray->GetSize();
            switch(type)
            {
            case Movie::SA_Int:
                {
                    int* parr = static_cast<int*>(pdata);
                    for (unsigned i = 0, n = Alg::Min(arrSize, count); i < n; ++i)
                    {
                        Value* pval = parray->GetElementPtr(i + index);
                        if (pval)
                            parr[i] = (int)pval->ToNumber(penv);
                        else
                            parr[i] = 0;
                    }
                }
                break;
            case Movie::SA_Float:
                {
                    float* parr = static_cast<float*>(pdata);
                    for (unsigned i = 0, n = Alg::Min(arrSize, count); i < n; ++i)
                    {
                        Value* pval = parray->GetElementPtr(i + index);
                        if (pval)
                            parr[i] = (float)pval->ToNumber(penv);
                        else
                            parr[i] = 0;
                    }
                }
                break;
            case Movie::SA_Double:
                {
                    Double* parr = static_cast<Double*>(pdata);
                    for (unsigned i = 0, n = Alg::Min(arrSize, count); i < n; ++i)
                    {
                        Value* pval = parray->GetElementPtr(i + index);
                        if (pval)
                            parr[i] = (Double)pval->ToNumber(penv);
                        else
                            parr[i] = 0;
                    }
                }
                break;
            case Movie::SA_Value:
                {
                    GFx::Value* parr = static_cast<GFx::Value*>(pdata);
                    for (unsigned i = 0, n = Alg::Min(arrSize, count); i < n; ++i)
                    {
                        Value* pval = parray->GetElementPtr(i + index);
                        GFx::Value* pdestVal = &parr[i];
                        pdestVal->SetUndefined();
                        if (pval)
                            ASValue2Value(penv, *pval, pdestVal);
                        else
                            pdestVal->SetUndefined();
                    }
                }
                break;
            case Movie::SA_String:
                {
                    const char** parr = static_cast<const char**>(pdata);
                    unsigned n = Alg::Min(arrSize, count);
                    rvHolder->ResizeStringArray(n);
                    for (unsigned i = 0; i < n; ++i)
                    {
                        Value* pval = parray->GetElementPtr(i + index);
                        if (pval)
                        {
                            ASString str = pval->ToString(penv);
                            parr[i] = str.ToCStr();
                            rvHolder->StringArray[rvHolder->StringArrayPos++] = str;   
                        }
                        else
                            parr[i] = NULL;
                    }
                }
                break;
            case Movie::SA_StringW:
                {
                    const wchar_t** parr = static_cast<const wchar_t**>(pdata);
                    // pre-calculate the size of required buffer
                    unsigned i, bufSize = 0;
                    unsigned n = Alg::Min(arrSize, count);
                    rvHolder->ResizeStringArray(n);
                    for (i = 0; i < arrSize; ++i)
                    {
                        Value* pval = parray->GetElementPtr(i + index);
                        if (pval)
                        {
                            ASString str = pval->ToString(penv);
                            rvHolder->StringArray[i] = str;
                            bufSize += str.GetLength()+1;
                        }
                    }
                    wchar_t* pwBuffer = (wchar_t*)rvHolder->PreAllocateBuffer(bufSize * sizeof(wchar_t));

                    for (i = 0; i < n; ++i)
                    {
                        const char* psrcStr = rvHolder->StringArray[i].ToCStr();
                        wchar_t* pwdstStr = pwBuffer;
                        UInt32 code;
                        while ((code = UTF8Util::DecodeNextChar(&psrcStr)) != 0)
                        {
                            *pwBuffer++ = (wchar_t)code;
                        }
                        *pwBuffer++ = 0;

                        parr[i] = pwdstStr;
                    }
                    rvHolder->ResizeStringArray(0);
                }
                break;
            }
            return true;
        }
    }
    return false;
}

void MovieRoot::AddStickyVariable(const ASString& fullPath, const Value &val, Movie::SetVarType setType)
{
    // Parse the path first.
    ASStringContext sc(pGlobalContext, 8);
    ASString   path(sc.GetBuiltin(ASBuiltin_empty_));
    ASString   name(sc.GetBuiltin(ASBuiltin_empty_));

    if (!Environment::ParsePath(&sc, fullPath, &path, &name))
    {
        if (name.IsEmpty())
            return;

        // This can happen if a sticky variable did not have a path. In that
        // case assume it is _level0.
        path = sc.GetBuiltin(ASBuiltin__level0);
    }
    else
    {
        // TBD: Verify/clean up path integrity?
        //  -> no _parent, _global, "..", ".", "this"
        //  -> _root converts to _level0
        // "/" converts to "."
        bool    needLevelPrefix = 0;

        if (path.GetSize() >= 5)
        {           
            if (!memcmp(path.ToCStr(), "_root", 5))
            {
                // Translate _root.
                path = sc.GetBuiltin(ASBuiltin__level0) + path.Substring(5, path.GetLength());
            }

            // Warn about invalid path components in debug builds.
#ifdef  SF_BUILD_DEBUG         
            const char *p = strstr(path.ToCStr(), "_parent");
            if (!p) p = strstr(path.ToCStr(), "_global");

            if (p)
            {
                if ((p == path.ToCStr()) || *(p-1) == '.')
                {
                    SF_DEBUG_WARNING(1, "SetVariable - Sticky/Permanent variable path can not include _parent or _global");
                }
            }
#endif

            // Path must start with _levelN.
            if (memcmp(path.ToCStr(), "_level", 6) != 0)
            {
                needLevelPrefix = 1;
            }
        }
        else
        {
            needLevelPrefix = 1;
        }

        if (needLevelPrefix)
            path = sc.GetBuiltin(ASBuiltin__level0dot_) + path;
    }

    StickyVarNode* pnode;
    if ((pnode = SF_HEAP_NEW(GetMovieHeap()) StickyVarNode(name, val, (setType == Movie::SV_Permanent))) != 0)
        pMovieImpl->AddStickyVariableNode(path, pnode);
}

void    MovieRoot::ResolveStickyVariables(InteractiveObject *pch)
{
    SF_ASSERT(pch);

    const ASString &path  = pch->GetCharacterHandle()->GetNamePath();
    MovieImpl::StickyVarNode* _pnode = 0;
    StickyVarNode*   p, *pnext, *pnode;
    AvmCharacter* pcharacter = ToAvmCharacter(pch);

    if (pMovieImpl->StickyVariables.Get(path, &_pnode))
    {       
        pnode = static_cast<StickyVarNode*>(_pnode);
        // If found, add variables.     
        // We also remove sticky nodes while keeping permanent ones.
        StickyVarNode*  ppermanent = 0;
        StickyVarNode*  ppermanentTail = 0;

        p = pnode;

        while (p)
        {
            pcharacter->SetMember(pcharacter->GetASEnvironment(), p->Name, p->mValue);
            pnext = static_cast<StickyVarNode*>(p->pNext);

            if (p->Permanent)
            {
                // If node is permanent, create a permanent-only linked list.
                if (!ppermanent)
                {
                    ppermanent = p;
                }
                else
                {
                    ppermanentTail->pNext = p;
                }

                ppermanentTail = p;
                p->pNext = 0;
            }
            else
            {
                // If not permanent, delete this node.
                delete p;
            }

            p = pnext;
        }


        if (ppermanent)
        {
            // If permanent list was formed, keep it.
            if (ppermanent != pnode)
                pMovieImpl->StickyVariables.Set(path, ppermanent);
        }
        else
        {
            // Otherwise delete hash key.
            pMovieImpl->StickyVariables.Remove(path);
        }
    }
}

InteractiveObject* MovieRoot::FindTarget(const ASString& path) const
{
    if (!GetLevelMovie(0) || path.IsEmpty())
        return 0;

    // This will work since environment parses _levelN prefixes correctly.
    // However, it would probably be good to move the general FindTarget logic here.
    return GetAvmLevelMovie(0)->GetASEnvironment()->FindTarget(path);
}

// Fills in a file system path relative to _level0.
bool MovieRoot::GetLevel0Path(String *ppath) const
{       
    if (!GetLevelMovie(0))
    {
        ppath->Clear();
        return false;
    }

    // Get URL.
    *ppath = GetMovieDefImpl()->GetFileURL();

    // Extract path by clipping off file name.
    if (!URLBuilder::ExtractFilePath(ppath))
    {
        ppath->Clear();
        return false;
    }
    return true;
}

void MovieRoot::NotifyOnResize()
{
#ifdef GFX_AS2_ENABLE_STAGE
    if (GetLevelMovie(0))
    {
        // queue onResize
        MovieRoot::ActionEntry* pe = InsertEmptyAction(MovieRoot::AP_Frame);
        if (pe) pe->SetAction(GetLevelMovie(0), StageCtorFunction::NotifyOnResize, NULL);
    }
#endif // SF_NO_FXPLAYER_AS_STAGE
}

bool MovieRoot::IsAvailable(const char* pathToVar) const
{
    if (!GetLevelMovie(0))
        return 0;

    Environment* penv = GetAvmLevelMovie(0)->GetASEnvironment();
    ASString     path(penv->CreateString(pathToVar));

    return penv->IsAvailable(path);
}

bool MovieRoot::Invoke(const char* pmethodName, GFx::Value *presult,
                       const GFx::Value* pargs, unsigned numArgs)
{
    return Invoke(GetLevelMovie(0), pmethodName, presult, pargs, numArgs);
}

// For ActionScript interfacing convenience.
bool MovieRoot::Invoke(InteractiveObject* thisCont, const char* pmethodName, GFx::Value *presult,
                       const GFx::Value* pargs, unsigned numArgs)
{
    if (!thisCont || thisCont->GetType() != CharacterDef::Sprite) return 0;
    Sprite* thisSpr = static_cast<Sprite*>(thisCont);

    // Need to restore high precision mode of FPU for X86 CPUs.
    // Direct3D may set the Mantissa Precision Control Bits to 24-bit (by default 53-bits) and this 
    // leads to bad precision of FP arithmetic. For example, the result of 0.0123456789 + 1.0 will 
    // be 1.0123456789 with 53-bit mantissa mode and 1.012345671653 with 24-bit mode.
    DoublePrecisionGuard dpg;

    Value resultVal;
    AvmSprite*   avmSpr = ToAvmSprite(thisSpr);
    Environment* penv   = avmSpr->GetASEnvironment();
    SF_ASSERT(penv);

    // push arguments directly into environment
    for (int i = (int)numArgs - 1; i >= 0; --i)
    {
        SF_ASSERT(pargs); // should be checked only if numArgs > 0
        const GFx::Value& gfxVal = pargs[i];
        Value asval;
        Value2ASValue(gfxVal, &asval);
        penv->Push(asval);
    }
    bool retVal;

    // try to resolve alias
    InvokeAliasInfo* palias;
    if (pInvokeAliases && (palias = ResolveInvokeAlias(pmethodName)) != NULL)
        retVal = InvokeAlias(pmethodName, *palias, &resultVal, numArgs);
    else
        retVal = avmSpr->Invoke(pmethodName, &resultVal, numArgs);
    penv->Drop(numArgs); // release arguments
    if (retVal && presult)
    {
        // convert result to Value
        ASValue2Value(penv, resultVal, presult);
    }
    return retVal;
}

bool MovieRoot::Invoke(const char* pmethodName, GFx::Value *presult, const char* pargFmt, ...)
{
    if (!GetLevelMovie(0)) return 0;

    // Need to restore high precision mode of FPU for X86 CPUs.
    // Direct3D may set the Mantissa Precision Control Bits to 24-bit (by default 53-bits) and this 
    // leads to bad precision of FP arithmetic. For example, the result of 0.0123456789 + 1.0 will 
    // be 1.0123456789 with 53-bit mantissa mode and 1.012345671653 with 24-bit mode.
    DoublePrecisionGuard dpg;

    Value resultVal;
    va_list args;
    va_start(args, pargFmt);

    bool retVal;
    // try to resolve alias
    InvokeAliasInfo* palias;
    if (pInvokeAliases && (palias = ResolveInvokeAlias(pmethodName)) != NULL)
        retVal = InvokeAliasArgs(pmethodName, *palias, &resultVal, pargFmt, args);
    else
        retVal = GetAvmLevelMovie(0)->InvokeArgs(pmethodName, &resultVal, pargFmt, args);
    va_end(args);
    if (retVal && presult)
    {
        // convert result to Value
        ASValue2Value(GetAvmLevelMovie(0)->GetASEnvironment(), resultVal, presult);
    }
    return retVal;
}


bool MovieRoot::InvokeArgs(const char* pmethodName, GFx::Value *presult, const char* pargFmt, va_list args)
{
    if (GetLevelMovie(0))
    {
        // Need to restore high precision mode of FPU for X86 CPUs.
        // Direct3D may set the Mantissa Precision Control Bits to 24-bit (by default 53-bits) and this 
        // leads to bad precision of FP arithmetic. For example, the result of 0.0123456789 + 1.0 will 
        // be 1.0123456789 with 53-bit mantissa mode and 1.012345671653 with 24-bit mode.
        DoublePrecisionGuard dpg;

        Value resultVal;
        bool retVal;
        // try to resolve alias
        InvokeAliasInfo* palias;
        if (pInvokeAliases && (palias = ResolveInvokeAlias(pmethodName)) != NULL)
            retVal = InvokeAliasArgs(pmethodName, *palias, &resultVal, pargFmt, args);
        else
            retVal = GetAvmLevelMovie(0)->InvokeArgs(pmethodName, &resultVal, pargFmt, args);
        if (retVal && presult)
        {
            // convert result to Value
            ASValue2Value(GetAvmLevelMovie(0)->GetASEnvironment(), resultVal, presult);
        }
        return retVal;
    }
    return false;
}

void MovieRoot::SetExternalInterfaceRetVal(const GFx::Value& retVal)
{
    Value2ASValue(retVal, &ExternalIntfRetVal);
}

void MovieRoot::NotifyMouseEvent(const InputEventsQueue::QueueEntry* qe, const MouseState& ms, int mi)
{
    Environment* penv = GetAvmLevelMovie(0)->GetASEnvironment();
    if (pASMouseListener && !pASMouseListener->IsEmpty() && 
        (ms.IsMouseMoved() || qe->GetMouseEntry().IsButtonsStateChanged() || 
        qe->GetMouseEntry().IsMouseWheel()))
    {
        // Notify Mouse class' AS listeners

        bool extensions = penv->CheckExtensions();
        if (ms.IsMouseMoved())
            pASMouseListener->OnMouseMove(penv, mi);
        if (qe->GetMouseEntry().IsMouseWheel() || qe->GetMouseEntry().IsButtonsStateChanged())
        {
            Ptr<InteractiveObject> ptopMouseCharacter = pMovieImpl->GetTopMostEntity(qe->GetMouseEntry().GetPosition(), mi, true);
            if (ptopMouseCharacter && !ptopMouseCharacter->IsAVM1()) // check AS version
                return;
            InteractiveObject* pasTopChar = static_cast<InteractiveObject*>(ptopMouseCharacter.GetPtr());
            if (qe->GetMouseEntry().IsMouseWheel())
                pASMouseListener->OnMouseWheel(penv, mi, qe->GetMouseEntry().WheelScrollDelta, pasTopChar);
            if (qe->GetMouseEntry().IsButtonsStateChanged())
            {
                for (unsigned mask = 1, buttonId = 1; MouseState::MouseButton_AllMask & mask; mask <<= 1, ++buttonId)
                {
                    if (qe->GetMouseEntry().ButtonsState & mask)
                    {
                        if (qe->GetMouseEntry().IsAnyButtonPressed())
                            pASMouseListener->OnMouseDown(penv, mi, buttonId, pasTopChar);
                        else
                            pASMouseListener->OnMouseUp(penv, mi, buttonId, pasTopChar);
                    }
                    if (!extensions) break; // support only left button, if extensions are not enabled
                }
            }
        }
    }
}

bool MovieRoot::NotifyOnFocusChange(InteractiveObject* curFocused, InteractiveObject* newFocus, 
                                    unsigned, FocusMovedType fmt, ProcessFocusKeyInfo*)
{
    if (fmt == GFx_FocusMovedByMouse)
    {
        if (curFocused && curFocused->IsFocusEnabled(fmt))
            return true;
        else if (newFocus && newFocus->IsFocusEnabled(fmt))
            return true;
        return false;
    }
    return true;
}

void MovieRoot::ChangeMouseCursorType(unsigned mouseIdx, unsigned newCursorType)
{
#ifdef GFX_AS2_ENABLE_MOUSE
    Environment* penv = GetAvmLevelMovie(0)->GetASEnvironment();
    if (G_IsFlagSet<MovieImpl::Flag_SetCursorTypeFuncOverloaded>(pMovieImpl->Flags) &&
        penv->CheckExtensions())
    {
        // extensions enabled and Mouse.setCursorType function was overloaded
        // via ActionScript.
        // find Mouse.setCursorType and invoke it
        // Note, we need to invoke Mouse.setCursorType even if cursor type is the
        // same as before: this will allow to track changes in cursor position from
        // the custom setCursorType.
        Value objVal;
        if (penv->GetGC()->pGlobal->GetMemberRaw(
                penv->GetSC(), penv->GetBuiltin(ASBuiltin_Mouse), &objVal))
        {
            Object* pobj = objVal.ToObject(penv);
            if (pobj)
            {
                Value scval;
                if (pobj->GetMember(penv, penv->GetBuiltin(ASBuiltin_setCursorType), &scval))
                {
                    FunctionRef funcRef = scval.ToFunction(penv);
                    if (!funcRef.IsNull())
                    {
                        Value res;
                        penv->Push(Number(mouseIdx));
                        penv->Push(Number(newCursorType));
                        funcRef.Invoke(FnCall(&res, objVal, penv, 2, penv->GetTopIndex()));
                        penv->Drop2();    
                    }
                }
            }
        }
    }
    else
    {
        // extensions disabled; just call static method MouseCtorFunction::SetCursorType
        if (newCursorType != pMovieImpl->mMouseState[mouseIdx].GetCursorType())
            MouseCtorFunction::SetCursorType(pMovieImpl, mouseIdx, newCursorType);
    }
#endif
}

void MovieRoot::NotifyQueueSetFocus(InteractiveObject* ch, unsigned controllerIdx, FocusMovedType fmt)
{
    // Queue setting focus to ch (steps 3-6)
    Selection::QueueSetFocus(GetAvmLevelMovie(0)->GetASEnvironment(), ch, controllerIdx, fmt); 
}

void MovieRoot::NotifyTransferFocus(InteractiveObject* curFocused, InteractiveObject* pNewFocus, unsigned controllerIdx)
{
    Selection::BroadcastOnSetFocus(GetAvmLevelMovie(0)->GetASEnvironment(), curFocused, pNewFocus, controllerIdx);
}



GFxAS2LoadQueueEntryMT_LoadMovie::GFxAS2LoadQueueEntryMT_LoadMovie(LoadQueueEntry* pqueueEntry, MovieImpl* pmovieRoot)
: LoadQueueEntryMT(pqueueEntry, pmovieRoot),
NewCharId(CharacterDef::CharId_EmptyMovieClip), CharSwitched(false), 
BytesLoaded(0), FirstFrameLoaded(false)
{
    bool stripped = false;

    GFxAS2LoadQueueEntry* qe = static_cast<GFxAS2LoadQueueEntry*>(pQueueEntry);
    if (qe->pCharacter)
    {
        // This is loadMovie, not level.
        SF_ASSERT(qe->Level == -1);
        SF_ASSERT(!(qe->Type & GFxAS2LoadQueueEntry::LTF_LevelFlag));

        // Make sure we have character targets and parents.
        Ptr<DisplayObject> poldChar = qe->pCharacter->ResolveCharacter(pMovieImpl);
        if (poldChar)
            stripped = (poldChar->GetResourceMovieDef()->GetSWFFlags() 
            & MovieInfo::SWF_Stripped) != 0;
    }
    else if (qe->Level != -1)
    {
        MovieRoot* as2mr = static_cast<MovieRoot*>(pMovieImpl->pASMovieRoot.GetPtr());
        SF_ASSERT(qe->Type & GFxAS2LoadQueueEntry::LTF_LevelFlag);
        if (as2mr->GetLevelMovie(qe->Level))
            stripped = ((as2mr->GetLevelMovie(qe->Level)->GetResourceMovieDef()->GetSWFFlags()
            & MovieInfo::SWF_Stripped) != 0);
        else if (as2mr->GetLevelMovie(0))
            stripped = ((as2mr->GetLevelMovie(0)->GetResourceMovieDef()->GetSWFFlags()
            & MovieInfo::SWF_Stripped) != 0);
    }
    pPreloadTask = *SF_NEW MoviePreloadTask(pMovieImpl, qe->URL, stripped, pqueueEntry->QuietOpen);
    pMovieImpl->GetTaskManager()->AddTask(pPreloadTask);
}

GFxAS2LoadQueueEntryMT_LoadMovie::~GFxAS2LoadQueueEntryMT_LoadMovie()
{
}

// Check if a movie is loaded. Returns false in the case if the movie is still being loaded.
// Returns true if the movie is loaded completely or in the case of errors.
bool GFxAS2LoadQueueEntryMT_LoadMovie::LoadFinished()
{
    bool btaskDone = pPreloadTask->IsDone();
    // If canceled, wait until task is done
    // [PPS] Change this logic to call AbandonTask and 
    // handle it appropriately inside the task to die
    // instantly
    if (pQueueEntry->Canceled && btaskDone)
        return true;

    GFxAS2LoadQueueEntry* qe = static_cast<GFxAS2LoadQueueEntry*>(pQueueEntry);
    MovieRoot*   root = ToAS2Root(pMovieImpl);
    if (!btaskDone)
    {
        return false;
    }
    // make sure that the character that we want to replace is alive.
    if (!pOldChar && qe->pCharacter)
    {
        //!AB: We need to get the character by name first time. It is kind of weird but
        // it seems that is how Flash works. For example, we have a movieclip 'mc'
        // and it already contains child 'holder'. Then we create an empty movie clip
        // within 'mc' and call it 'holder' as well. If we resolve string 'mc.holder' to
        // a character when the first (native) 'holder' will be returned. However, if we 
        // use returning value from createEmptyMovieClip we will have a pointer to the new
        // 'holder'. Now, we call loadMovie using the pointer returned from createEmptyMovieClip.
        // We expect, that movie will be loaded into newly created 'holder'. However, it loads
        // it into the native 'holder'. So, looks like the implementation of 'loadMovie' always
        // resolve character by name. Test file: Test\LoadMovie\test_loadmovie_with_two_same_named_clips.swf 
        DisplayObject* objChar = qe->pCharacter->ForceResolveCharacter(pMovieImpl);
        if (!objChar)
        {
            Sprite* pmovie0 = root->GetLevelMovie(0);
            if (pmovie0)
            {
                Environment* penv = ToAvmSprite(pmovie0)->GetASEnvironment();
                SF_ASSERT(penv);
                MovieClipLoader* pmovieClipLoader = 
                    static_cast<MovieClipLoader*>(qe->MovieClipLoaderHolder.ToObject(penv));
                if (pmovieClipLoader)
                    pmovieClipLoader->NotifyOnLoadError(penv, NULL, "Error", 0);
            }
            return true;
        }
        pOldChar = objChar->CharToInteractiveObject();
        //AB: if loading in the character then we need to assign the
        // same Id as the old character had; this is important to avoid
        // re-creation of the original character when timeline rollover 
        // of the parent character occurs.
        NewCharId = pOldChar->GetId();
    }
    // prepare env and listener object.
    Sprite* pmovie0 = root->GetLevelMovie(0);
    // Just finish loading if _level0 has already been unloaded
    if (!pmovie0)
    {
        return true;
    }

    Environment* penv0 = ToAvmSprite(pmovie0)->GetASEnvironment();
    SF_ASSERT(penv0);
    MovieClipLoader* pmovieClipLoader = 
        static_cast<MovieClipLoader*>(qe->MovieClipLoaderHolder.ToObject(penv0));
    bool extensions = penv0->CheckExtensions();
    penv0   = NULL; // can't use later, might be invalid!
    pmovie0 = NULL; // can't use later, might be invalid (if loading into _level0)

    // Preload task is finished by now. Check if a movedef is created
    MovieDefImpl* pdefImpl = pPreloadTask->GetMoiveDefImpl();
    if (!pdefImpl)
    {
        // A moviedef was not created. This means that loader couldn't load a file. 
        // Creating an empty movie and set it instead of the old one.
        if (qe->pCharacter)
        {
            InteractiveObject* pparent = pOldChar->GetParent();
            // make sure that the parent of the character that we want to
            // replace is still alive.
            if (!pparent)
                return true;
            CharacterCreateInfo ccinfo =
                pparent->GetResourceMovieDef()->GetCharacterCreateInfo
                (ResourceId(CharacterDef::CharId_EmptyMovieClip));
            Ptr<Sprite> emptyChar = *(Sprite*)root->pASSupport->CreateCharacterInstance
                (pMovieImpl, ccinfo, pparent, NewCharId, CharacterDef::Sprite);
            //AB: we need to replicate the same CreateFrame, Depth and the Name.
            // Along with replicated Id this is important to maintain the loaded
            // movie on the stage. Otherwise, once the parent clip reaches the end
            // of its timeline the original movieclip will be recreated. Flash
            // doesn't recreate the original movie clip in this case, and it also
            // doesn't stop timeline animation for the target movieclip. The only way
            // to achieve both of these features is to copy Id, CreateFrame, Depth and
            // Name from the old character to the new one.
            // See last_to_first_frame_mc_no_reloadX.fla and 
            // test_timeline_anim_after_loadmovie.swf
            emptyChar->SetCreateFrame(pOldChar->GetCreateFrame());
            emptyChar->SetDepth(pOldChar->GetDepth());
            if (!pOldChar->HasInstanceBasedName())
                emptyChar->SetName(pOldChar->GetName());
            emptyChar->AddToPlayList();
            ToAvmCharacter(pparent)->ReplaceChildCharacterOnLoad(pOldChar, emptyChar);
            pOldChar->SetParent(NULL);
            pOldChar = emptyChar;
        } 
        else if (qe->Level != -1)
        {
            pOldChar = root->GetLevelMovie(qe->Level);
        }
        if (pOldChar && pmovieClipLoader)
        {
            Environment* penv = ToAvmCharacter(pOldChar)->GetASEnvironment();
            SF_ASSERT(penv);
            pmovieClipLoader->NotifyOnLoadError(penv, pOldChar, "URLNotFound", 0);
        }
        if (qe->Level != -1)
        {
            pMovieImpl->ReleaseLevelMovie(qe->Level);
        }
        return true;
    }
    // check if the movie is already unloaded or being unloaded. This is possible when
    // loadMovie is initiated and then the container movie is removed. In this case 
    // one of the _parent of pOldChar will be a dangling pointer. Thus, if the movie
    // is unloaded already - do nothing.
    if (pOldChar && (pOldChar->IsUnloaded() || pOldChar->IsUnloading()))
    {
        return true;
    }

    if (pdefImpl->GetVersion() != ~0u && (pdefImpl->GetVersion() >= 9 && pdefImpl->GetDataDef()->GetASVersion() > 2))
    {
        pQueueEntry->Canceled = true;
        if (root->GetLogState() && !qe->QuietOpen)
        {
            root->GetLogState()->LogScriptWarning("Failed loading SWF \"%s\": ActionScript version mismatch", 
                qe->URL.ToCStr());
        }
        if (pOldChar && pmovieClipLoader)
        {
            Environment* penv = ToAvmCharacter(pOldChar)->GetASEnvironment();
            SF_ASSERT(penv);
            pmovieClipLoader->NotifyOnLoadError(penv, pOldChar, "ActionScriptMismatch", 0);
        }
        return true;
    }

    // A moviedef was created successfully.
    if (!CharSwitched)
    {
        //Replace the old character with the new sprite. This operation should be done only once
        //per loading movie.
        InteractiveObject* pparent = NULL;
        if (qe->Level != -1)
        {
            pMovieImpl->ReleaseLevelMovie(qe->Level);
            NewCharId = ResourceId(); // Assign invalid id.
        } 
        else if (qe->pCharacter)
        {
            pparent = pOldChar->GetParent();
            // make sure that the parent of the character that we want to
            // replace is still alive.
            if (!pparent)
                return true;
        }
        CharacterCreateInfo ccinfo(pdefImpl->GetDataDef(), pdefImpl);
        Ptr<Sprite> pnewChar = *static_cast<Sprite*>(root->GetASSupport()->CreateCharacterInstance(
            root->GetMovieImpl(), ccinfo, pparent, NewCharId, CharacterDef::Sprite));
        pnewChar->SetLoadedSeparately();
//                 Ptr<Sprite> pnewChar = *SF_HEAP_NEW(pMovieRoot->GetMovieHeap())
//                     Sprite(pdefImpl->GetDataDef(), pdefImpl,
//                     root, pparent, NewCharId, true);

        if (!extensions)
            pnewChar->SetNoAdvanceLocalFlag();

        if (qe->pCharacter)
        {
            pnewChar->AddToPlayList();
            //AB: we need to replicate the same CreateFrame, Depth and the Name.
            // Along with replicated Id this is important to maintain the loaded
            // movie on the stage. Otherwise, once the parent clip reaches the end
            // of its timeline the original movieclip will be recreated. Flash
            // doesn't recreate the original movie clip in this case, and it also
            // doesn't stop timeline animation for the target movieclip. The only way
            // to achieve both of these features is to copy Id, CreateFrame, Depth and
            // Name from the old character to the new one.
            // See last_to_first_frame_mc_no_reloadX.fla and 
            // test_timeline_anim_after_loadmovie.swf
            pnewChar->SetCreateFrame(pOldChar->GetCreateFrame());
            pnewChar->SetDepth(pOldChar->GetDepth());
            if (!pOldChar->HasInstanceBasedName())
                pnewChar->SetName(pOldChar->GetName());
            ToAvmCharacter(pparent)->ReplaceChildCharacter(pOldChar, pnewChar);
            pOldChar->SetParent(NULL);
        }
        else 
        {
            ToAvmSprite(pnewChar)->SetLevel(qe->Level);
            pMovieImpl->SetLevelMovie(qe->Level, pnewChar);
            // we will execute action script for frame 0 ourself here
            // this is needed because this script has to be executed 
            // before calling OnLoadInit
            G_SetFlag<MovieImpl::Flag_LevelClipsChanged>(pMovieImpl->Flags, false);
        }
        // we need to disable calling advance on this movie until its first frame is loaded
        pnewChar->SetPlayState(State_Stopped);

        pOldChar = pnewChar;
        // If we have a movie loader listener send notifications.
        if (pmovieClipLoader)
        {
            Environment* penv = ToAvmCharacter(pOldChar)->GetASEnvironment();
            SF_ASSERT(penv);
            pmovieClipLoader->NotifyOnLoadStart(penv, pOldChar);
            BytesLoaded = pdefImpl->GetBytesLoaded();
            pmovieClipLoader->NotifyOnLoadProgress(penv, pOldChar, BytesLoaded, pdefImpl->GetFileBytes());
        }
        CharSwitched = true;
    }

    // The number of loaded bytes has changed since the last iteration and we have a movie loader listener
    // sent a progress notification.
    if (BytesLoaded != pdefImpl->GetBytesLoaded() && pmovieClipLoader)
    {
        Environment* penv = ToAvmCharacter(pOldChar)->GetASEnvironment();
        SF_ASSERT(penv);
        BytesLoaded = pdefImpl->GetBytesLoaded();
        pmovieClipLoader->NotifyOnLoadProgress(penv, pOldChar, BytesLoaded, pdefImpl->GetFileBytes());
    }
    // when we loaded the first frame process its actions.
    if (extensions && !FirstFrameLoaded && 
        (pdefImpl->pBindData->GetBindStateFlags() & MovieDefImpl::BSF_Frame1Loaded) != 0)
    {
        // This branch will be executed when gfxExtensions == true, to 
        // produce different (better) behavior during the loading. If extensions
        // are on then we can start the loading movie after the first frame is loaded.
        // Flash, for some reasons, waits until whole movie is loaded.
        Ptr<Sprite> psprite;
        if (qe->Level == -1)
            psprite = pOldChar->CharToSprite();
        else
            psprite = root->GetLevelMovie(qe->Level);
        SF_ASSERT(psprite.GetPtr());
        if (psprite)
        {
            psprite->SetPlayState(State_Playing);
            psprite->SetRootNodeLoadingStat(pdefImpl->GetBytesLoaded(), pdefImpl->GetLoadingFrame());
            //psprite->OnEventLoad();
            //!AB: note, we can't use OnEventLoad here, need to use ExecuteFrame0Events. 
            // ExecuteFrame0Events executes onLoad event AFTER the first frame and Flash
            // does exactly the same.
            psprite->ExecuteFrame0Events();
            root->DoActions();
            if (pmovieClipLoader)
            {
                Environment* penv = ToAvmSprite(psprite)->GetASEnvironment();
                SF_ASSERT(penv);
                pmovieClipLoader->NotifyOnLoadInit(penv, psprite);
            }
        }
        FirstFrameLoaded = true;
    }

    bool finished = false;
    switch (pdefImpl->pBindData->GetBindStateType())
    {
    case MovieDefImpl::BS_NotStarted:
    case MovieDefImpl::BS_InProgress:
        // If the movie is not completely loaded yet try it on the next iteration.
        break;
    case MovieDefImpl::BS_Finished:
    {
        Ptr<Sprite> psprite;
        if (qe->Level == -1)
        {
            SF_ASSERT(pOldChar.GetPtr());
            psprite = pOldChar->CharToSprite();
            root->ResolveStickyVariables(pOldChar);
        }
        else
        {
            psprite = root->GetLevelMovie(qe->Level);
        }
        SF_ASSERT(psprite.GetPtr());

        if (psprite)
        {
            if (!extensions)
            {
                psprite->ClearNoAdvanceLocalFlag();
            }
            // movie is loaded completely here. Send notifications for a listener.
            if (pmovieClipLoader)
            {
                Environment* penv = ToAvmSprite(psprite)->GetASEnvironment();
                SF_ASSERT(penv);
                pmovieClipLoader->NotifyOnLoadComplete(penv, psprite, 0);
            }
            // If we did not process the actions in the first frame do it now
            if (!FirstFrameLoaded)
            {
                psprite->SetPlayState(State_Playing);
                psprite->SetRootNodeLoadingStat(pdefImpl->GetBytesLoaded(), pdefImpl->GetLoadingFrame());
                //psprite->OnEventLoad();
                //!AB: note, we can't use OnEventLoad here, need to use ExecuteFrame0Events. 
                // ExecuteFrame0Events executes onLoad event AFTER the first frame and Flash
                // does exactly the same.
                psprite->ExecuteFrame0Events();
                root->DoActions();
                if (pmovieClipLoader)
                {
                    Environment* penv = ToAvmSprite(psprite)->GetASEnvironment();
                    SF_ASSERT(penv);
                    pmovieClipLoader->NotifyOnLoadInit(penv, psprite);
                }
            }
            FirstFrameLoaded = true;
        }
        finished = true;
        break;
    } 
    case MovieDefImpl::BS_Canceled:
    case MovieDefImpl::BS_Error:
        // A error happened during the movie loading 
        if (pmovieClipLoader)
        {
            SF_ASSERT(pOldChar.GetPtr());
            Environment* penv = ToAvmCharacter(pOldChar)->GetASEnvironment();
            SF_ASSERT(penv);
            if (pdefImpl->GetLoadState() == MovieDataDef::LS_LoadError)
            {
                pmovieClipLoader->NotifyOnLoadError(penv, pOldChar, "Error", 0);
            }
            else
            {
                pmovieClipLoader->NotifyOnLoadError(penv, pOldChar, "Canceled", 0);
            }
        }
        finished = true;
        break;
    default:
        SF_ASSERT(false); // unhandled case
        finished = true;
        break;
    }

    // Preload task is not finished yet. Check again on the next iteration.
    return finished;
}

bool GFxAS2LoadQueueEntryMT_LoadMovie::IsPreloadingFinished()
{
    return pPreloadTask->IsDone();
}


#ifdef GFX_ENABLE_XML
/////////////////// load xml /////////////////
GFxAS2LoadXMLTask::GFxAS2LoadXMLTask(LoadStates* pls, const String& level0Path, const String& url, 
                                     GFxAS2LoadQueueEntry::XMLHolderType xmlholder)
                                     : Task(Task::Id_MovieDataLoad),
                                     pLoadStates(pls), Level0Path(level0Path), Url(url), pXMLLoader(xmlholder.Loader), Done(0)
{
}

void GFxAS2LoadXMLTask::Execute()
{
    URLBuilder::LocationInfo loc(URLBuilder::File_LoadXML, Url, Level0Path);
    String                   fileName;
    pLoadStates->BuildURL(&fileName, loc);

    SF_ASSERT(pXMLLoader);
    pXMLLoader->Load(fileName.ToCStr(), pLoadStates->GetFileOpener());

    AtomicOps<unsigned>::Store_Release(&Done, 1);
}

bool GFxAS2LoadXMLTask::IsDone() const 
{
    unsigned done = AtomicOps<unsigned>::Load_Acquire(&Done);
    return done == 1;
}

GFxAS2LoadQueueEntryMT_LoadXML::GFxAS2LoadQueueEntryMT_LoadXML(LoadQueueEntry* pqueueEntry, MovieRoot* pmovieRoot)
: LoadQueueEntryMT(pqueueEntry, pmovieRoot->GetMovieImpl()), pASMovieRoot(pmovieRoot)
{
    pLoadStates = *new LoadStates(pMovieImpl->pMainMovieDef->pLoaderImpl, pMovieImpl->GetStateBagImpl());
    String level0Path;
    pASMovieRoot->GetLevel0Path(&level0Path);
    pTask = *SF_NEW GFxAS2LoadXMLTask(pLoadStates, level0Path, pqueueEntry->URL, 
        static_cast<GFxAS2LoadQueueEntry*>(pqueueEntry)->XMLHolder);
    pMovieImpl->GetTaskManager()->AddTask(pTask);
}

GFxAS2LoadQueueEntryMT_LoadXML::~GFxAS2LoadQueueEntryMT_LoadXML()
{
}

// Check if a movie is loaded. Returns false in the case if the movie is still being loaded.
// Returns true if the movie is loaded completely or in the case of errors.
bool GFxAS2LoadQueueEntryMT_LoadXML::LoadFinished()
{
    bool btaskDone = pTask->IsDone();
    GFxAS2LoadQueueEntry* as2entry = static_cast<GFxAS2LoadQueueEntry*>(pQueueEntry);
    // If canceled, wait until task is done
    // [PPS] Change this logic to call AbandonTask and 
    // handle it appropriately inside the task to die
    // instantly
    if (as2entry->Canceled && btaskDone)
        return true;
    if (!btaskDone)
        return false;
    Environment* penv = pASMovieRoot->GetAvmLevelMovie(0)->GetASEnvironment();
    as2entry->XMLHolder.Loader->InitASXml(penv, as2entry->XMLHolder.ASObj.ToObject(penv));
    return true;
}
#endif


#ifdef GFX_ENABLE_CSS
/////////////////// load css /////////////////
GFxAS2LoadCSSTask::GFxAS2LoadCSSTask(LoadStates* pls, const String& level0Path, const String& url, 
                                     GFxAS2LoadQueueEntry::CSSHolderType holder)
                                     : Task(Task::Id_MovieDataLoad),
                                     pLoadStates(pls), Level0Path(level0Path), Url(url), pLoader(holder.Loader), Done(0)
{
}

void GFxAS2LoadCSSTask::Execute()
{
    URLBuilder::LocationInfo loc(URLBuilder::File_LoadCSS, Url, Level0Path);
    String                   fileName;
    pLoadStates->BuildURL(&fileName, loc);

    SF_ASSERT(pLoader);
    pLoader->Load(fileName.ToCStr(), pLoadStates->GetFileOpener());

    AtomicOps<unsigned>::Store_Release(&Done, 1);
}

bool GFxAS2LoadCSSTask::IsDone() const 
{
    unsigned done = AtomicOps<unsigned>::Load_Acquire(&Done);
    return done == 1;
}

GFxAS2LoadQueueEntryMT_LoadCSS::GFxAS2LoadQueueEntryMT_LoadCSS(LoadQueueEntry* pqueueEntry,MovieRoot* pmovieRoot)
: LoadQueueEntryMT(pqueueEntry,pmovieRoot->GetMovieImpl())
{
    pLoadStates = *new LoadStates(pMovieImpl->pMainMovieDef->pLoaderImpl, pMovieImpl->GetStateBagImpl());
    String level0Path;
    ToAS2Root(pMovieImpl)->GetLevel0Path(&level0Path);
    pTask = *SF_NEW GFxAS2LoadCSSTask(pLoadStates, level0Path, pqueueEntry->URL, 
        static_cast<GFxAS2LoadQueueEntry*>(pqueueEntry)->CSSHolder);
    pMovieImpl->GetTaskManager()->AddTask(pTask);
}

GFxAS2LoadQueueEntryMT_LoadCSS::~GFxAS2LoadQueueEntryMT_LoadCSS()
{
}

// Check if a movie is loaded. Returns false in the case if the movie is still being loaded.
// Returns true if the movie is loaded completely or in the case of errors.
bool GFxAS2LoadQueueEntryMT_LoadCSS::LoadFinished()
{
    bool btaskDone = pTask->IsDone();
    // If canceled, wait until task is done
    // [PPS] Change this logic to call AbandonTask and 
    // handle it appropriately inside the task to die
    // instantly
    if (pQueueEntry->Canceled && btaskDone)
        return true;
    if (!btaskDone)
        return false;
    Environment* penv = ToAvmSprite(ToAS2Root(pMovieImpl)->GetLevel0Movie())->GetASEnvironment();
    GFxAS2LoadQueueEntry* pqueueEntry = static_cast<GFxAS2LoadQueueEntry*>(pQueueEntry);
    pqueueEntry->CSSHolder.Loader->Init(penv, pqueueEntry->CSSHolder.ASObj.ToObject(penv));
    return true;
}
#endif

#ifdef GFX_AS_ENABLE_GC
//////////////////////////////////////////////////////////////////////////
// GASRefCountCollection
//
#ifdef SF_BUILD_DEBUG
//#define SF_TRACE_COLLECTIONS
#endif

ASRefCountCollector::ASRefCountCollector()
{
    FrameCnt            = 0;
    PeakRootCount       = 0;
    LastRootCount       = 0;
    LastCollectedRoots  = 0;
    LastPeakRootCount   = 0;
    TotalFramesCount    = 0;
    LastCollectionFrameNum = 0;

    SetParams(~0u, ~0u);
}

void ASRefCountCollector::SetParams(unsigned frameBetweenCollections, unsigned maxRootCount)
{
    // max num of roots before collection
    if (frameBetweenCollections != ~0u)
        MaxFramesBetweenCollections = frameBetweenCollections;
    else
        MaxFramesBetweenCollections = 0; // off by default

    // force collection for every N-frames
    if (maxRootCount != ~0u)
        PresetMaxRootCount  = MaxRootCount = maxRootCount;
    else
        PresetMaxRootCount  = MaxRootCount = 1000;
}

void ASRefCountCollector::AdvanceFrame(unsigned* movieFrameCnt, unsigned* movieLastCollectFrame, AmpStats* ampStats)
{
    // Is this the first advance since a collection by a different MovieView?
    if (*movieLastCollectFrame != LastCollectionFrameNum)
    {
        *movieLastCollectFrame = LastCollectionFrameNum;
        *movieFrameCnt = 1;
        return;
    }

    // Make sure we don't collect multiple times per frame
    // in the case where several views share the same GC
    if (*movieFrameCnt < FrameCnt)
    {
        // the calling MovieView is advancing the GC during a frame where
        // the GC has already been advanced more times by different MovieView
        ++(*movieFrameCnt);
        return;
    }

    unsigned curRootCount = (unsigned)GetRootsCount();
    // Do collection only every 10th frame for now
    ++TotalFramesCount;
    ++FrameCnt;
    PeakRootCount = Alg::Max(PeakRootCount, curRootCount);

    // Collection occurs if:
    // 1) if number of root exceeds currently set MaxRootCount;
    // 2) if MaxFramesBetweenCollections is set to value higher than 0 and the
    //    frame counter (FrameCnt) exceeds this value, and number of roots
    //    exceeds PresetMaxRootCount.
    if ((PresetMaxRootCount != 0 && curRootCount > MaxRootCount) || 
        (MaxFramesBetweenCollections != 0 && 
        FrameCnt >= MaxFramesBetweenCollections && 
        curRootCount > PresetMaxRootCount))
    {
        ASRefCountCollector::Stats stats(ampStats);
        Collect(&stats);

#ifdef SF_TRACE_COLLECTIONS        
        printf("Collect! Roots %d, MaxRoots %d, Peak %d, Roots collected %d, frames between %d ", 
            curRootCount, MaxRootCount, PeakRootCount, stats.RootsFreedTotal,
            TotalFramesCount - LastCollectionFrameNum);
#endif

        // If number of roots exceeds the preset max root count then we need to reset the PeakRootCount
        // in order to decrease currently set MaxRootCount.
        if (stats.RootsFreedTotal > PresetMaxRootCount)
        {
            PeakRootCount = curRootCount; // reset peak count
            MaxRootCount = PresetMaxRootCount;
        }

        // MaxRootCount has been updated every collection event
        //MaxRootCount = Alg::Max(PresetMaxRootCount, PeakRootCount - stats.RootsFreedTotal);
        if (curRootCount > stats.RootsFreedTotal)
        {
            MaxRootCount = Alg::Max(MaxRootCount, curRootCount - stats.RootsFreedTotal);
        }

        if (PeakRootCount < (unsigned)(MaxRootCount * 0.7))
            MaxRootCount = (unsigned)(MaxRootCount * 0.7);

#ifdef SF_TRACE_COLLECTIONS        
        SF_ASSERT((int)MaxRootCount >= 0);
        printf("new maxroots %d\n", MaxRootCount);
#endif

        LastCollectionFrameNum = TotalFramesCount;

        FrameCnt          = 0;
        LastPeakRootCount = PeakRootCount;
        LastCollectedRoots= stats.RootsFreedTotal;
    }
    LastRootCount = curRootCount;
    *movieFrameCnt = FrameCnt;
    *movieLastCollectFrame = LastCollectionFrameNum;
}

void ASRefCountCollector::ForceCollect(AmpStats* ampStats)
{
    unsigned curRootCount = (unsigned)GetRootsCount();

    ASRefCountCollector::Stats stats(ampStats);
    Collect(&stats);

#ifdef SF_TRACE_COLLECTIONS        
    printf("Forced collect! Roots %d, MaxRoots %d, Peak %d, Roots collected %d\n", 
        curRootCount, MaxRootCount, PeakRootCount, stats.RootsFreedTotal);
#endif
    FrameCnt  = 0;
    PeakRootCount = Alg::Max(PeakRootCount, curRootCount);
    LastRootCount = curRootCount;
}

void ASRefCountCollector::ForceEmergencyCollect(AmpStats* ampStats)
{
    ForceCollect(ampStats);

    // DO NOT shrink roots, if this was called while in adding roots in Release
    if (!IsAddingRoot())
        ShrinkRoots();

    // Reset peak and max root counters.
    PeakRootCount = 0;
    MaxRootCount  = PresetMaxRootCount;
}

#endif // SF_NO_GC

//
// ***** AvmSwfEvent
//
// For embedding event handlers in PlaceObject2Tag

void    AvmSwfEvent::Read(StreamContext* psc, UInt32 flags)
{
    SF_ASSERT(flags != 0);

    Event = EventId(flags);

    UInt32  eventLength = psc->ReadU32();

    // if Event_KeyPress, read keyCode (UI8) and decrement eventLength.
    if (Event.Id & EventId::Event_KeyPress)
    {
        Event.KeyCode = psc->ReadU8();
        --eventLength;
    }

    // Read the actions.
    // We need to allocate ActionBufferMemData from global heap, since it is possible 
    // to have a pointer to ActionBufferData after its MovieDataDef is unloaded (like, AS
    // class was registered in _global from SWF loaded by "loadMovie" and when the movie
    // is unloaded). So, we allocate all actions from global heap for now.
    pActionOpData = *ActionBufferData::CreateNew();

    // Note, the Read will copy the Actions from StreamContext to newly allocated
    // buffer from global memory heap (see comment above why).
    pActionOpData->Read(psc, eventLength);

    if (pActionOpData->GetLength() != eventLength)
    {
        if (eventLength > pActionOpData->GetLength())
        {
            // This occasionally happens, so we need to adjust
            // the stream so the remaining entries are correctly loaded.
            unsigned i, remainder = eventLength - pActionOpData->GetLength();        
            for (i = 0; i<remainder; i++)
                psc->ReadS8();

            // MA: Seems that ActionScript can actually jump outside of action buffer
            // bounds  into the binary content stored within other tags. Handling this
            // would complicate things. Ex: f1.swf 
            // For event tags this also means that data can actually be stored 
            // after ActionId == 0, hence we get this "remainder".
        }
        else
        {
            UPInt pos = psc->CurByteIndex;
            psc->CurByteIndex = pos - (pActionOpData->GetLength() - eventLength);
        }
    }
}


void    AvmSwfEvent::AttachTo(InteractiveObject* ch) const
{
    // Create a function to execute the actions.
    if (pActionOpData && !pActionOpData->IsNull())
    {                    
        AvmCharacter*       pach  = GFx::AS2::ToAvmCharacter(ch);
        Environment*        penv  = pach->GetASEnvironment();
        MemoryHeap*         pheap = penv->GetHeap();
        Ptr<ActionBuffer>   pbuff = *SF_HEAP_NEW(pheap) ActionBuffer(penv->GetSC(), pActionOpData);

        // we need to set a different type for special events, such as Initialize,
        // Construct, Load and Unload. These events behave differently when character
        // it unloading: the regular events' and action buffers' execution is terminated
        // if the event/action owner is going to be unloaded, but this is not true
        // for those events above.
        ActionBuffer::ExecuteType et;
        switch(Event.Id)
        {
        case EventId::Event_Initialize:
        case EventId::Event_Construct:
        case EventId::Event_Load:
        case EventId::Event_Unload:
            et = ActionBuffer::Exec_SpecialEvent;
            break;
        default:
            et = ActionBuffer::Exec_Event;
        }
        Value method(FunctionRef(*SF_HEAP_NEW(pheap)
            AsFunctionObject(penv, pbuff, 0, pActionOpData->GetLength(),
            0, et)));
        pach->SetClipEventHandlers(Event, method);
    }
}

/*
OnExceedLimit and OnFreeSegment are used to control MovieView's memory heap growth.
OnExceedLimit is called then the pre-set heap limit it exceeded.
OnFreeSegment is called then a chunk of memory is freed (not less than heap granularity).

User can set two parameters: UserLevelLimit (in bytes) and HeapLimitMultiplier (0..1].

The heap has initial pre-set limit 128K (so called "dynamic" limit). When this limit
is exceeded then OnExceedLimit is called. In this case it is necessary to determine:
either to collect to free up space, or to expand the heap.  The heuristic used to make the 
decision to collect or expand is taken from the Boehm-Demers-Weiser (BDW) garbage 
collector and memory allocator.
The BDW algorithm is (pseudo-code):

if (allocs since collect >= heap footprint * HeapLimitMultiplier)
collect
else
expand(heap footprint + overlimit + heap footprint * HeapLimitMultiplier)

The default value for HeapLimitMultiplier is 0.25.
Thus, it will collect only if allocated memory since the last collection is more
than 25% (default value of HeapLimitMultiplier) of the current heap size. Otherwise,
it will expand the limit up to requested size plus 25% of the heap size.

If collection occurred, it flushes all possible caches (text/paragraph formats, for example) and 
invoke garbage collector for ActionScript.

If user has specified UserLevelLimit then the algorithm above works the same way
up to that limit. If heap limit exceeds the UserLevelLimit then collection will be
invoked regardless to number of allocations since last collect. The dynamic heap limit
will be set to heap's footprint after collection + delta required for the requested allocation.

OnFreeSegment call reduces the limit by the freeing size.
*/
bool MemoryContextImpl::HeapLimit::OnExceedLimit(MemoryHeap* heap, UPInt overLimit)
{
    UPInt footprint = heap->GetFootprint();
    UPInt heapLimit = heap->GetLimit();
#ifdef SF_TRACE_COLLECTIONS        
    printf("\n! Limit is exceeded. Used: %u, footprint: %u, lim: %u, over limit: %u\n", 
        (unsigned)heap->GetUsedSpace(), (unsigned)footprint, (unsigned)heapLimit, (unsigned)overLimit);
#endif

    SPInt allocsSinceLastCollect = (SPInt)footprint - LastCollectionFootprint;
    UPInt newLimit = (heapLimit + overLimit) + (UPInt)(footprint * HeapLimitMultiplier);

    if (allocsSinceLastCollect >= (SPInt)(footprint * HeapLimitMultiplier) ||
        (UserLevelLimit != 0 && newLimit > UserLevelLimit))
    {
        if (!IsInsideAlloc())
        {
            // if called from realloc then do nothing, next allocation will take care about collection.
            Collect(heap);

            if (UserLevelLimit != 0 && newLimit > UserLevelLimit)
            {
                // check, if user limit is specified. If so, and if it is exceeded
                // then increase the limit just for absolutely required delta to minimize
                // the heap growth.
                SF_ASSERT(LastCollectionFootprint <= footprint);
                if (overLimit > (footprint - LastCollectionFootprint))
                {
                    CurrentLimit = heapLimit + (overLimit - (footprint - LastCollectionFootprint));
                    heap->SetLimit(CurrentLimit);

#ifdef SF_TRACE_COLLECTIONS        
                    printf("-        UserLimit exceeded. increasing limit up to: %u (%u)\n", 
                        (unsigned)CurrentLimit, (unsigned)heap->GetLimit());
#endif

                    CurrentLimit = heap->GetLimit(); // take an actual value of the limit
                }
                else
                {
                    // even though limit is not changed - set it to heap again to make sure
                    // the acutual heap's limit is set correctly.
                    heap->SetLimit(CurrentLimit);

#ifdef SF_TRACE_COLLECTIONS        
                    printf("-        no limit increase is necessary. Current limit is %u (%u)\n", 
                        (unsigned)CurrentLimit, (unsigned)heap->GetLimit());
#endif

                    CurrentLimit = heap->GetLimit(); // take an actual value of the limit
                }
            }
        }
    }
    else
    {
        heap->SetLimit(newLimit);

#ifdef SF_TRACE_COLLECTIONS        
        printf("-    increasing limit up to: %u (%u)\n", (unsigned)newLimit, (unsigned)heap->GetLimit());
#endif

        CurrentLimit = heap->GetLimit(); // take an actual value of the limit
    }

    return true;
}
void MemoryContextImpl::HeapLimit::OnFreeSegment(MemoryHeap* heap, UPInt freeingSize)
{
    UPInt oldLimit = CurrentLimit;
    if (oldLimit > UserLevelLimit)
    {
        if (oldLimit > freeingSize)
        {
            CurrentLimit = oldLimit - freeingSize;
            heap->SetLimit(CurrentLimit);
#ifdef SF_TRACE_COLLECTIONS        
            printf("!!   reducing limit from %u to %u (%u)\n", (unsigned)oldLimit, (unsigned)CurrentLimit, (unsigned)heap->GetLimit());
#endif
        }
    }
}
void MemoryContextImpl::HeapLimit::Collect(MemoryHeap* heap)
{
    // The collection part.
    // We may release cached text format objects from Text::Allocator...
    if (MemContext->TextAllocator)
    {
        MemContext->TextAllocator->FlushTextFormatCache(true);
        MemContext->TextAllocator->FlushParagraphFormatCache(true);
    }
#ifdef GFX_AS_ENABLE_GC
    // If garbage collection is used - force collect. Use ForceEmergencyCollect - it
    // guarantees that no new allocations will be made during the collection.
#ifdef SF_AMP_SERVER
    AmpServer& ampServer = AmpServer::GetInstance();
    MovieImpl* movie;
    // If garbage collection is used - force collect. Use ForceEmergencyCollect - it
    // guarantees that no new allocations will be made during the collection.
    if (ampServer.FindMovieByHeap(heap, &movie))
    {
        MemContext->ASGC->ForceEmergencyCollect(movie->AdvanceStats);
        movie->Release(); // addref=ed by FindMovieByHeap above
    }
    else
        MemContext->ASGC->ForceEmergencyCollect(NULL);
#else
    MemContext->ASGC->ForceEmergencyCollect(NULL);
#endif // SF_AMP_SERVER
#endif // SF_NO_GC
    LastCollectionFootprint = heap->GetFootprint(); 

#ifdef SF_TRACE_COLLECTIONS        
    printf("+    footprint after collection: %u, used mem: %u\n", 
        (unsigned)heap->GetFootprint(), (unsigned)heap->GetUsedSpace());
#endif
}
void MemoryContextImpl::HeapLimit::Reset(MemoryHeap* heap)
{
    Collect(heap);
    heap->SetLimit(INITIAL_DYNAMIC_LIMIT);   // reset to initial 128 
    CurrentLimit = heap->GetLimit(); // take an actual value of the limit
}

ASBuiltinType   EventId_GetFunctionNameBuiltinType(const EventId& evt) 
{
    // Note: There are no function names for Event_KeyPress, Event_Initialize, and 
    // Event_Construct. We use "@keyPress@", "@initialize@", and "@construct@" strings.
    static const ASBuiltinType  functionTypes[] =
    {        
        ASBuiltin_INVALID,             // Event_Invalid "INVALID"
        ASBuiltin_onLoad,              // Event_Load
        ASBuiltin_onEnterFrame,        // Event_EnterFrame
        ASBuiltin_onUnload,            // Event_Unload
        ASBuiltin_onMouseMove,         // Event_MouseMove
        ASBuiltin_onMouseDown,         // Event_MouseDown
        ASBuiltin_onMouseUp,           // Event_MouseUp
        ASBuiltin_onKeyDown,           // Event_KeyDown
        ASBuiltin_onKeyUp,             // Event_KeyUp
        ASBuiltin_onData,              // Event_Data
        ASBuiltin_ainitializea_,       // Event_Initialize "@initialize@"
        ASBuiltin_onPress,             // Event_Press
        ASBuiltin_onRelease,           // Event_Release
        ASBuiltin_onReleaseOutside,    // Event_ReleaseOutside
        ASBuiltin_onRollOver,          // Event_RollOver
        ASBuiltin_onRollOut,           // Event_RollOut
        ASBuiltin_onDragOver,          // Event_DragOver
        ASBuiltin_onDragOut,           // Event_DragOut
        ASBuiltin_akeyPressa_,         // Event_KeyPress  "@keyPress@"
        ASBuiltin_onConstruct,        // Event_Construct

        ASBuiltin_onPressAux,
        ASBuiltin_onReleaseAux,
        ASBuiltin_onReleaseOutsideAux,
        ASBuiltin_onDragOverAux,
        ASBuiltin_onDragOutAux,

        // These are for the MoveClipLoader ActionScript only
        ASBuiltin_onLoadStart,         // Event_LoadStart
        ASBuiltin_onLoadError,         // Event_LoadError
        ASBuiltin_onLoadProgress,      // Event_LoadProgress
        ASBuiltin_onLoadInit,          // Event_LoadInit
        // These are for the XMLSocket ActionScript only
        ASBuiltin_onSockClose,         // Event_SockClose
        ASBuiltin_onSockConnect,       // Event_SockConnect
        ASBuiltin_onSockData,          // Event_SockData
        ASBuiltin_onSockXML,           // Event_SockXML
        // These are for the XML ActionScript only
        ASBuiltin_onXMLLoad,           // Event_XMLLoad
        ASBuiltin_onXMLData,           // Event_XMLData

        //!AB:? GFXSTATICSTRING("onTimer"),  // setInterval GFxTimer expired
    };  
    SF_ASSERT(evt.GetEventsCount() == 1);
    unsigned idx;
    SF_COMPILER_ASSERT(EventId::Event_Invalid == 0);
    if (evt.Id <= EventId::Event_LastCombined)
        idx = Alg::BitCount32(evt.Id);
    else
        idx = (evt.Id - EventId::Event_NextAfterCombined) + 20 + 5; // PPS: 5 for the Aux events
    SF_ASSERT(idx > EventId::Event_Invalid && idx < sizeof(functionTypes)/sizeof(functionTypes[0]));
    //DBG:SF_ASSERT(Id != Event_Press);

    if (idx > EventId::Event_Invalid && idx < sizeof(functionTypes)/sizeof(functionTypes[0]))
        return functionTypes[idx];
    return ASBuiltin_unknown_;
}

ASString   EventId_GetFunctionName(StringManager *psm, const EventId& evt) 
{
    return psm->GetBuiltin(EventId_GetFunctionNameBuiltinType(evt)); 
}

}}} //SF::GFx::AS2



