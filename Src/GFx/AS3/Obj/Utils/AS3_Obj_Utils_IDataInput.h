//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Utils_IDataInput.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Utils_IDataInput_H
#define INC_AS3_Obj_Utils_IDataInput_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_utils
{
    extern const TypeInfo IDataInputTI;
    extern const ClassInfo IDataInputCI;
    extern const TypeInfo ByteArrayTI;
    extern const ClassInfo ByteArrayCI;
} // namespace fl_utils
namespace fl
{
    extern const TypeInfo uintTI;
    extern const ClassInfo uintCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
} // namespace fl

namespace ClassTraits { namespace fl_utils
{
    class IDataInput;
}}

namespace InstanceTraits { namespace fl_utils
{
    class IDataInput;
}}

namespace Classes { namespace fl_utils
{
    class IDataInput;
}}

//##protect##"forward_declaration"
namespace Instances
{
    class ByteArray;
}
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_utils
{
    class IDataInput : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::IDataInput"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::Interface InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        IDataInput(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

