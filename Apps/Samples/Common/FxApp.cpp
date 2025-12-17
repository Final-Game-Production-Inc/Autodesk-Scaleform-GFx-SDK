/**************************************************************************

Filename    :   FxApp.cpp
Content     :   Base FxApp class implementation.
Created     :   Jan, 2008
Authors     :   Maxim Didenko, Dmitry Polenur

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "FxApp.h"
#include <stddef.h>


FxApp* FxApp::pApp = NULL;

FxApp::FxApp() 
{ 
    Created     = 0;
    Active      = 0;
    QuitFlag    = 0;
    LockOnSize  = 0;
    ExitCode    = 0;

    Console     = false;

    // Requested 3D state
    FullScreen  = 0;
    FSAntialias = 0;
    BitDepth    = 0;
    VSync       = 0;

    SupportDropFiles    = 0;
    SizableWindow = 0;

    Width       = 0;
    Height      = 0;

    // Set later on in CreateWindow
    FSAASupported= 0;

    VMCFlags = 0;

    // No old pos, save during FullScreen mode
    OldWindowX = OldWindowY = 0;
    OldWindowWidth = OldWindowHeight = 0;

    // Cursor variables
    CursorHidden  = 0;
    CursorHiddenSaved = 0;
    CursorIsOutOfClientArea = 0;
    CursorDisabled = 0;

    pApp = this; 
}

FxApp::~FxApp() 
{
}

bool FxApp::Init()
{
    return true;
}

void FxApp::InitArgDescriptions( FxArgs* args )
{
    FxArgDesc options[] =
    {
        {"help",  "Help",       FxArgs::Flag,      "",   "Print command line options.",},
        {"print", "Print",      FxArgs::Flag,      "",   "Print argument values after command line parsing."},
        {"f",     "FullScreen", FxArgs::Flag,      "",   "Run in full-screen mode."},
        {"sy",    "VSync",      FxArgs::Flag,      "",   "Enable vertical synchronization."},
        {"b",     "BitDepth",   FxArgs::IntOption, "32", "<bits>   Bit depth of output window (16 or 32, default is 32)."},
        {"",      "",           FxArgs::ArgEnd,    "",  ""}
    };
    args->AddDesriptions(options);
}

bool FxApp::OnArgsHelp(const FxArgs& args)
{
    args.PrintOptionsHelp();
    return false;
}

bool FxApp::OnArgsError(const FxArgs& args)
{
    args.PrintOptionsHelp();
    return false;
}

bool FxApp::OnArgsParsed(const FxArgs& args)
{
    if(!Console)
        FullScreen   = args.GetBool("FullScreen");
    else
        FullScreen = 1;

    VSync        = args.GetBool("VSync");
    BitDepth     = args.GetInt("BitDepth");
    
    if(args.GetBool("Print"))
        args.PrintArgValues();
    return true;
}

int FxApp::Run()
{
    return 0;
}

bool FxApp::Exit()
{
    return true;
}

void FxApp::ResetCursor()
{
    CursorIsOutOfClientArea = 0;
    CursorHidden = 0;
    CursorHiddenSaved = 0;

    if (!CursorDisabled)
        ShowCursorInstantly(true);
    else
    {
        CursorHidden = 1; //?
        ShowCursorInstantly(false);
    }
}

void FxApp::ShowCursor(bool show)
{
    if (CursorDisabled)
        return;
    if (show)
    {
        if (CursorHidden)
        {
            CursorHidden = !CursorHidden;
            if (!CursorIsOutOfClientArea)
                ShowCursorInstantly(true);
        }
    }
    else
    {
        if (!CursorHidden)
        {
            CursorHidden = !CursorHidden;
            if (!CursorIsOutOfClientArea)
                ShowCursorInstantly(false);
        }
    }
}

int FxApp::AppMain( int argc, char* argv[] )
{
    int ret = -1;
    if (Init())
    {
        bool cont_prog = true;
        InitArgDescriptions(&Arguments);
        switch (Arguments.ParseCommandLine(argc, argv))
        {
        case -1:
            cont_prog = OnArgsHelp(Arguments);
            break;
        case 0:
            cont_prog = OnArgsParsed(Arguments);
            break;
        default:
            cont_prog = OnArgsError(Arguments);
            break;
        }

        if (!cont_prog)
            return -1;
        ret = Run();
        Exit();
    }
    return ret;
}

