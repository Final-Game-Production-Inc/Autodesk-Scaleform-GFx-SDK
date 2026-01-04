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
public:
    DrawableBundle(HAL* hal, TreeCacheRoot *proot)
        : Bundle(hal, 0), pRoot(proot)
    { }

    // Interface used by TreeCacheShape/ShapeLayer
    // Needed because ComplexPrimitiveBundle represents its data differently.
    virtual void    UpdateMesh(HAL* hal, BundleEntry* entry) = 0;

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
    PrimitiveBundle(HAL* hal, TreeCacheRoot *proot, const SortKey& key);

    void    Draw(HAL* hal)
    {        
        hal->Draw(&Prim);
    }

    virtual void    UpdateMesh(HAL* hal, BundleEntry* entry);
    virtual void    InsertEntry(HAL* hal, UPInt index, BundleEntry* shape);
    virtual void    RemoveEntries(UPInt index, UPInt count);

    void    CheckMatrixConsistency();   
};


// ***** ComplexPrimitiveBundle

class ComplexPrimitiveBundle : public Bundle, public RenderQueueItem::Interface
{
public:
    ComplexPrimitiveBundle(HAL* hal);
    ~ComplexPrimitiveBundle();

    void    Draw(HAL* hal);
    virtual void    UpdateMesh(HAL* hal, BundleEntry* entry);
    virtual void    InsertEntry(HAL* hal, UPInt index, BundleEntry* shape);
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
    MaskBundle(HAL* hal);

    void    Draw(HAL* hal)
    {
        hal->Draw(&Prim);
    }
    
    virtual void    InsertEntry(HAL* hal, UPInt index, BundleEntry* shape);
    virtual void    RemoveEntries(UPInt index, UPInt count);
};

// ***** FilterBundle

// FilterBundle maintains FilterPrimitive and updates it by pulling Filter data from nodes.

class FilterBundle : public Bundle
{
public:    
    FilterPrimitive Prim;

    // Insertions & removals should manipulate the Primitive and Matrices
    FilterBundle(HAL* hal, FilterSet* filters, bool maskPresent);

    void    Draw(HAL* hal)
    {
        hal->Draw(&Prim);
    }
    
    virtual void    InsertEntry(HAL* hal, UPInt index, BundleEntry* shape);
    virtual void    RemoveEntries(UPInt index, UPInt count);
};

// ***** BlendModeBundle

// BlendModeBundle maintains BlendPrimitive and updates it by pulling blend data from nodes.

class BlendModeBundle : public Bundle
{
public:    
    BlendPrimitive Prim;

    // Insertions & removals should manipulate the Primitive and Matrices
    BlendModeBundle(HAL* hal, BlendMode mode, bool maskPresent);

    void    Draw(HAL* hal)
    {
        hal->Draw(&Prim);
    }

    virtual void    InsertEntry(HAL* hal, UPInt index, BundleEntry* shape);
    virtual void    RemoveEntries(UPInt index, UPInt count);
};

// ***** ViewMatrix3DBundle

// ViewMatrix3DBundle manages a per node 3D View Matrix

class ViewMatrix3DBundle : public Bundle
{
public:    
    ViewMatrix3DPrimitive Prim;

    ViewMatrix3DBundle(HAL* hal, Matrix3FRef* pvm);

    void    Draw(HAL* hal)   {        hal->Draw(&Prim);    }
};

// ***** ProjectionMatrix3DBundle

// ProjectionMatrix3DBundle manages a per node 3D Projection Matrix

class ProjectionMatrix3DBundle : public Bundle
{
public:    
    ProjectionMatrix3DPrimitive Prim;

    ProjectionMatrix3DBundle(HAL* hal, Matrix4FRef *projMat);

    void    Draw(HAL* hal)   {        hal->Draw(&Prim);    }
};

// ***** UserDataBundle

// UserDataStringBundle passes string supplied by the user to the HAL.
class UserDataBundle : public Bundle
{
public:    
    UserDataPrimitive Prim;
    UserDataBundle(HAL* hal, UserDataState::Data* data );
    void    Draw(HAL* hal)   { hal->Draw(&Prim); }
};

}} // Scaleform::Render

#endif

