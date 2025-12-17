//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_SpreadMethod.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Display_SpreadMethod_H
#define INC_AS3_Obj_Display_SpreadMethod_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_display
{
    extern const TypeInfo SpreadMethodTI;
    extern const ClassInfo SpreadMethodCI;
} // namespace fl_display

namespace ClassTraits { namespace fl_display
{
    class SpreadMethod;
}}

namespace InstanceTraits { namespace fl_display
{
    class SpreadMethod;
}}

namespace Classes { namespace fl_display
{
    class SpreadMethod;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_display
{
    class SpreadMethod : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::SpreadMethod"; }
#endif
    public:
        typedef Classes::fl_display::SpreadMethod ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        SpreadMethod(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 3 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_display
{
    class SpreadMethod : public Class
    {
        friend class ClassTraits::fl_display::SpreadMethod;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_display::SpreadMethodTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::SpreadMethod"; }
#endif
    public:
        typedef SpreadMethod SelfType;
        typedef SpreadMethod ClassType;
        
    private:
        SpreadMethod(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* PAD;
        const char* REFLECT;
        const char* REPEAT;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

