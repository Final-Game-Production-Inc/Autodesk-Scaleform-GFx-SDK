/**************************************************************************

Filename    :   AS2_ArrayObject.cpp
Content     :   Array object functinality
Created     :   March 10, 2006
Authors     :   Maxim Shemanarev & Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/


#include "Kernel/SF_AutoPtr.h"
#include "GFx/AS2/AS2_ArrayObject.h"
#include "GFx/AS2/AS2_NumberObject.h"
#include "GFx/GFx_Log.h"
#include "Kernel/SF_MsgFormat.h"

#include <stdlib.h>

namespace Scaleform { namespace GFx { namespace AS2 {

RecursionGuard::RecursionGuard(const ArrayObject* pthis) :
    ThisPtr(pthis)
{
    ThisPtr->CallProlog();
}

RecursionGuard::~RecursionGuard()
{
    ThisPtr->CallEpilog();
}

int RecursionGuard::Count() const
{
    return ThisPtr->RecursionCount;
}

static bool GAS_ParseNumber(const char* str, Number* retVal)
{
    if(str && *str)
    {
        if((*str >= '0' && *str <= '9') ||
            *str == '+' ||
            *str == '-' ||
            *str == '.')
        {
            char* end;
            *retVal = (Number)SFstrtod(str, &end);
            if(end == 0 || *end == 0)
            {
                return true;
            }
        }
    }
    return false;
}

int ArraySortFunctor::Compare(const Value* a, const Value* b) const
{
    Value dummy;
    if(a == 0) a = &dummy;
    if(b == 0) b = &dummy;
    int cmpVal = 0;

    if(Func != NULL)
    {
        Value retVal;
        Env->Push(*b);
        Env->Push(*a);

        // The "This" isn't correct here. Needs to support "undefined";
        // Since FnCall doesn't support it we just use "This" from
        // the array object.
        FnCall fn(&retVal, This, Env, 2, Env->GetTopIndex());

        Func.Invoke(fn);
        Env->Drop(2);
        if(fn.Result) 
        {
            cmpVal = fn.Result->ToInt32(Env);
            return (Flags & ArrayObject::SortFlags_Descending) ? -cmpVal : cmpVal;
        }
    }
    else
    {
        bool numericA = false;
        bool numericB = false;
        Number valA=0;
        Number valB=0;

        if (Flags & ArrayObject::SortFlags_Numeric)
        {
            // This is an attempt to make the "NUMERIC"
            // sort logic compliant with Flash. If flag
            // "NUMERIC" is set we try to convert the
            // values to numbers. If both can be successfully
            // converted we compare them as numbers, otherwise
            // they are compared as strings.
            //
            // It actually breaks the logic of sorting if we use
            // something faster than O(N^2). Any sort algorithm that
            // is better than O(N^2) relies on the transitivity
            // of the "less-than" relation. With this logic there's
            // no transitivity. That is, a<b<c does not obligatory 
            // mean that a<c. For example:
            // a = "99 ";
            // b = 99.9;
            // c = 100;
            // Here 'a' is a string and it's compared with 'b' as a string.
            // So, we have a<b. Then, 'b' and 'c" are numbers and they
            // are compared as numbers, so, we have b<c. But 'a' and 'c' 
            // are compared as strings again, and we have a>c. 
            // 
            // It means that if we use quick sort on a mixed array
            // with Array.NUMERIC flag the result will differ from 
            // the original Flash, but still correct. 
            // It should be fine because in real life there is 
            // no need to sort mixed arrays as NUMERIC (it's a crazy 
            // case and it'd be crazy to make it 100% Flash compliant).
            if (a->IsNumber())
            {
                valA = a->ToNumber(Env);
                numericA = true;
            }
            else
            {
                ASString sa(a->ToString(Env));
                numericA = GAS_ParseNumber(sa.ToCStr(), &valA);
            }

            if (b->IsNumber())
            {
                valB = b->ToNumber(Env);
                numericB = true;
            }
            else
            {
                ASString sb(b->ToString(Env));
                numericB = GAS_ParseNumber(sb.ToCStr(), &valB);
            }
        }

        if(numericA && numericB)
        {
            if(valA < valB) cmpVal = -1;
            if(valB < valA) cmpVal =  1;
            return (Flags & ArrayObject::SortFlags_Descending) ? -cmpVal : cmpVal;
        }
        else
        {
            ASString sa(a->ToString(Env));
            ASString sb(b->ToString(Env));
            
            if (Flags & ArrayObject::SortFlags_Locale)
            {
                // this is an extension, sorting according to locale
                cmpVal = ASString::LocaleCompare_CaseCheck(sa, sb, 
                    (Flags & ArrayObject::SortFlags_CaseInsensitive) == 0);
            }
            else
            {
                cmpVal = (Flags & ArrayObject::SortFlags_CaseInsensitive)?
                    String::CompareNoCase(sa.ToCStr(), sb.ToCStr()) :
                    SFstrcmp (sa.ToCStr(), sb.ToCStr());
            }
            return (Flags & ArrayObject::SortFlags_Descending) ? -cmpVal : cmpVal;
        }
    }
    return 0;
}


ArraySortOnFunctor::ArraySortOnFunctor(ObjectInterface* pThis, 
                                             const ArrayCC<ASString, StatMV_ActionScript_Mem>& fieldArray,
                                             const Array<int>& flagsArray,
                                             Environment* env,
                                             const Log* log) : 
    This(pThis),
    FieldArray(&fieldArray),
    Env(env),
    LogPtr(log)
{
    FunctorArray.Resize(flagsArray.GetSize());
    unsigned i;
    for(i = 0; i < FunctorArray.GetSize(); i++)
    {
        FunctorArray[i] = ArraySortFunctor(pThis, 
                                              flagsArray[i],
                                              0,
                                              env,
                                              log);
    }
}


int ArraySortOnFunctor::Compare(const Value* a, const Value* b) const
{
    Value dummy;
    if(a == 0) a = &dummy;
    if(b == 0) b = &dummy;
    int cmpVal = 0;

    unsigned i;
    SF_ASSERT(Env);
    ASStringContext *psc = Env->GetSC();    

    for(i = 0; i < FunctorArray.GetSize(); i++)
    {
        ObjectInterface* objA = a->ToObjectInterface(Env);
        ObjectInterface* objB = b->ToObjectInterface(Env);
        if(objA && objB)
        {
            Value valA;
            Value valB;
            const ASString& fieldName = (*FieldArray)[i];
            if(objA->GetMemberRaw(psc, fieldName, &valA) &&
               objB->GetMemberRaw(psc, fieldName, &valB))
            {
                cmpVal = FunctorArray[i].Compare(&valA, &valB);
                if(cmpVal) return cmpVal;
            }
        }
    }
    return 0;
}






ArrayObject::ArrayObject(Environment* penv) : 
    Object(penv), LogPtr(penv->GetLog()), Elements(), RecursionCount(0),
    LengthValueOverriden(false)
{    
    Set__proto__(penv->GetSC(), penv->GetPrototype(ASBuiltin_Array));
}

ArrayObject::ArrayObject(ASStringContext *psc) :
    Object(psc), LogPtr(0), Elements(), RecursionCount(0),
    LengthValueOverriden(false)
{
    Set__proto__(psc, psc->pContext->GetPrototype(ASBuiltin_Array));
}

ArrayObject::ArrayObject(ASStringContext *psc, Object* pprototype) :
    Object(psc), LogPtr(0), Elements(), RecursionCount(0),
    LengthValueOverriden(false)
{
    Set__proto__(psc, pprototype);
}

ArrayObject::~ArrayObject()
{
    Resize(0);
}

#ifdef GFX_AS_ENABLE_GC
void ArrayObject::ExecuteForEachChild_GC(Collector* prcc, OperationGC operation) const
{
    ASRefCountBaseType::CallForEachChild<ArrayObject>(prcc, operation);
}

template <class Functor>
void ArrayObject::ForEachChild_GC(Collector* prcc) const
{
    Object::template ForEachChild_GC<Functor>(prcc);
    UPInt i, n;
    for(i = 0, n = Elements.GetSize(); i < n; i++)
    {
        Value* pval = Elements[i];
        if (pval)
            pval->template ForEachChild_GC<Functor>(prcc);
    }
}
SF_GC_PREGEN_FOR_EACH_CHILD(ArrayObject)

void ArrayObject::Finalize_GC()
{
    UPInt i, n;
    for(i = 0, n = Elements.GetSize(); i < n; i++)
    {
        Value* pval = Elements[i];
        if (pval)
            pval->Finalize_GC();
        SF_FREE(Elements[i]);
    }
    Elements.~ArrayLH();
    StringValue.~StringLH();
    Object::Finalize_GC();
}
#endif //SF_NO_GC

bool ArrayObject::RecursionLimitReached() const
{
    if(RecursionCount >= 255)
    {
        LogPtr->LogMessageById(Log_ScriptError,"256 levels of recursion is reached\n");
        return true;
    }
    return false;
}

void ArrayObject::Resize(int size) 
{ 
    if(size < 0) size = 0;
    UPInt i;
    UPInt newSize = (UPInt)size;
    UPInt oldSize = Elements.GetSize();
    for(i = newSize; i < oldSize; i++)
    {
        delete Elements[i];
    }
    Elements.Resize(newSize);
    for(i = oldSize; i < newSize; i++)
    {
        Elements[i] = 0;
    }
}

void ArrayObject::ShallowCopyFrom(const ArrayObject& ao)
{
    Elements.Resize(ao.Elements.GetSize());
    for (UPInt i = 0, n = Elements.GetSize(); i < n; i++)
    {
        Elements[i] = ao.Elements[i];
    }
}

void ArrayObject::MakeDeepCopy(MemoryHeap *pheap)
{
    for (UPInt i = 0, n = Elements.GetSize(); i < n; i++)
    {
        if (Elements[i])
        {
            Value* newVal = SF_HEAP_NEW(pheap) Value(*Elements[i]);
            Elements[i] = newVal;
        }
    }
}

void ArrayObject::MakeDeepCopyFrom(MemoryHeap *pheap, const ArrayObject& ao)
{
    Elements.Resize(ao.Elements.GetSize());
    for (UPInt i = 0, n = Elements.GetSize(); i < n; i++)
    {
        if (ao.Elements[i])
        {
            Value* newVal = SF_HEAP_NEW(pheap) Value(*ao.Elements[i]);
            Elements[i] = newVal;
        }
    }
}

void ArrayObject::DetachAll()
{ 
    Elements.Clear(); 
}

void ArrayObject::JoinToString(Environment* pEnv, StringBuffer* pbuffer, const char* pDelimiter) const
{
    pbuffer->Clear();
    Value undefined;

    for(unsigned i = 0; i < Elements.GetSize(); i++)
    {
        if (i) *pbuffer += pDelimiter;

        if (Elements[i] != 0)
        {
            ASString s(Elements[i]->ToString(pEnv));
            *pbuffer += s.ToCStr();
        }
        else
        {
            ASString s(undefined.ToString(pEnv));
            *pbuffer += s.ToCStr();
        }
    }    
}

void ArrayObject::Concat(Environment* penv, const Value& val)
{
    RecursionGuard rg(this);
    if(RecursionLimitReached()) return;

    MemoryHeap* pheap = penv->GetHeap();

    Object* pObj = val.ToObject(penv);
    if(pObj && pObj->GetObjectType() == ObjectInterface::Object_Array)
    {
        ArrayObject* pArray = (ArrayObject*)pObj;
        if(pArray->Elements.GetSize())
        {
            unsigned oldSize = (unsigned)Elements.GetSize();
            Resize((int)(oldSize + pArray->Elements.GetSize()));
            for(UPInt i = 0; i < pArray->Elements.GetSize(); i++)
            {
                Elements[oldSize + i] = SF_HEAP_NEW(pheap) Value(*pArray->Elements[i]);
            }
        }
    }
    else
    {
        Elements.PushBack(SF_HEAP_NEW(pheap) Value(val));
    }
}

void ArrayObject::PushBack(const Value& val)
{
    Elements.PushBack(SF_HEAP_AUTO_NEW(this) Value(val));
}

void ArrayObject::PushBack()
{
    Elements.PushBack(0);
}

void ArrayObject::PopBack()
{
    if(Elements.GetSize()) 
    {
        Resize((int)(Elements.GetSize() - 1));
    }
}

void ArrayObject::RemoveElements(int start, int count)
{
    if(Elements.GetSize())
    {
        int i;
        for(i = 0; i < count; i++)
        {
            delete Elements[start + i];
        }
        for(i = start + count; i < (int)Elements.GetSize(); i++)
        {
            Elements[i - count] = Elements[i];
            Elements[i] = 0;
        }
        Elements.Resize(Elements.GetSize() - count);
    }
}

void ArrayObject::InsertEmpty(int start, int count)
{
    int i;
    int oldSize = (int)Elements.GetSize();
    Elements.Resize(oldSize + count);
    if(oldSize)
    {
        for(int ie = (int)Elements.GetSize() - 1; ie >= start + count; --ie)
        {
            Elements[ie] = Elements[static_cast<Scaleform::UPInt>(ie) - count];
        }
    }
    for(i = 0; i < count; i++)
    {
        Elements[static_cast<Scaleform::UPInt>(start) + i] = 0;
    }
}

void ArrayObject::PopFront()
{
    if(Elements.GetSize())
    {
        delete Elements[0];
        for(unsigned i = 1; i < Elements.GetSize(); i++)
        {
            Elements[i-1] = Elements[i];
        }
        Elements[Elements.GetSize() - 1] = 0;
        Elements.Resize(Elements.GetSize() - 1);
    }
}

void ArrayObject::SetElement(int i, const Value& val)
{
    if (i < 0 || i >= int(Elements.GetSize())) return;

    LengthValueOverriden = false;
    if(Elements[i] == 0) Elements[i] = SF_HEAP_AUTO_NEW(this) Value;
    *Elements[i] = val;
}

void ArrayObject::SetElementSafe(int idx, const Value& val)
{
    SF_ASSERT(idx >= 0);
    LengthValueOverriden = false;
    if (idx >= (int)Elements.GetSize())
    {
        Resize(idx + 1);
    }
    if (Elements[idx] == 0)
    {
        Elements[idx] = SF_HEAP_AUTO_NEW(this) Value();
    }
    *Elements[idx] = val;
}

void ArrayObject::Reverse()
{
    int i = 0;
    int j = (int)Elements.GetSize() - 1;
    while(i < j)
    {
        Alg::Swap(Elements[i], Elements[j]);
        ++i;
        --j;
    }
}

void ArrayObject::ArrayConcat(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Array);
    ArrayObject* pThis = (ArrayObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    pThis->LengthValueOverriden = false;
    Ptr<ArrayObject> ao = *static_cast<ArrayObject*>
        (fn.Env->OperatorNew(fn.Env->GetBuiltin(ASBuiltin_Array)));
    if (ao)
    {
        ao->Concat(fn.Env, pThis);
        for(int i = 0; i < fn.NArgs; i++)
        {
            ao->Concat(fn.Env, fn.Arg(i));
        }
    }
    fn.Result->SetAsObject(ao.GetPtr());
}

void ArrayObject::ArrayJoin(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Array);
    ArrayObject* pThis = (ArrayObject*) fn.ThisPtr;
    SF_ASSERT(pThis);    

    pThis->LengthValueOverriden = false;
    StringBuffer sbuffer(fn.Env->GetHeap());

    if (fn.NArgs == 0)
    {        
        pThis->JoinToString(fn.Env,& sbuffer, ",");
    }
    else
    {
        ASString sa0(fn.Arg(0).ToString(fn.Env));
        pThis->JoinToString(fn.Env, &sbuffer, sa0.ToCStr());
    }    
    fn.Result->SetString(fn.Env->CreateString(sbuffer.ToCStr(), sbuffer.GetSize()));
}

void ArrayObject::ArrayPop(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Array);
    ArrayObject* pThis = (ArrayObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    pThis->LengthValueOverriden = false;
    if(pThis->GetSize())
    {
        Value* pVal = pThis->GetElementPtr(pThis->GetSize() - 1);
        if(pVal) *fn.Result = *pVal;
        else      fn.Result->SetUndefined();
        pThis->PopBack();
    }
    else
    {
        fn.Result->SetUndefined();
    }
}

void ArrayObject::ArrayPush(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Array);
    ArrayObject* pThis = (ArrayObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    pThis->LengthValueOverriden = false;
    for(int i = 0; i < fn.NArgs; i++)
    {
        pThis->PushBack(fn.Arg(i));
    }
    fn.Result->SetInt(pThis->GetSize());
}

void ArrayObject::ArrayReverse(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Array);
    ArrayObject* pThis = (ArrayObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    pThis->LengthValueOverriden = false;
    pThis->Reverse();

    // The documentation says that "reverse" returns Void, but
    // actually Flash 8 returns self.
    fn.Result->SetAsObject(pThis);
}

void ArrayObject::ArrayShift(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Array);
    ArrayObject* pThis = (ArrayObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    if(pThis->GetSize() == 0)
    {
        fn.Result->SetUndefined();
        return;
    }
    
    pThis->LengthValueOverriden = false;
    Value* pVal = pThis->GetElementPtr(0);
    if(pVal) *fn.Result = *pVal;
    else      fn.Result->SetUndefined();
    pThis->PopFront();
}

void ArrayObject::ArraySlice(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Array);
    ArrayObject* pThis = (ArrayObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    pThis->LengthValueOverriden = false;
    int start = 0;
    int end   = pThis->GetSize();
    if (fn.NArgs >= 1)
    {
        start = fn.Arg(0).ToInt32(fn.Env);
        if(start < 0) start += pThis->GetSize();
        if(start < 0) start = 0;
        if(start > pThis->GetSize()) start = pThis->GetSize();
    }
    if (fn.NArgs >= 2)
    {
        end = fn.Arg(1).ToInt32(fn.Env);
        if(end < 0) end += pThis->GetSize();
        if(end < 0) end = 0;
        if(end > pThis->GetSize()) end = pThis->GetSize();
    }
    
    //Ptr<ArrayObject> ao = *SF_HEAP_NEW(fn.Env->GetHeap()) ArrayObject(fn.Env);
    Ptr<ArrayObject> ao = *static_cast<ArrayObject*>
                              (fn.Env->OperatorNew(fn.Env->GetBuiltin(ASBuiltin_Array)));
    if (ao)
    {
        for(int i = start; i < end; i++)
        {
            if(pThis->GetElementPtr(i) != 0) 
            {
                ao->PushBack(*pThis->GetElementPtr(i));
            }
            else
            {
                ao->PushBack();
            }
        }
    }
    fn.Result->SetAsObject(ao.GetPtr());
}

void ArrayObject::ArraySort(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Array);
    ArrayObject* pThis = (ArrayObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    pThis->LengthValueOverriden = false;
    FunctionRef scriptFunctor;
    int  flags = 0;
    if(fn.NArgs >= 1)
    {
        if(fn.Arg(0).IsFunction()) scriptFunctor = fn.Arg(0).ToFunction(fn.Env);
        else flags = fn.Arg(0).ToInt32(fn.Env);
    }

    if(scriptFunctor != NULL && fn.NArgs >= 2)
    {
        flags = fn.Arg(1).ToInt32(fn.Env);
    }

    //Ptr<ArrayObject> retArray = *SF_HEAP_NEW(fn.Env->GetHeap()) ArrayObject(fn.Env);
    Ptr<ArrayObject> retArray = *static_cast<ArrayObject*>
                                    (fn.Env->OperatorNew(fn.Env->GetBuiltin(ASBuiltin_Array)));
    if (retArray)
    {
        retArray->ShallowCopyFrom(*pThis);

        ArraySortFunctor sortFunctor(retArray, 
                                        flags, 
                                        scriptFunctor, 
                                        fn.Env, 
                                        pThis->GetLogPtr());
        if (!retArray->Sort(sortFunctor))
        {
            // returned false: most likely sortFunctor is incorrect
            fn.Env->LogScriptError("Array.sort - sorting failed, check your sort functor");
        }

        bool uniqueSortFailed = false;
        if(flags & ArrayObject::SortFlags_UniqueSort)
        {
            int i;
            for(i = 1; i < retArray->GetSize(); i++)
            {
                if(sortFunctor.Compare(retArray->GetElementPtr(i-1), 
                                       retArray->GetElementPtr(i)) == 0) 
                {
                    uniqueSortFailed = true;
                    break;
                }
            }
        }

        if(uniqueSortFailed)
        {
            fn.Result->SetInt(0);
            retArray->DetachAll();
            return;
        }

        if(flags & ArrayObject::SortFlags_ReturnIndexedArray)
        {
            retArray->MakeDeepCopy(fn.Env->GetHeap());
            fn.Result->SetAsObject(retArray);
        }
        else
        {
            pThis->ShallowCopyFrom(*retArray);
            retArray->DetachAll();
            fn.Result->SetAsObject(pThis);
        }
    }
}

void ArrayObject::ArraySortOn(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Array);
    ArrayObject* pThis = (ArrayObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    // GASString array requires a default value, since it only uses copy constructors.
    typedef  ArrayCC<ASString, StatMV_ActionScript_Mem> GASStringArray;

    ScopePtr<GASStringArray>   pfieldNames(SF_HEAP_NEW(fn.Env->GetHeap()) 
                                    GASStringArray(fn.Env->GetBuiltin(ASBuiltin_empty_)));

    GASStringArray                  &fieldNames(*pfieldNames);
    Array<int>                     fieldFlags;
    int                             commonFlags = 0;

    if(fn.NArgs == 0)
    {
        fn.Result->SetUndefined();
        return;
    }

    pThis->LengthValueOverriden = false;
    int       i;
    ASString dummyName(fn.Env->GetBuiltin(ASBuiltin_undefined_));

    if(fn.NArgs >= 1)
    {
        // *** Create an array of field names

        Object* fields = fn.Arg(0).ToObject(fn.Env);
        if(fields && fields->GetObjectType() == ObjectInterface::Object_Array)
        {
            ArrayObject* pFieldArray = (ArrayObject*)fields;
            for(i = 0; i < pFieldArray->GetSize(); i++)
            {
                const Value* pName = pFieldArray->GetElementPtr(i);

                // If by accident there's no field name defined use some name
                // that the function GetMember() will return 'false' for sure.
                // This field will simply be skipped.
                if (pName)
                    fieldNames.PushBack(pName->ToString(fn.Env));
                else
                    fieldNames.PushBack(dummyName);
            }
        }
        else
        {
            fieldNames.PushBack(fn.Arg(0).ToString(fn.Env));
        }
    }

    for(i = 0; i < (int)fieldNames.GetSize(); i++)
    {
        fieldFlags.PushBack(0);
    }

    if(fn.NArgs >= 2)
    {
        // *** Create an array of flags

        Object* flags = fn.Arg(1).ToObject(fn.Env);
        if(flags && flags->GetObjectType() == ObjectInterface::Object_Array)
        {
            ArrayObject* pFlagsArray = (ArrayObject*)flags;
            for(i = 0; i < pFlagsArray->GetSize() && 
                       i < (int)fieldNames.GetSize(); i++)
            {
                const Value* pFlags = pFlagsArray->GetElementPtr(i);
                if(pFlags)
                {
                    fieldFlags[i] = pFlags->ToInt32(fn.Env);
                }
            }
        }
        else
        {
            commonFlags = fn.Arg(1).ToInt32(fn.Env);
            for(i = 0; i < (int)fieldNames.GetSize(); i++)
            {
                fieldFlags[i] = commonFlags;
            }
        }
    }

    //Ptr<ArrayObject> retArray = *SF_HEAP_NEW(fn.Env->GetHeap()) ArrayObject(fn.Env);
    Ptr<ArrayObject> retArray = *static_cast<ArrayObject*>
                                    (fn.Env->OperatorNew(fn.Env->GetBuiltin(ASBuiltin_Array)));
    if (retArray)
    {
        retArray->ShallowCopyFrom(*pThis);

        ArraySortOnFunctor sortFunctor(retArray, 
                                          fieldNames, 
                                          fieldFlags, 
                                          fn.Env, 
                                          pThis->GetLogPtr());
        retArray->Sort(sortFunctor);

        bool uniqueSortFailed = false;
        if(commonFlags & ArrayObject::SortFlags_UniqueSort)
        {
            int igs;
            for(igs = 1; igs < retArray->GetSize(); igs++)
            {
                if(sortFunctor.Compare(retArray->GetElementPtr(igs-1), retArray->GetElementPtr(igs)) == 0)
                {
                    uniqueSortFailed = true;
                    break;
                }
            }
        }

        if(uniqueSortFailed)
        {
            fn.Result->SetInt(0);
            return;
        }

        if(commonFlags & ArrayObject::SortFlags_ReturnIndexedArray)
        {
            retArray->MakeDeepCopy(fn.Env->GetHeap());
            fn.Result->SetAsObject(retArray);
        }
        else
        {
            pThis->ShallowCopyFrom(*retArray);
            retArray->DetachAll();
            fn.Result->SetAsObject(pThis);
        }
    }
}

void ArrayObject::ArraySplice(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Array);
    ArrayObject* pThis = (ArrayObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    if (fn.NArgs == 0)
    {
        fn.Result->SetUndefined();
        return;
    }

    pThis->LengthValueOverriden = false;
    int start = fn.Arg(0).ToInt32(fn.Env);
    if(start < 0) start += pThis->GetSize();
    if(start < 0) start = 0;
    if(start > pThis->GetSize()) start = pThis->GetSize();

    int count = pThis->GetSize() - start;
    if (fn.NArgs >= 2)
    {
        count = fn.Arg(1).ToInt32(fn.Env);
        if(count < 0) count = 0;
        if(start + count >= pThis->GetSize()) 
        {
            count = pThis->GetSize() - start;
        }
    }

    int i;
    //Ptr<ArrayObject> ao = *SF_HEAP_NEW(fn.Env->GetHeap()) ArrayObject(fn.Env);
    Ptr<ArrayObject> ao = *static_cast<ArrayObject*>
                              (fn.Env->OperatorNew(fn.Env->GetBuiltin(ASBuiltin_Array)));
    if (ao)
    {
        for(i = 0; i < count; i++)
        {
            if(pThis->GetElementPtr(start + i) != 0) 
            {
                ao->PushBack(*pThis->GetElementPtr(start + i));
            }
            else
            {
                ao->PushBack();
            }
        }
        fn.Result->SetAsObject(ao.GetPtr());

        if(count)
        {
            pThis->RemoveElements(start, count);
        }

        if (fn.NArgs >= 3)
        {
            pThis->InsertEmpty(start, fn.NArgs - 2);
            for(i = 2; i < fn.NArgs; i++)
            {
                pThis->SetElement(start + i - 2, fn.Arg(i));
            }
        }
    }
}

void ArrayObject::ArrayToString(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Array);
    ArrayObject* pThis = (ArrayObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    RecursionGuard rg(pThis);
    if(pThis->RecursionLimitReached()) 
    {
        fn.Result->SetString(fn.Env->GetBuiltin(ASBuiltin_empty_));
    }
    else
    {
        StringBuffer sbuffer(fn.Env->GetHeap());
        pThis->JoinToString(fn.Env, &sbuffer, ",");

        fn.Result->SetString(fn.Env->CreateString(sbuffer.ToCStr(), sbuffer.GetSize()));
    }
}

void ArrayObject::ArrayUnshift(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Array);
    ArrayObject* pThis = (ArrayObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    pThis->LengthValueOverriden = false;
    if(fn.NArgs > 0)
    {
        pThis->InsertEmpty(0, fn.NArgs);
        for(int i = 0; i < fn.NArgs; i++)
        {
            pThis->SetElement(i, fn.Arg(i));
        }
    }
    fn.Result->SetInt(pThis->GetSize());
}

void ArrayObject::ArrayLength(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, Array);
    ArrayObject* pThis = (ArrayObject*) fn.ThisPtr;
    SF_ASSERT(pThis);

    fn.Result->SetInt(pThis->GetSize());
}

void ArrayObject::ArrayValueOf(const FnCall& fn)
{
    // looks like Array.valueOf returns same value as .toString.
    ArrayToString (fn);
}

static const NameNumber GASArrayConstTable[] = 
{
    { "CASEINSENSITIVE",    ArrayObject::SortFlags_CaseInsensitive },
    { "DESCENDING",         ArrayObject::SortFlags_Descending },
    { "LOCALE",             ArrayObject::SortFlags_Locale },
    { "NUMERIC",            ArrayObject::SortFlags_Numeric },
    { "RETURNINDEXEDARRAY", ArrayObject::SortFlags_ReturnIndexedArray },
    { "UNIQUESORT",         ArrayObject::SortFlags_UniqueSort },
    { 0, 0 }
};

// Must be sorted!
static const NameFunction GASArrayFunctionTable[] = 
{
    { "concat",             &ArrayObject::ArrayConcat     },
    { "join",               &ArrayObject::ArrayJoin       },
    { "pop",                &ArrayObject::ArrayPop        },
    { "push",               &ArrayObject::ArrayPush       },
    { "reverse",            &ArrayObject::ArrayReverse    },
    { "shift",              &ArrayObject::ArrayShift      },
    { "slice",              &ArrayObject::ArraySlice      },
    { "sort",               &ArrayObject::ArraySort       },
    { "sortOn",             &ArrayObject::ArraySortOn     },
    { "splice",             &ArrayObject::ArraySplice     },
    { "toString",           &ArrayObject::ArrayToString   },
    { "unshift",            &ArrayObject::ArrayUnshift    },
    { "valueOf",            &ArrayObject::ArrayValueOf    },
    { 0, 0 }
};




// TODO: Check this function for possible incompatibility
// with Flash. The behavior of Flash is somewhat strange:
//
// var a = new Array; 
// var b = new Array; 
// a["100"]  = 100; trace(a.length + " " + a[100]); // Output: 101 100
// b[" 100"] = 100; trace(b.length + " " + b[100]); // Output: 101 undefined
//
// That is, an index with leading spaces affects the size, but
// does not create an element. This function is more strict, that is, 
// if  'name' is not a non-negative number (leading spaces are 
// considered as not a number) it returns -1, so that, in any suspicious
// case the standard "member" mechanism works for the array.
// 
// The UTF-8 strings should work correctly: There ain't no digits in UNICODE
// other than ASCII 0...9, are there? And in the UTF-8 encoding, as soon
// as the most significant bit is set (multibyte character) the function will 
// return -1 right away.
//---------------------------------------------------------------------------
int ArrayObject::ParseIndex(const ASString& name)
{
    const char* p = name.ToCStr();
    while(*p && *p >= '0' && *p <= '9') 
    {
        p++;
    }
    if(*p) return -1;
    return atoi(name.ToCStr());
}

// Overridden Object::SetMember
bool ArrayObject::SetMember(Environment* penv, const ASString& name, const Value& val, const PropFlags& flags)
{
    int idx;
    if (penv->GetBuiltin(ASBuiltin_length).CompareBuiltIn_CaseCheck(name, penv->IsCaseSensitive()))        
    {
        int nlen = val.ToInt32(0);
        Resize((nlen < 0) ? 0 : nlen);   // don't resize to negative length, but...
        LengthValueOverriden = true;
        return Object::SetMember(penv, name, val, flags); // store the original length's value in members.
    }
    else if((idx = ParseIndex(name)) >= 0)
    {
        LengthValueOverriden = false;
        if(idx >= (int)Elements.GetSize())
        {
            Resize(idx + 1);
        }
        if(Elements[idx] == 0)
        {
            Elements[idx] = SF_HEAP_NEW(penv->GetHeap()) Value();
        }
        *Elements[idx] = val;
        return true;
    }
    else
    {
        return Object::SetMember(penv, name, val, flags);
    }
}




// Overridden Object::GetMemberRaw
bool ArrayObject::GetMemberRaw(ASStringContext *psc, const ASString& name, Value* val)
{
    int idx;
    if((idx = ParseIndex(name)) >= 0)
    {
        if(idx >= (int)Elements.GetSize() || Elements[idx] == 0)
        {
            val->SetUndefined();
            return true;
        }
        *val = *Elements[idx];
        return true;
    }
    else

    // if (name == "length")
    if (psc->GetBuiltin(ASBuiltin_length).CompareBuiltIn_CaseCheck(name, psc->IsCaseSensitive()))
    {
        if (!LengthValueOverriden || Elements.GetSize() != 0)
        {
            val->SetInt((int)Elements.GetSize());
            LengthValueOverriden = false;
            return true;
        }
        // if Elements.size () == 0 && HasMember ("length") then 
        // report length as it set. For example, "length" may be set to
        // negative value and it should be reported as negative value
        // while the actual size of array is 0.
    }
    return Object::GetMemberRaw(psc, name, val);
}

bool ArrayObject::DeleteMember(ASStringContext *psc, const ASString& name)
{
    // special case for delete array[n]: in this case name is 
    // an index in array to be removed. Item at the index becomes
    // "undefined" after removing.
    if (name.GetSize() > 0 && isdigit (name[0]))
    {
        int index;
        if((index = ParseIndex(name)) >= 0)
        {
            SetElement(index, Value());
            return true;
        }
        return false;
    }
    return Object::DeleteMember(psc, name);
}

void ArrayObject::VisitMembers(ASStringContext *psc, MemberVisitor *pvisitor, unsigned visitFlags, const ObjectInterface* instance) const
{
    // invoke base class method first
    // for .. in operator puts all elements in stack first
    // and then pops them up one by one. So, after popping elements from the stack
    // the last array element will be first, and after all elements all other 
    // properties will be iterated (if not hidden)
    Object::VisitMembers(psc, pvisitor, visitFlags, instance);

    // iterate through array's elements first...
    UPInt i, n = Elements.GetSize();
    UPInt n1 = (n > ASBuiltinConst_DigitMax_) ? (ASBuiltinConst_DigitMax_+1) : n;

    //char buffer[20];

    for(i = 0; i < n1; ++i)
    {
        if (!Elements[i]) continue;      
        pvisitor->Visit(psc->GetBuiltin((ASBuiltinType)(ASBuiltin_0 + i)), *Elements[i], 0);
    }
    for(; i < n; ++i)
    {
        if (!Elements[i]) continue;
        
        LongFormatter f(i);
        f.Convert();
        StringDataPtr result = f.GetResult();

        pvisitor->Visit(psc->CreateString(result.ToCStr(), result.GetSize()), *Elements[i], 0);
    }
}

// Overridden Object::GetTextValue
const char* ArrayObject::GetTextValue(Environment* pEnv) const
{
    RecursionGuard rg(this);
    if (RecursionLimitReached()) return "";

    StringBuffer sbuffer(pEnv->GetHeap());
    JoinToString(pEnv, &sbuffer, ",");
    StringValue = sbuffer;
    return StringValue.ToCStr();
}

// Overridden Object::GetObjectType
ObjectInterface::ObjectType  ArrayObject::GetObjectType() const
{
    return Object_Array;
}

void ArrayObject::InitArray(const FnCall& fn)
{
    // Use the arguments as initializers.
    Value    IndexNumber;
    for (int i = 0; i < fn.NArgs; i++)
    {
        IndexNumber.SetInt(i);
        SetMember(fn.Env, IndexNumber.ToString(fn.Env), fn.Arg(i));
    }
}

bool ArrayObject::HasMember(ASStringContext *psc, const ASString& name, bool inclPrototypes)
{
    int idx;
    if((idx = ParseIndex(name)) >= 0)
    {
        if (idx >= (int)Elements.GetSize() || Elements[idx] == NULL || 
            Elements[idx]->IsUndefined() || Elements[idx]->IsNull())
            return false;
        return true;
    }
    return Object::HasMember(psc, name, inclPrototypes);
}

//////////////////////////
ArrayProto::ArrayProto(ASStringContext *psc, Object* pprototype, const FunctionRef& constructor) : 
    Prototype<ArrayObject>(psc, pprototype, constructor)
{
    InitFunctionMembers(psc, GASArrayFunctionTable);
}

ArrayCtorFunction::ArrayCtorFunction(ASStringContext *psc) :
    CFunctionObject (psc, GlobalCtor)
{
    for(int i = 0; GASArrayConstTable[i].Name; i++)
    {
        SetConstMemberRaw(psc, GASArrayConstTable[i].Name, GASArrayConstTable[i].Number, 
            PropFlags::PropFlag_DontDelete | PropFlags::PropFlag_DontEnum);
    }
}

void ArrayCtorFunction::GlobalCtor(const FnCall& fn)
{
    Ptr<ArrayObject> ao;
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object::Object_Array && 
        !fn.ThisPtr->IsBuiltinPrototype())
    {
        ao = static_cast<ArrayObject*>(fn.ThisPtr);
    }
    else
        ao = *SF_HEAP_NEW(fn.Env->GetHeap()) ArrayObject(fn.Env);
    ao->SetMember(fn.Env, fn.Env->GetBuiltin(ASBuiltin_length),
        Value(int(0)), PropFlags::PropFlag_DontEnum);

    if (fn.NArgs == 0)
    {
    }
    else if (fn.NArgs == 1 && fn.Arg(0).IsNumber())
    {
        ao->Resize(fn.Arg(0).ToInt32(fn.Env));
    }
    else
    {
        // Use the arguments as initializers.
        ao->InitArray (fn);
    }

    fn.Result->SetAsObject(ao.GetPtr());
}

void ArrayCtorFunction::DeclareArray(const FnCall& fn)
{
    Ptr<ArrayObject> ao = *SF_HEAP_NEW(fn.Env->GetHeap()) ArrayObject(fn.Env);
    ao->SetMember(fn.Env, fn.Env->GetBuiltin(ASBuiltin_length),
        Value(int(0)), PropFlags::PropFlag_DontEnum);

    // declared array has a "constructor" property and doesn't have "__constructor__"
    FunctionRef ctor = fn.Env->GetConstructor(ASBuiltin_Array);
    ao->Set_constructor(fn.Env->GetSC(), ctor);

    if (fn.NArgs == 0)
    {
    }
    else
    {
        // Use the arguments as initializers.
        ao->InitArray (fn);
    }

    fn.Result->SetAsObject(ao.GetPtr());
}

Object* ArrayCtorFunction::CreateNewObject(Environment* penv) const
{
    return SF_HEAP_NEW(penv->GetHeap()) ArrayObject(penv);
}

FunctionRef ArrayCtorFunction::Register(GlobalContext* pgc)
{
    ASStringContext sc(pgc, 8);
    FunctionRef  ctor(*SF_HEAP_NEW(pgc->GetHeap()) ArrayCtorFunction(&sc));
    Ptr<Object> proto = *SF_HEAP_NEW(pgc->GetHeap()) ArrayProto(&sc, pgc->GetPrototype(ASBuiltin_Object), ctor);
    pgc->SetPrototype(ASBuiltin_Array, proto);
    pgc->pGlobal->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_Array), Value(ctor));
    return ctor;
}
}}} // SF::GFx::AS2
