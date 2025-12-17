/**************************************************************************

PublicHeader:   Render
Filename    :   Render_PrimitiveBundle.h
Content     :   PrimitiveBundle classes, SortKey and sorting support
Created     :
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_PrimitiveBundle_H
#define INC_SF_Render_PrimitiveBundle_H

#include "Render_TreeCacheNode.h"
#include "Kernel/SF_Array.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render {



// ***** DrawableBundle


class DrawableBundle : public Bundle
{
protected:
    TreeCacheRoot*  pRoot;    
    Renderer2DImpl* pRenderer2D;

public:
    DrawableBundle(TreeCacheRoot *proot, Renderer2DImpl* pr2d)
        : Bundle(0), pRoot(proot), pRenderer2D(pr2d)
    { }

    HAL*  GetHAL() const;
    inline Renderer2DImpl* GetRenderer() const { return pRenderer2D; }

    // Interface used by TreeCacheShape/ShapeLayer
    // Needed because ComplexPrimitiveBundle represents its data differently.
    virtual void    UpdateMesh(BundleEntry* entry) = 0;

    //virtual void    InsertEntry(UPInt index, BundleEntry* shape);
    //virtual void    RemoveEntries(UPInt index, UPInt count);
};


//--------------------------------------------------------------------
// ***** PrimitiveBundle

// PrimitiveBundle maintains a primitive that a BundleLayer tree corresponds to,
// with the top of the tree being pTop. In-order traversal of the tree should yield
// shapes in the order of MatrixSourceNodes.

class PrimitiveBundle : public DrawableBundle
{
public:    
    Primitive Prim;
    
    // Insertions & removals should manipulate the Primitive and Matrices
    PrimitiveBundle(TreeCacheRoot *proot, const SortKey& key, Renderer2DImpl* pr2d);

    void    Draw()
    {        
        GetHAL()->Draw(&Prim);
    }

    virtual void    UpdateMesh(BundleEntry* entry);
    virtual void    InsertEntry(UPInt index, BundleEntry* shape);
    virtual void    RemoveEntries(UPInt index, UPInt count);

    void    CheckMatrixConsistency();   
};


// ***** ComplexPrimitiveBundle

class ComplexPrimitiveBundle : public Bundle, public RenderQueueItem::Interface
{
public:
    ComplexPrimitiveBundle();
    ~ComplexPrimitiveBundle();

    void    Draw(HAL* hal);
    virtual void    UpdateMesh(BundleEntry* entry);
    virtual void    InsertEntry(UPInt index, BundleEntry* shape);
    virtual void    RemoveEntries(UPInt index, UPInt count);
    
    // RenderQueueItem::Interface implementation
    virtual QIPrepareResult  Prepare(RenderQueueItem&, RenderQueueProcessor&, bool);
    virtual void             EmitToHAL(RenderQueueItem&, RenderQueueProcessor&);

private:

    struct InstanceEntry
    {
        HMatrix          M;
        Ptr<ComplexMesh> pMesh;
    };
    
    UPInt   countConsecutiveMeshesAtIndex(UPInt i) const
    {
        UPInt        j = i, count = Instances.GetSize();
        ComplexMesh* mesh = Instances[i].pMesh;
        while((j < (count-1)) && (Instances[j+1].pMesh == mesh))
            j++;
        return j-i+1;
    }

    // For now, keep one primitive per element
    ArrayLH<InstanceEntry> Instances;
};



// ***** MaskBundle

// MaskBundle maintains MaskPrimitive and updates it by pulling mask data from nodes.

class MaskBundle : public Bundle
{
public:    
    MaskPrimitive Prim;

    // Insertions & removals should manipulate the Primitive and Matrices
    MaskBundle();

    void    Draw(HAL* hal)
    {
        hal->Draw(&Prim);
    }
    
    virtual void    InsertEntry(UPInt index, BundleEntry* shape);
    virtual void    RemoveEntries(UPInt index, UPInt count);
};

// ***** FilterBundle

// FilterBundle maintains FilterPrimitive and updates it by pulling Filter data from nodes.

class FilterBundle : public Bundle
{
public:    
    FilterPrimitive Prim;

    // Insertions & removals should manipulate the Primitive and Matrices
    FilterBundle(FilterSet* filters, bool maskPresent);

    void    Draw(HAL* hal)
    {
        hal->Draw(&Prim);
    }
    
    virtual void    InsertEntry(UPInt index, BundleEntry* shape);
    virtual void    RemoveEntries(UPInt index, UPInt count);
};

// ***** BlendModeBundle

// BlendModeBundle maintains BlendPrimitive and updates it by pulling blend data from nodes.

class BlendModeBundle : public Bundle
{
public:    
    BlendPrimitive Prim;

    // Insertions & removals should manipulate the Primitive and Matrices
    BlendModeBundle(BlendMode mode, bool maskPresent);

    void    Draw(HAL* hal)
    {
        hal->Draw(&Prim);
    }

    virtual void    InsertEntry(UPInt index, BundleEntry* shape);
    virtual void    RemoveEntries(UPInt index, UPInt count);
};

// ***** ViewMatrix3DBundle

// ViewMatrix3DBundle manages a per node 3D View Matrix

class ViewMatrix3DBundle : public Bundle
{
public:    
    ViewMatrix3DPrimitive Prim;

    ViewMatrix3DBundle(Matrix3FRef* pvm);

    void    Draw(HAL* hal)   {        hal->Draw(&Prim);    }
};

// ***** ProjectionMatrix3DBundle

// ProjectionMatrix3DBundle manages a per node 3D Projection Matrix

class ProjectionMatrix3DBundle : public Bundle
{
public:    
    ProjectionMatrix3DPrimitive Prim;

    ProjectionMatrix3DBundle(Matrix4FRef *projMat);

    void    Draw(HAL* hal)   {        hal->Draw(&Prim);    }
};

// ***** UserDataBundle

// UserDataStringBundle passes string supplied by the user to the HAL.
class UserDataBundle : public Bundle
{
public:    
    UserDataPrimitive Prim;
    UserDataBundle(UserDataState::Data* data );
    void    Draw(HAL* hal)   { hal->Draw(&Prim); }
};

}} // Scaleform::Render

#endif

