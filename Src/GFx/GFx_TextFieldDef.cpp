/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_TextFieldDef.cpp
Content     :   Common definitions for text field
Created     :   Nov, 2009
Authors     :   Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/GFx_TextFieldDef.h"
#include "GFx/GFx_PlayerImpl.h"
#include "GFx/GFx_LoadProcess.h"
#include "GFx/GFx_TagLoaders.h"
#include "Render/Text/Text_StyledText.h"

namespace Scaleform {
namespace GFx {

namespace Text
{
    using Render::Text::Allocator;
}

//
// TextFieldDef
//
TextFieldDef::TextFieldDef()
:
FontId(0),
TextHeight(1.0f),
MaxLength(0),
LeftMargin(0.0f),
RightMargin(0.0f),
Indent(0.0f),
Leading(0.0f),
Flags(0),
Alignment(ALIGN_LEFT)
{
    ColorV.SetColor(0, 0, 0, 255);
    TextRect.Clear();
}
TextFieldDef::~TextFieldDef()
{
}

void    TextFieldDef::Read(LoadProcess* p, TagType tagType)
{
    SF_UNUSED(tagType);        
    SF_ASSERT(tagType == Tag_DefineEditText);
    Stream* in = p->GetStream();

    in->ReadRect(&TextRect);

    in->LogParse("  TextRect = { l: %f, t: %f, r: %f, b: %f }\n", 
        (float)TextRect.x1, (float)TextRect.y1, (float)TextRect.x2, (float)TextRect.y2);

    in->Align();
    bool    hasText = in->ReadUInt(1) ? true : false;
    SetWordWrap(in->ReadUInt(1) != 0);
    SetMultiline(in->ReadUInt(1) != 0);
    SetPassword(in->ReadUInt(1) != 0);
    SetReadOnly(in->ReadUInt(1) != 0);

    in->LogParse("  WordWrap = %d, Multiline = %d, Password = %d, ReadOnly = %d\n", 
        (int)IsWordWrap(), (int)IsMultiline(), (int)IsPassword(), (int)IsReadOnly());

    bool    hasColor = in->ReadUInt(1) ? true : false;
    bool    hasMaxLength = in->ReadUInt(1) ? true : false;
    bool    hasFont = in->ReadUInt(1) ? true : false;
    bool    hasFontClass = in->ReadUInt(1) ? true : false;

    SetAutoSize(in->ReadUInt(1) != 0);
    bool    hasLayout = in->ReadUInt(1) ? true : false;
    SetSelectable(in->ReadUInt(1) == 0);
    SetBorder(in->ReadUInt(1) != 0);
    SetWasStatic(in->ReadUInt(1) != 0);

    // In SWF 8 text is *ALWAYS* marked as HTML.
    // Correction, AB: no, only if kerning is ON
    // In SWF <= 7, that is not the case.
    SetHtml(in->ReadUInt(1) != 0);
    SetUseDeviceFont(in->ReadUInt(1) == 0);

    in->LogParse("  AutoSize = %d, Selectable = %d, Border = %d, Html = %d, UseDeviceFont = %d\n", 
        (int)IsAutoSize(), (int)IsSelectable(), (int)IsBorder(), (int)IsHtml(), (int)DoesUseDeviceFont());

    if (hasFont)
    {
        FontId = ResourceId(in->ReadU16());
        in->LogParse("  HasFont: font id = %d\n", FontId.GetIdIndex());

        // Create a font resource handle.
        ResourceHandle hres;
        p->GetResourceHandle(&hres, FontId);
        pFont.SetFromHandle(hres);
    }
    else if (hasFontClass)
    {
        in->ReadString(&FontClass);
        in->LogParse("  HasFontClass: font class = %s\n", FontClass.ToCStr());
    }
    if (hasFont || hasFontClass)
    {
        TextHeight = (float) in->ReadU16();
        in->LogParse("  FontHeight = %f\n", TwipsToPixels(TextHeight));
    }

    if (hasColor)
    {               
        in->ReadRgba(&ColorV);
        in->LogParse("  HasColor\n");
    }

    if (hasMaxLength)
    {
        MaxLength = in->ReadU16();
        in->LogParse("  HasMaxLength: len = %d\n", MaxLength);
    }

    if (hasLayout)
    {
        SetHasLayout();
        Alignment = (alignment) in->ReadU8();
        LeftMargin = (float) in->ReadU16();
        RightMargin = (float) in->ReadU16();
        Indent = (float) in->ReadS16();
        Leading = (float) in->ReadS16();
        in->LogParse("  HasLayout: alignment = %d, leftmarg = %f, rightmarg = %f, indent = %f, leading = %f\n", 
            (int)Alignment, LeftMargin, RightMargin, Indent, Leading);
    }

    in->ReadString(&VariableName);
    if (hasText)    
        in->ReadString(&DefaultText);    

    in->LogParse("EditTextChar, varname = %s, text = %s\n",
        VariableName.ToCStr(), DefaultText.ToCStr());
}

void TextFieldDef::InitEmptyTextDef()
{
    SetEmptyTextDef();

    ColorV = Color((UByte)0, (UByte)0, (UByte)0, (UByte)255);
    TextHeight = PixelsToTwips(12.f);
    SetReadOnly();
    SetSelectable();
    SetUseDeviceFont();
    SetAAForReadability();
    // font name?
}

// Read a DefineText tag.
void    SF_STDCALL GFx_DefineEditTextLoader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_ASSERT(tagInfo.TagType == Tag_DefineEditText);

    UInt16  characterId = p->ReadU16();

    Ptr<GFx::TextFieldDef> pch = 
        *SF_HEAP_NEW_ID(p->GetLoadHeap(),StatMD_CharDefs_Mem) GFx::TextFieldDef();
    p->LogParse("EditTextChar, id = %d\n", characterId);
    pch->Read(p, tagInfo.TagType);

    p->AddResource(ResourceId(characterId), pch);
}


#ifdef GFX_ENABLE_TEXT_INPUT
////////////////////////////////////////
// TextKeyMap

TextKeyMap::TextKeyMap() : State(State_TextKeyMap)
{

}

namespace
{
    struct KeyMapEntryComparator
    {
        static int Compare(const TextKeyMap::KeyMapEntry& p1, const TextKeyMap::KeyMapEntry& p2)
        {
            return (int)p1.KeyCode - (int)p2.KeyCode;
        }
        static int Less(const TextKeyMap::KeyMapEntry& p1, const TextKeyMap::KeyMapEntry& p2)
        {
            return Compare(p1, p2) < 0;
        }
    };

    struct KeyCodeComparator
    {
        static int Compare(const TextKeyMap::KeyMapEntry& p1, unsigned keyCode)
        {
            return (int)p1.KeyCode - (int)keyCode;
        }
        static int Less(const TextKeyMap::KeyMapEntry& p1, unsigned keyCode)
        {
            return Compare(p1, keyCode) < 0;
        }
    };
}

void TextKeyMap::AddKeyEntry(const KeyMapEntry& keyMapEntry)
{
    UPInt i = Alg::LowerBound(Map, keyMapEntry, KeyMapEntryComparator::Less);
    Map.InsertAt(i, keyMapEntry);
}

const TextKeyMap::KeyMapEntry* TextKeyMap::FindFirstEntry(unsigned keyCode) const
{
    UPInt i = Alg::LowerBound(Map, keyCode, KeyCodeComparator::Less);
    if (i < Map.GetSize() && Map[i].KeyCode == keyCode)
        return &Map[i];
    return NULL;
}

const TextKeyMap::KeyMapEntry* TextKeyMap::FindNextEntry(const KeyMapEntry* pcur) const
{
    UPInt curIdx = pcur - &Map[0];
    if (curIdx + 1 < Map.GetSize() && Map[curIdx + 1].KeyCode == pcur->KeyCode)
    {
        return &Map[curIdx + 1];
    }
    return NULL;
}

const TextKeyMap::KeyMapEntry* TextKeyMap::Find(unsigned keyCode, const KeyModifiers& specKeys, KeyState state) const
{
    const KeyMapEntry* p = FindFirstEntry(keyCode);
    while(p)
    {
        if (p->mState == state && (p->SpecKeysPressed & specKeys.States) == p->SpecKeysPressed)
            return p;
        p = FindNextEntry(p);
    }
    return NULL;
}


TextKeyMap* TextKeyMap::InitWindowsKeyMap()
{
    AddKeyEntry(TextKeyMap::KeyMapEntry(KeyAct_EnterSelectionMode, Key::Shift));
    AddKeyEntry(TextKeyMap::KeyMapEntry(KeyAct_LeaveSelectionMode, Key::Shift, 0, State_Up));

    AddKeyEntry(TextKeyMap::KeyMapEntry(KeyAct_Up, Key::Up));
    AddKeyEntry(TextKeyMap::KeyMapEntry(KeyAct_Down, Key::Down));
    AddKeyEntry(TextKeyMap::KeyMapEntry(KeyAct_Left, Key::Left));
    AddKeyEntry(TextKeyMap::KeyMapEntry(KeyAct_Right, Key::Right));
    AddKeyEntry(TextKeyMap::KeyMapEntry(KeyAct_PageUp, Key::PageUp));
    AddKeyEntry(TextKeyMap::KeyMapEntry(KeyAct_PageDown, Key::PageDown));
    AddKeyEntry(TextKeyMap::KeyMapEntry(KeyAct_LineHome, Key::Home));
    AddKeyEntry(TextKeyMap::KeyMapEntry(KeyAct_LineEnd, Key::End));
    AddKeyEntry(TextKeyMap::KeyMapEntry(KeyAct_PageHome, Key::PageUp, KeyModifiers::Key_CtrlPressed));
    AddKeyEntry(TextKeyMap::KeyMapEntry(KeyAct_PageEnd, Key::PageDown, KeyModifiers::Key_CtrlPressed));
    AddKeyEntry(TextKeyMap::KeyMapEntry(KeyAct_DocHome, Key::Home, KeyModifiers::Key_CtrlPressed));
    AddKeyEntry(TextKeyMap::KeyMapEntry(KeyAct_DocEnd, Key::End, KeyModifiers::Key_CtrlPressed));
    AddKeyEntry(TextKeyMap::KeyMapEntry(KeyAct_Backspace, Key::Backspace));
    AddKeyEntry(TextKeyMap::KeyMapEntry(KeyAct_Delete, Key::Delete));
    AddKeyEntry(TextKeyMap::KeyMapEntry(KeyAct_Return, Key::Return));

    AddKeyEntry(TextKeyMap::KeyMapEntry(KeyAct_Copy, Key::C, KeyModifiers::Key_CtrlPressed));
    AddKeyEntry(TextKeyMap::KeyMapEntry(KeyAct_Copy, Key::Insert, KeyModifiers::Key_CtrlPressed));
    AddKeyEntry(TextKeyMap::KeyMapEntry(KeyAct_Paste, Key::V, KeyModifiers::Key_CtrlPressed));
    AddKeyEntry(TextKeyMap::KeyMapEntry(KeyAct_Paste, Key::Insert, KeyModifiers::Key_ShiftPressed));
    AddKeyEntry(TextKeyMap::KeyMapEntry(KeyAct_Cut, Key::X, KeyModifiers::Key_CtrlPressed));
    AddKeyEntry(TextKeyMap::KeyMapEntry(KeyAct_Cut, Key::Delete, KeyModifiers::Key_ShiftPressed));
    AddKeyEntry(TextKeyMap::KeyMapEntry(KeyAct_SelectAll, Key::A, KeyModifiers::Key_CtrlPressed));
    return this;
}

TextKeyMap* TextKeyMap::InitMacKeyMap()
{
    return InitWindowsKeyMap();//TODO
}
#endif //SF_NO_TEXT_INPUT_SUPPORT

// Read CSMTextSettings tag
void    SF_STDCALL GFx_CSMTextSettings(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED(tagInfo);        
    SF_ASSERT(tagInfo.TagType == Tag_CSMTextSettings);

    UInt16  characterId = p->ReadU16();

    Stream*  pin = p->GetStream();    

    unsigned flagType  = pin->ReadUInt(2);
    unsigned gridFit   = pin->ReadUInt(3);
    float    thickness = pin->ReadFloat();
    float    sharpness = pin->ReadFloat();

    if (pin->IsVerboseParse())
    {
        static const char* const gridfittypes[] = { "None", "Pixel", "LCD" };
        p->LogParse("CSMTextSettings, id = %d\n", characterId);
        p->LogParse("  FlagType = %s, GridFit = %s\n", 
            (flagType == 0) ? "System" : "Internal", gridfittypes[gridFit] );
        p->LogParse("  Thinkness = %f, Sharpnesss = %f\n", 
            (float)thickness, (float)sharpness);
    }

    ResourceHandle handle;
    if (p->GetResourceHandle(&handle, ResourceId(characterId)))
    {
        Resource* rptr = handle.GetResourcePtr();
        if (rptr)
        { 
            if (rptr->GetResourceType() == Resource::RT_EditTextDef)
            {
                GFx::TextFieldDef* pch = static_cast<GFx::TextFieldDef*>(rptr);
                pch->SetAAForReadability();
            }
            else if (rptr->GetResourceType() == Resource::RT_TextDef)
            {
                GFx::StaticTextDef* pch = static_cast<GFx::StaticTextDef*>(rptr);
                pch->SetAAForReadability();
            }
        }
    }
}

//////////////////////////////////////////////////
Clipboard::Clipboard() : 
State(State_Clipboard), pStyledText(NULL)
{
}

Clipboard::~Clipboard() 
{
    ReleaseStyledText();
}

void Clipboard::ReleaseStyledText()
{
    if (pStyledText)
    {
        pStyledText->Release();
        pStyledText = NULL;
    }
}

void Clipboard::SetPlainText(const wchar_t* ptext, UPInt len)
{
    PlainText.SetString(ptext, len);
    OnTextStore(PlainText.ToWStr(), PlainText.GetLength());
}

void Clipboard::SetText(const String& str)
{
    ReleaseStyledText();
    UPInt len = str.GetLength();
    PlainText.Resize(len + 1);
    UTF8Util::DecodeStringSafe(PlainText.GetBuffer(), len + 1, str.ToCStr(), (SPInt)str.GetSize());
    OnTextStore(PlainText.ToWStr(), PlainText.GetLength());
}

void Clipboard::SetText(const wchar_t* ptext, UPInt len)
{
    ReleaseStyledText();
    SetPlainText(ptext, len);
}

const WStringBuffer& Clipboard::GetText()
{
    return PlainText;
}

void Clipboard::SetStyledText(class Text::StyledText* pstyledText)
{
    if (pStyledText)
        pStyledText->Release();
    // need to make a copy of passed styledText into the global heap one.
    MemoryHeap* pheap = Memory::GetGlobalHeap();
    Ptr<Text::Allocator> pallocator = *SF_HEAP_NEW(pheap) Text::Allocator(pheap, Text::Allocator::Flags_Global);
    pStyledText = SF_HEAP_NEW(pheap) Text::StyledText(pallocator);
    pstyledText->CopyStyledText(pStyledText);
}

void Clipboard::SetTextAndStyledText(const wchar_t* ptext, UPInt len, class Text::StyledText* pstyledText)
{
    SetStyledText(pstyledText);
    SetText(ptext, len);
}

class Text::StyledText* Clipboard::GetStyledText()
{
    return pStyledText;
}

}} // namespace Scaleform::GFx
