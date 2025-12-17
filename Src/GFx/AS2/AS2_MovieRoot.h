/**************************************************************************

PublicHeader:   GFx
Filename    :   AS2_MovieRoot.h
Content     :   Implementation of AS2 part of MovieImpl
Created     :   
Authors     :   Artem Bolgar, Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_SF_GFX_AS2_MOVIEROOT_H
#define INC_SF_GFX_AS2_MOVIEROOT_H
#include "GFx/GFx_Types.h"
#include "GFx/GFx_PlayerImpl.h"
#include "GFx/GFx_Sprite.h"

#include "GFx/AS2/AS2_AvmSprite.h"
#include "GFx/AS2/AS2_MovieClipLoader.h"
#include "GFx/AS2/AS2_LoadVars.h"
#include "GFx/GFx_MediaInterfaces.h"

#include "GFx/AS2/AS2_StringManager.h"
#include "Render/Text/Text_Core.h"

#ifdef GFX_ENABLE_CSS
#include "GFx/AS2/AS2_StyleSheet.h"
#endif

namespace Scaleform {

namespace GFx {

class TimelineDef;
namespace Text { using namespace Render::Text; }

namespace AS2 {

//  ****** MouseListener

class MouseListener
{
public:
    virtual ~MouseListener() {}

    virtual void OnMouseMove(Environment *penv, unsigned mouseIndex) const = 0;
    virtual void OnMouseDown(Environment *penv, unsigned mouseIndex, unsigned button, InteractiveObject* ptarget) const = 0;
    virtual void OnMouseUp(Environment *penv, unsigned mouseIndex, unsigned button, InteractiveObject* ptarget) const   = 0;
    virtual void OnMouseWheel(Environment *penv, unsigned mouseIndex, int sdelta, InteractiveObject* ptarget) const = 0;

    virtual bool IsEmpty() const = 0;
};


#ifdef GFX_ENABLE_XML

//
// ActionScript XML file loader interface. 
//
// This interface is used by the GFx core to load an XML file to an 
// ActionScript XML object. An instance of this object is provided by 
// the XML library and passed through the movie load queues. The two 
// methods are called at the appropriate moments from within GFx. Its 
// implementation is defined in the XML library. This interface only 
// serves as a temporary wrapper for the duration of passing through 
// the GFx core.
//
class XMLFileLoader : public RefCountBase<XMLFileLoader, StatMV_XML_Mem>
{
public:
    XMLFileLoader() { }
    virtual         ~XMLFileLoader() {}

    //
    // Load a an xml file into a DOM tree using the provide file opener
    //
    virtual void Load(const String& filename, FileOpener* pfo) = 0;

    //
    // Initialize the ActionScript XML object with the loaded DOM
    //
    virtual void    InitASXml(Environment* penv, Object* pTarget) = 0;
};

#endif


class GFxAS2LoadQueueEntry : public LoadQueueEntry
{
public:
    int                     Level;
    Ptr<CharacterHandle>    pCharacter;
    // movie clip loader and variables loader should be held by Value because 
    // there is a cross-reference inside of their instances: loader._listener[0] = loader. 
    // Value can release such cross-referenced objects w/o memory leak, whereas Ptr can't.
    Value                   MovieClipLoaderHolder;
    Value                   LoadVarsHolder;
#ifdef GFX_ENABLE_XML
    // the following structure is used to load xml files. the Value holds the
    // actionscript object (explanation above). The loader is kept as a thread safe
    // reference. this seperation is required because the loader object is used
    // inside the loading thread in progressive loading mode. this causes problems
    // with the Value reference.
    struct XMLHolderType
    {
        Value               ASObj;
        Ptr<XMLFileLoader>  Loader;
    };
    XMLHolderType           XMLHolder;
#endif

#ifdef GFX_ENABLE_CSS
    // the following structure is used to load xml files. the Value holds the
    // actionscript object (explanation above). The loader is kept as a thread safe
    // reference. this seperation is required because the loader object is used
    // inside the loading thread in progressive loading mode. this causes problems
    // with the Value reference.
    struct CSSHolderType
    {
        Value                ASObj;
        Ptr<ASCSSFileLoader> Loader;
    };
    CSSHolderType           CSSHolder;
#endif

    // Constructor helper.      
    void    PConstruct(CharacterHandle* pchar, int level)
    {
        pCharacter  = pchar;
        Level       = level;
    }

    GFxAS2LoadQueueEntry(const String &url, LoadMethod method, bool loadingVars = false, bool quietOpen = false)
        : LoadQueueEntry(url, method, loadingVars, quietOpen)
    { 
        PConstruct(NULL, -1);
    }
    GFxAS2LoadQueueEntry(CharacterHandle* pchar, const String &url, LoadMethod method, bool loadingVars = false, bool quietOpen = false) 
        : LoadQueueEntry(url, method, loadingVars, quietOpen)
    {
        PConstruct(pchar, -1);
    }   

    GFxAS2LoadQueueEntry(int level, const String &url, LoadMethod method, bool loadingVars = false, bool quietOpen = false)
        : LoadQueueEntry(url, method, loadingVars, quietOpen)
    {
        PConstruct(NULL, level);
        LoadTypeFlags typeFlag;
        if (loadingVars)
        {
            typeFlag = LTF_VarsFlag;
        }
        else
        {
            typeFlag = url ? LTF_None : LTF_UnloadFlag;
        }
        Type = (LoadType) (LTF_LevelFlag | typeFlag);
    }

    void CancelByNamePath(const ASString& namePath)
    {
        if (pCharacter && pCharacter->GetNamePath() == namePath)
            Canceled = true;
    }
    void CancelByLevel(int level)
    {
        if (Level != -1 && Level == level)
            Canceled = true;
    }

#ifdef GFX_ENABLE_XML
    void CancelByXMLASObjPtr(Object* pxmlobj)
    {
        if (!XMLHolder.ASObj.IsUndefined() && pxmlobj == XMLHolder.ASObj.ToObject(NULL))
            Canceled = true;
    }
#endif
#ifdef GFX_ENABLE_CSS
    void CancelByCSSASObjPtr(Object* pobj)
    {
        if (!CSSHolder.ASObj.IsUndefined() && pobj == CSSHolder.ASObj.ToObject(NULL))
            Canceled = true;
    }
#endif
};

//  ****** GFxLoadingMovieEntry
class GFxAS2LoadQueueEntryMT_LoadMovie : public LoadQueueEntryMT
{
    friend class GFx::MovieImpl;

    Ptr<MoviePreloadTask>   pPreloadTask;
    Ptr<Sprite>             pNewChar;
    bool                    FirstCheck;
    Ptr<InteractiveObject>  pOldChar;      
    ResourceId              NewCharId;
    bool                    CharSwitched;
    unsigned                BytesLoaded;
    bool                    FirstFrameLoaded;
public:
    GFxAS2LoadQueueEntryMT_LoadMovie(LoadQueueEntry* pqueueEntry, GFx::MovieImpl* pmovieRoot);
    ~GFxAS2LoadQueueEntryMT_LoadMovie();

    // Check if a movie is loaded. Returns false in the case if the movie is still being loaded.
    // Returns true if the movie is loaded completely or in the case of errors.
    bool LoadFinished();
    bool IsPreloadingFinished();

};

#ifdef GFX_ENABLE_XML

// ****** GFxLoadXMLTask
// Reads a file with loadXML data on a separate thread
class GFxAS2LoadXMLTask : public Task
{
public:
    GFxAS2LoadXMLTask(LoadStates* pls, const String& level0Path, const String& url, 
        GFxAS2LoadQueueEntry::XMLHolderType xmlholder);

    virtual void    Execute();

    bool IsDone() const;

private:
    Ptr<LoadStates>         pLoadStates;
    String                  Level0Path;
    String                  Url;
    Ptr<XMLFileLoader>      pXMLLoader;

    volatile unsigned       Done;
};

class GFxAS2LoadQueueEntryMT_LoadXML : public LoadQueueEntryMT
{
    Ptr<GFxAS2LoadXMLTask>  pTask;
    Ptr<LoadStates>         pLoadStates;
    MovieRoot*              pASMovieRoot;

public:
    GFxAS2LoadQueueEntryMT_LoadXML(LoadQueueEntry* pqueueEntry, MovieRoot* pmovieRoot);
    ~GFxAS2LoadQueueEntryMT_LoadXML();

    // Check if a movie is loaded. Returns false in the case if the movie is still being loaded.
    // Returns true if the movie is loaded completely or in the case of errors.
    bool LoadFinished();
};
#endif


#ifdef GFX_ENABLE_CSS
// ****** GFxLoadCSSTask
// Reads a file with loadXML data on a separate thread
class GFxAS2LoadCSSTask : public Task
{
public:
    GFxAS2LoadCSSTask(LoadStates* pls, const String& level0Path, const String& url, 
        GFxAS2LoadQueueEntry::CSSHolderType xmlholder);

    virtual void    Execute();

    bool IsDone() const;

private:
    Ptr<LoadStates>         pLoadStates;
    String                  Level0Path;
    String                  Url;
    Ptr<ASCSSFileLoader>    pLoader;

    volatile unsigned       Done;
};

class GFxAS2LoadQueueEntryMT_LoadCSS : public LoadQueueEntryMT
{
    Ptr<GFxAS2LoadCSSTask>  pTask;
    Ptr<LoadStates>         pLoadStates;

public:
    GFxAS2LoadQueueEntryMT_LoadCSS(LoadQueueEntry* pqueueEntry, MovieRoot* pmovieRoot);
    ~GFxAS2LoadQueueEntryMT_LoadCSS();

    // Check if a movie is loaded. Returns false in the case if the movie is still being loaded.
    // Returns true if the movie is loaded completely or in the case of errors.
    bool LoadFinished();
};
#endif


class MovieRoot : public ASMovieRootBase, public ActionPriority
{
public:
    Ptr<Render::TreeContainer> TopNode;

    Ptr<ASSupport>          pASSupport;

    // Memory heap context
    Ptr<MemoryContextImpl>  MemContext;

    // Keep track of the number of advances for shared GC
    unsigned                NumAdvancesSinceCollection;
    unsigned                LastCollectionFrame;

    const MouseListener* pASMouseListener; // a listener for AS Mouse class. Only one is necessary.

    // Global Action Script state
    Ptr<GlobalContext>   pGlobalContext;

    // Return value storage for ExternalInterface.call.
    Value                ExternalIntfRetVal;
    struct InvokeAliasInfo // aliases set by ExtIntf.addCallback
    {
        Ptr<Object>          ThisObject;
        Ptr<CharacterHandle> ThisChar;
        FunctionRef          Function;
    };
    ASStringHash<InvokeAliasInfo>* pInvokeAliases; // aliases set by ExtIntf.addCallback

    // *** Action Script execution

    // Action queue is stored as a singly linked list queue. List nodes must be traversed
    // in order for execution. New actions are inserted at the insert location, which is
    // commonly the end; however, in some cases insert location can be modified to allow
    // insertion of items before other items.


    // Action list to be executed 
    struct ActionEntry : public NewOverrideBase<StatMV_ActionScript_Mem>
    {
        enum EntryType
        {
            Entry_None,
            Entry_Buffer,   // Execute pActionBuffer(pSprite env)
            Entry_Event,        // 
            Entry_Function,
            Entry_CFunction
        };

        ActionEntry*        pNextEntry;
        EntryType           Type;
        Ptr<InteractiveObject> pCharacter;
        Ptr<ActionBuffer>   pActionBuffer;
        EventId             mEventId;
        FunctionRef         Function;
        CFunctionPtr        CFunction;
        ValueArray          FunctionParams;

        unsigned            SessionId;

        SF_INLINE ActionEntry();
        SF_INLINE ActionEntry(const ActionEntry &src);
        SF_INLINE const ActionEntry& operator = (const ActionEntry &src);

        // Helper constructors
        SF_INLINE ActionEntry(InteractiveObject *pcharacter, ActionBuffer* pbuffer);
        SF_INLINE ActionEntry(InteractiveObject *pcharacter, const EventId id);
        SF_INLINE ActionEntry(InteractiveObject *pcharacter, const FunctionRef& function, const ValueArray* params = 0);
        SF_INLINE ActionEntry(InteractiveObject *pcharacter, const CFunctionPtr function, const ValueArray* params = 0);

        SF_INLINE void SetAction(InteractiveObject *pcharacter, ActionBuffer* pbuffer);
        SF_INLINE void SetAction(InteractiveObject *pcharacter, const EventId id);
        SF_INLINE void SetAction(InteractiveObject *pcharacter, const FunctionRef& function, const ValueArray* params = 0);
        SF_INLINE void SetAction(InteractiveObject *pcharacter, const CFunctionPtr function, const ValueArray* params = 0);
        SF_INLINE void ClearAction();

        // Executes actions in this entry
        void    Execute(MovieRoot *proot) const;

        bool operator==(const ActionEntry&) const;
    };


    struct ActionQueueEntry
    {
        // This is a root of an action list. Root node action is 
        // always Entry_None and thus does not have to be executed.
        ActionEntry*        pActionRoot;

        // This is an action insert location. In a beginning, &ActionRoot, afterwards
        // points to the node after which new actions are added.
        ActionEntry*        pInsertEntry;

        // Pointer to a last entry
        ActionEntry*        pLastEntry;

        ActionQueueEntry() { Reset(); }
        inline void Reset() { pActionRoot = pLastEntry = pInsertEntry = NULL; }
    };
    struct ActionQueueType
    {
        ActionQueueEntry    Entries[AP_Count];
        // This is a modification id to track changes in queue during its execution
        int                 ModId;
        // This is a linked list of un-allocated entries.   
        ActionEntry*        pFreeEntry;
        unsigned            CurrentSessionId;
        unsigned            FreeEntriesCount;
        unsigned            LastSessionId;

        MemoryHeap*         pHeap;

        ActionQueueType(MemoryHeap* pheap);
        ~ActionQueueType();

        MemoryHeap*                GetMovieHeap() const { return pHeap; }

        ActionQueueEntry& operator[](unsigned i) { SF_ASSERT(i < AP_Count); return Entries[i]; }
        const ActionQueueEntry& operator[](unsigned i) const { SF_ASSERT(i < AP_Count); return Entries[i]; }

        ActionEntry*                InsertEntry(Priority prio);
        void                        AddToFreeList(ActionEntry*);
        void                        Clear();
        ActionEntry*                GetInsertEntry(Priority prio)
        {
            return Entries[prio].pInsertEntry;
        }
        ActionEntry*                SetInsertEntry(Priority prio, ActionEntry* pinsertEntry)
        {
            ActionEntry* pie = Entries[prio].pInsertEntry;
            Entries[prio].pInsertEntry = pinsertEntry;
            return pie;
        }
        ActionEntry*                FindEntry(Priority prio, const ActionEntry&);

        unsigned                    StartNewSession(unsigned* pprevSessionId);
        void                        RestoreSession(unsigned sId)  { CurrentSessionId = sId; }
    };
    struct ActionQueueIterator
    {
        int                 ModId;
        ActionQueueType*    pActionQueue;
        ActionEntry*        pLastEntry;
        int                 CurrentPrio;

        ActionQueueIterator(ActionQueueType*);
        ~ActionQueueIterator();

        const ActionEntry* getNext();
    };
    struct ActionQueueSessionIterator : public ActionQueueIterator
    {
        unsigned SessionId;

        ActionQueueSessionIterator(ActionQueueType*, unsigned sessionId);

        const ActionEntry* getNext();
    };
    ActionQueueType     ActionQueue;


    // AS2 implementation of StickyVarNode
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

    StringManager       BuiltinsMgr;
    ArrayLH<Ptr<Sprite>, StatMV_Other_Mem>  SpritesWithHitArea;
public:
    MovieRoot(MemoryContextImpl* memContext, GFx::MovieImpl* pmovie, ASSupport* pas);
    ~MovieRoot();

    SF_INLINE MovieDefImpl* GetMovieDefImpl() const { return pMovieImpl->GetMovieDefImpl(); }
    SF_INLINE void SetMovieDefImpl(MovieDefImpl* p) const { pMovieImpl->SetMovieDefImpl(p); }
    SF_INLINE Sprite* GetLevel0Movie() const { return static_cast<Sprite*>(pMovieImpl->pMainMovie); }

    SF_INLINE MemoryHeap*         GetMovieHeap() const { return pMovieImpl->GetMovieHeap(); }
    SF_INLINE AS2::StringManager* GetStringManager() { return &BuiltinsMgr; }
    SF_INLINE const StringManager& GetBuiltinsMgr() const { return BuiltinsMgr; }
    SF_INLINE Log*                GetLog() const { return pMovieImpl->GetLog(); }
    SF_INLINE LogState*           GetLogState() const { return pMovieImpl->GetLogState(); }

    ASRefCountCollector*  GetASGC()
    {
#ifdef GFX_AS_ENABLE_GC
        return MemContext->ASGC;
#else
        return NULL;
#endif
    }

    // Creates an ObjectInterface class inside of MovieImpl, used in constructor.
    void    CreateObjectInterface(GFx::MovieImpl* movie);


    Sprite* CreateSprite(TimelineDef* pdef, 
                         MovieDefImpl* pdefImpl,
                         InteractiveObject* parent,
                         ResourceId id, 
                         bool loadedSeparately = false);

    // Sets a movie at level; used for initialization.
    bool                SetLevelMovie(int level, Sprite *psprite);
    // Returns a movie at level, or null if no such movie exists.
    Sprite*             GetLevelMovie(int level) const;
    SF_INLINE AvmSprite*  GetAvmLevelMovie(int level) const 
    {
        return ToAvmSprite(GetLevelMovie(level));
    }

    // Helper: parses _levelN tag and returns the end of it.
    // Returns level index, of -1 if there is no match.
    static int          ParseLevelName(const char* pname, const char **ptail, bool caseSensitive);

    InvokeAliasInfo*    ResolveInvokeAlias(const char* pstr) const;
    bool                InvokeAlias(const char* pmethodName, const InvokeAliasInfo& alias, 
                                    Value *presult, unsigned numArgs);
    bool                InvokeAliasArgs(const char* pmethodName, const InvokeAliasInfo& alias, 
                                        Value *presult, const char* methodArgFmt, va_list args);

    // Inserts an empty action and returns a pointer to it. Should call SetAction afterwards.   
    ActionEntry*        InsertEmptyAction(Priority prio = AP_Frame) { return ActionQueue.InsertEntry(prio); }
    bool                HasActionEntry(const ActionEntry& entry,Priority prio = AP_Frame)       
    { return ActionQueue.FindEntry(prio, entry) != NULL; }

    bool                GetLevel0Path(String *ppath) const;

    // Adds load queue entry based on parsed url and target path.
    SF_INLINE void      AddLoadQueueEntry(LoadQueueEntry *pentry)
    {
        pMovieImpl->AddLoadQueueEntry(pentry);
    }
    void                AddLoadQueueEntryMT(LoadQueueEntry *pqueueEntry);
    void                AddMovieLoadQueueEntry(LoadQueueEntry* pentry);
    void                AddLoadQueueEntry(const char* ptarget, const char* purl, Environment* env,
                                          LoadQueueEntry::LoadMethod method = LoadQueueEntry::LM_None,
                                          MovieClipLoader* pmovieClipLoader = NULL);
    void                AddLoadQueueEntry(InteractiveObject* ptarget, const char* purl,
                                          LoadQueueEntry::LoadMethod method = LoadQueueEntry::LM_None,
                                          MovieClipLoader* pmovieClipLoader = NULL);
    // Load queue entries for loading variables
    void                AddVarLoadQueueEntry(const char* ptarget, const char* purl,
                                             LoadQueueEntry::LoadMethod method = LoadQueueEntry::LM_None);
    void                AddVarLoadQueueEntry(InteractiveObject* ptarget, const char* purl,
                                             LoadQueueEntry::LoadMethod method = LoadQueueEntry::LM_None);
    // Process load queue entry (single threaded)
    virtual void        ProcessLoadQueueEntry(LoadQueueEntry *pentry, LoadStates* pls);

    // multi-threaded logic for loading vars
    void                ProcessLoadVarsMT(LoadQueueEntry *pentry, LoadStates* pls, 
        const String& data, UPInt fileLen, bool succeeded);

#ifdef GFX_AS2_ENABLE_LOADVARS
    // a common logic for MT and ST loading vars.
    void                DoProcessLoadVars(LoadQueueEntry *pentry, LoadStates* pls, 
                                          const String& data, UPInt fileLen, bool succeeded);
    void                AddVarLoadQueueEntry(LoadVarsObject* ploadVars, const char* purl,
                                             LoadQueueEntry::LoadMethod method = LoadQueueEntry::LM_None);
    // single threaded logic for loading vars
    void                ProcessLoadVars(LoadQueueEntry *pentry, LoadStates* pls);
    // called from LoadVars should be private
    Sprite*             CreateEmptySprite(LoadStates* pls, int level);
#endif //#ifdef GFX_AS2_ENABLE_LOADVARS

#ifdef GFX_ENABLE_XML
    void                AddXmlLoadQueueEntry(Object* pxmlobj, XMLFileLoader* pxmlLoader, 
                                             const char* purl,
                                             LoadQueueEntry::LoadMethod method = LoadQueueEntry::LM_None);
#endif

#ifdef GFX_ENABLE_CSS
    void                AddCssLoadQueueEntry(Object* pobj, ASCSSFileLoader* pLoader, const char* purl,
                                             LoadQueueEntry::LoadMethod method = LoadQueueEntry::LM_None);
#endif

#ifdef GFX_ENABLE_XML
    void                ProcessLoadXML(LoadQueueEntry *pentry, LoadStates* pls);
#endif

#ifdef GFX_ENABLE_CSS
    void                ProcessLoadCSS(LoadQueueEntry *pentry, LoadStates* pls);
#endif

    void                ProcessLoadMovieClip(LoadQueueEntry *pentry, LoadStates* pls);

    // Implementation used by SetVariable/GetVariable.
    virtual void        CreateString(GFx::Value* pvalue, const char* pstring);
    virtual void        CreateStringW(GFx::Value* pvalue, const wchar_t* pstring);
    virtual void        CreateObject(GFx::Value* pvalue, const char* className = NULL, 
                                     const GFx::Value* pargs = NULL, unsigned nargs = 0);
    virtual void        CreateArray(GFx::Value* pvalue);
    virtual void        CreateFunction(GFx::Value* pvalue, GFx::FunctionHandler* pfc, 
                                     void* puserData = NULL);

    void                Value2ASValue(const GFx::Value& gfxVal, Value* pdestVal);
    void                ASValue2Value(Environment* penv, const Value& value, GFx::Value* pdestVal) const;    
    bool                CreateObjectValue(GFx::Value* pvalue, GFx::Value::ObjectInterface* pobjifc, 
                                     void* pdata, bool isdobj);

    // Take care of this frame's actions, by executing ActionList. 
    void                DoActions();
    // Execute only actions from the certain session; all other actions are staying in the queue
    void                DoActionsForSession(unsigned sessionId); 

    void                AddStickyVariable(const ASString& fullPath, const Value &val, Movie::SetVarType setType);
    void                ResolveStickyVariables(InteractiveObject *pcharacter);

    // implementation of ASMovieRootBase methods
    // Check for AVM; create it, if it is not created yet.
    virtual bool        CheckAvm() { return true; }
    virtual void        SetMovie(GFx::MovieImpl* pmovie);
    virtual bool        Init(MovieDefImpl* pmovieDef);
    virtual void        ClearDisplayList();
    virtual MovieDefRootNode* CreateMovieDefRootNode(MemoryHeap* pheap, MovieDefImpl *pdefImpl, bool importFlag = false);
    // Register AS classes defined in aux libraries
    virtual void        RegisterAuxASClasses();

    // This method returns the root display object container for the specified object. 
    // The "dobj" parameter might be NULL; in this case, for AS2 this method will return _root 
    // (same as GetMainContainer), for AS3 it will return "root".
    // If "dobj" is not NULL and it belongs to SWF loaded by loadMovie then the result is the
    // parent movie on which "loadMovie" was called. (@TODO)
    DisplayObjContainer*    GetRootMovie(DisplayObject* dobj = NULL) const 
    { 
        SF_UNUSED(dobj); //@TODO: look for correct root
        if (pMovieImpl->GetMainContainer())
        {
            SF_ASSERT(pMovieImpl->GetMainContainer()->IsDisplayObjContainer());
            return pMovieImpl->GetMainContainer()->CharToDisplayObjContainer_Unsafe(); 
        }
        return NULL;
    }

    virtual void        AdvanceFrame(bool nextFrame);

    // forces garbage collection (if GC is enabled)
    virtual void        ForceCollect(unsigned);
    // forces emergency garbage collection (if GC is enabled). This method is called
    // when heap is overflown. 
    virtual void        ForceEmergencyCollect();

    virtual void        SuspendGC(bool) {}
    virtual void        ScheduleGC(unsigned) {}

    // Generate button events (onRollOver, onPress, etc)
    virtual void        GenerateMouseEvents(unsigned mouseIndex);

    virtual void        SetExternalInterfaceRetVal(const GFx::Value&);
    virtual void        SetMemoryParams(unsigned frameBetweenCollections, unsigned maxRootCount);
    virtual bool        SetVariable(const char* ppathToVar, const GFx::Value& value,
                                    Movie::SetVarType setType = Movie::SV_Sticky);
    virtual bool        GetVariable(GFx::Value *pval, const char* ppathToVar) const;
    virtual bool        SetVariableArray(Movie::SetArrayType type, const char* ppathToVar,
                                         unsigned index, const void* pdata, unsigned count,
                                         Movie::SetVarType setType = Movie::SV_Sticky);
    virtual bool        SetVariableArraySize(const char* ppathToVar, unsigned count,
                                             Movie::SetVarType setType = Movie::SV_Sticky);
    virtual unsigned    GetVariableArraySize(const char* ppathToVar);
    virtual bool        GetVariableArray(Movie::SetArrayType type, const char* ppathToVar,
                                         unsigned index, void* pdata, unsigned count);

    virtual bool        IsAvailable(const char* ppathToVar) const;

    virtual bool        Invoke(const char* pmethodName, GFx::Value *presult, const GFx::Value* pargs, unsigned numArgs);
    virtual bool        Invoke(const char* pmethodName, GFx::Value *presult, const char* pargFmt, ...);
    virtual bool        InvokeArgs(const char* pmethodName, GFx::Value *presult, const char* pargFmt, va_list args);

    virtual bool        Invoke(InteractiveObject* thisSpr, const char* pmethodName, GFx::Value *presult, const GFx::Value* pargs, unsigned numArgs);
    virtual InteractiveObject* FindTarget(const ASString& path) const;
    virtual void        NotifyOnResize();
    virtual void        NotifyMouseEvent(const InputEventsQueue::QueueEntry* qe, const MouseState& ms, int mi);
    virtual void        ChangeMouseCursorType(unsigned mouseIdx, unsigned newCursorType);
    virtual void        Shutdown();
    virtual void        NotifyQueueSetFocus(InteractiveObject* ch, unsigned controllerIdx, FocusMovedType fmt);
    virtual void        NotifyTransferFocus(InteractiveObject* curFocused, InteractiveObject* pNewFocus, 
        unsigned controllerIdx);
    virtual bool        NotifyOnFocusChange(InteractiveObject* curFocused, InteractiveObject* newFocus, 
                                            unsigned, FocusMovedType fmt, ProcessFocusKeyInfo*);

    // invoked when whole movie view gets or loses focus
    virtual void        OnMovieFocus(bool) {}
    // called BEFORE processing complete (non-partial) AdvanceFrame.
    virtual void        OnNextFrame() {}

    void                AddInvokeAlias(const ASString& alias, CharacterHandle* pthisChar,
                                       Object* pthisObj, const FunctionRef& func);

    virtual MemoryContext* GetMemoryContext() const
    {
        return MemContext;
    }

    virtual Text::Allocator* GetTextAllocator();

    // Accessor to string manager
    virtual ASStringManager* GetStringManager() const { return MemContext->StringMgr; }
};

// ** Inline Implementation


SF_INLINE MovieRoot::ActionEntry::ActionEntry()
{ 
    pNextEntry = 0;
    Type = Entry_None; pActionBuffer = 0;
    SessionId = 0;
}

SF_INLINE MovieRoot::ActionEntry::ActionEntry(const MovieRoot::ActionEntry &src)
{
    pNextEntry      = src.pNextEntry;
    Type            = src.Type;
    pCharacter      = src.pCharacter;
    pActionBuffer   = src.pActionBuffer;
    mEventId         = src.mEventId;
    Function        = src.Function;
    CFunction       = src.CFunction;
    FunctionParams  = src.FunctionParams;
    SessionId       = src.SessionId;
}

SF_INLINE const MovieRoot::ActionEntry& 
MovieRoot::ActionEntry::operator = (const MovieRoot::ActionEntry &src)
{           
    pNextEntry      = src.pNextEntry;
    Type            = src.Type;
    pCharacter      = src.pCharacter;
    pActionBuffer   = src.pActionBuffer;
    mEventId         = src.mEventId;
    Function        = src.Function;
    CFunction       = src.CFunction;
    FunctionParams  = src.FunctionParams;
    SessionId       = src.SessionId;
    return *this;
}

SF_INLINE MovieRoot::ActionEntry::ActionEntry(InteractiveObject *pcharacter, ActionBuffer* pbuffer)
{
    pNextEntry      = 0;
    Type            = Entry_Buffer;
    pCharacter      = pcharacter;
    pActionBuffer   = pbuffer;
    SessionId       = 0;
}

SF_INLINE MovieRoot::ActionEntry::ActionEntry(InteractiveObject *pcharacter, const EventId id)
{
    pNextEntry      = 0;
    Type            = Entry_Event;
    pCharacter      = pcharacter;
    pActionBuffer   = 0;
    mEventId         = id;
    SessionId       = 0;
}

SF_INLINE MovieRoot::ActionEntry::ActionEntry(InteractiveObject *pcharacter, const FunctionRef& function, const ValueArray* params)
{
    pNextEntry      = 0;
    Type            = Entry_Function;
    pCharacter      = pcharacter;
    pActionBuffer   = 0;
    Function        = function;
    if (params)
        FunctionParams = *params;
    SessionId       = 0;
}

SF_INLINE MovieRoot::ActionEntry::ActionEntry(InteractiveObject *pcharacter, const CFunctionPtr function, const ValueArray* params)
{
    pNextEntry      = 0;
    Type            = Entry_CFunction;
    pCharacter      = pcharacter;
    pActionBuffer   = 0;
    CFunction        = function;
    if (params)
        FunctionParams = *params;
    SessionId       = 0;
}

SF_INLINE void MovieRoot::ActionEntry::SetAction(InteractiveObject *pcharacter, ActionBuffer* pbuffer)
{
    Type            = Entry_Buffer;
    pCharacter      = pcharacter;
    pActionBuffer   = pbuffer;
    mEventId.Id      = EventId::Event_Invalid;
}

SF_INLINE void MovieRoot::ActionEntry::SetAction(InteractiveObject *pcharacter, const EventId id)
{
    Type            = Entry_Event;
    pCharacter      = pcharacter;
    pActionBuffer   = 0;
    mEventId         = id;
}

SF_INLINE void MovieRoot::ActionEntry::SetAction(InteractiveObject *pcharacter, const FunctionRef& function, const ValueArray* params)
{
    Type            = Entry_Function;
    pCharacter      = pcharacter;
    pActionBuffer   = 0;
    Function        = function;
    if (params)
        FunctionParams = *params;
}

SF_INLINE void MovieRoot::ActionEntry::SetAction(InteractiveObject *pcharacter, const CFunctionPtr function, const ValueArray* params)
{
    Type            = Entry_CFunction;
    pCharacter      = pcharacter;
    pActionBuffer   = 0;
    CFunction       = function;
    if (params)
        FunctionParams = *params;
}

SF_INLINE void MovieRoot::ActionEntry::ClearAction()
{
    Type            = Entry_None;
    pActionBuffer   = 0;
    pCharacter      = 0;
    Function.DropRefs();
    FunctionParams.Resize(0);
}

SF_INLINE bool MovieRoot::ActionEntry::operator==(const MovieRoot::ActionEntry& e) const
{
    return Type == e.Type && pActionBuffer == e.pActionBuffer && pCharacter == e.pCharacter &&
        CFunction == e.CFunction && Function == e.Function && mEventId == e.mEventId;
}

SF_INLINE MovieRoot* ToAS2Root(Sprite* spr)
{
    SF_ASSERT(spr && spr->GetASMovieRoot() && spr->GetAVMVersion() == 1);
    return static_cast<MovieRoot*>(spr->GetASMovieRoot());
}

SF_INLINE MovieRoot* ToAS2Root(GFx::MovieImpl* pm)
{
    SF_ASSERT(pm->pASMovieRoot && pm->pASMovieRoot->GetAVMVersion() == 1);
    return static_cast<MovieRoot*>(pm->pASMovieRoot.GetPtr());
}

ASString   EventId_GetFunctionName(StringManager *psm, const EventId& evt);

}}} //SF::GFx::AS2

#endif // INC_SF_GFX_AS2_MOVIEROOT_H

