//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Vec_Vector_int.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Vec_Vector_int_H
#define INC_AS3_Obj_Vec_Vector_int_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
#include "../AS3_Obj_int.h"
#include "AS3_Obj_Vec_VectorBase.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_vec
{
    extern const TypeInfo Vector_intTI;
    extern const ClassInfo Vector_intCI;
} // namespace fl_vec
namespace fl
{
    extern const TypeInfo uintTI;
    extern const ClassInfo uintCI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo FunctionTI;
    extern const ClassInfo FunctionCI;
    extern const TypeInfo ObjectTI;
    extern const ClassInfo ObjectCI;
    extern const TypeInfo anyTI;
    extern const ClassInfo anyCI;
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
} // namespace fl

namespace ClassTraits { namespace fl_vec
{
    class Vector_int;
}}

namespace InstanceTraits { namespace fl_vec
{
    class Vector_int;
}}

namespace Classes { namespace fl_vec
{
    class Vector_int;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_vec
{
    class Vector_int : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_vec::Vector_int;
        
    public:
        typedef Vector_int SelfType;
        typedef Classes::fl_vec::Vector_int ClassType;
        typedef InstanceTraits::fl_vec::Vector_int TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_vec::Vector_intTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_vec::Vector_int"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        Vector_int(InstanceTraits::Traits& t);

//##protect##"instance$methods"
    public:
        Vector_int(InstanceTraits::Traits& t, UInt32 length, bool fixed);

        void Append(const SelfType& other) { V.Append(other.V); }
        void Append(unsigned argc, const Value* const argv) { V.Append(argc, argv, GetEnclosedClassTraits()); }
        void Append(const Instances::fl::Array& arr) { V.Append(arr, GetEnclosedClassTraits()); }
        void Append(const ArrayDH<SInt32>& arr) { V.Append(arr); }

        void AppendCoerce(const Instances::fl::Array& arr) { V.AppendCoerce(arr, GetEnclosedClassTraits()); }
        bool AppendCoerce(const Value& v) { return V.AppendCoerce(v, GetEnclosedClassTraits()); }

        void PushBack(SInt32 v) { V.PushBack(v); }

        const ClassTraits::Traits& GetEnclosedClassTraits() const { return GetVM().GetClassTraitsSInt(); }
        const ArrayBase& GetArrayBase() const { return V; }
        const VectorBase<SInt32>& GetArray() const { return V; }

    public:
        virtual void AS3Constructor(unsigned argc, const Value* argv);

    public:
        AbsoluteIndex GetNextArrayIndex(AbsoluteIndex ind) const { return V.GetNextArrayIndex(ind); }
        void GetNextValue(Value& value, GlobalSlotIndex ind) const { V.GetNextValue(value, ind); }

        void SetUnsafe(UInt32 ind, SInt32 v) { V.SetUnsafeT(ind, v); }
        CheckResult Set(UInt32 ind, const Value& v, const ClassTraits::Traits& tr) { return V.Set(ind, v, tr); }
        void Get(UInt32 ind, Value& v) const { V.Get(ind, v); }
        CheckResult RemoveAt(UInt32 ind) { return V.RemoveAt(ind); }

    public:
        virtual void GetNextPropertyName(Value& name, GlobalSlotIndex ind) const;
        virtual GlobalSlotIndex GetNextDynPropIndex(GlobalSlotIndex ind) const;

        virtual CheckResult SetProperty(const Multiname& prop_name, const Value& value);
        virtual CheckResult GetProperty(const Multiname& prop_name, Value& value);
        virtual void GetDynamicProperty(AbsoluteIndex ind, Value& value);
        virtual CheckResult DeleteProperty(const Multiname& prop_name);
        virtual bool HasProperty(const Multiname& prop_name, bool check_prototype);

//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_lengthGet, 
            mid_lengthSet, 
            mid_fixedSet, 
            mid_fixedGet, 
            mid_AS3toString, 
            mid_AS3toLocaleString, 
            mid_AS3join, 
            mid_AS3every, 
            mid_AS3forEach, 
            mid_AS3map, 
            mid_AS3push, 
            mid_AS3some, 
            mid_AS3unshift, 
            mid_AS3concat, 
            mid_AS3filter, 
            mid_AS3pop, 
            mid_AS3reverse, 
            mid_AS3shift, 
            mid_AS3slice, 
            mid_AS3sort, 
            mid_AS3splice, 
            mid_AS3indexOf, 
            mid_AS3lastIndexOf, 
        };
        void lengthGet(UInt32& result);
        void lengthSet(const Value& result, UInt32 value);
        void fixedSet(const Value& result, bool f);
        void fixedGet(bool& result);
        void AS3toString(ASString& result);
        void AS3toLocaleString(ASString& result);
        void AS3join(ASString& result, const ASString& separator);
        void AS3every(bool& result, const Value& checker, const Value& thisObj = Value::GetNull());
        void AS3forEach(const Value& result, const Value& eacher, const Value& thisObj = Value::GetNull());
        void AS3map(SPtr<Instances::fl_vec::Vector_int>& result, const Value& mapper, const Value& thisObj = Value::GetNull());
        void AS3push(Value& result, unsigned argc, const Value* const argv);
        void AS3some(bool& result, const Value& checker, const Value& thisObj = Value::GetNull());
        void AS3unshift(Value& result, unsigned argc, const Value* const argv);
        void AS3concat(Value& result, unsigned argc, const Value* const argv);
        void AS3filter(SPtr<Instances::fl_vec::Vector_int>& result, const Value& checker, const Value& thisObj = Value::GetNull());
        void AS3pop(SInt32& result);
        void AS3reverse(SPtr<Instances::fl_vec::Vector_int>& result);
        void AS3shift(SInt32& result);
        void AS3slice(Value& result, unsigned argc, const Value* const argv);
        void AS3sort(SPtr<Instances::fl_vec::Vector_int>& result, const Value& comparefn);
        void AS3splice(Value& result, unsigned argc, const Value* const argv);
        void AS3indexOf(SInt32& result, SInt32 value, SInt32 from = 0);
        void AS3lastIndexOf(SInt32& result, SInt32 value, SInt32 from = 0x7fffffff);

        // C++ friendly wrappers for AS3 methods.
        UInt32 lengthGet()
        {
            UInt32 result;
            lengthGet(result);
            return result;
        }
        void lengthSet(UInt32 value)
        {
            lengthSet(Value::GetUndefined(), value);
        }
        void fixedSet(bool f)
        {
            fixedSet(Value::GetUndefined(), f);
        }
        bool fixedGet()
        {
            bool result;
            fixedGet(result);
            return result;
        }
        ASString AS3toString()
        {
            ASString result(GetStringManager().CreateEmptyString());
            AS3toString(result);
            return result;
        }
        ASString AS3toLocaleString()
        {
            ASString result(GetStringManager().CreateEmptyString());
            AS3toLocaleString(result);
            return result;
        }
        ASString AS3join(const ASString& separator)
        {
            ASString result(GetStringManager().CreateEmptyString());
            AS3join(result, separator);
            return result;
        }
        bool AS3every(const Value& checker, const Value& thisObj = Value::GetNull())
        {
            bool result;
            AS3every(result, checker, thisObj);
            return result;
        }
        void AS3forEach(const Value& eacher, const Value& thisObj = Value::GetNull())
        {
            AS3forEach(Value::GetUndefined(), eacher, thisObj);
        }
        SPtr<Instances::fl_vec::Vector_int> AS3map(const Value& mapper, const Value& thisObj = Value::GetNull())
        {
            SPtr<Instances::fl_vec::Vector_int> result;
            AS3map(result, mapper, thisObj);
            return result;
        }
        bool AS3some(const Value& checker, const Value& thisObj = Value::GetNull())
        {
            bool result;
            AS3some(result, checker, thisObj);
            return result;
        }
        SPtr<Instances::fl_vec::Vector_int> AS3filter(const Value& checker, const Value& thisObj = Value::GetNull())
        {
            SPtr<Instances::fl_vec::Vector_int> result;
            AS3filter(result, checker, thisObj);
            return result;
        }
        SInt32 AS3pop()
        {
            SInt32 result;
            AS3pop(result);
            return result;
        }
        SPtr<Instances::fl_vec::Vector_int> AS3reverse()
        {
            SPtr<Instances::fl_vec::Vector_int> result;
            AS3reverse(result);
            return result;
        }
        SInt32 AS3shift()
        {
            SInt32 result;
            AS3shift(result);
            return result;
        }
        SPtr<Instances::fl_vec::Vector_int> AS3sort(const Value& comparefn)
        {
            SPtr<Instances::fl_vec::Vector_int> result;
            AS3sort(result, comparefn);
            return result;
        }
        SInt32 AS3indexOf(SInt32 value, SInt32 from = 0)
        {
            SInt32 result;
            AS3indexOf(result, value, from);
            return result;
        }
        SInt32 AS3lastIndexOf(SInt32 value, SInt32 from = 0x7fffffff)
        {
            SInt32 result;
            AS3lastIndexOf(result, value, from);
            return result;
        }

//##protect##"instance$data"
    private:
        VectorBase<SInt32> V;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_vec
{
    class Vector_int : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::Vector_int"; }
#endif
    public:
        typedef Instances::fl_vec::Vector_int InstanceType;

    public:
        Vector_int(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_vec::Vector_int> MakeInstance(Vector_int& t)
        {
            return Pickable<Instances::fl_vec::Vector_int>(new(t.Alloc()) Instances::fl_vec::Vector_int(t));
        }
        SPtr<Instances::fl_vec::Vector_int> MakeInstanceS(Vector_int& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 23 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[43];
        static const Abc::ConstValue dva[2];
//##protect##"instance_traits$methods"
        static Pickable<Instances::fl_vec::Vector_int> MakeInstance(Vector_int& t, UInt32 length, bool fixed)
        {
            return Pickable<Instances::fl_vec::Vector_int>(new(t.Alloc()) Instances::fl_vec::Vector_int(t, length, fixed));
        }
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_vec
{
    class Vector_int : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Vector_int"; }
#endif
    public:
        typedef Classes::fl_vec::Vector_int ClassType;
        typedef InstanceTraits::fl_vec::Vector_int InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Vector_int(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
        Pickable<Instances::fl_vec::Vector_int> MakeInstance() const
        {
            InstanceTraits::fl_vec::Vector_int& itr = static_cast<InstanceTraits::fl_vec::Vector_int&>(GetInstanceTraits());
            return itr.MakeInstance(itr);
        }
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_vec
{
    class Vector_int : public Class
    {
        friend class ClassTraits::fl_vec::Vector_int;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_vec::Vector_intTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::Vector_int"; }
#endif
    public:
        typedef Vector_int SelfType;
        typedef Vector_int ClassType;
        
    private:
        Vector_int(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
        virtual void Call(const Value& _this, Value& result, unsigned argc, const Value* const argv);
//##protect##"class_$methods"

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

