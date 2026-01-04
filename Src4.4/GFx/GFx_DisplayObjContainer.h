/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_DisplayObjContainer.h
Content     :   Implementation of DisplayObjContainer
Created     :   
Authors     :   Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_SF_GFX_DISPLAYOBJCONTAINER_H
#define INC_SF_GFX_DISPLAYOBJCONTAINER_H

#include "GFx/GFx_InteractiveObject.h"
#include "GFx/GFx_DisplayList.h"

namespace Scaleform { namespace GFx {

class AvmDisplayObjContainerBase : public AvmInteractiveObjBase 
{
public:
    typedef DisplayObjectBase::TopMostDescr TopMostDescr;
    typedef DisplayObjectBase::TopMostResult TopMostResult;

    virtual TopMostResult GetTopMostEntity(const Render::PointF &localPt, TopMostDescr* pdescr,
                                           const ArrayPOD<UByte>& hitTest) =0;
    virtual void    FillTabableArray(InteractiveObject::FillTabableParams* params) =0;
};

// Implementation of DisplayObjectContainer class. This is a base class
// for Sprite. It contains DisplayList.
class DisplayObjContainer : public InteractiveObject
{
protected:
    DisplayList         mDisplayList;    

    // A node used by every root sprite in a tree so that they can be
    // scanned and initialized efficiently to a thread-synched Advance.
    // Null for all non-root sprites.
    MovieDefRootNode*   pRootNode;

public:
    DisplayObjContainer
        (MovieDefImpl* pbindingDefImpl, 
         ASMovieRootBase* pasRoot,
         InteractiveObject* pparent, 
         ResourceId id);
    virtual ~DisplayObjContainer();

    //*** DisplayObjectBase overloaded methods
    // Return character bounds in specified coordinate space.
    virtual RectF           GetBounds(const Matrix &t) const     
    { 
        return mDisplayList.GetBounds(t); 
    }
    virtual CharacterDef*   GetCharacterDef() const { return NULL; }
    virtual void            PropagateScale9GridExists();
    virtual void            SetScale9Grid(const RectF& rect);

    // Returns character bounds excluding stroke
    virtual RectF           GetRectBounds(const Matrix &t) const 
    { 
        return mDisplayList.GetRectBounds(t); 
    }
    virtual FontManager*    GetFontManager() const;

    virtual DisplayObjectBase*  AddDisplayObject(
        const CharPosInfo &pos, 
        const ASString& name,
        const ArrayLH<SwfEvent*, StatMD_Tags_Mem>* peventHandlers,
        const void *pinitSource,
        unsigned createFrame = SF_MAX_UINT,
        UInt32 addFlags = 0,
        CharacterCreateInfo* pcharCreateOverride = 0,
        InteractiveObject* origChar = 0)
    {
        SF_ASSERT(0); // shouldn't be called!
        SF_UNUSED7(pos, name, peventHandlers, pinitSource, createFrame, addFlags, pcharCreateOverride);
        SF_UNUSED(origChar);
        return NULL;
    }
    // Set state changed flags
    virtual void            SetStateChangeFlags(UInt8 flags);

    void                ClearDisplayList()
    {
        mDisplayList.Clear(this);
        SetDirtyFlag();
    }
    // Determines whether the specified display object is a child (or grandchild, or grandgrand...)
    // of the DisplayObjectContainer instance or the instance itself.
    bool                Contains(DisplayObjectBase* ch) const;

    virtual void        ExecuteImportedInitActions(MovieDef*) {}
    virtual void        ForceShutdown ();

    virtual Sprite*     GetHitArea() const { return NULL; }
    virtual Sprite*     GetHitAreaHolder() const { return NULL; }

    // For debugging -- return the id of the DisplayObjectBase at the specified depth.
    // Return 0 if nobody's home.
    DisplayObjectBase*  GetCharacterAtDepth(int depth);
    DisplayObjectBase*  GetCharacterAtDepth(int depth, 
                                                bool *markedForRemove);
    DisplayObjectBase*  GetChildAt(unsigned index) const;
    DisplayObjectBase*  GetChildByName(const ASString& name) const
    {
        return mDisplayList.GetDisplayObjectByName(name);
    }
    // Returns index of the child display object. Returns ~0u if specified
    // display object is not child.
    SPInt               GetChildIndex(DisplayObjectBase* ch) const
    {
        return mDisplayList.FindDisplayIndex(ch);
    }
    
    // Return -1 if nobody's home.
    ResourceId          GetIdAtDepth(int depth);
    // Returns maximum used depth, -1 if no depth is available.
    int                 GetLargestDepthInUse() const { return mDisplayList.GetLargestDepthInUse();}
    unsigned            GetNumChildren() const { return (unsigned)mDisplayList.GetCount(); }

    virtual TopMostResult GetTopMostMouseEntity(const Render::PointF &pt, TopMostDescr* pdescr);

    virtual void        OnInsertionAsLevel(int level);

    void                MoveDisplayObject(const CharPosInfo &pos); 
    virtual void        CreateAndReplaceDisplayObject(const CharPosInfo &pos, const ASString& name, DisplayObjectBase** newChar);

    virtual bool        PointTestLocal(const Render::PointF &pt, UInt8 hitTestMask = 0) const;

    // Propagates an event to all children.
    virtual void        PropagateMouseEvent(const EventId& id);
    virtual void        PropagateKeyEvent(const EventId& id, int* pkeyMask);
    virtual void        PropagateFocusGroupMask(unsigned mask);

    void                ReplaceDisplayObject(const CharPosInfo &pos, DisplayObjectBase* ch, const ASString& name);
    void                RemoveDisplayObject(int depth, ResourceId id); 
    void                RemoveDisplayObject() { InteractiveObject::RemoveDisplayObject(); }
    virtual void        RemoveDisplayObject(DisplayObjectBase* ch);

    SF_INLINE void      VisitChildren(DisplayList::MemberVisitor *pvisitor) const
    {
        mDisplayList.VisitMembers(pvisitor);
    }
    DisplayList&        GetDisplayList()        { return mDisplayList; }
    const DisplayList&  GetDisplayList() const  { return mDisplayList; }
    //virtual void        InsertBranchToPlayList(InteractiveObject* pbegin, InteractiveObject* pend);
    void                GetChildDescTree(AmpMovieObjectDesc* parent, MemoryHeap* heap) const;

    void                FillTabableArray(InteractiveObject::FillTabableParams* params);

#ifdef SF_BUILD_DEBUG
    void                DumpDisplayList(int indent, const char* title = NULL);
#else
    inline void         DumpDisplayList(int , const char*  = NULL) {}
#endif

    void                CalcDisplayListHitTestMaskArray(
        ArrayPOD<UByte> *phitTest, const Render::PointF &p, bool testShape) const;

    MovieDefRootNode*   FindRootNode() const;

    virtual bool        Has3D();                        // checks object and descendants
    virtual void        UpdateViewAndPerspective();

protected:

    void AssignRootNode(bool importFlag = false);

private:
    SF_INLINE MemoryHeap*       GetMovieHeap() const; // impl in cpp
    SF_INLINE AvmDisplayObjContainerBase*  GetAvmDispContainer()
    {
        return GetAvmObjImpl()->ToAvmDispContainerBase();
    }
};

// conversion methods 
SF_INLINE
DisplayObjContainer* DisplayObjectBase::CharToDisplayObjContainer()
{
    return IsDisplayObjContainer() ? static_cast<DisplayObjContainer*>(this) : 0;
}

SF_INLINE
DisplayObjContainer* DisplayObjectBase::CharToDisplayObjContainer_Unsafe()
{
    SF_ASSERT(IsDisplayObjContainer());
    return static_cast<DisplayObjContainer*>(this);
}

SF_INLINE
const DisplayObjContainer* DisplayObjectBase::CharToDisplayObjContainer() const 
{
    return IsDisplayObjContainer() ? static_cast<const DisplayObjContainer*>(this) : 0;
}

SF_INLINE
const DisplayObjContainer* DisplayObjectBase::CharToDisplayObjContainer_Unsafe() const 
{
    SF_ASSERT(IsDisplayObjContainer());
    return static_cast<const DisplayObjContainer*>(this);
}

SF_INLINE
DisplayObjectBase* DisplayObjContainer::GetCharacterAtDepth(int depth, 
                                                            bool *markedForRemove)
{
    return mDisplayList.GetDisplayObjectAtDepth(depth, markedForRemove);
}


}} // namespace Scaleform::GFx
#endif //INC_SF_GFX_DISPLAYOBJCONTAINER_H
