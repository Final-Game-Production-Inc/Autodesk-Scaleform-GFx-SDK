/**************************************************************************

Filename    :   GFx_MovieDef.cpp
Content     :   SWF Player Core movie data structures.
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

#include "GFx/GFx_MovieDef.h"
#include "GFx/GFx_LoadProcess.h"
#include "Kernel/SF_File.h"

// For GFxSprite and MovieImpl
#include "GFx/GFx_Sprite.h"
#include "GFx/GFx_PlayerImpl.h"
#include "GFx/GFx_TextFieldDef.h"
#include "GFx/GFx_ButtonDef.h"

#include "GFx/GFx_FontGlyphPacker.h"
#include "GFx/GFx_FontManager.h"

#include "GFx/GFx_ImagePacker.h"

#include "GFx/GFx_ShapeSwf.h"
#include "GFx/Audio/GFx_Sound.h"

#include "GFx/AMP/Amp_Server.h"
#include "Render/Render_ShapeMeshProvider.h"

#ifdef GFX_ENABLE_VIDEO
#include "Video/Video_VideoCharacter.h"
#endif

namespace Scaleform {
    namespace GFx {

        // This is the key string that will cause import substitution
        // to use FontLib instead in order to support internationalization.
#define GFX_FONTLIB_IMPORT_KEY  "gfxfontlib.swf"


// ***** DataAllocator

// Allocation class used for tag data in MovieDataDef.
        DataAllocator::DataAllocator(MemoryHeap* pheap)
            : pHeap(pheap)
        {
            pAllocations = 0;
            pCurrent = 0;
            BytesLeft = 0;
        }

        DataAllocator::~DataAllocator()
        {
            while (pAllocations)
            {
                Block* pnext = pAllocations->pNext;
                SF_HEAP_FREE(pHeap, pAllocations);
                pAllocations = pnext;
            }
        }

        void* DataAllocator::OverflowAlloc(UPInt bytes)
        {
            // If the request is big, give it an individual chunk.
            if (bytes > BlockSize / 2)
                return AllocIndividual(bytes);

            if (bytes > BytesLeft)
            {
                Block* pblock = (Block*)SF_HEAP_ALLOC(pHeap, BlockSize + sizeof(Block), StatMD_Tags_Mem);
                if (!pblock)
                    return 0;
                // Insert to allocated list.
                pblock->pNext = pAllocations;
                pAllocations = pblock;
                // Assign free space.
                pCurrent = (UByte*)(pblock + 1);
                BytesLeft = BlockSize;
            }

            void* pmem = pCurrent;
            pCurrent += bytes;
            BytesLeft -= bytes;

            return pmem;
        }

        void* DataAllocator::AllocIndividual(UPInt bytes)
        {
            Block* pblock = (Block*)SF_HEAP_ALLOC(pHeap, bytes + sizeof(Block), StatMD_Tags_Mem);
            if (!pblock)
                return 0;
            // Insert to allocated list.
            pblock->pNext = pAllocations;
            pAllocations = pblock;
            return (pblock + 1);
        }

        void* DataAllocator::AllocIndividualAlign(UPInt bytes, size_t a)
        {
            if (a == 0)
            {
                return AllocIndividual(bytes);
            }
            size_t blockSize = sizeof(Block);
            size_t headerSize = ((blockSize + a - 1) / a) * a;
            SF_ASSERT(headerSize % blockSize == 0);
            Block* pblock = (Block*)SF_HEAP_MEMALIGN(pHeap, bytes + headerSize, a, StatMD_Tags_Mem);
            if (!pblock)
                return 0;
            // Insert to allocated list.
            pblock->pNext = pAllocations;
            pAllocations = pblock;
            return (pblock + headerSize / blockSize);
        }


        // ***** MovieDataDef


        MovieDataDef::MovieDataDef(const ResourceKey& creationKey,
            MovieDataType mtype, const char* purl,
            MemoryHeap* pargHeap, bool debugHeap, UPInt memoryArena)
            : mResourceKey(creationKey), MovieType(mtype)
        {
            // printf("Thr %4d, %8x : MovieDataDef()\n", GetCurrentThreadId(), this);

            // LoadTaskData gets its own heap, if it loaded separately. It is not possible
            // to put MovieDataDef into the same heap since destruction order is not known,
            // so we don't know to which object we should attach heap destroy.
            MemoryHeap* pheap = pargHeap;
            if (!pheap)
            {
                String heapName("MovieData \"", GetShortFilename(purl), "\"");
                unsigned    heapFlags = debugHeap ? MemoryHeap::Heap_UserDebug : 0;
                MemoryHeap::HeapDesc desc(heapFlags);
                desc.Flags |= MemoryHeap::Heap_FixedGranularity;
                desc.Granularity = 8 * 1024;
                desc.Reserve = 0;
                desc.HeapId = HeapId_MovieData;
                desc.Arena = memoryArena;
                pheap = Memory::GetGlobalHeap()->CreateHeap(heapName.ToCStr(), desc);
            }
            pData = *SF_HEAP_NEW(pheap) LoadTaskData(this, purl, pheap);

            if (!pargHeap)
                pheap->ReleaseOnFree(pData.GetPtr());
        }

        MovieDataDef::~MovieDataDef()
        {
            //printf("Thr %4d, %8x : ~MovieDataDef()\n", GetCurrentThreadId(), this);
            // Notify LoadDataTask about our death.
            pData->OnMovieDataDefRelease();
        }


        // ~DefBindingData is executed as a part of ~MovieDataDef, it is
        // responsible for calling destructors on BindData linked list content
        // that was allocated from tag allocator.
        MovieDataDef::DefBindingData::~DefBindingData()
        {
            // Destroy bind frames.
            FrameBindData* pframeData = pFrameData;
            pFrameData = 0;
            while (pframeData)
            {
                FrameBindData* pframeNext = pframeData->pNextFrame;
                pframeData->~FrameBindData();
                pframeData = pframeNext;
            }

            // Destroy imports.
            while (pImports)
            {
                ImportData* pimport = pImports;
                pImports = pImports->pNext;
                pimport->~ImportData();
            }

            // Destroy bind data.
            while (pResourceNodes)
            {
                ResourceDataNode* pnode = pResourceNodes;
                pResourceNodes = pResourceNodes->pNext;
                pnode->~ResourceDataNode();
            }

            // Destroy fonts data nodes.
            while (pFonts)
            {
                FontDataUseNode* pnode = pFonts;
                pFonts = pFonts->pNext;
                pnode->~FontDataUseNode();
            }
        }

        // Helper used to look up labeled frames and/or translate frame numbers from a string.
        bool    MovieDataDef::TranslateFrameString(
            const StringHashLH<unsigned>& namedFrames,
            const char* label, unsigned* frameNumber, bool translateNumbers)
        {
            if (!label || !label[0])
                return 0;

            // First, determines if a string is a frame number. Generally,
            // if it is NOT a frame number, the string is treated as a label.
            // However, there are some cases when a string number check is
            // not done (i.e. op 0x8C), so that is treated as an option.
            if (translateNumbers)
            {
                // The string must evaluate to an integer to be converted to
                // frame number; frames like "8.5" are treated as labels. Whitespace,
                // '+' or '-' characters, however, are allowed in front of a number string.
                // However, any other characters in the string would cause it to be a label.
                // TBD: May be that matches ECMA ActionScript number conversion ?

                bool        digitFound = 0;
                int         i;

                // Check whether to interpret as a number or a label.
                for (i = 0; label[i] != 0; i++)
                {
                    UByte ch = label[i];
                    if ((ch >= '0') && (ch <= '9'))
                    {
                        digitFound = 1;
                        continue;
                    }
                    if (ch == '+' || ch == '-')
                    {
                        if (!digitFound)
                            continue;
                    }
                    if (ch == ' ' || ch == '\t')
                        continue;
                    // Any other char? It's a label.
                    goto translate_frame_label;
                }

                // This must be  a frame number: do conversion.
                char* tail = 0;
                unsigned    number = (unsigned)SFstrtod(label, &tail);
                // Check for conversion failure.
                if (tail == label || *tail != 0)
                    return 0;

                // Frames number arguments start with 1, so make it 0-based.
                *frameNumber = number - 1;
                return 1;
            }

        translate_frame_label:

            // We have found a label, do lookup.
            return namedFrames.GetCaseInsensitive(String(label), frameNumber);
        }

        const String* MovieDataDef::TranslateNumberToFrameString(
            const  StringHashLH<unsigned>& namedFrames,
            unsigned frameNumber, unsigned* exactFrameNumberForLabel)
        {
            StringHashLH<unsigned>::ConstIterator it = namedFrames.Begin();
            unsigned exctFrame = 0;
            const String* frameName = NULL;
            for (; it != namedFrames.End(); ++it)
            {
                if (frameNumber >= it->Second && it->Second >= exctFrame)
                {
                    exctFrame = it->Second;
                    frameName = &it->First;
                }
            }
            if (exactFrameNumberForLabel)
                *exactFrameNumberForLabel = exctFrame;
            return frameName;
        }


        // *** MovieDataDef::LoadTaskData


        MovieDataDef::LoadTaskData::LoadTaskData(MovieDataDef* pdataDef, const char* purl,
            MemoryHeap* pheap)
            : LoadTaskDataBase(pheap), pHeap(pheap),
            FileURL(purl), GradientIdGenerator(ResourceId::IdType_GradientImage)
#ifdef GFX_ENABLE_SOUND
            , pSoundStream(NULL)
#endif
        {
            // We don't store the pDataDef pointer because it is unsafe if
            // MovieDataDef dies in main thread. Just use it for SpriteDef.

            FileAttributes = 0;
            pMetadata = 0;
            MetadataSize = 0;

            pPathAllocator = SF_HEAP_NEW(pHeap) PathAllocator();

            LoadingCanceled = false;
            LoadState = LS_Uninitialized;
            LoadingFrame = 0;
            TagCount = 0; // Should be based on export?

            ResIndexCounter = 0;
            InitActionsCnt = 0;

#ifdef SF_ENABLE_THREADS
            // do not create LoadUpdateSync for protocol images! Save 2 mutexes.
            if (pdataDef->MovieType != MovieDataDef::MT_Image || !LoaderImpl::IsProtocolImage(purl))
                pFrameUpdate = *SF_NEW LoadUpdateSync();
#endif

            // Add an empty character id, for createEmptyMovieClip.
            Ptr<SpriteDef> pdef = *SF_HEAP_NEW(pHeap) SpriteDef(pdataDef);
            pdef->InitEmptyClipDef();
            AddResource(ResourceId(CharacterDef::CharId_EmptyMovieClip), pdef);

            Ptr<TextFieldDef> ptextDef = *SF_HEAP_NEW(pHeap) TextFieldDef();
            ptextDef->InitEmptyTextDef();
            AddResource(ResourceId(CharacterDef::CharId_EmptyTextField), ptextDef);

            Ptr<ButtonDef> pbuttonDef = *SF_HEAP_NEW(pHeap) ButtonDef();
            pbuttonDef->InitEmptyButtonDef();
            AddResource(ResourceId(CharacterDef::CharId_EmptyButton), pbuttonDef);

            Ptr<SwfShapeCharacterDef> pshapeDef = *SF_HEAP_NEW(pHeap) SwfShapeCharacterDef(
                Ptr<ConstShapeNoStyles>(*SF_HEAP_NEW(pHeap) ConstShapeNoStyles(ConstShapeNoStyles::Empty_Shape)));
            AddResource(ResourceId(CharacterDef::CharId_EmptyShape), pshapeDef);

#ifdef GFX_ENABLE_VIDEO
            Ptr<Video::VideoCharacterDef> pvideoDef = *SF_HEAP_NEW(pHeap) Video::VideoCharacterDef();
            pvideoDef->InitEmptyDef();
            AddResource(ResourceId(CharacterDef::CharId_EmptyVideo), pvideoDef);
#endif
        }

        MovieDataDef::LoadTaskData::~LoadTaskData()
        {
            // Wait for the lock in case if anther object is still accessing this
            ResourceLocker lock(this);
            // Destroy frame data.   
            // Should not need to lock playlist here since when destroying MovieDef /
            // LoadTaskData loading must have completed and playback is finished.    
            unsigned i;
            for (i = 0; i < Playlist.GetSize(); i++)
                Playlist[i].DestroyTags();
            for (i = 0; i < InitActionList.GetSize(); i++)
                InitActionList[i].DestroyTags();

            if (pPathAllocator)
                delete pPathAllocator;

            if (pMetadata)
            {
                SF_HEAP_FREE(pHeap, pMetadata);
                pMetadata = 0;
            }

#ifdef GFX_ENABLE_SOUND
            if (pSoundStream)
                pSoundStream->Release();
#endif // SF_NO_SOUND

        }

        // Notifies LoadTaskData that MovieDataDef is being destroyed. This may be a
        // premature destruction if we are in the process of loading (in that case it
        // will lead to loading being canceled).
        void    MovieDataDef::LoadTaskData::OnMovieDataDefRelease()
        {
            if (LoadState <= LS_LoadingFrames)
                LoadingCanceled = true;
        }


        // Initialize an empty movie definition.
        void    MovieDataDef::LoadTaskData::InitEmptyMovieDef()
        {
            SF_ASSERT(LoadState == LS_Uninitialized);

            // FrameCount of 1 is the default for an empty clip.
            SF_ASSERT(Header.FrameCount == 1);

            {   // Not critical if MovieDataDef is never used for playback while
                // InitEmptyMovieDef hasn't been called; but lock for consistency.
                Lock::Locker lock(&PlaylistLock);
                Playlist.Resize(Header.FrameCount);
                InitActionList.Resize(Header.FrameCount);
                InitActionsCnt = 0;
            }

            UpdateLoadState(Header.FrameCount, LS_LoadFinished);
        }

        // Create a definition describing an image file.
        bool    MovieDataDef::LoadTaskData::InitImageFileMovieDef(
            unsigned fileLength, ImageResource* pimageResource, ImageCreator* imgCreator,
            Log* plog, bool bilinear)
        {
            SF_ASSERT(LoadState == LS_Uninitialized);

            // Original FrameCount of 1 is critical for proper image load waiting.
            SF_ASSERT(Header.FrameCount == 1);
            Header.FileLength = fileLength;

            // Create image resource and shape character using it.    
            if (imgCreator && pimageResource->GetImage()->GetImageType() == Render::ImageBase::Type_ImageBase)
            {
                ImageCreateInfo icreateInfo(ImageCreateInfo::Create_FileImage, Memory::GetHeapByAddress(pimageResource));

                Ptr<Render::Image> img = *imgCreator->CreateImage(icreateInfo, static_cast<ImageSource*>(pimageResource->GetImage()));
                if (img)
                {
                    Ptr<ImageResource> pch = *SF_HEAP_NEW(icreateInfo.pHeap) ImageResource(img.GetPtr());
                    AddResource(ResourceId(CharId_ImageMovieDef_ImageResource), pch);
                    pimageResource = pch;
                }
                else
                {
                    pimageResource = NULL;
                    if (plog)
                        plog->LogError("Can't create or decode image.");
                }
            }
            else
                AddResource(ResourceId(CharId_ImageMovieDef_ImageResource), pimageResource);

            if (pimageResource) //@REVISE: this code is not necessary for AS3 SWF files
            {
                Ptr<ImageShapeCharacterDef> shapeDef = *SF_HEAP_NEW(pHeap) ImageShapeCharacterDef(pimageResource, imgCreator, bilinear);
                AddCharacter(ResourceId(CharId_ImageMovieDef_ShapeDef), shapeDef);

                // Configure us to have one frame - same as empty initialization.
                {
                    Lock::Locker lock(&PlaylistLock);
                    Playlist.Resize(Header.FrameCount);
                    InitActionList.Resize(Header.FrameCount);
                    InitActionsCnt = 0;
                    SF_ASSERT(Playlist.GetSize() == 1);

                    // Add a PlaceObject command, so that it creates a shape.
                    // We use an individual allocator since we don't want a block of memory to be wasted.

                    // ptag needs to be 16-byte aligned for SIMD, so pad the header
                    static const size_t tagAlign = 16;
                    static const size_t tagSize = sizeof(ExecuteTag*);
                    static const size_t headerSize = ((tagSize + tagAlign - 1) / tagAlign) * tagAlign;
                    void* ptagMem = TagMemAllocator.AllocIndividualAlign(
                        sizeof(GFxPlaceObjectUnpacked) + headerSize, tagAlign);
                    if (ptagMem)
                    {
                        ExecuteTag** pptagArray = (ExecuteTag**)ptagMem;
                        GFxPlaceObjectUnpacked* ptag = (GFxPlaceObjectUnpacked*)(pptagArray + headerSize / tagSize);
                        Construct<GFxPlaceObjectUnpacked>(ptag);

                        // Verified: Flash assigns depth -16383 to image shapes (depth value = 1 in our list).
                        int             depth = 1;
                        CharPosInfo  pos(ResourceId(CharacterDef::CharId_ImageMovieDef_ShapeDef),
                            depth, 0, Cxform(), 1, Matrix2F());
                        ptag->InitializeToAdd(pos);

                        // Add playlist frame.
                        SF_ASSERT(Playlist.GetSize() == 1);
                        pptagArray[0] = ptag;
                        Playlist[0].pTagPtrList = pptagArray;
                        Playlist[0].TagCount = 1;
                    }
                    else
                    {
                        // Not enough memory for tags.
                        SF_ASSERT(0);
                    }
                }
            }

            // Notify of load completion.
            UpdateLoadState(Header.FrameCount, LS_LoadFinished);

            return !!pimageResource;
        }

        // Return heap used for image data of this movie; creates the heap if it doesn't exist.
        MemoryHeap* MovieDataDef::LoadTaskData::GetImageHeap()
        {
            if (!pImageHeap)
            {
                MemoryHeap::HeapDesc desc;

                desc.Flags |= MemoryHeap::Heap_FixedGranularity;
                desc.MinAlign = 32;
                desc.Granularity = 4 * 1024;
                desc.Reserve = 0;
                desc.Threshold = ~UPInt(0);
                desc.Limit = 0;
                desc.HeapId = HeapId_Images;

                pImageHeap = *pHeap->CreateHeap("_Images", desc);
            }
            return pImageHeap.GetPtr();
        }

        // Initializes SWF/GFX header for loading.
        void    MovieDataDef::LoadTaskData::BeginSWFLoading(const MovieHeaderData& header)
        {
            Header = header;
            UpdateLoadState(LoadingFrame, LS_LoadingFrames);
        }


        // Read a .SWF pMovie.
        void    MovieDataDef::LoadTaskData::Read(LoadProcess* plp, MovieBindProcess* pbp)
        {
            SF_ASSERT(LoadState == LS_LoadingFrames);

            // Reference stream for easy access.
            Stream& stream = *plp->GetStream();

            // Create list and process tags
            {
                Lock::Locker lock(&PlaylistLock);
                Playlist.Resize(Header.FrameCount);
                InitActionList.Resize(Header.FrameCount);
            }

            stream.LogParseClass(Header.FrameRect);
            stream.LogParse("Note: SWF Frame Rate = %f, Frames = %d\n",
                Header.FPS, Header.FrameCount);

            // Initialize LoadProcess lists(?)
            //  -> they should be already empty, since each process is only used once
            TagCount = 0;

            SF_AMP_CODE(plp->SetSwdHandle(AmpServer::GetInstance().GetNextSwdHandle());)

                // Accumulate tag bytes to know when to send notify to binding thread.
                // We compute update increment based on total file size so that we
                // don't get too few (bad progress) or too many notifications (slow
                // due to expensive thread switching on OS).
                int  tagLoadedBytes = 0;
            bool notifyNeeded = false;
            int  loadUpdateIncrement = (Header.FileLength / 30);
            if (loadUpdateIncrement < 1024 * 8)
                loadUpdateIncrement = 1024 * 8;

            const SWFProcessInfo& pi = plp->GetProcessInfo();

            while ((UInt32)stream.Tell() < pi.FileEndPos)
            {
                // Cancel loading if requested externally.
                if (LoadingCanceled)
                {
                    // There may be frame tags left; clean them up.
                    plp->CleanupFrameTags();
                    if (pbp)
                        pbp->SetBindState(MovieDefImpl::BS_Canceled);
                    UpdateLoadState(LoadingFrame, LS_LoadCanceled);
                    return;
                }

                TagInfo tagInfo;
                TagType tagType = stream.OpenTag(&tagInfo);

#ifdef SF_DEBUG_COUNT_TAGS
                plp->CountTag(tagType);
#endif

                tagLoadedBytes += tagInfo.TagLength;

                // Report previous progress to sleeping/waiting binding threads
                // once a while (more efficient then doing it every frame).
                // Update waiters if a lot of data was loaded or if the next
                // tag is big (i.e. could cause slow load).
#ifdef SF_ENABLE_THREADS
                if (notifyNeeded &&
                    ((LoadingFrame == 1) ||
                        (tagLoadedBytes > loadUpdateIncrement) ||
                        (tagInfo.TagLength > 1024 * 8)))
                {
                    pFrameUpdate->UpdateNotify();
                    notifyNeeded = false;
                    tagLoadedBytes = 0;
                }
#endif

                plp->ReportProgress(FileURL, tagInfo, false);

                LoaderImpl::LoaderFunction   lf = NULL;
                //in->LogParse("tagType = %d\n", tagType);

                if (tagType == Tag_EndFrame)
                {
                    // We handle EndFrame logic after the tag is closed below.
                    // That is helpful to ensure that reported bytesLoaded position is correct.
                }
                else if (LoaderImpl::GetTagLoader(tagType, &lf))
                {
                    /* Tag decoding helper code
                    //  if ((tagType == 83) || (tagType == 2) || (tagType == 32) ||
                    // Morph
                    //      (tagType == 46) || (tagType == 84) )
                    {
                    int start = stream.Tell();
                    stream.LogParse("*** Tag data for tag %d:\n", tagType);
                    stream.LogTagBytes();
                    stream.SetPosition(start);
                    }
                    */

                    // call the tag loader.  The tag loader should add
                    // characters or tags to the GFxMovieSub data structure.
                    (*lf)(plp, tagInfo);
                }
                else
                {
                    // no tag loader for this tag type.
                    stream.LogParse("*** no tag loader for type %d\n", tagType);
                    stream.LogTagBytes();
                }

                stream.CloseTag();
                TagCount++;

                // Handle EndFrame after it's tag is closed.
                if (tagType == Tag_EndFrame)
                {
                    // Post bound frame and update count.
                    if (!FinishLoadingFrame(plp, false))
                    {
                        // Error occurred; LS_LoadError should already be set internally.
                        return;
                    }

                    notifyNeeded = true;

                    // Show frame tag -- advance to the next frame.
                    stream.LogParse("  ShowFrame\n");

                    // Transfer loaded resources to frame
                    // If binding was requested, do binding.
                    if (pbp)
                    {
                        pbp->BindNextFrame();
                        // Do we need to do anything about bind errors? Technically
                        // they will be sticky in the bind process.
                    }
                }

                else if (tagType == 0)
                {
                    if ((unsigned)stream.Tell() != pi.FileEndPos)
                    {
                        // Safety break, so we don't read past the end of the pMovie.               
                        stream.LogWarning("Loader - Stream-end tag hit, but not at the "
                            "end of the file yet; stopping for safety");
                        break;
                    }
                }
            }

            // TBD: Shouldn't we verify the number of frames loaded
            // and set a different state otherwise?

            if (plp->FrameTagsAvailable())
            {
                FinishLoadingFrame(plp, true);

                if (pbp)
                    pbp->BindNextFrame();
                // Do we need to do anything about bind errors? Technically
                // they will be sticky in the bind process.
            }
            else
            {
                // Change state to LS_LoadFinished with notification.
                UpdateLoadState(LoadingFrame, LS_LoadFinished);
            }
        }


        // Updates bind data and increments LoadingFrame. 
        // It is expected that it will also release any threads waiting on it in the future.
        bool    MovieDataDef::LoadTaskData::FinishLoadingFrame(LoadProcess* plp, bool finished)
        {
            plp->CommitFrameTags();

            // TBD: We could record a pending error status in LoadProcess
            // and check it here...
            bool    success = true;

            FrameBindData* pfdata = plp->CreateFrameBindData();
            if (pfdata)
            {
                // Record the number of resources in frame so that they can be bound.
                pfdata->Frame = LoadingFrame;
                pfdata->BytesLoaded = plp->GetStream()->Tell() -
                    plp->GetProcessInfo().FileStartPos;
            }

#ifdef SF_ENABLE_THREADS
            Mutex::Locker lock(&pFrameUpdate->GetMutex());
#endif

            if (pfdata)
            {
                // Update the front if necessary and insert item at the end.
                if (!BindData.pFrameData)
                    BindData.pFrameData = pfdata;
                else
                    BindData.pFrameDataLast->pNextFrame = pfdata;
                BindData.pFrameDataLast = pfdata;

                LoadingFrame++;
                if (finished)
                    LoadState = LS_LoadFinished;
            }
            else
            {
                LoadState = LS_LoadError;
                success = false;
            }

            // Notify bind thread(s) that file has finished loading.
            // More detailed per-frame data notifications take place in the main
            // loop but only once substantial enough chunk is read.
#ifdef SF_ENABLE_THREADS
    //printf("Next frame is ready %d\n", LoadingFrame);
            if (finished || !success)
                pFrameUpdate->UpdateNotify();
#endif   

            return success;
        }

        // Update frame and loading state with proper notification, for use image loading, etc.
        void    MovieDataDef::LoadTaskData::UpdateLoadState(unsigned loadingFrame, MovieLoadState loadState)
        {
#ifdef SF_ENABLE_THREADS
            if (pFrameUpdate)
            {
                Mutex::Locker lock(&pFrameUpdate->GetMutex());
                LoadingFrame = loadingFrame;
                LoadState = loadState;
                pFrameUpdate->UpdateNotify();
            }
            else
            {
                LoadingFrame = loadingFrame;
                LoadState = loadState;
            }
#else
            LoadingFrame = loadingFrame;
            LoadState = loadState;
#endif
        }

        void  MovieDataDef::LoadTaskData::NotifyFrameUpdated()
        {
#ifdef SF_ENABLE_THREADS
            if (pFrameUpdate)
            {
                Mutex::Locker lock(&pFrameUpdate->GetMutex());
                pFrameUpdate->UpdateNotify();
            }
#endif
        }

        // Waits until loading is completed, used by FontLib.
        void MovieDataDef::LoadTaskData::WaitForLoadFinish()
        {
#ifdef SF_ENABLE_THREADS
            if (LoadState <= LS_LoadingFrames)
            {
                pFrameUpdate->WaitForLoadFinished();
                // LoadState must've been modified before LoadFinished flag is set.
                SF_ASSERT(LoadState > LS_LoadingFrames);
            }
#endif
        }

        // Waits until loading of a given frame is completed
        void MovieDataDef::LoadTaskData::WaitForFrame(unsigned frame)
        {
#ifdef SF_ENABLE_THREADS
            if (LoadState <= LS_LoadingFrames && LoadingFrame <= frame)
            {
                Mutex::Locker lock(&pFrameUpdate->GetMutex());
                while (LoadState <= LS_LoadingFrames && LoadingFrame <= frame)
                    pFrameUpdate->WaitForNotify();
            }
#else
            SF_UNUSED(frame);
#endif
        }


        unsigned    MovieDataDef::LoadTaskData::GetMetadata(char* pbuff, unsigned buffSize) const
        {
            if (!pbuff)
                return MetadataSize;
            buffSize = Alg::Min<unsigned>(buffSize, MetadataSize);
            if (pMetadata)
                memcpy(pbuff, pMetadata, buffSize);
            return buffSize;
        }


        // Sets MetaData of desired size.
        void    MovieDataDef::LoadTaskData::SetMetadata(UByte* pdata, unsigned size)
        {
            // Should only set metadata once.
            SF_ASSERT(pMetadata == 0);
            if ((pMetadata = (UByte*)SF_HEAP_ALLOC(pHeap, size, StatMD_Other_Mem)) != 0)
            {
                MetadataSize = size;
                memcpy(pMetadata, pdata, size);
            }
        }

        // Add a resource during loading.
        void    MovieDataDef::LoadTaskData::AddResource(ResourceId rid, Resource* pres)
        {
            ResourceLocker lock(this);
            Resources.Add(rid, ResourceHandle(pres));
        }

        // Creates a new resource handle with a binding index for the resourceId; used 
        // to register ResourceData objects that need to be bound later.
        ResourceHandle   MovieDataDef::LoadTaskData::AddNewResourceHandle(ResourceId rid)
        {
            ResourceHandle rh(ResourceHandle::RH_Index, ResIndexCounter);
            ResIndexCounter++;

            ResourceLocker lock(this);
            Resources.Add(rid, rh);
            return rh;
        }

        bool    MovieDataDef::LoadTaskData::GetResourceHandle(ResourceHandle* phandle, ResourceId rid) const
        {
            ResourceLocker lock(this);

            ResourceHash::ConstIterator ir = Resources.Find(rid);
            if (ir != Resources.End())
            {
                *phandle = ir->Second;
                return 1;
            }
            return 0;
        }



        // Get font data bi ResourceId.
        Font* MovieDataDef::LoadTaskData::GetFontData(ResourceId rid)
        {
            FontDataUseNode* pfonts = GetFirstFont();

            // Thread-safe traversal since only completed fonts are added to
            // list. TBD: We might want to check that loading has completed.
            while (pfonts)
            {
                if (pfonts->Id == rid)
                    return pfonts->pFontData.GetPtr();
                pfonts = pfonts->pNext;
            }
            return 0;
        }



        // Labels the frame currently being loaded with the given name.
        // A copy of the name string is made and kept in this object.    
        void    MovieDataDef::LoadTaskData::AddFrameName(const String& name, LogState* plog)
        {
            SF_UNUSED(plog);
            SF_ASSERT(LoadingFrame < Header.FrameCount);

            Lock::Locker lock(&PlaylistLock);
            NamedFrames.SetCaseInsensitive(name, LoadingFrame);   // stores 0-based frame #
        }


        void    MovieDataDef::LoadTaskData::SetLoadingPlaylistFrame(const Frame& frame, LogState* plog)
        {
            // We should not call SetLoadingPlaylistFrame() for the same frame multiple times.
            Lock::Locker lock(&PlaylistLock);
            if (Playlist.GetSize() <= LoadingFrame)
            {
                if (plog && plog->GetLog())
                    plog->GetLog()->LogError("Invalid SWF file: failed to load frame #%d since total frames counter is %d",
                        int(LoadingFrame + 1), int(Playlist.GetSize()));
                return;
            }
            SF_ASSERT(Playlist[LoadingFrame].TagCount == 0);
            Playlist[LoadingFrame] = frame;
        }

        void    MovieDataDef::LoadTaskData::SetLoadingInitActionFrame(const Frame& frame, LogState* plog)
        {
            Lock::Locker lock(&PlaylistLock);
            if (InitActionList.GetSize() <= LoadingFrame)
            {
                if (plog && plog->GetLog())
                    plog->GetLog()->LogError("Invalid SWF file: failed to load init action frame #%d since total frames counter is %d",
                        int(LoadingFrame + 1), int(InitActionList.GetSize()));
                return;
            }
            SF_ASSERT(InitActionList[LoadingFrame].TagCount == 0);
            InitActionList[LoadingFrame] = frame;
            ++InitActionsCnt;
        }


        // Expose one of our resources under the given symbol,
        // for export.  Other movies can import it.    
        void    MovieDataDef::LoadTaskData::ExportResource(const String& symbol, ResourceId rid, const ResourceHandle& hres)
        {
            ResourceLocker lock(this);
            // SWF sometimes exports the same thing more than once!
            // Export names are case-insensitive.
            Exports.SetCaseInsensitive(symbol, hres);
            SF_ASSERT(Exports.FindCaseInsensitive(symbol) != Exports.End());
            InvExports.Set(rid, Exports.FindCaseInsensitive(symbol)->First);
        }


        // Returns 0-based frame #
        bool    MovieDataDef::LoadTaskData::GetLabeledFrame(
            const char* label, unsigned* frameNumber, bool translateNumbers) const
        {
            if (LoadState >= LS_LoadFinished)
                return TranslateFrameString(NamedFrames, label, frameNumber, translateNumbers);
            // NamedFrames access is synchronized if loading is not finished.
            Lock::Locker lock(&PlaylistLock);
            return TranslateFrameString(NamedFrames, label, frameNumber, translateNumbers);
        }

        const String* MovieDataDef::LoadTaskData::GetFrameLabel(
            unsigned frameNumber, unsigned* exactFrameNumberForLabel) const
        {
            if (LoadState >= LS_LoadFinished)
                return TranslateNumberToFrameString(NamedFrames, frameNumber, exactFrameNumberForLabel);
            // NamedFrames access is synchronized if loading is not finished.
            Lock::Locker lock(&PlaylistLock);
            return TranslateNumberToFrameString(NamedFrames, frameNumber, exactFrameNumberForLabel);
        }

        Array<String>* MovieDataDef::LoadTaskData::GetFrameLabels(
            unsigned frameNumber, Array<String>* destArr) const
        {
            SF_ASSERT(destArr);

            // NamedFrames access is synchronized if loading is not finished.
            bool locked = false;
            if (LoadState < LS_LoadFinished)
            {
                PlaylistLock.DoLock();
                locked = true;
            }
            StringHashLH<unsigned>::ConstIterator it = NamedFrames.Begin();
            int i = 0;
            for (; it != NamedFrames.End(); ++it)
            {
                if (frameNumber == it->Second)
                {
                    destArr->PushBack(it->First);
                    ++i;
                }
            }
            if (locked)
                PlaylistLock.Unlock();
            return (i) ? destArr : NULL;
        }

        void    MovieDataDef::LoadTaskData::AddScene(const String& name, unsigned off)
        {
            if (!Scenes)
                Scenes = SF_HEAP_NEW(pHeap) ArrayLH<SceneInfo>();
            Scenes->PushBack(SceneInfo(pHeap, name, off));
        }

        const  MovieDataDef::SceneInfo* MovieDataDef::LoadTaskData::GetScene(UPInt index) const
        {
            if (Scenes && index < Scenes->GetSize())
                return &(*Scenes)[index];
            return NULL;
        }

        const  MovieDataDef::SceneInfo* MovieDataDef::LoadTaskData::GetScenes(UPInt* count) const
        {
            SF_ASSERT(count);
            if (Scenes)
            {
                *count = Scenes->GetSize();
                return Scenes->GetDataPtr();
            }
            *count = 0;
            return NULL;
        }

        MovieDataDef::SceneInfo* MovieDataDef::LoadTaskData::GetScene(UPInt index)
        {
            return const_cast<SceneInfo*>(const_cast<const LoadTaskData*>(this)->GetScene(index));
        }

        MovieDataDef::SceneInfo* MovieDataDef::LoadTaskData::GetScenes(UPInt* count)
        {
            return const_cast<SceneInfo*>(const_cast<const LoadTaskData*>(this)->GetScenes(count));
        }

        const TimelineDef::Frame   MovieDataDef::LoadTaskData::GetPlaylist(int frameNumber) const
        {
            if (LoadState >= LS_LoadFinished)
                return Playlist[frameNumber];

            // Only lock access if we are still loading.
            // This lock shouldn't be a big problem because most playlists
            // will come from GFxSprite where they don't need to be locked.
            // However, we could still get a bottleneck here due to FindPreviousPlaceObject2.
            // If this happens, we might have to come up with a buffered multi-frame copy solution.
            Lock::Locker lock(&PlaylistLock);
            return Playlist[frameNumber];
        }

        bool    MovieDataDef::LoadTaskData::GetInitActions(Frame* pframe, int frameNumber) const
        {
            Lock::Locker lock(&PlaylistLock);

            if (((unsigned)frameNumber) >= InitActionList.GetSize())
                return false;
            *pframe = InitActionList[frameNumber];
            return true;
        }

        unsigned   MovieDataDef::LoadTaskData::GetInitActionListSize() const
        {
            Lock::Locker lock(&PlaylistLock);
            return (unsigned)InitActionList.GetSize();
        }


#ifdef GFX_ENABLE_SOUND
        SoundStreamDef* MovieDataDef::LoadTaskData::GetSoundStream() const
        {
            return pSoundStream;
        }
        void MovieDataDef::LoadTaskData::SetSoundStream(SoundStreamDef* psoundStream)
        {
            if (pSoundStream)
                pSoundStream->Release();
            if (psoundStream)
                psoundStream->AddRef();
            pSoundStream = psoundStream;
        }
#endif

        // *** Creating MovieDefData file keys


        // MovieDataDef key: {FileName, pFileOpener, optional pImageCreator}
        class GFxMovieDataDefFileKeyData : public RefCountBase<GFxMovieDataDefFileKeyData, Stat_Default_Mem>
        {
            friend class GFxMovieDataDefFileKeyInterface;

            String          FileName;
            Ptr<FileOpener> pFileOpener;
            SInt64          ModifyTime;

            // pImageCreator is unique in that it is only used for MovieDataDef binding
            // if ImageCreator::IsKeepingImageData returns false and Loader::LoadKeepBindData
            // flag is not specified. If one of those flags are set, ImageCreator is only
            // used as an argument for MovieDefImpl and not MovieDataDef. In that case,
            // its value here will be null and image resources are not created until binding time.
            Ptr<ImageCreator>       pImageCreator;

        public:

            GFxMovieDataDefFileKeyData(const char* pfilename, SInt64 modifyTime,
                FileOpener* pfileOpener, ImageCreator* pimageCreator)
            {
                FileName = pfilename;
                ModifyTime = modifyTime;
                pFileOpener = pfileOpener;
                pImageCreator = pimageCreator;
            }

            bool operator == (GFxMovieDataDefFileKeyData& other) const
            {
                return  pFileOpener == other.pFileOpener &&
                    pImageCreator == other.pImageCreator &&
                    ModifyTime == other.ModifyTime &&
                    FileName == other.FileName;
            }
            bool operator != (GFxMovieDataDefFileKeyData& other) const
            {
                return !operator == (other);
            }

            UPInt  GetHashCode() const
            {
                UPInt fileHashCode = String::BernsteinHashFunction(FileName.ToCStr(),
                    FileName.GetSize());
                return fileHashCode ^
                    ((UPInt)ModifyTime) ^
                    ((UPInt)pFileOpener.GetPtr()) ^ (((UPInt)pFileOpener.GetPtr()) >> 7) ^
                    ((UPInt)pImageCreator.GetPtr()) ^ (((UPInt)pImageCreator.GetPtr()) >> 7);
            }
        };


        class GFxMovieDataDefFileKeyInterface : public ResourceKey::KeyInterface
        {
        public:
            typedef ResourceKey::KeyHandle KeyHandle;

            virtual void    AddRef(KeyHandle hdata)
            {
                SF_ASSERT(hdata); ((GFxMovieDataDefFileKeyData*)hdata)->AddRef();
            }
            virtual void    Release(KeyHandle hdata)
            {
                SF_ASSERT(hdata); ((GFxMovieDataDefFileKeyData*)hdata)->Release();
            }

            // Key/Hash code implementation.
            virtual ResourceKey::KeyType GetKeyType(KeyHandle hdata) const
            {
                SF_UNUSED(hdata);
                return ResourceKey::Key_File;
            }

            virtual UPInt  GetHashCode(KeyHandle hdata) const
            {
                SF_ASSERT(hdata);
                return ((GFxMovieDataDefFileKeyData*)hdata)->GetHashCode();
            }

            virtual bool    KeyEquals(KeyHandle hdata, const ResourceKey& other)
            {
                if (this != other.GetKeyInterface())
                    return 0;
                return *((GFxMovieDataDefFileKeyData*)hdata) ==
                    *((GFxMovieDataDefFileKeyData*)other.GetKeyData());
            }

            const char* GetFileURL(KeyHandle hdata) const
            {
                GFxMovieDataDefFileKeyData* pdata = (GFxMovieDataDefFileKeyData*)hdata;
                SF_ASSERT(pdata);
                return pdata->FileName.ToCStr();
            }
        };


        static GFxMovieDataDefFileKeyInterface GFxMovieDataDefFileKeyInterface_Instance;

        // Create a key for an SWF file corresponding to MovieDef.
        Scaleform::GFx::ResourceKey MovieDataDef::CreateMovieFileKey(const char* pfilename, SInt64 modifyTime, FileOpener* pfileOpener, ImageCreator* pimageCreator)
        {
            Ptr<GFxMovieDataDefFileKeyData> pdata =
                *SF_NEW GFxMovieDataDefFileKeyData(pfilename, modifyTime, pfileOpener,
                    pimageCreator);

            return ResourceKey(&GFxMovieDataDefFileKeyInterface_Instance,
                (ResourceKey::KeyHandle)pdata.GetPtr());
        }

        String MovieDataDef::GetResourceName() const
        {
            //return String("MovieData \"", GetShortFilename(GetFileURL()), "\"");
            return String(GetShortFilename(GetFileURL()));
        }



        //
        // ***** MovieDefImpl
        //

        MovieDefImpl::MovieDefImpl(MovieDataDef* pdataDef,
            MovieDefBindStates* pstates,
            LoaderImpl* ploaderImpl,
            unsigned loadConstantFlags,
            StateBagImpl* pdelegateState,
            MemoryHeap* pargHeap,
            bool fullyLoaded,
            UPInt memoryArena)
        {
            //printf("MovieDefImpl::MovieDefImpl: %x, thread : %d\n", this, GetCurrentThreadId());
            MemoryHeap* pheap = pargHeap;
            if (!pheap)
            {
                // Heap name above includes extra space to align with "Movie Data".
                String heapName("MovieDef  \"", GetShortFilename(pdataDef->GetFileURL()), "\"");
                unsigned    heapFlags = (loadConstantFlags & Loader::LoadDebugHeap) ?
                    MemoryHeap::Heap_UserDebug : 0;
                // Binding heap gets small granularity since its initially empty.
                MemoryHeap::HeapDesc desc(heapFlags, 16, 4 * 1024, 4 * 1024);
                desc.HeapId = HeapId_MovieDef;
                desc.Arena = memoryArena;
                pheap = Memory::GetGlobalHeap()->CreateHeap(heapName.ToCStr(), desc);
            }


            pBindData = *SF_HEAP_NEW(pheap) BindTaskData(pheap, pdataDef, this,
                loadConstantFlags, fullyLoaded);
            if (!pargHeap)
                pheap->ReleaseOnFree(pBindData.GetPtr());

            pLoaderImpl = ploaderImpl;

            // We MUST have states and DataDef
            SF_ASSERT(pstates);
            pBindStates = pstates;

            // Create a delegated shared state.
            pStateBag = *SF_NEW StateBagImpl(pdelegateState);
        }

        MovieDefImpl::~MovieDefImpl()
        {
            SF_DEBUG_MESSAGE2(GFX_UNLOAD_TRACE_ENABLED,
                "~MovieDefImpl %s (%p)\n", GetDataDef()->GetFileURL(), this);
            {
                Lock::Locker lock(&ReleaseNotifiersLock);
                HashSetLH<ReleaseNotifier*>::Iterator it = ReleaseNotifiers.Begin();
                for (; it != ReleaseNotifiers.End(); ++it)
                {
                    if (*it)
                        (*it)->OnMovieDefRelease(this);
                }
            }
            pBindData->OnMovieDefRelease();
        }

        void MovieDefImpl::AddReleaseNotifier(ReleaseNotifier* rn)
        {
            Lock::Locker lock(&ReleaseNotifiersLock);
            ReleaseNotifiers.Set(rn);
        }

        void MovieDefImpl::RemoveReleaseNotifier(ReleaseNotifier* rn)
        {
            Lock::Locker lock(&ReleaseNotifiersLock);
            ReleaseNotifiers.Remove(rn);
        }


        // *** GFxMovieDefBindProcess - used for MovieDefImpl Binding

        MovieBindProcess::MovieBindProcess(LoadStates* pls,
            MovieDefImpl* pdefImpl,
            LoaderImpl::LoadStackItem* ploadStack)
            : LoaderTask(pls, Id_MovieBind),
            pFrameBindData(0), GlyphTextureIdGen(ResourceId::IdType_DynFontImage),
            /*pLoadStates(pls), */pBindData(pdefImpl->pBindData), pLoadStack(ploadStack)
        {
            // RefCountMode is already thread safe for GFxTask.

            pDataDef = pBindData->GetDataDef();
            Stripped = ((pDataDef->GetSWFFlags() & MovieInfo::SWF_Stripped) != 0);

            ImagePackParamsBase* pimagePacker = pls->GetBindStates()->pImagePackParams;
            if (pimagePacker && !Stripped)
            {
                //ImageCreateInfo     icreateInfo(NULL, Resource::Use_Bitmap);
                ImageCreateExportInfo     icreateInfo;
                icreateInfo.SetStates(pls->GetLog(), pls->GetFileOpener(), pls->GetImageFileHandlerRegistry());
                //icreateInfo.ThreadedLoading = pls->IsThreadedLoading();
                icreateInfo.pHeap = pdefImpl->GetBindDataHeap();

                pImagePacker = *pimagePacker->Begin(&GlyphTextureIdGen, pls->GetBindStates()->pImageCreator, &icreateInfo);
                pImagePacker->SetBindData(pBindData);

                pTempBindData = SF_NEW TempBindData;
            }
            else
                pTempBindData = 0;
        }

        MovieBindProcess::~MovieBindProcess()
        {
#ifdef SF_ENABLE_THREADS
            Ptr<LoadUpdateSync> ploadSync = pBindData ? pBindData->GetBindUpdateSync() : 0;
#endif

            if (pBindData)
            {
                if (pBindData->GetBindStateType() == MovieDefImpl::BS_InProgress)
                {
                    pBindData->SetBindState(MovieDefImpl::BS_Canceled);
                }
                pBindData = 0;
            }

            if (pTempBindData)
                delete pTempBindData;
            pImagePacker = 0;

#ifdef SF_ENABLE_THREADS
            // Notify any waiters that our task thread has released
            // all of its references to loaded data.
            if (ploadSync)
                ploadSync->NotifyLoadFinished();
#endif
        }

        // matches filenames in two paths, return 'true' if filenames match.
        static bool MatchFileNames(const String& path1, const String& path2)
        {
            int i1 = (int)path1.GetSize() - 1;
            int i2 = (int)path2.GetSize() - 1;

            if ((i1 < 0 || i2 < 0) && i1 != i2)
                return false;

            for (; i1 >= 0 && i2 >= 0; --i1, --i2)
            {
                if ((path1[i1] == '\\' || path1[i1] == '/') && (path2[i2] == '\\' || path2[i2] == '/'))
                    return true;
                if (SFtolower(path1[i1]) != SFtolower(path2[i2]))
                    return false;
            }
            if (i1 == i2)
                return true;
            if (i2 < i1)
                return (path1[i1] == '\\' || path1[i1] == '/');
            else
                return (path2[i2] == '\\' || path2[i2] == '/');
        }

        MovieBindProcess::BindStateType MovieBindProcess::BindNextFrame()
        {
            // For some reason this check seems to be required, as we
            // may com in here with null pointer after cancel. This check
            // prevents GetBindStateType() from crashing.
            if (!pBindData)
                return MovieDefImpl::BS_Canceled;

            // Check binding state and return correct value if appropriate.
            BindStateType startBindStateType = GetBindStateType();

            if (startBindStateType != MovieDefImpl::BS_InProgress)
            {
                if (startBindStateType == MovieDefImpl::BS_NotStarted)
                {
                    SetBindState(MovieDefImpl::BS_InProgress | GetBindStateFlags());
                    // Binding can only take place once.
                    SF_ASSERT(pBindData->BindingFrame == 0);
                }
                else
                {
                    return startBindStateType;
                }
            }


            // Get the next frame data if possible, waiting for it if necessary.
            // The nature of frames linked list allows us to avoid mutex locks
            // on every access. This is ok as long as updates happen within the
            // lock on the producer (DataDef) thread side.
            FrameBindData* pframeData = GetNextFrameBindData();

#ifdef SF_ENABLE_THREADS
            if (!pframeData)
            {
                MovieDataDef::LoadTaskData* ploadData = pDataDef->pData;

                Mutex::Locker lock(&ploadData->pFrameUpdate->GetMutex());
                pframeData = GetNextFrameBindData();
                // Wait until next frame data is available or loading fails.
                //printf("Waiting for the next frame: %p\n", this);
                while ((pframeData == 0) &&
                    (ploadData->LoadState == MovieDataDef::LS_LoadingFrames) &&
                    !pBindData->BindingCanceled)
                {
                    ploadData->pFrameUpdate->WaitForNotify();
                    pframeData = GetNextFrameBindData();
                };
                //printf("Done waiting for the next frame: %p\n", this);
                if (ploadData->LoadState == MovieDataDef::LS_LoadCanceled)
                    pBindData->BindingCanceled = true;
            }
#endif

            if (!pframeData || pBindData->BindingCanceled)
            {
                // Error occurred in load, turn it into error in processing.
                FinishBinding();
                BindStateType bindState = pBindData->BindingCanceled ?
                    MovieDefImpl::BS_Canceled : MovieDefImpl::BS_Error;
                SetBindState(bindState | GetBindStateFlags());
                // If binding was canceled, release pBindData so that the load process
                // can terminate as well (necessary if we are on the same thread).
#ifdef SF_ENABLE_THREADS
                Ptr<LoadUpdateSync> psync = pBindData->GetBindUpdateSync();
                pBindData = 0;
                psync->NotifyLoadFinished();
#else
                pBindData = 0;
#endif
                return bindState;
            }

            // At this point we must be processing binding frame (unless we
            // allow empty frame entries in the future). Not that allowing
            // such entries would cause problem with waits & BindingFrame
            // updates, unless done intelligently.
            SF_ASSERT(pBindData->BindingFrame == pframeData->Frame);
            pFrameBindData = pframeData;


            // Perform the main tasks of binding:
            //  1. Resolve all imports and store imported movie references
            //     in ImportSourceMovies.
            //  2. Populate the ResourceBinding table to resolve Handles by using
            //     the resource binding data.
            //  3. Process/pack fonts included in this frame.
            //  4. Update frame data and bytes.


            LoadStates* pls = pLoadStates;
            unsigned       loadFlags = pBindData->LoadFlags;
            bool           fontImportsSucceeded = true;

            // Resolve all imports for this frame.
            if (!(loadFlags & Loader::LoadDisableImports))
            {
                unsigned        iimport;
                ImportData* pimportData = pframeData->pImportData;

                // Grab temporary reference to MovieDefImpl to access pSharedSate
                Ptr<FontLib> pfontLib;
                Ptr<MovieDefImpl> pdefImpl = *pBindData->GetMovieDefImplAddRef();
                if (pdefImpl)
                {
                    // Check if FontLib is installed. if it is not or import name is not "$IMECandidateListFont" 
                    // we will treat 'gfxfontlib.swf' file like a usual import file.
                    pfontLib = pdefImpl->GetFontLib();
                }

                // Iterate imports for this frame. We must limit the number of
                // imports iterated and not go till the end of the list because
                // the list contains ALL imports in the file.
                for (iimport = 0; iimport < pframeData->ImportCount;
                    iimport++, pimportData = pimportData->pNext)
                {
                    // These must hold at this point.
                    SF_ASSERT(pimportData != 0);
                    SF_ASSERT(pimportData->Frame <= pBindData->BindingFrame);

                    const String& sourceURL = pimportData->SourceUrl;

                    // Check if FontLib is installed. if it is not or import name is not "$IMECandidateListFont" 
                    // we will treat 'gfxfontlib.swf' file like a usual import file.
                    if (
#ifndef SF_NO_IME_SUPPORT
                    (pimportData->Imports.GetSize() > 0 && pimportData->Imports[0].SymbolName == "$IMECandidateListFont") ||
#endif
                        // See if the source URL ends in 'gfxfontlib.swf', in which
                        // case we do import font substitution instead.
                        (pfontLib && (//MatchFileNames(sourceURL, pfontLib->GetSubstitute()) || 
                            (pdefImpl && MatchFileNames(sourceURL, pdefImpl->GetDefaultFontLibName())) ||
                            MatchFileNames(sourceURL, String("gfxfontlib.swf")))))
                    {
                        // This if a substituted font import, handle it in a custom manner.
                        if (!pBindData->ResolveImportThroughFontLib(pimportData))
                            fontImportsSucceeded = 0;
                        // TO DO: Font import substitution could cause font resource to be assigned 
                        // to resource data slots of different type (import, etc.) in pathological
                        // cases. We should check for that in the future perhaps by verifying the
                        // type during binding load time...    
                        continue;
                    }

                    MovieDefImpl* pdef = 0;

                    // LoadStates are separate in import because import has a separate MovieDef.
                    Ptr<LoadStates> pimportLoadStates = *pls->CloneForImport();
                    // Load flags must be the same, but we wait for load completion with imports.
                    unsigned            importLoadFlags = loadFlags | Loader::LoadWaitCompletion;

                    // Create import stack. We need it for detecting recursion int loading process.
                    // It is safe to create the item on stack because we set LoadWaitCompletion 
                    // load state and call to CreateMovie_LoadState will not return until the whole movie
                    // with all its dependencies if any is loaded.
                    LoaderImpl::LoadStackItem loadStack(pBindData->pDefImpl_Unsafe);
                    if (!pLoadStack)
                        pLoadStack = &loadStack;
                    else
                    {
                        LoaderImpl::LoadStackItem* ptail = pLoadStack;
                        while (ptail->pNext)
                            ptail = ptail->pNext;
                        ptail->pNext = &loadStack;
                    }

                    // Try both SWF and GFX files.
                    // If the file is stripped, its imports may be stripped too,
                    // so try to load '.gfx' import file first.

                    // We don't use GetLength() because ".swf" is represented as 4 bytes in UTF8
                    if (Stripped && (sourceURL.GetSize() > 4) &&
                        (String::CompareNoCase(sourceURL.ToCStr() +
                            (sourceURL.GetSize() - 4), ".swf") == 0))
                    {
                        URLBuilder::LocationInfo loc(URLBuilder::File_Import,
                            sourceURL, pls->GetRelativePath());
                        loc.FileName.Clear();
                        loc.FileName.AppendString(sourceURL.ToCStr(), sourceURL.GetSize() - 4);
                        loc.FileName += ".gfx";

                        // Use our captured load state to load the imported movie. This
                        // guarantees that the states are consistent.
                        pdef = LoaderImpl::CreateMovie_LoadState(pimportLoadStates,
                            loc, importLoadFlags, pLoadStack);
                    }

                    if (!pdef)
                    {
                        URLBuilder::LocationInfo loc(URLBuilder::File_Import,
                            sourceURL, pls->GetRelativePath());
                        pdef = LoaderImpl::CreateMovie_LoadState(pimportLoadStates,
                            loc, importLoadFlags, pLoadStack);
                    }

                    // Remove stack item from the stack
                    if (pLoadStack == &loadStack)
                        pLoadStack = NULL;
                    else
                    {
                        LoaderImpl::LoadStackItem* ptail = pLoadStack;
                        while (ptail->pNext)
                        {
                            if (ptail->pNext == &loadStack)
                            {
                                ptail->pNext = ptail->pNext->pNext;
                                break;
                            }
                            ptail = ptail->pNext;
                        }
                        SF_ASSERT(!ptail->pNext);
                    }

                    // check if ActionScript version matches between the files.
                    // Currently we do not support importing from SWF file with different
                    // AS version than the main file.
                    if (pdef &&
                        ((pdef->GetFileAttributes() & MovieDef::FileAttr_UseActionScript3) !=
                            (pDataDef->GetFileAttributes() & MovieDef::FileAttr_UseActionScript3)))
                    {
                        pdef->Release();
                        pdef = NULL;
                        if (pls->GetLogState())
                            pls->GetLogState()->LogError("ActionScript version mismatched between main and import '%s' files",
                                sourceURL.ToCStr());
                    }

                    if (!pdef)
                    {
                        // Failed loading; leave us at correct bind frame.
                        FinishBinding();
                        SetBindState(MovieDefImpl::BS_Error | GetBindStateFlags());
                        return MovieDefImpl::BS_Error;
                    }
                    else
                    {
                        // It is possible for SWF files to import themselves, which Flash
                        // seems to handle gracefully. What happens with indirect recursiveness?
                        // TBD: For now we the pass the recursive flag to avoid leaks,
                        // but we will need to investigate this in detail in the future.
                        bool recursive = (pdef->pBindData == pBindData);
                        if (recursive && pls->GetLogState())
                            pls->GetLogState()->LogWarning("Self recursive import detected in '%s'",
                                sourceURL.ToCStr());

                        pBindData->ResolveImport(pimportData, pdef, pls, recursive);
                        // Invoke default import visitor to notify it of resolve.
                        ImportVisitor* pimportVistor = pls->GetBindStates()->pImportVisitor;
                        if (pimportVistor)
                        {
                            // Do temporary locked grab of our pDefImpl pointer.
                            Ptr<MovieDefImpl> pMovieDefImpl = *pBindData->GetMovieDefImplAddRef();
                            if (pMovieDefImpl)
                                pimportVistor->Visit(pMovieDefImpl, pdef, sourceURL.ToCStr());
                        }

                        pdef->Release();
                    }
                } // for (iimport...)
            }



            // Create & Resolve all resources in the frame.
            ResourceDataNode* presData = pframeData->pResourceData;
            unsigned            ires;
            ResourceBinding& resourceBinding = pBindData->ResourceBinding;
            MemoryHeap* pheap = pBindData->GetBindDataHeap();


            for (ires = 0; ires < pframeData->ResourceCount; ires++, presData = presData->pNext)
            {
                SF_ASSERT(presData != 0);

                if (!presData->Data.IsValid())
                {
                    // We can get here for several reasons:
                    //  1) This presData->BindIndex corresponds to an imported resource that failed
                    //     to load. In that case we would've already emitted a warning inside of ResolveImport.
                    //  2) An image loading JPEG/ZLib Support was not installed causing image
                    //     resource data to be null.
                    // For now, leave the slot as null.

                    // MA NOTE: There used to be a non-null import slot with ASSERT here that was triggered
                    // only if fontImportsSucceeded was true, but is seems to serve no purpose.
                    SF_UNUSED(fontImportsSucceeded);
                }
                else
                {
                    // Create a resource based on its loaded data in DataDef.
                    ResourceBindData bd;
                    bd.pBinding = &resourceBinding;
                    bd.pResource = 0;

                    if (!pBindData->BindingCanceled && presData->Data.CreateResource(&bd, pls, pheap))
                    {
                        if (pImagePacker && bd.pResource->GetResourceType() == Resource::RT_Image)
                        {//&&
                            if (bd.pResource->GetResourceUse() == Resource::Use_Bitmap)
                            {
                                ImageResource* pimageRes = (ImageResource*)(Resource*)bd.pResource;
                                pImagePacker->AddResource(presData, pimageRes);
                            }
                        }

                        // SetBindData AddRefs to the resource.
                        resourceBinding.SetBindData(presData->BindIndex, bd);
                    }
                    else
                    {
                        if (pBindData->BindingCanceled)
                        {
                            // Handle cancel. We may and up here if resource creating
                            // failed due to cancel (if it tired to access MovieDefImpl).
                            FinishBinding();
                            SetBindState(MovieDefImpl::BS_Canceled | GetBindStateFlags());
#ifdef SF_ENABLE_THREADS
                            Ptr<LoadUpdateSync> psync = pBindData->GetBindUpdateSync();
                            pBindData = 0;
                            psync->NotifyLoadFinished();
#else
                            pBindData = 0;
#endif
                            return MovieDefImpl::BS_Canceled;
                        }

                        // Set empty binding otherwise: this allows for indexing
                        // without problems.
                        resourceBinding.SetBindData(presData->BindIndex, bd);

                        // Fail the rest of loading/binding?
                        // Do we block on a certain frame?
                        // Who emits the error message?
                    }
                }
            }


            // Make a list of fonts generated in this frame.
            if (pframeData->FontCount)
            {
                Array<FontResource*>   fonts;

                FontDataUseNode* pfontData = pframeData->pFontData;
                unsigned            ifont;

                for (ifont = 0; ifont < pframeData->FontCount;
                    ifont++, pfontData = pfontData->pNext)
                {
                    SF_ASSERT(pfontData != 0);

                    ResourceBindData rbd;
                    resourceBinding.GetResourceData(&rbd, pfontData->BindIndex);

                    if (rbd.pResource)
                    {
                        SF_ASSERT(rbd.pResource->GetResourceType() == Resource::RT_Font);
                        FontResource* pfont = (FontResource*)rbd.pResource.GetPtr();
                        pfont->ResolveTextureGlyphs();
                        fonts.PushBack(pfont);
                    }
                }

                // Generate font bitmaps for the frame.
#ifdef SF_RENDER_ENABLE_GLYPH_PACKER
                if (fonts.GetSize())
                {
                    // NOTE: Some fonts may have already been rasterized if they were imported 
                    // through '_glyphs' substitution, which can happen during CreateResource above.

                    if (pls->GetFontPackParams() != 0)
                    {
                        GFx_GenerateFontBitmaps(pls->GetFontPackParams(), fonts,
                            pls->GetBindStates()->pImageCreator,
                            pls->GetLog(),
                            &GlyphTextureIdGen,
                            pheap,
                            pls->IsThreadedLoading());
                    }
                    else
                    {
                        // TO DO: Revise and Restore
                        //// Warn if there is no pack params and cache;
                        //// this would cause glyphs to be rendered as shapes.
                        //SF_DEBUG_WARNING(!pls->GetFontCacheManager() ||
                        //    !pls->GetFontCacheManager()->IsDynamicCacheEnabled(),
                        //    "Both font packing and dynamic cache disabled - text will be vectorized");
                    }
                }
#endif
            }


            // This frame done, advance and update state.
            pBindData->BytesLoaded = pframeData->BytesLoaded;

            AtomicOps<unsigned>::Store_Release(&pBindData->BindingFrame, pBindData->BindingFrame + 1);

            if (pBindData->BindingFrame == 1)
            {
                // Frame 1 is special because clients can wait for it, so alter state.
                SetBindState(GetBindState() | MovieDefImpl::BSF_Frame1Loaded);
            }

            if (pBindData->BindingFrame == pDataDef->GetFrameCount())
            {
                // Update bytes loaded for the end of the file. This is necessary
                // because we don't count the size of final tag 0 during loading.
                pBindData->BytesLoaded = pDataDef->GetFileBytes();

                FinishBinding();
                // Update state and we are done.
                SetBindState(MovieDefImpl::BS_Finished |
                    GetBindStateFlags() | MovieDefImpl::BSF_LastFrameLoaded);
            }

            ProgressHandler* ph = pls->GetProgressHandler();
            if (ph)
            {
                ph->ProgressUpdate(ProgressHandler::Info(pBindData->GetDataDef()->pData->FileURL,
                    pBindData->BytesLoaded, pBindData->GetDataDef()->GetFileBytes(),
                    pBindData->BindingFrame, pBindData->GetDataDef()->GetFrameCount()));
            }

            return GetBindStateType();
        }


        void    MovieBindProcess::FinishBinding()
        {
            if (pImagePacker)
            {
                struct ImagePackVisitor : public MovieDef::ResourceVisitor
                {
                    ImagePacker* pImagePacker;
                    TempBindData* pTempBindData;
                    ImagePackVisitor(ImagePacker* pimagePacker, TempBindData* ptempBindData) :pImagePacker(pimagePacker),
                        pTempBindData(ptempBindData) {
                    }

                    virtual void    Visit(MovieDef* pmovieDef, Resource* presource,
                        ResourceId rid, const char* pexportName)
                    {
                        SF_UNUSED2(rid, pmovieDef);
                        bool packable = (pTempBindData->FillStyleImageWrap.Get(pImagePacker->GetResourceDataNode(presource)->BindIndex) == 0);
                        bool forcepack = (pexportName && (strstr(pexportName, "-forcepack") || strstr(pexportName, ".forcepack")));
                        if (packable || forcepack)
                            pImagePacker->AddImageFromResource((ImageResource*)presource, pexportName);

                    }
                } visitor(pImagePacker, pTempBindData);
                Ptr<MovieDefImpl> pdefImpl = *pBindData->GetMovieDefImplAddRef();
                pdefImpl->VisitResources(&visitor, MovieDef::ResVisit_Bitmaps);
                pImagePacker->Finish();
            }


            pBindData->ResourceBinding.Freeze();
        }


        MemoryContext* MovieDefImpl::CreateMemoryContext(const char* heapName,
            const MemoryParams& memParams,
            bool debugHeap)
        {
            ASSupport* pASSupport = GetASSupport();
            if (pASSupport == NULL)
            {
                return NULL;
            }
            return pASSupport->CreateMemoryContext(heapName, memParams, debugHeap);
        }



        // Create a playable GFxMovieSub instance from a def.
        Movie* MovieDefImpl::CreateInstance(const MemoryParams& memParams, bool initFirstFrame, ActionControl* actionControl, Render::ThreadCommandQueue* queue)
        {
            String heapName("MovieView \"", GetShortFilename(GetFileURL()), "\"");

            MemoryContext* memContext = CreateMemoryContext(heapName.ToCStr(), memParams,
                (GetLoadFlags() & Loader::LoadDebugHeap) != 0);

            if (memContext)
            {
                Movie* movieView = CreateInstance(memContext, initFirstFrame, actionControl, queue);
                memContext->Release();  // the MovieView owns the context now
                return movieView;
            }
            return NULL;
        }


        Movie* MovieDefImpl::CreateInstance(MemoryContext* memContext, bool initFirstFrame, ActionControl* actionControl, Render::ThreadCommandQueue* queue)
        {
            ASSupport* pASSupport = GetASSupport();
            if (pASSupport == NULL)
            {
                return NULL;
            }

            MovieImpl* proot = pASSupport->CreateMovie(memContext);
            if (proot == NULL)
            {
                return NULL;
            }

            if (actionControl != NULL)
            {
                proot->SetActionControl(actionControl);
            }

            if (!proot->Init(this))
            {
                proot->Release();
                return NULL;
            }

            if (proot->AdvanceStats)
            {
                proot->AdvanceStats->SetMovieDef(proot->GetMovieDef());
            }

            // Set the ThreadCommandQueue instance (if available) before the Advance.
            proot->SetThreadCommandQueue(queue);

            if (initFirstFrame)
            {
                proot->Advance(0.0f, 0);
            }

            // AddRef unnecessary, refCount == 1 on new
            return proot;
        }



        // *** MovieDefImpl::LoadData


        MovieDefImpl::BindTaskData::BindTaskData(MemoryHeap* pheap,
            MovieDataDef* pdataDef,
            MovieDefImpl* pdefImpl,
            unsigned loadFlags, bool fullyLoaded)
            : pHeap(pheap), pDataDef(pdataDef), pDefImpl_Unsafe(pdefImpl), ResourceBinding(pheap)
        {
            // Technically BindTaskData should not store a pointer to MovieDefImpl,
            // however, we just pass it to initialize regular ptr in ResourceBinding.
            // This should not be a problem since it is only used where DefImpl reference
            // is guaranteed to exist.
            // (We should reset it to 0 on DefImpl death for consistency).
            ResourceBinding.SetOwnerDefImpl(pdefImpl);

            LoadFlags = loadFlags;

            // Initialize binding progress state.
            BindingCanceled = 0;
            BindingFrame = 0;
            BytesLoaded = 0;
            BindState = BS_NotStarted;

#ifdef SF_ENABLE_THREADS
            // do not create LoadUpdateSync for protocol images! Save 2 mutexes.
            if (pdataDef->MovieType != MovieDataDef::MT_Image || !LoaderImpl::IsProtocolImage(pdataDef->GetFileURL()))
                pBindUpdate = *SF_NEW LoadUpdateSync();
#endif

            if (fullyLoaded)
            {
                BindingFrame = GetDataDef()->GetLoadingFrame();
                BytesLoaded = GetDataDef()->GetFileBytes();
            }
        }

        MovieDefImpl::BindTaskData::~BindTaskData()
        {
#ifdef SF_ENABLE_THREADS
            // Wait for the lock if some other objects is still accessing the object
            if (pBindUpdate)
                pBindUpdate->GetMutex().DoLock();
#endif
            // Release all binding references before ImportSourceMovies is
            // cleared. This is required because SpriteDef references contain
            // ExecuteTag tags that must be destroyed before MovieDefImpl is
            // released; thus all imported binding references must be cleared first.    
            // If this is not done tag destructors may crash if MovieDefImpl was
            // released first.
            ResourceBinding.Destroy();
#ifdef SF_ENABLE_THREADS
            // Wait for the lock if some other objects is still accessing the object
            if (pBindUpdate)
                pBindUpdate->GetMutex().Unlock();
#endif
        }


        void    MovieDefImpl::BindTaskData::OnMovieDefRelease()
        {
            // Clear pointers to our pDataDef.
            ResourceBinding.SetOwnerDefImpl(0);
            {   // lock scope.
                Lock::Locker lock(&ImportSourceLock);
                pDefImpl_Unsafe = 0;
            }

            // Set flag to cancel binding process.
            if (GetBindStateType() <= BS_InProgress)
                BindingCanceled = true;
            // Signal the frame update mutex, since we could be waiting on it.
            pDataDef->pData->NotifyFrameUpdated();
        }


        // Grabs OUR MovieDefImpl through a lock; can return null.
        MovieDefImpl* MovieDefImpl::BindTaskData::GetMovieDefImplAddRef()
        {
            Lock::Locker lock(&ImportSourceLock);
            if (pDefImpl_Unsafe && pDefImpl_Unsafe->AddRef_NotZero())
                return pDefImpl_Unsafe;
            return 0;
        }


        // Bind State Updates / Waiting.

        void    MovieDefImpl::BindTaskData::SetBindState(unsigned newState)
        {
#ifdef SF_ENABLE_THREADS
            if (pBindUpdate)
            {
                Mutex::Locker lock(&pBindUpdate->GetMutex());
                BindState = newState;
                pBindUpdate->UpdateNotify();
            }
            else
                BindState = newState;
#else
            BindState = newState;
#endif
            //    if (GetBindStateType() == BS_Canceled)
            //        printf("MovieDefImpl::BindTaskData::SetBindState, thread: %d\n", GetCurrentThreadId());
        }

        bool   MovieDefImpl::BindTaskData::WaitForBindStateFlags(unsigned flags)
        {
            //printf("MovieDefImpl::BindTaskData::WaitForBindStateFlags: %x, thread: %d\n", this, GetCurrentThreadId());
#ifdef SF_ENABLE_THREADS
            if (pBindUpdate)
            {
                // Wait for for bind state flag or error. Return true for success,
                // false if bind state was changed to error without setting the flags.
                Mutex::Locker lock(&pBindUpdate->GetMutex());
                while (GetBindStateType() < BS_Canceled && (GetBindState() & flags) == 0)
                {
                    pBindUpdate->WaitForNotify();
                }
            }
#endif

            return (GetBindState() & flags) != 0;
        }

        // Updates binding state Frame and BytesLoaded (called from image loading task).
        void    MovieDefImpl::BindTaskData::UpdateBindingFrame(unsigned frame, UInt32 bytesLoaded)
        {
            // Assign BytesLoaded first so that Advance gets them correctly for frame.
            BytesLoaded = bytesLoaded;
            AtomicOps<unsigned>::Store_Release(&BindingFrame, frame);
        }



        // Access import source movie based on import index with a lock.
        MovieDefImpl* MovieDefImpl::BindTaskData::GetImportSourceMovie(unsigned importIndex)
        {
            // Access ImportSourceMovies in a lock.
            Lock::Locker loc(&ImportSourceLock);
            if (importIndex >= ImportSourceMovies.GetSize())
                return 0;
            return ImportSourceMovies[importIndex];
        }

        // Adds a movie reference to ResourceImports array.
        void    MovieDefImpl::BindTaskData::AddResourceImportMovie(MovieDefImpl* pdefImpl)
        {
            ResourceImports.PushBack(pdefImpl);
        }


        // Grabs the stuff we want from the source pMovie.
        void    MovieDefImpl::BindTaskData::ResolveImport(
            ImportData* pimport, MovieDefImpl* pdefImpl,
            LoadStates* pls, bool recursive)
        {
            bool imported = false;
            SF_UNUSED(imported);

            // Go through all character ids and resolve their handles.
            for (unsigned i = 0; i < pimport->Imports.GetSize(); i++)
            {
                ImportData::Symbol& symbol = pimport->Imports[i];

                // Look for exports and substitute.

                // Do the import.
                ResourceBindData bindData;

                if (!pdefImpl->GetExportedResource(&bindData, symbol.SymbolName))
                {
                    if (pls->GetLogState())
                        pls->GetLogState()->LogError(
                            "Import failed - resource '%s' is not exported from movie '%s'",
                            symbol.SymbolName.ToCStr(), pimport->SourceUrl.ToCStr());
                }
                else
                {
                    // Add it to binding for the character.
                    // Set imported flag so that we can AddRef to movieDef of an import,
                    // allowing resources such as fonts to work right.
                    imported = SetResourceBindData(ResourceId(symbol.CharacterId),
                        bindData, symbol.SymbolName.ToCStr());
                }
            }

            // Hold a ref, to keep this source MovieDef alive.
            // TBD: Why only add if imported ??
            //if (imported)
            //!AB: actually, AS3 SWF files may contain ImportAsset2 tags with empty table of
            // imports; we still need to hold such movies since symbols could be accessed by
            // export names from PlaceObject3 (using ClassName) or from AS3.
            {
                if (!recursive)
                {
                    Lock::Locker loc(&ImportSourceLock);
                    ImportSourceMovies.PushBack(pdefImpl);
                }
            }

            // No font substitution for recursive imports.
            if (recursive)
                return;


            // Name-based font import substitution:
            // The Font EXPORT feature in Flash Studio ignores the embedded glyphs and instead,
            // exports a fixed number of glyphs (243 or larger, depending on studio version), 
            // which makes it unstable for font imports. 
            // This name-based substitution can be used instead, replacing "Device Fonts" with
            // identically named font from the shared font.swf file.

            // To make exported fonts match up using export name we need glyphs to come
            // from a different file. Font substitution occurs if source import file name
            // has '_glyphs' in it.
            bool        forceFontSubstitution = 0;
            String   lowerURL = pimport->SourceUrl.ToLower();

            if (strstr(lowerURL.ToCStr(), "_glyphs") != 0)
            {
                forceFontSubstitution = 1;

                // Save substitution so that it can be potentially checked.
                pls->SubstituteFontMovieDefs.PushBack(pdefImpl);
            }


            /*  Font substitution difficulties and how they are addressed:

            Full font substitution can not take place here because some of the
            FontUse entries (for future frames) may not yet be created. Similarly
            there might be some thread considerations having to do with overwriting
            fonts bound in previous frames.

            Specifically, there are two substitution problems:

            1) Previously bound fonts could have already been used by text field
            instances; substituting them may be too late.

            For now: Go through list, substitute anyway. This would not be
            a problem if we could force all imports to take place before
            font definitions. It is not clear if this is practical and can be
            controlled in Flash studio to the extent we need - TBD (need
            to research this).

            For now, we can have text field cached texture glyph batches store
            original font pointers and compare them to indexed font handle lookup
            result. Text fields would then be updated in the event of change.
            TBD - any thread issues with this?


            2) Future fonts may not have FontUse array entries and thus need to
            be substituted later on.

            Solution: we add all forced substitutes sources to the
            pls->SubstituteFontMovieDefs array. This array will need to be checked
            when GFxFontResourceCreator::CreateResource is called to create
            FontResource from FontData.

            */


            // Iterate through fonts and resolve fonts with no glyphs from the def.
            FontDataUseNode* pfont = GetDataDef()->GetFirstFont();
            FontDataUseNode* psourceFont = pdefImpl->GetDataDef()->GetFirstFont();

            for (; pfont != 0; pfont = pfont->pNext)
            {
                Font* pfontData = pfont->pFontData;

                if ((pfontData->GetGlyphShapeCount() == 0) || forceFontSubstitution)
                {
                    // Search imports for the font with the same name which has glyphs.       
                    FontDataUseNode* psfont = psourceFont;

                    for (; psfont != 0; psfont = psfont->pNext)
                    {
                        Font* psourceFontData = psfont->pFontData;

                        if (psourceFontData->GetGlyphShapeCount() > 0)
                        {
                            if (pfontData->MatchSubstituteFont(psourceFontData))
                            {
                                // With DefData separation, we do not need to ReplaceExport
                                // any more; updating the binding should be good enough.

                                // Set our binding.
                                // Note: Unlike us, the source is guaranteed to be fully loaded when
                                // this takes place, so we can just look up its binding.
                                ResourceBindData sourceBindData;
                                pdefImpl->pBindData->ResourceBinding.GetResourceData(
                                    &sourceBindData, psfont->BindIndex);

                                if (sourceBindData.pResource)
                                {
                                    ResourceBinding.SetBindData(pfont->BindIndex, sourceBindData);
                                }

                                break;
                            }
                        }
                    }
                }
            }

        }


        // Resolves an import of 'gfxfontlib.swf' through the FontLib object.
        bool    MovieDefImpl::BindTaskData::ResolveImportThroughFontLib(ImportData* pimport)
        {
            // Go through all character ids and resolve their handles.
            for (unsigned i = 0; i < pimport->Imports.GetSize(); i++)
            {

                ImportData::Symbol& symbol = pimport->Imports[i];
                ResourceBindData bindData;
                bindData.pBinding = &ResourceBinding;
                // Create a new font with FF_NotResolved flag set. This will instruct FontManager
                // to search this font through FontLib and FontProvider
                Ptr<Font> font = *new FontData(symbol.SymbolName.ToCStr(), 0);
                font->SetNotResolvedFlag();
                bindData.pResource = *new FontResource(font, &ResourceBinding);

                SetResourceBindData(ResourceId(symbol.CharacterId), bindData,
                    symbol.SymbolName.ToCStr());
            }

            // Add an empty import so that ImportSourceMovies still matches ImportData.
            Lock::Locker loc(&ImportSourceLock);
            ImportSourceMovies.PushBack(0);
            return true;
        }


        // Internal helper for import updates.
        bool    MovieDefImpl::BindTaskData::SetResourceBindData(
            ResourceId rid, ResourceBindData& bindData,
            const char* pimportSymbolName)
        {
            ResourceHandle rh;
            if (GetDataDef()->GetResourceHandle(&rh, rid))
            {
                SF_ASSERT(rh.IsIndex());

                // Establish an association for BindIndex; note that
                // ResourceBindData is not used here. For imported characters
                // ResourceData elements will be empty.
                ResourceBinding.SetBindData(rh.GetBindIndex(), bindData);

                // Return imported flag so that the caller knows when to AddRef to the MovieDef
                return true;
            }

            // The handle for import MUST exists in DataDef since it is created 
            // during the Read of import tags. If it doesn't, there is an error or
            // perhaps we are out of memory.   
            SF_DEBUG_WARNING1(1,
                "Internal import bind error: bind handle not found for resource '%s'",
                pimportSymbolName);
            SF_UNUSED(pimportSymbolName); // For release build.

            return false;
        }

        bool    MovieDefImpl::BindTaskData::SetResourceBindData(ResourceDataNode* presnode, Resource* pres)
        {
            ResourceBindData bd;
            bd.pBinding = &ResourceBinding;
            bd.pResource = pres;
            ResourceBinding.SetBindData(presnode->BindIndex, bd);
            return true;
        }

        Ptr<Render::ShapeMeshProvider> MovieDefImpl::BindTaskData::GetShapeMeshProvider(Render::ShapeMeshProvider* defMeshProv)
        {
            Lock::Locker loc(&ImportSourceLock);
            Ptr<Render::ShapeMeshProvider>* r = BoundShapeMeshProviders.Get(defMeshProv);
            return (r) ? *r : Ptr<Render::ShapeMeshProvider>();
        }

        void MovieDefImpl::BindTaskData::AddShapeMeshProvider(Render::ShapeMeshProvider* defMeshProv,
            Render::ShapeMeshProvider* resolvedMeshProv)
        {
            Lock::Locker loc(&ImportSourceLock);
            BoundShapeMeshProviders.Set(defMeshProv, resolvedMeshProv);
        }

        // ***** MovieDefBindStates

        // MovieDefBindStates is used as a part of the key object in MovieDefImpl.
        UPInt  MovieDefBindStates::GetHashCode() const
        {
            return
                ((UPInt)pFileOpener.GetPtr()) ^ (((UPInt)pFileOpener.GetPtr()) >> 7) ^
                ((UPInt)pURLBulider.GetPtr()) ^ (((UPInt)pURLBulider.GetPtr()) >> 7) ^
                ((UPInt)pImageCreator.GetPtr()) ^ (((UPInt)pImageCreator.GetPtr()) >> 7) ^
                ((UPInt)pImportVisitor.GetPtr()) ^ (((UPInt)pImportVisitor.GetPtr()) >> 7) ^
                ((UPInt)pFontPackParams.GetPtr()) ^ (((UPInt)pFontPackParams.GetPtr()) >> 7) ^
                ((UPInt)pFontCompactorParams.GetPtr()) ^ (((UPInt)pFontCompactorParams.GetPtr()) >> 7) ^
                ((UPInt)pImagePackParams.GetPtr()) ^ (((UPInt)pImagePackParams.GetPtr()) >> 7);
        }

        bool MovieDefBindStates::operator == (MovieDefBindStates& other) const
        {
            // For bind states to be identical all of the binding
            // states must match.
            return pFileOpener == other.pFileOpener &&
                pURLBulider == other.pURLBulider &&
                pImageCreator == other.pImageCreator &&
                pImportVisitor == other.pImportVisitor &&
                //      pImageVisitor == other.pImageVisitor &&
                pFontPackParams == other.pFontPackParams &&
                pFontCompactorParams == other.pFontCompactorParams &&
                pImagePackParams == other.pImagePackParams;
        }


        // ***** MovieDefImpl Key 

        class  GFxMovieDefImplKey : public RefCountBase<GFxMovieDefImplKey, Stat_Default_Mem>
        {
            // MovieDefImpl key consists of DataDef and bind states pointers.
            // pDataDef can not be inside of MovieDefBindStates because
            // the later is used in LoadStates and thus referenced by the
            // loading process.
            Ptr<MovieDataDef>       pDataDef;
            Ptr<MovieDefBindStates> pBindStates;
        public:

            GFxMovieDefImplKey(MovieDataDef* pdataDef, MovieDefBindStates* pbindStates)
                : pDataDef(pdataDef), pBindStates(pbindStates)
            {
            }

            // Functionality necessary for this MovieDefBindStates to be used
            // as a key object for MovieDefImpl.
            UPInt   GetHashCode() const
            {
                return  ((UPInt)pDataDef.GetPtr()) ^ (((UPInt)pDataDef.GetPtr()) >> 7) ^
                    pBindStates->GetHashCode();
            }
            bool operator == (GFxMovieDefImplKey& other) const
            {
                return pDataDef == other.pDataDef && *pBindStates == *other.pBindStates;
            }
            bool operator != (GFxMovieDefImplKey& other) const { return !operator == (other); }
        };


        class GFxMovieDefImplKeyInterface : public ResourceKey::KeyInterface
        {
        public:
            typedef ResourceKey::KeyHandle KeyHandle;

            virtual void    AddRef(KeyHandle hdata)
            {
                SF_ASSERT(hdata); ((GFxMovieDefImplKey*)hdata)->AddRef();
            }
            virtual void    Release(KeyHandle hdata)
            {
                SF_ASSERT(hdata); ((GFxMovieDefImplKey*)hdata)->Release();
            }
            virtual ResourceKey::KeyType GetKeyType(KeyHandle hdata) const
            {
                SF_UNUSED(hdata); return ResourceKey::Key_Unique;
            }

            virtual UPInt   GetHashCode(KeyHandle hdata) const
            {
                SF_ASSERT(hdata);
                return ((GFxMovieDefImplKey*)hdata)->GetHashCode();
            }
            virtual bool    KeyEquals(KeyHandle hdata, const ResourceKey& other)
            {
                if (this != other.GetKeyInterface())
                    return 0;
                return *((GFxMovieDefImplKey*)hdata) ==
                    *((GFxMovieDefImplKey*)other.GetKeyData());
            }
        };

        static GFxMovieDefImplKeyInterface GFxMovieDefImplKeyInterface_Instance;

        // Create a key for an SWF file corresponding to MovieDef.
        ResourceKey  MovieDefImpl::CreateMovieKey(MovieDataDef* pdataDef,
            MovieDefBindStates* pbindStates)
        {
            Ptr<GFxMovieDefImplKey> pkey = *new GFxMovieDefImplKey(pdataDef, pbindStates);

            return ResourceKey(&GFxMovieDefImplKeyInterface_Instance,
                (ResourceKey::KeyHandle)pkey.GetPtr());
        }




        // Fill in the binding resource information together with its binding.
        bool MovieDefImpl::GetExportedResource(ResourceBindData* pdata, const String& symbol, MovieDefImpl* ignoreDef)
        {
            // Thread TBD: Should we block for availability depending on whether
            // the source has finished loading?

            ResourceHandle hres;
            bool              exportFound = 0;

            {   // Lock Exports.
                MovieDataDef::LoadTaskData::ResourceLocker lock(GetDataDef()->pData);
                exportFound = GetDataDef()->pData->Exports.GetCaseInsensitive(symbol, &hres);
            }

            if (exportFound)
            {
                // Determine the exact binding
                if (hres.IsIndex())
                {
                    pBindData->ResourceBinding.GetResourceData(pdata, hres.GetBindIndex());
                }
                else
                {
                    pdata->pBinding = &pBindData->ResourceBinding;
                    pdata->pResource = hres.GetResource(&pBindData->ResourceBinding);
                }
                return (pdata->pResource.GetPtr() != 0);
            }

            else
            {
                // Nested import symbols are also visible in parent, although
                // our own Exports table has precedence (checked for above).

                Array<Ptr<MovieDefImpl>, StatMD_Other_Mem> importsCopy;

                { // Access ImportSourceMovies in a lock.
                    Lock::Locker loc(&pBindData->ImportSourceLock);

                    importsCopy.Reserve(pBindData->ImportSourceMovies.GetSize());
                    for (UPInt i = 0; i < pBindData->ImportSourceMovies.GetSize(); i++)
                    {
                        MovieDefImpl* pdef = pBindData->ImportSourceMovies[i];
                        if (pdef != ignoreDef)
                            importsCopy.PushBack(pdef);
                    }
                }

                // TBD: Any particular traversal order ?
                for (unsigned i = 0; i < importsCopy.GetSize(); i++)
                {
                    MovieDefImpl* pdef = importsCopy[i].GetPtr();
                    if (pdef && pdef->GetExportedResource(pdata, symbol))
                        return 1;
                }
            }

            return 0;
        }

        bool MovieDefImpl::DoesDirectlyImport(const MovieDefImpl* import)
        {
            Lock::Locker loc(&pBindData->ImportSourceLock);

            for (UPInt i = 0; i < pBindData->ImportSourceMovies.GetSize(); i++)
            {
                MovieDefImpl* pdef = pBindData->ImportSourceMovies[i];
                if (pdef == import)
                    return true;
            }
            return false;
        }

        const String* MovieDefImpl::GetNameOfExportedResource(ResourceId rid) const
        {
            MovieDataDef::LoadTaskData::ResourceLocker lock(GetDataDef()->pData);
            return GetDataDef()->pData->InvExports.Get(rid);
        }
        ResourceId MovieDefImpl::GetExportedResourceIdByName(const String& name) const
        {
            typedef HashLH<ResourceId, StringLH, FixedSizeHash<ResourceId> > THASH;
            MovieDataDef::LoadTaskData::ResourceLocker lock(GetDataDef()->pData);
            THASH::ConstIterator it = GetDataDef()->pData->InvExports.Begin();
            for (; it != GetDataDef()->pData->InvExports.End(); ++it)
            {
                if (it->Second == name)
                    return it->First;
            }
            return ResourceId();
        }

        Ptr<ASSupport> MovieDefImpl::GetASSupport() const
        {
            int asver = GetDataDef()->GetASVersion();
            ASSupport* pASSupport = NULL;
            if (asver <= 2)
            {
                pASSupport = GetAS2Support();
            }
            else
            {
                pASSupport = GetAS3Support();
            }
            if (pASSupport == NULL)
            {
                // Appropriate AS support is not installed!
                SF_DEBUG_WARNING1(1, "MovieDefImpl::CreateInstance: ASSupport for AS%d.0 is not installed!", asver);
                return NULL;
            }
            return pASSupport;
        }

        // *** Resource Lookup


        // Obtains full character creation information, including CharacterDef.
        CharacterCreateInfo     MovieDefImpl::GetCharacterCreateInfo(ResourceId rid)
        {
            ResourceHandle      rh;

            CharacterCreateInfo ccinfo;
            ccinfo.pCharDef = 0;
            ccinfo.pBindDefImpl = 0;
            ccinfo.pResource = 0;

            if (GetDataDef()->GetResourceHandle(&rh, rid))
            {
                ResourceBinding* pbinding;
                Resource* pres =
                    rh.GetResourceAndBinding(&pBindData->ResourceBinding, &pbinding);
                if (pres)
                {
                    ccinfo.pResource = pres;
                    if (pres->GetResourceType() & Resource::RT_CharacterDef_Bit)
                    {
                        ccinfo.pBindDefImpl = pbinding->GetOwnerDefImpl();
                        ccinfo.pCharDef = (CharacterDef*)pres;
                    }
                }
            }

            return ccinfo;
        }




        // Calls back the visitor for each GFxMovieSub that we
        // import symbols from.
        void    MovieDefImpl::VisitImportedMovies(MovieDefImpl::ImportVisitor* visitor)
        {

            // Thread TBD: Ensure that all imports are loaded (?)
            ImportData* pimportData = GetDataDef()->GetFirstImport();

            if (pimportData)
            {
                // TBD: Visited may no longer be necessary in 2.0. However,
                // we need to verify that it's not possible to produce to identical import
                // file references in the Flash studio.
                StringHash<bool>        visited;

                while (pimportData)
                {
                    MovieDefImpl* pindexedMovie = 0;

                    { // Access ImportSourceMovies in a lock.
                        Lock::Locker loc(&pBindData->ImportSourceLock);

                        if (pimportData->ImportIndex >= pBindData->ImportSourceMovies.GetSize())
                        {
                            // We may not want to allow this. The only time this could happen is
                            // if file is still being bound by another thread that has not caught
                            // up to this point, or binding error has occurred.
                            SF_ASSERT(0);
                            break;
                        }

                        pindexedMovie = pBindData->ImportSourceMovies[pimportData->ImportIndex];
                    }

                    if (visited.FindCaseInsensitive(pimportData->SourceUrl) == visited.End())
                    {
                        // Call back the visitor.
                        if (pindexedMovie)
                            visitor->Visit(this, pindexedMovie,
                                pimportData->SourceUrl.ToCStr());
                        visited.SetCaseInsensitive(pimportData->SourceUrl, true);
                    }

                    pimportData = pimportData->pNext;
                }
            }
        }


        void MovieDefImpl::VisitResources(ResourceVisitor* pvisitor, unsigned visitMask)
        {

            if (visitMask & (MovieDef::ResVisit_AllImages |
                MovieDef::ResVisit_Fonts |
                MovieDef::ResVisit_EditTextFields |
                MovieDef::ResVisit_Sounds |
                MovieDef::ResVisit_Sprite))
            {
                // Lock DataDef's Resources and Exports.
                MovieDataDef::LoadTaskData::ResourceLocker lock(GetDataDef()->pData);

                MovieDataDef::ResourceHash& resources = GetDataDef()->pData->Resources;
                MovieDataDef::ResourceHash::ConstIterator  ihash = resources.Begin();

                for (; ihash != resources.End(); ++ihash)
                {
                    Resource* pres = ihash->Second.GetResource(&pBindData->ResourceBinding);
                    bool         resourceMatch = 0;

                    if (pres)
                    {
                        Resource::ResourceUse use = pres->GetResourceUse();

                        switch (pres->GetResourceType())
                        {
                        case Resource::RT_Image:
                        {
                            if (use == Resource::Use_Bitmap)
                            {
                                if (visitMask & MovieDef::ResVisit_Bitmaps)
                                    resourceMatch = 1;
                            }
                            else if (use == Resource::Use_Gradient)
                            {
                                if (visitMask & MovieDef::ResVisit_GradientImages)
                                    resourceMatch = 1;
                            }
                        }
                        break;

                        case Resource::RT_Font:
                            if (visitMask & MovieDef::ResVisit_Fonts)
                                resourceMatch = 1;
                            break;

                        case Resource::RT_EditTextDef:
                            if (visitMask & MovieDef::ResVisit_EditTextFields)
                                resourceMatch = 1;
                            break;

                        case Resource::RT_SoundSample:
                            if (visitMask & MovieDef::ResVisit_Sounds)
                                resourceMatch = 1;
                            break;

                        case Resource::RT_SpriteDef:
                            if (visitMask & MovieDef::ResVisit_Sprite)
                                resourceMatch = 1;
                            break;

                        default: break;
                        }
                    }

                    if (resourceMatch)
                    {
                        // Determine export name by doing a search.
                        const char* pexportName = 0;

                        StringHashLH<ResourceHandle>& exports = GetDataDef()->pData->Exports;
                        StringHashLH<ResourceHandle>::Iterator iexport = exports.Begin();

                        while (iexport != exports.End())
                        {
                            if (iexport->Second.Equals(ihash->Second))
                            {
                                pexportName = iexport->First.ToCStr();
                                break;
                            }
                            ++iexport;
                        }

                        pvisitor->Visit(this, pres, ihash->First, pexportName);
                    }
                }
            }

            // Check to see if we need to traverse nested files.
            if (visitMask & MovieDef::ResVisit_NestedMovies)
            {
                // visit imported movies
                Array<Ptr<MovieDefImpl>, StatMD_Other_Mem> importsCopy;

                { // Access ImportSourceMovies in a lock.
                    Lock::Locker loc(&pBindData->ImportSourceLock);

                    //importsCopy = pBindData->ImportSourceMovies;
                    importsCopy.Reserve(pBindData->ImportSourceMovies.GetSize());
                    for (UPInt i = 0; i < pBindData->ImportSourceMovies.GetSize(); i++)
                        importsCopy.PushBack(pBindData->ImportSourceMovies[i]);
                }

                for (UPInt i = 0, n = importsCopy.GetSize(); i < n; ++i)
                {
                    if (importsCopy[i])
                        importsCopy[i]->VisitResources(pvisitor, visitMask);
                }
            }
        }


        Resource* MovieDefImpl::GetResource(const char* pexportName) const
        {
            if (!pexportName)
                return 0;

            // Find the export string
            String str(pexportName);

            MovieDataDef::LoadTaskData::ResourceLocker lock(GetDataDef()->pData);

            StringHashLH<ResourceHandle>& exports = GetDataDef()->pData->Exports;
            StringHashLH<ResourceHandle>::ConstIterator iexport = exports.FindCaseInsensitive(str);
            if (iexport == exports.End())
                return 0;

            // TBD: Thread issues?
            // Check if the resource has been resolved yet. Not quite right since
            // pointer-based handles are possible.
            //if (iexport->Second.GetBindIndex() >= ResourceBinding.GetResourceCount())    
            //    return 0;

            Resource* pres = iexport->Second.GetResource(&pBindData->ResourceBinding);
            return pres;
        }


        // Locate a font resource by name and style.
        // It's ok to return FontResource* without the binding because pBinding
        // is embedded into font resource allowing imports to be handled properly.
        FontResource* MovieDefImpl::GetFontResource(const char* pfontName, unsigned styleFlags, SearchInfo* psearchInfo)
        {
            MovieDataDef* pdataDef = GetDataDef();

            // TBD: We will need to do something about threading here, since it is legit
            // to call GetFontResource while loading still hasn't completed.

            FontDataUseNode* pfont = pdataDef->GetFirstFont();

            for (; pfont != 0; pfont = pfont->pNext)
            {
                if (pfont->pFontData->MatchFont(pfontName, styleFlags))
                {
                    // TBD: Wait for resource binding?
                    ResourceBindData rbd;
                    pBindData->ResourceBinding.GetResourceData(&rbd, pfont->BindIndex);
                    SF_DEBUG_WARNING1(!rbd.pResource, "rbd.pResource = 0 for font '%s' ", pfontName);
                    if (rbd.pResource)
                    {
                        SF_ASSERT(rbd.pResource->GetResourceType() == Resource::RT_Font);
                        FontResource* pResourceFont = (FontResource*)rbd.pResource.GetPtr();
                        if (psearchInfo)
                        {
                            if (!pResourceFont->GetFont()->IsResolved())
                                psearchInfo->Status = SearchInfo::FoundInResourcesNoGlyphs;
                            else if ((styleFlags & Font::FF_BoldItalic) && !pResourceFont->GetFont()->HasVectorOrRasterGlyphs())
                                psearchInfo->Status = SearchInfo::FoundInResourcesNeedFaux;
                            else
                                psearchInfo->Status = SearchInfo::FoundInResources;
                        }
                        return pResourceFont;
                    }
                }
            }

            // Import names also look like font names for lookup purposes.
            // In particular, we need to do this to support imported shared font,
            // where export names are used during lookup.
            // We should look not only for import names but also for real font names. 
            // Imported font should be locatable either by import name (like $TitleFont)
            // or by the real font name (like Times New Roman).
            ImportData* pimport = pdataDef->GetFirstImport();

            for (; pimport != 0; pimport = pimport->pNext)
            {
                for (unsigned j = 0; j < pimport->Imports.GetSize(); j++)
                {
                    // Locate the font and match style flags.
                    // We will also have to report the new name so that it can be
                    // detected in the font manager.
                    if (psearchInfo)
                        psearchInfo->ImportSearchUrls.Set(pimport->SourceUrl);

                    ResourceHandle rh;
                    if (pdataDef->GetResourceHandle(&rh, ResourceId(pimport->Imports[j].CharacterId)))
                    {
                        Resource* pres = rh.GetResource(&pBindData->ResourceBinding);
                        // The resource must be a font.
                        if (pres && (pres->GetResourceType() == Resource::RT_Font))
                        {
                            FontResource* pfontRes = (FontResource*)pres;
                            if (pfontRes->GetFont()->MatchFontFlags(styleFlags))
                            {
                                // Compare the import font name and the real font name.
                                if (!pimport->Imports[j].SymbolName.CompareNoCase(pfontName) ||
                                    !String::CompareNoCase(pfontRes->GetName(), pfontName))
                                {
                                    // We found it!
                                    // Fill in name, since font will not have its export name.
                                    if (psearchInfo)
                                    {
                                        static const char fontlibKey[] = GFX_FONTLIB_IMPORT_KEY;
                                        const UPInt       fontlibKeySize = sizeof(fontlibKey) - 1;

                                        if ((pimport->SourceUrl.GetSize() >= fontlibKeySize) &&
                                            (String::CompareNoCase(pimport->SourceUrl.ToCStr() +
                                                (pimport->SourceUrl.GetSize() - fontlibKeySize), fontlibKey) == 0))
                                            psearchInfo->Status = SearchInfo::FoundInImportsFontLib;
                                        else
                                            psearchInfo->Status = SearchInfo::FoundInImports;
                                        psearchInfo->ImportFoundUrl = pimport->SourceUrl;
                                    }
                                    return pfontRes;
                                }
                            }
                        }
                    }
                }
            }

            // Look for the names of EXPORTED fonts too (!AB)
            Resource* pres = GetResource(pfontName);
            if (pres && (pres->GetResourceType() == Resource::RT_Font))
            {
                FontResource* pfontRes = (FontResource*)pres;
                if (pfontRes->GetFont()->MatchFontFlags(styleFlags))
                {
                    // We found it!
                    // Fill in name, since font will not have its export name.
                    if (psearchInfo)
                        psearchInfo->Status = SearchInfo::FoundInExports;
                    return pfontRes;
                }
            }

            if (!pres && pdataDef->GetASVersion() >= 3 && pdataDef->GetFirstImport())
            {
                // If still not found and this is AS3 SWF we can look through all imports and search
                // the font by its name. Seems like Flash does this too.
                Lock::Locker loc(&pBindData->ImportSourceLock);
                for (UPInt i = 0; i < pBindData->ImportSourceMovies.GetSize(); i++)
                {
                    MovieDefImpl* pdef = pBindData->ImportSourceMovies[i];
                    if (pdef)
                    {
                        FontResource* rv = pdef->GetFontResource(pfontName, styleFlags, psearchInfo);
                        if (rv)
                            return rv;
                    }
                }
            }

            if (psearchInfo)
                psearchInfo->Status = SearchInfo::NotFound;
            return 0;
        }


        // *** ExecuteTag - debug counters

#ifdef SF_BUILD_DEBUG

        class ASClassCounter
        {
        public:
            AtomicInt<unsigned> Count;

            ASClassCounter() { Count = 0; }
            // If this assertion is hit, it means that some allocated
            // ExecuteTag(s) weren't properly cleaned up.
            ~ASClassCounter() {}//{ SF_ASSERT(Count == 0); }
        };

        ASClassCounter AS_Counter_Instance;

        ExecuteTag::ExecuteTag()
        {
            AS_Counter_Instance.Count++;
            //Sentinel = 0xE465F1A9;
        }
        ExecuteTag::~ExecuteTag()
        {
            //SF_ASSERT(Sentinel == 0xE465F1A9);
            //Sentinel = 0;
            AS_Counter_Instance.Count--;
        }

#endif



        // ***** GFxInitImportActions - created for import tags.

        // Queues up logic to execute InitClip actions from the import,
        // using Highest priority by default.
        void    GFxInitImportActions::Execute(DisplayObjContainer* m)
        {
            // For regular execute our context is the same as of target sprite,
            // but it gets overriden if called from GFxSprite::ExecuteImportedInitActions.
            ExecuteInContext(m, m->GetResourceMovieDef(), 0);
        }

        // InitImportActions that come from imported files need to be executed
        // in the MovieDefImpl binding context (otherwise we would index parent's
        // source movies incorrectly, resulting in recursive loop).
        void    GFxInitImportActions::ExecuteInContext(DisplayObjContainer* m, MovieDefImpl* pbindDef, bool recursiveCheck)
        {
            MovieDefImpl* psourceImportMovie =
                pbindDef->pBindData->GetImportSourceMovie(ImportIndex);

            if (psourceImportMovie &&
                psourceImportMovie->GetDataDef()->HasInitActions())
            {
                // It is possible that SWF files will import themselves or other files recursively.
                // Check for that and don't run sprite's ResourceMovieDef's actions again.
                if (recursiveCheck && (psourceImportMovie == m->GetResourceMovieDef()))
                    return;

                m->ExecuteImportedInitActions(psourceImportMovie);
            }
        }

    }
} // Scaleform::GFx
