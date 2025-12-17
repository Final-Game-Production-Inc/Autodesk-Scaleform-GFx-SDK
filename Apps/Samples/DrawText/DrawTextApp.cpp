/**************************************************************************

Filename    :   DrawTextApp.cpp
Content     :   DrawText API Sample Program
Created     :
Authors     :   Artem Bolgar, Dmitry Polenur

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "../Common/FxPlayerAppBase.h"
//#include "../FxPlayer/FxHUD.h"

#ifdef GFX_ENABLE_DRAWTEXT
#include "GFx/GFx_DrawText.h"
#ifdef DrawText
#undef DrawText
#endif
#endif

#include "Text.h"

#include "GFx/AS3/AS3_Global.h"


class CustomImageCreator : public GFx::ImageCreator
{
public:
    CustomImageCreator(Render::TextureManager* texMgr = 0) : GFx::ImageCreator(texMgr){}
    virtual ~CustomImageCreator(){}
    virtual Image*  LoadProtocolImage(const ImageCreateInfo& info, const String& url)
    {
        const char* filename = url.ToCStr() + 6;
        return GFx::ImageCreator::LoadImageFile(info, filename);
    }
};

class   FxPlayerApp : public FxPlayerAppBase
{
public:
    FxPlayerApp();

    virtual bool            OnInit(Platform::ViewConfig& config);
    virtual void            OnUpdateFrame(bool needRepaint);
    virtual void            OnShutdown();

	Ptr<GFx::DrawTextManager>	pDm1, pDm2;
    Ptr<GFx::DrawText>          ptxt11, ptxt12,  ptxt21, ptxt22, ptxtImg,
                                pblurTxt, pglowTxt, pdropShTxt, pblurglowTxt;
    float   Angle;
    UInt32  Color;
};



FxPlayerApp::FxPlayerApp()
{


}

bool FxPlayerApp::OnInit(Platform::ViewConfig& config)
{
#define FONT_NAME "Slate Mobile"

	if (!FxPlayerAppBase::OnInit(config))
		return false;
	pRenderThread->SetBackgroundColor(0xFFFFFF);

    Ptr<CustomImageCreator> pimageCreator = * SF_NEW CustomImageCreator();
    mLoader.SetImageCreator(pimageCreator);
	// Let's create two GFx::DrawTextManagers. Actually, one instance of GFx::DrawTextManager
	// is enough for most cases, but here we will create two of them for demonstration
	// purposes only.
	// The first one will be created using a pointer to GFxLoader to share all states from
	// the loader.
	pDm1 = *new DrawTextManager(&mLoader);

	// The second one will be created using GFxMovieDef pointer to share all fonts 
	// containing in the GFxMovieDef.
	Ptr<MovieDef> pmovieDef;
    if (!(pmovieDef = *mLoader.CreateMovie("AS2\\Samples\\drawtext_fonts.swf", Loader::LoadAll)))
    {
        if (!(pmovieDef = *mLoader.CreateMovie("drawtext_fonts.swf", Loader::LoadAll)))
        {
#ifdef SF_OS_WIN32
            String errorString = "Unable to load drawtext_fonts.swf ";
            MessageBoxA(NULL, errorString.ToCStr(), "Error", MB_OK | MB_ICONEXCLAMATION);
#endif
            return 0;
        }
    }
	pDm2 = *new DrawTextManager(pmovieDef);

	Render::Size<unsigned> viewSize = GetViewSize();
	Render::Viewport dmViewport(viewSize.Width, viewSize.Height,
		int(viewSize.Width * GetSafeArea().Width),
		int(viewSize.Height * GetSafeArea().Height),
		int(viewSize.Width - 2 * viewSize.Width * GetSafeArea().Width),
		int(viewSize.Height - 2 * viewSize.Height * GetSafeArea().Height));
	pDm1->SetViewport(dmViewport);
	pDm2->SetViewport(dmViewport);


	// Our first GFx::DrawTextManager will use system fonts. GFxFontProviderWin32 should be
	// created (done in FxPlayerAppBase::InitRun()) and set to it. 
	pDm1->SetFontProvider(mLoader.GetFontProvider());
	// Create text using plain text and default text parameters.
	DrawTextManager::TextParams defParams = pDm1->GetDefaultTextParams();
	defParams.TextColor = Render::Color(0xF0, 0, 0, 0xFF); // red, alpha = 255
	defParams.FontName  = FONT_NAME;
	defParams.FontSize  = 16;                       // Slate Mobile, 16 pts will be our default font
	// now, default text will be created with red color, font Slate Mobile, 16 pts
	pDm1->SetDefaultTextParams(defParams);

	// Since we are using pdm1, the newly created text will use system fonts.
	ptxt11 = *pDm1->CreateText("Plain text, red, Slate Mobile, 16pts",
		RectF(20, 20, 500, 400));

	SizeF sz;
	sz = pDm2->GetTextExtent("Animated");
	ptxt21 = *pDm2->CreateText("Animated", RectF(600, 400, sz));
	ptxt21->SetColor(Render::Color(0, 0, 255, 255));
	Angle = 0;
	Color = 0xFF00;

	// Create HTML text, using fonts from GFxMovieDef
	GFx::DrawTextManager::TextParams defParams2 = pDm2->GetDefaultTextParams();
	defParams2.TextColor = Render::Color(0xF0, 0, 0, 0xFF); // red, alpha = 255
	defParams2.Multiline = true;
	defParams2.WordWrap  = false;
	SizeF htmlSz = pDm2->GetHtmlTextExtent(HtmlText, 0, &defParams2);
	ptxt22 = *pDm2->CreateHtmlText(HtmlText, RectF(00, 100, htmlSz), &defParams2);

	// Create a HTML DrawText instance with images using <img> HTML tags.
	// Images can be loaded from the SWF using the image symbol's export name or 
	// via a custom ImageLoader. The "img://" or "imgps://" user protocol prefix
	// is used to load images via the installed ImageLoader. 
	// For this sample, one image is loaded using a simple ImageLoader for JPEGs.
    String imgStr( "SWF Library: <img src=\"gfxlogo\">\n\n"
	    "External JPEG Using Custom ImageLoader: <img src=\"img://scaleform.jpg\">\n\n"
	    "Manipulated Using HTML Tags: "
	    "<img src=\"gfxlogo\" width=\"150\" height=\"20\" vspace=\"-25\">");

	ptxtImg = *pDm2->CreateHtmlText(imgStr, RectF(10, 500, 500, 750));
	ptxtImg->SetBackgroundColor(Render::Color(0, 0, 0, 0x96));

	// Create text with using String and TextParams
	String str(
		"Scaleform GFx is a light-weight high-performance rich media vector "
		"graphics and user interface (UI) engine. GFx combines the scalability "
		"and development ease of proven visual authoring tools, such as the "
		"Adobe Flash(R) Studio, with the latest hardware graphics acceleration "
		"that cutting-edge PC and Console game developers demand.");
	GFx::DrawTextManager::TextParams params;
	params.FontName   = "Arial";
	params.FontSize   = 14;
	params.FontStyle  = GFx::DrawText::Italic;
	params.Multiline  = true;
	params.WordWrap   = true;
	params.HAlignment = GFx::DrawText::Align_Justify;
	sz = pDm1->GetTextExtent(str, 200, &params);
	ptxt12 = *pDm1->CreateText(str, RectF(200, 300, sz), &params);
	ptxt12->SetColor(Render::Color(0, 0, 255, 130), 0, 1);
	ptxt12->SetBorderColor(Render::Color(0xFFFF3399));
	ptxt12->SetBackgroundColor(Render::Color(0xFF0099FF));

	//// Demonstration of setting custom user data to be passed into the renderer
	//// by PushUserData call.
	//ptxt11->SetRendererFloat(98.3f);
	//ptxt11->SetRendererString("str");

	pblurTxt = *pDm2->CreateText("Blur", RectF(800, 300, SizeF(200, 160)));
	//pblurTxt->SetText("Blur");
	pblurTxt->SetColor(Render::Color(120, 30, 192, 255));
	pblurTxt->SetFontSize(32);
    pblurTxt->SetLetterSpacing(15);
	GFx::DrawText::Filter blurF(GFx::DrawText::Filter_Blur);
	blurF.Blur.BlurX = blurF.Blur.BlurY = 5;
	blurF.Blur.Strength = 120;
	pblurTxt->SetFilters(&blurF);

	pglowTxt = *pDm2->CreateText("Glow", RectF(800, 350, SizeF(200, 220)));
	pglowTxt->SetColor(Render::Color(120, 30, 192, 255));
	pglowTxt->SetFontSize(72);
	GFx::DrawText::Filter glowF(GFx::DrawText::Filter_Glow);
	glowF.Glow.BlurX = glowF.Glow.BlurY = 2;
	glowF.Glow.Strength = 1000;
	glowF.Glow.Color = Render::Color(0, 0, 0, 255).ToColor32();
	pglowTxt->SetFilters(&glowF);

	pdropShTxt = *pDm2->CreateText("DropShadow", RectF(800, 450, SizeF(250, 120)));
	pdropShTxt->SetColor(Render::Color(220, 130, 192, 255));
	pdropShTxt->SetFontSize(32);
	GFx::DrawText::Filter dshF(GFx::DrawText::Filter_DropShadow);
	dshF.DropShadow.BlurX = dshF.DropShadow.BlurY = 2;
	dshF.DropShadow.Strength = 1000;
	dshF.DropShadow.Color = Render::Color(0, 0, 0, 255).ToColor32();
	dshF.DropShadow.Distance = 3;
	pdropShTxt->SetFilters(&dshF);

	pblurglowTxt = *pDm2->CreateText("Blur & Glow", RectF(800, 550, SizeF(400, 220)));
	pblurglowTxt->SetColor(Render::Color(120, 30, 192, 255));
	pblurglowTxt->SetFontSize(32);
	GFx::DrawText::Filter filters[2];
	filters[0].SetFilterType(GFx::DrawText::Filter_Blur);
	filters[0].Blur.BlurX = filters[0].Blur.BlurY = 10;
	filters[0].Blur.Strength = 120;
	filters[1].SetFilterType(GFx::DrawText::Filter_Glow);
	filters[1].Glow.BlurX = filters[1].Glow.BlurY = 2;
	filters[1].Glow.Strength = 1000;
	filters[1].Glow.Color = Render::Color(0, 0, 0, 255).ToColor32();
	filters[1].SetKnockOut();
	pblurglowTxt->SetFilters(filters, 2);


	pDm1->Capture();
	pDm2->Capture();
	pRenderThread->AddDisplayHandle(pDm1->GetDisplayHandle(), FxRenderThread::DHCAT_Overlay, false);
	pRenderThread->AddDisplayHandle(pDm2->GetDisplayHandle(), FxRenderThread::DHCAT_Overlay, false);

	return true;
}

void FxPlayerApp::OnShutdown()
{
    ptxt11 = ptxt12 = ptxt21 = ptxt22 = ptxtImg = 0;
    pblurTxt = pglowTxt = pdropShTxt = pblurglowTxt = 0;
    pDm1 = pDm2 = 0;

    FxPlayerAppBase::OnShutdown();
}

void FxPlayerApp::OnUpdateFrame( bool needRepaint )
{
    DrawText::Matrix txt21matrix;
    Angle += 1;
    RectF r = ptxt21->GetRect();
    txt21matrix.AppendTranslation(-r.x1, -r.y1);
    txt21matrix.AppendRotation(Angle*3.14159f / 180.f);
    txt21matrix.AppendScaling(2);
    txt21matrix.AppendTranslation(r.x1, r.y1);
    ptxt21->SetMatrix(txt21matrix);
    //DrawText::Cxform txt21cx;
    //txt21cx.M[0][0] = float(color & 0xFF) /255.f;
    //txt21cx.M[0][1] = float((color >> 8) & 0xFF) /255.f;
    //txt21cx.M[0][2] = float((color >> 16) & 0xFF) /255.f;
    //color += 32;
    //ptxt21->SetCxform(txt21cx);

    pDm2->Capture();

    FxPlayerAppBase::OnUpdateFrame(needRepaint);
}


SF_PLATFORM_SYSTEM_APP(FxPlayer, Scaleform::GFx::System, FxPlayerApp)