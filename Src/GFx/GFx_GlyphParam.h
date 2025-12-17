/**************************************************************************

Filename    :   GFx_GlyphParam.h
Content     :   
Created     :   
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_GlyphParam_H
#define INC_SF_GFX_GlyphParam_H

#include <math.h>
#include "Kernel/SF_Types.h"
#include "GFx/GFx_FilterDesc.h"

namespace Scaleform { namespace GFx {

class FontResource;

//------------------------------------------------------------------------
struct GlyphParam
{
    FontResource*   pFont;
    UInt16          GlyphIndex;

    UInt8           FontSize;     // 255 is the max size for raster glyphs
    UInt8           Flags;        // See enum FlagsType
    UInt8           BlurX;        // fixed point 4.4, unsigned
    UInt8           BlurY;        // fixed point 4.4, unsigned
    UInt8           BlurStrength; // fixed point 4.4, unsigned
    UInt8           FlagsEx;      // Extended Flags

    enum FlagsType
    {
        OptRead    = 0x01,
        AutoFit    = 0x02,
        Stretch    = 0x04,
        FauxBold   = 0x08,
        FauxItalic = 0x10,
        KnockOut   = 0x20,
        HideObject = 0x40,
        FineBlur   = 0x80,
    };

    enum FlagsExType
    {
        BitmapFont = 0x01
    };

    static float GetMaxBlur()    { return  15.75f; }
    static float GetMaxOutline() { return  15.75f; }
    static float GetMinOutline() { return -15.75f; }

    void Clear()
    {
        pFont=0; GlyphIndex=0; FontSize=0; Flags=0;
        BlurX=0; BlurY=0; BlurStrength=16; FlagsEx=0;
    }

    unsigned  GetFontSize()     const { return FontSize; }
    bool  IsOptRead()       const { return (Flags & OptRead) != 0; }
    bool  IsAutoFit()       const { return (Flags & AutoFit) != 0; }
    bool  IsBitmapFont()    const { return (FlagsEx & BitmapFont) != 0; }
    unsigned  GetStretch()      const { return (Flags & Stretch) ? 3 : 1; }
    bool  IsFauxBold()      const { return (Flags & FauxBold) != 0; }
    bool  IsFauxItalic()    const { return (Flags & FauxItalic) != 0; }
    bool  IsKnockOut()      const { return (Flags & KnockOut) != 0; }
    bool  IsHiddenObject()  const { return (Flags & HideObject) != 0; }
    bool  IsFineBlur()      const { return (Flags & FineBlur) != 0; }
    float GetBlurX()        const { return float(BlurX) / 16.0f; }
    float GetBlurY()        const { return float(BlurY) / 16.0f; }
    float GetBlurStrength() const { return float(BlurStrength) / 16.0f; }

    void SetFontSize(unsigned s)      { FontSize = UInt8(s); }
    void SetOptRead(bool f)       { if(f) Flags |= OptRead;    else Flags &= ~OptRead; }
    void SetAutoFit(bool f)       { if(f) Flags |= AutoFit;    else Flags &= ~AutoFit; }
    void SetStretch(bool f)       { if(f) Flags |= Stretch;    else Flags &= ~Stretch; }
    void SetFauxBold(bool f)      { if(f) Flags |= FauxBold;   else Flags &= ~FauxBold; }
    void SetFauxItalic(bool f)    { if(f) Flags |= FauxItalic; else Flags &= ~FauxItalic; }
    void SetKnockOut(bool f)      { if(f) Flags |= KnockOut;   else Flags &= ~KnockOut; }
    void SetHideObject(bool f)    { if(f) Flags |= HideObject; else Flags &= ~HideObject; }
    void SetFineBlur(bool f)      { if(f) Flags |= FineBlur;   else Flags &= ~FineBlur; }
    void SetBlurX(float v)        { BlurX = UInt8(v * 16.0f + 0.5f); }
    void SetBlurY(float v)        { BlurY = UInt8(v * 16.0f + 0.5f); }
    void SetBlurStrength(float v) { BlurStrength = UInt8(v * 16.0f + 0.5f); }
    void SetBitmapFont(bool f)   
    { 
        if(f) 
        {
            FlagsEx |= BitmapFont; 
            SetOptRead(true);
            SetAutoFit(false);
            SetStretch(false);
        }
        else 
        {
            FlagsEx &= ~BitmapFont;
        }
    }

    UPInt operator()(const GlyphParam& key) const
    {
        return (((UPInt)key.pFont) >> 6) ^ (UPInt)key.pFont ^ 
                 (UPInt)key.GlyphIndex ^
                 (UPInt)key.FontSize ^
                 (UPInt)key.Flags ^
                 (UPInt)key.BlurX ^     
                ((UPInt)key.BlurY << 1) ^
                 (UPInt)key.BlurStrength ^
                 (UPInt)key.FlagsEx;
    }

    bool operator == (const GlyphParam& key) const 
    { 
        return  pFont        == key.pFont && 
                GlyphIndex   == key.GlyphIndex &&
                FontSize     == key.FontSize &&
                Flags        == key.Flags &&
                BlurX        == key.BlurX &&     
                BlurY        == key.BlurY &&
                BlurStrength == key.BlurStrength &&
                FlagsEx      == key.FlagsEx;
    }
    
    bool operator != (const GlyphParam& key) const
    {
        return !(*this == key);
    }

};




//------------------------------------------------------------------------
struct TextFieldParam
{
    enum { ShadowDisabled = 0x01 };

    GlyphParam  TextParam;
    GlyphParam  ShadowParam;
    Color       ShadowColor;
    SInt16      ShadowOffsetX; // In Twips
    SInt16      ShadowOffsetY; // In Twips

    void Clear()
    {
        TextParam.Clear();
        ShadowParam.Clear();
        ShadowColor = 0; ShadowOffsetX = 0; ShadowOffsetY = 0;
    }

    TextFieldParam() { Clear(); }

    bool operator == (const TextFieldParam& key) const 
    { 
        return TextParam     == key.TextParam &&
               ShadowParam   == key.ShadowParam &&
               ShadowColor   == key.ShadowColor &&
               ShadowOffsetX == key.ShadowOffsetX &&
               ShadowOffsetY == key.ShadowOffsetY;
    }

    bool operator != (const TextFieldParam& key) const
    {
        return !(*this == key);
    }

    void LoadFromTextFilter(const TextFilter& filter)
    {
        TextParam.BlurX        = filter.BlurX;
        TextParam.BlurY        = filter.BlurY;
        TextParam.Flags        = GlyphParam::FineBlur;
        TextParam.BlurStrength = filter.BlurStrength;
      //TextParam.Outline      = ...;

        if ((filter.ShadowFlags & ShadowDisabled) == 0)
        {
            ShadowParam.Flags        = UInt8(filter.ShadowFlags & ~ShadowDisabled);
            ShadowParam.BlurX        = filter.ShadowBlurX;
            ShadowParam.BlurY        = filter.ShadowBlurY;
            ShadowParam.BlurStrength = filter.ShadowStrength;
          //ShadowParam.Outline      = ...;
            ShadowColor              = filter.ShadowColor;
            ShadowOffsetX            = filter.ShadowOffsetX;
            ShadowOffsetY            = filter.ShadowOffsetY;

        }
    }

};

}} // namespace Scaleform::GFx

#endif
