//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Filesystem_FileStream.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Filesystem_FileStream_H
#define INC_AS3_Obj_Filesystem_FileStream_H

#include "../Events/AS3_Obj_Events_EventDispatcher.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_filesystem
{
    extern const TypeInfo FileStreamTI;
    extern const ClassInfo FileStreamCI;
    extern const TypeInfo FileTI;
    extern const ClassInfo FileCI;
} // namespace fl_filesystem
namespace fl
{
    extern const TypeInfo uintTI;
    extern const ClassInfo uintCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
    extern const TypeInfo anyTI;
    extern const ClassInfo anyCI;
} // namespace fl
namespace fl_utils
{
    extern const TypeInfo ByteArrayTI;
    extern const ClassInfo ByteArrayCI;
    extern const TypeInfo IDataInputTI;
    extern const ClassInfo IDataInputCI;
    extern const TypeInfo IDataOutputTI;
    extern const ClassInfo IDataOutputCI;
} // namespace fl_utils

namespace ClassTraits { namespace fl_filesystem
{
    class FileStream;
}}

namespace InstanceTraits { namespace fl_filesystem
{
    class FileStream;
}}

namespace Classes { namespace fl_filesystem
{
    class FileStream;
}}

//##protect##"forward_declaration"
namespace Instances
{
    class File;
    class ByteArray;
}
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_filesystem
{
    class FileStream : public fl_events::EventDispatcher
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::FileStream"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_events::EventDispatcher InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        FileStream(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

