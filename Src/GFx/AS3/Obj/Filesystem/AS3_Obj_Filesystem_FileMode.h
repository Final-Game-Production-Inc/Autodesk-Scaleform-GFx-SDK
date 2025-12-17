//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Filesystem_FileMode.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Filesystem_FileMode_H
#define INC_AS3_Obj_Filesystem_FileMode_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_filesystem
{
    extern const TypeInfo FileModeTI;
    extern const ClassInfo FileModeCI;
} // namespace fl_filesystem

namespace ClassTraits { namespace fl_filesystem
{
    class FileMode;
}}

namespace InstanceTraits { namespace fl_filesystem
{
    class FileMode;
}}

namespace Classes { namespace fl_filesystem
{
    class FileMode;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_filesystem
{
    class FileMode : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::FileMode"; }
#endif
    public:
        typedef Classes::fl_filesystem::FileMode ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        FileMode(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 4 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_filesystem
{
    class FileMode : public Class
    {
        friend class ClassTraits::fl_filesystem::FileMode;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_filesystem::FileModeTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::FileMode"; }
#endif
    public:
        typedef FileMode SelfType;
        typedef FileMode ClassType;
        
    private:
        FileMode(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* APPEND;
        const char* READ;
        const char* UPDATE;
        const char* WRITE;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

