//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_System_IMEConversionMode.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_System_IMEConversionMode_H
#define INC_AS3_Obj_System_IMEConversionMode_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_system
{
    extern const TypeInfo IMEConversionModeTI;
    extern const ClassInfo IMEConversionModeCI;
} // namespace fl_system

namespace ClassTraits { namespace fl_system
{
    class IMEConversionMode;
}}

namespace InstanceTraits { namespace fl_system
{
    class IMEConversionMode;
}}

namespace Classes { namespace fl_system
{
    class IMEConversionMode;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_system
{
    class IMEConversionMode : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::IMEConversionMode"; }
#endif
    public:
        typedef Classes::fl_system::IMEConversionMode ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        IMEConversionMode(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 8 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_system
{
    class IMEConversionMode : public Class
    {
        friend class ClassTraits::fl_system::IMEConversionMode;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_system::IMEConversionModeTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::IMEConversionMode"; }
#endif
    public:
        typedef IMEConversionMode SelfType;
        typedef IMEConversionMode ClassType;
        
    private:
        IMEConversionMode(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* ALPHANUMERIC_FULL;
        const char* ALPHANUMERIC_HALF;
        const char* CHINESE;
        const char* JAPANESE_HIRAGANA;
        const char* JAPANESE_KATAKANA_FULL;
        const char* JAPANESE_KATAKANA_HALF;
        const char* KOREAN;
        const char* UNKNOWN;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

