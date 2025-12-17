//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Html_HTMLHost.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Html_HTMLHost_H
#define INC_AS3_Obj_Html_HTMLHost_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_html
{
    extern const TypeInfo HTMLHostTI;
    extern const ClassInfo HTMLHostCI;
    extern const TypeInfo HTMLLoaderTI;
    extern const ClassInfo HTMLLoaderCI;
    extern const TypeInfo HTMLWindowCreateOptionsTI;
    extern const ClassInfo HTMLWindowCreateOptionsCI;
} // namespace fl_html
namespace fl_geom
{
    extern const TypeInfo RectangleTI;
    extern const ClassInfo RectangleCI;
} // namespace fl_geom
namespace fl
{
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl

namespace ClassTraits { namespace fl_html
{
    class HTMLHost;
}}

namespace InstanceTraits { namespace fl_html
{
    class HTMLHost;
}}

namespace Classes { namespace fl_html
{
    class HTMLHost;
}}

//##protect##"forward_declaration"
namespace Instances
{
    class Rectangle;
    class HTMLWindowCreateOptions;
}
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_html
{
    class HTMLHost : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::HTMLHost"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        HTMLHost(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

