//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Filesystem_File.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Filesystem_File_H
#define INC_AS3_Obj_Filesystem_File_H

#include "../Net/AS3_Obj_Net_FileReference.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_filesystem
{
    extern const TypeInfo FileTI;
    extern const ClassInfo FileCI;
} // namespace fl_filesystem
namespace fl
{
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo ArrayTI;
    extern const ClassInfo ArrayCI;
} // namespace fl
namespace fl_desktop
{
    extern const TypeInfo IconTI;
    extern const ClassInfo IconCI;
} // namespace fl_desktop
namespace fl_net
{
    extern const TypeInfo FileReferenceTI;
    extern const ClassInfo FileReferenceCI;
} // namespace fl_net

namespace ClassTraits { namespace fl_filesystem
{
    class File;
}}

namespace InstanceTraits { namespace fl_filesystem
{
    class File;
}}

namespace Classes { namespace fl_filesystem
{
    class File;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_filesystem
{
    class File : public fl_net::FileReference
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::File"; }
#endif
    public:
        typedef Classes::fl_filesystem::File ClassType;
        typedef InstanceTraits::fl_events::EventDispatcher InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        File(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_filesystem
{
    class File : public Class
    {
        friend class ClassTraits::fl_filesystem::File;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_filesystem::FileTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::File"; }
#endif
    public:
        typedef File SelfType;
        typedef File ClassType;
        
    private:
        File(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

