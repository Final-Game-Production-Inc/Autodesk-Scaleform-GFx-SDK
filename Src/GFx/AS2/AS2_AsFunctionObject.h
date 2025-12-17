/**************************************************************************

Filename    :   AS2_AsFunctionObject.h
Content     :   ActionScript function object
Created     :   May 5, 2008
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/


#ifndef INC_SF_GFx_AS2_AsFunctionObject_H
#define INC_SF_GFx_AS2_AsFunctionObject_H

#include "GFx/GFx_PlayerImpl.h"

namespace Scaleform { namespace GFx { namespace AS2 {

// Function object for ActionScript functions ("As-functions")
class AsFunctionObject : public FunctionObject 
{
    friend class FunctionObject;
    friend class InvokeContext;
protected:
    MovieImpl*                  pMovieRoot;
    Ptr<GFx::CharacterHandle>   TargetHandle;

    Ptr<ActionBuffer>           pActionBuffer;
    WithStackArray              WithStack;      // initial with-stack on function entry.
    unsigned                    StartPc;
    unsigned                    Length;

    struct ArgSpec
    {
        int         Register;
        ASString   Name;

        ArgSpec(int r, const ASString &n)
            : Register(r), Name(n) { }
        ArgSpec(const ArgSpec& s)
            : Register(s.Register), Name(s.Name) { }
    };

    ArrayCC<ArgSpec, StatMV_ActionScript_Mem> Args;
    UInt16                  Function2Flags; // used by function2 to control implicit arg register assignments
    UByte                   ExecType;   // ActionBuffer::ExecuteType - Function2, Event, etc.    
    UByte                   LocalRegisterCount;

    AsFunctionObject(ASStringContext* psc);
    AsFunctionObject(Environment* penv);

#ifdef GFX_AS_ENABLE_GC
    void Finalize_GC()
    {
        Args.~ArrayCC();
        WithStack.~WithStackArray();
        pActionBuffer   = NULL;
        TargetHandle    = NULL;
        //pMovieRoot = 0;
        FunctionObject::Finalize_GC();
    }
#endif // SF_NO_GC
public:
    AsFunctionObject(Environment* penv, ActionBuffer* ab,  
        unsigned start, unsigned length,
        const WithStackArray* pwithStack,
        ActionBuffer::ExecuteType execType = ActionBuffer::Exec_Function);

    virtual Environment* GetEnvironment(const FnCall& fn, Ptr<InteractiveObject>* ptargetCh);
    void Invoke (const FnCall& fn, LocalFrame*, const char* pmethodName);

    virtual bool IsNull () const { return false; }
    virtual bool IsEqual(const FunctionObject&) const { return false; } // not impl

    virtual bool IsAsFunction() const  { return true; }

    //virtual GASFunctionRef  ToFunction();

    // returns number of arguments expected by this function;
    // returns -1 if number of arguments is unknown (for C functions)
    int                 GetNumArgs() const { return (unsigned)Args.GetSize(); }
    const ActionBuffer* GetActionBuffer() const { return pActionBuffer; }
    unsigned            GetStartPC()      const { return StartPc; }
    unsigned            GetLength()       const { return Length; }

    virtual Object* CreateNewObject(Environment* penv) const;

    // AS-function specific methods
    void    SetExecType(ActionBuffer::ExecuteType execType)      { ExecType = (UByte)execType; }
    ActionBuffer::ExecuteType GetExecType() const                { return (ActionBuffer::ExecuteType)ExecType; }

    bool    IsFunction2() const { return ExecType == ActionBuffer::Exec_Function2; }

    void    SetLocalRegisterCount(UByte ct) { SF_ASSERT(IsFunction2()); LocalRegisterCount = ct; }
    void    SetFunction2Flags(UInt16 flags) { SF_ASSERT(IsFunction2()); Function2Flags = flags; }

    class InteractiveObject* GetTargetCharacter();

    const ArgSpec&  AddArg(int argRegister, const ASString& name)
    {
        SF_ASSERT(argRegister == 0 || IsFunction2() == true);
        Args.Resize(Args.GetSize() + 1);
        ArgSpec& arg = Args.Back();
        arg.Register = argRegister;
        arg.Name = name;
        return arg;
    }

    void    SetLength(int len) { SF_ASSERT(len >= 0); Length = len; }
};

}}} //SF::GFx::AS2

#endif //INC_GASASFUNCOBJ_H
