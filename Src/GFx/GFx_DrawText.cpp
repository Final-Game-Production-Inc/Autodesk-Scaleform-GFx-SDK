/**************************************************************************

Filename    :   GFx_DrawText.cpp
Content     :   External text interface
Created     :   May 23, 2008
Authors     :   Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "GFx/GFx_DrawText.h"
#include "GFx/GFx_FontManager.h"
#include "GFx/GFx_LoaderImpl.h"
#include "GFx/GFx_MovieDef.h"
#include "Kernel/SF_AutoPtr.h"
#include "Render/Render_TreeNode.h"

#ifdef GFX_ENABLE_DRAWTEXT

namespace Scaleform { namespace GFx {

using namespace Scaleform::Render;

class DrawTextManagerImpl : public NewOverrideBase<StatMV_Text_Mem>
{
public:
    Ptr<TreeRoot>               pRootNode;
    Ptr<StateBagImpl>           pStateBag;
    Ptr<MovieDef>               pMovieDef;
    Ptr<Text::Allocator>        pTextAllocator;
    Ptr<FontManager>            pFontManager;
    Ptr<FontManagerStates>      pFontStates;
    Ptr<ResourceWeakLib>        pWeakLib;
    DrawTextManager::TextParams DefaultTextParams;
    Ptr<LoaderImpl>             pLoaderImpl;
    enum
    {
        RTFlags_BeginDisplayInvoked = 1,
    };
    UInt8                       RTFlags; // run-time flags
    // should be defined last to make sure its dtor is called first.
    Context                     RenderContext; 
    TreeRootDisplayHandle       DispHandle;

    DrawTextManagerImpl():RTFlags(0), RenderContext(Memory::GetGlobalHeap())
    {
    }
    ~DrawTextManagerImpl()
    {
        DispHandle.Clear();
        pRootNode = NULL;
        RenderContext.Shutdown(true);
    }

    // ShutdownRendering shuts down rendering of the movie; it can be called
    // before Movie is released to avoid render-thread block in destructor.
    // After ShutdownRendering(false), IsShutdownRenderingComplete call can be 
    // used to determine when shutdown is complete.
    // Advance, Invoke or other state-modifying functions should NOT be called
    // once shutdown has started.
    void    ShutdownRendering(bool wait) { RenderContext.Shutdown(wait); }
    // Returns true once rendering has been shut down after ShutdownRendering call; 
    // intended for polling from main thread.
    bool    IsShutdownRenderingComplete() const { return RenderContext.IsShutdownComplete(); }
};

class DrawTextImpl : public DrawText
{
public:
    Ptr<DrawTextManager>    pDrawTextCtxt;
    Ptr<TreeText>           pTextNode;
    MemoryHeap*             pHeap;
//     struct UserRendererData : public NewOverrideBase<StatMV_Text_Mem>
//     {           
//         String                  StringVal;
//         float                   FloatVal;
//         float                   MatrixVal[16];
//         //Render::Renderer::UserData UserData;
// 
//         UserRendererData() : FloatVal(0.0f) { }
//     };
//    AutoPtr<UserRendererData>  pUserData;
public:
    DrawTextImpl(DrawTextManager* pdtMgr);
    ~DrawTextImpl();

    void SetText(const char* putf8Str, UPInt lengthInBytes = UPInt(-1))
    {
        pDrawTextCtxt->CheckFontStatesChange();
        pTextNode->SetText(putf8Str, lengthInBytes);
    }
    void SetText(const wchar_t* pstr,  UPInt lengthInChars = UPInt(-1))
    {
        pDrawTextCtxt->CheckFontStatesChange();
        pTextNode->SetText(pstr, lengthInChars);
    }
    void SetText(const String& str)
    {
        pDrawTextCtxt->CheckFontStatesChange();
        pTextNode->SetText(str.ToCStr(), str.GetSize());
    }
    String GetText() const
    {
        return pTextNode->GetText();
    }

    void SetHtmlText(const char* putf8Str, UPInt lengthInBytes = UPInt(-1))
    {
        pDrawTextCtxt->CheckFontStatesChange();
        Text::StyledText::HTMLImageTagInfoArray imgInfoArr(pDrawTextCtxt->pHeap);
        pTextNode->SetHtmlText(putf8Str, lengthInBytes, &imgInfoArr);
        if (imgInfoArr.GetSize() > 0)
        {
            ProcessImageTags(pTextNode->GetDocView(), pDrawTextCtxt, imgInfoArr);
        }
    }
    void SetHtmlText(const wchar_t* pstr,  UPInt lengthInChars = UPInt(-1))
    {
        pDrawTextCtxt->CheckFontStatesChange();
        Text::StyledText::HTMLImageTagInfoArray imgInfoArr(pDrawTextCtxt->pHeap);
        pTextNode->SetHtmlText(pstr, lengthInChars, &imgInfoArr);
        if (imgInfoArr.GetSize() > 0)
        {
            ProcessImageTags(pTextNode->GetDocView(), pDrawTextCtxt, imgInfoArr);
        }
    }
    void SetHtmlText(const String& str)
    {
        pDrawTextCtxt->CheckFontStatesChange();
        Text::StyledText::HTMLImageTagInfoArray imgInfoArr(pDrawTextCtxt->pHeap);
        pTextNode->SetHtmlText(str.ToCStr(), str.GetLength(), &imgInfoArr);
        if (imgInfoArr.GetSize() > 0)
        {
            ProcessImageTags(pTextNode->GetDocView(), pDrawTextCtxt, imgInfoArr);
        }
    }
    String GetHtmlText() const
    {
        return pTextNode->GetHtmlText();
    }

    void SetRect(const RectF& viewRect)
    {
        pTextNode->SetBounds(PixelsToTwips(viewRect));
    }
    RectF GetRect() const
    {
        return TwipsToPixels(pTextNode->GetBounds());
    }

    void SetMatrix(const Matrix& matrix)
    {
        Matrix2F m(matrix);
        m.Tx() = PixelsToTwips(m.Tx());
        m.Ty() = PixelsToTwips(m.Ty());
        pTextNode->SetMatrix(m);
    }
    Matrix GetMatrix() const 
    { 
        Matrix m(pTextNode->M2D());
        m.Tx() = TwipsToPixels(m.Tx());
        m.Ty() = TwipsToPixels(m.Ty());
        return m; 
    }

    void SetCxform(const Cxform& cx)
    {
        pTextNode->SetCxform(cx);
    }
    const Cxform& GetCxform() const
    {
        return pTextNode->GetCxform();
    }

    void SetColor(Color c, UPInt startPos = 0, UPInt endPos = UPInt(-1))
    {
        pTextNode->SetColor(c, startPos, endPos);
    }
    void SetFont (const char* pfontName, UPInt startPos = 0, UPInt endPos = UPInt(-1))
    {
        pTextNode->SetFont(pfontName, startPos, endPos);
    }
    void SetFontSize(float fontSize, UPInt startPos = 0, UPInt endPos = UPInt(-1))
    {
        pTextNode->SetFontSize(fontSize, startPos, endPos);
    }
    void SetLetterSpacing(float letterSpacing, UPInt startPos = 0, UPInt endPos = UPInt(-1))
    {
        pTextNode->SetLetterSpacing(letterSpacing, startPos, endPos);
    }

    void SetFontStyle(FontStyle fontStyle, UPInt startPos = 0, UPInt endPos = UPInt(-1))
    {
        pTextNode->SetFontStyle((TreeText::FontStyle)fontStyle, startPos, endPos);
    }
    void SetUnderline(bool underline, UPInt startPos = 0, UPInt endPos = UPInt(-1))
    {
        pTextNode->SetUnderline(underline, startPos, endPos);
    }
    void SetMultiline(bool multiline)
    {
        pTextNode->SetMultiline(multiline);
    }
    bool IsMultiline() const
    {
        return pTextNode->IsMultiline();
    }

    // Turns wordwrapping on/off
    void SetWordWrap(bool wordWrap)
    {
        pTextNode->SetWordWrap(wordWrap);
    }
    // Returns state of wordwrapping.
    bool IsWordWrap() const
    {
        return pTextNode->IsWordWrap();
    }

    void SetAlignment(Alignment a)
    {
        pTextNode->SetAlignment((TreeText::Alignment)a);
    }
    Alignment  GetAlignment() const
    {
        return (Alignment)pTextNode->GetAlignment();
    }

    void SetVAlignment(VAlignment a)
    {
        pTextNode->SetVAlignment((TreeText::VAlignment)a);
    }
    VAlignment  GetVAlignment() const
    {
        return (VAlignment)pTextNode->GetVAlignment();
    }

    void   SetBorderColor(const Color& borderColor)
    {
        pTextNode->SetBorderColor(borderColor);
    }
    Color GetBorderColor() const { return pTextNode->GetBorderColor(); }

    void   SetBackgroundColor(const Color& bkgColor)
    {
        pTextNode->SetBackgroundColor(bkgColor);
    }
    Color GetBackgroundColor() const { return pTextNode->GetBackgroundColor(); }

//     void SetRendererString(const String& str)
//     {
//         if (!pUserData)
//             pUserData = SF_HEAP_NEW(pHeap) UserRendererData();
//         pUserData->StringVal = str;
//         pUserData->UserData.PropFlags |= Render::Renderer::UD_HasString;
//         pUserData->UserData.pString = pUserData->StringVal.ToCStr();
//     }
//     String GetRendererString() const
//     {
//         if (!pUserData)
//             return String();
//         return pUserData->StringVal;
//     }
// 
//     void SetRendererFloat(float num)
//     {
//         if (!pUserData)
//             pUserData = SF_HEAP_NEW(pHeap) UserRendererData();
//         pUserData->FloatVal = num;
//         pUserData->UserData.PropFlags |= Render::Renderer::UD_HasFloat;
//         pUserData->UserData.pFloat = &pUserData->FloatVal;
//     }
// 
//     float GetRendererFloat() const
//     {
//         if (!pUserData)
//             return 0;
//         return pUserData->FloatVal;
//     }

//     void SetRendererMatrix(float *m, unsigned count = 16)
//     {
//         if (!pUserData)
//             pUserData = SF_HEAP_NEW(pHeap) UserRendererData();
//         memcpy(pUserData->MatrixVal, m, count*sizeof(float));
//         pUserData->UserData.PropFlags |= Render::Renderer::UD_HasMatrix;
//         pUserData->UserData.pMatrix = pUserData->MatrixVal;
//         pUserData->UserData.MatrixSize = count;
//     }
    
    void        SetAAMode(AAMode aa)
    {
        pTextNode->SetAAMode((TreeText::AAMode)aa);
    }

    AAMode      GetAAMode() const 
    { 
        return (AAMode)pTextNode->GetAAMode();
    }

    // Sets filters on the text. More than one filter may be applied to one
    // text: "blur" filter might be combined with either "glow" or "dropshadow" filter.
    void        SetFilters(const Filter* filters, UPInt filtersCnt = 1)
    {
        pTextNode->SetFilters(filters, filtersCnt);
    }

    // Remove all filters from the text
    void        ClearFilters()
    {
        pTextNode->ClearFilters();
    }

    void        SetDepth(unsigned newDepth)
    {
        unsigned curDepth = GetDepth();
        if (newDepth > pDrawTextCtxt->pImpl->pRootNode->GetSize())
            newDepth = (unsigned)pDrawTextCtxt->pImpl->pRootNode->GetSize();
        pDrawTextCtxt->pImpl->pRootNode->Remove(curDepth, 1);
        pDrawTextCtxt->pImpl->pRootNode->Insert(newDepth, pTextNode);
    }

    unsigned    GetDepth() const
    {
        UPInt n = pDrawTextCtxt->pImpl->pRootNode->GetSize();
        for (UPInt i = 0; i < n; ++i)
        {
            if (pDrawTextCtxt->pImpl->pRootNode->GetAt(i) == pTextNode)
                return unsigned(i);
        }
        // ? Can it be not found?
        return ~0u;
    }

    // Set visibility property.
    void        SetVisible(bool visible)
    {
        pTextNode->SetVisible(visible);
    }
    // Returns true if visible.
    bool        IsVisible() const
    {
        return pTextNode->IsVisible();
    }

    Text::DocView* GetDocView() const { return pTextNode->GetDocView(); }

    static void ProcessImageTags(Text::DocView* ptextDocView, 
                                 const DrawTextManager* pmgr, 
                                 Text::StyledText::HTMLImageTagInfoArray& imageInfoArray);

};

DrawTextImpl::DrawTextImpl(DrawTextManager* pdtMgr) : 
    pDrawTextCtxt(pdtMgr)
{
    pHeap = Memory::GetHeapByAddress(this);

    pTextNode = *pdtMgr->GetRenderContext().CreateEntry<TreeText>();
    pTextNode->Init(pDrawTextCtxt->GetTextAllocator(), pDrawTextCtxt->GetFontManager(), pDrawTextCtxt->GetLog());
}

DrawTextImpl::~DrawTextImpl()
{
    // Remove the node from render tree
    unsigned i = GetDepth();
    if (i != ~0u)
        pDrawTextCtxt->pImpl->pRootNode->Remove(i, 1);
}

void DrawTextImpl::ProcessImageTags(Text::DocView* ptextDocView, 
                                    const DrawTextManager* pmgr, 
                                    Text::StyledText::HTMLImageTagInfoArray& imageInfoArray)
{
    UPInt numTags = imageInfoArray.GetSize();

    MovieDefImpl*        pmdImpl = NULL;
    Ptr<ImageCreator>    pimageCreator;

    // If the DrawTextManager was created with a MovieDef, it can load 
    // images from the MovieDef or disk. If it was created with a Loader, 
    // it will load only be able to load images from disk.
    if (pmgr->pImpl->pMovieDef)
        pmdImpl = (MovieDefImpl*)pmgr->pImpl->pMovieDef.GetPtr();

    if (pmgr->GetImageCreator())
        pimageCreator = pmgr->GetImageCreator();

    for (UPInt i = 0; i < numTags; ++i)
    {
        Text::StyledText::HTMLImageTagInfo& imgTagInfo = imageInfoArray[i];
        SF_ASSERT(imgTagInfo.pTextImageDesc);
        // TODO: Url should be used to load image by the same way as loadMovie does.
        // At the moment there is no good way to get a callback when movie is loaded.
        // Therefore, at the moment, Url is used as export name only. "imgps://" also
        // may be used.

        // check for user protocol img:// and imgps://
        bool userImageProtocol = LoaderImpl::IsProtocolImage(imgTagInfo.Url);

        ResourceBindData resBindData;
        Ptr<Render::Image> img;
        if (!userImageProtocol)
        {
            if (!pmdImpl)
            {
                if (pmgr->GetLog())
                    pmgr->GetLog()->LogWarning
                    ("DrawText::ProcessImageTags: can't find a resource since there is no moviedef\n");
                continue;
            }
            if (!pmdImpl->GetExportedResource(&resBindData, imgTagInfo.Url))
            {
                if (pmgr->GetLog())
                    pmgr->GetLog()->LogWarning
                    ("DrawText::ProcessImageTags: can't find a resource for export name '%s'\n", imgTagInfo.Url.ToCStr());
                continue;
            }
            SF_ASSERT(resBindData.pResource.GetPtr() != 0);
        }

        // Must check resource type, since users can theoretically pass other resource ids.
        if (userImageProtocol || resBindData.pResource->GetResourceType() == Resource::RT_Image)
        {
            // bitmap
            if (!userImageProtocol)
            {
                Ptr<ImageResource> pimgRes = static_cast<ImageResource*>(resBindData.pResource.GetPtr());
                if (!pimgRes)
                {
                    if (pmgr->GetLog())
                        pmgr->GetLog()->LogWarning
                        ("DrawText::ProcessImageTags: can't load the image '%s'\n", imgTagInfo.Url.ToCStr());
                    continue;
                }
                if (pimgRes->GetImage()->GetImageType() != Render::ImageBase::Type_ImageBase)
                    img = static_cast<Render::Image*>(pimgRes->GetImage());
                else
                {
                    if (!pimageCreator)
                    {
                        LogDebugMessage(Log_Warning, "ImageCreator is null in DrawText::ProcessImageTags");
                        return;
                    }
                    else
                    {
                        ImageCreateInfo cinfo(ImageCreateInfo::Create_SourceImage, pmgr->pHeap);
                        img = *pimageCreator->CreateImage(cinfo, static_cast<Render::ImageSource*>(pimgRes->GetImage()));
                    }
                }
            }
            else
            {
                // user image protocol case.
                MemoryHeap* pimageHeap = pmgr->pImpl->pWeakLib->GetImageHeap();
                ImageCreateInfo cinfo(ImageCreateInfo::Create_FileImage, pimageHeap, 0);
                cinfo.SetStates(pmgr->GetLog(), 
                    static_cast<FileOpener*>(pmgr->GetStateBagImpl()->GetFileOpener().GetPtr()), 
                    pmgr->GetStateBagImpl()->GetImageFileHandlerRegistry());

                if (pimageCreator)
                    img = *pimageCreator->LoadProtocolImage(cinfo, imgTagInfo.Url);
                else
                {
                    LogDebugMessage(Log_Warning, "Image resource creation failed - ImageCreator not installed");        
                    return;
                }
            }
            if (!img)
            {
                LogDebugMessage(Log_Warning, "Image '%s' wasn't created in ProcessImageTags", imgTagInfo.Url.ToCStr());        
                continue;
            }
            Render::ImageRect dimr = img->GetRect();
            imgTagInfo.pTextImageDesc->pImage = img;
            float origWidth = (float)dimr.Width(), origHeight = (float)dimr.Height();
            float screenWidth = 0, screenHeight = 0;
            if (imgTagInfo.Width)
                screenWidth = float(imgTagInfo.Width);
            else
                screenWidth  = (float)PixelsToTwips(dimr.Width());

            if (imgTagInfo.Height)
                screenHeight = float(imgTagInfo.Height);
            else
                screenHeight = (float)PixelsToTwips(dimr.Height());

            float baseLineY = PixelsToTwips(origHeight);
            baseLineY += imgTagInfo.VSpace;
            imgTagInfo.pTextImageDesc->ScreenWidth  = screenWidth;
            imgTagInfo.pTextImageDesc->ScreenHeight = screenHeight;
            imgTagInfo.pTextImageDesc->BaseLineY    = TwipsToPixels(baseLineY);
            imgTagInfo.pTextImageDesc->Matrix.AppendTranslation(0, -imgTagInfo.pTextImageDesc->BaseLineY);
            imgTagInfo.pTextImageDesc->Matrix.AppendScaling(screenWidth/origWidth, screenHeight/origHeight);

            ptextDocView->SetCompleteReformatReq();
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// DrawTextManager
DrawTextManager::DrawTextManager(MovieDef* pmovieDef)
{
    unsigned flags = (MemoryHeap::Heap_UserDebug & Memory::GetHeapByAddress(this)->GetFlags());
    pHeap = Memory::GetGlobalHeap()->CreateHeap("DrawText Manager", flags);
    
    pImpl = SF_HEAP_NEW(pHeap) DrawTextManagerImpl();
    pImpl->pMovieDef = pmovieDef;
    if (pmovieDef)
    {
        pmovieDef->WaitForLoadFinish();
        MovieDefImpl* pdefImpl = static_cast<MovieDefImpl*>(pmovieDef);
        pImpl->pStateBag   = *new StateBagImpl(pdefImpl->pStateBag);
    }
    else
    {
        pImpl->pStateBag   = *new StateBagImpl(NULL);
        pImpl->pStateBag->SetLog(Ptr<Log>(*new Log));
    }
    if (pImpl->pStateBag)
    {
        // By default there should be no glyph packer
        //pImpl->pStateBag->SetFontPackParams(0);

        pImpl->pTextAllocator  = *SF_HEAP_NEW(pHeap) Text::Allocator(pHeap);
        pImpl->pFontStates     = *new FontManagerStates(pImpl->pStateBag);

        if (pImpl->pMovieDef)
        {
            //pStateBag->SetFontCacheManager(pMovieDef->GetFontCacheManager());
            //pStateBag->SetFontLib(pMovieDef->GetFontLib());
            //pStateBag->SetFontMap(pMovieDef->GetFontMap());

            MovieDefImpl* pdefImpl = static_cast<MovieDefImpl*>(pmovieDef);
            pImpl->pFontManager    = *SF_HEAP_NEW(pHeap) FontManager(NULL, pdefImpl, pImpl->pFontStates);
            pImpl->pWeakLib        = pdefImpl->GetWeakLib();
        }
        else
        {
            // It's mandatory to have the cache manager for text rendering to work,
            // even if the dynamic cache isn't used. 
            //pImpl->pStateBag->SetFontCacheManager(Ptr<FontCacheManager>(*new FontCacheManager(true)));
            pImpl->pWeakLib        = *new ResourceWeakLib(NULL);
            pImpl->pFontManager    = *SF_HEAP_NEW(pHeap) FontManager(pImpl->pWeakLib, pImpl->pFontStates);
        }
        //pImpl->pStateBag->SetMeshCacheManager(Ptr<MeshCacheManager>(*SF_NEW MeshCacheManager(false)));
    }
    pImpl->pRootNode = *pImpl->RenderContext.CreateEntry<TreeRoot>();
    pImpl->pRootNode->SetBackgroundColor(0);
    pImpl->DispHandle = Render::TreeRootDisplayHandle(pImpl->pRootNode);
}

DrawTextManager::DrawTextManager(Loader* ploader)
{
    unsigned flags = (MemoryHeap::Heap_UserDebug & Memory::GetHeapByAddress(this)->GetFlags());
    pHeap = Memory::GetGlobalHeap()->CreateHeap("DrawText Manager", flags);

    pImpl = SF_HEAP_NEW(pHeap) DrawTextManagerImpl();
    pImpl->pMovieDef = 0;
    pImpl->pStateBag    = *new StateBagImpl(NULL);
    if (ploader->GetLog())
        pImpl->pStateBag->SetLog(ploader->GetLog());
    else
        pImpl->pStateBag->SetLog(Ptr<Log>(*new Log));

    // By default there should be no glyph packer
    //pImpl->pStateBag->SetFontPackParams(0);

    pImpl->pTextAllocator  = *SF_HEAP_NEW(pHeap) Text::Allocator(pHeap);
    pImpl->pFontStates     = *new FontManagerStates(pImpl->pStateBag);

    // It's mandatory to have the cache manager for text rendering to work,
    // even if the dynamic cache isn't used.
//     if (ploader->GetFontCacheManager())
//         pImpl->pStateBag->SetFontCacheManager(ploader->GetFontCacheManager());
//     else
//         pImpl->pStateBag->SetFontCacheManager(Ptr<FontCacheManager>(*new FontCacheManager(true)));

    //if (ploader->GetRenderConfig())
    //    pImpl->pStateBag->SetRenderConfig(ploader->GetRenderConfig());

    if (ploader->GetFontLib())
        pImpl->pStateBag->SetFontLib(ploader->GetFontLib());
    if (ploader->GetFontMap())
        pImpl->pStateBag->SetFontMap(ploader->GetFontMap());
    if (ploader->GetFontProvider())
        pImpl->pStateBag->SetFontProvider(ploader->GetFontProvider());

    Ptr<ResourceLib> pstrongLib = ploader->GetResourceLib();
    if (pstrongLib)
        pImpl->pWeakLib    = pstrongLib->GetWeakLib();
    else
        pImpl->pWeakLib    = *new ResourceWeakLib(NULL);
    pImpl->pFontManager    = *SF_HEAP_NEW(pHeap) FontManager(pImpl->pWeakLib, pImpl->pFontStates);
    //pImpl->pStateBag->SetMeshCacheManager(ploader->GetMeshCacheManager());

    pImpl->pRootNode = *pImpl->RenderContext.CreateEntry<TreeRoot>();
    pImpl->pRootNode->SetBackgroundColor(0);
    pImpl->DispHandle = Render::TreeRootDisplayHandle(pImpl->pRootNode);
}

DrawTextManager::~DrawTextManager()
{
    delete pImpl;
    pHeap->Release();
}

void DrawTextManager::SetBeginDisplayInvokedFlag(bool v) 
{ 
    (v) ? pImpl->RTFlags |= DrawTextManagerImpl::RTFlags_BeginDisplayInvoked : 
          pImpl->RTFlags &= (~DrawTextManagerImpl::RTFlags_BeginDisplayInvoked); 
}
void DrawTextManager::ClearBeginDisplayInvokedFlag()            
{ 
    SetBeginDisplayInvokedFlag(false); 
}

bool DrawTextManager::IsBeginDisplayInvokedFlagSet() const      
{ 
    return (pImpl->RTFlags & DrawTextManagerImpl::RTFlags_BeginDisplayInvoked) != 0; 
}

DrawTextManager::TextParams::TextParams()
{
    TextColor = Color(0, 0, 0, 255);
    HAlignment = DrawText::Align_Left;
    VAlignment = DrawText::VAlign_Top;
    FontStyle  = DrawText::Normal;
    FontSize   = 12;
    FontName   = "Times New Roman";
    Underline  = false;
    Multiline  = true;
    WordWrap   = true;
}

void DrawTextManager::SetDefaultTextParams(const DrawTextManager::TextParams& params) 
{ 
    pImpl->DefaultTextParams = params; 
}

// Returns currently set default text parameters.
const DrawTextManager::TextParams& DrawTextManager::GetDefaultTextParams() const 
{ 
    return pImpl->DefaultTextParams; 
}

void DrawTextManager::SetTextParams(Text::DocView* pdoc, const TextParams& txtParams,
                                    const Text::TextFormat* tfmt, const Text::ParagraphFormat* pfmt)
{
    Text::TextFormat textFmt(pHeap);
    Text::ParagraphFormat paraFmt;
    if (tfmt)
        textFmt = *tfmt;
    if (pfmt)
        paraFmt = *pfmt;
    textFmt.SetColor(txtParams.TextColor);
    switch(txtParams.FontStyle)
    {
    case DrawText::Normal: 
        textFmt.SetBold(false);
        textFmt.SetItalic(false);
        break;
    case DrawText::Bold:
        textFmt.SetBold(true);
        textFmt.SetItalic(false);
        break;
    case DrawText::Italic:
        textFmt.SetBold(false);
        textFmt.SetItalic(true);
        break;
    case DrawText::BoldItalic:
        textFmt.SetBold(true);
        textFmt.SetItalic(true);
        break;
    }
    textFmt.SetFontName(txtParams.FontName);
    textFmt.SetFontSize(txtParams.FontSize);
    textFmt.SetUnderline(txtParams.Underline);

    Text::ParagraphFormat::AlignType pa;
    switch(txtParams.HAlignment)
    {
    case DrawText::Align_Right:   pa = Text::ParagraphFormat::Align_Right; break;
    case DrawText::Align_Center:  pa = Text::ParagraphFormat::Align_Center; break;
    case DrawText::Align_Justify: pa = Text::ParagraphFormat::Align_Justify; break;
    default:                      pa = Text::ParagraphFormat::Align_Left; break;
    }
    paraFmt.SetAlignment(pa);

    Text::DocView::ViewVAlignment va;
    switch(txtParams.VAlignment)
    {
    case DrawText::VAlign_Bottom:  va = Text::DocView::VAlign_Bottom; break;
    case DrawText::VAlign_Center:  va = Text::DocView::VAlign_Center; break;
    default:                       va = Text::DocView::VAlign_Top; break;
    }
    pdoc->SetVAlignment(va);

    if (txtParams.Multiline)
    {
        pdoc->SetMultiline();
        if (txtParams.WordWrap)
            pdoc->SetWordWrap();
    }
    pdoc->SetTextFormat(textFmt);
    pdoc->SetParagraphFormat(paraFmt);
    pdoc->SetDefaultTextFormat(textFmt);
    pdoc->SetDefaultParagraphFormat(paraFmt);
}

DrawText* DrawTextManager::CreateText()
{
    return SF_HEAP_NEW(pHeap) DrawTextImpl(this);
}

DrawText* DrawTextManager::CreateText(const char* putf8Str, const RectF& viewRect, const TextParams* ptxtParams, unsigned depth)
{
    DrawTextImpl* ptext = SF_HEAP_NEW(pHeap) DrawTextImpl(this);
    ptext->SetRect(viewRect);
    ptext->SetText(putf8Str);
    SetTextParams(ptext->GetDocView(), (ptxtParams) ? *ptxtParams : pImpl->DefaultTextParams);
    if (depth == ~0u)
        pImpl->pRootNode->Add(ptext->pTextNode);
    else
        pImpl->pRootNode->Insert(depth, ptext->pTextNode);
    return ptext;
}

DrawText* DrawTextManager::CreateText(const wchar_t* pwstr, const RectF& viewRect, const TextParams* ptxtParams, unsigned depth)
{
    DrawTextImpl* ptext = SF_HEAP_NEW(pHeap) DrawTextImpl(this);
    ptext->SetRect(viewRect);
    ptext->SetText(pwstr);
    SetTextParams(ptext->GetDocView(), (ptxtParams) ? *ptxtParams : pImpl->DefaultTextParams);
    if (depth == ~0u)
        pImpl->pRootNode->Add(ptext->pTextNode);
    else
        pImpl->pRootNode->Insert(depth, ptext->pTextNode);
    return ptext;
}

DrawText* DrawTextManager::CreateText(const String& str, const RectF& viewRect, const TextParams* ptxtParams, unsigned depth)
{
    DrawTextImpl* ptext = SF_HEAP_NEW(pHeap) DrawTextImpl(this);
    ptext->SetRect(viewRect);
    ptext->SetText(str);
    SetTextParams(ptext->GetDocView(), (ptxtParams) ? *ptxtParams : pImpl->DefaultTextParams);
    if (depth == ~0u)
        pImpl->pRootNode->Add(ptext->pTextNode);
    else
        pImpl->pRootNode->Insert(depth, ptext->pTextNode);
    return ptext;
}

DrawText* DrawTextManager::CreateHtmlText(const char* putf8Str, const RectF& viewRect, const TextParams* ptxtParams, unsigned depth)
{
    DrawTextImpl* ptext = SF_HEAP_NEW(pHeap) DrawTextImpl(this);
    ptext->SetRect(viewRect);
    SetTextParams(ptext->GetDocView(), (ptxtParams) ? *ptxtParams : pImpl->DefaultTextParams);
    ptext->SetHtmlText(putf8Str);
    if (depth == ~0u)
        pImpl->pRootNode->Add(ptext->pTextNode);
    else
        pImpl->pRootNode->Insert(depth, ptext->pTextNode);
    return ptext;
}

DrawText* DrawTextManager::CreateHtmlText(const wchar_t* pwstr, const RectF& viewRect, const TextParams* ptxtParams, unsigned depth)
{
    DrawTextImpl* ptext = SF_HEAP_NEW(pHeap) DrawTextImpl(this);
    ptext->SetRect(viewRect);
    SetTextParams(ptext->GetDocView(), (ptxtParams) ? *ptxtParams : pImpl->DefaultTextParams);
    ptext->SetHtmlText(pwstr);
    if (depth == ~0u)
        pImpl->pRootNode->Add(ptext->pTextNode);
    else
        pImpl->pRootNode->Insert(depth, ptext->pTextNode);
    return ptext;
}

DrawText* DrawTextManager::CreateHtmlText(const String& str, const RectF& viewRect, const TextParams* ptxtParams, unsigned depth)
{
    DrawTextImpl* ptext = SF_HEAP_NEW(pHeap) DrawTextImpl(this);
    ptext->SetRect(viewRect);
    SetTextParams(ptext->GetDocView(), (ptxtParams) ? *ptxtParams : pImpl->DefaultTextParams);
    ptext->SetHtmlText(str);
    if (depth == ~0u)
        pImpl->pRootNode->Add(ptext->pTextNode);
    else
        pImpl->pRootNode->Insert(depth, ptext->pTextNode);
    return ptext;
}

Text::DocView* DrawTextManager::CreateTempDoc(const TextParams& txtParams, 
                                              Text::TextFormat* tfmt, Text::ParagraphFormat *pfmt,
                                              float width, float height)
{
    Text::DocView* ptempText = SF_HEAP_NEW(pHeap) Text::DocView(pImpl->pTextAllocator, pImpl->pFontManager, GetLog());
    tfmt->InitByDefaultValues();
    pfmt->InitByDefaultValues();

    SizeF sz(width, height);
    ptempText->SetViewRect(RectF(0, 0, sz));
    if (txtParams.Multiline)
        ptempText->SetMultiline();
    else      
        ptempText->ClearMultiline();

    if (txtParams.WordWrap && width > 0)
    {
        ptempText->SetWordWrap();
        if (txtParams.Multiline)
            ptempText->SetAutoSizeY();
    }
    else
    {
        ptempText->SetAutoSizeX();
        ptempText->ClearWordWrap();
    }

    return ptempText;
}

SizeF DrawTextManager::GetTextExtent(const char* putf8Str, float width, const TextParams* ptxtParams)
{
    CheckFontStatesChange();
    Text::TextFormat tfmt(pHeap);
    Text::ParagraphFormat pfmt;
    TextParams txtParams = (ptxtParams) ? *ptxtParams : pImpl->DefaultTextParams;
    Ptr<Text::DocView> ptempText = *CreateTempDoc(txtParams, &tfmt, &pfmt, PixelsToTwips(width), 0);
    // need to reset Multiline and WordWrap, since they are already set up
    // correctly inside the CreateTempDoc.
    txtParams.Multiline = txtParams.WordWrap = false; 
    SetTextParams(ptempText, txtParams, &tfmt, &pfmt);
    ptempText->SetText(putf8Str);

    SizeF sz(TwipsToPixels(ptempText->GetTextWidth()), TwipsToPixels(ptempText->GetTextHeight()));
    sz.Expand(TwipsToPixels(GFX_TEXT_GUTTER*2));
    return sz;
}
SizeF DrawTextManager::GetTextExtent(const wchar_t* pwstr, float width, const TextParams* ptxtParams)
{
    CheckFontStatesChange();
    Text::TextFormat tfmt(pHeap);
    Text::ParagraphFormat pfmt;
    TextParams txtParams = (ptxtParams) ? *ptxtParams : pImpl->DefaultTextParams;
    Ptr<Text::DocView> ptempText = *CreateTempDoc(txtParams, &tfmt, &pfmt, PixelsToTwips(width), 0);
    // need to reset Multiline and WordWrap, since they are already set up
    // correctly inside the CreateTempDoc.
    txtParams.Multiline = txtParams.WordWrap = false; 
    SetTextParams(ptempText, txtParams, &tfmt, &pfmt);
    ptempText->SetText(pwstr);

    SizeF sz(TwipsToPixels(ptempText->GetTextWidth()), TwipsToPixels(ptempText->GetTextHeight()));
    sz.Expand(TwipsToPixels(GFX_TEXT_GUTTER*2));
    return sz;
}
SizeF DrawTextManager::GetTextExtent(const String& str, float width, const TextParams* ptxtParams)
{
    CheckFontStatesChange();
    Text::TextFormat tfmt(pHeap);
    Text::ParagraphFormat pfmt;
    TextParams txtParams = (ptxtParams) ? *ptxtParams : pImpl->DefaultTextParams;
    Ptr<Text::DocView> ptempText = *CreateTempDoc(txtParams, &tfmt, &pfmt, PixelsToTwips(width), 0);
    // need to reset Multiline and WordWrap, since they are already set up
    // correctly inside the CreateTempDoc.
    txtParams.Multiline = txtParams.WordWrap = false; 
    SetTextParams(ptempText, txtParams, &tfmt, &pfmt);
    ptempText->SetText(str);

    //ptempText->Dump();
    SizeF sz(TwipsToPixels(ptempText->GetTextWidth()), TwipsToPixels(ptempText->GetTextHeight()));
    sz.Expand(TwipsToPixels(GFX_TEXT_GUTTER*2));
    return sz;
}

SizeF DrawTextManager::GetHtmlTextExtent(const char* putf8Str, float width, const TextParams* ptxtParams)
{
    CheckFontStatesChange();
    Text::TextFormat tfmt(pHeap);
    Text::ParagraphFormat pfmt;
    TextParams txtParams = (ptxtParams) ? *ptxtParams : pImpl->DefaultTextParams;
    Ptr<Text::DocView> ptempText = *CreateTempDoc(txtParams, &tfmt, &pfmt, PixelsToTwips(width), 0);
    // need to reset Multiline and WordWrap, since they are already set up
    // correctly inside the CreateTempDoc.
    txtParams.Multiline = txtParams.WordWrap = false; 
    SetTextParams(ptempText, txtParams, &tfmt, &pfmt);
    ptempText->ParseHtml(putf8Str);

    SizeF sz(TwipsToPixels(ptempText->GetTextWidth()), TwipsToPixels(ptempText->GetTextHeight()));
    sz.Expand(TwipsToPixels(GFX_TEXT_GUTTER*2));
    return sz;
}
SizeF DrawTextManager::GetHtmlTextExtent(const wchar_t* pwstr, float width, const TextParams* ptxtParams)
{
    CheckFontStatesChange();
    Text::TextFormat tfmt(pHeap);
    Text::ParagraphFormat pfmt;
    TextParams txtParams = (ptxtParams) ? *ptxtParams : pImpl->DefaultTextParams;
    Ptr<Text::DocView> ptempText = *CreateTempDoc(txtParams, &tfmt, &pfmt, PixelsToTwips(width), 0);
    // need to reset Multiline and WordWrap, since they are already set up
    // correctly inside the CreateTempDoc.
    txtParams.Multiline = txtParams.WordWrap = false; 
    SetTextParams(ptempText, txtParams, &tfmt, &pfmt);
    ptempText->ParseHtml(pwstr);

    SizeF sz(TwipsToPixels(ptempText->GetTextWidth()), TwipsToPixels(ptempText->GetTextHeight()));
    sz.Expand(TwipsToPixels(GFX_TEXT_GUTTER*2));
    return sz;
}
SizeF DrawTextManager::GetHtmlTextExtent(const String& str, float width, const TextParams* ptxtParams)
{
    CheckFontStatesChange();
    Text::TextFormat tfmt(pHeap);
    Text::ParagraphFormat pfmt;
    TextParams txtParams = (ptxtParams) ? *ptxtParams : pImpl->DefaultTextParams;
    Ptr<Text::DocView> ptempText = *CreateTempDoc(txtParams, &tfmt, &pfmt, PixelsToTwips(width), 0);
    // need to reset Multiline and WordWrap, since they are already set up
    // correctly inside the CreateTempDoc.
    txtParams.Multiline = txtParams.WordWrap = false; 
    SetTextParams(ptempText, txtParams, &tfmt, &pfmt);
    ptempText->ParseHtml(str);

    SizeF sz(TwipsToPixels(ptempText->GetTextWidth()), TwipsToPixels(ptempText->GetTextHeight()));
    sz.Expand(TwipsToPixels(GFX_TEXT_GUTTER*2));
    return sz;
}

StateBag* DrawTextManager::GetStateBagImpl() const 
{ 
    return pImpl->pStateBag; 
}

Text::Allocator*       DrawTextManager::GetTextAllocator() 
{ 
    return pImpl->pTextAllocator; 
}

FontManager*         DrawTextManager::GetFontManager()   
{ 
    return pImpl->pFontManager; 
}

FontManagerStates*   DrawTextManager::GetFontManagerStates() 
{ 
    return pImpl->pFontStates; 
}

void DrawTextManager::CheckFontStatesChange()
{
    pImpl->pFontStates->CheckStateChange(GetFontLib(), GetFontMap(), GetFontProvider(), GetTranslator());
}

void DrawTextManager::SwapDepths(DrawText* t1, DrawText* t2) const
{
    unsigned d1 = t1->GetDepth();
    unsigned d2 = t2->GetDepth();
    t2->SetDepth(d1);
    t1->SetDepth(d2);
}

void DrawTextManager::SetViewport(const Render::Viewport& vp)
{
    pImpl->pRootNode->SetViewport(vp);
    Render::Matrix2F m;
    m.PrependScaling(0.05f);

    pImpl->pRootNode->SetMatrix(m);
}

void DrawTextManager::Capture(bool onChangeOnly)
{
    if (!onChangeOnly || pImpl->RenderContext.HasChanges())
        pImpl->RenderContext.Capture();
}

Render::Context&    DrawTextManager::GetRenderContext()
{
    return pImpl->RenderContext;
}

const Render::TreeRootDisplayHandle& DrawTextManager::GetDisplayHandle() const
{
    return pImpl->DispHandle;
}

void DrawTextManager::SetCaptureThread(ThreadId captureThreadId)
{
    pImpl->RenderContext.SetCaptureThreadId(captureThreadId);
}

void DrawTextManager::ShutdownRendering(bool wait)
{
    pImpl->ShutdownRendering(wait);
}

bool DrawTextManager::IsShutdownRenderingComplete() const
{
    return pImpl->IsShutdownRenderingComplete();
}

}} // namespace Scaleform::GFx

#endif //GFX_ENABLE_DRAWTEXT
