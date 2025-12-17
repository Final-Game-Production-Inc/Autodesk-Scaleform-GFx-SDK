//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Net_FileReference.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Net_FileReference_H
#define INC_AS3_Obj_Net_FileReference_H

#include "../Events/AS3_Obj_Events_EventDispatcher.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_net
{
    extern const TypeInfo FileReferenceTI;
    extern const ClassInfo FileReferenceCI;
    extern const TypeInfo URLRequestTI;
    extern const ClassInfo URLRequestCI;
} // namespace fl_net
namespace fl
{
    extern const TypeInfo DateTI;
    extern const ClassInfo DateCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo ArrayTI;
    extern const ClassInfo ArrayCI;
} // namespace fl

namespace ClassTraits { namespace fl_net
{
    class FileReference;
}}

namespace InstanceTraits { namespace fl_net
{
    class FileReference;
}}

namespace Classes { namespace fl_net
{
    class FileReference;
}}

//##protect##"forward_declaration"
namespace Instances
{
    class Date;
    class URLRequest;
}
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_net
{
    class FileReference : public fl_events::EventDispatcher
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::FileReference"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_events::EventDispatcher InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        FileReference(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

