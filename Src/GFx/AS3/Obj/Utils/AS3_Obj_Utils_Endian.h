//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Utils_Endian.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Utils_Endian_H
#define INC_AS3_Obj_Utils_Endian_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_utils
{
    extern const TypeInfo EndianTI;
    extern const ClassInfo EndianCI;
} // namespace fl_utils

namespace ClassTraits { namespace fl_utils
{
    class Endian;
}}

namespace InstanceTraits { namespace fl_utils
{
    class Endian;
}}

namespace Classes { namespace fl_utils
{
    class Endian;
}}

//##protect##"forward_declaration"
#ifdef BIG_ENDIAN
#undef BIG_ENDIAN
#endif

#ifdef LITTLE_ENDIAN
#undef LITTLE_ENDIAN
#endif
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_utils
{
    class Endian : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Endian"; }
#endif
    public:
        typedef Classes::fl_utils::Endian ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Endian(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 2 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_utils
{
    class Endian : public Class
    {
        friend class ClassTraits::fl_utils::Endian;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_utils::EndianTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::Endian"; }
#endif
    public:
        typedef Endian SelfType;
        typedef Endian ClassType;
        
    private:
        Endian(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* BIG_ENDIAN;
        const char* LITTLE_ENDIAN;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

