//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Ui_Mouse.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Ui_Mouse_H
#define INC_AS3_Obj_Ui_Mouse_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_ui
{
    extern const TypeInfo MouseTI;
    extern const ClassInfo MouseCI;
} // namespace fl_ui
namespace fl
{
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl

namespace ClassTraits { namespace fl_ui
{
    class Mouse;
}}

namespace InstanceTraits { namespace fl_ui
{
    class Mouse;
}}

namespace Classes { namespace fl_ui
{
    class Mouse;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_ui
{
    class Mouse : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Mouse"; }
#endif
    public:
        typedef Classes::fl_ui::Mouse ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Mouse(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { ThunkInfoNum = 4 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[5];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_ui
{
    class Mouse : public Class
    {
        friend class ClassTraits::fl_ui::Mouse;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_ui::MouseTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::Mouse"; }
#endif
    public:
        typedef Mouse SelfType;
        typedef Mouse ClassType;
        
    private:
        Mouse(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_cursorSet, 
            mid_cursorGet, 
            mid_hide, 
            mid_show, 
        };
        void cursorSet(const Value& result, const ASString& value);
        void cursorGet(ASString& result);
        void hide(const Value& result);
        void show(const Value& result);

        // C++ friendly wrappers for AS3 methods.
        void cursorSet(const ASString& value)
        {
            cursorSet(Value::GetUndefined(), value);
        }
        ASString cursorGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            cursorGet(result);
            return result;
        }
        void hide()
        {
            hide(Value::GetUndefined());
        }
        void show()
        {
            show(Value::GetUndefined());
        }

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

