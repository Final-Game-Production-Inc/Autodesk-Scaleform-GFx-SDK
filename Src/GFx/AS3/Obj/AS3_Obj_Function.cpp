/**************************************************************************

Filename    :   AS3_Obj_Function.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Kernel/SF_MsgFormat.h"
#include "../AS3_VM.h"
#include "../AS3_VTable.h" // Because of InstanceTraits::MethodInd.
#include "../AS3_Marshalling.h"
#include "AS3_Obj_Function.h"
#include "AS3_Obj_Array.h"
#include "AS3_Obj_UserDefined.h"
#include "AS3_Obj_Global.h"

#include "GFx/AMP/Amp_Server.h"

namespace Scaleform { namespace GFx { namespace AS3 
{

///////////////////////////////////////////////////////////////////////////
namespace Classes
{
    ///////////////////////////////////////////////////////////////////////
    // !!! In this method we miss an object ...
    Pickable<Instances::Function> Function::MakeInstance(VMAbcFile& file, const UInt32 method_ind, const ScopeStackType& ss, Instances::fl::GlobalObjectScript& gos) const
    {
		// Name retrieved from MethodInfo is always empty for some reason.
		//DEBUG_CODE(const ASString name = GetVM().GetStringManager().CreateString(file.GetMethodInfo(method_ind).GetName());)

        InstanceTraits::Function& itr = file.GetFunctionInstanceTraits(gos, method_ind);

        // Functions do not have traits of their own, they use ActivationObject instead ...
        return Pickable<Instances::Function>(new(itr.Alloc()) Instances::Function(
            itr,
            ss
            SF_DEBUG_ARG(GetStringManager().CreateConstString("method id: ") + Scaleform::AsString(method_ind))
			//SF_DEBUG_ARG(name)
            ));
    }
    
    Pickable<Instances::ThunkFunction> Function::MakeThunkFunction(const ThunkInfo& thunk SF_DEBUG_ARG(const Traits& ot)) const
    {
        InstanceTraits::ThunkFunction& itr = GetClassTraits().GetThunkFunctionTraits();

        return Pickable<Instances::ThunkFunction>(new(itr.Alloc()) Instances::ThunkFunction(
            itr,
            thunk
            SF_DEBUG_ARG(ot)
            ));
    }

    Pickable<Instances::CheckTypeTF> Function::MakeCheckTypeInstance(const Class& data_type, const ThunkInfo& thunk SF_DEBUG_ARG(const Traits& ot)) const
    {
        InstanceTraits::ThunkFunction& itr = GetClassTraits().GetThunkFunctionTraits();

        // CheckTypeTF is bigger than ThunkFunction. Let's allocate memory manually.
        const unsigned size = sizeof(Instances::CheckTypeTF);
        void* addr = Memory::AllocInHeap(GetVM().GetMemoryHeap(), size);

        return Pickable<Instances::CheckTypeTF>(new(addr) Instances::CheckTypeTF(
            itr,
            thunk,
            data_type
            SF_DEBUG_ARG(ot)
            ));
    }

    Pickable<AS3::Object> Function::MakePrototype() const
    {
        InstanceTraits::Function& itr = static_cast<InstanceTraits::Function&>(GetClassTraits().GetInstanceTraits());
        return itr.MakeInstance(itr);
    }

    void Function::InitPrototype(AS3::Object& obj) const
    {
        Class* cl = GetParentClass();

        if (cl)
            cl->InitPrototype(obj);

        for (unsigned i = 0; i < 4; ++i)
            AddDynamicFunc(obj, f[i]);

        AddConstructor(obj);
    }

    void Function::Call(const Value& /*_this*/, Value& result, unsigned argc, const Value* const argv)
    {
        // The same logic as in Construct.
        Construct(result, argc, argv);
    }

    void Function::Construct(Value& _this, unsigned argc, const Value* argv, bool extCall)
    {
        if (argc > 0)
            return GetVM().ThrowEvalError(VM::Error(VM::eFunctionConstructorError, GetVM()));

        Class::Construct(_this, argc, argv, extCall);
    }

    const TypeInfo* Function::tit[6] = {
        &AS3::fl::StringTI,
        &AS3::fl::StringTI,
        NULL, NULL, NULL, 
        NULL, 
    };

    const ThunkInfo Function::f[4] = {
        {&Instances::FunctionBase::toStringProto, &Function::tit[0], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0},
        {&Instances::FunctionBase::toStringProto, &Function::tit[1], "toLocaleString", NULL, Abc::NS_Public, CT_Method, 0, 0},
        {&Instances::FunctionBase::apply, &Function::tit[2], "apply", NULL, Abc::NS_Public, CT_Method, 0, 2, 1},
        {&Instances::FunctionBase::call, &Function::tit[5], "call", NULL, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1},
    };

} // namespace Classes

namespace Instances
{
    ///////////////////////////////////////////////////////////////////////
    FunctionBase::FunctionBase(InstanceTraits::Traits& t)
        : Instances::fl::Object(t)
    {
        Prototype.Pick(GetVM().MakeObject());
    }

    FunctionBase::~FunctionBase()
    {
    }

    void FunctionBase::toStringProto(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
        SF_UNUSED3(ti, argc, argv);

        if (_this.IsFunction())
            result = vm.GetStringManager().CreateConstString("function Function() {}");
        else if (_this.IsThunk())
        {
            LongFormatter f((uintptr_t)&_this.AsThunk());
            ASString s = vm.GetStringManager().CreateConstString("[object Function-");

            f.SetBase(16).Convert();
            s += f.ToCStr();
            s += "]";

            result = s;
        }
        /* DO NOT delete this code.
        // Commenting this code out breaks only one test 
        // (acceptance\ecma3\String\e15_5_4_6_2_rt.abc).
        // Other tests are pretty fine.
        else if (_this.IsThunkFunction())
        {
            // We need this code because Thunk can be substituted by ThunkFunction
            // (for example, in AS3_Obj_Object::Construct()).

            LongFormatter f((uintptr_t)&_this.AsThunkFunction().GetThunk());
            ASString s = vm.GetStringManager().CreateConstString("[object Function-");

            f.SetBase(16).Convert();
            s += f.ToCStr();
            s += "]";

            result = s;
        }
        */
        else
            // Below is a correct behavior.
            // vm.ThrowTypeError(VM::eUndefinedVarError);
            // !!! This is not correct, but it works for the time being.
            result = vm.GetStringManager().CreateConstString("function Function() {}");
    }
    
    void FunctionBase::ExecuteUnsafe(const Value& /*_this*/, Value& /*result*/, unsigned /*argc*/, const Value* /*argv*/)
    {
        ; // Do nothing.
    }

    void FunctionBase::Execute(const Value& /*_this*/, unsigned /*argc*/, const Value* /*argv*/, bool /*discard_result*/)
    {
        ; // Do nothing.
    }

    void FunctionBase::Call(const Value& _this, Value& result, unsigned argc, const Value* const argv)
    {
        Value r = Value::GetUndefined();
        ExecuteUnsafe(_this, r, argc, argv);
        r.Swap(result);
    }

    void FunctionBase::apply(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
        SF_UNUSED1(ti);
        Value r = Value::GetUndefined();

        switch (argc)
        {
        case 0:
            // argv[0] is supposed to be *this*.
            vm.ExecuteUnsafe(_this, Value::GetUndefined(), r, argc, argv);
            break;
        case 1:
            // There is no an Array object with arguments.
            vm.ExecuteUnsafe(_this, argv[0], r, 0, NULL);
            break;
        default:
            {
                Value resultValue;
                const ClassTraits::fl::Array& ctr = vm.GetClassTraitsArray();
                const Value& v = argv[1]; // For debugging.

                if (ctr.Coerce(v, resultValue))
                {
                    unsigned f_argc = 0;
                    const Value* f_argv = NULL;

                    if (!resultValue.IsNull())
                    {
                        Instances::fl::Array* array = static_cast<Instances::fl::Array*>(result.GetObject());

                        if (array->IsSparse())
                        {
                            ValueArrayDH args(vm.GetMemoryHeap());
                            // Repack "array" into "args" because "array" is a sparse array.
                            args.Resize(array->GetSize());
                            for (UPInt i = 0; i < args.GetSize(); ++i)
                                args[i] = array->At(i);

                            f_argc = static_cast<unsigned>(args.GetSize());
                            f_argv = args.GetDataPtr();
                        }
                        else
                        {
                            const ValueArrayDH& cont_args = array->GetContiguousPart();

                            f_argc = static_cast<unsigned>(cont_args.GetSize());
                            f_argv = cont_args.GetDataPtr();
                        }
                    }

                    vm.ExecuteUnsafe(_this, argv[0], r, f_argc, f_argv);
                }
                else
                {
                    return vm.ThrowTypeError(VM::Error(VM::eApplyError, vm));
                }

            }
            break;
        }

        result.Swap(r);
    }

#if 0
    void FunctionBase::ForEachChild_GC(Collector* prcc, GcOp op) const
    {
        Instances::fl::Object::ForEachChild_GC(prcc, op);

        // Theoretically, Prototype is registered with slots, but we
        // will get memory leaks if we do not visit it here.
        // This is weird.
        AS3::ForEachChild_GC<AS3::Object, Mem_Stat>(prcc, Prototype, op);
    }
#endif

    void FunctionBase::Construct(Value& result, unsigned argc, const Value* argv, bool /*extCall*/)
    {
        // Create a new object ...
        result = GetVM().MakeObject();

        // *discard_result* MUST be *true*, because it is a constructor.
        // Otherwise nothing will work.
        Execute(result, argc, argv, true);
    }

    UInt32 FunctionBase::GetMaxParamNum() const
    {
        return 0;
    }

    void FunctionBase::lengthGet(UInt32& result)
    {
        result = GetMaxParamNum();
    }

    void FunctionBase::call(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
        SF_UNUSED1(ti);
        Value r = Value::GetUndefined();

        if (argc < 1)
            vm.ExecuteUnsafe(_this, Value(vm.GetGlobalObject()), r, argc, argv);
        else
            vm.ExecuteUnsafe(
                _this, 
                (argv[0].IsNullOrUndefined() ? Value(vm.GetGlobalObject()) : argv[0]), 
                r, 
                argc - 1, 
                argv + 1
                );

        result.Swap(r);
    }

    void FunctionBase::PushResult(Value& value) const
    {
        GetVM().PushResult(value);
    }

    void FunctionBase::PickPushResult(Value& value) const
    {
        GetVM().PickPushResult(value);
    }

    void FunctionBase::RetrieveResult(Value& result) const
    {
        GetVM().PickResult(result);
    }

    void FunctionBase::RetrieveResultUnsafe(Value& result) const
    {
        GetVM().PickResultUnsafe(result);
    }

    ///////////////////////////////////////////////////////////////////////
    FunctionAOT::FunctionAOT(
        InstanceTraits::Traits& t, 
        const ScopeStackType& ss,
        Instances::fl::GlobalObject& go
        )
        : FunctionBase(t)
        , GO(go)
        , StoredScopeStack(ss)
    {
        // This is a correct implementation.
        // We HAVE TO combine Saved Scope and Scope Stack.

        const VM& vm = GetVM();
        const ScopeStackType& vm_ss = vm.GetScopeStack();
        const UPInt size = vm_ss.GetSize();

        for (UPInt i = vm.GetScopeStackBaseInd(); i < size; ++i)
            StoredScopeStack.PushBack(vm_ss[i]);
    }

    void FunctionAOT::ForEachChild_GC(Collector* prcc, GcOp op) const
    {
        FunctionBase::ForEachChild_GC(prcc, op);

        AS3::ForEachChild_GC(prcc, StoredScopeStack, op SF_DEBUG_ARG(*this));
    }

    ///////////////////////////////////////////////////////////////////////
    Function::Function(
        InstanceTraits::Function& tr,
        const ScopeStackType& ss
        SF_DEBUG_ARG(const ASString& name)
        )
    : FunctionBase(tr)
    , StoredScopeStack(ss) // Add Saved Scope.
    SF_DEBUG_ARG(Name(name))
    {
        // This is a correct implementation.
        // We HAVE TO combine Saved Scope and Scope Stack.

        VM& vm = GetVM();
        const ScopeStackType& vm_ss = vm.GetScopeStack();
        const UPInt size = vm_ss.GetSize();

        for (UPInt i = vm.GetScopeStackBaseInd(); i < size; ++i)
            StoredScopeStack.PushBack(vm_ss[i]);
    }

    Function::~Function()
    {
    }
    
    void Function::Execute(const Value& _this, unsigned argc, const Value* argv, bool discard_result)
    {
        VM& vm = GetVM();
        const InstanceTraits::Function& itr = GetInstanceTraitsFunction();
        const ScopeStackType& sss = itr.GetStoredScopeStack();
        const Value& correct_this = (_this.IsNullOrUndefined() && !sss.IsEmpty()) ? sss[0] : _this;
        const Abc::MbiInd mbi_ind = itr.GetMethodBodyInfoInd();
        const Traits& otr = itr.GetOriginationTraits();
        
        vm.AddFrame(
            Value(const_cast<Function*>(this)),
            itr.GetFile(),
            mbi_ind,
            correct_this,
            argc,
            argv,
            discard_result,
#ifdef SF_AS3_ENABLE_EXPLICIT_GO
            itr.GetGOS(),
#endif
            GetStoredScopeStack(),
            otr
            SF_DEBUG_ARG(Name)
            );
    }
    
    void Function::ExecuteUnsafe(const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
        Execute(_this, argc, argv);

        if (IsException())
            return;

        // At this point an extra reference to "this" is stored in a CallFrame.
        // gcGuard exists to prevent garbage-collecting of this objects after
        // CallFrame is destroyed.
        SPtr<Function> gcGuard(const_cast<Function*>(this));
        const int stack_depth = GetVM().ExecuteCode();
        SF_UNUSED(stack_depth);
        SF_ASSERT(stack_depth == 0);
        // At this point CallFrame is destroyed and it is possible that this object is
        // garbage-collected.
        // But gcGuard still prevents this object from deleting.
        
        if (IsException())
            return;
        
        RetrieveResult(result);
    }

    UInt32 Function::GetMaxParamNum() const
    {
        const VMAbcFile& file = GetInstanceTraitsFunction().GetFile();
        const Abc::MethodBodyInfo& mbi = GetInstanceTraitsFunction().GetMBI();

        return static_cast<UInt32>(mbi.GetMethodInfo(file.GetMethods()).GetParamCount());
    }

    void Function::ForEachChild_GC(Collector* prcc, RefCountBaseGC<Mem_Stat>::GcOp op) const
    {
        FunctionBase::ForEachChild_GC(prcc, op);
        
        AS3::ForEachChild_GC(prcc, StoredScopeStack, op SF_DEBUG_ARG(*this));
    }

    void Function::StoreScopeStack(const UPInt baseSSInd, const ScopeStackType& ss)
    {
        StoredScopeStack.Clear();

        const UPInt size = ss.GetSize();
        for (UPInt i = baseSSInd; i < size; ++i)
            StoredScopeStack.PushBack(ss[i]);
    }

    ///////////////////////////////////////////////////////////////////////
    void ThunkFunction::ExecuteUnsafe(const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
        // Just delegate a call to VM.
        GetVM().ExecuteUnsafe(Value(Thunk), _this, result, argc, argv);
    }

    void ThunkFunction::Execute(const Value& _this, unsigned argc, const Value* argv, bool discard_result)
    {
        // Just delegate a call to VM.
        GetVM().Execute(Value(Thunk), _this, argc, argv, discard_result);
    }

    void ThunkFunction::length(Value& result, unsigned argc, const Value* argv)
    {
        InstanceTraits::Thunk::lengthGet(GetThunk(), GetVM(), Thunk, result, argc, argv);
    }

#ifdef GFX_AS3_VERBOSE
    void ThunkFunction::ForEachChild_GC(Collector* prcc, RefCountBaseGC<Mem_Stat>::GcOp op) const
    {
        FunctionBase::ForEachChild_GC(prcc, op);

        AS3::ForEachChild_GC_Const<const Traits, Mem_Stat>(prcc, OriginationTraits, op SF_DEBUG_ARG(*this));
    }
#endif

    ///////////////////////////////////////////////////////////////////////
    void CheckTypeTF::ExecuteUnsafe(const Value& _this, Value& result, unsigned argc, const Value* argv)
    {
        /*
        // Can it be replaced with IsOfType()?
        Value coerced_this;
        if (!DataTypeClass.Coerce(_this, coerced_this))
            return GetVM().ThrowTypeError(VM::eUndefinedVarError);

        ParentType::Execute(coerced_this, result, argc, argv);
        */

        if (!GetVM().GetClassTraits(_this).IsOfType(DataTypeClass.GetClassTraits()))
            return GetVM().ThrowTypeError(VM::Error(VM::eCheckTypeFailedError, GetVM()
                SF_DEBUG_ARG(GetVM().GetClassTraits(_this).GetName().ToCStr())
                SF_DEBUG_ARG(DataTypeClass.GetClassTraits().GetName().ToCStr())
            ));

        ParentType::ExecuteUnsafe(_this, result, argc, argv);
    }

    void CheckTypeTF::Execute(const Value& _this, unsigned argc, const Value* argv, bool discard_result)
    {
        /*
        // Can it be replaced with IsOfType()?
        Value coerced_this;
        if (!DataTypeClass.Coerce(_this, coerced_this))
            return GetVM().ThrowTypeError(VM::eUndefinedVarError);

        ParentType::Execute(coerced_this, argc, argv, discard_result);
        */

        if (!GetVM().GetClassTraits(_this).IsOfType(DataTypeClass.GetClassTraits()))
        {
            return GetVM().ThrowTypeError(VM::Error(VM::eCheckTypeFailedError, GetVM()
                SF_DEBUG_ARG(GetVM().GetClassTraits(_this).GetName().ToCStr())
                SF_DEBUG_ARG(DataTypeClass.GetClassTraits().GetName().ToCStr())
                ));
        }

        ParentType::Execute(_this, argc, argv, discard_result);
    }

}

typedef ThunkFunc2<Instances::ThunkFunction, __LINE__, Value, unsigned, const Value*> TFunc_ThunkFunction_length;
typedef ThunkFunc0<Instances::FunctionBase, __LINE__, UInt32> TFunc_FunctionBase_lengthGet;

template <> const TFunc_ThunkFunction_length::TMethod TFunc_ThunkFunction_length::Method = &Instances::ThunkFunction::length;
template <> const TFunc_FunctionBase_lengthGet::TMethod TFunc_FunctionBase_lengthGet::Method = &Instances::FunctionBase::lengthGet;

namespace InstanceTraits
{
    ///////////////////////////////////////////////////////////////////////////
    Thunk::Thunk(VM& vm)
    : CTraits(vm, AS3::fl::FunctionCICpp)
    {
        SetTraitsType(Traits_Function);

        // Thunk doesn't belong to any VMAbcFile.

        for (unsigned i = 0; i < 3; ++i)
            Add2VT(AS3::fl::FunctionCI, f[i]);
    }

    ASString Thunk::GetThunkName(const Value& _this) const
    {
        LongFormatter ftm((uintptr_t)&_this.AsThunk());
        ASString s = GetVM().GetStringManager().CreateConstString("Function-");

        ftm.SetBase(16).Convert();
        s += ftm.ToCStr();

        return s;
    }

    void Thunk::lengthGet(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned /*argc*/, const Value* /*argv*/)
    {
        SF_UNUSED2(ti, vm);
        const ThunkInfo& ti2 = _this.AsThunk();
        UInt32 MaxArgNum = static_cast<UInt32>(ti2.GetMaxArgNum());

        MaxArgNum = (MaxArgNum == SF_AS3_VARARGNUM ? static_cast<UInt32>(ti2.GetMinArgNum()) : MaxArgNum);
        result.SetUInt32(MaxArgNum);
    }

    void Thunk::MakeObject(Value& result, Traits& t)
    {
        SF_UNUSED2(result, t);
        SF_ASSERT(false);
    }

    const TypeInfo* Thunk::tit[5] = {
        NULL, NULL, NULL,
        NULL,
        &AS3::fl::uintTI,
    };

    const ThunkInfo Thunk::f[3] = {
        // Apply() should go first because we refer to it in GetApply() (This is related to a different class).
        {&Instances::FunctionBase::apply, &Thunk::tit[0], "apply", NS_AS3, Abc::NS_Public, CT_Method, 0, 2, 1},
        {&Instances::FunctionBase::call, &Thunk::tit[3], "call", NS_AS3, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1},
        {&Thunk::lengthGet, &Thunk::tit[4], "length", NULL, Abc::NS_Public, CT_Get, 0, 0},
    };

    ///////////////////////////////////////////////////////////////////////////
    MethodInd::MethodInd(VM& vm)
    : CTraits(vm, AS3::fl::FunctionCICpp)
    {
        SetTraitsType(Traits_Function);

        // Thunk doesn't belong to any VMAbcFile.

        for (unsigned i = 0; i < 3; ++i)
            Add2VT(AS3::fl::FunctionCI, f[i]);
    }

    ASString MethodInd::GetMethodIndName(const Value& _this) const
    {
        LongFormatter flf((unsigned long)_this.GetMethodInfoInd().Get());
        ASString s = GetVM().GetStringManager().CreateConstString("Function-");

        flf.SetBase(16).Convert();
        s += flf.ToCStr();

        return s;
    }

    void MethodInd::lengthGet(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned /*argc*/, const Value* /*argv*/)
    {
        SF_UNUSED2(ti, vm);
        const Abc::MiInd ind(_this.GetMethodInfoInd());
        const AS3::Traits& tr = (_this.IsMethodInd() ? _this.GetTraits() : _this.GetClosure()->GetTraits());

        if (tr.GetFilePtr() == NULL)
            return;

        const VMAbcFile& file = *tr.GetFilePtr();
        const Abc::MethodInfo& mi = file.GetMethods().Get(ind);

        result.SetUInt32(static_cast<UInt32>(mi.GetParamCount()));
    }

    void MethodInd::MakeObject(Value& result, Traits& t)
    {
        SF_UNUSED2(result, t);
        SF_ASSERT(false);
    }

    const TypeInfo* MethodInd::tit[5] = {
        NULL, NULL, NULL,
        NULL,
        &AS3::fl::uintTI,
    };

    const ThunkInfo MethodInd::f[3] = {
        // Apply() should go first because we refer to it in GetApply() (This is related to a different class).
        {&Instances::FunctionBase::apply, &MethodInd::tit[0], "apply", NS_AS3, Abc::NS_Public, CT_Method, 0, 2, 1},
        {&Instances::FunctionBase::call, &MethodInd::tit[3], "call", NS_AS3, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1},
        {&MethodInd::lengthGet, &MethodInd::tit[4], "length", NULL, Abc::NS_Public, CT_Get, 0, 0},
    };

    ///////////////////////////////////////////////////////////////////////////
    VTableInd::VTableInd(VM& vm)
    : CTraits(vm, AS3::fl::FunctionCICpp)
    {
        SetTraitsType(Traits_Function);

        for (unsigned i = 0; i < 3; ++i)
            Add2VT(AS3::fl::FunctionCI, f[i]);
    }

    void VTableInd::lengthGet(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned /*argc*/, const Value* /*argv*/)
    {
        SF_UNUSED2(ti, vm);
        const SInt32 vt_ind = _this.GetVTableInd();
        const AS3::Traits& tr = (_this.IsVTableInd() ? _this.GetTraits() : _this.GetClosure()->GetTraits());
        const Value& real_value = tr.GetVT().GetRaw(AbsoluteIndex(vt_ind));
        const Abc::MiInd method_ind(real_value.GetMethodInfoInd());

        if (tr.GetFilePtr() == NULL)
            return;

        const VMAbcFile& file = *tr.GetFilePtr();

        const Abc::MethodInfo& mi = file.GetMethods().Get(method_ind);

        result.SetUInt32(static_cast<UInt32>(mi.GetParamCount()));
    }

    void VTableInd::MakeObject(Value& result, Traits& t)
    {
        SF_UNUSED2(result, t);
        SF_ASSERT(false);
    }

    const TypeInfo* VTableInd::tit[5] = {
        NULL, NULL, NULL,
        NULL,
        &AS3::fl::uintTI,
    };

    const ThunkInfo VTableInd::f[3] = {
        // Apply() should go first because we refer to it in GetApply() (This is related to a different class).
        {&Instances::FunctionBase::apply, &VTableInd::tit[0], "apply", NS_AS3, Abc::NS_Public, CT_Method, 0, 2, 1},
        {&Instances::FunctionBase::call, &VTableInd::tit[3], "call", NS_AS3, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1},
        {&VTableInd::lengthGet, &VTableInd::tit[4], "length", NULL, Abc::NS_Public, CT_Get, 0, 0},
    };

    ///////////////////////////////////////////////////////////////////////////
    // Thunk delegate is supposed to be a real Function object, which is dynamic.
    ThunkFunction::ThunkFunction(VM& vm)
    : CTraits(vm, AS3::fl::FunctionCIThunk) 
    {
        SetTraitsType(Traits_Function);

        // ThunkFunction doesn't belong to any VMAbcFile.

        for (unsigned i = 0; i < 1; ++i)
            Add2VT(AS3::fl::FunctionCI, f[i]);
    }

    ASString ThunkFunction::GetThunkName(const Value& _this) const
    {
        LongFormatter flf((uintptr_t)&_this.AsThunkFunction().GetThunk());
        ASString s = GetVM().GetStringManager().CreateConstString("Function-");

        flf.SetBase(16).Convert();
        s += flf.ToCStr();

        return s;
    }

    void ThunkFunction::MakeObject(Value& result, Traits& t)
    {
        SF_UNUSED2(result, t);
        SF_ASSERT(false);
    }

    const TypeInfo* ThunkFunction::tit[1] = {
        &AS3::fl::uintTI,
    };

    const ThunkInfo ThunkFunction::f[1] = {
        {TFunc_ThunkFunction_length::Func, &ThunkFunction::tit[0], "length", NULL, Abc::NS_Public, CT_Get, 0, 0},
    };

    ///////////////////////////////////////////////////////////////////////////
    Function::Function(
        VMAbcFile& file,
        const ClassInfo& ci,
        const UInt32 mi,
        Instances::fl::GlobalObjectScript& gos
        )
    : CTraits(file.GetVM(), ci)
    , MethodInfoInd(mi)
    , File(&file)
    , GOS(&gos)
    {
        SetConstructor(file.GetVM().GetClassFunction());
        SetTraitsType(Traits_Function);

        /* !!! DO NOT delete this code.
        // !!! We MAY NOT do this !!!
        const Abc::MethodBodyInfo& mbi = file.GetMethodBodyInfo(MethodInfoInd);
        const Abc::HasTraits& code_traits = static_cast<const Abc::HasTraits&>(mbi);

        // MemSize shouldn't depend on parent.
        if (!AddSlots(code_traits, file, GetFixedMemSize()))
            return;
        */

        RegisterSlots();
    }
    
    Function::Function(VM& vm, const ClassInfo& ci)
    : CTraits(vm, ci)
    , MethodInfoInd(0)
    {
        SetTraitsType(Traits_Function);

        RegisterSlots();
    }

    unsigned Function::GetFixedMemSize() const
    {
        return sizeof(Instances::Function);
    }

    void Function::ForEachChild_GC(Collector* prcc, GcOp op) const
    {
        Traits::ForEachChild_GC(prcc, op);

        AS3::ForEachChild_GC<VMAbcFile, Mem_Stat>(prcc, File, op SF_DEBUG_ARG(*this));
        AS3::ForEachChild_GC<Instances::fl::GlobalObjectScript, Mem_Stat>(prcc, GOS, op SF_DEBUG_ARG(*this));
    }
    
    void Function::RegisterSlots()
    {
        AddSlotCPP(
            GetVM().GetStringManager().CreateConstString("prototype"),
            GetVM().MakePublicNamespace(), 
            &GetVM().GetClassTraitsObject(),
            SlotInfo::BT_ObjectCpp,
            AbsoluteIndex(OFFSETOF(Instances::FunctionBase, Prototype)),
            false // ??? is it const or not ?
            );
        
        // Do not forget to change array size if you change static ThunkInfo f[].
        for (unsigned i = 0; i < 3; ++i)
            Add2VT(AS3::fl::FunctionCI, GetThunkInfo(i));
    }
    
    ASString Function::GetFunctionName(const Value& _this) const
    {
        // !!! Not safe.
        const Instances::Function& funct = static_cast<Instances::Function&>(*_this.GetObject());
        LongFormatter flf((unsigned long)funct.GetInstanceTraitsFunction().GetMethodInfoInd().Get());
        ASString s = GetVM().GetStringManager().CreateConstString("Function-");

        flf.SetBase(16).Convert();
        s += flf.ToCStr();

        return s;
    }

    void Function::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(t);
    }

    const TypeInfo* Function::tit[5] = {
        NULL, NULL, NULL,
        NULL,
        &AS3::fl::uintTI,
    };

    const ThunkInfo Function::f[3] = {
        // Apply() should go first because we refer to it in GetApply().
        {&Instances::FunctionBase::apply, &Function::tit[0], "apply", NS_AS3, Abc::NS_Public, CT_Method, 0, 2, 1},
        {&Instances::FunctionBase::call, &Function::tit[3], "call", NS_AS3, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1},
        {TFunc_FunctionBase_lengthGet::Func, &Function::tit[4], "length", NULL, Abc::NS_Public, CT_Get, 0, 0},
    };

    Abc::MbiInd Function::GetMethodBodyInfoInd() const
    {
        return GetFile().GetMethodBodyInfoInd(GetMethodInfoInd());
    }

    const Abc::MethodBodyInfo& Function::GetMBI() const
    {
        return GetFile().GetMethodBodyInfo(GetMethodInfoInd());
    }

    const Abc::Multiname& Function::GetReturnType() const
    {
        return GetMBI().GetMethodInfo(GetFile().GetMethods()).GetReturnType(GetFile().GetConstPool());
    }

    const AS3::Traits& Function::GetOriginationTraits() const
    {
        return GOS->GetTraits();
    }

    Instances::fl::GlobalObjectScript& Function::GetGOS() const
    {
        SF_ASSERT(GOS.GetPtr());

        if (!GOS->IsInitialized())
            GOS->InitOnDemand();

        return *GOS;
    }

} // namespace InstanceTraits

///////////////////////////////////////////////////////////////////////////////
Value::Value(Instances::Function* v)
: Flags(kFunction)
, value(v)
{
    if (v)
        v->AddRef();
}

Value::Value(Instances::ThunkFunction* v)
: Flags(kThunkFunction)
, value(v)
{
    if (v)
        v->AddRef();
}

void Value::Assign(Instances::Function* v)
{
    Release();
    SetKind(kFunction);
    value = v;
    if (v)
        v->AddRef();
}

void Value::AssignUnsafe(Instances::Function* v)
{
    SF_ASSERT(!IsRefCounted());
    SetKind(kFunction);
    value = v;
    if (v)
        v->AddRef();
}

void Value::Assign(Instances::ThunkFunction* v)
{
    Release();
    SetKind(kThunkFunction);
    value = v;
    if (v)
        v->AddRef();
}

void Value::AssignUnsafe(Instances::ThunkFunction* v)
{
    SF_ASSERT(!IsRefCounted());
    SetKind(kThunkFunction);
    value = v;
    if (v)
        v->AddRef();
}

void Value::Pick(Instances::Function* v)
{
    Release();
    SetKind(kFunction);
    value = v;
    // No AddRef() is necessary.
}

void Value::Pick(Instances::FunctionBase* v)
{
    Release();
    SetKind(kFunction);
    value = v;
    // No AddRef() is necessary.
}

void Value::PickUnsafe(Instances::Function* v)
{
    SF_ASSERT(!IsRefCounted());
    SetKind(kFunction);
    value = v;
    // No AddRef() is necessary.
}

void Value::PickUnsafe(Instances::FunctionBase* v)
{
    SF_ASSERT(!IsRefCounted());
    SetKind(kFunction);
    value = v;
    // No AddRef() is necessary.
}

void Value::Pick(Instances::ThunkFunction* v)
{
    Release();
    SetKind(kThunkFunction);
    value = v;
    // No AddRef() is necessary.
}

void Value::PickUnsafe(Instances::ThunkFunction* v)
{
    SF_ASSERT(!IsRefCounted());
    SetKind(kThunkFunction);
    value = v;
    // No AddRef() is necessary.
}

///////////////////////////////////////////////////////////////////////////////
namespace ClassTraits
{
    ///////////////////////////////////////////////////////////////////////
    Function::Function(VM& vm, const ClassInfo& ci)
    : Traits(vm, ci)
    {
        SetTraitsType(Traits_Function);
        MemoryHeap* mh = vm.GetMemoryHeap();

        // InstanceTraites created below is for compatibility only.
        // It is fake.
        Pickable<InstanceTraits::Traits> it(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraits::Function(vm, ci));
        SetInstanceTraits(it);

        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) Classes::Function(*this));
#ifndef SF_AS3_SETUP_INSTANCE_CONSTRUCTOR_IN_CLASS
        it->SetConstructor(cl);
#endif
        // Extra stuff.

        ThunkTraits = MakePickable(SF_HEAP_NEW_ID(vm.GetMemoryHeap(), StatMV_VM_ITraits_Mem) InstanceTraits::Thunk(vm));
        ThunkTraits->SetConstructor(Pickable<Class>(cl.GetPtr(), PickValue));

        ThunkFunctionTraits = MakePickable(SF_HEAP_NEW_ID(vm.GetMemoryHeap(), StatMV_VM_ITraits_Mem) InstanceTraits::ThunkFunction(vm));
        ThunkFunctionTraits->SetConstructor(Pickable<Class>(cl.GetPtr(), PickValue));

        MethodIndTraits = MakePickable(SF_HEAP_NEW_ID(vm.GetMemoryHeap(), StatMV_VM_ITraits_Mem) InstanceTraits::MethodInd(vm));
        MethodIndTraits->SetConstructor(Pickable<Class>(cl.GetPtr(), PickValue));

        VTableTraits = MakePickable(SF_HEAP_NEW_ID(vm.GetMemoryHeap(), StatMV_VM_ITraits_Mem) InstanceTraits::VTableInd(vm));
        VTableTraits->SetConstructor(Pickable<Class>(cl.GetPtr(), PickValue));
    }

    Pickable<Traits> Function::MakeClassTraits(VM& vm)
    {
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(vm.GetMemoryHeap(), StatMV_VM_CTraits_Mem) Function(vm, AS3::fl::FunctionCI));

        return ctr;
    }

    void Function::ForEachChild_GC(Collector* prcc, RefCountBaseGC<Mem_Stat>::GcOp op) const
    {
        Traits::ForEachChild_GC(prcc, op);

        AS3::ForEachChild_GC<InstanceTraits::Thunk, Mem_Stat>(prcc, ThunkTraits, op SF_DEBUG_ARG(*this));
        AS3::ForEachChild_GC<InstanceTraits::ThunkFunction, Mem_Stat>(prcc, ThunkFunctionTraits, op SF_DEBUG_ARG(*this));
        AS3::ForEachChild_GC<InstanceTraits::MethodInd, Mem_Stat>(prcc, MethodIndTraits, op SF_DEBUG_ARG(*this));
        AS3::ForEachChild_GC<InstanceTraits::VTableInd, Mem_Stat>(prcc, VTableTraits, op SF_DEBUG_ARG(*this));
    }

    bool Function::Coerce(const Value& value, Value& result) const
    {
        return CoerceValue(value, result);
    }

    bool Function::CoerceValue(const Value& value, Value& result) const
    {
        switch (value.GetKind())
        {
        case Value::kFunction:
        case Value::kThunk:
        case Value::kVTableInd:
        case Value::kVTableIndClosure:
        case Value::kThunkFunction:
        case Value::kThunkClosure:
            result = value;
            return true;
        case Value::kObject:
            {
                // In case tag "is lost in translation" we can check TraitsType.
                AS3::Object* obj = value.GetObject();
                if (obj)
                {
                    const AS3::Traits& tr = obj->GetTraits();
                    if (tr.GetTraitsType() == Traits_Function && tr.IsInstanceTraits())
                    {
                        result = value;
                        return true;
                    }
                }
            }
            break;
        default:
            break;
        }

        return ClassTraits::Traits::CoerceValue(value, result);
    }

} // namespace ClassTraits

namespace fl
{
    const TypeInfo FunctionTI = {
        TypeInfo::CompileTime | TypeInfo::DynamicObject | TypeInfo::Abc,
        sizeof(ClassTraits::Function::InstanceType),
        0, 0, 0, 0,
        "Function", "", &ObjectTI,
        TypeInfo::None
    };
    const ClassInfo FunctionCI = {&FunctionTI, NULL};

    const TypeInfo FunctionTICpp = {
        TypeInfo::CompileTime, 
        // ???
        sizeof(ClassTraits::Function::InstanceType),
        0, 0, 0, 0,
        "Function", "", &ObjectTI,
        TypeInfo::None
    };
    const ClassInfo FunctionCICpp = {&FunctionTICpp, NULL};

    const TypeInfo FunctionTIThunk = {
        TypeInfo::CompileTime | TypeInfo::DynamicObject, 
        sizeof(Instances::ThunkFunction),
        0, 0, 0, 0,
        "Function", "", &ObjectTI,
        TypeInfo::None
    };
    const ClassInfo FunctionCIThunk = {&FunctionTIThunk, NULL};
}
    
}}} // namespace Scaleform { namespace GFx { namespace AS3
