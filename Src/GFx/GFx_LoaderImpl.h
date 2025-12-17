/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_LoaderImpl.h
Content     :   SWF loading interface implementation for GFxPlayer
Created     :   
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_LOADERIMPL_H
#define INC_SF_GFX_LOADERIMPL_H

#include "Kernel/SF_Array.h"
#include "Kernel/SF_List.h"
#include "Kernel/SF_RefCount.h"
#include "GFx/GFx_Loader.h"

//#include "GFxCharacter.h"
#include "GFx/GFx_ResourceHandle.h"

#include "GFx/GFx_Log.h"
#include "GFx/GFx_Stream.h"
#include "GFx/GFx_String.h"

#include "GFx/GFx_TagLoaders.h"

#include "GFx/GFx_TaskManager.h"
#include "Render/ImageFiles/Render_ImageFile.h"

namespace Scaleform { 

namespace R30 = Render;

namespace GFx {

// ***** Declared Classes
class LoaderImpl;
class StateBagImpl;

class ExporterInfoImpl;

class ImageResource;

class MovieDefImpl;

class MovieBindProcess;


// ***** Helper classes for extractor tool data

class ExporterInfoImpl
{
    ExporterInfo SI;    
    // String buffers that actually hold data SI elements point to.
    String Prefix;
    String SWFName;

public:
    Array<UInt32> CodeOffsets;

    ExporterInfoImpl()
    {
        SI.Format       = Loader::File_Unopened;
        SI.pSWFName     = 0;
        SI.pPrefix      = 0;
        SI.ExportFlags  = 0;
        SI.Version      = 0;

    }
    ExporterInfoImpl(const ExporterInfoImpl &src)
    { SetData(src.SI.Version, src.SI.Format, src.SI.pSWFName, src.SI.pPrefix, src.SI.ExportFlags); }
    const ExporterInfoImpl& operator = (const ExporterInfoImpl &src)
    { SetData(src.SI.Version, src.SI.Format, src.SI.pSWFName, src.SI.pPrefix, src.SI.ExportFlags); return *this; }

    // Assigns data
    void    SetData(UInt16 version, Loader::FileFormatType format,
        const char* pname, const char* pprefix, unsigned flags = 0, const Array<UInt32>* codeOffsets = NULL);

    const ExporterInfo* GetExporterInfo() const
    { return (SI.Format == Loader::File_Unopened) ? 0 : &SI; }


    // Read in the data from tag Tag_ExporterInfo. It is assumed the tag has been opened.    
    void    ReadExporterInfoTag(Stream *pstream, TagType tagType);
};



struct MovieHeaderData
{
    UInt32              FileLength;
    int                 Version;
    RectF               FrameRect;
    float               FPS;
    unsigned            FrameCount;
    unsigned            SWFFlags;
    // Exporter info, read through tag 'Tag_ExporterInfo' during header loading.
    ExporterInfoImpl    mExporterInfo;

    // We need default constructor because it is used in MovieDataDef
    // for image files and non-SWF movie defs.

    MovieHeaderData()
        : FileLength(0), Version(-1), FPS(1.0f), SWFFlags(0)
    {
        // Note that it is particularly important for FrameCount to start at 1,
        // since image def loading and empty def creation relies on that.
        FrameCount = 1;
    }

    // Applies our content to GFxMovieInfo.
    void GetMovieInfo(MovieInfo *pinfo) const
    {        
        // Store header data.
        pinfo->Version          = Version;
        pinfo->Flags            = SWFFlags;
        pinfo->FPS              = FPS;
        pinfo->FrameCount       = FrameCount;
        pinfo->Width            = int(FrameRect.Width() / 20.0f + 0.5f);
        pinfo->Height           = int(FrameRect.Height() / 20.0f + 0.5f);

        const ExporterInfo* pexi = mExporterInfo.GetExporterInfo();
        if (pexi)
        {
            pinfo->ExporterVersion  = pexi->Version;
            pinfo->ExporterFlags    = pexi->ExportFlags;
        }
        else
        {
            pinfo->ExporterVersion  = 0;
            pinfo->ExporterFlags    = 0;
        }       
    }
};


// Structure used to load SWF/GFX file header and export tags. 
// Normally created inside of GFxLoadProcess, but can also exist stand-alone.

struct SWFProcessInfo
{
    GFx::Stream      Stream;
    UInt32           FileStartPos;
    UInt32           FileEndPos;
    UInt32           NextActionBlock;
    MovieHeaderData  Header;   
    UInt16           FileAttributes; // bitmask, refer to MovieDef::FileAttr_*

    SWFProcessInfo(MemoryHeap *pheap) : Stream(0, pheap, 0, 0), FileAttributes(0) { }

    // Processes and reads in a SWF file header and opens the Stream.
    // 'parseMsg' flag specifies whether parse log messages are to be generated.
    // If 0 is returned, there was an error and error message is already displayed
    bool    Initialize(File *pfile, LogState *plog, ZlibSupportBase* zlib,
                       ParseControl* pparseControl, bool parseMsg = 0);

    void    ShutDown() { Stream.ShutDown(); }
};




// ***** Loader Implementation class


// Shared state - allocated in loader, movieDef, and movie root instance.
class StateBagImpl : public RefCountBase<StateBagImpl, Stat_Default_Mem>,
    public StateBag,                          
    public LogBase<StateBagImpl>
{
protected:

    // Pointer to the delegate implementation which we fall
    // back onto if our fields do not provide a certain value.
    Ptr<StateBagImpl> pDelegate;


    struct StatePtr
    {
        Ptr<State> pState;

        StatePtr() { }
        StatePtr(State* pstate)
            : pState(pstate) { }
        StatePtr(const Ptr<State> &pstate)
            : pState(pstate) { }
        StatePtr(const StatePtr& other)
            : pState(other.pState) { }

        bool operator == (State::StateType stype) const
        { return pState->GetStateType() == stype; }
        bool operator != (State::StateType stype) const
        { return !operator == (stype); }

        bool operator == (const StatePtr& other) const
        { return pState->GetStateType() == other.pState->GetStateType(); }
        bool operator != (const StatePtr& other) const
        { return !operator == (other); }
    };


    // Hash function used for nodes.
    struct StatePtrHashOp
    {                
        // Hash code is computed based on a state key.
        UPInt  operator() (const StatePtr& pstate) const
        {
            SF_ASSERT(pstate.pState.GetPtr() != 0);
            return (UPInt) pstate.pState->GetStateType();
        }
        UPInt  operator() (const Ptr<State>& pstate) const
        {
            SF_ASSERT(pstate.GetPtr() != 0);
            return (UPInt) pstate->GetStateType();
        }
        UPInt  operator() (const State* pstate) const
        {
            SF_ASSERT(pstate != 0);
            return (UPInt) pstate->GetStateType();
        }
        UPInt  operator() (State::StateType stype) const
        {
            return (UPInt) stype;
        }
    };


    // State hash
    typedef HashSet<StatePtr, StatePtrHashOp> StateHash;

    StateHash       States;
    // Lock to keep state updates thread-safe.
    mutable Lock   StateLock;

public:

    StateBagImpl(StateBagImpl *pdelegate)   
    {     
        pDelegate = pdelegate;
    }
    ~StateBagImpl()
    {
    }

    // Change delegate
    void    SetDelegate(StateBagImpl* pdelegate)
    {
        pDelegate = pdelegate;
    }
    StateBagImpl* GetDelegate() const
    {
        return pDelegate;
    }

    // Copy states, as necessary when creating a new loader from an old one.    
    void    CopyStatesFrom(StateBagImpl *pother)
    {  
        Lock::Locker lock(&StateLock);
        Lock::Locker lockOther(&pother->StateLock);
        States = pother->States;
    }

    // *** GFxStateBag member implementation.

    // General state access.
    virtual void        SetState(State::StateType state, State* pstate)
    {
        Lock::Locker lock(&StateLock);
        if (pstate)
            States.Set(StatePtr(pstate));
        else
            States.Remove(state);
    }

    virtual State*   GetStateAddRef(State::StateType state) const
    {
        const StatePtr* pstate;       
        {   // Lock scope for 'get'.
            Lock::Locker lock(&StateLock);
            pstate = States.Get(state);

            if (pstate)
            {
                pstate->pState->AddRef();
                return pstate->pState;
            }            
        }

        if (pDelegate) 
            return pDelegate->GetStateAddRef(state);
        return 0;
    }

    // Fills in a set of states with one call.
    virtual void        GetStatesAddRef(State** pstateList,
                                        const State::StateType *pstates, unsigned count) const
    {
        const StatePtr* pstate;
        // Not Found flag is set to call a delegate if at least one
        // of the requested states isn't yet filled in.
        bool            notFound = 0;        

        // Lock scope for 'get'.
        {
            Lock::Locker lock(&StateLock);

            for (unsigned i=0; i<count; i++)
            {
                if (pstateList[i] == 0)
                {
                    pstate = States.Get(pstates[i]);
                    if (pstate)
                    {
                        pstate->pState->AddRef();
                        pstateList[i] = pstate->pState;
                    }
                    else
                    {
                        notFound = true;
                    }
                }
            }
        }

        if (notFound && pDelegate) 
            pDelegate->GetStatesAddRef(pstateList, pstates, count);
    }

};



// ***** LoaderTask
// All loader tasks should be inherited from this class. It unregister a task from 
// the loader on the task destructor.
class LoaderImpl;
class LoaderTask : public Task
{
protected:

    // Load states used during load processing.
    Ptr<LoadStates>         pLoadStates;

public:
    LoaderTask(LoadStates* pls, TaskId id = Id_Unknown);
    ~LoaderTask();
};

// LoaderImpl keeps a list of currently loading task. We need this list so we
// can cancel all loading tasks.
struct LoadProcessNode : public ListNode<LoadProcessNode>, public NewOverrideBase<Stat_Default_Mem>
{
    LoadProcessNode(LoaderTask* ptask) : pTask(ptask) { SF_ASSERT(pTask); }
    LoaderTask* pTask;
};

// ***** LoaderImpl - loader

class LoaderImpl : public RefCountBase<LoaderImpl, Stat_Default_Mem>, public StateBag, public LogBase<LoaderImpl>
{
public:
    // This structure is used to detect movies recursive loading.
    // Its instances are created on stack in MovieBindProcess::BindNextFrame() method
    // and recursive loading is detected in LoaderImpl::BindMovieAndWait method
    struct LoadStackItem
    {
        LoadStackItem(MovieDefImpl* pdefImpl) : pDefImpl(pdefImpl), pNext(NULL) {}
        MovieDefImpl*   pDefImpl;
        LoadStackItem*  pNext;
    };

private:
    // Shared loader state overridable in movies.
    Ptr<StateBagImpl>       pStateBag;

    // Weak library of resources created so far. This needs to be a weak library 
    // because GFxMovieImpl instances keep strong pointers to LoaderImpl.
    // A strong version of this library exists in LoaderImpl.
    Ptr<ResourceWeakLib>    pWeakResourceLib;

    // List of currently loading tasks.
    List<LoadProcessNode>   LoadProcesses;
    Lock                    LoadProcessesLock;

    // Set if this loader is use for debug/tool purposes (usually means
    // that created state heaps have Heap_UserDebug flag).
    bool                    DebugHeap;

    // Cache of files received by HTTP requests
    // Often a file is read more than once, to get the header information
    class HttpFileCache : public RefCountBase<HttpFileCache, Stat_Default_Mem>
    {
    public:
        Array<UByte> HttpFileBytes;
    };
    StringHashLH<Ptr<HttpFileCache> > HttpFilesOpened;

    // Implement GFxStateBag through delegation.
    virtual StateBag* GetStateBagImpl() const { return pStateBag; }


    // Helper used from CreateMovie_LoadState; looks up or registers MovieDefImpl.
    static MovieDefImpl* CreateMovieDefImpl(LoadStates* pls,
        MovieDataDef* pmd, unsigned loadConstants,
        MovieBindProcess** ppbindProcess, bool checkCreate,
        LoadStackItem* ploadStack, UPInt memoryArena = 0);

public:

    // Creates a loader with new states.
    LoaderImpl(ResourceLib *pownerLib = 0, bool debugHeap = false);
    // Creates a loader impl, copying state pointers from another loader.
    LoaderImpl(LoaderImpl* psource);
    // Creates a loader impl, using EXISTING states bag (it DOES NOT copy them!)
    LoaderImpl(StateBag* pstates, ResourceLib* plib, bool debugHeap = false);

    virtual ~LoaderImpl();    

    void            SetWeakResourceLib(ResourceWeakLib* pweakLib) { pWeakResourceLib = pweakLib; }


    // *** Movie Loading

    bool GetMovieInfo(const char *pfilename, MovieInfo *pinfo, bool getTagCount, unsigned loadLibConstants);
    bool IsMovieLoaded(const char *pfilename, unsigned loadConstants);
    MovieDef* CreateMovie(const char* filename, unsigned loadConstants, UPInt memoryArena = 0);    
#ifdef SF_ENABLE_HTTP_LOADING
    Array<UByte>* ReadProtocolFile(const String& filename);
    void LoadingDone(const char* filename);
#endif
    // The actual creation function; called from CreateMovie.
    // This function uses an externally captured specified load state.   
    static MovieDefImpl*  CreateMovie_LoadState(LoadStates* pls,
        const URLBuilder::LocationInfo& loc,
        unsigned loadConstants, LoadStackItem* ploadStack = NULL, UPInt memoryArena = 0);

    // Loading version used for look up / bind MovieDataDef based on provided states.
    // Used to look up movies serving fonts from GFxFontProviderSWF.
    static MovieDefImpl*  CreateMovie_LoadState(LoadStates* pls,
        MovieDataDef* pmd,
        unsigned loadConstants, UPInt memoryArena = 0);

    // Loading version used to load from byte array.
    // This function uses an externally captured specified load state.   
    static MovieDefImpl*  CreateMovie_LoadState(LoadStates* pls,
        const ArrayPOD<UByte>& bytes,
        unsigned loadConstants, LoadStackItem* ploadStack = NULL, UPInt memoryArena = 0);

    // Binds a MovieDef and waits for completion, based on flags. 
    // If wait failed, releases the movieDef returning 0.
    static MovieDefImpl* BindMovieAndWait(MovieDefImpl* pm, MovieBindProcess* pbp,
        LoadStates* pls, unsigned loadConstants, 
        LoadStackItem* ploadStack = NULL);

    typedef Loader::FileFormatType FileFormatType;

    static FileFormatType   DetectFileFormat(File *pfile);

    // LogBase support
    /*
    bool    IsVerboseParse() const              { return (VerboseOptions & VerboseParse) != 0; }
    bool    IsVerboseParseShape() const         { return (VerboseOptions & VerboseParseShape) != 0; }
    bool    IsVerboseParseMorphShape() const    { return (VerboseOptions & VerboseParseMorphShape) != 0; }
    bool    IsVerboseParseAction() const        { return (VerboseOptions & VerboseParseAction) != 0; }
    */


    // *** Tag Processing support

    typedef TagLoaderFunctionType LoaderFunction;

    // Looks up a tag loader.
    static bool    GetTagLoader(unsigned tagType, LoaderFunction *plf)
    { 
        if (tagType < Tag_SWF_TagTableEnd)
            *plf = SWF_TagLoaderTable[tagType];
        else if ((tagType >= Tag_GFX_TagTableBegin) && (tagType < Tag_GFX_TagTableEnd))
            *plf = GFx_GFX_TagLoaderTable[tagType - Tag_GFX_TagTableBegin];                    
        else
            *plf = 0;
        return (*plf) ? 1 : 0;
    }

    bool    CheckTagLoader(unsigned tagType) const 
    {
        LoaderFunction lf;
        return GetTagLoader(tagType, &lf);
    }


    ResourceWeakLib* GetWeakLib() const  { return pWeakResourceLib; }

    // Create a filler image that will be displayed in place of loadMovie() user images.
// @IMG    static Render::ImageInfoBase*     CreateStaticUserImage();

    // Register a task in the tasks loading list
    void RegisterLoadProcess(LoaderTask* ptask);
    // Unregister a task from the tasks loading list
    void UnRegisterLoadProcess(LoaderTask* ptask);
    // Cancel all currently loading tasks
    void CancelLoading();

    static Render::ImageSource* LoadBuiltinImage(File* pfile,
        FileTypeConstants::FileFormatType format,
        Resource::ResourceUse use,
        LoadStates* pls,
        Log* plog,
        MemoryHeap* pimageHeap);

    static Render::ImageFileFormat 
        FileFormat2RenderImageFile(FileTypeConstants::FileFormatType format);

    // Returns true if url specifies an image to be used with image protocol
    // (has one of the prefixes: img://, imgps://, simg://, simgps://)
    // bilinear is set to false if imgps or simgps is used;
    // sync is set to true if simg or simgps is used.
    static bool IsProtocolImage(const String& url, bool *bilinear = NULL, bool* sync = NULL);

private:
    LoaderImpl& operator =(const LoaderImpl&);
    LoaderImpl(const LoaderImpl&);

    static Lock HttpLock;

};

}} // Scaleform::GFx

#endif // INC_SF_GFX_IMPL_H
