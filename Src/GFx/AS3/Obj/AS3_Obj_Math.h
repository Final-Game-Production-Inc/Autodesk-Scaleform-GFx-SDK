//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Math.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Math_H
#define INC_AS3_Obj_Math_H

#include "AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl
{
    extern const TypeInfo MathTI;
    extern const ClassInfo MathCI;
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
} // namespace fl

namespace ClassTraits { namespace fl
{
    class Math;
}}

namespace InstanceTraits { namespace fl
{
    class Math;
}}

namespace Classes { namespace fl
{
    class Math;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl
{
    class Math : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Math"; }
#endif
    public:
        typedef Classes::fl::Math ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Math(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 8 };
        static const MemberInfo mi[MemberInfoNum];
        enum { ThunkInfoNum = 18 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[39];
        static const Abc::ConstValue dva[6];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl
{
    class Math : public Class
    {
        friend class ClassTraits::fl::Math;
        static const TypeInfo& GetTypeInfo() { return AS3::fl::MathTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::Math"; }
#endif
    public:
        typedef Math SelfType;
        typedef Math ClassType;
        
    private:
        Math(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
#undef max
#undef min

        virtual void Call(const Value& _this, Value& result, unsigned argc, const Value* const argv);
        virtual void Construct(Value& _this, unsigned argc, const Value* argv, bool extCall = false);
//##protect##"class_$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_abs, 
            mid_acos, 
            mid_asin, 
            mid_atan, 
            mid_atan2, 
            mid_ceil, 
            mid_cos, 
            mid_exp, 
            mid_floor, 
            mid_log, 
            mid_round, 
            mid_sin, 
            mid_sqrt, 
            mid_tan, 
            mid_pow, 
            mid_max, 
            mid_min, 
            mid_random, 
        };
        void abs(Value::Number& result, Value::Number x);
        void acos(Value::Number& result, Value::Number x);
        void asin(Value::Number& result, Value::Number x);
        void atan(Value::Number& result, Value::Number x);
        void atan2(Value::Number& result, Value::Number y, Value::Number x);
        void ceil(Value::Number& result, Value::Number x);
        void cos(Value::Number& result, Value::Number x);
        void exp(Value::Number& result, Value::Number x);
        void floor(Value::Number& result, Value::Number x);
        void log(Value::Number& result, Value::Number x);
        void round(Value::Number& result, Value::Number x);
        void sin(Value::Number& result, Value::Number x);
        void sqrt(Value::Number& result, Value::Number x);
        void tan(Value::Number& result, Value::Number x);
        void pow(Value::Number& result, Value::Number x, Value::Number y);
        void max(Value& result, unsigned argc, const Value* const argv);
        void min(Value& result, unsigned argc, const Value* const argv);
        void random(Value::Number& result);

        // C++ friendly wrappers for AS3 methods.
        Value::Number abs(Value::Number x)
        {
            Value::Number result;
            abs(result, x);
            return result;
        }
        Value::Number acos(Value::Number x)
        {
            Value::Number result;
            acos(result, x);
            return result;
        }
        Value::Number asin(Value::Number x)
        {
            Value::Number result;
            asin(result, x);
            return result;
        }
        Value::Number atan(Value::Number x)
        {
            Value::Number result;
            atan(result, x);
            return result;
        }
        Value::Number atan2(Value::Number y, Value::Number x)
        {
            Value::Number result;
            atan2(result, y, x);
            return result;
        }
        Value::Number ceil(Value::Number x)
        {
            Value::Number result;
            ceil(result, x);
            return result;
        }
        Value::Number cos(Value::Number x)
        {
            Value::Number result;
            cos(result, x);
            return result;
        }
        Value::Number exp(Value::Number x)
        {
            Value::Number result;
            exp(result, x);
            return result;
        }
        Value::Number floor(Value::Number x)
        {
            Value::Number result;
            floor(result, x);
            return result;
        }
        Value::Number log(Value::Number x)
        {
            Value::Number result;
            log(result, x);
            return result;
        }
        Value::Number round(Value::Number x)
        {
            Value::Number result;
            round(result, x);
            return result;
        }
        Value::Number sin(Value::Number x)
        {
            Value::Number result;
            sin(result, x);
            return result;
        }
        Value::Number sqrt(Value::Number x)
        {
            Value::Number result;
            sqrt(result, x);
            return result;
        }
        Value::Number tan(Value::Number x)
        {
            Value::Number result;
            tan(result, x);
            return result;
        }
        Value::Number pow(Value::Number x, Value::Number y)
        {
            Value::Number result;
            pow(result, x, y);
            return result;
        }
        Value::Number random()
        {
            Value::Number result;
            random(result);
            return result;
        }

    public:
        // AS3 API members.
        const Value::Number LN10;
        const Value::Number E;
        const Value::Number LN2;
        const Value::Number LOG10E;
        const Value::Number LOG2E;
        const Value::Number PI;
        const Value::Number SQRT1_2;
        const Value::Number SQRT2;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

