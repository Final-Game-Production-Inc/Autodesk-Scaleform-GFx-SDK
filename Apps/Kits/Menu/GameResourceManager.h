/**************************************************************************

Filename    :   GameResourceManager.h
Content     :   Sample resource manager for Menu Kit
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GameResourceManager_H
#define INC_GameResourceManager_H

// Includes
#include "Kernel/SF_RefCount.h"
#include "Render/Renderer2D.h"
#include "GFx/GFx_Loader.h"
#include "GFx/GFx_TaskManager.h"
#include "GFx/XML/XML_Support.h"

#include "FxPlayerLog.h"
#include "FxRenderThread.h"

#ifdef GFX_ENABLE_VIDEO
#include "Video/Video_Video.h"
#endif

#ifdef GFX_ENABLE_SOUND
#include "Sound/Sound_SoundRenderer.h"
#include "Sound/Sound_SoundEvent.h"
#include "Sound/Sound_SoundEventFMOD.h"
#include "FxSoundFMOD.h"
#endif

#include "GameUIManager.h"
#include "Util/GameDataLoader.h"

using namespace Scaleform;
using namespace GFx;

// Forward declarations
namespace Scaleform { namespace GFx { 
    class MovieDef;
}}

class GameSoundManagerImpl;
class GameResourceManager;

class CellSpurs;

// ********************************************************************
// Sound manager
//
class GameSoundManager : public RefCountBase<GameSoundManager, Stat_Default_Mem>
{
public:
    GameSoundManager(GameResourceManager* presMgr, const String& pathPrefix);
    virtual ~GameSoundManager();

    void    LoadSound(const char* id, const char* source);
    bool    PlaySound(const char* id, float volume = 1.0f, float pan = 0.f);

private:
    GameSoundManagerImpl*   pImpl;
    String                  PathPrefix;
};


// ********************************************************************
// Resource manager. Provides an interface to load game assets.
//
#ifdef GFX_ENABLE_VIDEO
class GameResourceManager : public Video::VideoBase::ReadCallback
#else
class GameResourceManager
#endif
{
public:

    // ** Resource access listener
    //
    // This listener is notified when IO locking is requested
    // Useful to prioritize high priority resource access (video reader, etc.) over lower 
    // priority loading (game data, etc.)
    //
    class   AccessListener
    {
    public:
        virtual ~AccessListener()       {}
        virtual void    OnIoLock()      = 0;
        virtual void    OnIoUnlock()    = 0;
    };

    GameResourceManager() : bInitialized(false), pResourceAccessListener(NULL)
    {
        pMemoryFileCache = *SF_HEAP_NEW(Memory::GetGlobalHeap()) GameMemoryFileCache();
    }
    virtual ~GameResourceManager()  {}

#ifdef GFX_ENABLE_SOUND
    void                Initialize(Platform::RenderThread* prenThread, Sound::SoundRenderer* psndrenderer);
#else
    void                Initialize(Platform::RenderThread* prenThread);
#endif
    void                Shutdown();

    Platform::RenderThread* GetRenderThread() const     { return pRenderThread; }
#ifdef GFX_ENABLE_SOUND
    Sound::SoundRenderer*   GetSoundRenderer() const    { return pSoundRenderer; }
#endif // GFX_ENABLE_SOUND

    // Add a new load task to be executed. Execution will begin immediately
    // if thread pool has inactive workers, else will be deferred until one
    // is freed up.
    // void                AddLoadTask(ResourceLoadTask* task);
    // void                AbandonTask(ResourceLoadTask* task);

    // *** GFx specific methods
    //
    // Helper to initialize GFxLoader objects
    void                InitLoader(GFx::Loader& loader);
    // Helper to load a GFxMovieDef from resource library
    GFx::MovieDef*      LoadMovieDef(GFx::Loader* loader, const char* filename);
#ifdef GFX_ENABLE_SOUND
    // Register a sound event manager with a movie.
    void                RegisterSoundEventBank(GameUISwfMovie* movie, GameSoundManager* psndMgr);
#endif
    // Accessors for specific resource subsystems
    GFx::Clipboard*     GetTextClipboard() const    { return pTextClipboard; }

    String             BuildPath(const char* pfilename);
    
    GameMemoryFileCache*    GetMemoryFileCache() const  { return pMemoryFileCache; }

    void                SetUserEventHandler(GFx::UserEventHandler* handler)
    {
        pUserEventHandler = handler;
    }
    void                SetResourceAccessListener(AccessListener* listener)
    {
        pResourceAccessListener = listener;
    }

    // ** Video state callbacks
    //
    // Notify then a video read operation is required. Upon receiving this notification
    // the application should stop all its disk IO operations (and wait until they are really
    // finished). After returning form this method the video system will start file read 
    // operation immediately.
    virtual void        OnReadRequested();
    // Notify then the video read operation has finished. Upon receiving this notification 
    // the application can resume its disk IO operations until it receives OnReadRequested 
    // notification.
    virtual void        OnReadCompleted();

private:
    bool                            bInitialized;

    Ptr<Platform::RenderThread>     pRenderThread;
    Ptr<GFx::Clipboard>             pTextClipboard;
    Ptr<GFx::ThreadedTaskManager>   pTaskManager;
#ifdef GFX_ENABLE_VIDEO
    Ptr<Video::Video>               pVideoState;
#endif
#ifdef GFX_ENABLE_SOUND
    Ptr<GFx::Audio>                 pAudioState;
    Sound::SoundRenderer*           pSoundRenderer;
#endif
    Ptr<GFx::FileOpener>            pFileOpener;
    Ptr<GFxPlayerLog>               pPlayerLog;
#ifdef GFX_ENABLE_XML
    Ptr<GFx::XML::SupportBase>      pXmlSupport;
#endif
    Ptr<GFx::UserEventHandler>      pUserEventHandler;

    AccessListener*                 pResourceAccessListener;

    Ptr<GameMemoryFileCache>        pMemoryFileCache;

    
};

#endif  // INC_GameResourceManager_H
