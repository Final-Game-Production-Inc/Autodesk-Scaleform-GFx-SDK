/**************************************************************************

Filename    :   AS3_AvmBitmap.h
Content     :   Implementation of timeline part of Bitmap.
Created     :   Jun, 2010
Authors     :   Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_AS3_AvmBitmap_H
#define INC_AS3_AvmBitmap_H

#include "GFx/AS3/AS3_AvmDisplayObj.h"

namespace Scaleform {
namespace GFx {

// ***** External Classes

namespace AS3 {

class BitmapObject;

namespace Instances
{
    namespace fl_display
    {
        class BitmapData;
        class Bitmap;
    }
}

// An AS3-dependable part of displaylist's DisplayObject.
class AvmBitmap : public DisplayObject, public AvmDisplayObj
{
    AvmBitmap* GetThis() { return this; }
protected:
    Ptr<MovieDefImpl>           pDefImpl;
    mutable Ptr<ImageResource>  pImage;
public:
    // Constructor.
    AvmBitmap(ASMovieRootBase* pasRoot,
        const CharacterCreateInfo& ccinfo,
        InteractiveObject* pparent, 
        ResourceId id);
    ~AvmBitmap();

    virtual unsigned    CreateASInstance(bool execute);
    virtual const char* GetDefaultASClassName() const { return "flash.display.Bitmap"; }
    virtual String&     GetASClassName(String* className) const
    {
        SF_ASSERT(className);
        // Always create "flash.display.Bitmap" class, since
        // the custom AS class is attached to BitmapData and not to Bitmap.
        *className = AvmBitmap::GetDefaultASClassName();
        return *className;
    }
    virtual MovieDefImpl* GetResourceMovieDef() const
    {
        // Return the definition where binding takes place.
        return pDefImpl.GetPtr();
    }

    // DisplayObject methods
    virtual CharacterDef* GetCharacterDef() const
    {
        return NULL;
    }

    // These are used for finding bounds, width and height.
    virtual RectF       GetBounds(const Matrix &transform) const;
    virtual RectF       GetRectBounds(const Matrix &transform) const
    {     
        return GetBounds(transform);
    }   

    virtual bool        PointTestLocal(const Render::PointF &pt, UInt8 hitTestMask = 0) const;

    virtual void        ForceShutdown();

    // Override this to hit-test shapes and make Button-Mode sprites work.
    virtual TopMostResult GetTopMostMouseEntity(const Render::PointF &pt, TopMostDescr* pdescr);

    //////////////////////////////////////////////////////////////////////////
    void                   SetResourceMovieDef(MovieDefImpl* md);

    Instances::fl_display::Bitmap*     GetAS3Bitmap() const;
    Instances::fl_display::BitmapData* GetBitmapData() const;
    void                   SetImage(ImageResource* img) { pImage = img; }

    Render::TreeNode*      RecreateRenderNode() const;

    SF_MEMORY_REDEFINE_NEW(AvmBitmap, StatMV_ActionScript_Mem)

protected:
    virtual Ptr<Render::TreeNode> CreateRenderNode(Render::Context& context) const;

    bool    CreateBitmapShape() const;

#ifdef SF_OBJ_COLLECTOR_ENABLED
public:
    virtual const char* GetObjectTypeName() const { return "GFx::Bitmap"; }
#endif
};

}}} // namespace SF::GFx::AS3

#endif // INC_AS3_AvmBitmap_H
