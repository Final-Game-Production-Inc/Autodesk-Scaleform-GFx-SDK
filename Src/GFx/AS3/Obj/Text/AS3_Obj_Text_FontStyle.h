//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Text_FontStyle.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Text_FontStyle_H
#define INC_AS3_Obj_Text_FontStyle_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_text
{
    extern const TypeInfo FontStyleTI;
    extern const ClassInfo FontStyleCI;
} // namespace fl_text

namespace ClassTraits { namespace fl_text
{
    class FontStyle;
}}

namespace InstanceTraits { namespace fl_text
{
    class FontStyle;
}}

namespace Classes { namespace fl_text
{
    class FontStyle;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_text
{
    class FontStyle : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::FontStyle"; }
#endif
    public:
        typedef Classes::fl_text::FontStyle ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        FontStyle(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 4 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_text
{
    class FontStyle : public Class
    {
        friend class ClassTraits::fl_text::FontStyle;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_text::FontStyleTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::FontStyle"; }
#endif
    public:
        typedef FontStyle SelfType;
        typedef FontStyle ClassType;
        
    private:
        FontStyle(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* BOLD;
        const char* BOLD_ITALIC;
        const char* ITALIC;
        const char* REGULAR;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

