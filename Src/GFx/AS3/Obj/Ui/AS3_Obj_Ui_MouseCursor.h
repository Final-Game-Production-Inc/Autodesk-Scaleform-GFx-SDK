//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Ui_MouseCursor.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Ui_MouseCursor_H
#define INC_AS3_Obj_Ui_MouseCursor_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_ui
{
    extern const TypeInfo MouseCursorTI;
    extern const ClassInfo MouseCursorCI;
} // namespace fl_ui

namespace ClassTraits { namespace fl_ui
{
    class MouseCursor;
}}

namespace InstanceTraits { namespace fl_ui
{
    class MouseCursor;
}}

namespace Classes { namespace fl_ui
{
    class MouseCursor;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_ui
{
    class MouseCursor : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::MouseCursor"; }
#endif
    public:
        typedef Classes::fl_ui::MouseCursor ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        MouseCursor(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 5 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_ui
{
    class MouseCursor : public Class
    {
        friend class ClassTraits::fl_ui::MouseCursor;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_ui::MouseCursorTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::MouseCursor"; }
#endif
    public:
        typedef MouseCursor SelfType;
        typedef MouseCursor ClassType;
        
    private:
        MouseCursor(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* ARROW;
        const char* AUTO;
        const char* BUTTON;
        const char* HAND;
        const char* IBEAM;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

