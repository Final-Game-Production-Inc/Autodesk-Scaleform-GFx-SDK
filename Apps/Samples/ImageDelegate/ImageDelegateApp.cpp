/**************************************************************************

Filename    :   FxPlayer.cpp
Content     :   Sample SWF/GFX file player leveraging GFxPlayer API
Created     :
Authors     :   Michael Antonov, Maxim Didenko, Boris Rayskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "../Common/FxPlayerAppBase.h"
#include "GFx/AS3/AS3_Global.h"
#include "Render/ImageFiles/PNG_ImageFile.h"


class   FxPlayerApp : public FxPlayerAppBase
{
public:
    FxPlayerApp();
    // Used by HUD to display statistics
    virtual bool            OnInit(Platform::ViewConfig& config);
   // virtual void            OnUpdateFrame(bool needRepaint);
    virtual String          GetFilePath() const { return "";}
    String                  GetBasePath() const;
    Ptr<Render::Image> pCImage;
    void UpdateImage();
    void UpdateMat();
};


#define TEXTURE1

void FxPlayerApp::UpdateMat()
{
    if (pMovie && pCImage)
    {
        SF::Render::Matrix2F m;
        pCImage->GetMatrix(&m);
        pCImage->SetMatrix(m, 0);
        pMovie->ForceUpdateImages();
    }
}

class ImgDel : public ImageDelegate
{
    ImageRect rect;
public:
    ImgDel(Image* i)
    {
        pImage = i;
        rect = i->GetRect();
    }
    //virtual ImageRect GetRect() const { return rect; }
};

void FxPlayerApp::UpdateImage()
{
    if (pMovie)
    {
        ImageBase* pimageOrig = 0;
        if (pCImage)
        {
            pimageOrig = pCImage;
        }
        else
        {
            Resource*      pres = pMovie->GetMovieDef()->GetResource("texture1");
            ImageResource* pimageRes = 0;
            if (pres && pres->GetResourceType() == Resource::RT_Image)
                pimageRes = (ImageResource*)pres;

            if (pimageRes)
            {
                pimageOrig = pimageRes->GetImage();
            }
        }

        if (pimageOrig)
        {
            Scaleform::Ptr<Scaleform::Render::Image> pImage;
            pImage = *Render::PNG::FileReader::ReadImage<SysFile>(GetBasePath() + "FishImage.png");
            if (!pImage)
            {
                pImage = *Render::PNG::FileReader::ReadImage<SysFile>("FishImage.png");
                if (!pImage)
                {
                    fprintf(stderr, "Error: Unable to load FishImage.png\n");
#ifdef SF_OS_WIN32
                    String errorString = "Unable to load  FishImage.png";
                    MessageBoxA(NULL, errorString.ToCStr(), "Error", MB_OK | MB_ICONEXCLAMATION);
#endif
                }
            }

            Render::ImageDelegate* id = (Render::ImageDelegate*)pimageOrig;
            Scaleform::GFx::Matrix2F mat = Scaleform::GFx::Matrix2F::Scaling
                ((float)pimageOrig->GetSize().Width / pImage->GetSize().Width, (float)pimageOrig->GetSize().Height / pImage->GetSize().Height);
            //printf("%f %f %f %f\n",mat.M[0][0],mat.M[0][1], mat.M[1][0],mat.M[1][1]);
            pImage->SetMatrix(mat);
            id->pImage = pImage;
        }
        pMovie->ForceUpdateImages();
    }
}

class DemoImageCreator : public GFx::ImageCreator
{
    FxPlayerApp *pApp;
public:
    DemoImageCreator(Render::TextureManager* texMgr, FxPlayerApp *papp) : GFx::ImageCreator(texMgr), pApp(papp) {SF_ASSERT(pApp);}
    virtual ~DemoImageCreator(){}
    virtual  Render::Image* LoadProtocolImage(const GFx::ImageCreateInfo& info, const String& url)
    {
        if (!pApp->pCImage)
        {
            String path = pApp->FileName;
            if ( !URLBuilder::ExtractFilePath(&path) )
            {
                path = "";
            }
            String filename = url;
            filename.Remove(0,6); //remove img://
            path = path + filename;
            Render::ImageFileHandlerRegistry* pregistry = info.GetImageFileHandlerRegistry();
            Ptr<Render::Image> img;
            if (pregistry)
            {
                Ptr<Render::ImageDelegate> imgd = new ImgDel(Ptr<Render::Image>(*pregistry->ReadImage<SysFile>(path)));
                img = imgd;
            }
            pApp->pCImage = img;
        }
        return pApp->pCImage;
    }
};

class FxPlayerCommandUpdateImage : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        SF_UNUSED2(controllerIdx, keyDown);
        FxPlayerApp* playerApp = (FxPlayerApp*)app;
        playerApp->UpdateImage();
        playerApp->KeyCommandMap.Set(Key::F5, NULL); // Run once
    }
    virtual String GetHelpString() const
    {
        return "";
    }
};

FxPlayerApp::FxPlayerApp()
{    
    KeyCommandMap.Set(Key::F5, *SF_NEW FxPlayerCommandUpdateImage());
}


//------------------------------------------------------------------------

bool FxPlayerApp::OnInit(Platform::ViewConfig& config)
{
    // Skip loading the movie until we have overriden the ImageCreator
    SkipLoadMovie = true;
    if (!FxPlayerAppBase::OnInit(config))
        return false;

    if (pRenderThread)
    {
        Ptr<ImageCreator> pimageCreator = * SF_NEW DemoImageCreator(pRenderThread->GetTextureManager(), this);
        mLoader.SetImageCreator(pimageCreator);
    }

    String fileName = "ImageDelegate.swf";

    //// Now load the movie, if needed
    //if (!SkipLoadMovie && !FileName.IsEmpty())
    //{
    if (!LoadMovie(GetBasePath() + fileName))
    {
        if (!LoadMovie(fileName))
        {
            fprintf(stderr, "Error: Failed to load %s\n", fileName.ToCStr());
#ifdef SF_OS_WIN32
            String errorString = "Failed to load  " + fileName;
            MessageBoxA(NULL, errorString.ToCStr(), "Error", MB_OK | MB_ICONEXCLAMATION);
#endif
            return 0;
        }
    }
    //}

    return true;
}

Scaleform::String FxPlayerApp::GetBasePath() const
{
#if defined (SF_OS_PS3)
    return "/app_home/AS2/Samples/";
#else 
    return "AS2/Samples/";
#endif
}

//------------------------------------------------------------------------
// ***** Main APP implementation

//SF_PLATFORM_SYSTEM_APP(FxPlayer, Scaleform::GFx::System, FxPlayerApp)

//  
// // Code below makes use of the SysAllocStatic allocator   
// static const int allocSize = 1024 * 1024 * 512;
// static UByte allocBuffer[allocSize];
// static SysAllocStatic sysAlloc;
// static GFx::System* gfxSystem;
// 
// Scaleform::Platform::AppBase* Scaleform::Platform::AppBase::CreateInstance()
// {
//     sysAlloc.AddMemSegment(allocBuffer, allocSize);
//     gfxSystem = new GFx::System(&sysAlloc);
//     return new FxPlayerApp();
// }
// 
// void Scaleform::Platform::AppBase::DestroyInstance(AppBase* app)
// {
//     delete app;
//     delete gfxSystem;
// }
SF_PLATFORM_SYSTEM_APP(FxPlayer, Scaleform::GFx::System, FxPlayerApp)

