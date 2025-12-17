/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_ShapeSwf.h
Content     :   SWF (Shockwave Flash) shape logic
Created     :   Sep 1, 2010
Authors     :   Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_SF_GFX_SHAPESWF_H
#define INC_SF_GFX_SHAPESWF_H

#include "GFx/GFx_StreamContext.h"
#include "GFx/GFx_CharacterDef.h"
#include "GFx/GFx_Shape.h"
#include "Render/Render_ShapeDataDefs.h"
#include "GFx_Tags.h"
#include "Render/Render_Context.h"
#include "Render/Render_TreeShape.h"

namespace Scaleform { 
namespace GFx {

class LoadProcess;
class ImageResource;
class ImageCreator;
class ResourceBinding;

using namespace Render;

typedef Array<FillStyleType, StatMD_ShapeData_Mem> FillStyleArrayTemp;
typedef Array<StrokeStyleType, StatMD_ShapeData_Mem> StrokeStyleArrayTemp;

enum  FillType
{
    Fill_Solid              = 0x00,
    Fill_LinearGradient     = 0x10,
    Fill_RadialGradient     = 0x12,
    Fill_FocalPointGradient = 0x13,
    Fill_TiledSmoothImage   = 0x40,
    Fill_ClippedSmoothImage = 0x41,
    Fill_TiledImage         = 0x42,
    Fill_ClippedImage       = 0x43,

    // Test bits for efficient type discrimination.
    Fill_Gradient_TestBit   = 0x10,
    Fill_Image_TestBit      = 0x40,
};

enum
{
    Default_MiterSize = 3
};

unsigned ConvertSwfLineStyles(unsigned swfLineStyle);

// A base class for shape with and without styles
class ShapeDataBase : public ShapeDataInterface
{
    friend class ShapeSwfReader;
public:
    enum
    {
        Flags_TexturedFill       = 1,
        Flags_Sfactor20          = 2,
        Flags_NeedsResolving     = 4,
        Flags_NonZeroFill        = 8,
        Flags_ValidBounds        = 0x10,

        Flags_StylesSupport      = 0x40,
        Flags_S9GSupport         = 0x80
    };


    ShapeDataBase() : Paths(0), Flags(0) {}
    ShapeDataBase(const ShapeDataBase& o) : ShapeDataInterface(), 
        Paths(o.Paths), Flags(o.Flags) {}

    enum EmptyShape { Empty_Shape };
    explicit ShapeDataBase(EmptyShape);

    // These methods are implemented only in shapes with styles, i.e.
    // it is not needed for glyph shapes.
    virtual RectF   GetBoundsLocal() const          { return RectF(0,0,0,0); }
    virtual void    SetBoundsLocal(const RectF&)    { }
    virtual RectF   GetRectBoundsLocal() const { return RectF(0,0,0,0); }
    virtual void    SetRectBoundsLocal(const RectF&) {}

    void    ComputeBound(RectF* r) const;
    bool    DefPointTestLocal(Render::ShapeMeshProvider* pshapeMeshProvider, const Render::PointF &pt, 
        bool testShape = 0, const DisplayObjectBase *pinst = 0) const;

    UInt32  ComputeGeometryHash() const;
    bool    IsEqualGeometry(const ShapeBaseCharacterDef& cmpWith) const;

    const UByte* GetNativePathData() const { return Paths; }

    // ShapeDataInterface methods:
    // The implementation must provide some abstract index (position)
    // to quickly navigate to the beginning of the shape data. In simple case,
    // when the data is stored is an isolated array it can just return 0.
    virtual unsigned GetStartingPos() const { return 0; }

    // Read path info. Returns Shape_NewPath, Shape_NewLayer, or Shape_EndShape and 
    // starting path info. Shape_NewLayer is the same as Shape_NewPath, but just
    // indicates that it's a new layer.
    // Coord must be at least float[2], styles must be unsigned[3].
    virtual ShapePathType ReadPathInfo(ShapePosInfo* pos, float* coord, unsigned* styles) const;

    // Read next edge. Returns Edge_LineTo, Edge_QuadTo, or Edge_EndPath. 
    // Coord must be float[Seg_MaxCoord].
    virtual PathEdgeType ReadEdge(ShapePosInfo* pos, float* coord) const;

    // I/O
    virtual bool Read(LoadProcess* p, TagType tagType, unsigned lenInBytes, bool withStyle) =0;

    // creates a copy of the shape. The copy shares geometry data with original. Styles
    // will be copied (see ConstShapeWithStyles).
    virtual ShapeDataBase* Clone() const =0;

    // Iterates through complex fills and replace binding indices by pointers to real
    // images. 
    virtual void    BindResourcesInStyles(const GFx::ResourceBinding&) {}

    void            SetValidBoundsFlag(bool flag)   { (flag) ? Flags |= Flags_ValidBounds : Flags &= ~Flags_ValidBounds; }
    bool            HasValidBounds() const          { return (Flags & Flags_ValidBounds) != 0; }

    UByte           GetFlags() const { return Flags; }
    void            SetNeedsResolving() { Flags |= Flags_NeedsResolving; }
protected:
    void            Read(LoadProcess* p, TagType tagType, unsigned lenInBytes, 
                         bool withStyle, FillStyleArrayTemp*, StrokeStyleArrayTemp*,
                         PathAllocator* pAllocator = NULL);

private:
    const UByte*    Paths;
    UByte           Flags; // See enum Flags_...
};

// An implementation of constant shape character definition. 
// Constant shape is coming from swf files.
// ConstShapeNoStyles doesn't contain any styles (used for font glyphs).
class ConstShapeNoStyles : public ShapeDataBase
{
public:
    ConstShapeNoStyles() {}
    explicit ConstShapeNoStyles(EmptyShape e):ShapeDataBase(e) {}

    virtual unsigned GetFillStyleCount()   const { return 0; }
    virtual unsigned GetStrokeStyleCount() const { return 0; }

    // idx is in range [1...GetFillStyleCount()] (inclusive)
    virtual void GetFillStyle(unsigned idx, FillStyleType* p) const 
    {
        SF_ASSERT(0);
        SF_UNUSED2(idx, p);
    }

    // idx is range [1...GetStrokeStyleCount()] (inclusive)
    virtual void GetStrokeStyle(unsigned idx, StrokeStyleType* p) const
    {
        SF_ASSERT(0);
        SF_UNUSED2(idx, p);
    }

    // I/O
    virtual bool Read(LoadProcess* p, TagType tagType, unsigned lenInBytes, bool withStyle);

    virtual ShapeDataBase* Clone() const;
};

// ConstShapeWithStyles contains styles.
class ConstShapeWithStyles : public ShapeDataBase
{
public:
    ConstShapeWithStyles() : Styles(NULL), FillStylesNum(0), StrokeStylesNum(0) {}
    explicit ConstShapeWithStyles(EmptyShape e):ShapeDataBase(e),
        Styles(NULL), FillStylesNum(0), StrokeStylesNum(0) {}
    ConstShapeWithStyles(const ConstShapeWithStyles&);
    virtual ~ConstShapeWithStyles();

    virtual RectF  GetBoundsLocal() const          { return Bound; }
    virtual void   SetBoundsLocal(const RectF& r)  { Bound = r; }

    virtual RectF  GetRectBoundsLocal() const          { return RectBound; }
    virtual void   SetRectBoundsLocal(const RectF& r)  { RectBound = r; }

    virtual unsigned GetFillStyleCount()   const { return FillStylesNum; }
    virtual unsigned GetStrokeStyleCount() const { return StrokeStylesNum; }

    // idx is in range [1...GetFillStyleCount()] (inclusive)
    virtual void    GetFillStyle(unsigned idx, FillStyleType* p) const;

    // idx is range [1...GetStrokeStyleCount()] (inclusive)
    virtual void    GetStrokeStyle(unsigned idx, StrokeStyleType* p) const;

    // I/O
    virtual bool    Read(LoadProcess* p, TagType tagType, unsigned lenInBytes, bool withStyle);

    void            SetStyles(unsigned fillStyleCount, const FillStyleType* fillStyles, 
                              unsigned strokeStyleCount, const StrokeStyleType* strokeStyles);

    virtual ShapeDataBase* Clone() const;
    virtual void    BindResourcesInStyles(const GFx::ResourceBinding& resourceBinding);
private:
    // a combined array of Fill and Stroke styles. The layout is as follows:
    // FillStylesNum * sizeof(FillStyleType), StrokeStylesNum * sizeof(StrokeStyleType)
    UByte*                  Styles;
    unsigned                FillStylesNum; 
    unsigned                StrokeStylesNum;
    RectF                   Bound;
    RectF                   RectBound; // Smaller bounds without stroke, SWF 8
};

class SwfShapeCharacterDef : public ShapeBaseCharacterDef
{
protected:
    SwfShapeCharacterDef() {}
public:
    SwfShapeCharacterDef(ShapeDataBase* shp);

    virtual RectF   GetBoundsLocal(float morphRatio = 0) const;

    // These methods are implemented only in shapes with styles, i.e.
    // it is not needed for glyph shapes.
    virtual RectF   GetRectBoundsLocal(float morphRatio = 0) const;
    virtual void    SetRectBoundsLocal(const RectF& r) { pShape->SetRectBoundsLocal(r); }

    //virtual void    ComputeBound(RectF* r) const { pShape->ComputeBound(r); }
    virtual bool    DefPointTestLocal(const Render::PointF &pt, bool testShape = 0, 
        const DisplayObjectBase *pinst = 0) const { return pShape->DefPointTestLocal(pShapeMeshProvider, pt, testShape, pinst); }

    virtual UInt32  ComputeGeometryHash() const { return pShape->ComputeGeometryHash(); }
    virtual bool    IsEqualGeometry(const ShapeBaseCharacterDef& cmpWith) const 
    { 
        return pShape->IsEqualGeometry(cmpWith); 
    }
    virtual ShapeDataInterface* GetShape() { return pShape; }
    virtual Ptr<Render::ShapeMeshProvider> BindResourcesInStyles(const GFx::ResourceBinding& resourceBinding) const;

    virtual bool    NeedsResolving() const { return (pShape->GetFlags() & ShapeDataBase::Flags_NeedsResolving) != 0; }
protected:
    Ptr<ShapeDataBase>              pShape;
};

// a shapedef used for image character
class ImageShapeCharacterDef : public ShapeBaseCharacterDef
{
public:
    ImageShapeCharacterDef(ImageResource* pimage, ImageCreator* imgCreator, bool bilinear);

    virtual RectF   GetBoundsLocal(float =0) const
    {
        SF_ASSERT(pShapeMeshProvider);
        return pShapeMeshProvider->GetIdentityBounds();
    }

    // These methods are implemented only in shapes with styles, i.e.
    // it is not needed for glyph shapes.
    virtual RectF   GetRectBoundsLocal(float =0) const { return GetBoundsLocal() ;  } //???
    //virtual void    SetRectBoundsLocal(const RectF& r) { pShape->SetRectBoundsLocal(r); }

    //virtual void    ComputeBound(RectF* r) const { pShape->ComputeBound(r); }
    virtual bool    DefPointTestLocal(const Render::PointF &pt, bool testShape = 0, 
        const DisplayObjectBase *pinst = 0) const;

    virtual UInt32  ComputeGeometryHash() const {  SF_ASSERT(0); return 0; }
    virtual bool    IsEqualGeometry(const ShapeBaseCharacterDef& ) const 
    { 
        SF_ASSERT(0);
        return false;
        //return pShape->IsEqualGeometry(cmpWith); 
    }
    virtual ShapeDataInterface* GetShape() { return pShape; }

protected:
    Ptr<ShapeDataInterface>         pShape;
};

}} // SF::GFx

#endif
