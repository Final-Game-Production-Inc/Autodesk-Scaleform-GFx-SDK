/**************************************************************************

PublicHeader:   Render
Filename    :   Render_GlyphParam.h
Content     :   
Created     :   
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_GlyphParam_H
#define INC_SF_Render_GlyphParam_H

#include <math.h>
#include "Kernel/SF_Types.h"
#include "Kernel/SF_RefCount.h"
#include "Render/Render_Image.h"

namespace Scaleform { namespace Render {

class FontCacheHandle;


enum TextLayerType
{
    // Layers in the order they are drawn.
    // These layer types should only be used when unmasked.
    TextLayer_Background,
    TextLayer_Selection,
    TextLayer_Shadow,
    TextLayer_ShadowText,
    TextLayer_RasterText,
    TextLayer_PackedText,
    TextLayer_PackedDFAText,
    TextLayer_Images,
    TextLayer_Shapes,
    TextLayer_Underline,
    TextLayer_Cursor,
    
    // If mask is used, these layer types should be marked as masked.
    // Masked layers are given different types to ensure that they are
    // drawn after the mask.
    TextLayer_Mask, // Mask rectangle shape.
    TextLayer_Shapes_Masked,
    TextLayer_Underline_Masked
};


enum TextUnderlineStyle
{
    TextUnderline_Single,
    TextUnderline_Thick,
    TextUnderline_Dotted,
    TextUnderline_DottedThick,
    TextUnderline_Dithered,
    TextUnderline_DitheredThick
};

//------------------------------------------------------------------------
struct TextureGlyph : public RefCountBase<TextureGlyph, Stat_Default_Mem>
{
    Ptr<Image>  pImage;
    RectF       UvBounds;
    PointF      UvOrigin;   // the origin of the glyph box, in uv coords
    unsigned    BindIndex;  // if not ~0u and pImage is NULL it is an index of resource to be resolved

    TextureGlyph() : BindIndex(~0u) {}
};

//------------------------------------------------------------------------
struct GlyphParam
{
    FontCacheHandle*    pFont;
    UInt16              GlyphIndex;
    UInt16              FontSize;     // Fix point 12.4, 255 is the max size for raster glyphs
    UInt16              Flags;        // See enum FlagsType
    UInt16              BlurX;        // Fix point 12.4, unsigned
    UInt16              BlurY;        // Fix point 12.4, unsigned
    UInt16              BlurStrength; // Fix point 12.4, unsigned

    enum FlagsType
    {
        OptRead    = 0x0001,
        AutoFit    = 0x0002,
        Stretch    = 0x0004,
        FauxBold   = 0x0008,
        FauxItalic = 0x0010,
        KnockOut   = 0x0020,
        HideObject = 0x0040,
        FineBlur   = 0x0080,
        BitmapFont = 0x0100,
        UseRaster  = 0x0200,
        ForceVector= 0x0400,
        OutlineMask= 0xF000
    };

    void Clear()
    {
        pFont=0; GlyphIndex=0; FontSize=0; Flags=0;
        BlurX=0; BlurY=0; BlurStrength=16;
    }

    float    GetFontSize()     const { return float(FontSize) / 16.0f; }
    bool     IsOptRead()       const { return (Flags & OptRead) != 0; }
    bool     IsAutoFit()       const { return (Flags & AutoFit) != 0; }
    bool     IsBitmapFont()    const { return (Flags & BitmapFont) != 0; }
    float    GetStretch()      const { return (Flags & Stretch) ? 2.5f : 1.0f; }
    bool     GetUseRaster()    const { return (Flags & UseRaster) != 0; }
    bool     IsFauxBold()      const { return (Flags & FauxBold) != 0; }
    bool     IsFauxItalic()    const { return (Flags & FauxItalic) != 0; }
    bool     IsForceVector()   const { return (Flags & ForceVector) != 0; }
    bool     IsKnockOut()      const { return (Flags & KnockOut) != 0; }
    bool     IsHiddenObject()  const { return (Flags & HideObject) != 0; }
    bool     IsFineBlur()      const { return (Flags & FineBlur) != 0; }
    float    GetBlurX()        const { return float(BlurX) / 16.0f; }
    float    GetBlurY()        const { return float(BlurY) / 16.0f; }
    float    GetBlurStrength() const { return float(BlurStrength) / 16.0f; }
    unsigned GetOutline()      const { return Flags >> 12; }

    void SetFontSize(float s)     { FontSize = UInt16(floor(s * 16.0)); }
    void SetOptRead(bool f)       { if(f) Flags |= OptRead;    else Flags &= ~OptRead; }
    void SetAutoFit(bool f)       { if(f) Flags |= AutoFit;    else Flags &= ~AutoFit; }
    void SetStretch(bool f)       { if(f) Flags |= Stretch;    else Flags &= ~Stretch; }
    void SetUseRaster(bool f)     { if(f) Flags |= UseRaster;  else Flags &= ~UseRaster; }
    void SetFauxBold(bool f)      { if(f) Flags |= FauxBold;   else Flags &= ~FauxBold; }
    void SetFauxItalic(bool f)    { if(f) Flags |= FauxItalic; else Flags &= ~FauxItalic; }
    void SetForceVector(bool f)   { if(f) Flags |= ForceVector;else Flags &= ~ForceVector; }
    void SetKnockOut(bool f)      { if(f) Flags |= KnockOut;   else Flags &= ~KnockOut; }
    void SetHideObject(bool f)    { if(f) Flags |= HideObject; else Flags &= ~HideObject; }
    void SetFineBlur(bool f)      { if(f) Flags |= FineBlur;   else Flags &= ~FineBlur; }
    void SetBlurX(float v)        { BlurX = UInt16(v * 16.0f + 0.5f); }
    void SetBlurY(float v)        { BlurY = UInt16(v * 16.0f + 0.5f); }
    void SetBlurStrength(float v) { BlurStrength = UInt16(v * 16.0f + 0.5f); }
    void SetBitmapFont(bool f)   
    { 
        if(f) 
        {
            Flags |= BitmapFont; 
            SetOptRead(true);
            SetAutoFit(false);
            SetStretch(false);
        }
        else 
        {
            Flags &= ~BitmapFont;
        }
    }
    void SetOutline(unsigned outline)
    {
        Flags &= ~OutlineMask;
        Flags |= (outline & 0xF) << 12;
    }

    //bool operator != (const GlyphParam& key) const
    //{
    //    return !(*this == key);
    //}
};




//------------------------------------------------------------------------
struct TextFieldParam
{
    enum { ShadowDisabled = 0x01 };

    GlyphParam  TextParam;     // pFont, GlyphIndex, and FontSize are not used here
    GlyphParam  ShadowParam;
    UInt32      ShadowColor;
    float       ShadowOffsetX;
    float       ShadowOffsetY;

    void Clear()
    {
        TextParam.Clear();
        ShadowParam.Clear();
        ShadowColor = 0; ShadowOffsetX = 0; ShadowOffsetY = 0;
    }

    TextFieldParam() { Clear(); }

    // TO DO: Restore
    //bool operator == (const TextFieldParam& key) const 
    //{ 
    //    return TextParam     == key.TextParam &&
    //           ShadowParam   == key.ShadowParam &&
    //           ShadowColor   == key.ShadowColor &&
    //           ShadowOffsetX == key.ShadowOffsetX &&
    //           ShadowOffsetY == key.ShadowOffsetY;
    //}

    //bool operator != (const TextFieldParam& key) const
    //{
    //    return !(*this == key);
    //}

    //void LoadFromTextFilter(const TextFilter& filter)
    //{
    //    TextParam.BlurX        = filter.BlurX;
    //    TextParam.BlurY        = filter.BlurY;
    //    TextParam.Flags        = GlyphParam::FineBlur;
    //    TextParam.BlurStrength = filter.BlurStrength;
    //  //TextParam.Outline      = ...;

    //    if ((filter.ShadowFlags & ShadowDisabled) == 0)
    //    {
    //        ShadowParam.Flags        = UInt8(filter.ShadowFlags & ~ShadowDisabled);
    //        ShadowParam.BlurX        = filter.ShadowBlurX;
    //        ShadowParam.BlurY        = filter.ShadowBlurY;
    //        ShadowParam.BlurStrength = filter.ShadowStrength;
    //      //ShadowParam.Outline      = ...;
    //        ShadowColor              = filter.ShadowColor;
    //        ShadowOffsetX            = filter.ShadowOffsetX;
    //        ShadowOffsetY            = filter.ShadowOffsetY;

    //    }
    //}

};


}} // Scaleform::Render

#endif
