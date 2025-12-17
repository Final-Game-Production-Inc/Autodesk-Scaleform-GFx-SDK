//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_ArgumentError.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_ArgumentError_H
#define INC_AS3_Obj_ArgumentError_H

#include "AS3_Obj_Error.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl
{
    extern const TypeInfo ArgumentErrorTI;
    extern const ClassInfo ArgumentErrorCI;
} // namespace fl

namespace ClassTraits { namespace fl
{
    class ArgumentError;
}}

namespace InstanceTraits { namespace fl
{
    class ArgumentError;
}}

namespace Classes { namespace fl
{
    class ArgumentError;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl
{
    class ArgumentError : public fl::Error
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::ArgumentError"; }
#endif
    public:
        typedef ClassCallConstruct ClassType;
        typedef InstanceTraits::fl::Error InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        ArgumentError(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

