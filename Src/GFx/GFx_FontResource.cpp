/**************************************************************************

Filename    :   GFx_FontResource.cpp
Content     :   Representation of SWF/GFX font data and resources.
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/GFx_FontResource.h"
#include "GFx/GFx_Stream.h"
#include "GFx/GFx_Log.h"
#include "Kernel/SF_Debug.h"
#include "GFx/GFx_ShapeSwf.h"
#include "Render/Render_Math2D.h"
#include "Render/Render_GlyphParam.h"
#include "Kernel/SF_Std.h"
#include "Kernel/SF_File.h"
#include "Kernel/SF_Alg.h"

#include "GFx/GFx_LoadProcess.h"
#include "Kernel/SF_MsgFormat.h"

namespace Scaleform { namespace GFx {

// ***** TextureGlyphData
TextureGlyphData::TextureGlyphData(const TextureGlyphData& orig) 
: PackTextureConfig(orig.PackTextureConfig), FileCreation(orig.FileCreation)
{
    TextureGlyphs.Resize(orig.TextureGlyphs.GetSize());
    for (UPInt i = 0, n = orig.TextureGlyphs.GetSize(); i < n; ++i)
    {
        TextureGlyphs[i] = orig.TextureGlyphs[i];
    }
    ImageResourceHash::ConstIterator iter = orig.GlyphsTextures.Begin();
    for (; iter != orig.GlyphsTextures.End(); ++iter)
    {
        GlyphsTextures.Set(iter->First, iter->Second);
    }
}


// Return a pointer to a TextureGlyph struct Corresponding to
// the given GlyphIndex, if we have one.  Otherwise return NULL.
const TextureGlyph&  TextureGlyphData::GetTextureGlyph(unsigned glyphIndex) const
{
   if (glyphIndex >= TextureGlyphs.GetSize())
   {
       static const TextureGlyph dummyTextureGlyph;
       return dummyTextureGlyph;
   }
   return TextureGlyphs[glyphIndex];
}


// Register some texture info for the glyph at the specified
// index.  The TextureGlyph can be used later to render the glyph.
void    TextureGlyphData::AddTextureGlyph(unsigned glyphIndex, const TextureGlyph& glyph)
{
//    SF_ASSERT(!IsMissingGlyph(glyphIndex));

   // Resize texture glyphs so that the index will fit.
   if (TextureGlyphs.GetSize() <= glyphIndex)
   {
       TextureGlyphs.Resize(glyphIndex+1);
       if (TextureGlyphs.GetSize() <= glyphIndex)
           return;
   }
   
   //SF_ASSERT(glyph.HasImageResource());
   //SF_ASSERT(TextureGlyphs[glyphIndex].HasImageResource() == false);
   TextureGlyphs[glyphIndex] = glyph;
}


// Delete all our texture glyph info.
void    TextureGlyphData::WipeTextureGlyphs()
{
   //SF_ASSERT(TextureGlyphs.GetSize() == Glyphs.GetSize());

   // Replace with default (empty) glyph info.
   TextureGlyph defaultTg;
   for (UPInt i = 0, n = TextureGlyphs.GetSize(); i < n; i++)
       TextureGlyphs[i] = defaultTg;
}


void TextureGlyphData::VisitTextureGlyphs(TextureGlyphVisitor* pvisitor)
{
   SF_ASSERT(pvisitor);

   for(UPInt i = 0, n = TextureGlyphs.GetSize(); i < n; ++i)    
       pvisitor->Visit((unsigned)i, &TextureGlyphs[i]);    
}

float TextureGlyphData::GetTextureGlyphScale() const
{
   return FontPackParams::GlyphBoundBox / PackTextureConfig.NominalSize;
}

void TextureGlyphData::AddTexture(ResourceId textureId, ImageResource* pimageRes)
{
   ResourcePtr<ImageResource> pres = pimageRes;
   GlyphsTextures.Set(textureId, pres);
}
void TextureGlyphData::AddTexture(ResourceId textureId, const ResourceHandle &rh)
{
   ResourcePtr<ImageResource> pres;
   pres.SetFromHandle(rh);
   GlyphsTextures.Set(textureId, pres);
}


void TextureGlyphData::VisitTextures(TexturesVisitor* pvisitor, ResourceBinding* pbinding)
{
   SF_ASSERT(pvisitor);

   ImageResourceHash::ConstIterator iter = GlyphsTextures.Begin();
   for (; iter != GlyphsTextures.End(); ++iter)
   {
       Resource* pres = (*pbinding) [iter->Second];
       SF_ASSERT(pres->GetResourceType() == Resource::RT_Image);        
       pvisitor->Visit(iter->First, (ImageResource*) pres);
   }
}


// ***** FontData


// DBG
//======================== Remove
//static Ptr<RawImage> SharedImg;
//Render::TextureGlyph SharedGlyph;
//const Render::TextureGlyph* FontData::GetTextureGlyph(unsigned) const
//{
//    if (SharedImg.GetPtr() == 0)
//    {
//        enum { w=1024, h=128, x1=395, y1=0, x2=417, y2=30 };
//        float ox = 396.53125f;
//        float oy = 25.65625;
//
//
//        ImageSize size(w, h);
//        unsigned use = ImageUse_Update;
//        ImageData d;
//        SharedImg = *RawImage::Create(Image_A8, 1, size, use, Memory::GetGlobalHeap());
//        SharedImg->GetImageData(&d);
//        ImagePlane& pl = d.pPlanes[0];
//
//        unsigned i;
//        for (i = 0; i < size.Height; i += 2)
//        {
//            UByte* p1 = pl.GetScanline(i);
//            UByte* p2 = pl.GetScanline(i+1);
//            memset(p1, 0, size.Width);
//            memset(p2, 255, size.Width);
//        }
//        float u1 = float(x1) / w;
//        float v1 = float(y1) / h;
//        float u2 = float(x2) / w;
//        float v2 = float(y2) / h;
//        float uo = ox / w;
//        float vo = oy / h;
//        SharedGlyph.UvBounds = RectF(u1, v1, u2, v2);
//        SharedGlyph.UvOrigin = PointF(uo, vo);
//        SharedGlyph.pImage = SharedImg;
//    }
//    return &SharedGlyph;
//}
//
//float FontData::GetTextureGlyphHeight() const
//{
//    return 48.0 * 1024.0 / 1536.0;
//}
//======================== Remove


const Render::TextureGlyph* FontData::GetTextureGlyph(unsigned glyphIndex) const
{
    if (pTGData && glyphIndex < pTGData->GetTextureGlyphCount())
        return &pTGData->GetTextureGlyph(glyphIndex);
    return NULL;
}

float FontData::GetTextureGlyphHeight() const
{
    return (pTGData) ? float(pTGData->GetNominalSize() * 1024.0 / 1536.0) : 0;
}


FontData::FontData()   
   : Font(), Name(NULL)
{
}

FontData::FontData(const char* name, unsigned fontFlags)
    : Font(fontFlags), Name(NULL)
{
    Name = (char*)SF_ALLOC(SFstrlen(name) + 1, StatMD_Fonts_Mem);
    if (Name)
        SFstrcpy(Name, SFstrlen(name) + 1, name);

    SetHasLayout(1);
}


FontData::~FontData()
{
    Glyphs.Resize(0);

    // Delete the name string.
    if (Name)
    {
        SF_FREE(Name);
        Name = NULL;
    }
}


void    FontData::Read(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_ASSERT(tagInfo.TagType == Tag_DefineFont ||
            tagInfo.TagType == Tag_DefineFont2 ||
            tagInfo.TagType == Tag_DefineFont3);

    Stream *    pin = p->GetStream();
    bool        isLoadingFontShapes = p->IsLoadingFontShapes();
    MemoryHeap* pheap = p->GetLoadHeap();

    // No AddRef() here, to avoid cycle.
    // OwningMovie is our owner, so it has a ref to us.
    //OwningMovie = p->GetDataDef();    

    if (tagInfo.TagType == Tag_DefineFont)
    {
        pin->LogParse("reading DefineFont\n");

        int tableBase = pin->Tell();

        // Read the glyph offsets.  Offsets
        // are measured from the start of the
        // offset table.
        Array<int>    offsets;
        offsets.PushBack(pin->ReadU16());
        pin->LogParse("offset[0] = %d\n", offsets[0]);
        int count = offsets[0] >> 1;
        for (int i = 1; i < count; i++)
        {
            UInt16 off = pin->ReadU16();
            if (off == 0)
            {
                // special case for tag with stripped font shapes
                isLoadingFontShapes = false;
                break;
            }
            offsets.PushBack(off);
            pin->LogParse("offset[%d] = %d\n", i, offsets[i]);
        }

        Glyphs.Resize(count);
      //  TextureGlyphs.Resize(Glyphs.GetSize());

        if (isLoadingFontShapes)
        {
            // Read the glyph shapes.
            for (int i = 0; i < count; i++)
            {
                // Seek to the start of the shape data.
                int newPos = tableBase + offsets[i];
                pin->SetPosition(newPos);
                
                SPInt shapeDataLen;
                if (i + 1 < count)
                    shapeDataLen = offsets[i + 1] - offsets[i];
                else
                    shapeDataLen = tagInfo.TagDataOffset + tagInfo.TagLength - newPos;

                // Create & read the shape.
                Ptr<ConstShapeNoStyles> s = *SF_HEAP_NEW_ID(pheap, StatMD_ShapeData_Mem) ConstShapeNoStyles;
                s->Read(p, Tag_DefineShape, (unsigned)shapeDataLen, false);

                Glyphs[i] = s;
            }
        }
        else
            SetGlyphShapesStripped();
    }
    else if ((tagInfo.TagType == Tag_DefineFont2) || (tagInfo.TagType == Tag_DefineFont3))
    {
        if (tagInfo.TagType == Tag_DefineFont2)
            pin->LogParse("reading DefineFont2: ");
        else
            pin->LogParse("reading DefineFont3: ");

        bool    hasLayout = (pin->ReadUInt(1) != 0);
        SetHasLayout(hasLayout);

        bool shiftJisFlag        = (pin->ReadUInt(1) != 0);
        bool pixelAlignedChars   = (pin->ReadUInt(1) != 0);
        bool ansiFlag            = (pin->ReadUInt(1) != 0);

        if (shiftJisFlag)  SetCodePage(FF_CodePage_ShiftJis);
        else if (ansiFlag) SetCodePage(FF_CodePage_Ansi);
        else               SetCodePage(FF_CodePage_Unicode);
        
        SetPixelAligned(pixelAlignedChars);
        
        bool    wideOffsets = (pin->ReadUInt(1) != 0);
        SetWideCodes(pin->ReadUInt(1) != 0);
        SetItalic(pin->ReadUInt(1) != 0);
        SetBold(pin->ReadUInt(1) != 0);
        UByte   langCode = pin->ReadU8(); // Language code

        // Inhibit warning.
        SF_UNUSED(langCode);

        Name = pin->ReadStringWithLength(p->GetLoadHeap());

        int glyphCount = pin->ReadU16();

        if (pin->IsVerboseParse())
        {
            pin->LogParse("  Name = %s, %d glyphs\n", Name ? Name : "(none)", glyphCount);

            const char* pcodePage = "Unicode";
            if (GetCodePage() == FF_CodePage_ShiftJis)
                pcodePage = "ShiftJIS";
            else if (GetCodePage() == FF_CodePage_Ansi)
                pcodePage = "ANSI";

            pin->LogParse("  HasLayout = %d, CodePage = %s, Italic = %d, Bold = %d\n", 
                (int)hasLayout, pcodePage, IsItalic(), IsBold());
            pin->LogParse("  LangCode = %d\n", langCode);
        }

        
        int tableBase = pin->Tell();

        // Read the glyph offsets.  Offsets
        // are measured from the start of the
        // offset table.
        Array<int>    offsets;
        int fontCodeOffset;

        int offsetsCount = glyphCount;
        if (glyphCount > 0)
        {
            // check for the first offset. If it is 0 then shapes
            // were stripped from the tag.
            unsigned off0 = (wideOffsets) ? (unsigned)pin->ReadU32() : (unsigned)pin->ReadU16();
            if (off0 == 0)
            {
                // special case for tags with stripped font shapes
                isLoadingFontShapes = false;
                offsetsCount = 0;
            }
            else
            {
                offsets.PushBack(off0);
                //pin->LogParse("offset[0] = %d\n", offsets[0]);
            }
        }
        if (wideOffsets)
        {
            // 32-bit offsets.
            for (int i = 1; i < offsetsCount; i++)
            {
                offsets.PushBack(pin->ReadU32());
                //pin->LogParse("offset[%d] = %d\n", i, offsets[i]);
            }
            fontCodeOffset = pin->ReadU32();
        }
        else
        {
            // 16-bit offsets.
            for (int i = 1; i < offsetsCount; i++)
            {
                offsets.PushBack(pin->ReadU16());
                //pin->LogParse("offset[%d] = %d\n", i, offsets[i]);
            }
            fontCodeOffset = pin->ReadU16();
        }

        Glyphs.Resize(glyphCount);
    //    TextureGlyphs.Resize(Glyphs.GetSize());

        if (isLoadingFontShapes)
        {
            TagType fontGlyphShapeTag = (tagInfo.TagType == Tag_DefineFont2) ?
                                           Tag_DefineShape2 : tagInfo.TagType;

            if (hasLayout)
            {
                AdvanceTable.Resize(glyphCount);
            }

            // Read the glyph shapes.
            for (int i = 0; i < glyphCount; i++)
            {
                // Seek to the start of the shape data.
                int newPos = tableBase + offsets[i];
                // if we're seeking backwards, then that looks like a bug.
                SF_ASSERT(newPos >= pin->Tell());
                pin->SetPosition(newPos);

                SPInt shapeDataLen;
                if (i + 1 < glyphCount)
                    shapeDataLen = offsets[i + 1] - offsets[i];
                else
                    shapeDataLen = fontCodeOffset - offsets[i];
                SF_ASSERT(shapeDataLen >= 0);

                // Create & read the shape.
                Ptr<ConstShapeNoStyles> s = *SF_HEAP_NEW_ID(pheap, StatMD_Fonts_Mem) ConstShapeNoStyles;
                if (!s->Read(p, fontGlyphShapeTag, (unsigned)shapeDataLen, false))
                {
                    // error occurred; correct number of glyphs and break
                    glyphCount = i;
                    AdvanceTable.Resize(i);
                    Glyphs.Resize(i);
                    return;
                }

                Glyphs[i] = s;

                if (hasLayout)
                {
                    RectF bounds;
                    s->ComputeBound(&bounds);
                    AdvanceEntry& ge = AdvanceTable[i];
                    if (bounds.IsNormal())
                    {
                        float left = bounds.x1;
                        float top  = bounds.y1;
                        float w = bounds.Width();
                        float h = bounds.Height();
                        /*
                        SF_ASSERT(left >= -32768./20 && left < 32767./20 && 
                            top  >= -32768./20 && top < 32767./20 && 
                            w < 65535./20 && h < 65535./20);
                            */
                        ge.Left    = (SInt16)(left * 20.f);
                        ge.Top     = (SInt16)(top * 20.f);
                        ge.Width   = (UInt16)(w * 20.f);
                        ge.Height  = (UInt16)(h * 20.f);
                    }
                    else
                    {
                        ge.Left  = ge.Top = 0;
                        ge.Width = ge.Height = 0;
                    }
                }
                //@DBG
                //RectF bounds;
                //s->ComputeBound(&bounds);
                //if (bounds.IsNormal())
                //{
                //    pin->LogParse("  chidx = %d, bounds = (%f, %f, {%f, %f}\n", 
                //        (int)i, bounds.x1, bounds.y1, bounds.Width(), bounds.Height());
                //}
            }

            int currentPosition = pin->Tell();
            if (fontCodeOffset + tableBase != currentPosition)
            {
                // Bad offset!  Don't try to read any more.
                return;
            }
        }
        else
        {
            // Skip the shape data.
            int newPos = tableBase + fontCodeOffset;
            if (newPos >= pin->GetTagEndPosition())
            {
                // No layout data!
                return;
            }

            pin->SetPosition(newPos);
            SetGlyphShapesStripped();
        }

        ReadCodeTable(pin);

        // Read layout info for the glyphs.
        if (hasLayout)
        {
            // Multiplier used for scaling.
            // In SWF 8, font shape constant resolution (tag 75) was increased by 20.
            float sfactor = (tagInfo.TagType == Tag_DefineFont3)  ? 1.0f / 20.0f : 1.0f;

            Ascent = (float) pin->ReadS16() * sfactor;
            Descent = (float) pin->ReadS16() * sfactor;
            Leading = (float) pin->ReadS16() * sfactor;
            
            if (pin->IsVerboseParse())
            {
                pin->LogParse("  Ascent = %d, Descent = %d, Leading = %d\n", 
                    (int)Ascent, (int)Descent, (int)Leading);
            }

            // Advance table; i.E. how wide each character is.
            if (AdvanceTable.GetSize() != Glyphs.GetSize())
                AdvanceTable.Resize(Glyphs.GetSize());
            for (UPInt i = 0, n = AdvanceTable.GetSize(); i < n; i++)
            {
                AdvanceEntry& ge = AdvanceTable[i];
                ge.Advance = (float) pin->ReadU16() * sfactor;
            }

            // Bounds table.
            //BoundsTable.Resize(Glyphs.GetSize());    // kill
            RectF  dummyRect;
            {for (UPInt i = 0, n = Glyphs.GetSize(); i < n; i++)
            {
                //BoundsTable[i].Read(pin);  // kill
                pin->ReadRect(&dummyRect);                   
            }}

            // Kerning pairs.
            int kerningCount = pin->ReadU16();
            if (pin->IsVerboseParse())
            {
                pin->LogParse("  KerningCount = %d\n", (int)kerningCount);
            }
            for (int i = 0; i < kerningCount; i++)
            {
                int off = pin->Tell();
                if (off >= tagInfo.TagDataOffset + tagInfo.TagLength)
                {
                    pin->LogError("Corrupted file %s, kerning table of the font '%s' is longer than tagLength.",
                                  pin->GetFileName().ToCStr(), (Name) ? Name : "<noname>");
                    break;
                }

                UInt16  char0, char1;
                if (AreWideCodes())
                {
                    char0 = pin->ReadU16();
                    char1 = pin->ReadU16();
                }
                else
                {
                    char0 = pin->ReadU8();
                    char1 = pin->ReadU8();
                }
                float   adjustment = (float) pin->ReadS16() * sfactor;

                KerningPair k;
                k.Char0 = char0;
                k.Char1 = char1;

                if (pin->IsVerboseParse())
                    pin->LogParse("     Pair: %d - %d,\tadj = %d\n", (int)char0, (int)char1, (int)adjustment);

                // Remember this adjustment; we can look it up quickly
                // later using the character pair as the key.
                KerningPairs.Add(k, adjustment);
            }
        }
        if (!isLoadingFontShapes && AdvanceTable.GetSize() > 0)
        {
            // reset glyphRects if no FontShapes loaded
            for (UPInt i = 0, n = AdvanceTable.GetSize(); i < n; i++)
            {
                AdvanceEntry& ge = AdvanceTable[i];
                ge.Left = ge.Top = ge.Height = ge.Width = 0;
            }
        }
    }
}


// Read additional information about this FontData, from a
// DefineFontInfo tag.  The caller has already read the tag
// type and FontData id.
void    FontData::ReadFontInfo(Stream* in, TagType tagType)
{
    if (Name)
    {
        SF_FREE(Name);
        Name = NULL;
    }

    Name = in->ReadStringWithLength(in->GetHeap());

    UByte flags = in->ReadU8();

    UByte langCode = 0;
    if (tagType == Tag_DefineFontInfo2)
    {
        langCode = in->ReadU8(); // Language code
        SF_UNUSED(langCode); // suppress warning
    }

    bool pixelAlignedChars  =  ((flags & 0x20) != 0);
    bool shiftJisFlag        = ((flags & 0x10) != 0);
    bool ansiFlag            = ((flags & 0x08) != 0);

    if (shiftJisFlag)  SetCodePage(FF_CodePage_ShiftJis);
    else if (ansiFlag) SetCodePage(FF_CodePage_Ansi);
    else               SetCodePage(FF_CodePage_Unicode);
    SF_UNUSED(pixelAlignedChars);
    
    SetItalic       ((flags & 0x04) != 0);
    SetBold         ((flags & 0x02) != 0);
    SetWideCodes    ((flags & 0x01) != 0);

    if (in->IsVerboseParse())
    {
        if (tagType == Tag_DefineFontInfo)        
            in->LogParse("reading DefineFontInfo\n");
        else
            in->LogParse("reading DefineFontInfo2\n");
        in->LogParse("  Name = %s\n", Name ? Name : "(none)");

        const char* pcodePage = "Unicode";
        if (GetCodePage() == FF_CodePage_ShiftJis)
            pcodePage = "ShiftJIS";
        else if (GetCodePage() == FF_CodePage_Ansi)
            pcodePage = "ANSI";

        in->LogParse("  CodePage = %s, Italic = %d, Bold = %d\n", 
            pcodePage, IsItalic(), IsBold());
        if (tagType == Tag_DefineFontInfo2)
            in->LogParse("  LangCode = %d\n", langCode);
    }

    ReadCodeTable(in);
}


// Read the table that maps from glyph indices to character codes.
void    FontData::ReadCodeTable(Stream* in)
{
    in->LogParse("reading code table at offset %d\n", in->Tell());

    SF_ASSERT(CodeTable.IsEmpty());

    CodeTable.SetCapacity(Glyphs.GetSize());
    if (AreWideCodes())
    {
        // Code table is made of UInt16's.
        for (UPInt i = 0, n = Glyphs.GetSize(); i < n; i++) 
            CodeTable.Add(in->ReadU16(), (UInt16)i);
    }
    else
    {
        // Code table is made of bytes.
        for (UPInt i = 0, n = Glyphs.GetSize(); i < n; i++)
            CodeTable.Add(in->ReadU8(), (UInt16)i); 
    }
}

int FontData::GetGlyphIndex(UInt16 code)
{
    UInt16 glyphIndex;
    if (CodeTable.Get(code, &glyphIndex))
    {
        return glyphIndex;
    }
    return -1;
}

int     FontData::GetCharValue(unsigned glyphIndex) const
{
    for (CodeTableType::ConstIterator iter = CodeTable.Begin();
        iter != CodeTable.End(); ++iter)
    {
        if (iter->Second == glyphIndex)
            return iter->First;
    }
    return -1;
}

const Render::ShapeDataInterface* FontData::GetPermanentGlyphShape(unsigned glyphIndex) const
{
    if (glyphIndex < Glyphs.GetSize())
        return Glyphs[glyphIndex].GetPtr();
    else
        return NULL;
}

float   FontData::GetAdvance(unsigned glyphIndex) const
{
    // how could this be if GlyphIndex is unsigned?
    if (IsMissingGlyph(glyphIndex))
        return GetNominalGlyphWidth();

    if (AdvanceTable.GetSize() == 0)
    {
        // No layout info for this FontData!!!
        static bool Logged = false;
        if (Logged == false)
        {
            Logged = true;
            SF_DEBUG_ERROR1(1, "Empty advance table in font %s", GetName());
        }
        
        // MA: It is better to return default advance, make char substitution show up better
        return GetNominalGlyphWidth();        
    }

    if (glyphIndex < AdvanceTable.GetSize())
    {
        return AdvanceTable[glyphIndex].Advance;
    }
    else
    {
        SF_ASSERT(0);
    }
    return 0;
}

float   FontData::GetGlyphWidth(unsigned glyphIndex) const
{
    if (IsMissingGlyph(glyphIndex))
        return GetNominalGlyphWidth();
    
    if (glyphIndex < AdvanceTable.GetSize())
    {
        SF_ASSERT(!IsMissingGlyph(glyphIndex));
        float w = float(AdvanceTable[glyphIndex].Width)/20.f;
        if (w != 0)
            return w;
    }
    return GetAdvance(glyphIndex); 
}

float   FontData::GetGlyphHeight(unsigned glyphIndex) const
{
    // How could this be if GlyphIndex is unsigned?
    if (IsMissingGlyph(glyphIndex) || AdvanceTable.GetSize() == 0)    
        return GetNominalGlyphHeight();

    if (glyphIndex < AdvanceTable.GetSize())
    {
        SF_ASSERT(!IsMissingGlyph(glyphIndex));
        return float(AdvanceTable[glyphIndex].Height)/20.f;
    }
    else
    {
        // Bad glyph index.  Due to bad data file?
        // This is possible, if font doesn't have layout (for static text).
        // But AdvanceTable should be empty in this case.
        SF_ASSERT(AdvanceTable.GetSize() == 0);
        return 0;
    }
}

RectF& FontData::GetGlyphBounds(unsigned glyphIndex, RectF* prect) const
{
    if (IsMissingGlyph(glyphIndex))
    {
        prect->x1 = prect->y1 = 0;
        prect->SetWidth(GetGlyphWidth(glyphIndex));
        prect->SetHeight(GetGlyphHeight(glyphIndex));
    }
    else if (glyphIndex < AdvanceTable.GetSize())
    {
        SF_ASSERT(!IsMissingGlyph(glyphIndex));
        const AdvanceEntry& e = AdvanceTable[glyphIndex];
        float w = float(e.Width)/20.f;
        if (w == 0)
            w = e.Advance;
        float h = float(e.Height)/20.f;
        prect->x1 = float(e.Left)/20.f;
        prect->y1  = float(e.Top)/20.f;
        prect->SetWidth(w);
        prect->SetHeight(h);
    }
    else
    {
        // Bad glyph index.  Due to bad data file?
        // This is possible, if font doesn't have layout (for static text).
        // But AdvanceTable should be empty in this case.
        //SF_ASSERT(AdvanceTable.GetSize() == 0);
        SF_DEBUG_ERROR3((AdvanceTable.GetSize() != 0), 
            "Glyph index %d exceeds advance table (size = %d), font = %s", 
            glyphIndex, (int)AdvanceTable.GetSize(), GetName());

        prect->x1 = prect->y1 = 0;
        prect->SetWidth(0);
        prect->SetHeight(0);

        // so try to calculate the bounds
        if (glyphIndex < Glyphs.GetSize())
        {
            ShapeDataBase* s = Glyphs[glyphIndex];
            if (s)
            {
                RectF bounds;
                s->ComputeBound(&bounds);
                if (bounds.IsNormal())
                {
                    float left = bounds.x1;
                    float top  = bounds.y1;
                    float w = bounds.Width();
                    float h = bounds.Height();
                    prect->x1    = left;
                    prect->y1     = top;
                    prect->SetWidth(w);
                    prect->SetHeight(h);
                }
            }
        }
    }
    return *prect;
}

// Return the adjustment in advance between the given two
// characters.  Normally this will be 0; i.E. the 
float   FontData::GetKerningAdjustment(unsigned LastCode, unsigned code) const
{
    float   adjustment;
    KerningPair k;
    k.Char0 = (UInt16)LastCode;
    k.Char1 = (UInt16)code;
    if (KerningPairs.Get(k, &adjustment))
    {
        return adjustment;
    }
    return 0;
}

namespace {
    struct Range { UInt16 start,end; };
    struct RangeLess  { bool operator()(const Range& r1, const Range& r2) const { return r1.start < r2.start; } };
}

static String BuildStringFromRanges(const Array<Range>& ranges)
{
    String ret;
    char buff[512];
    int countPrinted = 0;
    bool lastPrinted = true;
    UInt16 start = 0;
    UPInt i = 0;
    for(; i < ranges.GetSize(); ++i)
    {
        if (i == 0)
        {
            start = ranges[i].start;
            lastPrinted = false;
        }
        else
        {
            if (ranges[i].start > ranges[i-1].end + 1)
            {
                if (start == ranges[i-1].end)
                    Format(StringDataPtr(buff, sizeof(buff)), "0x{0:x}", start);
                else
                    Format(StringDataPtr(buff, sizeof(buff)), "0x{0:x}-0x{1:x}", start, ranges[i-1].end);
                lastPrinted = true;
                if (countPrinted)
                    ret += ", ";
                ret += buff;
                countPrinted++;
                if (countPrinted > 4)
                    break;
                start = ranges[i].start;
                lastPrinted = false;
            }
        }
    }
    if (!lastPrinted)
    {
        Format(StringDataPtr(buff, sizeof(buff)), "0x{0:x}-0x{1:x}", start, ranges[ranges.GetSize()-1].end);
        if (countPrinted)
            ret += ", ";
        ret += buff;
    }
    if ( i < ranges.GetSize() )
        ret += " (truncated)";
    return ret;
}
String FontData::GetCharRanges() const
{
    Array<Range> ranges;
    CodeTableType::ConstIterator it = CodeTable.Begin();
    UInt16 rangeStart = 0, prevValue = 0;
    bool rangeStarted = false;
    while( it != CodeTable.End() )
    {
        if (!rangeStarted)
        {
            rangeStart = it->First;
            rangeStarted = true;
        }
        else
        {
            if (prevValue != it->First-1)
            {
                Range range;
                range.start = rangeStart;
                range.end = prevValue;
                ranges.PushBack(range);
                rangeStarted = false;
                continue;
            }
        }
        prevValue = it->First;
        ++it;
    }
    if (rangeStarted)
    {
        Range range;
        range.start = rangeStart;
        range.end = prevValue;
        ranges.PushBack(range);
    }
    Alg::QuickSort(ranges, RangeLess());
    return BuildStringFromRanges(ranges);
}

//
struct Normalizer
{
    enum { FontHeight = 1024  };

    Normalizer(unsigned nominalSize) : NominalSize(nominalSize) {}

    template<typename T>
    inline T norm(T v) const { return v * FontHeight / T(NominalSize); }
    template<typename T>
    inline T denorm(T v) const { return v * T(NominalSize) / FontHeight; }

    unsigned    NominalSize;
};

#if defined(GFX_ENABLE_COMPACTED_FONTS) || defined(GFX_ENABLE_FONT_COMPACTOR)


template<class FontType>
static bool GetGlyphShape(const FontType& font, unsigned glyphIndex, GlyphShape* shape)
{
    // typedef typename FontType::ContainerType ContainerType;
    typedef typename FontType::CompactedFontType CompactedFontType;

    if (glyphIndex >= font.GetCompactedFont().GetNumGlyphs())
        return NULL;

    typename CompactedFontType::GlyphPathIteratorType glyph(font.GetContainer());
    font.GetCompactedFont().GetGlyphShape(glyphIndex, &glyph);
    Normalizer n(font.GetCompactedFont().GetNominalSize());

    ShapePosInfo pos(0);

    shape->Clear();
    while(!glyph.IsFinished())
    {
        shape->StartPath(&pos, Shape_NewPath, 1, 0, 0, 
                         n.norm((float)glyph.GetMoveX()), (float)n.norm(glyph.GetMoveY()));

        int edge[5];
        bool empty = true;
        while(!glyph.IsPathFinished())
        {
            glyph.ReadEdge(edge);
            if (edge[0] == CompactedFontType::PathDataDecoderType::Edge_Line)
            {
                shape->LineTo(&pos, n.norm((float)edge[1]), n.norm((float)edge[2]));
                empty = false;
            }
            else
            {
                shape->QuadTo(&pos, n.norm((float)edge[1]), n.norm((float)edge[2]), 
                                    n.norm((float)edge[3]), n.norm((float)edge[4]));
                empty = false;
            }
        }

        if(!empty)
        {
            shape->ClosePath(&pos);
            shape->EndPath();
        }
        glyph.AdvancePath();
    }

    if (!shape->IsEmpty())
    {
        shape->EndShape();
        return true;
    }
    return false;
}


template <class FontType>
static inline RectF& GetGlyphBounds(const FontType& font, unsigned glyphIndex, RectF* prect)
{
    if (font.IsMissingGlyph(glyphIndex))
    {
        prect->x1 = prect->y1 = 0;
        prect->SetRect(font.GetNominalGlyphWidth(), font.GetNominalGlyphHeight());
    }
    else if (glyphIndex < font.GetCompactedFont().GetNumGlyphs())
    {
        font.GetCompactedFont().GetGlyphBounds(glyphIndex,prect);
    }
    else
    {
        prect->x1 = prect->y1 = 0;
        prect->SetRect(font.GetNominalGlyphWidth(), font.GetNominalGlyphHeight());
        //return *prect;
        //SF_ASSERT(0);
    }
    Normalizer n(font.GetCompactedFont().GetNominalSize());
    prect->x1   = n.norm(prect->x1);
    prect->y1    = n.norm(prect->y1);
    prect->x2  = n.norm(prect->x2);
    prect->y2 = n.norm(prect->y2);
    return *prect;
}

template<class FontType>
static String GetCharRanges(const FontType& font)
{
    Array<Range> ranges;
    UInt16 rangeStart = 0, prevValue = 0;
    bool rangeStarted = false;
    for(unsigned i = 0; i < font.GetGlyphShapeCount();)
    {
        if (!rangeStarted)
        {
            rangeStart = (UInt16)font.GetCompactedFont().GetGlyphCode(i);
            rangeStarted = true;
        }
        else
        {
            if (prevValue != (UInt16)font.GetCompactedFont().GetGlyphCode(i)-1)
            {
                Range range;
                range.start = rangeStart;
                range.end = prevValue;
                ranges.PushBack(range);
                rangeStarted = false;
                continue;
            }
        }
        prevValue = (UInt16)font.GetCompactedFont().GetGlyphCode(i);
        ++i;
    }
    if (rangeStarted)
    {
        Range range;
        range.start = rangeStart;
        range.end = prevValue;
        ranges.PushBack(range);
    }
    Alg::QuickSort(ranges, RangeLess());
    return BuildStringFromRanges(ranges);
}
#endif

#ifdef GFX_ENABLE_FONT_COMPACTOR
// ***** FontDataCompactedSwf

FontDataCompactedSwf::FontDataCompactedSwf() 
    : Container(),CompactedFontValue(Container),NumGlyphs(0)
{
}
FontDataCompactedSwf::~FontDataCompactedSwf()
{

}
// *** GFX/SWF Loading methods.

void FontDataCompactedSwf::Read(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_ASSERT(tagInfo.TagType == Tag_DefineFont2 ||
            tagInfo.TagType == Tag_DefineFont3);

    Stream * pin = p->GetStream();
    bool        isLoadingFontShapes = p->IsLoadingFontShapes();

    // No AddRef() here, to avoid cycle.
    // OwningMovie is our owner, so it has a ref to us.
    //OwningMovie = p->GetDataDef();    

    if ((tagInfo.TagType == Tag_DefineFont2) || (tagInfo.TagType == Tag_DefineFont3))
    {
        SF_ASSERT(p->GetLoadStates()->GetFontCompactorParams());
        unsigned nominalSize = p->GetLoadStates()->GetFontCompactorParams()->GetNominalSize();
        bool mergeContours = p->GetLoadStates()->GetFontCompactorParams()->NeedMergeContours();
        FontCompactor compactor(Container);

        if (tagInfo.TagType == Tag_DefineFont2)
            pin->LogParse("reading DefineFont2: ");
        else
            pin->LogParse("reading DefineFont3: ");

        bool    hasLayout = (pin->ReadUInt(1) != 0);
        SetHasLayout(hasLayout);

        bool shiftJisFlag        = (pin->ReadUInt(1) != 0);
        bool pixelAlignedChars   = (pin->ReadUInt(1) != 0);
        bool ansiFlag            = (pin->ReadUInt(1) != 0);

        if (shiftJisFlag)  SetCodePage(FF_CodePage_ShiftJis);
        else if (ansiFlag) SetCodePage(FF_CodePage_Ansi);
        else               SetCodePage(FF_CodePage_Unicode);

        SF_UNUSED(pixelAlignedChars);       

        bool    wideOffsets = (pin->ReadUInt(1) != 0);
        SetWideCodes(pin->ReadUInt(1) != 0);
        SetItalic(pin->ReadUInt(1) != 0);
        SetBold(pin->ReadUInt(1) != 0);
        UByte   langCode = pin->ReadU8(); // Language code

        // Inhibit warning.
        SF_UNUSED(langCode);

        String name;
        pin->ReadStringWithLength(&name);        

        NumGlyphs = pin->ReadU16();

        if (pin->IsVerboseParse())
        {
            pin->LogParse("  Name = %s, %d glyphs\n", name.ToCStr() ? name.ToCStr() : "(none)", NumGlyphs);

            const char* pcodePage = "Unicode";
            if (GetCodePage() == FF_CodePage_ShiftJis)
                pcodePage = "ShiftJIS";
            else if (GetCodePage() == FF_CodePage_Ansi)
                pcodePage = "ANSI";

            pin->LogParse("  HasLayout = %d, CodePage = %s, Italic = %d, Bold = %d\n", 
                (int)hasLayout, pcodePage, IsItalic(), IsBold());
            pin->LogParse("  LangCode = %d\n", langCode);
        }

        int tableBase = pin->Tell();

        // Read the glyph offsets.  Offsets
        // are measured from the start of the
        // offset table.
        Array<int>    offsets;
        int fontCodeOffset;

        int offsetsCount = NumGlyphs;
        if (NumGlyphs > 0)
        {
            // check for the first offset. If it is 0 then shapes
            // were stripped from the tag.
            unsigned off0 = (wideOffsets) ? (unsigned)pin->ReadU32() : (unsigned)pin->ReadU16();
            if (off0 == 0)
            {
                // special case for tags with stripped font shapes
                isLoadingFontShapes = false;
                offsetsCount = 0;
            }
            else
                offsets.PushBack(off0);
        }
        if (wideOffsets)
        {
            // 32-bit offsets.
            for (int i = 1; i < offsetsCount; i++)
            {
                offsets.PushBack(pin->ReadU32());
            }
            fontCodeOffset = pin->ReadU32();
        }
        else
        {
            // 16-bit offsets.
            for (int i = 1; i < offsetsCount; i++)
            {
                offsets.PushBack(pin->ReadU16());
            }
            fontCodeOffset = pin->ReadU16();
        }

        compactor.StartFont(name.ToCStr(),Flags,nominalSize,0,0,0);
        Normalizer n(nominalSize);
        
        //    TextureGlyphs.Resize(Glyphs.GetSize());
        //Glyphs.Resize(glyphCount);
        //pPath = new PathAllocator;
        if (isLoadingFontShapes)
        {
            TagType fontGlyphShapeTag = (tagInfo.TagType == Tag_DefineFont2) ?
                                            Tag_DefineShape2 : tagInfo.TagType;

            // Allocate PathAllocator so that it properly uses 'this' heap.
            PathAllocator* ppathAllocator = SF_HEAP_AUTO_NEW(this) PathAllocator;

            // Read the glyph shapes.
            for (unsigned i = 0; i < NumGlyphs; i++)
            {
                // Seek to the start of the shape data.
                int newPos = tableBase + offsets[i];
                // if we're seeking backwards, then that looks like a bug.
                SF_ASSERT(newPos >= pin->Tell());
                pin->SetPosition(newPos);

                SPInt shapeDataLen;
                if (i + 1 < NumGlyphs)
                    shapeDataLen = offsets[i + 1] - offsets[i];
                else
                    shapeDataLen = fontCodeOffset - offsets[i];
                SF_ASSERT(shapeDataLen >= 0);

                // Create & read the shape.
                compactor.StartGlyph();                
                ConstShapeNoStyles s;

                s.Read(p, fontGlyphShapeTag, (unsigned)shapeDataLen, false);

                //Ptr<ConstShapeNoStyles> s = *new ConstShapeNoStyles;
                //s->Read(p, fontGlyphShapeTag, (unsigned)shapeDataLen, false, pPath);
                //Glyphs[i] = s;

                ShapePosInfo pos(s.GetStartingPos());
                ShapePathType pathType;
                float coord[Edge_MaxCoord];
                unsigned styles[3];

                while((pathType = s.ReadPathInfo(&pos, coord, styles)) != Shape_EndShape)
                {
                    if (pathType == Shape_NewLayer)
                        continue;

                    compactor.MoveTo((SInt16)n.denorm(coord[0]),(SInt16)n.denorm(coord[1]));

                    PathEdgeType edgeType;
                    while((edgeType = s.ReadEdge(&pos, coord)) != Edge_EndPath)
                    {
                        if(edgeType == Edge_LineTo)
                        {
                            compactor.LineTo((SInt16)n.denorm(coord[0]),(SInt16)n.denorm(coord[1]));
                        }
                        else if(edgeType == Edge_QuadTo)
                        {
                            compactor.QuadTo((SInt16)n.denorm(coord[0]),(SInt16)n.denorm(coord[1]),
                                (SInt16)n.denorm(coord[2]),(SInt16)n.denorm(coord[3]));
                        }
                    }
                }

                compactor.EndGlyph(mergeContours);

                ppathAllocator->Clear();
            }

            delete ppathAllocator;

            int currentPosition = pin->Tell();
            if (fontCodeOffset + tableBase != currentPosition)
            {
                // Bad offset!  Don't try to read any more.
                return;
            }
        }
        else
        {
            // Skip the shape data.
            int newPos = tableBase + fontCodeOffset;
            if (newPos >= pin->GetTagEndPosition())
            {
                // No layout data!
                return;
            }

            pin->SetPosition(newPos);
            SetGlyphShapesStripped();
        }

        pin->LogParse("reading code table at offset %d\n", pin->Tell());

        if (AreWideCodes())
        {
            // Code table is made of UInt16's.
            for (UPInt i = 0, dn = NumGlyphs; i < dn; i++)
                compactor.AssignGlyphCode((unsigned)i, pin->ReadU16());
        }
        else
        {
            // Code table is made of bytes.
            for (UPInt i = 0, dn = NumGlyphs; i < dn; i++)
                compactor.AssignGlyphCode((unsigned)i, pin->ReadU8());
        }

        // Read layout info for the glyphs.
        if (hasLayout)
        {
            // Multiplier used for scaling.
            // In SWF 8, font shape constant resolution (tag 75) was increased by 20.
            float sfactor = (tagInfo.TagType == Tag_DefineFont3) ? 1.0f / 20.0f : 1.0f;

            Ascent = (float) pin->ReadS16() * sfactor;
            Descent = (float) pin->ReadS16() * sfactor;
            Leading = (float) pin->ReadS16() * sfactor;

            compactor.UpdateMetrics(n.denorm((int)Ascent),n.denorm((int)Descent),n.denorm((int)Leading));

            if (pin->IsVerboseParse())
            {
                pin->LogParse("  Ascent = %d, Descent = %d, Leading = %d\n", 
                    (int)Ascent, (int)Descent, (int)Leading);
            }

            for (UPInt i = 0, k = NumGlyphs; i < k; i++)
                compactor.AssignGlyphAdvance((unsigned)i,n.denorm((int)(pin->ReadU16() * sfactor)));

            // Bounds table.
            //BoundsTable.Resize(Glyphs.GetSize());    // kill
            RectF  dummyRect;
            {for (UPInt i = 0, k = NumGlyphs; i < k; i++)
            {
                //BoundsTable[i].Read(pin);  // kill
                pin->ReadRect(&dummyRect);                   
            }}

            // Kerning pairs.
            int kerningCount = pin->ReadU16();
            if (pin->IsVerboseParse())
            {
                pin->LogParse("  KerningCount = %d\n", (int)kerningCount);
            }
            {for (int i = 0; i < kerningCount; i++)
            {
                UInt16  char0, char1;
                if (AreWideCodes())
                {
                    char0 = pin->ReadU16();
                    char1 = pin->ReadU16();
                }
                else
                {
                    char0 = pin->ReadU8();
                    char1 = pin->ReadU8();
                }
                float   adjustment = (float) pin->ReadS16() * sfactor;

                if (pin->IsVerboseParse())
                    pin->LogParse("     Pair: %d - %d,\tadj = %d\n", (int)char0, (int)char1, (int)adjustment);

                compactor.AddKerningPair(char0, char1, n.denorm((int)adjustment));
            }}
        }
        compactor.EndFont();
        CompactedFontValue.AcquireFont(0);
    }
}

int FontDataCompactedSwf::GetGlyphIndex(UInt16 code)
{
    return CompactedFontValue.GetGlyphIndex(code);
}

int FontDataCompactedSwf::GetCharValue(unsigned glyphIndex) const
{
    if (glyphIndex < CompactedFontValue.GetNumGlyphs())
        return CompactedFontValue.GetGlyphCode(glyphIndex);    
    return -1;
}

bool FontDataCompactedSwf::GetTemporaryGlyphShape(unsigned glyphIndex, unsigned hintedSize, GlyphShape* shape)
{
    SF_UNUSED(hintedSize);
    return GetGlyphShape(*this, glyphIndex, shape);
}

float FontDataCompactedSwf::GetAdvance(unsigned glyphIndex) const
{
    if (IsMissingGlyph(glyphIndex) || glyphIndex >= CompactedFontValue.GetNumGlyphs())
        return GetNominalGlyphWidth();

    Normalizer n(CompactedFontValue.GetNominalSize());
    return n.norm(CompactedFontValue.GetAdvance(glyphIndex));
}
float FontDataCompactedSwf::GetKerningAdjustment(unsigned lastCode, unsigned thisCode) const
{
    Normalizer n(CompactedFontValue.GetNominalSize());
    return n.norm(CompactedFontValue.GetKerningAdjustment(lastCode,thisCode));
}

unsigned FontDataCompactedSwf::GetGlyphShapeCount() const
{
    return NumGlyphs;//CompactedFontValue.GetNumGlyphs();
}

float FontDataCompactedSwf::GetGlyphWidth(unsigned glyphIndex) const
{
    if (IsMissingGlyph(glyphIndex) || glyphIndex >= CompactedFontValue.GetNumGlyphs())
        return GetNominalGlyphWidth();
    Normalizer n(CompactedFontValue.GetNominalSize());
    return n.norm(CompactedFontValue.GetGlyphWidth(glyphIndex));
}

float FontDataCompactedSwf::GetGlyphHeight(unsigned glyphIndex) const
{
    if (IsMissingGlyph(glyphIndex) || glyphIndex >= CompactedFontValue.GetNumGlyphs())
        return GetNominalGlyphHeight();
    Normalizer n(CompactedFontValue.GetNominalSize());
    return n.norm(CompactedFontValue.GetGlyphHeight(glyphIndex));
}

RectF& FontDataCompactedSwf::GetGlyphBounds(unsigned glyphIndex, RectF* prect) const
{
    return GFx::GetGlyphBounds(*this, glyphIndex, prect);
}

// Slyph/State access.
const char* FontDataCompactedSwf::GetName() const
{
    return CompactedFontValue.GetName();
}

String FontDataCompactedSwf::GetCharRanges() const
{
    return GFx::GetCharRanges(*this);
}
bool FontDataCompactedSwf::HasVectorOrRasterGlyphs() const
{
    return NumGlyphs != 0; //CompactedFontValue.GetNumGlyphs() != 0;
}
#endif //#ifdef GFX_ENABLE_FONT_COMPACTOR


#ifdef GFX_ENABLE_COMPACTED_FONTS
// *** FontDataCompactedGfx implementation
//
FontDataCompactedGfx::FontDataCompactedGfx() 
: CompactedFontValue(Container)
{
}
FontDataCompactedGfx::~FontDataCompactedGfx()
{
}
// *** GFX/SWF Loading methods.
void FontDataCompactedGfx::Read(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_ASSERT(tagInfo.TagType == Tag_DefineCompactedFont);

    Stream * pin = p->GetStream();
    //bool        isLoadingFontShapes = p->IsLoadingFontShapes();
    //UInt16 fontId = pin->ReadU16();

    pin->LogParse("reading DefineCompactedFont:\n");

    int csize = tagInfo.TagLength - 2;
    ArrayUnsafePOD<UByte> buf;
    buf.Resize(4096);
    Container.Reserve(csize);
    int bytes = 0;
    do
    {
        int bytesToRead = Alg::Min(4096, csize - bytes);
        int bytesRead = pin->ReadToBuffer(&buf[0], bytesToRead);
        bytes += bytesRead;
        if (bytesRead > 0)
        {
            for(int i = 0; i < bytesRead; ++i)
                Container.PushBack(buf[i]);
        }
        if (bytesRead != bytesToRead)
        {
            //SF_DEBUG_ERROR1(1, "Empty advance table in font %s", GetName());
            pin->LogError("Could not read tag DefineCompactedFont. Broken gfx file.");
            break;
        }
    }
    while(bytes < csize);

    CompactedFontValue.AcquireFont(0);
    if (CompactedFontValue.GetNominalSize() <= 0)
    {
        pin->LogError("Invalid nominal size for DefineCompactedFont, font %s. Broken gfx file.", GetName());
        SetFontMetrics(0, 960, 1024-960);
    }
    else
    {
        const float scale = 1024.f/CompactedFontValue.GetNominalSize();
        SetFontMetrics(CompactedFontValue.GetLeading()*scale, CompactedFontValue.GetAscent()*scale, CompactedFontValue.GetDescent()*scale);
        pin->LogParse(   "read font \"%s\"\n", GetName());
        Flags = CompactedFontValue.GetFontFlags();
    }
}

int FontDataCompactedGfx::GetGlyphIndex(UInt16 code)
{
    return CompactedFontValue.GetGlyphIndex(code);
}

int     FontDataCompactedGfx::GetCharValue(unsigned glyphIndex) const
{
    if (glyphIndex < CompactedFontValue.GetNumGlyphs())
        return CompactedFontValue.GetGlyphCode(glyphIndex);    
    return -1;
}

bool FontDataCompactedGfx::GetTemporaryGlyphShape(unsigned glyphIndex, unsigned hintedSize, GlyphShape* shape)
{
    SF_UNUSED(hintedSize);
    return GetGlyphShape(*this, glyphIndex, shape);
}

float FontDataCompactedGfx::GetAdvance(unsigned glyphIndex) const
{
    if (IsMissingGlyph(glyphIndex))
        return GetNominalGlyphWidth();
    Normalizer n(CompactedFontValue.GetNominalSize());
    return n.norm(CompactedFontValue.GetAdvance(glyphIndex));
}

float FontDataCompactedGfx::GetKerningAdjustment(unsigned lastCode, unsigned thisCode) const
{
    Normalizer n(CompactedFontValue.GetNominalSize());
    return n.norm(CompactedFontValue.GetKerningAdjustment(lastCode,thisCode));
}

unsigned FontDataCompactedGfx::GetGlyphShapeCount() const
{
    return CompactedFontValue.GetNumGlyphs();
}

float FontDataCompactedGfx::GetGlyphWidth(unsigned glyphIndex) const
{
    if (IsMissingGlyph(glyphIndex))
        return GetNominalGlyphWidth();
    Normalizer n(CompactedFontValue.GetNominalSize());
    return n.norm(CompactedFontValue.GetGlyphWidth(glyphIndex));
}
float FontDataCompactedGfx::GetGlyphHeight(unsigned glyphIndex) const
{
    if (IsMissingGlyph(glyphIndex))
        return GetNominalGlyphHeight();
    Normalizer n(CompactedFontValue.GetNominalSize());
    return n.norm(CompactedFontValue.GetGlyphHeight(glyphIndex));
}
RectF& FontDataCompactedGfx::GetGlyphBounds(unsigned glyphIndex, RectF* prect) const
{
    return GFx::GetGlyphBounds(*this, glyphIndex, prect);
}

// Glyph/State access.
const char* FontDataCompactedGfx::GetName() const
{
    return CompactedFontValue.GetName();
}

String FontDataCompactedGfx::GetCharRanges() const
{
    return GFx::GetCharRanges(*this);
}
bool FontDataCompactedGfx::HasVectorOrRasterGlyphs() const
{
    return CompactedFontValue.GetNumGlyphs() != 0;
}
#endif //#ifdef GFX_ENABLE_COMPACTED_FONTS

// ***** FontDataBound
struct TextureGlyphBinder : public TextureGlyphData::TextureGlyphVisitor
{
    ResourceBinding* ResBinding;

    TextureGlyphBinder(ResourceBinding* resBnd) : ResBinding(resBnd) {}
    virtual void Visit(unsigned /*index*/, TextureGlyph* ptextureGlyph)
    {
        if (!ptextureGlyph->pImage && ptextureGlyph->BindIndex != ~0u)
        {
            ResourceBindData rdata;
            ResBinding->GetResourceData(&rdata, ptextureGlyph->BindIndex);
            if (rdata.pResource.GetPtr() && rdata.pResource->GetResourceType() == Resource::RT_Image)
            {
                ImageResource* imgRes       = static_cast<ImageResource*>(rdata.pResource.GetPtr());
                ptextureGlyph->pImage = static_cast<Image*>(imgRes->GetImage());
                ptextureGlyph->BindIndex = ~0u;
            }
            else // - type of resource is wrong, warning?
            {
                //SF_ASSERT(0); //?
            }
        }
    }
};

FontDataBound::FontDataBound(Font* pfont, ResourceBinding* pbinding) 
    : Font(*pfont), pFont(pfont)
{
    // make a copy of TextureGlyphData and resolve image resources in it.
    SF_ASSERT(pfont->HasTextureGlyphs() && pbinding);

    const TextureGlyphData* srcTGData = static_cast<const TextureGlyphData*>(pfont->GetTextureGlyphData());
    pTGData = *SF_HEAP_AUTO_NEW(this) TextureGlyphData(*srcTGData);

    TextureGlyphBinder binder(pbinding);
    pTGData->VisitTextureGlyphs(&binder);
}

const TextureGlyph* FontDataBound::GetTextureGlyph(unsigned glyphIndex) const
{
    if (pTGData && glyphIndex < pTGData->GetTextureGlyphCount())
        return &pTGData->GetTextureGlyph(glyphIndex);
    return NULL;
}

// ***** FontResource

FontResource::FontResource(Font *pfont, ResourceBinding* pbinding)
    : pBinding(pbinding)
{    
    pFont           = pfont;
    LowerCaseTop    = 0;
    UpperCaseTop    = 0;
}

FontResource::FontResource(Font *pfont, const ResourceKey& key)
    : pBinding(0)
{    
    pFont           = pfont;
    FontKey         = key;
    LowerCaseTop    = 0;
    UpperCaseTop    = 0;
}


UInt16 FontResource::calcTopBound(UInt16 code)
{
    RectF bounds;

    int idx = pFont->GetGlyphIndex(code);
    if (idx != -1)
    {
        /*
        Ptr<ShapeDataBase> shape = *GetGlyphShape(idx, 0);
        if (shape)
        {
            shape->ComputeBound(&bounds);
            return UInt16(-bounds.y1);
        }
        */
        pFont->GetGlyphBounds(idx,&bounds);
        return UInt16(-bounds.y1);
    }
    return 0;
}


void FontResource::calcLowerUpperTop(Log* log)
{
    SInt16 lowerCaseTop = 0;
    SInt16 upperCaseTop = 0;
    if (pFont && LowerCaseTop == 0 && UpperCaseTop == 0)
    {
        const UByte upperCaseCandidates[] = "HEFTUVWXZ";
        const UByte lowerCaseCandidates[] = "zxvwy";
        
        const UByte* p;
        for (p = upperCaseCandidates; *p; ++p)
        {
            upperCaseTop = calcTopBound(*p);
            if (upperCaseTop)
                break;
        }

        if (upperCaseTop)
        {
            for (p = lowerCaseCandidates; *p; ++p)
            {
                lowerCaseTop = calcTopBound(*p);
                if (lowerCaseTop)
                    break;
            }
        }
    }
    if (lowerCaseTop && upperCaseTop)
    {
        LowerCaseTop = lowerCaseTop;
        UpperCaseTop = upperCaseTop;
    }
    else
    {
        if (log)
            log->LogWarning("Font '%s%s%s' has no hinting chars "
                            "(any of 'HEFTUVWXZ' and 'zxvwy'). Auto-Hinting disabled.", 
                            GetName(), 
                            pFont->IsBold() ? " Bold" : "", 
                            pFont->IsItalic() ? " Italic" : "");
        LowerCaseTop = -1;
        UpperCaseTop = -1;
    }
}


FontResource::~FontResource()
{
//     if (HandlerArrayFlag)
//     {
//         for (unsigned i=0; i<pHandlerArray->GetSize(); i++)
//             (*pHandlerArray)[i]->OnDispose(this);
//     }
//     else if (pHandler)
//     {
//         pHandler->OnDispose(this);
//     }
}

// *** System FontProvider resource key

// We store system fonts obtained from FontProvider is ResourceLib so
// that they are re-used it requested several times.

class GFxSystemFontResourceKey : public RefCountBase<GFxSystemFontResourceKey, StatMD_Fonts_Mem>
{
    // Image States.  
    Ptr<FontProvider>   pFontProvider;
    String               FontName;
    unsigned                CreateFontFlags;
public:

    GFxSystemFontResourceKey(const char* pname, unsigned fontFlags,
                             FontProvider* pfontProvider)
    {        
        FontName        = String(pname).ToLower(); // Font names are case-insensitive.   
        CreateFontFlags = fontFlags & FontData::FF_Style_Mask;
        pFontProvider   = pfontProvider;
    }

    bool operator == (GFxSystemFontResourceKey& other) const
    {
        return (FontName == other.FontName &&
                pFontProvider == other.pFontProvider &&
                CreateFontFlags == other.CreateFontFlags);
    }

    UPInt  GetHashCode() const
    {
        UPInt hashCode = String::BernsteinHashFunctionCIS(
                            FontName.ToCStr(), FontName.GetSize());
        return CreateFontFlags ^ hashCode ^            
            ((UPInt)pFontProvider.GetPtr()) ^ (((UPInt)pFontProvider.GetPtr()) >> 7);
    }
};

class GFxSystemFontResourceKeyInterface : public ResourceKey::KeyInterface
{
public:
    typedef ResourceKey::KeyHandle KeyHandle;    

    // ResourceKey::KeyInterface implementation.    
    virtual void                AddRef(KeyHandle hdata)     { ((GFxSystemFontResourceKey*) hdata)->AddRef(); }
    virtual void                Release(KeyHandle hdata)    { ((GFxSystemFontResourceKey*) hdata)->Release(); }
    
    virtual ResourceKey::KeyType GetKeyType(KeyHandle hdata) const
    {
        SF_UNUSED(hdata);
        return ResourceKey::Key_None;
    }
    virtual UPInt               GetHashCode(KeyHandle hdata) const
    {
        return ((GFxSystemFontResourceKey*) hdata)->GetHashCode();
    }

    virtual bool                KeyEquals(KeyHandle hdata, const ResourceKey& other);
};

static GFxSystemFontResourceKeyInterface GFxSystemFontResourceKeyInterface_Instance;

bool    GFxSystemFontResourceKeyInterface::KeyEquals(KeyHandle hdata, const ResourceKey& other)
{
    if (this != other.GetKeyInterface())
        return 0;

    GFxSystemFontResourceKey* pthisData = (GFxSystemFontResourceKey*) hdata;
    GFxSystemFontResourceKey* potherData = (GFxSystemFontResourceKey*) other.GetKeyData();
    SF_ASSERT(pthisData && potherData);    
    return (*pthisData == *potherData);
}


ResourceKey FontResource::CreateFontResourceKey(const char* pname, unsigned fontFlags,
                                                      FontProvider* pfontProvider)
{
    Ptr<GFxSystemFontResourceKey> pdata =
        *new GFxSystemFontResourceKey(pname, fontFlags, pfontProvider);

    return ResourceKey(&GFxSystemFontResourceKeyInterface_Instance,
                          (ResourceKey::KeyHandle)pdata.GetPtr() );
}



// Static helper function used to lookup and/or create a font resource from provider.
FontResource* FontResource::CreateFontResource(const char* pname, unsigned fontFlags,
                                                     FontProvider* pprovider,
                                                     ResourceWeakLib *plib)
{
    SF_ASSERT(pname && pprovider && plib);

    // Create a search key.
    ResourceKey              fontKey = CreateFontResourceKey(pname, fontFlags, pprovider);
    ResourceLib::BindHandle  bh;
    FontResource*            pfontRes = 0;

    if (plib->BindResourceKey(&bh, fontKey) == ResourceLib::RS_NeedsResolve)
    {
        // If hot found, create the font object.
        Ptr<Render::Font> pexternalFont = *pprovider->CreateFont(pname, fontFlags);
        if (pexternalFont)        
            pfontRes = SF_NEW FontResource(pexternalFont, fontKey);
        
        if (pfontRes)
        {
            bh.ResolveResource(pfontRes);
        }
        else
        {   // No error messages are logged for failed system font creation.
            bh.CancelResolve("");
        }
    }
    else
    {
        // If Available, wait for resource to become available.
        pfontRes = (FontResource*)bh.WaitForResolve();
        SF_ASSERT(pfontRes->GetResourceType() == RT_Font);        
    }

    return pfontRes;
}

// Resolves TextureGlyphData (images), if any. Called from BindNextFrame.
void FontResource::ResolveTextureGlyphs()
{
    if (pFont->HasTextureGlyphs())
        pFont       = *SF_HEAP_AUTO_NEW(this) FontDataBound(pFont, pBinding);
}

}} // Scaleform::GFx
