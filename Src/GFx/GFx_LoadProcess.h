/**************************************************************************

Filename    :   GFx_LoadProcess.h
Content     :   LoadProcess - tracks loading and binding state.
Created     :   
Authors     :   Michael Antonov

Notes       :   This file contains class declarations used in
                GFxPlayerImpl.cpp only. Declarations that need to be
                visible by other player files should be placed
                in GFxCharacter.h.

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_LOADPROCESS_H
#define INC_SF_GFX_LOADPROCESS_H

#include "Kernel/SF_Array.h"

#include "GFx/GFx_Types.h"
#include "GFx/GFx_Stream.h"
#include "GFx/GFx_LoaderImpl.h"

//#include "GFx/GFx_TaskManager.h"

// LoadProcess relies no many MovieDef function and APIs.
#include "GFx/GFx_MovieDef.h"
#include "GFx/AMP/Amp_ViewStats.h"

namespace Scaleform { 

namespace Render { namespace JPEG {
    class Input;
}} //Ren::JPEG

namespace GFx {

// ***** Declared Classes

class LoadProcess;

// ***** External Classes

class Loader;
class FontPackParams;
class MovieImpl;
class LoadQueueEntry;

#ifdef GFX_ENABLE_SOUND
class SoundData;
class SoundStreamDef;
struct GFxSoundFileInfo;
#endif

// Not accessed by binding thread until FinishBinding (when loading is complete)
// LoadProcess and GFxBindProcess both have a reference. Bind process releases it.

struct TempBindData : public NewOverrideBase<Stat_Default_Mem>
{
    HashSetLH<unsigned>       FillStyleImageWrap;
};

// ***** Loading Process

class LoadProcess : public LoaderTask, public LogBase<LoadProcess>
{
public:
    typedef MovieDataDef::LoadTaskData LoadTaskData;

private:
    // Load states used during load processing.
    //Ptr<LoadStates>         pStates;
    // Cached ParseControl flags from the states for more efficient checks.
    unsigned                ParseFlags;
    
    // Optional binding process. This pointer will be assigned if
    // we are doing frame-by-frame interleaved load/bind on one thread.    
    Ptr<MovieBindProcess>   pBindProcess;


    // We keep a smart pointer to MovieDataDef::LoadTaskData and not
    // the MovieDataDef itself; this allows us to cancel loading
    // gracefully if user's MovieDataDef is released.
    Ptr<LoadTaskData>       pLoadData;

    // A pointer to DataDef used to pass to SpriteDef constructor.
    // This pointer is usable from the sprite instance point of view,
    // but it can go bad if pMovieDef is released in playback thread.
    // Therefore, we can't really use it during loading.
    MovieDataDef*           pDataDef_Unsafe;


    // *** Temporary load process state

    SWFProcessInfo          ProcessInfo;

    Ptr<Render::JPEG::TablesHeader> pJpegTables;
    // Load flags, from GfxLoader::LoadConstants
    unsigned                LoadFlags;

    // Running total index of import in file, used to
    // initialize ImportIndex in ImportData objects
    unsigned                ImportIndex;

    // Shared with binding
    TempBindData*           pTempBindData;

    
    // *** Frame Bind Data content allocated

    // This data is used to initialize FrameBindData object for
    // each frame, committed at the end of each root frame.   
    unsigned                ImportDataCount;
    unsigned                ResourceDataCount;
    unsigned                FontDataCount;

    // Singly linked lists of imports for this frame.   
    ImportData*          pImportDataLast;
    ImportData*          pImportData;
    // A linked list of resource data that will be used for binding.
    ResourceDataNode*    pResourceData;
    ResourceDataNode*    pResourceDataLast;
    // Fonts referenced in this frame.
    FontDataUseNode*     pFontData;
    FontDataUseNode*     pFontDataLast;


    // *** Sprite/Root time-line loading stack

    enum LoadStateType
    {
        LS_LoadingRoot      = 0,
        LS_LoadingSprite    = 1,
        LS_LoadState_Count 
    };

    LoadStateType           LoadState;

    // Sprite definition into which data is being loaded. This will either
    // be SpriteDef for loading a sprite or null if we should load into pLoadData.
    TimelineIODef*           pTimelineDef;

    typedef ArrayConstPolicy<32, 16, false> SizePolicyType; // MinCapacity=32, Granularity=16, Shrink
    typedef Array<ExecuteTag*, Stat_Default_Mem, SizePolicyType> ExecuteTagArrayType;

    // Array of loaded frame pointers. Once the frame is done, this 
    // array is re-allocated as tag data and saved in MovieDataDef.
    // Note that tags themselves also come from Def's allocator.    
    ExecuteTagArrayType     FrameTags[LS_LoadState_Count];
    ExecuteTagArrayType     InitActionTags;
    unsigned                ASInitActionTagsNum;

    Stream*                 pAltStream;

public:

    LoadProcess(MovieDataDef* pdataDef, LoadStates *pstates, unsigned loadFlags);    
    ~LoadProcess();

    // Initializes SWF/GFX header for loading; returns 0 if there was
    // an error with loading header. Messages are properly logged.
    bool                BeginSWFLoading(File *pfile);

    // Assign a bind process, if we are to do simultaneous loading and binding.
    void                SetBindProcess(MovieBindProcess *pbindProcess)
    {
        pBindProcess = pbindProcess;
    }
    

    // Save/access an input object used for later loading DefineBits
    // Images (JPEG images without the table info).
    void                        SetJpegHeader(Render::JPEG::TablesHeader* pth) { SF_ASSERT(!pJpegTables); pJpegTables = pth; }
    Render::JPEG::TablesHeader* GetJpegHeader() const           { return pJpegTables; }


    // Return pDataDef pointer; can turn invalid if GFxMovieDef is relased asynchronously.
    MovieDataDef*           GetDataDef_Unsafe() const   { return pDataDef_Unsafe; }
    const SWFProcessInfo&   GetProcessInfo() const     { return ProcessInfo; }
    LoadTaskData*           GetLoadTaskData() const     { return pLoadData; }

    const char*             GetFileURL() const          { return pLoadData->GetFileURL(); }
    unsigned                GetVersion() const          { return pLoadData->GetVersion(); }
    unsigned                GetLoadingFrame() const     { return pLoadData->LoadingFrame; }
    unsigned                GetTotalFrames() const      { return pLoadData->GetTotalFrames(); }
    const ExporterInfo*     GetExporterInfo() const     { return pLoadData->GetExporterInfo(); }
    
    ResourceWeakLib*     GetWeakLib() const          { return pLoadStates->pWeakResourceLib; }
    LoadStates*          GetLoadStates() const       { return pLoadStates; }
    MovieDefBindStates*  GetBindStates() const       { return pLoadStates->GetBindStates(); }


    // *** Stream Support

    inline  Stream* GetStream()          { return !pAltStream ? &ProcessInfo.Stream : pAltStream; }

    // Sets alternative stream. Use with care, since pAltStream is not refcounted
    // and LoadProcess class does not own it.
    inline  void       SetAltStream(Stream* ns) { pAltStream = ns; }
    inline  bool       HasAltStream() const { return pAltStream != NULL; }

    // Stream inlines
    inline UInt8    ReadU8()                { return GetStream()->ReadU8(); }
    inline UInt16   ReadU16()               { return GetStream()->ReadU16(); }
    inline UInt32   ReadU32()               { return GetStream()->ReadU32(); }
    inline SInt8    ReadS8()                { return GetStream()->ReadS8(); }
    inline SInt16   ReadS16()               { return GetStream()->ReadS16(); }
    inline SInt32   ReadS32()               { return GetStream()->ReadS32(); }

    inline void     AlignStream()           { return GetStream()->Align(); }

    inline File*    GetUnderlyingFile ()    { return GetStream()->GetUnderlyingFile();  }
    inline int      GetTagEndPosition()     { return GetStream()->GetTagEndPosition(); }
    inline int      Tell()                  { return GetStream()->Tell();  }
    inline void     SetPosition(int pos)    { GetStream()->SetPosition(pos);  }

    void            ReadRgbaTag(Color *pc, TagType tagType);


    // *** Delegated Logging Support 

    // LogBase will output log messages to the appropriate logging stream,
    // but only if the enable option in context is set. 
    LogState*       GetLogState() const     { return pLoadStates->pLog; }
    Log*            GetLog() const          { return pLoadStates->pLog ? pLoadStates->pLog->GetLog() : 0 ; }

    // These work of captured parse control flags for efficiency. This is ok because
    // user updates that take place in the middle of loading are not guaranteed to take effect.
    inline bool     IsVerboseParse() const              { return (ParseFlags & ParseControl::VerboseParse) != 0; }
    inline bool     IsVerboseParseShape() const         { return (ParseFlags & ParseControl::VerboseParseShape) != 0; }
    inline bool     IsVerboseParseMorphShape() const    { return (ParseFlags & ParseControl::VerboseParseMorphShape) != 0; }
    inline bool     IsVerboseParseAction() const        { return (ParseFlags & ParseControl::VerboseParseAction) != 0; }


    inline bool     IsLoadingImageData() const          { return 1; }
    inline bool     IsLoadingFontShapes() const         { return 1; }


    // *** Resource Loading / Access

    // There function are used to add resource data to LoadTaskData during loading,
    // or query resources loaded so far. Most of I/O data is assigned through here,
    // GFxMovieDef only implements accessor methods.    

    MemoryHeap*     GetLoadHeap() const             { return pLoadData->GetHeap(); }

    // If we are keeping image data, we load it into the MovieData's image heap; if not,
    // that means it is a temporary so we place it into the global instead.
    MemoryHeap*     GetLoadImageHeap() const
    {
        // The LoadTimeImageCreator is used only if we are NOT keeping the image data.
        // This may be counter-intuitive but is logical:
        //   - If we are NOT keeping the image data, we would like to create a texture
        //     out of it ASAP, so that the temporary image is discarded. Since LoadTimeImageCreator
        //     is responsible for creating ImageInfo and GTexture from renderer, it is used
        //     during loading in that case.
        //   - If we ARE keeping the image data, we don't need a texture immediately since
        //     we can create it later on anyway. Thus, we'll have no image creator. But we will
        //     want to put such images into a dedicated heap for efficient storage and unloading
        //     (avoid fragmenting/growing the global heap).
#ifdef GFX_USE_IMAGE_HEAPS
        return (pLoadStates->GetLoadTimeImageCreator(LoadFlags)) ?
            Memory::GetGlobalHeap() : pLoadData->GetImageHeap();
#else
        return Memory::GetGlobalHeap();
#endif
    }    


    void                SetMetadata(UByte *pdata, unsigned size) { pLoadData->SetMetadata(pdata, size); }
    void                SetFileAttributes(unsigned attrs)   { pLoadData->SetFileAttributes(attrs); }
    unsigned            GetFileAttributes() const       { return pLoadData->GetFileAttributes(); }

    // Allocate MovieData local memory.
    inline void*        AllocTagMemory(UPInt bytes)     { return pLoadData->AllocTagMemory(bytes);  }
    // Allocate a tag directly through method above.
    template<class T>
    inline T*           AllocTag()                      { return Construct<T>(AllocTagMemory(sizeof(T))); }
    template<class T>
    inline T*           AllocTag(UPInt len)
    {
        return Construct<T>(AllocTagMemory(sizeof(T) + len - 1));
    }
    template<class T>
    inline T*           AllocMovieDefClass()            { return Construct<T>(AllocTagMemory(sizeof(T))); }

    ResourceId          GetNextGradientId()             { return pLoadData->GetNextGradientId(); }
    
    SF_AMP_CODE(void SetSwdHandle(UInt32 iSwdHandle) { pLoadData->SetSwdHandle(iSwdHandle); })

    ResourceHandle      AddNewResourceHandle(ResourceId rid)
    {
        return pLoadData->AddNewResourceHandle(rid);
    }
    // Add a resource during loading.
    inline void         AddResource(ResourceId rid, Resource* pres)
    {
        SF_ASSERT(LoadState == LS_LoadingRoot);
        if (LoadState == LS_LoadingRoot)
            pLoadData->AddResource(rid, pres);
    }    
    inline void         AddCharacter(ResourceId rid, CharacterDef* pdef)
    {
        return pLoadData->AddCharacter(rid, pdef);
    }

    // Adds a new resource and generates a handle for it.
    ResourceHandle      AddDataResource(ResourceId rid, const ResourceData &resData);
    ResourceHandle      AddFontDataResource(ResourceId rid, Font* pfontData);
    // Add a dynamically-loaded image resource, with unique key.
    // This is normally used for SWF embedded images.
    // Based on image creator flags, either the resource itself or
    // its resource data will be added.
    void                AddImageResource(ResourceId rid, Render::ImageSource *pimage);

    void                AddImportData(ImportData* pimportData);

    void                ExportResource(const String& symbol, ResourceId rid, const ResourceHandle &hres)
    {
        pLoadData->ExportResource(symbol, rid, hres);
    }

    inline bool         GetResourceHandle(ResourceHandle* phandle, ResourceId rid) const
    {
        return pLoadData->GetResourceHandle(phandle, rid);
    }

    inline Font*        GetFontData(ResourceId rid) { return pLoadData->GetFontData(rid); }
    
#ifdef GFX_ENABLE_SOUND

    void SetSoundStream( SoundStreamDef* psoundDef)
    {
        if (LoadState == LS_LoadingSprite)
        {
            SF_ASSERT(pTimelineDef);
            pTimelineDef->SetSoundStream(psoundDef);
        }
        else
            pLoadData->SetSoundStream(psoundDef);
    }
    SoundStreamDef* GetSoundStream()
    {
        if (LoadState == LS_LoadingSprite)
        {
            SF_ASSERT(pTimelineDef);
            return pTimelineDef->GetSoundStream();
        }
        else
            return pLoadData->GetSoundStream();
    }

    unsigned GetLoadingFrameForStreamSound()
    {
        if (LoadState == LS_LoadingSprite)
        {
            SF_ASSERT(pTimelineDef);
            return pTimelineDef->GetLoadingFrame();
        }
        return GetLoadingFrame();
    }

    unsigned GetFrameCountForStreamSound()
    {
        if (LoadState == LS_LoadingSprite)
        {
            SF_ASSERT(pTimelineDef);
            return pTimelineDef->GetFrameCount();
        }
        return GetProcessInfo().Header.FrameCount;
    }
#endif // SF_NO_SOUND

    // *** Frame Loading

    // Creates a frame binding object; should be called only when
    // a root frame is finished. Clears the internal import/font/resource lists.
    FrameBindData* CreateFrameBindData();


    // Enter and leave Sprite. 
    // Only one level of nesting is possible in SWF files.
    void                EnterSpriteDef(TimelineIODef *psprite)
    {
        SF_ASSERT(LoadState == LS_LoadingRoot);
        SF_ASSERT(psprite != 0);
        LoadState    = LS_LoadingSprite;
        pTimelineDef = psprite;
    }

    void                LeaveSpriteDef()
    {
        SF_ASSERT(LoadState == LS_LoadingSprite);
        LoadState    = LS_LoadingRoot;
        pTimelineDef = 0;
    }


    // Get allocator used for path shape storage.
    PathAllocator*  GetPathAllocator() const        { return pLoadData->GetPathAllocator(); }

    // Labels the frame currently being loaded with the given name.
    // A copy of the name string is made and kept in this object.    
    inline void     AddFrameName(const String& name, LogState *plog)
    {
        if (LoadState == LS_LoadingSprite)
            pTimelineDef->AddFrameName(name, plog);
        else
            pLoadData->AddFrameName(name, plog);
    }

    inline void     AddScene(const String& name, unsigned off)
    {
        pLoadData->AddScene(name, off);
    }
    inline bool     HasScenes() const 
    {
        return pLoadData->HasScenes();
    }
    inline const MovieDataDef::SceneInfo* GetScene(UPInt index) const
    {
        return pLoadData->GetScene(index);
    }
    inline const MovieDataDef::SceneInfo* GetScenes(UPInt* count) const
    {
        return pLoadData->GetScenes(count);
    }
    inline MovieDataDef::SceneInfo* GetScene(UPInt index)
    {
        return pLoadData->GetScene(index);
    }
    inline MovieDataDef::SceneInfo* GetScenes(UPInt* count)
    {
        return pLoadData->GetScenes(count);
    }
#ifdef SF_DEBUG_COUNT_TAGS
    String             FilePath;
    struct TagCountType
    {
        const char*     Name;
        unsigned        Count;
        unsigned        ClassSize;
        TagCountType(const char* name, unsigned csz) : Name(name), Count(0), ClassSize(csz) {}
    };
    HashIdentity<TagType, TagCountType>     TagCounts;

    inline void     CountTag(TagType tagType)
    {
        TagCountType* tagCount = TagCounts.Get(tagType);
        if (tagCount)
            tagCount->Count++;
    }
#endif

    inline void     AddExecuteTag(ExecuteTag* ptag)
    {
        FrameTags[LoadState].PushBack(ptag);
    }
    // Need to execute the given tag before entering the
    // currently-loading frame for the first time.    
    inline void     AddInitAction(ResourceId spriteId, ExecuteTag* ptag)
    {
        SF_UNUSED(spriteId);
        SF_ASSERT(LoadState == LS_LoadingRoot);
        InitActionTags.PushBack(ptag);
    }
    inline UPInt    GetInitActionTagsNum() const { return InitActionTags.GetSize(); } 
    inline unsigned GetASInitActionTagsNum() const { return ASInitActionTagsNum; }
    inline void     IncASInitActionTagsNum() { ++ASInitActionTagsNum; }
    
    // Helper method used to convert accumulated tags into a frame.
    TimelineDef::Frame TagArrayToFrame(ExecuteTagArrayType &tagArray);

    // Apply frame tags that have been accumulated to the MovieDef/SpriteDef.
    void            CommitFrameTags();
    // Cleans up frame tags; only used if loading was canceled.
    void            CleanupFrameTags();
    

    inline bool     FrameTagsAvailable() const
    {
        return (FrameTags[LoadState].GetSize() > 0) ||
               ((LoadState == LS_LoadingRoot) && (InitActionTags.GetSize() > 0));
    }

    void            SetTempBindData(TempBindData* ptempdata)     { pTempBindData = ptempdata; }
    TempBindData*   GetTempBindData() const                      { return pTempBindData; }


    void ReportProgress(const String& fileURL, const TagInfo& tagInfo, bool calledFromDefSprite)
    { 
        if (pLoadStates->pProgressHandler)
            pLoadStates->pProgressHandler->LoadTagUpdate(
                                              ProgressHandler::TagInfo(fileURL,tagInfo.TagType,tagInfo.TagOffset,
                                                                          tagInfo.TagLength, tagInfo.TagDataOffset),
                                              calledFromDefSprite); 
    }    // *** GFxTask implementation

    virtual void    Execute()
    {   
        SF_AMP_SCOPE_TIMER(LoadProcessStats, "LoadProcess::Execute", Amp_Profile_Level_Low);
        //MovieDefImpl::BindTaskData* pbd = NULL;
        //if (pBindProcess)
        //    pbd = pBindProcess->pBindData.GetPtr();
        //printf("LoadProcess::Execute: this: %x, BindData: %x, thread: %d --->>\n",this,pbd,GetCurrentThreadId());
        // Do the loading.
        pLoadData->Read(this, pBindProcess);
        //printf("LoadProcess::Execute: this: %x, BindData: %x, thread: %d ---<<\n",this,pbd,GetCurrentThreadId());
    }

    virtual void    OnAbandon(bool started)
    {        
        //MovieDefImpl::BindTaskData* pbd = NULL;
        //if (pBindProcess)
        //    pbd = pBindProcess->pBindData.GetPtr();
        //printf("LoadProcess::OnAbandon: this: %x, BindData: %x, thread: %d, started: %d\n",this,pbd,GetCurrentThreadId(),started);

        if (started)
            pLoadData->OnMovieDataDefRelease();
        if (pBindProcess && !started)
            pBindProcess->SetBindState(MovieDefImpl::BS_Canceled);
    }

    bool GetNextCodeOffset(int* offset)
    {
        Array<UInt32>& offsets = ProcessInfo.Header.mExporterInfo.CodeOffsets;

        if (offsets.GetSize() <= ProcessInfo.NextActionBlock)
        {
            return false;
        }
        *offset = offsets[ProcessInfo.NextActionBlock++];
        return true;
    }

    SF_AMP_CODE(Ptr<AMP::ViewStats> LoadProcessStats;)
};



// ***** Image creator from files

class ImageFileResourceCreator : public ResourceData::DataInterface
{
    typedef ResourceData::DataHandle DataHandle;

    // Creates/Loads resource based on data and loading process
    virtual bool    CreateResource(DataHandle hdata, ResourceBindData *pbindData,
                                   LoadStates *plp, MemoryHeap* pbindHeap) const;
public:

    static  ResourceData CreateImageFileResourceData(ImageFileInfo * prfi);    
};

class ImageResourceCreator : public ResourceData::DataInterface
{
    typedef ResourceData::DataHandle DataHandle;

    // Creates/Loads resource based on data and loading process
    virtual bool    CreateResource(DataHandle hdata, ResourceBindData *pbindData,
                                   LoadStates *plp, MemoryHeap* pbindHeap) const;
public:

    static  ResourceData CreateImageResourceData(Render::ImageSource *pimage);    
};

class SubImageResourceCreator : public ResourceData::DataInterface
{
    typedef ResourceData::DataHandle DataHandle;

    // Creates/Loads resource based on data and loading process
    virtual bool    CreateResource(DataHandle hdata, ResourceBindData *pbindData,
        LoadStates *plp, MemoryHeap* pbindHeap) const;
public:

    static  ResourceData CreateSubImageResourceData(SubImageResourceInfo* pinfo);    
};

// ***** Font resource creators from embedded font data

class FontResourceCreator : public ResourceData::DataInterface
{
    typedef ResourceData::DataHandle DataHandle;

    // Creates/Loads resource based on data and loading process
    virtual bool    CreateResource(DataHandle hdata, ResourceBindData *pbindData,
                                   LoadStates *plp, MemoryHeap* pbindHeap) const;
};
}} // Scaleform::GFx

#endif // INC_SF_GFX_LOADPROCESS_H
