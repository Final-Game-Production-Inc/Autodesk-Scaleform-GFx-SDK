//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Text_FontType.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Text_FontType_H
#define INC_AS3_Obj_Text_FontType_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_text
{
    extern const TypeInfo FontTypeTI;
    extern const ClassInfo FontTypeCI;
} // namespace fl_text

namespace ClassTraits { namespace fl_text
{
    class FontType;
}}

namespace InstanceTraits { namespace fl_text
{
    class FontType;
}}

namespace Classes { namespace fl_text
{
    class FontType;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_text
{
    class FontType : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::FontType"; }
#endif
    public:
        typedef Classes::fl_text::FontType ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        FontType(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 2 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_text
{
    class FontType : public Class
    {
        friend class ClassTraits::fl_text::FontType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_text::FontTypeTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::FontType"; }
#endif
    public:
        typedef FontType SelfType;
        typedef FontType ClassType;
        
    private:
        FontType(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* DEVICE;
        const char* EMBEDDED;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

