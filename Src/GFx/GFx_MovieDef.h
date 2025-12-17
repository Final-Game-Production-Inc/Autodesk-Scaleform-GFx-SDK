/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_MovieDef.h
Content     :   MovieDataDef and MovieDefImpl classes used to
                represent loaded and bound movie data, respectively.
Created     :   
Authors     :   Michael Antonov

Notes       :   Starting with GFx 2.0, loaded and bound data is stored
                separately in MovieDataDef and MovieDefImpl
                classes. MovieDataDef represents data loaded from
                a file, shared by all bindings. MovieDefImpl binds
                resources referenced by the loaded data, by providing
                a custom ResourceBinding table. Bindings can vary
                depending on State objects specified during loading.

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_MOVIEDEF_H
#define INC_SF_GFX_MOVIEDEF_H

#include "Kernel/SF_Array.h"
#include "Kernel/SF_Threads.h"
#include "Kernel/SF_AutoPtr.h"

#include "GFx/GFx_CharacterDef.h"
#include "GFx/GFx_ResourceHandle.h"
#include "GFx/GFx_LoaderImpl.h"
#include "GFx/GFx_FontResource.h"
#include "GFx/GFx_FontLib.h"
#include "GFx/GFx_StringHash.h"
#include "GFx/GFx_CharPosInfo.h"
#include "GFx/GFx_FilterDesc.h"
#include "GFx/GFx_VideoBase.h"
#include "GFx/GFx_Audio.h"
#include "GFx/GFx_ImageCreator.h"

// For GFxTask base.
#include "GFx/GFx_TaskManager.h"

#include "Kernel/SF_HeapNew.h"


#ifdef SF_BUILD_DEBUG
//#define SF_DEBUG_COUNT_TAGS

//    #define GFX_TRACE_TIMELINE_EXECUTION
//    #define GFX_DUMP_DISPLAYLIST
//    #define GFX_TRACE_DIPLAYLIST_EVERY_FRAME

#endif

namespace Scaleform { 

namespace Render
{
    class ShapeMeshProvider;
}

namespace GFx {

// ***** Declared Classes

class MovieDataDef;
class MovieDefImpl;
//class GFxSprite;
// Helpers
class ImportInfo;
class SwfEvent;

// Tag classes
class PlaceObject2Tag;
class RemoveObject2Tag;
class SetBackgroundColorTag;
// class GASDoAction;           - in GFxAction.cpp
// class GFxStartSoundTag;      - in GFxSound.cpp


class MovieBindProcess;

// ***** External Classes

class LoadStates;

class FontPackParams;
class ImagePackParamsBase;
class ImagePacker;

class MovieImpl;
class LoadQueueEntry;

class SoundStreamDef;

#ifdef GFX_ENABLE_SOUND
class SoundStreamDef;
#endif

struct TempBindData;

class DisplayObjContainer;
class TimelineSnapshot;
class SpriteDef;

// ***** Movie classes

    
// Helper for MovieDefImpl
class ImportData
{  
public:

    struct Symbol
    {
        String   SymbolName;
        int      CharacterId;
        unsigned BindIndex;
    };

    ArrayLH<Symbol> Imports;
    StringLH        SourceUrl;
    unsigned        Frame;
    
    // Index of this import in the file, used to index
    // us within MovieDefImpl::ImportSourceMovies array.
    unsigned        ImportIndex;

    // Pointer to next import node.
    AtomicPtr<ImportData> pNext;

    ImportData()
        : Frame(0), ImportIndex(0)
    { }

    ImportData(const char* source, unsigned frame)
        : SourceUrl(source), Frame(frame), ImportIndex(0)
    { }

    void    AddSymbol(const char* psymbolName, int characterId, unsigned bindIndex)
    {
        Symbol s;
        s.SymbolName  = psymbolName;
        s.CharacterId = characterId;
        s.BindIndex   = bindIndex;
        Imports.PushBack(s);
    }

};

struct ResourceDataNode
{
    // Resource binding data used here.
    ResourceData      Data;
    unsigned             BindIndex;
    AtomicPtr<ResourceDataNode> pNext;    

    ResourceDataNode()        
    {  }
};

// Font data reference. We need to keep these in case we need to substitute fonts.
// We can scan through this list to figure out which fonts to substitute.
struct FontDataUseNode
{
    ResourceId      Id;
    // pointer is technically redundant
    Ptr<Font>       pFontData;          
    // This font data is used at specified font index.
    unsigned        BindIndex;

    AtomicPtr<FontDataUseNode> pNext;

    FontDataUseNode()
        : BindIndex(0)
    { }
};


// Frame bind data consists of 
// - imports, fonts, resource data for creation

struct FrameBindData
{
    // Frame that this structure represents. Stroring this field
    // allows us to skip frames that don't require binding.
    unsigned            Frame;
    // Number of bytes loaded by this frame.
    unsigned            BytesLoaded;

    // Number of Imports, Resources, and Fonts for this frame.
    // We use these cumbers instead of traversing the entire linked
    // lists, which can include elements from other frames.
    unsigned            FontCount;
    unsigned            ImportCount;
    unsigned            ResourceCount;
   
    // Singly linked lists of imports for this frame.   
    ImportData*         pImportData;
    // Fonts referenced in this frame.
    FontDataUseNode*    pFontData;

    // A list of ResourceData objects associated with each bindIndex in ResourceHandle.
    // Except for imports, which will have an invalid data object, appropriate resources can
    // be created by calling ResourceData::CreateResource
    ResourceDataNode*   pResourceData;
       
    // Pointer to next binding frame. It will be 0 if there are
    // either no more frames of they haven't been loaded yet (this
    // pointer is updated asynchronously while FrameUpdateMutex
    // is locked).    
    AtomicPtr<FrameBindData> pNextFrame;

    FrameBindData()
        : Frame(0), BytesLoaded(0),
          FontCount(0), ImportCount(0), ResourceCount(0),
          pImportData(0), pFontData(0), pResourceData(0)
    { }
};



// ***** DataAllocator

// DataAllocator is a simple allocator used for tag data in MovieDef.
// Memory is allocated consecutively and can only be freed all at once in a
// destructor. This scheme is used to save on allocation overhead, to reduce
// the number of small allocated chunks and to improve loading performance.

class DataAllocator
{   
    struct Block
    {
        Block*      pNext;
    };

    enum AllocConstants
    {
        // Leave 4 byte space to be efficient with buddy allocator.
        BlockSize = 8192 - sizeof(Block*) - 8
    };

    // Current pointer and number of bytes left there.
    UByte*          pCurrent;
    UPInt           BytesLeft;
    // All allocated blocks, including current.
    Block*          pAllocations;

    MemoryHeap*     pHeap;

    void*   OverflowAlloc(UPInt bytes);

public:

    DataAllocator(MemoryHeap* pheap);
    ~DataAllocator();

    inline void*   Alloc(UPInt bytes)
    {
        // Round up to pointer size.
        bytes = (bytes + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1);

        if (bytes <= BytesLeft)
        {
            void* pmem = pCurrent;
            pCurrent += bytes;
            BytesLeft -= bytes;
            return pmem;
        }
        return OverflowAlloc(bytes);
    }

    // Allocates an individual chunk of memory, without using the pool.
    // The chunk will be freed later together with the pool.
    void*    AllocIndividual(UPInt bytes);
    void*    AllocIndividualAlign(UPInt bytes, size_t a);

};


// LoadUpdateSync constrains Mutex and WaitCondition used to notify of loading
// progress when it takes place on a different thread. It is separated into a
// globally allocated object to allow notification as when loading task completes,
// *AFTER* loading task has released all other references to loaded memory. Once
// such full release tooks place, waiting thread can destroy loaded heaps and
// memory arenas.
#ifdef SF_ENABLE_THREADS

class LoadUpdateSync : public RefCountBase<LoadUpdateSync, StatMD_Other_Mem>
{
    Mutex           mMutex;
    WaitCondition   WC;
    bool            LoadFinished;

public:
    LoadUpdateSync() : LoadFinished(false)
    {
        SF_ASSERT(Memory::GetHeapByAddress(this) == Memory::GetGlobalHeap());
    }
    ~LoadUpdateSync()
    {
        //@DBG printf("~LoadUpdateSync\n");
    }

    Mutex&          GetMutex()      { return mMutex; }
    void            UpdateNotify()  { WC.NotifyAll(); }
    void            WaitForNotify() { WC.Wait(&mMutex); }

    // Called by loading thread to notify resource owner that it is done
    // and has released all of its memory pointers to the loaded data
    // (that is besides the pointer to this object).
    void            NotifyLoadFinished()
    {
        Mutex::Locker l(&mMutex);
        LoadFinished = true;
        WC.NotifyAll();
    }

    bool            IsLoadFinished() const { return LoadFinished; }

    void            WaitForLoadFinished()
    {
        Mutex::Locker lock(&mMutex);
        while (!IsLoadFinished())
            WaitForNotify();
    }
};

#endif


// ***** MovieDataDef - Stores file loaded data.

class MovieDataDef : public GFx::TimelineDef, public ResourceReport
{
public:

    // Store the resource key used to create us.
    // Note: There is a bit of redundancy here with respect to filename;
    // could resolve it with shared strings later on...
    ResourceKey         mResourceKey;    

    // MovieDataDefType - Describe the type of data that is stored in this DataDef.    
    enum MovieDataType
    {
        MT_Empty,   // No data, empty clip - useful for LoadVars into new level.
        MT_Flash,   // SWF or GFX file.
        MT_Image    // An image file.
    };

    MovieDataType       MovieType;


    // *** Binding and enumeration lists

    // DefBindingData contains a number of linked lists which are updated during
    // by the LoadProcess and used later on by BindProcess. Linked lists are used 
    // here because they can be updated without disturbing the reader thread; all
    // list data nodes are allocated from tag allocator. Containing DefBindingData
    // to a single object allows it to be easily passed to LoadProcess, which can
    // add items to appropriate lists. If load process fails, these entries are
    // correctly cleaned up by the virtue of being here.
    // 
    // The following points are significant:
    //  - pFrameData / pFrameDataLast writes are protected by the FrameUpdateMutex
    //    and can be waited on through FrameUpdated WaitCondition.
    //  - pImports and pFonts lists are maintained to allow independent traversal
    //    from other non-bind process functions. Parts of these lists are also
    //    referenced from FrameBindData, limited by a count.
    //  - pResourceNodes is kept to allow destructors to be called.
    //  - AtomicPtr<> is used to have proper Acquire/Release semantics on
    //    variables updated across multiple threads. 'pLast' pointers do not need
    //    sync because they are for loading thread housekeeping only.

    struct DefBindingData
    {
        // A linked list of binding data for each frame that requires it.
        // This may be empty if no frames required binding data.
        AtomicPtr<FrameBindData>    pFrameData;
        FrameBindData*              pFrameDataLast;

        // A linked list of all import nodes.
        AtomicPtr<ImportData>       pImports;
        ImportData*                 pImportsLast;
        // A linked list of all fonts; we keep these in case we need to substitute
        // fonts. Scan through this list to figure out which fonts to substitute.
        AtomicPtr<FontDataUseNode>  pFonts;
        FontDataUseNode*            pFontsLast;

        // A list of resource data nodes; used for cleanup.
        AtomicPtr<ResourceDataNode> pResourceNodes;
        ResourceDataNode*           pResourceNodesLast;

        DefBindingData()
            : pFrameDataLast(0),
            pImportsLast(0), pFontsLast(0), pResourceNodesLast(0)
        { }

        // Clean up data instanced created from tag allocator.
        ~DefBindingData();
    };


    // MovieDataDef starts out in LS_Uninitialized and switches to
    // the LS_LoadingFrames state as the frames start loading. During this
    // process the LoadingFrames counter will be incremented, with playback
    // being possible after it is greater then 0. 
    // For image files, state can be set immediately to LS_LoadFinished
    // while LoadingFrame is set to 1.
    enum MovieLoadState
    {
        LS_Uninitialized,
        LS_LoadingFrames,
        LS_LoadFinished,
        LS_LoadCanceled, // Canceled by user.
        LS_LoadError
    };


    // *** LoadTaskData

    // Most of the MovieDataDef data is contained in LoadTaskData class,
    // which is AddRef-ed separately. Such setup is necessary to allow
    // the loading thread to be canceled when all user reference count
    // for MovieDataDef are released. The loading thread only references
    // the LoadData object and thus can be notified when MovieDataDef dies.

    // Use a base class for LoadTaskData to make sure that TagMemAllocator
    // is destroyed last. That is necessary because LoadTaskData contains
    // multiple references to sprites and objects that use that allocator
    class LoadTaskDataBase : public RefCountBase<LoadTaskDataBase, StatMD_Other_Mem>
    {
    protected:
        // Tag/Frame Array Memory allocator.
        // Memory is allocated permanently until MovieDataDef/LoadTaskData dies.
        DataAllocator        TagMemAllocator;

        // Path allocator used for shape data. Use a pointer
        // to avoid including GFxShape.h.
        class PathAllocator* pPathAllocator;

        LoadTaskDataBase(MemoryHeap* pheap)
            : TagMemAllocator(pheap), pPathAllocator(0) { }
    };

    // Resource hash table used in LoadTaskData.
    typedef HashUncachedLH<
        ResourceId, ResourceHandle, ResourceId::HashOp>  ResourceHash;

    struct FrameLabelInfo
    {
        StringDH                Name;
        UInt32                  Number;

        FrameLabelInfo(MemoryHeap* heap, const String& name, UInt32 num)
            : Name(heap, name), Number(num) {}
    };
    struct SceneInfo
    {
        StringDH                Name;
        UInt32                  Offset;
        UInt32                  NumFrames;
        ArrayDH<FrameLabelInfo> Labels;

        SceneInfo(MemoryHeap* heap, const String& n, UInt32 off) 
            : Name(heap, n), Offset(off), NumFrames(0), Labels(heap) {}

        void AddFrameLabel(const String& n, UInt32 num)
        {
            Labels.PushBack(FrameLabelInfo(Name.GetHeap(), n, num));
        }
    };

    class LoadTaskData : public LoadTaskDataBase
    {
        friend class MovieDataDef;
        friend class LoadProcess;
        friend class MovieDefImpl;
        friend class MovieBindProcess;

        // Memory heap used for us and containing parent MovieDataDef.
        // This heap is destroyed on LoadTaskData deallocation.
        MemoryHeap*         pHeap;

        // This heap is used to allocate images in MovieData; these images will come
        // from this heap if KeepBindData flag has been specified. This heap is NOT
        // created until the first request, since it is optional.
        Ptr<MemoryHeap>     pImageHeap;


        // *** Header / Info Tag data.

        // General info about an SWF/GFX file

        // File path passed to FileOpenCallback for loading this object, un-escaped.
        // For Flash, this should always include the protocol; however, we do not require that.
        // For convenience of users we also support this being relative to the cwd.
        StringLH            FileURL;
        // Export header.
        MovieHeaderData     Header;
        // These attributes should become available any time after loading has started.
        // File Attributes, described by FileAttrType.
        unsigned            FileAttributes;
        // String storing meta-data. May have embedded 0 chars ?
        UByte*              pMetadata;
        unsigned            MetadataSize;


        // *** Current Loading State        

        // These values are updated based the I/O progress.
        MovieLoadState      LoadState;
        // Current frame being loaded; playback/rendering is only possible
        // when this value is greater then 0.
        unsigned            LoadingFrame;
        // This flag is set to 'true' if loading is being canceled;
        // this flag will be set if the loading process should terminate.
        // This value is different from LoadState == BS_Canceled
        volatile bool       LoadingCanceled;

        // Tag Count, used for reporting movie info; also advanced during loading.
        // Serves as a general information 
        unsigned            TagCount;


#ifdef SF_ENABLE_THREADS
        // Mutex locked when incrementing a frame and
        // thus adding data to pFrameBindData.
        Ptr<LoadUpdateSync> pFrameUpdate;
#endif

        // Read-time updating linked lists of binding data, as 
        // described by comments above for DefBindingData.
        DefBindingData      BindData;


        // *** Resource Data

        // Counter used to assign resource handle indices during loading.
        unsigned            ResIndexCounter;

        // Resource data is progressively loaded as new tags become available; during
        // loading new entries are added to the data structures below.
       
        // Lock applies to all resource data; only used during loading,
        // i.e. when (pdataDef->LoadState < LS_LoadFinished).
        // Locks: Resources, Exports and InvExports.
        mutable Lock        ResourceLock;

        // 'Resources' may contain some resources coming from another moviedef (for example,
        // ImageResource for ImageFileMovieDef (CharId_ImageMovieDef_ImageResource) that came
        // from different moviedef). In this case, we need to hold a strong reference to 
        // to its movieDef.
        Ptr<MovieDef>       pExtMovieDef; // holder for MovieDef to avoid "_Images" heap release

        // {resourceCharId -> resourceHandle} 
        // A library of all resources defined in this SWF/GFX file.
        ResourceHash        Resources;

        // A resource can also be exported
        // (it is possible to both import AND export a resource in the same file).
        // {export SymbolName -> Resource}   
        StringHashLH<ResourceHandle>                                Exports;
        // Inverse exports map from ResourceId, since mapping from ResourceHandle
        // would not be useful (we don't know how to create a handle from a Resource*).
        // This is used primarily in GetNameOfExportedResource.
        HashLH<ResourceId, StringLH, FixedSizeHash<ResourceId> >    InvExports;


        // *** Playlist Frames

        // Playlist lock, only applied when LoadState < LoadingFrames.
        // Locks: Playlist, InitActionList, NamedFrames.
        // TBD: Can become a bottleneck in FindPreviousPlaceObject2 during loading
        //      if a movie has a lot of frames and seek-back occurs.
        mutable Lock        PlaylistLock;

        ArrayLH<Frame>      Playlist;          // A list of movie control events for each frame.
        ArrayLH<Frame>      InitActionList;    // Init actions for each frame.
        int                 InitActionsCnt;    // Total number of init action buffers in InitActionList

        StringHashLH<unsigned> NamedFrames;        // 0-based frame #'s

        // Incremented progressively as gradients are loaded to assign ids
        // to their data, and match them with loaded files.
        ResourceId          GradientIdGenerator;
#ifdef GFX_ENABLE_SOUND
        SoundStreamDef*     pSoundStream;
#endif
        UInt32              SwdHandle;

        AutoPtr<ArrayLH<SceneInfo> > Scenes;
        
    public:

        // ***** Initialization / Cleanup

        LoadTaskData(MovieDataDef* pdataDef, const char *purl,
                     MemoryHeap* pheap);
        ~LoadTaskData();

        // Notifies LoadTaskData that MovieDataDef is being destroyed. This may be a
        // premature destruction if we are in the process of loading (in that case it
        // will lead to loading being canceled).
        void                OnMovieDataDefRelease();

        // Create an empty usable definition with no content.        
        void                InitEmptyMovieDef();
        // Create a definition describing an image file.
        bool                InitImageFileMovieDef(unsigned fileLength,
                                                  ImageResource *pimageResource, ImageCreator* imgCreator, 
                                                  Log* plog, bool bilinear = 1);


        // *** Accessors

        MemoryHeap*         GetHeap() const             { return pHeap; }      
        // Return heap used for image data of this movie; creates the heap if it doesn't exist.
        MemoryHeap*         GetImageHeap();

        // Get allocator used for path shape storage.
        PathAllocator*      GetPathAllocator()          { return pPathAllocator; }

        const char* GetFileURL() const                  { return FileURL.ToCStr(); }
        unsigned            GetTotalFrames() const      { return Header.FrameCount; }
        unsigned            GetVersion() const          { return Header.Version; }
        const ExporterInfo* GetExporterInfo() const { return Header.mExporterInfo.GetExporterInfo(); }

#ifdef SF_ENABLE_THREADS
        LoadUpdateSync*     GetFrameUpdateSync() const  { return pFrameUpdate; }
#endif
        
        // ** Loading

        // Initializes SWF/GFX header for loading.
        void                BeginSWFLoading(const MovieHeaderData &header);

        // Read a .SWF/GFX pMovie. Binding will take place interleaved with
        // loading if passed; no binding is done otherwise.
        void                Read(LoadProcess *plp, MovieBindProcess* pbp = 0);
        
        // Updates bind data and increments LoadingFrame. 
        // It is expected that it will also release any threads waiting on it in the future.
        // Returns false if there was an error during loading,
        // in which case LoadState is set to LS_LoadError can loading finished.
        bool                FinishLoadingFrame(LoadProcess* plp, bool finished = 0);

        // Update frame and loading state with proper notification, for use image loading, etc.
        void                UpdateLoadState(unsigned loadingFrame, MovieLoadState loadState);
        // Signals frame updated; can be used to wake up threads waiting on it
        // if another condition checked externally was met (such as cancel binding).
        void                NotifyFrameUpdated();

        // Waits until loading is completed, used by GFxFontLib.
        void                WaitForLoadFinish();
        
        // Waits until a specified frame is loaded
        void                WaitForFrame(unsigned frame);

        // *** Init / Access IO related data

        unsigned            GetMetadata(char *pbuff, unsigned buffSize) const;
        void                SetMetadata(UByte *pdata, unsigned size); 
        void                SetFileAttributes(unsigned attrs)   { FileAttributes = attrs; }
        unsigned            GetFileAttributes() const       { return FileAttributes; }

        // Allocate MovieData local memory.
        inline void*        AllocTagMemory(UPInt bytes)     { return TagMemAllocator.Alloc(bytes);  }
        // Allocate a tag directly through method above.
        template<class T>
        inline T*           AllocMovieDefClass()            { return Construct<T>(AllocTagMemory(sizeof(T))); }

        ResourceId          GetNextGradientId()             { return GradientIdGenerator.GenerateNextId(); }


        // Creates a new resource handle with a binding index for the resourceId; used 
        // to register ResourceData objects that need to be bound later.
        ResourceHandle      AddNewResourceHandle(ResourceId rid);
        // Add a resource during loading.
        void                AddResource(ResourceId rid, Resource* pres);   
        // A character is different from other resources because it tracks its creation Id.
        void                AddCharacter(ResourceId rid, CharacterDef* c)
        {
            c->SetId(rid);
            AddResource(rid, c);
        }

        // Expose one of our resources under the given symbol, for export.  Other movies can import it.        
        void                ExportResource(const String& symbol, ResourceId rid, const ResourceHandle &hres);

        bool                GetResourceHandle(ResourceHandle* phandle, ResourceId rid) const;

        Font*               GetFontData(ResourceId rid);

        // Gets binding data for the first frame, if any.
        FrameBindData*      GetFirstFrameBindData() const   { return BindData.pFrameData; }
        FontDataUseNode*    GetFirstFont() const            { return BindData.pFonts; }
        ImportData*         GetFirstImport() const          { return BindData.pImports; }
        
        
        // Labels the frame currently being loaded with the given name.
        // A copy of the name string is made and kept in this object.    
        virtual void        AddFrameName(const String& name, LogState *plog);
        virtual void        SetLoadingPlaylistFrame(const Frame& frame, LogState *plog);
        virtual void        SetLoadingInitActionFrame(const Frame& frame, LogState *plog);  

        bool                GetLabeledFrame(const char* label, unsigned* frameNumber, bool translateNumbers = 1) const;
        const String*       GetFrameLabel(unsigned frameNumber, unsigned* exactFrameNumberForLabel = NULL) const;
        // fills array of labels for the passed frame. One frame may have multiple labels.
        Array<String>*      GetFrameLabels(unsigned frameNumber, Array<String>* destArr) const;

        void                AddScene(const String& name, unsigned off);
        bool                HasScenes() const { return Scenes; }
        const SceneInfo*    GetScene(UPInt index) const;
        const SceneInfo*    GetScenes(UPInt* count) const;
        SceneInfo*          GetScene(UPInt index);
        SceneInfo*          GetScenes(UPInt* count);

        // *** Playlist access
        const Frame         GetPlaylist(int frameNumber) const;
        bool                GetInitActions(Frame* pframe, int frameNumber) const;
        unsigned            GetInitActionListSize() const;
        inline bool         HasInitActions() const   { return InitActionsCnt > 0; }

        SoundStreamDef*     GetSoundStream() const;
        void                SetSoundStream(SoundStreamDef* psoundStream);

        void                SetSwdHandle(UInt32 swdHandle) { SwdHandle = swdHandle; }
        UInt32              GetSwdHandle() const { return SwdHandle; }

        // Locker class used for LoadTaskData::Resources, Exports and InvExports.
        struct ResourceLocker
        {
            const LoadTaskData *pLoadData;

            ResourceLocker(const LoadTaskData* ploadData)
            {
                pLoadData = 0;
                if (ploadData->LoadState < LS_LoadFinished)
                {
                    pLoadData = ploadData;
                    pLoadData->ResourceLock.DoLock();
                }            
            }
            ~ResourceLocker()
            {
                if (pLoadData)
                    pLoadData->ResourceLock.Unlock();
            }
        };

        void SetExtMovieDef(MovieDef* m) { pExtMovieDef = m; }
    };
   


    // Data for all of our content.
    Ptr<LoadTaskData>  pData;



    // *** Initialization / Cleanup

    MovieDataDef(const ResourceKey &creationKey,
                    MovieDataType mtype, const char *purl,
                    MemoryHeap* pheap, bool debugHeap = false,
                    UPInt memoryArena = 0);
    ~MovieDataDef();

    virtual CharacterDefType GetType() const { return CharacterDef::MovieData; }

    // Creates MovieDataDef and also initializes its heap. If parent heap is not
    // specified, the new heap lifetime is ties to the internal LoadTaskData object,
    // since its reference goes away last.
    static MovieDataDef* Create(const ResourceKey &creationKey,
                                   MovieDataType mtype, const char *purl,
                                   MemoryHeap *pargHeap = 0, bool debugHeap = 0, UPInt memoryArena = 0)
    {        
        MovieDataDef* proot = SF_HEAP_NEW(pargHeap ? pargHeap : Memory::GetGlobalHeap())
                                    MovieDataDef(creationKey, mtype, purl, pargHeap, debugHeap, memoryArena);
        return proot;
    }

    MemoryHeap*        GetHeap() const { return pData->GetHeap(); }
    MemoryHeap*        GetImageHeap()  { return pData->GetImageHeap(); }


    // Create an empty MovieDef with no content; Used for LoadVars into a new level.
    void    InitEmptyMovieDef()
    {
        pData->InitEmptyMovieDef();
    }

    // Create a MovieDef describing an image file.
    bool    InitImageFileMovieDef(unsigned fileLength,
                                  ImageResource *pimageResource, ImageCreator* imgCreator, 
                                  Log* plog, bool bilinear = 1)
    {
        return pData->InitImageFileMovieDef(fileLength, pimageResource, imgCreator, plog, bilinear);
    }

    // Waits until loading is completed, used by GFxFontLib.
    void                WaitForLoadFinish(bool cancel = false) const
    {
        if (cancel)
            pData->OnMovieDataDefRelease();
        pData->WaitForLoadFinish();
    }

    // Waits until a specified frame is loaded
    void                WaitForFrame(unsigned frame) const { pData->WaitForFrame(frame); }

    
    // *** Information query methods.

    // All of the accessor methods just delegate to LoadTaskData; there
    // are no loading methods exposed here since loading happens only through LoadProcess.    
    float               GetFrameRate() const        { return pData->Header.FPS; }
    const RectF&        GetFrameRectInTwips() const { return pData->Header.FrameRect; }
    RectF               GetFrameRect() const        { return TwipsToPixels(pData->Header.FrameRect); }
    float               GetWidth() const            { return ceilf(TwipsToPixels(pData->Header.FrameRect.Width())); }
    float               GetHeight() const           { return ceilf(TwipsToPixels(pData->Header.FrameRect.Height())); }
    virtual unsigned    GetVersion() const          { return pData->GetVersion(); }
    virtual unsigned    GetLoadingFrame() const     { return pData->LoadingFrame; }
    virtual unsigned    GetSWFFlags() const         { return pData->Header.SWFFlags; }

    MovieLoadState      GetLoadState() const        { return pData->LoadState; }

    UInt32              GetFileBytes() const        { return pData->Header.FileLength; }
    unsigned            GetTagCount() const         { return pData->TagCount;  }
    const char*         GetFileURL() const          { return pData->GetFileURL(); }
    unsigned            GetFrameCount() const       { return pData->Header.FrameCount; }
    
    const ExporterInfo* GetExporterInfo() const { return pData->GetExporterInfo(); }    
    void                GetMovieInfo(MovieInfo *pinfo) const { pData->Header.GetMovieInfo(pinfo); }

    int                 GetASVersion() const        
    { 
        return (GetFileAttributes() & MovieDef::FileAttr_UseActionScript3) ? 3 : 2; 
    } 
    Ptr<ASSupport>              GetASSupport() const;
    
    // Sets MetaData of desired size.
    unsigned            GetFileAttributes() const   { return pData->FileAttributes; }
    unsigned            GetMetadata(char *pbuff, unsigned buffSize) const { return pData->GetMetadata(pbuff, buffSize); }
           

    // Fills in 0-based frame number.
    virtual bool        GetLabeledFrame(const char* label, unsigned* frameNumber, bool translateNumbers = 1) const
    {
        return pData->GetLabeledFrame(label, frameNumber, translateNumbers);
    }
    virtual const String* GetFrameLabel(unsigned frameNumber, unsigned* exactFrameNumberForLabel = NULL) const
    {
        return pData->GetFrameLabel(frameNumber, exactFrameNumberForLabel);
    }
    // fills array of labels for the passed frame. One frame may have multiple labels.
    virtual Array<String>* GetFrameLabels(unsigned frameNumber, Array<String>* destArr) const
    {
        return pData->GetFrameLabels(frameNumber, destArr);
    }

    const SceneInfo*    GetScenes(UPInt* count) const
    {
        return pData->GetScenes(count);
    }

    // Get font data by ResourceId.   
    bool                GetResourceHandle(ResourceHandle* phandle, ResourceId rid) const
    {
        return pData->GetResourceHandle(phandle, rid);
    }

    FontDataUseNode*    GetFirstFont() const     { return pData->GetFirstFont(); }
    ImportData*         GetFirstImport() const   { return pData->GetFirstImport(); }


    // Helper used to look up labeled frames and/or translate frame numbers from a string.
    static bool         TranslateFrameString(
                            const  StringHashLH<unsigned> &namedFrames,
                            const char* label, unsigned* frameNumber, bool translateNumbers);
    
    static const String* TranslateNumberToFrameString(const  StringHashLH<unsigned> &namedFrames, 
        unsigned frameNumber, unsigned* exactFrameNumberForLabel = NULL);
         
    // *** TimelineDef implementation.

    // Frame access from TimelineDef.
    virtual const Frame GetPlaylist(int frame) const        { return pData->GetPlaylist(frame); }
    unsigned            GetInitActionListSize() const       { return pData->GetInitActionListSize(); }
    inline  bool        HasInitActions() const              { return pData->HasInitActions(); }
    virtual bool        GetInitActions(Frame* pframe, int frame) const { return pData->GetInitActions(pframe, frame); }

#ifdef GFX_ENABLE_SOUND
    virtual SoundStreamDef*  GetSoundStream() const      { return pData->GetSoundStream(); }
    virtual void                SetSoundStream(SoundStreamDef* pdef) { pData->SetSoundStream(pdef); }
#endif

    // *** Creating MovieDefData file keys

    // Create a key for an SWF file corresponding to MovieDef.
    // Note that ImageCreator is only used as a key if is not null.
    static ResourceKey CreateMovieFileKey(const char* pfilename, SInt64 modifyTime, FileOpener* pfileOpener, ImageCreator* pimageCreator);

    // *** Resource implementation
    
    virtual ResourceKey     GetKey()                        { return mResourceKey; }
    virtual unsigned        GetResourceTypeCode() const     { return MakeTypeCode(RT_MovieDataDef); }

    // Resource report.
    virtual ResourceReport* GetResourceReport()          { return this; }
    virtual String          GetResourceName() const;
    virtual MemoryHeap*     GetResourceHeap() const         { return GetHeap(); }
    virtual void            GetStats(StatBag* pbag, bool reset = true) { SF_UNUSED2(pbag, reset); }

};


// State values that cause generation of a different binding
class  MovieDefBindStates : public RefCountBase<MovieDefBindStates, Stat_Default_Mem>
{
public:
    // NOTE: We no longer store pDataDef here, instead it is now a part of
    // a separate GFxMovieDefImplKey object. Such separation is necessary
    // to allow release of MovieDefImpl to cancel the loading process, which
    // is thus not allowed to AddRef to pDataDef. 

    // GFxStates that would cause generation of a new DefImpl binding.
    // These are required for the following reasons:
    //
    //  Class               Binding Change Cause
    //  ------------------  ----------------------------------------------------
    //  FileOpener       Substitute File can produce different data.
    //  URLBuilder   Can cause different import file substitutions.
    //  ImageCreator     Different image representation = different binding.
    //  ImportVisitor     (?) Import visitors can substitute import files/data.
    //  GFxImageVisitor      (?) Image visitors  can substitute image files/data.
    //  GradientParams   Different size of gradient = different binding.
    //  GFxFontParams       Different font texture sizes = different binding.
    //  FontCompactorParams


    Ptr<FileOpener>          pFileOpener;
    Ptr<URLBuilder>          pURLBulider;
    Ptr<ImageCreator>        pImageCreator;       
    Ptr<ImageFileHandlerRegistry> pImageFileHandlerRegistry;
    Ptr<ImportVisitor>       pImportVisitor;
    Ptr<FontPackParams>      pFontPackParams;
    Ptr<FontCompactorParams> pFontCompactorParams;
    Ptr<ImagePackParamsBase> pImagePackParams;

    MovieDefBindStates(StateBag* psharedState)
    {        
        // Get multiple states at once to avoid extra locking.
        State*                        pstates[7]    = {0,0,0,0,0,0,0};
        const static State::StateType stateQuery[7] =
        { State::State_FileOpener,     
        State::State_URLBuilder,
        State::State_ImageCreator,   
        State::State_ImportVisitor,
        State::State_FontPackParams,
        State::State_FontCompactorParams,
        State::State_ImagePackerParams
        };

        // Get states and assign them locally.
        psharedState->GetStatesAddRef(pstates, stateQuery, 7);
        pFileOpener          = *(FileOpener*)          pstates[0];
        pURLBulider          = *(URLBuilder*)          pstates[1];
        pImageCreator        = *(ImageCreator*)        pstates[2];
        pImportVisitor       = *(ImportVisitor*)       pstates[3];
        pFontPackParams      = *(FontPackParams*)      pstates[4];
        pFontCompactorParams = *(FontCompactorParams*) pstates[5];
        pImagePackParams     = *(ImagePackParamsBase*) pstates[6];
    }

    MovieDefBindStates(MovieDefBindStates *pother)
    {        
        pFileOpener         = pother->pFileOpener;
        pURLBulider         = pother->pURLBulider;
        pImageCreator       = pother->pImageCreator;
        pImportVisitor      = pother->pImportVisitor;
        pFontPackParams     = pother->pFontPackParams;
        pFontCompactorParams = pother->pFontCompactorParams;
        pImagePackParams  = pother->pImagePackParams;
        // Leave pDataDef uninitialized since this is used
        // from LoadStates::CloneForImport, and imports
        // have their own DataDefs.
    }

    // Functionality necessary for this MovieDefBindStates to be used
    // as a key object for MovieDefImpl.
    UPInt  GetHashCode() const;
    bool operator == (MovieDefBindStates& other) const;  
    bool operator != (MovieDefBindStates& other) const { return !operator == (other); }
};


// ***** Loading state collection


// Load states are used for both loading and binding, they 
// are collected atomically at the beginning of CreateMovie call,
// and apply for the duration of load/bind time.

class LoadStates : public RefCountBase<LoadStates, Stat_Default_Mem>
{
public:

    // *** These states are captured from Loader in constructor.

    // States used for binding of MovieDefImpl.
    Ptr<MovieDefBindStates> pBindStates;

    // Other states we might need
    // Load-affecting State(s).
    Ptr<LogState>           pLog;     
    Ptr<ParseControl>       pParseControl;
    Ptr<ProgressHandler>    pProgressHandler;
    Ptr<TaskManager>        pTaskManager;
    // Store cache manager so that we can issue font params warning if
    // it is not available.

    Ptr<ImageFileHandlerRegistry> pImageFileHandlerRegistry;

    Ptr<ZlibSupportBase>    pZlibSupport;
#ifdef GFX_ENABLE_VIDEO
    Ptr<Video::VideoBase>   pVideoPlayerState;
#endif
#ifdef GFX_ENABLE_SOUND
    Ptr<AudioBase>          pAudioState;
#endif
    Ptr<ASSupport>          pAS2Support;
    Ptr<ASSupport>          pAS3Support;
    // Weak States
    Ptr<ResourceWeakLib>    pWeakResourceLib;    


    // *** Loader

    // Loader back-pointer (contents may change).
    // We do NOT use this to access states because that may result 
    // in inconsistent state use when loading is threaded.    
    Ptr<LoaderImpl>         pLoaderImpl;

    // Cached relativePath taken from DataDef URL, stored
    // so that it can be easily passed for imports.
    String                  RelativePath;

    // Set to true if multi-threaded loading is using
    bool                    ThreadedLoading; 

    // *** Substitute fonts

    // These are 'substitute' fonts provider files that need to be considered when
    // binding creating a FontResource from ResourceData. Substitute fonts come from
    // files that have '_glyphs' in filename and replace fonts with matching names
    // in the file being bound.
    // Technically, this array should be a part of 'GFxBindingProcess'. If more
    // of such types are created, we may need to institute one.
    Array<Ptr<MovieDefImpl> > SubstituteFontMovieDefs;


    LoadStates();
    ~LoadStates();

    // Creates LoadStates by capturing states from pstates. If pstates
    // is null, all states come from loader. If binds states are null,
    // bind states come from pstates.
    LoadStates(LoaderImpl* ploader, StateBag* pstates = 0,
               MovieDefBindStates *pbindStates = 0);

    // Helper that clones load states, pBindSates.
    // The only thing left un-copied is MovieDefBindStates::pDataDef
    LoadStates*          CloneForImport() const;


    ResourceWeakLib*     GetLib() const              { return pWeakResourceLib.GetPtr();  }
    MovieDefBindStates*  GetBindStates() const       { return pBindStates.GetPtr(); }
    LogState*            GetLogState() const         { return pLog; }
    Log*                 GetLog() const              { return pLog ? pLog->GetLog() : 0; }
    TaskManager*         GetTaskManager() const      { return pTaskManager; }
    ProgressHandler*     GetProgressHandler() const  { return pProgressHandler; }
    ImageFileHandlerRegistry* GetImageFileHandlerRegistry() const { return pImageFileHandlerRegistry; }
 
    ZlibSupportBase*     GetZlibSupport() const      { return pZlibSupport; }
    FontCompactorParams* GetFontCompactorParams() const { return pBindStates->pFontCompactorParams; }

    FileOpener*          GetFileOpener() const       { return pBindStates->pFileOpener;  }
    FontPackParams*      GetFontPackParams() const   { return pBindStates->pFontPackParams; }

#ifdef GFX_ENABLE_VIDEO
    Video::VideoBase*    GetVideoPlayerState() const { return pVideoPlayerState; }
#endif
#ifdef GFX_ENABLE_SOUND
    AudioBase*           GetAudio() const            { return pAudioState; }
#endif
    // Initializes the relative path.
    void                 SetRelativePathForDataDef(MovieDataDef* pdef);
    const String&        GetRelativePath() const     { return RelativePath;  }


    // Obtains an image creator, but only if image data is not supposed to
    // be preserved; considers Loader::LoadKeepBindData flag from arguments.
    ImageCreator*        GetLoadTimeImageCreator(unsigned loadConstants) const; 
    // Obtains an image creator after binding is completed.
    ImageCreator*        GetImageCreator() const;
    // Delegated state access helpers. If loader flag LoadQuietOpen is set in loadConstants
    // and requested file can not be opened all error messages will be suppressed.
    // The 'pfilename' should be encoded as UTF-8 to support international names.
    File*      OpenFile(const char *pfilename, unsigned loadConstants = Loader::LoadAll);

    // Perform filename translation and/or copy by relying on translator.
    void        BuildURL(String *pdest, const URLBuilder::LocationInfo& loc) const;

    // Submit a background task. Returns false if TaskManager is not set or it couldn't 
    // add the task
    bool        SubmitBackgroundTask(LoaderTask* ptask);

    bool        IsThreadedLoading() const { return ThreadedLoading || pTaskManager; }
};



// ***  CharacterCreateInfo

// Simple structure containing information necessary to create a character.
// This is returned bu MovieDefImpl::GetCharacterCreateInfo().

// Since characters can come from an imported file, we also contain a 
// MovieDefImpl that should be used for a context of any characters
// created from this character def.
struct CharacterCreateInfo
{
    CharacterDef* pCharDef;
    MovieDefImpl* pBindDefImpl;
    Resource*     pResource;

    CharacterCreateInfo():pCharDef(NULL), pBindDefImpl(NULL), pResource(NULL) {}
    CharacterCreateInfo(CharacterDef* pcharDef, MovieDefImpl* pbindDefImpl) : 
        pCharDef(pcharDef), pBindDefImpl(pbindDefImpl), pResource(NULL) {}
};



// ***** MovieDefImpl

// This class holds the immutable definition of a GFxMovieSub's
// contents.  It cannot be played directly, and does not hold
// current state; for that you need to call CreateInstance()
// to get a MovieInstance.


class MovieDefImpl : public MovieDef
{
public:

    // Shared state, for loading, images, logs, etc.
    // Note that we actually own our own copy here.
    Ptr<StateBagImpl>       pStateBag;

    // We store the loader here; however, we do not take our
    // states from it -> instead, the states come from pStateBag.
    // The loader is here to give access to pWeakLib. This loader is also used
    // from MovieImpl to load child movies (but with a different state bag).
    // TBD: Do we actually need Loader then? Or just Weak Lib?
    // May need task manger too...
    // NOTE: This instead could be stored in MovieRoot. What's a more logical place?    
    Ptr<LoaderImpl>         pLoaderImpl;


    // States form OUR Binding Key.

    // Bind States contain the DataDef.    
    // It could be 'const MovieDefBindStates*' but we can't do that with Ptr.
    Ptr<MovieDefBindStates> pBindStates;

    class ReleaseNotifier
    {
    public:
        virtual ~ReleaseNotifier() {}

        virtual void OnMovieDefRelease(MovieDefImpl*) = 0;
    };
    HashSetLH<ReleaseNotifier*> ReleaseNotifiers;
    Lock ReleaseNotifiersLock;

    // *** BindTaskData

    // Most of the MovieDefImpl data is contained in BindTaskData class,
    // which is AddRef-ed separately. Such setup is necessary to allow
    // the loading thread to be canceled when all user reference count
    // for MovieDefImpl are released. The loading thread only references
    // the LoadData object and thus can be notified when MovieDefImpl dies.

    // BindStateType is used by BindTaskData::BindState.
    enum BindStateType
    {
        BS_NotStarted       = 0,
        BS_InProgress       = 1,
        BS_Finished         = 2,
        BS_Canceled         = 3, // Canceled due to a user request.
        BS_Error            = 4,
        // Mask for above states.
        BS_StateMask        = 0xF,

        // These bits store the status of what was
        // actually loaded; we can wait based on them.
        BSF_Frame1Loaded    = 0x100,
        BSF_LastFrameLoaded = 0x200,
    };


    class BindTaskData : public RefCountBase<BindTaskData, Stat_Default_Mem>
    {
        friend class MovieDefImpl;
        friend class MovieBindProcess;

        // Our own heap.
        MemoryHeap*                 pHeap;

        // AddRef to DataDef because as long as binding process
        // is alive we must exist.
        Ptr<MovieDataDef>           pDataDef;

        // A shadow pointer to pDefImpl that can 'turn' bad unless
        // accessed through a lock in GetDefImplAddRef(). Can be null
        // if MovieDefImpl has been deleted by another thread.
        MovieDefImpl*               pDefImpl_Unsafe;

        // Save load flags so that they can be propagated
        // into the nested LoadMovie calls.
        unsigned                    LoadFlags;
              

        // Binding table for handles in pMovieDataDef.
	    GFx::ResourceBinding        ResourceBinding;

        // NOTE: ImportSourceMovies and ResourceImports MUST BE defined BEFORE BoundShapeMeshProviders to
        // perform correct cleanup order in a destructor. BoundShapeMeshProviders may contain references to resources from
        // MovieDefs stored in these arrays (such as images in FillStyles of shapes).

        // Movies we import from; hold a ref on these, to keep them alive.
        // This array directly corresponds to MovieDataDef::ImportData.
        ArrayLH<Ptr<MovieDefImpl>, StatMD_Other_Mem>    ImportSourceMovies;
        // Lock for accessing above.
        //  - We also use this lock to protect pDefImpl
        //  - we use it for BoundShapeMeshProviders.
        Lock                                            ImportSourceLock;

        // Other imports such as through FontLib. These do not need
        // to be locked because it is never accessed outside binding
        // thread and destructor.
        ArrayLH<Ptr<MovieDefImpl>, StatMD_Other_Mem>    ResourceImports;

        // This hash table stores associations between ShapeMeshProvider in def (where images are 
        // used as fill styles and them need to be resolved) and an actual provider with resolved resources.
        // "ImportSourceLock" is used to sync access to it.
        HashLH<Render::ShapeMeshProvider*, Ptr<Render::ShapeMeshProvider> > BoundShapeMeshProviders;

        // *** Binding Progress State

        // Binding state variables are modified as binding progresses; there is only
        // one writer - the binding thread. The binding variables modified progressively
        // are BindState, BindingFrame and BytesLoaded. BindingFrame and BytesLoaded
        // are polled in the beginning of the next frame Advance, thus no extra sync
        // is necessary for them. BindState; however, can be waited on with BindStateMutex.

        // Current binding state modified after locking BindStateMutex.
        volatile unsigned           BindState;

#ifdef SF_ENABLE_THREADS
        // This mutex/WC pair is used to wait on the BindState, modified when a
        // critical event such as binding completion, frame 1 bound or error
        // take place. We have to wait on these when/if Loader::LoadWaitFrame1
        // or LoadWaitCompletion load flags are passed.
        Ptr<LoadUpdateSync>         pBindUpdate;        
#endif

        // Bound Frame - the frame we are binding now. This frame corresponds
        // to the Loading Frame, as only bound frames (with index < BindingFrame)
        // can actually be used.
        volatile unsigned           BindingFrame;

        // Bound amount of bytes loaded for the Binding frame,
        // must be assigned before BindingFrame.
        volatile UInt32             BytesLoaded;

        // This flag is set to 'true' if binding is being canceled;
        // this flag will be set if the binding process should terminate.
        // This value is different from BinsState == BS_Canceled
        volatile bool               BindingCanceled;


    public:
        
        // Technically BindTaskData should not store a pointer to MovieDefImpl,
        // however, we just pass it to initialize regular ptr in ResourceBinding.
        BindTaskData(MemoryHeap *pheap,
                     MovieDataDef *pdataDef,
                     MovieDefImpl *pdefImpl,
                     unsigned loadFlags, bool fullyLoaded);
        ~BindTaskData();

        MemoryHeap*         GetBindDataHeap() const { return pHeap; }

        // Notifies BindData that MovieDefImpl is being destroyed. This may be a premature
        // destruction if we are in the process of loading (in that case it will lead to
        // loading being canceled).
        void                OnMovieDefRelease();


        MovieDataDef*       GetDataDef() const { return pDataDef; }

        // Grabs OUR MovieDefImpl through a lock; can return null.
        MovieDefImpl*       GetMovieDefImplAddRef();

        
        // Bind state accessors; calling SetBindState notifies BindStateUpdated.
        void                SetBindState(unsigned newState);
        unsigned            GetBindState() const { return BindState; }    
        BindStateType       GetBindStateType() const { return (BindStateType) (BindState & BS_StateMask); }
        unsigned            GetBindStateFlags() const { return BindState & ~BS_StateMask; }

#ifdef SF_ENABLE_THREADS
        LoadUpdateSync*     GetBindUpdateSync() const  { return pBindUpdate; }
        void                ReleaseBindUpdateSync() 
        { 
            //SF_ASSERT(pBindUpdate->IsLoadFinished());
            pBindUpdate = NULL; 
        }
#endif
        // Wait for for bind state flag or error. Return true for success,
        // false if bind state was changed to error without setting the flags.
        bool                WaitForBindStateFlags(unsigned flags);

        // Query progress.
        unsigned            GetLoadingFrame() const     { return AtomicOps<unsigned>::Load_Acquire(&BindingFrame); }
        UInt32              GetBytesLoaded() const      { return BytesLoaded; }
        
        // Updates binding state Frame and BytesLoaded (called from image loading task).
        void                UpdateBindingFrame(unsigned frame, UInt32 bytesLoaded);


        // Access import source movie based on import index (uses a lock).
        MovieDefImpl*       GetImportSourceMovie(unsigned importIndex);
        // Adds a movie reference to ResourceImports array.
        void                AddResourceImportMovie(MovieDefImpl *pdefImpl);


        // *** Import binding support.

        // After MovieDefImpl constructor, the most important part of MovieDefImpl 
        // is initialization binding, i.e. resolving all of dependencies based on the binding states.
        // This is a step where imports and images are loaded, gradient images are
        // generated and fonts are pre-processed.
        // Binding is done by calls to MovieBindProcess::BindNextFrame.

        // Resolves and import during binding.
        void                ResolveImport(ImportData* pimport, MovieDefImpl* pdefImpl,
                                          LoadStates* pls, bool recursive);

        // Resolves an import of 'gfxfontlib.swf' through the GFxFontLib object.
        // Returns 1 if ALL mappings succeeded, otherwise 0.
        bool                ResolveImportThroughFontLib(ImportData* pimport);
                                                            //LoadStates* pls,
                                                            //MovieDefImpl* pourDefImpl);

        // Internal helper for import updates.
        bool                SetResourceBindData(ResourceId rid, ResourceBindData& bindData,
                                                const char* pimportSymbolName);

        bool                SetResourceBindData(ResourceDataNode *presnode, Resource* pres);

        Ptr<Render::ShapeMeshProvider> GetShapeMeshProvider(Render::ShapeMeshProvider* defMeshProv);
        void                AddShapeMeshProvider(Render::ShapeMeshProvider* defMeshProv, 
                                                 Render::ShapeMeshProvider* resolvedMeshProv);
    };

    Ptr<BindTaskData>  pBindData;
    
    // *** Constructor / Destructor

    MovieDefImpl(MovieDataDef* pdataDef,
                    MovieDefBindStates* pstates,
                    LoaderImpl* ploaderImpl,
                    unsigned loadConstantFlags,                    
                    StateBagImpl *pdelegateState = 0,
                    MemoryHeap* pargHeap = 0,
                    bool fullyLoaded = 0,
                    UPInt memoryArena = 0);
    ~MovieDefImpl();


    // Create a movie instance.
    MemoryContext*  CreateMemoryContext(const char* heapName, const MemoryParams& memParams, bool debugHeap);
    Movie*          CreateInstance(const MemoryParams& memParams, bool initFirstFrame, ActionControl* actionControl, Render::ThreadCommandQueue* renderQueue);
    Movie*          CreateInstance(MemoryContext* memContext, bool initFirstFrame, ActionControl* actionControl, Render::ThreadCommandQueue* renderQueue); 

    //  TBD: Should rename for Memory API cleanup
    virtual MemoryHeap*     GetLoadDataHeap() const { return pBindData->pDataDef->GetHeap(); }
    virtual MemoryHeap*     GetBindDataHeap() const { return pBindData->GetBindDataHeap(); }
    virtual MemoryHeap*     GetImageHeap() const { return pBindData->pDataDef->GetImageHeap(); }
    virtual Resource*       GetMovieDataResource() const { return pBindData->pDataDef; }


    // *** Creating MovieDefImpl keys

    // MovieDefImpl key depends (1) pMovieDefData, plus (2) all of the states
    // used for its resource bindings, such as file opener, file translator, image creator,
    // visitors, etc. A snapshot of these states is stored in MovieDefBindStates.
    // Movies that share the same bind states are shared through ResourceLib.

    // Create a key for an SWF file corresponding to MovieDef.
    static  ResourceKey  CreateMovieKey(MovieDataDef *pdataDef,
                                        MovieDefBindStates* pbindStates);
    
    
    // *** Property access

    MovieDataDef*       GetDataDef() const          { return pBindData->GetDataDef(); }

    // ...
    unsigned            GetFrameCount() const       { return GetDataDef()->GetFrameCount(); }
    float               GetFrameRate() const        { return GetDataDef()->GetFrameRate(); }
    RectF               GetFrameRect() const        { return GetDataDef()->GetFrameRect(); }
    float               GetWidth() const            { return GetDataDef()->GetWidth(); }
    float               GetHeight() const           { return GetDataDef()->GetHeight(); }
    virtual unsigned    GetVersion() const          { return GetDataDef()->GetVersion(); }
    virtual unsigned    GetSWFFlags() const         { return GetDataDef()->GetSWFFlags(); }    
    virtual const char* GetFileURL() const          { return GetDataDef()->GetFileURL(); }

    UInt32              GetFileBytes() const        { return GetDataDef()->GetFileBytes(); }
    virtual unsigned    GetLoadingFrame() const     { return pBindData->GetLoadingFrame(); }
    UInt32              GetBytesLoaded() const      { return pBindData->GetBytesLoaded(); }
    MovieDataDef::MovieLoadState      GetLoadState() const      { return GetDataDef()->GetLoadState(); }
    unsigned            GetTagCount() const         { return GetDataDef()->GetTagCount();  }
    
    inline unsigned     GetLoadFlags() const        { return pBindData->LoadFlags; }

    void                WaitForLoadFinish(bool cancel = false) const
    {
        GetDataDef()->WaitForLoadFinish(cancel);
#ifdef SF_ENABLE_THREADS
        // If using binding thread, make sure that binding thread has finished
        // and released its references to us as well.
        pBindData->GetBindUpdateSync()->WaitForLoadFinished();
#endif
    }

    void                WaitForFrame(unsigned frame) const { GetDataDef()->WaitForFrame(frame); }

    // Stripper info query.
    virtual const ExporterInfo*  GetExporterInfo() const  { return GetDataDef()->GetExporterInfo(); }

    RectF               GetFrameRectInTwips() const { return GetDataDef()->GetFrameRectInTwips(); }
    ResourceWeakLib*    GetWeakLib() const          { return pLoaderImpl->GetWeakLib(); }

    // Shared state implementation.
    virtual StateBag*   GetStateBagImpl() const       { return pStateBag.GetPtr(); }    

    // Overrides for users
    virtual unsigned    GetMetadata(char *pbuff, unsigned buffSize) const
        { return GetDataDef()->GetMetadata(pbuff, buffSize); }
    virtual unsigned    GetFileAttributes() const { return GetDataDef()->GetFileAttributes(); }


    // Log Error delegation
    void    LogError(const char* pfmt, ...)
    { 
        va_list argList; va_start(argList, pfmt);
        Ptr<Log> plog = GetLog();
        if (plog) plog->LogMessageVarg(Log_Error, pfmt, argList);
        va_end(argList); 
    }


    // Wait for for bind state flag or error. Return true for success,
    // false if bind state was changed to error without setting the flags.
    bool                WaitForBindStateFlags(unsigned flags) { return pBindData->WaitForBindStateFlags(flags); }
       

    // *** Resource Lookup

    // Obtains a resource based on its id. If resource is not yet resolved,
    // NULL is returned. Should be used only before creating an instance.
    // Type checks the resource based on specified type.
   // Resource*                GetResource(ResourceId rid, Resource::ResourceType rtype);
    // Obtains full character creation information, including CharacterDef.
    CharacterCreateInfo GetCharacterCreateInfo(ResourceId rid);

    // Get a binding table reference.
    const ResourceBinding&  GetResourceBinding() const { return pBindData->ResourceBinding; }
    ResourceBinding&        GetResourceBinding()       { return pBindData->ResourceBinding; }
 
   
    // *** MovieDef implementation

    virtual void            VisitImportedMovies(ImportVisitor* visitor);           
    virtual void            VisitResources(ResourceVisitor* pvisitor, unsigned visitMask = ResVisit_AllImages);
    virtual Resource*       GetResource(const char *pexportName) const;

    // Locate a font resource by name and style.
    // It's ok to return FontResource* without the binding because pBinding
    // is embedded into font resource allowing imports to be handled properly.
    struct SearchInfo
    {
        enum SearchStatus
        {
            NotFound = 0,
            FoundInResources,
            FoundInResourcesNoGlyphs,
            FoundInResourcesNeedFaux,
            FoundInImports,
            FoundInImportsFontLib,
            FoundInExports
        } Status;
        typedef HashSet<String, String::NoCaseHashFunctor> StringSet;
        StringSet ImportSearchUrls;
        String   ImportFoundUrl;
    };
    virtual FontResource*   GetFontResource(const char* pfontName, unsigned styleFlags, SearchInfo* psearchInfo = NULL);

    // Fill in the binding resource information together with its binding.
    // 'ignoreDef' is the def that will be ignored (incl all sub-defs), used to search globally when
    // local search was unsuccessful, to exclude the already searched branch.
    // Return 0 if Get failed and no bind data was returned.
    bool                    GetExportedResource(ResourceBindData *pdata, const String& symbol, MovieDefImpl* ignoreDef = NULL);   
    const String*           GetNameOfExportedResource(ResourceId rid) const;
    ResourceId              GetExportedResourceIdByName(const String& name) const;


    Ptr<ASSupport>          GetASSupport() const;

    // *** Resource implementation

    // checks if this moviedefimpl imports the "import" one directly
    bool                    DoesDirectlyImport(const MovieDefImpl* import);

    virtual ResourceKey     GetKey()                        { return CreateMovieKey(GetDataDef(), pBindStates); }
    virtual unsigned        GetResourceTypeCode() const     { return MakeTypeCode(RT_MovieDef); }

    void AddReleaseNotifier(ReleaseNotifier* rn);
    void RemoveReleaseNotifier(ReleaseNotifier* rn);
};




// *** GFxMovieDefBindProcess

// GFxMovieDefBindProcess stores the states necessary for binding. The actual
// binding is implemented by calling BindFrame for every frame that needs to be bound.
//
// Binding is separated into a separate object so that it can be reused independent
// of whether it takes place in a separate thread (which just calls BindFrame until
// its done) or is interleaved together with the loading process, which can call
// it after each frame.

class MovieBindProcess : public LoaderTask
{
    typedef MovieDefImpl::BindTaskData BindTaskData;


    // This is either current (if in BindNextFrame) or previous frame bind data.
    FrameBindData*      pFrameBindData;

    // Perform binding of resources.
    ResourceId          GlyphTextureIdGen;
  
	Ptr<ImagePacker> pImagePacker;
  
    // We keep a smart pointer to MovieDefImpl::BindTaskData and not
    // the MovieDefImpl itself; this allows us to cancel loading
    // gracefully if user's MovieDef is released.
    Ptr<BindTaskData>   pBindData;

    // Ok to store weak ptr since BindTaskData AddRefs to DataDef. 
    MovieDataDef*       pDataDef;   

    bool                Stripped;

    // We need to keep load import stack so we can detect recursion.
    LoaderImpl::LoadStackItem* pLoadStack;

    // Shared with binding
    TempBindData*       pTempBindData;

public:
   
    MovieBindProcess(LoadStates *pls,
                        MovieDefImpl* pdefImpl,
                        LoaderImpl::LoadStackItem* ploadStack = NULL);

    ~MovieBindProcess();


    typedef MovieDefImpl::BindStateType BindStateType;

    // Bind a next frame.
    // If binding failed, then BS_Error will be returned.
    BindStateType       BindNextFrame();


    // BindState delegates to MovieDefImpl.
    void                SetBindState(unsigned newState) { if (pBindData) pBindData->SetBindState(newState); }
    unsigned            GetBindState() const        { return pBindData->GetBindState(); }
    BindStateType       GetBindStateType() const    { return pBindData->GetBindStateType(); }
    unsigned            GetBindStateFlags() const   { return pBindData->GetBindStateFlags(); }

    // Gets binding data for the next frame, if any.
    FrameBindData*      GetNextFrameBindData()
    {
        if (pFrameBindData)
            return pFrameBindData->pNextFrame;
        return pDataDef->pData->GetFirstFrameBindData();
    }

    void                FinishBinding();

    void                SetTempBindData(TempBindData* ptempdata)     { pTempBindData = ptempdata; }
    TempBindData*       GetTempBindData() const                         { return pTempBindData; }


    // *** GFxTask implementation
    
    virtual void        Execute()
    {
        // Do the binding.
        while(BindNextFrame() == MovieDefImpl::BS_InProgress)
        { }     
    }

    virtual void        OnAbandon(bool started) 
    { 
        if (pBindData)
        {
            if (started)
                pBindData->BindingCanceled = true;
            else
                SetBindState(MovieDefImpl::BS_Canceled); 
        }
    }
};


class TimelineSnapshot;


// ***** Execute Tags

class ActionPriority
{
public:
    enum Priority
    {
        AP_Highest      = 0, // initclips for imported source movies
        AP_Initialize      , // onClipEvent(initialize)
        AP_InitClip        , // local initclips
        AP_Construct       , // onClipEvent(construct)/ctor
        AP_Frame           , // frame code
        AP_Normal = AP_Frame,
        AP_Load            , // onLoad-only

        AP_Count
    };
};


// Execute tags include things that control the operation of
// the GFxInteractiveObject.  Essentially, these are the events associated with a frame.
class ExecuteTag //: public GNewOverrideBase
{
public:

#ifdef SF_BUILD_DEBUG    
    ExecuteTag();
    virtual ~ExecuteTag();
#else
    virtual ~ExecuteTag() {}
#endif    
    
    static void     LoadData(Stream*  pin, UByte* pdata, UPInt datalen, UPInt offsetInBytes = 0) 
    {  
        pin->ReadToBuffer(pdata + offsetInBytes, (unsigned)datalen); 
    }

    virtual void    Execute(DisplayObjContainer* m)                           { SF_UNUSED(m); }
    virtual void    ExecuteWithPriority(DisplayObjContainer* m, ActionPriority::Priority prio) { SF_UNUSED(prio);  Execute(m); }

    virtual bool    IsRemoveTag() const { return false; }
    virtual bool    IsActionTag() const { return false; }
    virtual bool    IsInitImportActionsTag() const { return false;  }

    // A combination of ResourceId and depth - used to identify a tag created
    // character in a display list.
    struct DepthResId
    {
        int         Depth;
        ResourceId  Id;

        DepthResId() { Depth = 0; }
        DepthResId(ResourceId rid, int depth) : Depth(depth), Id(rid) { }
        DepthResId(const DepthResId& src) : Depth(src.Depth), Id(src.Id) { }
        inline DepthResId& operator = (const DepthResId& src) { Id = src.Id; Depth = src.Depth; return *this; }
        // Comparison - used during searches.
        inline bool operator == (const DepthResId& other) const { return (Id == other.Id) && (Depth == other.Depth); }
        inline bool operator != (const DepthResId& other) const { return !operator == (other); }
    };

    virtual void AddToTimelineSnapshot(TimelineSnapshot*, unsigned) {}
    virtual void Trace(const char*) {}
};


//
// GFxPlaceObjectBase
//
// Base interface for place object execute tags
//
class GFxPlaceObjectBase : public ExecuteTag
{
public:
    typedef ArrayLH<SwfEvent*, StatMD_Tags_Mem> EventArrayType;

    enum PlaceActionType
    {
        Place_Add       = 0x00,
        Place_Move      = 0x01,
        Place_Replace   = 0x02,
    };

    // Matrices, depth and character index.
    struct UnpackedData
    {
        CharPosInfo         Pos;
        EventArrayType*     pEventHandlers;
        const char*         Name;
        PlaceActionType     PlaceType;
    };

    virtual void                Unpack(UnpackedData& data) = 0;
    virtual EventArrayType*     UnpackEventHandlers() { return NULL; }
    virtual CharPosInfoFlags    GetFlags() const = 0;
    virtual void                Trace(const char* str) =0;
    virtual void                Execute(DisplayObjContainer* m) =0;

};


//
// GFxPlaceObjectUnpacked
//
// Unpacked version of PlaceObjectTag. Used for placing images on stage
// (loadMovie for images)
//
class GFxPlaceObjectUnpacked : public GFxPlaceObjectBase
{
private:
    GFxPlaceObjectUnpacked(const GFxPlaceObjectUnpacked& o);
    GFxPlaceObjectUnpacked& operator=(const GFxPlaceObjectUnpacked& o);

public: 
    // Unpacked matrices, depth and character index.
    CharPosInfo      Pos;

    // Constructors
    GFxPlaceObjectUnpacked() {};
    ~GFxPlaceObjectUnpacked() {};

    // *** GFxPlaceObjectBase implementation
    void        Unpack(GFxPlaceObjectBase::UnpackedData& data) 
    { 
        data.Name = NULL;
        data.pEventHandlers = NULL;
        data.PlaceType = GFxPlaceObjectBase::Place_Add;
        data.Pos = Pos; 
    }
    CharPosInfoFlags GetFlags() const
    {
        return Pos.Flags;
    }

    // *** ExecuteTag implementation
    // Place/move/whatever our object in the given pMovie.
    virtual void    Execute(DisplayObjContainer* m);
    void            AddToTimelineSnapshot(TimelineSnapshot*, unsigned frame);
    virtual void    Trace(const char* str);

    // *** Custom
    // Initialize to a specified value (used when generating MovieDataDef for images).
    void        InitializeToAdd(const CharPosInfo& posInfo)  { Pos     = posInfo; }

};

//
// PlaceObjectTag
//
// SWF1 PlaceObject record in packed form.
//
class PlaceObjectTag : public GFxPlaceObjectBase
{
private:
    PlaceObjectTag(const PlaceObjectTag& o);
    PlaceObjectTag& operator=(const PlaceObjectTag& o);

public: 
    bool            HasCxForm;   // Cxform is optional
    UByte           pData[1];

    // Constructors
    PlaceObjectTag();
    ~PlaceObjectTag();

    static UPInt SF_STDCALL ComputeDataSize(Stream* pin);
    void            CheckForCxForm(UPInt dataSz);

    // *** GFxPlaceObjectBase implementation
    void            Unpack(GFxPlaceObjectBase::UnpackedData& data);
    CharPosInfoFlags GetFlags() const;

    // *** ExecuteTag implementation
    // Place/move/whatever our object in the given pMovie.
    virtual void    Execute(DisplayObjContainer* m);
    void            AddToTimelineSnapshot(TimelineSnapshot*, unsigned frame);
    virtual void    Trace(const char* str);    

protected:
    UInt16          GetDepth() const;
};


//
// PlaceObject2Tag
//
// >=SWF6 PlaceObject2Tag record in packed form.
//
class PlaceObject2Tag : public GFxPlaceObjectBase
{
private:
    PlaceObject2Tag(const PlaceObject2Tag& o);
    PlaceObject2Tag& operator=(const PlaceObject2Tag& o);
public: 
   
    // Bit packings for tag bytes.
    enum PlaceObject2Flags
    {
        PO2_HasActions      = 0x80,
        PO2_HasClipBracket  = 0x40,
        PO2_HasName         = 0x20,
        PO2_HasRatio        = 0x10,
        PO2_HasCxform       = 0x08,
        PO2_HasMatrix       = 0x04,
        PO2_HasChar         = 0x02,
        PO2_FlagMove        = 0x01
    };

    UByte               pData[1];
   
    // Constructors
    PlaceObject2Tag() {}
    ~PlaceObject2Tag();

    // *** Helper methods for packed event handler pointer
    static bool SF_STDCALL HasEventHandlers(Stream* pin);
    // Move flags to the beginning and initialize packed event handler pointer
    static void SF_STDCALL RestructureForEventHandlers(UByte* pdata);
    // Get/Set the packed event handler pointer
    static EventArrayType* SF_STDCALL GetEventHandlersPtr(const UByte* pdata);
    static void SF_STDCALL SetEventHandlersPtr(UByte* pdata, EventArrayType* peh);

    static UPInt SF_STDCALL ComputeDataSize(Stream* pin, unsigned movieVersion);

    // *** GFxPlaceObjectBase implementation
    virtual void        Unpack(GFxPlaceObjectBase::UnpackedData& data) { UnpackBase(data, 6); }
    EventArrayType*     UnpackEventHandlers();
    CharPosInfoFlags    GetFlags() const;

    // *** ExecuteTag implementation
    // Place/move/whatever our object in the given pMovie.
    virtual void        Execute(DisplayObjContainer* m) { ExecuteBase(m, 6); }
    void                AddToTimelineSnapshot(TimelineSnapshot* snapshot, unsigned frame);
    virtual void        Trace(const char* str) { TraceBase(str, 6); }

protected:
    UInt16              GetDepth() const;
    PlaceActionType     GetPlaceType() const;

    void                ExecuteBase(DisplayObjContainer* m, UInt8 version);
    void                AddToTimelineSnapshotBase(TimelineSnapshot*, unsigned frame, UInt8 version);
    void                TraceBase(const char* str, UInt8 version);

    void                UnpackBase(GFxPlaceObjectBase::UnpackedData& data, UInt8 version);

    virtual void        ProcessEventHandlers
        (UnpackedData& data, StreamContext& sc, const UByte* prawdata, UInt8 version)
    { SF_UNUSED4(data, sc, prawdata, version); }
};

//
// < SWF6 PlaceObject2Tag record in packed form.
//
// Some fields are 16-bit instead of 32 in the regular version
//
class PlaceObject2Taga : public PlaceObject2Tag
{
private:
    PlaceObject2Taga(const PlaceObject2Taga& o);
    PlaceObject2Taga& operator=(const PlaceObject2Taga& o);
public:
    PlaceObject2Taga() {}

    // *** GFxPlaceObjectBase implementation
    virtual void        Unpack(GFxPlaceObjectBase::UnpackedData& data) { UnpackBase(data, 3); }

    // *** ExecuteTag implementation
    // Place/move/whatever our object in the given pMovie.
    virtual void        Execute(DisplayObjContainer* m) { ExecuteBase(m, 3); }
    virtual void        Trace(const char* str) { TraceBase(str, 3); }
};

//
// PlaceObject3Tag
//
// SWF8 PlaceObject3Tag record in packed form.
//
class PlaceObject3Tag : public GFxPlaceObjectBase
{
private:
    PlaceObject3Tag(const PlaceObject3Tag& o);
    PlaceObject3Tag& operator=(const PlaceObject3Tag& o);
public: 

    enum PlaceObject3Flags
    {
        PO3_Invisible       = 0x20,
        PO3_HasImage        = 0x10,
        PO3_HasClassName    = 0x08,
        PO3_BitmapCaching   = 0x04,
        PO3_HasBlendMode    = 0x02,
        PO3_HasFilters      = 0x01,
    };

    UByte               pData[1];
    
    // Constructors
    PlaceObject3Tag() {}
    ~PlaceObject3Tag();

    static UPInt SF_STDCALL ComputeDataSize(Stream* pin);

    // *** GFxPlaceObjectBase implementation
    void                Unpack(GFxPlaceObjectBase::UnpackedData& data);
    EventArrayType*     UnpackEventHandlers();
    CharPosInfoFlags    GetFlags() const;

    // *** ExecuteTag implementation
    // Place/move/whatever our object in the given pMovie.
    virtual void        Execute(DisplayObjContainer* m);
    void                AddToTimelineSnapshot(TimelineSnapshot*, unsigned frame);
    virtual void        Trace(const char* str);

protected:
    virtual void ProcessEventHandlers
        (UnpackedData& data, StreamContext& sc, const UByte* prawdata)
    { SF_UNUSED3(data, sc, prawdata); }

protected:
    UInt16              GetDepth() const;
    PlaceActionType     GetPlaceType() const;
};

class RemoveObject2Tag : public ExecuteTag
{
public:
    UInt16          Depth;

    // *** ExecuteTag implementation

    virtual void    Read(LoadProcess* p);

    virtual void    Execute(DisplayObjContainer* m);

    virtual bool    IsRemoveTag() const{ return true; }

    virtual void    AddToTimelineSnapshot(TimelineSnapshot*, unsigned frame);
    void            Trace(const char* str);

protected:
    // pse is ptr to ptr to TimelineSnapshot::SnapshotElement. 
    // CheckEventHandlers can set it to NULL to prevent further processing.
    virtual void CheckEventHandlers
        (void** pse, 
        GFxPlaceObjectBase::EventArrayType* pevts) { SF_UNUSED2(pse, pevts); }

    void            AddToTimelineSnapshot
        (UInt16 depth, TimelineSnapshot* psnapshot);
};

class RemoveObjectTag : public RemoveObject2Tag
{
public:
    UInt16          Id;

    // *** ExecuteTag implementation

    virtual void    Read(LoadProcess* p);

    virtual void    Execute(DisplayObjContainer* m);

    void            Trace(const char* str);
};


class SetBackgroundColorTag : public ExecuteTag
{
public:
    Render::Color  Color;

    void    Execute(DisplayObjContainer* m);

    void    ExecuteState(DisplayObjContainer* m)
        { Execute(m); }

    void    Read(LoadProcess* p);   
};



//
// GFxInitImportActions
//

// GFxInitImportActions - created for import tags.
class GFxInitImportActions : public ExecuteTag
{
    unsigned    ImportIndex;
public:
   
    GFxInitImportActions()
    {
        ImportIndex = 0;
    }
    void            SetImportIndex(unsigned importIndex)
    {
        ImportIndex = importIndex;
    }

    // Queues up logic to execute InitClip actions from the import,
    // using Highest priority by default.
    virtual void    Execute(DisplayObjContainer* m);  

    // InitImportActions that come from imported files need to be executed
    // in the MovieDefImpl binding context (otherwise we would index parent's
    // source movies incorrectly, resulting in recursive loop).
    void            ExecuteInContext(DisplayObjContainer* m, MovieDefImpl *pbindDef, bool recursiveCheck = 1);  

    virtual bool    IsInitImportActionsTag() const { return true; }
};



// ** Inline Implementation

// ** End Inline Implementation

}} // Scaleform::GFx

#endif // INC_SF_GFX_IMPL_H
