//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Net_URLStream.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Net_URLStream_H
#define INC_AS3_Obj_Net_URLStream_H

#include "../Events/AS3_Obj_Events_EventDispatcher.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_net
{
    extern const TypeInfo URLStreamTI;
    extern const ClassInfo URLStreamCI;
    extern const TypeInfo URLRequestTI;
    extern const ClassInfo URLRequestCI;
} // namespace fl_net
namespace fl
{
    extern const TypeInfo uintTI;
    extern const ClassInfo uintCI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
} // namespace fl
namespace fl_utils
{
    extern const TypeInfo ByteArrayTI;
    extern const ClassInfo ByteArrayCI;
    extern const TypeInfo IDataInputTI;
    extern const ClassInfo IDataInputCI;
} // namespace fl_utils

namespace ClassTraits { namespace fl_net
{
    class URLStream;
}}

namespace InstanceTraits { namespace fl_net
{
    class URLStream;
}}

namespace Classes { namespace fl_net
{
    class URLStream;
}}

//##protect##"forward_declaration"
namespace Instances
{
    class URLRequest;
    class ByteArray;
}
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_net
{
    class URLStream : public fl_events::EventDispatcher
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::URLStream"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_events::EventDispatcher InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        URLStream(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

