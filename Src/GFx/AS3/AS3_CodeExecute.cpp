/**************************************************************************

Filename    :   AS3_CodeExecute.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "AS3_CodeExecute.h"
#include "Abc/AS3_Abc.h"
#include "AS3_VM.h"
#include "AS3_VMRead.h"
#include "AS3_MovieRoot.h"
#include "Obj/AS3_Obj_Namespace.h" // We need this header for GCC Release configuration to pass linking.
#include "Obj/AS3_Obj_Array.h"
#include "Obj/AS3_Obj_XML.h"
#ifdef GFX_ENABLE_XML
    #include "Obj/AS3_Obj_XMLList.h"
#endif
#include "Obj/AS3_Obj_Global.h"
#include "GFx/AMP/Amp_Server.h"

#define AS3_INLINE //SF_INLINE

namespace Scaleform { namespace GFx { namespace AS3 
{

///////////////////////////////////////////////////////////////////////////
// (ECMA-262 section 11.6.1)
// Can throw exceptions.
CheckResult Add(StringManager& sm, Value& result, const Value& l, const Value& r)
{
    // We cannot restrict "result" to deal with non-reference counted values only
    // because "result" can be a value on op-stack.
    //SF_ASSERT(!result.IsRefCounted());

#ifdef GFX_ENABLE_XML
    // XML part.
    if (l.IsObject() && r.IsObject() && l.GetObject() && r.GetObject())
    {
        Object* obj = l.GetObject();
        const XMLSupport& xmls = obj->GetVM().GetXMLSupport();

        if (xmls.Add(result, *l.GetObject(), *r.GetObject()))
            return true;
    }
#endif

    // Strings.
    // Step 7.
    if (l.IsString() || r.IsString())
    {
        ASString left(sm.CreateEmptyString());
        ASString right(sm.CreateEmptyString());

        if (l.Convert2String(left) && r.Convert2String(right))
        {
            result = left + right;
            return true;
        }

        // Exception
        return false;
    }

    // Operands are primitives but strings.
    if (l.IsPrimitive() && r.IsPrimitive())
    {
        // Convert to Number.
        Value::Number l_num = 0.0;
        Value::Number r_num = 0.0;

        if (l.Convert2NumberInline(l_num) && r.Convert2NumberInline(r_num))
        {
            result.SetNumber(l_num + r_num);
            return true;
        }

        // Exception
        return false;
    }

    // Convert to primitive values.
    Value l_prim;
    Value r_prim;
    ASStringManager* sm_ptr = sm.GetStringManager();

    // Steps 5 and 6.
    // NOTE 1. No hint is provided in the calls to ToPrimitive in steps 5 and 6.
    if (l.Convert2PrimitiveValueUnsafe(*sm_ptr, l_prim) && r.Convert2PrimitiveValueUnsafe(*sm_ptr, r_prim))
        return Add(sm, result, l_prim, r_prim);

    // Exception
    return false;
}

///////////////////////////////////////////////////////////////////////////
SF_INLINE
void VM::exec_nop() 
{ 
    // Do nothing 
}

AS3_INLINE
int VM::exec_throw(Abc::TOpCodeVTP cp, CallFrame& cf) 
{ 
    // Get exception object from the stack ...
    SF_ASSERT(OpStack.GetSize() > 0);
    ExceptionObj.Pick(OpStack);

    Abc::TOpCodeVTP code = cf.GetCode().GetDataPtr();
    const Abc::TCodeOffset offset = cp - code;

    return OnException(offset, cf);
}

SF_INLINE
void VM::exec_dxns(CallFrame& cf, UInt32 index) 
{
    // Sets the default XML namespace.

    // index is a u30 that must be an index into the string constant pool. The string at index is used
    // as the uri for the default XML namespace for this method.
    SetDefXMLNamespace(&cf.GetFile().GetInternedNamespace(index));
}

void VM::exec_dxnslate() 
{
    // Sets the default XML namespace with a value determined at runtime.

    const Value& v = OpStack.Top0();

    if (v.IsNamespace())
        // It is already a namespace. We do not need to do any conversion.
        SetDefXMLNamespace(v.GetNamespace());
    else
    {
        // The top value on the stack is popped, converted to a string, and that string is used as the uri
        // for the default XML namespace for this method.
        StringManager& sm = GetStringManager();
        ASString uri = sm.CreateEmptyString();

        // Empty string doesn't seem to affect default namespace.
        if (v.Convert2String(uri) && !uri.IsEmpty())
            SetDefXMLNamespace(MakeNamespace(Abc::NS_Public, uri));
    }

    OpStack.PopBack();
}

SF_INLINE
void VM::exec_kill(UInt32 v) 
{
    SetRegister(AbsoluteIndex(v), Value::GetUndefined());
}

SF_INLINE
int VM::exec_ifnlt(SInt32 v) 
{
    SH2<2> stack(OpStack);
    // No exceptions at this point.
    Boolean3 result;
    int r = 0;

    if (AbstractLessThan(result, stack._1, stack._2))
    {
        // If the result of the comparison is false ...
        // kUndefined is not true ...
        if (result != true3)
            r = v;
    }

    return r;
}

SF_INLINE
int VM::exec_ifnlt_ti(SInt32 v) 
{
    TSH2<2, SInt32, SInt32> stack(OpStack);
    int r = 0;

    if (stack._1 >= stack._2)
        r = v;

    return r;
}

SF_INLINE
int VM::exec_ifnlt_td(SInt32 v) 
{
    TSH2<2, Value::Number, Value::Number> stack(OpStack);
    int r = 0;

    if (stack._1 >= stack._2)
        r = v;

    return r;
}

SF_INLINE
int VM::exec_ifnle(SInt32 v) 
{
    SH2<2> stack(OpStack);
    // No exceptions at this point.
    Boolean3 result;
    int r = 0;

    if (AbstractLessThan(result, stack._2, stack._1))
    {
        // If the result of the comparison is true ...
        // kUndefined is not false ...
        if (result != false3)
            r = v;
    }

    return r;
}

SF_INLINE
int VM::exec_ifnle_ti(SInt32 v) 
{
    TSH2<2, SInt32, SInt32> stack(OpStack);
    int r = 0;

    if (stack._1 > stack._2)
        r = v;

    return r;
}

SF_INLINE
int VM::exec_ifnle_td(SInt32 v) 
{
    TSH2<2, Value::Number, Value::Number> stack(OpStack);
    int r = 0;

    if (stack._1 > stack._2)
        r = v;

    return r;
}

SF_INLINE
int VM::exec_ifngt(SInt32 v) 
{
    SH2<2> stack(OpStack);
    // No exceptions at this point.
    Boolean3 result;
    int r = 0;

    if (AbstractLessThan(result, stack._2, stack._1))
    {
        // If the result of the comparison is not true ...
        if (result != true3)
            r = v;
    }

    return r;
}

SF_INLINE
int VM::exec_ifngt_ti(SInt32 v) 
{
    TSH2<2, SInt32, SInt32> stack(OpStack);
    int r = 0;

    if (stack._1 <= stack._2)
        r = v;

    return r;
}

SF_INLINE
int VM::exec_ifngt_td(SInt32 v) 
{
    TSH2<2, Value::Number, Value::Number> stack(OpStack);
    int r = 0;

    if (stack._1 <= stack._2)
        r = v;

    return r;
}

SF_INLINE
int VM::exec_ifnge(SInt32 v) 
{
    SH2<2> stack(OpStack);
    // No exceptions at this point.
    Boolean3 result;
    int r = 0;

    if (AbstractLessThan(result, stack._1, stack._2))
    {
        // If the result of the comparison is not false ...
        if (result != false3)
            r = v;
    }

    return r;
}

SF_INLINE
int VM::exec_ifnge_ti(SInt32 v) 
{
    TSH2<2, SInt32, SInt32> stack(OpStack);
    int r = 0;

    if (stack._1 < stack._2)
        r = v;

    return r;
}

SF_INLINE
int VM::exec_ifnge_td(SInt32 v) 
{
    TSH2<2, Value::Number, Value::Number> stack(OpStack);
    int r = 0;

    if (stack._1 < stack._2)
        r = v;

    return r;
}

SF_INLINE
int VM::exec_jump(SInt32 v) 
{
    return v;
}

SF_INLINE
int VM::exec_iftrue(SInt32 v) 
{
    SH1<1, bool> stack(OpStack);
    int r = 0;

    if (stack.IsOK() && stack._1 == true)
        r = v;

    return r;
}

SF_INLINE
int VM::exec_iftrue_tb(SInt32 v) 
{
    TSH1<1, bool> stack(OpStack);
    int r = 0;

    if (stack._1 == true)
        r = v;

    return r;
}

SF_INLINE
int VM::exec_iffalse(SInt32 v)
{
    SH1<1, bool> stack(OpStack);
    int r = 0;

    if (stack.IsOK() && stack._1 == false)
        r = v;

    return r;
}

SF_INLINE
int VM::exec_iffalse_tb(SInt32 v)
{
    TSH1<1, bool> stack(OpStack);
    int r = 0;

    if (stack._1 == false)
        r = v;

    return r;
}

SF_INLINE
int VM::exec_ifeq(SInt32 v) 
{
    SH2<2> stack(OpStack);
    // No exceptions at this point.
    int r = 0;
    bool result;

    // Order of parameter evaluation is not important here ...
    if (AbstractEqual(result, stack._1, stack._2))
        if (result)
            r = v;

    return r;
}

SF_INLINE
int VM::exec_ifeq_ti(SInt32 v) 
{
    TSH2<2, SInt32, SInt32> stack(OpStack);
    int r = 0;

    // Order of parameter evaluation is not important here ...
    if (stack._1 == stack._2)
        r = v;

    return r;
}

SF_INLINE
int VM::exec_ifeq_td(SInt32 v) 
{
    TSH2<2, Value::Number, Value::Number> stack(OpStack);
    int r = 0;

    // Order of parameter evaluation is not important here ...
    if (stack._1 == stack._2)
        r = v;

    return r;
}

SF_INLINE
int VM::exec_ifne(SInt32 v) 
{
    SH2<2> stack(OpStack);
    // No exceptions at this point.
    int r = 0;
    bool result;

    // Order of parameter evaluation is not important here ...
    if (AbstractEqual(result, stack._1, stack._2))
        if (!result)
            r = v;

    return r;
}

SF_INLINE
int VM::exec_ifne_ti(SInt32 v) 
{
    TSH2<2, SInt32, SInt32> stack(OpStack);
    int r = 0;

    // Order of parameter evaluation is not important here ...
    if (stack._1 != stack._2)
        r = v;

    return r;
}

SF_INLINE
int VM::exec_ifne_td(SInt32 v) 
{
    TSH2<2, Value::Number, Value::Number> stack(OpStack);
    int r = 0;

    // Order of parameter evaluation is not important here ...
    if (stack._1 != stack._2)
        r = v;

    return r;
}

SF_INLINE
int VM::exec_iflt(SInt32 v)
{
    SH2<2> stack(OpStack);
    // No exceptions at this point.
    int r = 0;
    bool result;

    if (LessThan(result, stack._1, stack._2))
        if (result)
            r = v;

    return r;
}

SF_INLINE
int VM::exec_iflt_ti(SInt32 v)
{
    TSH2<2, SInt32, SInt32> stack(OpStack);
    int r = 0;

    if (stack._1 < stack._2)
        r = v;

    return r;
}

SF_INLINE
int VM::exec_iflt_td(SInt32 v)
{
    TSH2<2, Value::Number, Value::Number> stack(OpStack);
    int r = 0;

    if (stack._1 < stack._2)
        r = v;

    return r;
}

SF_INLINE
int VM::exec_ifle(SInt32 v)
{
    SH2<2> stack(OpStack);
    // No exceptions at this point.
    int r = 0;
    bool result;

    if (LessThanOrEqual(result, stack._1, stack._2))
        if (result)
            r = v;

    // RVO ...
    return r;

//          SH2<2> stack(op_stack);
//          return (LessThanOrEqual(stack._1, stack._2) ? v : 0);

// Below is an example of code in C-stile, which is almost 10% slower ...
//          const Value& _2 = op_stack.Top();
//          const Value& _1 = op_stack[op_stack.GetSize() - 2];
//          int r = 0;
//          
//          if (LessThanOrEqual(_1, _2))
//              r = v;
//          
//          op_stack.Drop(2);
//          return r;
}

SF_INLINE
int VM::exec_ifle_ti(SInt32 v)
{
    TSH2<2, SInt32, SInt32> stack(OpStack);
    int r = 0;

    if (stack._1 <= stack._2)
        r = v;

    return r;
}

SF_INLINE
int VM::exec_ifle_td(SInt32 v)
{
    TSH2<2, Value::Number, Value::Number> stack(OpStack);
    int r = 0;

    if (stack._1 <= stack._2)
        r = v;

    return r;
}

SF_INLINE
int VM::exec_ifgt(SInt32 v) 
{
    SH2<2> stack(OpStack);
    // No exceptions at this point.
    int r = 0;
    bool result;

    if (GreaterThan(result, stack._1, stack._2))
        if (result)
            r = v;

    return r;
}

SF_INLINE
int VM::exec_ifgt_ti(SInt32 v)
{
    TSH2<2, SInt32, SInt32> stack(OpStack);
    int r = 0;

    if (stack._1 > stack._2)
        r = v;

    return r;
}

SF_INLINE
int VM::exec_ifgt_td(SInt32 v)
{
    TSH2<2, Value::Number, Value::Number> stack(OpStack);
    int r = 0;

    if (stack._1 > stack._2)
        r = v;

    return r;
}

SF_INLINE
int VM::exec_ifge(SInt32 v) 
{
    SH2<2> stack(OpStack);
    // No exceptions at this point.
    int r = 0;
    bool result;

    if (GreaterThanOrEqual(result, stack._1, stack._2))
        if (result)
            r = v;

    return r;
}

SF_INLINE
int VM::exec_ifge_ti(SInt32 v)
{
    TSH2<2, SInt32, SInt32> stack(OpStack);
    int r = 0;

    if (stack._1 >= stack._2)
        r = v;

    return r;
}

SF_INLINE
int VM::exec_ifge_td(SInt32 v)
{
    TSH2<2, Value::Number, Value::Number> stack(OpStack);
    int r = 0;

    if (stack._1 >= stack._2)
        r = v;

    return r;
}

SF_INLINE
int VM::exec_ifstricteq(SInt32 v) 
{
    int r = 0;

    SF_ASSERT(OpStack.GetSize() > 1);
    // Order of parameter evaluation is not important here ...
    if (StrictEqual(OpStack.Top0(), OpStack.Top1()))
        r = v;
    OpStack.PopBack2();

    return r;
}

SF_INLINE
int VM::exec_ifstrictne(SInt32 v) 
{
    int r = 0;

    SF_ASSERT(OpStack.GetSize() > 1);
    // Order of parameter evaluation is not important here ...
    if (!StrictEqual(OpStack.Top0(), OpStack.Top1()))
        r = v;
    OpStack.PopBack2();

    return r;
}

SF_INLINE
int VM::exec_lookupswitch() 
{
    // Value on the stack must be of type integer ...
    SF_ASSERT(OpStack.GetSize() > 0);
    int result = OpStack.Top0().AsInt();
    OpStack.PopBack();

    return result;
}

AS3_INLINE
void VM::exec_pushwith() 
{
    SF_ASSERT(OpStack.GetSize() > 0);
    Value& op_value = OpStack.Top0();

    // A TypeError is thrown if scope_obj is null or undefined.
    if (op_value.IsNullOrUndefined())
    {            
        return ThrowTypeError(VM::Error(
            op_value.IsNull() ? eConvertNullToObjectError :  eConvertUndefinedToObjectError,
            *this
            ));
    }

    GetScopeStack().PushBack(Value::GetUndefined());
    Value& scope_value = GetScopeStack().Back();
    scope_value.PickUnsafe(op_value);
    OpStack.PopBack();

    // Set up the *scope* flag.
    scope_value.SetWith();
}

SF_INLINE
void VM::exec_popscope() 
{
    GetScopeStack().PopBack();
}

void VM::exec_nextname() 
{
    SH2<1, Value, SInt32> stack(OpStack);

    if (stack.IsOK())
    {
        if (stack._1.IsNamespace())
            stack._1.GetNamespace()->GetNextPropertyName(stack._1, GlobalSlotIndex(stack._2));
        else
            stack._1.GetObject()->GetNextPropertyName(stack._1, GlobalSlotIndex(stack._2));
    }
}

SF_INLINE 
void VM::exec_pushnull()
{
    OpStack.Acquire1();
    OpStack.Top0().SetNullUnsafe();
}

SF_INLINE
void VM::exec_pushundefined()
{
    OpStack.Acquire1();
    OpStack.Top0().SetUndefinedUnsafe();
}

SF_INLINE
void VM::exec_not_tb()
{
    bool& b = OpStack.Top0();
    b = !b;
}

SF_INLINE 
void VM::exec_pushbyte(UInt8 v) 
{
    // The value is promoted to an int.
    OpStack.Acquire1();
    OpStack.Top0().SetSInt32Unsafe(SInt32(SInt8(v)));
}

SF_INLINE 
void VM::exec_pushshort(SInt32 v)
{
    OpStack.Acquire1();
    OpStack.Top0().SetSInt32Unsafe(v);
}

SF_INLINE 
void VM::exec_pushtrue()
{
    OpStack.Acquire1();
    OpStack.Top0().SetBoolUnsafe(true);
}

SF_INLINE 
void VM::exec_pushfalse()
{
    OpStack.Acquire1();
    OpStack.Top0().SetBoolUnsafe(false);
}

SF_INLINE 
void VM::exec_pushnan()
{
    OpStack.Acquire1();
    OpStack.Top0().SetNumberUnsafe(NumberUtil::NaN());
}

SF_INLINE
void VM::exec_pop()
{
    SF_ASSERT(OpStack.GetSize() > 0);
    OpStack.PopBack();
}

SF_INLINE
void VM::exec_pop_nrc()
{
    SF_ASSERT(OpStack.GetSize() > 0);
    OpStack.Release0();
}

SF_INLINE
void VM::exec_dup() 
{
    OpStack.PushBack(OpStack.Top0());
}

SF_INLINE
void VM::exec_dup_nrc() 
{
    OpStack.PushBackNRC(OpStack.Top0());
}

SF_INLINE 
void VM::exec_swap() 
{
    // Optimized version ...

    OpStack.SwapTop();
}

SF_INLINE 
void VM::exec_pushstring(VMFile& file, const UPInt ind) 
{
    OpStack.Acquire1();

#ifdef SF_AS3_STORE_PRIMITIVE_VALUES_IN_BYTECODE
    SF_UNUSED1(file);
    OpStack.Top0().AssignUnsafe(ASString(reinterpret_cast<ASStringNode*>(ind)));
#else
    OpStack.Top0().AssignUnsafe(file.GetInternedString(ind));
#endif
}

SF_INLINE 
void VM::exec_pushint(SInt32 v) 
{
    OpStack.Acquire1();
    OpStack.Top0().SetSInt32Unsafe(v);
}

SF_INLINE 
void VM::exec_pushuint(UInt32 v) 
{
    OpStack.Acquire1();
    OpStack.Top0().SetUInt32Unsafe(v);
}

SF_INLINE 
void VM::exec_pushdouble(double v) 
{
    OpStack.Acquire1();
    OpStack.Top0().SetNumberUnsafe(v);
}

AS3_INLINE
void VM::exec_pushscope() 
{
    SF_ASSERT(OpStack.GetSize() > 0);
    Value& op_value = OpStack.Top0();

    // A TypeError is thrown if value is null or undefined.
    if (op_value.IsNullOrUndefined())
    {            
        return ThrowTypeError(VM::Error(
            op_value.IsNull() ? eConvertNullToObjectError :  eConvertUndefinedToObjectError,
            *this
            ));
    }

    GetScopeStack().PushBack(Value::GetUndefined());
    Value& scope_value = GetScopeStack().Back();
    scope_value.PickUnsafe(op_value);
    OpStack.PopBack();
}

SF_INLINE 
void VM::exec_pushnamespace(Instances::fl::Namespace& v) 
{
    OpStack.Acquire1();
    OpStack.Top0().AssignUnsafe(&v);
}

#if 0
SF_INLINE
void VM::exec_li8() 
{
    // Load data ... FP10 ...
    // Not documented ...
    // Not implemented yet ...
    SF_ASSERT(false);
}

SF_INLINE
void VM::exec_li16() 
{
    // Load data ... FP10 ...
    // Not documented ...
    // Not implemented yet ...
    SF_ASSERT(false);
}

SF_INLINE
void VM::exec_li32() 
{
    // Load data ... FP10 ...
    // Not documented ...
    // Not implemented yet ...
    SF_ASSERT(false);
}

SF_INLINE
void VM::exec_lf32() 
{
    // Load data ... FP10 ...
    // Not documented ...
    // Not implemented yet ...
    SF_ASSERT(false);
}
#endif

SF_INLINE
void VM::exec_lf64() 
{
    // Load data ... FP10 ...
    // Not documented ...
    // Not implemented yet ...
    SF_ASSERT(false);
}

SF_INLINE
void VM::exec_si8() 
{
    // Store data ... FP10 ...
    // Not documented ...
    // Not implemented yet ...
    SF_ASSERT(false);
}

SF_INLINE
void VM::exec_si16() 
{
    // Store data ... FP10 ...
    // Not documented ...
    // Not implemented yet ...
    SF_ASSERT(false);
}

SF_INLINE
void VM::exec_si32() 
{
    // Store data ... FP10 ...
    // Not documented ...
    // Not implemented yet ...
    SF_ASSERT(false);
}

SF_INLINE
void VM::exec_sf32() 
{
    // Store data ... FP10 ...
    // Not documented ...
    // Not implemented yet ...
    SF_ASSERT(false);
}

SF_INLINE
void VM::exec_sf64() 
{
    // Store data ... FP10 ...
    // Not documented ...
    // Not implemented yet ...
    SF_ASSERT(false);
}

SF_INLINE
void VM::exec_sxi1() 
{
    // Sign extend ... FP10 ...
    // Not documented ...
    // Not implemented yet ...
    SF_ASSERT(false);
}

SF_INLINE
void VM::exec_sxi8() 
{
    // Sign extend ... FP10 ...
    // Not documented ...
    // Not implemented yet ...
    SF_ASSERT(false);
}

SF_INLINE
void VM::exec_sxi16() 
{
    // Sign extend ... FP10 ...
    // Not documented ...
    // Not implemented yet ...
    SF_ASSERT(false);
}

AS3_INLINE
void VM::exec_newactivation(const CallFrame& cf)
{
#if 0
    OpStack.PushBack(GetClassActivation().MakeInstance(cf.GetFile(), cf.GetMethodBodyInfo() DEBUG_ARG(ASString(cf.GetName()))));
#else
    InstanceTraits::Traits& it = cf.GetFile().GetActivationInstanceTraits(cf.GetMethodBodyInd() SF_DEBUG_ARG(ASString(cf.GetName())));
    OpStack.PushBack(Pickable<Instance>(new(it.Alloc()) Instance(it)));
#endif

    // No exceptions in this method;
    // Tamarin throws exceptions in this opcode.
}

SF_INLINE 
void VM::exec_getlocal(UInt32 v) 
{
    OpStack.PushBack(GetRegister(AbsoluteIndex(v)));
}

SF_INLINE 
void VM::exec_getglobalscope() 
{
    // Gets the global scope.
    
    // DO NOT delete this code !!!
    // !!! This is definitely not correct.
    // Version 1.
    // op_stack.Add(GetScopeStack()[0]);
    
    // Version 2.
    // First object in a first frame.
    // (Doesn't seem to work)
    //OpStack.PushBack(CallStack[0].ScopeStack[0]);
    
    // Version 3.
    // It works fine with non-flash code.
    /*
    if (CallStack.GetSize() > 0)
    {
        if (CallStack[0].ScopeStack.GetSize() > 0)
        {
            OpStack.PushBack(CallStack[0].ScopeStack[0]);
            return;
        } 
        else if (GetScopeStack().GetSize() > 0)
        {
            OpStack.PushBack(GetScopeStack().At(0));
            return;
        }
    }

    // ???
    // Last chance.
    // GetGlobalObjectCPP() is correct here.
    OpStack.PushBack(GetGlobalObjectCPPValue());
    */

    // Version 4.
    // Works fine
    OpStack.PushBack(Value(GetGlobalObject()));

    // Version 5.
    //OpStack.PushBack(GetRegister(AbsoluteIndex(0)));

    // Version 6
    /*
    if (CallStack.GetSize() > 0)
    {
        const CallFrame& cf = CallStack.Back();

        if (GetScopeStack().GetSize() > cf.GetScopeStackBaseInd())
        {
            // Current scope stack is not empty.
            OpStack.PushBack(GetScopeStack().At(cf.GetScopeStackBaseInd()));
        }
    }

    // Last chance.
    OpStack.PushBack(GetGlobalObject());
    */
}

AS3_INLINE
void VM::exec_getscopeobject(UInt32 index) 
{
    OpStack.PushBack(GetScopeStack().At(index));
    // Clean up the *with* flag.
    OpStack.Top0().SetWith(false);
}

AS3_INLINE
void VM::exec_getouterscope(CallFrame& cf, UInt32 scope_index) 
{
    // Get a stored scope object.
    // This opcode is generated by VM. It is not supposed to be generated by a compiler.

    const ScopeStackType& ss = cf.GetSavedScope();
    if (scope_index >= ss.GetSize())
    {
        SF_ASSERT(false);
        // index must be less than the current depth of the scope stack.
        return ThrowReferenceError(VM::Error(VM::eParamRangeError, *this));
    }

    OpStack.PushBack(ss[scope_index]);
}

AS3_INLINE
void VM::exec_getglobalslot(UInt32 slot_index) 
{
    // Get the value of a slot on the global scope.
    
    OpStack.PushBack(Value::GetUndefined());
    GetGlobalObject()->GetSlotValueUnsafe(SlotIndex(slot_index), OpStack.Top0()).DoNotCheck();
}

AS3_INLINE
void VM::exec_setglobalslot(UInt32 slot_index) 
{
    // Set the value of a slot on the global scope.
    
    //GetRegister(AbsoluteIndex(0)).GetObject()->SetSlotValue(SlotIndex(slot_index), OpStack.Back());
    GetGlobalObject()->SetSlotValue(SlotIndex(slot_index), OpStack.Top0()).DoNotCheck();
    // We will need to pop this value even in case of an exception.
    OpStack.PopBack();
}

SF_INLINE 
void VM::exec_convert_s() 
{
    OpStack.Top0().ToStringValue(GetStringManager()).DoNotCheck();
    // Can throw exceptions.
}

AS3_INLINE
void VM::exec_esc_xelem() 
{
    // Escape an xml element.
    // This uses the ToXmlString algorithm as described in the E4X specification,
    // ECMA-357 section 10.2, to perform the conversion.
    
    SF_ASSERT(OpStack.GetSize() > 0);
    
#ifdef GFX_ENABLE_XML
    const XMLSupport& xmls = GetXMLSupport();

    if (!xmls.IsEnabled())
        return ThrowVerifyError(VM::Error(VM::eNotImplementedError, *this SF_DEBUG_ARG("exec_esc_xelem")));

    xmls.ToXMLString(*this, OpStack.Back()).DoNotCheck();
#else
    ThrowVerifyError(VM::Error(VM::eNotImplementedError, *this SF_DEBUG_ARG("exec_esc_xelem")));
#endif
}

void VM::exec_esc_xattr() 
{
    // Escape an xml attribute.
    // This uses the EscapeAttributeValue algorithm as described in the E4X
    // specification, ECMA-357 section 10.2.1.2, to perform the conversion.
    
#ifdef GFX_ENABLE_XML
    SF_ASSERT(OpStack.GetSize() > 0);
    StringManager& sm = GetStringManager();
    Value& v = OpStack.Back();
    if (v.ToStringValue(sm))
    {
        StringBuffer buf;

        Instances::fl::XML::EscapeElementValue(buf, v.AsString());
        v = sm.CreateString(buf.ToCStr(), buf.GetSize());
    }
#endif
}

SF_INLINE 
void VM::exec_convert_i() 
{
    // Extra check to satisfy postcondition checker.
    OpStack.Top0().ToInt32Value().DoNotCheck();
    // Can throw exceptions.
}

SF_INLINE 
void VM::exec_convert_u()
{
    // Extra check to satisfy postcondition checker.
    OpStack.Top0().ToUInt32Value().DoNotCheck();
    // Can throw exceptions.
}

SF_INLINE 
void VM::exec_convert_d() 
{
    // Extra check to satisfy postcondition checker.
    OpStack.Top0().ToNumberValue().DoNotCheck();
    // Can throw exceptions.
}

SF_INLINE 
void VM::exec_convert_b() 
{
    OpStack.Top0().ToBooleanValue();
    // Doesn't throw exceptions.
}

SF_INLINE 
void VM::exec_convert_o() 
{
    const Value& value = OpStack.Top0();
    
    if (value.IsNullOrUndefined())
    {
        // A TypeError is thrown if obj is null or undefined.
        //SF_ASSERT(false);
        return ThrowTypeError(VM::Error(eConvertNullToObjectError, *this));
    }
    
    /* DO NOT delete this code.
    // Tamarin doesn't seem to check for primitive data types.
    if (!value.IsObjectStrict())
    {
        // If value is an Object then nothing happens. Otherwise an exception is thrown.
        //SF_ASSERT(false);
        return ThrowError(eNotImplementedYet SF_DEBUG_ARG("Cannot convert to Object"));
    }
    */
}

void VM::exec_checkfilter() 
{
    // XML-related ...
    
    // If value is of type XML or XMLList then nothing happens. 
    // If value is of any other type a TypeError is thrown.

    const Value& value = OpStack.Top0();

    if (!IsXMLObject(value) && !IsXMLListObject(value))
        return ThrowTypeError(VM::Error(eFilterError, *this
            SF_DEBUG_ARG(GetValueTraits(value).GetName().ToCStr())
        ));
}

SF_INLINE
void VM::exec_coerce_a() 
{
    // Coerce a value to the any type.
    // Indicates to the verifier that the value on the stack is of the any type (*). Does nothing to value.
}

SF_INLINE 
void VM::exec_coerce_s() 
{
    Value& v = OpStack.Top0();

    if (v.IsNullOrUndefined())
        v.SetNull();
    else
        v.ToStringValue(GetStringManager()).DoNotCheck();
    // Can throw exceptions.
}

SF_INLINE
void VM::exec_negate() 
{
    Value& value = OpStack.Top0();

    // Conversion is necessary here.
    if (value.ToNumberValue())
    {
        Value::Number& _1 = value;
        _1 = -_1;
    }
}

SF_INLINE
void VM::exec_increment()
{
    Value& value = OpStack.Top0();

    // Conversion is necessary here.
    if (value.ToNumberValue())
    {
        Value::Number& _1 = value;
        _1 = _1 + 1;
    }
}

SF_INLINE
void VM::exec_inclocal(UInt32 v)
{
    Value& reg = GetRegister(AbsoluteIndex(v));
    Value::Number num;
    if (reg.Convert2NumberInline(num))
        reg.SetNumber(num + 1);
}

SF_INLINE
void VM::exec_decrement()
{
    Value& value = OpStack.Top0();

    // Conversion is necessary here.
    if (value.ToNumberValue())
    {
        Value::Number& _1 = value;
        _1 = _1 - 1;
    }
}

SF_INLINE
void VM::exec_declocal(UInt32 v)
{
    Value& reg = GetRegister(AbsoluteIndex(v));
    Value::Number num;
    if (reg.Convert2NumberInline(num))
        reg.SetNumber(num - 1);
}

SF_INLINE
void VM::exec_not() 
{
    Value& value = OpStack.Top0();

    // Conversion is necessary here.
    value.ToBooleanValue();
    bool& _1 = value;
    _1 = !_1;

    // No exceptions.
}

SF_INLINE
void VM::exec_bitnot()
{
    SInt32 v;
    Value& value = OpStack.Top0();

    if (value.Convert2Int32(v))
        value.SetSInt32(~v);

    // Can throw exceptions.
}

SF_INLINE
void VM::exec_add_d() 
{
    // Version of "add", which works with numbers ...
    SH2<1, Value::Number, Value::Number> stack(OpStack);

    if (stack.IsOK())
        stack._1 += stack._2;
}

SF_INLINE
void VM::exec_add() 
{
    SH2<1> stack(OpStack);
    // No exceptions at this point.
    Add(GetStringManager(), stack._1, stack._1, stack._2).DoNotCheck();
    // Add() can throw exceptions.
}

SF_INLINE
void VM::exec_subtract() 
{
    Value::Number left;
    Value::Number right = 0.0;

    SH2<1> stack(OpStack);

    if (stack._1.Convert2NumberInline(left) && stack._2.Convert2NumberInline(right))
        stack._1.SetNumber(left - right);
}

SF_INLINE
void VM::exec_multiply() 
{
    Value::Number left;
    Value::Number right = 0.0;
    SH2<1> stack(OpStack);

    if (stack._1.Convert2NumberInline(left) && stack._2.Convert2NumberInline(right))
        stack._1.SetNumber(left * right);
}

SF_INLINE
void VM::exec_divide() 
{
    SH2<1, Value::Number, Value::Number> stack(OpStack);

    if (stack.IsOK())
        stack._1 /= stack._2;
}

SF_INLINE
void VM::exec_divide_td() 
{
    TSH2<1, Value::Number, Value::Number> stack(OpStack);

    stack._1 /= stack._2;
}

SF_INLINE
void VM::exec_modulo() 
{
    SH2<1, Value::Number, Value::Number> stack(OpStack);

    if (stack.IsOK())
        stack._1 = fmod(stack._1, stack._2);
}

SF_INLINE
void VM::exec_lshift() 
{
    // (ECMA-262 section 11.7.1)
    // Documentation says that stack._1 should be SInt32
    SH2<1, SInt32, UInt32> stack(OpStack);

    if (stack.IsOK())
        stack._1 <<= (stack._2 & 0x1F);
}

SF_INLINE
void VM::exec_rshift()
{
    // The right shift is sign extended, resulting in a signed 32-bit integer. See
    // ECMA-262 section 11.7.2
    // (C++) The type of the result is the same as the type of the left operand.
    SH2<1, SInt32, UInt32> stack(OpStack);

    if (stack.IsOK())
        stack._1 = stack._1 >> (stack._2 & 0x1F);
}

SF_INLINE
void VM::exec_urshift() 
{
    // !!! May be implemented incorrectly ...

    // The right shift is sign extended, resulting in a signed 32-bit integer. See
    // ECMA-262 section 11.7.3
    // (C++) The type of the result is the same as the type of the left operand.
    SH2<1, UInt32, UInt32> stack(OpStack);

    if (stack.IsOK())
        stack._1 = stack._1 >> (stack._2 & 0x1F);
}

SF_INLINE
void VM::exec_bitand() 
{
    SH2<1, SInt32, SInt32> stack(OpStack);

    if (stack.IsOK())
        stack._1 &= stack._2;
}

SF_INLINE
void VM::exec_bitor() 
{
    SH2<1, SInt32, SInt32> stack(OpStack);

    if (stack.IsOK())
        stack._1 |= stack._2;
}

SF_INLINE
void VM::exec_bitxor()
{
    SH2<1, SInt32, SInt32> stack(OpStack);

    if (stack.IsOK())
        stack._1 ^= stack._2;
}

SF_INLINE
void VM::exec_equals() 
{
    SH2<1> stack(OpStack);
    // No exceptions at this point.
    bool result;
    if (AbstractEqual(result, stack._1, stack._2))
        stack._1.SetBool(result);
}

SF_INLINE
void VM::exec_strictequals() 
{
    SH2<1> stack(OpStack);
    // No exceptions at this point.
    stack._1.SetBool(StrictEqual(stack._1, stack._2));
}

SF_INLINE
void VM::exec_lessthan() 
{
    SH2<1> stack(OpStack);
    // No exceptions at this point.
    bool result;
    if (LessThan(result, stack._1, stack._2))
        stack._1.SetBool(result);
}

SF_INLINE
void VM::exec_lessequals() 
{
    SH2<1> stack(OpStack);
    // No exceptions at this point.
    bool result;
    if (LessThanOrEqual(result, stack._1, stack._2))
        stack._1.SetBool(result);
}

SF_INLINE
void VM::exec_greaterthan()
{
    SH2<1> stack(OpStack);
    // No exceptions at this point.
    bool result;
    if (GreaterThan(result, stack._1, stack._2))
        stack._1.SetBool(result);
}

SF_INLINE
void VM::exec_greaterequals() 
{
    SH2<1> stack(OpStack);
    // No exceptions at this point.
    bool result;
    if (GreaterThanOrEqual(result, stack._1, stack._2))
        stack._1.SetBool(result);
}

SF_INLINE
void VM::exec_istypelate() 
{
    // Check whether an Object is of a certain type.
    SH2<1> stack(OpStack);
    // No exceptions at this point.

    // stack._1 - value
    // stack._2 - type

    // A TypeError is thrown if type is not a Class.
    if (stack._2.IsClass())
    {
        const ClassTraits::Traits& type = stack._2.AsClass().GetClassTraits();

        stack._1.SetBool(IsOfType(stack._1, type));
    }
    else
        ThrowTypeError(VM::Error(eIsTypeMustBeClassError, *this));
}

SF_INLINE 
void VM::exec_getabsobject(UPInt addr)
{
    OpStack.PushBack(GetAbsObject(addr));
}

SF_INLINE
void VM::exec_increment_i()
{
    Value& value = OpStack.Top0();

    // Conversion is necessary here.
    if (value.ToInt32Value())
    {
        SInt32& _1 = value;
        ++_1;
    }
}

SF_INLINE
void VM::exec_decrement_i() 
{
    Value& value = OpStack.Top0();

    // Conversion is necessary here.
    if (value.ToInt32Value())
    {
        SInt32& _1 = value;
        --_1;
    }
}

SF_INLINE
void VM::exec_increment_ti()
{
    // No conversion is required.
    SInt32& i = OpStack.Top0();
    ++i;
}

SF_INLINE
void VM::exec_increment_tu()
{
    // No conversion is required.
    UInt32& i = OpStack.Top0();
    ++i;
}

SF_INLINE
void VM::exec_decrement_ti() 
{
    // No conversion is required.
    SInt32& i = OpStack.Top0();
    --i;
}

SF_INLINE
void VM::exec_decrement_tu() 
{
    // No conversion is required.
    UInt32& i = OpStack.Top0();
    --i;
}

SF_INLINE
void VM::exec_inclocal_i(UInt32 v)
{
    Value& value = GetRegister(AbsoluteIndex(v));

    if (value.ToInt32Value())
    {
        SInt32& _1 = value;
        ++_1;
    }
}

SF_INLINE
void VM::exec_inclocal_ti(UInt32 v)
{
    SInt32& i = GetRegister(AbsoluteIndex(v));
    ++i;
}

SF_INLINE
void VM::exec_inclocal_tu(UInt32 v)
{
    UInt32& i = GetRegister(AbsoluteIndex(v));
    ++i;
}

SF_INLINE
void VM::exec_declocal_i(UInt32 v) 
{
    Value& value = GetRegister(AbsoluteIndex(v));

    if (value.ToInt32Value())
    {
        SInt32& _1 = value;
        --_1;
    }
}

SF_INLINE
void VM::exec_declocal_ti(UInt32 v) 
{
    // No conversion is required.
    SInt32& i = GetRegister(AbsoluteIndex(v));
    --i;
}

SF_INLINE
void VM::exec_declocal_tu(UInt32 v) 
{
    // No conversion is required.
    UInt32& i = GetRegister(AbsoluteIndex(v));
    --i;
}

SF_INLINE
void VM::exec_negate_i() 
{
    Value& value = OpStack.Top0();

    // Conversion is necessary here.
    if (value.ToInt32Value())
    {
        SInt32& _1 = value;
        _1 = -_1;
    }
}

SF_INLINE
void VM::exec_negate_ti() 
{
    // No conversion is required.
    SInt32& i = OpStack.Top0();
    i = -i;
}

SF_INLINE
void VM::exec_negate_td() 
{
    // No conversion is required.
    Value::Number& n = OpStack.Top0();
    n = -n;
}

SF_INLINE
void VM::exec_add_i() 
{
    SH2<1, SInt32, SInt32> stack(OpStack);

    if (stack.IsOK())
        stack._1 += stack._2;
}

SF_INLINE
void VM::exec_add_ti() 
{
    TSH2<1, SInt32, SInt32> stack(OpStack);

    stack._1 += stack._2;
}

SF_INLINE
void VM::exec_add_td() 
{
    TSH2<1, Value::Number, Value::Number> stack(OpStack);

    stack._1 += stack._2;
}

SF_INLINE
void VM::exec_subtract_i() 
{
    SH2<1, SInt32, SInt32> stack(OpStack);

    if (stack.IsOK())
        stack._1 -= stack._2;
}

SF_INLINE
void VM::exec_subtract_ti() 
{
    TSH2<1, SInt32, SInt32> stack(OpStack);

    stack._1 -= stack._2;
}

SF_INLINE
void VM::exec_subtract_td() 
{
    TSH2<1, Value::Number, Value::Number> stack(OpStack);

    stack._1 -= stack._2;
}

SF_INLINE
void VM::exec_multiply_i() 
{
    SH2<1, SInt32, SInt32> stack(OpStack);

    if (stack.IsOK())
        stack._1 *= stack._2;
}

SF_INLINE
void VM::exec_multiply_ti() 
{
    TSH2<1, SInt32, SInt32> stack(OpStack);

    stack._1 *= stack._2;
}

SF_INLINE
void VM::exec_multiply_td() 
{
    TSH2<1, Value::Number, Value::Number> stack(OpStack);

    stack._1 *= stack._2;
}

SF_INLINE
void VM::exec_getlocal0() 
{
    OpStack.PushBack(GetRegister(AbsoluteIndex(0)));
}

SF_INLINE
void VM::exec_getlocal1()
{
    OpStack.PushBack(GetRegister(AbsoluteIndex(1)));
}   

SF_INLINE
void VM::exec_getlocal2()
{
    OpStack.PushBack(GetRegister(AbsoluteIndex(2)));
}

SF_INLINE
void VM::exec_getlocal3()
{
    OpStack.PushBack(GetRegister(AbsoluteIndex(3)));
}

SF_INLINE 
void VM::exec_setlocal(UInt32 v) 
{
    SF_ASSERT(OpStack.GetSize() > 0);
#if 0
    SwapRegister(AbsoluteIndex(v), OpStack.Top0());
    OpStack.PopBack();
#else
    SetRegisterPick(AbsoluteIndex(v), OpStack.Top0());
    OpStack.Release0();
#endif
}

SF_INLINE
void VM::exec_setlocal0() 
{
    SF_ASSERT(OpStack.GetSize() > 0);
#if 0
    SwapRegister(AbsoluteIndex(0), OpStack.Top0());
    OpStack.PopBack();
#else
    SetRegisterPick(AbsoluteIndex(0), OpStack.Top0());
    OpStack.Release0();
#endif
}

SF_INLINE
void VM::exec_setlocal1() 
{
    SF_ASSERT(OpStack.GetSize() > 0);
#if 0
    SwapRegister(AbsoluteIndex(1), OpStack.Top0());
    OpStack.PopBack();
#else
    SetRegisterPick(AbsoluteIndex(1), OpStack.Top0());
    OpStack.Release0();
#endif
}

SF_INLINE
void VM::exec_setlocal2() 
{
    SF_ASSERT(OpStack.GetSize() > 0);
#if 0
    SwapRegister(AbsoluteIndex(2), OpStack.Top0());
    OpStack.PopBack();
#else
    SetRegisterPick(AbsoluteIndex(2), OpStack.Top0());
    OpStack.Release0();
#endif
}

SF_INLINE
void VM::exec_setlocal3() 
{
    SF_ASSERT(OpStack.GetSize() > 0);
#if 0
    SwapRegister(AbsoluteIndex(3), OpStack.Top0());
    OpStack.PopBack();
#else
    SetRegisterPick(AbsoluteIndex(3), OpStack.Top0());
    OpStack.Release0();
#endif
}

#ifdef ENABLE_STRICT_SETSLOT
SF_INLINE
void VM::exec_setslot_str(UInt32 offset)
{
    // Read arguments ...
    ASStringNode* v = OpStack.Top0().AsStringNode();
    OpStack.Release0();

    ReadObject args(*this);

    if (IsException())
        return;

    // Set slot.
    SetSlotStringNode(args.ArgObject.GetObject(), offset, v);
}

SF_INLINE
void VM::exec_setslot_num(UInt32 offset)
{
    // Read arguments ...
    const Value::Number v = OpStack.PopPrimitive<Value::Number>();
    ReadObject args(*this);

    if (IsException())
        return;

    // Set slot.
    SetSlotNumber(args.ArgObject.GetObject(), offset, v);
}

SF_INLINE
void VM::exec_setslot_uint(UInt32 offset)
{
    // Read arguments ...
    const UInt32 v = OpStack.PopPrimitive<UInt32>();
    ReadObject args(*this);

    if (IsException())
        return;

    // Set slot.
    SetSlotUInt(args.ArgObject.GetObject(), offset, v);
}

SF_INLINE
void VM::exec_setslot_sint(UInt32 offset)
{
    // Read arguments ...
    const SInt32 v = OpStack.PopPrimitive<SInt32>();
    ReadObject args(*this);

    if (IsException())
        return;

    // Set slot.
    SetSlotSInt(args.ArgObject.GetObject(), offset, v);
}

SF_INLINE
void VM::exec_setslot_bool(UInt32 offset)
{
    // Read arguments ...
    const bool v = OpStack.PopPrimitive<bool>();
    ReadObject args(*this);

    if (IsException())
        return;

    // Set slot.
    SetSlotBoolean(args.ArgObject.GetObject(), offset, v);
}

SF_INLINE
void VM::exec_setslot_value(UInt32 offset)
{
    // Read arguments ...
    ReadValueObject args(*this);

    if (IsException())
        return;

    // Set slot.
    SetSlotValue(args.ArgObject.GetObject(), offset, args.ArgValue);
}

SF_INLINE
void VM::exec_setslot_obj_as(UInt32 offset)
{
    // Read arguments ...
    ReadValueObject args(*this);

    if (IsException())
        return;

    // Set slot.
    SetSlotObjectAS(args.ArgObject.GetObject(), offset, args.ArgValue);
}

SF_INLINE
void VM::exec_setslot_obj_cpp(UInt32 offset)
{
    // Read arguments ...
    ReadValueObject args(*this);

    if (IsException())
        return;

    // Set slot.
    SetSlotObjectCpp(args.ArgObject.GetObject(), offset, args.ArgValue);
}
#endif

SF_INLINE
void VM::exec_debug() 
{
    // Not implemented yet ...
}

void VM::exec_typeof() 
{
    SH1<0> stack(OpStack);
    const Value::KindType kind = stack._1.GetKind();        
    AS3::BuiltinType type_name;

    switch(kind)
    {
    case Value::kUndefined:
        type_name = AS3Builtin_undefined;
        break;
    case Value::kBoolean:
        type_name = AS3Builtin_boolean;
        break;
    case Value::kInt:
    case Value::kUInt:
    case Value::kNumber:
        type_name = AS3Builtin_number;
        break;
    case Value::kString:
        if (stack._1.GetStringNode())
            type_name = AS3Builtin_string;
        else
            // Null is of the object type.
            type_name = AS3Builtin_object;
        break;
    case Value::kFunction:
    case Value::kThunk:
    case Value::kThunkFunction:
    case Value::kThunkClosure:
    case Value::kVTableInd:
    case Value::kVTableIndClosure:
        type_name = AS3Builtin_function;
        break;
    case Value::kNamespace:
    case Value::kObject:
        switch (GetValueTraits(stack._1).GetTraitsType())
        {
        case Traits_XML:
        case Traits_XMLList:
            type_name = AS3Builtin_xml;
            break;
        default:
            // Null is also of the object type.
            type_name = AS3Builtin_object;
            break;
        }
        break;
    default:
        type_name = AS3Builtin_unknown;
        break;
    }

    stack._1 = Value(GetStringManager().GetBuiltin(type_name));
}

AS3_INLINE
void VM::exec_hasnext() 
{
    SH2<1, Value, SInt32> stack(OpStack);

    if (stack.IsException())
        return;

    // This logic should work correctly.
    SF_ASSERT(stack._1.GetObject());
    stack._1.SetUInt32(stack._1.GetObject()->GetNextDynPropIndex(GlobalSlotIndex(stack._2)).Get());
}

AS3_INLINE
void VM::exec_nextvalue() 
{
    SH2<1, Value, SInt32> stack(OpStack);

    if (stack.IsException())
        return;

    if (stack._1.IsNamespace())
        stack._1.GetNamespace()->GetNextPropertyValue(stack._1, GlobalSlotIndex(stack._2));
    else
    {
        SF_ASSERT(stack._1.GetObject());
        Object& obj = *stack._1.GetObject();

        obj.GetNextPropertyValue(stack._1, GlobalSlotIndex(stack._2));
    }
}

void VM::exec_in() 
{
    // Determine whether an object has a named property.

    // name is converted to a String, and is looked up in obj. If no property is found, then the
    // prototype chain is searched by calling [[HasProperty]] on the prototype of obj. If the
    // property is found result is true. Otherwise result is false. Push result onto the stack.

    SH2<1> stack(OpStack);
    // No exceptions at this point.

    //stack._2 is obj ...
    //stack._1 is name_obj ...

    // name is converted to a String ...
    //     if (!stack._1.ToStringValue(GetStringManager()))
    //         return;

    // This check is necessary.
    if (stack._2.IsNullOrUndefined())
        return ThrowTypeError(VM::Error(eConvertNullToObjectError, *this));

    Multiname prop_name(GetPublicNamespace(), stack._1);
    if (stack._2.IsObject())
    {
        Object* obj = stack._2.GetObject();
        SF_ASSERT(obj);
        stack._1.SetBool(obj->HasProperty(prop_name, true));
        return;
    }
    else
    {
        // Find property ...
        PropRef prop;

        FindObjProperty(
            prop,
            *this,
            stack._2, 
            prop_name
            );

        stack._1.SetBool(prop.IsFound());
    }
}

AS3_INLINE
void VM::exec_getabsslot(UPInt slot_ind)
{
    // Get the value of a slot.
    // slot_ind is ONE based.
    // slot_ind - 1 is an index in a slot table.

    // Read arguments ...
    ReadObjectRef args(*this);

    if (IsException())
        return;

    const Value _this(args.ArgObject, PickValue);

#if 1
    // Tracer shouldn't emit getabsslot for primitive types.
    SF_ASSERT(_this.IsObject());
    SF_ASSERT(_this.GetObject());
    Object& obj = *_this.GetObject();
    const Traits& tr = obj.GetTraits();

    tr.GetSlotValueUnsafe(args.ArgObject, AbsoluteIndex(slot_ind - 1), &obj).DoNotCheck();
#else
    const Traits& tr = GetValueTraits(_this);

    tr.GetSlotValueUnsafe(args.ArgObject, AbsoluteIndex(slot_ind - 1)).DoNotCheck();
#endif
}

///////////////////////////////////////////////////////////////////////////
SF_INLINE 
bool VM::ProcessException(Abc::TOpCodeVTP& cp, CallFrame& cf, State& state)
{
    if (IsException())
    {
        const Abc::TOpCode::ValueType* code = cf.GetCode().GetDataPtr();
        Abc::TCodeOffset offset = cp - code;
        int position = OnException(offset, cf);

        if (position < 0)
        {
            // Exception handling code is not in this frame ...
            state = sReturn;
            return true;
        }

        cp = code + position;
    }

    return false;
}

SF_INLINE 
bool VM::NeedToStepInto(UPInt call_stack_size, State& state)
{
    bool result = false;
    const UPInt size = CallStack.GetSize();

    // Check if we have a new call_frame to execute ...
    if (call_stack_size != size)
    {
        SF_ASSERT(size == call_stack_size + 1);
        state = sStepInto;
        result = true;
    }

    return result;
}

SF_INLINE
void VM::DropCallFrame()
{
#ifdef SF_AMP_SERVER
    if (GetAdvanceStats() != NULL)
    {
#ifdef SF_AS3_ENABLE_CALLFRAME_CACHE
        CallFrame& frame = *CallStack.Back();
#else
        CallFrame& frame = CallStack.Back();
#endif
        UInt64 timeDiff = 0;
        if (AmpServer::GetInstance().IsProfiling() && AmpServer::GetInstance().GetProfileLevel() >= Amp_Profile_Level_ActionScript)
        {
            timeDiff = Timer::GetProfileTicks() - frame.GetStartTicks();
        }
        UInt32 fileId = frame.GetFile().GetAbcFile().GetFileHandle() + (frame.GetMethodBodyInfo().GetMethodInfoInd().Get() << 16);
        GetAdvanceStats()->PopCallstack(fileId, frame.GetFile().GetAbcFile().GetSwfFileOffset(), timeDiff);
    }
#endif

#ifdef SF_AS3_ENABLE_CALLFRAME_CACHE
    CallFrame* cf = CallStack.Back();

    // Restore stack in case of exception.
    if (IsException())
        cf->ClearOpStack();

    CFCache.DestroyCallFrame(cf);
    CallStack.PopBack();
#else
    // Restore stack in case of exception or multiple values on stack.
    CallStack.Back().ClearOpStack();

    CallStack.PopBack();
#endif
}

///////////////////////////////////////////////////////////////////////////
// By default execute only one top stack frame (including all function calls).
int VM::ExecuteCode(unsigned max_stack_depth)
{
    using namespace Abc;

#ifdef SF_FPE_ENABLE
    FPEGuard fpe_guard;
#endif
#ifdef GFX_AS3_TRACE
    FlashUI& ui = GetUI();
#endif

    while (CallStack.GetSize() != 0)
    {
        State state = sExecute;
        const UPInt call_stack_size = CallStack.GetSize();
        // !!! call_frame may be invalidated ...
#ifdef SF_AS3_ENABLE_CALLFRAME_CACHE
        CallFrame& call_frame = *CallStack.Back();
#else
        CallFrame& call_frame = CallStack.Back();
#endif
        // This prefetch doesn't seem to make any difference on Windows.
        // SIMD::IS::PrefetchObj(&call_frame);
        VMAbcFile& file = call_frame.GetFile();
        const Abc::ConstPool& constp = file.GetConstPool();
        const Abc::TOpCode::ValueType* curr_cp = NULL;
        
        // We may already have an exception.
        // Let us store it because call_frame.GetCode() may throw it's own exception.
        const bool tmpHandleException = IsException();
        const Value tmpExceptionValue = GetExceptionValue();
        HandleException = false;
        ExceptionObj.Clean();

        // call_frame.GetCP() can throw exceptions.
        const TOpCode::ValueType* cp = call_frame.GetCP();
        SIMD::IS::Prefetch(cp, 0);

#ifdef GFX_AS3_TRACE
        const TOpCode::ValueType* dp = call_frame.GetCode().GetDataPtr();
#endif

        if (IsException())
        {
            state = sReturn;
            goto call_stack_label;
        }

        // Restore exception.
        if (tmpHandleException)
        {
            HandleException = tmpHandleException;
            ExceptionObj = tmpExceptionValue;
        }

        // Exceptions ...
        if (ProcessException(cp, call_frame, state))
            goto call_stack_label;

        // Regular opcodes ...              
        do
        {
            curr_cp = cp; // We need this line for a debugger for the time being ...
            
            const Code::OpCode opcode = static_cast<Code::OpCode>(Read8(cp));

#ifdef GFX_AS3_TRACE
            // Removing the IF statement below makes code ~20% faster with MSVC ...     
            if (ui.NeedToCheckOpCode())
            {
                Abc::TCodeOffset curr_offset = curr_cp - dp;

                if (ui.OnOpCode(curr_offset))
                {
                    call_frame.SetCP(curr_cp);
                    return max_stack_depth;
                }
            }
            
            // Exception may happen during execution of GetUI().OnOpCode() because it calls toString().
            // Lets not go into opcode loop with a thrown exception.
            if (ProcessException(cp, call_frame, state))
                goto call_stack_label;
#endif
                
            // Below this point call_frame may be invalidated ...
            switch (opcode)
            {
            case Code::op_nop:
                exec_nop();
                break;
            case Code::op_throw:
                {
                    // Probably, we should use *curr_offset* here ...
                    int position = exec_throw(cp, call_frame);
                    if (position < 0)
                    {
                        // Exception handling code is not in this frame ...
                        state = sReturn;
                        goto call_stack_label;
                    } else
                    {
                        const Abc::TOpCode::ValueType* code = call_frame.GetCode().GetDataPtr();
                        cp = code + position;
                    }
                }
                break;
            case Code::op_getsuper:
                exec_getsuper(file, call_frame.GetOriginationTraits(), constp.GetMultiname(ReadU30(cp)));
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_setsuper:
                exec_setsuper(file, call_frame.GetOriginationTraits(), constp.GetMultiname(ReadU30(cp)));
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_dxns:
                exec_dxns(call_frame, ReadU30(cp));
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_dxnslate:
                exec_dxnslate();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_kill:
                exec_kill(ReadU30(cp));
                break;
            case Code::op_label:
                break;
            case Code::op_ifnlt:
                {
                    int offset = exec_ifnlt(ReadS24(cp));

                    if (ProcessException(cp, call_frame, state))
                        goto call_stack_label;

                    cp += offset;
                }
                break;
            case Code::op_ifnlt_ti:
                {
                    int offset = exec_ifnlt_ti(ReadS24(cp));
                    cp += offset;
                }
                break;
            case Code::op_ifnlt_td:
                {
                    int offset = exec_ifnlt_td(ReadS24(cp));
                    cp += offset;
                }
                break;
            case Code::op_ifnle:
                {
                    int offset = exec_ifnle(ReadS24(cp));

                    if (ProcessException(cp, call_frame, state))
                        goto call_stack_label;

                    cp += offset;
                }
                break;
            case Code::op_ifnle_ti:
                {
                    int offset = exec_ifnle_ti(ReadS24(cp));
                    cp += offset;
                }
                break;
            case Code::op_ifnle_td:
                {
                    int offset = exec_ifnle_td(ReadS24(cp));
                    cp += offset;
                }
                break;
            case Code::op_ifngt:
                {
                    int offset = exec_ifngt(ReadS24(cp));

                    if (ProcessException(cp, call_frame, state))
                        goto call_stack_label;

                    cp += offset;
                }
                break;
            case Code::op_ifngt_ti:
                {
                    int offset = exec_ifngt_ti(ReadS24(cp));
                    cp += offset;
                }
                break;
            case Code::op_ifngt_td:
                {
                    int offset = exec_ifngt_td(ReadS24(cp));
                    cp += offset;
                }
                break;
            case Code::op_ifnge:
                {
                    int offset = exec_ifnge(ReadS24(cp));

                    if (ProcessException(cp, call_frame, state))
                        goto call_stack_label;

                    cp += offset;
                }
                break;
            case Code::op_ifnge_ti:
                {
                    int offset = exec_ifnge_ti(ReadS24(cp));
                    cp += offset;
                }
                break;
            case Code::op_ifnge_td:
                {
                    int offset = exec_ifnge_td(ReadS24(cp));
                    cp += offset;
                }
                break;
            case Code::op_jump:
                {
                    int offset = exec_jump(ReadS24(cp));
                    cp += offset;
                }
                break;
            case Code::op_iftrue:
                {
                    // No exceptions.
                    int offset = exec_iftrue(ReadS24(cp));
                    cp += offset;
                }
                break;
            case Code::op_iftrue_tb:
                {
                    int offset = exec_iftrue_tb(ReadS24(cp));
                    cp += offset;
                }
                break;
            case Code::op_iffalse:
                {
                    // No exceptions.
                    int offset = exec_iffalse(ReadS24(cp));
                    cp += offset;
                }
                break;
            case Code::op_iffalse_tb:
                {
                    int offset = exec_iffalse_tb(ReadS24(cp));
                    cp += offset;
                }
                break;
            case Code::op_ifeq:
                {
                    int offset = exec_ifeq(ReadS24(cp));

                    if (ProcessException(cp, call_frame, state))
                        goto call_stack_label;

                    cp += offset;
                }
                break;
            case Code::op_ifeq_ti:
                {
                    int offset = exec_ifeq_ti(ReadS24(cp));
                    cp += offset;
                }
                break;
            case Code::op_ifeq_td:
                {
                    int offset = exec_ifeq_td(ReadS24(cp));
                    cp += offset;
                }
                break;
            case Code::op_ifne:
                {
                    int offset = exec_ifne(ReadS24(cp));

                    if (ProcessException(cp, call_frame, state))
                        goto call_stack_label;

                    cp += offset;
                }
                break;
            case Code::op_ifne_ti:
                {
                    int offset = exec_ifne_ti(ReadS24(cp));
                    cp += offset;
                }
                break;
            case Code::op_ifne_td:
                {
                    int offset = exec_ifne_td(ReadS24(cp));
                    cp += offset;
                }
                break;
            case Code::op_iflt:
                {
                    int offset = exec_iflt(ReadS24(cp));

                    if (ProcessException(cp, call_frame, state))
                        goto call_stack_label;

                    cp += offset;
                }
                break;
            case Code::op_iflt_ti:
                {
                    int offset = exec_iflt_ti(ReadS24(cp));
                    cp += offset;
                }
                break;
            case Code::op_iflt_td:
                {
                    int offset = exec_iflt_td(ReadS24(cp));
                    cp += offset;
                }
                break;
            case Code::op_ifle:
                {
                    int offset = exec_ifle(ReadS24(cp));
                    
                    if (ProcessException(cp, call_frame, state))
                        goto call_stack_label;

                    cp += offset;
                }
                break;
            case Code::op_ifle_ti:
                {
                    int offset = exec_ifle_ti(ReadS24(cp));
                    cp += offset;
                }
                break;
            case Code::op_ifle_td:
                {
                    int offset = exec_ifle_td(ReadS24(cp));
                    cp += offset;
                }
                break;
            case Code::op_ifgt:
                {
                    int offset = exec_ifgt(ReadS24(cp));
                    
                    if (ProcessException(cp, call_frame, state))
                        goto call_stack_label;

                    cp += offset;
                }
                break;
            case Code::op_ifgt_ti:
                {
                    int offset = exec_ifgt_ti(ReadS24(cp));
                    cp += offset;
                }
                break;
            case Code::op_ifgt_td:
                {
                    int offset = exec_ifgt_td(ReadS24(cp));
                    cp += offset;
                }
                break;
            case Code::op_ifge:
                {
                    int offset = exec_ifge(ReadS24(cp));

                    if (ProcessException(cp, call_frame, state))
                        goto call_stack_label;

                    cp += offset;
                }
                break;
            case Code::op_ifge_ti:
                {
                    int offset = exec_ifge_ti(ReadS24(cp));
                    cp += offset;
                }
                break;
            case Code::op_ifge_td:
                {
                    int offset = exec_ifge_td(ReadS24(cp));
                    cp += offset;
                }
                break;
            case Code::op_callobject:
                exec_callobject(ReadU30(cp));

                if (ProcessException(cp, call_frame, state) || NeedToStepInto(call_stack_size, state))
                    goto call_stack_label;

                break;
            case Code::op_ifstricteq:
                {
                    int offset = exec_ifstricteq(ReadS24(cp));
                    cp += offset;
                }
                break;
            case Code::op_ifstrictne:
                {
                    int offset = exec_ifstrictne(ReadS24(cp));
                    cp += offset;
                }
                break;
            case Code::op_lookupswitch:
                {
                    const Abc::TOpCode::ValueType* base_location = curr_cp;
                    int default_offset = ReadS24(cp);
                    UInt32 case_count = ReadU30(cp);
                    
                    int index = exec_lookupswitch();
                    
                    if (index < 0 || static_cast<UInt32>(index) > case_count)
                        cp = base_location + default_offset;
                    else
                    {
                        cp += index * JumpOffsetCorrection<TOpCode::ValueType>();
                        int case_offset = ReadS24(cp);
                        cp = base_location + case_offset;
                    }
                }
                break;
            case Code::op_pushwith:
                exec_pushwith();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_popscope:
                exec_popscope();
                break;
            case Code::op_nextname:
                exec_nextname();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_hasnext:
                exec_hasnext();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_pushnull:
                exec_pushnull();
                break;
            case Code::op_pushundefined:
                exec_pushundefined();
                break;
            case Code::op_not_tb:
                exec_not_tb();
                break;
            case Code::op_nextvalue:
                exec_nextvalue();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_pushbyte:
                exec_pushbyte(static_cast<UInt8>(Read8(cp)));
                break;
            case Code::op_pushshort:
                exec_pushshort(ReadU30(cp));
                break;
            case Code::op_pushtrue:
                exec_pushtrue();
                break;
            case Code::op_pushfalse:
                exec_pushfalse();
                break;
            case Code::op_pushnan:
                exec_pushnan();
                break;
            case Code::op_pop:
                exec_pop();
                break;
            case Code::op_dup:
                exec_dup();
                break;
            case Code::op_swap:
                exec_swap();
                break;
            case Code::op_pushstring:
                exec_pushstring(file, ReadUPInt(cp));
                break;
            case Code::op_pushint:
#ifdef SF_AS3_STORE_PRIMITIVE_VALUES_IN_BYTECODE
                exec_pushint(ReadU30(cp));
#else
                exec_pushint(constp.GetInt(ReadU30(cp)));
#endif
                break;
            case Code::op_pushuint:
#ifdef SF_AS3_STORE_PRIMITIVE_VALUES_IN_BYTECODE
                exec_pushuint(static_cast<UInt32>(ReadU30(cp)));
#else
                exec_pushuint(constp.GetUInt(ReadU30(cp)));
#endif
                break;
            case Code::op_pushdouble:
#ifdef SF_AS3_STORE_PRIMITIVE_VALUES_IN_BYTECODE
                if (sizeof(Abc::TOpCode::ValueType) == sizeof(AS3::Value::Number))
                {
                    // This case handles 64-bit platforms and SF_NO_DOUBLE.
                    union
                    {
                        TOpCode::ValueType  I;
                        Value::Number       D;
                    } u;

                    u.I = ReadUPInt(cp);
                    exec_pushdouble(u.D);
                }
                else
                {
                    union
                    {
                        UInt32  I[2];
                        double  D;
                    } u;

                    u.I[0] = ReadU30(cp);
                    u.I[1] = ReadU30(cp);
                    exec_pushdouble(u.D);
                }
#else
                exec_pushdouble(constp.GetDouble(ReadU30(cp)));
#endif
                break;
            case Code::op_pushscope:
                exec_pushscope();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_pushnamespace:
                exec_pushnamespace(file.GetInternedNamespace(ReadU30(cp)));
                // ??? Exceptions ?
                break;
            case Code::op_hasnext2:
                {
                    UInt32 object_reg = ReadU30(cp);
                    UInt32 index_reg = ReadU30(cp);
                    exec_hasnext2(object_reg, index_reg);
                }
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
#if 0
            case Code::op_li8:
                exec_li8();
                break;
            case Code::op_li16:
                exec_li16();
                break;
            case Code::op_li32:
                exec_li32();
                break;
            case Code::op_lf32:
                exec_lf32();
                break;
#endif
            case Code::op_increment_tu:
                exec_increment_tu();
                break;
            case Code::op_decrement_tu:
                exec_decrement_tu();
                break;
            case Code::op_inclocal_tu:
                exec_inclocal_tu(ReadU30(cp));
                break;
            case Code::op_declocal_tu:
                exec_declocal_tu(ReadU30(cp));
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
                exec_newfunction(call_frame, ReadU30(cp));
                // No exceptions in exec_newfunction.
                // Tamarin throws exceptions in this opcode.
                break;
            case Code::op_call:
                exec_call(ReadU30(cp));

                if (ProcessException(cp, call_frame, state) || NeedToStepInto(call_stack_size, state))
                    goto call_stack_label;
                
                break;
            case Code::op_construct:
                exec_construct(ReadU30(cp));
                
                if (ProcessException(cp, call_frame, state) || NeedToStepInto(call_stack_size, state))
                    goto call_stack_label;
                
                break;
            case Code::op_callmethod:
                {
                    UInt32 method_index = ReadU30(cp);
                    UInt32 arg_count = ReadU30(cp);
                    exec_callmethod(method_index, arg_count);
                }
                
                if (ProcessException(cp, call_frame, state) || NeedToStepInto(call_stack_size, state))
                    goto call_stack_label;
                
                break;
            case Code::op_callsupermethod:
                {
                    UInt32 method_index = ReadU30(cp);
                    UInt32 arg_count = ReadU30(cp);
                    exec_callsupermethod(call_frame.GetOriginationTraits(), method_index, arg_count);
                }

                if (ProcessException(cp, call_frame, state) || NeedToStepInto(call_stack_size, state))
                    goto call_stack_label;

                break;
            case Code::op_callgetter:
                {
                    UInt32 method_index = ReadU30(cp);
                    UInt32 arg_count = ReadU30(cp);
                    exec_callgetter(method_index, arg_count);
                }

                if (ProcessException(cp, call_frame, state) || NeedToStepInto(call_stack_size, state))
                    goto call_stack_label;

                break;
            case Code::op_callsupergetter:
                {
                    UInt32 method_index = ReadU30(cp);
                    UInt32 arg_count = ReadU30(cp);
                    exec_callsupergetter(call_frame.GetOriginationTraits(), method_index, arg_count);
                }

                if (ProcessException(cp, call_frame, state) || NeedToStepInto(call_stack_size, state))
                    goto call_stack_label;

                break;
            case Code::op_callstatic:
                {
                    Abc::MiInd _1(ReadU30(cp));
                    UInt32 _2 = ReadU30(cp);
                    exec_callstatic(file, _1, _2);
                }
            
                if (ProcessException(cp, call_frame, state) || NeedToStepInto(call_stack_size, state))
                    goto call_stack_label;
                
                break;
            case Code::op_callsuper:
                {
                    UInt32 _1 = ReadU30(cp);
                    UInt32 _2 = ReadU30(cp);
                    exec_callsuper(file, call_frame.GetOriginationTraits(), constp.GetMultiname(_1), _2);
                }
                
                if (ProcessException(cp, call_frame, state) || NeedToStepInto(call_stack_size, state))
                    goto call_stack_label;
                
                break;
            case Code::op_callproperty:
                {
                    UInt32 _1 = ReadU30(cp);
                    UInt32 _2 = ReadU30(cp);
                    exec_callproperty(file, constp.GetMultiname(_1), _2);
                }
                
                if (ProcessException(cp, call_frame, state) || NeedToStepInto(call_stack_size, state))
                    goto call_stack_label;
                
                break;
            case Code::op_returnvoid:
                exec_returnvoid();
                state = sReturn;
                goto call_stack_label;
            case Code::op_returnvalue:
                exec_returnvalue();
                
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                
                state = sReturn;
                goto call_stack_label;
            case Code::op_constructsuper:
                {
                    const Traits& ot = call_frame.GetOriginationTraits();
                    exec_constructsuper(ot, ReadU30(cp));
                }
                
                if (ProcessException(cp, call_frame, state) || NeedToStepInto(call_stack_size, state))
                    goto call_stack_label;
                
                break;
            case Code::op_constructprop:
                {
                    UInt32 mn_ind = ReadU30(cp);
                    UInt32 arg_count = ReadU30(cp);
                    exec_constructprop(file, constp.GetMultiname(mn_ind), arg_count);
                }
                
                if (ProcessException(cp, call_frame, state) || NeedToStepInto(call_stack_size, state))
                    goto call_stack_label;
                
                break;
            case Code::op_callproplex:
                {
                    UInt32 mn_ind = ReadU30(cp);
                    UInt32 arg_count = ReadU30(cp);
                    exec_callproplex(file, constp.GetMultiname(mn_ind), arg_count);
                }
                
                if (ProcessException(cp, call_frame, state) || NeedToStepInto(call_stack_size, state))
                    goto call_stack_label;
                
                break;
            case Code::op_callsupervoid:
                {
                    UInt32 mn_ind = ReadU30(cp);
                    UInt32 _2 = ReadU30(cp);
                    exec_callsupervoid(file, call_frame.GetOriginationTraits(), constp.GetMultiname(mn_ind), _2);
                }
                
                if (ProcessException(cp, call_frame, state) || NeedToStepInto(call_stack_size, state))
                    goto call_stack_label;
                
                break;
            case Code::op_callpropvoid:
                {
                    UInt32 mn_ind = ReadU30(cp);
                    UInt32 _2 = ReadU30(cp);
                    exec_callpropvoid(file, constp.GetMultiname(mn_ind), _2);
                }
                
                if (ProcessException(cp, call_frame, state) || NeedToStepInto(call_stack_size, state))
                    goto call_stack_label;
                
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
                exec_applytype(ReadU30(cp));
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_newobject:
                exec_newobject(ReadU30(cp));
                // Tamarin throws exceptions in this opcode.
                break;
            case Code::op_newarray:
                exec_newarray(ReadU30(cp));
                // No exceptions in exec_newarray().
                // Tamarin throws exceptions in this opcode.
                break;
            case Code::op_newactivation:
                exec_newactivation(call_frame);
                // No exceptions in exec_newactivation().
                // Tamarin throws exceptions in this opcode.
                break;
            case Code::op_newclass:
                exec_newclass(file, ReadU30(cp));

                if (ProcessException(cp, call_frame, state) || NeedToStepInto(call_stack_size, state))
                    goto call_stack_label;
                
                break;
            case Code::op_getdescendants:
                exec_getdescendants(file, constp.GetMultiname(ReadU30(cp)));
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_newcatch:
                exec_newcatch(file, call_frame.GetException().Get(ReadU30(cp)));
                // Tamarin throws exceptions in this opcode.
                break;
            case Code::op_findpropstrict:
                exec_findpropstrict(file, constp.GetMultiname(ReadU30(cp)), call_frame.GetSavedScope());
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_findproperty:
                exec_findproperty(
                    file, 
                    constp.GetMultiname(ReadU30(cp)), 
                    call_frame.GetSavedScope(),
                    *call_frame.GetGlobalObject()
                    );
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_getlex:
                exec_getlex(file, constp.GetMultiname(ReadU30(cp)), call_frame.GetSavedScope());
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_setproperty:
                exec_setproperty(file, constp.GetMultiname(ReadU30(cp)));
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_getlocal:
                exec_getlocal(ReadU30(cp));
                break;
            case Code::op_setlocal:
                exec_setlocal(ReadU30(cp));
                break;
            case Code::op_getglobalscope:
                exec_getglobalscope();
                break;
            case Code::op_getscopeobject:
                {
                    // Get a scope object.
                    const int scope_index = ReadU30(cp);
                    const UPInt index = call_frame.GetScopeStackBaseInd() + scope_index;

                    SF_ASSERT (index < GetScopeStack().GetSize());
                    exec_getscopeobject(static_cast<UInt32>(index));
                }
                /* This check should be eliminated by the Verifier.
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                */
                break;
            case Code::op_getproperty:
                exec_getproperty(file, constp.GetMultiname(ReadU30(cp)));
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_getouterscope:
                exec_getouterscope(call_frame, ReadU30(cp));
                /* This check should be eliminated by the Verifier.
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                */
                break;
            case Code::op_initproperty:
                exec_initproperty(file, constp.GetMultiname(ReadU30(cp)));
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_pop_nrc:
                exec_pop_nrc();
                break;
            case Code::op_deleteproperty:
                exec_deleteproperty(file, constp.GetMultiname(ReadU30(cp)));
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_dup_nrc:
                exec_dup_nrc();
                break;
            case Code::op_getslot:
                exec_getslot(ReadU30(cp));
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_setslot:
                exec_setslot(ReadU30(cp));
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_getglobalslot:
                exec_getglobalslot(ReadU30(cp));
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_setglobalslot:
                exec_setglobalslot(ReadU30(cp));
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_convert_s:
                exec_convert_s();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_esc_xelem:
                exec_esc_xelem();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_esc_xattr:
                exec_esc_xattr();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_convert_i:
                exec_convert_i();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_convert_u:
                exec_convert_u();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_convert_d:
                exec_convert_d();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_convert_b:
                exec_convert_b();
                // Doesn't throw exceptions.
                break;
            case Code::op_convert_o:
                exec_convert_o();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_checkfilter:
                exec_checkfilter();
                // ??? Exceptions?
                break;
            case Code::op_coerce:
                exec_coerce(file, constp.GetMultiname(ReadU30(cp)));
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_coerce_a:
                exec_coerce_a();
                break;
            case Code::op_coerce_s:
                exec_coerce_s();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_astype:
                exec_astype(file, constp.GetMultiname(ReadU30(cp)));
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_astypelate:
                exec_astypelate();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_negate:
                exec_negate();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_increment:
                exec_increment();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_inclocal:
                exec_inclocal(ReadU30(cp));
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_decrement:
                exec_decrement();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_declocal:
                exec_declocal(ReadU30(cp));
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_typeof:
                exec_typeof();
                break;
            case Code::op_not:
                exec_not();
                break;
            case Code::op_bitnot:
                exec_bitnot();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_add_d:
                exec_add_d();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_add:
                exec_add();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_subtract:
                exec_subtract();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_multiply:
                exec_multiply();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_divide:
                exec_divide();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_divide_td:
                exec_divide_td();
                // We shouldn't have exceptions here.
                break;
            case Code::op_modulo:
                exec_modulo();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_lshift:
                exec_lshift();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_rshift:
                exec_rshift();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_urshift:
                exec_urshift();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_bitand:
                exec_bitand();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_bitor:
                exec_bitor();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_bitxor:
                exec_bitxor();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_equals:
                exec_equals();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_strictequals:
                exec_strictequals();
                break;
            case Code::op_lessthan:
                exec_lessthan();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_lessequals:
                exec_lessequals();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_greaterthan:
                exec_greaterthan();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_greaterequals:
                exec_greaterequals();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_instanceof:
                exec_instanceof();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_istype:
                exec_istype(file, constp.GetMultiname(ReadU30(cp)));
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_istypelate:
                exec_istypelate();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_in:
                exec_in();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_getabsobject:
                exec_getabsobject(ReadUPInt(cp));
                break;
            case Code::op_getabsslot:
                exec_getabsslot(ReadU30(cp));
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_setabsslot:
                exec_setabsslot(ReadU30(cp));
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_initabsslot:
                exec_initabsslot(ReadU30(cp));
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_increment_i:
                exec_increment_i();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_decrement_i:
                exec_decrement_i();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_increment_ti:
                exec_increment_ti();
                break;
            case Code::op_decrement_ti:
                exec_decrement_ti();
                break;
            case Code::op_inclocal_i:
                exec_inclocal_i(ReadU30(cp));
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_inclocal_ti:
                exec_inclocal_ti(ReadU30(cp));
                break;
            case Code::op_declocal_i:
                exec_declocal_i(ReadU30(cp));
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_declocal_ti:
                exec_declocal_ti(ReadU30(cp));
                break;
            case Code::op_negate_i:
                exec_negate_i();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_negate_ti:
                exec_negate_ti();
                // We shouldn't have exceptions here.
                break;
            case Code::op_negate_td:
                exec_negate_td();
                // We shouldn't have exceptions here.
                break;
            case Code::op_add_i:
                exec_add_i();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_add_ti:
                exec_add_ti();
                // We shouldn't have exceptions here.
                break;
            case Code::op_add_td:
                exec_add_td();
                // We shouldn't have exceptions here.
                break;
            case Code::op_subtract_i:
                exec_subtract_i();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_subtract_ti:
                exec_subtract_ti();
                // We shouldn't have exceptions here.
                break;
            case Code::op_subtract_td:
                exec_subtract_td();
                // We shouldn't have exceptions here.
                break;
            case Code::op_multiply_i:
                exec_multiply_i();
                if (ProcessException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_multiply_ti:
                exec_multiply_ti();
                // We shouldn't have exceptions here.
                break;
            case Code::op_multiply_td:
                exec_multiply_td();
                // We shouldn't have exceptions here.
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
#ifdef ENABLE_STRICT_SETSLOT
            case Code::op_setslot_str:
                exec_setslot_str(ReadU30(cp));
                break;
            case Code::op_setslot_num:
                exec_setslot_num(ReadU30(cp));
                break;
            case Code::op_setslot_uint:
                exec_setslot_uint(ReadU30(cp));
                break;
            case Code::op_setslot_sint:
                exec_setslot_sint(ReadU30(cp));
                break;
            case Code::op_setslot_bool:
                exec_setslot_bool(ReadU30(cp));
                break;
            case Code::op_setslot_value:
                exec_setslot_value(ReadU30(cp));
                break;
            case Code::op_setslot_obj_as:
                exec_setslot_obj_as(ReadU30(cp));
                break;
            case Code::op_setslot_obj_cpp:
                exec_setslot_obj_cpp(ReadU30(cp));
                break;
#endif
            case Code::op_debug:
                {
                    const UInt8 debug_type = static_cast<UInt8>(Read8(cp));
                    const int name_ind = ReadU30(cp);
                    const UInt8 dreg = static_cast<UInt8>(Read8(cp));
                    const int extra = ReadU30(cp);
                    SF_UNUSED4(debug_type, name_ind, dreg, extra);
                }
                exec_debug();
                break;
            case Code::op_debugline:
                exec_debugline(call_frame, ReadU30(cp));
                break;
            case Code::op_debugfile:
                exec_debugfile(call_frame, ReadU30(cp));
                break;
            case Code::op_0xF2:
                ReadU30(cp);
                // ???
                break;
            }

            // Prefetch here will slow down the code on Windows.
            // SIMD::IS::Prefetch(cp, 0);

        } while (true);

        SF_ASSERT(!IsException());
        SF_ASSERT(CallStack.GetSize() == call_stack_size);

        call_stack_label:
        
        SF_AMP_CODE(SetActiveLine(0);)
        SF_AMP_CODE(SetActiveFile(0);)

            // We shouldn't get here in the *Execute* state.
        SF_ASSERT(state != sExecute);
                    
        if (state == sStepInto)
        {
            // Store current offset in a right frame ...
#ifdef SF_AS3_ENABLE_CALLFRAME_CACHE
            CallStack[call_stack_size - 1]->SetCP(cp);
#else
            CallFrame& cf = CallStack[call_stack_size - 1];
            cf.SetCP(cp);
#endif

            SF_ASSERT(CallStack.GetSize() == call_stack_size + 1);
            ++max_stack_depth;
        } else
        {
            // Finally drop the call frame ...
            DropCallFrame();

            // call_stack may be not empty at this point.
            if (--max_stack_depth == 0)
                break;
        }
    }
    
    return max_stack_depth;
}

}}} // namespace Scaleform { namespace GFx { namespace AS3 {

