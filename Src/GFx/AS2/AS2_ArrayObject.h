/**************************************************************************

Filename    :   AS2_ArrayObject.h
Content     :   Array object functinality
Created     :   March 10, 2006
Authors     :   Maxim Shemanarev, Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_ARRAY_H
#define INC_SF_GFX_ARRAY_H

#include "Kernel/SF_Alg.h"
#include "GFx/AS2/AS2_Action.h"
#include "GFx/AS2/AS2_ObjectProto.h"


namespace Scaleform { namespace GFx { namespace AS2 {

// ***** Declared Classes
class RecursionGuard;
class ArraySortFunctor;
class ArraySortOnFunctor;
class ArrayObject;
class ArrayProto;
class ArrayCtorFunction;



class RecursionGuard
{
public:
    RecursionGuard(const ArrayObject* pThis);
    ~RecursionGuard();
    int Count() const;
private:
    const ArrayObject* ThisPtr;
};


class ArraySortFunctor
{
public:
    ArraySortFunctor() {}
    ArraySortFunctor(ObjectInterface* pThis, 
                        int flags,
                        const FunctionRef& func,
                        Environment* env,
                        const Log* log=0) : 
        This(pThis), 
        Flags(flags), 
        Func(func), 
        Env(env), 
        LogPtr(log)
    {}

    int Compare(const Value* a, const Value* b) const;
    bool operator()(const Value* a, const Value* b) const
    {
        return Compare(a, b) < 0;
    }

private:
    ObjectInterface*    This;
    int                 Flags;
    FunctionRef         Func;
    Environment*        Env;
    const Log*          LogPtr;
};


class ArraySortOnFunctor
{
public:
    ArraySortOnFunctor(ObjectInterface* pThis, 
                          const ArrayCC<ASString, StatMV_ActionScript_Mem>& fieldArray,
                          const Array<int>& flagsArray,
                          Environment* env,
                          const Log* log=0); 

    int Compare(const Value* a, const Value* b) const;
    bool operator()(const Value* a, const Value* b) const
    {
        return Compare(a, b) < 0;
    }

private:
    ObjectInterface*             This;
    const ArrayCC<ASString, StatMV_ActionScript_Mem>* FieldArray;
    Environment*                 Env;
    const Log*                   LogPtr;
    Array<ArraySortFunctor>     FunctorArray;
};




class ArrayObject : public Object
{
    friend class ArrayProto;
    friend class ArrayCtorFunction;

#ifdef GFX_AS_ENABLE_GC
protected:
    friend class RefCountBaseGC<StatMV_ActionScript_Mem>;
    template <class Functor> void ForEachChild_GC(Collector* prcc) const;
    virtual void                  ExecuteForEachChild_GC(Collector* prcc, OperationGC operation) const;
    virtual void                  Finalize_GC();
#endif // SF_NO_GC
protected:
    ArrayObject(ASStringContext *psc, Object* pprototype); // for prototype only

    void InitArray(const FnCall& fn);
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
    
    friend class RecursionGuard;

    ArrayObject(Environment* penv);
    ArrayObject(ASStringContext *psc);
    ~ArrayObject();
    virtual bool    SetMember(Environment* penv, const ASString& name, const Value& val, const PropFlags& flags = PropFlags());
    virtual bool    GetMemberRaw(ASStringContext *psc, const ASString& name, Value* val);
    virtual bool    DeleteMember(ASStringContext *psc, const ASString& name);
    virtual void    VisitMembers(ASStringContext *psc, MemberVisitor *pvisitor, unsigned visitFlags, const ObjectInterface* instance = 0) const;   
    virtual bool    HasMember(ASStringContext *psc, const ASString& name, bool inclPrototypes);

    // pEnv is required for Array's GetTextValue.
    virtual const char* GetTextValue(Environment* penv) const;
    virtual ObjectType  GetObjectType() const;

    int             GetSize() const { return (unsigned)Elements.GetSize(); }
    void            Resize(int newSize);
          Value*    GetElementPtr(int i)       { return Elements[i]; }
    const Value*    GetElementPtr(int i) const { return Elements[i]; }
    void            SetElement(int i, const Value& val);
    // SetElementSafe may set any element even if idx is beyond the array's size.
    void            SetElementSafe(int idx, const Value& val);
    void            PushBack(const Value& val);
    void            PushBack();
    void            PopBack();
    void            PopFront();
    void            RemoveElements(int start, int count);
    void            InsertEmpty(int start, int count);
    void            JoinToString(Environment* pEnv, StringBuffer* pbuffer, const char* pDelimiter) const;
    void            Concat(Environment* pEnv, const Value& val);
    void            ShallowCopyFrom(const ArrayObject& ao);
    void            MakeDeepCopy(MemoryHeap *pheap);
    void            MakeDeepCopyFrom(MemoryHeap *pheap, const ArrayObject& ao);
    void            DetachAll();
    void            Reverse();
    void            Reserve(UPInt sz) { Elements.Reserve(sz); }

    template<class SortFunctor>
    bool            Sort(SortFunctor& sf)
    {
        if (Elements.GetSize() > 0)
        {
            Alg::ArrayAdaptor<Value*> a(&Elements[0], (unsigned)Elements.GetSize());
            return Alg::QuickSortSafe(a, sf);
        }
        return true;
    }

    static int      ParseIndex(const ASString& name);

    bool            RecursionLimitReached() const;
    const Log*      GetLogPtr() const { return LogPtr; }

    static void     ArrayConcat(const FnCall& fn);
    static void     ArrayJoin(const FnCall& fn);
    static void     ArrayPop(const FnCall& fn);
    static void     ArrayPush(const FnCall& fn);
    static void     ArrayReverse(const FnCall& fn);
    static void     ArrayShift(const FnCall& fn);
    static void     ArraySlice(const FnCall& fn);
    static void     ArraySort(const FnCall& fn);
    static void     ArraySortOn(const FnCall& fn);
    static void     ArraySplice(const FnCall& fn);
    static void     ArrayToString(const FnCall& fn);
    static void     ArrayUnshift(const FnCall& fn);
    static void     ArrayLength(const FnCall& fn);
    static void     ArrayValueOf(const FnCall& fn);
private:
    void CallProlog() const { RecursionCount++; }
    void CallEpilog() const { RecursionCount--; }

    Log* const              LogPtr;
    ArrayLH<Value*>         Elements;
    mutable StringLH        StringValue;
    mutable int             RecursionCount;
    bool                    LengthValueOverriden;
};


class ArrayProto : public Prototype<ArrayObject>
{
public:
    ArrayProto(ASStringContext *psc, Object* prototype, const FunctionRef& constructor);
};


class ArrayCtorFunction : public CFunctionObject
{
public:
    ArrayCtorFunction (ASStringContext *psc);

    static void GlobalCtor(const FnCall& fn);
    static void DeclareArray(const FnCall& fn);

    virtual Object* CreateNewObject(Environment* penv) const;
    static FunctionRef Register(GlobalContext* pgc);
};

}}} // SF::GFx::AS2

#endif
