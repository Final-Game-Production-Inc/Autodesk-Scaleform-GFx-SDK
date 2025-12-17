/**************************************************************************

Filename    :   AS3_Tracer.cpp
Content     :   
Created     :   Sep, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "AS3_Tracer.h"
#include "AS3_AsString.h"
#include "AS3_VTable.h"

#include "Obj/AS3_Obj_Array.h"
#include "Obj/AS3_Obj_UserDefined.h"
#include "Obj/AS3_Obj_Global.h"
#include "Obj/Vec/AS3_Obj_Vec_Vector_object.h"
#include "Obj/AS3_Obj_int.h"
#include "Obj/AS3_Obj_uint.h"
#include "Obj/AS3_Obj_Number.h"
#include "Obj/AS3_Obj_Boolean.h"
#include "Obj/AS3_Obj_String.h"
#include "Obj/AS3_Obj_Catch.h"

#include "Obj/Vec/AS3_Obj_Vec_Vector.h"
#include "Obj/Vec/AS3_Obj_Vec_Vector_int.h"
#include "Obj/Vec/AS3_Obj_Vec_Vector_uint.h"
#include "Obj/Vec/AS3_Obj_Vec_Vector_double.h"
#include "Obj/Vec/AS3_Obj_Vec_Vector_String.h"
#include "Obj/Vec/AS3_Obj_Vec_Vector_object.h"

// Not finished yet.
// #define SF_AS3_TRACE_DEAD_BLOCK

namespace Scaleform { namespace GFx { namespace AS3 
{

    namespace fl
    {
        extern const TypeInfo ArgumentErrorTI;
        extern const ClassInfo ArgumentErrorCI;
    } // namespace fl

///////////////////////////////////////////////////////////////////////////////
// static
// const ClassTraits::Traits* FindScopeTraits(VM& vm, const UPInt baseSSInd, const ScopeStackType& scope_stack, const Multiname& mn)
// {
//     const ClassTraits::Traits* result = NULL;
// 
//     for (UPInt i = scope_stack.GetSize(); i > baseSSInd ; --i)
//     {
//         const Value& v = scope_stack[i - 1];
//         const Traits& tr = vm.GetValueTraits(v);
// 
//         result = FindFixedTraits(vm, tr, mn);
//         
//         if (result)
//             // For RVO sake ...
//             break;
//     }
// 
//     return result;
// }

namespace TR 
{

    ///////////////////////////////////////////////////////////////////////////
    void StackReader::CheckObject(const Value& v) const
    {
        SF_UNUSED1(v);

        /* Disable checks temporarily till we implement all operations with stack.
        if (v.IsNull())
        {
            // A TypeError is thrown if obj is null or undefined.
            GetVM().ThrowVerifyError(VM::eConvertNullToObjectError);
        }
        else if (v.IsUndefined())
        {
            GetVM().ThrowVerifyError(VM::eConvertUndefinedToObjectError);
        }
        */
    }

    int StackReader::Read(Multiname& obj) const
    {
        switch (obj.GetKind())
        {
        case Abc::MN_QName:
        case Abc::MN_QNameA:
            // QName is not read from stack; both name and namespase known at compile-time.
            break;
        case Abc::MN_RTQName:
        case Abc::MN_RTQNameA:
            // Name is known at compile time, Namespace is resolved at runtime.

            /*
            // For maniacs.
            CheckObject(GetState().BackOpValue());
            if (GetVM().IsException())
                break;
            */

            // !!! We should get a Namespace here.
            // It is up to verifier to check that.
            // In our case we will just get "any namespace".
            GetState().PopOpValue();
            return 1;
        case Abc::MN_RTQNameL:
        case Abc::MN_RTQNameLA:
            // Both name and namespace are resolved at runtime.
            // We will get "any name" here.
            GetState().PopOpValue();

            /*
            // For maniacs.
            CheckObject(GetState().BackOpValue());
            if (GetVM().IsException())
                break;
            */

            // !!! We should get a Namespace here.
            // It is up to verifier to check that.
            // In our case we will just get "any namespace".
            //GetState().PopOpValue();
            obj.SetRTName(GetState().PopOpValue());
            return 2;
        case Abc::MN_Multiname:
        case Abc::MN_MultinameA:
            // MInfo is not supposed to be read from stack.
            break;
        case Abc::MN_MultinameL:
        case Abc::MN_MultinameLA:
            // Namespace set is known at compile time, Name is resolved at runtime.        
            // name_ind is a string, and we will get an int or an uint as an index instead;
            // We will get "any name" here.
            //GetState().PopOpValue();
            obj.SetRTName(GetState().PopOpValue());
            return 1;
        case Abc::MN_Typename:
            // Recursive!!!?
            GetVM().GetUI().Output(FlashUI::Output_Warning, "Reading chained multiname in itself.");
            return Read(obj);
        default:
            SF_ASSERT(false);
            break;
        }

        return 0;
    }

    ///////////////////////////////////////////////////////////////////////////
    ReadArgs::ReadArgs(VM& vm, State& s, UInt32 arg_count )
        : StackReader(vm, s)
        , ArgNum(arg_count)
        , CallArgs(GetVM().GetMemoryHeap())
    {
        if (arg_count)
        {
            const UPInt size = GetOpStack().GetSize();

            if (arg_count <= eFixedArrSize)
            {
                unsigned j = 0;
                for (UPInt i = size - arg_count; i < size; ++i, ++j)
                {
                    FixedArr[j] = GetState().GetOpValue(i);
                }
            }
            else
            {
                for (UPInt i = size - arg_count; i < size; ++i)
                {
                    CallArgs.PushBack(GetState().GetOpValue(i));
                }
            }

            GetOpStack().Resize(size - arg_count);
        }

        HaveRead(arg_count);
    }

    ///////////////////////////////////////////////////////////////////////////////
    void State::exec_construct(UInt32 arg_count)
    {
        // Constructed object is never null.

#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_construct, arg_count);
#endif

        GetTracer().PushNewOpCode(Abc::Code::op_construct, arg_count);

        TR::ReadArgsObject args(GetVM(), *this, arg_count);
        const Traits* tr = GetValueTraits(args.ArgObject);

#ifdef SF_AS3_AOTC
        using namespace AOT;

        StringManager& sm = GetVM().GetStringManager();
        SPtr<SNode> sn;

        if (tr)
        {
            SPtr<SNode> sn_name;

            sn_name.Pick(SF_NEW SNodeVar(sm.CreateString(tr->GetName().ToCStr()), tr));
            sn.Pick(SF_NEW SNodeN(GetThis(), SNodeN::opConstruct, sn_name, arg_count, args.GetCallArgs() SF_DEBUG_ARG(OpcodeCP)));
        }
#endif

        // Retrieve InstanceTraits if possible.
        const Value::KindType kind = args.ArgObject.GetKind();
        switch (kind)
        {
        case Value::kInstanceTraits:
#if defined(SF_AS3_AOTC) || defined(SF_AS3_AOTC2)
        case Value::kSNodeIT:
#endif
            PushOp(Value(args.ArgObject.GetInstanceTraits(), Value::NotNull SF_AOTC_ARG(*sn)));
            return;
        case Value::kClassTraits:
#if defined(SF_AS3_AOTC) || defined(SF_AS3_AOTC2)
        case Value::kSNodeCT:
#endif
            PushOp(Value(args.ArgObject.GetClassTraits().GetInstanceTraits(), Value::NotNull SF_AOTC_ARG(*sn)));
            return;
        case Value::kObject:
            {
#ifdef SF_AS3_AOTC
                SPtr<SNode> sn;
                sn.Pick(SF_NEW SNodeVar(sm.CreateString("Throw exception."), tr));
#endif

                Traits* nc_tr = const_cast<Traits*>(tr);
                if (tr->IsInstanceTraits())
                    PushOp(Value(*static_cast<InstanceTraits::Traits*>(nc_tr), Value::NotNull SF_AOTC_ARG(*sn)));
                else
                    PushOp(Value(*static_cast<ClassTraits::Traits*>(nc_tr), Value::NotNull SF_AOTC_ARG(*sn)));
            }
            return;
//         case Value::kObject:
//             PushOp(Value(static_cast<InstanceTraits::Traits&>(args.ArgObject.GetObject()->GetTraits()), Value::NotNull SF_AOTC_ARG(*sn)));
//             return;
        case Value::kClass:
            PushOp(Value(args.ArgObject.AsClass().GetClassTraits().GetInstanceTraits(), Value::NotNull SF_AOTC_ARG(*sn)));
            return;
        default:
            break;
        }

        // Theoretically, we shouldn't be able to reach this point at run-time.
        SF_ASSERT(false);
        PushOp(args.ArgObject);
    }

    void State::exec_constructsuper(UInt32 arg_count)
    {
#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_constructsuper, arg_count);
#endif

        GetTracer().PushNewOpCode(Abc::Code::op_constructsuper, arg_count);

        TR::ReadArgsObject args(GetVM(), *this, arg_count);
    }

    InstanceTraits::Traits& State::GetFunctType(const Value& value) const
    {
        switch (value.GetKind())
        {
        case Value::kThunk:
            return GetVM().GetClassTraitsFunction().GetThunkTraits();
        case Value::kThunkClosure:
            return GetVM().GetClassTraitsFunction().GetThunkFunctionTraits();
        case Value::kVTableInd: // So far kVTableInd is used with MethodInd only.
        case Value::kVTableIndClosure:
            return GetVM().GetClassTraitsFunction().GetVTableTraits();
        default:
            break;
        }

        return GetVM().GetClassTraitsFunction().GetInstanceTraits();
    }

    CheckResult State::GetPropertyType(const Traits& obj_traits, const SlotInfo& si, Value& type SF_AOTC_ARG(SNode& sn)) const
    {
        if (si.IsCode() && si.GetAValueInd().IsValid())
        {
            if (si.GetBindingType() == SlotInfo::BT_Code)
            {
                // Retrieve function type.
                InstanceTraits::Traits& tr = GetFunctType(obj_traits.GetVT().GetValue(si.GetAValueInd()));
                type = Value(tr, CanBeNull(tr) SF_AOTC_ARG(sn));
            }
            else
            {
                // Retrieve function return type.
                InstanceTraits::Traits& tr = GetVM().GetFunctReturnType(obj_traits.GetVT().GetRaw(si.GetAValueInd()), GetFile().GetAppDomain());
                type = Value(tr, CanBeNull(tr) SF_AOTC_ARG(sn));
            }

            return true;
        }

        VM& vm = GetVM();
        const ClassTraits::Traits* ctr = const_cast<SlotInfo&>(si).GetDataType(vm);

        if (ctr == NULL)
        {
            vm.ThrowVerifyError(VM::Error(VM::eClassNotFoundError, vm
                SF_DEBUG_ARG(si.GetDataTypeName(vm))
                ));
            return false;
        }

        if (si.IsClass() || si.IsClassType())
            type = Value(const_cast<ClassTraits::Traits&>(*ctr), Value::NotNull SF_AOTC_ARG(sn));
        else
        {
            InstanceTraits::Traits& itr = ctr->GetInstanceTraits();
            type = Value(itr, CanBeNull(itr) SF_AOTC_ARG(sn));
        }

        return true;
    }

    void State::exec_call(UInt32 arg_count)
    {
        // function is the closure that is being called. receiver is the object to use for the "this" value. This will
        // invoke the [[Call]] property on function with the arguments receiver, arg1, ..., argn. 
        // The result of invoking the [[Call]] property will be pushed onto the stack.

        // ??? Do we need to handle default values of arguments here ???

        GetTracer().PushNewOpCode(Abc::Code::op_call, arg_count);

        TR::ReadArgsObjectValue args(GetVM(), *this, arg_count);

#ifdef SF_AS3_AOTC
        using namespace AOT;

        const Traits* tr = GetValueTraits(args.value);
        SPtr<SNode> sn;

        if (tr)
        {
            SPtr<SNode> sn_name;
            StringManager& sm = GetVM().GetStringManager();

            sn_name.Pick(SF_NEW SNodeVar(sm.CreateString(tr->GetName().ToCStr()), tr));

            Pickable<SNodeN> snn(SF_NEW SNodeN(GetThis(), SNodeN::opCall, sn_name, arg_count, args.GetCallArgs() SF_DEBUG_ARG(OpcodeCP)));
            sn.Pick(snn);

            // ??? Result type ???.
            snn->RType = &GetObjectType();
        }
#endif

#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_call, arg_count);
#endif

        InstanceTraits::Traits& rt = GetVM().GetFunctReturnType(args.value, GetFile().GetAppDomain());
        PushOp(Value(rt, CanBeNull(rt) SF_AOTC_ARG(*sn)));
    }

    void State::exec_sxi1() 
    {
        // Sign extend ... FP10 ...
        // Not documented ...
        // Not implemented yet ...
        GetVM().ThrowVerifyError(VM::Error(VM::eNotImplementedError, GetVM() SF_DEBUG_ARG("exec_sxi1")));
    }

    void State::exec_sxi8() 
    {
        // Sign extend ... FP10 ...
        // Not documented ...
        // Not implemented yet ...
        GetVM().ThrowVerifyError(VM::Error(VM::eNotImplementedError, GetVM() SF_DEBUG_ARG("exec_sxi8")));
    }

    void State::exec_sxi16() 
    {
        // Sign extend ... FP10 ...
        // Not documented ...
        // Not implemented yet ...
        GetVM().ThrowVerifyError(VM::Error(VM::eNotImplementedError, GetVM() SF_DEBUG_ARG("exec_sxi16")));
    }

    void State::exec_applytype(UInt32 arg_count)
    {
#if 0
        // For maniacs.
        if (arg_count == 0)
        {
            return GetVM().ThrowVerifyError(VM::Error(VM::eClassNotFoundError, GetVM()
                SF_DEBUG_ARG(args.ArgMN.GetName(GetFile().GetConstPool()).ToCStr())
                ));
        }
#endif

        GetTracer().PushNewOpCode(Abc::Code::op_applytype, arg_count);

        VM& vm = GetVM();
        TR::ReadArgsObject args(vm, *this, arg_count);

        const Value& arg0 = args.GetCallArgs()[0];

        const ClassTraits::Traits* ctr = NULL;

        switch (arg0.GetKind())
        {
        case Value::kUndefined:
            ctr = &vm.GetClassTraitsObject();
            break;
        case Value::kClass:
            ctr = &arg0.AsClass().GetClassTraits();
            break;
        case Value::kObject:
            if (arg0.IsNull())
                ctr = &vm.GetClassTraitsObject();
            break;
#if defined(SF_AS3_AOTC) || defined(SF_AS3_AOTC2)
        case Value::kSNodeCT:
#endif
        case Value::kClassTraits:
            ctr = &arg0.GetClassTraits();
            break;
        default:
            break;
        }

        const ClassTraits::Traits* v_ctr = &vm.GetClassTraitsObject();

        if (ctr)
        {
            const BuiltinTraitsType tt = ctr->GetTraitsType();
            switch (tt)
            {
            case Traits_SInt:
                v_ctr = &vm.GetClassTraitsVectorSInt();
                break;
            case Traits_UInt:
                v_ctr = &vm.GetClassTraitsVectorUInt();
                break;
            case Traits_Number:
                v_ctr = &vm.GetClassTraitsVectorNumber();
                break;
            case Traits_String:
                v_ctr = &vm.GetClassTraitsVectorString();
                break;
            default:
                v_ctr = &vm.GetClassVector().Resolve2Vector(*ctr);
                break;
            }
        }

#ifdef SF_AS3_AOTC
        using namespace AOT;

        SPtr<SNode> sn;
        sn.Pick(SF_NEW SNodeVar(GetVM().GetStringManager().CreateString(v_ctr->GetName().ToCStr()), v_ctr));
#endif

#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_applytype, arg_count);
#endif

        // Result is a type.
        PushOp(Value(const_cast<ClassTraits::Traits&>(*v_ctr), Value::NotNull SF_AOTC_ARG(*sn)));
    }

    void State::exec_newobject(UInt32 arg_count)
    {
        GetTracer().PushNewOpCode(Abc::Code::op_newobject, arg_count);

        TR::ReadArgs args(GetVM(), *this, arg_count * 2);

#ifdef SF_AS3_AOTC
        using namespace AOT;

        SPtr<SNode> sn;
        sn.Pick(SF_NEW SNodeArgs(SNodeArgs::opNewObject, args.GetCallArgsNum(), args.GetCallArgs() SF_DEBUG_ARG(OpcodeCP)));
#endif

#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_newobject, arg_count);
#endif

        // This is always an object. There is nothing else to do.
        PushOp(Value(GetObjectType(), Value::NotNull SF_AOTC_ARG(*sn)));
    }

    void State::exec_newarray(UInt32 arr_size)
    {
        GetTracer().PushNewOpCode(Abc::Code::op_newarray, arr_size);

        TR::ReadArgs args(GetVM(), *this, arr_size);

#ifdef SF_AS3_AOTC
        using namespace AOT;

        SPtr<SNode> sn;
        sn.Pick(SF_NEW SNodeArgs(SNodeArgs::opNewArray, args.GetCallArgsNum(), args.GetCallArgs() SF_DEBUG_ARG(OpcodeCP)));
#endif

#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_newarray, arr_size);
#endif

        PushOp(Value(GetArrayType(), Value::NotNull SF_AOTC_ARG(*sn)));
    }

    void State::exec_li8() 
    {
        // Load data ... FP10 ...
        // Not documented ...
        // Not implemented yet ...
        GetVM().ThrowVerifyError(VM::Error(VM::eNotImplementedError, GetVM() SF_DEBUG_ARG("exec_li8")));
    }

    void State::exec_li16() 
    {
        // Load data ... FP10 ...
        // Not documented ...
        // Not implemented yet ...
        GetVM().ThrowVerifyError(VM::Error(VM::eNotImplementedError, GetVM() SF_DEBUG_ARG("exec_li16")));
    }

    void State::exec_li32() 
    {
        // Load data ... FP10 ...
        // Not documented ...
        // Not implemented yet ...
        GetVM().ThrowVerifyError(VM::Error(VM::eNotImplementedError, GetVM() SF_DEBUG_ARG("exec_li32")));
    }

    void State::exec_lf32() 
    {
        // Load data ... FP10 ...
        // Not documented ...
        // Not implemented yet ...
        GetVM().ThrowVerifyError(VM::Error(VM::eNotImplementedError, GetVM() SF_DEBUG_ARG("exec_lf32")));
    }

    void State::exec_lf64() 
    {
        // Load data ... FP10 ...
        // Not documented ...
        // Not implemented yet ...
        GetVM().ThrowVerifyError(VM::Error(VM::eNotImplementedError, GetVM() SF_DEBUG_ARG("exec_lf64")));
    }

    void State::exec_si8() 
    {
        // Store data ... FP10 ...
        // Not documented ...
        // Not implemented yet ...
        GetVM().ThrowVerifyError(VM::Error(VM::eNotImplementedError, GetVM() SF_DEBUG_ARG("exec_si8")));
    }

    void State::exec_si16() 
    {
        // Store data ... FP10 ...
        // Not documented ...
        // Not implemented yet ...
        GetVM().ThrowVerifyError(VM::Error(VM::eNotImplementedError, GetVM() SF_DEBUG_ARG("exec_si16")));
    }

    void State::exec_si32()
    {
        // Store data ... FP10 ...
        // Not documented ...
        // Not implemented yet ...
        GetVM().ThrowVerifyError(VM::Error(VM::eNotImplementedError, GetVM() SF_DEBUG_ARG("exec_si32")));
    }

    void State::exec_sf32()
    {
        // Store data ... FP10 ...
        // Not documented ...
        // Not implemented yet ...
        GetVM().ThrowVerifyError(VM::Error(VM::eNotImplementedError, GetVM() SF_DEBUG_ARG("exec_sf32")));
    }

    void State::exec_sf64() 
    {
        // Store data ... FP10 ...
        // Not documented ...
        // Not implemented yet ...
        GetVM().ThrowVerifyError(VM::Error(VM::eNotImplementedError, GetVM() SF_DEBUG_ARG("exec_sf64")));
    }

    void State::exec_newfunction(UInt32 method_ind)
    {
        GetTracer().PushNewOpCode(Abc::Code::op_newfunction, method_ind);

#if defined(SF_AS3_AOTC)
        Instances::fl::GlobalObjectScript& gos = GetTracer().GetGlobalObjectScript();
        InstanceTraits::Function& itr = GetFile().GetFunctionInstanceTraits(gos, method_ind);
        SPtr<SNode> sn;
        sn.Pick(SF_NEW AOT::SNodeArgs(AOT::SNodeArgs::opNewFunction, &itr SF_DEBUG_ARG(OpcodeCP)));

        GetTracer().GetInfoCollector().UsedFunctions.Add(&itr);

        PushOp(Value(itr, Value::NotNull, *sn));
#elif defined(SF_AS3_AOTC2)
        Instances::fl::GlobalObjectScript& gos = GetTracer().GetGlobalObjectScript();
        InstanceTraits::Function& itr = GetFile().GetFunctionInstanceTraits(gos, method_ind);

        GetTracer().GetInfoCollector().UsedFunctions.Add(&itr);

        PushSNodeOC(AOT::SNodeOC::op_newfunction, method_ind);

        InstanceTraits::Traits& rt = GetFunctionType();
        PushOp(Value(rt, Value::NotNull));
#else
        InstanceTraits::Traits& rt = GetFunctionType();
        PushOp(Value(rt, Value::NotNull));
#endif
    }

    void State::exec_getdescendants(UInt32 mn_index)
    {
#ifdef GFX_ENABLE_XML
        if (!GetVM().GetXMLSupport().IsEnabled())
            return GetVM().ThrowVerifyError(VM::Error(VM::eNotImplementedError, GetVM() SF_DEBUG_ARG("getdescendants")));

        GetTracer().PushNewOpCode(Abc::Code::op_getdescendants, mn_index);

        TR::ReadMnObject args(GetFile(), *this, mn_index);

#ifdef SF_AS3_AOTC
        using namespace AOT;

        SPtr<SNode> sn;
        sn.Pick(SF_NEW SNodeName("getdescendants"));
#endif

#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_getdescendants, mn_index);
#endif

        PushOp(Value(GetXMLListType(), Value::NotNull SF_AOTC_ARG(*sn)));
#else
        SF_UNUSED1(mn_index);
        GetVM().ThrowVerifyError(VM::Error(VM::eNotImplementedError, GetVM() SF_DEBUG_ARG("getdescendants")));
#endif
    }

    void State::exec_newcatch(UInt32 v)
    {
        GetTracer().PushNewOpCode(Abc::Code::op_newcatch, v);

#if defined(SF_AS3_AOTC)
        VM& vm = GetVM();
        const Abc::MethodBodyInfo::Exception& e = GetTracer().GetMethodBodyInfo().GetException();
        const Abc::MethodBodyInfo::ExceptionInfo& ei = e.Get(v);
        Classes::fl::Catch& cl = (Classes::fl::Catch&)vm.GetClassTraitsCatch().GetInstanceTraits().GetClass();
        SPtr<InstanceTraits::fl::Catch> tr = cl.MakeInstanceTraits(GetFile(), ei);
        // ??? Do we really need to store InstanceTraits::fl::Catch separately?
        pTracer->CatchTraits.PushBack(tr);

        PushOp(Value(*tr, Value::NotNull));
#elif defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_newcatch, v);

        // There is no need for precise type of the catch object because
        // abc pCode will address data using slot numbers.
        PushOp(Value(GetObjectType(), Value::NotNull));
#else
        // There is no need for precise type of the catch object because
        // abc pCode will address data using slot numbers.
        PushOp(Value(GetObjectType(), Value::NotNull));
#endif
    }

    void State::FindProp(PropRef& result, const Multiname& mn, ScopeType& stype, UPInt& scope_index)
    {
        // Ignore run-time multinames for the time being.
        if (mn.IsRunTime() || mn.IsNameLate())
            return;

        VM& vm = GetVM();
        UPInt slot_index = 0;

        // 1) Check scope stack.
        {
            for (UPInt i = GetScopeStack().GetSize(); i > 0; --i)
            {
                scope_index = i - 1;
                const Value& v = GetScopeStack()[scope_index];
                const Traits* tr = GetValueTraits(v);

                if (v.GetWith())
                    return;

                // Skip global object.
                if (tr && !tr->IsGlobal())
                {
                    const SlotInfo* si = FindFixedSlot(vm, *tr, mn, slot_index, NULL);
                    if (si)
                    {
                        stype = stScopeStack;
                        result = PropRef(v, si, slot_index);
                        return;
                    }
                }
            }
        }

        // 2) Look in Saved Scope.
        // Saved Scope has classes, Activation objects, regular object, and a global object.
        // No primitive types so far.
        {
            const ValueArrayDH& ss = GetSavedScope();
            for (UPInt i = ss.GetSize(); i > 0 ; --i)
            {
                scope_index = i - 1;
                const Value& v = ss[scope_index];
                const Traits& tr = vm.GetValueTraits(v);

                if (v.GetWith())
                    return;

                /* Search in global objects has the same pCode.
                if (tr.IsGlobal())
                    // We will deal with global objects later.
                    continue;
                */

                const SlotInfo* si = FindFixedSlot(vm, tr, mn, slot_index, NULL);

                if (si)
                {
                    // We found it

                    stype = stStoredScope;
                    result = PropRef(v, si, slot_index);
                    return;
                }
            }
        }

        // Check global objects.

        // We are looking for classes.
        // We know about all registered class objects.
//             {
//                 const ClassTraits::Traits* ctr = vm.GetRegisteredClassTraits(mn);
//                 if (ctr && ctr->IsValid() && ctr->GetInstanceTraits().HasConstructorSetup())
//                 {
//                     // Not correct. This should be a global object.
//                     // This is just a class.
//                     value = Value(&ctr->GetInstanceTraits().GetClass());
// 
//                     return stGlobalObject;
//                 }
//             }

        if (!mn.GetName().IsTraits())
        {
#if 0
            // Old way of doing things.
            const ClassTraits::Traits* ctr = vm.GetRegisteredClassTraits(mn);
#else
            // New way. Should be stable.
            const ClassTraits::Traits* ctr = FindClassTraits(vm, mn, GetFile().GetAppDomain());
#endif

#if 1 // Experimental
            // This shouldn't cause any problem.
            if (ctr)
            {
                if (ctr->IsValid())
                {
                    // This is just a class.

                    InstanceTraits::Traits& itr = ctr->GetInstanceTraits();
                    AS3::Object* obj = NULL;

                    if (itr.IsAbcObject())
                    {
                        // Class will be created later. Just get a slot.
                        InstanceTraits::UserDefined& ud = static_cast<InstanceTraits::UserDefined&>(itr);
                        obj = &ud.GetScript();
                    }
                    else
                        obj = &vm.GetGlobalObjectCPP();

                    SF_ASSERT(obj);
                    UPInt slot_index_traits = 0;
                    const SlotInfo* si = AS3::FindFixedSlot(vm, obj->GetTraits(), mn, slot_index_traits, obj);

                    if (si)
                    {
                        result = PropRef(obj, si, slot_index_traits);
                        stype = stGlobalObject;
                        return;
                    }
                    else
#ifdef SF_AS3_CLASS_AS_SLOT
                        SF_ASSERT(false);
#else
                        // There is no slot for Class any more.
                        return;
#endif
                }
                else
                    SF_ASSERT(false);
            }
#endif

            FindGOProperty(result, vm, vm.GetGlobalObjects(), mn, ctr);
            if (result)
            {
                stype = stGlobalObject;
                return;
            }
        }
    }

    void State::exec_findpropstrict(UInt32 mn_index)
    {
        GetTracer().PushNewOpCode(Abc::Code::op_findpropstrict, mn_index);

        TR::ReadMn args(GetFile(), *this, mn_index);

        UPInt scope_index;
        ScopeType stype;
        PropRef prop;

        FindProp(prop, args.ArgMN, stype, scope_index);

        if (prop)
        {
            PushOp(prop.GetThis());
            return;
        }

#ifdef SF_AS3_AOTC
        using namespace AOT;

        SPtr<SNode> sn;
        // sn.Pick(SF_NEW SNodeVar(GetVM().GetStringManager().CreateConstString("global")));
        // This should be variable, not value.
        sn.Pick(SF_NEW SNodeValue(args.ArgMN.GetName()));
#endif

#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_findpropstrict, mn_index);
#endif

        // In case of null we should get an exception.
        PushOp(Value(GetObjectType(), Value::NotNull SF_AOTC_ARG(*sn)));
    }

    void State::exec_findproperty(UInt32 mn_index)
    {
        GetTracer().PushNewOpCode(Abc::Code::op_findproperty, mn_index);

        TR::ReadMn args(GetFile(), *this, mn_index);

        UPInt scope_index;
        ScopeType stype;
        PropRef prop;

        FindProp(prop, args.ArgMN, stype, scope_index);

        if (prop)
        {
            PushOp(prop.GetThis());
            return;
        }

#ifdef SF_AS3_AOTC
        using namespace AOT;

        SPtr<SNode> sn;
        // sn.Pick(SF_NEW SNodeVar(GetVM().GetStringManager().CreateConstString("global")));
        // This should be variable, not value.
        sn.Pick(SF_NEW SNodeValue(args.ArgMN.GetName()));
#endif

#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_findproperty, mn_index);
#endif

        // Can be null.
        PushOp(Value(GetObjectType(), Value::NullOrNot SF_AOTC_ARG(*sn)));
    }

    void State::exec_getlex(UInt32 mn_index)
    {
#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_getlex, mn_index);
#endif

        GetTracer().PushNewOpCode(Abc::Code::op_getlex, mn_index);

        //TR::ReadMnCt args(GetFile(), *this, mn_index); // ReadMnCt() doesn't change stack state.

        // We can do better than this.
        PushOp(Value(GetObjectType(), Value::NotNull));
    }

    void State::exec_getabsobject(Abc::Code::OpCode opcode, InstanceTraits::Traits& tr)
    {
        // It is a replacement opcode, so, we do not have to store mn_index.
        //TR::ReadMnCt args(GetFile(), *this, mn_index); // ReadMnCt() doesn't change stack state.

        GetTracer().PushNewOpCode(opcode);
        PushOp(Value(tr, Value::NotNull));
    }

    void State::exec_getlocal(UInt32 reg_ind) 
    {
#if defined(SF_AS3_AOTC2)
        PushSNodeGetLocal(reg_ind);
#endif

        GetTracer().PushNewOpCode(Abc::Code::op_getlocal, reg_ind);
        const Value& reg_value = GetRegister(AbsoluteIndex(reg_ind));

        PushOp(reg_value);
    }

    void State::exec_newactivation()
    {
#if defined(SF_AS3_AOTC2)
        const Abc::MbiInd mbi_ind = GetTracer().GetMethodInfo().GetMethodBodyInfoInd();
        PushSNodeOC(AOT::SNodeOC::op_newactivation, mbi_ind.Get());
#endif

        GetTracer().PushNewOpCode(Abc::Code::op_newactivation);
        InstanceTraits::Traits& it = GetFile().GetActivationInstanceTraits(GetTracer().GetMethodBodyInd() SF_DEBUG_ARG(GetTracer().GetName()));

        PushOp(Value(it, Value::NotNull));
    }

    void State::exec_pushbyte(SInt32 v)
    {
#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_pushbyte, v);
#endif

        GetTracer().PushNewOpCode(Abc::Code::op_pushbyte, v);

        // The value is promoted to an int.
        PushOp(Value(SInt32(SInt8(v))));
    }

    void State::exec_pushshort(SInt32 v)
    {
#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_pushshort, v);
#endif

        GetTracer().PushNewOpCode(Abc::Code::op_pushshort, v);

        PushOp(Value(v));
    }

    void State::exec_pushstring(SInt32 v)
    {
#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_pushstring, v);
#endif
        VMAbcFile& file = GetFile();
        const StringDataPtr strView = file.GetConstPool().GetString(AbsoluteIndex(v));

#if 0
        const ASString str = file.GetInternedString(v);
#else
        const ASString str = GetVM().GetStringManager().CreateString(strView.ToCStr(), strView.GetSize());
#endif

#ifdef SF_AS3_STORE_PRIMITIVE_VALUES_IN_BYTECODE
        // Store reference to a string.
        file.ASStringNodeSet.Set(str.GetNode());
        // Store pointer to a string.
        GetTracer().PushNewOpCode(Abc::Code::op_pushstring, reinterpret_cast<UPInt>(str.GetNode()));
#else
        GetTracer().PushNewOpCode(Abc::Code::op_pushstring, v);
#endif

        PushOp(Value(str));
    }

    void State::exec_pushint(SInt32 v)
    {
#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_pushint, v);
#endif

        const SInt32 value = GetFile().GetConstPool().GetInt(v);

#ifdef SF_AS3_STORE_PRIMITIVE_VALUES_IN_BYTECODE
        GetTracer().PushNewOpCode(Abc::Code::op_pushint, value);
#else
        GetTracer().PushNewOpCode(Abc::Code::op_pushint, v);
#endif

        PushOp(Value(value));
    }

    void State::exec_pushuint(SInt32 v)
    {
#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_pushuint, v);
#endif

        const UInt32 value = GetFile().GetConstPool().GetUInt(v);

#ifdef SF_AS3_STORE_PRIMITIVE_VALUES_IN_BYTECODE
        GetTracer().PushNewOpCode(Abc::Code::op_pushuint, value);
#else
        GetTracer().PushNewOpCode(Abc::Code::op_pushuint, v);
#endif

        PushOp(Value(value));
    }

    void State::exec_pushdouble(SInt32 v)
    {
#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_pushdouble, v);
#endif

        const Value::Number value = MakeValueNumber(GetFile().GetConstPool().GetDouble(v));

#ifdef SF_AS3_STORE_PRIMITIVE_VALUES_IN_BYTECODE
        GetTracer().PushNewOpCodeNumber(Abc::Code::op_pushdouble, value);
#else
        GetTracer().PushNewOpCode(Abc::Code::op_pushdouble, v);
#endif

        PushOp(Value(value));
    }

    void State::exec_hasnext2(UInt32 object_reg, UInt32 index_reg)
    {
        GetTracer().PushNewOpCode(Abc::Code::op_hasnext2, object_reg, index_reg);

        AbsoluteIndex reg_ind(index_reg); // Value must be of type int.
        //AbsoluteIndex obj_ind(object_reg); // Can be set to null.

        // A kind of verification.
        // reg_ind can be undefined.
        SF_ASSERT(GetRegister(reg_ind).IsUndefined() || GetValueTraits(GetRegister(reg_ind)) == &GetSIntType());

#ifdef SF_AS3_AOTC
        using namespace AOT;

        SPtr<SNode> sn;
        sn.Pick(SF_NEW SNodeName("hasnext2"));
#endif

#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_hasnext2, object_reg, index_reg);
#endif

        PushOp(Value(GetBooleanType(), Value::NotNull SF_AOTC_ARG(*sn)));
    }

    void State::exec_nextname()
    {
        PopOp(); // index: int
        Value obj = PopOpValue();
        const Traits* tr = GetValueTraits(obj);

#ifdef SF_AS3_AOTC
        using namespace AOT;

        SPtr<SNode> sn;
        sn.Pick(SF_NEW SNodeName("nextname"));
#endif

        GetTracer().PushNewOpCode(Abc::Code::op_nextname);

#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_nextname);
#endif

        // In general case name can be of any type.

        // In case of a Dictionary name can be of any type.
        if (tr->GetTraitsType() == Traits_Dictionary && tr->IsInstanceTraits())
        {
            // General case.
            PushOp(Value(GetObjectType(), Value::NullOrNot SF_AOTC_ARG(*sn))); // name
            return;
        }

        // In case of Array or Vector name is either UInt or String.
        // String is mostly a theoretical case.
        if (tr->IsArrayLike())
        {
            PushOp(Value(GetUIntType(), Value::NotNull SF_AOTC_ARG(*sn))); // name
            return;
        }

        // In case of an Object name will be a String.
        // String can be null.
        PushOp(Value(GetStringType(), Value::NullOrNot SF_AOTC_ARG(*sn))); // name
    }

    void State::exec_hasnext()
    {
        GetTracer().PushNewOpCode(Abc::Code::op_hasnext);

#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_hasnext);
#endif

        PopOp(); // cur_index: int
        PopOp(); // obj

        PushOp(GetSIntType(), Value::NotNull); // next_index
    }

    void State::exec_pushundefined()
    {
        GetTracer().PushNewOpCode(Abc::Code::op_pushundefined);

#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_pushundefined);
#endif
        // Tamarin style.
        //PushOp(Value(GetVoidType()));
        PushOp(Value::GetUndefined());
    }

    void State::exec_nextvalue()
    {
        GetTracer().PushNewOpCode(Abc::Code::op_nextvalue);

#if defined(SF_AS3_AOTC)
        using namespace AOT;

        Value index = PopOpValue(); // index: int
        Value obj = PopOpValue(); // obj: Value
        SPtr<SNode> sn;
        sn.Pick(SF_NEW SNode2(obj, SNode2::opNextValue, index SF_DEBUG_ARG(OpcodeCP)));

        PushOp(Value(GetObjectType(), Value::NotNull, *sn)); // next_value: Value, not null.
#elif defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_nextvalue);
        {
            PopOp(); // index: int
            PopOp(); // obj: Value

            PushOp(Value(GetObjectType(), Value::NotNull)); // next_value: Value, not null.
        }
#else
        PopOp(); // index: int
        PopOp(); // obj: Value

        PushOp(Value(GetObjectType(), Value::NotNull)); // next_value: Value, not null.
#endif
    }

    void State::exec_convert_o() 
    {
        GetTracer().PushNewOpCode(Abc::Code::op_convert_o);

        // Error if value.IsNullOrUndefined() || !value.IsObjectStrict()
    }

    void State::exec_checkfilter() 
    {
        GetTracer().PushNewOpCode(Abc::Code::op_checkfilter);

#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_checkfilter);
#endif
        //GetVM().ThrowVerifyError(VM::Error(VM::eNotImplementedError, GetVM() DEBUG_ARG("exec_checkfilter")));
        // Nothing happens.
    }

    void State::exec_coerce_s()
    {
        GetTracer().PushNewOpCode(Abc::Code::op_coerce_s);

        if (BackOpValue().IsString())
            return;

        exec_1OpString(SF_AOTC_CODE(AOT::SNode1::opCoerceS) SF_AOTC2_CODE(AOT::SNodeOC::op_coerce_s));
    }

    void State::exec_inclocal(UInt32 v)
    {
        GetTracer().PushNewOpCode(Abc::Code::op_inclocal);

#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_inclocal, v);
#endif
        ConvertRegisterTo(AbsoluteIndex(v), GetNumberType(), Value::NotNull);
    }

    void State::exec_declocal(UInt32 v)
    {
        GetTracer().PushNewOpCode(Abc::Code::op_declocal);

#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_declocal, v);
#endif
        ConvertRegisterTo(AbsoluteIndex(v), GetNumberType(), Value::NotNull);
    }

    void State::exec_not()
    {
        RefineOpCodeStack1(Abc::Code::op_not, GetBooleanType(), Abc::Code::op_not_tb SF_AOTC_ARG(AOT::SNode1::opNot));
    }

    void State::exec_negate()
    {
        // Result type of "negate" should always be number.
        RefineOpCodeStack1(Abc::Code::op_negate, GetNumberType(), Abc::Code::op_negate_td SF_AOTC_ARG(AOT::SNode1::opNegate));
    }

    void State::exec_increment_i()
    {
        RefineOpCodeStack1(Abc::Code::op_increment_i, GetSIntType(), Abc::Code::op_increment_ti SF_AOTC_ARG(AOT::SNode1::opIncr));
    }

    void State::exec_decrement_i()
    {
        RefineOpCodeStack1(Abc::Code::op_decrement_i, GetSIntType(), Abc::Code::op_decrement_ti SF_AOTC_ARG(AOT::SNode1::opDecr));
    }

    void State::exec_negate_i()
    {
        RefineOpCodeStack1(Abc::Code::op_negate_i, GetSIntType(), Abc::Code::op_negate_ti SF_AOTC_ARG(AOT::SNode1::opNegate));
    }

    void State::exec_add_d()
    {
        RefineOpCodeStack2(Abc::Code::op_add_d, GetNumberType(), Abc::Code::op_add_td SF_AOTC_ARG(AOT::SNode2::opAdd));
    }

    void State::exec_add_i()
    {
        RefineOpCodeStack2(Abc::Code::op_add_i, GetSIntType(), Abc::Code::op_add_ti SF_AOTC_ARG(AOT::SNode2::opAdd));
    }

    void State::exec_subtract()
    {
        RefineOpCodeStack2(Abc::Code::op_subtract, GetNumberType(), Abc::Code::op_subtract_td SF_AOTC_ARG(AOT::SNode2::opSub));
    }

    void State::exec_subtract_i()
    {
        RefineOpCodeStack2(Abc::Code::op_subtract_i, GetSIntType(), Abc::Code::op_subtract_ti SF_AOTC_ARG(AOT::SNode2::opSub));
    }

    void State::exec_multiply()
    {
        RefineOpCodeStack2(Abc::Code::op_multiply, GetNumberType(), Abc::Code::op_multiply_td SF_AOTC_ARG(AOT::SNode2::opMul));
    }

    void State::exec_multiply_i()
    {
        RefineOpCodeStack2(Abc::Code::op_multiply_i, GetSIntType(), Abc::Code::op_multiply_ti SF_AOTC_ARG(AOT::SNode2::opMul));
    }

    void State::exec_divide()
    {
        RefineOpCodeStack2(Abc::Code::op_divide, GetNumberType(), Abc::Code::op_divide_td SF_AOTC_ARG(AOT::SNode2::opDiv));
    }

    void State::exec_1OpSInt(SF_AOTC_CODE(AOT::SNode1::OP op) SF_AOTC2_CODE(AOT::SNodeOC::OP op))
    {
#ifdef SF_AS3_AOTC
        using namespace AOT;

        SPtr<SNode> sn;
        sn.Pick(SF_NEW SNode1(BackOpValue(), op SF_DEBUG_ARG(OpcodeCP)));
#endif

#if defined(SF_AS3_AOTC2)
        PushSNodeOC(op);
#endif

        ConvertOpTo(GetSIntType(), Value::NotNull SF_AOTC_ARG(*sn));
    }

    void State::exec_1OpNumber(SF_AOTC_CODE(AOT::SNode1::OP op) SF_AOTC2_CODE(AOT::SNodeOC::OP op))
    {
#ifdef SF_AS3_AOTC
        using namespace AOT;

        SPtr<SNode> sn;
        sn.Pick(SF_NEW SNode1(BackOpValue(), op SF_DEBUG_ARG(OpcodeCP)));
#endif

#if defined(SF_AS3_AOTC2)
        PushSNodeOC(op);
#endif

        ConvertOpTo(GetNumberType(), Value::NotNull SF_AOTC_ARG(*sn));
    }

    void State::exec_1OpString(SF_AOTC_CODE(AOT::SNode1::OP op) SF_AOTC2_CODE(AOT::SNodeOC::OP op))
    {
#ifdef SF_AS3_AOTC
        using namespace AOT;

        SPtr<SNode> sn;
        sn.Pick(SF_NEW SNode1(BackOpValue(), op SF_DEBUG_ARG(OpcodeCP)));
#endif

#if defined(SF_AS3_AOTC2)
        PushSNodeOC(op);
#endif

        ConvertOpTo(GetStringType(), Value::NullOrNot SF_AOTC_ARG(*sn));
    }

    void State::exec_2OpNumber(SF_AOTC_CODE(AOT::SNode2::OP op))
    {
        const Value _2 = PopOpValue(); // value2

#ifdef SF_AS3_AOTC
        using namespace AOT;

        SPtr<SNode> sn;
        sn.Pick(SF_NEW SNode2(BackOpValue(), op, _2 SF_DEBUG_ARG(OpcodeCP)));
#endif

#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::GetOP(GetTracer().GetNewOpCode()));
#endif

        ConvertOpTo(GetNumberType(), Value::NotNull SF_AOTC_ARG(*sn));
    }

    void State::exec_2OpSInt(SF_AOTC_CODE(AOT::SNode2::OP op))
    {
        const Value _2 = PopOpValue(); // value2

#ifdef SF_AS3_AOTC
        using namespace AOT;

        SPtr<SNode> sn;
        sn.Pick(SF_NEW SNode2(BackOpValue(), op, _2 SF_DEBUG_ARG(OpcodeCP)));
#endif

#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::GetOP(GetTracer().GetNewOpCode()));
#endif

        ConvertOpTo(GetSIntType(), Value::NotNull SF_AOTC_ARG(*sn));
    }

    void State::exec_2OpUInt(SF_AOTC_CODE(AOT::SNode2::OP op))
    {
        const Value _2 = PopOpValue(); // value2

#ifdef SF_AS3_AOTC
        using namespace AOT;

        SPtr<SNode> sn;
        sn.Pick(SF_NEW SNode2(BackOpValue(), op, _2 SF_DEBUG_ARG(OpcodeCP)));
#endif

#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::GetOP(GetTracer().GetNewOpCode()));
#endif

        ConvertOpTo(GetUIntType(), Value::NotNull SF_AOTC_ARG(*sn));
    }

    void State::exec_2OpBoolean(SF_AOTC_CODE(AOT::SNodeBoolExpr::OP op))
    {
        const Value _2 = PopOpValue(); // value2

#ifdef SF_AS3_AOTC
        using namespace AOT;

        SPtr<SNode> sn;
        sn.Pick(SF_NEW SNodeBoolExpr(op, BackOpValue(), _2 SF_DEBUG_ARG(OpcodeCP)));
#endif

#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::GetOP(GetTracer().GetNewOpCode()));
#endif

        ConvertOpTo(GetBooleanType(), Value::NotNull SF_AOTC_ARG(*sn));
    }

    void State::exec_istype(UInt32 mn_index)
    {
        GetTracer().PushNewOpCode(Abc::Code::op_istype, mn_index);

        TR::ReadMnCt args(GetFile(), *this, mn_index);

        ConvertOpTo(GetBooleanType(), Value::NotNull);
    }

    void State::exec_pushscope()
    {
        GetTracer().PushNewOpCode(Abc::Code::op_pushscope);

#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_pushscope);
#endif
        // A TypeError is thrown if value is null or undefined.

        // It is safe to use BackOpValue() this way because op stack won't be midified.
        PushScope(BackOpValue());
        PopOp();
    }

    void State::exec_getscopeobject(UInt32 scope_index) 
    {
#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_getscopeobject, scope_index);
#endif

        GetTracer().PushNewOpCode(Abc::Code::op_getscopeobject, scope_index);

        PushOp(GetScopeValue(scope_index));
        // Clean up the *with* flag.
        GetOpStack().Back().SetWith(false);
    }

    void State::exec_deleteproperty(UInt32 mn_index)
    {
        GetTracer().PushNewOpCode(Abc::Code::op_deleteproperty, mn_index);

        TR::ReadMnObject args(GetFile(), *this, mn_index);

#ifdef SF_AS3_AOTC
        using namespace AOT;

        SPtr<SNode> sn;
        SPtr<SNode> name = AotGetProperty(args.ArgObject, args.ArgMN);
        if (name.GetPtr() == NULL)
            name.Pick(SF_NEW AOT::SNodeName("some property"));
        sn.Pick(SF_NEW SNode1(name, SNode1::opDelete SF_DEBUG_ARG(OpcodeCP)));
#endif

#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_deleteproperty, mn_index);
#endif

        PushOp(Value(GetBooleanType(), Value::NotNull SF_AOTC_ARG(*sn)));
    }

    void State::exec_getslot(UInt32 slot_index, bool simulate)
    {
#if defined(SF_AS3_AOTC2)
        if (!simulate)
            PushSNodeOC(AOT::SNodeOC::op_getslot, slot_index);
#endif

        if (!simulate)
            GetTracer().PushNewOpCode(Abc::Code::op_getslot, slot_index);

        TR::ReadObject args(GetVM(), *this);

        Tracer& tracer = GetTracer();
        const Traits* vtr = tracer.GetValueTraits(args.ArgObject);

        if (vtr)
        {
            /*
            // Original pCode
            // Slot's indices begin from ONE.
            if (slot_index <= vtr->GetSlots().GetSize())
            {
                const ClassTraits::Traits& ctr = tracer.GetSlotCTraits(*vtr, SlotIndex(slot_index));

                PushOp(Value(ctr.GetInstanceTraits()));
            }
            else
            {
                // !!! Check that vtr is a dynamic type.
                PushOp(Value(GetObjectType()));
            }

            return;
            */

            // Slot's indices begin from ONE.
//             if (slot_index <= vtr->GetSlots().GetSize())
//             {
//                 const SlotInfo& si = vtr->GetSlotInfo(AbsoluteIndex(SlotIndex(slot_index), *vtr));
// 
//                 /* 
//                 // Attempt 1.
//                 // Class is not initialized yet because script wasn't executed yet.
//                 if (si.IsClassOrConst() && args.ArgObject.IsObject())
//                 {
//                     // In case this is a class or a const we can get a real value.
//                     // It is already initialized.
// 
//                     Value value;
// 
//                     if (si.GetSlotValueUnsafe(GetVM(), value, args.ArgObject.GetObject()))
//                     {
//                         PushOp(value);
//                         return;
//                     }
//                 }
//                 */
// 
//                 /*
//                 // Attempt 2.
//                 // Class is not initialized yet because script wasn't executed yet.
//                 if (si.IsClass())
//                 {
//                     const ClassTraits::Traits& ctr = tracer.GetSlotCTraits(*vtr, SlotIndex(slot_index));
// 
//                     PushOp(Value(&ctr.GetInstanceTraits().GetClass()));
//                     return;
//                 }
//                 else
//                 {
//                     // Use InstanceTraits.
// 
//                     const ClassTraits::Traits& ctr = tracer.GetSlotCTraits(*vtr, SlotIndex(slot_index));
// 
//                     PushOp(Value(ctr.GetInstanceTraits()));
//                     return;
//                 }
//                 */
//             }
// 
//             // !!! Check that vtr is a dynamic type.
//             PushOp(Value(GetObjectType()));
// 
//             return;

            // Slot's indices begin from ONE.
            if (slot_index <= vtr->GetSlotInfoNum())
            {
                const AbsoluteIndex aind = AbsoluteIndex(SlotIndex(slot_index), *vtr);

#ifdef SF_AS3_AOTC
                using namespace AOT;

                SPtr<SNode> sn = AotGetSlot(args.ArgObject, aind);
#endif

                const SlotInfo& si = vtr->GetSlotInfo(aind);
                const ClassTraits::Traits* ctr = tracer.GetSlotCTraits(*vtr, SlotIndex(slot_index));

                if (ctr == NULL)
                {
                    return GetVM().ThrowVerifyError(VM::Error(VM::eClassNotFoundError, GetVM()
                        SF_DEBUG_ARG(si.GetDataTypeName(GetVM()))
                        ));
                }

                InstanceTraits::Traits& itr = ctr->GetInstanceTraits();
                if (si.IsClass())
                    PushOp(Value(const_cast<ClassTraits::Traits&>(*ctr), CanBeNull(itr) SF_AOTC_ARG(*sn)));
                else
                    PushOp(Value(itr, CanBeNull(itr) SF_AOTC_ARG(*sn)));
            }
            else
            {
                // !!! Check that vtr is a dynamic type.
                PushOp(Value(GetObjectType(), Value::NullOrNot));
            }

            return;
        }

        PushOp(Value());
    }

    void State::exec_setlocal(UInt32 rnum)
    {
        GetTracer().PushNewOpCode(Abc::Code::op_setlocal, rnum);

        Value& value = BackOpValue();

#ifdef SF_AS3_AOTC
        AotUpdateRegisterValue(rnum, value);
#endif

#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_setlocal, rnum);
#endif

        SetRegister(AbsoluteIndex(rnum), value);
        PopOp();
    }

    void State::exec_inclocal_i(UInt32 v)
    {
        RefineOpCodeReg1(Abc::Code::op_inclocal_i, GetSIntType(), Abc::Code::op_inclocal_ti, v);
    }

    void State::exec_declocal_i(UInt32 v) 
    {
        RefineOpCodeReg1(Abc::Code::op_declocal_i, GetSIntType(), Abc::Code::op_declocal_ti, v);
    }

    void State::exec_getlocal0() 
    {
        GetTracer().PushNewOpCode(Abc::Code::op_getlocal0);

#if defined(SF_AS3_AOTC2)
        PushSNodeGetLocal(0);
#endif

        const Value& reg_value = GetRegister(AbsoluteIndex(0));

        PushOp(reg_value);
    }

    void State::exec_getlocal1()
    {
        GetTracer().PushNewOpCode(Abc::Code::op_getlocal1);

#if defined(SF_AS3_AOTC2)
        PushSNodeGetLocal(1);
#endif

        const Value& reg_value = GetRegister(AbsoluteIndex(1));

        PushOp(reg_value);
    }   

    void State::exec_getlocal2()
    {
        GetTracer().PushNewOpCode(Abc::Code::op_getlocal2);

#if defined(SF_AS3_AOTC2)
        PushSNodeGetLocal(2);
#endif

        const Value& reg_value = GetRegister(AbsoluteIndex(2));

        PushOp(reg_value);
    }

    void State::exec_getlocal3()
    {
        GetTracer().PushNewOpCode(Abc::Code::op_getlocal3);

#if defined(SF_AS3_AOTC2)
        PushSNodeGetLocal(3);
#endif

        const Value& reg_value = GetRegister(AbsoluteIndex(3));

        PushOp(reg_value);
    }

    void State::exec_setlocal0()
    {
        Value& value = BackOpValue();

#ifdef SF_AS3_AOTC
        AotUpdateRegisterValue(0, value);
#endif

        GetTracer().PushNewOpCode(Abc::Code::op_setlocal0);

#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_setlocal, 0);
#endif

        SetRegister(AbsoluteIndex(0), value);
        PopOp();
    }

    void State::exec_setlocal1()
    {
        Value& value = BackOpValue();

#ifdef SF_AS3_AOTC
        AotUpdateRegisterValue(1, value);
#endif

        GetTracer().PushNewOpCode(Abc::Code::op_setlocal1);

#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_setlocal, 1);
#endif

        SetRegister(AbsoluteIndex(1), value);
        PopOp();
    }

    void State::exec_setlocal2()
    {
        Value& value = BackOpValue();

#ifdef SF_AS3_AOTC
        AotUpdateRegisterValue(2, value);
#endif

        GetTracer().PushNewOpCode(Abc::Code::op_setlocal2);

#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_setlocal, 2);
#endif

        SetRegister(AbsoluteIndex(2), value);
        PopOp();
    }

    void State::exec_setlocal3()
    {
        Value& value = BackOpValue();

#ifdef SF_AS3_AOTC
        AotUpdateRegisterValue(3, value);
#endif

        GetTracer().PushNewOpCode(Abc::Code::op_setlocal3);

#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_setlocal, 3);
#endif

        SetRegister(AbsoluteIndex(3), value);
        PopOp();
    }

    void State::ConvertOpTo(InstanceTraits::Traits& tr, Value::TraceNullType isNull)
    {
        Value& value = OpStack.Back();

#ifdef SF_AS3_AOTC
        if (value.IsSNode())
            value.UpdateTraits(tr);
        else
            value = Value(tr, isNull);
#else
        value = Value(tr, isNull);
#endif
    }

#ifdef SF_AS3_AOTC
    bool State::IsInternalVMType(const Traits& tr)
    {
        bool result = false;

        switch (tr.GetTraitsType())
        {
        case Traits_Activation:
        case Traits_Catch:
            result = true;
        default:
            break;
        }

        return result;
    }

    void State::AotUpdateRegisterValue(UInt32 rnum, Value& value) const
    {
        using namespace AOT;

        const Traits* tr = GetValueTraits(value);

        // We do not need to track internal VM types.
        if (tr && !IsInternalVMType(*tr))
        {
            SPtr<SNodeRT> rt = GetTracer().GetSNodeRT(rnum, tr SF_DEBUG_ARG(OpcodeCP));
            Pickable<SNode> sn(SF_NEW SNodeRTV(rt, value SF_DEBUG_ARG(OpcodeCP)));

            PushSNode(sn);

            const Value::TraceNullType null = CanBeNull(value);
            if (value.IsSNode())
                value.UpdateSNode(*sn, null);
            else
            {
                Traits* tr = const_cast<Traits*>(GetTracer().GetValueTraits(value));
                if (tr)
                {
                    if (tr->IsClassTraits())
                        value = Value(static_cast<ClassTraits::Traits&>(*tr), null, *sn);
                    else
                        value = Value(static_cast<InstanceTraits::Traits&>(*tr), null, *sn);
                }
                else
                    SF_ASSERT(false);
            }
        }
    }

    void State::aot_reg_op(Value& reg, AOT::SNode1::OP op) const
    {
        Pickable<SNode> sn(SF_NEW AOT::SNode1(reg, op SF_DEBUG_ARG(OpcodeCP)));
        reg.UpdateSNode(*sn);
        PushSNode(sn);
    }

    SPtr<SNode> State::AotGetSlot(const Value& obj, AbsoluteIndex slot_index, bool _this) const
    {
        using namespace AOT;

        SPtr<SNode> slot;

        const Traits* tr = GetValueTraits(obj);
        if (tr)
        {
            const ASString name = tr->GetSlotName(slot_index);
            const SlotInfo& si = tr->GetSlotInfo(slot_index);
            const ClassTraits::Traits* dt = si.GetDataType(GetVM());

            const Value::KindType kind = obj.GetKind();
            if (obj.IsSNode(kind))
            {
                SNode& prefix = obj.GetSNode();

                slot.Pick(SF_NEW AOT::SNodeVar(&prefix, name, dt));
            }
            else
            {
                // Try to figure out whether current obj is "this" or not.
                if (tr->IsGlobal())
                {
                    const Value& r0 = GetRegister(AbsoluteIndex(0));
                    const Traits* r0tr = GetValueTraits(r0);
                    _this |= (tr == r0tr);
                }

                SPtr<SNode> prefix;
                prefix.Pick(SF_NEW AOT::SNodeVar(obj, tr, _this));

                Pickable<AOT::SNodeVar> nname(SF_NEW AOT::SNodeVar(prefix, name, dt));
                slot.Pick(nname);
            }
        }

        return slot;
    }

    SPtr<SNode> State::AotRetrieveClassName(const Value& v, const Multiname& mn) const
    {
        SPtr<SNode> result;

        if (v.IsNullOrUndefined())
        {
            // Run-time error. We should throw an exception here.
            result.Pick(SF_NEW AOT::SNodeName("null on stack error"));

            return result;
        }

        const Value::KindType k = v.GetKind();
        switch (k)
        {
        case Value::kSNodeCT:
        case Value::kClassTraits:
        case Value::kClass:
            // Use class as a name.
            result.Pick(SF_NEW AOT::SNodeVar(v, GetValueTraits(v)));
            return result;
        default:
            result = AotGetProperty(v, mn);
            break;
        }

        return result;
    }

    SPtr<SNode> State::AotGetProperty(const Value& o, const Multiname& mn, bool _this) const
    {
        VM& vm = GetVM();
        UPInt slot_ind = 0;
        Object* obj = NULL;
        const Traits* tr = NULL;
        SPtr<SNode> result;

        if (o.IsNullOrUndefined())
        {
            // Run-time error. We should throw an exception here.
            result.Pick(SF_NEW AOT::SNodeName("null on stack error"));

            return result;
        }

        tr = GetValueTraits(o);

        SF_ASSERT(tr);

#if 0
        switch (o.GetKind())
        {
        case Value::kSNodeCT:
        case Value::kClassTraits:
        case Value::kClass:
            // Use class as a name.
            result.Pick(SF_NEW AOT::SNodeVar(o, tr));
            return result;
        default:
            break;
        }
#endif
        if (mn.IsRunTime() || (mn.IsQName() && mn.IsAnyNamespace()))
        {
#if 0
            result.Pick(SF_NEW AOT::SNodeName("run-time multiname"));
#endif

            const Value& name = mn.GetName();

            if (name.IsSNode())
                result = &name.GetSNode();
            else
            {
                Pickable<AOT::SNodeVar> var(SF_NEW AOT::SNodeVar(
                    SNode::RetrieveNode(o), 
                    name,
                    tr
                    ));
                var->SetDynamic();

                result = var;
            }
        }
        else
        {
            const SlotInfo* si = FindFixedSlot(vm, *tr, mn, slot_ind, obj);

            if (si)
                result = AotGetSlot(o, AbsoluteIndex(slot_ind), _this);
            else
            {
                const Value& name = mn.GetName();

                if (name.IsSNode())
                    result = &name.GetSNode();
                else
                {
                    Pickable<AOT::SNodeVar> var(SF_NEW AOT::SNodeVar(
                        SNode::RetrieveNode(o), 
                        name,
                        tr
                        ));
                    var->SetDynamic();

                    result = var;
                }
            }
        }

        return result;
    }
#endif

    void State::ConvertRegisterTo(AbsoluteIndex index, InstanceTraits::Traits& tr, Value::TraceNullType isNull)
    {
        SF_ASSERT(RegisterIsAlive(index));

#ifdef SF_AS3_AOTC
        const Value& reg = GetRegister(index);
        if (reg.IsSNode())
        {
            Registers[index.Get()] = Value(tr, isNull, reg.GetSNode());

            return;
        }
#endif

        Registers[index.Get()] = Value(tr, isNull);
    }

    void State::exec_pushnamespace(UInt32 v)
    {
#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_pushnamespace, v);
#endif

        GetTracer().PushNewOpCode(Abc::Code::op_pushnamespace, v);

//         PushOp(Value(GetNamespaceType(), Value::NotNull));
        PushOp(Value(&GetFile().GetInternedNamespace(v)));
    }

    void State::exec_setslot(UInt32 slot_index, bool simulate)
    {
        if (!simulate)
            GetTracer().PushNewOpCode(Abc::Code::op_setslot, slot_index);

        TR::ReadValueObject args(GetVM(), *this);

#ifdef SF_AS3_AOTC
        using namespace AOT;
        Tracer& tracer = GetTracer();
        const Traits* vtr = tracer.GetValueTraits(args.ArgObject);

        if (vtr)
        {
            // Slot's indices begin with ONE.
            if (slot_index <= vtr->GetSlotInfoNum())
            {
                const AbsoluteIndex aind = AbsoluteIndex(SlotIndex(slot_index), *vtr);

                SPtr<SNode> sn = AotGetSlot(args.ArgObject, aind);
                Value value = args.ArgValue;

                switch (value.GetKind())
                {
                case Value::kInstanceTraits:
                    value = Value(value.GetInstanceTraits(), CanBeNull(value.GetInstanceTraits()), *sn);
                    break;
                case Value::kClassTraits:
                    value = Value(value.GetClassTraits(), CanBeNull(value.GetClassTraits().GetInstanceTraits()), *sn);
                    break;
                default:
                    break;
                }

                PushSNode(Pickable<SNode>(SF_NEW SNode2(sn, SNode2::opAssign, value SF_DEBUG_ARG(OpcodeCP))));

                // Try to retrieve function name.
                if (value.IsSNode())
                {
                    SNode& n = value.GetSNode();

                    if (n.Is(SNode::tArgs))
                    {
                        SNodeArgs& na = static_cast<SNodeArgs&>(n);

                        if (na.Op == SNodeArgs::opNewFunction)
                        {
                            SF_ASSERT(na.FITR);
                            const ASString name = vtr->GetSlotName(aind);
                            GetTracer().GetInfoCollector().FunctNameHash.Add(na.FITR->GetMethodInfoInd(), String(name.ToCStr(), name.GetSize()));
                        }
                    }
                }
            }
        }
#endif

#if defined(SF_AS3_AOTC2)
        if (!simulate)
            PushSNodeOC(AOT::SNodeOC::op_setslot, slot_index);
#endif
    }

    void State::exec_astype(UInt32 mn_index)
    {
#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_astype, mn_index);
#endif

        GetTracer().PushNewOpCode(Abc::Code::op_astype, mn_index);

        TR::ReadMnCtValue args(GetFile(), *this, mn_index);
    }

    void State::exec_newclass(UInt32 v)
    {
        GetTracer().PushNewOpCode(Abc::Code::op_newclass, v);

        PopOp(); // Base Class: Object or null

        VMAbcFile& file = GetFile();

        const Abc::ClassInfo& class_info = file.GetClasses().Get(v);

        // check for *scaleform extension* packages.
        const Abc::Multiname& name = class_info.GetName(file.GetConstPool());
        Instances::fl::Namespace& ns = file.GetInternedNamespace(name.GetNamespaceInd());
        ClassTraits::Traits* ctr = NULL;

        if (IsScaleformGFx(ns))
            // Hack on constness.
            ctr = const_cast<ClassTraits::Traits*>(GetVM().Resolve2ClassTraits(file, name));
        else
            ctr = &GetVM().GetUserDefinedTraits(file, class_info);

        if (ctr == NULL)
            ctr = &GetVM().GetClassTraitsObject();

#if defined(SF_AS3_AOTC)
        SPtr<SNode> sn_name;
        sn_name.Pick(SF_NEW AOT::SNodeVar(ctr->GetName(), ctr));

        SPtr<SNode> sn;
        sn.Pick(SF_NEW AOT::SNode1(sn_name, AOT::SNode1::opNewClass SF_DEBUG_ARG(OpcodeCP)));
#endif

#if defined(SF_AS3_AOTC2)
        SPtr<SNode> sn_name;
        sn_name.Pick(SF_NEW AOT::SNodeVar(ctr->GetQualifiedName(), ctr));

        SPtr<SNode> sn;
        sn.Pick(SF_NEW AOT::SNode1(AOT::SNode1::opNewClass, sn_name SF_DEBUG_ARG(OpcodeCP)));
        PushSNode(sn);
#endif

        PushOp(Value(*ctr, CanBeNull(ctr->GetInstanceTraits()) SF_AOTC_ARG(*sn)));
    }

    State::State(Tracer& tr, Abc::TCodeOffset cp) 
    : pTracer(&tr)
    , BCP(cp)
    , OpcodeCP(cp)
    , Registers(tr.GetHeap())
    , OpStack(tr.GetHeap())
    , ScopeStack(tr.GetHeap())
    , RegistersAlive(tr.GetHeap(), tr.GetMethodBodyInfo().GetMaxLocalRegisterIndex())
    {
    }

    ValueArrayDH& State::GetValueArray(MergeSet ms)
    {
        ValueArrayDH* result = NULL;;

        switch (ms)
        {
        case msOpStack:
            result = &OpStack;
            break;
        case msScopeStack:
            result = &ScopeStack;
            break;
        case msRegisterFile:
            result = &Registers;
            break;
        }

        return *result;
    }

    const ValueArrayDH& State::GetValueArray(MergeSet ms) const
    {
        const ValueArrayDH* result = NULL;;

        switch (ms)
        {
        case msOpStack:
            result = &OpStack;
            break;
        case msScopeStack:
            result = &ScopeStack;
            break;
        case msRegisterFile:
            result = &Registers;
            break;
        }

        return *result;
    }

    void State::RefineOpCodeStack1(Abc::Code::OpCode opcode, InstanceTraits::Traits& type, Abc::Code::OpCode op SF_AOTC_ARG(AOT::SNode1::OP sn_op))
    {
#ifdef SF_AS3_AOTC
        SPtr<SNode> sn;
        sn.Pick(SF_NEW AOT::SNode1(BackOpValue(), sn_op SF_DEBUG_ARG(OpcodeCP)));
#endif

        if (GetValueTraits(BackOpValue()) == &type)
        {
            // Value already has a correct type.
            GetTracer().PushNewOpCode(op);

#ifdef SF_AS3_AOTC
            ConvertOpTo(type, CanBeNull(type) SF_AOTC_ARG(*sn));
#endif

#if defined(SF_AS3_AOTC2)
            PushSNodeOC(AOT::SNodeOC::GetOP(op));
#endif
        }
        else
        {
            GetTracer().PushNewOpCode(opcode);

#if defined(SF_AS3_AOTC2)
            PushSNodeOC(AOT::SNodeOC::GetOP(opcode));
#endif
            ConvertOpTo(type, CanBeNull(type) SF_AOTC_ARG(*sn));
        }
    }

    void State::RefineOpCodeStack2(Abc::Code::OpCode opcode, InstanceTraits::Traits& type, Abc::Code::OpCode op SF_AOTC_ARG(AOT::SNode2::OP sn_op))
    {
        const Value _2 = PopOpValue(); // value2

#ifdef SF_AS3_AOTC
        SPtr<SNode> sn;
        sn.Pick(SF_NEW AOT::SNode2(BackOpValue(), sn_op, _2 SF_DEBUG_ARG(GetTracer().CurrOffset)));
#endif

        if (GetValueTraits(BackOpValue()) == &type && GetValueTraits(_2) == &type)
        {
            // Values already have correct types.
            GetTracer().PushNewOpCode(op);

#ifdef SF_AS3_AOTC
            ConvertOpTo(type, CanBeNull(type) SF_AOTC_ARG(*sn));
#endif

#if defined(SF_AS3_AOTC2)
            PushSNodeOC(AOT::SNodeOC::GetOP(op));
#endif
        }
        else
        {
            GetTracer().PushNewOpCode(opcode);

#if defined(SF_AS3_AOTC2)
            PushSNodeOC(AOT::SNodeOC::GetOP(opcode));
#endif
            ConvertOpTo(type, CanBeNull(type) SF_AOTC_ARG(*sn));
        }
    }

    void State::RefineOpCodeReg1(Abc::Code::OpCode opcode, InstanceTraits::Traits& type, Abc::Code::OpCode op, int reg_num)
    {
        const Value& r = GetRegister(AbsoluteIndex(reg_num));

        if (GetValueTraits(r) == &type)
        {
            // Value already has a correct type.
            GetTracer().PushNewOpCode(op, reg_num);

#if defined(SF_AS3_AOTC2)
            PushSNodeOC(AOT::SNodeOC::GetOP(op), reg_num);
#endif
        }
        else
        {
            GetTracer().PushNewOpCode(opcode, reg_num);

#if defined(SF_AS3_AOTC2)
            PushSNodeOC(AOT::SNodeOC::GetOP(opcode), reg_num);
#endif
            ConvertRegisterTo(AbsoluteIndex(reg_num), type, CanBeNull(type));
        }
    }

    void State::exec_pop()
    {
        using namespace Abc;

#ifdef SF_AS3_AOTC
        const Value& v = BackOpValue();
        if (v.IsSNode())
        {
            SNode& n = v.GetSNode();
            if (n.Is(SNode::tN))
                PushSNode(&n);
        }
#endif

#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_pop);
#endif

        if (GetTracer().IsNotRefCountedType(GetValueTraits(BackOpValue())))
            // Value has not ref-counted type.
            GetTracer().PushNewOpCode(Code::op_pop_nrc);
        else
            GetTracer().PushNewOpCode(Code::op_pop);

        PopOp();
    }

    void State::exec_dup()
    {
#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_dup);
#endif

        using namespace Abc;

        if (GetTracer().IsNotRefCountedType(GetValueTraits(BackOpValue())))
            // Value has not ref-counted type.
            GetTracer().PushNewOpCode(Code::op_dup_nrc);
        else
            GetTracer().PushNewOpCode(Code::op_dup);

        // This sequence of calls is necessary to prevent problems related
        // to Array reallocation.
        PushOp(Value::GetUndefined());
        const Value& v = BackOp2ndValue();

#if 0
#ifdef SF_AS3_AOTC
        SPtr<SNode> vn = SNode::RetrieveNode(v);
        const Traits* tr = AOT::GetTraits(GetVM(), *vn);
        if (tr)
        {
            // Create an SNodeRTV.
            Pickable<AOT::SNodeRT> rt(SF_NEW AOT::SNodeRT(100, tr SF_DEBUG_ARG(OpcodeCP)));
            SPtr<SNode> sn;
            const Value::TraceNullType IsNull = GetTracer().CanBeNull(v);

            sn.Pick(SF_NEW AOT::SNodeRTV(rt, vn SF_DEBUG_ARG(OpcodeCP)));
            Value nv(const_cast<Traits&>(*tr), IsNull, *sn);

            PushSNode(sn);
            SetBackOp2ndValue(nv);
            SetBackOpValueUnsafe(nv);
        }
        else
            SetBackOpValueUnsafe(v);
#else
        SetBackOpValueUnsafe(v);
#endif
#else
        SetBackOpValueUnsafe(v);
#endif
    }

    void State::exec_swap(bool simulate) 
    {
        if (!simulate)
            GetTracer().PushNewOpCode(Abc::Code::op_swap);

        SwapOp();
    }

    void State::exec_convert_reg_i(UInt32 reg_num) 
    {
        // reg_num should be stored in MI.OpCode outside of this method.
        ConvertRegisterTo(AbsoluteIndex(reg_num), GetSIntType(), Value::NotNull);
    }

    void State::exec_convert_u()
    {
        GetTracer().PushNewOpCode(Abc::Code::op_convert_u);

#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_convert_u);
#endif

        Value& value = BackOpValue(); // value

        if (value.IsPrimitive())
            // Try to preserve real value on stack.
            value.ToUInt32Value().DoNotCheck();
        else
            ConvertOpTo(GetUIntType(), Value::NotNull);
    }

    void State::exec_getglobalslot(UInt32 slot_index) 
    {
#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_getglobalslot, slot_index);
#endif

        GetTracer().PushNewOpCode(Abc::Code::op_getglobalslot, slot_index);

        // We can do better than this.
        PushOp(Value());
    }

    void State::exec_setglobalslot(UInt32 slot_index) 
    {
#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_setglobalslot, slot_index);
#endif

        GetTracer().PushNewOpCode(Abc::Code::op_setglobalslot, slot_index);

        // We can do better than this.
        PopOp();
    }

    void State::exec_esc_xelem() 
    {
#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_esc_xelem);
#endif
        GetTracer().PushNewOpCode(Abc::Code::op_esc_xelem);

        // ??? Can it be null?
        // It look like it cannot be null.
        ConvertOpTo(GetStringType(), Value::NotNull);
    }

    void State::exec_esc_xattr() 
    {
        GetTracer().PushNewOpCode(Abc::Code::op_esc_xattr);

        // ??? Can it be null?
        // It look like it cannot be null.
        ConvertOpTo(GetStringType(), Value::NotNull);
    }

    void State::exec_convert_i()
    {
        GetTracer().PushNewOpCode(Abc::Code::op_convert_i);

#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_convert_i);
#endif

        Value& value = BackOpValue(); // value

        if (value.IsPrimitive())
            // Try to preserve real value on stack.
            value.ToInt32Value().DoNotCheck();
        else
            ConvertOpTo(GetSIntType(), Value::NotNull);
    }

    void State::exec_convert_reg_u(UInt32 reg_num) 
    {
        // reg_num should be stored in MI.OpCode outside of this method.
        ConvertRegisterTo(AbsoluteIndex(reg_num), GetUIntType(), Value::NotNull);
    }

    void State::exec_convert_d()
    {
        GetTracer().PushNewOpCode(Abc::Code::op_convert_d);

#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_convert_d);
#endif

        Value& value = BackOpValue(); // value

        if (value.IsPrimitive())
            // Try to preserve real value on stack.
            value.ToNumberValue().DoNotCheck();
        else
            ConvertOpTo(GetNumberType(), Value::NotNull);
    }

    void State::exec_convert_reg_d(UInt32 reg_num) 
    {
        // reg_num should be stored in MI.OpCode outside of this method.
        ConvertRegisterTo(AbsoluteIndex(reg_num), GetNumberType(), Value::NotNull);
    }

    void State::exec_convert_b()
    {
        GetTracer().PushNewOpCode(Abc::Code::op_convert_b);

#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_convert_b);
#endif

        Value& value = BackOpValue(); // value

        if (value.IsPrimitive())
            // Try to preserve real value on stack.
            value.ToBooleanValue();
        else
            ConvertOpTo(GetBooleanType(), Value::NotNull);
    }

    void State::SwapOp()
    {
        // Optimized version ...
        SPInt num      = OpStack.GetSize();
        const Value _2 = OpStack[--num];

        OpStack[num] = OpStack[num - 1];
        OpStack[--num] = _2;
    }

    void State::exec_throw(Abc::TCodeOffset bcp)
    {
#ifdef SF_AS3_AOTC
        Pickable<SNode> sn(SF_NEW AOT::SNode1(BackOpValue(), AOT::SNode1::opThrow SF_DEBUG_ARG(OpcodeCP)));
        PushSNode(sn);
#endif

        GetTracer().PushNewOpCode(Abc::Code::op_throw);

#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_throw);
#endif

        // Get exception object from the stack ...
        PopOp(); // Index

#if 0 // It looks like we do not need a block here.
#ifdef SF_AS3_TRACE_DEAD_BLOCK
        GetTracer().AddBlock(*this, bcp, true, true);
#else
        // Block after throw. It should NOT be marked as a dead block as previously.
        GetTracer().AddBlock(*this, bcp, false, true);
#endif
#else
        SF_UNUSED1(bcp);
#endif

    }

    void State::exec_dxns(UInt32 index)
    {
        // Sets the default XML namespace.

#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_dxns, index);
#endif

#ifdef GFX_ENABLE_XML
        if (!GetVM().GetXMLSupport().IsEnabled())
            return GetVM().ThrowVerifyError(VM::Error(VM::eNotImplementedError, GetVM() SF_DEBUG_ARG("dxns")));

        // index is a u30 that must be an index into the string constant pool. The string at index is used
        // as the uri for the default XML namespace for this method.

        GetTracer().PushNewOpCode(Abc::Code::op_dxns, index);

        // A VerifyError is thrown if dxns is used in a method that does not have the SETS_DXNS flag set.
        if (!GetTracer().GetMethodBodyInfo().GetMethodInfo(GetFile().GetMethods()).NeedToSetDXNS())
            GetVM().ThrowVerifyError(VM::Error(VM::eNotImplementedError, GetVM() SF_DEBUG_ARG("does not have the SETS_DXNS flag set")));
#else
        SF_UNUSED1(index);
        GetVM().ThrowVerifyError(VM::Error(VM::eNotImplementedError, GetVM() SF_DEBUG_ARG("dxns")));
#endif
    }

    void State::exec_dxnslate()
    {
        // Sets the default XML namespace with a value determined at runtime.

        GetTracer().PushNewOpCode(Abc::Code::op_dxnslate);

#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_dxnslate);
#endif

#ifdef GFX_ENABLE_XML
        if (!GetVM().GetXMLSupport().IsEnabled())
            return GetVM().ThrowVerifyError(VM::Error(VM::eNotImplementedError, GetVM() SF_DEBUG_ARG("dxnslate")));

        PopOp();

        // A VerifyError is thrown if dxnslate is used in a method that does not have the SETS_DXNS flag set.
        if (!GetTracer().GetMethodBodyInfo().GetMethodInfo(GetFile().GetMethods()).NeedToSetDXNS())
            GetVM().ThrowVerifyError(VM::Error(VM::eNotImplementedError, GetVM() SF_DEBUG_ARG("does not have the SETS_DXNS flag set")));
#else
        GetVM().ThrowVerifyError(VM::Error(VM::eNotImplementedError, GetVM() SF_DEBUG_ARG("dxnslate")));
#endif
    }

    void State::exec_pushwith()
    {
        GetTracer().PushNewOpCode(Abc::Code::op_pushwith);

        // A TypeError is thrown if scope_obj is null or undefined.
#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_pushwith);
#endif

        PushScope(PopOpValue());
        // Set up the *scope* flag.
        GetScopeStack().Back().SetWith();
    }

    void State::exec_popscope()
    {
        GetTracer().PushNewOpCode(Abc::Code::op_popscope);

#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_popscope);
#endif
        PopScope();
    }

#if defined(SF_AS3_AOTC) || defined(SF_AS3_AOTC2)
    void State::PushSNode(Pickable<SNode> n) const
    {
        pTracer->CurrBlock->PushSNode(n);
    }
    void State::PushSNode(const SPtr<SNode>& n) const
    {
        pTracer->CurrBlock->PushSNode(n);
    }
#endif

#if defined(SF_AS3_AOTC2)
    void State::PopSNodeOC() const
    {
        pTracer->CurrBlock->PopSNode();
    }

    void State::PushSNodeOC(AOT::SNodeOC::OP op, SInt32 v1, SInt32 v2) const
    {
        Pickable<SNode> sn(SF_NEW AOT::SNodeOC(op, v1, v2 SF_DEBUG_ARG(OpcodeCP)));
        PushSNode(sn);
    }

    void State::PushSNodeComment(const String& str) const
    {
        Pickable<SNode> sn(SF_NEW AOT::SNodeStr(str, AOT::SNodeStr::tComment));
        PushSNode(sn);
    }

    void State::PushSNodeAbrupt(AOT::SNodeAbrupt::OP op) const
    {
        Pickable<SNode> sn(SF_NEW AOT::SNodeAbrupt(op SF_DEBUG_ARG(OpcodeCP)));
        PushSNode(sn);
    }

    void State::PushSNodeAbrupt(AOT::SNodeAbrupt::OP op, const Block& b) const
    {
        Pickable<SNode> sn(SF_NEW AOT::SNodeAbrupt(op, &b.GetNodes() SF_DEBUG_ARG(OpcodeCP)));
        PushSNode(sn);
    }

    void State::PushSNodeIF(AOT::SNodeIF::OP op, const Block& t) const
    {
#if 1
        Pickable<SNode> sn(SF_NEW AOT::SNodeIF(
            op, 
            &t.GetNodes()
            SF_DEBUG_ARG(OpcodeCP)
            ));
        PushSNode(sn);
#else
        Pickable<SNode> snt(SF_NEW AOT::SNodeAbrupt(AOT::SNodeAbrupt::opGoto, &t.GetNodes(), t.GetFrom() SF_DEBUG_ARG(OpcodeCP)));

        Pickable<SNode> sn(SF_NEW AOT::SNodeIF(
            op, 
            snt,
            t.GetFrom()
            SF_DEBUG_ARG(OpcodeCP)
            ));

        PushSNode(sn);
#endif
    }

    void State::PushSNodeIF(AOT::SNodeIF::OP op, const Block& t, const Block& f) const
    {
#if 1
        Pickable<SNode> sn(SF_NEW AOT::SNodeIF(
            op, 
            &t.GetNodes(),
            &f.GetNodes()
            SF_DEBUG_ARG(OpcodeCP)
            ));
        PushSNode(sn);
#else
        Pickable<SNode> snt(SF_NEW AOT::SNodeAbrupt(AOT::SNodeAbrupt::opGoto, &t.GetNodes(), t.GetFrom() SF_DEBUG_ARG(OpcodeCP)));
        Pickable<SNode> snf(SF_NEW AOT::SNodeAbrupt(AOT::SNodeAbrupt::opGoto, &f.GetNodes(), f.GetFrom() SF_DEBUG_ARG(OpcodeCP)));

        Pickable<SNode> sn(SF_NEW AOT::SNodeIF(
            op, 
            snt,
            snf,
            t.GetFrom(),
            f.GetFrom()
            SF_DEBUG_ARG(OpcodeCP)
            ));

        PushSNode(sn);
#endif
    }

    AOT::SNodeSwitchOC& State::PushSNodeSwitch(const Block& def) const
    {
        SPtr<AOT::SNodeSwitchOC> sn;

        sn.Pick(SF_NEW AOT::SNodeSwitchOC(&def.GetNodes() SF_DEBUG_ARG(OpcodeCP)));
        PushSNode(sn.GetPtr());

        return *sn;
    }

    void State::PushSNodeGetLocal(UInt32 reg_ind) const
    {
        const UPInt param_num = GetTracer().GetMethodInfo().GetParamCount();

        // reg_ind == 0 is "this".
        if (reg_ind > param_num)
            PushSNodeOC(AOT::SNodeOC::op_getlocal, reg_ind);
        else
            PushSNodeOC(AOT::SNodeOC::op_getarg, reg_ind);
    }
#endif

    void State::exec_if(Abc::TCodeOffset& bcp, const Abc::Code::OpCode opcode)
    {
        using namespace Abc;

        Tracer& tr = GetTracer();

#if defined(SF_AS3_AOTC2)
        AOT::SNodeIF::OP if_op = AOT::SNodeIF::op_ifnlt;
#endif

        if (tr.IsSIntType(BackOpValue()) && tr.IsSIntType(BackOp2ndValue()))
        {
            Code::OpCode new_opcode = opcode;
            switch (opcode)
            {
            case Code::op_ifnlt:
                new_opcode = Code::op_ifnlt_ti;
                break;
            case Code::op_ifnle:
                new_opcode = Code::op_ifnle_ti;
                break;
            case Code::op_ifngt:
                new_opcode = Code::op_ifngt_ti;
                break;
            case Code::op_ifnge:
                new_opcode = Code::op_ifnge_ti;
                break;
            case Code::op_ifeq:
                new_opcode = Code::op_ifeq_ti;
                break;
            case Code::op_ifge:
                new_opcode = Code::op_ifge_ti;
                break;
            case Code::op_ifgt:
                new_opcode = Code::op_ifgt_ti;
                break;
            case Code::op_ifle:
                new_opcode = Code::op_ifle_ti;
                break;
            case Code::op_iflt:
                new_opcode = Code::op_iflt_ti;
                break;
            case Code::op_ifne:
                new_opcode = Code::op_ifne_ti;
                break;
            case Code::op_ifstricteq:
                new_opcode = Code::op_ifeq_ti;
                break;
            case Code::op_ifstrictne:
                new_opcode = Code::op_ifne_ti;
                break;
            default:
                break;
            }

#if defined(SF_AS3_AOTC2)
            switch (new_opcode)
            {
            case Code::op_ifnlt_ti:
                if_op = AOT::SNodeIF::op_ifnlt_ti;
                break;
            case Code::op_ifnle_ti:
                if_op = AOT::SNodeIF::op_ifnle_ti;
                break;
            case Code::op_ifngt_ti:
                if_op = AOT::SNodeIF::op_ifngt_ti;
                break;
            case Code::op_ifnge_ti:
                if_op = AOT::SNodeIF::op_ifnge_ti;
                break;
            case Code::op_ifeq_ti:
                if_op = AOT::SNodeIF::op_ifeq_ti;
                break;
            case Code::op_ifge_ti:
                if_op = AOT::SNodeIF::op_ifge_ti;
                break;
            case Code::op_ifgt_ti:
                if_op = AOT::SNodeIF::op_ifgt_ti;
                break;
            case Code::op_ifle_ti:
                if_op = AOT::SNodeIF::op_ifle_ti;
                break;
            case Code::op_iflt_ti:
                if_op = AOT::SNodeIF::op_iflt_ti;
                break;
            case Code::op_ifne_ti:
                if_op = AOT::SNodeIF::op_ifne_ti;
                break;
            default:
                break;
            }
#endif

            tr.PushNewOpCode(new_opcode);
        } else if (tr.IsNumberType(BackOpValue()) && tr.IsNumberType(BackOp2ndValue()))
        {
            Code::OpCode new_opcode = opcode;
            switch (opcode)
            {
            case Code::op_ifnlt:
                new_opcode = Code::op_ifnlt_td;
                break;
            case Code::op_ifnle:
                new_opcode = Code::op_ifnle_td;
                break;
            case Code::op_ifngt:
                new_opcode = Code::op_ifngt_td;
                break;
            case Code::op_ifnge:
                new_opcode = Code::op_ifnge_td;
                break;
            case Code::op_ifeq:
                new_opcode = Code::op_ifeq_td;
                break;
            case Code::op_ifge:
                new_opcode = Code::op_ifge_td;
                break;
            case Code::op_ifgt:
                new_opcode = Code::op_ifgt_td;
                break;
            case Code::op_ifle:
                new_opcode = Code::op_ifle_td;
                break;
            case Code::op_iflt:
                new_opcode = Code::op_iflt_td;
                break;
            case Code::op_ifne:
                new_opcode = Code::op_ifne_td;
                break;
            case Code::op_ifstricteq:
                new_opcode = Code::op_ifeq_td;
                break;
            case Code::op_ifstrictne:
                new_opcode = Code::op_ifne_td;
                break;
            default:
                break;
            }

#if defined(SF_AS3_AOTC2)
            switch (new_opcode)
            {
            case Code::op_ifnlt_td:
                if_op = AOT::SNodeIF::op_ifnlt_td;
                break;
            case Code::op_ifnle_td:
                if_op = AOT::SNodeIF::op_ifnle_td;
                break;
            case Code::op_ifngt_td:
                if_op = AOT::SNodeIF::op_ifngt_td;
                break;
            case Code::op_ifnge_td:
                if_op = AOT::SNodeIF::op_ifnge_td;
                break;
            case Code::op_ifeq_td:
                if_op = AOT::SNodeIF::op_ifeq_td;
                break;
            case Code::op_ifge_td:
                if_op = AOT::SNodeIF::op_ifge_td;
                break;
            case Code::op_ifgt_td:
                if_op = AOT::SNodeIF::op_ifgt_td;
                break;
            case Code::op_ifle_td:
                if_op = AOT::SNodeIF::op_ifle_td;
                break;
            case Code::op_iflt_td:
                if_op = AOT::SNodeIF::op_iflt_td;
                break;
            case Code::op_ifne_td:
                if_op = AOT::SNodeIF::op_ifne_td;
                break;
            default:
                break;
            }
#endif

            tr.PushNewOpCode(new_opcode);
        }
        else
            tr.PushNewOpCode(opcode);

        // Relative
        const int offset = ReadS24(tr.pCode, bcp);

#if defined(SF_AS3_AOTC)
        using namespace AOT;

        SNodeBoolExpr::OP cop = SNodeBoolExpr::opNLT;
        switch (opcode)
        {
        case Code::op_ifnlt:
            cop = SNodeBoolExpr::opNLT;
            break;
        case Code::op_ifnle:
            cop = SNodeBoolExpr::opNLE;
            break;
        case Code::op_ifngt:
            cop = SNodeBoolExpr::opNGT;
            break;
        case Code::op_ifnge:
            cop = SNodeBoolExpr::opNGE;
            break;
        case Code::op_ifeq:
            cop = SNodeBoolExpr::opEQ;
            break;
        case Code::op_ifge:
            cop = SNodeBoolExpr::opGE;
            break;
        case Code::op_ifgt:
            cop = SNodeBoolExpr::opGT;
            break;
        case Code::op_ifle:
            cop = SNodeBoolExpr::opLE;
            break;
        case Code::op_iflt:
            cop = SNodeBoolExpr::opLT;
            break;
        case Code::op_ifne:
            cop = SNodeBoolExpr::opNE;
            break;
        case Code::op_ifstricteq:
            cop = SNodeBoolExpr::opEQ;
            break;
        case Code::op_ifstrictne:
            cop = SNodeBoolExpr::opNE;
            break;
        default:
            break;
        }

        SPtr<SNode> expr;
        expr.Pick(SF_NEW SNodeBoolExpr(cop, BackOp2ndValue(), BackOpValue() SF_DEBUG_ARG(OpcodeCP)));

        // Do not move these pops below because pState can be changed.
        PopOp();
        PopOp();

        if (offset >= 0)
        {
            // Regular IF statement.

            Pickable<SNodeIF> sn(SF_NEW SNodeIF(expr SF_DEBUG_ARG(OpcodeCP)));
            PushSNode(sn);

            // True part.
            // Relative
            tr.StoreOffset(bcp, *this, offset, -1 SF_AOTC_ARG(sn->GetTrueNC()));

            // False part
            Block* block = tr.AddBlock(*this, bcp, Block::tUnknown, true);
            block->AddTarget(sn->GetFalseNC());
        }
        else
        {
            // Loop statement.

            Block* pCurBlock = tr.CurrBlock;
            pCurBlock->SetType(Block::tLoopCondition);

            // True part
            Block* true_block = tr.GetBlock(bcp + offset);
            SF_ASSERT(true_block);

            // False part
            Block* false_block = tr.AddBlock(*this, bcp, Block::tUnknown, true);
            SF_ASSERT(false_block);

            // Visitor will set up parent of the expression.
            SetLoopCondVisitor v(expr);
            true_block->GetNodes().Accept(v);

            // Add an goto statement after a loop.
            SPtr<SNode> sna;
            sna.Pick(SF_NEW SNodeAbrupt(SNodeAbrupt::opGoto, &false_block->GetNodes(), false_block->GetFrom() SF_DEBUG_ARG(tr.CurrOffset)));
            true_block->GetNodes().Elems.PushBack(sna);

            // Because this is a block with condition we can insert a fake
            // "continue" statement to make this block not empty.
            const Block& cb = GetTracer().GetCurrBlock();
            Pickable<SNode> sn(SF_NEW SNodeAbrupt(SNodeAbrupt::opContinue, SPtr<SNodeBlock>(), cb.GetFrom() SF_DEBUG_ARG(cb.GetFrom())));
            PushSNode(sn);
        }
#elif defined(SF_AS3_AOTC2)
        {
            // Do not move these pops below because pState can be changed.
            PopOp();
            PopOp();

            // Relative
            tr.StoreOffset(bcp, *this, offset, -1);
        }

        // True part
        // Call GetBlock() after StoreOffset() because StoreOffset() will create it.
        Block* true_block = tr.GetBlock(bcp + offset);
        SF_ASSERT(true_block);

        // False part
        Block* false_block = tr.AddBlock(*this, bcp, Block::tUnknown, true);
        SF_ASSERT(false_block);

        switch (opcode)
        {
        case Code::op_ifnlt:
            if_op = AOT::SNodeIF::op_ifnlt;
            break;
        case Code::op_ifnle:
            if_op = AOT::SNodeIF::op_ifnle;
            break;
        case Code::op_ifngt:
            if_op = AOT::SNodeIF::op_ifngt;
            break;
        case Code::op_ifnge:
            if_op = AOT::SNodeIF::op_ifnge;
            break;
        case Code::op_ifeq:
            if_op = AOT::SNodeIF::op_ifeq;
            break;
        case Code::op_ifge:
            if_op = AOT::SNodeIF::op_ifge;
            break;
        case Code::op_ifgt:
            if_op = AOT::SNodeIF::op_ifgt;
            break;
        case Code::op_ifle:
            if_op = AOT::SNodeIF::op_ifle;
            break;
        case Code::op_iflt:
            if_op = AOT::SNodeIF::op_iflt;
            break;
        case Code::op_ifne:
            if_op = AOT::SNodeIF::op_ifne;
            break;
        case Code::op_ifstricteq:
            if_op = AOT::SNodeIF::op_ifstricteq;
            break;
        case Code::op_ifstrictne:
            if_op = AOT::SNodeIF::op_ifstrictne;
            break;
        default:
            SF_ASSERT(false);
            break;
        }

#if 1
        PushSNodeIF(
            if_op, 
            *true_block
            );

        PushSNodeAbrupt(AOT::SNodeAbrupt::opGoto, *false_block);
#else
        PushSNodeIF(
            if_op, 
            *true_block,
            *false_block
            );
#endif

#else
        // Do not move these pops below because pState can be changed.
        PopOp();
        PopOp();

        // Relative
        tr.StoreOffset(bcp, *this, offset, -1);
#endif
    }

    void State::exec_if_boolean(Abc::TCodeOffset& bcp, const Abc::Code::OpCode opcode)
    {
        using namespace Abc;

        Tracer& tr = GetTracer();

        if (tr.IsBooleanType(BackOpValue()))
            tr.PushNewOpCode(opcode == Code::op_iftrue ? Code::op_iftrue_tb : Code::op_iffalse_tb);
        else
            tr.PushNewOpCode(opcode);

        // Relative
        const int offset = ReadS24(tr.pCode, bcp);

#if defined(SF_AS3_AOTC)
        using namespace AOT;

        SPtr<SNode> r;
        r.Pick(SF_NEW SNodeValue(Value(opcode == Code::op_iftrue)));
        SPtr<SNode> expr;
        expr.Pick(SF_NEW SNodeBoolExpr(SNodeBoolExpr::opEQ, BackOpValue(), r SF_DEBUG_ARG(OpcodeCP)));

        if (offset >= 0)
        {
            // Regular IF statement.

            Pickable<SNodeIF> sn(SF_NEW SNodeIF(expr SF_DEBUG_ARG(OpcodeCP)));
            PushSNode(sn);

            PopOp();

            // True part.
            tr.StoreOffset(bcp, *this, offset, -1 SF_AOTC_ARG(sn->GetTrueNC()));

            // False part.
            TR::Block* block = tr.AddBlock(*this, bcp, Block::tUnknown, true);
            block->AddTarget(sn->GetFalseNC());
        }
        else
        {
            // Loop statement.

            Block* pCurBlock = tr.CurrBlock;
            pCurBlock->SetType(Block::tLoopCondition);

            // True part
            TR::Block* true_block = tr.GetBlock(bcp + offset);
            SF_ASSERT(true_block);

            // False part
            TR::Block* false_block = tr.AddBlock(*this, bcp, Block::tUnknown, true);
            SF_ASSERT(false_block);

            // Visitor will set up parent of the expression.
            SetLoopCondVisitor v(expr);
            true_block->GetNodes().Accept(v);

            // Add an goto statement after a loop.
            SPtr<SNode> sna;
            sna.Pick(SF_NEW SNodeAbrupt(SNodeAbrupt::opGoto, &false_block->GetNodes(), false_block->GetFrom() SF_DEBUG_ARG(tr.CurrOffset)));
            true_block->GetNodes().Elems.PushBack(sna);

            // Because this is a block with condition we can insert a fake
            // "continue" statement to make this block not empty.
            const TR::Block& cb = GetTracer().GetCurrBlock();
            Pickable<SNode> sn(SF_NEW SNodeAbrupt(SNodeAbrupt::opContinue, SPtr<SNodeBlock>(), cb.GetFrom() SF_DEBUG_ARG(cb.GetFrom())));
            PushSNode(sn);
        }
#elif defined(SF_AS3_AOTC2)
        {
            PopOp();

            // True part.
            tr.StoreOffset(bcp, *this, offset, -1 SF_AOTC_ARG(sn->True));
        }

        // True part
        // Call GetBlock() after StoreOffset() because StoreOffset() will create it.
        Block* true_block = tr.GetBlock(bcp + offset);
        SF_ASSERT(true_block);

        // False part
        Block* false_block = tr.AddBlock(*this, bcp, Block::tUnknown, true);
        SF_ASSERT(false_block);

        AOT::SNodeIF::OP if_op = AOT::SNodeIF::op_iftrue;

        switch (opcode)
        {
        case Code::op_iftrue:
            if_op = AOT::SNodeIF::op_iftrue;
            break;
        case Code::op_iffalse:
            if_op = AOT::SNodeIF::op_iffalse;
            break;
        case Code::op_iftrue_tb:
            if_op = AOT::SNodeIF::op_iftrue_tb;
            break;
        case Code::op_iffalse_tb:
            if_op = AOT::SNodeIF::op_iffalse_tb;
            break;
        default:
            SF_ASSERT(false);
            break;
        }

#if 1
        PushSNodeIF(
            if_op, 
            *true_block
            );

        PushSNodeAbrupt(AOT::SNodeAbrupt::opGoto, *false_block);
#else
        PushSNodeIF(
            if_op, 
            *true_block, 
            *false_block
            );
#endif

#else
        PopOp();

        // True part.
        tr.StoreOffset(bcp, *this, offset, -1 SF_AOTC_ARG(sn->True));
#endif
    }

    void State::exec_jump(Abc::TCodeOffset& bcp)
    {
        using namespace Abc;

        Tracer& tr = GetTracer();

        tr.PushNewOpCode(Abc::Code::op_jump);

        const int offset = ReadS24(tr.pCode, bcp);

#if defined(SF_AS3_AOTC)

#ifdef GFX_AS3_TRACE
        FlashUI& ui = tr.GetUI();
#endif

        using namespace AOT;
        TCodeOffset ccp = bcp;
        const Code::OpCode next_opcode = static_cast<Code::OpCode>(Read8(tr.pCode, ccp));
        Block* pBlock = tr.CurrBlock;

#if 0
        const Code::OpCode target_opcode = tr.GetCurrOrigOpCode(bcp + offset);
        if (target_opcode == Code::op_returnvoid || target_opcode == Code::op_returnvalue)
        {
            Pickable<SNodeAbrupt> sn(SF_NEW SNodeAbrupt(SNodeAbrupt::opReturn, SPtr<SNodeBlock>(), bcp SF_DEBUG_ARG(bcp - 1)));
            PushSNode(sn);

            if (target_opcode == Code::op_returnvalue)
            {
                sn->ReturnValue = SNode::RetrieveNode(BackOpValue());
                PopOp();
            }

            // Target is empty.
            SPtr<SNode> target;
            tr.StoreOffset(bcp, *this, offset, -1, target);
        }
        else
#endif
        if (next_opcode == Code::op_label)
        {
            TR::Block* prevBlock = pBlock;

            // Special block for op_label.
            {
                // Block starts with LABEL.
                tr.AddBlock(*this, bcp, Block::tUnknown, true);

                // !!! Be careful. pBlock gets changed here.
                // Check for next block
                if (!tr.UpdateBlock(bcp, next_opcode))
                    // Exception.
                    return;

                pBlock = tr.CurrBlock;

#ifdef GFX_AS3_TRACE
                if (ui.NeedToCheckOpCode() && ui.OnOpCode(bcp)) { SF_ASSERT(false); }
#endif
            }

            const bool isSwitchStmt = pBlock->IsSwitchCaseBlock();

            if (isSwitchStmt)
            {
                // Switch.
                if (tr.HandledSwitchStmt.Get(pBlock->GetSwitchOffset()) == NULL)
                {
                    // This switch statement wasn't handled yet.

                    TCodeOffset sws_offset = pBlock->GetSwitchOffset();
                    Pickable<SNodeSwitch> sn_sw(SF_NEW SNodeSwitch(SF_DEBUG_CODE(sws_offset)));
                    prevBlock->PushSNode(sn_sw);

                    TCodeOffset base_location = sws_offset;
                    TR::Block* block;

                    tr.HandledSwitchStmt.Add(base_location);
                    ++sws_offset;

                    const int default_offset = ReadS24(tr.pCode, sws_offset);
                    const int case_count = ReadU30(tr.pCode, sws_offset);

                    // There are case_count + 1 cases.
                    sn_sw->Cases.Resize(case_count + 1);

                    for(int i = 0; i <= case_count; ++i)
                    {
                        const int offset = ReadS24(tr.pCode, sws_offset);

                        if (offset == default_offset)
                            sn_sw->DefaultN = i;

                        block = tr.GetBlock(base_location + offset);
                        block->AddTarget(sn_sw->Cases[i].CaseBody);

                        // Change parent of the block. block belongs to the switch statement now.
                        block->GetNodes().SetParent(sn_sw.GetPtr());
                    }

                    // This jump is going to jump to a condition.
                    {
                        block = tr.AddBlock(*this, bcp + offset, Block::tUnknown, true);

                        block->AddTarget(sn_sw->Cond);

                        // Change parent of the block. block belongs to the switch statement now.
                        block->GetNodes().SetParent(sn_sw.GetPtr());
                    }
                }
            }
            else
            {
                // Loop.

                // This is an artificial block. This is actually a single loop statement.
                AOT::SNodeBlock& nodes = pBlock->GetNodes();
                nodes.SetArtificial();

                Pickable<SNodeAbrupt> sn(SF_NEW SNodeAbrupt(SNodeAbrupt::opGoto, &nodes, pBlock->GetFrom() SF_DEBUG_ARG(bcp)));
                prevBlock->PushSNode(sn);

//                 // Store this new block as a part of an old block.
//                 prevBlock->PushSNode(SPtr<SNode>(&nodes));

                Pickable<SNodeLoop> sn_loop(SF_NEW SNodeLoop(
                    bcp + offset,
                    offset == 1 ? SNodeLoop::opDoWhile : SNodeLoop::opWhile
                    SF_DEBUG_ARG(OpcodeCP)
                    ));

                // Loop goes inside of the "label" block.
                PushSNode(sn_loop);
                // sn_loop->Expr will be picked up automatically.

                // Block right after the "op_label".
                // Offset relative bcp.
                // Offset is 1 because this is a size of the op_label opcode.
                tr.StoreOffset(bcp, *this, 1, -1, sn_loop->Body);

                // Set type of the block.
                tr.GetBlock(bcp + 1)->SetType(Block::tLoopBody);;

//                 // Change parent of current block. Current block belongs to the loop statement now.
//                 nodes.SetParent(sn_loop.GetPtr());

                if (offset > 1)
                    tr.AddBlock(*this, bcp + offset, Block::tLoopCondition, true);
            }

            // Skip prev combination ...
            tr.SkipOrigOpCode(bcp, ccp);
            tr.CurrOffset = bcp;

            return;
        }
        else
        {
            // No manipulation with stack.
#if 1
            TR::Block* block = tr.AddBlock(*this, bcp + offset, Block::tUnknown, true);
            SF_ASSERT(block);
            Pickable<SNodeAbrupt> sn(SF_NEW SNodeAbrupt(SNodeAbrupt::opGoto, &block->GetNodes(), block->GetFrom() SF_DEBUG_ARG(OpcodeCP)));
            pBlock->PushSNode(sn);
#else
            tr.AddBlock(*this, bcp + offset, false, true);
#endif
        }
#elif defined(SF_AS3_AOTC2)
        TR::Block* block = tr.AddBlock(*this, bcp + offset, Block::tUnknown, true);

        if (block)
            PushSNodeAbrupt(AOT::SNodeAbrupt::opGoto, *block);

        // No manipulation with stack.
        tr.StoreOffset(bcp, *this, offset, -1);
#else
        // No manipulation with stack.
        tr.StoreOffset(bcp, *this, offset, -1);
#endif


        // Code immediately following the jump. Possible dead block.
        tr.AddBlock(*this, bcp, Block::tDead, true);
    }

    void State::exec_switch(Abc::TCodeOffset& bcp)
    {
        using namespace Abc;

        Tracer& tr = GetTracer();

        tr.PushNewOpCode(Abc::Code::op_lookupswitch);

#if defined(SF_AS3_AOTC)
        {
            // This switch statement should be already handled.
            // SF_ASSERT(HandledSwitchStmt.Get(CurrOffset) != 0);

            // We, probably, shouldn't try to generate correct byte code here.

            // Value on the stack must be of type integer ...
            PopOp(); // Index

            const int default_offset = ReadS24(tr.pCode, bcp);
            tr.PushNewOpCodeArg(default_offset);

            int case_count = ReadU30(tr.pCode, bcp);
            tr.PushNewOpCodeArg(case_count);

            for(int i = 0; i <= case_count; ++i)
            {
                const int offset = ReadS24(tr.pCode, bcp);

                tr.PushNewOpCodeArg(offset);
            }
        }
#elif defined(SF_AS3_AOTC2)
        // Value on the stack must be of type integer ...
        PopOp(); // Index

        Abc::TCodeOffset base_location = tr.CurrOffset;
        int default_offset = ReadS24(tr.pCode, bcp);

        tr.StoreOffset(bcp, *this, static_cast<SInt32>(base_location + default_offset - bcp), 1);

        Abc::TCodeOffset def_addr = base_location + default_offset;
        Block* def_block = tr.GetBlock(def_addr);
        SF_ASSERT(def_block);

        AOT::SNodeSwitchOC& sw = PushSNodeSwitch(*def_block);

        int case_count = ReadU30(tr.pCode, bcp);
        tr.PushNewOpCodeArg(case_count);

        Abc::TCodeOffset case_addr;
        for(int i = 0; i <= case_count; ++i)
        {
            const int offset = ReadS24(tr.pCode, bcp);

            tr.StoreOffset(bcp, *this, static_cast<SInt32>(base_location + offset - bcp), i + 3);

            case_addr = base_location + offset;
            Block* case_block = tr.GetBlock(case_addr);
            SF_ASSERT(case_block);

            sw.PushCase(&case_block->GetNodes());
        }

#else
        // Regular tracing.
        {
            // Value on the stack must be of type integer ...
            PopOp(); // Index

            Abc::TCodeOffset base_location = tr.CurrOffset;
            const int default_offset = ReadS24(tr.pCode, bcp);

            tr.StoreOffset(bcp, *this, static_cast<SInt32>(base_location + default_offset - bcp), 1);

            int case_count = ReadU30(tr.pCode, bcp);
            tr.PushNewOpCodeArg0(case_count);

            for(int i = 0; i <= case_count; ++i)
            {
                const int offset = ReadS24(tr.pCode, bcp);

                tr.StoreOffset(bcp, *this, static_cast<SInt32>(base_location + offset - bcp), i + 3);
            }
        }
#endif

        // Code immediately following the lookupswitch. Possible dead block.
        tr.AddBlock(*this, bcp, Block::tDead, true);
    }

    void State::exec_opcode(const Abc::Code::OpCode opcode, Abc::TCodeOffset& bcp)
    {
        using namespace Abc;

        StringBuffer sb;
        Tracer& tr = GetTracer();
        const UInt8* pCode = tr.pCode;
        OpcodeCP = bcp - 1;

//         tr.PushNewOpCode(opcode);

        // We do not need to handle new MI.OpCode here ...
        switch (opcode)
        {
        case Code::op_throw:
            exec_throw(bcp);
            break;
        case Code::op_getsuper:
        case Code::op_setsuper:
            // Should be handled by SubstituteOpCode().
            SF_ASSERT(false);
            break;
        case Code::op_dxns:
            exec_dxns(ReadU30(pCode, bcp));
            break;
        case Code::op_dxnslate:
            exec_dxnslate();
            break;
        case Code::op_kill:
            // Should be handled by SubstituteOpCode().
            SF_ASSERT(false);
            break;
        case Code::op_label:
            // Block starts from LABEL.
            tr.AddBlock(*this, bcp - 1, Block::tUnknown, true);

            // Get rid of op_label. (Do not push the opcode)
            break;
        case Code::op_ifnlt:
        case Code::op_ifnle:
        case Code::op_ifngt:
        case Code::op_ifnge:
        case Code::op_ifeq:
        case Code::op_ifge:
        case Code::op_ifgt:
        case Code::op_ifle:
        case Code::op_iflt:
        case Code::op_ifne:
        case Code::op_ifstricteq:
        case Code::op_ifstrictne:
            exec_if(bcp, opcode);
            break;
        case Code::op_iftrue:
        case Code::op_iffalse:
            exec_if_boolean(bcp, opcode);
            break;
        case Code::op_jump:
            exec_jump(bcp);
            break;
        case Code::op_lookupswitch:
            exec_switch(bcp);
            break;
        case Code::op_pushwith:
            exec_pushwith();
            break;
        case Code::op_popscope:
            exec_popscope();
            break;
        case Code::op_nextname:
            exec_nextname();
            break;
        case Code::op_hasnext:
            exec_hasnext();
            break;
        case Code::op_pushnull:
            tr.PushNewOpCode(Abc::Code::op_pushnull);

#if defined(SF_AS3_AOTC2)
            PushSNodeOC(AOT::SNodeOC::op_pushnull);
#endif
            // Tamarin style.
            //PushOp(Value(GetNullType(), true));
            PushOp(Value::GetNull());
            break;
        case Code::op_pushundefined:
            exec_pushundefined();
            break;
        case Code::op_nextvalue:
            exec_nextvalue();
            break;
        case Code::op_pushbyte:
            exec_pushbyte(Read8(pCode, bcp));
            break;
        case Code::op_pushshort:
            exec_pushshort(ReadU30(pCode, bcp));
            break;
        case Code::op_pushtrue:
            tr.PushNewOpCode(Abc::Code::op_pushtrue);

#if defined(SF_AS3_AOTC2)
            PushSNodeOC(AOT::SNodeOC::op_pushtrue);
#endif
            PushOp(Value(true));
            break;
        case Code::op_pushfalse:
            tr.PushNewOpCode(Abc::Code::op_pushfalse);

#if defined(SF_AS3_AOTC2)
            PushSNodeOC(AOT::SNodeOC::op_pushfalse);
#endif
            PushOp(Value(false));
            break;
        case Code::op_pushnan:
            tr.PushNewOpCode(Abc::Code::op_pushnan);

#if defined(SF_AS3_AOTC2)
            PushSNodeOC(AOT::SNodeOC::op_pushnan);
#endif
            PushOp(Value(NumberUtil::NaN()));
            break;
        case Code::op_pop:
            exec_pop();
            break;
        case Code::op_dup:
            exec_dup();
            break;
        case Code::op_swap:
            tr.PushNewOpCode(Abc::Code::op_swap);

#if defined(SF_AS3_AOTC2)
            PushSNodeOC(AOT::SNodeOC::op_swap);
#endif
            SwapOp();
            break;
        case Code::op_pushstring:
            exec_pushstring(ReadU30(pCode, bcp));
            break;
        case Code::op_pushint:
            exec_pushint(ReadU30(pCode, bcp));
            break;
        case Code::op_pushuint:
            exec_pushuint(ReadU30(pCode, bcp));
            break;
        case Code::op_pushdouble:
            exec_pushdouble(ReadU30(pCode, bcp));
            break;
        case Code::op_pushscope:
            exec_pushscope();
            break;
        case Code::op_pushnamespace:
            exec_pushnamespace(ReadU30(pCode, bcp));
            // ??? Exceptions ?
            break;
        case Code::op_hasnext2:
            {
                UInt32 object_reg = ReadU30(pCode, bcp);
                UInt32 index_reg = ReadU30(pCode, bcp);
                exec_hasnext2(object_reg, index_reg);
            }
            break;
        case Code::op_increment_tu:
            exec_li8();
            break;
        case Code::op_decrement_tu:
            exec_li16();
            break;
        case Code::op_inclocal_tu:
            exec_li32();
            break;
        case Code::op_declocal_tu:
            exec_lf32();
            break;
        case Code::op_lf64:
            exec_lf64();
            break;
        case Code::op_si8:
            exec_si8();
            break;
        case Code::op_si16:
            exec_si16();
            break;
        case Code::op_si32:
            exec_si32();
            break;
        case Code::op_sf32:
            exec_sf32();
            break;
        case Code::op_sf64:
            exec_sf64();
            break;
        case Code::op_newfunction:
            exec_newfunction(ReadU30(pCode, bcp));
            break;
        case Code::op_call:
            exec_call(ReadU30(pCode, bcp));
            break;
        case Code::op_construct:
            exec_construct(ReadU30(pCode, bcp));
            break;
        case Code::op_callmethod:
        case Code::op_callstatic:
            // These opcodes shouldn't be generated by a compiler.
            GetVM().ThrowVerifyError(VM::Error(VM::eNotImplementedError, GetVM() SF_DEBUG_ARG(Code::GetOpCodeInfo(opcode).name)));
            break;
        case Code::op_callsuper:
            // Should be handled by SubstituteOpCode().
            SF_ASSERT(false);
            break;
        case Code::op_callproperty:
            // Should be handled by SubstituteOpCode().
            SF_ASSERT(false);
            break;
        case Code::op_returnvalue:
            tr.PushNewOpCode(Abc::Code::op_returnvalue);

#if defined(SF_AS3_AOTC)
            {
                using namespace AOT;

                Pickable<SNodeAbrupt> sn(SF_NEW SNodeAbrupt(SNodeAbrupt::opReturn, SPtr<SNodeBlock>(), bcp SF_DEBUG_ARG(OpcodeCP)));

                if (!NeedToDiscardResult())
                    sn->ReturnValue = SNode::RetrieveNode(BackOpValue());

                PushSNode(sn);
            }
#elif defined(SF_AS3_AOTC2)
            if (NeedToDiscardResult())
                PushSNodeAbrupt(AOT::SNodeAbrupt::opReturn);
            else
                PushSNodeAbrupt(AOT::SNodeAbrupt::opReturnValue);
#endif

            PopOp();

            // Possible dead block.
            tr.AddBlock(*this, bcp, Block::tDead, true);

            // Exceptions are possible, but we can ignore them here.
            break;
        case Code::op_returnvoid:
            tr.PushNewOpCode(Abc::Code::op_returnvoid);

#if defined(SF_AS3_AOTC)
            {
                using namespace AOT;

                Pickable<SNodeAbrupt> sn(SF_NEW SNodeAbrupt(SNodeAbrupt::opReturn, SPtr<SNodeBlock>(), bcp SF_DEBUG_ARG(OpcodeCP)));

                PushSNode(sn);
            }
#elif defined(SF_AS3_AOTC2)
            PushSNodeAbrupt(AOT::SNodeAbrupt::opReturn);
#endif

            // Possible dead block.
            tr.AddBlock(*this, bcp, Block::tDead, true);

            // Exceptions are possible, but we can ignore them here.
            break;
        case Code::op_constructsuper:
            exec_constructsuper(ReadU30(pCode, bcp));
            break;
        case Code::op_constructprop:
            // Should be handled by SubstituteOpCode().
            SF_ASSERT(false);
            break;
        case Code::op_callproplex:
            // Should be handled by SubstituteOpCode().
            SF_ASSERT(false);
            break;
        case Code::op_callsupervoid:
            // Should be handled by SubstituteOpCode().
            SF_ASSERT(false);
            break;
        case Code::op_callpropvoid:
            // Should be handled by SubstituteOpCode().
            SF_ASSERT(false);
            break;
        case Code::op_sxi1:
            exec_sxi1();
            break;
        case Code::op_sxi8:
            exec_sxi8();
            break;
        case Code::op_sxi16:
            exec_sxi16();
            break;
        case Code::op_applytype:
            exec_applytype(ReadU30(pCode, bcp));
            break;
        case Code::op_newobject:
            exec_newobject(ReadU30(pCode, bcp));
            break;
        case Code::op_newarray:
            exec_newarray(ReadU30(pCode, bcp));
            break;
        case Code::op_newactivation:
            exec_newactivation();
            break;
        case Code::op_newclass:
            exec_newclass(ReadU30(pCode, bcp));
            break;
        case Code::op_getdescendants:
            exec_getdescendants(ReadU30(pCode, bcp));
            break;
        case Code::op_newcatch:
            exec_newcatch(ReadU30(pCode, bcp));
            break;
        case Code::op_findpropstrict:
            exec_findpropstrict(ReadU30(pCode, bcp));
            break;
        case Code::op_findproperty:
            exec_findproperty(ReadU30(pCode, bcp));
            break;
        case Code::op_getlex:
            exec_getlex(ReadU30(pCode, bcp));
            break;
        case Code::op_setproperty:
            // Should be handled by SubstituteOpCode().
            SF_ASSERT(false);
            break;
        case Code::op_setlocal:
            exec_setlocal(ReadU30(pCode, bcp));
            break;
        case Code::op_getglobalscope:
            exec_getglobalscope();
            break;
        case Code::op_getscopeobject:
            exec_getscopeobject(ReadU30(pCode, bcp));
            break;
        case Code::op_getproperty:
        case Code::op_initproperty:
            // Should be handled by SubstituteOpCode().
            SF_ASSERT(false);
            break;
        case Code::op_deleteproperty:
            exec_deleteproperty(ReadU30(pCode, bcp));
            break;
        case Code::op_getslot:
            exec_getslot(ReadU30(pCode, bcp));
            break;
        case Code::op_setslot:
            exec_setslot(ReadU30(pCode, bcp));
            break;
        case Code::op_getglobalslot:
            exec_getglobalslot(ReadU30(pCode, bcp));
            break;
        case Code::op_setglobalslot:
            exec_setglobalslot(ReadU30(pCode, bcp));
            break;
        case Code::op_convert_s:
            tr.PushNewOpCode(Abc::Code::op_convert_s);

            exec_1OpString(SF_AOTC_CODE(AOT::SNode1::opConvertS) SF_AOTC2_CODE(AOT::SNodeOC::op_convert_s));
            break;
        case Code::op_esc_xelem:
            exec_esc_xelem();
            break;
        case Code::op_esc_xattr:
            exec_esc_xattr();
            break;
        case Code::op_convert_i:
            exec_convert_i();
            break;
        case Code::op_convert_u:
            exec_convert_u();
            break;
        case Code::op_convert_d:
            exec_convert_d();
            break;
        case Code::op_convert_b:
            exec_convert_b();
            break;
        case Code::op_convert_o:
            exec_convert_o();
            break;
        case Code::op_checkfilter:
            exec_checkfilter();
            break;
        case Code::op_coerce:
            // Should be handled by SubstituteOpCode().
            SF_ASSERT(false);
            break;
        case Code::op_coerce_a:
            // Should be handled by SubstituteOpCode().
            SF_ASSERT(false);
            break;
        case Code::op_coerce_s:
            exec_coerce_s();
            break;
        case Code::op_astype:
            exec_astype(ReadU30(pCode, bcp));
            break;
        case Code::op_astypelate:
            // Should be handled by SubstituteOpCode().
            SF_ASSERT(false);
            break;
        case Code::op_negate:
            exec_negate();
            break;
        case Code::op_increment:
            tr.PushNewOpCode(Abc::Code::op_increment);

            exec_1OpNumber(SF_AOTC_CODE(AOT::SNode1::opIncr) SF_AOTC2_CODE(AOT::SNodeOC::op_increment));
            break;
        case Code::op_inclocal:
            exec_inclocal(ReadU30(pCode, bcp));
            break;
        case Code::op_decrement:
            tr.PushNewOpCode(Abc::Code::op_decrement);

            exec_1OpNumber(SF_AOTC_CODE(AOT::SNode1::opDecr) SF_AOTC2_CODE(AOT::SNodeOC::op_decrement));
            break;
        case Code::op_declocal:
            exec_declocal(ReadU30(pCode, bcp));
            break;
        case Code::op_typeof:
            tr.PushNewOpCode(Abc::Code::op_typeof);

            exec_1OpString(SF_AOTC_CODE(AOT::SNode1::opTypeOf) SF_AOTC2_CODE(AOT::SNodeOC::op_typeof));
            break;
        case Code::op_not:
            exec_not();
            break;
        case Code::op_bitnot:
            tr.PushNewOpCode(Abc::Code::op_bitnot);

            exec_1OpSInt(SF_AOTC_CODE(AOT::SNode1::opBitNot) SF_AOTC2_CODE(AOT::SNodeOC::op_bitnot));
            break;
        case Code::op_add_d:
            exec_add_d();
            break;
        case Code::op_add:
            // Should be handled by SubstituteOpCode().
            SF_ASSERT(false);
            break;
        case Code::op_subtract:
            exec_subtract();
            break;
        case Code::op_multiply:
            exec_multiply();
            break;
        case Code::op_divide:
            exec_divide();
            break;
        case Code::op_modulo:
            tr.PushNewOpCode(Abc::Code::op_modulo);

            exec_2OpNumber(SF_AOTC_CODE(AOT::SNode2::opModulo));
            break;
        case Code::op_lshift:
            tr.PushNewOpCode(Abc::Code::op_lshift);

            exec_2OpSInt(SF_AOTC_CODE(AOT::SNode2::opLSchift));
            break;
        case Code::op_rshift:
            tr.PushNewOpCode(Abc::Code::op_rshift);

            exec_2OpSInt(SF_AOTC_CODE(AOT::SNode2::opRSchift));
            break;
        case Code::op_urshift:
            tr.PushNewOpCode(Abc::Code::op_urshift);

            exec_2OpUInt(SF_AOTC_CODE(AOT::SNode2::opURSchift));
            break;
        case Code::op_bitand:
            tr.PushNewOpCode(Abc::Code::op_bitand);

            exec_2OpSInt(SF_AOTC_CODE(AOT::SNode2::opBitAnd));
            break;
        case Code::op_bitor:
            tr.PushNewOpCode(Abc::Code::op_bitor);

            exec_2OpSInt(SF_AOTC_CODE(AOT::SNode2::opBitOr));
            break;
        case Code::op_bitxor:
            tr.PushNewOpCode(Abc::Code::op_bitxor);

            exec_2OpSInt(SF_AOTC_CODE(AOT::SNode2::opBitXOr));
            break;
        case Code::op_equals:
            tr.PushNewOpCode(Abc::Code::op_equals);

            exec_2OpBoolean(SF_AOTC_CODE(AOT::SNodeBoolExpr::opEQ));
            break;
        case Code::op_strictequals:
            tr.PushNewOpCode(Abc::Code::op_strictequals);

            exec_2OpBoolean(SF_AOTC_CODE(AOT::SNodeBoolExpr::opStrictEQ));
            break;
        case Code::op_lessthan:
            tr.PushNewOpCode(Abc::Code::op_lessthan);

            exec_2OpBoolean(SF_AOTC_CODE(AOT::SNodeBoolExpr::opLT));
            break;
        case Code::op_lessequals:
            tr.PushNewOpCode(Abc::Code::op_lessequals);

            exec_2OpBoolean(SF_AOTC_CODE(AOT::SNodeBoolExpr::opLE));
            break;
        case Code::op_greaterthan:
            tr.PushNewOpCode(Abc::Code::op_greaterthan);

            exec_2OpBoolean(SF_AOTC_CODE(AOT::SNodeBoolExpr::opGT));
            break;
        case Code::op_greaterequals:
            tr.PushNewOpCode(Abc::Code::op_greaterequals);

            exec_2OpBoolean(SF_AOTC_CODE(AOT::SNodeBoolExpr::opGE));
            break;
        case Code::op_istypelate:
            tr.PushNewOpCode(Abc::Code::op_istypelate);

            exec_2OpBoolean(SF_AOTC_CODE(AOT::SNodeBoolExpr::opIsTypeLate));
            break;
        case Code::op_instanceof:
            tr.PushNewOpCode(Abc::Code::op_instanceof);

            exec_2OpBoolean(SF_AOTC_CODE(AOT::SNodeBoolExpr::opInstanceOf));
            break;
        case Code::op_istype:
            exec_istype(ReadU30(pCode, bcp));
            break;
        case Code::op_in:
            tr.PushNewOpCode(Abc::Code::op_in);

            exec_2OpBoolean(SF_AOTC_CODE(AOT::SNodeBoolExpr::opIn));
            break;
        case Code::op_increment_i:
            exec_increment_i();
            break;
        case Code::op_decrement_i:
            exec_decrement_i();
            break;
        case Code::op_negate_i:
            exec_negate_i();
            break;
        case Code::op_add_i:
            exec_add_i();
            break;
        case Code::op_subtract_i:
            exec_subtract_i();
            break;
        case Code::op_multiply_i:
            exec_multiply_i();
            break;
        case Code::op_inclocal_i:
            exec_inclocal_i(ReadU30(pCode, bcp));
            break;
        case Code::op_declocal_i:
            exec_declocal_i(ReadU30(pCode, bcp));
            break;
        case Code::op_getlocal:
            exec_getlocal(ReadU30(pCode, bcp));
            break;
        case Code::op_getlocal0:
            exec_getlocal0();
            break;
        case Code::op_getlocal1:
            exec_getlocal1();
            break;
        case Code::op_getlocal2:
            exec_getlocal2();
            break;
        case Code::op_getlocal3:
            exec_getlocal3();
            break;
        case Code::op_setlocal0:
            exec_setlocal0();
            break;
        case Code::op_setlocal1:
            exec_setlocal1();
            break;
        case Code::op_setlocal2:
            exec_setlocal2();
            break;
        case Code::op_setlocal3:
            exec_setlocal3();
            break;
        case Code::op_debug:
            tr.PushNewOpCode(Abc::Code::op_debug);

            tr.PushNewOpCodeArg0(Read8(pCode, bcp));
            tr.PushNewOpCodeArg0(ReadU30(pCode, bcp));
            tr.PushNewOpCodeArg0(Read8(pCode, bcp));
            tr.PushNewOpCodeArg0(ReadU30(pCode, bcp));
            break;
        case Code::op_debugline:
            {
                tr.PushNewOpCode(Abc::Code::op_debugline);

                const int line_num = ReadU30(pCode, bcp);

                tr.PushNewOpCodeArg0(line_num);

#if defined(SF_AS3_AOTC2)
                sb.Clear();
                sb << "Line Num: " << line_num;
                PushSNodeComment(sb);
#endif
            }
            break;
        case Code::op_debugfile:
            {
                tr.PushNewOpCode(Abc::Code::op_debugfile);

                const int str_ind = ReadU30(pCode, bcp);

                tr.PushNewOpCodeArg0(str_ind);

#if defined(SF_AS3_AOTC2)
                const StringDataPtr fn = tr.GetAbcFile().GetConstPool().GetString(AbsoluteIndex(str_ind));
                sb.Clear();
                sb << "File Name: " << fn;
                PushSNodeComment(sb);
#endif
            }
            break;
        case Code::op_0xF2:
            ReadU30(pCode, bcp);
            // ???
            break;
        default:
            // This part should be gone eventually.
            switch (Code::GetOpCodeInfo(opcode).operandCount)
            {
            case 1:
                {
                    const int _1 = ReadU30(pCode, bcp);
                    tr.PushNewOpCode(opcode, _1);

#if defined(SF_AS3_AOTC2)
                    PushSNodeOC(AOT::SNodeOC::GetOP(opcode), _1);
#endif
                }
                break;
            case 2:
                {
                    const int _1 = ReadU30(pCode, bcp);
                    const int _2 = ReadU30(pCode, bcp);

                    tr.PushNewOpCode(opcode, _1, _2);

#if defined(SF_AS3_AOTC2)
                    PushSNodeOC(AOT::SNodeOC::GetOP(opcode), _1, _2);
#endif
                }
                break;
            case 0:
                tr.PushNewOpCode(opcode);

#if defined(SF_AS3_AOTC2)
                PushSNodeOC(AOT::SNodeOC::GetOP(opcode));
#endif
                break;
            default:
                GetVM().ThrowVerifyError(VM::Error(VM::eNotImplementedError, GetVM() SF_DEBUG_ARG(Code::GetOpCodeInfo(opcode).name)));
                break;
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    Block::Block(Abc::TCodeOffset f)
    : IsInitialized(true)
    , Type(tUnknown)
    , State(NULL)
    , From(f)
    , CurOpStackSize(0)
#if defined(SF_AS3_AOTC) || defined(SF_AS3_AOTC2)
    , SwitchOffset(0)
    , TryOffset(0)
#endif
    {
#if defined(SF_AS3_AOTC)
        Nodes.Pick(SF_HEAP_AUTO_NEW(GetThis()) AOT::SNodeBlock(*this SF_DEBUG_ARG(f)));
#elif defined(SF_AS3_AOTC2)
        Nodes.Pick(SF_HEAP_AUTO_NEW(GetThis()) AOT::SNodeBlock(f));
#endif
    }

    Block::Block(TR::State& st, Abc::TCodeOffset f, EType t) 
    : IsInitialized(true)
    , Type(t)
    , State(&st)
    , From(f)
    , CurOpStackSize(0)
#if defined(SF_AS3_AOTC) || defined(SF_AS3_AOTC2)
    , SwitchOffset(0)
    , TryOffset(0)
#endif
    {
#if defined(SF_AS3_AOTC)
        Nodes.Pick(SF_HEAP_AUTO_NEW(GetThis()) AOT::SNodeBlock(*this SF_DEBUG_ARG(f)));
#elif defined(SF_AS3_AOTC2)
        Nodes.Pick(SF_HEAP_AUTO_NEW(GetThis()) AOT::SNodeBlock(f));
#endif
    }

    Block::~Block()
    {
    }
} // namespace TR

///////////////////////////////////////////////////////////////////////////////
Tracer::Tracer(
    MemoryHeap* heap,
    const CallFrame& cf,
    MethodInfo& mi
    SF_AOTC_ARG(AOT::InfoCollector* ic)
    SF_AOTC2_ARG(AOT::InfoCollector* ic)
    )
    : Done(false)
    , Heap(heap)
    , CF(cf)
    , MI(mi)
    , CurrOffset(0)
    , BCode(cf.GetMethodBodyInfo().GetCode().GetCode())
    , pCode(reinterpret_cast<const UInt8*>(BCode.ToCStr()))
    , CodeEnd(BCode.GetSize())
    , PrintOffset(0)
    , CurrBlock(NULL)
    , OrigOpcodePos(heap)
    , NewOpcodePos(heap)
    , PosToRecalculate(heap)
    , Orig2newPosMap(heap)
    , States(heap)
    , CatchTraits(heap)
#if defined(SF_AS3_AOTC) || defined(SF_AS3_AOTC2)
    , pIC(ic)
#endif
#if defined(SF_AS3_AOTC)
    , HandledSwitchStmt(heap)
    , RTHash(heap)
#endif
{
    VM& vm = GetVM();

    // Switch on State tracing
    {
        SetNeedToCheckOpCode(vm.GetUI().NeedToCheckOpCode());
        SetState(FlashUI::sStep);
        PrintOffset = vm.GetCallStack().GetSize();

        if (PrintOffset)
            --PrintOffset;
    }

    MI.OpCode.Clear();
    MI.OpCode.Reserve(BCode.GetSize());
    Orig2newPosMap.Resize(BCode.GetSize());

    TR::State* st = SF_HEAP_NEW(GetHeap()) TR::State(*this, 0);
    States.PushBack(st);
    ValueArrayDH& reg_file = st->Registers;
    FixedBitSetDH<StatMV_VM_Tracer_Mem>& reg_alive = st->RegistersAlive;
    unsigned reg = 0;

    // Initialize registers ...
    {
        // Initialize *this*
        {
            Traits& ot = const_cast<Traits&>(CF.GetOriginationTraits());

#ifdef SF_AS3_AOTC
            ASString this_str = vm.GetStringManager().CreateConstString("this");
            SPtr<SNode> sn;
            // Mark this name as "this".
            sn.Pick(SF_NEW AOT::SNodeVar(this_str, ot, true));
#endif

            // We MAY NOT use real object for *this* because this may be passed
            // at run time and can be of different type
            // "this" is not null.
            if (ot.IsClassTraits())
            {
                ClassTraits::Traits& ctr = static_cast<ClassTraits::Traits&>(ot);
                reg_file.PushBack(Value(ctr, Value::NotNull SF_AOTC_ARG(*sn)));
            }
            else
            {
                const Value& invoker = CF.GetInvoker();

                if (invoker.GetKind() == Value::kFunction)
                    // Correct object type for constructor function.
                    reg_file.PushBack(Value(GetObjectType(), Value::NotNull SF_AOTC_ARG(*sn)));
                else
#if 1
                    // This is the original way of doing things.
                    reg_file.PushBack(Value(static_cast<InstanceTraits::Traits&>(ot), Value::NotNull SF_AOTC_ARG(*sn)));
#else
                    // This has a chance to work, but it looks like this breaks a lot of optimization.
                    reg_file.PushBack(invoker);
#endif
            }

            // "this" is always alive.
            reg_alive.Set(reg++);
        }

        ///////////
        // Set up the rest of the registers.
        ///////////

        const Abc::MethodBodyInfo& mbi = GetMethodBodyInfo();

        const Abc::MethodInfo& mifo = GetMethodBodyInfo().GetMethodInfo(GetAbcFile().GetMethods());
        const SInt32 maxInd = mbi.GetMaxLocalRegisterIndex();

        for(SInt32 i = 1; i < maxInd; ++i)
            reg_file.PushBack(Value::GetUndefined());

        // Get register's types.
        const UPInt param_count = mifo.GetParamCount();
#ifdef SF_AS3_AOTC
        StringManager& sm = vm.GetStringManager();
#endif

        // Registers 1 through method_info.param_count holds parameter values coerced to the declared types
        // of the parameters. If fewer than method_body_info.local_count values are supplied to the call then
        // the remaining values are either the values provided by default value declarations (optional arguments)
        // or the value undefined.

        for(UPInt i = 0; i < param_count; ++i)
        {
            const Abc::Multiname& amn = mifo.GetParamType(GetAbcFile().GetConstPool(), i);
            const ClassTraits::Traits* tr = vm.Resolve2ClassTraits(GetFile(), amn);

#ifdef SF_AS3_AOTC
            StringBuffer sink;
            if (mi.HasParamNames())
            {
                const StringDataPtr sdp = mi.GetParamName(GetFile().GetConstPool(), i);
                sink.AppendString(sdp.ToCStr(), sdp.GetSize());
            }
            else
                Format(sink, "ARG{0}", i);

            SPtr<SNode> sn;
            const ASString name = sm.CreateString(sink.ToCStr(), sink.GetSize());
            sn.Pick(SF_NEW AOT::SNodeVar(name, tr));
#endif

            if (tr)
            {
                InstanceTraits::Traits& itr = tr->GetInstanceTraits();
                reg_file[reg] = Value(itr, CanBeNull(itr) SF_AOTC_ARG(*sn));
                reg_alive.Set(reg++);
            }
            else
            {
                // Class wasn't found. It happens.
                vm.ThrowVerifyError(VM::Error(VM::eClassNotFoundError, vm
                    SF_DEBUG_ARG(GetFile().GetInternedString(amn.GetNameInd()))
                    ));

                return;
            }
        }

        if (mifo.NeedRest())
        {
            // If NEED_REST is set in method_info.flags, the method_info.param_count+1 register is set up to
            // reference an array that holds the superfluous arguments.

#ifdef SF_AS3_AOTC
            SPtr<SNode> sn;
            const ASString name = sm.CreateConstString("...");
            sn.Pick(SF_NEW AOT::SNodeVar(name, &vm.GetITraitsArray()));
#endif

            reg_file[reg] = Value(vm.GetClassTraitsArray().GetInstanceTraits(), Value::NotNull SF_AOTC_ARG(*sn));
            reg_alive.Set(reg++);
        } else if (mifo.NeedArguments())
        {
            // If NEED_ARGUMENTS is set in method_info.flags, the method_info.param_count+1 register is set up
            // to reference an "arguments" object that holds all the actual arguments: see ECMA-262 for more
            // information. (The AVM2 is not strictly compatible with ECMA-262; it creates an Array object for
            // the "arguments" object, whereas ECMA-262 requires a plain Object.)

#ifdef SF_AS3_AOTC
            SPtr<SNode> sn;
            const ASString name = sm.CreateConstString("ArgArray");
            sn.Pick(SF_NEW AOT::SNodeVar(name, &vm.GetITraitsArray()));
#endif

            // "arguments" is a regular Array. There is no an explicit "arguments" class.
            reg_file[reg] = Value(vm.GetClassTraitsArray().GetInstanceTraits(), Value::NotNull SF_AOTC_ARG(*sn));
            reg_alive.Set(reg++);
        }
    }

    // Create initial state/block.
    Blocks.PushBack(SF_HEAP_NEW(GetHeap()) TR::Block(*st, 0, TR::Block::tUnknown));
}

Tracer::~Tracer()
{
    // Free list elements.
    {
        TR::Block* block = Blocks.GetFirst();
        while(!Blocks.IsNull(block))
        {
            TR::Block* next = Blocks.GetNext(block);

            block->RemoveNode();
            delete block;
            block = next;
        }

        Blocks.Clear();
    }

    // Free States.
    const UPInt size = States.GetSize();
    for (UPInt i = 0; i < size; ++i)
        delete States[i];
}

void Tracer::EmitCode()
{
    using namespace Abc;

    // Sometimes compiler generates dummy pCode, which produces
    // absolute address outside of a pCode block. Such opcodes should
    // be dropped from a newly generated pCode. TBD.

    if (Done)
        return;

#ifdef GFX_AS3_TRACE
    FlashUI& ui = GetUI();

    if (ui.NeedToCheckOpCode())
    {
        MakeOffset(ui, GetPrintOffset());

        ui.Output(FlashUI::Output_Action, "=== Start optimization ");
        SF_DEBUG_CODE(ui.Output(FlashUI::Output_Action, GetName().ToCStr());)
        ui.Output(FlashUI::Output_Action, " ===\n");
    }
#endif

    VM& vm = GetVM();
    const MethodBodyInfo::Exception& e = GetMethodBodyInfo().exception;

    // Create blocks for exceptions if any.
    {
        for (UPInt i = 0; i < e.GetSize(); ++i)
        {
            const MethodBodyInfo::ExceptionInfo& ei = e.Get(i);

            // A "from" block is uninitialized, but AddBlock() will mark it as initialized.
            // We will need to reset this flag.
            const UInd from = ei.GetFrom();
            TR::Block* try_block = AddBlock(GetFirstBlock().GetState(), from, TR::Block::tUnknown, false);

            if (try_block)
            {
                // Reset the "initialized" flag.
                try_block->SetInitialized(false);
                try_block->SetTryBlock();
#ifdef SF_AS3_AOTC
                try_block->SetTryOffset(from);
#endif
            }

            // !!! ei.GetTargetPos() can be a dead block, if there is no throw.
            // If there is no explicit throw opcode we need to simulate it,
            // otherwise stack will be broken.
            // We can specify only the beginning of the block.
            // catch_block is already initialized.
            // A "target" block is initialized.
            TR::Block* catch_block = AddBlock(GetFirstBlock().GetState(), ei.GetTargetPos(), TR::Block::tUnknown, false);

            if (catch_block)
            {
                const SInt32 et_ind = ei.GetExceptionTypeInd();
                catch_block->SetCatchBlock();

#ifdef SF_AS3_AOTC
                catch_block->SetTryOffset(from);
                try_block->Catches.PushBack(catch_block);

                // Create catch node.
                // !!! No parent is set up.
                catch_block->Catch.Pick(SF_NEW AOT::SNodeCatch(SF_DEBUG_CODE(ei.GetTargetPos())));
                // Circular dependency.
                catch_block->AddTarget(catch_block->Catch->Body);

                UInt32 ind = ei.GetVariableNameInd();
//                     // Adobe is doing weird things with exception variable name.
//                     if (ind)
//                         ind = static_cast<UInt32>(file.GetConstPool().GetStringCount() - ind);
                if (ind)
                    ++ind;

                const ASString name = GetFile().GetInternedString(ind);

                // Create variable name of this catch block.
                if (et_ind == 0)
                    catch_block->Catch->Var.Pick(SF_NEW AOT::SNodeVar(name, &vm.GetITraitsObject()));
                else
                {
                    VMAbcFile& file = GetFile();
                    const Abc::Multiname& amn = file.GetConstPool().GetMultiname(et_ind);
                    const ClassTraits::Traits* etr = vm.Resolve2ClassTraits(GetFile(), amn);

                    if (etr)
                        catch_block->Catch->Var.Pick(SF_NEW AOT::SNodeVar(name, etr));
                    else
                    {
                        // Class wasn't found. It happens.
                        return vm.ThrowVerifyError(VM::Error(VM::eClassNotFoundError, vm
                            SF_DEBUG_ARG(file.GetInternedString(amn.GetNameInd()))
                            ));
                    }
                }
#endif

                TR::State& state = GetState(*catch_block);

                if (et_ind == 0)
                {
                    // This handler can handle any type of exceptions ...
                    state.PushOp(Value(GetObjectType(), Value::NullOrNot SF_AOTC_ARG(*catch_block->Catch->Var)));
                }
                else
                {
                    VMAbcFile& file = GetFile();
                    const Abc::Multiname& amn = file.GetConstPool().GetMultiname(et_ind);
                    const ClassTraits::Traits* etr = vm.Resolve2ClassTraits(GetFile(), amn);

                    if (etr)
                    {
                        InstanceTraits::Traits& itr = etr->GetInstanceTraits();
                        state.PushOp(Value(itr, CanBeNull(itr) SF_AOTC_ARG(*catch_block->Catch->Var)));
#ifdef SF_AS3_AOTC
                        // Create type of this catch block.
                        catch_block->Catch->Type.Pick(SF_NEW AOT::SNodeVar(etr->GetName(), etr));
#endif
                    }
                    else
                    {
                        // Class wasn't found. It happens.
                        return vm.ThrowVerifyError(VM::Error(VM::eClassNotFoundError, vm
                            SF_DEBUG_ARG(file.GetInternedString(amn.GetNameInd()))
                            ));
                    }
                }
            }
        }
    }

    TraceBlock(0, GetFirstBlock());

    if (vm.IsException())
    {
        // Clear pCode to prevent execution of not completely generated pCode.
        MI.OpCode.Clear();
        return;
    }

#ifdef GFX_AS3_TRACE
    if (ui.NeedToCheckOpCode())
    {
        MakeOffset(ui, GetPrintOffset());

        ui.Output(FlashUI::Output_Action, "=== Finish optimization ");
        SF_DEBUG_CODE(ui.Output(FlashUI::Output_Action, GetName().ToCStr());)
        ui.Output(FlashUI::Output_Action, " ===\n");
    }
#endif

    // Recalculate positions.
    for(UPInt i = 0; i < PosToRecalculate.GetSize(); ++i)
    {
        // Get position in MI.OpCode where an absolute address is stored.
        const Recalculate& r = PosToRecalculate[i];
        const Abc::TCodeOffset pos = r.pos;
        const TOpCode::ValueType absolute_address = MI.OpCode[pos];

        // Fix relative offset.
        // Convert it back from absolute to relative.
        TOpCode::ValueType new_address = 0;

        // It is possible that compiler generates dummy pCode, which addresses
        // pCode outside of a pCode segment. Let's set address to ZERO in such
        // case.
        if (absolute_address < Orig2newPosMap.GetSize())
        {
            // long version for debugging.
            new_address = static_cast<TOpCode::ValueType>(Orig2newPosMap[absolute_address]);
            new_address -= static_cast<TOpCode::ValueType>(pos);
            new_address += r.base;
        }

        SF_ASSERT(new_address < MI.OpCode.GetSize());
        MI.OpCode[pos] = new_address;
    }

    // Create exception info for word pCode.
    {
        const MethodBodyInfo::Exception& embi = GetMethodBodyInfo().exception;
        for (UPInt i = 0; i < embi.GetSize(); ++i)
        {
            const MethodBodyInfo::ExceptionInfo& ei = embi.Get(i);

            MI.Exception.info.PushBack(MethodBodyInfo::ExceptionInfo(
                static_cast<SInd>(Orig2newPosMap[ei.GetFrom()]),
                static_cast<SInd>(Orig2newPosMap[ei.GetTo()]),
                static_cast<SInd>(Orig2newPosMap[ei.GetTargetPos()]),
                ei.GetExceptionTypeInd(),
                ei.GetVariableNameInd()
                ));
        }
    }

    // A hack with constness.
    {
        Abc::MethodBodyInfo& mbi = const_cast<Abc::MethodBodyInfo&>(CF.GetMethodBodyInfo());

        // Clear original pCode.
        mbi.ClearCode();
    }

    Done = true;
}

CheckResult Tracer::UpdateBlock(const Abc::TCodeOffset bcp SF_AOTC_ARG(Abc::Code::OpCode opcode) SF_AOTC2_ARG(Abc::Code::OpCode opcode))
{
    const TR::Block& prevBlock = *CurrBlock;
    TR::Block* nextBlock = GetBlock(bcp);

    if (nextBlock && nextBlock->GetFrom() == bcp)
    {
        // We entered a new block.

        TR::Block* pBlock = nextBlock;
        CurrBlock = pBlock;

#ifdef GFX_AS3_TRACE
        FlashUI& ui = GetUI();
#endif

#if defined(SF_AS3_AOTC) || defined(SF_AS3_AOTC2)
        using namespace AOT;

        // Add missing terminating statement.
        if (
            bcp != 0 && 
            opcode != Abc::Code::op_label && 
            !pBlock->IsDeadBlock() && 
            !pBlock->IsTryBlock() && 
            !IsTerminatingOpCode(opcode)
            )
        {
            const Abc::Code::OpCode prev_opcode = GetOrigPrevOpCode2(bcp);

            if (prev_opcode != Abc::Code::op_returnvoid && prev_opcode != Abc::Code::op_returnvalue)
            {
                SNodeBlock::TElems& elems = prevBlock.GetNodes().Elems;
                SF_DEBUG_CODE(UPInt addr = 0);
                SPtr<SNode> sn;

                SF_DEBUG_CODE(if (!elems.IsEmpty()) addr = elems.Back()->GetAddr());

                sn.Pick(SF_NEW SNodeAbrupt(SNodeAbrupt::opGoto, &pBlock->GetNodes() SF_DEBUG_ARG(addr)));
                elems.PushBack(sn);
#ifdef GFX_AS3_TRACE
                if (ui.NeedToCheckOpCode())
                {
                    LongFormatter f(pBlock->GetFrom());
                    f.Convert();

                    MakeOffset(ui, GetPrintOffset());
                    ui.Output(FlashUI::Output_Action, "AC jump ");
                    ui.Output(FlashUI::Output_Action, f.ToCStr());
                    ui.Output(FlashUI::Output_Action, "\n");
                }
#endif
            }
        }
#endif

#ifdef GFX_AS3_TRACE
        if (ui.NeedToCheckOpCode())
        {
            MakeOffset(ui, GetPrintOffset());
            ui.Output(FlashUI::Output_Action, "// Basic Block.\n");

            if (pBlock->IsDeadBlock())
            {
                MakeOffset(ui, GetPrintOffset());
                ui.Output(FlashUI::Output_Action, "// Dead Block.\n");
            }

            if (pBlock->IsExcTryBlock())
            {
                MakeOffset(ui, GetPrintOffset());
                ui.Output(FlashUI::Output_Action, "// Try Block.\n");
            }

            if (pBlock->IsExcCatchBlock())
            {
                MakeOffset(ui, GetPrintOffset());
                ui.Output(FlashUI::Output_Action, "// Catch Block.\n");
            }

            if (pBlock->IsExcFinallyBlock())
            {
                MakeOffset(ui, GetPrintOffset());
                ui.Output(FlashUI::Output_Action, "// Finally Block.\n");
            }

            if (pBlock->IsLoopBodyBlock())
            {
                MakeOffset(ui, GetPrintOffset());
                ui.Output(FlashUI::Output_Action, "// Loop Body Block.\n");
            }

            if (pBlock->IsLoopConditionBlock())
            {
                MakeOffset(ui, GetPrintOffset());
                ui.Output(FlashUI::Output_Action, "// Loop Condition Block.\n");
            }

            if (pBlock->IsSwitchCaseBlock())
            {
                MakeOffset(ui, GetPrintOffset());
                ui.Output(FlashUI::Output_Action, "// Switch Case Block.\n");
            }
        }
#endif

#ifdef SF_AS3_AOTC
        if (pBlock->IsTryBlock())
        {
            // Let's add a new exception handling node.

            Pickable<AOT::SNodeException> sn_exc(SF_NEW AOT::SNodeException(SF_DEBUG_CODE(pBlock->GetFrom())));
            prevBlock.PushSNode(sn_exc);

            // Current block belongs to the try part of the exception handling node.
            pBlock->AddTarget(sn_exc->Try);

            // Handle catches.
            TR::Block* block = NULL;
            const UPInt size = pBlock->Catches.GetSize();
            sn_exc->Catches.Reserve(size);

            // Fill up sn_exc->Catches.
            for (UPInt i = 0; i < size; ++i)
            {
                block = pBlock->Catches[i];
                SF_ASSERT(block);
                SF_ASSERT(block->IsCatchBlock());
                sn_exc->Catches.PushBack(block->Catch);

                // Change parent of the block. block belongs to the switch statement now.
                block->Catch->SetParent(sn_exc.GetPtr());
            }
        }
#endif

        if (pBlock->IsInitializedBlock())
        {
            if (pBlock != &prevBlock)
            {
                if (!MergeBlock(*pBlock, prevBlock))
                    // Oops. Exception.
                    return false;
            }
        }
        else
            InitializeBlock(*pBlock, prevBlock);
    }

    return true;
}

void Tracer::MarkAllBlocks()
{
    using namespace Abc;

    Code::OpCode opcode = Code::op_nop;
    Abc::TCodeOffset bcp = 0;

    while (bcp < CodeEnd)
    {
        opcode = static_cast<Code::OpCode>(Read8(pCode, bcp));

        //
        {
            const Code::OpCodeInfo& oi = Code::GetOpCodeInfo(opcode);

            if (oi.pop_mn)
                // Read multiname index.
                ReadU30(pCode, bcp);

            if (oi.pop_args)
                // Read number of popped arguments.
                ReadU30(pCode, bcp);

            if (oi.pop_args | oi.pop_mn)
                // We have already read all operands.
                continue;
        }

        // Read operands.
        switch (opcode)
        {
        case Code::op_label:
            // label is block by itself.
            AddBlock(bcp - 1);
            // Block starts AFTER label.
            // AddBlock(bcp);
            break;
        case Code::op_ifnlt:
        case Code::op_ifnle:
        case Code::op_ifngt:
        case Code::op_ifnge:
        case Code::op_ifeq:
        case Code::op_ifge:
        case Code::op_ifgt:
        case Code::op_ifle:
        case Code::op_iflt:
        case Code::op_ifne:
        case Code::op_ifstricteq:
        case Code::op_ifstrictne:
        case Code::op_iftrue:
        case Code::op_iffalse:
        case Code::op_jump:
            {
                const int offset = ReadS24(pCode, bcp);

                AddBlock(bcp);
                AddBlock(bcp + offset);
            }
            break;
        case Code::op_lookupswitch:
            {
                TCodeOffset base_location = bcp - 1;
                const int default_offset = ReadS24(pCode, bcp);
                AddBlock(base_location + default_offset);

                const int case_count = ReadU30(pCode, bcp);
                for(int i = 0; i <= case_count; ++i)
                {
                    const int offset = ReadS24(pCode, bcp);
                    AddBlock(base_location + offset);
                }
            }
            break;
        case Code::op_pushbyte:
            Read8(pCode, bcp);
            break;
        case Code::op_debug:
            Read8(pCode, bcp);
            ReadU30(pCode, bcp);
            Read8(pCode, bcp);
            ReadU30(pCode, bcp);
            break;
        default:
            switch (Code::GetOpCodeInfo(opcode).operandCount)
            {
            case 2:
                ReadU30(pCode, bcp);
                // No break on purpose.
            case 1:
                ReadU30(pCode, bcp);
                break;
            default:
                break;
            }
        }
    }
}

void Tracer::SkipDeadCode(Abc::TCodeOffset& bcp)
{
    using namespace Abc;

#ifdef GFX_AS3_TRACE
    FlashUI& ui = GetUI();
#endif

    TR::Block*& pBlock = CurrBlock;

    SF_ASSERT(pBlock);
    while (pBlock->IsDeadBlock() && bcp < CodeEnd)
    {
        // This call is mandatory to maintain Orig2newPosMap consistency.
        RegisterOrigOpCode(bcp);

        const UInt8 byte_code = static_cast<UInt8>(Read8(pCode, bcp));
        const Code::OpCode dopcode = static_cast<Code::OpCode>(byte_code);

#ifdef GFX_AS3_TRACE
        if (ui.NeedToCheckOpCode())
        {
            MakeOffset(ui, GetPrintOffset());
            ui.Output(FlashUI::Output_Action, "// Skip opcode below.\n");
            ui.OnOpCode(CurrOffset);
        }
#endif
        switch (dopcode)
        {
        case Code::op_ifnlt:
        case Code::op_ifnle:
        case Code::op_ifngt:
        case Code::op_ifnge:
        case Code::op_ifeq:
        case Code::op_ifge:
        case Code::op_ifgt:
        case Code::op_ifle:
        case Code::op_iflt:
        case Code::op_ifne:
        case Code::op_ifstricteq:
        case Code::op_ifstrictne:
        case Code::op_iftrue:
        case Code::op_iffalse:
        case Code::op_jump:
            ReadS24(pCode, bcp);
            break;
        case Code::op_lookupswitch:
            {
                ReadS24(pCode, bcp);
                int case_count = ReadU30(pCode, bcp);

                for(int i = 0; i <= case_count; ++i)
                    ReadS24(pCode, bcp);
            }
            break;
        case Code::op_pushbyte:
            Read8(pCode, bcp);
            break;
        case Code::op_debug:
            Read8(pCode, bcp);
            ReadU30(pCode, bcp);
            Read8(pCode, bcp);
            ReadU30(pCode, bcp);
            break;
        default:
            switch (Code::GetOpCodeInfo(dopcode).operandCount)
            {
            case 2:
                ReadU30(pCode, bcp);
                // No break on purpose.
            case 1:
                ReadU30(pCode, bcp);
                break;
            default:
                break;
            }
        }

        // Check for next block
        if (!UpdateBlock(bcp SF_AOTC_ARG(dopcode) SF_AOTC2_ARG(dopcode)))
            // Exception.
            return;
    }
}

#if defined(SF_AS3_AOTC) || defined(SF_AS3_AOTC2)
bool Tracer::IsTerminatingOpCode(Abc::Code::OpCode opcode)
{
    using namespace Abc;

    bool result = false;

    switch (opcode)
    {
    case Code::op_ifnlt:
    case Code::op_ifnle:
    case Code::op_ifngt:
    case Code::op_ifnge:
    case Code::op_ifeq:
    case Code::op_ifge:
    case Code::op_ifgt:
    case Code::op_ifle:
    case Code::op_iflt:
    case Code::op_ifne:
    case Code::op_ifstricteq:
    case Code::op_ifstrictne:
    case Code::op_iftrue:
    case Code::op_iffalse:
    case Code::op_jump:
    case Code::op_lookupswitch:
        result = true;
    default:
        break;
    }

    return result;
}
#endif

#ifdef SF_AS3_AOTC
void Tracer::MarkSwitchStatements(Abc::TCodeOffset bcp)
{
    using namespace Abc;

    TCodeOffset sws_offset;
    TCodeOffset ccp = bcp;
    TR::Block* block;

    TR::Block*& pBlock = CurrBlock;

    while ((sws_offset = GetNextSwitchStmt(ccp)) != 0)
    {
        TCodeOffset base_location = sws_offset;

        ++sws_offset;
        const int default_offset = ReadS24(pCode, sws_offset);

        // default_offset can point to the next statement after switch.
        // Do not check block-related opcodes.
        block = AddBlock(pBlock->GetState(), base_location + default_offset, TR::Block::tUnknown, false);

        // default_offset can be positive.
        if (default_offset < 0)
        {
            block->SetSwitchOffset(base_location);
            block->SetType(TR::Block::tSwitchCase);
        }

        int case_count = ReadU30(pCode, sws_offset);

        for(int i = 0; i <= case_count; ++i)
        {
            const int offset = ReadS24(pCode, sws_offset);

            block = AddBlock(pBlock->GetState(), base_location + offset, TR::Block::tSwitchCase, true);
            block->SetSwitchOffset(base_location);
        }
    }
}
#endif

void Tracer::TraceBlock(Abc::TCodeOffset bcp, const TR::Block& initBlock)
{
    using namespace Abc;

    TR::Block*& pBlock = CurrBlock;
    pBlock = GetBlock(bcp);
    SF_ASSERT(pBlock);

#if defined(SF_AS3_AOTC) || defined(SF_AS3_AOTC2)
    FirstNodeBlock = pBlock->Nodes;
#endif

    if (pBlock->IsInitializedBlock())
    {
        if (pBlock != &initBlock)
        {
            // Block is already initialized. We just need to merge states
            // and, probably, retrace it.
            if (!MergeBlock(*pBlock, initBlock))
                // Oops. Exception.
                return;
        }
    }
    else
    {
        // Block is not yet initialized and traced.
        InitializeBlock(*pBlock, initBlock);
    }

#ifdef GFX_AS3_TRACE
    FlashUI& ui = GetUI();
#endif

#ifdef SF_AS3_AOTC
    // Mark blocks without creating of states.
    // This can cause problems introduced by dead code.
    // (The create fake BB, which are not handled later.)
//     MarkAllBlocks();

    // Handle switch statements first.
    // We will create and mark blocks related to switch statements.
    MarkSwitchStatements(bcp);
#endif

    Code::OpCode opcode = Code::op_nop;

    while (bcp < CodeEnd)
    {
        if (GetVM().IsException())
            return;

        // Check for next block
        if (!UpdateBlock(bcp SF_AOTC_ARG(opcode) SF_AOTC2_ARG(opcode)))
            // Exception.
            return;

#ifndef SF_AS3_TRACE_DEAD_BLOCK
        // Handle dead blocks
        SkipDeadCode(bcp);

        if (bcp >= CodeEnd)
            break;
#endif

        // This call is mandatory to maintain Orig2newPosMap consistency.
        RegisterOrigOpCode(bcp);

        const UInt8 byte_code = static_cast<UInt8>(Read8(pCode, bcp));
        opcode = static_cast<Code::OpCode>(byte_code);

#ifdef GFX_AS3_TRACE
        if (ui.NeedToCheckOpCode() && ui.OnOpCode(CurrOffset)) { SF_ASSERT(false); }
#endif

        // Try to substitute with a strictly typed opcode.
        // Opcode can be replaced.
        if (SubstituteOpCode(opcode, bcp, pBlock->GetState()))
            continue;

        // SubstituteOpCode() can throw exceptions.
        if (GetVM().IsException())
            return;

        pBlock->GetState().exec_opcode(opcode, bcp);
    }
}

TR::Block* Tracer::GetBlock(Abc::TCodeOffset offset)
{
    TR::Block* block = Blocks.GetLast();

    // Skip blocks with bigger offset.
    while (block != NULL && offset < block->GetFrom())
    {
        block = Blocks.GetPrev(block);
    }

    return block;
}

void Tracer::StoreOffset(Abc::TCodeOffset bcp, const TR::State& st, SInt32 offset, int base SF_AOTC_ARG(SPtr<SNode>& target))
{
    using namespace Abc;

    const Abc::TCodeOffset bcp_offset = bcp + offset;

    if (offset < 0)
    {
        /* DO NOT delete this code.
        // This check proves that each back reference is marked with the Code::op_label.
        {
            TCodeOffset ccp = cp;
            const Code::OpCode offset_opcode = static_cast<Code::OpCode>(Read8(pCode + offset, ccp));
            SF_UNUSED(offset_opcode);
            SF_ASSERT(offset_opcode == Code::op_label);
        }
        */

        /* DO NOT delete this code.
        // This check proves that each Code::op_label is referenced only by ONE opcode.
        // It can be referenced twice within op_lookupswitch (because of "default" branch).
        {
            // HashDH<UPInt, UPInt> Refs2Labels;
            TCodeOffset ccp = bcp;
            const Code::OpCode offset_opcode = static_cast<Code::OpCode>(Read8(pCode + offset, ccp));
            const UPInt opcode_base = OrigOpcodePos.Back();

            if (offset_opcode == Code::op_label)
            {
                UPInt base = 0;
                if (Refs2Labels.Get(bcp_offset, &base))
                {
                    if (base != opcode_base)
                        SF_ASSERT(false);
                }
                else
                    Refs2Labels.Add(bcp_offset, opcode_base);
            }
        }
        */

#ifdef SF_AS3_AOTC
        TR::Block* block = GetBlock(bcp_offset);
        SF_ASSERT(block);
        block->AddTarget(target);
#endif

        // We already know a new address. We just need to get it and store it.
        TCodeOffset new_offset = Orig2newPosMap[bcp_offset] - MI.OpCode.GetSize() + base;

        //SF_ASSERT((MI.OpCode.GetSize() + new_offset) >= 0);
        PushNewOpCodeArg0(static_cast<Abc::TOpCode::ValueType>(new_offset));
    } else
    {
        TR::Block* block = AddBlock(st, bcp_offset, TR::Block::tUnknown, true);

#ifdef SF_AS3_AOTC

#ifdef SF_AS3_TRACE_DEAD_BLOCK
        if (!block->IsDeadBlock())
            block->AddTarget(target);
#else
        block->AddTarget(target);
#endif

#else
        SF_UNUSED1(block);
#endif

        // A new address is not known at this time because it is in front of current pCode pointer.
        // Translate offset to an absolute address and store it instead of a relative address.
        PushNewOpCodeArg0(static_cast<Abc::TOpCode::ValueType>(bcp_offset));
        // Store a position in MI.OpCode where we stored this absolute address, which we will need 
        // to recalculate later.
        PosToRecalculate.PushBack(Recalculate(MI.OpCode.GetSize() - 1, base));
    }
}

Abc::Code::OpCode Tracer::GetNewTopOpCode(UPInt num) const
{
    using namespace Abc;

    if (NewOpcodePos.GetSize() > num)
    {
        TCodeOffset ccp = NewOpcodePos[NewOpcodePos.GetSize() - 1 - num];
        const Code::OpCode prev_opcode = static_cast<Code::OpCode>(MI.OpCode[ccp]);

        return prev_opcode;
    }

    return Code::op_nop;
}

void Tracer::SkipOrigOpCode(Abc::TCodeOffset& opcode_cp, Abc::TCodeOffset new_cp)
{
    if (opcode_cp < CodeEnd)
    {
        // Store pos in case it is referenced.
        OrigOpcodePos.PushBack(opcode_cp);
        Orig2newPosMap[opcode_cp] = MI.OpCode.GetSize();
    }

    // This assert triggers in several tests.
    //SF_ASSERT(Orig2newPosMap[opcode_cp] < 100000);

    // Change CP.
    opcode_cp = new_cp;
}

void Tracer::RegisterOrigOpCode(Abc::TCodeOffset opcode_cp)
{
    if (opcode_cp < CodeEnd)
    {
        CurrOffset = opcode_cp;

        OrigOpcodePos.PushBack(opcode_cp);
        Orig2newPosMap[opcode_cp] = MI.OpCode.GetSize();

        // This assert triggers in several tests.
        //SF_ASSERT(Orig2newPosMap[opcode_cp] < 100000);
    }
}

Abc::Code::OpCode Tracer::GetOrigPrevOpCode(UPInt opcode_num_diff) const
{
    using namespace Abc;

    if (OrigOpcodePos.GetSize() > opcode_num_diff)
    {
        TCodeOffset ccp = OrigOpcodePos[OrigOpcodePos.GetSize() - opcode_num_diff - 1];
        const Code::OpCode prev_opcode = static_cast<Code::OpCode>(Read8(pCode, ccp));

        return prev_opcode;
    }

    return Code::op_nop;
}

Abc::Code::OpCode Tracer::GetOrigPrevOpCode2(Abc::TCodeOffset offset) const
{
    using namespace Abc;

    UPInt i = Alg::LowerBound(OrigOpcodePos, offset);
    TCodeOffset ccp = OrigOpcodePos[i - 1];
    const Code::OpCode prev_opcode = static_cast<Code::OpCode>(Read8(pCode, ccp));

    return prev_opcode;
}

Abc::Code::OpCode Tracer::GetOrigValueConsumer(Abc::TCodeOffset bcp) const
{
    using namespace Abc;

    Code::OpCode opcode = Code::op_nop;
    int st_size = 1;

    while (bcp < CodeEnd)
    {
        opcode = static_cast<Code::OpCode>(Read8(pCode, bcp));

        if (opcode == Code::op_jump)
        {
            int offset = ReadS24(pCode, bcp);
            bcp += offset;
            continue;
        }

        // Adjust stack size.
        {
            const Code::OpCodeInfo& oi = Code::GetOpCodeInfo(opcode);

            st_size -= oi.pop;

            if (oi.pop_mn)
            {
                const int mn_index = ReadU30(pCode, bcp);
                const Abc::Multiname& mn = GetAbcFile().GetConstPool().GetMultiname(mn_index);

                switch (mn.GetKind())
                {
                case Abc::MN_QName:
                case Abc::MN_QNameA:
                    // QName is not read from stack; both name and namespase known at compile-time.
                    break;
                case Abc::MN_RTQName:
                case Abc::MN_RTQNameA:
                    // Name is known at compile time, Namespace is resolved at runtime.
                    --st_size;
                    break;
                case Abc::MN_RTQNameL:
                case Abc::MN_RTQNameLA:
                    // Both name and namespace are resolved at runtime.
                    st_size -= 2;
                    break;
                case Abc::MN_Multiname:
                case Abc::MN_MultinameA:
                    // MInfo is not supposed to be read from stack.
                    break;
                case Abc::MN_MultinameL:
                case Abc::MN_MultinameLA:
                    // Namespace set is known at compile time, Name is resolved at runtime.        
                    --st_size;
                    break;
                case Abc::MN_Typename:
                    // Recursive!!!?
                    SF_ASSERT(false);
                    break;
                default:
                    SF_ASSERT(false);
                    break;
                }
            }

            if (oi.pop_args)
            {
                const int arg_count = ReadU30(pCode, bcp);

                if (opcode == Code::op_newobject)
                    st_size -= arg_count * 2;
                else
                    st_size -= arg_count;
            }

            if (st_size > 0)
                st_size += Code::GetOpCodeInfo(opcode).push;
            else
                break;

            if (oi.pop_args | oi.pop_mn)
                // We have already read all operands.
                continue;
        }

        // Read operands.
        switch (opcode)
        {
        case Code::op_ifnlt:
        case Code::op_ifnle:
        case Code::op_ifngt:
        case Code::op_ifnge:
        case Code::op_ifeq:
        case Code::op_ifge:
        case Code::op_ifgt:
        case Code::op_ifle:
        case Code::op_iflt:
        case Code::op_ifne:
        case Code::op_ifstricteq:
        case Code::op_ifstrictne:
        case Code::op_iftrue:
        case Code::op_iffalse:
        case Code::op_jump:
            ReadS24(pCode, bcp);
            break;
        case Code::op_lookupswitch:
            {
                ReadS24(pCode, bcp);
                int case_count = ReadU30(pCode, bcp);

                for(int i = 0; i <= case_count; ++i)
                    ReadS24(pCode, bcp);

            }
            break;
        case Code::op_pushbyte:
            Read8(pCode, bcp);
            break;
        case Code::op_debug:
            Read8(pCode, bcp);
            ReadU30(pCode, bcp);
            Read8(pCode, bcp);
            ReadU30(pCode, bcp);
            break;
        default:
            switch (Code::GetOpCodeInfo(opcode).operandCount)
            {
            case 2:
                ReadU30(pCode, bcp);
                // No break on purpose.
            case 1:
                ReadU30(pCode, bcp);
                break;
            default:
                break;
            }
        }
    }

    return opcode;
}

Abc::TCodeOffset Tracer::GetNextSwitchStmt(Abc::TCodeOffset& bcp) const
{
    using namespace Abc;

    TCodeOffset result = 0;
    Code::OpCode opcode = Code::op_nop;
    TCodeOffset ccp;

    while (bcp < CodeEnd)
    {
        ccp = bcp;
        opcode = static_cast<Code::OpCode>(Read8(pCode, bcp));

        //
        {
            const Code::OpCodeInfo& oi = Code::GetOpCodeInfo(opcode);

            if (oi.pop_mn)
                // Read multiname index.
                ReadU30(pCode, bcp);

            if (oi.pop_args)
                // Read number of popped arguments.
                ReadU30(pCode, bcp);

            if (oi.pop_args | oi.pop_mn)
                // We have already read all operands.
                continue;
        }

        // Read operands.
        switch (opcode)
        {
        case Code::op_ifnlt:
        case Code::op_ifnle:
        case Code::op_ifngt:
        case Code::op_ifnge:
        case Code::op_ifeq:
        case Code::op_ifge:
        case Code::op_ifgt:
        case Code::op_ifle:
        case Code::op_iflt:
        case Code::op_ifne:
        case Code::op_ifstricteq:
        case Code::op_ifstrictne:
        case Code::op_iftrue:
        case Code::op_iffalse:
        case Code::op_jump:
            ReadS24(pCode, bcp);
            break;
        case Code::op_lookupswitch:
            {
                result = ccp;

                ReadS24(pCode, bcp);
                int case_count = ReadU30(pCode, bcp);

                for(int i = 0; i <= case_count; ++i)
                    ReadS24(pCode, bcp);

            }
            goto end;
        case Code::op_pushbyte:
            Read8(pCode, bcp);
            break;
        case Code::op_debug:
            Read8(pCode, bcp);
            ReadU30(pCode, bcp);
            Read8(pCode, bcp);
            ReadU30(pCode, bcp);
            break;
        default:
            switch (Code::GetOpCodeInfo(opcode).operandCount)
            {
            case 2:
                ReadU30(pCode, bcp);
                // No break on purpose.
            case 1:
                ReadU30(pCode, bcp);
                break;
            default:
                break;
            }
        }
    }

    end:
    return result;
}

void Tracer::ThrowMergeTypeError(const Traits& to_tr, const Traits& from_tr) const
{
    SF_UNUSED2(to_tr, from_tr);

    VM& vm = GetVM();
    vm.ThrowVerifyError(VM::Error(VM::eCannotMergeTypesError, vm
        SF_DEBUG_ARG(to_tr.GetName())
        SF_DEBUG_ARG(from_tr.GetName())
        ));
}

void Tracer::JoinSNodesNoAssignment(UPInt rnum, Value& to, const Value& from, TR::MergeSet ms)
{
#ifdef SF_AS3_AOTC
    using namespace AOT;

    // ??? Shouldn't we preserve original null because there is no assignment?
    const Value::TraceNullType null = AS3::CanBeNull(to, from);

    if (to.IsSNode())
    {
        SNode& to_sn = to.GetSNode();

        if (from.IsSNode())
        {
            SNode& from_sn = from.GetSNode();

            if (&to_sn == &from_sn)
            {
                // Nodes are absolutely identical.
                to.SetTraceNullType(null);
                return;
            }

            if (to_sn.GetType() == from_sn.GetType())
            {
                // Nodes have the same type.
                if (to_sn.Is(SNode::tVar))
                {
                    SNodeVar& to_snv = static_cast<SNodeVar&>(to_sn);
                    SNodeVar& from_snv = static_cast<SNodeVar&>(from_sn);

                    if (to_snv == from_snv)
                    {
                        to.SetTraceNullType(null);
                        return;
                    }
                }
            }

            if (ms == TR::msRegisterFile)
            {
                // Merge registers.
                // Nodes are not identical at this point.
                if (to_sn.Is(SNode::tPhi))
                {
                    // We already have a Phi node.
                    SNodePhi& phi = static_cast<SNodePhi&>(to_sn);

                    phi.AddSNode(&from_sn);
                }
                else
                {
                    // Let's create a Phi-node.
                    SPtr<SNode> phi;
                    SPtr<SNode> rtv;
                    const Traits& to_tr = GetVM().GetValueTraits(to);
                    // Register.
                    // It is, probably, not correct to use register's type.
                    SPtr<SNodeRT> rt = GetSNodeRT(rnum, &to_tr SF_DEBUG_ARG(CurrOffset));
                    // Phi-node.
                    phi.Pick(SF_NEW SNodePhi(&to_sn, &from_sn SF_DEBUG_ARG(CurrOffset)));
                    // RTV-node.
                    rtv.Pick(SF_NEW SNodeRTV(rt, phi SF_DEBUG_ARG(CurrOffset)));
                    to.UpdateSNode(*rtv, null);
                }

                return;
            }

            // We should preserve type and create new SNode.
            SPtr<SNode> sn;
            sn.Pick(SF_NEW AOT::SNodeName("something1"));
            to.UpdateSNode(*sn, null);
        }
    }
    else
    {
        if (from.IsSNode())
        {
            // Try to preserve SNode.
            switch (to.GetKind())
            {
            case Value::kInstanceTraits:
                to = Value(to.GetInstanceTraits(), null, from.GetSNode());
                break;
            case Value::kClassTraits:
                to = Value(to.GetClassTraits(), null, from.GetSNode());
                break;
            default:
                break;
            }
        }
    }
#else
    // Just do nothing.
    SF_UNUSED4(to, from, ms, rnum);
#endif
}

void Tracer::JoinSNodesAssign(Value& to, const Value& from) const
{
#ifdef SF_AS3_AOTC
    using namespace AOT;

    const Value::TraceNullType null = AS3::CanBeNull(to, from);

    if (to.IsSNode())
    {
        SNode& to_sn = to.GetSNode();

        if (from.IsSNode())
        {
            SNode& from_sn = from.GetSNode();

            if (to_sn.Is(SNode::tVar) && from_sn.Is(SNode::tVar))
            {
                SNodeVar& to_snv = static_cast<SNodeVar&>(to_sn);
                SNodeVar& from_snv = static_cast<SNodeVar&>(from_sn);

                if (to_snv == from_snv)
                {
                    to.UpdateSNode(to_sn, null);
                    return;
                }
            }

            // We should create new SNode.
            SPtr<SNode> sn;
            sn.Pick(SF_NEW AOT::SNodeName("something3"));

            to = from;
            to.UpdateSNode(*sn, null);
        }
        else
        {
            // We can loose SNode during assignment.
            const Traits* tr = GetValueTraits(from);
            if (tr)
            {
                if (tr->IsInstanceTraits())
                {
                    const InstanceTraits::Traits& itr = *static_cast<const InstanceTraits::Traits*>(tr);
                    to = Value(const_cast<InstanceTraits::Traits&>(itr), null, to.GetSNode());
                }
                else
                {
                    const ClassTraits::Traits& ctr = *static_cast<const ClassTraits::Traits*>(tr);
                    to = Value(const_cast<ClassTraits::Traits&>(ctr), null, to.GetSNode());
                }
            }
            else
                // ??? null?
                // What else can we do about this?
                to = from;
        }
    }
    else
        // ??? null?
        // If "from" is SNode then we are getting an SNode.
        // If both "to" and "from" are not SNode then there is nothing else we can do about this.
        to = from;
#else
    to = from;
#endif
}

void Tracer::JoinSNodesUpdateType(Value& to, const Value& from, InstanceTraits::Traits& result_type) const
{
    // If a new type is always not null, then there is no reason to check "to" and "from".
    // If the new type can be null, then result depends on "to" and "from"
    Value::TraceNullType null = CanBeNull(result_type);
    if (null != Value::NotNull)
        null = AS3::CanBeNull(to, from);

#ifdef SF_AS3_AOTC
    using namespace AOT;

    if (to.IsSNode())
    {
        SNode& to_sn = to.GetSNode();

        if (from.IsSNode())
        {
            SNode& from_sn = from.GetSNode();

            if (to_sn.Is(SNode::tVar) && from_sn.Is(SNode::tVar))
            {
                SNodeVar& to_snv = static_cast<SNodeVar&>(to_sn);
                SNodeVar& from_snv = static_cast<SNodeVar&>(from_sn);

                if (to_snv == from_snv)
                {
                    to = Value(result_type, null, to_sn);
                    return;
                }
            }

            SPtr<SNode> sn;
            sn.Pick(SF_NEW AOT::SNodeName("something2"));

            to = Value(result_type, null, *sn);
        }
        else
            to.UpdateTraits(result_type);
    }
    else
    {
        if (from.IsSNode())
            to = Value(result_type, null, from.GetSNode());
        else
            // We do not have any SNode.
            to = Value(result_type, null);
    }
#else
    SF_UNUSED1(from);
    to = Value(result_type, null);
#endif
}

void Tracer::JoinSNodesUpdateType(Value& to, const Value& from, ClassTraits::Traits& result_type) const
{
    // If a new type is always not null, then there is no reason to check "to" and "from".
    // If the new type can be null, then result depends on "to" and "from"
    Value::TraceNullType null = CanBeNull(result_type.GetInstanceTraits());
    if (null != Value::NotNull)
        null = AS3::CanBeNull(to, from);

#ifdef SF_AS3_AOTC
    using namespace AOT;

    if (to.IsSNode())
    {
        SNode& to_sn = to.GetSNode();

        if (from.IsSNode())
        {
            SNode& from_sn = from.GetSNode();

            if (to_sn.Is(SNode::tVar) && from_sn.Is(SNode::tVar))
            {
                SNodeVar& to_snv = static_cast<SNodeVar&>(to_sn);
                SNodeVar& from_snv = static_cast<SNodeVar&>(from_sn);

                if (to_snv == from_snv)
                {
                    to = Value(result_type, null, to_sn);
                    return;
                }
            }

            SPtr<SNode> sn;
            sn.Pick(SF_NEW AOT::SNodeName("something4"));

            to = Value(result_type, null, *sn);
        }
        else
            to.UpdateTraits(result_type);
    }
    else
    {
        if (from.IsSNode())
            to = Value(result_type, null, from.GetSNode());
        else
            // We do not have any SNode.
            to = Value(result_type, null);
    }
#else
    SF_UNUSED1(from);
    to = Value(result_type, null);
#endif
}

CheckResult Tracer::MergeValues(UPInt ind, TR::State& to, const TR::State& from, TR::MergeSet ms)
{
    ValueArrayDH& to_va = to.GetValueArray(ms);
    const ValueArrayDH& from_va = from.GetValueArray(ms);

    Value& to_v = to_va[ind];
    const Value& from_v = from_va[ind];

    const Traits* to_tr = GetValueTraits(to_v);
    const Traits* from_tr = GetValueTraits(from_v);

    bool result = false;

    if (ms == TR::msScopeStack)
	{
		if (to_v.GetWith() != from_v.GetWith())
            goto error;
	}

    if (ms == TR::msRegisterFile)
    {
        const unsigned i = static_cast<unsigned>(ind);
        if (!from.RegistersAlive.IsSet(i))
            // "from" register is not set.
            // It doesn't contain any meaningful value.
            goto success;

        if (!to.RegistersAlive.IsSet(i))
        {
            // We do not need to merge types if "to" is not alive.
            to_v = from_v;
            goto success;
        }
    }

    // Same types.
    if (to_tr == from_tr)
    {
        JoinSNodesNoAssignment(ind, to_v, from_v, ms);
        goto success;
    }

    // Undefined.
	if (to_v.IsUndefined())
	{
		to_v = from_v;
        goto success;
	}

    // Any type
    if (IsAnyType(to_tr))
    {
        JoinSNodesNoAssignment(ind, to_v, from_v, ms);
        goto success;
    }

    if (IsAnyType(from_tr))
    {
        // Make "to" of "any" type.
        JoinSNodesUpdateType(to_v, from_v, GetObjectType());
        goto success;
    }

    // Numeric types.
#ifdef SF_AS3_TR_UINT_AS_SINT
    if (IsNumericType(to_tr) && IsNumericType(from_tr))
    {
        // If one of the types is Number then result is Number.
        if (IsNumberType(to_tr) || IsNumberType(from_tr))
            JoinSNodesUpdateType(to_v, from_v, GetNumberType());

        goto success;
    }
#else
    if (IsNumericType(to_tr) && IsNumericType(from_tr))
    {
        // Promote type to Number because types are not equal and SInt is
        // not the same as UInt.
        JoinSNodesUpdateType(to_v, from_v, GetNumberType());
        goto success;
    }
#endif

    // Null.
    if (to_v.IsNull() || IsNullType(to_tr))
    {
        if (from_v.IsNull() || IsNullType(from_tr))
        {
            JoinSNodesNoAssignment(ind, to_v, from_v, ms);
            goto success;
        }
        else if (IsStringType(from_tr) || !IsNumericType(from_tr))
        {
            // We can store Null in Strings and non-numeric types.
            JoinSNodesAssign(to_v, from_v);
            goto success;
        }
        else
            goto error;
    }

    if (from_v.IsNull() || IsNullType(from_tr))
    {
        if (IsStringType(to_tr) || !IsNumericType(to_tr))
        {
            // We can store Null in Strings and non-numeric types.
            // "to" already has a right type.
            JoinSNodesNoAssignment(ind, to_v, from_v, ms);
            goto success;
        }
        else
            goto error;
    }

    // Try to find a common base.
    {
        // Set tmp bit.
        const Traits* tr = to_tr;
        while (tr)
        {
            tr->SetTmpBit();
            tr = tr->GetParent();
        }

        // Check tmp bit.
        tr = from_tr;
        while (tr)
        {
            if (tr->IsTmpBit())
            {
                // We found a common base.
                break;
            }

            tr = tr->GetParent();
        }

        if (tr)
        {
            result = true;

            // "tr" can be either ClassTraits or InstanceTraits.
            Traits* tr1 = const_cast<Traits*>(tr);
            if (tr->IsClassTraits())
                JoinSNodesUpdateType(to_v, from_v, *static_cast<ClassTraits::Traits*>(tr1));
            else
                JoinSNodesUpdateType(to_v, from_v, *static_cast<InstanceTraits::Traits*>(tr1));
        }

        // Clear tmp bit. It should always be false.
        {
            tr = to_tr;
            while (tr)
            {
                tr->SetTmpBit(false);
                tr = tr->GetParent();
            }
        }
    }

    // For debugging.
    if (result)
        goto success;
    else
        goto error;

    success:
        if (ms == TR::msRegisterFile)
        {
            // Mark register as alive if at least one of them is alive.
            const unsigned i = static_cast<unsigned>(ind);
            const bool isAlive = from.RegistersAlive.IsSet(i) || to.RegistersAlive.IsSet(i);
            to.RegistersAlive.Set(i, isAlive);
        }
        return true;

    error:
        ThrowMergeTypeError(*to_tr, *from_tr);
        return false;
}

CheckResult Tracer::MergeLists(TR::State& to, const TR::State& from, bool check_size, TR::MergeSet ms)
{
    // Skip merging if lists, which are of different size.
    // Is this a correct approach?
    // There are three tests, which will break if we remove this check.
    // 1) as3/Definitions\Classes\ClassDef\Bug118272Package.abc
    // 2) as3/Statements\Exceptions\TryCatchBlockUserWithBuiltInExceptions.abc
    // 3) ecma3/Statements\etry_006.abc

    ValueArrayDH& to_va = to.GetValueArray(ms);
    const ValueArrayDH& from_va = from.GetValueArray(ms);

    if (check_size && to_va.GetSize() != from_va.GetSize())
        return false;

    for (UPInt i = 0; i < from_va.GetSize(); ++i)
    {
        if (i >= to_va.GetSize())
            to_va.PushBack(from_va[i]);
        else if (!MergeValues(i, to, from, ms))
            return false;
    }

    return true;
}

void Tracer::InitializeBlock(TR::Block& to, const TR::Block& from)
{
    SF_ASSERT(!to.IsInitializedBlock());

    const TR::State& from_st = GetState(from);
    TR::State& to_st = GetState(to);

    to.CurOpStackSize = from.CurOpStackSize;
    MergeLists(to_st, from_st, false, TR::msOpStack).DoNotCheck();
    MergeLists(to_st, from_st, false, TR::msScopeStack).DoNotCheck();
    MergeLists(to_st, from_st, true,  TR::msRegisterFile).DoNotCheck();

    to.SetInitialized();
}

CheckResult Tracer::MergeBlock(TR::Block& to, const TR::Block& from)
{
    SF_ASSERT(to.IsInitializedBlock());

    const TR::State& from_st = GetState(from);
    TR::State& to_st = GetState(to);

    if (!to.IsCatchBlock() && !to.IsDeadBlock())
    {
        // SF_ASSERT(from.IsCatchBlock() || to.CurOpStackSize == from.CurOpStackSize);
        to.CurOpStackSize = Alg::Max(to.CurOpStackSize, from.CurOpStackSize);

        if (!MergeLists(to_st, from_st, true, TR::msOpStack))
        {
#if 0
            // Too much trouble so far.
            VM& vm = GetVM();
            vm.ThrowVerifyError(VM::Error(VM::eStackDepthUnbalancedError, vm
                SF_DEBUG_ARG(to_st.GetOpStack().GetSize())
                SF_DEBUG_ARG(from_st.GetOpStack().GetSize())
                ));

            return false;
#endif
        }

        // It looks like the only place where not matched sizes of scope stack 
        // are allowed is Code::op_label.
        if (!MergeLists(to_st, from_st, false, TR::msScopeStack))
        {
            VM& vm = GetVM();
            vm.ThrowVerifyError(VM::Error(VM::eScopeDepthUnbalancedError, vm
                SF_DEBUG_ARG((int)to_st.GetScopeStack().GetSize())
                SF_DEBUG_ARG((int)from_st.GetScopeStack().GetSize())
                ));

            return false;
        }
    }

    return MergeLists(to_st, from_st, true, TR::msRegisterFile);
}

int Tracer::GetNextU30(UPInt opcode_offset) const
{
    using namespace Abc;

    TCodeOffset ccp = OrigOpcodePos[OrigOpcodePos.GetSize() - opcode_offset - 1];

    return ReadU30(pCode, ++ccp);
}

bool Tracer::SubstituteOpCode(const Abc::Code::OpCode opcode, Abc::TCodeOffset& bcp, TR::State& st)
{
    using namespace Abc;

#ifdef GFX_AS3_TRACE
    FlashUI& ui = GetUI();;
#endif

    // Try to substitute with a strictly typed opcode ...

    // Opcodes op_coerce_a, op_label, and op_kill are always skipped and 
    // will not be presented in a running VM.
    // op_label is handled in TraceBlock() because it creates new block.

    switch(opcode)
    {
    case Code::op_kill:
        {
            const int index = ReadU30(pCode, bcp);

#if defined(SF_AS3_AOTC2)
            st.PushSNodeOC(AOT::SNodeOC::op_kill, index);
#endif

            st.SetRegister(AbsoluteIndex(index), Value::GetUndefined());
            // Kill the register.
            st.RegistersAlive.Clear(index);

            // Skip this opcode.
            SkipOrigOpCode(bcp, bcp);
        }
        return true;
    case Code::op_coerce_a:
        {
            Value& v = st.BackOpValue();

#if 1
            // Let's be not very conservative and convert integer into Number.
            if (IsSIntType(v) || IsUIntType(v))
            {
                PushNewOpCode(Code::op_convert_d);

#if defined(SF_AS3_AOTC2)
                st.PushSNodeOC(AOT::SNodeOC::op_convert_d);
#endif

                // Try to preserve real value on stack.
                if (v.IsInt())
                    // We do not expect any errors here.
                    v.ToNumberValue().DoNotCheck();
                else
                    st.ConvertOpTo(GetNumberType(), Value::NotNull);
            }
            else
                // Just skip this opcode.
                SkipOrigOpCode(bcp, bcp);
#else
            // This simple logic doesn't work at this time.
            // Just skip this opcode.
            SkipOrigOpCode(bcp, bcp);
#endif
        }
        return true;
    case Code::op_coerce_s:
        {
            const Value& v = st.BackOpValue();

            if (v.IsString())
                return true;

            // Can be null.
            if (v.IsNull() || IsStringType(v))
            {
                const Value::TraceNullType null = v.IsNull() ? Value::Null : Value::NullOrNot;
                st.ConvertOpTo(GetStringType(), null);

                // Skip this opcode.
                SkipOrigOpCode(bcp, bcp);
                return true;
            }

#if defined(SF_AS3_AOTC2)
            st.PushSNodeOC(AOT::SNodeOC::op_coerce_s);
#endif
        }
        break;
    case Code::op_coerce:
        {
            TCodeOffset ccp = bcp;
            const int mn_index = ReadU30(pCode, ccp);
            const Abc::Multiname& mn = GetFile().GetConstPool().GetMultiname(mn_index);
            // ATTN: We do not remove value from the stack.
            const Value& value = st.BackOpValue();

            const ClassTraits::Traits* ctr = GetVM().Resolve2ClassTraits(GetFile(), mn);
            if (ctr == NULL)
            {
                GetVM().ThrowVerifyError(VM::Error(VM::eClassNotFoundError, GetVM()
                    SF_DEBUG_ARG(mn.GetName(GetFile().GetConstPool()).ToCStr())
                    ));

                return true;
            }

            InstanceTraits::Traits& itr = ctr->GetInstanceTraits();

            // Check for "undefined" plus ObjectType
            if (value.IsUndefined() && &itr != &GetClassType() && !IsNotObjectType(&itr))
            {
                // If previous opcode was op_pushundefined we can replace it with op_pushnull.
                if (GetNewTopOpCode() == Code::op_pushundefined)
                {
                    PushNewOpCode(Code::op_pushnull);

#if defined(SF_AS3_AOTC2)
                    st.PushSNodeOC(AOT::SNodeOC::op_pushnull);
#endif

                    // Do not push "null". Preserve type.
                    st.ConvertOpTo(itr, Value::Null);

                    // Skip prev combination ...
                    SkipOrigOpCode(bcp, ccp);

                    return true;
                }
            }

            // Null value can be of any Object type.
            if (value.IsNull() || ValueIsOfType(value, *ctr))
            {
                // We can eliminate this "coerce".

                // TBD: It, probably, should be st.ConvertOpTo(*GetValueTraits(value));
                // It will be a more precise type.
                // Value wasn't popped from stack.
                Value::TraceNullType null = CanBeNull(itr);
                if (value.IsNull())
                    null = Value::Null;

                st.ConvertOpTo(itr, null);

                // Skip prev combination ...
                SkipOrigOpCode(bcp, ccp);

                return true;
            }

            // Skip prev combination ...
            SkipOrigOpCode(bcp, ccp);

            PushNewOpCode(Code::op_coerce, mn_index);

#if defined(SF_AS3_AOTC2)
            st.PushSNodeOC(AOT::SNodeOC::op_coerce, mn_index);
#endif

            st.ConvertOpTo(itr, Value::NullOrNot);
        }
        return true;
    case Code::op_getglobalscope:
        {
            TCodeOffset ccp = bcp;
#ifdef GFX_AS3_TRACE
            TCodeOffset next_offset = ccp;
#endif
            const Code::OpCode next_opcode = static_cast<Code::OpCode>(Read8(pCode, ccp));
            int drop_num = 0;

            if (next_opcode == Code::op_getslot)
            {
                // Simulate prev opcode.
                st.exec_getglobalscope(true);

#ifdef GFX_AS3_TRACE
                if (ui.NeedToCheckOpCode()) { ui.OnOpCode(next_offset); }
#endif

                const UInt32 slot_index = ReadU30(pCode, ccp);
                PushNewOpCode(Code::op_getglobalslot, slot_index);

#if defined(SF_AS3_AOTC2)
                st.PushSNodeOC(AOT::SNodeOC::op_getglobalslot, slot_index);
#endif
                // Simulate prev opcode.
                st.exec_getslot(slot_index, true);

                // Skip prev combination ...
                SkipOrigOpCode(bcp, ccp);

                return true;
            } else if (next_opcode == Code::op_swap)
            {
                OrigOpcodePos.PushBack(ccp);
                ++drop_num;

#ifdef GFX_AS3_TRACE
                TCodeOffset next_offset2 = ccp;
#endif
                const Code::OpCode next_opcode2 = static_cast<Code::OpCode>(Read8(pCode, ccp));

                if (next_opcode2 == Code::op_setslot)
                {
                    const UInt32 slot_index = ReadU30(pCode, ccp);

                    PushNewOpCode(Code::op_setglobalslot, slot_index);

#if defined(SF_AS3_AOTC2)
                    st.PushSNodeOC(AOT::SNodeOC::op_setglobalslot, slot_index);
#endif

                    // Simulate a set of opcodes below.
                    st.exec_getglobalscope(true);
#ifdef GFX_AS3_TRACE
                    if (ui.NeedToCheckOpCode()) { ui.OnOpCode(next_offset); }
#endif
                    st.exec_swap(true);
#ifdef GFX_AS3_TRACE
                    if (ui.NeedToCheckOpCode()) { ui.OnOpCode(next_offset2); }
#endif
                    st.exec_setslot(slot_index, true);


                    // Instead of simulating a simple one below.
                    //exec_setglobalslot(ReadU30(pCode, ccp));

                    // Skip prev combination ...
                    SkipOrigOpCode(bcp, ccp);

                    return true;
                } else
                {
                    OrigOpcodePos.PopBack(drop_num);
                }
            }

#if defined(SF_AS3_AOTC2)
            st.PushSNodeOC(AOT::SNodeOC::op_getglobalscope);
#endif
        }
        break;
    case Code::op_getlocal:
        {
            TCodeOffset ccp = bcp;
            const int reg_num = ReadU30(pCode, ccp);

            return SubstituteGetlocal(bcp, ccp, st, reg_num);
        }
        break;
    case Code::op_getlocal0:
        return SubstituteGetlocal(bcp, bcp, st, 0);
    case Code::op_getlocal1:
        return SubstituteGetlocal(bcp, bcp, st, 1);
    case Code::op_getlocal2:
        return SubstituteGetlocal(bcp, bcp, st, 2);
    case Code::op_getlocal3:
        return SubstituteGetlocal(bcp, bcp, st, 3);
    case Code::op_initproperty:
        // It looks like it is not possible to optimize out initproperty for classes because
        // compiler can generate *getslot* for this property in the same function and we do
        // not have absolute object for this class at optimization time.
    case Code::op_setproperty:
    case Code::op_setsuper:
        {
            using namespace Abc;

            TCodeOffset ccp = bcp;
            const int mn_index = ReadU30(pCode, ccp);

            TR::ReadValueMnObject args(GetFile(), st, mn_index);

            // EmitSetProperty() always returns true.
            if (EmitSetProperty(opcode, args, mn_index))
            {
                // Skip prev combination ...
                SkipOrigOpCode(bcp, ccp);

#if 0
                // Try to retrieve function name.
                if (value.IsSNode())
                {
                    SNode& n = value.GetSNode();

                    if (n.Is(SNode::tArgs))
                    {
                        SNodeArgs& na = static_cast<SNodeArgs&>(n);

                        if (na.Op == SNodeArgs::opNewFunction)
                        {
                            SF_ASSERT(na.FITR);
                            const ASString name = vtr->GetSlotName(aind);
                            GetTracer().GetInfoCollector().FunctNameHash.Add(na.FITR->GetMethodInfoInd(), String(name.ToCStr(), name.GetSize()));
                        }
                    }
                }
#endif

                return true;
            }
        }
        break;
    case Code::op_getproperty:
    case Code::op_getsuper:
        {
            using namespace Abc;

            TCodeOffset ccp = bcp;
            const int mn_index = ReadU30(pCode, ccp);

            TR::ReadMnObject args(GetFile(), st, mn_index);

            // EmitGetProperty() always returns true.
            if (EmitGetProperty(opcode, st, args, mn_index))
            {
                // Skip prev combination ...
                SkipOrigOpCode(bcp, ccp);

                return true;
            }
        }
        break;
    case Code::op_findpropstrict:
        {
            // Try to eliminate (findpropstrict + getproperty) or (findpropstrict + constructprop)
            TCodeOffset ccp0 = bcp;
            const int mn_index = ReadU30(pCode, ccp0);
            TCodeOffset ccp = ccp0;
            const Code::OpCode next_opcode = static_cast<Code::OpCode>(Read8(pCode, ccp));

            const Code::OpCode consumer = GetOrigValueConsumer(ccp0);

            if (next_opcode == Code::op_getproperty)
            {
                const int get_mn_index = ReadU30(pCode, ccp);
                
#if 1
                SF_UNUSED2(mn_index, get_mn_index);
#else
                // Compiler can generate different namespace sets for op_findpropstrict and
                // op_getproperty in case of Vector.<>.
                if (mn_index == get_mn_index)
#endif
                {
                    // This is the same as getlex.
                    if (EmitFindProperty(st, mn_index, true, consumer SF_DEBUG_ARG(true)))
                    {
                        // Skip prev combination ...
                        SkipOrigOpCode(bcp, ccp);

                        // This combination should add one element to the stack.
                        //SF_ASSERT(st.GetOpStack().GetSize() == origOpStackSize + 1);

                        return true;
                    }
                }
            }
            else
            {
                //  && !IsOrigCall(consumer)
                //const bool get_prop = !IsOrigPropSetter(consumer) && !IsOrigConstruct(consumer) || IsOrigCall(consumer);
                // 
                if (EmitFindProperty(st, mn_index, false, consumer SF_DEBUG_ARG(true)))
                {
                    // Skip prev combination ...
                    SkipOrigOpCode(bcp, ccp0);

                    // This combination should add one element to the stack.
                    //SF_ASSERT(st.GetOpStack().GetSize() == origOpStackSize + 1);

                    return true;
                }
            }
        }
        break;
    case Code::op_findproperty:
        {
            TCodeOffset ccp = bcp;
            const int mn_index = ReadU30(pCode, ccp);

            if (EmitFindProperty(st, mn_index))
            {
                // Skip prev combination.
                SkipOrigOpCode(bcp, ccp);
                return true;
            }
        }
        break;
    case Code::op_getlex:
        {
            TCodeOffset ccp = bcp;
            const int mn_index = ReadU30(pCode, ccp);
#ifdef SF_BUILD_DEBUG
            const UPInt origOpStackSize = st.GetOpStack().GetSize();
#endif

#if 0 // For debugging.
            const Abc::Multiname& mn = GetAbcFile().GetConstPool().GetMultiname(mn_index);
            AS3::Multiname as3_mn(GetFile(), mn);
            if (as3_mn.GetName().AsString() == "InputDetails")
            {
                int i = 0;
                ++i;
            }
#endif

            if (EmitFindProperty(st, mn_index, true))
            {
                // Skip prev combination ...
                SkipOrigOpCode(bcp, ccp);

                // op_getlex should add one element to the stack.
#ifdef SF_BUILD_DEBUG
                SF_ASSERT(st.GetOpStack().GetSize() == origOpStackSize + 1);
#endif

                return true;
            }
        }
        break;
    case Code::op_callproplex:
    case Code::op_callpropvoid:
    case Code::op_callproperty:
    case Code::op_callsuper:
    case Code::op_callsupervoid:
        {
            // Call a property.

            TCodeOffset ccp = bcp;

            UInt32 mn_index = ReadU30(pCode, ccp);
            UInt32 arg_count = ReadU30(pCode, ccp);

            // Read arguments ...
            TR::ReadArgsMnObject args(GetFile(), st, arg_count, mn_index);

            /*
            if (IsException())
                return false;
            */

            // EmitCall() always returns true.
            if (EmitCall(opcode, st, args, mn_index))
            {
                // Skip prev combination ...
                SkipOrigOpCode(bcp, ccp);
                return true;
            }
        }
        break;
    case Code::op_constructprop:
        {
            // Result is never null.

            TCodeOffset ccp = bcp;
            UInt32 mn_index = ReadU30(pCode, ccp);
            UInt32 arg_count = ReadU30(pCode, ccp);

            // Read arguments ...
            TR::ReadArgsMnObject args(GetFile(), st, arg_count, mn_index);

#ifdef SF_AS3_AOTC
            using namespace AOT;

            SPtr<SNode> sn;
            SPtr<SNode> sn_name = st.AotRetrieveClassName(args.ArgObject, args.ArgMN);
            sn.Pick(SF_NEW SNodeN(st.GetThis(), SNodeN::opConstructProp, sn_name, arg_count, args.GetCallArgs() SF_DEBUG_ARG(bcp - 1)));
#endif

            if (!args.ArgMN.IsRunTime())
            {
                if (args.ArgObject.IsArtificialClass())
                {
                    switch (args.ArgObject.GetKind())
                    {
#ifdef SF_AS3_AOTC
                    case Value::kSNodeCT:
#endif
                    case Value::kClassTraits:
                        // Emit *construct*.
                        PushNewOpCode(Code::op_construct, arg_count);
#if defined(SF_AS3_AOTC2)
                        st.PushSNodeOC(AOT::SNodeOC::op_construct, arg_count);
#endif
                        st.PushOp(Value(args.ArgObject.GetClassTraits().GetInstanceTraits(), Value::NotNull SF_AOTC_ARG(*sn)));
                        // Skip prev combination ...
                        SkipOrigOpCode(bcp, ccp);
                        return true;
                    case Value::kClass:
                        // Emit *construct*.
                        PushNewOpCode(Code::op_construct, arg_count);
#if defined(SF_AS3_AOTC2)
                        st.PushSNodeOC(AOT::SNodeOC::op_construct, arg_count);
#endif
                        st.PushOp(Value(args.ArgObject.AsClass().GetClassTraits().GetInstanceTraits(), Value::NotNull SF_AOTC_ARG(*sn)));
                        // Skip prev combination ...
                        SkipOrigOpCode(bcp, ccp);
                        return true;
                    default:
                        break;
                    }
                }
                else
                {
                    if (arg_count == 0)
                    {
                        VM& vm = GetVM();
                        const Traits* tr = GetValueTraits(args.ArgObject);
                        UPInt slot_ind;

                        SF_ASSERT(tr);
                        const SlotInfo* si = FindFixedSlot(vm, *tr, args.ArgMN, slot_ind, NULL);
                        if (si)
                        {
                            EmitGetAbsSlot(st, slot_ind);
                            // Emit *construct*.
                            PushNewOpCode(Code::op_construct, arg_count);
#if defined(SF_AS3_AOTC2)
                            st.PushSNodeOC(AOT::SNodeOC::op_construct, arg_count);
#endif

                            // Type of result
                            {
                                Value type;

                                if (!st.GetPropertyType(*tr, *si, type SF_AOTC_ARG(*sn)))
                                    // Exception.
                                    return false;

                                st.PushOp(type);
                            }

                            // Skip prev combination ...
                            SkipOrigOpCode(bcp, ccp);
                            return true;
                        }
                    }
                }
            }

            // Restore back previous opcode.
            PushNewOpCode(Code::op_constructprop, mn_index, arg_count);
#if defined(SF_AS3_AOTC2)
            st.PushSNodeOC(AOT::SNodeOC::op_constructprop, mn_index, arg_count);
#endif
            st.PushOp(Value(GetObjectType(), Value::NotNull SF_AOTC_ARG(*sn)));

            // Skip prev combination ...
            SkipOrigOpCode(bcp, ccp);

            return true;
        }
        break;
    case Code::op_astypelate:
        {
            // Top of the stack is a class. Below it is a value.
            const Value class_ = st.PopOpValue(); // class
            InstanceTraits::Traits* itr = NULL;
            const Value::KindType kind = class_.GetKind();

            switch (kind)
            {
#ifdef SF_AS3_AOTC
            case Value::kSNodeIT:
#endif
            case Value::kInstanceTraits:
                itr = &class_.GetInstanceTraits();
                break;
#ifdef SF_AS3_AOTC
            case Value::kSNodeCT:
#endif
            case Value::kClassTraits:
                {
                    const ClassTraits::Traits& ctr = class_.GetClassTraits();
                    itr = &ctr.GetInstanceTraits();
                }
                break;
            case Value::kClass:
                {
                    const Class& cl = class_.AsClass();
                    const ClassTraits::Traits& ctr = cl.GetClassTraits();
                    itr = &ctr.GetInstanceTraits();
                }
                break;
            default:
                // Do nothing.
                break;
            }

            if (itr)
            {
                const Value& value = st.BackOpValue(); // value

                // We, probably, should also check for undefined here.
                if (value.IsNull() || ValueIsOfType(value, *itr))
                {
                    // We can get rid of this "astypelate".

                    // Get rid of an argument.
                    EmitPopPrevResult(st);

                    SkipOrigOpCode(bcp, bcp);

                    // It can be null, so, we have to convert.
                    st.ConvertOpTo(*itr, Value::NullOrNot);

                    // return statement is necessary here because we do not want to restore op_astypelate back.
                    return true;
                }
                else if (IsNotRefCountedType(itr))
                    // Primitive types cannot be null (well, except of String).
                    // But astypelate can result into null. So, we have to use
                    // a generic type.
                    itr = &GetObjectType();


#ifdef SF_AS3_AOTC
                SPtr<SNode> sn;
                sn.Pick(SF_NEW AOT::SNode2(value, AOT::SNode2::opAsTypeLate, class_ SF_DEBUG_ARG(st.OpcodeCP)));
#endif

                st.ConvertOpTo(*itr, Value::NullOrNot SF_AOTC_ARG(*sn));
            }

            // Restore opcode.
            PushNewOpCode(Code::op_astypelate);
#if defined(SF_AS3_AOTC2)
            st.PushSNodeOC(AOT::SNodeOC::op_astypelate);
#endif

            return true;
        }
        break;
    case Code::op_convert_i:
        {
            const Value& value = st.BackOpValue(); // value

            if (IsSIntType(value))
            {
                SkipOrigOpCode(bcp, bcp);

                return true;
            }
        }
        break;
    case Code::op_convert_u:
        {
            const Value& value = st.BackOpValue(); // value

            if (IsUIntType(value))
            {
                SkipOrigOpCode(bcp, bcp);

                return true;
            }
        }
        break;
    case Code::op_convert_d:
        {
            const Value& value = st.BackOpValue(); // value

            if (IsNumberType(value))
            {
                SkipOrigOpCode(bcp, bcp);

                return true;
            }
        }
        break;
    case Code::op_convert_b:
        {
            const Value& value = st.BackOpValue(); // value

            if (IsBooleanType(value))
            {
                SkipOrigOpCode(bcp, bcp);

                return true;
            }
        }
        break;
    case Code::op_convert_s:
        {
            const Value& value = st.BackOpValue(); // value

            if (value.IsNull() || IsStringType(value))
            {
                st.ConvertOpTo(GetStringType(), Value::NullOrNot);
                SkipOrigOpCode(bcp, bcp);

                return true;
            }
        }
        break;
    case Code::op_add:
        {
            const Value _2 = st.PopOpValue(); // value2
            const Value _1 = st.PopOpValue(); // value1

            const Traits* _1tr = GetValueTraits(_1);
            const Traits* _2tr = GetValueTraits(_2);

#ifdef SF_AS3_AOTC
            SPtr<SNode> sn;
#endif

            // Strings.
            if (_1tr == &GetStringType() || _2tr == &GetStringType())
            {
#ifdef SF_AS3_AOTC
                if (_1.IsString() && _2.IsString())
                    sn.Pick(SF_NEW AOT::SNodeValue(_1.AsString() + _2.AsString()));
                else
                    sn.Pick(SF_NEW AOT::SNode2(_1, AOT::SNode2::opAdd, _2 SF_DEBUG_ARG(bcp - 1)));
#endif

                PushNewOpCode(Code::op_add);
#if defined(SF_AS3_AOTC2)
                st.PushSNodeOC(AOT::SNodeOC::op_add);
#endif
                st.PushOp(Value(GetStringType(), Value::NullOrNot SF_AOTC_ARG(*sn)));
            }
            else if (IsNumberType(_1) && IsNumberType(_2))
            {
#ifdef SF_AS3_AOTC
                if (_1.IsNumber() && _2.IsNumber())
                    sn.Pick(SF_NEW AOT::SNodeValue(Value(_1.AsNumber() + _2.AsNumber())));
                else
                    sn.Pick(SF_NEW AOT::SNode2(_1, AOT::SNode2::opAdd, _2 SF_DEBUG_ARG(bcp - 1)));
#endif

                // Values are already of Number type.
                PushNewOpCode(Code::op_add_td);
#if defined(SF_AS3_AOTC2)
                st.PushSNodeOC(AOT::SNodeOC::op_add_td);
#endif
                st.PushOp(Value(GetNumberType(), Value::NotNull SF_AOTC_ARG(*sn)));
            }
            else if (GetNextOrigOpCode(bcp) == Code::op_convert_d)
            {
#ifdef SF_AS3_AOTC
                sn.Pick(SF_NEW AOT::SNode2(_1, AOT::SNode2::opAdd, _2 SF_DEBUG_ARG(bcp - 1)));
#endif

                // Result is Number any way.
                PushNewOpCode(Code::op_add_d);
#if defined(SF_AS3_AOTC2)
                st.PushSNodeOC(AOT::SNodeOC::op_add_d);
#endif

                // Skip next opcode.
                // Size op_convert_d/op_coerce_d is one.
                SkipOrigOpCode(bcp, bcp + 1);

                st.PushOp(Value(GetNumberType(), Value::NotNull SF_AOTC_ARG(*sn)));
            }
#if 0
            else if (GetNextOrigOpCode(bcp) == Code::op_convert_i && IsSIntType(_1) && IsSIntType(_2))
            {
                // This is addition of integers.
                PushNewOpCode(Code::op_add_ti);

                // Skip next opcode.
                // Size of Code::op_convert_i is one.
                SkipOrigOpCode(bcp, bcp + 1);

                st.PushOp(GetSIntType());
            }
#else
            // Theoretically, this can lead to problems because "add" is supposed to return Number.
            else if (IsSIntType(_1) && IsSIntType(_2))
            {
#ifdef SF_AS3_AOTC
                if (_1.IsInt() && _2.IsInt())
                    sn.Pick(SF_NEW AOT::SNodeValue(Value(_1.AsInt() + _2.AsInt())));
                else
                    sn.Pick(SF_NEW AOT::SNode2(_1, AOT::SNode2::opAdd, _2 SF_DEBUG_ARG(bcp - 1)));
#endif

                // This is addition of integers.
                PushNewOpCode(Code::op_add_ti);
#if defined(SF_AS3_AOTC2)
                st.PushSNodeOC(AOT::SNodeOC::op_add_ti);
#endif

                st.PushOp(Value(GetSIntType(), Value::NotNull SF_AOTC_ARG(*sn)));
            }
#endif
            else
            {
#ifdef SF_AS3_AOTC
                sn.Pick(SF_NEW AOT::SNode2(_1, AOT::SNode2::opAdd, _2 SF_DEBUG_ARG(bcp - 1)));
#endif

                PushNewOpCode(Code::op_add);
#if defined(SF_AS3_AOTC2)
                st.PushSNodeOC(AOT::SNodeOC::op_add);
#endif

                // Operands are primitives but strings.
                if (IsPrimitiveType(_1tr) && IsPrimitiveType(_2tr))
                    st.PushOp(Value(GetNumberType(), Value::NotNull SF_AOTC_ARG(*sn)));
                else
                    // Result should be either String or Number, so, let us make it of Object type.
                    st.PushOp(Value(GetObjectType(), Value::NullOrNot SF_AOTC_ARG(*sn)));
            }

            return true;
        }
        break;
    default:
        break;
    }

    return false;
}

bool Tracer::SubstituteGetlocal(Abc::TCodeOffset& bcp, Abc::TCodeOffset ccp, TR::State& st, int src_reg_num)
{
    using namespace Abc;

#ifdef SF_AS3_AOTC
    using namespace AOT;

    Value& reg = st.GetRegister(AbsoluteIndex(src_reg_num));
#else
    const Value& reg = st.GetRegister(AbsoluteIndex(src_reg_num));
#endif

#ifdef GFX_AS3_TRACE
    TCodeOffset next_offset = ccp;
#endif
    const Code::OpCode next_opcode = static_cast<Code::OpCode>(Read8(pCode, ccp));
    bool check_setlocal = false;
    int drop_num = 0;

    switch(next_opcode)
    {
    case Code::op_increment:
    case Code::op_decrement:
    case Code::op_increment_i:
    case Code::op_decrement_i:
        check_setlocal = true;
        break;
    default:
        break;
    }

    if (check_setlocal)
    {
#ifdef GFX_AS3_TRACE
        FlashUI& ui = GetUI();;
        if (ui.NeedToCheckOpCode()) { ui.OnOpCode(next_offset); }
#endif

        OrigOpcodePos.PushBack(ccp);
        ++drop_num;

#ifdef GFX_AS3_TRACE
        TCodeOffset current_offset = ccp;
#endif
        Code::OpCode current_opcode = static_cast<Code::OpCode>(Read8(pCode, ccp));
        // We want to reuse enum values from BuiltinTraitsType here.
        BuiltinTraitsType result_dt = Traits_Unknown;

        switch (current_opcode)
        {
        case Code::op_convert_i:
            result_dt = Traits_SInt;
            break;
        case Code::op_convert_u:
            result_dt = Traits_UInt;
            break;
        case Code::op_convert_d:
            result_dt = Traits_Number;
            break;
        default:
            break;
        }

        if (result_dt == Traits_Unknown)
        {
            // No conversion operator was found.
            // Get type info from the opcode itself.
            switch (next_opcode)
            {
            case Code::op_increment_i:
            case Code::op_decrement_i:
                result_dt = Traits_SInt;
                break;
            default:
                result_dt = Traits_Number;
                break;
            }
        }
        else
        {
            OrigOpcodePos.PushBack(ccp);
            ++drop_num;
#ifdef GFX_AS3_TRACE
            if (ui.NeedToCheckOpCode()) { ui.OnOpCode(current_offset); }
            current_offset = ccp;
#endif
            current_opcode = static_cast<Code::OpCode>(Read8(pCode, ccp));
        }

        int dest_reg_num = 0;
        bool is_setlocal = true;

        switch (current_opcode)
        {
        case Code::op_setlocal:
            dest_reg_num = ReadU30(pCode, ccp);
            break;
        case Code::op_setlocal0:
            dest_reg_num = 0;
            break;
        case Code::op_setlocal1:
            dest_reg_num = 1;
            break;
        case Code::op_setlocal2:
            dest_reg_num = 2;
            break;
        case Code::op_setlocal3:
            dest_reg_num = 3;
            break;
        default:
            is_setlocal = false;
            break;
        }

        if (is_setlocal && dest_reg_num == src_reg_num)
        {
            // We want to reuse enum values from BuiltinTraitsType here.
            BuiltinTraitsType reg_dt = Traits_Number;
            const Traits* reg_tr = GetValueTraits(reg);

            if (reg_tr && reg_tr->IsInstanceTraits())
                reg_dt = reg_tr->GetTraitsType();

            switch(next_opcode)
            {
            case Code::op_increment:
            case Code::op_increment_i:
#ifdef GFX_AS3_TRACE
                if (ui.NeedToCheckOpCode()) { ui.OnOpCode(current_offset); }
#endif

                if (reg_dt == result_dt)
                {
                    switch (reg_dt)
                    {
                    case Traits_SInt:
                        PushNewOpCode(Code::op_inclocal_ti, src_reg_num);
#if defined(SF_AS3_AOTC2)
                        st.PushSNodeOC(AOT::SNodeOC::op_inclocal_ti, src_reg_num);
#endif
                        st.exec_convert_reg_i(src_reg_num);
#ifdef SF_AS3_AOTC
                        st.aot_reg_op(reg, SNode1::opIncr);
#endif
                        break;
                    case Traits_UInt:
                        PushNewOpCode(Code::op_inclocal_tu, src_reg_num);
#if defined(SF_AS3_AOTC2)
                        st.PushSNodeOC(AOT::SNodeOC::op_inclocal_tu, src_reg_num);
#endif
                        st.exec_convert_reg_u(src_reg_num);
#ifdef SF_AS3_AOTC
                        st.aot_reg_op(reg, SNode1::opIncr);
#endif
                        break;
                    case Traits_Number:
                        PushNewOpCode(Code::op_inclocal, src_reg_num);
#if defined(SF_AS3_AOTC2)
                        st.PushSNodeOC(AOT::SNodeOC::op_inclocal, src_reg_num);
#endif
                        st.exec_convert_reg_d(src_reg_num);
#ifdef SF_AS3_AOTC
                        st.aot_reg_op(reg, SNode1::opIncr);
#endif
                        break;
                    default:
                        break;
                    }
                }
                else if (result_dt == Traits_SInt)
                {
                    PushNewOpCode(Code::op_inclocal_i, src_reg_num);
#if defined(SF_AS3_AOTC2)
                    st.PushSNodeOC(AOT::SNodeOC::op_inclocal_i, src_reg_num);
#endif
                    st.exec_convert_reg_i(src_reg_num);
#ifdef SF_AS3_AOTC
                    st.aot_reg_op(reg, SNode1::opIncr);
#endif
                }
                else
                {
                    PushNewOpCode(Code::op_inclocal, src_reg_num);
#if defined(SF_AS3_AOTC2)
                    st.PushSNodeOC(AOT::SNodeOC::op_inclocal, src_reg_num);
#endif
                    st.exec_convert_reg_d(src_reg_num);
#ifdef SF_AS3_AOTC
                    st.aot_reg_op(reg, SNode1::opIncr);
#endif
                }

                // Skip prev combination ...
                SkipOrigOpCode(bcp, ccp);

                return true;
            case Code::op_decrement:
            case Code::op_decrement_i:
#ifdef GFX_AS3_TRACE
                if (ui.NeedToCheckOpCode()) { ui.OnOpCode(CurrOffset); }
#endif

                if (reg_dt == result_dt)
                {
                    switch (reg_dt)
                    {
                    case Traits_SInt:
                        PushNewOpCode(Code::op_declocal_ti, src_reg_num);
#if defined(SF_AS3_AOTC2)
                        st.PushSNodeOC(AOT::SNodeOC::op_declocal_ti, src_reg_num);
#endif
                        st.exec_convert_reg_i(src_reg_num);
#ifdef SF_AS3_AOTC
                        st.aot_reg_op(reg, SNode1::opDecr);
#endif
                        break;
                    case Traits_UInt:
                        PushNewOpCode(Code::op_declocal_tu, src_reg_num);
#if defined(SF_AS3_AOTC2)
                        st.PushSNodeOC(AOT::SNodeOC::op_declocal_tu, src_reg_num);
#endif
                        st.exec_convert_reg_u(src_reg_num);
#ifdef SF_AS3_AOTC
                        st.aot_reg_op(reg, SNode1::opDecr);
#endif
                        break;
                    case Traits_Number:
                        PushNewOpCode(Code::op_declocal, src_reg_num);
#if defined(SF_AS3_AOTC2)
                        st.PushSNodeOC(AOT::SNodeOC::op_declocal, src_reg_num);
#endif
                        st.exec_convert_reg_d(src_reg_num);
#ifdef SF_AS3_AOTC
                        st.aot_reg_op(reg, SNode1::opDecr);
#endif
                        break;
                    default:
                        break;
                    }
                }
                else if (result_dt == Traits_SInt)
                {
                    PushNewOpCode(Code::op_declocal_i, src_reg_num);
#if defined(SF_AS3_AOTC2)
                    st.PushSNodeOC(AOT::SNodeOC::op_declocal_i, src_reg_num);
#endif
                    st.exec_convert_reg_i(src_reg_num);
#ifdef SF_AS3_AOTC
                    st.aot_reg_op(reg, SNode1::opDecr);
#endif
                }
                else
                {
                    PushNewOpCode(Code::op_declocal, src_reg_num);
#if defined(SF_AS3_AOTC2)
                    st.PushSNodeOC(AOT::SNodeOC::op_declocal, src_reg_num);
#endif
                    st.exec_convert_reg_d(src_reg_num);
#ifdef SF_AS3_AOTC
                    st.aot_reg_op(reg, SNode1::opDecr);
#endif
                }

                // Skip prev combination ...
                SkipOrigOpCode(bcp, ccp);

                return true;
            default:
                break;
            }
        } else
        {
            OrigOpcodePos.PopBack(drop_num);
        }
    }

    return false;
}

void Tracer::EmitPopPrevResult(TR::State& st)
{
    using namespace Abc;

    SF_UNUSED1(st);
    // Get rid of previous result in two different ways.
    // 1) If result was retrieved via "getabsobject" command,
    //    then get rid of this command.
    // 2) If it is something else, then just do "pop" instead.
    const Code::OpCode prev_opcode = GetNewTopOpCode();

    switch (prev_opcode)
    {
    /*
    // DO NOT delete.
    // Opcodes below retrieve result, but they also read arguments from stack.
    case Code::op_getsuper:
    case Code::op_getproperty:
    case Code::op_getslot:
    case Code::op_getabsslot:
    */

    // Opcodes below do not read arguments from stack.
    // So, it is safe to get rid of them.
    case Code::op_pushnull:
    case Code::op_pushundefined:
    case Code::op_pushbyte:
    case Code::op_pushshort:
    case Code::op_pushtrue:
    case Code::op_pushfalse:
    case Code::op_pushnan:
    case Code::op_dup:
    case Code::op_pushstring:
    case Code::op_pushint:
    case Code::op_pushuint:
    case Code::op_pushdouble:
    case Code::op_pushnamespace:
    case Code::op_getlex:
    case Code::op_getlocal:
    case Code::op_getglobalscope:
    case Code::op_getscopeobject:
    case Code::op_getouterscope:
    case Code::op_getglobalslot:
    case Code::op_getabsobject:
    case Code::op_getlocal0:
    case Code::op_getlocal1:
    case Code::op_getlocal2:
    case Code::op_getlocal3:
        PopNewOpCode();
#if defined(SF_AS3_AOTC2)
        st.PopSNodeOC();
#endif
        break;
    default:
        PushNewOpCode(Code::op_pop);
#if defined(SF_AS3_AOTC2)
        st.PushSNodeOC(AOT::SNodeOC::op_pop);
#endif
        break;
    }
}

bool Tracer::EmitGetClassTraits(TR::State& st, AS3::Multiname as3_mn, bool objOnStack, bool altered)
{
    using namespace Abc;

#if 0
    // This should be checked outside of this method.
    if (as3_mn.IsRunTime())
        return false;
#endif

    VM& vm = GetVM();

    // FindClassTraits() can handle Multiname (multiple namespaces).
    const ClassTraits::Traits* ctr = FindClassTraits(vm, as3_mn, GetFile().GetAppDomain());
    if (ctr == NULL)
        return false;

#if defined(SF_AS3_AOTC2)
    Pickable<SNode> sn(SF_NEW AOT::SNodeValue(Value(*const_cast<ClassTraits::Traits*>(ctr), Value::NotNull), AOT::SNodeValue::tAbsObject));
    st.PushSNode(sn);
#endif

    if (ctr->IsValid())
    {
        // This is just a class.

        InstanceTraits::Traits& itr = ctr->GetInstanceTraits();
        if (itr.HasConstructorSetup())
        {
            // Class is already created.
            // Emit getabsobject.

            Value value(&ctr->GetInstanceTraits().GetClass());

            if (EmitGetAbsObject(st, value, objOnStack SF_AOTC2_ARG(true)))
            {
                if (altered)
                    value.SetArtificialClass();

                st.PushOp(value);

                return true;
            }
        }
        else if (itr.IsAbcObject())
        {
            // Class will be created later. Just get a slot.
            InstanceTraits::UserDefined& ud = static_cast<InstanceTraits::UserDefined&>(itr);
            AS3::Object& obj = ud.GetScript();

            UPInt slot_index = 0;
            const SlotInfo* si = AS3::FindFixedSlot(GetVM(), obj.GetTraits(), as3_mn, slot_index, &obj);

            if (si)
            {
                // Not sure about Value::NullOrNot.
                Value type(const_cast<ClassTraits::Traits&>(*ctr), Value::NullOrNot);

                if (altered)
                    type.SetArtificialClass();

                st.PushOp(type);

                EmitGetAbsObject(st, Value(&obj), false SF_AOTC2_ARG(true));
                EmitGetAbsSlot(st, slot_index SF_AOTC2_ARG(true));

                return true;
            }
            else
                SF_ASSERT(false);
        }
        else
            SF_ASSERT(false);
    }
    else
        SF_ASSERT(false);

    return false;
}

bool Tracer::EmitFindProperty(TR::State& st, const int mn_index, bool get_prop, const Abc::Code::OpCode consumer SF_DEBUG_ARG(bool show_getprop))
{
    using namespace Abc;
    using namespace TR;

    SF_DEBUG_CODE(show_getprop = show_getprop;)
    SF_UNUSED1(consumer);

    const Abc::Multiname& mn = GetAbcFile().GetConstPool().GetMultiname(mn_index);
    AS3::Multiname as3_mn(GetFile(), mn);
    UPInt scope_ind = 0;
    State::ScopeType stype;
#ifdef GFX_AS3_TRACE
    FlashUI& ui = GetUI();;
#endif

    // Theoretically, we can get us into trouble if somebody decides to declare a property
    // having same name as a Class and will use object having this property with the "with"
    // statement.
    if (as3_mn.IsRunTime() || as3_mn.IsNameLate())
        return false;

#if 0
    // DO NOT delete this code, so, I do not make this mistake again.
    // We may not do that because property can have the same name as class.
    // Handle Classes.
    if (!IsOrigPropSetter(consumer) && EmitGetClassTraits(st, as3_mn))
        return true;
#endif

    PropRef prop;
    st.FindProp(prop, as3_mn, stype, scope_ind);

    if (prop)
    {
        if (prop.IsAsValue())
            // It is a dynamic property. Skip binding.
            // Fixed properties have SlotInfo.
            return false;

        SF_ASSERT(prop.GetAsSlotInfo());
        const UPInt slot_ind = prop.GetSlotIndex();
        const SlotInfo* si = prop.GetAsSlotInfo();
        bool alteredBehaviour = (!IsOrigPropSetter(consumer) && si->IsClass());
        const Traits* tr = GetValueTraits(prop.GetThis());
        SF_ASSERT(tr);

        switch (stype)
        {
        case State::stScopeStack:
        case State::stStoredScope:
            // Altered behavior. Always get Class.
            if (alteredBehaviour && EmitGetClassTraits(st, as3_mn, false, alteredBehaviour))
                return true;

            // We may not use tr->IsClassTraits() here because class itself
            // can be not created yet.

            if (EmitGetAbsObject(st, prop.GetThis(), false))
            {
                // Well, this is an absolute object.
            } else if (stype == State::stScopeStack)
            {
                // Let's replace it with op_getscopeobject.
                PushNewOpCode(Code::op_getscopeobject, scope_ind);

#if defined(SF_AS3_AOTC2)
                st.PushSNodeOC(AOT::SNodeOC::op_getscopeobject, scope_ind);
#endif
            } else if (!EmitGetAbsObject(st, prop.GetThis(), false))
            {
                // Activation object is always different.
                // It would be a bad idea to store address to it.
                // Regular object can be garbage collected by the time we refer to it.
                // Let's replace it with op_getouterscope.
                EmitGetOuterScope(st, scope_ind);
            }

            if (!get_prop && consumer == Code::op_constructprop)
            {
                // Call construct on a regular slot.
                get_prop = true;
                alteredBehaviour = true;
            }

            if (get_prop)
            {
                // Code below is similar to one in EmitGetProperty().

                // !!! ATTN: We MAY NOT skip BT_Set here.
                if (si->IsCode())
                {
                    if (si->GetAValueInd().IsValid() && !tr->IsInterface())
                    {
                        if (si->IsMethod())
                        {
                            EmitGetAbsSlot(st, slot_ind);

                            // Retrieve function type.
                            InstanceTraits::Traits& itr = st.GetFunctType(tr->GetVT().GetValue(si->GetAValueInd()));
                            st.PushOp(Value(itr, CanBeNull(itr)));
                        }
                        else
                        {
                            const Code::OpCode opcode = Code::op_callmethod;

                            // Emit op_callmethod.
                            // Zero arguments. No adjustment is required.
                            PushNewOpCode(opcode, si->GetValueInd(), 0);

#if defined(SF_AS3_AOTC2)
                            st.PushSNodeOC(AOT::SNodeOC::GetOP(opcode), si->GetValueInd(), 0);
#endif

                            // Retrieve function return type. In case of SlotInfo::BT_Code we need to retrieve function type.
                            InstanceTraits::Traits& itr = GetVM().GetFunctReturnType(tr->GetVT().GetValue(si->GetAValueInd()), tr->GetAppDomain());

#ifdef SF_AS3_AOTC
                            SPtr<SNode> sn;
                            Pickable<AOT::SNodeN> psn(SF_NEW AOT::SNodeN(st.GetThis(), AOT::SNodeN::opCallGet, MakeCallName(opcode, prop.GetThis(), slot_ind) SF_DEBUG_ARG(CurrOffset - 1)));
                            sn.Pick(psn);

                            // Result type.
                            psn->RType = &itr;
#endif

                            st.PushOp(Value(itr, CanBeNull(itr) SF_AOTC_ARG(*sn)));
                        }

                        // We are done. Result type is on stack.
                        return true;
                    }
                }

                EmitGetAbsSlot(st, slot_ind);

                // Retrieve type of the property.
                {
                    Value type;

#ifdef SF_AS3_AOTC
                    SPtr<SNode> sn;
                    sn.Pick(SF_NEW AOT::SNodeName("something is missing here."));
#endif

                    if (!st.GetPropertyType(*tr, *si, type SF_AOTC_ARG(*sn)))
                        return false;

                    if (alteredBehaviour)
                        type.SetArtificialClass();

                    st.PushOp(type);
                }

#ifdef GFX_AS3_TRACE
                if (show_getprop && ui.NeedToCheckOpCode())
                {
                    Abc::TCodeOffset ccp = CurrOffset;

                    Read8(pCode, ccp);
                    ReadU30(pCode, ccp);
                    ui.OnOpCode(ccp);
                }
#endif
            }
            else
                st.PushOp(prop.GetThis());

            return true;
        case State::stGlobalObject:
            // Check global objects. We are looking for classes.
            // We know about all registered class objects.

            // Altered behavior. Always get Class.
            if (alteredBehaviour && EmitGetClassTraits(st, as3_mn, false, alteredBehaviour))
                return true;

            if (get_prop)
            {
                // It looks like there is no need to handle getters in global objects.
                if (!si->IsCode())
                {
                    // It is possible that we are trying to retrieve "undefined" or "NaN".
                    const Value& _this = prop.GetThis();
                    if (_this.IsObject() && _this.GetObject() == &GetVM().GetGlobalObjectCPP())
                    {
                        // We found a property of the GlobalObjectCPP
                        const ASString name = tr->GetSlotName(AbsoluteIndex(slot_ind));

                        enum en{nothing, undef, nan};
                        en e = nothing;
                        if (name == "undefined")
                            e = undef;
                        else if (name == "NaN")
                            e = nan;

                        if (e != nothing)
                        {
                            // Emit op_pushundefined or op_pushnan.
                            PushNewOpCode(e == undef ? Code::op_pushundefined : Code::op_pushnan);
#if defined(SF_AS3_AOTC2)
                            st.PushSNodeOC(e == undef ? AOT::SNodeOC::op_pushundefined : AOT::SNodeOC::op_pushnan);
#endif
                            st.PushOp(e == undef ? Value::GetUndefined() : Value(NumberUtil::NaN()));

#ifdef GFX_AS3_TRACE
                            if (show_getprop && ui.NeedToCheckOpCode())
                            {
                                Abc::TCodeOffset ccp = CurrOffset;

                                Read8(pCode, ccp);
                                ReadU30(pCode, ccp);
                                ui.OnOpCode(ccp);
                            }
#endif

                            return true;
                        }
                    }

                    // Emit getabsobject.
                    if (EmitGetAbsObject(st, prop.GetThis(), false))
                    {
                        // Get type.
                        {
                            Value type;

#ifdef SF_AS3_AOTC
                            using namespace AOT;

                            SPtr<SNode> sn;
                            sn.Pick(SF_NEW SNodeName("something is missing here."));
#endif

                            if (!st.GetPropertyType(*GetValueTraits(prop.GetThis()), *si, type SF_AOTC_ARG(*sn)))
                                return false;

                            if (alteredBehaviour)
                                type.SetArtificialClass();

                            st.PushOp(type);
                        }

                        // Get absolute slot on a global object.
                        EmitGetAbsSlot(st, slot_ind);

#ifdef GFX_AS3_TRACE
                        if (show_getprop && ui.NeedToCheckOpCode())
                        {
                            Abc::TCodeOffset ccp = CurrOffset;

                            Read8(pCode, ccp);
                            ReadU30(pCode, ccp);
                            ui.OnOpCode(ccp);
                        }
#endif

                        return true;
                    }
                }
            }
            else
            {
                const Value& value = prop.GetThis();

                const bool rc = EmitGetSlot(st, value, slot_ind);
                if (rc)
                {
                    st.PushOp(value);
#ifdef GFX_AS3_TRACE
                    if (show_getprop && ui.NeedToCheckOpCode())
                    {
                        Abc::TCodeOffset ccp = CurrOffset;

                        Read8(pCode, ccp);
                        ReadU30(pCode, ccp);
                        ui.OnOpCode(ccp);
                    }
#endif
                }

                return rc;
            }

            break;
        }
    }
    else if (get_prop)
    {
        // ??? Is this an artificial case?
        if (EmitGetClassTraits(st, as3_mn))
            return true;
    }

    return false;
}

void Tracer::EmitGetAbsSlot(TR::State& st, const UPInt index SF_AOTC2_ARG(bool simulate))
{
    using namespace Abc;

    SF_UNUSED1(st);
    const GlobalSlotIndex gsind = GlobalSlotIndex(AbsoluteIndex(index));
    SF_ASSERT(gsind.IsValid());
    PushNewOpCode(Code::op_getabsslot, gsind.Get());

#if defined(SF_AS3_AOTC2)
    if (!simulate)
        st.PushSNodeOC(AOT::SNodeOC::op_getabsslot, gsind.Get());
#endif
}

void Tracer::EmitSetAbsSlot(TR::State& st, const SlotInfo& si, const Traits* val_tr, const UPInt index)
{
    using namespace Abc;

#ifdef ENABLE_STRICT_SETSLOT
    const UPInt offset = si.GetValueInd();
    VM& vm = GetVM();

    if (!si.IsConst())
    {
        switch (si.GetBindingType())
        {
        case SlotInfo::BT_Value:
            // We do not care about type of value.
            return PushNewOpCode(Code::op_setslot_value, offset);
        case SlotInfo::BT_ObjectAS:
            if (!IsNotObjectType(val_tr))
                return PushNewOpCode(Code::op_setslot_obj_as, offset);
            break;
        case SlotInfo::BT_ObjectCpp:
            if (!IsNotObjectType(val_tr))
                return PushNewOpCode(Code::op_setslot_obj_cpp, offset);
            break;
        case SlotInfo::BT_Boolean:
            if (val_tr == &vm.GetITraitsBoolean())
                return PushNewOpCode(Code::op_setslot_bool, offset);
            break;
        case SlotInfo::BT_Int:
            if (val_tr == &vm.GetITraitsSInt())
                return PushNewOpCode(Code::op_setslot_sint, offset);
            break;
        case SlotInfo::BT_UInt:
            if (val_tr == &vm.GetITraitsUInt())
                return PushNewOpCode(Code::op_setslot_uint, offset);
            break;
        case SlotInfo::BT_Number:
            if (val_tr == &vm.GetITraitsNumber())
                return PushNewOpCode(Code::op_setslot_num, offset);
            break;
        case SlotInfo::BT_String:
            if (val_tr == &vm.GetITraitsString())
                return PushNewOpCode(Code::op_setslot_str, offset);
            break;
        default:
            break;
        }
    }
#else
    SF_UNUSED2(si, val_tr);
#endif

    SF_UNUSED1(st);
    const GlobalSlotIndex gsind = GlobalSlotIndex(AbsoluteIndex(index));
    SF_ASSERT(gsind.IsValid());
    PushNewOpCode(Code::op_setabsslot, gsind.Get());

#if defined(SF_AS3_AOTC2)
    st.PushSNodeOC(AOT::SNodeOC::op_setabsslot, gsind.Get());
#endif
}

void Tracer::EmitInitAbsSlot(TR::State& st, const UPInt index)
{
    using namespace Abc;

    SF_UNUSED1(st);
    const GlobalSlotIndex gsind = GlobalSlotIndex(AbsoluteIndex(index));
    SF_ASSERT(gsind.IsValid());
    PushNewOpCode(Code::op_initabsslot, gsind.Get());

#if defined(SF_AS3_AOTC2)
    st.PushSNodeOC(AOT::SNodeOC::op_initabsslot, gsind.Get());
#endif
}

bool Tracer::EmitGetSlot(TR::State& st, const Value& value, const UPInt index, bool objOnStack)
{
    using namespace Abc;

    SF_UNUSED1(st);

    if (value.IsTraits())
    {
        EmitGetAbsSlot(st, index);

        return true;
    }
    else
    {
        switch (value.GetKind())
        {
        case Value::kUndefined:
            if (objOnStack)
                EmitPopPrevResult(st);
            PushNewOpCode(Code::op_pushundefined);
#if defined(SF_AS3_AOTC2)
            st.PushSNodeOC(AOT::SNodeOC::op_pushundefined);
#endif
            return true;
        case Value::kBoolean:
            if (objOnStack)
                EmitPopPrevResult(st);
            PushNewOpCode(value.AsBool() ? Code::op_pushtrue : Code::op_pushfalse);
#if defined(SF_AS3_AOTC2)
            st.PushSNodeOC(value.AsBool() ? AOT::SNodeOC::op_pushtrue : AOT::SNodeOC::op_pushfalse);
#endif
            return true;
            /* TBD: pushint/pushuint
        case Value::kInt:
        case Value::kUInt:
            */
        case Value::kNumber:
            if (value.IsNaN())
            {
                if (objOnStack)
                    EmitPopPrevResult(st);
                PushNewOpCode(Code::op_pushnan);
#if defined(SF_AS3_AOTC2)
                st.PushSNodeOC(AOT::SNodeOC::op_pushnan);
#endif
                return true;
            }
            break;
            /* TBD: We can introduce an ObjectTag for strings.
        case Value::kString:
            break;
            */
        /*
        // Life time of objects and namespaces is not known.
        // TBD: We can keep a list of used objects in a function metainfo.
        case Value::kObject:
        case Value::kFunction:
        case Value::kNamespace:
        */
#if 1 // SS Dev.
        // This code had issues with unloading. See ComponentBrowser.swf
        case Value::kObject:
            // Duplicate code for debugging purposes.
            return EmitGetAbsObject(st, value, objOnStack);
#endif
        case Value::kClass:
            return EmitGetAbsObject(st, value, objOnStack);
        default:
            break;
        }
    }

    return false;
}

bool Tracer::EmitGetProperty(const Abc::Code::OpCode opcode, TR::State& st, const TR::ReadMnObject& args, const int mn_index)
{
    using namespace Abc;

    const AS3::Multiname& mn = args.ArgMN;
    const bool super_tr = (opcode == Code::op_getsuper);
    const Traits* tr = GetValueTraits(args.ArgObject, super_tr);

#ifdef SF_AS3_AOTC
    SPtr<SNode> sn = st.AotGetProperty(args.ArgObject, mn);
#endif

    // Ignore run-time multinames for the time being.
    if (!mn.IsRunTime() && !mn.IsAttr() && !(mn.IsQName() && mn.IsAnyNamespace()))
    {
        if (tr)
        {
            VM& vm = GetVM();
            UPInt slot_ind = 0;

            if (mn.GetName().IsTraits())
            {
#if 0
                // Disabled because it probably causes problems.
                if (tr->IsArrayLike() && IsNumericType(mn.GetName()))
                {
                    const AS3::Multiname mn_getValue(vm.GetPublicNamespace(), vm.GetStringManager().CreateConstString("getValue"));
                    const SlotInfo* si = FindFixedSlot(vm, *tr, mn_getValue, slot_ind, NULL);
                    if (si && si->GetValueInd().IsValid())
                    {
                        // It looks like there is no reason to try to call callsupermethod here.

                        // Emit op_callmethod.
                        PushCode(Code::op_callmethod);
                        PushCode(static_cast<UInt32>(si->GetValueInd().Get()));
                        PushCode(1);

                        // Retrieve type of elements.
                        InstanceTraits::Traits* elem_tr = NULL;

                        // TODO: Vector.<Class>
                        if (tr->IsInstanceTraits())
                        {
                            const BuiltinTraitsType tt = tr->GetTraitsType();
                            switch (tt)
                            {
                            case Traits_Vector_int:
                                elem_tr = &st.GetSIntType();
                                break;
                            case Traits_Vector_uint:
                                elem_tr = &st.GetUIntType();
                                break;
                            case Traits_Vector_double:
                                elem_tr = &st.GetNumberType();
                                break;
                            case Traits_Vector_String:
                                elem_tr = &st.GetStringType();
                                break;
                            case Traits_Vector_object:
                                {
                                    const InstanceTraits::Traits* itr = static_cast<const InstanceTraits::Traits*>(tr);

                                    if (itr && itr->HasConstructorSetup())
                                        elem_tr = &static_cast<const ClassTraits::fl_vec::Vector_object&>(itr->GetClass().GetClassTraits()).GetEnclosedClassTraits().GetInstanceTraits();
                                    else
                                        elem_tr = &st.GetObjectType();
                                }
                                break;
                            default:
                                elem_tr = &st.GetObjectType();
                                break;
                            }
                        }
                        else
                            elem_tr = &st.GetObjectType();

                        st.PushOp(Value(*elem_tr SF_AOTC_ARG(sn)));

                        return true;
                    }
                }
#endif

                // Fall back.
                goto fall_back;
            }

            /* DO NOT delete this.
             * Theoretically, I can use this path with primitive data types.
             * There is no data, we have only code and getters in this case.
            if (!tr->IsInterface())
            */
            if (!tr->IsInterface() && !IsPrimitiveType(tr) && !IsNamespaceType(tr))
            {
                // Object is on stack. This is the difference from EmitFindProperty().
                // ??? Is this an artificial case?
                if (tr->IsGlobal() && EmitGetClassTraits(st, mn, true))
                    return true;

                const SlotInfo* si = FindFixedSlot(vm, *tr, mn, slot_ind, NULL);

                if (si)
                {
                    // !!! ATTN: We MAY NOT skip BT_Set here.
                    if (si->IsCode())
                    {
                        if (si->GetAValueInd().IsValid())
                        {
                            // Call the code.
                            if (si->IsMethod())
                            {
                                // op_getsuper cannot be handled correctly by op_getabsslot.
                                if (opcode == Code::op_getproperty)
                                {
                                    if (mn.IsNameLate())
                                        // Get rid of runtime multiname on stack.
                                        PushNewOpCode(Code::op_pop);

                                    EmitGetAbsSlot(st, slot_ind);

                                    // Retrieve function type.
                                    // We do not call GetFunctReturnType() here because found code is not
                                    // supposed to be called. It can be just assigned to a variable.
                                    InstanceTraits::Traits& itr = st.GetFunctType(tr->GetVT().GetValue(si->GetAValueInd()));
                                    st.PushOp(Value(itr, CanBeNull(itr) SF_AOTC_ARG(*sn)));

                                    return true;
                                }
                            }
                            else
                            {
                                if (mn.IsNameLate())
                                {
                                    // Get rid of runtime multiname on stack.
                                    PushNewOpCode(Code::op_pop);
#if defined(SF_AS3_AOTC2)
                                    st.PushSNodeOC(AOT::SNodeOC::op_pop);
#endif
                                }

                                // Zero arguments. No adjustment is required.
                                PushNewOpCode(opcode == Code::op_getsuper ? Code::op_callsupermethod : Code::op_callmethod, si->GetValueInd(), 0); // Num of arguments.
#if defined(SF_AS3_AOTC2)
                                st.PushSNodeOC(opcode == Code::op_getsuper ? AOT::SNodeOC::op_callsupermethod : AOT::SNodeOC::op_callmethod, si->GetValueInd(), 0);
#endif

                                // Retrieve function return type. In case of SlotInfo::BT_Code we need to retrieve function type.
                                InstanceTraits::Traits& itr = GetVM().GetFunctReturnType(tr->GetVT().GetValue(si->GetAValueInd()), tr->GetAppDomain());

#ifdef SF_AS3_AOTC
                                SPtr<SNode> sn;
                                Pickable<AOT::SNodeN> psn(SF_NEW AOT::SNodeN(st.GetThis(), AOT::SNodeN::opCallGet, MakeCallName(opcode, args.ArgObject, slot_ind) SF_DEBUG_ARG(CurrOffset - 1)));
                                sn.Pick(psn);

                                // Result type.
                                psn->RType = &itr;
#endif

                                st.PushOp(Value(itr, CanBeNull(itr) SF_AOTC_ARG(*sn)));

                                return true;
                            }
                        }
                    }
                    else
                    {
                        // Get type.
                        {
                            Value type;

                            if (!st.GetPropertyType(*tr, *si, type SF_AOTC_ARG(*sn)))
                                return false;

                            st.PushOp(type);
                        }

                        if (mn.IsNameLate())
                        {
                            // Get rid of runtime multiname on stack.
                            PushNewOpCode(Code::op_pop);
#if defined(SF_AS3_AOTC2)
                            st.PushSNodeOC(AOT::SNodeOC::op_pop);
#endif
                        }

                        EmitGetAbsSlot(st, slot_ind);

                        return true;
                    }
                }
            }


            // Default implementation.
            {
                // The property with the name specified by the multiname will be resolved in object, and the
                // value of that property will be pushed onto the stack. If the property is unresolved,
                // undefined is pushed onto the stack.

                UPInt slot_ind_def = 0;
                const SlotInfo* si = FindFixedSlot(GetVM(), *tr, mn, slot_ind_def, NULL);
                if (si)
                {
                    // Get type.
                    {
                        Value type;

#ifdef SF_AS3_AOTC
                        if (si->IsGetter())
                        {
                            Pickable<AOT::SNodeN> psn(SF_NEW AOT::SNodeN(st.GetThis(), AOT::SNodeN::opCallGet, MakeCallName(opcode, args.ArgObject, slot_ind) SF_DEBUG_ARG(CurrOffset - 1)));
                            sn.Pick(psn);

                            // Retrieve function return type. In case of SlotInfo::BT_Code we need to retrieve function type.
                            InstanceTraits::Traits& itr = GetVM().GetFunctReturnType(tr->GetVT().GetValue(si->GetAValueInd()));

                            // Result type.
                            psn->RType = &itr;
                        }
#endif

                        if (!st.GetPropertyType(*tr, *si, type SF_AOTC_ARG(*sn)))
                            return false;

                        st.PushOp(type);
                    }

                    // Restore back old opcode.
                    PushNewOpCode(opcode, mn_index);

#if defined(SF_AS3_AOTC2)
                    st.PushSNodeOC(AOT::SNodeOC::GetOP(opcode), mn_index);
#endif

                    return true;
                }

                // Check for predefined Vector<> types.
                {
                    InstanceTraits::Traits* vitr = NULL;
                    const BuiltinTraitsType tt = tr->GetTraitsType();

                    if (tr->IsInstanceTraits())
                    {
                        switch (tt)
                        {
                        case Traits_Vector_int:
                            vitr = &vm.GetITraitsSInt();
                            break;
                        case Traits_Vector_uint:
                            vitr = &vm.GetITraitsUInt();
                            break;
                        case Traits_Vector_double:
                            vitr = &vm.GetITraitsNumber();
                            break;
                        case Traits_Vector_String:
                            vitr = &vm.GetITraitsString();
                            break;
                        default:
                            break;
                        }
                    }

                    if (!vitr && tt == Traits_Vector_object)
                    {
                        // Traits_Vector_object can also be of type Vector.<Class>.
                        const ClassTraits::fl_vec::Vector_object& ctr = static_cast<const ClassTraits::fl_vec::Vector_object&>(tr->GetClass().GetClassTraits());
                        const ClassTraits::Traits& elem_ctr = ctr.GetEnclosedClassTraits();

                        vitr = &elem_ctr.GetInstanceTraits();
                    }

                    if (vitr)
                    {
                        st.PushOp(Value(*vitr, CanBeNull(*vitr) SF_AOTC_ARG(*sn)));

                        // Restore back old opcode.
                        PushNewOpCode(opcode, mn_index);

#if defined(SF_AS3_AOTC2)
                        st.PushSNodeOC(AOT::SNodeOC::GetOP(opcode), mn_index);
#endif

                        return true;
                    }
                }

            }
        }

    }

    // Fall back.
    fall_back:
    {
        st.PushOp(Value(GetObjectType(), Value::NullOrNot SF_AOTC_ARG(*sn)));

        // Restore back old opcode.
        PushNewOpCode(opcode, mn_index);

#if defined(SF_AS3_AOTC2)
        st.PushSNodeOC(AOT::SNodeOC::GetOP(opcode), mn_index);
#endif
    }

    return true;
}

bool Tracer::EmitSetProperty(const Abc::Code::OpCode opcode, const TR::ReadValueMnObject& args, const int mn_index)
{
    using namespace Abc;

    TR::State& st = args.GetState();
    const Value& obj = args.ArgObject;
    const Value& val = args.ArgValue;
    const AS3::Multiname& mn = args.ArgMN;

    SF_UNUSED1(st);

    const bool super_tr = (opcode == Code::op_setsuper);
    const Traits* obj_tr = GetValueTraits(obj, super_tr);
    const Traits* val_tr = GetValueTraits(val);

    // Ignore run-time multinames for the time being.
    if (!mn.IsRunTime() && !mn.IsAttr() && !(mn.IsQName() && mn.IsAnyNamespace()))
    {
        if (obj_tr)
        {
            VM& vm = GetVM();
            UPInt slot_ind = 0;
            const SlotInfo* si = FindFixedSlot(vm, *obj_tr, mn, slot_ind, NULL);

            if (si)
            {
                if (si->IsCode())
                {
                    if (si->GetAValueInd().IsValid() && !obj_tr->IsInterface())
                    {
                        if (si->IsSetter())
                        {
                            // Get rid of Multiname on stack.
                            if (mn.IsNameLate())
                            {
                                PushNewOpCode(Code::op_swap);
                                PushNewOpCode(Code::op_pop);
#if defined(SF_AS3_AOTC2)
                                st.PushSNodeOC(AOT::SNodeOC::op_swap);
                                st.PushSNodeOC(AOT::SNodeOC::op_pop);
#endif
                            }

                            // One argument. It is supposed to be on stack.
                            // No adjustment is required.
                            PushNewOpCode(
                                super_tr ? Code::op_callsupermethod : Code::op_callmethod, 
                                si->GetValueInd() + 1, 
                                1
                                );
                            PushNewOpCode(Code::op_pop);

#if defined(SF_AS3_AOTC2)
                            st.PushSNodeOC(
                                super_tr ? AOT::SNodeOC::op_callsupermethod : AOT::SNodeOC::op_callmethod,
                                si->GetValueInd() + 1, 
                                1
                                );
                            st.PushSNodeOC(AOT::SNodeOC::op_pop);
#endif

#ifdef SF_AS3_AOTC
                            Pickable<AOT::SNodeN> sn(SF_NEW AOT::SNodeN(st.GetThis(), AOT::SNodeN::opCallSet, MakeCallName(opcode, obj, slot_ind) SF_DEBUG_ARG(CurrOffset - 1)));
                            st.PushSNode(sn);

                            // Argument.
                            sn->Args.PushBack(SNode::RetrieveNode(val));

                            // Result type.
                            sn->RType = &vm.GetITraitsVoid();

                            return true;
#else
                            goto end;
#endif
                        }
                    }
                }
                else
                {
                    // Get rid of Multiname on stack.
                    if (mn.IsNameLate())
                    {
                        PushNewOpCode(Code::op_swap);
                        PushNewOpCode(Code::op_pop);
#if defined(SF_AS3_AOTC2)
                        st.PushSNodeOC(AOT::SNodeOC::op_swap);
                        st.PushSNodeOC(AOT::SNodeOC::op_pop);
#endif
                    }

                    if (opcode == Code::op_initproperty)
                        EmitInitAbsSlot(st, slot_ind);
                    else
                        EmitSetAbsSlot(st, *si, val_tr, slot_ind);

                    goto end;
                }
            }
        }
    }

    // Restore back old opcode.
    PushNewOpCode(opcode, mn_index);

#if defined(SF_AS3_AOTC2)
    st.PushSNodeOC(AOT::SNodeOC::GetOP(opcode), mn_index);
#endif

    end:

#ifdef SF_AS3_AOTC
    {
        using namespace AOT;

        SPtr<SNode> sn_obj = st.AotGetProperty(obj, mn);
        if (sn_obj.GetPtr())
            st.PushSNode(Pickable<SNode>(SF_NEW SNode2(sn_obj, SNode2::opAssign, val SF_DEBUG_ARG(st.BCP - 1))));
    }
#endif

    return true;
}

#ifdef SF_AS3_AOTC

SPtr<SNode> Tracer::MakeCallName(const Abc::Code::OpCode opcode, const Value& obj, UPInt slot_index) const
{
    using namespace Abc;

    const bool super_tr = (opcode == Code::op_callsuper || opcode == Code::op_callsupervoid);
    const Traits* tr = GetValueTraits(obj, super_tr);

    SPtr<SNode> sn_pref;

    if (tr->IsGlobal())
        sn_pref.Pick(SF_NEW AOT::SNodeVar(tr->GetName(), tr));
    else
    {
        Value::KindType kind = obj.GetKind();

        switch (kind)
        {
        case Value::kSNodeIT: // SNode + InstanceTraits
        case Value::kSNodeCT: // SNode + ClassTraits
            {
                SNode& n = obj.GetSNode();
                if (n.Is(SNode::tVar))
                {
                    const AOT::SNodeVar& var = static_cast<const AOT::SNodeVar&>(n);
                    sn_pref.Pick(SF_NEW AOT::SNodeVar(var.Name, tr));
//                     if (var.Name.IsString())
//                     {
//                         const ASString name = var.Name.AsString();
//                         sn_pref.Pick(SF_NEW AOT::SNodeVar(tr->GetName(), tr));
//                     }
                }
				else
					sn_pref = &n;
            }
            break;
#if 0
        case Value::kObject:
        case Value::kClass:
            sn_pref.Pick(SF_NEW AOT::SNodeVar(tr->GetName(), tr));
            break;
#else
        case Value::kClass:
            sn_pref.Pick(SF_NEW AOT::SNodeVar(obj, tr));
            break;
            break;
#endif
        default:
            break;
        }
    }

    SPtr<SNode> sn_name;
    const AbsoluteIndex aind(slot_index);
    const SlotInfo& si = tr->GetSlotInfo(aind);
    const Instances::fl::Namespace& ns = si.GetNamespace();
    ASString name(GetVM().GetStringManager().CreateEmptyString());

    if (ns.IsAS3())
        name += "AS3";

    name += tr->GetSlotName(aind);

    sn_name.Pick(SF_NEW AOT::SNodeVar(sn_pref, Value(name), tr));

    return sn_name;
}

#endif

bool Tracer::EmitCall(const Abc::Code::OpCode opcode, TR::State& st, const TR::ReadArgsMnObject& args, const UInt32 mn_index)
{
    using namespace Abc;

    const bool super_tr = (opcode == Code::op_callsuper || opcode == Code::op_callsupervoid);
    const Traits* tr = GetValueTraits(args.ArgObject, super_tr);
    const AS3::Multiname& mn = args.ArgMN;
    const unsigned argc = args.GetCallArgsNum();

    // Ignore run-time multinames for the time being.
    if (!mn.IsRunTime() && !mn.IsAttr() && !(mn.IsQName() && mn.IsAnyNamespace()))
    {
        UPInt slot_index = 0;
        VM& vm = GetVM();

        if (tr && !tr->IsInterface())
        {
            const SlotInfo* si = FindFixedSlot(vm, *tr, mn, slot_index, NULL);

            // In case of a method or a getter ...
            if (si && si->GetAValueInd().IsValid() && (si->IsMethod() || si->IsGetter()))
            {
                // Retrieve a function return type.
                InstanceTraits::Traits& itr = vm.GetFunctReturnType(tr->GetVT().GetValue(si->GetAValueInd()), GetFile().GetAppDomain());

#ifdef SF_AS3_AOTC
                SPtr<SNode> sn;
                Pickable<AOT::SNodeN> psn(SF_NEW AOT::SNodeN(st.GetThis(), AOT::SNodeN::opCall, MakeCallName(opcode, args.ArgObject, slot_index), args.GetCallArgsNum(), args.GetCallArgs() SF_DEBUG_ARG(CurrOffset - 1)));
                sn.Pick(psn);

                // Result type.
                psn->RType = &itr;

//                 st.PushSNode(sn);
#endif

                // Call the code.
                if (si->IsMethod())
                {
                    Code::OpCode new_opcode;

                    switch (opcode)
                    {
                    case Code::op_callsuper:
                    case Code::op_callsupervoid:
                        // Emit op_callsupermethod.
                        new_opcode = Code::op_callsupermethod;
                        break;
                    default:
                        // Emit op_callmethod.
                        new_opcode = Code::op_callmethod;
                        break;
                    }

                    UPInt new_argc = argc;

#ifdef SF_AS3_EMIT_DEF_ARGS
                    new_argc = EmitDefaultArgs(tr->GetVT().GetRaw(si->GetAValueInd()), argc, st);
#endif

                    PushNewOpCode(new_opcode, si->GetValueInd(), new_argc);
#if defined(SF_AS3_AOTC2)
                    st.PushSNodeOC(AOT::SNodeOC::GetOP(new_opcode), si->GetValueInd(), new_argc);
#endif

                    if (opcode == Code::op_callpropvoid || opcode == Code::op_callsupervoid)
                    {
                        PushNewOpCode(Code::op_pop);
#if defined(SF_AS3_AOTC2)
                        st.PushSNodeOC(AOT::SNodeOC::op_pop);
#endif
                    }
                    else
                        st.PushOp(Value(itr, CanBeNull(itr) SF_AOTC_ARG(*sn)));

                    return true;
                }
                else if (si->IsGetter())
                {
                    Code::OpCode new_opcode = opcode;
                    switch (opcode)
                    {
                    case Code::op_callsuper:
                    case Code::op_callsupervoid:
                        // Emit op_callsupergetter.
                        new_opcode = Code::op_callsupergetter;
                        break;
                    default:
                        // Code::op_callproperty || Code::op_callpropvoid
                        // Emit op_callgetter.
                        new_opcode = Code::op_callgetter;
                        break;
                    }

                    // Getter can be called with more than zero arguments.
                    // Example: Function.prototype(1,true,false,'string', new Date(),null);
                    // Zero arguments. No adjustment is required.
                    PushNewOpCode(new_opcode, si->GetValueInd(), argc);
#if defined(SF_AS3_AOTC2)
                    st.PushSNodeOC(AOT::SNodeOC::GetOP(new_opcode), si->GetValueInd(), argc);
#endif

                    if (opcode == Code::op_callpropvoid || opcode == Code::op_callsupervoid)
                    {
                        PushNewOpCode(Code::op_pop);
#if defined(SF_AS3_AOTC2)
                        st.PushSNodeOC(AOT::SNodeOC::op_pop);
#endif
                    }
                    else
                        st.PushOp(Value(itr, CanBeNull(itr) SF_AOTC_ARG(*sn)));

                    return true;
                }
            }
        }

        {
            // It can happen that we "call" a type name.
            const ClassTraits::Traits* ctr = FindClassTraits(vm, mn, GetFile().GetAppDomain());
            if (ctr)
            {
#ifdef SF_AS3_AOTC
                SPtr<SNode> sn_name;
                sn_name.Pick(SF_NEW AOT::SNodeVar(ctr->GetName(), ctr));
                SPtr<AOT::SNodeN> sn;
                sn.Pick(SF_NEW AOT::SNodeN(st.GetThis(), AOT::SNodeN::opCallType, sn_name, argc, args.GetCallArgs() SF_DEBUG_ARG(CurrOffset - 1)));
//                 st.PushSNode(SPtr<SNode>(sn));
#endif

                // If we are here we have a valid type.
                // At least we know a result type.

                // It can happen that Class wasn't put on stack. This can happen in case of "with" statement.
                if (tr->IsClassTraits())
                {
                    // Call method Call() on an object. No adjustment is required because arguments are handled by the method..
                    PushNewOpCode(Code::op_callobject, argc);

#if defined(SF_AS3_AOTC2)
                    st.PushSNodeOC(AOT::SNodeOC::op_callobject, argc);
#endif

                    if (opcode == Code::op_callpropvoid || opcode == Code::op_callsupervoid)
                        PushNewOpCode(Code::op_pop);
                    else
                    {
                        InstanceTraits::Traits& itr = ctr->GetInstanceTraits();
                        // Retrieve type of result.
                        // Are we 100% sure that this is not null?
                        st.PushOp(Value(itr, Value::NotNull SF_AOTC_ARG(*sn)));
                    }

                    return true;
                }

#if 1
                // DO NOT DELETE this code!
                // !!! This "type name" may actually be something else.
                // Emit original op_callproperty and set precise type, which, by the way, is not null.
                // It looks like there is no need to validate argc here because all
                // constructors take variable number of arguments.
                // Example: Test\AS3\ABC\Testcases\acceptance\scope_stack\scope_stack.02.as
                {
                    // Method signature is not resolved. We have to check arguments at run-time.
                    PushNewOpCode(opcode, mn_index, argc);
#if defined(SF_AS3_AOTC2)
                    st.PushSNodeOC(AOT::SNodeOC::GetOP(opcode), mn_index, argc);
#endif

                    if (opcode != Code::op_callpropvoid && opcode != Code::op_callsupervoid)
                    {
                        InstanceTraits::Traits& itr = ctr->GetInstanceTraits();
                        // Are we 100% sure that this is not null?
                        st.PushOp(Value(itr, Value::NotNull SF_AOTC_ARG(*sn)));
                    }
                }

                return true;
#endif
            }
        }

    }

    // There is nothing we can do any more. Emit original op_callproperty.
    // We cannot validate argc here because name wasn't resolved.
    // Theoretically, this can be a problem.
    {
#ifdef SF_AS3_AOTC
        SPtr<AOT::SNodeN> sn;
        // ??? How do we know that this is a call on "this" object?
        // SPtr<SNode> sn_name = st.AotGetProperty(args.ArgObject, args.ArgMN);
        SPtr<SNode> sn_name = st.AotGetProperty(args.ArgObject, args.ArgMN);
        Pickable<AOT::SNodeN> psn(SF_NEW AOT::SNodeN(st.GetThis(), AOT::SNodeN::opCall, sn_name, argc, args.GetCallArgs() SF_DEBUG_ARG(CurrOffset - 1)));
        sn.Pick(psn);

        // Result type.
        psn->RType = &GetObjectType();

//         st.PushSNode(SPtr<SNode>(sn));
#endif

        // Method signature is not resolved. We have to check arguments at run-time.
        PushNewOpCode(opcode, mn_index, argc);
#if defined(SF_AS3_AOTC2)
        st.PushSNodeOC(AOT::SNodeOC::GetOP(opcode), mn_index, argc);
#endif

        if (opcode != Code::op_callpropvoid && opcode != Code::op_callsupervoid)
            st.PushOp(Value(GetObjectType(), Value::NullOrNot SF_AOTC_ARG(*sn)));
    }

#if 0
    // For debugging.
    switch (opcode)
    {
    case Code::op_callproperty:
    case Code::op_callpropvoid:
    case Code::op_callproplex:
    case Code::op_callsuper:
    case Code::op_callsupervoid:
        break;
    default:
        SF_ASSERT(false);
        break;
    }
#endif

    return true;
}

UPInt Tracer::EmitDefaultArgs(const Value& func, const unsigned argc, TR::State& st)
{
    using namespace Abc;

    UPInt param_count = 0;
    VM& vm = GetVM();
    SF_UNUSED1(st);

    // VTable stores MethodInd or Thunk.
    if (func.GetKind() == Value::kMethodInd)
    {
        // MethodInd.
        const AS3::Traits& real_tr = func.GetTraits();
        SF_ASSERT(real_tr.GetFilePtr());
        VMAbcFile& file = *real_tr.GetFilePtr();
        const Abc::ConstPool& cp = file.GetConstPool();
        const Abc::MiInd method_ind(func.GetMethodInfoInd());
        const Abc::MethodInfo& mi = file.GetMethodInfo(method_ind);
        param_count = mi.GetParamCount();
        const UPInt first_opt_param_num = param_count - mi.GetOptionalParamCount();

        for (UPInt i = argc; i < param_count; ++i)
        {
            const Abc::Multiname& arg_type = mi.GetParamType(cp, i);

            if (i < first_opt_param_num)
            {
                // Generic default value.

                if (arg_type.IsAnyType())
                {
                    PushNewOpCode(Code::op_pushundefined);
#if defined(SF_AS3_AOTC2)
                    st.PushSNodeOC(AOT::SNodeOC::op_pushundefined);
#endif
                }
                else if (arg_type.IsBoolean(cp))
                {
                    PushNewOpCode(Code::op_pushfalse);
#if defined(SF_AS3_AOTC2)
                    st.PushSNodeOC(AOT::SNodeOC::op_pushfalse);
#endif
                }
                else if (arg_type.IsInt(cp))
                {
                    PushNewOpCode(Code::op_pushint, 0);
#if defined(SF_AS3_AOTC2)
                    st.PushSNodeOC(AOT::SNodeOC::op_pushint);
#endif
                }
                else if (arg_type.IsUInt(cp))
                {
                    PushNewOpCode(Code::op_pushuint, 0);
#if defined(SF_AS3_AOTC2)
                    st.PushSNodeOC(AOT::SNodeOC::op_pushuint);
#endif
                }
                else if (arg_type.IsString(cp))
                {
#if defined(SF_AS3_AOTC2)
                    st.PushSNodeOC(AOT::SNodeOC::op_pushstring);
#endif
                    const ASString str = vm.GetStringManager().CreateEmptyString();
                    // Store reference to a string.
                    file.ASStringNodeSet.Set(str.GetNode());
                    // Store pointer to a string.
                    PushNewOpCode(Code::op_pushstring, reinterpret_cast<Abc::TOpCode::ValueType>(str.GetNode()));
                }
                else if (arg_type.IsNumber(cp))
                {
                    PushNewOpCode(Code::op_pushnan);
#if defined(SF_AS3_AOTC2)
                    st.PushSNodeOC(AOT::SNodeOC::op_pushnan);
#endif
                }
                else
                {
                    PushNewOpCode(Code::op_pushnull);
#if defined(SF_AS3_AOTC2)
                    st.PushSNodeOC(AOT::SNodeOC::op_pushnull);
#endif
                }
            }
            else
            {
                // Stored default value.

                const Abc::ValueDetail& d = mi.GetOptionalParam(i - first_opt_param_num);
                const Abc::ValueKind k = d.GetKind();
                const int value_ind = d.GetIndex();

                switch(k)
                {
                case Abc::CONSTANT_Int:
                    PushNewOpCode(Code::op_pushint, cp.GetInt(value_ind));
#if defined(SF_AS3_AOTC2)
                    st.PushSNodeOC(AOT::SNodeOC::op_pushint);
#endif
                    break;
                case Abc::CONSTANT_UInt:
                    PushNewOpCode(Code::op_pushuint, cp.GetUInt(value_ind));
#if defined(SF_AS3_AOTC2)
                    st.PushSNodeOC(AOT::SNodeOC::op_pushuint);
#endif
                    break;
                case Abc::CONSTANT_Double:
                    PushNewOpCodeNumber(Code::op_pushdouble, MakeValueNumber(cp.GetDouble(value_ind)));
#if defined(SF_AS3_AOTC2)
                    st.PushSNodeOC(AOT::SNodeOC::op_pushdouble);
#endif
                    break;
                case Abc::CONSTANT_Utf8:
                    {
#if defined(SF_AS3_AOTC2)
                        st.PushSNodeOC(AOT::SNodeOC::op_pushstring);
#endif

                        const ASString str = file.GetInternedString(value_ind);
                        // There is no need to store reference to a string because this is an interned string.
                        // It is already stored.
                        PushNewOpCode(Code::op_pushstring, reinterpret_cast<Abc::TOpCode::ValueType>(str.GetNode()));
                    }
                    break;
                case Abc::CONSTANT_True:
                    PushNewOpCode(Code::op_pushtrue);
#if defined(SF_AS3_AOTC2)
                    st.PushSNodeOC(AOT::SNodeOC::op_pushtrue);
#endif
                    break;
                case Abc::CONSTANT_False:
                    PushNewOpCode(Code::op_pushfalse);
#if defined(SF_AS3_AOTC2)
                    st.PushSNodeOC(AOT::SNodeOC::op_pushfalse);
#endif
                    break;
                case Abc::CONSTANT_Null:
                    PushNewOpCode(Code::op_pushnull);
#if defined(SF_AS3_AOTC2)
                    st.PushSNodeOC(AOT::SNodeOC::op_pushnull);
#endif
                    break;
                case Abc::CONSTANT_Undefined:
                    PushNewOpCode(Code::op_pushundefined);
#if defined(SF_AS3_AOTC2)
                    st.PushSNodeOC(AOT::SNodeOC::op_pushundefined);
#endif
                    break;
                case Abc::CONSTANT_Namespace:
                case Abc::CONSTANT_PackageNamespace:
                case Abc::CONSTANT_PackageInternalNs:
                case Abc::CONSTANT_ProtectedNamespace:
                case Abc::CONSTANT_ExplicitNamespace:
                case Abc::CONSTANT_StaticProtectedNs:
                case Abc::CONSTANT_PrivateNs:
                    SF_ASSERT(false);
                    // No break on purpose.
                default:
                    PushNewOpCode(Code::op_pushundefined);
#if defined(SF_AS3_AOTC2)
                    st.PushSNodeOC(AOT::SNodeOC::op_pushundefined);
#endif
                    break;
                }

            }
        }
    }
    else
    {
        // Thunk.

        const ThunkInfo& thunk = func.AsThunk();

        if (!thunk.HasEllipsis())
        {
            param_count = thunk.GetMaxArgNum() == SF_AS3_VARARGNUM ? thunk.GetMinArgNum() : thunk.GetMaxArgNum();
            const UPInt first_opt_param_num = param_count - thunk.GetDefArgNum();

            if (argc < thunk.GetMinArgNum())
            {
                const TypeInfo& ti = AS3::fl::ArgumentErrorTI;

                Value obj;
                const Value argv[2] = {Value(vm.GetStringManager().CreateConstString("Error #1063: Argument count mismatch.")), Value((SInt32)VM::eWrongArgumentCountError)};

                // We shouldn't have any problems here.
                vm.ConstructBuiltinValue(obj, ti, 2, argv).DoNotCheck();
                
                EmitGetAbsObject2(st, obj);

                PushNewOpCode(Abc::Code::op_throw);

#if defined(SF_AS3_AOTC2)
                st.PushSNodeOC(AOT::SNodeOC::op_throw);
#endif

                return argc;
            }
            else
            {
                for (UPInt i = argc; i < param_count; ++i)
                {
                    if (i < first_opt_param_num)
                    {
                        // Generic default value.

                        // ArgType[0] is type of return value.
                        const TypeInfo* arg_type = thunk.ArgType[i + 1];

                        if (arg_type == NULL)
                        {
                            PushNewOpCode(Code::op_pushundefined);
#if defined(SF_AS3_AOTC2)
                            st.PushSNodeOC(AOT::SNodeOC::op_pushundefined);
#endif
                        }
                        else if (arg_type->IsBoolean())
                        {
                            PushNewOpCode(Code::op_pushfalse);
#if defined(SF_AS3_AOTC2)
                            st.PushSNodeOC(AOT::SNodeOC::op_pushfalse);
#endif
                        }
                        else if (arg_type->IsInt())
                        {
                            PushNewOpCode(Code::op_pushint, 0);
#if defined(SF_AS3_AOTC2)
                            st.PushSNodeOC(AOT::SNodeOC::op_pushint);
#endif
                        }
                        else if (arg_type->IsUInt())
                        {
                            PushNewOpCode(Code::op_pushuint, 0);
#if defined(SF_AS3_AOTC2)
                            st.PushSNodeOC(AOT::SNodeOC::op_pushuint);
#endif
                        }
                        else if (arg_type->IsString())
                        {
#if defined(SF_AS3_AOTC2)
                            st.PushSNodeOC(AOT::SNodeOC::op_pushstring);
#endif
                            const ASString str = vm.GetStringManager().CreateEmptyString();
                            // Store reference to a string.
                            GetFile().ASStringNodeSet.Set(str.GetNode());
                            // Store pointer to a string.
                            PushNewOpCode(Code::op_pushstring, reinterpret_cast<Abc::TOpCode::ValueType>(str.GetNode()));
                        }
                        else if (arg_type->IsNumber())
                        {
                            PushNewOpCode(Code::op_pushnan);
#if defined(SF_AS3_AOTC2)
                            st.PushSNodeOC(AOT::SNodeOC::op_pushnan);
#endif
                        }
                        else
                        {
                            PushNewOpCode(Code::op_pushnull);
#if defined(SF_AS3_AOTC2)
                            st.PushSNodeOC(AOT::SNodeOC::op_pushnull);
#endif
                        }
                    }
                    else
                    {
                        // Stored default value.

                        const Abc::ConstValue& cv = thunk.DefArgValue[i - first_opt_param_num];
                        const Abc::ValueKind k = cv.GetKind();

                        switch (k)
                        {
                        case Abc::CONSTANT_Undefined:
                            PushNewOpCode(Code::op_pushundefined);
#if defined(SF_AS3_AOTC2)
                            st.PushSNodeOC(AOT::SNodeOC::op_pushundefined);
#endif
                            break;
                        case Abc::CONSTANT_Utf8:
                            {
#if defined(SF_AS3_AOTC2)
                                st.PushSNodeOC(AOT::SNodeOC::op_pushstring);
#endif

                                const ASString str = vm.GetStringManager().CreateConstString(Instances::fl::GlobalObjectCPP::Strings[cv.GetIndex()]);
                                // Store reference to a string.
                                GetFile().ASStringNodeSet.Set(str.GetNode());
                                // Store pointer to a string.
                                PushNewOpCode(Code::op_pushstring, reinterpret_cast<Abc::TOpCode::ValueType>(str.GetNode()));
                            }
                            break;
                        case Abc::CONSTANT_Int:
                            PushNewOpCode(Code::op_pushint, Instances::fl::GlobalObjectCPP::Ints[cv.GetIndex()]);
#if defined(SF_AS3_AOTC2)
                            st.PushSNodeOC(AOT::SNodeOC::op_pushint);
#endif
                            break;
                        case Abc::CONSTANT_UInt:
                            PushNewOpCode(Code::op_pushuint, Instances::fl::GlobalObjectCPP::UInts[cv.GetIndex()]);
#if defined(SF_AS3_AOTC2)
                            st.PushSNodeOC(AOT::SNodeOC::op_pushuint);
#endif
                            break;
                        case Abc::CONSTANT_Double:
                            PushNewOpCodeNumber(Code::op_pushdouble, Instances::fl::GlobalObjectCPP::Numbers[cv.GetIndex()]);
#if defined(SF_AS3_AOTC2)
                            st.PushSNodeOC(AOT::SNodeOC::op_pushdouble);
#endif
                            break;
                        case Abc::CONSTANT_True:
                            PushNewOpCode(Code::op_pushtrue);
#if defined(SF_AS3_AOTC2)
                            st.PushSNodeOC(AOT::SNodeOC::op_pushtrue);
#endif
                            break;
                        case Abc::CONSTANT_False:
                            PushNewOpCode(Code::op_pushfalse);
#if defined(SF_AS3_AOTC2)
                            st.PushSNodeOC(AOT::SNodeOC::op_pushfalse);
#endif
                            break;
                        case Abc::CONSTANT_Null:
                            PushNewOpCode(Code::op_pushnull);
#if defined(SF_AS3_AOTC2)
                            st.PushSNodeOC(AOT::SNodeOC::op_pushnull);
#endif
                            break;
                        default:
                            break;
                        }
                    }
                }
            }
        }
    }

    return Alg::Max(param_count, static_cast<UPInt>(argc));
}

bool Tracer::EmitGetAbsObject(TR::State& st, const Value& value, bool objOnStack SF_AOTC2_ARG(bool simulate))
{
    using namespace Abc;

    bool result = false;

#ifdef SF_AS3_ENABLE_GETABSOBJECT
    const Traits& tr = GetVM().GetValueTraits(value);

    if (value.IsClass() || tr.IsGlobal())
    {
        if (objOnStack)
            EmitPopPrevResult(st);

        EmitGetAbsObject2(st, value SF_AOTC2_ARG(simulate));

        result = true;
    }
#else
    SF_UNUSED3(st, value, objOnStack);
#endif

    return result;
}

void Tracer::EmitGetAbsObject2(TR::State& st, const Value& value SF_AOTC2_ARG(bool simulate))
{
    using namespace Abc;

    Value::ObjectTag tag = value.GetObjectTag();

    PushNewOpCode(
        Code::op_getabsobject,
        reinterpret_cast<UPInt>(value.GetObject()) + tag
        );

#if defined(SF_AS3_AOTC2)
    if (!simulate)
    {
        // Push object itself.
        Pickable<SNode> sn(SF_NEW AOT::SNodeValue(value, AOT::SNodeValue::tAbsObject));
        st.PushSNode(sn);
    }
#else
    SF_UNUSED1(st);
#endif

    // Putting GetGlobalObjectCPP in AbsObjects list will cause memory leaks.
    if (value.GetObject() != (Object*)&GetVM().GetGlobalObjectCPP())
        GetFile().AbsObjects.Set(value.GetObject());
}

void Tracer::EmitGetOuterScope(TR::State& st, const UPInt index)
{
    using namespace Abc;

    SF_UNUSED1(st);
    PushNewOpCode(Code::op_getouterscope, index);

#if defined(SF_AS3_AOTC2)
    st.PushSNodeOC(AOT::SNodeOC::op_getouterscope, index);
#endif
}

void Tracer::Output(OutputMessageType type, const char* msg)
{
    GetFile().GetVM().GetUI().Output(type, msg);
}

bool Tracer::OnOpCode(Abc::TCodeOffset offset) 
{ 
    SF_UNUSED1(offset);
#ifdef GFX_AS3_TRACE
    // Trace trace state and opcode.
    AS3::TraceVM(*this, "\t");
    // We trace original byte code.
    AS3::TraceOpCode(*this, BCode, offset);
#endif

    return false;
}

void Tracer::AddBlock(Abc::TCodeOffset from)
{
    TR::Block* block = GetBlock(from);
    SF_ASSERT(block);

    if (block && from == block->GetFrom())
        return;

    TR::Block* newNode = SF_HEAP_NEW(GetHeap()) TR::Block(from);
    block->InsertNodeAfter(newNode);

    if (CurrBlock)
        newNode->CurOpStackSize = CurrBlock->CurOpStackSize;
}

TR::Block* Tracer::AddBlock(const TR::State& st, Abc::TCodeOffset from, TR::Block::EType type, bool checkOpCode)
{
    using namespace Abc;

#ifdef SF_AS3_TRACE_DEAD_BLOCK
    if (CurrBlock && CurrBlock->IsDeadBlock())
        // Do nothing.
        return CurrBlock;
#endif

    if (from >= CodeEnd)
        // No blocks after CodeEnd.
        return NULL;

    bool is_dead = type == TR::Block::tDead;
    TR::Block* block = GetBlock(from);
    SF_ASSERT(block);
    bool onlyCreateState = false;

    if (block && from == block->GetFrom())
    {
        // Block already exists.
        if (block->GetStatePtr())
        {
            // State is already set up.
            // The "dead" flag will be ignored here.
            return block;
        }
        else
            // Block is already created but State.
            onlyCreateState = true;
    }

    // We need to create a new block.

    //
    //const bool not_initialized = dead;

    // Check current opcode.
    if (checkOpCode)
    {
        // !!! We can store prev_opcode and pass as an argument. That would save time.
        const Code::OpCode curr_opcode = GetCurrOrigOpCode(from);

        switch (curr_opcode)
        {
        // We still need to handle this op_label here.
        case Code::op_label:
        case Code::op_debug:
        case Code::op_debugline:
        case Code::op_debugfile:
            // If there is a label, then this block is not dead.
            is_dead = false;
            break;
#if 0
        // This block doesn't seem to be necessary any more.
        // Do not delete it for the time being.
        case Code::op_jump:
            {
                const Code::OpCode prev_opcode = GetOrigPrevOpCode2(from);

                if (prev_opcode == Code::op_throw)
                    dead = false;
            }
            break;
#endif
        default:
            break;
        }
    }

    TR::State* state = SF_HEAP_NEW(GetHeap()) TR::State(st);
    States.PushBack(state);
    TR::Block* newNode = NULL;

    if (type == TR::Block::tDead && !is_dead)
        // Block was originally marked as dead, but it is not.
        type = TR::Block::tUnknown;

    if (onlyCreateState)
    {
        newNode = block;
        newNode->State = state;
        newNode->SetType(type);
    }
    else
    {
        newNode = SF_HEAP_NEW(GetHeap()) TR::Block(*state, from, type);
        block->InsertNodeAfter(newNode);

        //if (not_initialized)
        if (type == TR::Block::tDead)
            // So far we do not check opcode only in exception blocks.
            newNode->SetInitialized(false);

        if (CurrBlock)
            newNode->CurOpStackSize = CurrBlock->CurOpStackSize;
    }

    return newNode;
}

const Traits* Tracer::GetValueTraits(const Value& v, const bool super_tr) const
{
    const Traits* result = NULL;

    if (super_tr)
        result = &GetOriginationTraits();

    if (result == NULL)
    {
        switch (v.GetKind())
        {
        case Value::kUndefined:
			result = &GetVoidType();
            break;
#ifdef SF_AS3_AOTC
        case Value::kSNodeIT:
#endif
        case Value::kInstanceTraits:
            result = &v.GetInstanceTraits();
            break;
#ifdef SF_AS3_AOTC
        case Value::kSNodeCT:
#endif
        case Value::kClassTraits:
            result = &v.GetClassTraits();
            break;
        default:
            result = &GetVM().GetValueTraits(v);
        }

        // A hack.
        if (result && result == &GetVM().GetClassTraitsClassClass())
            result = &GetVM().GetClassTraitsObject();
    }

    if (super_tr && result)
        result = result->GetParent();

    return result;
}

const InstanceTraits::Traits& Tracer::GetInstanceTraits(const Value& v) const
{
    const InstanceTraits::Traits* result = NULL;

	if (v.IsNull())
		result = &GetNullType();
	else
	{
		switch (v.GetKind())
		{
		case Value::kUndefined:
			result = &GetClassType();
			break;
#ifdef SF_AS3_AOTC
        case Value::kSNodeIT:
#endif
		case Value::kInstanceTraits:
			result = &v.GetInstanceTraits();
			break;
#ifdef SF_AS3_AOTC
        case Value::kSNodeCT:
#endif
		case Value::kClassTraits:
			result = &v.GetClassTraits().GetInstanceTraits();
			break;
		default:
			result = &GetVM().GetInstanceTraits(v);
		}
	}

#if 0
    // ClassType (any type) is not the same as ObjectType.
    // ClassType can be "undefined". ObjectType can be only "null".
    // A hack.
    if (result && result == &GetClassType())
        result = &GetObjectType();
#endif

    return *result;
}

Scaleform::GFx::AS3::Value Tracer::GetGlobalObject(const TR::State& st) const
{
#if defined(SF_AS3_ENABLE_EXPLICIT_GO)
#if defined(SF_AS3_AOTC)
    const Value& _this = st.GetThis();

    if (_this.IsInstanceTraits() && _this.GetInstanceTraits().IsGlobal())
    {
        // _this.IsObject() && _this.GetObject() == &CF.GetGlobalObjectScript()
        // There is still no guarantee that this is a "this" object.
        InstanceTraits::Traits& itr = _this.GetInstanceTraits();
        // This is "this" object.
        ASString this_str = GetVM().GetStringManager().CreateConstString("this");
        SPtr<SNode> sn;

        // Mark this name as "this".
        sn.Pick(SF_NEW AOT::SNodeVar(this_str, &itr, true));
        return Value(itr, AS3::Value::NotNull, *sn);
    }
#endif

    SF_UNUSED1(st);
    return Value(&CF.GetGlobalObjectScript());

#else
    const ScopeStackType& sss = GetSavedScope();

    if (!sss.IsEmpty())
        return sss[0];

    return st.GetRegister(AbsoluteIndex(0));
#endif
}

const ClassTraits::Traits* Tracer::GetSlotCTraits(const Traits& tr, SlotIndex ind)
{
    const SlotInfo& si = tr.GetSlotInfo(AbsoluteIndex(ind, tr));
    return const_cast<SlotInfo&>(si).GetDataType(tr.GetVM());
}

bool Tracer::IsAnyType(const Traits* tr) const
{
#if 0
    // Not checking for ObjectType breaks code (VerifyError)
    // Example: 3DGenerator_AS3.swf
    // AnyType allows undefined, Object type doesn't.
    return  tr == &GetClassType();
#else
    return  tr == &GetObjectType() || tr == &GetClassType();
#endif
}

bool Tracer::IsPrimitiveType(const Traits* tr) const
{
    return  IsNotRefCountedType(tr) ||
            tr == &GetStringType();
}

bool Tracer::IsNotRefCountedType(const Traits* tr) const
{
    // ??? kUndefined = 0, // = 0
    return  tr == &GetBooleanType() ||
            tr == &GetVoidType() ||
            tr == &GetNullType() ||
            IsNumericType(tr);
}

bool Tracer::IsNumericType(const Traits* tr) const
{
    return  tr == &GetSIntType() ||
            tr == &GetUIntType() ||
            tr == &GetNumberType();
}

bool Tracer::IsNotObjectType(const Traits* tr) const
{
    return  IsNumericType(tr) ||
            tr == &GetBooleanType() ||
            tr == &GetVoidType() ||
            tr == &GetStringType();
}

bool Tracer::IsNotNullableType(const InstanceTraits::Traits* tr) const
{
    return IsNumericType(tr) || tr == &GetBooleanType();
}

bool Tracer::ValueIsOfType(const Value& value, const ClassTraits::Traits& type) const
{
    // There is no Tracer::GetClassTraits() function at this time.
    bool result = false;
    const Value::KindType kind = value.GetKind();

    switch (kind)
    {
#ifdef SF_AS3_AOTC
    case Value::kSNodeCT:
#endif
    case Value::kClassTraits:
        {
            const ClassTraits::Traits& ctr = value.GetClassTraits();
            result = ctr.IsOfType(type);
        }
        break;
    case Value::kClass:
        {
            const Class& cl = value.AsClass();
            const ClassTraits::Traits& ctr = cl.GetClassTraits();
            result = ctr.IsOfType(type);
        }
        break;
    default:
        {
            const InstanceTraits::Traits& itr = GetInstanceTraits(value);

            result = itr.IsOfType(type.GetInstanceTraits());
        }
        break;
    }

    return result;
}

void Tracer::PushNewOpCode(Abc::Code::OpCode opcode)
{
#ifdef SF_AS3_TRACE_DEAD_BLOCK
    if (CurrBlock->IsDeadBlock())
        // Do nothing.
        return;
#endif

    // Calculate OpStack size.
    CalcOpStackSize(opcode);

    NewOpcodePos.PushBack(MI.OpCode.GetSize());
    MI.OpCode.PushBack(opcode);
}

void Tracer::PushNewOpCode(Abc::Code::OpCode opcode, UPInt arg1)
{
#ifdef SF_AS3_TRACE_DEAD_BLOCK
    if (CurrBlock->IsDeadBlock())
        // Do nothing.
        return;
#endif

    // Calculate OpStack size.
    CalcOpStackSize(opcode, arg1);

    NewOpcodePos.PushBack(MI.OpCode.GetSize());
    MI.OpCode.PushBack(opcode);
    MI.OpCode.PushBack(arg1);
}

void Tracer::PushNewOpCodeNumber(Abc::Code::OpCode opcode, Value::Number arg1)
{
    using namespace Abc;

#ifdef SF_AS3_TRACE_DEAD_BLOCK
    if (CurrBlock->IsDeadBlock())
        // Do nothing.
        return;
#endif

    // Calculate OpStack size.
    // We do not need an argument here.
    CalcOpStackSize(opcode);

    NewOpcodePos.PushBack(MI.OpCode.GetSize());
    MI.OpCode.PushBack(opcode);

    if (sizeof(Abc::TOpCode::ValueType) == sizeof(Value::Number))
    {
        // This case handles 64-bit platforms and SF_NO_DOUBLE.
        union
        {
            TOpCode::ValueType  I;
            Value::Number       D;
        } u;

        u.D = arg1;
        MI.OpCode.PushBack(u.I);
    }
    else
    {
        union
        {
            UInt32  I[2];
            double  D;
        } u;

        u.D = arg1;
        MI.OpCode.PushBack(u.I[0]);
        MI.OpCode.PushBack(u.I[1]);
    }
}

void Tracer::PushNewOpCode(Abc::Code::OpCode opcode, UPInt arg1, UPInt arg2)
{
#ifdef SF_AS3_TRACE_DEAD_BLOCK
    if (CurrBlock->IsDeadBlock())
        // Do nothing.
        return;
#endif

    // Calculate OpStack size.
    CalcOpStackSize(opcode, arg1, arg2);

    NewOpcodePos.PushBack(MI.OpCode.GetSize());
    MI.OpCode.PushBack(opcode);
    MI.OpCode.PushBack(arg1);
    MI.OpCode.PushBack(arg2);
}

void Tracer::CalcOpStackSize(Abc::Code::OpCode opcode, UPInt arg1, UPInt arg2)
{
    using namespace Abc;

    // Adjust stack size.
    SInt32 st_size = 0;
    const Code::OpCodeInfo& oi = Code::GetOpCodeInfo(opcode);

    st_size -= oi.pop;

    if (oi.pop_mn)
    {
        const UPInt mn_index = arg1;
        const Abc::Multiname& mn = GetAbcFile().GetConstPool().GetMultiname(mn_index);

        switch (mn.GetKind())
        {
        case Abc::MN_QName:
        case Abc::MN_QNameA:
            // QName is not read from stack; both name and namespase known at compile-time.
            break;
        case Abc::MN_RTQName:
        case Abc::MN_RTQNameA:
            // Name is known at compile time, Namespace is resolved at runtime.
            --st_size;
            break;
        case Abc::MN_RTQNameL:
        case Abc::MN_RTQNameLA:
            // Both name and namespace are resolved at runtime.
            st_size -= 2;
            break;
        case Abc::MN_Multiname:
        case Abc::MN_MultinameA:
            // MInfo is not supposed to be read from stack.
            break;
        case Abc::MN_MultinameL:
        case Abc::MN_MultinameLA:
            // Namespace set is known at compile time, Name is resolved at runtime.        
            --st_size;
            break;
        case Abc::MN_Typename:
            // Recursive!!!?
            SF_ASSERT(false);
            break;
        default:
            SF_ASSERT(false);
            break;
        }
    }

    if (oi.pop_args)
    {
        const SInt32 arg_count = static_cast<SInt32>(arg2);

        if (opcode == Code::op_newobject)
            st_size -= arg_count * 2;
        else
            st_size -= arg_count;
    }

    st_size += oi.push;

    SF_ASSERT(CurrBlock);
    TR::Block& bl = *CurrBlock;
    SF_ASSERT(st_size >= 0 || bl.CurOpStackSize >= static_cast<UInt32>(-st_size));
    bl.CurOpStackSize += st_size;
    MI.MaxOpStackSize = Alg::Max(MI.MaxOpStackSize, bl.CurOpStackSize);
}

void Tracer::PushNewOpCodeArg0(Abc::TOpCode::ValueType arg)
{
#ifdef SF_AS3_TRACE_DEAD_BLOCK
    if (CurrBlock->IsDeadBlock())
        // Do nothing.
        return;
#endif

    MI.OpCode.PushBack(arg);
}

void Tracer::PushNewOpCodeArgNumber0(Value::Number arg)
{
    using namespace Abc;

#ifdef SF_AS3_TRACE_DEAD_BLOCK
    if (CurrBlock->IsDeadBlock())
        // Do nothing.
        return;
#endif

    if (sizeof(TOpCode::ValueType) == sizeof(Value::Number))
    {
        // This case handles 64-bit platforms and SF_NO_DOUBLE.
        union
        {
            TOpCode::ValueType  I;
            Value::Number       D;
        } u;

        u.D = arg;
        MI.OpCode.PushBack(u.I);
    }
    else
    {
        union
        {
            UInt32  I[2];
            double  D;
        } u;

        u.D = arg;
        MI.OpCode.PushBack(u.I[0]);
        MI.OpCode.PushBack(u.I[1]);
    }
}

void Tracer::PopNewOpCode()
{
#ifdef SF_AS3_TRACE_DEAD_BLOCK
    if (CurrBlock->IsDeadBlock())
        // Do nothing.
        return;
#endif

    SF_ASSERT(!NewOpcodePos.IsEmpty());
    Abc::TCodeOffset offset = NewOpcodePos.Back();

    NewOpcodePos.Pop();
    SF_ASSERT(MI.OpCode.GetSize() > offset);
    MI.OpCode.Resize(offset);
}

Abc::TOpCode::ValueType Tracer::GetNewOpCode() const
{
    return MI.OpCode.Back();
}

bool Tracer::IsOrigCall(Abc::Code::OpCode opcode)
{
    using namespace Abc;

    bool result = false;
    switch (opcode)
    {
    case Code::op_call:
    case Code::op_callmethod:
    case Code::op_callstatic:
    case Code::op_callsuper:
    case Code::op_callproperty:
    case Code::op_callproplex:
    case Code::op_callsupervoid:
    case Code::op_callpropvoid:
        result = true;
        break;
    default:
        break;
    }

    return result;
}

bool Tracer::IsOrigConstruct(Abc::Code::OpCode opcode)
{
    using namespace Abc;

    bool result = false;
    switch (opcode)
    {
    case Code::op_construct:
    case Code::op_constructsuper:
    case Code::op_constructprop:
        result = true;
        break;
    default:
        break;
    }

    return result;
}

Value::TraceNullType Tracer::CanBeNull(const Value& v) const
{
    Value::TraceNullType result = Value::NullOrNot;
    const Traits* tr = GetValueTraits(v);

    if (tr)
    {
        if (tr->IsClassTraits())
            // Class is always available.
            result = Value::NotNull;
        else
            result = CanBeNull(static_cast<const InstanceTraits::Traits&>(*tr));
    }

    return result;
}

}}} // namespace Scaleform { namespace GFx { namespace AS3 {

