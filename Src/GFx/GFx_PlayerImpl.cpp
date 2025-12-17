/**************************************************************************

Filename    :   GFx_PlayerImpl.cpp
Content     :   MovieRoot and Definition classes
Created     :   
Authors     :   Michael Antonov, Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "Kernel/SF_File.h"
#include "Render/Render_Image.h"
//#include "Render/Render_JPEGUtil.h"
#include "Kernel/SF_Alg.h"

//#include "Render/Render_Renderer.h"
#include "Kernel/SF_UTF8Util.h"
#include "GFxConfig.h"
#include "GFx/GFx_PlayerImpl.h"
#include "GFx/GFx_LoaderImpl.h"
#include "GFx/GFx_LoadProcess.h"
//#include "GFx/GFx_RenderGen.h"

#include "GFx/GFx_FontResource.h"
#include "GFx/GFx_Log.h"
#include "GFx/GFx_MorphCharacter.h"
#include "GFx/GFx_Shape.h"
#include "GFx/GFx_Stream.h"
//#include "GFx/GFx_Styles.h"
#include "GFx/GFx_DisplayList.h"

#include "GFx/GFx_ASUtils.h"

//#include "GFx/GFx_DisplayContext.h"

#include "GFx/IME/GFx_IMEManager.h"

#include "GFx/GFx_Audio.h"
#include "GFx/GFx_VideoBase.h"
#include "Kernel/SF_MsgFormat.h"

#include "GFx/GFx_PlayerStats.h"
#include "GFx/AMP/Amp_Server.h"

#include <string.h> // for memset
#include <float.h>
#include <stdlib.h>
#ifdef SF_MATH_H
#include SF_MATH_H
#else
#include <math.h>
#endif

#ifdef SF_BUILD_LOGO
#include "GFxLogo.cpp"
#endif

#ifdef GFX_GESTURE_RECOGNIZE
#include "GFx/GFx_Gesture.h"
#endif

// Increment this when the cache data format changes.
#define CACHE_FILE_VERSION  4

//#define SF_ADVANCE_EXECUTION_TRACE

#if defined(SF_BUILD_CONSUMER)

#include "GFx_Watermark.h"

struct sf_consumer 
{
    bool isWMRK;
    int days_left;
    char* licenseMessage;
};

sf_consumer GetWmrkResult();

sf_consumer sLastWmrkResult;
#endif //SF_BUILD_CONSUMER

namespace Scaleform { namespace GFx {

// ***** Statistics declarations

// MovieDef Memory.
SF_DECLARE_MEMORY_STAT_AUTOSUM_GROUP(StatMD_Mem, "MovieDef",     Stat_Mem)
SF_DECLARE_MEMORY_STAT(StatMD_CharDefs_Mem,      "CharDefs",     StatMD_Mem)
SF_DECLARE_MEMORY_STAT(StatMD_ShapeData_Mem,     "ShapeData",    StatMD_Mem)
SF_DECLARE_MEMORY_STAT(StatMD_Tags_Mem,          "Tags",         StatMD_Mem)
SF_DECLARE_MEMORY_STAT(StatMD_Fonts_Mem,         "Fonts",        StatMD_Mem)
SF_DECLARE_MEMORY_STAT(StatMD_Images_Mem,        "Images",       StatMD_Mem)
#ifdef GFX_ENABLE_SOUND
SF_DECLARE_MEMORY_STAT(StatMD_Sounds_Mem,        "Sounds",       StatMD_Mem)
#endif
SF_DECLARE_MEMORY_STAT(StatMD_ASBinaryData_Mem,  "ASBinaryData", StatMD_Mem)
SF_DECLARE_MEMORY_STAT(StatMD_Other_Mem,         "MD_Other",     StatMD_Mem)
// Load timings.
SF_DECLARE_TIMER_STAT_SUM_GROUP(StatMD_Time,     "Time",         StatGroup_Default)
SF_DECLARE_TIMER_STAT(StatMD_Load_Tks,           "Load",         StatMD_Time)
SF_DECLARE_TIMER_STAT(StatMD_Bind_Tks,           "Bind",         StatMD_Time)
      
// MovieView Memory.
SF_DECLARE_MEMORY_STAT_AUTOSUM_GROUP(StatMV_Mem, "MovieView",    Stat_Mem)
SF_DECLARE_MEMORY_STAT(StatMV_MovieClip_Mem,     "MovieClip",    StatMV_Mem)

SF_DECLARE_MEMORY_STAT_AUTOSUM_GROUP(StatMV_ActionScript_Mem,    "ActionScript", StatMV_Mem)
SF_DECLARE_MEMORY_STAT(StatMV_ASString_Mem,      "ASString",     StatMV_ActionScript_Mem)

SF_DECLARE_MEMORY_STAT(StatMV_Text_Mem,          "Text",         StatMV_Mem)
SF_DECLARE_MEMORY_STAT(StatMV_XML_Mem,           "XML",          StatMV_Mem)
SF_DECLARE_MEMORY_STAT(StatMV_Other_Mem,         "MV_Other",     StatMV_Mem)
SF_DECLARE_MEMORY_STAT_AUTOSUM_GROUP(StatMV_VM,  "VM",           StatMV_Mem)

SF_DECLARE_MEMORY_STAT(StatMV_VM_VM_Mem,            "AS3 VM",         StatMV_VM)
SF_DECLARE_MEMORY_STAT(StatMV_VM_CallFrame_Mem,     "CallFrame",      StatMV_VM)
SF_DECLARE_MEMORY_STAT(StatMV_VM_VTable_Mem,        "VTable",         StatMV_VM)
SF_DECLARE_MEMORY_STAT_AUTOSUM_GROUP(StatMV_VM_SlotInfo_Mem, "SlotInfo",    StatMV_VM)
SF_DECLARE_MEMORY_STAT(StatMV_VM_SlotInfoHash_Mem,  "SlotInfoHash",   StatMV_VM_SlotInfo_Mem)
SF_DECLARE_MEMORY_STAT(StatMV_VM_CTraits_Mem,       "ClassTraits",    StatMV_VM)
SF_DECLARE_MEMORY_STAT(StatMV_VM_Class_Mem,         "Class",          StatMV_VM)
SF_DECLARE_MEMORY_STAT(StatMV_VM_ITraits_Mem,       "InstanceTraits", StatMV_VM)
SF_DECLARE_MEMORY_STAT(StatMV_VM_Instance_Mem,      "Instance",       StatMV_VM)

SF_DECLARE_MEMORY_STAT_AUTOSUM_GROUP(StatMV_VM_AbcFile_Mem, "AbcFile",StatMV_VM)
SF_DECLARE_MEMORY_STAT(StatMV_VM_AbcConstPool_Mem,  "AbcConstPool",   StatMV_VM_AbcFile_Mem)

SF_DECLARE_MEMORY_STAT(StatMV_VM_VMAbcFile_Mem,     "VMAbcFile",      StatMV_VM)
SF_DECLARE_MEMORY_STAT(StatMV_VM_Tracer_Mem,        "Tracer",         StatMV_VM)

SF_DECLARE_MEMORY_STAT(StatIME_Mem,                 "IME",          Stat_Mem)


SF_DECLARE_MEMORY_STAT_AUTOSUM_GROUP(StatAmp_Mem,   "AMP",              Stat_Mem)
SF_DECLARE_MEMORY_STAT(StatAmp_ProfileFrame,        "Frame Profile",    StatAmp_Mem)
SF_DECLARE_MEMORY_STAT(StatAmp_Message,             "Messages",         StatAmp_Mem)
SF_DECLARE_MEMORY_STAT(StatAmp_Server,              "Server",           StatAmp_Mem)
SF_DECLARE_MEMORY_STAT(StatAmp_Callstack,           "Call stack",       StatAmp_Mem)
SF_DECLARE_MEMORY_STAT(StatAmp_InstrBuffer,         "AS Instructions",  StatAmp_Mem)

SF_DECLARE_MEMORY_STAT_AUTOSUM_GROUP(StatFC_Mem,   "FontCache",    Stat_Mem)
SF_DECLARE_MEMORY_STAT(StatFC_Batch_Mem,           "Batches",      StatFC_Mem)
SF_DECLARE_MEMORY_STAT(StatFC_GlyphCache_Mem,      "GlyphCache",   StatFC_Mem)
SF_DECLARE_MEMORY_STAT(StatFC_Other_Mem,           "FC_Other",     StatFC_Mem)


// MovieView Timings.
SF_DECLARE_TIMER_STAT_AUTOSUM_GROUP(StatMV_Tks,            "Ticks",          StatGroup_Default)
SF_DECLARE_TIMER_STAT_SUM_GROUP(    StatMV_Advance_Tks,    "Advance",        StatMV_Tks)
SF_DECLARE_TIMER_STAT_SUM_GROUP(    StatMV_Action_Tks,     "Action",         StatMV_Advance_Tks)
SF_DECLARE_TIMER_STAT(              StatMV_Seek_Tks,       "Seek",           StatMV_Action_Tks)
SF_DECLARE_TIMER_STAT(              StatMV_Timeline_Tks,   "Timeline",       StatMV_Advance_Tks)
SF_DECLARE_TIMER_STAT(              StatMV_Input_Tks,      "Input",          StatMV_Advance_Tks)
SF_DECLARE_TIMER_STAT(              StatMV_Mouse_Tks,      "Mouse",          StatMV_Input_Tks)

SF_DECLARE_TIMER_STAT_AUTOSUM_GROUP(StatMV_ScriptCommunication_Tks, "Script Communication",  StatMV_Tks)
SF_DECLARE_TIMER_STAT(              StatMV_GetVariable_Tks,"GetVariable",    StatMV_ScriptCommunication_Tks)
SF_DECLARE_TIMER_STAT(              StatMV_SetVariable_Tks,"SetVariable",    StatMV_ScriptCommunication_Tks)
SF_DECLARE_TIMER_STAT(              StatMV_Invoke_Tks,     "Invoke",         StatMV_ScriptCommunication_Tks)
SF_DECLARE_TIMER_STAT(              StatMV_InvokeAction_Tks, "InvokeAction", StatMV_Invoke_Tks)

SF_DECLARE_TIMER_STAT_SUM_GROUP(    StatMV_Display_Tks,     "Display",       StatMV_Tks)
SF_DECLARE_TIMER_STAT(              StatMV_FontThrash_Tks,  "FontThrash",    StatMV_Display_Tks)
SF_DECLARE_TIMER_STAT(              StatMV_GradientGen_Tks, "GradientGen",   StatMV_Display_Tks)
SF_DECLARE_TIMER_STAT(              StatMV_FontMiss_Tks,    "FontMiss",      StatMV_Display_Tks)

// MovieView Counters.
SF_DECLARE_COUNTER_STAT_SUM_GROUP(StatMV_Counters, "Counters",     StatGroup_Default)
SF_DECLARE_COUNTER_STAT(StatMV_Invoke_Cnt,         "Invoke",       StatMV_Counters)
SF_DECLARE_COUNTER_STAT(StatMV_MCAdvance_Cnt,      "MCAdvance",    StatMV_Counters)
SF_DECLARE_COUNTER_STAT(StatMV_Tessellate_Cnt,     "Tessellate",   StatMV_Counters)

//
// ***** ASUserData
//
#ifdef GFX_AS_ENABLE_USERDATA

void    ASUserData::SetLastObjectValue(Value::ObjectInterface* pobjIfc, void* pdata, bool isdobj)
{
    pLastObjectInterface = pobjIfc;
    pLastData = pdata;
    IsLastDispObj = isdobj;
}

bool    ASUserData::GetLastObjectValue(Value* value) const
{
    if (!pLastObjectInterface || !pLastData || !value) return false;

    return pLastObjectInterface->CreateObjectValue(value, pLastData, IsLastDispObj);
}

#endif  // GFX_AS_ENABLE_USERDATA

//
// ***** Movie
//
Movie::Movie() : pASMovieRoot(NULL)
{
}

Movie::~Movie()
{
    SF_ASSERT(!pASMovieRoot);
}

int Movie::GetAVMVersion() const
{
    return pASMovieRoot->GetAVMVersion();
}

bool Movie::GetVariable(Value *pval, const char* ppathToVar) const
{
    SF_AMP_SCOPE_TIMER_ID(pASMovieRoot->GetMovieImpl()->AdvanceStats, "Movie::GetVariable", Amp_Native_Function_Id_GetVariable);
    return pASMovieRoot->GetVariable(pval, ppathToVar);
}

bool Movie::SetVariable(const char* ppathToVar, const Value& value, SetVarType setType)
{
    SF_AMP_SCOPE_TIMER_ID(pASMovieRoot->GetMovieImpl()->AdvanceStats, "Movie::SetVariable", Amp_Native_Function_Id_SetVariable);
    return pASMovieRoot->SetVariable(ppathToVar, value, setType);
}

bool Movie::SetVariableArray(SetArrayType type, const char* ppathToVar,
                                        unsigned index, const void* pdata, unsigned count, SetVarType setType)
{
    SF_AMP_SCOPE_TIMER_ID(pASMovieRoot->GetMovieImpl()->AdvanceStats, "Movie::SetVariableArray", Amp_Native_Function_Id_SetVariableArray);
    return pASMovieRoot->SetVariableArray(type, ppathToVar, index, pdata, count, setType);
}

bool Movie::SetVariableArraySize(const char* ppathToVar, unsigned count, SetVarType setType)
{
    SF_AMP_SCOPE_TIMER_ID(pASMovieRoot->GetMovieImpl()->AdvanceStats, "Movie::SetVariableArraySize", Amp_Native_Function_Id_SetVariableArraySize);
    return pASMovieRoot->SetVariableArraySize(ppathToVar, count, setType);
}

unsigned Movie::GetVariableArraySize(const char* ppathToVar)
{
    SF_AMP_SCOPE_TIMER_ID(pASMovieRoot->GetMovieImpl()->AdvanceStats, "Movie::GetVariableArraySize", Amp_Native_Function_Id_GetVariableArraySize);
    return pASMovieRoot->GetVariableArraySize(ppathToVar);
}

bool Movie::GetVariableArray(SetArrayType type, const char* ppathToVar,
                                        unsigned index, void* pdata, unsigned count)
{
    SF_AMP_SCOPE_TIMER_ID(pASMovieRoot->GetMovieImpl()->AdvanceStats, "Movie::GetVariableArray", Amp_Native_Function_Id_GetVariableArray);
    return pASMovieRoot->GetVariableArray(type, ppathToVar, index, pdata, count);
}

bool Movie::Invoke(const char* pmethodName, Value *presult, const Value* pargs, unsigned numArgs)
{
    SF_AMP_SCOPE_TIMER_ID(pASMovieRoot->GetMovieImpl()->AdvanceStats, "Movie::Invoke", Amp_Native_Function_Id_Invoke);
    return pASMovieRoot->Invoke(pmethodName, presult, pargs, numArgs);
}

bool Movie::Invoke(const char* pmethodName, Value *presult, const char* pargFmt, ...)
{
    SF_AMP_SCOPE_TIMER_ID(pASMovieRoot->GetMovieImpl()->AdvanceStats, "Movie::Invoke", Amp_Native_Function_Id_Invoke);
    va_list args;
    bool retVal;
    va_start(args, pargFmt);
    retVal = pASMovieRoot->InvokeArgs(pmethodName, presult, pargFmt, args);
    va_end(args);
    return retVal;
}

bool Movie::InvokeArgs(const char* pmethodName, Value *presult, const char* pargFmt, va_list args)
{
    SF_AMP_SCOPE_TIMER_ID(pASMovieRoot->GetMovieImpl()->AdvanceStats, "Movie::InvokeArgs", Amp_Native_Function_Id_InvokeArgs);
    return pASMovieRoot->InvokeArgs(pmethodName, presult, pargFmt, args);
}

void Movie::CreateString(Value* pvalue, const char* pstring)
{ 
    pASMovieRoot->CreateString(pvalue, pstring); 
}
void Movie::CreateStringW(Value* pvalue, const wchar_t* pstring)
{ 
    pASMovieRoot->CreateStringW(pvalue, pstring); 
}
void Movie::CreateObject(Value* pvalue, const char* className,
                                 const Value* pargs, unsigned nargs)
{ 
    pASMovieRoot->CreateObject(pvalue, className, pargs, nargs); 
}
void Movie::CreateArray(Value* pvalue)
{ 
    pASMovieRoot->CreateArray(pvalue); 
}
void Movie::CreateFunction(Value* pvalue, FunctionHandler* pfc, void* puserData /* = NULL */)
{
    pASMovieRoot->CreateFunction(pvalue, pfc, puserData);
}

bool Movie::IsAvailable(const char* ppathToVar) const
{ 
    return pASMovieRoot->IsAvailable(ppathToVar); 
}

void Movie::SetExternalInterfaceRetVal(const Value& v)
{ 
    pASMovieRoot->SetExternalInterfaceRetVal(v); 
}

void Movie::Release()
{
    if ((AtomicOps<int>::ExchangeAdd_NoSync(&RefCount, -1) - 1) == 0)
    {
        // Since deleting the memory context releases the heap,
        // we AddRef the memory context so that it doesn't get deleted before this object
        Ptr<MemoryContext> memContext =  pASMovieRoot->GetMemoryContext();
        delete this;
        // now it's safe to delete the context
    }
}

#if defined (SF_BUILD_CONSUMER)
static bool HandleExpiredEval(MovieImpl* pImpl)
{
    sLastWmrkResult = GetWmrkResult();
    if (sLastWmrkResult.days_left >= 0)
    {
        return false;
    }

    Render::Context& RenderContext = pImpl->GetRenderContext();
    RectF VisibleFrameRect = pImpl->GetVisibleFrameRect();

    // Clear the existing render tree
    Render::TreeRoot* pRenderRoot = pImpl->GetRenderRoot();

    pRenderRoot->Remove(0, pRenderRoot->GetSize());

    if (pImpl->pWMLicenseText == NULL)
    {
        MemoryHeap* pHeap = pImpl->GetMovieHeap();

        pImpl->pWMFontProvider = *SF_HEAP_NEW(pHeap) Render::FontProviderHUD();
        if (pImpl->pWMFontProvider)
        {
            pImpl->pWMFont = *pImpl->pWMFontProvider->CreateFont("Lucida Console", 0);
        }

        pImpl->pWMLicenseText = *RenderContext.CreateEntry<TreeText>();

        TextLayout::Builder dateBuilder(pHeap);
        dateBuilder.SetBounds(RectF(0.0f, 0.0f, 1280.0f, 720.0f));
        dateBuilder.SetBackground(0, 0);
        dateBuilder.ChangeFont(pImpl->pWMFont, 13*20);
        dateBuilder.ChangeColor(0xffffffff);
        dateBuilder.AddText(sLastWmrkResult.licenseMessage);
        pImpl->pWMLicenseText->SetLayout(dateBuilder);
    }

    float width = SFstrlen(sLastWmrkResult.licenseMessage) * 6.0f; // We're using a 6x13 fixed-width font
    float x = (pImpl->mViewport.Width - width) * 10.0f;
    float y = (pImpl->mViewport.Height + 15.0f) * 10.0f;

    pImpl->pWMLicenseText->SetBounds(VisibleFrameRect);
    pImpl->pWMLicenseText->SetMatrix(Matrix2F::Translation(x, y).AppendScaling(pImpl->ViewScaleX, pImpl->ViewScaleY));

    pRenderRoot->Insert(0, pImpl->pWMLicenseText);

    RenderContext.Capture();

    return true;
}

bool MovieImpl::IsValid()
{
    return (sLastWmrkResult.days_left >= 0);
}
#else
bool MovieImpl::IsValid()
{
    return true;
}
#endif //SF_BUILD_CONSUMER

//--------------------------------------------------------------------
//
// ***** MovieImpl
//
// Global, shared root state for a GFxMovieSub and all its characters.
MovieImpl::MovieImpl(MemoryHeap* pheap)
:       
    pHeap(pheap),
    PixelScale(1.0f),
    VisibleFrameRect(0, 0, 0, 0),
    SafeRect(0, 0, 0, 0),
    pXMLObjectManager(NULL),
    BackgroundColor(0, 0, 0, 255),  
#if defined(GFX_MOUSE_SUPPORT_ENABLED) && GFX_MOUSE_SUPPORT_ENABLED != 0
    MouseCursorCount(1),
#else
    MouseCursorCount(0),
#endif
    ControllerCount(1),
    UserData(NULL),
    Flags(0),
    Flags2(0),
    RenderContext(Memory::GetGlobalHeap()),
    DIContext(0),
    pRTCommandQueue(0)
{
    pRenderRoot = *RenderContext.CreateEntry<Render::TreeRoot>();
    hDisplayRoot= MovieDisplayHandle(pRenderRoot);
    pTopMostRoot = *RenderContext.CreateEntry<Render::TreeContainer>();
    pRenderRoot->Add(pTopMostRoot);

#if defined (SF_BUILD_CONSUMER)
    pWMLicenseText = NULL;
    pWMDateText = NULL;
    pWMFont = NULL;
    pWMFontProvider = NULL;

    if (GetWmrkResult().isWMRK)
    {
        // Expiration handled by static function so customers can't be "clever" and nuke an obviously-named member function
        if (!HandleExpiredEval(this))
        {
            pWMFontProvider = *SF_HEAP_NEW(pheap) Render::FontProviderHUD();
            if (pWMFontProvider)
            {
                pWMFont = *pWMFontProvider->CreateFont("Lucida Console", 0);
            }

            pWMDateText = *RenderContext.CreateEntry<TreeText>();

            char tempBuf[256];
            SFsprintf(tempBuf, 256, SF_WATERMARK_TEXT_LINE3, GetWmrkResult().days_left);

            TextLayout::Builder dateBuilder(pheap);
            dateBuilder.SetBounds(RectF(0.0f, 0.0f, 1280.0f, 720.0f));
            dateBuilder.SetBackground(0, 0);
            dateBuilder.ChangeFont(pWMFont, 13*20);
            dateBuilder.ChangeColor(0xffffffff);
            dateBuilder.AddText(tempBuf);
            pWMDateText->SetLayout(dateBuilder);

            pRenderRoot->Insert(0, pWMDateText);
        }
    }
#endif

    // Capture to ensure that hDisplayHandle is immediately safe to pass to 
    // the render thread.
    RenderContext.Capture();

    pMainMovie  = 0;    

    G_SetFlag<Flag_NeedMouseUpdate>(Flags, true);
    G_SetFlag<Flag_MovieIsFocused>(Flags, true);
    G_SetFlag<Flag_LevelClipsChanged>(Flags, true);

    pPlayListHead = pPlayListOptHead = NULL;

    // Make sure all our allocations occur in the nested heap.
    TimeElapsed     = 0;
    TimeRemainder   = 0.0f;
    // Correct FrameTime is assigned to (1.0f / pMainMovieDef->GetFrameRate())
    // when _level0 is set/changed.
    FrameTime       = 1.0f / 12.0f;
    ForceFrameCatchUp = 0;

    // No entries in load queue.
    pLoadQueueHead  = 0;

    pLoadQueueMTHead = 0;

    // Create a delegated shared state and ensure that it has a log.
    // Delegation is configured later to Level0Def's shared state.
    pStateBag       = *SF_HEAP_NEW(pHeap) StateBagImpl(0);

#ifdef GFX_ENABLE_KEYBOARD
    for (UInt8 i = 0; i < GFX_MAX_KEYBOARD_SUPPORTED; ++i)
        KeyboardStates[i].SetKeyboardIndex(i);
#endif
    pRetValHolder = NULL;
    //pRetValHolder       =  SF_HEAP_NEW(pHeap) ReturnValueHolder(&StringManager);

    pFontManagerStates  = *SF_HEAP_NEW(pHeap) FontManagerStates(pStateBag);

    InstanceNameCount  = 0;

    // Viewport: un-initialized by default.
    ViewScaleX = 1.0f;
    ViewScaleY = 1.0f;
    ViewOffsetX = ViewOffsetY = 0;
    ViewScaleMode = SM_ShowAll;
    ViewAlignment = Align_Center;

    // Focus
    FocusGroupsCnt          = 1;
    memset(FocusGroupIndexes, 0, sizeof(FocusGroupIndexes));

    LastIntervalTimerId = 0;

    pIMECandidateListStyle  = NULL;

    StartTickMs = Timer::GetTicks()/1000;
    PauseTickMs = 0;

    SafeRect.Clear();

#ifdef GFX_ENABLE_SOUND
    pAudio = NULL;
    pSoundRenderer = NULL;
#endif

    // Initialized within VM-specific MovieRoot constructor.
    pObjectInterface = 0;

    LastLoadQueueEntryCnt   = 0;
    pUnloadListHead         = NULL;

    bool ampEnabled = false;
    SF_AMP_CODE(ampEnabled = AmpServer::GetInstance().IsEnabled();)
    if (ampEnabled)
    {
        SF_AMP_CODE(AdvanceStats = *SF_HEAP_AUTO_NEW(&AmpServer::GetInstance()) AMP::ViewStats();)
        SF_AMP_CODE(AmpServer::GetInstance().AddMovie(this);)
    }
    else
    {
#ifdef SF_ENABLE_STATS
        AdvanceStats = *SF_HEAP_NEW(pHeap) AMP::ViewStats();
#endif
    }
    MultitouchMode = MTI_None;

    FocusRectChanged = true;
    PreviouslyCaptured = 0;
#if defined(GFX_CHECK_PLAYLIST_DEBUG)
    _PLCheckCnt = 0;
#endif
}

MovieImpl::~MovieImpl()
{
    if (FocusRectContainerNode)
    {
        FocusRectContainerNode->Remove(0, FocusRectContainerNode->GetSize());
        for (unsigned i = 0; i < FocusGroupsCnt; ++i)
        {
            FocusGroups[i].FocusRectNode = NULL;
        }
        FocusRectContainerNode = NULL;
    }

    ClearDrawingContextList();
    SF_AMP_CODE(AmpServer::GetInstance().RemoveMovie(this);)

    ProcessUnloadQueue();

    pRenderRoot = NULL;

#ifndef SF_NO_IME_SUPPORT
    // clean up IME manager
    Ptr<IMEManagerBase> pIMEManager = GetIMEManager();
    if (pIMEManager)
    {
        if (pIMEManager->IsMovieActive(this))
            pIMEManager->ClearActiveMovie();
    }
    delete pIMECandidateListStyle;
#endif //#ifdef SF_NO_IME_SUPPORT
    pFontManagerStates = NULL;

    ShutdownTimers();

#ifdef GFX_ENABLE_VIDEO
    VideoProviders.Clear();
#endif

#ifdef GFX_ENABLE_SOUND
    InteractiveObject* psprite = GetMainContainer();
    if (psprite)
        psprite->StopActiveSounds();
#endif

    InvalidateOptAdvanceList();
    
    CheckPlaylistConsistency();

    ClearDisplayList();
    ClearIndirectTransformPairs();

    ClearStickyVariables();

    delete pRetValHolder;

    // clean up threaded load queue before freeing
    LoadQueueEntryMT* plq = pLoadQueueMTHead;
    unsigned plqCount = 0;
    while (plq)
    {
        plqCount++;
        plq->Cancel();
        plq = plq->pNext;
    }
    // wait until all threaded loaders exit
    // this is required to avoid losing the movieroot
    // heap before the load tasks complete.
    unsigned plqDoneCount = 0;
    while (plqCount > plqDoneCount)
    {
        plqDoneCount = 0;
        plq = pLoadQueueMTHead;
        while (plq)
        {
            if (plq->LoadFinished())
                plqDoneCount++;
            plq = plq->pNext;
        }
    }

    // free load queue
    while(pLoadQueueHead)
    {
        // Remove from queue.
        LoadQueueEntry *pentry = pLoadQueueHead;
        pLoadQueueHead = pentry->pNext;
        delete pentry;
    }
    while(pLoadQueueMTHead)
    {
        LoadQueueEntryMT *pentry = pLoadQueueMTHead;
        pLoadQueueMTHead = pentry->pNext;
        delete pentry;
    }
    
#if defined(SF_BUILD_DEBUG) && !defined(GFX_AS_ENABLE_GFXVALUE_CLEANUP)
    //
    // Check if there are GFx::Values still holding refs to AS objects.
    // Warn the developer if there are any because this condition
    // will lead to crash where the Value dtor, etc. will be
    // accessing freed memory.
    //
    if ( pObjectInterface && !(pObjectInterface->ExternalObjRefs.IsEmpty()) )
    {
        SF_DEBUG_MESSAGE(1, "** Begin Tagged GFx::Values Dump **\n");
        const Value* data = pObjectInterface->ExternalObjRefs.GetFirst();
        while (!pObjectInterface->ExternalObjRefs.IsNull(data))
        {
            const char* ptypestr = NULL;
            String instName = "";
            switch (data->GetType())
            {
            case Value::VT_Array: ptypestr = "Array"; break;
            case Value::VT_DisplayObject: 
                {
                    ptypestr = "DispObj"; 
                    Value name;
                    data->GetMember("name", &name);
                    instName = name.GetString();
                    break;
                }
            default: ptypestr = "Object";
            }
            SF_DEBUG_MESSAGE3(1, "> [%s] : %p %s\n", ptypestr, data, instName.ToCStr());
            data = pObjectInterface->ExternalObjRefs.GetNext(data);
        }
        SF_DEBUG_MESSAGE(1, "** End Tagged GFx::Values Dump **\n");
        SF_DEBUG_MESSAGE(1, "\n");
        SF_DEBUG_ERROR(1, "There are GFx::Values still holding references to AS Objects. If not released\n"
                           "before the MovieImpl that owns the AS objects dies, then a crash will occur\n"
                           "due to the Value trying refer to freed memory. (When the MovieImpl dies,\n"
                           "it drops the heap that contains all AS objects.) Make sure all GFx::Values that\n"
                           "hold AS object references are released before MovieImpl dies.\n");
        SF_ASSERT(0);
    }
#endif

#if defined(GFX_AS_ENABLE_GFXVALUE_CLEANUP)
    // 
    // If there are GFx::Values still holding refs to AS objects then
    // explicitly set them to undefined and set their orphaned flags
    // to true.
    //
    if ( pObjectInterface && !(pObjectInterface->ExternalObjRefs.IsEmpty()) )
    {
        Value* data = pObjectInterface->ExternalObjRefs.GetFirst();
        while (!pObjectInterface->ExternalObjRefs.IsNull(data))
        {
            data->SetUndefined();
            data->Type = Value::ValueType(data->Type | Value::VTC_OrphanedBit);
            data = pObjectInterface->ExternalObjRefs.GetNext(data);
        }        
    }
#endif
    TopmostLevelCharacters.Clear();

    delete pObjectInterface;

    pASMovieRoot->Shutdown();
    pASMovieRoot = NULL;
    pTopMostRoot = NULL;

    // Note: render context shutdown must come after all movie shutdown, as BitmapData objects may be
    // destroyed at any point. These may push more commands on the ThreadCommandQueue, which will need 
    // to be processed before shutdown on the render thread occurs.
    RenderContext.Shutdown(true);

#if defined (SF_BUILD_CONSUMER)
    if (GetWmrkResult().isWMRK)
    {
        pWMDateText = NULL;
        pWMLicenseText = NULL;
        pWMFont = NULL;
        pWMFontProvider = NULL;
    }
#endif

     hDisplayRoot.Clear();
}

void MovieImpl::ClearPlayList()
{
    // clear playlist
    for (InteractiveObject* cur = pPlayListHead, *next; cur; cur = next)
    {
        next = cur->pPlayNext;
        cur->pPlayNext = cur->pPlayPrev = cur->pPlayNextOpt = cur->pPlayPrevOpt = NULL;
    }
    pPlayListHead = pPlayListOptHead = NULL;
}

void MovieImpl::SetASMovieRoot(ASMovieRootBase* pasmgr)
{
    SF_ASSERT(!pASMovieRoot); // set only once!
    SF_ASSERT(pasmgr);        // not NULL
    pASMovieRoot = pSavedASMovieRoot = pasmgr;
}

// Fills in a file system path relative to _level0.
bool MovieImpl::GetMainMoviePath(String *ppath) const
{       
    if (!pMainMovie)
    {
        ppath->Clear();
        return false;
    }

    // Get URL.
    *ppath = pMainMovieDef->GetFileURL();

    // Extract path by clipping off file name.
    if (!URLBuilder::ExtractFilePath(ppath))
    {
        ppath->Clear();
        return false;
    }
    return true;
}


#ifdef SF_BUILD_DEBUG
//#define SF_TRACE_COLLECTIONS
#endif


// *** Load/Unload movie support

// ****************************************************************************
// Adds load queue entry and takes ownership of it.
//
void    MovieImpl::AddLoadQueueEntry(LoadQueueEntry *pentry)
{
    pentry->EntryTime = ++LastLoadQueueEntryCnt;
    if (!pLoadQueueHead)
    {
        pLoadQueueHead = pentry;
    }
    else
    {
        // Find tail.
        LoadQueueEntry *ptail = pLoadQueueHead;
        while (ptail->pNext)    
            ptail = ptail->pNext;   

        // Insert at tail.
        ptail->pNext = pentry;
    }   
}

void    MovieImpl::AddLoadQueueEntryMT(LoadQueueEntryMT* pentryMT)
{
    pentryMT->pQueueEntry->EntryTime = ++LastLoadQueueEntryCnt;
    if (!pLoadQueueMTHead)
    {
        pLoadQueueMTHead = pentryMT;
    }
    else
    {
        // Find tail.
        LoadQueueEntryMT *ptail = pLoadQueueMTHead;
        while (ptail->GetNext())    
            ptail = ptail->GetNext();   

        // Insert at tail.
        ptail->pNext = pentryMT;
        pentryMT->pPrev = ptail;
    }   
}


MovieDefImpl*  MovieImpl::CreateImageMovieDef(
                        ImageResource *pimageResource, bool bilinear,
                        const char *purl, LoadStates *pls)
{   
    MovieDefImpl*    pdefImpl = 0; 
    Ptr<LoadStates> plsRef;

    // If load states were not passed, create them. This is necessary if
    // we are being called from sprite's attachBitmap call.
    if (!pls)
    {
        plsRef = *SF_NEW LoadStates(GetLoaderImpl(), pStateBag);
        pls = plsRef.GetPtr();
    }

    // Create a loaded image sprite based on imageInfo.
    if (pimageResource)
    {
        // No file opener here, since it was not used (ok since file openers are
        // used as a key only anyway). Technically this is not necessary since
        // image 'DataDefs' are not stored in the library right now, but the
        // argument must be passed for consistency.
        ResourceKey createKey = MovieDataDef::CreateMovieFileKey(purl, 0, 0, 0);
        MemoryHeap* pheap       = GetMovieHeap();

        // Create a MovieDataDef containing our image (and an internal corresponding ShapeDef).
        Ptr<MovieDataDef> pimageMovieDataDef =
            *MovieDataDef::Create(createKey, MovieDataDef::MT_Image, purl, pheap);

        if (!pimageMovieDataDef)
            return NULL;        
        if (!pimageMovieDataDef->InitImageFileMovieDef(0, pimageResource, GetImageCreator(), pls->GetLog(), bilinear))
            return NULL;

        // We can alter load states since they are custom for out caller.
        // (ProcessLoadQueue creates a copy of LoadStates)
        pls->SetRelativePathForDataDef(pimageMovieDataDef);

        // TBD: Should we use pStateBag or its delegate??       
        pdefImpl = SF_HEAP_NEW(pheap) MovieDefImpl(pimageMovieDataDef,
                                                    pls->GetBindStates(), pls->pLoaderImpl,
                                                    Loader::LoadAll,
                                                    pStateBag->GetDelegate(),
                                                    pheap, true); // Images are fully loaded.
    }
    return pdefImpl;
}


// Processes the load queue handling load/unload instructions.  
void    MovieImpl::ProcessLoadQueue()
{
    SF_AMP_SCOPE_TIMER(AdvanceStats, "MovieImpl::ProcessLoadQueue", Amp_Profile_Level_Medium);

    // Only currently existing entries are processed
    // This allows the opportunity for events to fire when a load adds another load to the queue
    UInt32 lastLoadQueueEntryCnt = LastLoadQueueEntryCnt;
    while (pLoadQueueHead != NULL && pLoadQueueHead->EntryTime <= lastLoadQueueEntryCnt)
    {
        // Remove from queue.
        LoadQueueEntry *pentry = pLoadQueueHead;
        pLoadQueueHead = pentry->pNext;

        // Capture load states - constructor copies their values from pStateBag;
        // this means that states used for 'loadMovie' can be overridden per movie view.
        // This means that state inheritance chain (Loader -> MovieDefImpl -> MovieImpl)
        // takes effect here, so setting new states in any of above can have effect
        // on loadMovie calls. Note that this is different from import logic, which
        // always uses exactly same states as importee. Such distinction is by design.
        Ptr<LoadStates>  pls = *new LoadStates(GetLoaderImpl(), pStateBag);

        pASMovieRoot->ProcessLoadQueueEntry(pentry, pls);

        delete pentry;
    }

    LoadQueueEntryMT* pentry = pLoadQueueMTHead;
    // First we need to make sure that our queue does not have any tasks with 
    // unfinished preloading task (which usually is just searching and opening a file). 
    // We need this check to avoid a race condition like this: let say we load 
    // A.swf into _level0 and B.swf into _level1 and preloading for A.swf takes more 
    // time then for B.swf . In this case B.swf will be loaded first and then unloaded 
    // by loading A.swf into _level0. This happens because we unload a level just after preloading
    // has finished and unloading of _level0 unloads all everything in the swf movie
    while(pentry)
    {
        if (!pentry->IsPreloadingFinished())
            return;
        pentry = pentry->pNext;
    }
    // Check status of all movies that are loading through the TaskManager if we have one.
    pentry = pLoadQueueMTHead;
    while(pentry)
    {
        if (pentry->LoadFinished()) 
        {
            LoadQueueEntryMT* next = pentry->pNext;
            if (next)
                next->pPrev = pentry->pPrev;
            if (pentry->pPrev)
                pentry->pPrev->pNext = next;
            if (pLoadQueueMTHead == pentry) 
                pLoadQueueMTHead = next;
            delete pentry;
            pentry = next;
        } 
        else 
        {
            pentry = pentry->pNext;
        }
    }
}


// *** Drag State support


void MovieImpl::DragState::InitCenterDelta(bool lockCenter, unsigned mouseIndex)
{
    LockCenter = lockCenter;
    MouseIndex = mouseIndex;

    if (!LockCenter)
    {
        typedef Matrix2F   Matrix;                     

        // We are not centering on the object, so record relative delta         
        Matrix          parentWorldMat;
        InteractiveObject*   pchar = pCharacter;
        if (pchar->GetParent())
            parentWorldMat = pchar->GetParent()->GetWorldMatrix();

        // Mouse location
        const MouseState* pmouseState = pchar->GetMovieImpl()->GetMouseState(mouseIndex);
        SF_ASSERT(pmouseState);
        PointF worldMouse(pmouseState->GetLastPosition());

        PointF parentMouse;
        // Transform to parent coordinates [world -> parent]
        parentWorldMat.TransformByInverse(&parentMouse, worldMouse);

        // Calculate the relative mouse offset, so that we can apply adjustment
        // before bounds checks.
        Matrix  local = pchar->GetMatrix();
        CenterDelta.x = local.Tx() - parentMouse.x;
        CenterDelta.y = local.Ty() - parentMouse.y;
    }
}


// *** MovieImpl's Movie interface implementation


void    MovieImpl::SetViewport(const Viewport& viewDesc)
{
    if (memcmp(&viewDesc, &mViewport, sizeof(mViewport)) == 0)
        return;

    int   prevLeft = mViewport.Left;
    int   prevTop  = mViewport.Top;
    int   prevWidth = mViewport.Width;
    int   prevHeight = mViewport.Height;
    float prevScale = mViewport.Scale;
    float prevRatio = mViewport.AspectRatio;

    G_SetFlag<Flag_ViewportSet>(Flags, true);
    mViewport        = viewDesc;

    if (mViewport.BufferHeight <= 0 || mViewport.BufferWidth <= 0)
        SF_DEBUG_WARNING(1, "MovieImpl::SetViewport: buffer size not set");

    RectF prevVisRect = VisibleFrameRect;
    UpdateViewport();

    if (prevVisRect != VisibleFrameRect ||
        (ViewScaleMode == SM_NoScale && 
            (prevWidth != mViewport.Width || prevHeight != mViewport.Height || 
             prevLeft  != mViewport.Left  || prevTop    != mViewport.Top ||
             prevScale != mViewport.Scale || prevRatio != mViewport.AspectRatio)) ||
        (ViewScaleMode != SM_ExactFit && 
            (prevWidth != mViewport.Width || 
             prevHeight != mViewport.Height || prevRatio != mViewport.AspectRatio)))
    {
        pASMovieRoot->NotifyOnResize();
    }
    pRenderRoot->SetViewport(mViewport);
    pRenderRoot->SetBackgroundColor(BackgroundColor);

    // MOOSE TODO, 3D doesn't use this, it would be better if this was part of renderHal Viewport matrix
    pRenderRoot->SetMatrix(ViewportMatrix);

#if defined (SF_BUILD_CONSUMER)
    if (GetWmrkResult().isWMRK)
    {
        // The * 10.0f is a bit odd, but it's to avoid doing a / 2.0f followed by a * 20.0f.
        if(pWMDateText)
        {
            float width = SFstrlen(SF_WATERMARK_TEXT_LINE3) * 6.0f; // We're using a 6x13 fixed-width font
            pWMDateText->SetBounds(VisibleFrameRect);
            pWMDateText->SetMatrix(Matrix2F::Translation((mViewport.Width - width) * 10.0f, (mViewport.Height + 15.0f) * 10.0f).AppendScaling(ViewScaleX, ViewScaleY));
        }
    }
#endif
}

void  MovieImpl::SetBackgroundColor(const Color color)      
{ 
    BackgroundColor = color; 
    pRenderRoot->SetBackgroundColor(BackgroundColor);
}

void  MovieImpl::SetBackgroundAlpha(float alpha)             
{ 
    BackgroundColor.SetAlpha( Alg::Clamp<UByte>((UByte)(alpha*255.0f), 0, 255) ); 
    pRenderRoot->SetBackgroundColor(BackgroundColor);
}

void    MovieImpl::UpdateViewport()
{
    RectF prevVisibleFrameRect = VisibleFrameRect;
    float prevViewOffsetX       = ViewOffsetX;
    float prevViewOffsetY       = ViewOffsetY;
    float prevViewScaleX        = ViewScaleX;
    float prevViewScaleY        = ViewScaleY;
    float prevPixelScale        = PixelScale;

    // calculate frame rect to be displayed and update    
    if (pMainMovieDef)
    {
        // We need to calculate VisibleFrameRect depending on scaleMode and alignment. Alignment is significant only for NoScale mode; 
        // otherwise it is ignored. 
        // Renderer will use VisibleFrameRect to inscribe it into Viewport's rectangle. VisibleFrameRect coordinates are in twips,
        // Viewport's coordinates are in pixels. For simplest case, for ExactFit mode VisibleFrameRect is equal to FrameSize, thus,
        // viewport is filled by the whole scene (the original aspect ratio is ignored in this case, the aspect ratio of viewport
        // is used instead).
        // ViewOffsetX and ViewOffsetY is calculated in stage pixels (not viewport's ones).

        // mViewport rectangle recalculated to twips.
        RectF viewportRect(PixelsToTwips(float(mViewport.Left)), 
            PixelsToTwips(float(mViewport.Top)), 
            PixelsToTwips(float(mViewport.Left + mViewport.Width)), 
            PixelsToTwips(float(mViewport.Top + mViewport.Height)));
        const float viewWidth = viewportRect.Width();
        const float viewHeight = viewportRect.Height();
        const float frameWidth = pMainMovieDef->GetFrameRectInTwips().Width();
        const float frameHeight = pMainMovieDef->GetFrameRectInTwips().Height();
        switch(ViewScaleMode)
        {
        case SM_NoScale:
            {
                // apply mViewport.Scale and .AspectRatio first
                const float scaledViewWidth  = viewWidth * mViewport.AspectRatio * mViewport.Scale;
                const float scaledViewHieght = viewHeight * mViewport.Scale;

                // calculate a VisibleFrameRect
                switch(ViewAlignment)
                {
                case Align_Center:
                    // We need to round centering to pixels in noScale mode, otherwise we lose pixel center alignment.
                    VisibleFrameRect.x1 = (float)PixelsToTwips((int)TwipsToPixels(frameWidth/2 - scaledViewWidth/2));
                    VisibleFrameRect.y1  = (float)PixelsToTwips((int)TwipsToPixels(frameHeight/2 - scaledViewHieght/2));
                    break;
                case Align_TopLeft:
                    VisibleFrameRect.x1 = 0;
                    VisibleFrameRect.y1  = 0;
                    break;
                case Align_TopCenter:
                    VisibleFrameRect.x1 = (float)PixelsToTwips((int)TwipsToPixels(frameWidth/2 - scaledViewWidth/2));
                    VisibleFrameRect.y1  = 0;
                    break;
                case Align_TopRight:
                    VisibleFrameRect.x1 = frameWidth - scaledViewWidth;
                    VisibleFrameRect.y1  = 0;
                    break;
                case Align_BottomLeft:
                    VisibleFrameRect.x1 = 0;
                    VisibleFrameRect.y1  = frameHeight - scaledViewHieght;
                    break;
                case Align_BottomCenter:
                    VisibleFrameRect.x1 = (float)PixelsToTwips((int)TwipsToPixels(frameWidth/2 - scaledViewWidth/2));
                    VisibleFrameRect.y1  = frameHeight - scaledViewHieght;
                    break;
                case Align_BottomRight:
                    VisibleFrameRect.x1 = frameWidth - scaledViewWidth;
                    VisibleFrameRect.y1  = frameHeight - scaledViewHieght;
                    break;
                case Align_CenterLeft:
                    VisibleFrameRect.x1 = 0;
                    VisibleFrameRect.y1  = (float)PixelsToTwips((int)TwipsToPixels(frameHeight/2 - scaledViewHieght/2));
                    break;
                case Align_CenterRight:
                    VisibleFrameRect.x1 = frameWidth - scaledViewWidth;
                    VisibleFrameRect.y1  = (float)PixelsToTwips((int)TwipsToPixels(frameHeight/2 - scaledViewHieght/2));
                    break;
                }
                VisibleFrameRect.SetWidth(scaledViewWidth);
                VisibleFrameRect.SetHeight(scaledViewHieght);
                ViewOffsetX = TwipsToPixels(VisibleFrameRect.x1);
                ViewOffsetY = TwipsToPixels(VisibleFrameRect.y1);
                ViewScaleX = mViewport.AspectRatio * mViewport.Scale;
                ViewScaleY = mViewport.Scale;
                break;
            }
        case SM_ShowAll: 
        case SM_NoBorder: 
            {
                const float viewWidthWithRatio = viewWidth * mViewport.AspectRatio;
                // For ShowAll and NoBorder we need to apply AspectRatio to viewWidth in order
                // to calculate correct VisibleFrameRect and scales.
                // Scale is ignored for these modes.
                if ((ViewScaleMode == SM_ShowAll && viewWidthWithRatio/frameWidth < viewHeight/frameHeight) ||
                    (ViewScaleMode == SM_NoBorder && viewWidthWithRatio/frameWidth > viewHeight/frameHeight))
                {
                    float visibleHeight = frameWidth * viewHeight / viewWidthWithRatio;    
                    VisibleFrameRect.x1 = 0;
                    VisibleFrameRect.y1 = frameHeight/2 - visibleHeight/2;
                    VisibleFrameRect.SetWidth(frameWidth);
                    VisibleFrameRect.SetHeight(visibleHeight);

                    ViewOffsetX = 0;
                    ViewOffsetY = TwipsToPixels(VisibleFrameRect.y1);
                    ViewScaleX = viewWidth ? (frameWidth / viewWidth) : 0.0f;
                    ViewScaleY = ViewScaleX / mViewport.AspectRatio;
                }
                else 
                {
                    float visibleWidth = frameHeight * viewWidthWithRatio / viewHeight;    
                    VisibleFrameRect.x1 = frameWidth/2 - visibleWidth/2;
                    VisibleFrameRect.y1 = 0;
                    VisibleFrameRect.SetWidth(visibleWidth);
                    VisibleFrameRect.SetHeight(frameHeight);

                    ViewOffsetX = TwipsToPixels(VisibleFrameRect.x1);
                    ViewOffsetY = 0;
                    ViewScaleY = viewHeight ? (frameHeight / viewHeight) : 0.0f;
                    ViewScaleX = ViewScaleY * mViewport.AspectRatio;
                }
            }
            break;
        case SM_ExactFit: 
            // AspectRatio and Scale is ignored for this mode.
            VisibleFrameRect.x1 = VisibleFrameRect.y1 = 0;
            VisibleFrameRect.SetWidth(frameWidth);
            VisibleFrameRect.SetHeight(frameHeight);
            ViewOffsetX = ViewOffsetY = 0;
            ViewScaleX = viewWidth  ? (VisibleFrameRect.Width() / viewWidth) : 0.0f;
            ViewScaleY = viewHeight ? (VisibleFrameRect.Height() / viewHeight) : 0.0f;
            break;
        }
        PixelScale = Alg::Max((ViewScaleX == 0) ? 0.005f : 1.0f/ViewScaleX, 
                           (ViewScaleY == 0) ? 0.005f : 1.0f/ViewScaleY);    
    }
    else
    {
        ViewOffsetX = ViewOffsetY = 0;
        ViewScaleX = ViewScaleY = 1.0f;
        PixelScale = SF_TWIPS_TO_PIXELS(20.0f);
    }
    //  RendererInfo.ViewportScale = PixelScale;

    ResetViewportMatrix();

    if (prevVisibleFrameRect != VisibleFrameRect || prevViewOffsetX != ViewOffsetX ||
        prevViewOffsetY      != ViewOffsetY      || prevViewScaleX  != ViewScaleX  ||
        prevViewScaleY       != ViewScaleY       || prevPixelScale  != PixelScale)
    {
        //SetDirtyFlag();
        UpdateViewAndPerspective();
    }
}

void MovieImpl::ResetViewportMatrix()
{
    ViewportMatrix = Render::Matrix2F::Translation(-VisibleFrameRect.x1, -VisibleFrameRect.y1);
    ViewportMatrix.AppendScaling(mViewport.Width / VisibleFrameRect.Width(),
                                 mViewport.Height / VisibleFrameRect.Height());
}

void MovieImpl::RestoreViewport()
{
    ResetViewportMatrix();
    pRenderRoot->SetMatrix(ViewportMatrix);
}

void MovieImpl::MakeViewAndPersp3D(Matrix3F *matView, Matrix4F *matPersp, 
                                   const RectF &frameRect,           // ex. width and height, in TWIPS, of the visible frame rect
                                   const PointF &projCenter,                // ex. perspective center of projection, in TWIPS
                                   float fieldOfView,                       // perspective field of view, in degrees, default flash FOV is 55
                                   float focalLength,                       // this is normally computed automatically, but the computed value can be overridden here 
                                   bool bInvertY)                           // if the Y axis should be inverted
{
    float displayWidth = (float)(fabs(frameRect.Width()));
    const float nearZ = focalLength ? focalLength / 100.0f : 1.0f;
    const float farZ  = focalLength ? focalLength * 100.0f : Alg::Max(displayWidth, 100000.0f);

    float r = frameRect.x2 - projCenter.x;
    float l = -(projCenter.x - frameRect.x1);

    // invert y
    float b = -(frameRect.y2 - projCenter.y);    
    float t = projCenter.y - frameRect.y1;


    // calculate view matrix
    float fovYAngle = (float)SF_DEGTORAD(fieldOfView);
    float pixelPerUnitRatio = displayWidth*.5f;    
    float eyeZ;
    if (focalLength)
        eyeZ = focalLength;
    else
        eyeZ = (fieldOfView <= 0) ? pixelPerUnitRatio : (pixelPerUnitRatio/tanf(fovYAngle/2.f));

    // compute view matrix
    if (matView)
    {
        Point3F vEyePt( projCenter.x, projCenter.y, (-eyeZ < -farZ) ? -farZ : -eyeZ);
        Point3F vLookatPt(projCenter.x, projCenter.y, 0);
        Point3F vUpVec( 0, bInvertY ? 1.f : -1.f, 0 );
        if (bInvertY)
            matView->ViewLH(vEyePt, vLookatPt, vUpVec );
        else
            matView->ViewRH(vEyePt, vLookatPt, vUpVec );
    }

    // compute perspective matrix    
    if (matPersp)
    {
        if (fieldOfView <= 0)
        {
            // make orthographic projection
            if (bInvertY)
                matPersp->OrthoOffCenterLH(l, r, b, t, nearZ, farZ);
            else
                matPersp->OrthoOffCenterRH(l, r, b, t, nearZ, farZ);
        }
        else
        {
            if (bInvertY) 
                matPersp->PerspectiveOffCenterLH(eyeZ, l, r, b, t, nearZ, farZ);
            else
                matPersp->PerspectiveOffCenterRH(eyeZ, l, r, b, t, nearZ, farZ);
        }
    }
}

// recompute view and perspective if necessary (usually when visible frame rect changes)
void MovieImpl::UpdateViewAndPerspective()         
{ 
    if (!VisibleFrameRect.IsNull())
    {
        for (unsigned movieIndex = 0; movieIndex < MovieLevels.GetSize(); movieIndex++) 
        {
            InteractiveObject* pmovie = MovieLevels[movieIndex].pSprite;
            if (pmovie)
                pmovie->UpdateViewAndPerspective();
        }
    }
}

// 3D view matrix for movie, get from root
void MovieImpl::GetViewMatrix3D(Matrix3F *viewMat) 
{ 
    DisplayObjContainer *proot = GetRootMovie();
    if (proot)
        proot->GetViewMatrix3D(viewMat);
}

// 3D view matrix for movie, set on root
void MovieImpl::SetViewMatrix3D(const Matrix3F& viewMat)
{
    DisplayObjContainer *proot = GetRootMovie();
    if (proot)
        proot->SetViewMatrix3D(viewMat);
}

// 3D projection matrix for movie, get from root
void MovieImpl::GetProjectionMatrix3D(Matrix4F* projMat)
{
    DisplayObjContainer *proot = GetRootMovie();
    if (proot)
        proot->GetProjectionMatrix3D(projMat);
}

// 3D projection matrix for movie, set on root
void MovieImpl::SetProjectionMatrix3D(const Matrix4F& projMat)
{
    DisplayObjContainer *proot = GetRootMovie();
    if (proot)
        proot->SetProjectionMatrix3D(projMat);
}

void    MovieImpl::GetViewport(Viewport *pviewDesc) const
{
    *pviewDesc = mViewport;
}

void    MovieImpl::SetViewScaleMode(ScaleModeType scaleMode)
{
    if (pUserEventHandler)
    {
        pUserEventHandler->HandleEvent(this, ClippingEvent(scaleMode == SM_NoScale ? Event::DisableClipping : Event::EnableClipping));
    }
    ViewScaleMode = scaleMode;
    Viewport v(mViewport);
    ++mViewport.Flags; // force viewport to be re-set
    SetViewport(v);
}

void    MovieImpl::SetViewAlignment(AlignType align)
{
    ViewAlignment = align;
    Viewport v(mViewport);
    ++mViewport.Flags; // force viewport to be re-set
    SetViewport(v);
}

InteractiveObject* MovieImpl::GetTopMostEntity(const PointF& mousePos, 
                                               unsigned controllerIdx, 
                                               bool testAll, 
                                               const InteractiveObject* ignoreMC)
{
    SF_AMP_SCOPE_TIMER(AdvanceStats, "MovieImpl::GetTopMostEntity", Amp_Profile_Level_Medium);

    //GetRenderContext().PropagateChangesUp();

    SetNormalizedScreenCoords(mousePos);
    Matrix4F proj;
    GetProjectionMatrix3D(&proj);
    ScreenToWorld.SetProjection(proj);
    Matrix3F view;
    GetViewMatrix3D(&view);
    ScreenToWorld.SetView(view);

    InteractiveObject* ptopMouseCharacter = 0;
    // look for chars marked as topmostLevel first.
    for (int i = (int)TopmostLevelCharacters.GetSize() - 1; i >= 0; --i)
    {
        DisplayObjectBase* pch = TopmostLevelCharacters[i];
        SF_ASSERT(pch);
        if (!pch->GetParent())
            continue;

        Matrix2F matrix(pch->GetParent()->GetWorldMatrix());
        PointF pp = matrix.TransformByInverse(mousePos);

        // need to test with 'testAll' = true first to detect any overlappings
        DisplayObjectBase::TopMostDescr descr;
        descr.pIgnoreMC = ignoreMC;
        descr.TestAll   = testAll;
        descr.ControllerIdx = controllerIdx;
        if (pch->GetTopMostMouseEntity(pp, &descr) == DisplayObjectBase::TopMost_Found)
        {
            SF_ASSERT(descr.pResult);
            ptopMouseCharacter = descr.pResult;
            break;
        }
    }
    if (!ptopMouseCharacter)
    {        
        for (int movieIndex = (int)MovieLevels.GetSize(); movieIndex > 0; movieIndex--)
        {   
            InteractiveObject* pmovie  = MovieLevels[movieIndex-1].pSprite;
            DisplayObjectBase::TopMostDescr descr;
            descr.pIgnoreMC = ignoreMC;
            descr.TestAll   = testAll;
            descr.ControllerIdx = controllerIdx;
            if (pmovie->GetTopMostMouseEntity(mousePos, &descr) == DisplayObjectBase::TopMost_Found)
            {
                SF_ASSERT(descr.pResult);
                ptopMouseCharacter = descr.pResult;
                break;
            }
        }
    }
    return ptopMouseCharacter;
}

template<typename T>
struct PtrGuard
{
    PtrGuard(T** ptr) : Ptr(ptr) {}
    ~PtrGuard() { if(*Ptr)  {  (*Ptr)->Release();  *Ptr = NULL; } }
    T** Ptr;
};

float MovieImpl::Advance(float deltaT, unsigned frameCatchUpCount, bool capture)
{    
#if defined(SF_ENABLE_STATS)
    SF_AMP_SCOPE_TIMER_ID(AdvanceStats, "MovieImpl::Advance", Amp_Native_Function_Id_Advance);
#endif

#if defined (SF_BUILD_CONSUMER)    
    if (HandleExpiredEval(this))
    {
        return 1.0f / 60.0f;
    }
#endif //SF_BUILD_CONSUMER

    if (G_IsFlagSet<Flag_Paused>(Flags))
    {   // Call capture anyway, since Invoke could've changed things.
        if (capture)
            MovieImpl::Capture();
        return 0.05f;
    }

    if (!pMainMovie)
    {   // Call capture to clear out content in case Invoke caused movie unload.
        if (capture)
            MovieImpl::Capture();
        return pMainMovieDef ? (1.0f / pMainMovieDef->GetFrameRate()) : 0.0f;
    }
    // DBG
    //printf("MovieImpl::Advance %d   -------------------\n", pMainMovie->GetLevelMovie(0)->ToSprite()->GetCurrentFrame());

    ProcessMovieDefToKillList();

    // Need to restore high precision mode of FPU for X86 CPUs.
    // Direct3D may set the Mantissa Precision Control Bits to 24-bit (by default 53-bits) and this 
    // leads to bad precision of FP arithmetic. For example, the result of 0.0123456789 + 1.0 will 
    // be 1.0123456789 with 53-bit mantissa mode and 1.012345671653 with 24-bit mode.
    DoublePrecisionGuard dpg;

    // Index used in iterating through root movies.
    unsigned   movieIndex;

    if (deltaT < 0.0f)
    {
        SF_DEBUG_WARNING(1, "Movie::Advance - negative deltaT argument specified");
        deltaT = 0.0f;
    }

    G_SetFlag<Flag_AdvanceCalled>(Flags, true);

    // Advance - Cache states for faster non-locked access.
    Ptr<ActionControl>           pac;

    State*                        pstates[10]    = {0,0,0,0,0,0,0,0,0,0};
    const static State::StateType stateQuery[10] =
    { State::State_UserEventHandler, State::State_FSCommandHandler,
      State::State_ExternalInterface, State::State_ActionControl,
      State::State_Log, State::State_FontLib, State::State_FontMap,
      State::State_FontProvider, State::State_Translator,
      State::State_Audio // Not obtained if no sound.
    };
    // Get multiple states at once to avoid extra locking.
#ifdef GFX_ENABLE_SOUND   
    pStateBag->GetStatesAddRef(pstates, stateQuery, 10);
#else
    pStateBag->GetStatesAddRef(pstates, stateQuery, 9);
#endif
  
    Ptr<LogState> logState;

    pUserEventHandler   = *(UserEventHandler*)  pstates[0];
    pFSCommandHandler   = *(FSCommandHandler*)  pstates[1];
    pExtIntfHandler     = *(ExternalInterface*) pstates[2];
    pac                 = *(ActionControl*)     pstates[3];
    logState            = *(LogState*)          pstates[4];
    pCachedLog = logState ? logState->GetLog() : 0;

    // Check if FontLib, FontMap, FontProvider or Translator is changed
    UInt8 stateChangeFlag = 0u;
    {
        Ptr<FontLib>      fl = *(FontLib*)            pstates[5];
        Ptr<FontMap>      fm = *(FontMap*)            pstates[6];
        Ptr<FontProvider> fp = *(FontProvider*)       pstates[7];
        Ptr<Translator>   tr = *(Translator*)         pstates[8];
        stateChangeFlag = pFontManagerStates->CheckStateChange(fl,fm,fp,tr);
    }

    // Set flag indicating that the log is cached.
    G_SetFlag<Flag_CachedLogFlag>(Flags, true);
    
    if (pac)
    {
        G_SetFlag<Flag_VerboseAction>(Flags, (pac->GetActionFlags() & ActionControl::Action_Verbose) != 0);
        G_SetFlag<Flag_SuppressActionErrors>(Flags, (pac->GetActionFlags() & ActionControl::Action_ErrorSuppress) != 0);
        G_SetFlag<Flag_LogRootFilenames>(Flags, (pac->GetActionFlags() & ActionControl::Action_LogRootFilenames) != 0);
        G_SetFlag<Flag_LogLongFilenames>(Flags, (pac->GetActionFlags() & ActionControl::Action_LongFilenames) != 0);
        G_SetFlag<Flag_LogChildFilenames>(Flags, (pac->GetActionFlags() & ActionControl::Action_LogChildFilenames) != 0);
    }
    else
    {
        G_SetFlag<Flag_VerboseAction>(Flags, 0);
        G_SetFlag<Flag_SuppressActionErrors>(Flags, 0);
        G_SetFlag<Flag_LogRootFilenames>(Flags, 0);
        G_SetFlag<Flag_LogLongFilenames>(Flags, 0);
        G_SetFlag<Flag_LogChildFilenames>(Flags, 0);
    }

#ifdef GFX_ENABLE_SOUND
    pAudio = (AudioBase*) pstates[9];
    PtrGuard<AudioBase> as_ptr(&pAudio);
    if (pAudio)
    {
        pSoundRenderer = pAudio->GetRenderer();
        if(pSoundRenderer)
            pSoundRenderer->AddRef();
    }
    PtrGuard<Sound::SoundRenderer> sr_ptr(&pSoundRenderer);
#endif    
    // Capture frames loaded so that the value of a loaded frame will be consistent
    // for all root nodes that share the same MovieDefImpl. This ensures that if
    // there 'loadMovie' is used several times on the same progressively loaded SWF,
    // all instances will see the same progress state.

    MovieDefRootNode *pdefNode = RootMovieDefNodes.GetFirst();
    while(!RootMovieDefNodes.IsNull(pdefNode))
    {        
        // Bytes loaded must always be grabbed before the frame, since there is a data
        // race between value returned 'getBytesLoaded' and the frame we can seek to.
        if (!pdefNode->ImportFlag)
        {
            pdefNode->BytesLoaded  = pdefNode->pDefImpl->GetBytesLoaded();
            pdefNode->LoadingFrame = pdefNode->pDefImpl->GetLoadingFrame();
        }

        // if FontLib, FontMap, FontProvider or Translator is changed we need to 
        // to clean FontManager cache
        if (stateChangeFlag)
            pdefNode->pFontManager->CleanCache();

        pdefNode = pdefNode->pNext;
    }

	if (pMainMovieDef->GetLoadingFrame() == 0)
	{
		// Main movie first frame not loaded yet. We are not ready to advance
		return 0;
	}

    // Notify TextField that states changed
    if (stateChangeFlag || AreRegisteredFontsChanged())
    {
        for (unsigned i = 0; i < MovieLevels.GetSize(); i++)
            MovieLevels[i].pSprite->SetStateChangeFlags(stateChangeFlag);
    }

    // Shadowed value from GFxSprite::GetLoadingFrame would have been grabbed in
    // the RootMovieDefNodes traversal above, so it must come after it.
    if ((pMainMovie->GetLoadingFrame() == 0))
    {
        if (capture)
            MovieImpl::Capture();
        G_SetFlag<Flag_CachedLogFlag>(Flags, 0);
        return 0;
    }

    // Execute loading/frame0 events for root level clips, if necessary.
    if (G_IsFlagSet<Flag_LevelClipsChanged>(Flags) && (pMainMovie->GetLoadingFrame() > 0))
    {
        G_SetFlag<Flag_LevelClipsChanged>(Flags, false);      
        // Queue up load events, tags and actions for frame 0.
        for (movieIndex = (unsigned)MovieLevels.GetSize(); movieIndex > 0; movieIndex--)
        {
            // ExecuteFrame0Events does an internal check, so this will only have an effect once.
            MovieLevels[movieIndex-1].pSprite->ExecuteFrame0Events();
        }

        // It is important to execute frame 0 events here before we get to AdvanceFrame() below,
        // so that actions like stop() are applied in timely manner.
        pASMovieRoot->DoActions();
        ProcessUnloadQueue();
        ProcessLoadQueue();
    }

    // *** Advance the frame based on time

    TimeElapsed += UInt64(deltaT * 1000000.f);
    TimeRemainder += deltaT;    

//if (TimeRemainder >= FrameTime)
//    MovieLevels[0].pSprite->DumpDisplayList(0, "_LEVEL");

    UInt64 advanceStart = Timer::GetProfileTicks();
    float minDelta = FrameTime;

    if (IntervalTimers.GetSize() > 0)
    {
        UInt64 minDeltaMicroSecs = static_cast<UInt64>(minDelta * 1000000.f);
        UPInt i, n;
        unsigned needCompress = 0;
        for (i = 0, n = IntervalTimers.GetSize(); i < n; ++i)
        {
            if (IntervalTimers[i] && IntervalTimers[i]->IsActive())
            {
                IntervalTimers[i]->Invoke(this, FrameTime);

                UInt64 delta = IntervalTimers[i]->GetNextInvokeTime() - TimeElapsed;
                if (delta < minDeltaMicroSecs)
                {
                    minDeltaMicroSecs = delta;
                }
            }
            else
            {
                ++needCompress;
            }
        }
        if (needCompress)
        {
            n = IntervalTimers.GetSize(); // size could be changed after Invoke
            unsigned j;
            // remove empty entries
            for (i = j = 0; i < n; ++i)
            {
                if (!IntervalTimers[j] || !IntervalTimers[j]->IsActive())
                {
                    IntervalTimers[j]->Clear();
                    IntervalTimers.RemoveAt(j);
                }
                else
                    ++j;
            }
        }
        minDelta = minDeltaMicroSecs * 1000000.f;
    }
#ifdef GFX_ENABLE_VIDEO
    if (VideoProviders.GetSize() > 0)
    {
        Array<Video::VideoProvider*> remove_list;
        for(HashSet<Ptr<Video::VideoProvider> >::Iterator it  = VideoProviders.Begin();
                                                          it != VideoProviders.End(); ++it)
        {
            SF_AMP_SCOPE_TIMER(AdvanceStats, "VideoProviderNetStream::Advance", Amp_Profile_Level_Medium);
            Ptr<Video::VideoProvider> pvideo = *it;
            pvideo->Advance();
            if (pvideo->IsActive())
            {
                if( pvideo->GetFrameTime() < minDelta)
                    minDelta = pvideo->GetFrameTime();
            }
            else {
                pvideo->Close();
                remove_list.PushBack(pvideo);
            }
        }
        for(UPInt i = 0; i < remove_list.GetSize(); ++i)
			RemoveVideoProvider(remove_list[i]);
    }
#endif
#ifdef GFX_ENABLE_SOUND
    if (pSoundRenderer)
    {
        float delta = pSoundRenderer->Update();
        if (delta < minDelta)
            minDelta = delta;
    }
#endif

    ProcessInput();
   
    if (TimeRemainder >= FrameTime)
    {
        // Mouse, keyboard and timer handlers above may queue up actions which MUST be executed before the
        // Advance. For example, if a mouse event contains "gotoAndPlay(n)" action and Advance is
        // going to queue up a frame with "stop" action, then that gotoAndPlay will be incorrectly stopped.
        // By executing actions before the Advance we avoid queuing the stop-frame and gotoAndPlay will
        // work correctly. (AB, 12/13/06)
        pASMovieRoot->DoActions();

        bool frameCatchUp = frameCatchUpCount > 0 || ForceFrameCatchUp > 0;
        do
        {
            if (frameCatchUp)
                TimeRemainder -= FrameTime;
            else
                TimeRemainder = (float)fmod(TimeRemainder, FrameTime);

            float framePos = (TimeRemainder >= FrameTime) ? 0.0f : (TimeRemainder / FrameTime);

            // Advance a frame.
            {
                AdvanceFrame(1, framePos);
#ifdef GFX_ENABLE_VIDEO
                // We need to call advance for video players here also because when we synchronize
                // Flash timeline and video frames we need to notify video players that our the timer
                // has changed.
                if (VideoProviders.GetSize() > 0)
                {
                    Array<Video::VideoProvider*> remove_list;
                    for(HashSet<Ptr<Video::VideoProvider> >::Iterator it  = VideoProviders.Begin();
                                                                      it != VideoProviders.End(); ++it)
                    {
                        Ptr<Video::VideoProvider> pvideo = *it;
                        pvideo->Advance();
                        if (pvideo->IsActive())
                        {
                            if( pvideo->GetFrameTime() < minDelta)
                                minDelta = pvideo->GetFrameTime();
                        }
                        else {
                            pvideo->Close();
                            remove_list.PushBack(pvideo);
                        }
                    }
                    for(UPInt i = 0; i < remove_list.GetSize(); ++i)
						RemoveVideoProvider(remove_list[i]);
                }
#endif
            }

            // Execute actions queued up due to actions and mouse.
            pASMovieRoot->DoActions();
            ProcessUnloadQueue();
            ProcessLoadQueue();
            if (ForceFrameCatchUp > 0)
                ForceFrameCatchUp--;

        } while((frameCatchUpCount-- > 0 && TimeRemainder >= FrameTime) || ForceFrameCatchUp > 0);
        // Force GetTopmostMouse update in next Advance so that buttons detect change, if any.
        G_SetFlag<Flag_NeedMouseUpdate>(Flags, true);

        // Let refcount collector to do its job
        pASMovieRoot->AdvanceFrame(true);
    }
    else
    {
        // Fractional advance only.
        float framePos = TimeRemainder / FrameTime;
        AdvanceFrame(0, framePos);

        TimeRemainder = (float)fmod(TimeRemainder, FrameTime);

        // Technically AdvanceFrame(0) above should not generate any actions.
        // However, we need to execute actions queued up due to mouse.
        pASMovieRoot->DoActions();
        ProcessUnloadQueue();
        ProcessLoadQueue();
        pASMovieRoot->AdvanceFrame(false);
    }    

	UpdateAllRenderNodes();

    ResetTabableArrays();
    G_SetFlag<Flag_CachedLogFlag>(Flags, false);

    SF_AMP_CODE(AmpServer::GetInstance().MovieAdvance(this);)

    // DBG
    //printf("MovieImpl::Advance %d   ^^^^^^^^^^^^^^^^^\n", pMainMovie->GetLevelMovie(0)->ToSprite()->GetCurrentFrame());
    UInt64 advanceStop = Timer::GetProfileTicks();
    minDelta -= (advanceStop - advanceStart)/1000000.0f;
    if (minDelta < 0.0f)
        minDelta = 0.0f;

    if (capture)
        MovieImpl::Capture();
    return Alg::Min(minDelta, FrameTime - TimeRemainder);
}

void MovieImpl::Capture(bool onChangeOnly)
{
    // need to capture when onChangeOnly is false, or there are changes, or kill list is not empty: 
    // the latter case is here because kill list relies on Context.GetFinalizedFrameId() that won't be
    // updated if Capture is not called (AB).
    if (!onChangeOnly || RenderContext.HasChanges() || MovieDefKillList.GetSize() != 0)
    {
        // process indirect transform nodes if any
        if (IndirectTransformPairs.GetSize())
        {
            for (UPInt i = 0, n = IndirectTransformPairs.GetSize(); i < n; ++i)
            {
                const IndirectTransPair& p = IndirectTransformPairs[i];
                SF_ASSERT(p.Obj->HasIndirectTransform());

                // first of all, determine if any node was changed (incl the node with
                // indirect transform itself, all of its old parents and all of its
                // new parents).
                TreeNode* node = p.Obj->GetRenderNode();
                SF_ASSERT(node);

                // Change bits, which may cause matrix recalculation for 
                // the mask node.
                static const unsigned CHBITS = Render::Change_Matrix | 
                                               Render::Change_State_MaskNode | 
                                               Render::Change_IsMask |
                                               Render::Change_ChildInsert | // to track parent's replacement
                                               (unsigned)Render::Change_Context_NewNode; // to track new parent node

                bool changed = node->HasChanges(CHBITS);
                if (!changed)
                {
                    // check all old ("transform") parents ("mask" branch).
                    TreeNode* cur;
                    int depth = 0;
                    for (cur = p.TransformParent; cur && cur != pRenderRoot; cur = cur->GetParent(), ++depth)
                    {
                        if (cur->HasChanges(CHBITS))
                        {
                            changed = true;
                            break;
                        }
                    }
                    // If depth is not the same then this may mean that the indirect transform
                    // node was detached from its parent: need to recalculate the matrix and
                    // update depth.
                    if (depth != p.OrigParentDepth)
                    {
                        changed = true;
                        p.OrigParentDepth = depth;
                    }
                    else if (!changed) // still not changed?
                    {
                        // check all new parents ("maskee" branch)
                        for (cur = node->GetParent() ; cur && cur != pRenderRoot; cur = cur->GetParent())
                        {
                            if (cur->HasChanges(CHBITS))
                            {
                                changed = true;
                                break;
                            }
                        }
                        // if one of the parent null then we don't need to do anything, since
                        // object is invisible. change is 'false' here in this case.
                    }
                }

                if (changed)
                {
                    //printf("R"); //@DBG
                    // gather reverse matrix from TranformParent back to treeroot
                    const DisplayObjectBase::IndirectTransformDataType* ixf = 
                        p.Obj->GetIndirectTransformData();
                    Matrix3F revm(ixf->OrigTransformMatrix);
                    bool _3d = ixf->IsOrig3D;
                    for (TreeNode* cur = p.TransformParent; cur && cur != pRenderRoot; cur = cur->GetParent())
                    {
                        if (cur->Is3D())
                        {
                            revm.Append(cur->M3D());
                            _3d = true;
                        }
                        else
                            revm.Append(cur->M2D());
                    }

                    Matrix3F wm;
                    for (TreeNode* cur = node->GetParent() ; cur && cur != pRenderRoot; cur = cur->GetParent())
                    {
                        if (cur->Is3D())
                        {
                            wm.Append(cur->M3D());
                            _3d = true;
                        }
                        else
                            wm.Append(cur->M2D());
                    }
                    if (_3d)
                    {
                        wm.Invert();
                        wm.Prepend(revm);
                        node->SetMatrix3D(wm);
                    }
                    else
                    {
                        Matrix2F& revm2 = *(Matrix2F*)(void*)&revm;
                        Matrix2F& wm2 = *(Matrix2F*)(void*)&wm;
                        wm2.Invert();
                        wm2.Prepend(revm2);
                        node->SetMatrix(wm2);
                    }
                }
            }
        }

        G_SetFlag<Flag_HasChanges>(Flags, 
            (G_IsFlagSet<Flag_HasChanges>(Flags) || RenderContext.HasChanges()));

        // Suspend GC while Capture is in action. Capture may cause some allocations (like
        // reformatting text in PropagateUp) that may lead to OnExceedLimit, that may lead to
        // crash, if any display object is freed (due to instable RenderContext).
        pASMovieRoot->SuspendGC(true);
        RenderContext.Capture();
        pASMovieRoot->SuspendGC(false);

        PreviouslyCaptured = 1;
        // For some reasons I can't process moviedefkillist right after Capture.
        // Needs investigation.
        //ProcessMovieDefToKillList();
    }
    else if (PreviouslyCaptured > 0)
    {
        // Suspend GC while Capture is in action. Capture may cause some allocations (like
        // reformatting text in PropagateUp) that may lead to OnExceedLimit, that may lead to
        // crash, if any display object is freed (due to instable RenderContext).
        pASMovieRoot->SuspendGC(true);
        RenderContext.Capture();
        pASMovieRoot->SuspendGC(false);

        --PreviouslyCaptured;
    }
}

void MovieImpl::ProcessInput()
{
    SF_AMP_SCOPE_TIMER_ID(AdvanceStats, "MovieImpl::ProcessInput", Amp_Native_Function_Id_ProcessInput);

#if defined(GFX_ENABLE_KEYBOARD) || defined(GFX_ENABLE_MOUSE)
    // *** Handle keyboard / mice
    if (pMainMovie) 
    {
        // mouse event handling is different between AVM 1 and 2: for AS3 (avm2)
        // we need to call GetTopMostEntity with the parameter "testAll" set to true, thus,
        // it will test all objects, not only ones having button events.
        bool avm2 = (pASMovieRoot->GetAVMVersion() == 2);
        SF_UNUSED(avm2);

        ProcessFocusKeyInfo focusKeyInfo;
        UInt32              miceProceededMask = 0;
        UInt32              miceSupportedMask = ((1U << MouseCursorCount) - 1);

        while (!InputEventsQueue.IsQueueEmpty())
        {
            const InputEventsQueue::QueueEntry* qe = InputEventsQueue.GetEntry();
            if (qe->IsKeyEntry())
            {
                ProcessKeyboard(qe, &focusKeyInfo);
            }
            else if (qe->IsMouseEntry())
            {
                ProcessMouse(qe, &miceProceededMask, avm2);
            }
#ifdef GFX_ENABLE_ANALOG_GAMEPAD
            else if (qe->IsAnalogGampadEntry())
            {
                ProcessGamePadAnalog(qe);
            }
#endif
#ifdef GFX_MULTITOUCH_SUPPORT_ENABLE
            else if (qe->IsTouchEntry())
            {
                ProcessTouch(qe, &focusKeyInfo);
            }
            else if (qe->IsGestureEntry())
            {
                ProcessGesture(qe);
            }
#endif
			else if (qe->IsStatusEntry())
			{
				ProcessStatus(qe);
			}
			else if (qe->IsAccelerometerEntry())
			{
				ProcessAccelerometer(qe);
			}
			else if (qe->IsGeolocationEntry())
			{
				ProcessGeolocation(qe);
			}
        }

        if (G_IsFlagSet<Flag_NeedMouseUpdate>(Flags) && (miceProceededMask & miceSupportedMask) != miceSupportedMask)
        {
            for (unsigned mi = 0, mask = 1; mi < MouseCursorCount; ++mi, mask <<= 1)
            {
                if (!(miceProceededMask & mask))
                {
                    if (!mMouseState[mi].IsActivated())
                        continue; // this mouse is not activated yet
                    mMouseState[mi].ResetPrevButtonsState();
                    Ptr<InteractiveObject> ptopMouseCharacter = 
                        GetTopMostEntity(mMouseState[mi].GetLastPosition(), mi, avm2);
                    mMouseState[mi].SetTopmostEntity(ptopMouseCharacter);    

                    // check for necessity of changing cursor
                    CheckMouseCursorType(mi, ptopMouseCharacter);

                    pASMovieRoot->GenerateMouseEvents(mi);
                }
            }
        }

        FinalizeProcessFocusKey(&focusKeyInfo);
        G_SetFlag<Flag_NeedMouseUpdate>(Flags, false);
    }
#endif // GFX_ENABLE_KEYBOARD || GFX_ENABLE_MOUSE
}

#ifdef GFX_ENABLE_ANALOG_GAMEPAD
void    MovieImpl::ProcessGamePadAnalog(const InputEventsQueue::QueueEntry* qe)
{
    SF_AMP_SCOPE_TIMER(AdvanceStats, "MovieImpl::ProcessGamePadAnalog", Amp_Profile_Level_Medium);

    const InputEventsQueue::QueueEntry::GamePadAnalogEntry& gpEntry = qe->GetGamePadAnalogEntry();
    // analog gamepad controls
    GamePadAnalogEventId eventId(gpEntry.Code, gpEntry.ControllerIndex, gpEntry.AnalogX, gpEntry.AnalogY);

    pASMovieRoot->NotifyGamePadAnalogEvent(&eventId);
}
#endif  // GFX_ENABLE_ANALOG_GAMEPAD

void MovieImpl::ProcessStatus(const InputEventsQueue::QueueEntry* qe)
{
    SF_AMP_SCOPE_TIMER(AdvanceStats, "MovieImpl::ProcessStatus", Amp_Profile_Level_Medium);

    const InputEventsQueue::QueueEntry::StatusEntry& sEntry = qe->GetStatusEntry();
    
	StatusEventId eventId(sEntry.Code, sEntry.Level, sEntry.ExtensionId, sEntry.ContextId);

    pASMovieRoot->NotifyStatusEvent(&eventId);
}


void MovieImpl::ProcessAccelerometer(const InputEventsQueue::QueueEntry* qe)
{
    SF_AMP_SCOPE_TIMER(AdvanceStats, "MovieImpl::ProcessAccelerometer", Amp_Profile_Level_Medium);

    const InputEventsQueue::QueueEntry::AccelerometerEntry& aEntry = qe->GetAccelerometerEntry();
    
	AccelerometerEventId eventId(aEntry.IdAcc, aEntry.Timestamp, aEntry.AccelerationX, aEntry.AccelerationY, aEntry.AccelerationZ);

    pASMovieRoot->NotifyAccelerometerEvent(&eventId, aEntry.IdAcc);
}


void MovieImpl::ProcessGeolocation(const InputEventsQueue::QueueEntry* qe)
{
    SF_AMP_SCOPE_TIMER(AdvanceStats, "MovieImpl::ProcessGeolocation", Amp_Profile_Level_Medium);

    const InputEventsQueue::QueueEntry::GeolocationEntry& gEntry = qe->GetGeolocationEntry();

	/* Global Geolocation Status Event */
	//if (gEntry.IdGeo < 0)
		//pASMovieRoot->NotifyGeolocationStatusEvent(gEntry.IdGeo);

	GeolocationEventId eventId(gEntry.IdGeo, gEntry.Latitude, gEntry.Longitude, gEntry.Altitude, gEntry.HAccuracy, gEntry.VAccuracy, gEntry.Speed, gEntry.Heading, gEntry.Timestamp);

    pASMovieRoot->NotifyGeolocationEvent(&eventId, gEntry.IdGeo);
}

void MovieImpl::ProcessKeyboard(const InputEventsQueue::QueueEntry* qe, 
                                ProcessFocusKeyInfo* focusKeyInfo)
{
#ifdef GFX_ENABLE_KEYBOARD

    SF_AMP_SCOPE_TIMER(AdvanceStats, "MovieImpl::ProcessKeyboard", Amp_Profile_Level_Medium);

    int                 keyMask = 0;
    const InputEventsQueue::QueueEntry::KeyEntry& keyEntry = qe->GetKeyEntry();
    // keyboard
    if (keyEntry.Code != 0)
    {
        // key down / up
        EventId::IdCode eventIdCode;
        Event::EventType event;
        if (keyEntry.KeyIsDown)
        {
            eventIdCode = EventId::Event_KeyDown;
            event       = Event::KeyDown;
        }
        else
        {
            eventIdCode = EventId::Event_KeyUp;
            event       = Event::KeyUp;
        }
        EventId eventId
            ((UByte)eventIdCode, 
            keyEntry.Code, 
            keyEntry.AsciiCode, 
            keyEntry.WcharCode, keyEntry.KeyboardIndex);
        eventId.KeysState = keyEntry.KeysState;
        if (!keyEntry.AsciiCode)
            eventId.AsciiCode = eventId.ConvertKeyCodeToAscii();

        //printf ("Key %d (ASCII %d) is %s\n", code, 
        // asciiCode, (event == Event::KeyDown) ? "Down":"Up");

        for (unsigned movieIndex = (unsigned)MovieLevels.GetSize(); movieIndex > 0; movieIndex--)
            MovieLevels[movieIndex-1].pSprite->PropagateKeyEvent(eventId, &keyMask);

        const KeyboardState* keyboardState = GetKeyboardState(keyEntry.KeyboardIndex);
        SF_ASSERT(keyboardState);
        keyboardState->NotifyListeners(pMainMovie, eventId, keyMask);

        if (!IsDisableFocusKeys())
            ProcessFocusKey(event, keyEntry, focusKeyInfo);
    }
    else if (keyEntry.WcharCode != 0)
    {
        // char
        FocusGroupDescr& focusGroup = GetFocusGroup(keyEntry.KeyboardIndex);
        Ptr<InteractiveObject> curFocused = focusGroup.LastFocused;
        if (curFocused)
        {
            curFocused->OnCharEvent(keyEntry.WcharCode, keyEntry.KeyboardIndex);
        }
    }
#else
    SF_UNUSED2(qe, focusKeyInfo);
#endif //SF_NO_KEYBOARD_SUPPORT

}

void MovieImpl::ProcessMouse(const InputEventsQueue::QueueEntry* qe, UInt32* miceProceededMask, bool avm2)
{
    SF_AMP_SCOPE_TIMER_ID(AdvanceStats, "MovieImpl::ProcessMouse", Amp_Native_Function_Id_ProcessMouse);

    // Proceed mouse
    *miceProceededMask |= (1 << qe->GetMouseEntry().MouseIndex);

    unsigned mi = qe->GetMouseEntry().MouseIndex;
    mMouseState[mi].UpdateState(*qe);

    Ptr<InteractiveObject> ptopMouseCharacter = 
        GetTopMostEntity(qe->GetMouseEntry().GetPosition(), mi, avm2);
    mMouseState[mi].SetTopmostEntity(ptopMouseCharacter);    

    if (GetDraggingCharacter(mi))
    {
        GetDraggingCharacter(mi)->DoMouseDrag(mi);
    }

    /*
    //Debug code
    static unsigned lastState = 0;
    //printf("Mouse Position: %5.2f, %5.2f\n", qe->GetMouseEntry().GetPosition().x, qe->GetMouseEntry().GetPosition().y );
    if (mMouseState[mi].GetButtonsState() != (lastState & 1)) 
    {
        if (ptopMouseCharacter)  //?
            printf("! %s\n", ptopMouseCharacter->GetCharacterHandle()->GetNamePath().ToCStr());
        else
            printf("! NONE!\n");
    } 
    if (ptopMouseCharacter)  //?
    {
        printf("? %p %s\n", ptopMouseCharacter, ptopMouseCharacter->GetCharacterHandle()->GetNamePath().ToCStr());
    }
    lastState = mMouseState[mi].GetButtonsState();
    */

#ifndef SF_NO_IME_SUPPORT
    unsigned buttonsState = mMouseState[mi].GetButtonsState();
    // notify IME about mouse down/up
    if (qe->GetMouseEntry().IsButtonsStateChanged())
    {
        Ptr<IMEManagerBase> pIMEManager = GetIMEManager();
        if (pIMEManager && pIMEManager->IsMovieActive(this))
        {
            if (qe->GetMouseEntry().IsAnyButtonPressed())
                pIMEManager->OnMouseDown(this, buttonsState, ptopMouseCharacter);
            else
                pIMEManager->OnMouseUp(this, buttonsState, ptopMouseCharacter);
        }
    }
#endif //#ifdef SF_NO_IME_SUPPORT

    // Send mouse events.
    EventId::IdCode buttonEvent = EventId::Event_Invalid;
    if (qe->GetMouseEntry().IsButtonsStateChanged() && qe->GetMouseEntry().IsLeftButton())
    {
        buttonEvent = (qe->GetMouseEntry().IsAnyButtonPressed()) ?
            EventId::Event_MouseDown : EventId::Event_MouseUp;
    }

    for (unsigned movieIndex = (unsigned)MovieLevels.GetSize(); movieIndex > 0; movieIndex--)
    {
        InteractiveObject* pmovie = MovieLevels[movieIndex-1].pSprite;
        ButtonEventId evt(buttonEvent, mi);
        evt.MouseWheelDelta = qe->GetMouseEntry().WheelScrollDelta;

        // Send mouse up/down events.
        if (buttonEvent != EventId::Event_Invalid)
            pmovie->PropagateMouseEvent(ButtonEventId(buttonEvent, mi));
        // Send move.
        if (mMouseState[mi].IsMouseMoved())
        {
            pmovie->PropagateMouseEvent(ButtonEventId(EventId::Event_MouseMove, mi));
        }
    }
    if ((!IsDisableFocusAutoReleaseByMouseClick() && qe->GetMouseEntry().IsButtonsStateChanged()) ||
        (!IsDisableFocusAutoRelease() && mMouseState[mi].IsMouseMoved()))
    {
        HideFocusRect(mi);
    }

    if (qe->GetMouseEntry().IsMouseWheel())
    {
        // no need to propagate the mouse wheel, just sent 
        if (ptopMouseCharacter)
            ptopMouseCharacter->OnMouseWheelEvent(qe->GetMouseEntry().WheelScrollDelta);
    }

    pASMovieRoot->NotifyMouseEvent(qe, mMouseState[mi], mi);

    // check for necessity of changing cursor
    CheckMouseCursorType(mi, ptopMouseCharacter);

    // Send onKillFocus, if necessary
    if (qe->GetMouseEntry().IsAnyButtonPressed() && qe->GetMouseEntry().IsLeftButton())
    {
        Ptr<InteractiveObject> curFocused = GetFocusGroup(mi).LastFocused;
        if (ptopMouseCharacter != curFocused)
        {
            QueueSetFocusTo(ptopMouseCharacter, ptopMouseCharacter, mi, GFx::GFx_FocusMovedByMouse);
        }
    }

    //!AB: button's events, such as onPress/onRelease should be fired AFTER
    // appropriate mouse events (onMouseDown/onMouseUp).
    pASMovieRoot->GenerateMouseEvents(mi);
}

void MovieImpl::SetDragState(const DragState& st)   
{ 
    SF_ASSERT(st.MouseIndex < GFX_MAX_MICE_SUPPORTED);
    CurrentDragStates[st.MouseIndex] = st; 
}

void MovieImpl::GetDragState(unsigned mouseIndex, DragState* st) 
{ 
    SF_ASSERT(mouseIndex < GFX_MAX_MICE_SUPPORTED);
    *st = CurrentDragStates[mouseIndex]; 
}

void MovieImpl::SetDragStateTopmostEntity(unsigned mouseIndex, InteractiveObject* topmostEntity)
{
    SF_ASSERT(mouseIndex < GFX_MAX_MICE_SUPPORTED);
    CurrentDragStates[mouseIndex].pTopmostEntity = topmostEntity;
}

void MovieImpl::StopDrag(unsigned mouseIndex)                    
{ 
    SF_ASSERT(mouseIndex < GFX_MAX_MICE_SUPPORTED);
    CurrentDragStates[mouseIndex].pCharacter = NULL; 
    CurrentDragStates[mouseIndex].pTopmostEntity = NULL;
    CurrentDragStates[mouseIndex].MouseIndex = ~0u; 
}

void MovieImpl::StopDragCharacter(const InteractiveObject* ch)
{ 
    for (unsigned i = 0; i < GFX_MAX_MICE_SUPPORTED; ++i)
    {
        if (CurrentDragStates[i].pCharacter == ch)
        {
            CurrentDragStates[i].pCharacter = NULL; 
            CurrentDragStates[i].pTopmostEntity = NULL;
            CurrentDragStates[i].MouseIndex = ~0u; 
        }
    }
}

void MovieImpl::StopAllDrags()
{ 
    for (unsigned i = 0; i < GFX_MAX_MICE_SUPPORTED; ++i)
    {
        CurrentDragStates[i].pCharacter = NULL; 
        CurrentDragStates[i].pTopmostEntity = NULL;
        CurrentDragStates[i].MouseIndex = ~0u; 
    }
}

bool MovieImpl::IsDragging(unsigned mouseIndex) const
{ 
    SF_ASSERT(mouseIndex < GFX_MAX_MICE_SUPPORTED);
    return CurrentDragStates[mouseIndex].pCharacter != NULL; 
}

bool MovieImpl::IsDraggingMouseIndex(unsigned mouseIndex) const
{
    SF_ASSERT(mouseIndex < GFX_MAX_MICE_SUPPORTED);
    return CurrentDragStates[mouseIndex].MouseIndex == mouseIndex;
}

#ifdef GFX_MULTITOUCH_SUPPORT_ENABLE
int  MovieImpl::FindMouseStateIndexByTouchID(unsigned touchID)
{
    for (unsigned i = 1; i < sizeof(mMouseState)/sizeof(mMouseState[0]); ++i)
    {
        if (mMouseState[i].GetTouchID() == touchID)
        {
            return int(i);
        }
    }
    return -1;
}

void MovieImpl::ProcessTouch(const InputEventsQueue::QueueEntry* qe, 
                             ProcessFocusKeyInfo* focusKeyInfo)
{
    SF_UNUSED(focusKeyInfo);
    SF_AMP_SCOPE_TIMER_ID(AdvanceStats, "MovieImpl::ProcessTouch", Amp_Native_Function_Id_ProcessMouse);

    // Proceed mouse
    //*miceProceededMask |= (1 << qe->GetMouseEntry().MouseIndex);

    const InputEventsQueueEntry::TouchEntry& te = qe->GetTouchEntry();
    unsigned touchID = te.TouchPointID;
    // find a mouse state related to the touchID
    unsigned mi = 0, possibleMi = 0, prevID = ~0u;

    if (te.Type == InputEventsQueueEntry::Touch_Begin)
    {
        // find free mouse state (or the one with the lowest touchID)
        for (unsigned i = 1; i < sizeof(mMouseState)/sizeof(mMouseState[0]); ++i)
        {
            if (mMouseState[i].GetTouchID() == touchID || mMouseState[i].GetTouchID() == ~0u)
            {
                mi = i;
                break;
            }
            if (mMouseState[i].GetTouchID() < prevID)
            {
                possibleMi = i;
                prevID     = mMouseState[i].GetTouchID();
            }
        }
        // not found... but if this is TouchBegin we still need to use a mouse state with lowest id
        if (mi == 0)
        {
            SF_ASSERT(possibleMi != 0);
            mi = possibleMi;
        }
    }
    else
    {
        // find existing mouse state for End and Move events
        for (unsigned i = 1; i < sizeof(mMouseState)/sizeof(mMouseState[0]); ++i)
        {
            if (mMouseState[i].GetTouchID() == touchID)
            {
                mi = i;
                break;
            }
            if (mMouseState[i].GetTouchID() < prevID)
            {
                possibleMi = i;
                prevID     = mMouseState[i].GetTouchID();
            }
        }
    }
    if (mi == 0)
        return;
    mMouseState[mi].UpdateState(te);

    Ptr<InteractiveObject> ptopMouseCharacter = GetTopMostEntity(te.GetPosition(), mi, true);
    mMouseState[mi].SetTopmostEntity(ptopMouseCharacter);    

    if (GetDraggingCharacter(mi))
    {
        GetDraggingCharacter(mi)->DoMouseDrag(mi);
    }

    /*//Debug code
    static unsigned lastState = 0;
    printf("Mouse Position: %5.2f, %5.2f\n", qe->GetMouseEntry().GetPosition().x, qe->GetMouseEntry().GetPosition().y );
    if (mMouseState[mi].GetButtonsState() != (lastState & 1)) 
    {
    if (ptopMouseCharacter)  //?
    printf("! %s\n", ptopMouseCharacter->GetCharacterHandle()->GetNamePath().ToCStr());
    else
    printf("! NONE!\n");
    } 
    if (ptopMouseCharacter)  //?
    {
    printf("? %s\n", ptopMouseCharacter->GetCharacterHandle()->GetNamePath().ToCStr());
    }
    lastState = mMouseState[mi].GetButtonsState();
    */

	pASMovieRoot->GenerateTouchEvents(mi);

    if (te.Type == InputEventsQueueEntry::Touch_End)
        mMouseState[mi].ResetTouchID();
//#endif
}

void MovieImpl::ProcessGesture(const InputEventsQueue::QueueEntry* qe)
{
    SF_AMP_SCOPE_TIMER_ID(AdvanceStats, "MovieImpl::ProcessGesture", Amp_Native_Function_Id_ProcessMouse);

    // Proceed mouse
    //*miceProceededMask |= (1 << qe->GetMouseEntry().MouseIndex);

    const InputEventsQueueEntry::GestureEntry& ge = qe->GetGestureEntry();

    if (ge.Phase == InputEventsQueueEntry::Phase_Begin || ge.Phase == InputEventsQueueEntry::Phase_All)
    {
        GestureTopMostChar = GetTopMostEntity(ge.GetPosition(), 0, true);
    }
	
    pASMovieRoot->GenerateGestureEvents(GestureTopMostChar, ge.Phase, 
        PointF(ge.PosX, ge.PosY), PointF(ge.OffsetX, ge.OffsetY), 
        PointF(ge.ScaleX, ge.ScaleY), ge.Rotation, ge.GestureMask);

    if (ge.Phase == InputEventsQueueEntry::Phase_End || ge.Phase == InputEventsQueueEntry::Phase_All)
    {
        GestureTopMostChar = NULL;
    }
}
#endif

void MovieImpl::AdvanceFrame(bool nextFrame, float framePos)
{
    SF_AMP_SCOPE_TIMER_ID(AdvanceStats, "MovieImpl::AdvanceFrame", Amp_Native_Function_Id_AdvanceFrame);

#ifdef SF_ADVANCE_EXECUTION_TRACE
    bool first = true;
#endif
#if defined(GFX_CHECK_PLAYLIST_DEBUG)
    _PLCheckCnt = 0;
#endif
    if (nextFrame)
        pASMovieRoot->OnNextFrame();

    // If optimized advance list is invalidated then we need to 
    // recreate it. We need a separate loop iteration, since call to Advance
    // may modify the optimized advance list and this may cause incorrect behavior.
    if (IsOptAdvanceListInvalid())
    {
        SIMD::IS::PrefetchObj(pPlayListHead);
        pPlayListOptHead = NULL;
        G_SetFlag<Flag_OptimizedAdvanceListInvalid>(Flags, false);

        // invert OptAdvListMarker, so we will be able to distinguish newly added ('valid')
        // members of optimized playlist from the old ('invalid') members by comparing the 
        // proot->OptAdvListMarker vs pobj->OptAdvListMarker (if equal - valid).
        bool marker = !G_IsFlagSet<Flag2_OptAdvListMarker>(Flags2);
        G_SetFlag<Flag2_OptAdvListMarker>(Flags2, marker);
        
        // uncomment if necessary @DBG
        //CheckPlaylistConsistency();

        Array<Ptr<InteractiveObject> > playList;
        for (InteractiveObject* pcurCh = pPlayListHead; pcurCh != NULL; pcurCh = pcurCh->pPlayNext)
        {
            playList.PushBack(pcurCh);
        }
        for (UPInt i = 0; i < playList.GetSize(); ++i)
        {
            InteractiveObject* pcurCh = playList[i];
            InteractiveObject* pnextCh = pcurCh->pPlayNext;
            SIMD::IS::PrefetchObj(pnextCh);

            // A previous calls to AdvanceFrame may make some character 'valid' optlist members.
            // Don't re-add them again.
            if (!pcurCh->IsValidOptAdvListMember(this))
            {
                pcurCh->ClearOptAdvListFlag();
                pcurCh->SetOptAdvListMarker(marker);
                pcurCh->pPlayNextOpt = pcurCh->pPlayPrevOpt = NULL;
            }

            // "unloaded" chars should be already removed.
            SF_ASSERT(!pcurCh->IsUnloaded());

            if (pcurCh->CanAdvanceChar() && !pcurCh->IsExecutionAborted())
            {
                // skip addition to the optlist if it is valid already.
                if (!pcurCh->IsValidOptAdvListMember(this) && pcurCh->CheckAdvanceStatus(false) == 1)
                    pcurCh->AddToOptimizedPlayList();
                if (nextFrame || pcurCh->IsReqPartialAdvanceFlagSet())
                {
#ifdef SF_ADVANCE_EXECUTION_TRACE
                    if (first)
                        printf("\n");
                    first = false;
                    String s;
                    //printf("!------------- %s\n", pcurCh->GetCharacterHandle()->GetNamePath().ToCStr());
                    printf("!------------- %s\n", pcurCh->GetAbsolutePath(&s));
#endif

                    pcurCh->AdvanceFrame(nextFrame, framePos);          
                }
            }
        }
    }
    else
    {
        SIMD::IS::PrefetchObj(pPlayListOptHead);
        Array<Ptr<InteractiveObject> > playList;
        for (InteractiveObject* pcurCh = pPlayListOptHead; pcurCh != NULL; pcurCh = pcurCh->pPlayNextOpt)
        {
            playList.PushBack(pcurCh);
        }
        for (UPInt i = 0; i < playList.GetSize(); ++i)
        {
            InteractiveObject* pcurCh = playList[i];

            SF_ASSERT(!pcurCh->IsUnloaded());
            SF_ASSERT(pcurCh->IsOptAdvListFlagSet());

            InteractiveObject* pnextCh = pcurCh->pPlayNextOpt;
            SIMD::IS::PrefetchObj(pnextCh);

            if (pcurCh->IsMarkedToRemoveFromOptimizedPlayList())
            {
                pcurCh->RemoveFromOptimizedPlayList();
                continue;
            }

            if (pcurCh->CanAdvanceChar() && (nextFrame || pcurCh->IsReqPartialAdvanceFlagSet()) && 
                !pcurCh->IsExecutionAborted())
            {
#ifdef SF_ADVANCE_EXECUTION_TRACE
                if (first)
                    printf("\n");
                first = false;
                printf("+------------- %s\n", pcurCh->GetCharacterHandle()->GetNamePath().ToCStr());
#endif
                pcurCh->AdvanceFrame(nextFrame, framePos);          
            }
        }
    }
    CheckPlaylistConsistency(true);                 // @DBG
    CheckOptPlaylistConsistency(NULL, NULL, true);  // @DBG

#ifdef SF_ADVANCE_EXECUTION_TRACE
    if (!first)
        printf("\n");
#endif
    SetRegisteredFontsChanged(false);
}

#if defined(GFX_CHECK_PLAYLIST_DEBUG)
void MovieImpl::CheckOptPlaylistConsistency(InteractiveObject* mustBeInList,
                                            InteractiveObject* mustBeOutOfList,
                                            bool enforce)
{
    if (!enforce && _PLCheckCnt++ >= PLCheckCnt_Limit_Per_Frame)
        return;
    bool inList = false, outOfList = true;
    if (!IsOptAdvanceListInvalid() && pPlayListOptHead)
    {
        InteractiveObject* slow = pPlayListOptHead;
        InteractiveObject* fast = pPlayListOptHead;
        InteractiveObject* prev = NULL;
        while (slow && slow->pPlayNextOpt && fast && fast->pPlayNextOpt)
        {
            SF_ASSERT(slow->IsOptAdvListFlagSet());
            SF_ASSERT(slow->IsValidOptAdvListMember(this));
            SF_ASSERT(fast->IsOptAdvListFlagSet());
            SF_ASSERT(fast->IsValidOptAdvListMember(this));
            SF_ASSERT(slow->pPlayPrevOpt == prev);
            prev = slow;
            slow = slow->pPlayNextOpt;
            fast = fast->pPlayNextOpt->pPlayNextOpt;
            if (slow == fast)
            {
                SF_ASSERT(0); //LOOP in forward direction!
                return;
            }
            if (prev == mustBeInList)
                inList = true;
            if (prev == mustBeOutOfList)
                outOfList = false;
        }
        SF_ASSERT(slow);
        // finalize the loop
        while (slow)
        {
            SF_ASSERT(slow->IsOptAdvListFlagSet());
            SF_ASSERT(slow->IsValidOptAdvListMember(this));
            SF_ASSERT(slow->pPlayPrevOpt == prev);
            prev = slow;
            slow = slow->pPlayNextOpt;
            if (prev == mustBeInList)
                inList = true;
            if (prev == mustBeOutOfList)
                outOfList = false;
        }
        SF_ASSERT(!mustBeInList || inList || mustBeInList == pPlayListOptHead);
        SF_ASSERT(!mustBeOutOfList || (outOfList && mustBeOutOfList != pPlayListOptHead));

        // check the playlist in reverse direction. 
        // To avoid big delays, by default, it will be performed only once per frame (when 'enforce'
        // is set to 'true'). 
        if (!enforce)
            return;    // comment out if need to check always

        inList = (slow == mustBeInList)?true:false; 
        outOfList = (slow == mustBeOutOfList)?false:true;

        slow = prev;
        prev = NULL;
        while (slow && slow->pPlayPrevOpt && fast && fast->pPlayPrevOpt)
        {
            SF_ASSERT(slow->IsOptAdvListFlagSet());
            SF_ASSERT(slow->IsValidOptAdvListMember(this));
            SF_ASSERT(fast->IsOptAdvListFlagSet());
            SF_ASSERT(fast->IsValidOptAdvListMember(this));
            SF_ASSERT(slow->pPlayNextOpt == prev);
            prev = slow;
            slow = slow->pPlayPrevOpt;
            fast = fast->pPlayPrevOpt->pPlayPrevOpt;
            if (slow == fast)
            {
                SF_ASSERT(0); //LOOP in reverse direction!
                return;
            }
            if (prev == mustBeInList)
                inList = true;
            if (prev == mustBeOutOfList)
                outOfList = false;
        }
        // finalize the loop
        while (slow)
        {
            SF_ASSERT(slow->IsOptAdvListFlagSet());
            SF_ASSERT(slow->IsValidOptAdvListMember(this));
            SF_ASSERT(slow->pPlayNextOpt == prev);
            prev = slow;
            slow = slow->pPlayPrevOpt;
            if (prev == mustBeInList)
                inList = true;
            if (prev == mustBeOutOfList)
                outOfList = false;
        }
        SF_ASSERT(pPlayListOptHead == prev);
        SF_ASSERT(!mustBeInList || inList || mustBeInList == pPlayListOptHead);
        SF_ASSERT(!mustBeOutOfList || (outOfList && mustBeOutOfList != pPlayListOptHead));
    }
}
void MovieImpl::CheckPlaylistConsistency(bool enforce)
{
    //SF_UNUSED(enforce);
    //return ;
    if (!enforce && _PLCheckCnt++ >= PLCheckCnt_Limit_Per_Frame)
        return;
    if (pPlayListHead)
    {
        InteractiveObject* slow = pPlayListHead;
        InteractiveObject* fast = pPlayListHead;
        InteractiveObject* prev = NULL;
        while (slow && slow->pPlayNext && fast && fast->pPlayNext)
        {
            SF_ASSERT(slow->pPlayPrev == prev);
            prev = slow;
            slow = slow->pPlayNext;
            fast = fast->pPlayNext->pPlayNext;
            if (slow == fast)
            {
                SF_ASSERT(0); //LOOP in forward direction!
                return;
            }

            // check that slow->parent appears AFTER the 'slow'
            if (prev->GetParent() && prev->GetParent()->IsInPlayList())
            {
                InteractiveObject* p = prev->pPlayNext;
                while(p)
                {
                    if (p == prev->GetParent())
                        break;
                    p = p->pPlayNext;
                }
                if (!p) // if !p - parent is in invalid place (before the node or not
                        // in the playlist at all).
                {
                    // it is allowed for non-displayobjcontainer parents not to be
                    // presented in the playlist (for example, Buttons and TextFields).
                    // But make sure it is not in the playlist at all.
                    // Textfield's are allowed to be out of order in the playlist.
                    SF_ASSERT(prev->GetType() == CharacterDef::TextField || !prev->GetParent()->IsDisplayObjContainer());
                }
            }
        }
        SF_ASSERT(slow);

        // finalize the loop
        while (slow)
        {
            SF_ASSERT(slow->pPlayPrev == prev);
            prev = slow;
            slow = slow->pPlayNext;

            // check that slow->parent appears AFTER the 'slow'
            if (prev->GetParent() && prev->GetParent()->IsInPlayList())
            {
                InteractiveObject* p = prev->pPlayNext;
                while(p)
                {
                    if (p == prev->GetParent())
                        break;
                    p = p->pPlayNext;
                }
                if (!p) // if !p - parent is in invalid place (before the node or not
                        // in the playlist at all).
                {
                    // it is allowed for non-displayobjcontainer parents not to be
                    // presented in the playlist (for example, Buttons and TextFields).
                    // But make sure it is not in the playlist at all
                    // Textfield's are allowed to be out of order in the playlist.
                    SF_ASSERT(prev->GetType() == CharacterDef::TextField || !prev->GetParent()->IsDisplayObjContainer());
                }
            }
        }

        // check the playlist in reverse direction. 
        // To avoid big delays, by default, it will be performed only once per frame (when 'enforce'
        // is set to 'true'). 
        if (!enforce)
            return;    // comment out if need to check always

        slow = prev;
        prev = NULL;
        while (slow && slow->pPlayPrev && fast && fast->pPlayPrev)
        {
            SF_ASSERT(slow->pPlayNext == prev);
            prev = slow;
            slow = slow->pPlayPrev;
            fast = fast->pPlayPrev->pPlayPrev;
            if (slow == fast)
            {
                SF_ASSERT(0); //LOOP in reverse direction!
                return;
            }
        }
        // finalize the loop
        while (slow)
        {
            SF_ASSERT(slow->pPlayNext == prev);
            prev = slow;
            slow = slow->pPlayPrev;
        }
        SF_ASSERT(pPlayListHead == prev);
    }
}
#endif

void MovieImpl::CheckMouseCursorType(unsigned mouseIdx, InteractiveObject* ptopMouseCharacter)
{
    if (mMouseState[mouseIdx].IsTopmostEntityChanged())
    {
        unsigned newCursorType = MouseCursorEvent::ARROW;
        if (ptopMouseCharacter)
            newCursorType = ptopMouseCharacter->GetCursorType();
        ChangeMouseCursorType(mouseIdx, newCursorType);
    }
}

void MovieImpl::ChangeMouseCursorType(unsigned mouseIdx, unsigned newCursorType)
{
    unsigned prevCursor = mMouseState[mouseIdx].GetCursorType();
    if (prevCursor != newCursorType)
        pASMovieRoot->ChangeMouseCursorType(mouseIdx,  newCursorType);
    mMouseState[mouseIdx].SetCursorType(newCursorType);
}

unsigned  MovieImpl::HandleEvent(const Event &event)
{
    if (!IsMovieFocused() && event.Type != Event::SetFocus)
        return HE_NotHandled;

    union
    {
        const Event *        pevent;
        const KeyEvent *     pkeyEvent;
        const MouseEvent *   pmouseEvent;
        const TouchEvent *   ptouchEvent;
        const GestureEvent * pgestureEvent;
        const OrientationEvent* porientationEvent;
#ifdef GFX_ENABLE_MOBILE_APP_SUPPORT
        const AppLifecycleEvent* pappLifecycleEvent;
#endif
        const CharEvent *    pcharEvent;
        const GamePadAnalogEvent * pgamepadAnalogEvent;
#ifndef SF_NO_IME_SUPPORT
        const IMEEvent *     pimeEvent;
#endif
        const SetFocusEvent* pfocusEvent;
		const AccelerometerEvent* paccelerometerEvent;
		const GeolocationEvent* pgeolocationEvent;
		const StatusEvent* pstatusEvent;
    };
    pevent = &event;

    SF_UNUSED4( ptouchEvent, pgestureEvent, porientationEvent, pgamepadAnalogEvent);

    // handle set focus event for movie
    if (event.Type == Event::SetFocus)
    {
#ifdef GFX_ENABLE_KEYBOARD
        // Handle special keys state, if initialized
        for (unsigned i = 0; i < GFX_MAX_KEYBOARD_SUPPORTED; ++i)
        {
            if (pfocusEvent->KeysStates[i].IsInitialized())
            {
                KeyboardState* keyboardState = GetKeyboardState(i);
                SF_ASSERT(keyboardState);
                keyboardState->SetKeyToggled(Key::NumLock, 
                    pfocusEvent->KeysStates[i].IsNumToggled());
                keyboardState->SetKeyToggled(Key::CapsLock, 
                    pfocusEvent->KeysStates[i].IsCapsToggled());
                keyboardState->SetKeyToggled(Key::ScrollLock, 
                    pfocusEvent->KeysStates[i].IsScrollToggled());
            }
        }
#endif // GFX_ENABLE_KEYBOARD
        OnMovieFocus(true);
        return HE_Handled;
    }
    else if (event.Type == Event::KeyDown || event.Type == Event::KeyUp)
    {
#ifdef GFX_ENABLE_KEYBOARD
        // Handle special keys state, if initialized
        KeyboardState* keyboardState = GetKeyboardState(pkeyEvent->KeyboardIndex);
        SF_DEBUG_WARNING(!keyboardState, "KeyEvent contains wrong index of keyboard");
        if (keyboardState && pkeyEvent->Modifiers.IsInitialized())
        {
            keyboardState->SetKeyToggled(Key::NumLock, 
                pkeyEvent->Modifiers.IsNumToggled());
            keyboardState->SetKeyToggled(Key::CapsLock, 
                pkeyEvent->Modifiers.IsCapsToggled());
            keyboardState->SetKeyToggled(Key::ScrollLock, 
                pkeyEvent->Modifiers.IsScrollToggled());
        }
#endif // GFX_ENABLE_KEYBOARD
    }

    // Process the event type

    switch(event.Type)
    {
        // Mouse.
    case Event::MouseMove:
        SF_DEBUG_WARNING(event.EventClassSize != sizeof(MouseEvent),
            "MouseEvent class required for mouse events");
        if (pmouseEvent->MouseIndex < MouseCursorCount)
        {
            PointF pt;
            ViewportMatrix.TransformByInverse(&pt, PointF(pmouseEvent->x, pmouseEvent->y));
//             PointF pt(PixelsToTwips(pmouseEvent->x * ViewScaleX + ViewOffsetX),
//                       PixelsToTwips(pmouseEvent->y * ViewScaleY + ViewOffsetY));
            InputEventsQueue.AddMouseMove(pmouseEvent->MouseIndex, pt);
            return HE_Completed;
        }
        break;

    case Event::MouseUp:
        SF_DEBUG_WARNING(event.EventClassSize != sizeof(MouseEvent),
            "MouseEvent class required for mouse events");
        if (pmouseEvent->MouseIndex < MouseCursorCount)
        {
            PointF pt;
            ViewportMatrix.TransformByInverse(&pt, PointF(pmouseEvent->x, pmouseEvent->y));
//             PointF pt(PixelsToTwips(pmouseEvent->x * ViewScaleX + ViewOffsetX),
//                       PixelsToTwips(pmouseEvent->y * ViewScaleY + ViewOffsetY));
            InputEventsQueue.AddMouseRelease(pmouseEvent->MouseIndex, pt, (1 << pmouseEvent->Button));
            return HE_Completed;
        }
        break;

    case Event::MouseDown:
        SF_DEBUG_WARNING(event.EventClassSize != sizeof(MouseEvent),
            "MouseEvent class required for mouse events");
        if (pmouseEvent->MouseIndex < MouseCursorCount)
        {
            PointF pt;
            ViewportMatrix.TransformByInverse(&pt, PointF(pmouseEvent->x, pmouseEvent->y));
//             PointF pt(PixelsToTwips(pmouseEvent->x * ViewScaleX + ViewOffsetX),
//                       PixelsToTwips(pmouseEvent->y * ViewScaleY + ViewOffsetY));
            InputEventsQueue.AddMousePress(pmouseEvent->MouseIndex, pt, (1 << pmouseEvent->Button));
            return HE_Completed;
        }
        break;

    case Event::MouseWheel:
        SF_DEBUG_WARNING(event.EventClassSize != sizeof(MouseEvent),
            "MouseEvent class required for mouse events");
        if (pmouseEvent->MouseIndex < MouseCursorCount)
        {
            PointF pt;
            ViewportMatrix.TransformByInverse(&pt, PointF(pmouseEvent->x, pmouseEvent->y));
//             PointF pt(PixelsToTwips(pmouseEvent->x * ViewScaleX + ViewOffsetX),
//                       PixelsToTwips(pmouseEvent->y * ViewScaleY + ViewOffsetY));
            InputEventsQueue.AddMouseWheel(pmouseEvent->MouseIndex, pt, (int)pmouseEvent->ScrollDelta);
            return HE_Completed;
        }
        break;


#ifdef GFX_ENABLE_KEYBOARD
        // Keyboard.
    case Event::KeyDown:
        {
            SF_DEBUG_WARNING(event.EventClassSize != sizeof(KeyEvent),
                "KeyEvent class required for keyboard events");
            KeyboardState* keyboardState = GetKeyboardState(pkeyEvent->KeyboardIndex);
            if (keyboardState)
            {
                keyboardState->SetKeyDown(pkeyEvent->KeyCode, pkeyEvent->AsciiCode, 
                                          pkeyEvent->Modifiers);
                // a special case for handling left/right shift, alt, control
                switch(pkeyEvent->KeyCode)
                {
                case Key::Shift: 
                    if (!pkeyEvent->Modifiers.IsExtendedKey())
                        keyboardState->SetKeyDown(Key::LeftShift, 0, 0, false); 
                    else
                        keyboardState->SetKeyDown(Key::RightShift, 0, 0, false);
                    break;
                case Key::Control: 
                    if (!pkeyEvent->Modifiers.IsExtendedKey())
                        keyboardState->SetKeyDown(Key::LeftControl, 0, 0, false); 
                    else
                        keyboardState->SetKeyDown(Key::RightControl, 0, 0, false);
                    break;
                case Key::Alt: 
                    if (!pkeyEvent->Modifiers.IsExtendedKey())
                        keyboardState->SetKeyDown(Key::LeftAlt, 0, 0, false); 
                    else
                        keyboardState->SetKeyDown(Key::RightAlt, 0, 0, false);
                    break;
                default:;
                }
            }
            InputEventsQueue.AddKeyDown(pkeyEvent->KeyCode, pkeyEvent->AsciiCode, 
                                        pkeyEvent->Modifiers, pkeyEvent->KeyboardIndex);
            if (pkeyEvent->WcharCode == 13 || (pkeyEvent->WcharCode >= 32 && pkeyEvent->WcharCode != 127))
            {
                InputEventsQueue.AddCharTyped(pkeyEvent->WcharCode, pkeyEvent->KeyboardIndex);
            }
            return HE_Completed;
        }

    case Event::KeyUp:
        {
            SF_DEBUG_WARNING(event.EventClassSize != sizeof(KeyEvent),
                "KeyEvent class required for keyboard events");
            KeyboardState* keyboardState = GetKeyboardState(pkeyEvent->KeyboardIndex);
            if (keyboardState)
            {
                keyboardState->SetKeyUp(pkeyEvent->KeyCode, pkeyEvent->AsciiCode, 
                                        pkeyEvent->Modifiers);
                // a special case for handling left/right shift, alt, control
                switch(pkeyEvent->KeyCode)
                {
                case Key::Shift: 
                    if (!pkeyEvent->Modifiers.IsExtendedKey())
                        keyboardState->SetKeyUp(Key::LeftShift, 0, 0, false); 
                    else
                        keyboardState->SetKeyUp(Key::RightShift, 0, 0, false);
                    break;
                case Key::Control: 
                    if (!pkeyEvent->Modifiers.IsExtendedKey())
                        keyboardState->SetKeyUp(Key::LeftControl, 0, 0, false); 
                    else
                        keyboardState->SetKeyUp(Key::RightControl, 0, 0, false);
                    break;
                case Key::Alt: 
                    if (!pkeyEvent->Modifiers.IsExtendedKey())
                        keyboardState->SetKeyUp(Key::LeftAlt, 0, 0, false); 
                    else
                        keyboardState->SetKeyUp(Key::RightAlt, 0, 0, false);
                    break;
                default:;
                }
            }
            InputEventsQueue.AddKeyUp(pkeyEvent->KeyCode, pkeyEvent->AsciiCode, 
                pkeyEvent->Modifiers, pkeyEvent->KeyboardIndex);
            return HE_Completed;
        }

    case Event::Char:
        SF_DEBUG_WARNING(event.EventClassSize != sizeof(CharEvent),
            "CharEvent class required for keyboard events");
        SF_DEBUG_WARNING(!GetKeyboardState(pcharEvent->KeyboardIndex), 
            "GFxCharEvent contains wrong index of keyboard");
        InputEventsQueue.AddCharTyped(pcharEvent->WcharCode, pcharEvent->KeyboardIndex);
        return HE_Completed;
#else
    case Event::KeyDown:
    case Event::KeyUp:
    case Event::Char:
        SF_DEBUG_WARNING(1, "Keyboard support is disabled due to SF_NO_KEYBOARD_SUPPORT macro.");
        break;
#endif //SF_NO_KEYBOARD_SUPPORT

#ifndef SF_NO_IME_SUPPORT
    case Event::IME:
        {
            // IME events should be handled immediately
            Ptr<IMEManagerBase> pIMEManager = GetIMEManager();
            if (pIMEManager)
            {
                return pIMEManager->HandleIMEEvent(this, *pimeEvent);
            }
#if defined(SF_OS_WIN32) && defined(GFX_ENABLE_BUILTIN_KOREAN_IME) && !defined(SF_NO_IME_SUPPORT)
            else
            {
                return HandleKoreanIME(*pimeEvent);
            }
#endif // SF_NO_BUILTIN_KOREAN_IME
        }
        break;
#endif //#ifdef SF_NO_IME_SUPPORT

    case Event::KillFocus:
        OnMovieFocus(false);
        break;

#ifdef GFX_ENABLE_ANALOG_GAMEPAD
    case Event::GamePadAnalog:
        {
            SF_DEBUG_WARNING(event.EventClassSize != sizeof(GamePadAnalogEvent),
                "GamePadAnalogEvent required for analog gamepad events");
            InputEventsQueue.AddGamePadAnalogEvent(pgamepadAnalogEvent->Code, 
                pgamepadAnalogEvent->AnalogX, pgamepadAnalogEvent->AnalogY, 
                pgamepadAnalogEvent->ControllerIndex );
            return HE_Completed;
        }
        break;
#endif

#ifdef GFX_MULTITOUCH_SUPPORT_ENABLE
    case Event::TouchBegin:
    case Event::TouchEnd:
    case Event::TouchMove:
        {
			// should feed touch events for gestures too (since some gestures, such as pressAndTap 
			// and twoFingerTap, are generated by the GFx core, not by the system.
            if (MultitouchMode != 0) 
            {
                SF_DEBUG_WARNING(event.EventClassSize != sizeof(TouchEvent),
                    "TouchEvent class required for touch events");
                PointF pt;
                ViewportMatrix.TransformByInverse(&pt, PointF(ptouchEvent->x, ptouchEvent->y));
//                 PointF pt(PixelsToTwips(ptouchEvent->x * ViewScaleX + ViewOffsetX),
//                           PixelsToTwips(ptouchEvent->y * ViewScaleY + ViewOffsetY));
                PointF pt1;
                ViewportMatrix.TransformByInverse(&pt1, PointF(ptouchEvent->x + ptouchEvent->WContact, 
                                                               ptouchEvent->y + ptouchEvent->HContact));
                SizeF sz(pt1.x - pt.x, pt1.y - pt.y);
//                 SizeF  sz(PixelsToTwips(ptouchEvent->WContact * ViewScaleX),
//                     PixelsToTwips(ptouchEvent->HContact * ViewScaleY));
                InputEventsQueueEntry::TouchType tt;
                switch(event.Type)
                {
				case Event::TouchBegin: 
					tt = InputEventsQueueEntry::Touch_Begin; 
                    break;
                case Event::TouchEnd:  
                    tt = InputEventsQueueEntry::Touch_End; 
                    break;
                case Event::TouchMove:  
                    tt = InputEventsQueueEntry::Touch_Move; 
                    break;
                default: SF_ASSERT(0);  tt = InputEventsQueueEntry::Touch_End;
                    }
                InputEventsQueue.AddTouchEvent(tt, ptouchEvent->TouchPointID, pt, sz, ptouchEvent->Pressure, ptouchEvent->PrimaryPoint);
                /*
                // If we are using the Scaleform Gesture Recognizer, report with these touch points
                #ifdef GFX_GESTURE_RECOGNIZE
                    GestureRecognizer.parent = this;
                    switch(event.Type)
                    {
                    case Event::TouchBegin: 
                        GestureRecognizer.AddTouchBegin(ptouchEvent->TouchPointID, pt);
                        break;
                    case Event::TouchMove:  
                        GestureRecognizer.AddTouchMove(ptouchEvent->TouchPointID, pt);
                        break;
                    case Event::TouchEnd:  
                    default: 
                        GestureRecognizer.AddTouchEnd(ptouchEvent->TouchPointID, pt);
                        break;
                    }
                #endif // GFX_GESTURE_RECOGNIZE
                */
                return HE_Completed;
            }
            break;
        }
    case Event::GestureBegin:
    case Event::GestureEnd:
    case Event::Gesture:
    case Event::GestureSimple:
        {
            if ((MultitouchMode & MTI_Gesture) != 0)
            {
                SF_DEBUG_WARNING(event.EventClassSize != sizeof(GestureEvent),
                    "GestureEvent class required for gesture events");
                PointF pt;
                ViewportMatrix.TransformByInverse(&pt, PointF(pgestureEvent->x, pgestureEvent->y));
//                 PointF pt(PixelsToTwips(pgestureEvent->x * ViewScaleX + ViewOffsetX),
//                     PixelsToTwips(pgestureEvent->y * ViewScaleY + ViewOffsetY));
//                 PointF off(PixelsToTwips(pgestureEvent->OffsetX * ViewScaleX),
//                     PixelsToTwips(pgestureEvent->OffsetY * ViewScaleY));
                PointF off;
                ViewportMatrix.TransformByInverse(&off, 
                    PointF(pgestureEvent->x + pgestureEvent->OffsetX, 
                           pgestureEvent->y + pgestureEvent->OffsetY));
                off -= pt;
                InputEventsQueueEntry::GesturePhase ph;
                switch(event.Type)
                {
                case Event::GestureBegin: ph = InputEventsQueueEntry::Phase_Begin; break;
                case Event::GestureEnd:   ph = InputEventsQueueEntry::Phase_End; break;
                case Event::Gesture:      ph = InputEventsQueueEntry::Phase_Update; break;
                default:                  ph = InputEventsQueueEntry::Phase_All;
                }
                InputEventsQueue.AddGestureEvent(ph, pgestureEvent->GestureMask, pt, off, 
                    pgestureEvent->ScaleX, pgestureEvent->ScaleY, pgestureEvent->Rotation);
                return HE_Completed;
            }
            break;
        }
#endif

	case Event::Accelerometer:
        {
            SF_DEBUG_WARNING(event.EventClassSize != sizeof(AccelerometerEvent),
                "AccelerometerEvent class required for accelerometer events");
            //pASMovieRoot->OnAccelerometerEvent(*paccelerometerEvent);

			  InputEventsQueue.AddAccelerometerEvent(paccelerometerEvent->idAcc, paccelerometerEvent->timestamp,
				  paccelerometerEvent->accelerationX, paccelerometerEvent->accelerationY,
				  paccelerometerEvent->accelerationZ);

            return HE_Completed;
		}

		case Event::Geolocation:
        {
            SF_DEBUG_WARNING(event.EventClassSize != sizeof(GeolocationEvent),
                "GeolocationEvent class required for geolocation events");

			InputEventsQueue.AddGeolocationEvent(pgeolocationEvent->idGeo, pgeolocationEvent->latitude, 
				pgeolocationEvent->longitude, pgeolocationEvent->altitude, pgeolocationEvent->hAccuracy, 
				pgeolocationEvent->vAccuracy, pgeolocationEvent->speed, pgeolocationEvent->heading, 
				pgeolocationEvent->timestamp);

            return HE_Completed;
		}

		case Event::Status:
        {
            SF_DEBUG_WARNING(event.EventClassSize != sizeof(StatusEvent),
                "StatusEvent class required for status events");

			InputEventsQueue.AddStatusEvent(pstatusEvent->code, pstatusEvent->level, pstatusEvent->extensionId, pstatusEvent->contextId);

            return HE_Completed;
		}

#ifdef GFX_ENABLE_MOBILE_APP_SUPPORT
    case Event::AppLifecycle:
        {
            SF_DEBUG_WARNING(event.EventClassSize != sizeof(AppLifecycleEvent),
                "AppLifecycleEvent class required for lifecycle events");
            pASMovieRoot->OnAppLifecycleEvent(*pappLifecycleEvent);
        }
        return HE_Completed;

    case Event::OrientationChanged:
        {
            SF_DEBUG_WARNING(event.EventClassSize != sizeof(OrientationEvent),
                "OrientationEvent class required for orientation events");
            SetDeviceOrientation(porientationEvent->Orientation);
            pASMovieRoot->OnDeviceOrientationChanged(*porientationEvent);
        }
        return HE_Completed;
#endif
    default:
        break;
    }
    return HE_NotHandled;
}

// The host app uses this to tell the GFxMovieSub where the
// user's mouse pointer is.
// Note, this function is deprecated, use HandleEvent instead.
void    MovieImpl::NotifyMouseState(float x, float y, unsigned buttons, unsigned mouseIndex)
{
//     float mx  = (x * ViewScaleX + ViewOffsetX);
//     float my  = (y * ViewScaleY + ViewOffsetY);
//     PointF     pt(PixelsToTwips(mx), PixelsToTwips(my));
    PointF pt;
    ViewportMatrix.TransformByInverse(&pt, PointF(x, y));

    if (mouseIndex < MouseCursorCount)
    {
        //mMouseState[mouseIndex].ResetQueue();
        //PointF p = mMouseState[mouseIndex].GetLastPosition();
        //if (pt != p)
        InputEventsQueue.AddMouseMove(mouseIndex, pt);
        unsigned lastButtons = mMouseState[mouseIndex].GetButtonsState();
        for (unsigned i = 0, mask = 1; i < MouseState::MouseButton_MaxNum; i++, mask <<= 1)
        {
            if ((buttons & mask) && !(lastButtons & mask))
                InputEventsQueue.AddMousePress(mouseIndex, pt, (buttons & mask));
            else if ((lastButtons & mask) && !(buttons & mask))
                InputEventsQueue.AddMouseRelease(mouseIndex, pt, (lastButtons & mask));
        }
    }
}

void MovieImpl::GetMouseState(unsigned mouseIndex, float* x, float* y, unsigned* buttons)
{
    SF_ASSERT(mouseIndex < GFX_MAX_MICE_SUPPORTED);
    if (mouseIndex < MouseCursorCount)
    {
        PointF p = mMouseState[mouseIndex].GetLastPosition();
        // recalculate coords back to window
        p.x = (TwipsToPixels(p.x) - ViewOffsetX) / ViewScaleX;
        p.y = (TwipsToPixels(p.y) - ViewOffsetY) / ViewScaleY;

        if (x)
            *x = p.x;
        if (y)
            *y = p.y;
        if (buttons)
            *buttons = mMouseState[mouseIndex].GetButtonsState();
    }
}

bool    MovieImpl::HitTest(float x, float y, HitTestType testCond, unsigned controllerIdx)
{
    SF_AMP_SCOPE_TIMER(AdvanceStats, "MovieImpl::HitTest", Amp_Profile_Level_Medium);

    PointF pt;
    ViewportMatrix.TransformByInverse(&pt, PointF(x, y));
//     float mx  = (x * ViewScaleX + ViewOffsetX);
//     float my  = (y * ViewScaleY + ViewOffsetY);
//     PointF     pt(PixelsToTwips(mx), PixelsToTwips(my));

    SetNormalizedScreenCoords(pt);

    unsigned movieIndex;
    for (movieIndex = (unsigned)MovieLevels.GetSize(); movieIndex > 0; movieIndex--)
    {
        InteractiveObject*  pmovie  = MovieLevels[movieIndex-1].pSprite;

        RectF      movieLocalBounds = pmovie->GetBounds(Matrix2F());

        PointF     ptMv;
        pmovie->TransformPointToLocal(&ptMv, pt, false);

        if (movieLocalBounds.Contains(ptMv) || pmovie->Has3D())
        {
            switch(testCond)
            {
            case HitTest_Bounds:
                if (pmovie->PointTestLocal(ptMv, 0))
                    return 1;
                break;
            case HitTest_Shapes:
                if (pmovie->PointTestLocal(ptMv, DisplayObjectBase::HitTest_TestShape))
                    return 1;
                break;
            case HitTest_ButtonEvents:
                {
                    DisplayObjectBase::TopMostDescr descr;
                    descr.ControllerIdx = controllerIdx;
                    if (GetAVMVersion() == 1)
                    {
                        // AS1.0 & 2.0 only.
                        if (pmovie->GetTopMostMouseEntity(ptMv, &descr) == DisplayObjectBase::TopMost_Found)
                            return 1;
                    }
                    else
                    {
                        // AS3
                        descr.TestAll = true;
                        int r = pmovie->GetTopMostMouseEntity(ptMv, &descr);
                        if (r == DisplayObjectBase::TopMost_Found)
                        {
                            // check if any of the parents ActsAsButton
                            for (InteractiveObject* p = descr.pResult; p; p = p->GetParent())
                            {
                                if (p->ActsAsButton())
                                    return 1;
                            }
                        }
                    }
                    break;
                }
            case HitTest_ShapesNoInvisible:
                if (pmovie->PointTestLocal(ptMv, 
                    DisplayObjectBase::HitTest_TestShape | DisplayObjectBase::HitTest_IgnoreInvisible))
                    return 1;
                break;
            }
        }
    }
    return 0;
}

int    MovieImpl::AddIntervalTimer(ASIntervalTimerIntf *timer)
{
    timer->SetId(++LastIntervalTimerId);
    IntervalTimers.PushBack(timer);
    return LastIntervalTimerId;
}

void    MovieImpl::ClearIntervalTimer(int timerId)
{
    for (UPInt i = 0, n = IntervalTimers.GetSize(); i < n; ++i)
    {
        if (IntervalTimers[i] && IntervalTimers[i]->GetId() == timerId)
        {
            // do not remove the timer's array's entry here, just null it;
            // it will be removed later in Advance.
            IntervalTimers[i]->Clear();
            return;
        }
    }
}

void    MovieImpl::ShutdownTimers()
{
    for (UPInt i = 0, n = IntervalTimers.GetSize(); i < n; ++i)
    {
        IntervalTimers[i]->Clear();
    }
    IntervalTimers.Clear();
}

void    MovieImpl::ShutdownTimersForMovieDef(MovieDefImpl* defimpl)
{
    for (UPInt i = 0, n = IntervalTimers.GetSize(); i < n; ++i)
    {
        if (IntervalTimers[i]->ClearFor(this, defimpl))
            IntervalTimers[i] = NULL;
    }
}

#ifdef GFX_ENABLE_VIDEO
void MovieImpl::AddVideoProvider(Video::VideoProvider* pvideo)
{
    if (pvideo)
        VideoProviders.Add(pvideo);
}
void MovieImpl::RemoveVideoProvider(Video::VideoProvider* pvideo)
{
    VideoProviders.Remove(pvideo);
}
#endif

// *** MovieImpl's Movie interface implementation

// These methods mostly delegate to _level0 movie.

MovieDef*    MovieImpl::GetMovieDef() const
{   
    return pMainMovieDef;
}
unsigned    MovieImpl::GetCurrentFrame() const
{
    return pMainMovie ? pMainMovie->GetCurrentFrame() : 0;
}
bool    MovieImpl::HasLooped() const
{
    return pMainMovie ? pMainMovie->HasLooped() : 0; 
}

// Destroys a level and its resources.
bool    MovieImpl::ReleaseLevelMovie(int level)
{
    if (level == 0)
    {
        StopAllDrags();

        ShutdownTimers();

        Ptr<InteractiveObject> level0 = (MovieLevels.GetSize()) ? 
            static_cast<InteractiveObject*>(MovieLevels[0].pSprite.GetPtr()) : 
            static_cast<InteractiveObject*>(NULL);

        // Not sure if this unload order is OK
        while (MovieLevels.GetSize())
        {
            InteractiveObject* plevel = MovieLevels[MovieLevels.GetSize() - 1].pSprite;
            plevel->OnEventUnload();
            pASMovieRoot->DoActions();
            plevel->ForceShutdown();
            MovieLevels.RemoveAt(MovieLevels.GetSize() - 1);
        }

        // delete all rendernodes
        TreeNode* treeNode = level0->GetRenderNode();
        if (treeNode)
        {
            TreeContainer* parentNode = static_cast<TreeContainer*>(treeNode->GetParent());
            if (parentNode)
            {
                parentNode->Remove(0, parentNode->GetSize());
            }
        }
        level0 = NULL;

        // Clear vars.
        pMainMovie = NULL;   
        FrameTime    = 1.0f / 12.0f;        
        // Keep GetMovieDef() till next load so that users don't get null from GetMovieDef().
        //  SetMovieDef(NULL);
        //  pStateBag->SetDelegate(0);

        G_SetFlag<Flag_LevelClipsChanged>(Flags, true);

        return 1;
    }

    // Exhaustive for now; could do binary.
    for (unsigned i = 0; i < MovieLevels.GetSize(); i++)
    {
        if (MovieLevels[i].Level == level)
        {
            Ptr<InteractiveObject> plevel = MovieLevels[i].pSprite;

            // Inform old character of unloading.
            plevel->OnEventUnload();
            pASMovieRoot->DoActions();            

            // TBD: Is this right, or should local frames persist till level0 unload?
            plevel->ForceShutdown(); 

            // Remove us.
            MovieLevels.RemoveAt(i);

            G_SetFlag<Flag_LevelClipsChanged>(Flags, true);

            // delete the rendernode
            TreeNode* treeNode = plevel->GetRenderNode();
            if (treeNode)
            {
                TreeContainer* parentNode = static_cast<TreeContainer*>(treeNode->GetParent());
                if (parentNode)
                {
                    SF_ASSERT(parentNode->GetAt(i) == treeNode);
                    parentNode->Remove(i, 1);
                }
            }
            return 1;
        }
    }   
    return 0;
}

void    MovieImpl::Restart(bool advance0)                         
{       
//    pASMovieRoot->Restart();
    if (pMainMovie)
    {
        G_SetFlag<Flag2_Restarting>(Flags2, true); 
        ProcessUnloadQueue();

        Ptr<MovieDefImpl> prootMovieDef = pMainMovie->GetResourceMovieDef();

        // It is important to destroy the sprite before the global context,
        // so that is is not used from OnEvent(unload) in sprite destructor
        // NOTE: We clear the list here first because users can store pointers in _global,
        // which would cause pMovie assignment to not release it early (avoid "aeon.swf" crash).
        int i;
        for (i = (int)MovieLevels.GetSize() - 1; i >= 0; i--)
            ReleaseLevelMovie(i);
        // Release all refs.
        MovieLevels.Clear();

        // clean up threaded load queue before freeing
        LoadQueueEntryMT* plq = pLoadQueueMTHead;
        unsigned plqCount = 0;
        while (plq)
        {
            plqCount++;
            plq->Cancel();
            plq = plq->pNext;
        }
        // wait until all threaded loaders exit
        // this is required to avoid losing the movieroot
        // heap before the load tasks complete.
        unsigned plqDoneCount = 0;
        while (plqCount > plqDoneCount)
        {
            plqDoneCount = 0;
            plq = pLoadQueueMTHead;
            while (plq)
            {
                if (plq->LoadFinished())
                    plqDoneCount++;
                plq = plq->pNext;
            }
        }

        // free load queue
        while(pLoadQueueHead)
        {
            // Remove from queue.
            LoadQueueEntry *pentry = pLoadQueueHead;
            pLoadQueueHead = pentry->pNext;
            delete pentry;
        }
        while(pLoadQueueMTHead)
        {
            LoadQueueEntryMT *pentry = pLoadQueueMTHead;
            pLoadQueueMTHead = pentry->pNext;
            delete pentry;
        }
        pLoadQueueHead      = 0;
        pLoadQueueMTHead    = 0;

        InvalidateOptAdvanceList();
        pPlayListHead = pPlayListOptHead = NULL;

#ifndef SF_NO_IME_SUPPORT
        // clean up IME manager
        bool wasIMEActive = false;

        Ptr<IMEManagerBase> pIMEManager = GetIMEManager();
        if (pIMEManager)
        {
            if (pIMEManager->IsMovieActive(this))
            {
                wasIMEActive = true;
                pIMEManager->SetActiveMovie(NULL);
            }
        }

        // delete params of candidate list so they could be recreated
        delete pIMECandidateListStyle;
        pIMECandidateListStyle = NULL;

#endif //#ifdef SF_NO_IME_SUPPORT

#ifdef GFX_ENABLE_VIDEO
        VideoProviders.Clear();
#endif
#ifdef GFX_ENABLE_SOUND
        InteractiveObject* psprite = GetMainContainer();
        if (psprite)
            psprite->CharToSprite()->StopActiveSounds();
#endif
        //ReleaseUnloadList();
        pASMovieRoot->Shutdown();
        ClearIndirectTransformPairs();

        delete pRetValHolder;pRetValHolder = NULL;

        // Reset focus states
        ResetFocusStates();

        G_Set3WayFlag<Shift_DisableFocusAutoRelease>(Flags, 0);
        G_Set3WayFlag<Shift_AlwaysEnableFocusArrowKeys>(Flags, 0);
        G_Set3WayFlag<Shift_AlwaysEnableKeyboardPress>(Flags, 0);
        G_Set3WayFlag<Shift_DisableFocusRolloverEvent>(Flags, 0);
        G_Set3WayFlag<Shift_DisableFocusKeys>(Flags, 0);
        ResetMouseState();

        // Recreate global context

        // this collect should collect everything from old global context; do not
        // remove!
        pASMovieRoot->ForceCollect(Movie::GCF_Full);
        //pRetValHolder = SF_HEAP_NEW(pHeap) ReturnValueHolder(GetStringManager());

        G_SetFlag<Flag2_Restarting>(Flags2, false); 

        pASMovieRoot->Init(prootMovieDef);

        if (!pMainMovie)
        {
            return;
        }

        // reset mouse state and cursor
        if (pUserEventHandler)
        {
            // i change to i_inner: Fix Warning 4456
            for (unsigned i_inner = 0; i_inner < MouseCursorCount; ++i_inner)
            {
                pUserEventHandler->HandleEvent(this, MouseCursorEvent(Event::DoShowMouse, i_inner));
                pUserEventHandler->HandleEvent(this, MouseCursorEvent(MouseCursorEvent::ARROW, i_inner));
            }
        }

        FocusRectChanged = true;

        // reset keyboard state
        ResetKeyboardState();

#ifndef SF_NO_IME_SUPPORT
        if (wasIMEActive)
            pIMEManager->SetActiveMovie(this);
#endif //#ifdef SF_NO_IME_SUPPORT

        if (advance0)
            Advance(0.0f, 0);
        //SetDirtyFlag();

        pASMovieRoot->ForceCollect(Movie::GCF_Full);
    }
}

void    MovieImpl::ResetMouseState()
{
    for (unsigned i = 0; i < sizeof(mMouseState)/sizeof(mMouseState[0]); ++i)
    {
        mMouseState[i].ResetState();
    }
}

void    MovieImpl::ResetKeyboardState()
{
#ifdef GFX_ENABLE_KEYBOARD
    for (unsigned i = 0; i < sizeof(KeyboardStates)/sizeof(KeyboardStates[0]); ++i)
    {
        KeyboardStates[i].ResetState();
    }
#endif
}

#ifdef GFX_ENABLE_KEYBOARD
void    MovieImpl::SetKeyboardListener(KeyboardState::IListener* l)
{
    for (unsigned i = 0; i < sizeof(KeyboardStates)/sizeof(KeyboardStates[0]); ++i)
    {
        KeyboardStates[i].SetListener(l);
    }
}
#endif

void    MovieImpl::GotoFrame(unsigned targetFrameNumber) 
{   // 0-based!!
    if (pMainMovie) pMainMovie->GotoFrame(targetFrameNumber); 
}

bool    MovieImpl::GotoLabeledFrame(const char* label, int offset)
{
    if (!pMainMovie)
        return 0;

    unsigned    targetFrame = SF_MAX_UINT;
    if (pMainMovieDef->GetDataDef()->GetLabeledFrame(label, &targetFrame, 0))
    {
        GotoFrame((unsigned)((int)targetFrame + offset));
        return 1;
    }
    else
    {
        Ptr<LogState> log = GetLogState();
        if (log)
            log->LogScriptError("MovieImpl::GotoLabeledFrame('%s') unknown label", label);
        return 0;
    }
}

void    MovieImpl::SetPause(bool pause)
{
    if ((G_IsFlagSet<Flag_Paused>(Flags) && pause) || (!G_IsFlagSet<Flag_Paused>(Flags) && !pause))
        return;
    G_SetFlag<Flag_Paused>(Flags, pause);
    if (pause)
        PauseTickMs = Timer::GetTicks()/1000;
    else
        StartTickMs += (Timer::GetTicks()/1000 - PauseTickMs); 

    InteractiveObject* pnextCh = NULL;
    for (InteractiveObject* pcurCh = pPlayListHead; pcurCh; pcurCh = pnextCh)
    {
        pnextCh = pcurCh->pPlayNext;
        pcurCh->SetPause(pause);
    }
#ifdef GFX_ENABLE_VIDEO
    if (VideoProviders.GetSize() > 0)
    {
        for(HashSet<Ptr<Video::VideoProvider> >::Iterator it = VideoProviders.Begin();
                                                          it != VideoProviders.End(); ++it)
        {
            Ptr<Video::VideoProvider> pvideo = *it;
            pvideo->Pause(pause);
        }
    }
#endif
}

void    MovieImpl::SetPlayState(PlayState s)
{   
    if (pMainMovie)
        pMainMovie->SetPlayState(s); 
}
PlayState MovieImpl::GetPlayState() const
{   
    if (!pMainMovie) return GFx::State_Stopped;
    return pMainMovie->GetPlayState(); 
}

void    MovieImpl::SetVisible(bool visible)
{
    if (pMainMovie) pMainMovie->SetVisible(visible);
}
bool    MovieImpl::GetVisible() const
{
    return pMainMovie ? pMainMovie->GetVisible() : 0;
}

void MovieImpl::AddStickyVariableNode(const ASString& path, StickyVarNode* p)
{
    SF_ASSERT(p);
    MovieImpl::StickyVarNode* pnode = 0;

    if (StickyVariables.Get(path, &pnode) && pnode)
    {
        MovieImpl::StickyVarNode* pcur  = pnode;
        //MovieImpl::StickyVarNode* pprev = NULL;

        while (pcur)
        {
            // If there is a name match, replace the node.
            if (pcur->Name == p->Name)
            {
                pcur->Assign(*p);
                delete p;
                return;
            }
            //pprev = pcur;
            pcur = pcur->pNext;
        };

        // Not found, add new node
        // Link prev node to us: order does not matter.
        p->pNext = pnode->pNext;
        pnode->pNext = p;
        
        // Done.
        return;
    }

    // Save node.
    StickyVariables.Set(path, p);
}
// 
// void    MovieImpl::ResolveStickyVariables(InteractiveObject *pcharacter)
// {
//     pASMovieRoot->ResolveStickyVariables(pcharacter);
// }

void    MovieImpl::ClearStickyVariables()
{
    ASStringHash<StickyVarNode*>::Iterator ihash = StickyVariables.Begin();

    // Travers hash and delete nodes.
    for( ;ihash != StickyVariables.End(); ++ihash)
    {
        StickyVarNode* pnode = ihash->Second;
        StickyVarNode* p;

        while (pnode)
        {
            p = pnode;
            pnode = p->pNext;
            delete p;
        }
    }

    StickyVariables.Clear();
}

void        MovieImpl::OnMovieFocus(bool set)
{
#ifdef GFX_ENABLE_KEYBOARD
    if (!set)
    {
        ResetKeyboardState();
        ResetMouseState();
    }
#endif //#ifdef GFX_ENABLE_KEYBOARD
    G_SetFlag<Flag_MovieIsFocused>(Flags, set);
#ifndef SF_NO_IME_SUPPORT
    Ptr<IMEManagerBase> pIMEManager = GetIMEManager();
    if (pIMEManager)
    {
        pIMEManager->SetActiveMovie(set?this:NULL);
    }
#endif //#ifdef SF_NO_IME_SUPPORT
    pASMovieRoot->OnMovieFocus(set);
}

void MovieImpl::ResetTabableArrays()
{
    for (unsigned i = 0; i < FocusGroupsCnt; ++i)
    {
        FocusGroups[i].ResetTabableArray();
    }
}

void MovieImpl::ResetFocusStates()
{
    for (unsigned i = 0; i < FocusGroupsCnt; ++i)
    {
        FocusGroups[i].ModalClip.Clear();
        FocusGroups[i].ResetTabableArray();
        FocusGroups[i].LastFocusKeyCode = 0;
    }
}

bool MovieImpl::SetControllerFocusGroup(unsigned controllerIdx, unsigned focusGroupIndex)
{
    if (controllerIdx >= GFX_MAX_CONTROLLERS_SUPPORTED || 
        focusGroupIndex >= GFX_MAX_CONTROLLERS_SUPPORTED)
        return false;
    FocusGroupIndexes[controllerIdx] = (UInt8)focusGroupIndex;
    if (focusGroupIndex >= FocusGroupsCnt)
        FocusGroupsCnt = focusGroupIndex + 1;
    return true;
}

unsigned MovieImpl::GetControllerFocusGroup(unsigned controllerIdx) const
{
    if (controllerIdx >= GFX_MAX_CONTROLLERS_SUPPORTED)
        return false;
    return FocusGroupIndexes[controllerIdx];
}

void MovieImpl::FillTabableArray(const ProcessFocusKeyInfo* pfocusInfo)
{
    SF_ASSERT(pfocusInfo);

    FocusGroupDescr& focusGroup = *pfocusInfo->pFocusGroup;
    if (pfocusInfo->InclFocusEnabled && 
        (focusGroup.TabableArrayStatus & FocusGroupDescr::TabableArray_Initialized) && 
        !(focusGroup.TabableArrayStatus & FocusGroupDescr::TabableArray_WithFocusEnabled))
        focusGroup.ResetTabableArray(); // need to refill array with focusEnabled chars
    if (!(focusGroup.TabableArrayStatus & FocusGroupDescr::TabableArray_Initialized))
    {
        Sprite* modalClip       = NULL;
        //bool tabIndexed         = false;
        InteractiveObject::FillTabableParams p;
        p.Array = &focusGroup.TabableArray;
        p.InclFocusEnabled = pfocusInfo->InclFocusEnabled;
        if ((modalClip = focusGroup.GetModalClip(this)) == NULL)
        {
            // fill array by focusable characters
            for (int movieIndex = (int)MovieLevels.GetSize(); movieIndex > 0; movieIndex--)
            {
                if (MovieLevels[movieIndex-1].pSprite->IsDisplayObjContainer())
                MovieLevels[movieIndex-1].pSprite->CharToDisplayObjContainer_Unsafe()->FillTabableArray(&p);
            }
        }
        else
        {
            // fill array by focusable characters but only by children of ModalClip
            modalClip->FillTabableArray(&p);
        }

        if (p.TabIndexed)
        {
            // sort by tabIndex if tabIndexed == true
            static TabIndexSortFunctor sf;
            Alg::QuickSort(focusGroup.TabableArray, sf);
        }
        else
        {
            // sort for automatic order
            static AutoTabSortFunctor sf;
            Alg::QuickSort(focusGroup.TabableArray, sf);
        }
        focusGroup.TabableArrayStatus = FocusGroupDescr::TabableArray_Initialized;
        if (pfocusInfo->InclFocusEnabled)
            focusGroup.TabableArrayStatus |= FocusGroupDescr::TabableArray_WithFocusEnabled;
    }
}

// Focus management
void MovieImpl::InitFocusKeyInfo(ProcessFocusKeyInfo* pfocusInfo, 
                                 const InputEventsQueue::QueueEntry::KeyEntry& keyEntry,
                                 bool inclFocusEnabled,
                                 FocusGroupDescr* pfocusGroup)
{
    if (!pfocusInfo->Initialized)
    {
        FocusGroupDescr& focusGroup = (!pfocusGroup) ? GetFocusGroup(keyEntry.KeyboardIndex) : *pfocusGroup;
        pfocusInfo->pFocusGroup     = &focusGroup;
        pfocusInfo->PrevKeyCode     = focusGroup.LastFocusKeyCode;
        pfocusInfo->Prev_aRect      = focusGroup.LastFocusedRect;
        pfocusInfo->InclFocusEnabled= inclFocusEnabled;
        pfocusInfo->ManualFocus     = false;
        pfocusInfo->KeyboardIndex   = keyEntry.KeyboardIndex;
        pfocusInfo->KeyCode         = keyEntry.Code;
        pfocusInfo->KeysState       = keyEntry.KeysState;

        FillTabableArray(pfocusInfo);

        //@DBG Debug code, do not remove!
        //for(int ii = 0; ii < tabableArraySize; ++ii)
        //{
        //printf("Focus[%d] = %s\n", ii, pfocusInfo->TabableArray[ii]->GetCharacterHandle()->GetNamePath().ToCStr());
        //}

        pfocusInfo->CurFocusIdx = -1;
        pfocusInfo->CurFocused  = focusGroup.LastFocused;
        if (pfocusInfo->CurFocused)
        {
            // find the index of currently focused
            for (UPInt i = 0; i < focusGroup.TabableArray.GetSize(); ++i)
            {
                if (focusGroup.TabableArray[i] == pfocusInfo->CurFocused)
                {
                    pfocusInfo->CurFocusIdx = (int)i;
                    break;
                }
            }
        }
        pfocusInfo->Initialized = true;
    }
}

void MovieImpl::ProcessFocusKey(Event::EventType event, 
                                const InputEventsQueue::QueueEntry::KeyEntry& keyEntry, 
                                ProcessFocusKeyInfo* pfocusInfo)
{
    if (event == Event::KeyDown)
    {
        if (keyEntry.Code == Key::Tab || 
            ((IsFocusRectShown(keyEntry.KeyboardIndex) || pfocusInfo->ManualFocus) && 
            (keyEntry.Code == Key::Left || keyEntry.Code == Key::Right || 
             keyEntry.Code == Key::Up || keyEntry.Code == Key::Down)))
        {
            // init focus info if it is not initialized yet
            InitFocusKeyInfo(pfocusInfo, keyEntry, false);
            const FocusGroupDescr& focusGroup = *pfocusInfo->pFocusGroup;
            int tabableArraySize = (int)focusGroup.TabableArray.GetSize();
            if (keyEntry.Code == Key::Tab)
            {
                int cnt;
                int curFocusIdx = pfocusInfo->CurFocusIdx;
                pfocusInfo->CurFocusIdx = -1;
                for (cnt = 0; cnt < tabableArraySize; ++cnt)
                {
                    if (KeyModifiers(keyEntry.KeysState).IsShiftPressed())
                    {
                        if (--curFocusIdx < 0)  
                            curFocusIdx = tabableArraySize-1;
                    }
                    else
                    {
                        if (++curFocusIdx >= tabableArraySize)
                            curFocusIdx = 0;
                    }
                    // check if candidate is tabable, or if manual focus mode is on. Break, if so.
                    if (focusGroup.TabableArray[curFocusIdx] && 
                        (pfocusInfo->InclFocusEnabled || focusGroup.TabableArray[curFocusIdx]->IsTabable()) &&
                        focusGroup.TabableArray[curFocusIdx]->IsFocusAllowed(this, pfocusInfo->KeyboardIndex))
                    {
                        pfocusInfo->CurFocusIdx = curFocusIdx;
                        break;
                    }
                } 
                //SetDirtyFlag();
            }
            else if (pfocusInfo->CurFocused)
            {
                if (pfocusInfo->CurFocused->IsFocusRectEnabled() || IsAlwaysEnableFocusArrowKeys() || pfocusInfo->ManualFocus)
                {
                    DisplayObjectBase::Matrix ma = pfocusInfo->CurFocused->GetLevelMatrix();
                    // aRect represents the rectangle of currently focused character.
                    RectF aRect  = ma.EncloseTransform(pfocusInfo->CurFocused->GetFocusRect());

                    // We need to adjust the existing source rectangle (aRect). If we have
                    // saved previous rectangle and the direction if focus moving is the same
                    // (for example, we pressed the up/down arrow key again), then we will use
                    // x1/x2 coordinates from the saved rectangle to maintain the same Y-axis.
                    // Same will happen if we move right/left, we will use original y1/y2
                    // to keep X-axis.
                    if (pfocusInfo->PrevKeyCode == keyEntry.Code)
                    {
                        if (keyEntry.Code == Key::Up || keyEntry.Code == Key::Down)
                        {
                            aRect.x1      = pfocusInfo->Prev_aRect.x1;
                            aRect.x2     = pfocusInfo->Prev_aRect.x2;
                        }
                        else if (keyEntry.Code == Key::Right || keyEntry.Code == Key::Left)
                        {
                            aRect.y1       = pfocusInfo->Prev_aRect.y1;
                            aRect.y2    = pfocusInfo->Prev_aRect.y2;
                        }
                    }
                    else
                    {
                        pfocusInfo->Prev_aRect  = aRect;
                        pfocusInfo->PrevKeyCode = keyEntry.Code;
                    }

                    if (keyEntry.Code == Key::Right || keyEntry.Code == Key::Left)
                    {
                        //SetDirtyFlag();
                        int newFocusIdx = pfocusInfo->CurFocusIdx;
                        RectF newFocusRect;
                        newFocusRect.x1 = newFocusRect.x2 = (keyEntry.Code == Key::Right) ? float(INT_MAX) : float(INT_MIN);
                        newFocusRect.y1 = newFocusRect.y2 = float(INT_MAX);
                        // find nearest from right or left side

                        bool hitStipe = false;
                        for (int i = 0; i < tabableArraySize - 1; ++i)
                        {
                            newFocusIdx = (keyEntry.Code == Key::Right) ? newFocusIdx + 1 : newFocusIdx - 1;
                            if (newFocusIdx >= tabableArraySize)
                                newFocusIdx = 0;
                            else if (newFocusIdx < 0)
                                newFocusIdx = tabableArraySize - 1;
                            Ptr<InteractiveObject> b = focusGroup.TabableArray[newFocusIdx];
                            if ((!pfocusInfo->InclFocusEnabled && !b->IsTabable()) ||
                                !b->IsFocusAllowed(this, pfocusInfo->KeyboardIndex))
                            {
                                // If this is not for manual focus and not tabable - ignore.
                                continue;
                            }
                            DisplayObjectBase::Matrix mb = b->GetLevelMatrix();
                            RectF bRect  = mb.EncloseTransform(b->GetFocusRect());

                            bool  curHitStripe    = false;
                            // check the "stripe zone"

                            RectF stripeRect;
                            if (keyEntry.Code == Key::Right)
                                stripeRect = RectF(aRect.x2+1, aRect.y1, SF_MAX_FLOAT, aRect.y2);
                            else  // x1
                                stripeRect = RectF(SF_MIN_FLOAT, aRect.y1, aRect.x1-1, aRect.y2);
                            // Check, if the current character ("b") is in the stripe zone or not.
                            if (bRect.Intersects(stripeRect))
                            {
                                RectF intersectionRect = bRect;
                                intersectionRect.Intersect(stripeRect);
                                //@DBG
                                //printf("Intersection height is %f\n", intersectionRect.Height());
                                if (intersectionRect.Height() >= 40) // 2 pixels threshold
                                    curHitStripe    = true;
                            }
                            if (curHitStripe)
                            {
                                if (!hitStipe)
                                { // first hit - save the current char index and rect
                                    pfocusInfo->CurFocusIdx = newFocusIdx;
                                    newFocusRect            = bRect;
                                    hitStipe                = true;

                                    //@DBG
                                    //printf("InitFocus = %s\n",  
                                    //    b->GetCharacterHandle()->GetNamePath().ToCStr());
                                    continue;
                                }
                            }
                            // if we already hit stripe once - ignore all chars NOT in the stripe zone
                            if (hitStipe && !curHitStripe)
                                continue;

                            PointF vector1, vector2;
                            // lets calculate the distance from the adjusted rectangle of currently
                            // focused character to the character currently being iterated, and compare
                            // the distance with the distance to existing candidate.
                            // The distance being calculated is from the middle of the right side (if "right" 
                            // key is down) to the middle of the left side, or other way around.
                            if (keyEntry.Code == Key::Right)
                            {
                                PointF ptBeg(aRect.x2, aRect.y1 + aRect.Height()/2);
                                RectF brect(bRect), newrect(newFocusRect);
                                brect.HClamp  (aRect.x2, bRect.x2);
                                newrect.HClamp(aRect.x2, newFocusRect.x2);
                                if (!brect.IsNormal() || Alg::IRound(TwipsToPixels(brect.Width())) <= 3) // threshold 3 pixels
                                    continue;
                                PointF ptEnd1(brect.x1, bRect.y1 + bRect.Height()/2);
                                PointF ptEnd2(newrect.x1, newFocusRect.y1 + newFocusRect.Height()/2);

                                vector1.x = floorf(TwipsToPixels(ptEnd1.x - ptBeg.x)); 
                                vector1.y = floorf(TwipsToPixels(ptEnd1.y - ptBeg.y));
                                vector2.x = floorf(TwipsToPixels(ptEnd2.x - ptBeg.x)); 
                                vector2.y = floorf(TwipsToPixels(ptEnd2.y - ptBeg.y));
                                if (vector1.x < 0) // negative, means it is not at the right
                                    continue;
                            }
                            else // left
                            {
                                PointF ptBeg(aRect.x1, aRect.y1 + aRect.Height()/2);
                                RectF brect(bRect), newrect(newFocusRect);
                                brect.HClamp  (bRect.x1, aRect.x1);
                                newrect.HClamp(newFocusRect.x1, aRect.x1);
                                if (!brect.IsNormal() || Alg::IRound(TwipsToPixels(brect.Width())) <= 3) // threshold 3 pixels
                                    continue;
                                PointF ptEnd1(brect.x2, bRect.y1 + bRect.Height()/2);
                                PointF ptEnd2(newrect.x2, newFocusRect.y1 + newFocusRect.Height()/2);

                                vector1.x = floorf(TwipsToPixels(ptEnd1.x - ptBeg.x)); 
                                vector1.y = floorf(TwipsToPixels(ptEnd1.y - ptBeg.y));
                                vector2.x = floorf(TwipsToPixels(ptEnd2.x - ptBeg.x)); 
                                vector2.y = floorf(TwipsToPixels(ptEnd2.y - ptBeg.y));
                                if (vector1.x > 0) // positive, means it is not at the left
                                    continue;
                            }
                            //@DBG
                            //printf("Checking for %s, vec1(%d,%d), vec2(%d,%d)\n",  
                            //    b->GetCharacterHandle()->GetNamePath().ToCStr(),
                            //    (int)vector1.x, (int)vector1.y, (int)vector2.x, (int)vector2.y);
                            // Check, if the character in the "stripe-zone". If yes - check, is the new char
                            // closer by the 'x' coordinate or not. If 'x' coordinate is the same - check if it is 
                            // close by the 'y' coordinate. Update 'newFocus' item if so.
                            // If stripe is not hit, then just measure the distance to the new char and
                            // update the 'newFocus' item if it is closer than previous one.
                            if ((hitStipe && 
                                 (Alg::Abs(vector1.x) < Alg::Abs(vector2.x) || (vector1.x == vector2.x && Alg::Abs(vector1.y) < Alg::Abs(vector2.y)))) || 
                                (!hitStipe &&
                                (vector1.x*vector1.x + vector1.y*vector1.y < 
                                vector2.x*vector2.x + vector2.y*vector2.y)))
                            {
                                //@DBG
                                //printf("   newFocus = %s, vec1(%d,%d), vec2(%d,%d)\n",  
                                //    b->GetCharacterHandle()->GetNamePath().ToCStr(),
                                //    (int)vector1.x, (int)vector1.y, (int)vector2.x, (int)vector2.y);
                                pfocusInfo->CurFocusIdx = newFocusIdx;
                                newFocusRect            = bRect;
                            }
                        }
                    }
                    else if (keyEntry.Code == Key::Up || keyEntry.Code == Key::Down)
                    {
                        //SetDirtyFlag();
                        int newFocusIdx = pfocusInfo->CurFocusIdx;
                        RectF newFocusRect(0);
                        newFocusRect.x1 = float(INT_MAX);
                        newFocusRect.y1 = (keyEntry.Code == Key::Down) ? float(INT_MAX) : float(INT_MIN);
                        // find nearest from top and bottom side
                        // The logic is as follows:
                        // 1. The highest priority characters are ones, which boundary rectangles intersect with the "stripe zone" 
                        // above or below the currently selected character. I.e. for Down key the "stripe zone" will be 
                        // ((aRect.x1, aRect.x2), (aRect.y2, Infinity)).
                        //   a) if there are more than one characters in the "stripe zone", then the best candidate should
                        //      have shortest distance by Y axis.
                        // 2. Otherwise, the closest character will be chosen by comparing Y-distance and only then X-distance.
                        bool hitStipe = false;

                        for (int i = 0; i < tabableArraySize - 1; ++i)
                        {
                            newFocusIdx = (keyEntry.Code == Key::Down) ? newFocusIdx + 1 : newFocusIdx - 1;
                            if (newFocusIdx >= tabableArraySize)
                                newFocusIdx = 0;
                            else if (newFocusIdx < 0)
                                newFocusIdx = tabableArraySize - 1;
                            Ptr<InteractiveObject> b  = focusGroup.TabableArray[newFocusIdx];
                            if ((!pfocusInfo->InclFocusEnabled && !b->IsTabable()) ||
                                !b->IsFocusAllowed(this, pfocusInfo->KeyboardIndex))
                            {
                                // If this is not for manual focus and not tabable - ignore.
                                continue;
                            }
                            DisplayObjectBase::Matrix mb = b->GetLevelMatrix();
                            RectF bRect  = mb.EncloseTransform(b->GetFocusRect());

                            bool  curHitStripe    = false;
                            // check the "stripe zone"
                            RectF stripeRect;
                            if (keyEntry.Code == Key::Down)
                                stripeRect = RectF(aRect.x1, aRect.y2 + 1, aRect.x2, SF_MAX_FLOAT);
                            else // Up
                                stripeRect = RectF(aRect.x1, SF_MIN_FLOAT, aRect.x2, aRect.y1 - 1);
                            //@DBG
                            //printf("bRect = %s\n",  b->GetCharacterHandle()->GetNamePath().ToCStr());
                            if (bRect.Intersects(stripeRect))
                            {
                                RectF intersectionRect = bRect;
                                intersectionRect.Intersect(stripeRect);
                                //@DBG
                                //printf("Intersection width is %f\n", intersectionRect.Width());
                                if (intersectionRect.Width() >= 40) // 2 pixels threshold
                                    curHitStripe    = true;
                            }
                            //@DBG
                            //printf ("curHitStripe is %d\n", (int)curHitStripe);
                            if (curHitStripe)
                            {
                                if (!hitStipe)
                                { // first hit - save the current char index and rect
                                    pfocusInfo->CurFocusIdx = newFocusIdx;
                                    newFocusRect = bRect;
                                    hitStipe = true;

                                    //@DBG
                                    //printf("InitFocus = %s\n",  
                                    //    b->GetCharacterHandle()->GetNamePath().ToCStr());
                                    continue;
                                }
                            }
                            // if we already hit stripe once - ignore all chars NOT in the stripe zone
                            if (hitStipe && !curHitStripe)
                                continue;

                            PointF vector1, vector2;
                            // lets calculate the distance from the adjusted rectangle of currently
                            // focused character to the character currently being iterated, and compare
                            // the distance with the distance to existing candidate.
                            // The distance being calculated is from the middle of the top side (if "up" 
                            // key is down) to the middle of the bottom side, or other way around.
                            if (keyEntry.Code == Key::Up)
                            {
                                PointF ptBeg(aRect.x1 + aRect.Width()/2, aRect.y1);
                                RectF brect(bRect), newrect(newFocusRect);
                                brect.VClamp  (bRect.y1, aRect.y1);
                                newrect.VClamp(newFocusRect.y1, aRect.y1);
                                if (!brect.IsNormal() || Alg::IRound(TwipsToPixels(brect.Height())) <= 3) // threshold 3 pixels
                                    continue;
                                PointF ptEnd1(brect.x1 + brect.Width()/2, brect.y2);
                                PointF ptEnd2(newrect.x1 + newrect.Width()/2, newrect.y2);
                                
                                vector1.x = floorf(TwipsToPixels(ptEnd1.x - ptBeg.x)); 
                                vector1.y = floorf(TwipsToPixels(ptEnd1.y - ptBeg.y));
                                vector2.x = floorf(TwipsToPixels(ptEnd2.x - ptBeg.x)); 
                                vector2.y = floorf(TwipsToPixels(ptEnd2.y - ptBeg.y));
                                if (vector1.y > 0) // positive, means it is not at the top
                                    continue;
                            }
                            else // down
                            {
                                PointF ptBeg(aRect.x1 + aRect.Width()/2, aRect.y2);
                                RectF brect(bRect), newrect(newFocusRect);
                                brect.VClamp  (aRect.y2, bRect.y2);
                                newrect.VClamp(aRect.y2, newFocusRect.y2);
                                if (!brect.IsNormal() || Alg::IRound(TwipsToPixels(brect.Height())) <= 3) // threshold 3 pixels
                                    continue;
                                PointF ptEnd1(brect.x1 + brect.Width()/2, brect.y1);
                                PointF ptEnd2(newrect.x1 + newrect.Width()/2, newrect.y1);
                                
                                vector1.x = floorf(TwipsToPixels(ptEnd1.x - ptBeg.x)); 
                                vector1.y = floorf(TwipsToPixels(ptEnd1.y - ptBeg.y));
                                vector2.x = floorf(TwipsToPixels(ptEnd2.x - ptBeg.x)); 
                                vector2.y = floorf(TwipsToPixels(ptEnd2.y - ptBeg.y));
                                if (vector1.y < 0) // negative, means it is not at the down
                                    continue;
                            }
                            //@DBG
                            //printf("Checking for %s, vec1(%d,%d), vec2(%d,%d)\n",  
                            //    b->GetCharacterHandle()->GetNamePath().ToCStr(),
                            //    (int)vector1.x, (int)vector1.y, (int)vector2.x, (int)vector2.y);
                            // Check, if the character in the "stripe-zone". If yes - check, is the new char
                            // closer by the 'y' coordinate or not. If 'x' coordinate is the same - check if it is 
                            // close by the 'x' coordinate. Update 'newFocus' item if so.
                            // If stripe is not hit, then just measure the distance to the new char and
                            // update the 'newFocus' item if it is closer than previous one.
                            if ((hitStipe && 
                                (Alg::Abs(vector1.y) < Alg::Abs(vector2.y) || (vector1.y == vector2.y && Alg::Abs(vector1.x) < Alg::Abs(vector2.x)))) || 
                                (!hitStipe &&
                                (vector1.x*vector1.x + vector1.y*vector1.y < 
                                vector2.x*vector2.x + vector2.y*vector2.y)))
                            {
                                //@DBG
                                //printf("   newFocus = %s, vec1(%d,%d), vec2(%d,%d)\n",  
                                //    b->GetCharacterHandle()->GetNamePath().ToCStr(),
                                //    (int)vector1.x, (int)vector1.y, (int)vector2.x, (int)vector2.y);
                                pfocusInfo->CurFocusIdx = newFocusIdx;
                                newFocusRect = bRect;
                            }
                        }
                    }
                }
            }
            if (pfocusInfo->CurFocusIdx >= 0 && pfocusInfo->CurFocusIdx < tabableArraySize)
            {
                pfocusInfo->CurFocused = focusGroup.TabableArray[pfocusInfo->CurFocusIdx];
            }
            else
            {
                pfocusInfo->CurFocused = NULL;
            }
        }
    }
}

void MovieImpl::FinalizeProcessFocusKey(ProcessFocusKeyInfo* pfocusInfo)
{
    FocusGroupDescr& focusGroup = GetFocusGroup(pfocusInfo->KeyboardIndex);
    if (pfocusInfo && pfocusInfo->Initialized && 
        (focusGroup.TabableArrayStatus & FocusGroupDescr::TabableArray_Initialized))
    {
        InteractiveObject* psetFocusToCh;
        if (pfocusInfo->CurFocusIdx >= 0 && pfocusInfo->CurFocusIdx < (int)focusGroup.TabableArray.GetSize())
            psetFocusToCh = focusGroup.TabableArray[pfocusInfo->CurFocusIdx];
        else
        {
            // if CurFocusIdx is out of the TabableArray then do nothing. This may happen
            // when TabableArray is empty; in this case we do not need to transfer focus
            // anywhere.
            return; 
        }

        Ptr<InteractiveObject> lastFocused = focusGroup.LastFocused;

        if (lastFocused != psetFocusToCh)
        {
            // keep tracking direction of focus movement
            focusGroup.LastFocusKeyCode = pfocusInfo->PrevKeyCode;
            focusGroup.LastFocusedRect  = pfocusInfo->Prev_aRect;

            QueueSetFocusTo(psetFocusToCh, NULL, pfocusInfo->KeyboardIndex, 
                GFx_FocusMovedByKeyboard, pfocusInfo);
        }
        if (!psetFocusToCh || psetFocusToCh->GetType() != CharacterDef::TextField)
        {
            if (!focusGroup.FocusRectShown)
                FocusRectChanged = true;
            focusGroup.FocusRectShown = true;
        }
        else
        {
            if (focusGroup.FocusRectShown)
                FocusRectChanged = true;
            focusGroup.FocusRectShown = false;
        }
        //SetDirtyFlag();
    }
}

void MovieImpl::ActivateFocusCapture(unsigned controllerIdx)
{
    ProcessFocusKeyInfo focusKeyInfo;
    InputEventsQueue::QueueEntry::KeyEntry keyEntry;
    keyEntry.Code = Key::Tab;
    keyEntry.KeysState = 0;
    keyEntry.KeyboardIndex = (UInt8)controllerIdx;
    ProcessFocusKey(KeyEvent::KeyDown, keyEntry, &focusKeyInfo);
    FinalizeProcessFocusKey(&focusKeyInfo);
}

void MovieImpl::SetModalClip(Sprite* pmovie, unsigned controllerIdx)
{
    FocusGroupDescr& focusGroup = GetFocusGroup(controllerIdx);

    if (!pmovie)
        focusGroup.ModalClip = NULL;
    else
        focusGroup.ModalClip = pmovie->GetCharacterHandle();
}

Sprite* MovieImpl::GetModalClip(unsigned controllerIdx)
{
    FocusGroupDescr& focusGroup = GetFocusGroup(controllerIdx);
    return focusGroup.GetModalClip(this);
}

UInt32 MovieImpl::GetControllerMaskByFocusGroup(unsigned focusGroupIndex)
{
    UInt32 v = 0;
    for (unsigned f = 0, mask = 0x1; f < GFX_MAX_CONTROLLERS_SUPPORTED; ++f, mask <<= 1)
    {
        if (FocusGroupIndexes[f] == focusGroupIndex)
            v |= mask;
    }
    return v;
}

void MovieImpl::UpdateFocusRectRenderNodes()
{
    SF_ASSERT(FocusRectChanged);

    if (pMainMovie)
    {
        // check if TreeContainer already exists
        if (!FocusRectContainerNode)
        {
            // create one
            FocusRectContainerNode = *GetRenderContext().CreateEntry<Render::TreeContainer>();
            GetRenderRoot()->Add(FocusRectContainerNode);
        }
        else
        {
            // if exists - clear all the nodes
            FocusRectContainerNode->Remove(0, FocusRectContainerNode->GetSize());
        }
        for (unsigned f = 0; f < FocusGroupsCnt; ++f)
        {
            Ptr<InteractiveObject> curFocused = FocusGroups[f].LastFocused;
            if (curFocused  && FocusGroups[f].FocusRectShown && curFocused->IsFocusRectEnabled())
            {
                // TODO: it still doesn't work correctly in 3D.
                Matrix3F                m3d;
                RectF focusLocalRect = curFocused->GetFocusRect();

                if (focusLocalRect.IsNull())
                    return;

                m3d = curFocused->GetWorldMatrix3D();

                RectF focusWorldRect = m3d.EncloseTransform(focusLocalRect);
                focusWorldRect.Expand(f*20.f, f*20.f);

                // create a shape
                Ptr<Render::ShapeDataFloat> shapeData = *SF_HEAP_NEW(GetRenderContext().GetHeap()) Render::ShapeDataFloat();
                Render::FillStyleType fill;

                UInt32 colorValue = 0xFFFF00;
                colorValue ^= (f * 0x1080D0);
                colorValue |= 0xFF000000; // alpha
                fill.Color = colorValue;

                shapeData->AddFillStyle(fill);
                shapeData->StartPath(1, 0, 0);
                shapeData->MoveTo(focusWorldRect.x1, focusWorldRect.y1);
                shapeData->LineTo(focusWorldRect.x2, focusWorldRect.y1);
                shapeData->LineTo(focusWorldRect.x2, focusWorldRect.y1 + 20);
                shapeData->LineTo(focusWorldRect.x1, focusWorldRect.y1 + 20);
                shapeData->ClosePath();
                shapeData->EndPath();

                shapeData->StartPath(1, 0, 0);
                shapeData->MoveTo(focusWorldRect.x2, focusWorldRect.y1);
                shapeData->LineTo(focusWorldRect.x2, focusWorldRect.y2);
                shapeData->LineTo(focusWorldRect.x2 - 20, focusWorldRect.y2);
                shapeData->LineTo(focusWorldRect.x2 - 20, focusWorldRect.y1);
                shapeData->ClosePath();
                shapeData->EndPath();

                shapeData->StartPath(1, 0, 0);
                shapeData->MoveTo(focusWorldRect.x2, focusWorldRect.y2);
                shapeData->LineTo(focusWorldRect.x1, focusWorldRect.y2);
                shapeData->LineTo(focusWorldRect.x1, focusWorldRect.y2 - 20);
                shapeData->LineTo(focusWorldRect.x2, focusWorldRect.y2 - 20);
                shapeData->ClosePath();
                shapeData->EndPath();

                shapeData->StartPath(1, 0, 0);
                shapeData->MoveTo(focusWorldRect.x1, focusWorldRect.y2);
                shapeData->LineTo(focusWorldRect.x1, focusWorldRect.y1);
                shapeData->LineTo(focusWorldRect.x1 + 20, focusWorldRect.y1);
                shapeData->LineTo(focusWorldRect.x1 + 20, focusWorldRect.y2);
                shapeData->ClosePath();
                shapeData->EndPath();

                shapeData->EndShape();
                Ptr<ShapeMeshProvider> pshapeMeshProvider = *SF_HEAP_NEW(GetRenderContext().GetHeap()) ShapeMeshProvider(shapeData.GetPtr());

                Ptr<Render::TreeShape> tshp = *GetRenderContext().CreateEntry<Render::TreeShape>();
                tshp->SetShape(pshapeMeshProvider);
                
                FocusRectContainerNode->Add(tshp);
                FocusGroups[f].FocusRectNode = tshp;
            }
        }
        FocusRectChanged = false;
    }
}

void MovieImpl::HideFocusRect(unsigned controllerIdx)
{
    FocusGroupDescr& focusGroup = GetFocusGroup(controllerIdx);
    if (focusGroup.FocusRectShown)
    {
        Ptr<InteractiveObject> curFocused = focusGroup.LastFocused;
        if (curFocused && curFocused->GetParent())
            if (!curFocused->OnLosingKeyboardFocus(NULL, controllerIdx))
                return; // focus loss was prevented
    }
    focusGroup.FocusRectShown = false;
    FocusRectChanged = true;
}

bool MovieImpl::SetFocusTo(InteractiveObject* ch, unsigned controllerIdx, FocusMovedType fmt)
{
    // the order of events, if Selection.setFocus is invoked is as follows:
    // Instantly:
    // 1. curFocus.onKillFocus, curFocus = oldFocus
    // 2. curFocus = newFocus
    // 3. curFocus.onSetFocus, curFocus = newFocus
    // 4. Selection focus listeners, curFocus = newFocus
    // Queued:
    // 5. oldFocus.onRollOut, curFocus = newFocus
    // 6. newFocus.onRollOver, curFocus = newFocus
    FocusGroupDescr& focusGroup = GetFocusGroup(controllerIdx);
    Ptr<InteractiveObject> curFocused = focusGroup.LastFocused;

    if (curFocused == ch) return false;

    FocusRectChanged = true;

    //?if (!pASMovieRoot->NotifyOnFocusChange(curFocused, ch, controllerIdx, fmt))
    //?    return false;

    if (curFocused && curFocused->GetParent())
    {
        // queue onRollOut, step 5 (since it is queued up, we may do it before TransferFocus)
        if (!curFocused->OnLosingKeyboardFocus(ch, controllerIdx, fmt))
            return false; // focus loss was prevented
    }

    // Do instant focus transfer (steps 1-4)
    TransferFocus(ch, controllerIdx, fmt);

    // invoke onSetFocus for newly set LastFocused
    if (ch)
    {
        // queue onRollOver, step 6
        ch->OnGettingKeyboardFocus(controllerIdx,  fmt);
    }
    return true;
}

void MovieImpl::ResetInputFocus(unsigned controllerIdx) 
{
    SetFocusTo(NULL, controllerIdx, GFx_FocusMovedByAS);
}

bool MovieImpl::QueueSetFocusTo(InteractiveObject* ch, InteractiveObject* ptopMostCh, 
                                unsigned controllerIdx, FocusMovedType fmt,
                                ProcessFocusKeyInfo* pfocusKeyInfo)
{
    // the order of events, if focus key is pressed is as follows:
    // 1. curFocus.onRollOut, curFocus = oldFocus
    // 2. newFocus.onRollOver, curFocus = oldFocus
    // 3. curFocus.onKillFocus, curFocus = oldFocus
    // 4. curFocus = newFocus
    // 5. curFocus.onSetFocus, curFocus = newFocus
    // 6. Selection focus listeners, curFocus = newFocus
    FocusGroupDescr& focusGroup = GetFocusGroup(controllerIdx);
    Ptr<InteractiveObject> curFocused = focusGroup.LastFocused;

    if (curFocused == ch) 
        return false;

    FocusRectChanged = true;

#ifndef SF_NO_IME_SUPPORT
    Ptr<IMEManagerBase> pIMEManager = GetIMEManager();
    if (pIMEManager)
    {
        // report about focus change to IME. IME may prevent focus changing by
        // returning previously focused character or grant by returning new
        // focusing one.
        ch = pIMEManager->HandleFocus(this, curFocused, ch, ptopMostCh);
        if (curFocused == ch) 
            return false;
    }
#else
    SF_UNUSED(ptopMostCh);
#endif //#ifdef SF_NO_IME_SUPPORT

    if (!pASMovieRoot->NotifyOnFocusChange(curFocused, ch, controllerIdx, fmt, pfocusKeyInfo))
        return false;

    if (ch && !ch->IsFocusEnabled(fmt))
        ch = NULL;

    if (curFocused && curFocused->GetParent())
    {
        // queue onRollOut (step 1)
        if (!curFocused->OnLosingKeyboardFocus(ch, controllerIdx, fmt))
            return false; // if focus loss was prevented - return
    }

    // invoke onSetFocus for newly set LastFocused
    if (ch)
    {
        // queue onRollOver (step 2)
        ch->OnGettingKeyboardFocus(controllerIdx, fmt);
    }

    pASMovieRoot->NotifyQueueSetFocus(ch, controllerIdx, fmt);
    return true;
}

// Instantly transfers focus w/o any queuing
void MovieImpl::TransferFocus(InteractiveObject* pNewFocus, unsigned controllerIdx, FocusMovedType fmt)
{
    FocusGroupDescr& focusGroup = GetFocusGroup(controllerIdx);
    Ptr<InteractiveObject> curFocused = focusGroup.LastFocused;

    if (curFocused == pNewFocus) return;

    FocusRectChanged = true;

    int avmVersion = pASMovieRoot->GetAVMVersion();

    // AVM1 (AS 1.0/2.0) and AVM2 (AS 3.0) behaves differently while changing the focus.
    // In AS2 onKillFocus event is delivered BEFORE currently focused item is changed
    // (thus, Selection.getFocus() called from onKillFocus returns old focused item).
    // In AS3 focusOut event is dispatched AFTER focused item is changed
    // (thus, stage.focus contains newly focused character already).

    if (avmVersion == 2) // AS 3.0
    {
        if (!pNewFocus || pNewFocus->IsFocusEnabled(fmt))
            focusGroup.LastFocused = pNewFocus;
        else
        {
            focusGroup.LastFocused = NULL;
            pNewFocus = NULL;
        }
    }

    if (curFocused && curFocused->GetParent())
    {
        // invoke onKillFocus for LastFocused
        curFocused->OnFocus(InteractiveObject::KillFocus, pNewFocus, controllerIdx, fmt);
    }

    if (avmVersion == 1) // AS 1.0/2.0
        focusGroup.LastFocused = pNewFocus;

    // invoke onSetFocus for newly set LastFocused
    if (pNewFocus)
    {
        pNewFocus->OnFocus(InteractiveObject::SetFocus, curFocused, controllerIdx, fmt);
    }

    pASMovieRoot->NotifyTransferFocus(curFocused, pNewFocus, controllerIdx);
}


void MovieImpl::SetKeyboardFocusTo(InteractiveObject* ch, unsigned controllerIdx, FocusMovedType fmt)
{
#ifndef SF_NO_IME_SUPPORT
    Ptr<IMEManagerBase> pIMEManager = GetIMEManager();
	if (pIMEManager && pIMEManager->GetASIMEManager() != NULL)
	{
		// report about focus change to IME. IME may prevent focus changing by
		// returning previously focused character or grant by returning new
		// focusing one.
		FocusGroupDescr& focusGroup = GetFocusGroup(controllerIdx);
		Ptr<InteractiveObject> curFocused = focusGroup.LastFocused;
		if (curFocused != ch)
			ch = pIMEManager->HandleFocus(this, curFocused, ch, NULL);
	}
#endif //#ifdef SF_NO_IME_SUPPORT
    FocusGroupDescr& focusGroup = GetFocusGroup(controllerIdx);

    if (!ch || ch->GetType() != CharacterDef::TextField)
    {
        if (!focusGroup.FocusRectShown)
            FocusRectChanged = true;
        focusGroup.FocusRectShown = true;
    }
    else
    {
        if (focusGroup.FocusRectShown)
            FocusRectChanged = true;
        focusGroup.FocusRectShown = false;
    }
    
    focusGroup.ResetFocusDirection();
    if (!SetFocusTo(ch, controllerIdx, fmt))
        return;
    // here is a difference with the Flash: in Flash, if you do Selection.setFocus on invisible character or
    // character with invisible parent then Flash sets the focus on it and shows the focusRect.
    // GFxPlayer sets the focus, but doesn't show the focusRect. Probably, it shouldn't set focus at all.
    // (AB, 02/22/07).
    if (focusGroup.FocusRectShown)
    {
        InteractiveObject* cur = ch;
        for (; cur && cur->GetVisible(); cur = cur->GetParent())
            ;
        if (focusGroup.FocusRectShown != (!cur))
            FocusRectChanged = true;
        focusGroup.FocusRectShown = !cur;
    }
}

void MovieImpl::ResetFocusForChar(InteractiveObject* ch)
{
    for (unsigned i = 0; i < FocusGroupsCnt; ++i)
    {
        if (FocusGroups[i].IsFocused(ch))
        {
            // send focus out event
            Ptr<InteractiveObject> curFocused = FocusGroups[i].LastFocused;
            if (curFocused && !IsShutdowning())
            {
                UInt32    m = GetControllerMaskByFocusGroup(i);
                unsigned cc = GetControllerCount();
                for (unsigned j = 0; m != 0 && j < cc; ++j, m >>= 1)
                    SetFocusTo(NULL, j, GFx_FocusMovedByKeyboard);
            }
            FocusGroups[i].ResetFocus();
        }
    }
}

bool MovieImpl::IsFocused(const InteractiveObject* ch) const
{
    for (unsigned i = 0; i < FocusGroupsCnt; ++i)
    {
        if (FocusGroups[i].IsFocused(ch))
            return true;
    }
    return false;
}


void MovieImpl::AddTopmostLevelCharacter(InteractiveObject* pch)
{
    SF_ASSERT(pch);
    // do not allow to mark _root/_levelN sprites as topmostLevel...
    if (pch->IsSprite() && pch->CharToSprite()->IsLevelMovie())
        return;

    // make sure this character is not in the list yet
    // we need insert the character at the right pos to maintain original order
    // of topmostLevel characters.
    // So, storing the char with lowest original depth first and the highest last.
    // All characters for higher _level will be stored in front of characters for lower _level.
    
    UPInt i = 0;
    if (TopmostLevelCharacters.GetSize() > 0)
    {
        ArrayDH<DisplayObject*> chParents(GetHeap());
        ArrayDH<DisplayObject*> curParents(GetHeap());
        // fill array of parents for the passed char
        InteractiveObject* pchTopPar = NULL;
        for (InteractiveObject* ppar = pch; ppar; ppar = ppar->GetParent())
        {
            chParents.PushBack(ppar);
            pchTopPar = ppar;
        }

        // find the position according to depth and level. Exhaustive search for now.
        for (UPInt n = TopmostLevelCharacters.GetSize(); i < n; ++i)
        {
            if (TopmostLevelCharacters[i] == pch)
                return;

            // fill array of parent for the current char
            curParents.Resize(0);
            InteractiveObject* pcurTopPar = NULL;
            for (InteractiveObject* ppar = TopmostLevelCharacters[i]; ppar; ppar = ppar->GetParent())
            {
                curParents.PushBack(ppar);
                pcurTopPar = ppar;
            }

            if (pcurTopPar == pchTopPar)
            {
                // compare parents, starting from the top ones
                SPInt chParIdx  = (SPInt)chParents.GetSize()-1;
                SPInt curParIdx = (SPInt)curParents.GetSize()-1;
                bool found = false, cancel_iteration = false;
                for(;chParIdx >= 0 && curParIdx >= 0; --chParIdx, --curParIdx)
                {
                    if (chParents[chParIdx] != curParents[curParIdx])
                    {
                        // parents are different: compare depths of first different parents then.
                        // Note, 'parents' arrays contain the characters itselves, so, there is no 
                        // need to test them separately.
                        SF_ASSERT(chParents[chParIdx]->GetDepth() != curParents[curParIdx]->GetDepth());
                        if (chParents[chParIdx]->GetDepth() < curParents[curParIdx]->GetDepth())
                            found = true;
                        else
                            cancel_iteration = true;
                        break;
                    }
                }
                if (found)
                    break;
                else if (cancel_iteration)
                    continue;
            }
            else
            {
                SF_ASSERT(pchTopPar->CharToSprite_Unsafe()->IsLevelMovie());
                SF_ASSERT(pcurTopPar->CharToSprite_Unsafe()->IsLevelMovie());
                SF_ASSERT(pcurTopPar->CharToSprite_Unsafe()->GetLevel() != 
                          pchTopPar->CharToSprite_Unsafe()->GetLevel());

                // different levels, compare their numbers
                if (pcurTopPar->CharToSprite_Unsafe()->GetLevel() > pchTopPar->CharToSprite_Unsafe()->GetLevel())
                    break; // stop, if we are done with our level
            }
        }
    }
    TopmostLevelCharacters.InsertAt(i, pch);

    Ptr<Render::TreeNode> node = pch->SetIndirectTransform(pTopMostRoot);
    if (node)
        pTopMostRoot->Insert(i, node);
}

void MovieImpl::RemoveTopmostLevelCharacter(DisplayObjectBase* pch)
{
    for (UPInt i = 0, n = TopmostLevelCharacters.GetSize(); i < n; ++i)
    {
        if (TopmostLevelCharacters[i] == pch)
        {
            pTopMostRoot->Remove(i, 1);
            pch->RemoveIndirectTransform();
            TopmostLevelCharacters.RemoveAt(i);
            return;
        }
    }
}

// Sets style of candidate list. Invokes OnCandidateListStyleChanged callback.
void MovieImpl::SetIMECandidateListStyle(const IMECandidateListStyle& st)
{
#ifndef SF_NO_IME_SUPPORT
    if (!pIMECandidateListStyle)
        pIMECandidateListStyle = SF_HEAP_NEW(pHeap) IMECandidateListStyle(st);
    else
        *pIMECandidateListStyle = st;
#else
    SF_UNUSED(st);
#endif //#ifdef SF_NO_IME_SUPPORT
}

// Gets style of candidate list
void MovieImpl::GetIMECandidateListStyle(IMECandidateListStyle* pst) const
{
#ifndef SF_NO_IME_SUPPORT
    if (!pIMECandidateListStyle)
        *pst = IMECandidateListStyle();
    else
        *pst = *pIMECandidateListStyle;
#else
    SF_UNUSED(pst);
#endif //#ifdef SF_NO_IME_SUPPORT
}

bool MovieImpl::GetDirtyFlag(bool doReset)
{
    bool rv = G_IsFlagSet<Flag_HasChanges>(Flags) || RenderContext.HasChanges();
    if (doReset)
        G_SetFlag<Flag_HasChanges>(Flags, false);
    return rv;
}

Render::Text::Allocator* MovieImpl::GetTextAllocator()
{
    return pASMovieRoot->GetTextAllocator();
}

UInt64 MovieImpl::GetASTimerMs() const
{
    TestStream* pts = GetTestStream();
    UInt64 timerMs;
    if (pts)
    {
        if (pts->TestStatus == TestStream::Record)
        {
            timerMs = Timer::GetTicks()/1000 - StartTickMs;
            LongFormatter f(timerMs);
            f.Convert();
            pts->SetParameter("timer", f.ToCStr());
        }
        else
        {
           String tstr;
           pts->GetParameter("timer", &tstr);
           timerMs = SFatouq(tstr.ToCStr());
        }

    }
    else
        timerMs = Timer::GetTicks()/1000 - StartTickMs;
    return timerMs;
}

PointF MovieImpl::TranslateToScreen(const PointF& p, Matrix2F* puserMatrix)
{
    Matrix2F worldMatrix   = pMainMovie->GetWorldMatrix();
    Matrix2F mat           = ViewportMatrix;

    if (puserMatrix)
        mat.Prepend(*puserMatrix);
    mat.Prepend(worldMatrix);
    return mat.Transform(PixelsToTwips(p));
}

RectF  MovieImpl::TranslateToScreen(const RectF& r, Matrix2F* puserMatrix)
{
    Matrix2F worldMatrix   = pMainMovie->GetWorldMatrix();
    Matrix2F mat           = ViewportMatrix;

    if (puserMatrix)
        mat.Prepend(*puserMatrix);
    mat.Prepend(worldMatrix);
    return mat.EncloseTransform(PixelsToTwips(r));
}

// pathToCharacter - path to a character, i.e. "_root.hud.mc";
// pt is in pixels, in coordinate space of the character specified by the pathToCharacter
// returning value is in pixels of screen.
bool MovieImpl::TranslateLocalToScreen(const char* pathToMovieClip, 
                                       const PointF& pt, 
                                       PointF* presPt, 
                                       Matrix2F* userMatrix)
{
    SF_ASSERT(pathToMovieClip && presPt);
    // context of MovieImpl
    Value res;
    bool found = GetVariable(&res, pathToMovieClip);
    if (!found)
        return false;// oops.
    Matrix2F worldMatrix;
    if (!res.GetWorldMatrix(&worldMatrix))
        return false;// oops!

    Matrix2F mat = ViewportMatrix;

    // GetWorldMatrix returns matrix with translation in pixels
    // ViewportMatrix has scale 1/20. We need to eliminate this scale.
    mat.AppendScaling(20.f, 20.f);

    if (userMatrix)
        mat.Prepend(*userMatrix);
    mat.Prepend(worldMatrix);
    *presPt = mat.Transform(pt);

    return true;
}

// Obtains statistics for the movie view.
void    MovieImpl::GetStats(StatBag* pbag, bool reset)
{
    if (AdvanceStats)
    {
        AdvanceStats->GetStats(pbag, reset);
    }
}

void   MovieImpl::ForceCollectGarbage(unsigned gcFlags) 
{ 
    pASMovieRoot->ForceCollect(gcFlags);
}

void   MovieImpl::SuspendGC(bool suspend)
{
    pASMovieRoot->SuspendGC(suspend);
}

void   MovieImpl::ScheduleGC(unsigned gcFlags)
{
    pASMovieRoot->ScheduleGC(gcFlags);
}

void   MovieImpl::PrintObjectsReport(UInt32 flags, 
                                     Log* log,  
                                     const char* swfName,
                                     Ptr<AmpMovieObjectDesc>* root,
                                     MemoryHeap* heap)
{
    pASMovieRoot->PrintObjectsReport(flags, log, swfName);
    if (root != NULL)
    {
        *root = *pASMovieRoot->GetDisplayObjectsTree(heap);
    }
}

// Sets a movie at level; used for initialization.
bool   MovieImpl::SetLevelMovie(int level, DisplayObjContainer *psprite)
{
    unsigned i = 0;
    SF_ASSERT(level >= 0);
    SF_ASSERT(psprite);

    for (; i < MovieLevels.GetSize(); i++)
    {
        if (MovieLevels[i].Level >= level)
        {           
            if (MovieLevels[i].Level == level)
            {
                SF_DEBUG_WARNING1(1, "MovieImpl::SetLevelMovie fails, level %d already occupied", level);
                return 0;
            }           
            // Found insert spot.
            break;              
        }
    }
    G_SetFlag<Flag_LevelClipsChanged>(Flags, true);

    // Insert the item.
    MovieImpl::LevelInfo li;
    li.Level  = level;
    li.pSprite= psprite;
    MovieLevels.InsertAt(i, li);

    psprite->OnInsertionAsLevel(level);

    if (level == 0)
    {
        SetMainContainer(psprite);
        SetMovieDefImpl(psprite->GetResourceMovieDef());
        if (pMainMovieDef && AdvanceStats)
        {
            AdvanceStats->SetName(pMainMovieDef->GetFileURL());
        }
        pStateBag->SetDelegate(pMainMovieDef->pStateBag);
        // Frame timing
        FrameTime    = 1.0f / GetMovieDef()->GetFrameRate();

        if (!G_IsFlagSet<Flag_ViewportSet>(Flags))
        {
            MovieDefImpl* pdef = psprite->GetResourceMovieDef();
            Viewport desc((int)pdef->GetWidth(), (int)pdef->GetHeight(), 0,0, (int)pdef->GetWidth(), (int)pdef->GetHeight());
            SetViewport(desc);
        }
    }

    G_SetFlag<Flag_NeedMouseUpdate>(Flags, true);
    return 1;
}

// Create new instance names for unnamed objects.
ASString    MovieImpl::CreateNewInstanceName()
{
    InstanceNameCount++; // Start at 0, so first value is 1.
    char pbuffer[48] = { 0 };

    Format(StringDataPtr(pbuffer, sizeof(pbuffer)), "instance{0}", InstanceNameCount);

    return GetStringManager()->CreateString(pbuffer);
}

Ptr<ImageResource> MovieImpl::GetImageResourceByLinkageId(MovieDefImpl* md, const char* linkageId)
{
    Ptr<ImageResource> pimageRes;
    if (md) // can it be NULL?
    {
        SF_ASSERT(linkageId);

        // Check to see if URL is a user image substitute.
        bool    userImageProtocol = LoaderImpl::IsProtocolImage(String(linkageId));

        if (userImageProtocol)
        {
            // Create image through image callback
            StateBag* pss       = pStateBag;
            MemoryHeap* pheap   = GetMovieHeap();
            ImageCreator* pimageCreator = pss->GetImageCreator();
            if (pimageCreator)
            {
                ImageCreateInfo icinfo(ImageCreateInfo::Create_Protocol, pheap);
                icinfo.SetStates(pss->GetLog(), static_cast<FileOpener*>(pss->GetFileOpener().GetPtr()), // is it safe to cast?
                                 pss->GetImageFileHandlerRegistry());
                icinfo.pMovie = this;
                Ptr<Image> pimg = *pimageCreator->LoadProtocolImage(icinfo, linkageId);
                if (pimg)
                {
                    pimageRes = 
                        *SF_HEAP_NEW(pheap) ImageResource(pimg, Resource::Use_Bitmap);
                }
            }
        }
        else
        {
            // Get exported resource for linkageId and verify that it is an image.
            ResourceBindData resBindData;
            if (!FindExportedResource(md, &resBindData, linkageId))
                return NULL;
            SF_ASSERT(resBindData.pResource.GetPtr() != 0);
            // Must check resource type, since users can theoretically pass other resource ids.
            if (resBindData.pResource->GetResourceType() == Resource::RT_Image)
            {
                pimageRes = (ImageResource*)resBindData.pResource.GetPtr();
            }
        }
    }
    return pimageRes;
}

// ****************************************************************************
// Helper function to load text data from a file
//
bool    MovieImpl::ReadTextData(String* pdata, File* pfile, int* pfileLen, bool urlEncoded)
{
    SF_ASSERT(pfile);
    SF_ASSERT(pdata);
    SF_ASSERT(pfileLen);

    if ((*pfileLen = pfile->GetLength()) == 0)
        return false;

    String str;

    UByte* td = (UByte*) SF_ALLOC(*pfileLen, Stat_Default_Mem);
    pfile->Read(td, *pfileLen);

    wchar_t*            wcsptr = NULL;
    UByte*              ptextData = td;
    int                 textLength = *pfileLen;

    // the following converts byte stream to appropriate endianness
    // for UTF16/UCS2 (wide char format)
    UInt16* prefix16 = (UInt16*)td;
    if (prefix16[0] == Alg::ByteUtil::BEToSystem((UInt16)0xFFFE)) // little endian
    {
        prefix16++;
        ptextData = (UByte*)prefix16;
        textLength = (*pfileLen / 2) - 1;
        if (sizeof(wchar_t) == 2)
        {
            for (int i=0; i < textLength; i++)
                prefix16[i] = (wchar_t)Alg::ByteUtil::LEToSystem(prefix16[i]);
        }
        else
        {
            // special case: create an aux buffer to hold the data
            wcsptr = (wchar_t*) SF_ALLOC(textLength * sizeof(wchar_t), Stat_Default_Mem);
            for (int i=0; i < textLength; i++)
                wcsptr[i] = (wchar_t)Alg::ByteUtil::LEToSystem(prefix16[i]);
            ptextData = (UByte*)wcsptr;
        }
        str.AppendString( (const wchar_t*)ptextData, textLength );
    }
    else if (prefix16[0] == Alg::ByteUtil::BEToSystem((UInt16)0xFEFF)) // big endian
    {
        prefix16++;
        ptextData = (UByte*)prefix16;
        textLength = (*pfileLen / 2) - 1;
        if (sizeof(wchar_t) == 2)
        {
            for (int i=0; i < textLength; i++)
                prefix16[i] = Alg::ByteUtil::BEToSystem(prefix16[i]);
        }
        else
        {
            wcsptr = (wchar_t*) SF_ALLOC(textLength * sizeof(wchar_t), Stat_Default_Mem);
            for (int i=0; i < textLength; i++)
                wcsptr[i] = Alg::ByteUtil::BEToSystem(prefix16[i]);
            ptextData = (UByte*)wcsptr;
        }
        str.AppendString( (const wchar_t*)ptextData, textLength );
    }
    else if (*pfileLen > 2 && td[0] == 0xEF && td[1] == 0xBB && td[2] == 0xBF)
    {
        // UTF-8 with explicit BOM
        ptextData += 3;
        textLength -= 3;
        str.AppendString( (const char*)ptextData, textLength );
    }
    else
    {
        str.AppendString( (const char*)ptextData, textLength );
    }

    if (wcsptr)
        SF_FREE(wcsptr);

    // following works directly on bytes
    if (urlEncoded)
        ASUtils::Unescape(str.ToCStr(), str.GetSize(), pdata);
    else 
        *pdata = str;

    SF_FREE(td);
    return true;
}

bool    MovieImpl::ReadBinaryData(ArrayPOD<UByte>* pdata, File* pfile, int* pfileLen)
{
    SF_ASSERT(pfile);
    SF_ASSERT(pdata);
    SF_ASSERT(pfileLen);

    if ((*pfileLen = pfile->GetLength()) == 0)
        return false;

    pdata->Resize(*pfileLen);
    if (pfile->Read(pdata->GetDataPtr(), *pfileLen) != *pfileLen)
        return false;
    return true;
}


/*

// In 

void MovieImpl::CreateString(Value* pvalue, const char* pstring)
{
    SF_ASSERT(0); //@TODO!
    SF_UNUSED2(pvalue, pstring);
}

void MovieImpl::CreateStringW(Value* pvalue, const wchar_t* pstring)
{
    SF_ASSERT(0); //@TODO!
    SF_UNUSED2(pvalue, pstring);
}

void MovieImpl::CreateObject(Value* pvalue, const char* className, 
                  const Value* pargs, unsigned nargs)
{
    SF_ASSERT(0); //@TODO!
    SF_UNUSED4(pvalue, className, pargs, nargs);
}
void MovieImpl::CreateArray(Value* pvalue)
{
    SF_ASSERT(0); //@TODO!
    SF_UNUSED1(pvalue);
}
*/


void MovieImpl::ProcessUnloadQueue()
{
    SF_AMP_SCOPE_TIMER(AdvanceStats, "MovieImpl::ProcessUnloadQueue", Amp_Profile_Level_Medium);

    // process unload queue
    if (pUnloadListHead)
    {
        InteractiveObject* pnextCh = NULL;
        for (InteractiveObject* pcurCh = pUnloadListHead; pcurCh; pcurCh = pnextCh)
        {
            SF_ASSERT(pcurCh->IsUnloaded());
            SF_ASSERT(!pcurCh->IsOptAdvListFlagSet());
#ifdef SF_BUILD_DEBUG
            InteractiveObject* psavedParent = pcurCh->GetParent();
#endif

            pnextCh = pcurCh->pNextUnloaded;
            pcurCh->pNextUnloaded = NULL;
            pcurCh->OnEventUnload();

            // Remove from parent's display list
            InteractiveObject* pparent = pcurCh->GetParent();

#ifdef SF_BUILD_DEBUG
            SF_ASSERT(psavedParent == pparent);
#endif

            if (pparent)
                pparent->RemoveDisplayObject(pcurCh);

            pcurCh->Release();
        }
        pUnloadListHead = NULL;
    }
}

void MovieImpl::ReleaseUnloadList()
{
//     InteractiveObject* pnextCh = NULL;
//     for (InteractiveObject* pcurCh = pUnloadListHead; pcurCh; pcurCh = pnextCh)
//     {
//         SF_ASSERT(pcurCh->IsUnloaded());
//         SF_ASSERT(!pcurCh->IsOptAdvListFlagSet());
// 
//         pnextCh = pcurCh->pNextUnloaded;
//         pcurCh->pNextUnloaded = NULL;
//         pcurCh->Release();
//     }
//     pUnloadListHead = NULL;
}

// finds first available font manager (by traversing through root nodes).
FontManager* MovieImpl::FindFontManager(MovieDefImpl* pdefImpl)
{
    MovieDefRootNode *pdefNode = RootMovieDefNodes.GetFirst();
    while(!RootMovieDefNodes.IsNull(pdefNode))
    {   
        if ((pdefNode->pDefImpl == pdefImpl || !pdefImpl) && pdefNode->pFontManager)
            return pdefNode->pFontManager;

        pdefNode = pdefNode->pNext;
    }
    return NULL; // not found
}

Sprite* FocusGroupDescr::GetModalClip(MovieImpl* proot)
{
    if (ModalClip)
    {
        Ptr<DisplayObject> modalChar = ModalClip->ResolveCharacter(proot);
        if (modalChar)
        {
            SF_ASSERT(modalChar->IsSprite());
            return modalChar->CharToSprite_Unsafe();
        }
    }
    return NULL;
}

// ***** GFxValue definitions

String GFx::Value::ToString() const
{
    String retVal;

    switch(GetType())
    {
    case VT_Undefined:
        {
            retVal = String("undefined");
        }
        break;
    case VT_Null:
        {  
            retVal = String("null");
        }
        break;
    case VT_Boolean:
        {  
            retVal = String(mValue.BValue ? "true" : "false");
        }
        break;
    case VT_Number:
        {
            char buf[NumberUtil::TOSTRING_BUF_SIZE];
            retVal = String(NumberUtil::ToString(mValue.NValue, buf, sizeof(buf))); // Precision is 10 by default.
        }
        break;

    case VT_Int:
        {
            char buf[32];
            SFsprintf(buf, 32, "%d", mValue.IValue);
            retVal = String(buf);
        }
        break;
    case VT_UInt:
        {
            char buf[32];
            SFsprintf(buf, 32, "%u", mValue.UIValue);
            retVal = String(buf);
        }
        break;

    case VT_String:
        {
            retVal = String(GetString());
        }
        break;
    case VT_StringW:
        {
            retVal = String(GetStringW());
        }
        break;
    case VT_Object:
    case VT_Array:
    case VT_DisplayObject:
#ifdef GFX_AS3_SUPPORT
    case VT_Closure:
#endif
        {               
            pObjectInterface->ToString(&retVal, *this);
        }
        break;
    default:
        {
            retVal = "<bad type>";
            SF_ASSERT(0);
        }
    }
    return retVal;
}

const wchar_t* GFx::Value::ToStringW(wchar_t* pwstr, UPInt buffSz) const
{
    switch(GetType())
    {
    case VT_StringW:
        {
            SFwcscpy(pwstr, buffSz, GetStringW());
            return pwstr;
        }
        break;
    default:
        {
            UTF8Util::DecodeStringSafe(pwstr, buffSz, ToString().ToCStr());   
        }
        break;
    }
    return pwstr;
}


bool MovieImpl::FindExportedResource(MovieDefImpl* localDef, ResourceBindData *presBindData, const String& symbol)
{
    if (localDef->GetExportedResource(presBindData, symbol))
        return true;

    MovieDefImpl* curDef = localDef;

    // not found in local def - look through "import parents" (movies which import the local one directly or indirectly)
    // Thus, if a moviedef was loaded by loadMovie it should look only inside the movie and ITS imports,
    // it shouldn't go to the movie that actually loaded it.
    MovieDefRootNode *pdefNode = RootMovieDefNodes.GetFirst();
    while(!RootMovieDefNodes.IsNull(pdefNode))
    {      
        if (pdefNode->pDefImpl != localDef && pdefNode->pDefImpl->DoesDirectlyImport(curDef))
        {
            if (pdefNode->pDefImpl->GetExportedResource(presBindData, symbol))
                return true;
            curDef = pdefNode->pDefImpl;
        }
        pdefNode = pdefNode->pNext;
    }
    return false;
}

Render::DrawableImageContext* MovieImpl::GetDrawableImageContext()
{
    if (DIContext)
        return DIContext;

    SF_DEBUG_ASSERT(1, "To access a DrawableImageContext, an instance "
        "of ThreadCommandQueue must be passed to the MovieDef::CreateInstance function.");
    DIContext = *SF_NEW DrawableImageContext(&RenderContext, pRTCommandQueue);
    return DIContext.GetPtr();
}

DrawingContext* MovieImpl::CreateDrawingContext()
{
    DrawingContext* dc = SF_HEAP_NEW(pHeap) DrawingContext(GetRenderContext(), GetImageCreator());
    DrawingContextList.PushBack(dc);
    return dc;
}

// !RH: Trampoline function so we can manually update from the AS3 VM.
// Needed by BitmapData.draw to force any active Shape-related commands
// to be pushed into their relevant TreeShape structures.
void MovieImpl::UpdateAllRenderNodes()
{
	UpdateAllDrawingContexts();

	if (FocusRectChanged)
		UpdateFocusRectRenderNodes();
}

void MovieImpl::UpdateAllDrawingContexts()
{
    DrawingContext* curDC = DrawingContextList.GetFirst();
    for (; !DrawingContextList.IsNull(curDC); curDC = curDC->pNext)
    {
        if (curDC->IsDirty())
        {
            curDC->UpdateRenderNode();
        }
    }
}

void MovieImpl::ClearDrawingContextList()
{
    DrawingContext* curDC = DrawingContextList.GetFirst();
    DrawingContext* pnext;
    for (; !DrawingContextList.IsNull(curDC); curDC = pnext)
    {
        pnext = curDC->pNext;
        curDC->pPrev = curDC->pNext = NULL;
    }
    DrawingContextList.Clear();
}

void MovieImpl::AddMovieDefToKillList(MovieDefImpl* md)
{
    if (md)
    {
        for (UPInt i = 0, n = MovieDefKillList.GetSize(); i < n; ++i)
        {
            if (MovieDefKillList[i].pMovieDef == md)
            {
                // already in the array: just update the FrameId
                MovieDefKillList[i].KillFrameId = RenderContext.GetFrameId();
                return;
            }
        }
        MDKillListEntry e;
        e.KillFrameId   = RenderContext.GetFrameId();
        e.pMovieDef = md;
        MovieDefKillList.PushBack(e);
    }
}

void MovieImpl::ProcessMovieDefToKillList()
{
    if (MovieDefKillList.GetSize() > 0)
    {
        UInt64 finFrId = RenderContext.GetFinalizedFrameId();
        for (UPInt i = 0; i < MovieDefKillList.GetSize(); )
        {
            if (finFrId > MovieDefKillList[i].KillFrameId)
            {
                MovieDefKillList.RemoveAt(i);
                continue;
            }
            ++i;
        }
    }
}

#ifdef SF_ENABLE_ANE			
// Extension context
bool MovieImpl::Call(const char* extensionID, const char* contextID, const char* methodName, unsigned argc, const Value* const argv, Value* const result)
{
    Ptr<ExtensionContextInterface> ec = GetExtensionContextInterface();
    if (ec)
    {
		return ec->Call(extensionID, contextID, methodName, argc, argv, result);
    }

	return false;
}

const char* MovieImpl::GetExtensionDirectory(const char* extensionID)
{
    Ptr<ExtensionContextInterface> ec = GetExtensionContextInterface();
    if (ec)
    {
		return ec->GetExtensionDirectory(extensionID);
    }

	return NULL;
}

void MovieImpl::InitializeExtensionContext(const char* extensionID, const char* contextID)
{
    Ptr<ExtensionContextInterface> ec = GetExtensionContextInterface();
    if (ec)
    {
		ec->InitializeExtensionContext(extensionID, contextID);
    }
}

void MovieImpl::FinalizeExtensionContext(const char* extensionID, const char* contextID)
{
    Ptr<ExtensionContextInterface> ec = GetExtensionContextInterface();
    if (ec)
    {
		ec->FinalizeExtensionContext(extensionID, contextID);
    }
}


GFx::Value*	MovieImpl::GetActionScriptData(const char* extensionID, const char* contextID)
{
	Ptr<ExtensionContextInterface> ec = GetExtensionContextInterface();
    if (ec)
    {
		return ec->GetActionScriptData(extensionID, contextID);
    }

	return NULL;
}

void MovieImpl::SetActionScriptData(const char* extensionID, const char* contextID, GFx::Value* data)
{
	Ptr<ExtensionContextInterface> ec = GetExtensionContextInterface();
    if (ec)
    {
		ec->SetActionScriptData(extensionID, contextID, data);
    }
}
#endif

MovieImpl::MultitouchInputMode MovieImpl::GetMultitouchInputMode() const
{
    return MultitouchMode;    
}

void MovieImpl::SetMultitouchInputMode(MovieImpl::MultitouchInputMode mode)
{
    Ptr<MultitouchInterface> mi = GetMultitouchInterface();
    if (mi)
    {
        bool v = mi->SetMultitouchInputMode((MultitouchInterface::MultitouchInputMode)mode);
        if (v)
            MultitouchMode = mode;
    }
}

unsigned MovieImpl::GetMaxTouchPoints() const
{
    Ptr<MultitouchInterface> mi = GetMultitouchInterface();
    if (mi) //? && (MultitouchMode & MultitouchInterface::MTI_TouchPoint) != 0)
    {
        return mi->GetMaxTouchPoints();
    }   
    return 0;
}

UInt32 MovieImpl::GetSupportedGesturesMask() const
{
    Ptr<MultitouchInterface> mi = GetMultitouchInterface();
    if (mi) //? && (MultitouchMode & MultitouchInterface::MTI_Gesture) != 0)
    {
        return mi->GetSupportedGesturesMask();
    }    
    return 0;
}

bool MovieImpl::RegisterAccelerometer(int accelerometerId)
{
	Ptr<AccelerometerInterface> ai = GetAccelerometerInterface();
    if (ai)
    {
        return ai->RegisterAccelerometer(accelerometerId);
    }    
    return false;
}

bool MovieImpl::UnregisterAccelerometer(int accelerometerId)
{
	Ptr<AccelerometerInterface> ai = GetAccelerometerInterface();
    if (ai)
    {
        return ai->UnregisterAccelerometer(accelerometerId);
    }    
    return false;
}

bool MovieImpl::IsAccelerometerMuted() const
{
	Ptr<AccelerometerInterface> ai = GetAccelerometerInterface();
    if (ai)
    {
        return ai->IsAccelerometerMuted();
    }    
    return false;
}

bool MovieImpl::IsAccelerometerSupported() const
{
	Ptr<AccelerometerInterface> ai = GetAccelerometerInterface();
    if (ai)
    {
        return ai->IsAccelerometerSupported();
    }    
    return false;
}

void MovieImpl::SetAccelerometerInterval(int accelerometerId, int interval)
{
	Ptr<AccelerometerInterface> ai = GetAccelerometerInterface();
    if (ai)
    {
        ai->SetAccelerometerInterval(accelerometerId, interval);
    }
}

bool MovieImpl::RegisterGeolocation(int geolocationId)
{
	Ptr<GeolocationInterface> ai = GetGeolocationInterface();
    if (ai)
    {
        return ai->RegisterGeolocation(geolocationId);
    }    
    return false;
}

bool MovieImpl::UnregisterGeolocation(int geolocationId)
{
	Ptr<GeolocationInterface> ai = GetGeolocationInterface();
    if (ai)
    {
        return ai->UnregisterGeolocation(geolocationId);
    }    
    return false;
}

bool MovieImpl::IsGeolocationMuted() const
{
	Ptr<GeolocationInterface> ai = GetGeolocationInterface();
    if (ai)
    {
        return ai->IsGeolocationMuted();
    }    
    return false;
}

bool MovieImpl::IsGeolocationSupported() const
{
	Ptr<GeolocationInterface> ai = GetGeolocationInterface();
    if (ai)
    {
        return ai->IsGeolocationSupported();
    }    
    return false;
}

void MovieImpl::SetGeolocationInterval(int geolocationId, int interval)
{
	Ptr<GeolocationInterface> ai = GetGeolocationInterface();
    if (ai)
    {
        ai->SetGeolocationInterval(geolocationId, interval);
    }
}

MovieImpl::ReturnValueHolder* MovieImpl::GetRetValHolder()
{
    if (!pRetValHolder)
        pRetValHolder = SF_HEAP_NEW(pHeap) ReturnValueHolder(GetStringManager());
    return pRetValHolder;
}

// Forces render tree to be updated in order to apply all possible changes
// related to images and/or textures.
void MovieImpl::ForceUpdateImages()
{
    GetRenderContext().ForceUpdateImages();
    ++PreviouslyCaptured; // force Capture
}

// saves indirect transform pair of nodes, where transformParent is the actual
// node where the transform should be taken from; 'node' is the node that should
// use indirect transfrom from 'transformParent' (it should be inserted in the 
// render tree somewhere).
void MovieImpl::AddIndirectTransformPair(DisplayObjContainer* origParent, 
                                         TreeNode* transformParent, 
                                         DisplayObjectBase* obj)
{
    IndirectTransPair p;
    p.TransformParent = transformParent;
    p.Obj             = obj;
    p.OriginalParent  = origParent;
    IndirectTransformPairs.PushBack(p);
}

MovieImpl::IndirectTransPair MovieImpl::RemoveIndirectTransformPair(DisplayObjectBase* obj)
{
    for (UPInt i = 0, n = IndirectTransformPairs.GetSize(); i < n; ++i)
    {
        if (IndirectTransformPairs[i].Obj == obj)
        {
            MovieImpl::IndirectTransPair e = IndirectTransformPairs[i];
            IndirectTransformPairs.RemoveAt(i);
            return e;
        }
    }
    return IndirectTransPair();
}

void MovieImpl::ClearIndirectTransformPairs()
{
    for (UPInt i = 0, n = IndirectTransformPairs.GetSize(); i < n; ++i)
    {
        MovieImpl::IndirectTransPair& srce = IndirectTransformPairs[i];
        srce.Clear();
    }
    IndirectTransformPairs.Clear();
}

void MovieImpl::UpdateTransformParent(DisplayObjectBase* obj, DisplayObjectBase* transfParent)
{
    for (UPInt i = 0, n = IndirectTransformPairs.GetSize(); i < n; ++i)
    {
        if (IndirectTransformPairs[i].Obj == obj)
        {
            MovieImpl::IndirectTransPair& e = IndirectTransformPairs[i];
            e.TransformParent = (transfParent) ? transfParent->GetRenderNode() : NULL;
            e.OrigParentDepth = -1; // force to update
            break;
        }
    }
}

// Register a single font resource.
bool MovieImpl::RegisterFont(MovieDef* md, FontResource* fontRes)
{
    // First, find a matching font in our MovieDefs.
    for (UPInt i = 0, n = RegisteredFonts.GetSize(); i < n; ++i)
    {
        if (RegisteredFonts[i].pFont == fontRes &&
            RegisteredFonts[i].pMovieDef == md)
            return 0;
        SF_ASSERT(RegisteredFonts[i].pFont != fontRes);
    }

    FontDesc fd;
    fd.pFont    = fontRes;
    fd.pMovieDef= md;
    RegisteredFonts.PushBack(fd);
    SetRegisteredFontsChanged();
    return 1;
}

// Unregister all single fonts for 'md', which were previously
// registered by RegisterFont. 
void MovieImpl::UnregisterFonts(MovieDefImpl* pdefImpl)
{
    // First, find a matching font in our MovieDefs.
    for (UPInt i = 0; i < RegisteredFonts.GetSize();)
    {
        if (RegisteredFonts[i].pMovieDef == pdefImpl)
        {
            RegisteredFonts.RemoveAt(i);
        }
        else
            ++i;
    }
    MovieDefRootNode *pdefNode = RootMovieDefNodes.GetFirst();
    while(!RootMovieDefNodes.IsNull(pdefNode))
    {        
        pdefNode->pFontManager->CleanCacheFor(pdefImpl);

        pdefNode = pdefNode->pNext;
    }
    SetRegisteredFontsChanged();
}

FontResource* MovieImpl::FindRegisteredFont(const char* pfontName, 
                                            unsigned matchFontFlags, 
                                            MovieDef** ppsrcMovieDef) const
{
    // check registered fonts first
    for (UPInt i = 0, n = RegisteredFonts.GetSize(); i < n; ++i)
    {
        // looks like Flash ignores fontFlags for registered fonts (by AS3 Font.registerFont).
        // Verify (@TODO).
        if (RegisteredFonts[i].pFont->GetFont()->MatchFont(pfontName, matchFontFlags))
        {
            *ppsrcMovieDef = RegisteredFonts[i].pMovieDef.GetPtr();
            return RegisteredFonts[i].pFont;
        }
    }
    return NULL;
}

// Adds all registered fonts into the hash 'fonts'.
void MovieImpl::LoadRegisteredFonts(HashSet<Ptr<Render::Font> >& fonts)
{
    for (UPInt i = 0, n = RegisteredFonts.GetSize(); i < n; ++i)
    {
        fonts.Set(RegisteredFonts[i].pFont->GetFont());
    }
}

void MovieImpl::MakeAreaVisible(const Render::RectF& screenRect, 
                                const Render::RectF& box,
                                UInt32 flags)
{
    if (!screenRect.Contains(box))
    {
        Render::Matrix2F vpm;
        ResetViewportMatrix();
        vpm.SetInverse(ViewportMatrix);
        RectF rectInStage;
        vpm.EncloseTransform(&rectInStage, screenRect);

        RectF boxInStage = PixelsToTwips(box);

        // calc scale if necessary
        double scaleX = 1.0, scaleY = 1.0;
        if (!(flags & MAVF_DontScaleDown))
        {
            if (boxInStage.Width() > rectInStage.Width())
                scaleX = double(rectInStage.Width())/boxInStage.Width();
            if (boxInStage.Height() > rectInStage.Height())
                scaleY = double(rectInStage.Height())/boxInStage.Height();
        }
        if ((flags & MAVF_ScaleUp50) && scaleX == 1.0 && scaleY == 1.0)
        {
            double scrA = rectInStage.Area();
            if (boxInStage.Area() * 2 < scrA)
            {
                double an = scrA/(100/50);
                double r = double(boxInStage.Width()) / boxInStage.Height();
                double wn = sqrt(an * r);
                double hn = an / wn;
                scaleX = wn / boxInStage.Width();
                scaleY = hn / boxInStage.Height();
                if (boxInStage.Width()*scaleX > rectInStage.Width())
                    scaleX = double(rectInStage.Width())/boxInStage.Width();
                if (boxInStage.Height()*scaleY > rectInStage.Height())
                    scaleY = double(rectInStage.Height())/boxInStage.Height();
            }
        }

        Render::Matrix2F delta;
        delta.AppendTranslation(-boxInStage.x1, -boxInStage.y1);
        delta.AppendScaling(float(Alg::Min(scaleX, scaleY)));
        delta.AppendTranslation(boxInStage.x1, boxInStage.y1);
        RectF r;
        delta.EncloseTransform(&r, boxInStage);
        Render::PointF vec(0,0);

        // calc offset, if necessary
        if (r.x1 > rectInStage.x2 || r.x2 < rectInStage.x1 || 
            r.x2 > rectInStage.x2 || r.x1 < rectInStage.x1)
        {
            if (!(flags & MAVF_LeftTopToZero))
                vec.x = (r.x1 + r.Width() / 2) - (rectInStage.x2 - rectInStage.Width() / 2);
            else
                vec.x = r.x1 - rectInStage.x1;
        }
        if (r.y1 > rectInStage.y2 || r.y2 < rectInStage.y1 || 
            r.y2 > rectInStage.y2 || r.y1 < rectInStage.y1)
        {
            if (!(flags & MAVF_LeftTopToZero))
                vec.y = (r.y1 + r.Height() / 2) - (rectInStage.y2 - rectInStage.Height() / 2);
            else
                vec.y = r.y1 - rectInStage.y1;
        }
        delta.AppendTranslation(-vec.x, -vec.y);

        ViewportMatrix.Prepend(delta);

        pRenderRoot->SetMatrix(ViewportMatrix);
    }
}

}} // namespace Scaleform::GFx
