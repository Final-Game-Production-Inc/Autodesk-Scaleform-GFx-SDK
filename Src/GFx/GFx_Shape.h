/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_Shape.h
Content     :   SWF (Shockwave Flash) player library
Created     :   Sep 1, 2010
Authors     :   Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_SHAPE_H
#define INC_SF_GFX_SHAPE_H

#include "GFx/GFx_StreamContext.h"
#include "GFx/GFx_CharacterDef.h"
#include "Render/Render_TreeNode.h"
#include "Render/Render_ShapeDataDefs.h"
#include "Render/Render_TreeShape.h"
#include "Render/Render_ShapeMeshProvider.h"

namespace Scaleform { namespace GFx {

class ResourceBinding;

using namespace Render;

// A path allocator class. Used to allocate memory for shapes to avoid
// fragmentation.
//
class PathAllocator : public NewOverrideBase<StatMD_CharDefs_Mem>
{
private:
    struct Page
    {
        Page*       pNext;
        UInt32      PageSize;
        //UByte Buffer[];

        UByte*          GetBufferPtr()                      { return ((UByte*)(&PageSize + 1)); }
        UByte*          GetBufferPtr(unsigned freeBytes)    { return ((UByte*)(&PageSize + 1)) + PageSize - freeBytes; }
        const UByte*    GetBufferPtr() const                { return ((UByte*)(&PageSize + 1)); }
        const UByte*    GetBufferPtr(unsigned freeBytes) const  { return ((UByte*)(&PageSize + 1)) + PageSize - freeBytes; }
    };
    Page        *pFirstPage, *pLastPage;
    UInt16      FreeBytes;
    UInt16      DefaultPageSize;

public:
    enum { Default_PageBufferSize = 8192 };

    PathAllocator(unsigned pageSize = Default_PageBufferSize);
    ~PathAllocator();

    void   Clear();

    UByte* AllocPath(unsigned edgesDataSize, unsigned pathSize, unsigned edgeSize);
    UByte* AllocRawPath(UInt32 sizeInBytes);
    UByte* AllocMemoryBlock(UInt32 sizeForCurrentPage, UInt32 sizeInNewPage);
    bool   ReallocLastBlock(UByte* ptr, UInt32 oldSize, UInt32 newSize);
    void   SetDefaultPageSize(unsigned dps);
    UInt16 GetPageOffset(const UByte* ptr) const
    {
        SF_ASSERT(ptr - (const UByte*)pLastPage >= 0 && ptr - (const UByte*)pLastPage < 65536);
        return (UInt16)(ptr - (const UByte*)pLastPage);
    }

    static UInt16 GetPageOffset(const void* ppage, const UByte* ptr)
    {
        const Page* p = (const Page*)ppage;
        SF_ASSERT(ptr - (const UByte*)p >= 0 && ptr - (const UByte*)p < 65536);
        return (UInt16)(ptr - (const UByte*)p);
    }
    static const void* GetPagePtr(const UByte* pinitialPtr, UInt16 offtopage) { return pinitialPtr - offtopage; }
    static bool  IsInPage(const void* ppage, const UByte* ptr) 
    { 
        SF_ASSERT(ppage && ptr);
        const Page* p = (const Page*)ppage;
        return (ptr - (const UByte*)(&p->PageSize + 1)) < (int)p->PageSize;
    }
    static const void* GetNextPage(const void* ppage, const UByte** pdata)
    {
        SF_ASSERT(ppage && pdata);
        const Page* p = (const Page*)ppage;
        p = p->pNext;
        if (p)
            *pdata = p->GetBufferPtr();
        else
            *pdata = NULL;
        return p;
    }
};


//////////////////////////////// CharacterDef implementations ///////////////////////////////////
// A base abstract class for shape character definition. 
// Implemented by ShapeCharacterDef and ConstShapeCharacterDef

class ShapeBaseCharacterDef : public GFx::CharacterDef
{
public:
    ShapeBaseCharacterDef() {}
    virtual ~ShapeBaseCharacterDef() {}

    virtual CharacterDefType GetType() const { return Shape; }

    virtual RectF       GetBoundsLocal(float morphRatio = 0) const = 0;

    // These methods are implemented only in shapes with styles, i.e.
    // it is not needed for glyph shapes.
    virtual RectF       GetRectBoundsLocal(float morphRatio = 0) const = 0;
    //virtual void        SetRectBoundsLocal(const RectF&) = 0;

    // calculate exact bounds, since Bounds may contain null or inexact info
    //virtual void        ComputeBound(RectF* r) const = 0;

    virtual UInt32      ComputeGeometryHash() const = 0;
    virtual bool        IsEqualGeometry(const ShapeBaseCharacterDef& cmpWith) const = 0;

    // Query Resource type code, which is a combination of ResourceType and ResourceUse.
    virtual unsigned    GetResourceTypeCode() const     { return MakeTypeCode(RT_ShapeDef); }

    virtual ShapeDataInterface* GetShape() =0;

    virtual Ptr<Render::ShapeMeshProvider> BindResourcesInStyles(const GFx::ResourceBinding&) const
    {   
        return pShapeMeshProvider; 
    }

    Ptr<Render::TreeNode> CreateTreeShape(Render::Context& context, 
                                          MovieDefImpl* defImpl) const;
    virtual bool        NeedsResolving() const { return false; }
protected:
    Ptr<Render::ShapeMeshProvider>  pShapeMeshProvider;
};

}} // namespace Scaleform::GFx

#endif // INC_SF_GFX_SHAPE_H

