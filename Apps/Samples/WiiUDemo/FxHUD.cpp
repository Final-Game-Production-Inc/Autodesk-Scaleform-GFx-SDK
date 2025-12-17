/**************************************************************************

Filename    :   FxHUD.cpp
Content     :   Ststistic and performance reporting system for the GFx
                player.
                
Created     :   November, 2008
Authors     :   Michael Antonov, Dmitry Polenur, Maxim Didenko, 

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx_AMP.h"
#include "GFx_Kernel.h"
#include "GFx.h"
#include "GFx_Render.h"
#include "Kernel/SF_MsgFormat.h"
//#include "Render/Render_TreeNode.h"

#include "FxHUD.h"
#include "Render/Text/Text_FontManager.h"
#include "Render/Text/Text_Core.h"

#include "GFx/GFx_FontProviderHUD.h"

#include "../Common/FxPlayerLog.h"

#ifdef GFX_ENABLE_DRAWTEXT
//#undef GFX_ENABLE_DRAWTEXT
#endif

#ifdef FXPLAYER_LOGO
#define   FXPLAYER_LOGO_TEXT "GFxPlayer 4.0 Copyright 2011 Autodesk, Inc. All Rights reserved."
#endif

FxHUDDataProvider::FxHUDDataProvider(
                                     Movie* pmovie1,
                                     Movie* pmovie2,
                                     FxRenderThread* prenderer, 
                                     FxHUDInfoProvider* pinfoProvider, 
                                     bool isConsole
                                     )
                                     : pTrackerHeap(NULL), pRenderer(prenderer), 
                                     pInfoProvider(pinfoProvider), Console(isConsole)
{
    SetMovie(0, pmovie1);
    SetMovie(1, pmovie2);
    pTrackerHeap = Memory::GetHeapByAddress(this);
    pInfoProvider->GetHelpStr(&HelpStr);
    MsgStr = "Drag and drop SWF/GFX file here";
}

void FxHUDDataProvider::SetMovie(int n, Movie* pmovie)
{
    pMovies[n] = pmovie;
    if (pmovie)
    {
        const char* pfilename = pmovie->GetMovieDef()->GetFileURL();
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
            if (len > SPInt(sizeof(ShortFileName[n]) - 1))
                len = SPInt(sizeof(ShortFileName[n]) - 1);
            SFstrncpy(ShortFileName[n], sizeof(ShortFileName[n]), pfilename, len);
            ShortFileName[n][len] = 0;
        }
    }

}

void FxHUDDataProvider::UpdateRendererStats()
{
	Render::HAL::Stats rstats;
	pRenderer->GetRenderStats(&rstats);
	LastRenderStats.Primitives = rstats.Primitives;
	LastRenderStats.Triangles = rstats.Triangles;
	LastRenderStats.Meshes = rstats.Meshes;
    LastRenderStats.Masks = rstats.Masks;
    LastRenderStats.Filters = rstats.Filters;
}

//void FxHUDDataProvider::UpdatePerformanceStats(bool resetOnly)
//{
//    if (pMovies[0])
//    {
//        bool reset = true;
//#ifdef SF_AMP_SERVER
//        if (AmpServer::GetInstance().IsValidConnection())
//        {
//            reset = false;
//        }
//#endif
//        if (resetOnly)
//        {
//            pMovies[0]->GetStats(NULL, reset);
//        }
//        else
//        {
//            StatBag statBag(pTrackerHeap);
//            pMovies[0]->GetStats(&statBag, reset);
//            statBag.UpdateGroups();
//            LastPerformanceStats.SetFromBag(&statBag);
//        }
//    }
//}

void FxHUDDataProvider::UpdatePerformanceStats( UInt64 advanceT, UInt64 displayT, float avgFps)
{
	LastPerformanceStats.AdvanceTime = advanceT;
	LastPerformanceStats.DisplayTime = displayT;
    LastPerformanceStats.AvgFps = avgFps;
}

void FxHUDDataProvider::GetHUDStrings(FxHUD::Info info, String* phudString, String* pmsgString)
{
    if (!pMovies[0])
    {
        *phudString = "";
        *pmsgString = MsgStr;
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

                MemoryHeap::RootStats stats;
                Memory::pGlobalHeap->GetRootStats(&stats);

                StringBuffer phudText;
                for (int i = 0; i < 2; i++)
                {
                Format(
                    phudText,
                    //"Playback Info (F1/F2)\n"
                    "Filename:	    {0}\n"
                    "               SWF {1} ({2}x{3}@{4:.0})\n"
                    "               {5}/{6} frames{7:sw: (Paused):}\n",
//                  "FPS:           {8:3.1}\n",
                    ShortFileName[i],
                    pMovies[i]->GetMovieDef()->GetVersion(),
                    (unsigned)pMovies[i]->GetMovieDef()->GetWidth(),
                    (unsigned)pMovies[i]->GetMovieDef()->GetHeight(),
                    pMovies[i]->GetMovieDef()->GetFrameRate(),
                    pMovies[i]->GetCurrentFrame(),
                    pMovies[i]->GetMovieDef()->GetFrameCount(),
                    ExtraInfo.Paused
                    //ExtraInfo.LastFPS
                    );
                }
                //Format(phudText,
                //    "Resolution:    {0}x{1}\n",
                //    ExtraInfo.AppRes.Width,
                //    ExtraInfo.AppRes.Height                    
                //    );


                Format(phudText,
                    "MemUse:	    {0:sep:,}K / {1:sep:,}K\n",
                    (stats.SysMemUsedSpace - stats.UserDebugFootprint - stats.DebugInfoFootprint + 1023) / 1024,
                    (stats.SysMemFootprint - stats.UserDebugFootprint - stats.DebugInfoFootprint + 1023) / 1024
                    );

                Format(
                    phudText,
//                    "NewTess:   {0} Tri/s\n"
					"Meshes:        {0} \n"
                    "Triangles:	    {1:sep:,} @ {2} DP\n"
//                     "Lines:     {3}\n"
                    "Masks:         {3}\n"
                    "Filters:       {4}\n"
//                    "AA Mode:       {5}\n"
					"RenderThread:  {5}\n"
                    "RasCount:      {6}\n"
                    //"MeshThrashing: {7}\n"
                    "\n",

//                    ExtraInfo.TessTriangles,
					LastRenderStats.Meshes,
                    LastRenderStats.Triangles,
                    LastRenderStats.Primitives,
                    //LastRenderStats.Lines,
                    LastRenderStats.Masks,
					LastRenderStats.Filters,
//                    ExtraInfo.AAMode,
                    //0 //pMeshCacheManager->GetMeshThrashing(), //@TOT
					pRenderer->IsSingleThreaded() ? "ST" : "MT",
                    ExtraInfo.RasCount
                    );

                //if (!IsConsole())
                //    Format(
                //    phudText,
                //    "CurveErr:  {0:3.1} (Ctrl - or +)\n"
                //    "Stroke:    {1}\n"
                //    "Zoom:      {2:3.2}\n"
                //    //"Move:      x:%3.1f y:%3.1f"
                //    "FontCfg:   {3}\n"
                //    "\n",

                //    ExtraInfo.CurvePixelError,
                //    ExtraInfo.StrokeType,
                //    ExtraInfo.Zoom,
                //    //Move.x/20, Move.y/20,
                //    ExtraInfo.FontConfig
                //    );

                Format(
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
                *phudString = HelpStr;
            }
            break;
        case FxHUD::Hidden:
        default:
            break;
        }
    }
}

bool FxHUDDataProvider::SaveStatistics(const String& filename, bool xmlFormat, MemoryHeap::MemReportType reportType)
{
    SysFile fout;
    fout.Open(filename, File::Open_Create|File::Open_Write);

    bool res = fout.IsWritable();
    if (res)
    {
        StringBuffer reportStr;
        Memory::pGlobalHeap->MemReport(reportStr, reportType, xmlFormat);
        GetPerformanceString(reportStr, xmlFormat);
        int len = (int)reportStr.GetLength();
        res = (len == fout.Write((UByte*)reportStr.ToCStr(), len));
    }
    fout.Close();
    if (!res)
        fprintf(stderr, "Error: Can not save statistics to %s\n", filename.ToCStr());
    return res;
}

void FxHUDDataProvider::PrintStatistics(bool xmlFormat, MemoryHeap::MemReportType reportType)
{
    StringBuffer reportStr;
    Memory::pGlobalHeap->MemReport(reportStr, reportType, xmlFormat);
    StringBuffer perfStr;
    GetPerformanceString(perfStr, xmlFormat);

    if (pMovies[0] && pMovies[0]->GetLog())
    {
        if (reportStr.GetLength() < Scaleform::Log::MaxLogBufferMessageSize)
        {
            pMovies[0]->GetLog()->LogMessageById(LogMessage_Report | LogChannel_Memory, "\n%s", reportStr.ToCStr());
        }
        else
        {
            static const UPInt chunkSize = 100;
            for (UPInt i = 0; i < reportStr.GetLength(); i += chunkSize)
            {
                pMovies[0]->GetLog()->LogMessage("%s", String(reportStr.ToCStr() + i, Alg::Min(chunkSize, reportStr.GetLength() - i)).ToCStr());
            }
        }
        pMovies[0]->GetLog()->LogMessageById(LogMessage_Report | LogChannel_Performance, "\n%s", perfStr.ToCStr());
    }
    else
    {
        printf("%s", reportStr.ToCStr());
        printf("%s", perfStr.ToCStr());
    }
}

void FxHUDDataProvider::GetPerformanceString(StringBuffer& report, bool xmlFormat)
{
    if (xmlFormat)
    {
        Format(report,
            "<PerfItem Name=\"AvgFPS\" Value={0:3.3}></PerfItem>\n"
            "<PerfItem Name=\"Advance\" Value={1:3.3}></PerfItem>\n"
            "<PerfItem Name=\"Display\" Value={2:3.3}></PerfItem>\n",
            LastPerformanceStats.AvgFps,
            LastPerformanceStats.AdvanceTime / 1000.0f,
            LastPerformanceStats.DisplayTime / 1000.0f
            );
    }
    else
    {
        Format(report,
            "Avg FPS:  {0:3.3}\n"
            "Advance:  {1:3.3} ms\n"
            "Display:  {2:3.3} ms\n",
            LastPerformanceStats.AvgFps,
            LastPerformanceStats.AdvanceTime / 1000.0f,
            LastPerformanceStats.DisplayTime / 1000.0f
            );
    }
}

//////////////////////////////////////////////////////////////////////////
//

FxHUD* FxHUD::CreateHUD( FxHUDDataProvider* pdataProvider)
{
    FxHUD* phud = SF_HEAP_AUTO_NEW(pdataProvider) FxHUD(pdataProvider);
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

FxHUD::FxHUD(FxHUDDataProvider* pdataProvider)
: mLoader(Loader::LoaderConfig(Loader::LoadDebugHeap)), pDataProvider(pdataProvider),
RenderContext(Memory::GetGlobalHeap()), CurrentTab(Hidden), LastTab(Help)
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
    SF_ASSERT(pDataProvider);

/*#ifdef SF_OS_MAC
    // Keep images for renderer changes
    Ptr<ImageCreator> pImageCreator = *new ImageCreator(1);
    Loader.SetImageCreator(pImageCreator);
#endif*/

#ifdef GFX_ENABLE_DRAWTEXT
    MemoryHeap* phudheap = Memory::GetHeapByAddress(this);
    pDrawTextManager = *SF_HEAP_NEW(phudheap) DrawTextManager(&mLoader);

    Ptr<GFx::FontProviderHUD> fontProvider = *SF_HEAP_NEW(phudheap) GFx::FontProviderHUD(phudheap);
    pDrawTextManager->SetFontProvider(fontProvider);

    DrawTextManager::TextParams defParams = pDrawTextManager->GetDefaultTextParams();
    defParams.TextColor = Render::Color(0xF0, 0xF0, 0, 0xFf);
    defParams.FontName  = "Courier New"; //"Lucida Console";
    defParams.FontSize  = 13;
    defParams.Multiline = true;
    //defParams.Leading   = 3;
    pDrawTextManager->SetDefaultTextParams(defParams);

    pHUDText =  *pDrawTextManager->CreateText(	"", 
        RectF(0, 0, 0, 0));
    pMsgText =  *pDrawTextManager->CreateText(	"", 
        RectF(mViewport.Width/2.f, mViewport.Height/2.f, mViewport.Width/2.f+100.f, mViewport.Height/2.f+100.f));

    pHUDText->SetAAMode(DrawText::AA_Readability);
    pHUDText->SetRect(RectF(40.f, 40.f, 500 * 20.0, mViewport.Height * 20.f));
    pMsgText->SetAAMode(DrawText::AA_Readability);
    pMsgText->SetRect(Render::RectF(0.f, 0.f, 0.f, 0.f));

    pMsgText->SetColor(Color(0xC8, 0xC8, 0xC8, 0xFf));

#ifdef FXPLAYER_LOGO
    pLogoText =  *pDrawTextManager->CreateText(	"TEST1\nTEST2", 
        RectF(0, mViewport.Height - 36.f, mViewport.Width - 10.f, mViewport.Height - 1.f));
    pLogoText->SetColor(Color(0xC8, 0xC8, 0xC8, 0xFf));
#endif

    pDrawTextManager->SetViewport(mViewport);
    pDrawTextManager->Capture(false);
    if (pDataProvider->GetRenderThread())
    {
        pDataProvider->GetRenderThread()->
            SetHUDDisplayHandle(pDrawTextManager->GetDisplayHandle());
    }
#endif //GFC_NO_DRAWTEXT_SUPPORT

    return true;
}

void FxHUD::SetViewport(const Render::Viewport& view)
{
    mViewport = view;
    mViewport.Height = mViewport.BufferHeight = view.Height + FXPLAYER_LOGO_BORDER;
#ifdef GFX_ENABLE_DRAWTEXT
    pDrawTextManager->SetViewport(mViewport);
    pDrawTextManager->Capture();
#endif
    Update();
}


void FxHUD::SetHUDText(String text, bool bNeedsInit)
{
#ifdef GFX_ENABLE_DRAWTEXT
    if (bNeedsInit)
    {
        SizeF htExtent = pDrawTextManager->GetTextExtent(text);
        pHUDText->SetRect(RectF(float(HUD_TEXT_X), float(HUD_TEXT_Y), 500.f, HUD_TEXT_Y + htExtent.Height + 5));
        pHUDText->SetBackgroundColor(Color(0, 0, 0, 0x96));
    }
    pHUDText->SetText(text);
    pDrawTextManager->Capture();
#endif
}

void FxHUD::ClearMsgText()
{
#ifdef GFX_ENABLE_DRAWTEXT
    pMsgText = 0;
    pDrawTextManager->Capture();
#endif
}

void FxHUD::Update()
{
    String hudText, MessageText;
    pDataProvider->GetHUDStrings(CurrentTab, &hudText, &MessageText);

if (CurrentTab == Summary)
    {
        SetHUDText(hudText, false);			
    }
#ifdef GFX_ENABLE_DRAWTEXT
   else if (pMsgText)
    {
        SizeF msgtExtent = pDrawTextManager->GetTextExtent(MessageText);
        pMsgText->SetText(MessageText);
        pMsgText->SetRect(RectF((mViewport.Width - msgtExtent.Width)/2, (mViewport.Height-msgtExtent.Height)/2, (mViewport.Width + msgtExtent.Width)/2, (mViewport.Height+msgtExtent.Height)/2));
        pDrawTextManager->Capture();
	}
#ifdef FXPLAYER_LOGO
    //FXPLAYER_LOGO_TEXTString LogoText = "TEST1\nTEST2";
    SizeF msgtExtent = pDrawTextManager->GetTextExtent(FXPLAYER_LOGO_TEXT);
    pLogoText->SetText(FXPLAYER_LOGO_TEXT);
    pLogoText->SetRect(RectF((mViewport.Width - msgtExtent.Width)/2, (mViewport.Height - msgtExtent.Height), (mViewport.Width + msgtExtent.Width)/2, (float)mViewport.Height));
    pDrawTextManager->Capture();
#endif

#endif	//GFC_NO_DRAWTEXT_SUPPORT
}

void FxHUD::ToggleTab(FxHUD::Info info)
{
    if (CurrentTab == info || info == Hidden)
	{
        SetVisible(false);
		if (pDataProvider->GetRenderThread())
		{
#if !defined(FXPLAYER_LOGO) && defined(GFX_ENABLE_DRAWTEXT)
			pDataProvider->GetRenderThread()->
				SetHUDDisplayHandle(0);
#endif
		}
	}
    else
	{
        ActivateTab(info);
		if (pDataProvider->GetRenderThread())
		{
#if !defined(FXPLAYER_LOGO) && defined(GFX_ENABLE_DRAWTEXT)
            pDataProvider->GetRenderThread()->
                SetHUDDisplayHandle(pDrawTextManager->GetDisplayHandle());
#endif
		}
	}
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
        SetVisible(false);
        break;
    case Hidden:
        SetVisible(true);
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
        SetVisible(false);
        break;
    case Summary:
        ActivateTab(Help);
        break;
    case Hidden:
        SetVisible(true);
        ActivateTab(Summary);
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
        if (pDataProvider->GetRenderThread())
        {
#if !defined(FXPLAYER_LOGO) && defined(GFX_ENABLE_DRAWTEXT)
            pDataProvider->GetRenderThread()->
                SetHUDDisplayHandle(pDrawTextManager->GetDisplayHandle());
#endif
        }
        Update();
    }
    else
    {
        if (pDataProvider->GetRenderThread())
        {
#if defined(GFX_ENABLE_DRAWTEXT)
#if !defined(FXPLAYER_LOGO)  
            pDataProvider->GetRenderThread()->
                SetHUDDisplayHandle(0);
#else
            pHUDText->SetRect(RectF(0, 0, 1, 1));
#endif
#endif
        }
        LastTab = CurrentTab;
        CurrentTab = Hidden;
        Clear();
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

    CurrentTab = info;
    String hudText, MessageText;
    pDataProvider->GetHUDStrings(CurrentTab, &hudText, &MessageText);
    SetHUDText(hudText, true);
}


void FxHUD::Clear()
{
#ifdef GFX_ENABLE_DRAWTEXT
    if (pHUDText)
        pHUDText->SetText("");
    if (pMsgText)
        pMsgText->SetText(""); 
#endif // GFC_NO_DRAWTEXT_SUPPORT
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

void FxHUDDataProvider::PerformanceStats::SetFromBag( StatBag* pbag )
{
    StatInfo si;           
    if (pbag->GetStat(&si, StatMV_Advance_Tks))
    {
        AdvanceTime = static_cast<UInt32>(si.ToTimerStat()->GetTicks());
    }
    if (pbag->GetStat(&si, StatMV_Display_Tks))
    {
        DisplayTime = static_cast<UInt32>(si.ToTimerStat()->GetTicks());
    }
}
