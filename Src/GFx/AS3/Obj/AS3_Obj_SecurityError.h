//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_SecurityError.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_SecurityError_H
#define INC_AS3_Obj_SecurityError_H

#include "AS3_Obj_Error.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl
{
    extern const TypeInfo SecurityErrorTI;
    extern const ClassInfo SecurityErrorCI;
} // namespace fl

namespace ClassTraits { namespace fl
{
    class SecurityError;
}}

namespace InstanceTraits { namespace fl
{
    class SecurityError;
}}

namespace Classes { namespace fl
{
    class SecurityError;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl
{
    class SecurityError : public fl::Error
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::SecurityError"; }
#endif
    public:
        typedef ClassCallConstruct ClassType;
        typedef InstanceTraits::fl::Error InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        SecurityError(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

