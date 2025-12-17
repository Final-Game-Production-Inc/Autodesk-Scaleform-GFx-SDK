/**********************************************************************

PublicHeader:   Render
Filename    :   Platform.cpp
Content     :   
Created     :   Oct 2011
Authors     :   Bart Muzzin

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#include "Platform/Platform.h"

namespace Scaleform { namespace Platform { 

AppImplBase::AppImplBase(AppBase* papp) :
    pApp(papp),
    NextUpdateTicks(0),
    MousePos(0,0),
    FrameTime(1.0f/60.0f),
    FrameTimeTicks(1000000 / 60)
{
}

AppImplBase::~AppImplBase()
{

}

void AppBase::InitArgDescriptions(Args* args)
{
    pImplBase->InitArgDescriptions(args);
}

void AppImplBase::InitArgDescriptions(Args* args)
{
    ArgDesc options[] =
    {
        {"help",  "Help",           Args::Flag,         "",   "Print command line options.",},
        {"print", "Print",          Args::Flag,         "",   "Print argument values after command line parsing."},
        {"mt",    "MultiThreaded",  Args::Flag,         "",   "Force multi-threaded rendering."},
        {"st",    "SingleThreaded", Args::Flag,         "",   "Force single-threaded rendering."},
        {"f",     "FullScreen",     Args::Flag,         "",   "Run in full-screen mode."},
        {"sy",    "VSync",          Args::Flag,         "",   "Enable vertical synchronization."},
        {"fmt",   "BufferFormat",   Args::StringOption,  0,   "Requested backbuffer format, in the form: C<bits>D<bits>S<bits>, the default "
                                                              "(if available) is C32D24S8. The requested buffer format is not available, the "
                                                              "default for the Platform will be used."},
        {"sw",    "SoftwareRender", Args::Flag,         "",   "Creates a software renderer instead of a hardware one (if available)."},
        {"wd",    "WatchDog",       Args::Flag,         "",   "Runs the watchdog thread, and crashes the application if it is not serviced."},
        {"",      "",               Args::ArgEnd,       "",   ""}
    };
    args->AddDesriptions(options);
}

void AppBase::SetFrameTime(float seconds)
{
    return pImplBase->SetFrameTime(seconds);
}

void AppImplBase::SetFrameTime(float seconds)
{
    if (seconds < 0.0f)
        seconds = 0.0f;
    FrameTime = seconds;
    FrameTimeTicks = 1000 * (UInt64)(FrameTime * 1000.0f);
    //
}

float AppBase::GetFrameTime() const
{
    return pImplBase->GetFrameTime();
}

float AppImplBase::GetFrameTime() const
{
    return FrameTime;
}

const Args& AppBase::GetArgs() const
{
    return pImplBase->GetArgs();
}

const Args& AppImplBase::GetArgs() const
{
    return Arguments;
}

ThreadingType AppBase::GetArgsThreadingType() const
{
    return pImplBase->GetArgsThreadingType();
}

ThreadingType AppImplBase::GetArgsThreadingType() const
{
    unsigned tt = TT_AutoDetect;
    if (Arguments.GetBool("SingleThreaded"))
        tt = TT_SingleThreaded;
    else if (Arguments.GetBool("MultiThreaded"))
        tt = TT_MultiThreaded;

    if (Arguments.GetBool("WatchDog"))
        tt |= TT_WatchDogFlag;

    return (ThreadingType)tt;
}

void AppBase::SetWindowTitle(const String& title)
{
    pImplBase->SetWindowTitle(title);
}

const String& AppBase::GetWindowTitle() const
{
    return pImplBase->GetWindowTitle();
}

void AppImplBase::SetWindowTitle(const String& title)
{
    Title = title;    
}

const String& AppImplBase::GetWindowTitle() const
{
    return Title;
}

bool AppBase::OnArgs(const Args& args, Args::ParseResult parseResult)
{
    return pImplBase->OnArgs(args, parseResult);
}

bool AppImplBase::OnArgs(const Args& args, Args::ParseResult parseResult)
{
    if (parseResult != Args::Parse_Success)
    {        
        args.PrintOptionsHelp();
        return false;
    }

    if (args.GetBool("Print"))
        args.PrintArgValues();
    return true;
}

void AppBase::ApplyViewConfigArgs(ViewConfig* config, const Args& args)
{
    pImplBase->ApplyViewConfigArgs(config, args);
    config->CommandLineArgs = &args;
}

// Applies default command line arguments to ViewConfig.
void AppImplBase::ApplyViewConfigArgs(ViewConfig* config, const Args& args)
{
    if (args.GetBool("FullScreen"))
        config->ViewFlags |= View_FullScreen;
    if (args.GetBool("VSync"))
        config->ViewFlags |= View_VSync;
    if (args.GetBool("SoftwareRender"))
        config->ViewFlags |= View_SoftwareRendering;

    // Parse the buffer format.
    if (args.HasValue("BufferFormat"))
    {
        static const int MaxNumberChars = 16;
        String fmtStr = args.GetString("BufferFormat");
        const char * strPtr = fmtStr.ToCStr();
        char numberBuffer[MaxNumberChars];
        int numberIndex = 0;
        int field = 0;
        while (*strPtr)
        {
            if (isdigit(*strPtr))
            {
                SF_DEBUG_ASSERT(numberIndex < MaxNumberChars, "Buffer bitcount too high.");
                numberBuffer[numberIndex++] = *strPtr;
            }

            if (!isdigit(*strPtr) || !*(strPtr+1))
            {
                numberBuffer[numberIndex] = 0;
                numberIndex = 0;
                switch(field)
                {
                case 0: break; // initialization
                case 'C': config->ColorBits   = (int)atoi(numberBuffer); break;
                case 'D': config->DepthBits   = (int)atoi(numberBuffer); break;
                case 'S': config->StencilBits = (int)atoi(numberBuffer); break;
                default:
                    SF_DEBUG_WARNING1(1, "Unrecognized buffer field format: %c", field);
                    break;
                }

                if (*strPtr)
                    field = SFtoupper(*strPtr);
            }
            strPtr++;
        } 
    }
}

void AppBase::GetViewConfig(ViewConfig* config) const
{
    *config = pImplBase->getConfig();
}

Size<unsigned> AppBase::GetViewSize() const
{
    return pImplBase->getConfig().ViewSize;
}

bool AppBase::ViewConfigHasFlag(ViewFlagConstants flag)
{
    return pImplBase->getConfig().HasFlag(flag);
}

const ViewConfig& AppImplBase::getConfig()
{
    // Do an update each time the configuration data is accessed.
    // This may be modified by changing window positions for example.
    updateConfig(); 
    return Config; 
}

bool AppBase::OnInit(ViewConfig& config)
{
    return pImplBase->OnInit(config);
}

bool AppImplBase::OnInit( ViewConfig& config )
{
    if (!pApp->SetupWindow("SF Platform AppBase", config))
        return false;
    return true;
}

void AppBase::OnShutdown()
{
    pImplBase->killWindow();
}

Point<int> AppBase::GetMousePos() const
{
    return pImplBase->GetMousePos();
}

Point<int> AppImplBase::GetMousePos() const
{
    return MousePos;
}

// Creates a window & initializes the application class.
// Returns 0 if window creation/initialization failed (the app should quit).
bool AppBase::SetupWindow(const String& title, const ViewConfig& viewConfig)
{
    return pImplBase->setupWindow(title, viewConfig);
}


//---------------------------------------------------------------------------------------
DirectoryImplBase::DirectoryImplBase(const String& path, const String& pattern)
: Path(path), Pattern(pattern), CurFile(-1)
{

}

DirectoryImplBase::~DirectoryImplBase()
{

}

bool DirectoryImplBase::GetNextFile(String* filename, bool prev)
{	
    if ( !filename )
        return false;

    checkDirectoryChange(*filename);
    if (CurFile >= 0 && CurFile < int(Filenames.GetSize()))
    {
        CurFile += (prev ? -1 : 1);
        if (CurFile < 0)
            CurFile = int(Filenames.GetSize() - 1);
        else if (CurFile >= int(Filenames.GetSize()))
            CurFile = 0;
        *filename = Path + Filenames[CurFile];
        return true;
    } 
    else {
        Filenames.Clear();
        return false;
    }
}

void DirectoryImplBase::SetCurrentFile(const String& filename)
{
    checkDirectoryChange(filename);
    CurFile = GetFileIndex(filename);
}

int DirectoryImplBase::GetFileIndex(const String& filename) const
{
    String fileOnly = filename.GetFilename();
    for ( unsigned i = 0; i < (unsigned)Filenames.GetSize(); ++i )
    {
        if ( Filenames[i] == fileOnly )
            return i;
    }
    return -1;
}

const char * DirectoryImplBase::GetFileAtIndex(unsigned index) const
{
    if ( index < (unsigned)Filenames.GetSize() )
        return Filenames[index].ToCStr();
    return 0;
}

void DirectoryImplBase::checkDirectoryChange(const String& filename)
{
    String newPath = filename.GetPath();
    if (!newPath.IsEmpty() && newPath != Path)
    {
        Path = newPath;
        Filenames.Clear();
    }

    if (Filenames.GetSize() == 0) 
    {
        ReadDirectory(Path, Pattern);
    }
}

//---------------------------------------------------------------------------------------
bool Directory::GetNextFile(String* pfilename, bool order)
{
    return pImpl->GetNextFile(pfilename, order);
}

void Directory::ReadDirectory(const String& path, const String& pattern)
{
    pImpl->ReadDirectory(path, pattern);
}

void Directory::SetCurrentFile(const String& filename)
{
    return pImpl->SetCurrentFile(filename);
}

int Directory::GetFileIndex(const String& filename) const
{
    return pImpl->GetFileIndex(filename);
}
const char * Directory::GetFileAtIndex(unsigned index) const
{
    return pImpl->GetFileAtIndex(index);
}

}} // Scaleform::Platform
