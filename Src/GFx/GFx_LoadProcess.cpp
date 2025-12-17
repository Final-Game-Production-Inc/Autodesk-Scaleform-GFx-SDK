/**************************************************************************

Filename    :   GFx_LoadProcess.cpp
Content     :   LoadProcess - tracks loading and binding state.
Created     :   
Authors     :   

Notes       :   This file contains class declarations used in
GFxPlayerImpl.cpp only. Declarations that need to be
visible by other player files should be placed
in GFxCharacter.h.

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/GFx_LoadProcess.h"

//#include "Render/Render_JPEGUtil.h"
#include "Kernel/SF_File.h"
#include "Kernel/SF_Debug.h"
#include "GFx/GFx_Audio.h"

// For GFxMovieImpl::IsPathAbsolute
//#include "GFx/GFx_PlayerImpl.h"

namespace Scaleform { namespace GFx {

// ***** LoadStates


LoadStates::LoadStates() : ThreadedLoading(false) 
{ 
    //printf("LoadStates::LoadStates : %x, thread : %d\n", this, GetCurrentThreadId());    
}

LoadStates::LoadStates(LoaderImpl* pimpl, StateBag* pstates,
                             MovieDefBindStates *pbindStates)
                             : ThreadedLoading(false)
{
    //printf("LoadStates::LoadStates : %x, thread : %d\n", this, GetCurrentThreadId());

    pLoaderImpl         = pimpl;
    pWeakResourceLib    = pimpl->GetWeakLib();

    // If pstates is null, states comde from loader.
    if (!pstates)
        pstates = pimpl;

    // Capture states.
    // We may override bind states when loading fonts from FontLib.
    pBindStates         = pbindStates ? *new MovieDefBindStates(pbindStates) :
                                        *new MovieDefBindStates(pstates);
    pLog                = pstates->GetLogState();
    pParseControl       = pstates->GetParseControl();
    pProgressHandler    = pstates->GetProgressHandler();
    pTaskManager        = pstates->GetTaskManager();
    // Record render config so that it can be used for image creation during loading.
    pImageFileHandlerRegistry = pstates->GetImageFileHandlerRegistry();
    pZlibSupport        = pstates->GetZlibSupport();
#ifdef GFX_AS2_SUPPORT
    pAS2Support         = pstates->GetAS2Support();
#endif
#ifdef GFX_AS3_SUPPORT
    pAS3Support         = pstates->GetAS3Support();
#endif
#ifdef GFX_ENABLE_VIDEO
    pVideoPlayerState   = pstates->GetVideo();
#endif
#ifdef GFX_ENABLE_SOUND
    pAudioState         = pstates->GetAudio();
#endif
}

LoadStates::~LoadStates()
{
    //printf("LoadStates::~LoadStates : %x, thread : %d\n", this, GetCurrentThreadId());
}

// Obtains an image creator, but only if image data is not supposed to
// be preserved; considers Loader::LoadKeepBindData flag from arguments.
ImageCreator* LoadStates::GetLoadTimeImageCreator(unsigned loadConstants) const
{
    // Image creator is only used as a key if it is bound to, as specified
    // by flags and IsKeepingImageData member.
    ImageCreator* pkeyImageCreator = 0;

    if (!(loadConstants & Loader::LoadKeepBindData) &&
        pBindStates->pImageCreator /*&&
        pBindStates->pImageCreator->GetBindKey(ImageCreator::Stage_Load)*/) //?AB
    {
        pkeyImageCreator = pBindStates->pImageCreator.GetPtr();
    }

    return pkeyImageCreator;
}

ImageCreator* LoadStates::GetImageCreator() const
{
    return pBindStates->pImageCreator;
}

// Initializes the relative path.
void        LoadStates::SetRelativePathForDataDef(MovieDataDef* pdef)
{    
    SF_ASSERT(pdef != 0);

    // Extract and store relative path from parent def.
    RelativePath = pdef->GetFileURL();    
    if (!URLBuilder::ExtractFilePath(&RelativePath))
        RelativePath.Clear();
}

// Implementation that allows us to override the log.
File*      LoadStates::OpenFile(const char *pfilename, unsigned loadConstants)
{
    if (!pBindStates->pFileOpener)
    {             
        // Don't even have a way to open the file.
        if (pLog && !(loadConstants & Loader::LoadQuietOpen))
            pLog->LogError(
            "Loader failed to open '%s', FileOpener not installe",
            pfilename);
        return 0;
    }

    return pBindStates->pFileOpener->OpenFileEx(pfilename, (loadConstants & Loader::LoadQuietOpen)? 0 : pLog->GetLog());
}

// Perform filename translation and/or copy by relying on translator.
void    LoadStates::BuildURL(String *pdest, const URLBuilder::LocationInfo &loc) const
{
    SF_ASSERT(pdest);
    URLBuilder* pbuilder = pBindStates->pURLBulider;

    if (!pbuilder)
        URLBuilder::DefaultBuildURL(pdest, loc);
    else
        pbuilder->BuildURL(pdest, loc);
}

// Helper that clones load states, pBindSates.
// The only thing left un-copied is MovieDefBindStates::pDataDef
LoadStates*  LoadStates::CloneForImport() const
{    
    Ptr<MovieDefBindStates> pnewBindStates = *new MovieDefBindStates(pBindStates);
    LoadStates*             pnewStates = new LoadStates;

    if (pnewStates)
    {
        pnewStates->pBindStates         = pnewBindStates;
        pnewStates->pLoaderImpl         = pLoaderImpl;
        pnewStates->pLog                = pLog;
        pnewStates->pProgressHandler    = pProgressHandler;
        pnewStates->pTaskManager        = pTaskManager;
        pnewStates->pParseControl       = pParseControl;
        pnewStates->pWeakResourceLib    = pWeakResourceLib;
        pnewStates->pImageFileHandlerRegistry = pImageFileHandlerRegistry;
        pnewStates->pZlibSupport        = pZlibSupport;
        pnewStates->pAS2Support         = pAS2Support;
        pnewStates->pAS3Support         = pAS3Support;
#ifdef GFX_ENABLE_SOUND
        pnewStates->pAudioState         = pAudioState;
#endif
#ifdef GFX_ENABLE_VIDEO
        pnewStates->pVideoPlayerState   = pVideoPlayerState;
#endif
    }
    return pnewStates;
}

bool LoadStates::SubmitBackgroundTask(LoaderTask* ptask)
{
    if (!pTaskManager)
        return false;
    //    pLoaderImpl->RegisterLoadProcess(ptask);
    if (!pTaskManager->AddTask(ptask))
    {
        //        pLoaderImpl->UnRegisterLoadProcess(ptask);
        return false;
    }
    return true;
}



// ***** LoadProcess


LoadProcess::LoadProcess(MovieDataDef* pdataDef,
                               LoadStates *pstates, unsigned loadFlags)
    : LoaderTask(pstates, Id_MovieDataLoad), ProcessInfo(pdataDef->GetHeap())
{
#ifdef SF_DEBUG_COUNT_TAGS
    TagCounts.Add(Tag_PlaceObject,           TagCountType("PlaceObject", sizeof(PlaceObjectTag)));
    TagCounts.Add(Tag_PlaceObject2,          TagCountType("PlaceObject2Tag", sizeof(PlaceObject2Tag)));
    TagCounts.Add(Tag_PlaceObject3,          TagCountType("PlaceObject3Tag", sizeof(PlaceObject3Tag)));
    TagCounts.Add(Tag_SetBackgroundColor,    TagCountType("SetBackgroundColor", sizeof(SetBackgroundColorTag)));
    TagCounts.Add(Tag_RemoveObject,          TagCountType("RemoveObject", sizeof(RemoveObjectTag)));
    TagCounts.Add(Tag_RemoveObject2,         TagCountType("RemoveObject2", sizeof(RemoveObject2Tag)));
    TagCounts.Add(Tag_DoAction,              TagCountType("DoAction", 0));
    TagCounts.Add(Tag_DoInitAction,          TagCountType("DoInitAction", 0));
#endif

    // Save states.
    //pStates             = pstates;
    // Cache parse flags for quick accesss.
    ParseFlags          = pstates->pParseControl.GetPtr() ?
        pstates->pParseControl->GetParseFlags() : 0;

    // Initialize other data used during loading.    
    pLoadData           = pdataDef->pData;
    pDataDef_Unsafe     = pdataDef;  // For SpriteDef argument only!
    pTimelineDef        = 0;

    //pJpegIn             = 0;
    LoadFlags           = loadFlags;
    LoadState           = LS_LoadingRoot;

    // Avoid resizing frame arrays to 0 (extra allocations).     // Anachronism
    //FrameTags[0].SetSizePolicy(GArrayPolicy::Buffer_NoShrink);
    //FrameTags[1].SetSizePolicy(GArrayPolicy::Buffer_NoShrink);
    //InitActionTags.SetSizePolicy(GArrayPolicy::Buffer_NoShrink);

    ImportIndex         = 0;
    ImportDataCount     = 0;
    ResourceDataCount   = 0;
    FontDataCount       = 0;
    // Clear linked lists.
    pImportData   = pImportDataLast   = 0;
    pResourceData = pResourceDataLast = 0;
    pFontData     = pFontDataLast     = 0;

    pAltStream          = NULL;
    pTempBindData       = 0;

    ASInitActionTagsNum = 0;

#ifdef SF_AMP_SERVER
    if (AmpServer::GetInstance().IsEnabled())
    {
        LoadProcessStats = *SF_HEAP_AUTO_NEW(&AmpServer::GetInstance()) AMP::ViewStats();
        AmpServer::GetInstance().AddLoadProcess(this);
    }
#endif
}

LoadProcess::~LoadProcess()
{
#ifdef SF_ENABLE_HTTP_LOADING
    String fileURL = pLoadData->GetFileURL();
    if (URLBuilder::IsProtocol(fileURL))
    {
        pLoadStates->pLoaderImpl->LoadingDone(fileURL);
    }
#endif
    pJpegTables = NULL; // MUST be released before pLoadData dies!
#ifdef SF_DEBUG_COUNT_TAGS
    SF_DEBUG_MESSAGE(1,    ">");
    SF_DEBUG_MESSAGE1(1,   "> Loading %s..",FilePath.ToCStr());
    HashIdentity<TagType, TagCountType>::ConstIterator iter = TagCounts.Begin();
    for (; iter != TagCounts.End(); ++iter)
    {
        const TagCountType& count = iter->Second;
        SF_DEBUG_MESSAGE3(1,   "> %s loaded: %d  (CSZ: %db)", count.Name, count.Count, count.ClassSize);
    }
    SF_DEBUG_MESSAGE(1,    ">");
#endif

    // Clear JPEG loader if it was used.
    //if (pJpegIn)
    //    delete pJpegIn;

    // Notify any waiters that our task thread is done with
    // all of its references to loaded data.
#ifdef SF_ENABLE_THREADS
    Ptr<LoadUpdateSync> ploadSync = pLoadData->GetFrameUpdateSync();
#endif
    ProcessInfo.ShutDown();
    pLoadData.Clear();
    pBindProcess.Clear();       
#ifdef SF_ENABLE_THREADS
    if (ploadSync)
        ploadSync->NotifyLoadFinished();
#endif
    SF_AMP_CODE(AmpServer::GetInstance().RemoveLoadProcess(this);)
}


bool    LoadProcess::BeginSWFLoading(File *pfile)
{
#ifdef SF_DEBUG_COUNT_TAGS
    FilePath = pfile->GetFilePath();
#endif
    // Read header while logging messages as appropriate, fail on wrong file format.
    if (!ProcessInfo.Initialize(pfile, GetLogState(),
                                pLoadStates->GetZlibSupport(), pLoadStates->pParseControl, true))
        return false;
    pLoadData->SetFileAttributes(ProcessInfo.FileAttributes);
    pLoadData->BeginSWFLoading(ProcessInfo.Header);
    return true;
}


void    LoadProcess::ReadRgbaTag(Color *pc, TagType tagType)
{
    if (tagType <= Tag_DefineShape2)
        GetStream()->ReadRgb(pc);
    else
        GetStream()->ReadRgba(pc);
}


GFx::TimelineDef::Frame LoadProcess::TagArrayToFrame(ExecuteTagArrayType &tagArray)
{
    GFx::TimelineDef::Frame frame;

    if (tagArray.GetSize())
    {
        UPInt memSize = sizeof(ExecuteTag*) * tagArray.GetSize();

        if ((frame.pTagPtrList = (ExecuteTag**)AllocTagMemory(memSize)) != 0)
        {
            memcpy(frame.pTagPtrList, &tagArray[0], memSize);
            frame.TagCount = (unsigned)tagArray.GetSize();
        }
        tagArray.Clear();
    }

    return frame;
}

// Apply frame tags that have been accumulated to the MovieDef.
void    LoadProcess::CommitFrameTags()
{
    // Add frame to time-line. Timeline can be either MovieDataDef or SpriteDef.
    if (LoadState == LS_LoadingSprite)
    {
        SF_ASSERT(pTimelineDef);
        pTimelineDef->SetLoadingPlaylistFrame(TagArrayToFrame(FrameTags[LoadState]), pLoadStates->GetLogState());
    }
    else
    {
        pLoadData->SetLoadingPlaylistFrame(TagArrayToFrame(FrameTags[LoadState]), pLoadStates->GetLogState());
        pLoadData->SetLoadingInitActionFrame(TagArrayToFrame(InitActionTags), pLoadStates->GetLogState());
    }
}

// Cleans up frame tags; only used if loading was canceled.
void  LoadProcess::CleanupFrameTags()
{    
    UPInt i;
    for(i=0; i<FrameTags[LS_LoadingSprite].GetSize(); i++)
        FrameTags[LS_LoadingSprite][i]->~ExecuteTag();
    for(i=0; i<FrameTags[LS_LoadingRoot].GetSize(); i++)
        FrameTags[LS_LoadingRoot][i]->~ExecuteTag();
    for(i=0; i<InitActionTags.GetSize(); i++)
        InitActionTags[i]->~ExecuteTag();

    FrameTags[LS_LoadingSprite].Clear();
    FrameTags[LS_LoadingRoot].Clear();
    InitActionTags.Clear();
}


// Adds a new resource and generates a handle for it.
ResourceHandle   LoadProcess::AddDataResource(ResourceId rid, const ResourceData &resData)
{
    SF_AMP_SCOPE_TIMER(LoadProcessStats, "LoadProcess::AddDataResource", Amp_Profile_Level_Low);
    // We get back a handle with a new bind index.
    ResourceHandle               rh = pLoadData->AddNewResourceHandle(rid);
    MovieDataDef::DefBindingData& bd = pLoadData->BindData;

    ResourceDataNode* pnode =  pLoadData->AllocMovieDefClass<ResourceDataNode>();
    if (pnode)
    {
        pnode->Data      = resData;
        pnode->BindIndex = rh.GetBindIndex();

        // Initialize beginnings of local and global lists.
        if (!pResourceData)
            pResourceData = pnode;

        // Insert data in the end of the global list.
        if (!bd.pResourceNodes)
            bd.pResourceNodes = pnode;
        else        
            bd.pResourceNodesLast->pNext = pnode;
        bd.pResourceNodesLast = pnode;

        ResourceDataCount++;
    }
    return rh;
}

static FontResourceCreator static_inst;
ResourceHandle   LoadProcess::AddFontDataResource(ResourceId rid, Font* pfontData)
{   
    // We get back a handler with bind index.
    ResourceData     resData(&static_inst, pfontData);
    ResourceHandle   rh = AddDataResource(rid, resData);    

    FontDataUseNode* pfnode = pLoadData->AllocMovieDefClass<FontDataUseNode>();
    if (pfnode)
    {
        MovieDataDef::DefBindingData& bd = pLoadData->BindData;

        pfnode->Id        = rid;
        pfnode->pFontData = pfontData;
        pfnode->BindIndex = rh.GetBindIndex();

        // Initialize this node as the beginning of the list.
        if (!pFontData)
            pFontData = pfnode;

        // Insert data in the end of the global list.
        if (!bd.pFonts)
            bd.pFonts = pfnode;
        else        
            bd.pFontsLast->pNext = pfnode;
        bd.pFontsLast = pfnode;

        FontDataCount++;
    }

    return rh;
}

void    LoadProcess::AddImportData(ImportData* pnode)
{
    pnode->ImportIndex = ImportIndex;
    ImportIndex++;

    // Insert data node in a list.
    if (!pImportData)
        pImportData = pnode;

    MovieDataDef::DefBindingData& bd = pLoadData->BindData;

    if (!bd.pImports)
        bd.pImports = pnode;
    else
        bd.pImportsLast->pNext = pnode;
    bd.pImportsLast = pnode;

    ImportDataCount++;
}


// Add a dynamically-loaded image resource, with unique key.
// This is normally used for SWF embedded images.
void    LoadProcess::AddImageResource(ResourceId rid, Render::ImageSource *pimage)
{    
    SF_AMP_SCOPE_TIMER(LoadProcessStats, "LoadProcess::AddImageResource", Amp_Profile_Level_Low);
    if (!pimage)
        return; //?
    ImageCreator* pkeyImageCreator = pLoadStates->GetLoadTimeImageCreator(LoadFlags);

    // Image creator is only used as a key if it is bound to, based on flags.
    if (pkeyImageCreator)
    {
        ImageCreateInfo icreateInfo(ImageCreateInfo::Create_FileImage, GetLoadHeap());

        Ptr<Render::Image> img = *pkeyImageCreator->CreateImage(icreateInfo, pimage);
        Ptr<ImageResource> pch = *SF_HEAP_NEW(icreateInfo.pHeap) ImageResource(img.GetPtr());
        AddResource(rid, pch);
    }
    else
    {
        // Creation of image resource is delayed due to late binding.
        // Create image resource data instead. This data will be used during binding.
        ImageCreateInfo icreateInfo(ImageCreateInfo::Create_FileImage, GetLoadHeap());
        ImageCreator imgCr;
        Ptr<Render::Image> img = *imgCr.CreateImage(icreateInfo, pimage);
        Ptr<Render::WrapperImageSource> imgSrc = *SF_HEAP_NEW(icreateInfo.pHeap) Render::WrapperImageSource(img);
        ResourceData rdata = ImageResourceCreator::CreateImageResourceData(imgSrc);
        AddDataResource(rid, rdata);
    }
}


// Creates a frame binding object; should be called only when
// a root frame is finished. Clears the internal import/font/resource lists.
FrameBindData* LoadProcess::CreateFrameBindData()
{
    FrameBindData* pbdata = pLoadData->AllocMovieDefClass<FrameBindData>();
    if (pbdata)
    {
        // Frame and Bytes loaded will be assigned by the caller.
        // pbdata->Frame           = ?;
        // pbdata->FrameBytesLoaded= ?;
        pbdata->ImportCount     = ImportDataCount;
        pbdata->pImportData     = pImportData;
        pbdata->FontCount       = FontDataCount;
        pbdata->pFontData       = pFontData;
        pbdata->ResourceCount   = ResourceDataCount;
        pbdata->pResourceData   = pResourceData;

        ImportDataCount         = 0;
        ResourceDataCount       = 0;
        FontDataCount           = 0;

        // Clear data pointers. We do not clear the data Last pointers,
        // allowing a full list of imports to be built up in the MovieDef.
        pImportData     = 0;
        pResourceData   = 0;
        pFontData       = 0;
    }
    return pbdata;
}



// ***** ImageResourceCreator - Image creator from files

// Creates/Loads resource based on data and loading process
bool    ImageFileResourceCreator::CreateResource(DataHandle hdata, ResourceBindData *pbindData,
                                                 LoadStates *pls, MemoryHeap *pbindHeap) const
{
    ImageFileInfo *prfi = (ImageFileInfo*) hdata;
    SF_ASSERT(prfi);   
    SF_UNUSED(pbindHeap); // Don't use bindHeap for shared resources.


    // Make a new fileInfo so that it can have a modified filename.   
    Ptr<ImageFileInfo> pimageFileInfo = *SF_NEW ImageFileInfo(*prfi);

    // Ensure format
    // TBD: If we always save the format for texture tags in gfxexport we
    // would not need to do this. Verify.
    if ((pimageFileInfo->Format == Loader::File_Unknown) && pimageFileInfo->pExporterInfo)
        pimageFileInfo->Format =
        (FileTypeConstants::FileFormatType) pimageFileInfo->pExporterInfo->Format;

    // Translate filename.
    URLBuilder::LocationInfo loc(URLBuilder::File_ImageImport,
                                    prfi->FileName, pls->GetRelativePath());
    pls->BuildURL(&pimageFileInfo->FileName, loc);


    // Now that we have a correct file object we need to match it against
    // the library so as to check if is is already loaded
    MemoryHeap* pimageHeap = pls->GetLib()->GetImageHeap();
    ResourceKey imageKey =
        ImageResource::CreateImageFileKey(pimageFileInfo, 
                                             pls->GetFileOpener(),
                                             pls->GetBindStates()->pImageCreator,
                                             pimageHeap);

    String                  errorMessage;
    ResourceLib::BindHandle bh;
    Ptr<ImageResource>      pimageRes = 0;

    if (pls->GetLib()->BindResourceKey(&bh, imageKey) == ResourceLib::RS_NeedsResolve)
    {
        // If not found, create an object of the right type.
        Ptr<Render::Image> pimage;
        ImageCreator*     pcreator = pls->GetBindStates()->pImageCreator;

        if (pcreator)
        {
            if (prfi->pExporterInfo)
            {
                ImageCreateExportInfo ico(pimageHeap, 0, prfi->Use);
                ico.SetStates(pls->GetLog(), pls->GetFileOpener(), pls->GetImageFileHandlerRegistry());
                ico.pExporterInfo = prfi->pExporterInfo;
                ico.Format      = LoaderImpl::FileFormat2RenderImageFile(prfi->Format);
                ico.TargetSize  = ImageSize(prfi->TargetWidth, prfi->TargetHeight);
                ico.ExportName  = prfi->ExportName;

                pimage = *pcreator->LoadExportedImage(ico, pimageFileInfo->FileName);
            }
            else
            {
                ImageCreateInfo ico(ImageCreateInfo::Create_FileImage, pimageHeap, 0, prfi->Use);
                ico.SetStates(pls->GetLog(), pls->GetFileOpener(), pls->GetImageFileHandlerRegistry());
                pimage = *pcreator->LoadImageFile(ico, pimageFileInfo->FileName);
            }
        }
        else
            SF_DEBUG_WARNING(1, "Image resource creation failed - ImageCreator not installed");        

        if (pimage)
        {
            // apply scaling matrix
            Matrix2F m;
            ImageSize s = pimage->GetSize();
            m.AppendScaling(float(prfi->TargetWidth)/s.Width, float(prfi->TargetHeight)/s.Height);
            pimage->SetMatrix(m);

            pimageRes = *SF_HEAP_NEW(pimageHeap) ImageResource(pimage.GetPtr(), imageKey, prfi->Use);
        }

        // Need to read header first.
        if (pimageRes)
            bh.ResolveResource(pimageRes);
        else
        {
            errorMessage = "Failed to load image '";
            errorMessage += pimageFileInfo->FileName;
            errorMessage += "'";

            bh.CancelResolve(errorMessage.ToCStr());
        }
    }
    else
    {
        // If Available and Waiting resources will be resolved here.
        if ((pimageRes = *(ImageResource*)bh.WaitForResolve()).GetPtr() == 0)
        {
            errorMessage = bh.GetResolveError();
        }
    }

    // If there was an error, display it
    if (!pimageRes)
    {
        pls->pLog->LogError("%s", errorMessage.ToCStr());
        return 0;
    }

    // Pass resource ownership to BindData.
    pbindData->pResource = pimageRes;
    return 1;    
}


ResourceData ImageFileResourceCreator::CreateImageFileResourceData(ImageFileInfo * prfi)
{
    static ImageFileResourceCreator inst;
    return ResourceData(&inst, prfi);
}



// Creates/Loads resource based on Render::Image during binding process
bool    ImageResourceCreator::CreateResource(DataHandle hdata, ResourceBindData *pbindData,
                                             LoadStates *pls, MemoryHeap *pbindHeap) const
{
    Render::ImageSource *pimageSrc = (Render::ImageSource*) hdata;
    SF_ASSERT(pimageSrc);

    // Create resource from image.
    ImageCreateInfo   icreateInfo(ImageCreateInfo::Create_FileImage, pbindHeap, 0, Resource::Use_None);
    icreateInfo.SetStates(pls->GetLog(), pls->GetFileOpener(), pls->GetImageFileHandlerRegistry());

    //icreateInfo.ThreadedLoading = pls->IsThreadedLoading();
    icreateInfo.pHeap           = pbindHeap;

    ImageCreator*     pcreator = pls->GetBindStates()->pImageCreator;
    if (!pcreator)
    {
        SF_DEBUG_WARNING(1, "Image resource creation failed - ImageCreator not installed");
        return 0;
    }
    Ptr<Render::Image> pimage;
    if (pcreator)    
        pimage= *pcreator->CreateImage(icreateInfo, pimageSrc);
    if (!pimage)
        return 0;

    Ptr<ImageResource> pimageRes = *SF_HEAP_NEW(pbindHeap) ImageResource(pimage.GetPtr());
    if (!pimageRes)
        return 0;

    // Pass resource ownership to BindData.
    pbindData->pResource = pimageRes;
    return 1;    
}

ResourceData ImageResourceCreator::CreateImageResourceData(Render::ImageSource* pimage)
{
    static ImageResourceCreator inst;
    //!AB: pimage might be NULL if ZLIB is not linked and zlib-compresed image is found.
    if (pimage)
        return ResourceData(&inst, pimage);
    else
        return ResourceData();
}


bool    SubImageResourceCreator::CreateResource(DataHandle hdata, ResourceBindData *pbindData,
                                                LoadStates *pls, MemoryHeap *pbindHeap) const
{
    SubImageResourceInfo* pinfo = (SubImageResourceInfo*)hdata;
    SF_ASSERT(pinfo); 
    SF_UNUSED2(pls, pbindHeap);

    ImageResource* pimageRes = pinfo->Image;

    if (!pimageRes)
    {
        ResourceHandle rh;
        pbindData->pBinding->GetOwnerDefImpl()->GetDataDef()->GetResourceHandle(&rh, pinfo->ImageId);
        Resource*      pres = rh.GetResource(pbindData->pBinding);
        if (pres && pres->GetResourceType() == Resource::RT_Image)
            pimageRes = (ImageResource*)pres;
    }

    if (pimageRes)
    {
//        Render::ImageBase* pbaseimage = pimageRes->GetImage();
//         if (pbaseimage->GetImageType() == Render::ImageBase::Type_Image)
//         {
//             Render::Image* pimg = static_cast<Render::Image*>(pbaseimage);
//             //pimg->SetTextureUsage(0);
//         }
        pbindData->pResource = 
            *SF_HEAP_NEW(pbindHeap) SubImageResource(pimageRes, ResourceId(0),
                                                     pinfo->Rect, pbindHeap);
        return 1;
    }

    return 0;
}

ResourceData SubImageResourceCreator::CreateSubImageResourceData(SubImageResourceInfo* pinfo)
{
    static SubImageResourceCreator inst;
    return ResourceData(&inst, pinfo);
}


// ***** FontResourceCreator - Font resource creator to use during binding


// Creates/Loads resource based on data and loading process.
bool   FontResourceCreator::CreateResource(DataHandle hdata, ResourceBindData *pbindData,
                                              LoadStates *pls, MemoryHeap *pbindHeap) const
{

    Font *pfd = (Font*) hdata;
    SF_ASSERT(pfd);

    // 1) Traverse through potential substitute fonts to see if substitution
    //    needs to take place.

    // FontResourceCreator may be responsible for substituting glyphs from
    // other files with '_glyphs' in their name.
    Array<Ptr<MovieDefImpl> >& fontDefs = pls->SubstituteFontMovieDefs;

    for(unsigned ifontDef=0; ifontDef<fontDefs.GetSize(); ifontDef++)
    {
        MovieDefImpl*    pdefImpl = fontDefs[ifontDef];
        FontDataUseNode* psourceFont = 
            pdefImpl->GetDataDef()->GetFirstFont();

        // Search imports for the font with the same name which has glyphs.
        for(; psourceFont != 0; psourceFont = psourceFont->pNext)
        {
            Font *psourceFontData = psourceFont->pFontData;

            if (psourceFontData->GetGlyphShapeCount() > 0)
            {
                if (pfd->MatchSubstituteFont(psourceFontData))
                {                    
                    // Set our binding.
                    // Note: Unlike us, the source is guaranteed to be fully loaded when
                    // this takes place, so we can just look up its binding.
                    // Set binding together with its internal table.
                    pdefImpl->GetResourceBinding().
                        GetResourceData(pbindData, psourceFont->BindIndex);
                    return 1;
                }
            }
        }
    }

    // 2) Not substitution. Create a font based on our data.


    // First, try to create a system font if the shape has no glyphs.      
    if (!pfd->HasVectorOrRasterGlyphs() && pfd->GetName())
    {
        // Just set not resolved flag for the font. This will instruct 
        // FontManager to search this font through FontLib or FontProvider
        pfd->SetNotResolvedFlag();
    }

    // If this is not a system font, create one based on out FontData.
    if (!pbindData->pResource)
    {
        // Our MovieDefImpl's pBinding should have been provided by caller.
        pbindData->pResource = *SF_HEAP_NEW(pbindHeap) FontResource(pfd, pbindData->pBinding);
    }


    // We could do font glyph rendering here, however it is better to delay it
    // till the end of frame binding so that textures from several fonts
    // can be potentially combined.

    return pbindData->pResource ? 1 : 0;
}
}} // Scaleform::GFx

