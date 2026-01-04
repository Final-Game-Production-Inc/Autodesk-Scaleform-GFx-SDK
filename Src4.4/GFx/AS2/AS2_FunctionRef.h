/**************************************************************************

PublicHeader:   GFx
Filename    :   AS2_FunctionRef.h
Content     :   ActionScript implementation classes
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/


#ifndef INC_SF_GFX_AS2_FUNCTIONREF_H
#define INC_SF_GFX_AS2_FUNCTIONREF_H

#include "GFx/AS2/AS2_ActionTypes.h"

namespace Scaleform { namespace GFx { namespace AS2 {

// ***** Declared Classes
class FunctionDef;
class CFunctionDef;
class AsFunctionDef;
class FunctionRefBase;
class FunctionRef;
class FunctionWeakRef;

// ***** External Classes
class FnCall;
class Environment;
class ActionBuffer;
class LocalFrame;
class FunctionObject;

// DO NOT use FunctionRefBase anywhere but as a base class for 
// FunctionRef and FunctionWeakRef. DO NOT add virtual methods,
// since FunctionRef and FunctionWeakRef should be very simple 
// and fast. DO NOT add ctor/dtor and assignment operators to FunctionRefBase
// since it is used in union.
class FunctionRefBase
{
//protected:
public:
    FunctionObject*      Function;
    LocalFrame*          pLocalFrame; 
    enum FuncRef_Flags
    {
        FuncRef_Internal = 1,
        FuncRef_Weak     = 2
    };
    UByte                   Flags;
public:
    SF_INLINE void Init (FunctionObject* funcObj = 0, UByte flags = 0);
    SF_INLINE void Init (FunctionObject& funcObj, UByte flags = 0);
    SF_INLINE void Init (const FunctionRefBase& orig, UByte flags = 0);
    SF_INLINE void DropRefs();

    void SetLocalFrame (LocalFrame* localFrame, bool internal = false);

    bool IsInternal () const { return (Flags & FuncRef_Internal); }
    void SetInternal (bool internal = false);

    // pmethodName is used for creation of "super" object:
    // it is important to find real owner of the function object
    // to make "super" works correctly.
    SF_INLINE void Invoke(const FnCall& fn, const char* pmethodName = NULL) const;  

    SF_INLINE bool operator== (const FunctionRefBase& f) const;
    SF_INLINE bool operator== (const CFunctionPtr f) const;
    SF_INLINE bool operator!= (const FunctionRefBase& f) const;
    SF_INLINE bool operator!= (const CFunctionPtr f) const;

    SF_INLINE bool IsCFunction () const;
    SF_INLINE bool IsAsFunction () const;
    SF_INLINE bool IsNull () const { return Function == 0; }

    void Assign(const FunctionRefBase& orig);

    SF_INLINE FunctionObject*           operator-> () { return Function; }
    SF_INLINE FunctionObject*           operator-> () const { return Function; }
    SF_INLINE FunctionObject&           operator* () { return *Function; }
    SF_INLINE FunctionObject&           operator* () const { return *Function; }
    SF_INLINE FunctionObject*           GetObjectPtr () { return Function; }
    SF_INLINE FunctionObject*           GetObjectPtr () const { return Function; }

#ifdef GFX_AS_ENABLE_GC
    typedef RefCountBaseGC<StatMV_ActionScript_Mem>::Collector Collector;
    template <class Functor> void ForEachChild_GC(Collector* prcc) const;
    void                          Finalize_GC() {}
#endif //SF_NO_GC
};

class FunctionRef : public FunctionRefBase
{
public:
    SF_INLINE FunctionRef(FunctionObject* funcObj = 0)  { Init(funcObj);  }
    SF_INLINE FunctionRef(FunctionObject& funcObj)      { Init(funcObj); }
    SF_INLINE FunctionRef(const FunctionRefBase& orig)
    {
        Init(orig);
    }
    SF_INLINE FunctionRef(const FunctionRef& orig)
    {
        Init(orig);
    }
    SF_INLINE ~FunctionRef()
    {
        DropRefs();
    }

    SF_INLINE const FunctionRefBase& operator = (const FunctionRefBase& orig)
    {
        Assign(orig);
        return *this;
    }
    SF_INLINE const FunctionRef& operator = (const FunctionRef& orig)
    {
        Assign(orig);
        return *this;
    }
};

#ifndef GFX_AS_ENABLE_GC
// used for "constructor" in prototypes, they should have 
// weak reference to the function, because constructor itself
// contains strong reference to its prototype.
// Convertible to and from regular FunctionRef
class FunctionWeakRef : public FunctionRefBase
{
public:
    FunctionWeakRef(FunctionObject* funcObj = 0) 
    { 
        Init(funcObj, FuncRef_Weak); 
        SetInternal(true);
    }
    FunctionWeakRef(const FunctionRefBase& orig)
    {
        Init(orig, FuncRef_Weak);
        SetInternal(true);
    }
    FunctionWeakRef(const FunctionWeakRef& orig)
    {
        Init(orig, FuncRef_Weak);
        SetInternal(true);
    }
    SF_INLINE ~FunctionWeakRef()
    {
        DropRefs();
    }
    SF_INLINE const FunctionRefBase& operator = (const FunctionRefBase& orig)
    {
        Assign(orig);
        return *this;
    }
    SF_INLINE const FunctionWeakRef& operator = (const FunctionWeakRef& orig)
    {
        Assign(orig);
        return *this;
    }
};
#endif // SF_NO_GC

}}} // SF::GFx::AS2
#endif // INC_SF_GFX_FUNCTIONREF_H

