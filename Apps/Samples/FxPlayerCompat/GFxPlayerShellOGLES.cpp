/*!****************************************************************************
 @File          GFxPlayerShellOGLES.cpp

 @Title         Scaleform GFx Player on PVRShell

 @Author        PowerVR

 @Date          02/03/2006

 @Copyright     Copyright 2006 by Imagination Technologies Limited.
                All rights reserved. No part of this software, either
                material or conceptual may be copied or distributed,
                transmitted, transcribed, stored in a retrieval system
                or translated into any human or computer language in any
                form by any means, electronic, mechanical, manual or
                other-wise, or disclosed to third parties without the
                express written permission of Imagination Technologies
                Limited, Unit 8, HomePark Industrial Estate,
                King's Langley, Hertfordshire, WD4 8LZ, U.K.

 @Platform      Independant

 @Description   
******************************************************************************/
#define GFC_RENDERER_TEST_CACHEDATA 1
/**************************************************************************

Filename    :   GFxPlayerShellOGLES.cpp

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#define PRINT_FPS

#ifdef USE_OPENGL_RENDERER
#include <windows.h>
#include "OGLTools.h"
#else
#include "OGLESTools.h"
#endif

#include "PVRShell.h"
//#include "PVRTPrint3D.h"

#include "GFile.h"
#include "GFxPlayer.h"
#include "GFxLoader.h"
#include "GFxLog.h"
#include "GTimer.h"

#ifdef USE_OPENGL_RENDERER
#ifdef GL_X_RENDERER
#include "GRendererOGLx.h"
#else
#include "GRendererOGL.h"
#endif
#else
#include "GRendererOGLES.h"
#endif
//#include "GLES/gl.h"


/**************************************************************************

Filename    :   GFxPlayerShellOGLES.cpp

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

/**************************************************************************

Filename    :   GFxPlayerShellOGLES.cpp

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
class CGFxPlayerOGLES : public PVRShell
{
    /* Print3D class */
    CPVRTPrint3D            m_Print3D;

    GFxLoader               m_Loader;
    GFxMovieInfo            m_MovieInfo;
    GPtr<GFxMovieDef>       m_pMovieDef;
    GPtr<GFxMovieView>      m_pMovie;
#ifdef USE_OPENGL_RENDERER
    GPtr<GRendererOGL>      m_pRenderer;
#else
    GPtr<GRendererOGLES>    m_pRenderer;
#endif

    UInt64 m_StartTime,
    m_ui32LastFrameTime,
    m_ui32FramesSinceLastFrameCount,
    m_ui32LastFrameCountTime;

    const char* m_pszFilename;

    bool bVerbose;
    bool bLeftKeyDown;
    bool bRightKeyDown;
    bool bUpKeyDown;
    bool bDownKeyDown;

    bool bAutoExit;

    float m_fPixelError;

public:
    CGFxPlayerOGLES() {}

    /* PVRShell functions */
    virtual bool InitApplication();
    virtual bool InitView();
    virtual bool ReleaseView();
    virtual bool QuitApplication();
    virtual bool RenderScene();

/**************************************************************************

Filename    :   GFxPlayerShellOGLES.cpp

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

    // *** Static callbacks

    // FileOpen callback function used by loader
    static GFile* FileOpener(const char* pszUrl);
    // "fscommand" callback, handles notification callbacks from ActionScript.
    static void FsCallback(GFxMovieView* pMovie, const char* pszCommand, const char* pszArgs);
};

class GFxPlayerLog : public GFxLog
{
public: 
    // We override this function in order to do custom logging.
    virtual void    LogMessageVarg(LogMessageType messageType, const char* pfmt, va_list argList)
    {
        // Output log to console
        vprintf(pfmt, argList);
    }
};


/**************************************************************************

Filename    :   GFxPlayerShellOGLES.cpp

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
bool CGFxPlayerOGLES::InitApplication()
{
    PVRShellSet(prefSwapInterval, 0);
    PVRShellSet(prefZbufferContext, true);

    m_pszFilename = NULL;
    bVerbose=false;

    m_StartTime = GTimer::GetTicks()/1000;
    bAutoExit = 1;

    // Get filename to open from command line options
    int i32NumCmdLineOpts = PVRShellGet(prefCommandLineOptNum);
    const SCmdLineOpt* psCmdLineOpts = (const SCmdLineOpt*)PVRShellGet(prefCommandLineOpts);
    for(int i = 0; i < i32NumCmdLineOpts; ++i)
    {
        if(strcmp(psCmdLineOpts[i].pArg, "-swf") == 0)
        {
            m_pszFilename = psCmdLineOpts[i].pVal;
            //break;
        }
        
        if(strcmp(psCmdLineOpts[i].pArg, "-pixerror") == 0)
        {
            m_fPixelError = atof(psCmdLineOpts[i].pVal);
            if(m_fPixelError > 16) m_fPixelError = 16;
            if(m_fPixelError < 0.5) m_fPixelError = 0.5;
            //break;
        }

        if(strcmp(psCmdLineOpts[i].pArg, "-verbose") == 0)
        {
            bVerbose=true;
            //break;
        }

        if (strcmp(psCmdLineOpts[i].pArg, "-noexit") == 0)
        {
            bAutoExit = 0;
        }
    }
    if(!m_pszFilename)
    {
        PVRShellOutputDebug("Use -swf=<Flash file> command line option.");
        return false;
    }

    return true;
}

/**************************************************************************

Filename    :   GFxPlayerShellOGLES.cpp

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
bool CGFxPlayerOGLES::QuitApplication()
{
    m_Loader.ClearLibrary();
    GMemory::DetectMemoryLeaks();
    return true;
}

/**************************************************************************

Filename    :   GFxPlayerShellOGLES.cpp

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
bool CGFxPlayerOGLES::InitView()
{
    UByte       verboseFlags = 0;    

    // Create renderer
#ifdef USE_OPENGL_RENDERER
    m_pRenderer = *GRendererOGL::CreateRenderer();
    m_pRenderer->SetDependentVideoMode((HWND)0);
#else
    m_pRenderer = *GRendererOGLES::CreateRenderer();
    m_pRenderer->SetDependentVideoMode();
#endif

    // Set up loader
    if(!bVerbose)
    {
        m_Loader.SetVerboseParse(verboseFlags);
        m_Loader.SetLog(NULL);
    }
    else
    {
        // Set the verbose flags
        
        verboseFlags |= GFxLoader::VerboseParse;
        verboseFlags |= GFxLoader::VerboseParseShape;
        verboseFlags |= GFxLoader::VerboseParseAction;
        
        m_Loader.SetVerboseParse(verboseFlags);
        m_Loader.SetLog(GPtr<GFxLog>(*new GFxPlayerLog()));
    }
    m_Loader.SetFileOpenCallback(FileOpener);

    // Get movie info from file
    m_Loader.GetMovieInfo(m_pszFilename, &m_MovieInfo);

    // Load movie
    m_pMovieDef = *m_Loader.CreateMovie(m_pszFilename);//, GFxLoader::LoadAll);
    if (!m_pMovieDef)
    {
        PVRShellOutputDebug("Error: failed to create a movie from '%s'\n", m_pszFilename);
        return false;
    }
    //m_pMovieDef->SetLog(NULL);    
    m_pMovieDef->SetLog(GPtr<GFxLog>(*new GFxPlayerLog()));
    m_pMovie = *m_pMovieDef->CreateInstance();  
    if (!m_pMovie)
    {
        PVRShellOutputDebug("Error: failed to create movie instance\n");
        return false;
    }

    // Set up movie for playback
    m_pMovie->SetVerboseAction(bVerbose?1:0);
    //m_pMovie->SetLog(NULL);
    m_pMovie->SetLog(GPtr<GFxLog>(*new GFxPlayerLog()));

    m_pMovie->SetFSCommandCallback(&FsCallback);
    m_pMovie->SetRenderer(m_pRenderer);
    m_pMovie->SetViewport(PVRShellGet(prefWidth), PVRShellGet(prefHeight), 0, 0, PVRShellGet(prefWidth), PVRShellGet(prefHeight));
//  m_pMovie->SetViewport(0, 0, PVRShellGet(prefWidth), PVRShellGet(prefHeight));


    /* Initialize Print3D textures */
    SPVRTContext Context;
    if(!m_Print3D.SetTextures(&Context, PVRShellGet(prefWidth), PVRShellGet(prefHeight)))
    {
        PVRShellOutputDebug ("ERROR: Cannot initialise Print3D\n");
        return false;
    }

    m_ui32LastFrameTime = GTimer::GetTicks()/1000;//PVRShellGetTime();
    m_ui32FramesSinceLastFrameCount = 0;
    m_ui32LastFrameCountTime = m_ui32LastFrameTime;

    m_fPixelError = 4.0f;

    bLeftKeyDown = bRightKeyDown = bUpKeyDown = bDownKeyDown = false;

    if(bVerbose)printf("InitView() END\n");
    return true;
}

/**************************************************************************

Filename    :   GFxPlayerShellOGLES.cpp

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
bool CGFxPlayerOGLES::ReleaseView()
{
    // Release movie and renderer
    m_pMovieDef.Clear();
    m_pMovie.Clear();
    m_pRenderer.Clear();

    /* Release the Print3D textures and windows */
    m_Print3D.DeleteAllWindows();
    m_Print3D.ReleaseTextures();

    return true;
}

/**************************************************************************

Filename    :   GFxPlayerShellOGLES.cpp

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
bool CGFxPlayerOGLES::RenderScene()
{
    
    if(bVerbose)printf("RenderScene()\n");
    static bool bStopped = false;

    UInt64 ui32CurrentFrameTime = GTimer::GetTicks()/1000;//PVRShellGetTime();
    if(!bStopped) m_pMovie->Advance((ui32CurrentFrameTime - m_ui32LastFrameTime) / 1000.0f);
    m_ui32LastFrameTime = ui32CurrentFrameTime;

    if(PVRShellIsKeyPressed(PVRShellKeyNameACTION1))
    {
        m_pMovie->GotoFrame(m_pMovie->GetCurrentFrame() + 1);
        m_pMovie->SetPlayState(GFxMovie::Playing);
    }

    
    if(PVRShellIsKeyPressed(PVRShellKeyNameRIGHT))
    {
        //printf("right DOWN\n");
        //if(!bRightKeyDown)
        {
            /*m_fPixelError = min(16, m_fPixelError + 0.5f);
            m_pMovie->SetMaxCurvePixelError(m_fPixelError);*/
            //using ascii code as 0 for now
            GFxKeyEvent keyEvent(GFxEvent::KeyDown, GFxKey::Right , 0); //do we need to handle keyUp?
            m_pMovie->HandleEvent(keyEvent);
            bRightKeyDown=true;
        }
    }
    else
    {
        //printf("right UP\n");
        //if(bRightKeyDown)
        {
            GFxKeyEvent keyEvent(GFxEvent::KeyUp, GFxKey::Right , 0); //do we need to handle keyUp?
            m_pMovie->HandleEvent(keyEvent);    
            bRightKeyDown=false;
        }

    }

    if(PVRShellIsKeyPressed(PVRShellKeyNameLEFT))
    {
        //printf("left\n");
        //if(!bLeftKeyDown)
        {
            /*
            m_fPixelError = max(0.5f, m_fPixelError - 0.5f);
            m_pMovie->SetMaxCurvePixelError(m_fPixelError);*/
            GFxKeyEvent keyEvent(GFxEvent::KeyDown, GFxKey::Left , 0); //do we need to handle keyUp?
            m_pMovie->HandleEvent(keyEvent);
            bLeftKeyDown=true;
        }
    }
    else
    {
        //if(bLeftKeyDown)
        {
            GFxKeyEvent keyEvent(GFxEvent::KeyUp, GFxKey::Left , 0); //do we need to handle keyUp?
            m_pMovie->HandleEvent(keyEvent);    
            bRightKeyDown=false;
        }

    }

    if(PVRShellIsKeyPressed(PVRShellKeyNameUP))
    {
        GFxKeyEvent keyEvent(GFxEvent::KeyDown, GFxKey::Up , 0);
        m_pMovie->HandleEvent(keyEvent);
    }
    else
    {
        GFxKeyEvent keyEvent(GFxEvent::KeyUp, GFxKey::Up , 0);
        m_pMovie->HandleEvent(keyEvent);
    }

    if(PVRShellIsKeyPressed(PVRShellKeyNameDOWN))
    {
        GFxKeyEvent keyEvent(GFxEvent::KeyDown, GFxKey::Down , 0);
        m_pMovie->HandleEvent(keyEvent);
    }
    else
    {
        GFxKeyEvent keyEvent(GFxEvent::KeyUp, GFxKey::Down , 0);
        m_pMovie->HandleEvent(keyEvent);
    }

    /* before adding up down, move current functionality (see below) to another key */
    //if(PVRShellIsKeyPressed(PVRShellKeyNameUP))
    //{/*
    //  m_fPixelError = max(0.5f, m_fPixelError - 0.5f);
    //  m_pMovie->SetMaxCurvePixelError(m_fPixelError);*/
    //  GFxKeyEvent keyEvent(GFxEvent::KeyDown, GFxKey::Up , 0); //do we need to handle keyUp?
    //  m_pMovie->HandleEvent(keyEvent);
    //}
    //if(PVRShellIsKeyPressed(PVRShellKeyNameDOWN))
    //{/*
    //  m_fPixelError = max(0.5f, m_fPixelError - 0.5f);
    //  m_pMovie->SetMaxCurvePixelError(m_fPixelError);*/
    //  GFxKeyEvent keyEvent(GFxEvent::KeyDown, GFxKey::Down , 0); //do we need to handle keyUp?
    //  m_pMovie->HandleEvent(keyEvent);
    //}

//ok
    if(bVerbose)printf("m_pMovie: %p\nDisplay() begin", (void*)m_pMovie);
    m_pMovie->Display();
//not ok
    if(bVerbose)printf("Display() end\n");

    if((m_pMovie->GetCurrentFrame() + 1 == m_pMovieDef->GetFrameCount()) )// || PVRShellIsKeyPressed(PVRShellKeyNameUP))
    {
        //m_pMovie->Restart();
        m_pMovie->GotoFrame(0);
        m_pMovie->SetPlayState(GFxMovie::Playing);
    }

    //if(PVRShellIsKeyPressed(PVRShellKeyNameDOWN))
    //{
    //  bStopped = !bStopped;
    //}


    //m_Print3D.DisplayDefaultTitle(NULL, NULL, PVR_LOGO);
    //m_Print3D.Flush();

    GRenderer::Stats RenderStats;
    m_pRenderer->GetRenderStats(&RenderStats, 1);
    m_ui32FramesSinceLastFrameCount++;

    unsigned int ui32TimeDiff = GTimer::GetTicks()/1000/*PVRShellGetTime()*/ - m_ui32LastFrameCountTime;
#ifdef PRINT_FPS
    if(ui32TimeDiff > 500)
    {
        PVRShellOutputDebug("Triangles: %d, Lines: %d\n", RenderStats.Triangles, RenderStats.Lines);
        PVRShellOutputDebug("FPS: %f\t\t", 1000.0f * m_ui32FramesSinceLastFrameCount / ui32TimeDiff);
        PVRShellOutputDebug("MaxCurvePixelError: %f\n", m_fPixelError);
        m_ui32FramesSinceLastFrameCount = 0;
        m_ui32LastFrameCountTime = GTimer::GetTicks()/1000;//PVRShellGetTime();
    }
#endif // PRINT_FPS

    if (bAutoExit && m_ui32LastFrameCountTime >= m_StartTime + 20 * 1000)
        return 0;
    
    if(bVerbose)printf("RenderScene() END\n");
    return true;
}

GFile* CGFxPlayerOGLES::FileOpener(const char* pszUrl)
{
    return new GSysFile(pszUrl);
}

// For handling notification callbacks from ActionScript.
void CGFxPlayerOGLES::FsCallback(GFxMovieView* pMovie, const char* pszCommand, const char* pszArgs)
{
    GFxLog* pLog = pMovie->GetLog();
    if(pLog)
    {
        pLog->LogMessage("FsCallback: '");
        pLog->LogMessage(pszCommand);
        pLog->LogMessage("' '");
        pLog->LogMessage(pszArgs);
        pLog->LogMessage("'\n");
    }
}


/**************************************************************************

Filename    :   GFxPlayerShellOGLES.cpp

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
PVRShell* NewDemo()
{
#undef new
    return new CGFxPlayerOGLES();
}

/**************************************************************************

Filename    :   GFxPlayerShellOGLES.cpp

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
