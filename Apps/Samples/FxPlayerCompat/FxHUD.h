/**************************************************************************

Filename    :   FxHUD.h
Content     :   
                
Created     :   Noveber, 2008
Authors     :   Michael Antonov, Dmitry Polenur, Maxim Didenko

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_FXHUD_H
#define INC_FXHUD_H

#include "FxHUDTreeControl.h"

class FxHUDFSCallback;
class FxHUDDataProvider;

//Interface to notify App about Hud events
class FxHUDEventHandler
{
public:
    //Notify app, when Hud becomes active/inactive
    virtual ~FxHUDEventHandler() {}

public:
    virtual void OnHudStatusChange (bool active) = 0;
};

class FxHUD : public GRefCountBase<FxHUD, GStat_Default_Mem> 
{
    friend class FxHUDFSCallback;

public:
    enum Info
    {
        Performance,
        Memory,
        Summary,
        Help,
        Hidden
    };

    enum UpdateFlags
    {
        UpdateText          = 1,
        UpdateGraph         = 2,
        UpdateResize        = 4,
        UpdateAll           = 0xFF
    };
    
    //Use CreateHUD Function to create FxHUD object
    static FxHUD* CreateHUD (FxHUDDataProvider* pdataProvider, FxHUDEventHandler* phudEventHandler);

    void ActivateTab(Info);
    void ToggleTab(Info);
    void NextTab();
    void PreviousTab();
	void SetVisible(bool visible);
    void SetBorder(bool visible);
    bool IsVisible() const { return CurrentTab != Hidden; }
    void Clear();

    void SetViewport(const GViewport& viewDesc);
    void SetRenderer(GRenderer* pren);

    Float Advance(Float deltaT);
    void  Display();
    void  Update();

	bool	GetDirtyFlag() const    { return bDirtyGlobal;  }
	void	ClearDirtyFlag()        { bDirtyGlobal = false; }

	void SetHUDText(GString text, bool bNeedsInit);
	void ToggleMsgText(bool toggle);


private:
    FxHUD();
    FxHUD(FxHUDDataProvider* pdataProvider, FxHUDEventHandler* phudEventsHandler);
    virtual ~FxHUD ();

private:

    bool Init();
    GFxLoader               Loader;
    GPtr<GFxMovieView>      pMovie;
    GPtr<GFxRenderConfig>   pRenderConfig;
    GViewport               Viewport;
    FxHUDDataProvider*      pDataProvider;
    FxHUDEventHandler*      pHudEventsHandler;

#ifndef GFC_NO_DRAWTEXT_SUPPORT
	GPtr<GFxDrawTextManager>	pDrawTextManager;
	GPtr<GFxDrawText>			pHUDText, pMsgText;
#endif // GFC_NO_DRAWTEXT_SUPPORT

	bool					bDirtyGlobal;

    Info                    CurrentTab;
	Info					LastTab; // Used to to restore tab after reopening HUD
    bool                    EventHandlerEnabled;
    bool                    GraphVisible;

    GString                 HudText, MessageText;
	bool					bIsHelpTextSet, bIsSummaryTextSetup, bDisplayMsgText;

	static const UInt		LEN_EMPTY_STRING = 4;  // Used to hide the background when appropriate.
	static const UInt		HUD_TEXT_X = 4;
	static const UInt		HUD_TEXT_Y = 4;
};


struct FxHUDExtraInfo
{
    Float           LastFPS;
    UInt            TessTriangles;
    Float           Zoom;
    GString         AAMode;
    bool            Paused;
    
    Float           CurvePixelError;
    GString         StrokeType;
    GString         FontConfig;
};

class FxHUDInfoProvider
{
public:
    virtual ~FxHUDInfoProvider() {}

public:
    virtual void GetExtraInfo(FxHUDExtraInfo* pinfo) = 0;
    virtual void GetHelpStr(GString* phelpStr) = 0;
};

class FxHUDDataProvider : public GRefCountBase<FxHUDDataProvider, GStat_Default_Mem>
{
public:
    FxHUDDataProvider(GFxMovieView* pmovie, GRenderer* prenderer,
                      GFxFontCacheManager* pfontCacheManager, GFxMeshCacheManager* pmeshCacheManager,
                      FxHUDInfoProvider* pinfoProvider, bool isConsole);

public:
    void SetMovie(GFxMovieView* pmovie);

    void UpdateRendererStats();
    void UpdatePerformanceStats(bool resetOnly);
    
    GRenderer* GetRenderer() const { return pRenderer; }

    void GetHUDStrings(FxHUD::Info, GString* phudString, GString* pmsgString);
    void PrintReport(bool reportAll = false);
    bool SaveStatistics(const GString& filename, bool reportAll = false);
    bool IsConsole() const { return Console; }

    void SetRenderer(GRenderer* pren) { pRenderer = pren; }

    GINLINE UInt    GetLastRenderMasksCount()   { return LastRenderStats.Masks; }
    
private:
    struct RenderStats
    {
        UInt    Triangles;
        UInt    Lines;
        UInt    Primitives;
        UInt    Masks;
	UInt    Filters;

        RenderStats()       { Clear();}
        void    Clear()     { Triangles = 0; Lines = 0; Primitives = 0; Masks = 0; Filters = 0; }

        void    SetFromBag(GStatBag* pbag);
        bool    Match(RenderStats& other) const;
    };

    struct PerformanceStats
    {
        UInt32  AdvanceTime;
        UInt32  DisplayTime;

        PerformanceStats()       { Clear();}
        void    Clear()     { AdvanceTime = 0; DisplayTime = 0; }
        void    SetFromBag(GStatBag* pbag);
    };

    GMemoryHeap*              pTrackerHeap;
    GPtr<GRenderer>           pRenderer;
    GPtr<GFxMovieView>        pMovie;
    GPtr<GFxFontCacheManager> pFontCacheManager;
    GPtr<GFxMeshCacheManager> pMeshCacheManager;
    char                      ShortFileName[64];
    FxHUDInfoProvider*        pInfoProvider;
    bool                      Console;

    RenderStats               LastRenderStats; 
    PerformanceStats          LastPerformanceStats;
};


#endif // INC_FXHUD_H

