//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Text_AntiAliasType.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Text_AntiAliasType_H
#define INC_AS3_Obj_Text_AntiAliasType_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_text
{
    extern const TypeInfo AntiAliasTypeTI;
    extern const ClassInfo AntiAliasTypeCI;
} // namespace fl_text

namespace ClassTraits { namespace fl_text
{
    class AntiAliasType;
}}

namespace InstanceTraits { namespace fl_text
{
    class AntiAliasType;
}}

namespace Classes { namespace fl_text
{
    class AntiAliasType;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_text
{
    class AntiAliasType : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::AntiAliasType"; }
#endif
    public:
        typedef Classes::fl_text::AntiAliasType ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        AntiAliasType(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 2 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_text
{
    class AntiAliasType : public Class
    {
        friend class ClassTraits::fl_text::AntiAliasType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_text::AntiAliasTypeTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::AntiAliasType"; }
#endif
    public:
        typedef AntiAliasType SelfType;
        typedef AntiAliasType ClassType;
        
    private:
        AntiAliasType(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* ADVANCED;
        const char* NORMAL;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

