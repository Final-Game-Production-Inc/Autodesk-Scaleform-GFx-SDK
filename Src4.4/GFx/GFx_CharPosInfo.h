/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_CharPosInfo.h
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX__CHARPOSINFO_H
#define INC_SF_GFX__CHARPOSINFO_H

//#include "Render/Render_Renderer.h"
#include "GFx/GFx_FilterDesc.h"
#include "GFx/GFx_Resource.h"

namespace Scaleform { namespace GFx {

struct CharPosInfoFlags
{
    enum
    {
        Flags_HasDepth          = 0x001,
        Flags_HasCharacterId    = 0x002,
        Flags_HasMatrix         = 0x004,
        Flags_HasCxform         = 0x008,
        Flags_HasRatio          = 0x010,
        Flags_HasFilters        = 0x020,
        Flags_HasClipDepth      = 0x040,
        Flags_HasBlendMode      = 0x080,
        Flags_HasClassName      = 0x100,
        Flags_HasVisibility     = 0x200,
    };

    UInt16               Flags;
    
    CharPosInfoFlags() : Flags(0) {}
    CharPosInfoFlags(UInt16 flags) : Flags(flags) {}

    SF_INLINE void SetMatrixFlag()        { Flags |= Flags_HasMatrix; }
    SF_INLINE void SetCxFormFlag()        { Flags |= Flags_HasCxform; }
    SF_INLINE void SetBlendModeFlag()     { Flags |= Flags_HasBlendMode; }
    SF_INLINE void SetFiltersFlag()       { Flags |= Flags_HasFilters; }
    SF_INLINE void SetDepthFlag()         { Flags |= Flags_HasDepth; }
    SF_INLINE void SetClipDepthFlag()     { Flags |= Flags_HasClipDepth; }
    SF_INLINE void SetRatioFlag()         { Flags |= Flags_HasRatio; }
    SF_INLINE void SetCharacterIdFlag()   { Flags |= Flags_HasCharacterId; }
    SF_INLINE void SetClassNameFlag()     { Flags |= Flags_HasClassName; }
    SF_INLINE void SetVisibilityFlag()    { Flags |= Flags_HasVisibility; }

    SF_INLINE bool HasMatrix() const      { return (Flags & Flags_HasMatrix) != 0; }
    SF_INLINE bool HasCxform() const      { return (Flags & Flags_HasCxform) != 0; }
    SF_INLINE bool HasBlendMode() const   { return (Flags & Flags_HasBlendMode) != 0; }
    SF_INLINE bool HasFilters() const     { return (Flags & Flags_HasFilters) != 0; }
    SF_INLINE bool HasDepth() const       { return (Flags & Flags_HasDepth) != 0; }
    SF_INLINE bool HasClipDepth() const   { return (Flags & Flags_HasClipDepth) != 0; }
    SF_INLINE bool HasRatio() const       { return (Flags & Flags_HasRatio) != 0; }
    SF_INLINE bool HasCharacterId() const { return (Flags & Flags_HasCharacterId) != 0; }
    SF_INLINE bool HasClassName() const   { return (Flags & Flags_HasClassName) != 0; }
    SF_INLINE bool HasVisibility() const  { return (Flags & Flags_HasVisibility) != 0; }
};


// A data structure that describes common positioning state.
class CharPosInfo 
{
public:    

    // -- CXFORM HEADER -----------------
    //     HasAddTerms      UB[1]                               Has color addition values if equal to 1
    //     HasMultTerms     UB[1]                               Has color multiply values if equal to 1
    //     Nbits            UB[4]                               Bits in each value field
    //     RedMultTerm      If HasMultTerms = 1, SB[Nbits]      Red multiply value
    //     GreenMultTerm    If HasMultTerms = 1, SB[Nbits]      Green multiply value
    //     BlueMultTerm     If HasMultTerms = 1, SB[Nbits]      Blue multiply value
    //     RedAddTerm       If HasAddTerms = 1, SB[Nbits]       Red addition value
    //     GreenAddTerm     If HasAddTerms = 1, SB[Nbits]       Green addition value
    //     BlueAddTerm      If HasAddTerms = 1, SB[Nbits]       Blue addition value
    Cxform              ColorTransform;

    // -- MATRIX HEADER -----------------
    //     HasScale         UB[1]                               Has scale values if equal to 1
    //     NScaleBits       If HasScale = 1, UB[5]              Bits in each scale value field
    //     ScaleX           If HasScale = 1, FB[NScaleBits]     x scale value
    //     ScaleY           If HasScale = 1, FB[NScaleBits]     y scale value
    //     HasRotate        UB[1]                               Has rotate and skew values if equal to 1
    //     NRotateBits      If HasRotate = 1, UB[5]             Bits in each rotate value field
    //     RotateSkew0      If HasRotate = 1,FB[NRotateBits]    First rotate and skew value
    //     RotateSkew1      If HasRotate = 1,FB[NRotateBits]    Second rotate and skew value
    //     NTranslateBits   UB[5]                               Bits in each translate value field
    //     TranslateX       SB[NTranslateBits]                  x translate value in twips
    //     TranslateY       SB[NTranslateBits]                  y translate value in twips
    Matrix2F            Matrix_1;

    Ptr<Render::FilterSet> pFilters;

    float               Ratio;              // [PPS] Should be U16
    int                 Depth;              // [PPS] Should be U16
    ResourceId          CharacterId;        // [PPS] Should be U16
    const char*         ClassName;          // For PlaceObject3/AVM2 usage
    UInt16              ClipDepth;
    CharPosInfoFlags    Flags;
    UInt8               BlendMode;          // [PPS] Only requires 4bits for storage
    UInt8               Visible;            // Only requires 1bit for storage.
  
    CharPosInfo()
    {
        Ratio       = 0.0f;
        Depth       = 0;        
        ClipDepth   = 0;
        BlendMode   = Render::Blend_None;
        ClassName   = NULL;
        Visible     = 1;
    }

    CharPosInfo(ResourceId chId, int depth,
        bool hasCxform, const Cxform &cxform,
        bool hasMatrix, const Matrix2F &matrix,
        const char* className = NULL,
        float ratio = 0.0f, UInt16 clipDepth = 0,
        bool hasBlendMode = false, Render::BlendMode blend = Render::Blend_None)
        : ColorTransform(cxform), Matrix_1(matrix), CharacterId(chId)
    {
        Ratio       = ratio;
        if (hasMatrix)
            Flags.SetMatrixFlag();
        if (hasCxform)
            Flags.SetCxFormFlag();
        if (hasBlendMode)
            Flags.SetBlendModeFlag();
        Depth       = depth;
        CharacterId = chId;
        ClipDepth   = clipDepth;
        BlendMode   = (UInt8)blend;        
        ClassName   = className;
        Visible     = 1;
    }

    SF_INLINE void SetMatrixFlag()        { Flags.SetMatrixFlag(); }
    SF_INLINE void SetCxFormFlag()        { Flags.SetCxFormFlag(); }
    SF_INLINE void SetBlendModeFlag()     { Flags.SetBlendModeFlag(); }
    SF_INLINE void SetFiltersFlag()       { Flags.SetFiltersFlag(); }
    SF_INLINE void SetDepthFlag()         { Flags.SetDepthFlag(); }
    SF_INLINE void SetClipDepthFlag()     { Flags.SetClipDepthFlag(); }
    SF_INLINE void SetRatioFlag()         { Flags.SetRatioFlag(); }
    SF_INLINE void SetCharacterIdFlag()   { Flags.SetCharacterIdFlag(); }
    SF_INLINE void SetClassNameFlag()     { Flags.SetClassNameFlag(); }
    SF_INLINE void SetVisibilityFlag()    { Flags.SetVisibilityFlag(); }

    SF_INLINE bool HasMatrix() const      { return Flags.HasMatrix(); }
    SF_INLINE bool HasCxform() const      { return Flags.HasCxform(); }
    SF_INLINE bool HasBlendMode() const   { return Flags.HasBlendMode(); }
    SF_INLINE bool HasFilters() const     { return Flags.HasFilters(); }
    SF_INLINE bool HasDepth() const       { return Flags.HasDepth(); }
    SF_INLINE bool HasClipDepth() const   { return Flags.HasClipDepth(); }
    SF_INLINE bool HasRatio() const       { return Flags.HasRatio(); }
    SF_INLINE bool HasCharacterId() const { return Flags.HasCharacterId(); }
    SF_INLINE bool HasClassName() const   { return Flags.HasClassName(); }
    SF_INLINE bool HasVisibility() const  { return Flags.HasVisibility(); }
};

}} // namespace Scaleform::GFx

#endif //INC_CHARPOSINFO_H
