/**************************************************************************

Filename    :   FxHUD.h
Content     :   
                
Created     :   Noveber, 2008
Authors     :   Michael Antonov, Dmitry Polenur, Maxim Didenko, 

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_FXHUD_H
#define INC_FXHUD_H

#include "GFx_Kernel.h"
#include "GFx_Render.h"

#include "Render/Render_TreeText.h"
#include "GFx.h"
#ifdef GFX_ENABLE_DRAWTEXT
#include "GFx/GFx_DrawText.h"
    #ifdef DrawText
        #undef DrawText
    #endif
#endif


#include "../Common/FxRenderThread.h"


namespace SF = Scaleform;
using namespace Scaleform;
using namespace GFx;
//using namespace Render;
class FxHUDDataProvider;

class FxHUD : public RefCountBase<FxHUD, Stat_Default_Mem> 
{

public:
    enum Info
    {
        Performance,
        Memory,
        Summary,
        Help,
        Hidden
    };

    //Use CreateHUD Function to create FxHUD object
    static FxHUD* CreateHUD (FxHUDDataProvider* pdataProvider);

    void ActivateTab(Info);
    void ToggleTab(Info);
    void NextTab();
    void PreviousTab();
    void SetVisible(bool visible);
    bool IsVisible() const { return CurrentTab != Hidden; }
    void Clear();

    void SetViewport(const SF::Render::Viewport& viewDesc);
    void SetStickMode(const int stickMode);
    //void SetRenderer(Render::Renderer* pren); 
    void  Update();

    void SetHUDText(String text, bool bNeedsInit);
    void ClearMsgText();


private:
    FxHUD();
    FxHUD(FxHUDDataProvider* pdataProvider);
    virtual ~FxHUD ();

private:

    bool Init();
    GFx::Loader               mLoader;
    //Ptr<Movie>              pMovie;
    Render::Viewport            mViewport;
    FxHUDDataProvider*          pDataProvider;
    Render::Context             RenderContext;

#ifdef GFX_ENABLE_DRAWTEXT
    Ptr<GFx::DrawTextManager>   pDrawTextManager;
    Ptr<GFx::DrawText>          pHUDText, pMsgText;
#ifdef FXPLAYER_LOGO    
    Ptr<GFx::DrawText>          pLogoText;
#endif
#endif // GFC_NO_DRAWTEXT_SUPPORT

    Info                        CurrentTab;
    Info                        LastTab; // Used to to restore tab after reopening HUD

    static const unsigned       HUD_TEXT_X = 4;
    static const unsigned       HUD_TEXT_Y = 4;
};


struct FxHUDExtraInfo
{
    float           LastFPS;
    unsigned        TessTriangles;
    float           Zoom;
    String          AAMode;
    bool            Paused;
     
    float          CurvePixelError;
    String         StrokeType;
    String         FontConfig;

    unsigned       RasCount;
    Render::Size<unsigned>    AppRes;
};

class FxHUDInfoProvider
{
public:
    virtual ~FxHUDInfoProvider() {}

public:
    virtual void GetExtraInfo(FxHUDExtraInfo* pinfo) = 0;
    virtual void GetHelpStr(String* phelpStr) = 0;
};

class FxHUDDataProvider : public RefCountBase<FxHUDDataProvider, Stat_Default_Mem>
{
public:
    FxHUDDataProvider(Movie* pmovie, FxRenderThread* prenderTread,
        FxHUDInfoProvider* pinfoProvider, bool isConsole);

public:
    void SetMovie(Movie* pmovie);
    bool isMovieLoaded()  const {return pMovie ? 1 : 0;}

    void UpdateRendererStats();
    void UpdatePerformanceStats(bool resetOnly);
    void UpdatePerformanceStats(UInt64 advanceT, UInt64 displayT, float avgFps);

    FxRenderThread* GetRenderThread() const { return pRenderer; }
    void SetRenderThread(FxRenderThread* pt) {pRenderer = pt;}

    void SetStickMode(const int stickMode) { StickMode = stickMode; }
    
    void GetHUDStrings(FxHUD::Info, String* phudString, String* pmsgString);
    bool SaveStatistics(const String& filename, bool xmlFormat, MemoryHeap::MemReportType reportType = MemoryHeap::MemReportHeapDetailed);
    void PrintStatistics(bool xmlFormat, MemoryHeap::MemReportType reportType = MemoryHeap::MemReportHeapDetailed);
    bool IsConsole() const { return Console; }

    //void SetRenderer(Render::Renderer* pren) { pRenderer = pren; }

    unsigned    GetLastRenderMasksCount()   { return LastRenderStats.Masks; }

private:
    struct RenderStats
    {
        unsigned    Triangles;
        unsigned    Lines;
        unsigned    Primitives;
        unsigned    Masks;
        unsigned    Filters;
        unsigned    Meshes;

        RenderStats()       { Clear();}
        void    Clear()     { Triangles = 0; Lines = 0; Primitives = 0; Masks = 0; Filters = 0; Meshes = 0;}

        void    SetFromBag(StatBag* pbag);
        bool    Match(RenderStats& other) const;
    };

    struct PerformanceStats
    {
        UInt64  AdvanceTime;
        UInt64  DisplayTime;
        float   AvgFps;

        PerformanceStats()       { Clear();}
        void    Clear()     { AdvanceTime = 0; DisplayTime = 0; AvgFps = 0.0f; }
        void    SetFromBag(StatBag* pbag);
    };
    String                  HelpStr, MsgStr;
    MemoryHeap*             pTrackerHeap;
    Ptr<FxRenderThread>     pRenderer;
    Ptr<Movie>              pMovie;

    char                    ShortFileName[64];
    FxHUDInfoProvider*      pInfoProvider;
    bool                    Console;

    RenderStats             LastRenderStats; 
    PerformanceStats        LastPerformanceStats;
    Render::MeshCache::Stats LastMeshCacheStats;

    int                             StickMode;
    static const char*              PadStickStrings[];

    void                    GetPerformanceString(StringBuffer& report, bool xmlFormat);
};

#endif // INC_FXHUD_H


