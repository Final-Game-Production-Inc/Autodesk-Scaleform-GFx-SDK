/**************************************************************************

Filename    :   Render_TreeText.cpp
Content     :   TreeText represents TextField implementation
Created     :   2009-2010
Authors     :   Maxim Shemanarev, Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render_TreeText.h"
#include "Render_TreeCacheText.h"

#include "Render_PrimitiveBundle.h" // For GetBundle inline
#include "Renderer2DImpl.h"


namespace Scaleform { namespace Render {


//------------------------------------------------------------------------
// ***** TreeText::NodeData Implementation

TreeText::NodeData::NodeData() 
:   ContextData_ImplMixin<NodeData, TreeNode::NodeData>(ET_Text), 
    TextFlags(0)
{ 
}

TreeText::NodeData::NodeData(NonlocalCloneArg<TreeText::NodeData> src)
:   TreeText::NodeData::BaseClass(NonlocalCloneArg<TreeText::NodeData::BaseClass>(*src.pC)), 
    pDocView(src->pDocView), pLayout(src->pLayout), TextFlags(src->TextFlags)
{
}


TreeText::NodeData::~NodeData()
{
}

bool TreeText::NodeData::PropagateUp(Entry* entry) const
{        
    RectF    bounds, parentBounds, orgBounds;

    if (pDocView)
    {
        if (pDocView->IsReformatReq())
            pDocView->Format();

        if ((TextFlags & TextFlags_RebuildLayout) != 0)
        {
            TextLayout::Builder bld(Memory::GetGlobalHeap());
            pDocView->CreateVisibleTextLayout(bld);

            TreeText* pc = (TreeText*)entry;
            NodeData* data = pc->GetWritableData(Change_TextLayout);
            data->pLayout = *SF_HEAP_AUTO_NEW(this) TextLayout(bld);
            data->TextFlags &= ~TextFlags_RebuildLayout;
        }
    }

    if (pLayout)
    {
        bounds = pLayout->GetBounds();

        // Must apply any filters to the bounds before it is transformed into parent space.
        orgBounds = bounds;
        expandByFilterBounds(&bounds, false );
        if (!Is3D())
            M2D().EncloseTransform(&parentBounds, bounds);
        else
            M3D().EncloseTransform(&parentBounds, bounds);
    }
   
    if ((bounds != AproxLocalBounds) || (parentBounds != bounds))
    {
        TreeText* pc = (TreeText*)entry;
        NodeData* d = pc->GetWritableData(Change_AproxBounds);

        d->AproxLocalBounds = bounds;
        d->AproxParentBounds = parentBounds;
        d->updateOriginalBoundState(orgBounds);
        return IsVisible();
    }
    return false;   
}

TreeCacheNode* TreeText::NodeData::updateCache(TreeCacheNode* pparent,
                                               TreeCacheNode* pinsert,
                                               TreeNode* pnode, UInt16 depth) const
{
    // Create & insert node if it doesn't already exist.
    TreeCacheText*  pcache = (TreeCacheText*)pnode->GetRenderData();
    if (!pcache)
    {
        // Pass NF_MaskNode|NF_PartOfMask to create proper SortKey; it is
        // also adjusted in Insert.
        unsigned cnodeFlags = (Flags & (NF_Visible|NF_MaskNode)) |
                              NF_MaskNode_MapTo_PartOfMask(Flags);
        cnodeFlags |= (pparent->GetFlags() & (NF_PartOfMask|NF_PartOfScale9));

        bool is3d = false;
        TreeCacheNode* parent = pparent;
        while (parent && !is3d)
        {
            is3d |= parent->GetNodeData()->Is3D();
            parent = parent->GetParent();
        }
        pcache = SF_HEAP_AUTO_NEW(pparent) 
            TreeCacheText((TreeText*)pnode, pparent->pRenderer2D, cnodeFlags | (is3d?NF_3D:0));

        if (!pcache) return 0;
        pnode->SetRenderData(pcache);
    }

    pcache->UpdateInsertIntoParent(pparent, pinsert, this, depth);    
    return pcache;
}

TreeNode* TreeText::NodeData::CloneCreate(Context* context) const
{
    return context->CreateEntry<TreeText>(NonlocalCloneArg<TreeText::NodeData>(*this));
}



void TreeText::SetLayout(const TextLayout::Builder& b)
{
    NodeData* data = GetWritableData(Change_TextLayout);
    data->pLayout = *SF_HEAP_AUTO_NEW(this) TextLayout(b);
    AddToPropagate();
}

void TreeText::Init(Text::Allocator* pallocator, Text::FontManagerBase* fntMgr, Log* log)
{
    NodeData* data = GetWritableData(Change_TextLayout); //? Is the enum correct?
    if (!data->pDocView) 
    {
        data->pDocView = *SF_HEAP_AUTO_NEW(this) Text::DocView(pallocator, fntMgr, log);
        Text::TextFormat tfmt(Memory::GetHeapByAddress(this));
        tfmt.InitByDefaultValues();
        data->pDocView->SetDefaultTextFormat(tfmt);

        Text::ParagraphFormat pfmt;
        pfmt.InitByDefaultValues();
        data->pDocView->SetDefaultParagraphFormat(pfmt);
        SetAAMode(AA_Animation);

//         Text::TextFormat fmt(Memory::GetGlobalHeap());
//         fmt.SetFontName("Arial");
//         fmt.SetFontSize(12);
//         fmt.SetColor32(0xFF000000);
//         data->pDocView->SetDefaultTextFormat(fmt);
    }
    else
        SF_ASSERT(0); //?
}

void TreeText::Init(Text::DocView* docView)
{
    NodeData* data = GetWritableData(Change_TextLayout); //? Is the enum correct?
    data->pDocView = docView; 
}

Text::DocView* TreeText::GetDocView() const
{
    const NodeData* data = GetReadOnlyData();
    return data->pDocView;
}

void TreeText::SetText(const char* putf8Str, UPInt lengthInBytes)
{
    const NodeData* data = GetReadOnlyData();
    if (data->pDocView) 
        data->pDocView->SetText(putf8Str, lengthInBytes);
    else
        ; // What to do here??? @TODO
    NotifyLayoutChanged();
}

void TreeText::SetText(const wchar_t* pstr,  UPInt lengthInChars)
{
    const NodeData* data = GetReadOnlyData();
    if (data->pDocView) 
        data->pDocView->SetText(pstr, lengthInChars);
    else
        ; // What to do here??? @TODO
    NotifyLayoutChanged();
}

void TreeText::SetText(const String& str)
{
    const NodeData* data = GetReadOnlyData();
    if (data->pDocView) 
        data->pDocView->SetText(str.ToCStr(), str.GetLength());
    else
        ; // What to do here??? @TODO
    NotifyLayoutChanged();
}

String TreeText::GetText() const
{
    const NodeData* data = GetReadOnlyData();
    if (data->pDocView) 
        return data->pDocView->GetText();
    return "";
}

void TreeText::SetHtmlText(const char* putf8Str, 
                           UPInt lengthInBytes, 
                           Text::StyledText::HTMLImageTagInfoArray* pimgInfoArr)
{
    const NodeData* data = GetReadOnlyData();
    if (data->pDocView) 
        data->pDocView->ParseHtml(putf8Str, lengthInBytes, false, pimgInfoArr);
    else
        ; // What to do here??? @TODO
    NotifyLayoutChanged();
}

void TreeText::SetHtmlText(const wchar_t* pstr,  
                           UPInt lengthInChars, 
                           Text::StyledText::HTMLImageTagInfoArray* pimgInfoArr)
{
    const NodeData* data = GetReadOnlyData();
    if (data->pDocView) 
        data->pDocView->ParseHtml(pstr, lengthInChars, false, pimgInfoArr);
    else
        ; // What to do here??? @TODO
    NotifyLayoutChanged();
}

void TreeText::SetHtmlText(const String& str, Text::StyledText::HTMLImageTagInfoArray* pimgInfoArr)
{
    const NodeData* data = GetReadOnlyData();
    if (data->pDocView) 
        data->pDocView->ParseHtml(str.ToCStr(), str.GetLength(), false, pimgInfoArr);
    else
        ; // What to do here??? @TODO
    NotifyLayoutChanged();
}

String TreeText::GetHtmlText() const
{
    const NodeData* data = GetReadOnlyData();
    if (data->pDocView) 
        return data->pDocView->GetHtml();
    return "";
}

void TreeText::SetBounds(const RectF& r)
{
    const NodeData* data = GetReadOnlyData();
    if (data->pDocView) 
    {
        data->pDocView->SetViewRect(r);
    }
    else
        ; // What to do here??? @TODO
    NotifyLayoutChanged();
}

RectF TreeText::GetBounds() const
{
    const NodeData* data = GetReadOnlyData();
    if (data->pDocView) 
    {
        return data->pDocView->GetViewRect();
    }
    else
        ; // What to do here??? @TODO
    return RectF();
}

void TreeText::UpdateDefaultTextFormat(Text::DocView* pdoc, const Text::TextFormat& fmt)
{
    const Text::TextFormat* ptextFmt = pdoc->GetDefaultTextFormat();
    pdoc->SetDefaultTextFormat(ptextFmt->Merge(fmt));
}

void TreeText::UpdateDefaultParagraphFormat(Text::DocView* pdoc, const Text::ParagraphFormat& fmt)
{
    const Text::ParagraphFormat* pparaFmt = pdoc->GetDefaultParagraphFormat();
    pdoc->SetDefaultParagraphFormat(pparaFmt->Merge(fmt));
}

void TreeText::SetColor(Color c, UPInt startPos, UPInt endPos)
{
    const NodeData* data = GetReadOnlyData();
    if (data->pDocView) 
    {
        Text::TextFormat fmt(data->pDocView->GetHeap());
        fmt.SetColor(c);
        data->pDocView->SetTextFormat(fmt, startPos, endPos);
        UpdateDefaultTextFormat(data->pDocView, fmt);
    }
    else
        ; // What to do here??? @TODO
    NotifyLayoutChanged();
}

void TreeText::SetFont (const char* pfontName, UPInt startPos, UPInt endPos)
{
    const NodeData* data = GetReadOnlyData();
    if (data->pDocView) 
    {
        Text::TextFormat fmt(data->pDocView->GetHeap());
        fmt.SetFontName(pfontName);
        data->pDocView->SetTextFormat(fmt, startPos, endPos);
        UpdateDefaultTextFormat(data->pDocView, fmt);
    }
    else
        ; // What to do here??? @TODO
    NotifyLayoutChanged();
}

void TreeText::SetFontSize(float fontSize, UPInt startPos, UPInt endPos)
{
    const NodeData* data = GetReadOnlyData();
    if (data->pDocView) 
    {
        Text::TextFormat fmt(data->pDocView->GetHeap());
        fmt.SetFontSize(fontSize);
        data->pDocView->SetTextFormat(fmt, startPos, endPos);
        UpdateDefaultTextFormat(data->pDocView, fmt);
    }
    else
        ; // What to do here??? @TODO
    NotifyLayoutChanged();
}

void TreeText::SetFontStyle(FontStyle fontStyle, UPInt startPos, UPInt endPos)
{
    const NodeData* data = GetReadOnlyData();
    if (data->pDocView) 
    {
        Text::TextFormat fmt(data->pDocView->GetHeap());
        switch(fontStyle)
        {
        case Normal: 
            fmt.SetBold(false);
            fmt.SetItalic(false);
            break;
        case Bold:
            fmt.SetBold(true);
            fmt.SetItalic(false);
            break;
        case Italic:
            fmt.SetBold(false);
            fmt.SetItalic(true);
            break;
        case BoldItalic:
            fmt.SetBold(true);
            fmt.SetItalic(true);
            break;
        }
        data->pDocView->SetTextFormat(fmt, startPos, endPos);
        UpdateDefaultTextFormat(data->pDocView, fmt);
    }
    else
        ; // What to do here??? @TODO
    NotifyLayoutChanged();
}

void TreeText::SetLetterSpacing(float letterSpacing, UPInt startPos, UPInt endPos)
{
    const NodeData* data = GetReadOnlyData();
    if (data->pDocView) 
    {
        Text::TextFormat fmt(data->pDocView->GetHeap());
        fmt.SetLetterSpacing(letterSpacing);
        data->pDocView->SetTextFormat(fmt, startPos, endPos);
        UpdateDefaultTextFormat(data->pDocView, fmt);
    }
    else
        ; // What to do here??? @TODO
    NotifyLayoutChanged();
}

void TreeText::SetUnderline(bool underline, UPInt startPos, UPInt endPos)
{
    const NodeData* data = GetReadOnlyData();
    if (data->pDocView) 
    {
        Text::TextFormat fmt(data->pDocView->GetHeap());
        fmt.SetUnderline(underline);
        data->pDocView->SetTextFormat(fmt, startPos, endPos);
        UpdateDefaultTextFormat(data->pDocView, fmt);
    }
    else
        ; // What to do here??? @TODO
    NotifyLayoutChanged();
}

void TreeText::SetMultiline(bool multiline)
{
    const NodeData* data = GetReadOnlyData();
    if (data->pDocView) 
    {
        if (multiline)
            data->pDocView->SetMultiline();
        else
            data->pDocView->ClearMultiline();
    }
    else
        ; // What to do here??? @TODO
    NotifyLayoutChanged();
}

bool TreeText::IsMultiline() const
{
    const NodeData* data = GetReadOnlyData();
    if (data->pDocView) 
    {
        return data->pDocView->IsMultiline();
    }
    return false;
}

// Turns wordwrapping on/off
void TreeText::SetWordWrap(bool wordWrap)
{
    const NodeData* data = GetReadOnlyData();
    if (data->pDocView) 
    {
        if (wordWrap)
            data->pDocView->SetWordWrap();
        else
            data->pDocView->ClearWordWrap();
    }
    else
        ; // What to do here??? @TODO
    NotifyLayoutChanged();
}

// Returns state of wordwrapping.
bool TreeText::IsWordWrap() const
{
    const NodeData* data = GetReadOnlyData();
    if (data->pDocView) 
    {
        return data->pDocView->IsWordWrap();
    }
    return false;
}

void TreeText::SetAlignment(Alignment a)
{
    const NodeData* data = GetReadOnlyData();
    if (data->pDocView) 
    {
        Text::ParagraphFormat::AlignType pa;
        switch(a)
        {
        case Align_Right:   pa = Text::ParagraphFormat::Align_Right; break;
        case Align_Center:  pa = Text::ParagraphFormat::Align_Center; break;
        case Align_Justify: pa = Text::ParagraphFormat::Align_Justify; break;
        default:            pa = Text::ParagraphFormat::Align_Left; break;
        }
        Text::ParagraphFormat parafmt;
        parafmt.SetAlignment(pa);
        data->pDocView->SetParagraphFormat(parafmt);
        UpdateDefaultParagraphFormat(data->pDocView, parafmt);
    }
    else
        ; // What to do here??? @TODO
    NotifyLayoutChanged();
}

TreeText::Alignment  TreeText::GetAlignment() const
{
    const NodeData* data = GetReadOnlyData();
    if (data->pDocView) 
    {
        Text::ParagraphFormat parafmt;
        data->pDocView->GetTextAndParagraphFormat(NULL, &parafmt, 0);
        if (parafmt.IsAlignmentSet())
        {
            switch(parafmt.GetAlignment())
            {
            case Text::ParagraphFormat::Align_Right: return Align_Right;
            case Text::ParagraphFormat::Align_Center: return Align_Center;
            case Text::ParagraphFormat::Align_Justify: return Align_Justify;
            default:;
            }
        }
        return Align_Left;
    }
    else
        ; // What to do here??? @TODO
    return Align_Left;
}

void TreeText::SetVAlignment(VAlignment a)
{
    const NodeData* data = GetReadOnlyData();
    if (data->pDocView) 
    {
        Text::DocView::ViewVAlignment pa;
        switch(a)
        {
        case VAlign_Bottom:  pa = Text::DocView::VAlign_Bottom; break;
        case VAlign_Center:  pa = Text::DocView::VAlign_Center; break;
        default:             pa = Text::DocView::VAlign_Top; break;
        }
        data->pDocView->SetVAlignment(pa);
    }
    else
        ; // What to do here??? @TODO
    NotifyLayoutChanged();
}

TreeText::VAlignment  TreeText::GetVAlignment() const
{
    const NodeData* data = GetReadOnlyData();
    if (data->pDocView) 
    {
        VAlignment pa;
        switch(data->pDocView->GetVAlignment())
        {
        case Text::DocView::VAlign_Bottom: pa = VAlign_Bottom; break;
        case Text::DocView::VAlign_Center: pa = VAlign_Center; break;
        default: pa = VAlign_Top;
        }
        return pa;
    }
    else
        ; // What to do here??? @TODO
    return VAlign_Top;
}

void   TreeText::SetBorderColor(const Color& borderColor)
{
    SetBorderColor(borderColor.ToColor32());
}

void   TreeText::SetBorderColor(UInt32 borderColor)
{
    const NodeData* data = GetReadOnlyData();
    if (data->pDocView) 
    {
        data->pDocView->SetBorderColor(borderColor);
    }
    else
        ; // What to do here??? @TODO
    NotifyLayoutChanged();
}

Color TreeText::GetBorderColor() const 
{ 
    const NodeData* data = GetReadOnlyData();
    if (data->pDocView) 
    {
        return Color(data->pDocView->GetBorderColor()); 
    }
    return Color(0);
}

void   TreeText::SetBackgroundColor(const Color& bkgColor)
{
    SetBackgroundColor(bkgColor.ToColor32());
}

void   TreeText::SetBackgroundColor(UInt32 bkgColor)
{
    const NodeData* data = GetReadOnlyData();
    if (data->pDocView) 
    {
        data->pDocView->SetBackgroundColor(bkgColor);
    }
    else
        ; // What to do here??? @TODO
    NotifyLayoutChanged();
}

Color TreeText::GetBackgroundColor() const 
{ 
    const NodeData* data = GetReadOnlyData();
    if (data->pDocView) 
    {
        return Color(data->pDocView->GetBackgroundColor()); 
    }
    return Color(0);
}

void TreeText::SetAAMode(AAMode aa)
{
    const NodeData* data = GetReadOnlyData();
    if (data->pDocView) 
    {
        if (aa == AA_Readability)
            data->pDocView->SetAAForReadability();
        else
            data->pDocView->ClearAAForReadability();
    }
    else
        ; // What to do here??? @TODO
    NotifyLayoutChanged();
}

TreeText::AAMode TreeText::GetAAMode() const 
{ 
    const NodeData* data = GetReadOnlyData();
    if (data->pDocView) 
    {
        return (data->pDocView->IsAAForReadability()) ? AA_Readability : AA_Animation; 
    }
    return AA_Animation;
}

void TreeText::Filter::InitByDefaultValues() 
{
    // fill out by default values
    Text::TextFilter tf;
    Blur.BlurX = tf.BlurX;
    Blur.BlurY = tf.BlurY;
    Blur.Strength = tf.BlurStrength * 100.f;
    DropShadow.Flags = (UInt8)(tf.ShadowFlags | TreeText::FilterFlag_FineBlur);
    DropShadow.Color = ((tf.ShadowParams.Colors[0].ToColor32() & 0xFFFFFF) | ((UInt32)tf.ShadowAlpha << 24));
    DropShadow.Angle = (float)tf.ShadowAngle / 10.f;
    DropShadow.Distance = Text::FixpToPixels((float)tf.ShadowDistance);
}


void TreeText::SetFilters(const Filter* filters, UPInt filtersCnt)
{
    const NodeData* data = GetReadOnlyData();
    if (data->pDocView) 
    {
        Text::TextFilter tf;
        for (UPInt i = 0; i < filtersCnt; ++i)
        {
            switch(filters[i].Type)
            {
            case Filter_Blur:
                tf.BlurX        = Text::PixelsToFixp(filters[i].Blur.BlurX);
                tf.BlurY        = Text::PixelsToFixp(filters[i].Blur.BlurY);
                tf.BlurStrength = filters[i].Blur.Strength /100.f;
                break;
            case Filter_DropShadow:
                tf.ShadowFlags    = filters[i].DropShadow.Flags;
                //tf.ShadowBlurX    = Text::PixelsToFixp(filters[i].DropShadow.BlurX);
                //tf.ShadowBlurY    = Text::PixelsToFixp(filters[i].DropShadow.BlurY);
                //tf.ShadowStrength = filters[i].DropShadow.Strength /100.f;
                tf.ShadowParams.BlurX = Text::PixelsToFixp(filters[i].DropShadow.BlurX);
                tf.ShadowParams.BlurY = Text::PixelsToFixp(filters[i].DropShadow.BlurY);
                tf.ShadowParams.Strength = filters[i].DropShadow.Strength /100.f;

                tf.ShadowAlpha    = Color(filters[i].DropShadow.Color).GetAlpha();
                tf.ShadowAngle    = float(SF_DEGTORAD(filters[i].DropShadow.Angle));
                tf.ShadowDistance = (SInt16)Text::PixelsToFixp(filters[i].DropShadow.Distance);
              //  tf.ShadowOffsetX  = 0;
              //  tf.ShadowOffsetY  = 0;
                tf.ShadowParams.Colors[0] = filters[i].DropShadow.Color;
                //tf.ShadowColor    = filters[i].DropShadow.Color;
                tf.UpdateShadowOffset();
                break;
            case Filter_Glow:
                tf.ShadowFlags    = filters[i].Glow.Flags;
                //tf.ShadowBlurX    = Text::PixelsToFixp(filters[i].Glow.BlurX);
                //tf.ShadowBlurY    = Text::PixelsToFixp(filters[i].Glow.BlurY);
                //tf.ShadowStrength = filters[i].Glow.Strength/100.f;
                tf.ShadowParams.BlurX = Text::PixelsToFixp(filters[i].Glow.BlurX);
                tf.ShadowParams.BlurY = Text::PixelsToFixp(filters[i].Glow.BlurY);
                tf.ShadowParams.Strength = filters[i].Glow.Strength /100.f;

                tf.ShadowAlpha    = Color(filters[i].Glow.Color).GetAlpha();
                tf.ShadowAngle    = 0;
                tf.ShadowDistance = 0;
              //  tf.ShadowOffsetX  = 0;
             //   tf.ShadowOffsetY  = 0;
                tf.ShadowParams.Colors[0] = filters[i].Glow.Color;
                //tf.ShadowColor    = filters[i].Glow.Color;
                tf.UpdateShadowOffset();
                break;
            default:
                break;
            }
        }
        data->pDocView->SetFilters(tf);
    }
    else
        ; // What to do here??? @TODO
    NotifyLayoutChanged();
}

UPInt TreeText::GetFilters(Filter* filtersBuf, UPInt filtersCntInBuf)
{
    const NodeData* data = GetReadOnlyData();
    if (data->pDocView) 
    {
        const Text::TextFilter& tf = data->pDocView->GetFilters();

        UPInt i = 0;
        if ((tf.BlurX != 0 || tf.BlurY != 0) && i < filtersCntInBuf)
        {
            filtersBuf[i].Type = Filter_Blur;
            filtersBuf[i].Blur.BlurX = Text::FixpToPixels(tf.BlurX);
            filtersBuf[i].Blur.BlurY = Text::FixpToPixels(tf.BlurY);
            filtersBuf[i].Blur.Strength = tf.BlurStrength * 100.f;
            ++i;
        }
        if ((tf.ShadowParams.BlurX != 0 ||  tf.ShadowParams.BlurY != 0) && i < filtersCntInBuf)
        {
            // DropShadow?
            if ((tf.ShadowAngle != 0 || tf.ShadowDistance != 0))
            {
                filtersBuf[i].Type = Filter_DropShadow;
                filtersBuf[i].DropShadow.Flags = (UInt8)tf.ShadowFlags;
                filtersBuf[i].DropShadow.BlurX = Text::FixpToPixels(tf.ShadowParams.BlurX);
                filtersBuf[i].DropShadow.BlurY = Text::FixpToPixels(tf.ShadowParams.BlurY);
                filtersBuf[i].DropShadow.Strength = tf.ShadowParams.Strength * 100.f;
                filtersBuf[i].DropShadow.Color = (tf.ShadowParams.Colors[0].ToColor32() & 0xFFFFFF) | 
                                                 (((UInt32)tf.ShadowAlpha) << 24);
                filtersBuf[i].DropShadow.Angle = float(SF_RADTODEG(tf.ShadowAngle));
                filtersBuf[i].DropShadow.Distance = Text::FixpToPixels(tf.ShadowDistance);
                ++i;
            }
            else // Glow!
            {
                filtersBuf[i].Type = Filter_Glow;
                filtersBuf[i].Glow.Flags = (UInt8)tf.ShadowFlags;
                filtersBuf[i].Glow.BlurX = Text::FixpToPixels(tf.ShadowParams.BlurX);
                filtersBuf[i].Glow.BlurY = Text::FixpToPixels(tf.ShadowParams.BlurY);
                filtersBuf[i].Glow.Strength = tf.ShadowParams.Strength * 100.f;
                filtersBuf[i].Glow.Color = (tf.ShadowParams.Colors[0].ToColor32() & 0xFFFFFF) | 
                    (((UInt32)tf.ShadowAlpha) << 24);
                ++i;
            }
        }
        return i;
    }
    return 0;
}

void TreeText::ClearFilters()
{
    TreeText::Filter filter;
    SetFilters(&filter, 1);
}

void TreeText::NotifyLayoutChanged()
{
    NodeData* data = GetWritableData(Change_TextLayout); 
    data->TextFlags |= NodeData::TextFlags_RebuildLayout; 
    AddToPropagate();
}

SizeF TreeText::GetTextSize()
{
    const NodeData* data = GetReadOnlyData();
    if (data->pDocView) 
    {
        SizeF sz(data->pDocView->GetTextWidth(), data->pDocView->GetTextHeight());
        sz.Expand(GFX_TEXT_GUTTER*2);
        return sz;
    }
    return SizeF(0,0);
}

}} // Scaleform::Render
