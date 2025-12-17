//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Ui_Multitouch.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Ui_Multitouch_H
#define INC_AS3_Obj_Ui_Multitouch_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_ui
{
    extern const TypeInfo MultitouchTI;
    extern const ClassInfo MultitouchCI;
} // namespace fl_ui
namespace fl
{
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
} // namespace fl
namespace fl_vec
{
    extern const TypeInfo Vector_StringTI;
    extern const ClassInfo Vector_StringCI;
} // namespace fl_vec

namespace ClassTraits { namespace fl_ui
{
    class Multitouch;
}}

namespace InstanceTraits { namespace fl_ui
{
    class Multitouch;
}}

namespace Classes { namespace fl_ui
{
    class Multitouch;
}}

//##protect##"forward_declaration"
namespace Instances
{
    namespace fl_vec
    {
        class Vector_String;
    }
}
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_ui
{
    class Multitouch : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Multitouch"; }
#endif
    public:
        typedef Classes::fl_ui::Multitouch ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Multitouch(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { ThunkInfoNum = 6 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[7];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_ui
{
    class Multitouch : public Class
    {
        friend class ClassTraits::fl_ui::Multitouch;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_ui::MultitouchTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::Multitouch"; }
#endif
    public:
        typedef Multitouch SelfType;
        typedef Multitouch ClassType;
        
    private:
        Multitouch(ClassTraits::Traits& t);
       
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
            mid_inputModeGet, 
            mid_inputModeSet, 
            mid_maxTouchPointsGet, 
            mid_supportedGesturesGet, 
            mid_supportsGestureEventsGet, 
            mid_supportsTouchEventsGet, 
        };
        void inputModeGet(ASString& result);
        void inputModeSet(const Value& result, const ASString& value);
        void maxTouchPointsGet(SInt32& result);
        void supportedGesturesGet(SPtr<Instances::fl_vec::Vector_String>& result);
        void supportsGestureEventsGet(bool& result);
        void supportsTouchEventsGet(bool& result);

        // C++ friendly wrappers for AS3 methods.
        ASString inputModeGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            inputModeGet(result);
            return result;
        }
        void inputModeSet(const ASString& value)
        {
            inputModeSet(Value::GetUndefined(), value);
        }
        SInt32 maxTouchPointsGet()
        {
            SInt32 result;
            maxTouchPointsGet(result);
            return result;
        }
        SPtr<Instances::fl_vec::Vector_String> supportedGesturesGet();
        bool supportsGestureEventsGet()
        {
            bool result;
            supportsGestureEventsGet(result);
            return result;
        }
        bool supportsTouchEventsGet()
        {
            bool result;
            supportsTouchEventsGet(result);
            return result;
        }

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

