/**************************************************************************

Filename    :   AS2_Math.h
Content     :   AS2 Implementation of Math class
Created     :   January 6, 2009
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_Math_H
#define INC_SF_GFX_Math_H

#include "GFx/AS2/AS2_Object.h"
#include "GFx/AS2/AS2_ObjectProto.h"
#include "GFx/GFx_PlayerImpl.h"

namespace Scaleform { namespace GFx { namespace AS2 {

class Math : public Object
{
protected:
    Math(ASStringContext *psc, Object* pprototype)
        : Object(psc)
    { 
        Set__proto__(psc, pprototype); // this ctor is used for prototype obj only
    }

public:
    Math(Environment* penv);

    static UInt32 GetNextRandom(MovieImpl* proot);
};

class MathProto : public Prototype<Math>
{
public:
    MathProto (ASStringContext* psc, Object* pprototype, const FunctionRef& constructor);
};

// One-argument functions
#define SF_AS2MATH_WRAP_FUNC1(funcname, cfunc)         \
    static SF_INLINE void funcname(const FnCall& fn)   \
    {                                                   \
        Number   arg = fn.Arg(0).ToNumber(fn.Env);   \
        fn.Result->SetNumber((Number)cfunc(arg));    \
    }

// Two-argument functions.
#define SF_AS2MATH_WRAP_FUNC2(funcname, expr)          \
    static SF_INLINE void funcname(const FnCall& fn)   \
    {                                                   \
        Number   arg0 = fn.Arg(0).ToNumber(fn.Env);  \
        Number   arg1 = fn.Arg(1).ToNumber(fn.Env);  \
        fn.Result->SetNumber((Number)expr);          \
    }

//
// Math static class
//
class MathCtorFunction : public CFunctionObject
{
    static const NameFunction StaticFunctionTable[];
    
    SF_AS2MATH_WRAP_FUNC1(Abs, Alg::Abs);
    SF_AS2MATH_WRAP_FUNC1(Acos, acos);
    SF_AS2MATH_WRAP_FUNC1(Asin, asin);
    SF_AS2MATH_WRAP_FUNC1(Atan, atan);
    SF_AS2MATH_WRAP_FUNC1(Ceil, ceil);
    SF_AS2MATH_WRAP_FUNC1(Cos, cos);
    SF_AS2MATH_WRAP_FUNC1(Exp, exp);
    SF_AS2MATH_WRAP_FUNC1(Floor, floor);
    SF_AS2MATH_WRAP_FUNC1(Log, log);
    SF_AS2MATH_WRAP_FUNC1(Sin, sin);
    SF_AS2MATH_WRAP_FUNC1(Sqrt, sqrt);
    SF_AS2MATH_WRAP_FUNC1(Tan, tan);

    SF_AS2MATH_WRAP_FUNC2(Atan2, (atan2(arg0, arg1)));
    SF_AS2MATH_WRAP_FUNC2(Max, (arg0 > arg1 ? arg0 : arg1));
    SF_AS2MATH_WRAP_FUNC2(Min, (arg0 < arg1 ? arg0 : arg1));
    SF_AS2MATH_WRAP_FUNC2(Pow, (pow(arg0, arg1)));

    static void Random(const FnCall& fn);
    static void Round(const FnCall& fn);

public:
    MathCtorFunction (ASStringContext *psc);

    static void GlobalCtor(const FnCall& fn);
    virtual Object* CreateNewObject(Environment*) const { return 0; }

    static FunctionRef Register(GlobalContext* pgc);
};

}}} //SF::GFx::AS2
#endif // INC_SF_GFX_Math_H
