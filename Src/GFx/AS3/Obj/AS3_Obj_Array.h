//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Array.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Array_H
#define INC_AS3_Obj_Array_H

#include "AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl
{
    extern const TypeInfo ArrayTI;
    extern const ClassInfo ArrayCI;
    extern const TypeInfo uintTI;
    extern const ClassInfo uintCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
    extern const TypeInfo ObjectTI;
    extern const ClassInfo ObjectCI;
    extern const TypeInfo anyTI;
    extern const ClassInfo anyCI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo FunctionTI;
    extern const ClassInfo FunctionCI;
} // namespace fl

namespace ClassTraits { namespace fl
{
    class Array;
}}

namespace InstanceTraits { namespace fl
{
    class Array;
}}

namespace Classes { namespace fl
{
    class Array;
}}

//##protect##"forward_declaration"

class ValueStack;

///////////////////////////////////////////////////////////////////////////////
namespace Impl
{

///////////////////////////////////////////////////////////////////////////////
template <typename T1, typename T2, typename T3>
struct Triple : public Pair<T1, T2>
{
    typedef T3 ThirdType;

    Triple(const T1& f, const T2& s, const T3& t) : Pair<T1, T2>(f, s), Third(t) {}

    T3 Third;
};

///////////////////////////////////////////////////////////////////////////////
typedef Triple<ASString, const Value*, UInt32> StringValueTriple;
typedef Triple<Value::Number, const Value*, UInt32> NumberValueTriple;
typedef Pair<const Value*, UInt32> ValuePtrPair;

///////////////////////////////////////////////////////////////////////////////
class CompareAsNumber
{
public:
    typedef NumberValueTriple DataType;
    typedef DataType::FirstType KeyType;

    CompareAsNumber(bool desc) : Desc(desc) {}

public:
    bool operator ()(const DataType& a, const DataType& b) const
    {
        if (Desc)
            return b.First < a.First;

        return a.First < b.First;
    }
    bool Equal(const DataType& a, const DataType& b) const
    {
        return a.First == b.First;
    }

private:
    CompareAsNumber& operator = (const CompareAsNumber&);

private:
    const bool Desc;
};

///////////////////////////////////////////////////////////////////////////////
class CompareAsString
{
public:
    typedef StringValueTriple DataType;
    typedef DataType::FirstType KeyType;

    CompareAsString(bool desc, bool ci, bool use_locale) 
        : Desc(desc)
        , CaseInsensitive(ci) 
        , UseLocale(use_locale)
    {

    }

public:
    bool operator ()(const DataType& a, const DataType& b) const
    {
        return Compare(a.First, b.First) < 0;
    }
    bool Equal(const DataType& a, const DataType& b) const
    {
        return Compare(a.First, b.First) == 0;
    }

private:
    CompareAsString& operator = (const CompareAsString&);

    int Compare(const ASString& a, const ASString& b) const;

private:
    const bool Desc;
    const bool CaseInsensitive;
    const bool UseLocale;
};

///////////////////////////////////////////////////////////////////////////////
class CompareValuePtr
{
public:
    typedef ValuePtrPair DataType;
    typedef DataType KeyType;

    CompareValuePtr(VM& vm, const Value& f, bool desc) : Desc(desc), Vm(vm), Func(f) {}

public:
    bool operator ()(const DataType& a, const DataType& b) const
    {
        if (Desc)
            return Compare(*b.First, *a.First) < 0;

        return Compare(*a.First, *b.First) < 0;
    }
    bool Equal(const DataType& a, const DataType& b) const
    {
        return Compare(*a.First, *b.First) == 0;
    }

public:
    VM& GetVM() const { return Vm; }

private:
    CompareValuePtr& operator = (const CompareValuePtr&);

    int Compare(const Value& a, const Value& b) const;

private:
    const bool Desc;
    VM& Vm;
    const Value& Func;
};

///////////////////////////////////////////////////////////////////////////////
class CompareOn
{
public:
    typedef ValuePtrPair DataType;
    typedef DataType KeyType;

    CompareOn(
        VM& vm, 
        const ValueArrayDH& fields,
        const ArrayDH<UInt32>& flags
        ) 
        : Vm(vm)
        , Fields(fields)
        , Flags(flags)
    {
    }

public:
    // Operator *less*.
    bool operator ()(const DataType& a, const DataType& b) const
    {
        return Compare(*a.First, *b.First) < 0;
    }
    bool Equal(const DataType& a, const DataType& b) const
    {
        return Compare(*a.First, *b.First) == 0;
    }

public:
    VM& GetVM() const { return Vm; }

private:
    Value::Number Compare(const Value& a, const Value& b) const;

private:
    CompareOn& operator = (const CompareOn&);

private:
    VM& Vm;
    const ValueArrayDH& Fields;
    const ArrayDH<UInt32>& Flags;
};


///////////////////////////////////////////////////////////////////////////////
class ArrayFunc
{
public:
    virtual ~ArrayFunc() {}

public:
    virtual void operator()(UInt32 ind, const Value& v) = 0;
};

///////////////////////////////////////////////////////////////////////////////
class SparseArray
{
public:
    SparseArray(MemoryHeap* heap);

    SparseArray& operator = (const SparseArray& other)
    {
        // Assign() will do self-assignment check.
        Assign(other);
        return *this;
    }

public:
    enum OP {opCut, opRemove};

    // Append to the end.
    // Add n "undefined" values.
    void AppendN(UPInt n);
    void Append(const ValueArrayDH& x);
    // Append *num* values starting from index *ind*.
    void Append(const ValueArray& x, UPInt ind, UPInt num);
    void Append(const SparseArray& other, UPInt ind, UPInt num);
    void Pick(ValueStack& x, UPInt num);
    void Append(unsigned argc, const Value* argv);

    void Assign(const SparseArray& other);

    void Resize(UPInt newLength);
    void PushBack(const Value& v);

    // Insert at position *pos*
    void Insert(UPInt pos, unsigned argc, const Value* argv);
    // Remove creates gaps.
    void RemoveMultipleAt(UPInt pos, UPInt count, OP op);
    // Cut shifts elements.
    void CutMultipleAt(UPInt pos, UPInt count, SparseArray* deleted = NULL);

    const Value& At(UPInt ind) const;
    const Value& Top() const { return At(GetSize() - 1); }

    void Set(UPInt ind, const Value& v);
    void RemoveAt(UPInt pos) { RemoveMultipleAt(pos, 1, opRemove); }
    void Pop() { if (GetSize()) RemoveMultipleAt(GetSize() - 1, 1, opCut); }
    void Reverse();
    void ShiftLeft();

    UPInt GetSize() const { return Length; }
    // Array is sparse if not all elements fit into ValueA.
    bool  IsSparse() const { return Length > ValueA.GetSize(); }

    AbsoluteIndex GetNextArrayIndex(AbsoluteIndex ind) const;

    const ValueArrayDH& GetContiguousPart() const { return ValueA; }

    // Optimized iteration.
    // Visit all elements.
    void ForEach(ArrayFunc& f) const;
    // Visit only dense part.
    void ForEachDense(ArrayFunc& f) const;
    // Visit only sparse part.
    void ForEachSparse(ArrayFunc& f) const;

public:
    typedef RefCountCollector<Mem_Stat> Collector;
    void ForEachChild_GC(Collector* prcc, RefCountBaseGC<Mem_Stat>::GcOp op
                         SF_DEBUG_ARG(const RefCountBaseGC<Mem_Stat>& owner)) const;

private:
    void MoveHashRight(UPInt first_ind, UPInt offset);
    // CutHash shifts elements.
    void CutHash(UPInt pos, UPInt del_num, SparseArray* deleted = NULL);
    void RemoveHash(UPInt first_ind, UPInt num);
    // Try to move data from hash table into array.
    void Optimize();
    // Try to move ValueHLowInd.
    void AdjustValueHLowInd();

    UPInt GetValueHLowInd() const 
    { 
        return ValueHLowInd; 
    }
    UPInt GetValueHHighInd() const 
    { 
        return ValueHHighInd; 
    }
    // Get left or equal index.
    UPInt GetLeftEqualInd(UPInt start_from) const;
    // Get right or equal index.
    UPInt GetRightEqualInd(UPInt start_from, UPInt end_ind) const;

private:
    typedef HashDH<UPInt, Value> ValueHashDH;

    UPInt           Length;
    UPInt           ValueHLowInd;
    UPInt           ValueHHighInd;

    const Value     DefaultValue;
    ValueArrayDH    ValueA;
    // Indices in ValueH are relative to the end of ValueA.
    ValueHashDH     ValueH;
}; // SparseArray

///////////////////////////////////////////////////////////////////////////////
class ValuePtrCollector : public ArrayFunc
{
public:
    ValuePtrCollector(ArrayDH<Impl::ValuePtrPair>& p) : Pairs(p) {}

public:
    virtual void operator()(UInt32 ind, const Value& v);

private:
    ValuePtrCollector& operator =(const ValuePtrCollector&);

private:
    ArrayDH<Impl::ValuePtrPair>& Pairs;
};

///////////////////////////////////////////////////////////////////////////////
class Value2StrCollector : public ArrayFunc
{
public:
    Value2StrCollector(VM& vm, ArrayDH<Impl::StringValueTriple>& p)
        : Vm(vm)
        , Pairs(p)
    {
    }

public:
    virtual void operator()(UInt32 ind, const Value& v);

private:
    Value2StrCollector& operator =(const Value2StrCollector&);

    VM& GetVM()
    {
        return Vm;
    }

private:
    VM& Vm;
    ArrayDH<Impl::StringValueTriple>& Pairs;
};

///////////////////////////////////////////////////////////////////////////////
class Value2NumberCollector : public ArrayFunc
{
public:
    Value2NumberCollector(ArrayDH<Impl::NumberValueTriple>& p)
        : Pairs(p)
    {
    }

public:
    virtual void operator()(UInt32 ind, const Value& v);

private:
    Value2NumberCollector& operator =(const Value2NumberCollector&);

private:
    ArrayDH<Impl::NumberValueTriple>& Pairs;
};

} // namespace Impl

//##protect##"forward_declaration"

namespace Instances { namespace fl
{
    class Array : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl::Array;
        
    public:
        typedef Array SelfType;
        typedef Classes::fl::Array ClassType;
        typedef InstanceTraits::fl::Array TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl::ArrayTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl::Array"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        Array(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        friend class Classes::fl::Array;

    public:
        SF_INLINE Instances::fl::Array& operator = (const Instances::fl::Array& other)
        {
            Assign(other);
            return *this;
        }

    public:
        enum SortFlags
        {
            SortFlags_CaseInsensitive    = 1,
            SortFlags_Descending         = 2, 
            SortFlags_UniqueSort         = 4, 
            SortFlags_ReturnIndexedArray = 8,
            SortFlags_Numeric            = 16,

            // extension
            SortFlags_Locale             = 1024
        };

        SF_INLINE void Resize(UPInt newLength) { SA.Resize(newLength); }
        SF_INLINE void PushBack(const Value& v) { SA.PushBack(v); }
        // Add n "undefined" values.
        SF_INLINE void AppendN(UPInt n) { SA.AppendN(n); }
        SF_INLINE void Append(const ValueArrayDH& x) { SA.Append(x); }
        // Append *num* values starting from index *ind*.
        SF_INLINE void Append(const ValueArray& x, UPInt ind, UPInt num) { SA.Append(x, ind, num); }
        SF_INLINE void Append(const Impl::SparseArray& other, UPInt ind, UPInt num) { SA.Append(other, ind, num); }
        SF_INLINE void Append(const Instances::fl::Array& other, UPInt ind, UPInt num) { SA.Append(other.SA, ind, num); }
        SF_INLINE void Append(const Instances::fl::Array& other) { Append(other, 0, other.GetSize()); }
        SF_INLINE void Pick(ValueStack& x, UPInt num) { SA.Pick(x, num); }
        // Append to the end.
        SF_INLINE void Append(unsigned argc, const Value* argv) { SA.Append(argc, argv); }

        SF_INLINE void Assign(const Instances::fl::Array& other)
        { 
            if (this != &other)
                SA.Assign(other.SA); 
        }
        // Insert at position *pos*
        SF_INLINE void Insert(UPInt pos, unsigned argc, const Value* argv) { SA.Insert(pos, argc, argv); }
        // Remove creates gaps.
        SF_INLINE void RemoveMultipleAt(UPInt pos, UPInt count) { SA.RemoveMultipleAt(pos, count, Impl::SparseArray::opRemove); }
        // Cut shifts elements.
        void CutMultipleAt(UPInt pos, UPInt count, Instances::fl::Array* deleted = NULL) { SA.CutMultipleAt(pos, count, deleted ? &deleted->SA : NULL); }

        SF_INLINE const Value& At(UPInt ind) const { return SA.At(ind); }
        SF_INLINE const Value& Top() const { return SA.Top(); }

        SF_INLINE void Set(UPInt ind, const Value& v) { SA.Set(ind, v); }
        SF_INLINE void RemoveAt(UPInt pos) { SA.RemoveAt(pos); }
        SF_INLINE void Pop() { SA.Pop(); }

        SF_INLINE void Reverse() { SA.Reverse(); }
        SF_INLINE void ShiftLeft() { SA.ShiftLeft(); }

        SF_INLINE bool  IsSparse() const { return SA.IsSparse(); }
        SF_INLINE UPInt GetSize() const { return SA.GetSize(); }
        SF_INLINE AbsoluteIndex GetNextArrayIndex(AbsoluteIndex ind) const { return SA.GetNextArrayIndex(ind); }

        SF_INLINE const ValueArrayDH& GetContiguousPart() const { return SA.GetContiguousPart(); }

        // Visit only dense part.
        SF_INLINE void ForEachDense(Impl::ArrayFunc& f) const { SA.ForEachDense(f); }
        // Visit only sparse part.
        SF_INLINE void ForEachSparse(Impl::ArrayFunc& f) const { SA.ForEachSparse(f); }

    protected:
        virtual void ForEachChild_GC(Collector* prcc, GcOp op) const;
        virtual void AS3Constructor(unsigned argc, const Value* argv);

        ASString ToStringInternal(const ASString& sep) const;
        ASString ToLocaleStringInternal() const;

        /*
        template<class SortFunctor>
        void Sort(SortFunctor& sf)
        {
            if (GetValue().GetSize() > 0)
            {
                Alg::ArrayAdaptor<Value*> a(&ValueA[0], (unsigned)GetValue().GetSize());
                Alg::QuickSort(a, sf);
            }
        }
        */
            
    protected:
//         void JoinToString(StringBuffer* pbuffer, const char* pDelimiter) const;

        // Deal with prototypes manually.
        static void toLocaleStringProto(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);
        static void toStringProto(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);

    public:
        virtual void GetNextPropertyName(Value& name, GlobalSlotIndex ind) const;
        virtual GlobalSlotIndex GetNextDynPropIndex(GlobalSlotIndex ind) const;
        virtual void GetNextPropertyValue(Value& value, GlobalSlotIndex ind);

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
            mid_AS3join, 
            mid_AS3pop, 
            mid_AS3push, 
            mid_AS3reverse, 
            mid_AS3concat, 
            mid_AS3shift, 
            mid_AS3slice, 
            mid_AS3unshift, 
            mid_AS3splice, 
            mid_AS3sort, 
            mid_AS3sortOn, 
            mid_AS3indexOf, 
            mid_AS3lastIndexOf, 
            mid_AS3every, 
            mid_AS3filter, 
            mid_AS3forEach, 
            mid_AS3map, 
            mid_AS3some, 
        };
        void lengthGet(UInt32& result);
        void lengthSet(const Value& result, UInt32 newLength);
        void AS3join(Value& result, unsigned argc, const Value* const argv);
        void AS3pop(Value& result);
        void AS3push(Value& result, unsigned argc, const Value* const argv);
        void AS3reverse(SPtr<Instances::fl::Array>& result);
        void AS3concat(Value& result, unsigned argc, const Value* const argv);
        void AS3shift(Value& result);
        void AS3slice(SPtr<Instances::fl::Array>& result, SInt32 startIndex = 0, SInt32 endIndex = 16777215);
        void AS3unshift(Value& result, unsigned argc, const Value* const argv);
        void AS3splice(Value& result, unsigned argc, const Value* const argv);
        void AS3sort(Value& result, unsigned argc, const Value* const argv);
        void AS3sortOn(SPtr<Instances::fl::Array>& result, const Value& fieldName, const Value& options = Value::GetNull());
        void AS3indexOf(SInt32& result, const Value& searchElement, SInt32 fromIndex = 0);
        void AS3lastIndexOf(SInt32& result, const Value& searchElement, SInt32 fromIndex = 0x7fffffff);
        void AS3every(bool& result, const Value& callback, const Value& thisObject = Value::GetNull());
        void AS3filter(SPtr<Instances::fl::Array>& result, const Value& callback, const Value& thisObject = Value::GetNull());
        void AS3forEach(const Value& result, const Value& callback, const Value& thisObject = Value::GetNull());
        void AS3map(SPtr<Instances::fl::Array>& result, const Value& callback, const Value& thisObject = Value::GetNull());
        void AS3some(bool& result, const Value& callback, const Value& thisObject = Value::GetNull());

        // C++ friendly wrappers for AS3 methods.
        UInt32 lengthGet()
        {
            UInt32 result;
            lengthGet(result);
            return result;
        }
        void lengthSet(UInt32 newLength)
        {
            lengthSet(Value::GetUndefined(), newLength);
        }
        Value AS3pop()
        {
            Value result;
            AS3pop(result);
            return result;
        }
        SPtr<Instances::fl::Array> AS3reverse()
        {
            SPtr<Instances::fl::Array> result;
            AS3reverse(result);
            return result;
        }
        Value AS3shift()
        {
            Value result;
            AS3shift(result);
            return result;
        }
        SPtr<Instances::fl::Array> AS3slice(SInt32 startIndex = 0, SInt32 endIndex = 16777215)
        {
            SPtr<Instances::fl::Array> result;
            AS3slice(result, startIndex, endIndex);
            return result;
        }
        SPtr<Instances::fl::Array> AS3sortOn(const Value& fieldName, const Value& options = Value::GetNull())
        {
            SPtr<Instances::fl::Array> result;
            AS3sortOn(result, fieldName, options);
            return result;
        }
        SInt32 AS3indexOf(const Value& searchElement, SInt32 fromIndex = 0)
        {
            SInt32 result;
            AS3indexOf(result, searchElement, fromIndex);
            return result;
        }
        SInt32 AS3lastIndexOf(const Value& searchElement, SInt32 fromIndex = 0x7fffffff)
        {
            SInt32 result;
            AS3lastIndexOf(result, searchElement, fromIndex);
            return result;
        }
        bool AS3every(const Value& callback, const Value& thisObject = Value::GetNull())
        {
            bool result;
            AS3every(result, callback, thisObject);
            return result;
        }
        SPtr<Instances::fl::Array> AS3filter(const Value& callback, const Value& thisObject = Value::GetNull())
        {
            SPtr<Instances::fl::Array> result;
            AS3filter(result, callback, thisObject);
            return result;
        }
        void AS3forEach(const Value& callback, const Value& thisObject = Value::GetNull())
        {
            AS3forEach(Value::GetUndefined(), callback, thisObject);
        }
        SPtr<Instances::fl::Array> AS3map(const Value& callback, const Value& thisObject = Value::GetNull())
        {
            SPtr<Instances::fl::Array> result;
            AS3map(result, callback, thisObject);
            return result;
        }
        bool AS3some(const Value& callback, const Value& thisObject = Value::GetNull())
        {
            bool result;
            AS3some(result, callback, thisObject);
            return result;
        }

//##protect##"instance$data"
    private:
        Impl::SparseArray SA;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl
{
    class Array : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::Array"; }
#endif
    public:
        typedef Instances::fl::Array InstanceType;

    public:
        Array(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl::Array> MakeInstance(Array& t)
        {
            return Pickable<Instances::fl::Array>(new(t.Alloc()) Instances::fl::Array(t));
        }
        SPtr<Instances::fl::Array> MakeInstanceS(Array& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 20 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[39];
        static const Abc::ConstValue dva[7];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl
{
    class Array : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Array"; }
#endif
    public:
        typedef Classes::fl::Array ClassType;
        typedef InstanceTraits::fl::Array InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Array(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 5 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
#if 0 // This Coerce() seems to cause problems.
        virtual bool Coerce(const Value& value, Value& result) const;
#endif
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl
{
    class Array : public Class
    {
        friend class ClassTraits::fl::Array;
        static const TypeInfo& GetTypeInfo() { return AS3::fl::ArrayTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::Array"; }
#endif
    public:
        typedef Array SelfType;
        typedef Array ClassType;
        
    private:
        Array(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
        virtual void Call(const Value& _this, Value& result, unsigned argc, const Value* const argv);
        virtual Pickable<AS3::Object> MakePrototype() const;
        virtual void InitPrototype(AS3::Object& obj) const;       
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const UInt32 CASEINSENSITIVE;
        const UInt32 DESCENDING;
        const UInt32 UNIQUESORT;
        const UInt32 RETURNINDEXEDARRAY;
        const UInt32 NUMERIC;

//##protect##"class_$data"
        static const TypeInfo* tit[2];
        static const ThunkInfo ti[2];
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

