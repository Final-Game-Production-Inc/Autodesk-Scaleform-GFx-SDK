/**************************************************************************

Filename    :   AS3_AvmLoader.h
Content     :   Implementation of timeline part of Loader.
Created     :   Jan, 2011
Authors     :   Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_AS3_AvmLoader_H
#define INC_AS3_AvmLoader_H

#include "GFx/AS3/AS3_AvmDisplayObjContainer.h"

namespace Scaleform {
namespace GFx {

// ***** External Classes

namespace AS3 {

class BitmapObject;

namespace Instances
{
    namespace fl_display
    {
        class Loader;
    }
}

// An AS3-dependable part of displaylist's DisplayObject.
class AvmLoader : public DisplayObjContainer, public AvmDisplayObjContainer
{
    AvmLoader* GetThis() { return this; }
protected:
public:
    // Constructor.
    AvmLoader(ASMovieRootBase* pasRoot,
        MovieDefImpl* pdefImpl,
        InteractiveObject* pparent, 
        ResourceId id);
    ~AvmLoader();

    virtual const char* GetDefaultASClassName() const { return "flash.display.Loader"; }
    virtual String&     GetASClassName(String* className) const
    {
        SF_ASSERT(className);
        // Always create "flash.display.Bitmap" class, since
        // the custom AS class is attached to BitmapData and not to Bitmap.
        *className = AvmLoader::GetDefaultASClassName();
        return *className;
    }

    // DisplayObject methods
    virtual CharacterDef* GetCharacterDef() const
    {
        return NULL;
    }

    SF_MEMORY_REDEFINE_NEW(AvmLoader, StatMV_ActionScript_Mem)

protected:
    //virtual Ptr<Render::TreeNode> CreateRenderNode(Render::Context& context) const;
};

}}} // namespace SF::GFx::AS3

#endif // INC_AS3_AvmLoader_H
