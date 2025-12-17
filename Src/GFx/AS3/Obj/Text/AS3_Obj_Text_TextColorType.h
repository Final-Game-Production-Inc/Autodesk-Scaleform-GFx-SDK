//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Text_TextColorType.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Text_TextColorType_H
#define INC_AS3_Obj_Text_TextColorType_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_text
{
    extern const TypeInfo TextColorTypeTI;
    extern const ClassInfo TextColorTypeCI;
} // namespace fl_text

namespace ClassTraits { namespace fl_text
{
    class TextColorType;
}}

namespace InstanceTraits { namespace fl_text
{
    class TextColorType;
}}

namespace Classes { namespace fl_text
{
    class TextColorType;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_text
{
    class TextColorType : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::TextColorType"; }
#endif
    public:
        typedef Classes::fl_text::TextColorType ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        TextColorType(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 2 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_text
{
    class TextColorType : public Class
    {
        friend class ClassTraits::fl_text::TextColorType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_text::TextColorTypeTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::TextColorType"; }
#endif
    public:
        typedef TextColorType SelfType;
        typedef TextColorType ClassType;
        
    private:
        TextColorType(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* DARK_COLOR;
        const char* LIGHT_COLOR;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

