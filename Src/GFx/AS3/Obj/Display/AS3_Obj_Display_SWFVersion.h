//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_SWFVersion.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Display_SWFVersion_H
#define INC_AS3_Obj_Display_SWFVersion_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_display
{
    extern const TypeInfo SWFVersionTI;
    extern const ClassInfo SWFVersionCI;
} // namespace fl_display

namespace ClassTraits { namespace fl_display
{
    class SWFVersion;
}}

namespace InstanceTraits { namespace fl_display
{
    class SWFVersion;
}}

namespace Classes { namespace fl_display
{
    class SWFVersion;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_display
{
    class SWFVersion : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::SWFVersion"; }
#endif
    public:
        typedef Classes::fl_display::SWFVersion ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        SWFVersion(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 9 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_display
{
    class SWFVersion : public Class
    {
        friend class ClassTraits::fl_display::SWFVersion;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_display::SWFVersionTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::SWFVersion"; }
#endif
    public:
        typedef SWFVersion SelfType;
        typedef SWFVersion ClassType;
        
    private:
        SWFVersion(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const UInt32 FLASH1;
        const UInt32 FLASH2;
        const UInt32 FLASH3;
        const UInt32 FLASH4;
        const UInt32 FLASH5;
        const UInt32 FLASH6;
        const UInt32 FLASH7;
        const UInt32 FLASH8;
        const UInt32 FLASH9;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

