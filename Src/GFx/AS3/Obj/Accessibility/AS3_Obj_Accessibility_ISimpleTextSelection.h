//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Accessibility_ISimpleTextSelection.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Accessibility_ISimpleTextSelection_H
#define INC_AS3_Obj_Accessibility_ISimpleTextSelection_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_accessibility
{
    extern const TypeInfo ISimpleTextSelectionTI;
    extern const ClassInfo ISimpleTextSelectionCI;
} // namespace fl_accessibility
namespace fl
{
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
} // namespace fl

namespace ClassTraits { namespace fl_accessibility
{
    class ISimpleTextSelection;
}}

namespace InstanceTraits { namespace fl_accessibility
{
    class ISimpleTextSelection;
}}

namespace Classes { namespace fl_accessibility
{
    class ISimpleTextSelection;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_accessibility
{
    class ISimpleTextSelection : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::ISimpleTextSelection"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::Interface InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        ISimpleTextSelection(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

