/**************************************************************************

Filename    :   AS3_Obj_Function.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_AS3_Obj_Function_H
#define INC_AS3_Obj_Function_H

#include "../AS3_Object.h"
#include "AS3_Obj_Object.h"

namespace Scaleform { namespace GFx { namespace AS3 
{

// Forward declaration.
class VMAbcFile;    
class VTable;

namespace fl
{
    extern const TypeInfo FunctionTI;
    extern const TypeInfo FunctionTICpp;
    extern const TypeInfo FunctionTIThunk;
    extern const ClassInfo FunctionCI;
    extern const ClassInfo FunctionCICpp;
    extern const ClassInfo FunctionCIThunk;
}

namespace InstanceTraits
{
    class Function;
    class ThunkFunction;
}

namespace Classes
{
    class Function;
} // namespace Classes

namespace Instances
{
    //////////////////////////////////////////////////////////////////////////
    class FunctionBase : public Instances::fl::Object
    //class FunctionBase : public Class
    {
        friend class InstanceTraits::Function; // Because of *Prototype* ...

    public:
        typedef FunctionBase SelfType;
        typedef Classes::Function ClassType;
        
    protected:
        FunctionBase(InstanceTraits::Traits& t);
        virtual ~FunctionBase();

    public:
        static void toStringProto(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);

        // *argv* does not include *this* ...
        virtual void ExecuteUnsafe(const Value& _this, Value& result, unsigned argc, const Value* argv);
        virtual void Execute(const Value& _this, unsigned argc, const Value* argv, bool discard_result = false);

        // Apply is similar to Call(), except arguments are passed as an Array object.
        virtual void Call(const Value& _this, Value& result, unsigned argc, const Value* const argv);
        virtual void Construct(Value& result, unsigned argc, const Value* argv, bool extCall = false);

#if 0 // We do not need this method because Prototype is registered with Traits and will be visited automatically.
        virtual void ForEachChild_GC(Collector* prcc, GcOp op) const;
#endif

        virtual UInt32 GetMaxParamNum() const;

    public:
        // AS3 methods.
        void lengthGet(UInt32& result);
        static void call(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);
        static void apply(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);

    protected:
        void PushResult(Value& value) const;
        void PickPushResult(Value& value) const;
        void RetrieveResult(Value& result) const;
        void RetrieveResultUnsafe(Value& result) const;

    protected:
        SPtr<AS3::Object> Prototype; // Function behaves similar to Class. Because of that it has a prototype ...
    };
    
    //////////////////////////////////////////////////////////////////////////
    class FunctionAOT : public FunctionBase
    {
    public:
        FunctionAOT(
            InstanceTraits::Traits& t, 
            const ScopeStackType& ss,
            Instances::fl::GlobalObject& go
            );

    public:
        Instances::fl::GlobalObject& GetGlobalObject() const
        {
            return GO;
        }
        // Get Stored Scope Stack.
        const ScopeStackType& GetStoredScopeStack() const
        {
            return StoredScopeStack;
        }

    public:
        virtual void ForEachChild_GC(Collector* prcc, GcOp op) const;

    private:
        Instances::fl::GlobalObject& GO;
        // We keep a copy of the scope stack ...
        ScopeStackType StoredScopeStack;
    };

    //////////////////////////////////////////////////////////////////////////
    // If function is included in a Virtual Table it stores Origination Traits
    // (Traits to which this Virtual Table belongs)
    class Function : public FunctionBase
    {
        friend class Classes::Function; // Because of constructor.
        friend class AS3::VTable; // Because of StoreScopeStack();

        typedef Function SelfType;
        typedef FunctionBase ParentType;
        
    private:
        Function(
            InstanceTraits::Function& tr,
            const ScopeStackType& ss
            SF_DEBUG_ARG(const ASString& name)
            );
            
    public:
        ~Function();
        
    public:
        virtual void ForEachChild_GC(Collector* prcc, GcOp op) const;
        
    public:
        // *argv* does not include *this* ...
        virtual void Execute(const Value& _this, unsigned argc, const Value* argv, bool discard_result = false);
        // *argv* does not include *this* ...
        void Execute(unsigned argc, const Value* argv, bool discard_result = false)
        {
            Execute(Value(this), argc, argv, discard_result);
        }
        // *argv* does not include *this* ...
        virtual void ExecuteUnsafe(const Value& _this, Value& result, unsigned argc, const Value* argv);

        virtual UInt32 GetMaxParamNum() const;

        const Abc::Multiname& GetReturnType() const;
        // Get Stored Scope Stack.
        const ScopeStackType& GetStoredScopeStack() const
        {
            return StoredScopeStack;
        }
        VMAbcFile& GetFile() const;

        SelfType& GetSelf()
        {
            return *this;
        }
        const InstanceTraits::Function& GetInstanceTraitsFunction() const;
        const Traits& GetOriginationTraits() const;

        SF_DEBUG_CODE(const ASString& GetFunctionName() const { return Name; })

    private:
        void StoreScopeStack(const UPInt baseSSInd, const ScopeStackType& ss);
        void Add2StoredScopeStack(const Value& o)
        {
            StoredScopeStack.PushBack(o);
        }

    private:
        // We keep a copy of the scope stack ...
        ScopeStackType StoredScopeStack;
        SF_DEBUG_CODE(const ASString Name;)
    };

    //////////////////////////////////////////////////////////////////////////
    class ThunkFunction : public FunctionBase
    {
        friend class Classes::Function;

    protected:
        ThunkFunction(
            InstanceTraits::Traits& t,
            const ThunkInfo& thunk
            SF_DEBUG_ARG(const Traits& ot)
            )
            : FunctionBase(t)
            , Thunk(thunk)
            SF_DEBUG_ARG(OriginationTraits(&ot))
        {
        }

    public:
        virtual void ExecuteUnsafe(const Value& _this, Value& result, unsigned argc, const Value* argv);
        virtual void Execute(const Value& _this, unsigned argc, const Value* argv, bool discard_result = false);

        const ThunkInfo& GetThunk() const { return Thunk; }

        SF_DEBUG_CODE(virtual void ForEachChild_GC(Collector* prcc, GcOp op) const;)
        SF_DEBUG_CODE(const Traits& GetOriginationTraits() const  { return *OriginationTraits; })

    public:
        // AS3 methods.
        void length(Value& result, unsigned argc, const Value* argv);

    private:
        // Function itself.
        const ThunkInfo& Thunk;
        // OriginationTraits is where this function came from.
        // OriginationTraits is optional here. We need it just to be able
        // to trace where it came from if it was assigned to an object
        // as a dynamic property. Main case is prototype.
        SF_DEBUG_CODE(const SPtr<const Traits> OriginationTraits;)
    };
    
    //////////////////////////////////////////////////////////////////////////
    // It is a function delegate, which will check type of *this* before a call.
    class CheckTypeTF : public ThunkFunction
    {
        friend class Classes::Function;

    public:
        typedef CheckTypeTF     SelfType;
        typedef ThunkFunction   ParentType;

    protected:
        CheckTypeTF(
            InstanceTraits::ThunkFunction& t,
            const ThunkInfo& thunk,
            const Class& data_type
            SF_DEBUG_ARG(const Traits& ot)
            );

    public:
        virtual void ExecuteUnsafe(const Value& _this, Value& result, unsigned argc, const Value* argv);
        virtual void Execute(const Value& _this, unsigned argc, const Value* argv, bool discard_result = false);

    private:
        const Class& DataTypeClass;
    };

} // namespace Instances

namespace InstanceTraits
{
    // All InstanceTraits related to Function have same Virtual Table.
    // ( apply(), call(), lengthGet() )

    ///////////////////////////////////////////////////////////////////////////
    // It is a SealedObject, what is not absolutely correct,
    // but we cannot do anything about this.
    class Thunk : public CTraits
    {
    public:
        // This doesn't look correct.
        typedef Thunk InstanceType;

    public:
        Thunk(VM& vm);

    public:
        virtual void MakeObject(Value& result, Traits& t);
        ASString GetThunkName(const Value& _this) const;

    public:
        // AS3 methods.
        // toString() belongs to a prototype.
        static void lengthGet(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);

    private:
        static const TypeInfo* tit[5];
        static const ThunkInfo f[3];
    };

    ///////////////////////////////////////////////////////////////////////////
    // It is a SealedObject, what is not absolutely correct,
    // but we cannot do anything about this.
    class MethodInd : public CTraits
    {
    public:
        // Doesn't exist.
        // typedef Instances::??? InstanceType;

    public:
        MethodInd(VM& vm);

    public:
        virtual void MakeObject(Value& result, Traits& t);
        ASString GetMethodIndName(const Value& _this) const;

    public:
        // AS3 methods.
        // toString() belongs to a prototype.
        static void lengthGet(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);

    private:
        static const TypeInfo* tit[5];
        static const ThunkInfo f[3];
    };

    ///////////////////////////////////////////////////////////////////////////
    // It is a SealedObject, what is not absolutely correct,
    // but we cannot do anything about this.
    class VTableInd : public CTraits
    {
    public:
        // Doesn't exist.
        // typedef Instances::??? InstanceType;

    public:
        VTableInd(VM& vm);

    public:
        virtual void MakeObject(Value& result, Traits& t);
        // Not implemented yet.
        //ASString GetName(const Value& _this) const;

    public:
        // AS3 methods.
        // toString() belongs to a prototype.
        static void lengthGet(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);

    private:
        static const TypeInfo* tit[5];
        static const ThunkInfo f[3];
    };

    ///////////////////////////////////////////////////////////////////////////
    // ThunkFunction is Traits for a dynamic object, which will delegate 
    // all calls to Thunk methods. ThunkFunction is supposed to be used as a proxy
    // to check types and do type conversions.
    class ThunkFunction : public CTraits
    {
    public:
        typedef Instances::ThunkFunction InstanceType;

    public:
        ThunkFunction(VM& vm);

    public:
        virtual void MakeObject(Value& result, Traits& t);
        ASString GetThunkName(const Value& _this) const;

    private:
        static const TypeInfo* tit[1];
        static const ThunkInfo f[1];
    };

    ///////////////////////////////////////////////////////////////////////////
    class Function : public CTraits
    {
        friend class Classes::Function; // Because of Function(VM& vm, Class& c).
        friend class ClassTraits::Function; // Because of Function(VM& vm, Class& c).
        friend class AS3::VM; // Because of Function(VM& vm, Class& c).

    public:
        // This is questionable.
        typedef Instances::Function InstanceType;

    public:
        Function(
            VMAbcFile& file,
            const ClassInfo& ci,
            const UInt32 mi,
            Instances::fl::GlobalObjectScript& gos
            );

    public:
        virtual void ForEachChild_GC(Collector* prcc, GcOp op) const;

        Pickable<Instances::FunctionBase> MakeInstance(Traits& t)
        {
            // Placement new.
            return Pickable<Instances::FunctionBase>(new(t.Alloc()) Instances::FunctionBase(*this));
        }
        virtual void MakeObject(Value& result, Traits& t);
        virtual unsigned GetFixedMemSize() const;

        ASString GetFunctionName(const Value& _this) const;

        static const ThunkInfo& GetApplyThunkInfo() { return GetThunkInfo(0); }
        static const ThunkInfo& GetCallThunkInfo() { return GetThunkInfo(1); }

        VMAbcFile& GetFile() const
        {
            return *File;
        }
        Abc::MiInd GetMethodInfoInd() const
        {
            return MethodInfoInd;
        }
        Abc::MbiInd GetMethodBodyInfoInd() const;
        const Abc::MethodBodyInfo& GetMBI() const;
        const Abc::Multiname& GetReturnType() const;
        // OriginationTraits is where this function came from.
        const AS3::Traits& GetOriginationTraits() const;
        Instances::fl::GlobalObjectScript& GetGOS() const;

    protected:
        Function(VM& vm, const ClassInfo& ci);

    private:
        void RegisterSlots();
        static const ThunkInfo& GetThunkInfo(UPInt ind)
        {
            return f[ind];
        }

    private:
        static const TypeInfo* tit[5];
        static const ThunkInfo f[3];

        const Abc::MiInd MethodInfoInd; // MethodInd is required to build a proper object's name.
        const SPtr<VMAbcFile> File;
        const SPtr<Instances::fl::GlobalObjectScript> GOS;
    };

} // namespace InstanceTraits

namespace ClassTraits
{
    // ClassTraits::Function shouldn't be related to any VMAbcFile,
    // because it can be created when VMAbcFile is not available yet.
    class Function : public Traits
    {
    public:
        typedef Classes::Function ClassType;
        typedef InstanceTraits::Function InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Function(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);

    public:
        virtual void ForEachChild_GC(Collector* prcc, RefCountBaseGC<Mem_Stat>::GcOp op) const;

    public:
        InstanceTraits::Thunk& GetThunkTraits() const
        {
            return *ThunkTraits;
        }

        InstanceTraits::ThunkFunction& GetThunkFunctionTraits() const
        {
            return *ThunkFunctionTraits;
        }

        InstanceTraits::MethodInd& GetMethodIndTraits() const
        {
            return *MethodIndTraits;
        }

        InstanceTraits::VTableInd& GetVTableTraits() const
        {
            return *VTableTraits;
        }

    public:
        // Coerce *value* to this type (value of this class) ...
        virtual bool Coerce(const Value& value, Value& result) const;
        bool CoerceValue(const Value& value, Value& result) const;

    private:
        SPtr<InstanceTraits::Thunk>         ThunkTraits;
        SPtr<InstanceTraits::ThunkFunction> ThunkFunctionTraits;
        SPtr<InstanceTraits::MethodInd>     MethodIndTraits;
        SPtr<InstanceTraits::VTableInd>     VTableTraits;
    };

} // namespace ClassTraits

namespace Classes
{
    // Class for the Function ...
    class Function : public Class
    {
        friend class ClassTraits::Function;
        
    public:
        typedef Function SelfType;
        
    protected:
        SF_INLINE
        Function(ClassTraits::Traits& t)
        : Class(t)
        {
        }
        
    public:
        virtual Pickable<AS3::Object> MakePrototype() const;
        virtual void InitPrototype(AS3::Object& obj) const;
        virtual void Call(const Value& _this, Value& result, unsigned argc, const Value* const argv);
        virtual void Construct(Value& _this, unsigned argc, const Value* argv, bool extCall = false);

    public:
        const ClassTraits::Function& GetClassTraits() const
        {
            return static_cast<const ClassTraits::Function&>(Class::GetClassTraits());
        }

        // !!! In this method we miss an object ...
        // So, it is a function by default.
        Pickable<Instances::Function> MakeInstance(
            VMAbcFile& file, 
            const UInt32 method_ind, 
            const ScopeStackType& ss, 
            Instances::fl::GlobalObjectScript& gos
            ) const;

        Pickable<Instances::ThunkFunction> MakeThunkFunction(
            const ThunkInfo& thunk 
            SF_DEBUG_ARG(const Traits& ot)
            ) const;
        Pickable<Instances::CheckTypeTF> MakeCheckTypeInstance(
            const Class& data_type, 
            const ThunkInfo& thunk 
            SF_DEBUG_ARG(const Traits& ot)
            ) const;

    protected:
        ClassTraits::Function& GetClassTraits()
        {
            return static_cast<ClassTraits::Function&>(Class::GetClassTraits());
        }

    private:
        SelfType& GetSelf()
        {
            return *this;
        }

    private:
        static const TypeInfo* tit[6];
        static const ThunkInfo f[4];
    };
    
} // namespace Classes

namespace Instances
{
    inline
    CheckTypeTF::CheckTypeTF(
        InstanceTraits::ThunkFunction& t,
        const ThunkInfo& thunk,
        const Class& data_type
        SF_DEBUG_ARG(const Traits& ot)
        )
        : ThunkFunction(t, thunk SF_DEBUG_ARG(ot))
        , DataTypeClass(data_type)
    {
    }

    inline
    const InstanceTraits::Function& Function::GetInstanceTraitsFunction() const
    {
        return static_cast<const InstanceTraits::Function&>(GetInstanceTraits());
    }

    inline
    VMAbcFile& Function::GetFile() const
    {
        return GetInstanceTraitsFunction().GetFile();
    }

    inline
    const Abc::Multiname& Function::GetReturnType() const
    {
        return GetInstanceTraitsFunction().GetReturnType();
    }

    inline
    const Traits& Function::GetOriginationTraits() const
    {
        return GetInstanceTraitsFunction().GetOriginationTraits();
    }

} // namespace Instances

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif // INC_AS3_Obj_Function_H
