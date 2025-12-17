//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Ui_KeyLocation.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Ui_KeyLocation_H
#define INC_AS3_Obj_Ui_KeyLocation_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_ui
{
    extern const TypeInfo KeyLocationTI;
    extern const ClassInfo KeyLocationCI;
} // namespace fl_ui

namespace ClassTraits { namespace fl_ui
{
    class KeyLocation;
}}

namespace InstanceTraits { namespace fl_ui
{
    class KeyLocation;
}}

namespace Classes { namespace fl_ui
{
    class KeyLocation;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_ui
{
    class KeyLocation : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::KeyLocation"; }
#endif
    public:
        typedef Classes::fl_ui::KeyLocation ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        KeyLocation(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 4 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_ui
{
    class KeyLocation : public Class
    {
        friend class ClassTraits::fl_ui::KeyLocation;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_ui::KeyLocationTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::KeyLocation"; }
#endif
    public:
        typedef KeyLocation SelfType;
        typedef KeyLocation ClassType;
        
    private:
        KeyLocation(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const UInt32 LEFT;
        const UInt32 NUM_PAD;
        const UInt32 RIGHT;
        const UInt32 STANDARD;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

