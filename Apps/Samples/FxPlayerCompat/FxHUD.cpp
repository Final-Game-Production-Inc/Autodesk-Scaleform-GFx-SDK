/**************************************************************************

Filename    :   FxHUD.cpp
Content     :   
                
Created     :   November, 2008
Authors     :   Michael Antonov, Dmitry Polenur, Maxim Didenko

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFxCompat.h"
#include "../Src/Kernel/SF_MsgFormat.h"
#include "../Src/Kernel/SF_String.h"
#include "FxFontProviderHUD.h"
#include "FxPlayerLog.h"
#include "FxHUD.h"

FxHUDDataProvider::FxHUDDataProvider(
    GFxMovieView* pmovie,
    GRenderer* prenderer, 
    GFxFontCacheManager* pfontCacheManager,
    GFxMeshCacheManager* pmeshCacheManager,
    FxHUDInfoProvider* pinfoProvider, 
    bool isConsole
    )
: pTrackerHeap(NULL), pRenderer(prenderer), pFontCacheManager(pfontCacheManager), pMeshCacheManager(pmeshCacheManager), 
  pInfoProvider(pinfoProvider), Console(isConsole)
{
    SetMovie(pmovie);

    pTrackerHeap = GMemory::GetHeapByAddress(this);
}

void FxHUDDataProvider::SetMovie(GFxMovieView* pmovie)
{
    pMovie = pmovie;
    if (pMovie)
    {
        const char* pfilename = pMovie->GetMovieDef()->GetFileURL();
        // Copy short filename (i.e. after last '/'),
        {
            SPInt len = strlen(pfilename);
            for (SPInt i=len; i>0; i--) 
            {
                if (pfilename[i]=='/' || pfilename[i]=='\\') 
                {
                    pfilename = pfilename+i+1;
                    break;
                }
            }
            len = strlen(pfilename);
            if (len > SPInt(sizeof(ShortFileName) - 1))
                len = SPInt(sizeof(ShortFileName) - 1);
            G_strncpy(ShortFileName, sizeof(ShortFileName), pfilename, len);
            ShortFileName[len] = 0;
        }
    }
}

void FxHUDDataProvider::UpdateRendererStats()
{
    GStatBag renderStatBag(pTrackerHeap);
    
    GetRenderer()->GetStats(&renderStatBag);
    renderStatBag.UpdateGroups();
    LastRenderStats.SetFromBag(&renderStatBag);
}

void FxHUDDataProvider::UpdatePerformanceStats(bool resetOnly)
{
    if (pMovie)
    {
        bool reset = true;
#ifdef GFX_AMP_SERVER
        if (GFxAmpServer::GetInstance().IsValidConnection())
        {
            reset = false;
        }
#endif
        if (resetOnly)
        {
            pMovie->GetStats(NULL, reset);
        }
        else
        {
            GStatBag statBag(pTrackerHeap);
            pMovie->GetStats(&statBag, reset);
            statBag.UpdateGroups();
            LastPerformanceStats.SetFromBag(&statBag);
        }
    }
}

void FxHUDDataProvider::GetHUDStrings(FxHUD::Info info, GString* phudString, GString* pmsgString)
{
    if (!pMovie)
    {
        *phudString = "";
#ifdef FXPLAYER_FILEPATH
        if (pExtraInfo->IsConsole)
            *pmsgString = "Copy a SWF/GFX file to\n" FXPLAYER_FILEPATH;
        else
#endif
            *pmsgString = "Drag and drop SWF/GFX file here";
    }
    else
    {
        *pmsgString ="";
        switch(info)
        {
            
        case FxHUD::Summary:
            {
                FxHUDExtraInfo ExtraInfo;
                pInfoProvider->GetExtraInfo(&ExtraInfo);

                GMemoryHeap::RootStats stats;
                GMemory::pGlobalHeap->GetRootStats(&stats);

                GStringBuffer phudText;

                G_Format(
                    phudText,
                    "Playback Info (F1/F2)\n"
                    "Filename:  {0}\n"
                    "           SWF {1} ({2}x{3}@{4:.0})\n"
                    "           {5}/{6} frames{7:sw: (Paused):}\n"
                    "FPS:       {8:3.1}\n",

                    ShortFileName,
                    pMovie->GetMovieDef()->GetVersion(),
                    (UInt)pMovie->GetMovieDef()->GetWidth(),
                    (UInt)pMovie->GetMovieDef()->GetHeight(),
                    pMovie->GetMovieDef()->GetFrameRate(),
                    pMovie->GetCurrentFrame(),
                    pMovie->GetMovieDef()->GetFrameCount(),
                    ExtraInfo.Paused,
                    ExtraInfo.LastFPS
                );

                G_Format(phudText,
                    "MemUse:    {0:sep:,}K / {1:sep:,}K\n",
                    (stats.SysMemUsedSpace - stats.UserDebugFootprint - stats.DebugInfoFootprint + 1023) / 1024,
                    (stats.SysMemFootprint - stats.UserDebugFootprint - stats.DebugInfoFootprint + 1023) / 1024
                    );

                G_Format(
                    phudText,
                    "NewTess:   {0} Tri/s\n"
                    "Triangles: {1:sep:,} @ {2} DP\n"
                    "Lines:     {3}\n"
                    "Masks:     {4}\n"
                    "Filters:   {8}\n"
                    "AA Mode:   {5}\n"
                    "RasGlyphs: {6}\n"
                    "MeshThrashing: {7}\n"
                    "\n",

                    ExtraInfo.TessTriangles,
                    LastRenderStats.Triangles,
                    LastRenderStats.Primitives,
                    LastRenderStats.Lines,
                    LastRenderStats.Masks,
                    ExtraInfo.AAMode,
                    pFontCacheManager->GetNumRasterizedGlyphs(),
                    pMeshCacheManager->GetMeshThrashing(),
                    LastRenderStats.Filters
                    );

                if (!IsConsole())
                    G_Format(
                        phudText,
                        "CurveErr:  {0:3.1} (Ctrl - or +)\n"
                        "Stroke:    {1}\n"
                        "Zoom:      {2:3.2}\n"
                        //"Move:      x:%3.1f y:%3.1f"
                        "FontCfg:   {3}\n"
						"\n",

                        ExtraInfo.CurvePixelError,
                        ExtraInfo.StrokeType,
                        ExtraInfo.Zoom,
                        //Move.x/20, Move.y/20,
                        ExtraInfo.FontConfig
                    );

				G_Format(
					phudText,
					"Advance:  {0:3.3} ms\n"
					"Display:  {1:3.3} ms\n",
					LastPerformanceStats.AdvanceTime / 1000.0f,
					LastPerformanceStats.DisplayTime / 1000.0f
					);


                *phudString = phudText;
            }
            break;

        case FxHUD::Help:
            {
                GString helpStr;
                pInfoProvider->GetHelpStr(&helpStr);
                *phudString = helpStr; 
            }
            break;
        case FxHUD::Hidden:
		default:
			break;
        }
    }
}

void FxHUDDataProvider::PrintReport( bool reportAll /*= false*/ )
{
    GStringBuffer reportStr;
    GMemory::pGlobalHeap->MemReport(reportStr, reportAll ? GMemoryHeap::MemReportFull : GMemoryHeap::MemReportMedium);
    printf("%s", reportStr.ToCStr());
}

bool FxHUDDataProvider::SaveStatistics(const GString& filename, bool reportAll /*= false*/ )
{
    GSysFile fout;
    fout.Open(filename, GFile::Open_Create|GFile::Open_Write);

    bool res = fout.IsWritable();
    if (res)
    {
        GStringBuffer reportStr;
        GMemory::pGlobalHeap->MemReport(reportStr, reportAll ? GMemoryHeap::MemReportFull : GMemoryHeap::MemReportMedium);
        SInt len = (SInt)reportStr.GetLength();
        res = (len == fout.Write((UByte*)reportStr.ToCStr(), len));
    }
    fout.Close();
    if (!res)
        fprintf(stderr, "Error: Can not save statistics to %s\n", filename.ToCStr());
    return res;
}

//void FxHUDDataProvider::SetTreeControls( FxTreeDisplayUpdate* pdisplayUpdate )
//{
//    //pMemoryTreeControl = *new FxTreeControl(pStatTracker->GetMemoryStats(), pdisplayUpdate);
//    //pObjetsTreeControl = *new FxTreeControl(pStatTracker->GetObjectStats(), pdisplayUpdate);
//    //pPerfomanceTreeControl = NULL;
//    pCurrentTreeControl  = *new FxTreeControl(pStatTracker->GetMemoryStats(), pdisplayUpdate);
//}


//////////////////////////////////////////////////////////////////////////
//

FxHUD* FxHUD::CreateHUD( FxHUDDataProvider* pdataProvider, FxHUDEventHandler* phudEventsHandler )
{
    FxHUD* phud = GHEAP_AUTO_NEW(pdataProvider) FxHUD(pdataProvider, phudEventsHandler);
    if (phud->Init())
    {
        return phud;
    }
    else
    {
        phud->Release();
        return NULL;
    }
    
}

FxHUD::FxHUD(FxHUDDataProvider* pdataProvider, FxHUDEventHandler* phudEventsHandler)
:Loader(GFxLoader::LoaderConfig(GFxLoader::LoadDebugHeap)), pDataProvider(pdataProvider), pHudEventsHandler(phudEventsHandler),
 CurrentTab(Hidden), LastTab(Help), EventHandlerEnabled(true), GraphVisible(false)
{
    if (pDataProvider)
        pDataProvider->AddRef();
}

FxHUD::~FxHUD()
{
    if (pDataProvider)
        pDataProvider->Release();
}

bool FxHUD::Init()
{
    GASSERT(pDataProvider);
    GASSERT(pHudEventsHandler);
    GRenderer* prenderer = pDataProvider->GetRenderer();
    pRenderConfig = *new GFxRenderConfig(prenderer, GFxRenderConfig::RF_StrokeNormal);
    Loader.SetRenderConfig(pRenderConfig);

    GPtr<GFxFontProviderHUD> fontProvider = *new GFxFontProviderHUD();
    Loader.SetFontProvider(fontProvider);

    Loader.SetLog(GPtr<GFxLog>(*new GFxPlayerLog()));

    GFxFontCacheManager::TextureConfig fontCacheConfig;
    fontCacheConfig.TextureWidth   = 256;
    fontCacheConfig.TextureHeight  = 256;
    fontCacheConfig.MaxNumTextures = 1;
    fontCacheConfig.MaxSlotHeight  = 32;
    //fontCacheConfig.SlotPadding    = 2;
    Loader.GetFontCacheManager()->SetTextureConfig(fontCacheConfig);
    Loader.GetFontCacheManager()->EnableDynamicCache(true);
    Loader.GetFontCacheManager()->SetMaxRasterScale(1.0f);

#ifdef GFC_OS_MAC
    // Keep images for renderer changes
    GPtr<GFxImageCreator> pImageCreator = *new GFxImageCreator(1);
    Loader.SetImageCreator(pImageCreator);
#endif

#ifndef GFC_NO_DRAWTEXT_SUPPORT
	GMemoryHeap* phudheap = GMemory::GetHeapByAddress(this);
	pDrawTextManager = *GHEAP_NEW(phudheap) GFxDrawTextManager(&Loader);    //, phudheap);
	pDrawTextManager->SetRenderConfig(pRenderConfig);
	pDrawTextManager->SetFontProvider(fontProvider);

	GFxDrawTextManager::TextParams defParams = pDrawTextManager->GetDefaultTextParams();
	defParams.TextColor = GColor(0xF0, 0xF0, 0, 0xFf);
	defParams.FontName  = "Lucida Console";
	defParams.FontSize  = 10;
	defParams.Multiline = true;
//	defParams.Leading   = 3;
	pDrawTextManager->SetDefaultTextParams(defParams);

	pHUDText =  *pDrawTextManager->CreateText(	HudText.ToCStr(), 
												GRectF(0, 0, 0, 0));
	pMsgText =  *pDrawTextManager->CreateText(	MessageText.ToCStr(), 
												GRectF(Viewport.Width/2.f, Viewport.Height/2.f, Viewport.Width/2.f+100.f, Viewport.Height/2.f+100.f));

	pHUDText->SetAAMode(GFxDrawText::AA_Readability);
	pMsgText->SetAAMode(GFxDrawText::AA_Readability);

	pMsgText->SetColor(GColor(0xC8, 0xC8, 0xC8, 0xFf));
#endif	//GFC_NO_DRAWTEXT_SUPPORT

	bIsHelpTextSet = false;
	bIsSummaryTextSetup = false;
	bDisplayMsgText = true;

	return true;
}

void FxHUD::SetViewport(const GViewport& view)
{
	Viewport = view;
	Update();
}

void FxHUD::SetRenderer(GRenderer* pren)
{
    pRenderConfig->SetRenderer(pren);
    if (pDataProvider)
        pDataProvider->SetRenderer(pren);
}

void FxHUD::Display()
{
#ifndef GFC_NO_DRAWTEXT_SUPPORT
	pDrawTextManager->BeginDisplay(Viewport);	
	if (bDisplayMsgText)
		pMsgText->Display();
	else
		pHUDText->Display();
	pDrawTextManager->EndDisplay();
#endif	//GFC_NO_DRAWTEXT_SUPPORT
	if (GetDirtyFlag())
		ClearDirtyFlag();
}

void FxHUD::SetHUDText(GString text, bool bNeedsInit)
{
#ifndef GFC_NO_DRAWTEXT_SUPPORT
	if (bNeedsInit)
	{
		GSizeF htExtent = pDrawTextManager->GetTextExtent(HudText);
		pHUDText->SetRect(GRectF(Float(HUD_TEXT_X), Float(HUD_TEXT_Y), 500.f, HUD_TEXT_Y + htExtent.Height + 5));
		pHUDText->SetBackgroundColor(GColor(0, 0, 0, 0x96));
	}
	pHUDText->SetText(HudText);
#else // GFC_NO_DRAWTEXT_SUPPORT
	GUNUSED(bNeedsInit);
#endif
}

void FxHUD::ToggleMsgText(bool toggle)
{
	bDisplayMsgText = toggle;
}

void FxHUD::Update()
{
#ifndef GFC_NO_DRAWTEXT_SUPPORT
    pDataProvider->GetHUDStrings(CurrentTab, &HudText, &MessageText);

	if (CurrentTab == Help)
	{
		if (!bIsHelpTextSet)
		{
			SetHUDText(HudText, true);
			bIsHelpTextSet = true;
			bIsSummaryTextSetup = false;
		}
	}
	else if (CurrentTab == Summary)
	{
		if (!bIsSummaryTextSetup)
		{
			SetHUDText(HudText, true);
			bIsSummaryTextSetup = true;
			bIsHelpTextSet = false;				
		}
		SetHUDText(HudText, false);			
	}

	if (bDisplayMsgText)
	{
		GSizeF msgtExtent = pDrawTextManager->GetTextExtent(MessageText);
		pMsgText->SetText(MessageText);
		pMsgText->SetRect(GRectF((Viewport.Width - msgtExtent.Width)/2, (Viewport.Height-msgtExtent.Height)/2, (Viewport.Width + msgtExtent.Width)/2, (Viewport.Height+msgtExtent.Height)/2));
	}
#endif	//GFC_NO_DRAWTEXT_SUPPORT
}

void FxHUD::ToggleTab(FxHUD::Info info)
{
    if (CurrentTab == info || info == Hidden)
        SetVisible(false);
    else
        ActivateTab(info);
	bDirtyGlobal = true;
	Update();	
}

void FxHUD::NextTab()
{
    switch(CurrentTab)
    {
    case Help:
        ActivateTab(Summary);
        break;
    case Summary:
        ActivateTab(Help);
        break;
	default:
		break;
    }
	Update();
}

void FxHUD::PreviousTab()
{
    switch(CurrentTab)
    {
    case Help:
        ActivateTab(Summary);
        break;
    case Summary:
        ActivateTab(Help);
        break;
	default:
		break;
    }
	Update();
}

void FxHUD::SetVisible(bool visible)
{
	if (visible)
	{
		ActivateTab(LastTab);
		Update();
	}
	else
	{
		LastTab = CurrentTab;
		CurrentTab = Hidden;
		Clear();
		pHudEventsHandler->OnHudStatusChange(false);
		bDirtyGlobal = true;
	}
}


void FxHUD::ActivateTab(FxHUD::Info info)
{
    if (CurrentTab == info || info == Hidden)
        return;

    int tabidx = 0;
    switch(info)
    {
    case Help:
        tabidx = 0;
        break;
    case Summary:
        tabidx = 1;
        break;
    default:
        break;
    }

	if (bDisplayMsgText)
		ToggleMsgText(false);

    CurrentTab = info;
	Update();
}


void FxHUD::Clear()
{
#ifndef GFC_NO_DRAWTEXT_SUPPORT
    pHUDText->SetText("");
	bIsHelpTextSet = false;

    pMsgText->SetText(""); 
	bIsSummaryTextSetup = false;
#endif // GFC_NO_DRAWTEXT_SUPPORT
}

void FxHUD::SetBorder( bool visible )
{
	GUNUSED(visible);
   // pMovie->Invoke("showBorder", "%b", visible);
}
///////////////////////////////////////////////////////////////////////////////
void FxHUDDataProvider::RenderStats::SetFromBag( GStatBag* pbag )
{
    Clear();

    GStatInfo si;           
    if (pbag->GetStat(&si, GStatRender_DP_Cnt))
        Primitives = (UInt)si.ToCounterStat()->GetCount();
    if (pbag->GetStat(&si, GStatRender_Line_Cnt))
        Lines = (UInt)si.ToCounterStat()->GetCount();
    if (pbag->GetStat(&si, GStatRender_Triangle_Cnt))
        Triangles = (UInt)si.ToCounterStat()->GetCount();
    if (pbag->GetStat(&si, GStatRender_Mask_Cnt))
        Masks = (UInt)si.ToCounterStat()->GetCount();
    if (pbag->GetStat(&si, GStatRender_Filter_Cnt))
        Filters = (UInt)si.ToCounterStat()->GetCount();
}

bool FxHUDDataProvider::RenderStats::Match( RenderStats& other ) const
{
    if (((Triangles >> 11) != (other.Triangles >> 11)) || 
        ((Lines >> 6) != (other.Lines >> 6)) || 
        (Primitives != other.Primitives) || 
        (Masks != other.Masks))
        return false;
    return true;
}

void FxHUDDataProvider::PerformanceStats::SetFromBag( GStatBag* pbag )
{
    Clear();

    GStatInfo si;           
    if (pbag->GetStat(&si, GFxStatMV_Advance_Tks))
    {
        AdvanceTime = static_cast<UInt32>(si.ToTimerStat()->GetTicks());
    }
    if (pbag->GetStat(&si, GFxStatMV_Display_Tks))
    {
        DisplayTime = static_cast<UInt32>(si.ToTimerStat()->GetTicks());
    }
}

