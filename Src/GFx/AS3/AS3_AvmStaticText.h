/**************************************************************************

Filename    :   AS3_AvmStaticText.h
Content     :   Implementation of timeline part of StaticText.
Created     :   Jun, 2010
Authors     :   Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_AS3_AvmStaticText_H
#define INC_AS3_AvmStaticText_H

#include "GFx/AS3/AS3_AvmDisplayObj.h"
#include "GFx/GFx_StaticText.h"

namespace Scaleform {
namespace GFx {

// ***** External Classes

namespace AS3 {

// An AS3-dependable part of displaylist's DisplayObject.
class AvmStaticText : public AvmDisplayObj
{
public:
    // Constructor.
    AvmStaticText(StaticTextCharacter*);
    ~AvmStaticText();

    virtual const char* GetDefaultASClassName() const { return "flash.text.StaticText"; }
    virtual String&     GetASClassName(String* className) const
    {
        SF_ASSERT(className);
        // Always create "flash.display.Bitmap" class, since
        // the custom AS class is attached to BitmapData and not to Bitmap.
        *className = AvmStaticText::GetDefaultASClassName();
        return *className;
    }

#ifdef SF_OBJ_COLLECTOR_ENABLED
public:
    virtual const char* GetObjectTypeName() const { return "GFx::StaticText"; }
#endif
};

}}} // namespace SF::GFx::AS3

#endif // INC_AS3_AvmStaticText_H
