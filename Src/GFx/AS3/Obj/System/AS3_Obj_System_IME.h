//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_System_IME.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_System_IME_H
#define INC_AS3_Obj_System_IME_H

#include "../Events/AS3_Obj_Events_EventDispatcher.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_system
{
    extern const TypeInfo IMETI;
    extern const ClassInfo IMECI;
} // namespace fl_system
namespace fl
{
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
} // namespace fl

namespace ClassTraits { namespace fl_system
{
    class IME;
}}

namespace InstanceTraits { namespace fl_system
{
    class IME;
}}

namespace Classes { namespace fl_system
{
    class IME;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_system
{
    class IME : public fl_events::EventDispatcher
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::IME"; }
#endif
    public:
        typedef Classes::fl_system::IME ClassType;
        typedef InstanceTraits::fl_events::EventDispatcher InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        IME(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { ThunkInfoNum = 6 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[9];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_system
{
    class IME : public Class
    {
        friend class ClassTraits::fl_system::IME;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_system::IMETI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::IME"; }
#endif
    public:
        typedef IME SelfType;
        typedef IME ClassType;
        
    private:
        IME(ClassTraits::Traits& t);
       
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
            mid_conversionModeSet, 
            mid_conversionModeGet, 
            mid_enabledGet, 
            mid_enabledSet, 
            mid_doConversion, 
            mid_setCompositionString, 
        };
        void conversionModeSet(const Value& result, const ASString& value);
        void conversionModeGet(ASString& result);
        void enabledGet(bool& result);
        void enabledSet(const Value& result, bool value);
        void doConversion(const Value& result);
        void setCompositionString(const Value& result, const ASString& composition);

        // C++ friendly wrappers for AS3 methods.
        void conversionModeSet(const ASString& value)
        {
            conversionModeSet(Value::GetUndefined(), value);
        }
        ASString conversionModeGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            conversionModeGet(result);
            return result;
        }
        bool enabledGet()
        {
            bool result;
            enabledGet(result);
            return result;
        }
        void enabledSet(bool value)
        {
            enabledSet(Value::GetUndefined(), value);
        }
        void doConversion()
        {
            doConversion(Value::GetUndefined());
        }
        void setCompositionString(const ASString& composition)
        {
            setCompositionString(Value::GetUndefined(), composition);
        }

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

