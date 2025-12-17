/**************************************************************************

Filename    :   GFx_MorphCharacter.h
Content     :   Morphable shape character definition
Created     :   
Authors     :   MS,MA,AB

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_MORPHCHARACTER_H
#define INC_SF_GFX_MORPHCHARACTER_H

#include "GFxConfig.h"
#ifdef SF_RENDER_ENABLE_MORPHING
#include "GFx/GFx_ShapeSwf.h"

namespace Scaleform { namespace GFx {

class LoadProcess;
struct TagInfo;

// ***** Morph Character Def class

class MorphCharacterDef : public ShapeBaseCharacterDef
{
public:
    MorphCharacterDef();
    virtual ~MorphCharacterDef();

    void            Read(LoadProcess* p, const TagInfo& tagInfo, bool withStyle);

    // These methods are implemented only in shapes with styles, i.e.
    // it is not needed for glyph shapes.
    virtual RectF   GetBoundsLocal(float morphRatio = 0) const;
    virtual RectF   GetRectBoundsLocal(float morphRatio = 0) const 
    { 
        return GetBoundsLocal(morphRatio);
    } //???

    virtual bool    DefPointTestLocal(const Render::PointF &pt, bool testShape = 0, 
                                      const DisplayObjectBase *pinst = 0) const;

    virtual UInt32  ComputeGeometryHash() const { return 0; } //?
    virtual bool    IsEqualGeometry(const ShapeBaseCharacterDef& cmpWith) const;
    virtual ShapeDataInterface* GetShape() { SF_ASSERT(0); return pShape1; } //?

    virtual Ptr<Render::ShapeMeshProvider> BindResourcesInStyles(const GFx::ResourceBinding& resourceBinding) const;

    virtual bool    NeedsResolving() const { return (pShape1->GetFlags() & ShapeDataBase::Flags_NeedsResolving) != 0 ||
                                                    (pShape2->GetFlags() & ShapeDataBase::Flags_NeedsResolving) != 0; }
private:
    Ptr<ConstShapeWithStyles>   pShape1;
    Ptr<ConstShapeWithStyles>   pShape2;

    void            ReadMorphFillStyle(LoadProcess* p, TagType tagType, FillStyleType& fs1, FillStyleType& fs2, 
                                       bool* needResolve);
};

}} // namespace Scaleform::GFx
#endif //#ifdef SF_RENDER_ENABLE_MORPHING

#endif // INC_SF_GFX_MORPH2_H
