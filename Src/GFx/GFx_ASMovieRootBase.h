/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_ASMovieRootBase.h
Content     :   An interface to ActionScript-dependent part of MovieImpl. 
Created     :   Dec, 2009
Authors     :   Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_ASMOVIEROOTBASE_H
#define INC_SF_GFX_ASMOVIEROOTBASE_H

#include "GFx/GFx_Player.h"

namespace Scaleform { 
namespace Render { namespace Text { class Allocator; } }
namespace GFx {

class LoadQueueEntry;
class MovieImpl;
class MovieDefImpl;
class LoadStates;
class DisplayObject;
class DisplayObjContainer;
struct ProcessFocusKeyInfo;
class OrientationEvent;
class AppLifecycleEvent;
class MovieDefRootNode;
class ASStringManager;
class EventId;

// An interface to ActionScript-dependent part of MovieImpl. 
// There are different implementations of this interface for AS 2 and AS 3.
class ASMovieRootBase : public RefCountBase<ASMovieRootBase, StatMV_Other_Mem>
{
protected:
    MovieImpl*          pMovieImpl;
    Ptr<ASSupport>      pASSupport;
    UByte               AVMVersion;
public:

    ASMovieRootBase(ASSupport* asSupport)
        : pMovieImpl(NULL), pASSupport(asSupport), AVMVersion(asSupport->GetAVMVersion()) 
    {
    }
    virtual ~ASMovieRootBase() {}

    SF_INLINE int GetAVMVersion() const           { return AVMVersion; }
    SF_INLINE ASSupport* GetASSupport() const  { return pASSupport; }
    SF_INLINE MovieImpl* GetMovieImpl() const  { return pMovieImpl; }

    virtual void SetMovie(MovieImpl* pmovie)
    {
        pMovieImpl = pmovie;
    }

    //     virtual void        AddStickyVariable
    //         (const GASString& fullPath, const Value &val, Movie::SetVarType setType) =0;
    virtual void        AdvanceFrame(bool nextFrame) =0;
    virtual void        ChangeMouseCursorType(unsigned mouseIdx, unsigned newCursorType) =0;
    // Check for AVM; create it, if it is not created yet.
    virtual bool        CheckAvm() =0;
    // called when movie is shutting down. Supposed to clean up MovieLevels.
    virtual void        ClearDisplayList() =0;
    // creates VM-specific MovieDefRootNode
    virtual MovieDefRootNode* CreateMovieDefRootNode(MemoryHeap* pheap, MovieDefImpl *pdefImpl, bool importFlag = false) =0;
    virtual void        DoActions() =0;
    virtual InteractiveObject* FindTarget(const ASString& path) const =0;

    // forces garbage collection (if GC is enabled)
    virtual void        ForceCollect(unsigned gcFlags) =0;
    // forces emergency garbage collection (if GC is enabled). This method is called
    // when heap is overflown. 
    virtual void        ForceEmergencyCollect() =0;

    // Additional GC control functions.
    // SuspendGC suspends/resumes garbage collection. It is counted operation, meaning
    // if it was suspended N-times then it should be re-enabled N-times to restore normal operation.
    virtual void        SuspendGC(bool suspend) = 0;
    // Schedule garbage collection. Unlike ForceCollectGarbage it doesn't execute collection immediately;
    // instead, it will be executed when next Advance is called.
    virtual void        ScheduleGC(unsigned gcFlags) = 0;

    // Generate button events (onRollOver, onPress, etc)
    virtual void        GenerateMouseEvents(unsigned mouseIndex) =0;
    virtual void        GenerateTouchEvents(unsigned /*mouseStateIndex*/) {}
    virtual void        GenerateGestureEvents(InteractiveObject* /*topmost*/, 
                                              unsigned /*phase*/,
                                              const PointF& /*pos*/, 
                                              const PointF& /*offset*/,
                                              const PointF& /*scale*/,
                                              float /*rotation*/,
                                              unsigned /*gestureMask*/) {}

    // This method returns the root display object container for the specified object. 
    // The "dobj" parameter might be NULL; in this case, for AS2 this method will return _root 
    // (same as GetMainContainer), for AS3 it will return "root".
    // If "dobj" is not NULL and it belongs to SWF loaded by loadMovie then the result is the
    // parent movie on which "loadMovie" was called. (@TODO)
    virtual DisplayObjContainer* GetRootMovie(DisplayObject* dobj = NULL) const =0;

    virtual bool        Init(MovieDefImpl* pmovieDef) =0;
    virtual void        NotifyMouseEvent(const InputEventsQueueEntry* qe,
                                         const MouseState& ms, int mi) =0;
    virtual void        NotifyOnResize() =0;
    virtual void        NotifyQueueSetFocus(InteractiveObject* ch, unsigned controllerIdx, FocusMovedType fmt) =0;
    virtual void        NotifyTransferFocus(InteractiveObject* curFocused,
                                            InteractiveObject* pNewFocus, 
                                            unsigned controllerIdx) =0;
    // Should return true if focus change is allowed; if false, then focus change is denied.
    virtual bool        NotifyOnFocusChange(InteractiveObject* curFocused, 
                                            InteractiveObject* toBeFocused, 
                                            unsigned controllerIdx, 
                                            FocusMovedType fmt,
                                            ProcessFocusKeyInfo* pfocusKeyInfo = NULL) =0;

#ifdef GFX_ENABLE_ANALOG_GAMEPAD
    virtual void        NotifyGamePadAnalogEvent(const EventId* id) { SF_UNUSED1(id); }
#endif
	virtual void        NotifyStatusEvent(const EventId* id) { SF_UNUSED(id); }
	
	virtual void        NotifyAccelerometerEvent(const EventId* id, int evtIdAvv) { SF_UNUSED2(id, evtIdAvv); }
	virtual void        NotifyGeolocationEvent(const EventId* id, int evtIdAvv) { SF_UNUSED2(id, evtIdAvv); }
	
    // invoked when whole movie view gets or loses focus
    virtual void        OnMovieFocus(bool set) =0;
    // called BEFORE processing complete (non-partial) AdvanceFrame.
    virtual void        OnNextFrame() =0; 
    // called when mobile device orientation is changed
    virtual void        OnDeviceOrientationChanged(const OrientationEvent&) {}
    virtual void        OnAppLifecycleEvent(const AppLifecycleEvent&) {}

	virtual void		OnAccelerometerEvent(const AccelerometerEvent&) {}
	virtual void		OnGeolocationEvent(const GeolocationEvent&) {}

    virtual void        PrintObjectsReport(UInt32 flags = 0, 
                                           Log* log = NULL,  
                                           const char* swfName = NULL) { SF_UNUSED3(flags, log, swfName); }
    virtual void        GetObjectsTree(Array<Ptr<AmpMovieObjectDesc> >* rootObjects, MemoryHeap* heap) { SF_UNUSED2(rootObjects, heap); }
    virtual AmpMovieObjectDesc* GetDisplayObjectsTree(MemoryHeap* heap) const { SF_UNUSED(heap); return NULL; }

    // Process load queue entry (single threaded)
    virtual void        ProcessLoadQueueEntry(GFx::LoadQueueEntry *pentry, LoadStates* pls) =0;
    virtual void        ProcessLoadVarsMT
        (GFx::LoadQueueEntry *pentry, LoadStates* pls, 
        const String& data, UPInt fileLen, bool succeeded) =0;
    virtual void        ProcessLoadBinaryMT
        (GFx::LoadQueueEntry *pentry, LoadStates* pls, 
        const ArrayPOD<UByte>& data, UPInt fileLen, bool succeeded) { SF_UNUSED5(pentry,pls,data,fileLen,succeeded); }

    // Register AS classes defined in aux libraries
    virtual void        RegisterAuxASClasses() =0;
    virtual void        ResolveStickyVariables(InteractiveObject *pcharacter) =0;
    virtual void        SetExternalInterfaceRetVal(const GFx::Value&) =0;
    virtual void        SetMemoryParams(unsigned frameBetweenCollections, unsigned maxRootCount) =0;
    virtual void        Shutdown() =0;

    // External API
    virtual void        CreateString(GFx::Value* pvalue, const char* pstring) = 0;
    virtual void        CreateStringW(GFx::Value* pvalue, const wchar_t* pstring) = 0;
    virtual void        CreateObject(GFx::Value* pvalue, const char* className = NULL, 
                                     const GFx::Value* pargs = NULL, unsigned nargs = 0) = 0;
    virtual void        CreateArray(GFx::Value* pvalue) = 0;
    virtual void        CreateFunction(GFx::Value* pvalue, GFx::FunctionHandler* pfc, 
                                     void* puserData = NULL) = 0;
    virtual bool        SetVariable(const char* ppathToVar, const GFx::Value& value, Movie::SetVarType setType = Movie::SV_Sticky) =0;
    virtual bool        GetVariable(GFx::Value *pval, const char* ppathToVar) const =0;
    virtual bool        SetVariableArray(Movie::SetArrayType type, const char* ppathToVar,
                                         unsigned index, const void* pdata, unsigned count, Movie::SetVarType setType = Movie::SV_Sticky) =0;
    virtual bool        SetVariableArraySize(const char* ppathToVar, unsigned count, Movie::SetVarType setType = Movie::SV_Sticky) =0;
    virtual unsigned    GetVariableArraySize(const char* ppathToVar) =0;
    virtual bool        GetVariableArray(Movie::SetArrayType type, const char* ppathToVar,
                                         unsigned index, void* pdata, unsigned count) =0;
    virtual bool        IsAvailable(const char* ppathToVar) const =0;
    virtual bool        Invoke(const char* ppathToMethod, GFx::Value *presult, const GFx::Value* pargs, unsigned numArgs) =0;
    virtual bool        Invoke(const char* ppathToMethod, GFx::Value *presult, const char* pargFmt, ...) =0;
    virtual bool        InvokeArgs(const char* ppathToMethod, GFx::Value *presult, const char* pargFmt, va_list args) =0;

    // Accessor to memory context
    virtual MemoryContext* GetMemoryContext() const = 0;

    // Accessor to string manager
    virtual ASStringManager* GetStringManager() const = 0;

    virtual Render::Text::Allocator* GetTextAllocator() = 0;
};

}} // Scaleform::GFx
    
#endif // INC_SF_GFX_ASMOVIEROOTBASE_H

