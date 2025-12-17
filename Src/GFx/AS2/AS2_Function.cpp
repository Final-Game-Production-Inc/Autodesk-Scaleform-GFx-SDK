/**************************************************************************

Filename    :   AS2_Function.cpp
Content     :   Implementation of C and AS function objects
Created     :   
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/GFx_PlayerImpl.h"
#include "GFx/AS2/AS2_Action.h"
#include "AS2_FunctionRefImpl.h"
#include "GFx/AS2/AS2_ArrayObject.h"
#include "GFx/GFx_ASString.h"
#include "GFx/AS2/AS2_StringObject.h"
#include "AS2_AsFunctionObject.h"
#include "GFx/AS2/AS2_AvmCharacter.h"

#include <stdio.h>
#include <stdlib.h>

//#define SF_TRACK_STACK_USAGE_PER_AS_FUNCTION_CALL
namespace Scaleform { namespace GFx { namespace AS2 {

//
////////// FunctionObject ///////////
//
#ifndef GFX_AS_ENABLE_GC
//////////////////////////////////////////////////////////////////////////
// non-CC versions
//
FunctionObject::FunctionObject (ASStringContext* psc) 
: Object(psc)
{
    SF_UNUSED(psc);
    // NOTE: psc can be 0 here to allow for default constructor; if we need 
    // a valid 'psc' here in the future we need to remove the default constructor.
    pPrototype = NULL;
}

FunctionObject::FunctionObject (Environment* penv) 
: Object(penv)
{
    SF_UNUSED(penv);
    pPrototype = NULL;
}

FunctionObject::~FunctionObject ()
{
    // Function object is being removed; make sure we cleanup
    // the prototype.constructor property to avoid crash.
    if (pPrototype)
    {
        pPrototype->CheckAndResetCtorRef(this);
    }
}

bool FunctionObject::SetMember(Environment *penv, 
                                  const ASString& name, 
                                  const Value& val, 
                                  const PropFlags& flags)
{
    if (name == penv->GetBuiltin(ASBuiltin_prototype))
    {   
        // Prototype is being set via ActionScript.
        // We need
        Object* pobj = val.ToObject(penv);
        if (pobj && pobj->IsBuiltinPrototype())
            pPrototype = static_cast<ObjectProto*>(pobj);
        else
            pPrototype = NULL;
    }
    return Object::SetMember(penv, name, val, flags);
}

void FunctionObject::SetPrototype(ASStringContext* psc, Object* pprototype)
{
    pPrototype = pprototype;
    SetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_prototype), Value(pprototype));
}
#else
////////////////////////////////////////////////
// CC versions
//
FunctionObject::FunctionObject (ASStringContext* psc) 
: Object(psc)
{
    SF_UNUSED(psc);
    // NOTE: psc can be 0 here to allow for default constructor; if we need 
    // a valid 'psc' here in the future we need to remove the default constructor.
}

FunctionObject::FunctionObject (Environment* penv) 
: Object(penv)
{
    SF_UNUSED(penv);
}

FunctionObject::~FunctionObject ()
{
}

void FunctionObject::SetPrototype(ASStringContext* psc, Object* pprototype)
{
    SetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_prototype), Value(pprototype));
}
#endif // SF_NO_GC

void FunctionObject::SetProtoAndCtor(ASStringContext* psc, Object* pprototype)
{
    Set__proto__(psc, pprototype);
    
    // function objects have "constructor" property as well
    FunctionRef ctor = pprototype->Get_constructor(psc);
    if (!ctor.IsNull())
        Set_constructor(psc, ctor);
}

FunctionRef  FunctionObject::ToFunction()
{
    return FunctionRef(this);
}

Object* FunctionObject::GetPrototype(ASStringContext* psc)
{
    Value prototypeVal;
    if (GetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_prototype), &prototypeVal))
    {
        return prototypeVal.ToObject(NULL);
    }
    return NULL;
}

//////////////////////////////////////////////////////////////////////////
// CFunctionObject

CFunctionObject::CFunctionObject (ASStringContext* psc) 
: FunctionObject(psc), pFunction(NULL)
{
    SF_UNUSED(psc);
    // NOTE: psc can be 0 here to allow for default constructor; if we need 
    // a valid 'psc' here in the future we need to remove the default constructor.
}

CFunctionObject::CFunctionObject (Environment* penv) 
: FunctionObject(penv), pFunction(NULL) 
{
    SF_UNUSED(penv);
}

//CFunctionObject::CFunctionObject (Environment* penv, GASCFunctionDef* func) 
//: Def (func)
//{
//    SF_UNUSED(penv);
//}

// this constructor is used for C functions
CFunctionObject::CFunctionObject(ASStringContext* psc, Object* pprototype, CFunctionPtr func)
: FunctionObject(psc), pFunction(func)
{
    // C-function objects have __proto__ = Object.prototype
    //!AB, correction: C-function objects have __proto__ = Function.prototype (see FunctionObject::GetMember)
    //Set__proto__ (pprototype);
    //SF_UNUSED(pprototype); // not sure yet, is it necessary or not at all... (AB)
    Set__proto__(psc, pprototype);
}

CFunctionObject::CFunctionObject(ASStringContext* psc, CFunctionPtr func)
: FunctionObject(psc), pFunction(func)
{
    Set__proto__(psc, psc->pContext->GetPrototype(ASBuiltin_Function));
}

CFunctionObject::~CFunctionObject ()
{
}

void CFunctionObject::Invoke 
(const FnCall& fn, LocalFrame*, const char*) 
{
    if (0 != pFunction) {
        ObjectInterface* pthis   = fn.ThisPtr;
        if (pthis && pthis->IsSuper())
        {
            ObjectInterface* prealThis = static_cast<SuperObject*>(pthis)->GetRealThis();
            FnCall fn2(fn.Result, prealThis, fn.Env, fn.NArgs, fn.FirstArgBottomIndex);
            (pFunction)(fn2);
            static_cast<SuperObject*>(pthis)->ResetAltProto(); // reset alternative proto, if set
        }
        else
            (pFunction)(fn);
    }
    //Def.Invoke (fn, localFrame, NULL, pmethodName);
}

Environment* CFunctionObject::GetEnvironment(const FnCall& fn, Ptr<InteractiveObject>*)
{
    return fn.Env;
}


bool CFunctionObject::IsEqual(const FunctionObject& f) const
{
    return f.IsCFunction() && pFunction == static_cast<const CFunctionObject&>(f).pFunction;
}



//////////////////////////////////////////////////////////////////////////
// AsFunctionObject

AsFunctionObject::AsFunctionObject(Environment* penv, ActionBuffer* ab, 
                                         unsigned start, unsigned length,
                                         const WithStackArray* pwithStack,
                                         ActionBuffer::ExecuteType execType)
:   FunctionObject(penv), 
    pMovieRoot(0),
    pActionBuffer(ab),    
    StartPc(start),
    Length(length),
    // Need to pass default value for copy-constructing array
    Args(ArgSpec(0, penv->GetBuiltin(ASBuiltin_empty_))),
    Function2Flags(0),
    ExecType((UByte)execType),
    LocalRegisterCount(0)
{
    SF_ASSERT(pActionBuffer);

    if (pwithStack)
        WithStack = *pwithStack;

    if (penv && (execType!= ActionBuffer::Exec_Event && execType!= ActionBuffer::Exec_SpecialEvent))
    {
        InteractiveObject* ch = penv->GetTarget();
        SF_ASSERT (ch != 0);
        TargetHandle = ch->GetCharacterHandle();

        // MA: MovieRoot Lives in diferent heap now!
        pMovieRoot = ch->GetMovieImpl();
    }
}

Object* AsFunctionObject::CreateNewObject(Environment* penv) const
{
    return SF_HEAP_NEW(penv->GetHeap()) Object(penv);
}

InteractiveObject* AsFunctionObject::GetTargetCharacter()
{    
    if (!pMovieRoot) return 0;
    DisplayObject* dob = TargetHandle->ResolveCharacter(pMovieRoot);
    return (dob) ? dob->CharToInteractiveObject() : NULL;
}

Environment* AsFunctionObject::GetEnvironment(const FnCall& fn, Ptr<InteractiveObject>* ptargetCh)
{
    Ptr<InteractiveObject> ch = GetTargetCharacter();
    Environment*     pourEnv = 0;
    if (ch)
    {
        pourEnv = GFx::AS2::ToAvmCharacter(ch)->GetASEnvironment();
    }
    if (ptargetCh)
        *ptargetCh = ch;

    SF_ASSERT(pourEnv || fn.Env);

    pourEnv = pourEnv ? pourEnv : fn.Env; // the final decision about environment...
    return pourEnv;
}

#ifdef SF_TRACK_STACK_USAGE_PER_AS_FUNCTION_CALL
volatile UInt32 __esp__ = 0, curESP = 0;
#endif

class InvokeContext
{
public:
    AsFunctionObject*       pThis;
    Environment*            pOurEnv;
    const FnCall&           mFnCall;
    LocalFrame*             pLocalFrame; 
    const char*             pMethodName;
    Ptr<InteractiveObject>  TargetCh; 
    Ptr<InteractiveObject>  FnEnvCh;
    Ptr<InteractiveObject>  PassedThisCh;
    Ptr<Object>             PassedThisObj;
    Ptr<LocalFrame>         CurLocalFrame;
    int                     LocalStackTop;

    InvokeContext(AsFunctionObject* pthis, const FnCall& fn, LocalFrame* localFrame, const char* pmethodName)
        : pThis(pthis), mFnCall(fn), pLocalFrame(localFrame), pMethodName(pmethodName)
    {
    }
    void Setup();
    void Cleanup();

private:
    InvokeContext& operator=(const InvokeContext&) { return *this; }
};

void InvokeContext::Setup() 
{
    pOurEnv->CallPush(pThis);

    // 'this' can be different from currently accessible environment scope.
    // In most 'this' is passed through FnCall, while the implicit scope
    // 'pEnv' is the scope of the original function declaration.

    ObjectInterface* pthis   = mFnCall.ThisPtr;
    //SF_ASSERT(pthis); // pthis can be null (i.e. undefined), for example inside interval's handler

    MemoryHeap* pheap = pOurEnv->GetHeap();

    // Set up local stack frame, for parameters and locals.
    LocalStackTop = pOurEnv->GetLocalFrameTop();

    if (pThis->IsFunction2() || (pThis->GetExecType() == ActionBuffer::Exec_Function))
    {
        CurLocalFrame = pOurEnv->CreateNewLocalFrame();
        CurLocalFrame->PrevFrame = pLocalFrame;
    }
    else
        pOurEnv->AddFrameBarrier();

    ObjectInterface* passedThis = pthis;
    // need to hold passedThis to avoid its release during the function execution
    if (passedThis)
    {
        PassedThisCh  = passedThis->ToCharacter();
        PassedThisObj = passedThis->ToASObject();
    }

    if (pthis && pthis->IsSuper())
    {
        pthis = static_cast<SuperObject*>(pthis)->GetRealThis();
    }

    if (!pThis->IsFunction2())
    {
        int i;
        int version = pOurEnv->GetVersion();

        // AB, Flash 6 and below uses function 1 for methods and events too.
        // So, we need to care about setting correct "this".
        if (pthis)
        {
            Value thisVal;
            thisVal.SetAsObjectInterface(pthis);
            pOurEnv->AddLocal(pOurEnv->GetBuiltin(ASBuiltin_this), thisVal);
        }

        // save some info for recreation of "super". For function1 "super" is created
        // on demand.
        if (version >= 6 && CurLocalFrame)
        {
            CurLocalFrame->SuperThis = passedThis;
        }
        // Conventional function.

        if (CurLocalFrame && mFnCall.Env && version >= 5)
        {
            // Optimization: function, type 1 may use "arguments" array and there is no
            // way to predict this usage. To avoid creation of this array for every call
            // (especially, the "arguments" array is used pretty rarely), we just
            // save some info, required for "lazy" creation of this array in the
            // Environment::GetVariableRaw method.
            if (pOurEnv != mFnCall.Env)
                FnEnvCh = mFnCall.Env->GetTarget(); // keep target char for FnCall.Env to ensure 
            // it won't be freed before call is completed.
            CurLocalFrame->Env                  = mFnCall.Env;
            CurLocalFrame->NArgs                = mFnCall.NArgs;
            CurLocalFrame->FirstArgBottomIndex  = mFnCall.FirstArgBottomIndex;
            CurLocalFrame->Callee               = pOurEnv->CallTop(0);
            CurLocalFrame->Caller               = pOurEnv->CallTop(1);
        }

        // Push the arguments onto the local frame.
        int ArgsToPass = Alg::Min<int>(mFnCall.NArgs, (int)pThis->Args.GetSize());
        for (i = 0; i < ArgsToPass; i++)
        {
            SF_ASSERT(pThis->Args[i].Register == 0);
            pOurEnv->AddLocal(pThis->Args[i].Name, mFnCall.Arg(i));
        }
        // need to fill remaining arguments by "undefined" values
        for (int n = (int)pThis->Args.GetSize(); i < n; i++)
        {
            SF_ASSERT(pThis->Args[i].Register == 0);
            pOurEnv->AddLocal(pThis->Args[i].Name, Value());
        }
    }
    else
    {
        // function2: most args go in registers; any others get pushed.

        // Create local registers.
        pOurEnv->AddLocalRegisters(pThis->LocalRegisterCount);

        // Handle the explicit args.
        int ArgsToPass = Alg::Min<int>(mFnCall.NArgs, (int)pThis->Args.GetSize());
        int i;
        for (i = 0; i < ArgsToPass; i++)
        {
            if (pThis->Args[i].Register == 0)
            {
                // Conventional arg passing: create a local var.
                pOurEnv->AddLocal(pThis->Args[i].Name, mFnCall.Arg(i));
            }
            else
            {
                // Pass argument into a register.
                int reg = pThis->Args[i].Register;
                *(pOurEnv->LocalRegisterPtr(reg)) = mFnCall.Arg(i);
            }
        }
        // need to fill remaining aguments by "undefined" values
        for (int n = (int)pThis->Args.GetSize(); i < n; i++)
        {
            if (pThis->Args[i].Register == 0)
            {
                // Conventional arg passing: create a local var.
                pOurEnv->AddLocal(pThis->Args[i].Name, Value());
            }
        }

        Ptr<SuperObject> superObj;
        if ((pThis->Function2Flags & 0x10) || !(pThis->Function2Flags & 0x20))
        {
            // need to create super
            SF_ASSERT(passedThis);

            Ptr<Object> proto = passedThis->Get__proto__();
            //printf ("!!! passedThis.__proto__ = %s\n", (const char*)pOurEnv->GetGC()->FindClassName(pOurEnv->GetSC(), proto).ToCStr());
            //printf ("!!! real pthis.__proto__ = %s\n", (const char*)pOurEnv->GetGC()->FindClassName(pOurEnv->GetSC(), pthis->Get__proto__()).ToCStr());
            if (proto)
            {
                if (pMethodName)
                {
                    // Correct the prototype according to the owner of the function.
                    // This is important in the case if "this"'s class do not have
                    // the method being invoked and only the base class has it.
                    Ptr<Object> newProto = proto->FindOwner(pOurEnv->GetSC(), pOurEnv->CreateString(pMethodName));
                    if (newProto)
                    {
                        proto = newProto;
                        //printf ("!!! newproto = %s\n", (const char*)pOurEnv->GetGC()->FindClassName(pOurEnv->GetSC(), newProto).ToCStr());
                    }
                }
                FunctionRef __ctor__ = proto->Get__constructor__(pOurEnv->GetSC());
                //printf ("!!! __proto__.__ctor__ = %s\n", (const char*)pOurEnv->GetGC()->FindClassName(pOurEnv->GetSC(), __ctor__.GetObjectPtr()).ToCStr());
                //printf ("!!! __proto__.__proto__ = %s\n", (const char*)pOurEnv->GetGC()->FindClassName(pOurEnv->GetSC(), proto->Get__proto__()).ToCStr());
                superObj = *SF_HEAP_NEW(pheap) SuperObject(proto->Get__proto__(), pthis, __ctor__);
            }
        }

        // Handle the implicit args.
        int CurrentReg = 1;
        if (pThis->Function2Flags & 0x01)
        {
            // preload 'this' into a register.
            if (pthis)
                (*(pOurEnv->LocalRegisterPtr(CurrentReg))).SetAsObjectInterface(pthis);
            else
                (*(pOurEnv->LocalRegisterPtr(CurrentReg))).SetUndefined();
            CurrentReg++;
        }

        if (pThis->Function2Flags & 0x02)
        {
            // Don't put 'this' into a local var.
        }
        else
        {
            // Put 'this' in a local var.
            Value pthisVal;
            if (pthis) pthisVal.SetAsObjectInterface(pthis);
            pOurEnv->AddLocal(pOurEnv->GetBuiltin(ASBuiltin_this), pthisVal);
        }

        // Init arguments Array, if it's going to be needed.
        Ptr<ArrayObject>    pargArray;
        if ((pThis->Function2Flags & 0x04) || ! (pThis->Function2Flags & 0x08))
        {
            pargArray = *SF_HEAP_NEW(pheap) ArrayObject(pOurEnv);
            pargArray->Resize(mFnCall.NArgs);
            for (int ipa = 0; ipa < mFnCall.NArgs; ipa++)
                pargArray->SetElement(ipa, mFnCall.Arg(ipa));
        }

        if (pThis->Function2Flags & 0x04)
        {
            // preload 'arguments' into a register.
            (*(pOurEnv->LocalRegisterPtr(CurrentReg))).SetAsObject(pargArray.GetPtr());
            CurrentReg++;
        }

        if (pThis->Function2Flags & 0x08)
        {
            // Don't put 'arguments' in a local var.
        }
        else
        {
            // Put 'arguments' in a local var.
            pOurEnv->AddLocal(pOurEnv->GetBuiltin(ASBuiltin_arguments), Value(pargArray.GetPtr()));
            pargArray->SetMemberRaw(pOurEnv->GetSC(), pOurEnv->GetBuiltin(ASBuiltin_callee), pOurEnv->CallTop(0), 
                PropFlags::PropFlag_DontEnum | PropFlags::PropFlag_DontDelete | PropFlags::PropFlag_ReadOnly);
            pargArray->SetMemberRaw(pOurEnv->GetSC(), pOurEnv->GetBuiltin(ASBuiltin_caller), pOurEnv->CallTop(1), 
                PropFlags::PropFlag_DontEnum | PropFlags::PropFlag_DontDelete | PropFlags::PropFlag_ReadOnly);
        }

        if (pThis->Function2Flags & 0x10)
        {
            // Put 'super' in a register.
            //SF_ASSERT(superObj);

            (*(pOurEnv->LocalRegisterPtr(CurrentReg))).SetAsObject(superObj);
            //(*(pOurEnv->LocalRegisterPtr(CurrentReg))).SetAsFunction(superObj);
            CurrentReg++;
        }

        if (pThis->Function2Flags & 0x20)
        {
            // Don't put 'super' in a local var.
        }
        else
        {
            // Put 'super' in a local var.
            //SF_ASSERT(superObj);

            Value superVal;
            superVal.SetAsObject(superObj);
            pOurEnv->AddLocal(pOurEnv->GetBuiltin(ASBuiltin_super), superVal);
        }

        if (pThis->Function2Flags & 0x40)
        {
            // Put '_root' in a register.
            (*(pOurEnv->LocalRegisterPtr(CurrentReg))).SetAsCharacter(
                pOurEnv->GetTarget()->GetTopParent());
            CurrentReg++;
        }

        if (pThis->Function2Flags & 0x80)
        {
            // Put 'Parent' in a register.
            Value    parent; 
            pOurEnv->GetVariable(pOurEnv->GetBuiltin(ASBuiltin__parent), &parent);
            (*(pOurEnv->LocalRegisterPtr(CurrentReg))) = parent;
            CurrentReg++;
        }

        if (pThis->Function2Flags & 0x100)
        {
            // Put 'pGlobal' in a register.
            (*(pOurEnv->LocalRegisterPtr(CurrentReg))).SetAsObject(pOurEnv->GetGC()->pGlobal);
            CurrentReg++;
        }
    }
}

void InvokeContext::Cleanup()
{
    ObjectInterface* passedThis = PassedThisObj.GetPtr();
    if (passedThis && passedThis->IsSuper())
    {
        static_cast<SuperObject*>(passedThis)->ResetAltProto(); // reset alternative proto, if set
    }

    if (!pThis->IsFunction2() || !(pThis->Function2Flags & 0x02))
    {
        // wipe explicit "this" off from the local frame to prevent memory leak
        pOurEnv->SetLocal(pOurEnv->GetBuiltin(ASBuiltin_this), Value());
    }
    if (!pThis->IsFunction2() || !(pThis->Function2Flags & 0x20))
    {
        // wipe explicit "super" off from the local frame to prevent memory leak
        pOurEnv->SetLocal(pOurEnv->GetBuiltin(ASBuiltin_super), Value());
    }

    if (CurLocalFrame)
        CurLocalFrame->ReleaseFramesForLocalFuncs ();

    // Clean up stack frame.
    pOurEnv->SetLocalFrameTop(LocalStackTop);

    if (pThis->IsFunction2())
    {
        // Clean up the local registers.
        pOurEnv->DropLocalRegisters(pThis->LocalRegisterCount);
    }

    // finalize...
    if (pOurEnv)
        pOurEnv->CallPop();
}

void AsFunctionObject::Invoke(const FnCall& fn, LocalFrame* localFrame, const char* pmethodName) 
{
    InvokeContext ctxt(this, fn, localFrame, pmethodName);
    // push current function object into the stack.
    // need for atguments.callee/.caller.
    ctxt.pOurEnv = GetEnvironment(fn, &ctxt.TargetCh);

    if (ctxt.pOurEnv->GetTarget()->IsUnloaded())
    {
        // Do not use environment from the unloaded target, use caller environment instead.
        ctxt.pOurEnv = fn.Env;
    }
    else
    {
        // if environment (pourEnv) is not NULL then use it (and it MUST be the same as 
        // GASAsFunctionDef::GetEnvironment method returns,
        // otherwise - call to GASAsFunctionDef::GetEnvironment
        SF_ASSERT(ctxt.pOurEnv == GetEnvironment(fn, 0));
    }
    SF_ASSERT(ctxt.pOurEnv);

#ifdef SF_TRACK_STACK_USAGE_PER_AS_FUNCTION_CALL
    //volatile UInt32 curESP;
    #ifdef SF_OS_WII
    asm 
    {   
        stw rsp, curESP  
    }
    #elif defined(SF_OS_XBOX360)
    __asm stw r1, curESP
    #elif defined(SF_OS_PS3)
    volatile UInt32 curESP;
    __asm__ volatile ("stw 1, %0\n\t" : "Z=" (curESP));
    #elif defined(SF_OS_WIN32) && defined(SF_CC_MSVC)
    __asm mov curESP, esp;
    #endif
    if (__esp__ != 0)
        printf("Call size is %d\n", __esp__ - curESP);
    __esp__ = curESP;
#endif // SF_TRACK_STAC_USAGE_PER_AS_FUNCTION_CALL

    if (!ctxt.pOurEnv->RecursionGuardStart())
    {
        #ifdef GFX_AS2_VERBOSE_ERRORS
        if (ctxt.pOurEnv->IsVerboseActionErrors())
            ctxt.pOurEnv->LogScriptError(
                "Stack overflow, max level of 255 nested calls is reached.");
        #endif
        ctxt.pOurEnv->RecursionGuardEnd();
        return;
    }

    ctxt.Setup();
    
    // Execute the actions.
    pActionBuffer->Execute(ctxt.pOurEnv, StartPc, Length, fn.Result, &WithStack, GetExecType());

    ctxt.Cleanup();

    ctxt.pOurEnv->RecursionGuardEnd();
}

//
//////////// FunctionRefBase //////////////////
//
void FunctionRefBase::Assign(const FunctionRefBase& orig)
{
    if (this != &orig)
    {
        FunctionObject* pprevFunc = Function;
        if (!(Flags & FuncRef_Weak) && Function && (Function != orig.Function))
            Function->Release ();

        Function = orig.Function;

        if (!(Flags & FuncRef_Weak) && orig.Function && (pprevFunc != orig.Function))
            orig.Function->AddRef ();

        if (orig.pLocalFrame != 0) 
            SetLocalFrame (orig.pLocalFrame, (orig.Flags & FuncRef_Internal));
        else
            SetLocalFrame (0, 0);
    }
}

void FunctionRefBase::SetInternal (bool internal)
{
#ifndef GFX_AS_ENABLE_GC
    if (LocalFrame != 0 && bool(Flags & FuncRef_Internal) != internal)
    {
        if (!(Flags & FuncRef_Internal))
            pLocalFrame->Release ();
        else
            pLocalFrame->AddRef ();
    }
    if (internal) Flags |= FuncRef_Internal;
    else          Flags &= (~FuncRef_Internal);
#else
    SF_UNUSED(internal);
#endif
}

void FunctionRefBase::SetLocalFrame (LocalFrame* localFrame, bool internal)
{
    if (pLocalFrame != 0 && !(Flags & FuncRef_Internal))
        pLocalFrame->Release ();
    pLocalFrame = localFrame;
    if (internal) Flags |= FuncRef_Internal;
    else          Flags &= (~FuncRef_Internal);
    if (localFrame != 0 && !(Flags & FuncRef_Internal))
        localFrame->AddRef ();
}

#ifdef GFX_AS_ENABLE_GC
template <class Functor>
void FunctionRefBase::ForEachChild_GC(Collector* prcc) const
{
    if (Function)
        Functor::Call(prcc, Function);
    if (pLocalFrame)
        Functor::Call(prcc, pLocalFrame);
}
SF_GC_PREGEN_FOR_EACH_CHILD(FunctionRefBase)

#endif // SF_NO_GC

///////////// FunctionProto //////////////////
static const NameFunction GAS_FunctionObjectTable[] = 
{
    { "apply",               &FunctionProto::Apply },
    { "call",                &FunctionProto::Call  },
    { "toString",            &FunctionProto::ToString },
    { "valueOf",             &FunctionProto::ValueOf },
    { 0, 0 }
};

FunctionProto::FunctionProto(ASStringContext* psc, Object* pprototype,
                             const FunctionRef& constructor, bool initFuncs)
: Prototype<Object>(psc, pprototype, constructor)
{
    if (initFuncs)
        InitFunctionMembers(psc, GAS_FunctionObjectTable);
}

void FunctionProto::GlobalCtor(const FnCall& fn)
{
    if (fn.NArgs == 1)
    {
        // Seems this is just a type casting call, such as "Function(func)"
        // Just return the unchanged parameter.
        // Flash should generate a special opcode for type casting, and it does
        // do this for casts like "s : Selection = Selection(ss)". 
        // But, for some reasons, if the cast operator looks like "f = Function(func)"
        // Flash generates a function call to global function "Function" with a param.
        // In this case, just return the passed parameter. Do the same, if 
        // "f = new Function(func) is used; in this case f == func anyway.
        if (fn.Arg(0).IsFunction() || fn.Arg(0).IsFunctionName())
            fn.Result->SetAsObject(fn.Arg(0).ToObject(fn.Env));
        else
            fn.Result->SetNull();
    }
    else
    {
        Ptr<CFunctionObject> obj = *SF_HEAP_NEW(fn.Env->GetHeap()) CFunctionObject(fn.Env);
        fn.Result->SetAsObject(obj.GetPtr());
    }
}

void FunctionProto::ToString(const FnCall& fn)
{
    fn.Result->SetString(fn.Env->GetBuiltin(ASBuiltin_typeFunction_));
}

void FunctionProto::ValueOf(const FnCall& fn)
{
    SF_ASSERT(fn.ThisPtr);

    fn.Result->SetAsObject(static_cast<Object*>(fn.ThisPtr));
}

void FunctionProto::Apply(const FnCall& fn)
{
    SF_ASSERT(fn.ThisPtr);

    Ptr<Object> objectHolder;
    Ptr<InteractiveObject> charHolder;
    ObjectInterface* thisObj = 0;
    int nArgs = 0;

    fn.Result->SetUndefined();
    Ptr<ArrayObject> arguments;

    if (fn.NArgs >= 1)
    {
        thisObj = fn.Arg(0).ToObjectInterface(fn.Env);
        if (thisObj)
        {
            if (thisObj->IsASCharacter())
                charHolder = thisObj->ToCharacter();
            else
                objectHolder = static_cast<Object*>(thisObj);
        }
    }
    if (fn.NArgs >= 2)
    {
        // arguments array
        Object* args = fn.Arg(1).ToObject(fn.Env);
        if (args && args->GetObjectType() == ObjectInterface::Object_Array)
        {
            arguments = static_cast<ArrayObject*>(args);
            nArgs = arguments->GetSize();

            // push arguments into the environment's stack
            if (nArgs > 0)
            {
                for (int i = nArgs - 1; i >= 0; --i)
                    fn.Env->Push(*arguments->GetElementPtr(i));
            }
        }
    }
    
    Value result;
    // invoke function
    if (!fn.ThisFunctionRef.IsNull())
    {
        //!AB: if ThisFunctionRef is not null then we may do call with using it.
        // In this case localFrame will be correct, and this is critical for calling nested functions.
        fn.ThisFunctionRef.Invoke(FnCall(&result, thisObj, fn.Env, nArgs, fn.Env->GetTopIndex()));
    }
    else //!AB: is it necessary or ThisFunctionRef should be not null allways?
    {
        Ptr<FunctionObject> func = static_cast<FunctionObject*>(fn.ThisPtr);
        func->Invoke(FnCall(&result, thisObj, fn.Env, nArgs, fn.Env->GetTopIndex()), 0, NULL);
    }
    
    // wipe out arguments
    if (nArgs > 0)
    {
        fn.Env->Drop(nArgs);
    }
    *fn.Result = result;
}

void FunctionProto::Call(const FnCall& fn)
{
    SF_ASSERT(fn.ThisPtr);

    Ptr<Object> objectHolder;
    Ptr<InteractiveObject> charHolder;
    ObjectInterface* thisObj = 0;
    int nArgs = 0;

    fn.Result->SetUndefined();
    Ptr<ArrayObject> arguments;

    if (fn.NArgs >= 1)
    {
        thisObj = fn.Arg(0).ToObjectInterface(fn.Env);
        if (thisObj)
        {
            if (thisObj->IsASCharacter())
                charHolder = thisObj->ToCharacter();
            else
                objectHolder = static_cast<Object*>(thisObj);
        }
    }
    if (fn.NArgs >= 2)
    {
        nArgs = fn.NArgs - 1;

        for (int i = nArgs; i >= 1; --i)
            fn.Env->Push(fn.Arg(i));
    }

    Value result;
    // invoke function
    if (!fn.ThisFunctionRef.IsNull())
    {
        //!AB: if ThisFunctionRef is not null then we may do call with using it.
        // In this case localFrame will be correct, and this is critical for calling nested functions.
        fn.ThisFunctionRef.Invoke(FnCall(&result, thisObj, fn.Env, nArgs, fn.Env->GetTopIndex()));
    }
    else //!AB: is it necessary or ThisFunctionRef should be not null allways?
    {
        Ptr<FunctionObject> func = static_cast<FunctionObject*>(fn.ThisPtr);
        func->Invoke(FnCall(&result, thisObj, fn.Env, nArgs, fn.Env->GetTopIndex()), 0, NULL);
    }

    // wipe out arguments
    if (nArgs > 0)
    {
        fn.Env->Drop(nArgs);
    }
    *fn.Result = result;
}

/////////////////////////////////////////////////////
FunctionCtorFunction::FunctionCtorFunction (ASStringContext* psc) :
    CFunctionObject(psc, FunctionProto::GlobalCtor)
{
    SF_UNUSED(psc);
}
}}} //SF::GFx::AS2
