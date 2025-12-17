/**************************************************************************

Filename    :   Text_Highlight.cpp
Content     :   Highlighting functionality for rich text.
Created     :   August 6, 2007
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render/Text/Text_Highlight.h"
#include "Render/Text/Text_DocView.h"
#include "Kernel/SF_Alg.h"

namespace Scaleform { 
namespace Render {
namespace Text {

struct IdComparator
{
    static int Compare(const HighlightDesc& p1, unsigned id)
    {
        return (int)p1.Id - (int)id;
    }
    static int Less(const HighlightDesc& p1, unsigned id)
    {
        return Compare(p1, id) < 0;
    }
};

//////////////////////////////////////////////////////////////////////////
// HighlightDesc
// 
void HighlightInfo::Append(const HighlightInfo& mergee)
{
    if (!HasUnderlineStyle() && mergee.HasUnderlineStyle())
    {
        SetUnderlineStyle(mergee.GetUnderlineStyle());
    }
    if (!HasBackgroundColor() && mergee.HasBackgroundColor())
        SetBackgroundColor(mergee.GetBackgroundColor());
    if (!HasTextColor() && mergee.HasTextColor())
        SetTextColor(mergee.GetTextColor());
    if (!HasUnderlineColor() && mergee.HasUnderlineColor())
        SetUnderlineColor(mergee.GetUnderlineColor());
}

void HighlightInfo::Prepend(const HighlightInfo& mergee)
{
    if (mergee.HasUnderlineStyle())
    {
        SetUnderlineStyle(mergee.GetUnderlineStyle());
    }
    if (mergee.HasBackgroundColor())
        SetBackgroundColor(mergee.GetBackgroundColor());
    if (mergee.HasTextColor())
        SetTextColor(mergee.GetTextColor());
    if (mergee.HasUnderlineColor())
        SetUnderlineColor(mergee.GetUnderlineColor());
}

bool HighlightInfo::IsEqualWithFlags(const HighlightInfo& right, unsigned flags)
{
    if (flags & Flag_UnderlineStyle)
    {
        if (GetUnderlineStyle() != right.GetUnderlineStyle())
            return false;
    }
    if (flags & Flag_Background)
    {
        if (HasBackgroundColor() != right.HasBackgroundColor() || 
            GetBackgroundColor() != right.GetBackgroundColor())
            return false;
    }
    if (flags & Flag_TextColor)
    {
        if (HasTextColor() != right.HasTextColor() || 
            GetTextColor() != right.GetTextColor())
            return false;
    }
    if (flags & Flag_UnderlineColor)
    {
        if (HasUnderlineColor() != right.HasUnderlineColor() || 
            GetUnderlineColor() != right.GetUnderlineColor())
            return false;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////
// Highlighter
// 
Highlighter::Highlighter() :
    LastId(0),CorrectionPos(0),CorrectionLen(0), Valid(false), HasUnderline(0) 
{
}

HighlightDesc* Highlighter::CreateNewHighlighter(HighlightDesc* pdesc)
{
    Invalidate();
    do 
    {
        ++LastId;
        HighlightDesc* pfdesc = GetHighlighterPtr(LastId);
        if (!pfdesc)
        {
            SF_ASSERT(pdesc);
            pdesc->Id = LastId;
            UPInt i = Alg::LowerBound(Highlighters, pdesc->Id, IdComparator::Less);
            Highlighters.InsertAt(i, *pdesc);
            return &Highlighters[i];       
        }
    } while(1);
    return NULL;
}

HighlightDesc* Highlighter::CreateHighlighter(const HighlightDesc& desc)
{
    SF_ASSERT(desc.Id > 0); // Id should be initialized to value other than zero

    Invalidate();
    // check, is id already occupied
    HighlightDesc* pdesc = GetHighlighterPtr(desc.Id);
    if (!pdesc)
    {
        UPInt i = Alg::LowerBound(Highlighters, desc.Id, IdComparator::Less);
        Highlighters.InsertAt(i, desc);
        return &Highlighters[i];       
    }
    return NULL;
}

HighlightDesc* Highlighter::GetHighlighterPtr(unsigned id)
{
    UPInt i = Alg::LowerBound(Highlighters, id, IdComparator::Less);
    if (i < Highlighters.GetSize() && Highlighters[i].Id == id)
        return &Highlighters[i];
    return NULL;
}


HighlightDesc Highlighter::GetHighlighter(unsigned id) const
{
    UPInt i = Alg::LowerBound(Highlighters, id, IdComparator::Less);
    if (i < Highlighters.GetSize() && Highlighters[i].Id == id)
        return Highlighters[i];
    return HighlightDesc();
}

bool Highlighter::SetHighlighter(unsigned id, const HighlightInfo& info)
{
    HighlightDesc* pdesc = GetHighlighterPtr(id);
    if (pdesc)
    {
        pdesc->Info = info;
        Invalidate();
        return true;
    }
    return false;
}

bool Highlighter::FreeHighlighter(unsigned id)
{
    UPInt i = Alg::LowerBound(Highlighters, id, IdComparator::Less);
    if (i < Highlighters.GetSize() && Highlighters[i].Id == id)
    {
        Highlighters.RemoveAt(i);
        Invalidate();
        return true;
    }
    return false;
}

HighlighterPosIterator Highlighter::GetPosIterator(UPInt startPos, UPInt len) const
{
    return HighlighterPosIterator(this, startPos, len);
}

HighlighterRangeIterator Highlighter::GetRangeIterator(UPInt startPos, unsigned flags) const
{
    return HighlighterRangeIterator(this, startPos, flags);
}

void Highlighter::UpdateGlyphIndices(const CompositionStringBase* pcs)
{
    CorrectionPos = CorrectionLen = 0;
#ifndef SF_NO_IME_SUPPORT
    if (pcs)
    {
        CorrectionPos = pcs->GetPosition();
        CorrectionLen = pcs->GetLength();
    }
#else
    SF_UNUSED(pcs);
#endif
    Invalidate();
    for(UPInt i = 0, n = Highlighters.GetSize(); i < n; ++i)
    {
        HighlightDesc& desc = Highlighters[i];

        desc.AdjStartPos = desc.StartPos;
        desc.GlyphNum   = desc.Length;
        if (CorrectionLen > 0)
        {
            if (desc.ContainsPos(CorrectionPos))
            {
                if (desc.Offset >= 0)
                    desc.AdjStartPos += desc.Offset;
                else
                    desc.GlyphNum += CorrectionLen;
            }
            else if (desc.StartPos > CorrectionPos)
            {
                desc.AdjStartPos += CorrectionLen;
            }
        }
    }
}

bool Highlighter::HasUnderlineHighlight() const
{
    if (HasUnderline == 0)
    {
        HasUnderline = -1;
        for(UPInt i = 0, n = Highlighters.GetSize(); i < n; ++i)
        {
            const HighlightDesc& desc = Highlighters[i];
            if (desc.Info.HasUnderlineStyle())
            {
                HasUnderline = 1;
                break;
            }
        }
    }
    return HasUnderline == 1;
}

//
// Merge with the existing descs
// If no merge desc is found, it is added to the set
//
void Highlighter::Add(HighlightDesc& merge)
{
    //SF_DEBUG_MESSAGE2(1, "Highlighter::Add - %d, %d", merge.StartPos, merge.Length);
    bool bmerged = false;
    const UPInt& mergeStart = merge.StartPos;
    UPInt mergeEnd    = merge.StartPos + merge.Length;
    for (UPInt i = 0, n = Highlighters.GetSize(); i < n; ++i)
    {        
        HighlightDesc& desc = Highlighters[i];
        //SF_DEBUG_MESSAGE2(1, "\tchecking.. %d, %d", desc.StartPos, desc.Length);
        // Can desc be merged
        const UPInt& descStart    = desc.StartPos;
        UPInt descEnd       = desc.StartPos + desc.Length;
        if (mergeStart >= descStart && mergeStart <= descEnd)
        {
            // Special case: merge in middle? Done
            if (mergeEnd <= descEnd)
                continue;
            else
            {
                // Append on right side
                SF_ASSERT(mergeEnd > descEnd);
                UPInt slice = (mergeEnd - descEnd);
                desc.Length += slice;
                desc.GlyphNum = desc.Length;
                bmerged = true;
                Invalidate();
            }
        }
        else if (descStart > mergeStart && descStart < mergeEnd)
        {
            // Append on left side
            SF_ASSERT(descStart > mergeStart);
            UPInt slice = (descStart - mergeStart);
            desc.StartPos -= slice;
            desc.AdjStartPos = desc.StartPos;
            desc.Length += slice;
            desc.GlyphNum = desc.Length;
            bmerged = true;
            Invalidate();
            
            // Special case: expand on right side too
            if (descEnd <= mergeEnd)
            {
                slice = (mergeEnd - descEnd);
                SF_ASSERT(mergeEnd >= descEnd);
                desc.Length += slice;
                desc.GlyphNum = desc.Length;
            }
        }
    }

    if (!bmerged)
    {
        HighlightDesc* pdesc = CreateNewHighlighter(&merge);
        SF_ASSERT(pdesc);
        SF_UNUSED(pdesc);
    }

    /*
    SF_DEBUG_MESSAGE(1, ">> Highlighter states");
    for (UPInt i = 0, n = Highlighters.GetSize(); i < n; ++i)
    {
        HighlightDesc& desc = Highlighters[i];
        SF_DEBUG_MESSAGE2(1, "\t%d, %d", desc.StartPos, desc.Length);
    }
    */
}


//
// Cut existing descs with new one
// No effect if no existing descs are affected
//
void Highlighter::Remove(const HighlightDesc& cut)
{
    //SF_DEBUG_MESSAGE2(1, "Highlighter::Remove - %d, %d", cut.StartPos, cut.Length);
    Array<HighlightDesc>   addDescs;

    const UPInt& cutStart = cut.StartPos;
    UPInt cutEnd    = cut.StartPos + cut.Length;
    for (UPInt i = 0; i < Highlighters.GetSize(); ++i)
    {
        HighlightDesc& desc = Highlighters[i];
        //SF_DEBUG_MESSAGE2(1, "\tchecking.. %d, %d", desc.StartPos, desc.Length);
        // Is desc affected?
        UPInt descStart    = desc.StartPos;
        UPInt descEnd      = desc.StartPos + desc.Length;
        if (cutStart > descStart && cutStart < descEnd)
        {
            // Special case: cut in middle? This causes a split
            if (cutEnd < descEnd)
            {
                // Fix left side
                SF_ASSERT(cutStart > descStart);
                UPInt slice = (cutStart - descStart);
                desc.Length = slice;
                desc.GlyphNum = desc.Length;
                // Spawn new right side
                SF_ASSERT(descEnd > cutEnd);
                slice = (descEnd - cutEnd);
                HighlightDesc newdesc = desc;                                
                newdesc.StartPos = cutEnd;
                newdesc.AdjStartPos = newdesc.StartPos;
                newdesc.Length = slice;
                newdesc.GlyphNum = newdesc.Length;
                addDescs.PushBack(newdesc);
                Invalidate();
            }
            else
            {
                // Cut right side
                SF_ASSERT(descEnd >= cutStart);
                UPInt slice = (descEnd - cutStart);
                desc.Length -= slice;
                desc.GlyphNum = desc.Length;
                Invalidate();
            }
        }
        else if (descStart >= cutStart && descStart < cutEnd)
        {
            // Special case: cut whole desc? Drop it
            if (descEnd <= cutEnd)
            {
                Highlighters.RemoveAt(i--);
                Invalidate();
            }
            else
            {
                // Cut left side
                SF_ASSERT(cutEnd >= descStart);
                UPInt slice = (cutEnd - descStart);
                desc.StartPos += slice;
                desc.AdjStartPos = desc.StartPos;
                desc.Length -= slice;
                desc.GlyphNum = desc.Length;
                Invalidate();
            }
        }  
    }

    for (UPInt i=0; i < addDescs.GetSize(); ++i)
    {
        CreateNewHighlighter(&addDescs[i]);
    }

    /*
    SF_DEBUG_MESSAGE(1, ">> Highlighter states");
    for (UPInt i = 0, n = Highlighters.GetSize(); i < n; ++i)
    {
        HighlightDesc& desc = Highlighters[i];
        SF_DEBUG_MESSAGE2(1, "\t%d, %d", desc.StartPos, desc.Length);
    }
    */
}


void Highlighter::SetSelectColor(const Color& color)
{
    for (UPInt i = 0, n = Highlighters.GetSize(); i < n; ++i)
    {
        HighlightDesc& desc = Highlighters[i];
        desc.Info.BackgroundColor = color;
    }
    Invalidate();
}


bool    Highlighter::IsAnyCharSelected(UPInt selectStart, UPInt selectEnd)
{
    for (UPInt i = 0, n = Highlighters.GetSize(); i < n; ++i)
    {
        HighlightDesc& desc = Highlighters[i];
        UPInt& descStart = desc.StartPos;
        UPInt descEnd = descStart + desc.Length;
        if ( (selectStart >= descStart && selectStart < descEnd) ||
             (descStart >= selectStart && descStart < selectEnd) )
        return true;
    }
    return false;
}


//////////////////////////////////////////////////////////////////////////
// HighlighterRangeIterator
// 
HighlighterRangeIterator::HighlighterRangeIterator
(const Highlighter* pmanager, UPInt startPos, unsigned flags) : 
    pManager(pmanager), CurTextPos(startPos), CurRangeIndex(0), Flags(flags)
{
    InitCurDesc();
}

void HighlighterRangeIterator::InitCurDesc()
{
    UPInt nearestNextPos;
    HighlightDesc desc;
    do 
    {
        nearestNextPos = SF_MAX_UPINT;
        for (UPInt i = 0, n = pManager->Highlighters.GetSize(); i < n; ++i)
        {
            const HighlightDesc& curd = pManager->Highlighters[i];

            if ((curd.Info.Flags & Flags) != 0)
            {
                if (curd.ContainsIndex(CurTextPos))
                {
                    if (desc.IsEmpty())
                    {
                        desc     = curd;
                        desc.GlyphNum -= CurTextPos - desc.AdjStartPos;
                        desc.AdjStartPos= CurTextPos;
                        if (desc.AdjStartPos + desc.GlyphNum > nearestNextPos)
                        {
                            desc.GlyphNum = nearestNextPos - desc.AdjStartPos;
                            nearestNextPos = desc.AdjStartPos + desc.GlyphNum;
                        }
                        //c//urLen   = curd.GlyphNum;
                    }
                    else
                    {
                        if ((curd.Info.Flags & Flags) != Flags)
                        {
                            desc.Info.Prepend(curd.Info);
                            desc.GlyphNum = Alg::PMin(curd.AdjStartPos + curd.GlyphNum, desc.AdjStartPos + desc.GlyphNum) - desc.AdjStartPos;
                            nearestNextPos = desc.AdjStartPos + desc.GlyphNum;
                        }
                    }
                }
                if (curd.AdjStartPos > CurTextPos)
                {
                    nearestNextPos = Alg::PMin(nearestNextPos, curd.AdjStartPos);
                    if (!desc.IsEmpty() && desc.AdjStartPos + desc.GlyphNum > nearestNextPos)
                        desc.GlyphNum = nearestNextPos - desc.AdjStartPos;
                }
            }
        }
        CurDesc = desc;
        CurTextPos = nearestNextPos;
    } while(desc.IsEmpty() && nearestNextPos != SF_MAX_UPINT);
}


HighlightDesc HighlighterRangeIterator::operator*()
{
    return CurDesc;
}

void HighlighterRangeIterator::operator++(int)
{
    if (!IsFinished())
    {
        //CurTextPos = CurDesc.AdjStartPos + CurDesc.GlyphNum;
        InitCurDesc();
    }
}

bool HighlighterRangeIterator::IsFinished() const
{
    return !(CurRangeIndex < pManager->Highlighters.GetSize()) || CurDesc.GlyphNum == 0;
}

//////////////////////////////////////////////////////////////////////////
// HighlighterPosIterator
// 
HighlighterPosIterator::HighlighterPosIterator
    (const Highlighter* pmanager, UPInt startPos, UPInt len) : 
    pManager(pmanager), CurAdjStartPos(startPos), NumGlyphs(len)
{
    InitCurDesc();
}

void HighlighterPosIterator::InitCurDesc()
{
    if (!IsFinished())
    {
        HighlightDesc desc;
        for (UPInt i = 0, n = pManager->Highlighters.GetSize(); i < n; ++i)
        {
            const HighlightDesc& curd = pManager->Highlighters[i];
            if (curd.ContainsIndex(CurAdjStartPos))
                desc.Info.Prepend(curd.Info);
        }
        CurDesc = desc;
        CurDesc.GlyphNum   = 1;
    }
    else
    {
        CurDesc.Info.Reset();
        CurDesc.GlyphNum   = 0;
    }
    CurDesc.AdjStartPos = CurAdjStartPos;
    CurDesc.Id          = 0;
}



void HighlighterPosIterator::operator++(int)
{
    if (!IsFinished())
    {
        ++CurAdjStartPos;
        InitCurDesc();
    }
}

void HighlighterPosIterator::operator+=(UPInt p)
{
    if (!IsFinished() && p > 0)
    {
        CurAdjStartPos += p;
        InitCurDesc();
    }
}

bool HighlighterPosIterator::IsFinished() const
{
    return CurAdjStartPos >= NumGlyphs;
}
}}} // Scaleform::Render::Text
