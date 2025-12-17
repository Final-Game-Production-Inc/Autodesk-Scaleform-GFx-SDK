//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Security_IURIDereferencer.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Security_IURIDereferencer_H
#define INC_AS3_Obj_Security_IURIDereferencer_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_security
{
    extern const TypeInfo IURIDereferencerTI;
    extern const ClassInfo IURIDereferencerCI;
} // namespace fl_security
namespace fl_utils
{
    extern const TypeInfo IDataInputTI;
    extern const ClassInfo IDataInputCI;
} // namespace fl_utils
namespace fl
{
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl

namespace ClassTraits { namespace fl_security
{
    class IURIDereferencer;
}}

namespace InstanceTraits { namespace fl_security
{
    class IURIDereferencer;
}}

namespace Classes { namespace fl_security
{
    class IURIDereferencer;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_security
{
    class IURIDereferencer : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::IURIDereferencer"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::Interface InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        IURIDereferencer(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

