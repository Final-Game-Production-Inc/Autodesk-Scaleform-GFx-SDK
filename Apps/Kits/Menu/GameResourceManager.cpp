/**************************************************************************

Filename    :   GameResourceManager.cpp
Content     :   Sample resource manager for Menu Kit
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GameResourceManager.h"

#include "GFx/GFx_Player.h"
#include "FxPlayerLog.h"

#include "Render/ImageFiles/PNG_ImageFile.h" 
#include "GFx/GFx_ImageCreator.h"

#ifdef GFX_ENABLE_XML
#include "GFx_XML.h"
#endif

#ifdef GFX_ENABLE_VIDEO
// Core video support
#include "Video/Video_Video.h"
#if   defined(SF_OS_WIN32)
#include "Video/Video_VideoPC.h"
#elif defined(SF_OS_XBOX360)
#include "Video/Video_VideoXbox360.h"
#elif defined(SF_OS_PS3)
#include "Video/Video_VideoPS3.h"
#endif
// Video system sound interfaces
#ifdef GFX_VIDEO_USE_SYSTEM_SOUND
#if   defined(SF_OS_WIN32)
#include "Video/Video_VideoSoundSystemDX8.h"
#elif defined(SF_OS_XBOX360)
#include "Video/Video_VideoSoundSystemXA2.h"
#elif defined(SF_OS_PS3)
#include "Video/Video_VideoSoundSystemPS3.h"
#elif defined(SF_OS_MAC)
#include "Video/Video_VideoSoundSystemMac.h"
#endif
#endif // GFX_VIDEO_USE_SYSTEM_SOUND
#endif // GFX_ENABLE_VIDEO

#ifdef SF_OS_PS3
#include <sys/paths.h>
#include <cell/spurs.h>
#endif
 
#include "GFx/AS2/AS2_FunctionRefImpl.h"

#ifdef GFX_ENABLE_SOUND
#include "Sound/Sound_SoundRenderer.h"

using namespace Scaleform;
using namespace Sound;

class GameSoundChannelHolder : public NewOverrideBase<Stat_Default_Mem>, 
    public ListNode<GameSoundChannelHolder>
{
public:
    GameSoundChannelHolder(SoundChannel* pchan) : pChannel(pchan) {}
    SoundChannel*      GetChannel() const  { return pChannel; }

private:
    Ptr<SoundChannel> pChannel;
};

class GameSound : public RefCountBase<GameSound, Stat_Default_Mem>
{
public:
    GameSound(Sound::SoundRenderer* psoundRenderer, const char* source, unsigned maxChannelCount = 4)
    {
        pSample     = *psoundRenderer->CreateSampleFromFile(source, false);
        SF_ASSERT(pSample);
        ChannelCount    = 0;
        MaxChannelCount = maxChannelCount;
    }
    virtual ~GameSound() 
    {
        GameSoundChannelHolder* data = Channels.GetFirst();
        while (!Channels.IsNull(data))
        {
            GameSoundChannelHolder* next = Channels.GetNext(data);
            Channels.Remove(data);
            delete data;
            data = next;
        }
        ChannelCount = 0;
    }

    void    Play(Sound::SoundRenderer* psoundRenderer, float volume, float pan)
    {
        GameSoundChannelHolder* pchanHolder = NULL;

        // Clear completed channels
        GameSoundChannelHolder* data = Channels.GetFirst();
        while (!Channels.IsNull(data))
        {
            GameSoundChannelHolder* next = Channels.GetNext(data);
            if (!data->GetChannel()->IsPlaying())
            {
                Channels.Remove(data);
                delete data;
                ChannelCount--;
            }
            data = next;
        }

        if (ChannelCount >= MaxChannelCount) return;

        Ptr<SoundChannel> pchannel = *psoundRenderer->PlaySample(pSample, true);
        pchannel->SetVolume(volume);
        pchannel->SetPan(pan);
        pchannel->Pause(false);
        pchanHolder = SF_HEAP_AUTO_NEW(this) GameSoundChannelHolder(pchannel);
        Channels.PushBack(pchanHolder);
        ChannelCount++;
    }

protected:
    Ptr<SoundSample>                    pSample;
    List<GameSoundChannelHolder>        Channels;
    unsigned                            ChannelCount;
    unsigned                            MaxChannelCount;
};

class GameSoundManagerImpl : public NewOverrideBase<Stat_Default_Mem>
{
public:
    GameSoundManagerImpl(GameResourceManager* presMgr)
    {
#ifdef GFX_ENABLE_SOUND
        pSoundRenderer = presMgr->GetSoundRenderer();
#else
    SF_UNUSED(presMgr);
#endif
    }

    void    LoadSample(const char* id, const char* source)
    {
        if (!pSoundRenderer) 
        {
            SF_DEBUG_MESSAGE1(1, "Trying to load sound for id '%s' but sound renderer is NULL!\n", id);
            return;
        }
        Ptr<GameSound> psound = *SF_HEAP_AUTO_NEW(this) GameSound(pSoundRenderer, source, 1);
        Sounds.Add(id, psound);
    }
    bool    PlaySample(const char* id, float volume = 1.0f, float pan = 0.f)
    {
        GameSound*  psound = GetSound(id);
        if (!psound) return false;
        SF_DEBUG_MESSAGE1(1, "Playing sound for id '%s'\n", id);
        psound->Play(pSoundRenderer, volume, pan);        
        return true;
    }

private:
    Sound::SoundRenderer*                                   pSoundRenderer;
    Hash<String, Ptr<GameSound>, String::HashFunctor>       Sounds;

    GameSound*      GetSound(const char* name)
    {
        Ptr<GameSound>* psound = Sounds.Get(name);
        if (psound == NULL) return NULL;
        return *psound;
    }
};

GameSoundManager::GameSoundManager(GameResourceManager* presMgr, const String& pathPrefix) : pImpl(NULL), PathPrefix(pathPrefix)
{
    pImpl = SF_HEAP_AUTO_NEW(this) GameSoundManagerImpl(presMgr);
}

GameSoundManager::~GameSoundManager()
{
    SF_ASSERT(pImpl);
    delete pImpl;
}

void    GameSoundManager::LoadSound(const char* id, const char* source)
{
    String sourcePath = PathPrefix;
    sourcePath.AppendString(source);
    pImpl->LoadSample(id, sourcePath);
}

bool    GameSoundManager::PlaySound(const char* id, float volume, float pan )
{
    return pImpl->PlaySample(id, volume, pan);
}
#endif // GFX_ENABLE_SOUND

// ********************************************************************
// Custom file opener that requires memory files in the file cache
// unless for .usm movies (which are streamed separately).
//
class MenuKitFileOpener : public GFx::FileOpener 
{
public:
    MenuKitFileOpener(GameMemoryFileCache* pcache) : pMemoryFileCache(pcache) {}
    virtual File* OpenFile(const char *purl, int flags, int mode) 
    {
        bool isvideofile = strstr(purl, ".usm") != 0;

        if (isvideofile)
        {
            // Buffered file wrapper is faster to use because it optimizes seeks.
            return GFx::FileOpener::OpenFile(purl, flags, mode);
        }
        else
        {
            GameMemoryFile* pfile = pMemoryFileCache->GetFile(purl);
            if (pfile)
            {
                return SF_HEAP_AUTO_NEW(this) MemoryFile(purl, pfile->GetData(), pfile->GetLength());
            }
            else
            {
                printf("Resource Manager cannot find file '%s' in memory!\n", purl);
                return NULL;
            }
        }
    }

private:
    GameMemoryFileCache*    pMemoryFileCache;
};

#ifdef GFX_ENABLE_SOUND
// ********************************************************************
// CLIK sound event callback listener
//
class MenuKitSoundEventCallback : public GFx::FunctionHandler
{
public:
    MenuKitSoundEventCallback (const GFx::Movie* movieView, GameSoundManager* pmgr)
    {   
        // Note: Currently there is no support for Wwise sound events in the MenuKit.
        pSoundManager = pmgr;
        pLog = movieView->GetLog();   // Used for logging debug messages
    }
    virtual ~MenuKitSoundEventCallback() 
    {

    }

    /** Called when _global.gfxProcessSound() is called in ActionScript */
    void Call(const Params& params)
    {
        if (params.ArgCount >= 1)
        {
            if (params.pArgs[0].IsDisplayObject() && params.ArgCount == 3)
                ProcessCLIKSoundEvent(params);
        }
    }

    bool ProcessCLIKSoundEvent(const Params& params)
    {
        // Retrieve arguments from the function parameters
        const String componentName = params.pArgs[0].ToString();
        const String soundThemeName = params.pArgs[1].GetString();
        const String soundEventName = params.pArgs[2].GetString(); 
        String soundName = String(soundThemeName.ToCStr(), soundEventName.ToCStr());

#ifdef SF_BUILD_DEBUG
        // This log message can be commented or uncommented at the user's discretion.
        /*
        if (pLog)
            pLog->LogMessage("gfxProcessSound Callback - \n"
                            "\tComponent: %s\n"
                            "\tSound Theme: %s\n"
                            "\tSoundEvent: %s\n"
                            "\tAttempting to play sound: %s\n\n",
                            componentName.ToCStr(), soundThemeName.ToCStr(),
                            soundEventName.ToCStr(), soundName.ToCStr());
        */
#endif

        pSoundManager->PlaySound(soundName, 1.f, 0.f);
        return true;
    }

private:
    Ptr<GameSoundManager>  pSoundManager;
    Ptr<GFx::Log>            pLog;
};

#endif


// ********************************************************************
// Text clipboard interface
//
#if defined(SF_OS_WIN32)
// Install system-specific clipboard implementation on Win32. If this is not done
// the clipboard will still work in FxPlayer, but it will be impossible to paste
// text to external applications.
class MenuKitTextClipboard : public GFx::Clipboard
{
public:
    void OnTextStore(const wchar_t* ptext, UPInt len)
    {
        // store the text in a system clipboard.
        if (OpenClipboard(NULL))
        {
            // Empty the Clipboard. This also has the effect
            // of allowing Windows to free the memory associated
            // with any data that is in the Clipboard
            EmptyClipboard();

            HGLOBAL hClipboardData;
            hClipboardData = GlobalAlloc(GMEM_DDESHARE, (len+1)*sizeof(wchar_t));
            SF_ASSERT(sizeof(wchar_t) == 2);

            // Calling GlobalLock returns a pointer to the 
            // data associated with the handle returned from 
            // GlobalAlloc
            wchar_t * pchData;
            pchData = (wchar_t*)GlobalLock(hClipboardData);
            SFwcscpy(pchData, len+1, ptext);

            // Once done, unlock the memory. 
            // don't call GlobalFree because Windows will free the 
            // memory automatically when EmptyClipboard is next 
            // called. 
            GlobalUnlock(hClipboardData);

            // Now, set the Clipboard data by specifying that 
            // Unicode text is being used and passing the handle to
            // the global memory.
            SetClipboardData(CF_UNICODETEXT, hClipboardData);

            // Finally, close the Clipboard which has the effect of 
            // unlocking it so that other  apps can examine or modify its contents.
            CloseClipboard();
        }
    }
};
#endif



//////////////////////////////////////////////////////////////////////////
#ifdef GFX_ENABLE_SOUND
void    GameResourceManager::Initialize(Platform::RenderThread* prenThread, Sound::SoundRenderer* psndrenderer)
#else
void    GameResourceManager::Initialize(Platform::RenderThread* prenThread)
#endif
{
    pRenderThread = prenThread;

#ifdef GFX_ENABLE_SOUND
    pSoundRenderer = psndrenderer;
#endif
    
    pTaskManager = *new GFx::ThreadedTaskManager;
#if defined(SF_OS_XBOX360)
    pTaskManager->AddWorkerThreads(GFx::Task::Type_IO, 3, 128 * 1024, 1);
#endif
   
#if defined(SF_OS_WIN32)
    pTextClipboard = *new MenuKitTextClipboard();
#else
    pTextClipboard = 0;
#endif // SF_OS_WIN32

#ifdef GFX_ENABLE_VIDEO
#if defined(SF_OS_WIN32)
    UInt32 affinityMasks[] = {
        DEFAULT_VIDEO_DECODING_AFFINITY_MASK,
        DEFAULT_VIDEO_DECODING_AFFINITY_MASK,
        DEFAULT_VIDEO_DECODING_AFFINITY_MASK
    };
    pVideoState = *new Video::VideoPC(Video::VideoVMSupportAll(),
                                      Thread::NormalPriority, MAX_VIDEO_DECODING_THREADS, affinityMasks);
#elif defined(SF_OS_XBOX360)
    pVideoState = *new Video::VideoXbox360(Video::VideoVMSupportAll(),
                                           Video::Xbox360_Proc3|Video::Xbox360_Proc4|Video::Xbox360_Proc5,
                                           Thread::NormalPriority, Video::Xbox360_Proc2);
#elif defined(SF_OS_PS3)
    UInt8 spursPrios[] = {0, 0, 1, 1, 1, 0, 0, 0};
    // TODO: GetSpurs() - add SPUs support
    pVideoState = *new Video::VideoPS3(Video::VideoVMSupportAll(),
                                       NULL/*GetSpurs()*/, 2, Thread::NormalPriority, 0, spursPrios);
#else
    pVideoState = *new Video::Video(Video::VideoVMSupportAll(), Thread::NormalPriority);
#endif

#ifdef GFX_VIDEO_USE_SYSTEM_SOUND
#if   defined(SF_OS_WIN32)
    pVideoState->SetSoundSystem(Ptr<Video::VideoSoundSystem>(*new Video::VideoSoundSystemDX8(0)));
#elif defined(SF_OS_XBOX360)
    pVideoState->SetSoundSystem(Ptr<Video::VideoSoundSystem>(*new Video::VideoSoundSystemXA2(0,0)));
#elif defined(SF_OS_PS3)
    // TODO: GetSpurs() - add SPUs support
    pVideoState->SetSoundSystem(Ptr<Video::VideoSoundSystem>(*new Video::VideoSoundSystemPS3(true, NULL/*GetSpurs()*/)));
#elif defined(SF_OS_MAC)
    pVideoState->SetSoundSystem(Ptr<Video::VideoSoundSystem>(*new Video::VideoSoundSystemMac()));
#endif
#endif // GFX_VIDEO_USE_SYSTEM_SOUND

    pVideoState->SetReadCallback(this);
#endif // GFX_ENABLE_VIDEO

#ifdef GFX_ENABLE_SOUND
    if (pSoundRenderer)
    {
        pAudioState = *new Audio(pSoundRenderer);
        pSoundRenderer->SetMasterVolume(1.0f);
        pSoundRenderer->Mute(false);
#if defined(GFX_ENABLE_VIDEO) && !defined(GFX_VIDEO_USE_SYSTEM_SOUND)
        pVideoState->SetSoundSystem(pSoundRenderer);
#endif
    }
#endif // GFX_ENABLE_SOUND

    pFileOpener = *new MenuKitFileOpener(pMemoryFileCache);
    pPlayerLog = *new GFxPlayerLog();

#ifdef GFX_ENABLE_XML
    Ptr<GFx::XML::Parser> pexpatXmlParser = *new GFx::XML::ParserExpat;
    pXmlSupport = *new GFx::XML::Support(pexpatXmlParser);
#endif
    bInitialized = true;
}

void    GameResourceManager::InitLoader(GFx::Loader& loader)
{
    loader.SetFileOpener(pFileOpener);

    // Uncomment this line and add libpng to linker's settings to enable
    // PNG support.
    Scaleform::Ptr<GFx::ImageFileHandlerRegistry> pimgReg = *new Scaleform::GFx::ImageFileHandlerRegistry(Scaleform::GFx::ImageFileHandlerRegistry::AddDefaultHandlers);
    loader.SetImageFileHandlerRegistry(pimgReg);

    Ptr<GFx::ImageCreator> pimageCreator = 
        *new ImageCreator(GetRenderThread()->GetTextureManager());
    loader.SetImageCreator(pimageCreator);

    Ptr<Scaleform::GFx::ASSupport> pAS2Support = *new Scaleform::GFx::AS2Support();
    loader.SetAS2Support(pAS2Support);
    Ptr<Scaleform::GFx::ASSupport> pAS3Support = *new Scaleform::GFx::AS3Support();
    loader.SetAS3Support(pAS3Support);

    // Task Manager.
    // loader.SetTaskManager(pTaskManager);
#ifdef GFX_ENABLE_SOUND
    loader.SetAudio(pAudioState);
#endif

#ifdef GFX_ENABLE_VIDEO
    loader.SetVideo(pVideoState);
#endif

    loader.SetLog(pPlayerLog);
#ifdef SF_OS_WIN32    
    loader.SetClipboard(Ptr<GFx::Clipboard>(pTextClipboard));
#endif    

    loader.SetUserEventHandler(pUserEventHandler);

    // ------------------------------

#ifdef GFX_ENABLE_XML
    loader.SetXMLSupport(pXmlSupport);
#endif
// PPS: IME is too coupled with fxplayer... Investigate a way to rip it out
    /*
    //!Create IMEManager after the application window has been set up since 
    // IME manager needs valid window handle as parameter.
    GFxIMEManager* pimemanager = IMEHelper::CreateManager(this);
    if (pimemanager)
    {
        // For error logging. If error logging not desired, just pass null to Init. User must 
        // call Init though!
        pimemanager->Init(Loader.GetLog());
        pimemanager->SetIMEMoviePath("IME.swf");
        Loader.SetIMEManager(pimemanager);
        // Loader keeps the object from this point
        pimemanager->Release();
    }

    if (pimemanager)
    {
GASSERT(0);
//        if(pMovie)
//            pMovie->HandleEvent(GFxEvent::SetFocus);
    }
    */

    // TODO
}


void    GameResourceManager::Shutdown()
{
    // If we started worker threads in the renderer we must end them here;
    // otherwise FinishAllThreads will lock up waiting indefinitely.
    pTaskManager->RequestShutdown();    
}


GFx::MovieDef*    GameResourceManager::LoadMovieDef(GFx::Loader* loader, const char* filename)
{
    GFx::MovieInfo newMovieInfo;
    GFx::MovieDef* pnewMovieDef;

    // Get info about the width & height of the movie.
    if (!loader->GetMovieInfo(filename, &newMovieInfo, 0,GFx::Loader::LoadKeepBindData)) 
    {
        fprintf(stderr, "Error: MenuResourceManager failed to get info about %s\n", filename);
        return 0;
    }

    unsigned loadConstants = GFx::Loader::LoadAll;

    // Load the actual new movie and crate instance.
    // Don't use library: this will ensure that the memory is released.
    pnewMovieDef = loader->CreateMovie(filename, loadConstants, 0);
    /*
        | GFx::Loader::LoadKeepBindData
        | GFx::Loader::LoadWaitFrame1); */
    if (!pnewMovieDef) 
    {
        fprintf(stderr, "Error: MenuResourceManager failed to create a movie from '%s'\n", filename);
        return 0;
    }

    return pnewMovieDef;
}

void    GameResourceManager::OnReadRequested()
{
    // Video reader needs disk I/O; pause everything else
    //SF_DEBUG_MESSAGE(1, "VIDEO: read request\n");
    if (pResourceAccessListener)
        pResourceAccessListener->OnIoLock();
}

void    GameResourceManager::OnReadCompleted()
{
    // Video reader has yielded disk I/O; resume everything else
    // SF_DEBUG_MESSAGE(1, "VIDEO: read complete\n");
    if (pResourceAccessListener)
        pResourceAccessListener->OnIoUnlock();
}

#ifdef GFX_ENABLE_SOUND
void    GameResourceManager::RegisterSoundEventBank(GameUISwfMovie* movie, GameSoundManager* psndMgr)
{
    GFx::Movie* pmovie = movie->GetSwf();
    SF_ASSERT(pmovie && psndMgr);

    GFx::Value soundFunc;
    Ptr<MenuKitSoundEventCallback> soundEventCallback = *SF_HEAP_NEW(Memory::GetGlobalHeap()) MenuKitSoundEventCallback(pmovie, psndMgr);
    pmovie->CreateFunction(&soundFunc, soundEventCallback);

    if (pmovie->GetAVMVersion() == 1)
    {
        GFx::Value _global;
        pmovie->GetVariable(&_global, "_global");
        if (!_global.IsUndefined())
        {            
            _global.SetMember("gfxProcessSound", soundFunc);
        }
    }
    else
    {
        GFx::Value arg("scaleform.gfx.Extensions");
        GFx::Value clikClass;
        pmovie->Invoke("flash.utils.getDefinitionByName", &clikClass, &arg, 1);
        SF_ASSERT(clikClass.IsObject());

        clikClass.SetMember("CLIK_playSound", soundFunc);
    }
}
#endif // GFX_ENABLE_SOUND

String  GameResourceManager::BuildPath(const char* pfilename)
{
    String path;
#if defined(SF_OS_XBOX360)
    path = "D:\\";
#elif defined(SF_OS_PS3)
    path = "/app_home/";
#elif defined(SF_OS_WIIU)
    path = "/vol/content/Kits/Menu/";
#endif
    path.AppendString(pfilename);
    return path;
}
