//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_External_ExternalInterface.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_External_ExternalInterface_H
#define INC_AS3_Obj_External_ExternalInterface_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_external
{
    extern const TypeInfo ExternalInterfaceTI;
    extern const ClassInfo ExternalInterfaceCI;
} // namespace fl_external
namespace fl
{
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo FunctionTI;
    extern const ClassInfo FunctionCI;
} // namespace fl

namespace ClassTraits { namespace fl_external
{
    class ExternalInterface;
}}

namespace InstanceTraits { namespace fl_external
{
    class ExternalInterface;
}}

namespace Classes { namespace fl_external
{
    class ExternalInterface;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_external
{
    class ExternalInterface : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::ExternalInterface"; }
#endif
    public:
        typedef Classes::fl_external::ExternalInterface ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        ExternalInterface(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { ThunkInfoNum = 6 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[10];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_external
{
    class ExternalInterface : public Class
    {
        friend class ClassTraits::fl_external::ExternalInterface;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_external::ExternalInterfaceTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::ExternalInterface"; }
#endif
    public:
        typedef ExternalInterface SelfType;
        typedef ExternalInterface ClassType;
        
    private:
        ExternalInterface(ClassTraits::Traits& t);
       
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
            mid_availableGet, 
            mid_marshallExceptionsGet, 
            mid_marshallExceptionsSet, 
            mid_objectIDGet, 
            mid_addCallback, 
            mid_call, 
        };
        void availableGet(bool& result);
        void marshallExceptionsGet(bool& result);
        void marshallExceptionsSet(const Value& result, bool value);
        void objectIDGet(ASString& result);
        void addCallback(const Value& result, const ASString& functionName, const Value& closure);
        void call(Value& result, unsigned argc, const Value* const argv);

        // C++ friendly wrappers for AS3 methods.
        bool availableGet()
        {
            bool result;
            availableGet(result);
            return result;
        }
        bool marshallExceptionsGet()
        {
            bool result;
            marshallExceptionsGet(result);
            return result;
        }
        void marshallExceptionsSet(bool value)
        {
            marshallExceptionsSet(Value::GetUndefined(), value);
        }
        ASString objectIDGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            objectIDGet(result);
            return result;
        }
        void addCallback(const ASString& functionName, const Value& closure)
        {
            addCallback(Value::GetUndefined(), functionName, closure);
        }

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

