/**************************************************************************

Filename    :   AS3_MovieRoot.h
Content     :   Implementation of AS3 part of MovieImpl
Created     :   
Authors     :   Artem Bolgar, Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFx_AS3_MovieRoot_H
#define INC_SF_GFx_AS3_MovieRoot_H

#include "GFx/GFx_Types.h"
#include "GFx/GFx_PlayerImpl.h"
#include "GFx/GFx_AS3Support.h"
#include "GFx/AS3/AS3_AvmSprite.h"
#include "GFx/AS3/AS3_AvmStage.h"
#include "GFx/AS3/AS3_VM.h"
#include "GFx/AS3/AS3_AbcDataBuffer.h"
#include "GFx/GFx_Input.h"
#include "Render/Text/Text_Core.h"

#include "GFx/AS3/AS3_Value.h"

#include "GFx/GFx_MediaInterfaces.h"

#ifdef GFX_ENABLE_XML
#include "GFx/XML/XML_Support.h"
#endif

#ifdef GFX_ENABLE_CSS
//#include "GFx/AS3/AS3_StyleSheet.h"
#endif

#if defined(SF_BUILD_DEBUG) || defined(SF_BUILD_DEBUGOPT)
#include "GFx/AS3/AS3_ObjCollector.h"
#endif 

namespace Scaleform {
namespace GFx {

class TimelineDef;
class LogState;
namespace Text { using namespace Render::Text; }

namespace AS3 {

namespace Instances
{
    namespace fl_display
    {
        class Loader;
    }

	namespace fl_external
    {
        class ExtensionContext;
    }

    namespace fl_net
    {
        class URLRequest;
        class URLLoader;
        class Socket;
    }

    namespace fl_sensors
    {
        class Accelerometer;
        class Geolocation;
    }

    namespace fl_utils
    {
        class ByteArray;
    }
}

class AbcDataBuffer;
class SocketThreadMgr;

//  ****** MouseListener

// class MouseListener
// {
// public:
//     virtual ~MouseListener() {}
// 
//     virtual void OnMouseMove(GASEnvironment *penv, unsigned mouseIndex) const = 0;
//     virtual void OnMouseDown(GASEnvironment *penv, unsigned mouseIndex, unsigned button, InteractiveObject* ptarget) const = 0;
//     virtual void OnMouseUp(GASEnvironment *penv, unsigned mouseIndex, unsigned button, InteractiveObject* ptarget) const   = 0;
//     virtual void OnMouseWheel(GASEnvironment *penv, unsigned mouseIndex, int sdelta, InteractiveObject* ptarget) const = 0;
// 
//     virtual bool IsEmpty() const = 0;
// };

class NotifyLoadInitC: public RefCountBase<NotifyLoadInitC, Stat_Default_Mem>
{
public:
	void virtual OpenEventCallback() = 0;
	void virtual InitEventCallback() = 0;
	void virtual ErrorEventCallback() = 0;
	void virtual LoadEventCallback() = 0;
};

class LoadQueueEntry : public GFx::LoadQueueEntry
{
public:
    SPtr<Instances::fl_display::Loader>     mLoader;
    SPtr<Instances::fl_net::URLLoader>  mURLLoader;
    SPtr<Instances::fl_net::URLRequest> mURLRequest;
    AutoPtr<ArrayPOD<UByte> >   mBytes; // Loader.loadBytes uses this
	Ptr<NotifyLoadInitC>		NotifyLoadInitCInterface;
    bool                        FirstExec;

    //int                  Level;
    //Ptr<CharacterHandle> pCharacter;
    // movie clip loader and variables loader should be held by Value because 
    // there is a cross-reference inside of their instances: loader._listener[0] = loader. 
    // Value can release such cross-referenced objects w/o memory leak, whereas Ptr can't.
//    Value                MovieClipLoaderHolder;
//    Value                LoadVarsHolder;
// #ifdef GFX_ENABLE_XML
//     // the following structure is used to load xml files. the Value holds the
//     // actionscript object (explanation above). The loader is kept as a thread safe
//     // reference. this seperation is required because the loader object is used
//     // inside the loading thread in progressive loading mode. this causes problems
//     // with the Value reference.
//     struct XMLHolderType
//     {
//         Value                   ASObj;
//         Ptr<GFxASXMLFileLoader> Loader;
//     };
//     XMLHolderType       XMLHolder;
// #endif

// #ifdef GFX_ENABLE_CSS
//     // the following structure is used to load xml files. the Value holds the
//     // actionscript object (explanation above). The loader is kept as a thread safe
//     // reference. this seperation is required because the loader object is used
//     // inside the loading thread in progressive loading mode. this causes problems
//     // with the Value reference.
//     struct CSSHolderType
//     {
//         Value                   ASObj;
//         Ptr<GFxASCSSFileLoader> Loader;
//     };
//     CSSHolderType               CSSHolder;
// #endif

    // Constructor helper.      
    LoadQueueEntry(Instances::fl_net::URLRequest* request, 
                   Instances::fl_display::Loader* loader, 
                   LoadMethod method, bool quietOpen = false);
    LoadQueueEntry(Instances::fl_utils::ByteArray* bytes, 
                   Instances::fl_display::Loader* loader, 
                   LoadMethod method);
    LoadQueueEntry(Instances::fl_net::URLRequest* request, 
                   Instances::fl_net::URLLoader* loader, 
                   LoadMethod method, bool quietOpen = false);
    virtual ~LoadQueueEntry();

	void SetNotifyLoadInitCInterface(Ptr<NotifyLoadInitC> pinterface = 0)
	{
		NotifyLoadInitCInterface = pinterface;
	}
    void Cancel(LoadQueueEntry* e)
    {
        if (mLoader && mLoader == e->mLoader)
            Canceled = true;
        else if (mURLLoader && mURLLoader == e->mURLLoader)
            Canceled = true;
    }

// #ifdef GFX_ENABLE_XML
//     void CancelByXMLASObjPtr(Object* pxmlobj)
//     {
//         if (!XMLHolder.ASObj.IsUndefined() && pxmlobj == XMLHolder.ASObj.ToObject(NULL))
//             Canceled = true;
//     }
// #endif
// #ifdef GFX_ENABLE_CSS
//     void CancelByCSSASObjPtr(Object* pobj)
//     {
//         if (!CSSHolder.ASObj.IsUndefined() && pobj == CSSHolder.ASObj.ToObject(NULL))
//             Canceled = true;
//     }
// #endif
};

//  ****** GFxLoadingMovieEntry
class LoadQueueEntryMT_LoadMovie : public GFx::LoadQueueEntryMT
{
    friend class GFx::MovieImpl;

    Ptr<MoviePreloadTask>   pPreloadTask;
//    Ptr<Sprite>              pNewChar;
//    bool                     FirstCheck;
//    Ptr<InteractiveObject>   pOldChar;      
//    ResourceId               NewCharId;
    bool                     CharSwitched;
    unsigned                 BytesLoaded;
    bool                     FirstFrameLoaded;
public:
    LoadQueueEntryMT_LoadMovie(LoadQueueEntry* pqueueEntry, GFx::MovieImpl* pmovieRoot);
    ~LoadQueueEntryMT_LoadMovie();

    // Check if a movie is loaded. Returns false in the case if the movie is still being loaded.
    // Returns true if the movie is loaded completely or in the case of errors.
    bool LoadFinished();
    bool IsPreloadingFinished();

};

// Memory context implementation specific to AS2
class MemoryContextImpl : public MemoryContext
{
public:
    MemoryHeap*                 Heap;

    Ptr<ASStringManager>        StringMgr; // should come before ASGC for
                                           // correct deinit order

#ifdef GFX_AS_ENABLE_GC 
    Ptr<ASRefCountCollector>    ASGC;
#endif
    Ptr<Text::Allocator>        TextAllocator;

    struct HeapLimit : MemoryHeap::LimitHandler
    {
        enum
        {
            INITIAL_DYNAMIC_LIMIT = (128*1024)
        };

        MemoryContextImpl*  MemContext;
        UPInt               UserLevelLimit;
        UPInt               LastCollectionFootprint;
        UPInt               CurrentLimit;
        float               HeapLimitMultiplier;

        HeapLimit() :  MemContext(NULL), UserLevelLimit(0), LastCollectionFootprint(0), 
            CurrentLimit(0), HeapLimitMultiplier(0.25) {}

        void Collect(MemoryHeap* heap);
        void Reset(MemoryHeap* heap);

        virtual bool OnExceedLimit(MemoryHeap* heap, UPInt overLimit);
        virtual void OnFreeSegment(MemoryHeap* heap, UPInt freeingSize);
    } LimHandler;

    MemoryContextImpl() : Heap(NULL)
    {
        LimHandler.MemContext = this;
    }
    ~MemoryContextImpl()
    {
        Heap->SetLimitHandler(NULL);
    }
};

// Extension of AS3::VM to hold pMovieRoot and to provide access
// to it from C++ implementations of AS3 classes.
class ASVM : public VM
{
protected:
    MovieRoot*          pMovieRoot;
public:
    SPtr<Class>         GraphicsClass;
    SPtr<Class>         TransformClass;
    SPtr<Class>         MatrixClass;
    SPtr<Class>         Matrix3DClass;
    SPtr<Class>         PerspectiveProjectionClass;
    SPtr<Class>         ColorTransformClass;
    SPtr<Class>         EventClass;
    SPtr<Class>         MouseEventClass;
    SPtr<Class>         MouseEventExClass;
    SPtr<Class>         KeyboardEventClass;
    SPtr<Class>         KeyboardEventExClass;
#if defined(GFX_ENABLE_ANALOG_GAMEPAD)
    SPtr<Class>         GamePadAnalogEventClass;
#endif
    SPtr<Class>         FocusEventClass;
    SPtr<Class>         FocusEventExClass;
    SPtr<Class>         TextEventClass;
    SPtr<Class>         TextEventExClass;
    SPtr<Class>         TimerEventClass;
    SPtr<Class>         ProgressEventClass;
#ifdef SF_AS3_VERSION_AIR
    SPtr<Class>         OutputProgressEventClass;
#endif
#if defined(GFX_MULTITOUCH_SUPPORT_ENABLE)
    SPtr<Class>         TouchEventClass;
    SPtr<Class>         GestureEventClass;
    SPtr<Class>         TransformGestureEventClass;
    SPtr<Class>         PressAndTapGestureEventClass;
#endif
    SPtr<Class>         StageOrientationEventClass;
    SPtr<Class>         AppLifecycleEventClass;
	SPtr<Class>         StatusEventClass;
    SPtr<Class>         AccelerometerEventClass;
    SPtr<Class>         GeolocationEventClass;
    SPtr<Class>         PointClass;
    SPtr<Class>         RectangleClass;
    SPtr<Class>         TextFormatClass;
    SPtr<Class>         EventDispatcherClass;
    SPtr<Class>         Vector3DClass;
    SPtr<Object>        GeneralClipboard;

    bool                ExtensionsEnabled;

    class AbcFileWithMovieDef : public Abc::File
    {
    public:
        Ptr<MovieDefImpl>   pDefImpl;
        Ptr<AbcDataBuffer>  pAbcData;

        AbcFileWithMovieDef(MovieDefImpl* pdefImpl, AbcDataBuffer* data)
            : pDefImpl(pdefImpl), pAbcData(data) {}
        ~AbcFileWithMovieDef()
        {
            SF_DEBUG_MESSAGE1(GFX_UNLOAD_TRACE_ENABLED, 
                "~AbcFileWithMovieDef %s\n", pAbcData->FileName.ToCStr());
        }
    };

private:
    bool                    _constructInstance(SPtr<Object> &pobj, Object* classObj,
                                               unsigned argc, const Value* argv);
public:
    ASVM(MovieRoot* pmr, FlashUI& _ui, AS3::StringManager& sm, ASRefCountCollector& gc);

    MovieRoot*              GetMovieRoot() const { return pMovieRoot; }
    inline GFx::MovieImpl*  GetMovieImpl() const;

    inline MemoryHeap*      GetHeap() const;
    
    SF_AMP_CODE(virtual AMP::ViewStats* GetAdvanceStats() const;)
    inline Log*             GetLog() const;
    inline LogState*        GetLogState() const;


    inline const ASString&  GetBuiltin(BuiltinType btype) const;

    template <class T>
    bool                    ConstructInstance(SPtr<T> &pobj, Object* classObj,
                                              unsigned argc = 0, const Value* argv = 0)
    {
        SF_ASSERT(((UPInt)static_cast<Object*>((T*)0)) == 0);
        SF_ASSERT(!IsException());
        return _constructInstance(
            reinterpret_cast<SPtr<Object>&>(pobj), classObj, argc, argv);
    }

    // returns a moviedef for the last abc in call stack
    MovieDefImpl*           GetResourceMovieDef(Instances::fl::Object* instance) const;

#if defined(SF_OBJ_COLLECTOR_ENABLED)
    virtual void            Collect(ObjectCollector&);
#endif
};

// This class represents chains of display object, which
// need to receive certain events. An example of such chain - 
// enterFrame event. It is necessary to have a list (array) of 
// all objects that can receive enterFrame event.
// Another events like this are 'activate', 'deactivate', 'render', 'frameConstructed', 'exitFrame'.
class EventChains
{
public:
    friend class MovieRoot;
    typedef ArrayLH<WeakPtr<DisplayObject> >     Chain;
    typedef HashIdentityLH<int, AutoPtr<Chain> > ChainHash;
private:
    //struct Chain : ArrayLH<Ptr<DisplayObject> > {};
    ChainHash Chains;

    UPInt FindObjectIndexInChain(Chain* chain, DisplayObject* obj);

public:
    void AddToChain(EventId::IdCode evtId, DisplayObject* obj);
    void RemoveFromChain(EventId::IdCode evtId, DisplayObject* obj);
    const Chain* GetChain(EventId::IdCode evtId) const;
    void Dispatch(EventId::IdCode evtId);
    void QueueEvents(EventId::IdCode evtId);

    void Shutdown() { Chains.Clear(); }
#if defined(SF_OBJ_COLLECTOR_ENABLED)
    void Collect(ObjectCollector&) const;
#endif
};

class MovieDefRootNode : public GFx::MovieDefRootNode
{
public:
    MovieDefRootNode(MovieDefImpl *pdefImpl, bool importFlag = 0) 
        : GFx::MovieDefRootNode(pdefImpl, importFlag) {}
    ~MovieDefRootNode();

    ArrayLH<SPtr<VMAbcFile> > AbcFiles;
};

// Implementation of ASMovieRootBase for AS 3.0. The pointer to this instance
// is stored inside of MovieImpl.
class MovieRoot :   public ASMovieRootBase, 
                    public FlashUI, 
                    public KeyboardState::IListener,
                    public MovieDefImpl::ReleaseNotifier
{
    friend class AvmSprite;
    friend class Stage;
    friend class LoadQueueEntryMT_LoadMovie;
protected:
    struct MemContextPtr : public Ptr<MemoryContextImpl>
    {
        MemContextPtr(MemoryContextImpl* p) : Ptr<MemoryContextImpl>(p) {}
        MemContextPtr(const MemContextPtr& p) : Ptr<MemoryContextImpl>(p) {}
        MemContextPtr(Ptr<MemoryContextImpl>& p) : Ptr<MemoryContextImpl>(p) {}
        ~MemContextPtr()
        {
            // Make sure no uncollected objects left in garbage collector.
            // Should be called after VM is destroyed but before MovieRoot does.
            GetPtr()->ASGC->ForceCollect(NULL, ASRefCountCollector::Collect_Shutdown);
        }
    };
    // Memory heap context
    MemContextPtr       MemContext; // must be declared first!
    AutoPtr<ASVM>       pAVM;       // must be declared second!
public:
    enum ActionLevel
    {
        AL_Highest      = 0,
        AL_High,
        AL_EnterFrame,
        AL_Frame,
        AL_ControllerEvents,
        AL_Render,

        AL_Count_,
        AL_Lowest      = AL_Count_ - 1,
        AL_Manual = AL_Count_, // special queue for manual execution
        AL_Total
    };
    struct MouseState
    {
        enum
        {
            MouseButton_Left = GFx::MouseState::MouseButton_Left,
            MouseButton_Middle = GFx::MouseState::MouseButton_Middle,
            MouseButton_Right = GFx::MouseState::MouseButton_Right,
            MouseButton_MaxNum = GFx::MouseState::MouseButton_MaxNum
        };

        ArrayLH<Ptr<InteractiveObject>, StatMV_ActionScript_Mem>    RolloverStack;
        Ptr<InteractiveObject>                                      LastMouseOverObj;
        struct DoubleClickInfo
        {
            UInt32 PrevClickTime;
            PointF PrevPosition;

            DoubleClickInfo():PrevClickTime(0) { }
        }                                                           DblClick[MouseButton_MaxNum];

        DoubleClickInfo& GetDoubleClickInfo(unsigned buttonMask);

        void Clear() { RolloverStack.Clear(); LastMouseOverObj = NULL;}
    };

    // Return value storage for ExternalInterface.call.
    Value                   ExternalIntfRetVal;

protected:

    // Keep track of the number of advances for shared GC
    unsigned                NumAdvancesSinceCollection;
    unsigned                LastCollectionFrame;

    ASStringHash<Value>*    pInvokeAliases; // aliases set by ExtIntf.addCallback

#ifdef GFX_MULTITOUCH_SUPPORT_ENABLE
    // this descriptor is necessary to track pressAndTap and twoFingerTap gestures
    struct PrimaryPointPosInfo
    {
        Ptr<InteractiveObject>  OverObj;
        PointF                  Position;
        UInt32                  ClickTime;

        PrimaryPointPosInfo():ClickTime(0) {}
    }	PrimaryTouchPointPos; // stores info about last primary touch point
#endif
public:
    // Action queue is stored as a singly linked list queue. List nodes must be traversed
    // in order for execution. New actions are inserted at the insert location, which is
    // commonly the end; however, in some cases insert location can be modified to allow
    // insertion of items before other items.
    // Action list to be executed 
    // Must be public for CFunction.
    struct ActionEntry : public NewOverrideBase<StatMV_ActionScript_Mem>
    {
        enum EntryType
        {
            Entry_None,
            Entry_Event,        // 
            Entry_Function,
            Entry_CFunction
        };
        typedef void (*CFunctionPtr)(const ActionEntry&);

        ActionEntry*        pNextEntry;
        EntryType           Type;
        Ptr<DisplayObject>  pCharacter;
        SPtr<Instances::fl_display::DisplayObject> pAS3Obj;
        EventId             mEventId;
        Value               Function;
        CFunctionPtr        CFunction;
		Ptr<NotifyLoadInitC> pNLoadInitCL;
        SF_INLINE ActionEntry();
        SF_INLINE ActionEntry(const ActionEntry &src);
        SF_INLINE const ActionEntry& operator = (const ActionEntry &src);

        // Helper constructors
        SF_INLINE ActionEntry(DisplayObject *pcharacter, const EventId& id);

        SF_INLINE void SetAction(DisplayObject *pcharacter, const EventId& id);
        SF_INLINE void SetAction(DisplayObject *pcharacter, const Value& v);
        SF_INLINE void SetAction(DisplayObject *pcharacter, Instances::fl_display::DisplayObject* pas3, CFunctionPtr func);
        SF_INLINE void ClearAction();

        // Executes actions in this entry
        void    Execute(MovieRoot *proot) const;
    private:
        bool operator==(const ActionEntry&) const { return false; }
    };
protected:
    struct ActionQueueEntry
    {
        // This is a root of an action list. Root node action is 
        // always Entry_None and thus does not have to be executed.
        ActionEntry*    pActionRoot;

        // This is an action insert location. In a beginning, &ActionRoot, afterwards
        // points to the node after which new actions are added.
        ActionEntry*    pInsertEntry;

        // Pointer to a last entry
        ActionEntry*    pLastEntry;

        ActionQueueEntry() { Reset(); }
        inline void Reset() { pActionRoot = pLastEntry = pInsertEntry = NULL; }
    };
    struct ActionQueueType
    {
        ActionQueueEntry    Entries[AL_Total];
        // This is a modification id to track changes in queue during its execution
        int                 ModId;
        // This is a linked list of un-allocated entries.   
        ActionEntry*        pFreeEntry;
        unsigned            FreeEntriesCount;

        MemoryHeap*         pHeap;

        ActionQueueType(MemoryHeap* pheap);
        ~ActionQueueType();

        MemoryHeap*                 GetMovieHeap() const { return pHeap; }

        ActionEntry*                GetNewEntry();
        ActionEntry*                InsertEntry(ActionLevel lvl);
        ActionEntry*                PrependEntry(ActionLevel lvl);
        void                        AddToFreeList(ActionEntry*);
        void                        Clear();
        ActionEntry*                GetInsertEntry(ActionLevel lvl) const
        {
            return Entries[lvl].pInsertEntry;
        }
        ActionEntry*                SetInsertEntry(
            ActionLevel lvl, ActionEntry* pinsertEntry)
        {
            ActionEntry* pie = Entries[lvl].pInsertEntry;
            Entries[lvl].pInsertEntry = pinsertEntry;
            return pie;
        }
        //ActionEntry*                FindEntry(ActionLevel lvl, const ActionEntry&);
        bool                        HasEntries(ActionLevel lvl) const 
        { 
            SF_ASSERT(lvl < AL_Total);
            return Entries[lvl].pActionRoot != 0; 
        }
    };
    struct ActionQueueIterator
    {
        int              ModId;
        ActionQueueType* pActionQueue;
        ActionEntry*     pCurEntry;
        ActionEntry*     pRootEntry;
        ActionEntry*     pLastEntry;
        ActionLevel      Level;

        ActionQueueIterator(ActionLevel lvl, ActionQueueType*, ActionEntry* rootEntry = NULL);
        ~ActionQueueIterator();

        const ActionEntry*          getNext();
    };

    ActionQueueType                 ActionQueue;

    EventChains                     mEventChains;

    Ptr<Stage>                      pStage;
    StringManager                   BuiltinsMgr;

    unsigned                        ASFramesToExecute;

    MouseState                      mMouseState[GFX_MAX_MICE_SUPPORTED];
    bool                            StageInvalidated;

    enum
    {
        MLISE_InitExecuted = 1,
        MLISE_Completed = 2
    };
    UInt8                           MainLoaderInfoEventsState;

    // AS3 implementation of StickyVarNode
    struct StickyVarNode : public GFx::MovieImpl::StickyVarNode
    {
        Value mValue;

        StickyVarNode(const ASString& name, const Value& value, bool permanent)
            : GFx::MovieImpl::StickyVarNode(name, permanent), mValue(value) { }
        StickyVarNode(const StickyVarNode &node)
            : GFx::MovieImpl::StickyVarNode(node), mValue(node.mValue) { }

        virtual void Assign(const GFx::MovieImpl::StickyVarNode &node)
        {
            GFx::MovieImpl::StickyVarNode::Assign(node);
            mValue = static_cast<const StickyVarNode&>(node).mValue;
        }
    };

    struct LoadedMovieDefInfo
    {
        //MovieDefImpl*   pMovieDef;
        unsigned        UseCnt;

        LoadedMovieDefInfo() : UseCnt(1) {}
    };
    HashIdentityLH<MovieDefImpl*, LoadedMovieDefInfo> LoadedMovieDefs;

    enum 
    {
        Mask_InsideGotoFrame = 0x01
    };
    UInt8                   Flags;

    // Net Socket support
#ifdef SF_ENABLE_SOCKETS
    ArrayLH<Ptr<SocketThreadMgr> >  Sockets;   // Need to keep a list of these here for event notifications
    SocketImplFactory*              SocketFactory;
#endif

public:

    MovieRoot(MemoryContextImpl* memContext, GFx::MovieImpl* pmovie, ASSupport* pas);
    ~MovieRoot();

    SF_INLINE MovieDefImpl*     GetMovieDefImpl() const { return pMovieImpl->GetMovieDefImpl(); }
    SF_INLINE void              SetMovieDefImpl(MovieDefImpl* p) const { pMovieImpl->SetMovieDefImpl(p); }
    SF_INLINE MemoryHeap*       GetMovieHeap() const { return pMovieImpl->GetMovieHeap(); }
    SF_INLINE AS3::StringManager* GetStringManager() { return &BuiltinsMgr; } // revise @TODO GetBuiltinsMgr?
    SF_INLINE Log*              GetLog() const { return pMovieImpl->GetLog(); }
    SF_INLINE LogState*         GetLogState() const { return pMovieImpl->GetLogState(); }
    const StringManager&        GetBuiltinsMgr() const { return BuiltinsMgr; } // revise @TODO GetStringManager?
	
    const MouseState* GetAS3MouseState(unsigned mouseIndex) const
    {
        if (mouseIndex >= GFX_MAX_MICE_SUPPORTED)
            return NULL;
        return &mMouseState[mouseIndex];
    }

    void SetInsideGotoFrameFlag(bool v = true) { (v) ? Flags |= Mask_InsideGotoFrame : Flags &= (~Mask_InsideGotoFrame); }
    void ClearInsideGotoFrameFlag()            { SetInsideGotoFrameFlag(false); }
    bool IsInsideGotoFrameFlagSet() const      { return (Flags & Mask_InsideGotoFrame) != 0; }

    // fills path to the root swf file
    bool GetRootFilePath(String *ppath) const;
    // Load queue

    // Adds load queue entry based on parsed url and target path.
    SF_INLINE void      AddLoadQueueEntry(LoadQueueEntry *pentry)
    {
        pMovieImpl->AddLoadQueueEntry(pentry);
    }
    void                AddLoadQueueEntryMT(LoadQueueEntry *pqueueEntry);

    void                AddNewLoadQueueEntry(
        Instances::fl_net::URLRequest* urlRequest,
        Instances::fl_display::Loader* loader,
        LoadQueueEntry::LoadMethod method = LoadQueueEntry::LM_None);

    void                AddNewLoadQueueEntry(
        Instances::fl_utils::ByteArray* bytes,
        Instances::fl_display::Loader* loader,
        LoadQueueEntry::LoadMethod method = LoadQueueEntry::LM_None);

    void                AddNewLoadQueueEntry(
        Instances::fl_net::URLRequest* urlRequest,
        Instances::fl_net::URLLoader* loader,
        LoadQueueEntry::LoadMethod method = LoadQueueEntry::LM_None);

    // Creates an GFx::Value::ObjectInterface from MovieImpl; called from constructor.
    void CreateObjectInterface(GFx::MovieImpl* movie);

    // Inserts an empty action and returns a pointer to it. Should call SetAction afterwards.   
    ActionEntry*        InsertEmptyAction(ActionLevel lvl) 
    { 
        return ActionQueue.InsertEntry(lvl); 
    }
    // Prepends (inserts at the begining) an empty action
    ActionEntry*        PrependEmptyAction(ActionLevel lvl) 
    { 
        return ActionQueue.PrependEntry(lvl); 
    }
    void*               GetActionQueueInsertionPoint(ActionLevel lvl) const
    {
        return ActionQueue.GetInsertEntry(lvl);
    }
    void                ExecuteActionQueue(ActionLevel lvl);
    // starts exectution from rootEntry->pNextEntry
    void                ExecuteActionQueue(ActionLevel lvl, void* rootEntry);
    void                ExecuteCtors();
    void                ExecuteActionQueues();
    void                RequeueActionQueue(ActionLevel srclvl, ActionLevel dstlvl);
    // queue all planned frame actions; it does not execute them, use ExecuteActionQueues.
    void                QueueFrameActions();
    void                RemoveActionQueueEntriesFor(ActionLevel lvl, DisplayObject* dobj);

    // net socket support
#ifdef SF_ENABLE_SOCKETS
    void                CheckSocketMessages();
    SocketThreadMgr*    AddSocket(bool initSockLib, Instances::fl_net::Socket* sock);
    void                SetSocketImplFactory(SocketImplFactory* factory);
#endif
    // FlashUI methods
    virtual void Output(FlashUI::OutputMessageType type, const char* msg);    
    virtual bool OnOpCode(Abc::TCodeOffset offset);

    //     virtual void        AddStickyVariable
    //         (const GASString& fullPath, const GFxValue &val, Movie::SetVarType setType) =0;
    virtual void        AdvanceFrame(bool nextFrame);
    // Check for AVM; create it, if it is not created yet.
    virtual bool        CheckAvm();
    virtual void        ChangeMouseCursorType(unsigned mouseIdx, unsigned newCursorType);
    virtual void        ClearDisplayList();
    virtual MovieDefRootNode* CreateMovieDefRootNode(MemoryHeap* pheap, MovieDefImpl *pdefImpl, bool importFlag = false);
    virtual void        DoActions();
    virtual InteractiveObject* FindTarget(const ASString& path) const { SF_UNUSED(path); return 0; }
    // forces garbage collection (if GC is enabled)
    virtual void        ForceCollect(unsigned gcFlags);
    // forces emergency garbage collection (if GC is enabled). This method is called
    // when heap is overflown. 
    virtual void        ForceEmergencyCollect();
    virtual void        SuspendGC(bool suspend);
    virtual void        ScheduleGC(unsigned gcFlags);
    // Generate button events (onRollOver, onPress, etc)
    virtual void        GenerateMouseEvents(unsigned mouseIndex);
#ifdef GFX_MULTITOUCH_SUPPORT_ENABLE
    virtual void        GenerateTouchEvents(unsigned mouseStateIndex);
    virtual void        GenerateGestureEvents(InteractiveObject* topmost, 
                                              unsigned phase,
                                              const PointF& pos, 
                                              const PointF& offset,
                                              const PointF& scale,
                                              float rotation,
                                              unsigned gestureMask);
#endif

    // This method returns the root display object container for the specified object. 
    // The "dobj" parameter might be NULL; in this case, for AS2 this method will return _root 
    // (same as GetMainContainer), for AS3 it will return "root".
    // If "dobj" is not NULL and it belongs to SWF loaded by loadMovie then the result is the
    // parent movie on which "loadMovie" was called.
    virtual DisplayObjContainer* GetRootMovie(DisplayObject* dobj = NULL) const;

    virtual bool        Init(MovieDefImpl* pmovieDef);
    virtual void        NotifyMouseEvent(const InputEventsQueueEntry* qe, const GFx::MouseState& ms, int mi) 
    { SF_UNUSED3(qe, ms, mi); }
    virtual void        NotifyOnResize();
    virtual void        NotifyQueueSetFocus(InteractiveObject* ch, unsigned controllerIdx, FocusMovedType fmt);
    virtual void        NotifyTransferFocus(InteractiveObject* curFocused, InteractiveObject* pNewFocus, unsigned controllerIdx);
    // If returns false, then focus change is denied.
    virtual bool        NotifyOnFocusChange(InteractiveObject* curFocused, InteractiveObject* toBeFocused, 
                                            unsigned controllerIdx, FocusMovedType fmt, ProcessFocusKeyInfo* pfocusKeyInfo = NULL);

#ifdef GFX_ENABLE_ANALOG_GAMEPAD
    virtual void        NotifyGamePadAnalogEvent(const EventId* id);
#endif
	virtual void        NotifyStatusEvent(const EventId* id);

    virtual void        NotifyAccelerometerEvent(const EventId* id, int evtIdAcc);
	virtual void		BroadcastGeolocationStatusEvent(const EventId* id);
    virtual void        NotifyGeolocationEvent(const EventId* id, int evtIdGeo);

    // invoked when whole movie view gets or loses focus
    virtual void        OnMovieFocus(bool set);
    // called BEFORE processing complete (non-partial) AdvanceFrame.
    virtual void        OnNextFrame(); 

#ifdef GFX_ENABLE_MOBILE_APP_SUPPORT
    virtual void        OnAppLifecycleEvent(const AppLifecycleEvent&);

    // called when mobile device orientation is changed
    virtual void        OnDeviceOrientationChanged(const OrientationEvent&);
#endif

    virtual MemoryContext* GetMemoryContext() const
    {
        return MemContext;
    }
    // Accessor to string manager
    virtual ASStringManager* GetStringManager() const { return MemContext->StringMgr; }

    virtual Text::Allocator* GetTextAllocator();

    virtual void        PrintObjectsReport(UInt32 flags = 0, 
                                           Log* log = NULL,  
                                           const char* swfName = NULL);
    virtual void        GetObjectsTree(Array<Ptr<AmpMovieObjectDesc> >* rootObjects, MemoryHeap* heap);
    AmpMovieObjectDesc* GetDisplayObjectsTree(MemoryHeap* heap) const;

    virtual void        ProcessLoadQueueEntry(GFx::LoadQueueEntry *pentry, LoadStates* pls);
    virtual void        ProcessLoadVarsMT
        (GFx::LoadQueueEntry *pentry, LoadStates* pls, const String& data, 
        UPInt fileLen, bool succeeded);
    virtual void        ProcessLoadBinaryMT
        (GFx::LoadQueueEntry *pentry, LoadStates* pls, 
        const ArrayPOD<UByte>& data, UPInt fileLen, bool succeeded);

    // Register AS classes defined in aux libraries
    virtual void        RegisterAuxASClasses() {}
    virtual void        ResolveStickyVariables(InteractiveObject *pcharacter);
    virtual void        SetExternalInterfaceRetVal(const GFx::Value&);
    virtual void        SetMemoryParams(unsigned frameBetweenCollections, unsigned maxRootCount) 
    { SF_UNUSED2(frameBetweenCollections, maxRootCount); }
    virtual void        Shutdown();

    // Value conversion for external communication
    void                GFxValue2ASValue(const GFx::Value& gfxVal, Value* pdestVal);
    void                ASValue2GFxValue(const Value& value, GFx::Value* pdestVal) const;
    bool                CreateObjectValue(GFx::Value* pvalue, GFx::Value::ObjectInterface* pobjifc, 
                                          void* pdata, bool isdobj);

    // External API
    virtual void        CreateString(GFx::Value* pvalue, const char* pstring);
    virtual void        CreateStringW(GFx::Value* pvalue, const wchar_t* pstring);
    virtual void        CreateObject(GFx::Value* pvalue, const char* className = NULL,
                                     const GFx::Value* pargs = NULL, unsigned nargs = 0);
    virtual void        CreateArray(GFx::Value* pvalue);
    virtual void        CreateFunction(GFx::Value* pvalue, GFx::FunctionHandler* pfc, 
                                     void* puserData = NULL);

    virtual bool        GetVariable(GFx::Value *pval, const char* ppathToVar) const;
    virtual bool        SetVariable(const char* ppathToVar, const GFx::Value& value, 
                                    Movie::SetVarType setType = Movie::SV_Sticky);
    virtual bool        GetVariableArray(Movie::SetArrayType type, const char* ppathToVar,
                                    unsigned index, void* pdata, unsigned count);
    virtual unsigned    GetVariableArraySize(const char* ppathToVar);
    virtual bool        SetVariableArray(Movie::SetArrayType type, const char* ppathToVar,
                                    unsigned index, const void* pdata, unsigned count, 
                                    Movie::SetVarType setType = Movie::SV_Sticky);
    virtual bool        SetVariableArraySize(const char* ppathToVar, unsigned count, 
                                    Movie::SetVarType setType = Movie::SV_Sticky);
    virtual bool        IsAvailable(const char* ppathToVar) const;
    virtual bool        Invoke(const char* pmethodName, GFx::Value *presult, 
                                    const GFx::Value* pargs, unsigned numArgs); 
    virtual bool        Invoke(const char* pmethodName, GFx::Value *presult, 
                                    const char* pargFmt, ...);
    virtual bool        InvokeArgs(const char* pmethodName, GFx::Value *presult, 
                                   const char* pargFmt, va_list args);

    // KeyboardState::IListener implementation
    // KeyboardState::IListener methods
    virtual void        OnKeyDown(InteractiveObject *pmovie, const EventId& evt, int keyMask);
    virtual void        OnKeyUp(InteractiveObject *pmovie, const EventId& evt, int keyMask);
    virtual void        Update(const EventId& evt);

    Sprite*             CreateMovieClip(TimelineDef* pdef, MovieDefImpl* pdefImpl,
                                        InteractiveObject* parent, ResourceId id, 
                                        bool loadedSeparately = false);

    Sprite*             CreateSprite(TimelineDef* pdef, MovieDefImpl* pdefImpl,
                                     InteractiveObject* parent, ResourceId id, 
                                     bool loadedSeparately = false);

    AvmStage*           CreateStage(MovieDefImpl* pdefImpl);

    DisplayObjContainer*    GetMainContainer() const 
    { 
        return static_cast<DisplayObjContainer*>(pMovieImpl->GetMainContainer()); 
    }
    AvmDisplayObjContainer* GetAvmMainMovie() const 
    {
        return ToAvmDisplayObjContainer(GetMainContainer());
    }
    DisplayObjContainer* GetMainTimeline() const
    {
        return GetStage()->GetRoot();
    }
    bool                ExecuteAbc(AbcDataBuffer*, DisplayObjContainer* m);
    ASVM*               GetAVM() const { return pAVM.GetPtr(); }
    SPtr<Instances::fl_events::Event> CreateEventObject(const ASString& type, bool bubbles, bool cancelable);

    Stage*              GetStage() const { return pStage; }
    void                IncASFramesCnt(unsigned i) { ASFramesToExecute += i; }

    void                AddToEventChain(EventId::IdCode evtId, DisplayObject* obj)
    {
        mEventChains.AddToChain(evtId, obj);
    }
    void                RemoveFromEventChain(EventId::IdCode evtId, DisplayObject* obj)
    {
        mEventChains.RemoveFromChain(evtId, obj);
    }
    void                QueueEventChain(EventId::IdCode evtId)
    {
        mEventChains.QueueEvents(evtId);
    }

    // accelerometer
    void AddToAccelerometers(Instances::fl_sensors::Accelerometer* pAccelerometer);
    UPInt FindAccelerometer(Instances::fl_sensors::Accelerometer* pAccelerometer);
    void RemoveFromAccelerometers(Instances::fl_sensors::Accelerometer* pAccelerometer);
    AutoPtr<ArrayLH<Instances::fl_sensors::Accelerometer*> > AccelerometerArray;

    // geolocation
    void AddToGeolocations(Instances::fl_sensors::Geolocation* pGeolocation);
    UPInt FindGeolocation(Instances::fl_sensors::Geolocation* pGeolocation);
    void RemoveFromGeolocations(Instances::fl_sensors::Geolocation* pGeolocation);
    AutoPtr<ArrayLH<Instances::fl_sensors::Geolocation*> > GeolocationArray;

	// extension contexts
	void AddToExtensionContexts(Instances::fl_external::ExtensionContext* pExtensionContext);
    UPInt FindExtensionContexts(Instances::fl_external::ExtensionContext* pExtensionContext);
	UPInt FindExtensionContexts(const ASString& extensionID, const ASString& contextType);
	void RemoveFromExtensionContexts(Instances::fl_external::ExtensionContext* pExtensionContext);
	AutoPtr<ArrayLH<Instances::fl_external::ExtensionContext*> > ExtensionContextArray;

    // forces RENDER events to fire up
    void                InvalidateStage() { StageInvalidated = true; }
    void                ValidateStage()   { StageInvalidated = false; }
    bool                IsStageInvalidated() const { return StageInvalidated; }

    void                AddInvokeAlias(const ASString& alias, const Value& closure);
    Value*              ResolveInvokeAlias(const char* pstr);

    void                AddScriptableMovieClip(DisplayObjContainer*);

    // Unloads content of the passed loader, as well as cancels all queued up loads for it.
    // "stop" - used for unloadAndStop;
    // "gc"   - executes GC afterwards. 
    void                UnloadMovie(Instances::fl_display::Loader* ploader, bool stop = false, bool gc = false);
    // cancels loading movie by ploader. Returns false if no active loadings found.
    bool                CancelMovieLoading(Instances::fl_display::Loader* ploader);

    // adds/removes a moviedefimpl to/from holding hash table
    void                AddLoadedMovieDef(MovieDefImpl*);
    // returns true, if the usecnt == 0 for this moviedefimpl
    bool                RemoveLoadedMovieDef(MovieDefImpl*);

    bool                FindLibrarySymbolInAllABCs(const Instances::fl_display::DisplayObject* dobj, 
                                                   CharacterCreateInfo* pccinfo);

    void                SetMouseCursorType(const ASString& cursor, UInt32 mouseIndex);
    void                GetMouseCursorType(ASString& cursor, UInt32 mouseIndex);
protected:
    void                GetMouseCursorTypeString(ASString& result, unsigned cursorType) const;

    void                AddStickyVariable(const ASString& path, const ASString& name, 
                                          const Value &val, Movie::SetVarType setType);
    bool                ExtractPathAndName(const char* fullPath, ASString* ppath, 
                                           ASString* pname);

    bool                GetASVariableAtPath(Value* pval, const char* ppathToVar) const;
    void                ParseValueArguments(Array<Value>& arr, const char* pmethodName, 
                                            const char* pargFmt, va_list args) const;

    virtual void        OnMovieDefRelease(MovieDefImpl*);

#if defined(SF_OBJ_COLLECTOR_ENABLED)
private:
    class CollectorProcessHelper
    {
    public:
        CollectorProcessHelper(MovieRoot* mov);
        ~CollectorProcessHelper();
    private:
        MovieRoot* MovRoot;
    };
public:
    void                CollectObjects(UInt32 flags, 
                                       Log* log,  
                                       const char* swfName);
    void                CollectObjects(Array<Ptr<AmpMovieObjectDesc> >* rootObjects, MemoryHeap* heap);
    ObjectCollector     Collector;
#endif
};

// ** Inline Implementation


SF_INLINE MovieRoot::ActionEntry::ActionEntry()
{ 
    pNextEntry = 0;
    ClearAction(); 
}

SF_INLINE MovieRoot::ActionEntry::ActionEntry(const MovieRoot::ActionEntry &src)
{
    // pNextEntry shouldn't be copied!
    pNextEntry      = 0;
    Type            = src.Type;
    pCharacter      = src.pCharacter;
    pAS3Obj         = src.pAS3Obj;
    mEventId        = src.mEventId;
    CFunction       = src.CFunction;
	pNLoadInitCL    = src.pNLoadInitCL;
}

SF_INLINE const MovieRoot::ActionEntry& 
MovieRoot::ActionEntry::operator = (const MovieRoot::ActionEntry &src)
{   
    // pNextEntry shouldn't be copied!
    Type            = src.Type;
    pCharacter      = src.pCharacter;
    pAS3Obj         = src.pAS3Obj;
    mEventId        = src.mEventId;
    CFunction       = src.CFunction;
	pNLoadInitCL    = src.pNLoadInitCL;
    return *this;
}

SF_INLINE MovieRoot::ActionEntry::ActionEntry(DisplayObject *pcharacter, const EventId& id)
{
    pNextEntry      = 0;
    Type            = Entry_Event;
    pCharacter      = pcharacter;
    mEventId        = id;
    CFunction       = 0;
    pAS3Obj         = 0;
	pNLoadInitCL	= 0;
}

SF_INLINE void MovieRoot::ActionEntry::SetAction(DisplayObject *pcharacter, const EventId& id)
{
    Type            = Entry_Event;
    pCharacter      = pcharacter;
    mEventId        = id;
    CFunction       = 0;
    pAS3Obj         = 0;
    Function.SetUndefined();
    pNLoadInitCL    = 0;
}

SF_INLINE void MovieRoot::ActionEntry::SetAction(DisplayObject *pcharacter, const Value& v)
{
    Type            = Entry_Function;
    pCharacter      = pcharacter;
    Function        = v;
    CFunction       = 0;
    pAS3Obj         = 0;
    pNLoadInitCL    = 0;
}

SF_INLINE void MovieRoot::ActionEntry::SetAction(DisplayObject *pcharacter, 
                                                 Instances::fl_display::DisplayObject* pas3, 
                                                 CFunctionPtr func)
{
    Type            = Entry_CFunction;
    pCharacter      = pcharacter;
    pAS3Obj         = pas3;
    CFunction       = func;
    Function.SetUndefined();
    pNLoadInitCL    = 0;
}

SF_INLINE void MovieRoot::ActionEntry::ClearAction()
{
    Type            = Entry_None;
    pCharacter      = 0;
    pAS3Obj         = 0;
    CFunction       = 0;
    Function.SetUndefined();
    pNLoadInitCL    = 0;
}

SF_INLINE MovieRoot* ToAS3Root(Sprite* spr)
{
    SF_ASSERT(spr && spr->GetASMovieRoot() && spr->GetAVMVersion() == 2);
    return static_cast<MovieRoot*>(spr->GetASMovieRoot());
}

SF_INLINE MovieRoot* ToAS3Root(GFx::MovieImpl* pm)
{
    SF_ASSERT(pm && pm->pASMovieRoot && pm->pASMovieRoot->GetAVMVersion() == 2);
    return static_cast<MovieRoot*>(pm->pASMovieRoot.GetPtr());
}

GFx::MovieImpl* ASVM::GetMovieImpl() const
{ 
    return pMovieRoot->GetMovieImpl(); 
}

MemoryHeap* ASVM::GetHeap() const
{
    return pMovieRoot->GetMovieHeap();
}

inline LogState* ASVM::GetLogState() const 
{ 
    SF_ASSERT(pMovieRoot); return pMovieRoot->GetLogState(); 
}
inline Log* ASVM::GetLog() const 
{ 
    SF_ASSERT(pMovieRoot); return pMovieRoot->GetLog(); 
}

inline const ASString& ASVM::GetBuiltin(BuiltinType btype) const
{
    return GetMovieRoot()->GetBuiltinsMgr().GetBuiltin(btype);
}

const ASString& GetEventString(const StringManager& mgr, const EventId& evt);

}}} // Scaleform::GFx::AS3

#endif // INC_SF_GFx_AS3_MovieRoot_H

