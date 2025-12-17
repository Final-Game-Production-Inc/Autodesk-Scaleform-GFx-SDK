/**************************************************************************

Filename    :   AS2_Object.cpp
Content     :   ActionScript Object implementation classes
Created     :   
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/AS2/AS2_Action.h"
#include "GFx/AS2/AS2_AvmSprite.h"
#include "GFx/AS2/AS2_AsFunctionObject.h"
#include "GFx/AS2/AS2_MovieRoot.h"
#include "GFx/AMP/Amp_Server.h"

namespace Scaleform { namespace GFx { namespace AS2 {

// **** Object Interface implementation

ObjectInterface::ObjectInterface()
#ifdef GFX_AS_ENABLE_USERDATA
: pUserDataHolder(0)
#endif
{

}

ObjectInterface::~ObjectInterface()
{
#ifdef GFX_AS_ENABLE_USERDATA
    if (pUserDataHolder)
    {
        pUserDataHolder->NotifyDestroy(this);
        delete pUserDataHolder;
    }
#endif  // GFX_AS_ENABLE_USERDATA
}

InteractiveObject* ObjectInterface::ToCharacter()
{
    return IsASCharacter() ? static_cast<AvmCharacter*>(this)->GetDispObj() : 0;
}

AvmCharacter* ObjectInterface::ToAvmCharacter()
{
    return IsASCharacter() ? static_cast<AvmCharacter*>(this) : 0;
}

Object* ObjectInterface::ToASObject()
{
    return IsASObject() ? static_cast<Object*>(this) : 0;
}

Sprite*      ObjectInterface::ToSprite()
{
    return IsSprite() ? static_cast<AvmSprite*>(this)->GetSprite() : 0;
}

AvmSprite*      ObjectInterface::ToAvmSprite()
{
    return IsSprite() ? static_cast<AvmSprite*>(this) : 0;
}

const InteractiveObject* ObjectInterface::ToCharacter() const 
{
    return IsASCharacter() ? static_cast<const AvmCharacter*>(this)->GetDispObj() : 0;
}

const AvmCharacter* ObjectInterface::ToAvmCharacter() const 
{
    return IsASCharacter() ? static_cast<const AvmCharacter*>(this) : 0;
}

const Object* ObjectInterface::ToASObject() const
{
    return IsASObject() ? static_cast<const Object*>(this) : 0;
}

const Sprite*        ObjectInterface::ToSprite() const 
{
    return IsSprite() ? static_cast<const AvmSprite*>(this)->GetSprite() : 0;
}

const AvmSprite*        ObjectInterface::ToAvmSprite() const 
{
    return IsSprite() ? static_cast<const AvmSprite*>(this) : 0;
}

FunctionRef  ObjectInterface::ToFunction()
{
    return 0;
}

Object* ObjectInterface::FindOwner(ASStringContext *psc, const ASString& name)
{
    for (ObjectInterface* proto = this; proto != 0; proto = proto->Get__proto__())
    {
        if (proto->HasMember(psc, name, false)) 
            return static_cast<Object*>(proto);
    }
    return NULL;
}

#ifdef GFX_AS_ENABLE_USERDATA

void    ObjectInterface::SetUserData(Movie* pmovieView, ASUserData* puserData, bool isdobj)
{
    if (pUserDataHolder)
    {
        pUserDataHolder->NotifyDestroy(this);
        delete pUserDataHolder;
    }

    pUserDataHolder = SF_HEAP_AUTO_NEW(this) UserDataHolder(pmovieView, puserData);

    if (puserData)
    {
        MovieImpl* pmovieImpl = static_cast<MovieImpl*>(pmovieView);
        if (isdobj)
        {
            InteractiveObject* aschar = ToCharacter();
            puserData->SetLastObjectValue(pmovieImpl->pObjectInterface, aschar->GetCharacterHandle(), isdobj);
        }
        else
            puserData->SetLastObjectValue(pmovieImpl->pObjectInterface, this, isdobj);
    }
}

#endif  // GFX_AS_ENABLE_USERDATA


//
// ***** Object Implementation
//
Object::Object(ASRefCountCollector *pcc)
: ASRefCountBase<Object>(pcc), pWatchpoints(0), ArePropertiesSet(false)
{
    // NOTE: psc is potentially NULL here.
    Init();
    pProto = 0;
}

Object::Object(ASStringContext *psc) 
: ASRefCountBase<Object>(psc->GetGC()), pWatchpoints(0), ArePropertiesSet(false)
{
    // NOTE: psc is potentially NULL here.
    SF_UNUSED(psc);
    Init();
    pProto = 0;//*SF_HEAP_NEW() ObjectProto ();
}

Object::Object(ASStringContext *psc, Object* proto) 
: ASRefCountBase<Object>(psc->GetGC()), pWatchpoints(0), ArePropertiesSet(false)
{
    Init();
    Set__proto__(psc, proto);
}

Object::Object (Environment* penv) 
: ASRefCountBase<Object>(penv->GetCollector()), pWatchpoints(0), ArePropertiesSet(false)
{
    Init();    
    Set__proto__(penv->GetSC(), penv->GetPrototype(ASBuiltin_Object));
}

Object::~Object()
{ 
    if (pWatchpoints)
        delete pWatchpoints;
}

void Object::Init()
{
    IsListenerSet = false; // See comments in Value.cpp, CheckForListenersMemLeak.
}

#ifdef GFX_AS_ENABLE_GC
template <class Functor>
void Object::ForEachChild_GC(Collector* prcc) const
{
    MemberHash::ConstIterator it = Members.Begin();
    while(it != Members.End())
    {   
        const Value& value = it->Second.GetMemberValue();
        value.template ForEachChild_GC<Functor>(prcc);
        ++it;
    }
    ResolveHandler.template ForEachChild_GC<Functor>(prcc);
    if (pWatchpoints)
    {
        WatchpointHash::ConstIterator wpIt = pWatchpoints->Begin();
        while(wpIt != pWatchpoints->End())
        {   
            const Watchpoint& value = wpIt->Second;
            value.template ForEachChild_GC<Functor>(prcc);
            ++wpIt;
        }
    }
    if (pProto)
    {
        Functor::Call(prcc, pProto);
    }
}
SF_GC_PREGEN_FOR_EACH_CHILD(Object)

void Object::ExecuteForEachChild_GC(Collector* prcc, OperationGC operation) const
{
    ASRefCountBaseType::CallForEachChild<Object>(prcc, operation);
}

void Object::Finalize_GC()
{
    Members.~MemberHash();
    ResolveHandler.Finalize_GC();
    if (pWatchpoints)
    {
        pWatchpoints->~WatchpointHash();
        SF_FREE(pWatchpoints);
    }

#ifdef GFX_AS_ENABLE_USERDATA
    if (pUserDataHolder)
    {
        pUserDataHolder->NotifyDestroy(this);
        delete pUserDataHolder;
    }
#endif  // GFX_AS_ENABLE_USERDATA

}
#endif //SF_NO_GC

void Object::SetValue(Environment* penv, const Value& val)
{
    SF_UNUSED2(penv, val);
}

Value Object::GetValue() const
{
    return Value();
}

bool Object::SetMember(Environment* penv, const ASString& name, const Value& val, const PropFlags& flags)
{
    static const Value notsetVal(Value::UNSET);
    PropFlags _flags;

    MemberHash::Iterator it = Members.FindCaseCheck(name, penv->IsCaseSensitive());
    Member*           pmember;
    if (it == Members.End())
    {
        // Need to check whole prototypes' chain in the case if this is a property set into
        // the prototype.
        for (Object* pproto = Get__proto__(); pproto != 0; pproto = pproto->Get__proto__())
        {
            if (pproto->ArePropertiesSet)
            {
                MemberHash::Iterator protoit = pproto->Members.FindCaseCheck(name, penv->IsCaseSensitive());
                if (protoit != pproto->Members.End())
                {
                    if (protoit->Second.mValue.IsProperty())
                        it = protoit;
                    break;
                }
            }
        }
    }
    if (it != Members.End())
    {
        pmember = &(*it).Second;
        _flags = pmember->GetMemberFlags();
        // Is the member read-only ?
        if (_flags.GetReadOnly())
            return false;
        if (pmember->mValue.IsProperty())
        {
            Value propVal = pmember->mValue;
            // check, is the object 'this' came from InteractiveObject. If so,
            // use original InteractiveObject as this.
            Ptr<InteractiveObject> paschar = GetASCharacter();
            if (paschar)
                propVal.SetPropertyValue(penv, GFx::AS2::ToAvmCharacter(paschar), val);
            else
                propVal.SetPropertyValue(penv, this, val);
            return true;
        }
        // Is the member read-only ?
        if (_flags.GetReadOnly())
            return false;
    }
    else
    {
        _flags = flags;
        pmember = 0;
    }

    Value        newVal;
    const Value *pval = &val;

    if (penv && pWatchpoints) // have set any watchpoints?
    {
        if (InvokeWatchpoint(penv, name, val, &newVal))
        {
            pval = &newVal;
        }
    }

    if (!pmember)
    {
        return SetMemberRaw(penv->GetSC(), name, *pval, _flags);
    }
    else
    {
        // Check is the value property or not. If yes - set ArePropertiesSet flag to true
        if (val.IsProperty())
            ArePropertiesSet = true;

        if (penv->IsCaseSensitive())
        {
            if (name == penv->GetBuiltin(ASBuiltin___proto__))
            {   
                if (val.IsSet())
                {
                    Set__proto__(penv->GetSC(), val.ToObject(penv));

                    // Invalidate optimized advance list, since setting __proto__
                    // may turn on/off onEnterFrame events.
                    penv->InvalidateOptAdvanceList();
                }
                pval = &notsetVal;
            }
            else if (name == penv->GetBuiltin(ASBuiltin___resolve))
            {   
                if (val.IsSet())
                    ResolveHandler = val.ToFunction(penv);
                pval = &notsetVal;
            }
            else if (name == penv->GetBuiltin(ASBuiltin_onEnterFrame))
            {
                // Invalidate optimized advance list.
                penv->InvalidateOptAdvanceList();
            }
        }
        else
        {
            name.ResolveLowercase();
            if (penv->GetBuiltin(ASBuiltin___proto__).CompareBuiltIn_CaseInsensitive_Unchecked(name))
            {   
                if (val.IsSet())
                {
                    Set__proto__(penv->GetSC(), val.ToObject(penv));

                    // Invalidate optimized advance list, since setting __proto__
                    // may turn on/off onEnterFrame events.
                    penv->InvalidateOptAdvanceList();
                }
                pval = &notsetVal;
            }
            else if (penv->GetBuiltin(ASBuiltin___resolve).CompareBuiltIn_CaseInsensitive_Unchecked(name))
            {   
                if (val.IsSet())
                    ResolveHandler = val.ToFunction(penv);
                pval = &notsetVal;
            }
            else if (name == penv->GetBuiltin(ASBuiltin_onEnterFrame))
            {
                // Invalidate optimized advance list.
                penv->InvalidateOptAdvanceList();
            }
        }
        pmember->mValue = *pval;
    }
    return true;
}

static const Value   notsetVal(Value::UNSET);

bool Object::SetMemberRaw(ASStringContext *psc, const ASString& name, const Value& val, const PropFlags& flags)
{
    const Value*         pval = &val;
    MemberHash::Iterator    it;

    // Hack for Tween class memory leak prevention. See comments in Value.cpp, CheckForListenersMemLeak.
    // This is also necessary for MovieClipLoader since it adds itself as a listener.
    if (!IsListenerSet && val.IsObject() && 
        (name == psc->GetBuiltin(ASBuiltin__listeners)))
    {
        Object *pobject = val.ToObject(NULL);
        if (pobject && (pobject->GetObjectType() == Object_Array))
            IsListenerSet = true;
    }

    // For efficiency, do extra case-sensitivity branch.
    if (psc->IsCaseSensitive())
    {
        if (name == psc->GetBuiltin(ASBuiltin___proto__))
        {   
            if (val.IsSet())
            {
                Set__proto__(psc, val.ToObject(NULL));

                // Invalidate optimized advance list, since setting __proto__
                // may turn on/off onEnterFrame events.
                psc->InvalidateOptAdvanceList();
            }
            pval = &notsetVal;
        }
        else if (name == psc->GetBuiltin(ASBuiltin___resolve))
        {   
            if (val.IsSet())
                ResolveHandler = val.ToFunction(NULL);
            pval = &notsetVal;
        }
        else if (name == psc->GetBuiltin(ASBuiltin_onEnterFrame))
        {
            // Invalidate optimized advance list.
            psc->InvalidateOptAdvanceList();
        }
        // Find member case-sensitively.
        it = Members.Find(name);
    }
    else
    {
        name.ResolveLowercase();
        if (psc->GetBuiltin(ASBuiltin___proto__).CompareBuiltIn_CaseInsensitive_Unchecked(name))
        {   
            if (val.IsSet())
            {
                Set__proto__(psc, val.ToObject(NULL));

                // Invalidate optimized advance list, since setting __proto__
                // may turn on/off onEnterFrame events.
                psc->InvalidateOptAdvanceList();
            }
            pval = &notsetVal;
        }
        else if (psc->GetBuiltin(ASBuiltin___resolve).CompareBuiltIn_CaseInsensitive_Unchecked(name))
        {   
            if (val.IsSet())
                ResolveHandler = val.ToFunction(NULL);
            pval = &notsetVal;
        }
        else if (name == psc->GetBuiltin(ASBuiltin_onEnterFrame))
        {
            // Invalidate optimized advance list.
            psc->InvalidateOptAdvanceList();
        }
        // Find member case-insensitively.
        it = Members.FindCaseInsensitive(name);
    }

    // Check is the value property or not. If yes - set ArePropertiesSet flag to true
    if (val.IsProperty())
        ArePropertiesSet = true;
    
#ifdef SF_AMP_SERVER
    if (AmpServer::GetInstance().IsEnabled() && !name.IsEmpty() && 
        AmpServer::GetInstance().GetProfileLevel() >= Amp_Profile_Level_ActionScript)
    {
        FunctionRef fn = pval->ToFunction(NULL);
        if (!fn.IsNull())
        {
            FunctionObject* fo = fn.GetObjectPtr();
            if (fo->IsAsFunction())
            {
                AsFunctionObject* asfo = static_cast<AsFunctionObject*>(fo);
                if (asfo->GetLength() > 0)
                {
                    const ActionBufferData* bufData = asfo->GetActionBuffer()->GetBufferData();

                    MovieImpl* pRoot = psc->pContext->GetAS2Root()->GetMovieImpl();
                    pRoot->GetAdvanceStats().RegisterScriptFunction(bufData->GetSwdHandle(), 
                        bufData->GetSWFFileOffset() + asfo->GetStartPC(), 
                        name.ToCStr(), asfo->GetLength(), 2, false);
                }
            }
        }
    }
#endif  // SF_AMP_SERVER

    if (it != Members.End())
    {   
        // This matches correct Flash behavior for both case sensitive and insensitive versions.
        // I.E. for case insensitive strings the identifier name is not overwritten, retaining
        // the original capitalization it had before assignment.
        Member* pmember = &(*it).Second;
        SF_ASSERT(pmember);
        pmember->mValue = *pval;
    }
    else
    {
        // Since object wasn't there, no need to check capitalization again.
        Members.Set(name, Member(*pval, flags));
    }

    return true;
}

bool    Object::GetMember(Environment* penv, const ASString& name, Value* val)
{
    //!AB: TODO - eliminate the extra virtual call here; though, to reach this point it 
    // is necessary to require overloading of both - GetMember and GetMemberRaw always.
    // Actually, the difference between GetMember and GetMemberRaw is in 1st parameter
    // only; functionally, they are the same now. TODO.
    bool rv = GetMemberRaw(penv->GetSC(), name, val);
    // GetMember now doesn't invoke getter methods as well as __resolve handlers.
    // This is done by the Environment::GetMember and Environment::GetVariable.
    return rv;
}

bool    Object::GetMemberRaw(ASStringContext *psc, const ASString& name, Value* val)
{    
    SF_ASSERT(psc && val);
    /*
    int sz = sizeof(GHashNode<GASString, Member, GASStringHashFunctor>);
    int sz2 = sizeof(Member);
    int sz3 = sizeof(Value);
    int sz4 = sizeof(GASFunctionRefBase);

    printf("Size Value: = %d\n", sz3);
    */

    Object* current = this;
    bool resolveHandlerSet = false;

    // For efficiency, do extra case-sensitivity branch.
    if (psc->IsCaseSensitive())
    {
        while (current != NULL)
        {
            MemberHash::Iterator im;
            if (name == psc->GetBuiltin(ASBuiltin___proto__))
            {   
                Object* pproto = current->Get__proto__();
                if (pproto) 
                    val->SetAsObject(pproto);
                else
                    val->SetUndefined();
                return true;
            }
            else if (name == psc->GetBuiltin(ASBuiltin___resolve))
            {   
                if (!current->ResolveHandler.IsNull())
                    val->SetAsFunction(current->ResolveHandler);
                else
                    val->SetUndefined();
                return true;
            }

            // Find member case-sensitively.
            im = current->Members.Find(name);

            if (im != current->Members.End())
            {
                const Value& memval = im->Second.GetMemberValue();
                // if member's value is unset and current is not equal to "this" (to avoid
                // indefinite recursion, then we need to call virtual method GetMemberRaw 
                // to get the value.
                if (!memval.IsSet() && current != this)
                    return current->GetMemberRaw(psc, name, val);
                *val = memval;
                return true;
            }

            if (!resolveHandlerSet && !current->ResolveHandler.IsNull())
            {
                // if not found and we have ResolveHandler - put it into the value, thus
                // we will be able to invoke it later in Execute.
                val->SetAsResolveHandler(current->ResolveHandler);
                resolveHandlerSet = true;
            }
            current = current->Get__proto__();
        }
    }
    else
    {
        name.ResolveLowercase();
        ASString::NoCaseKey ikey(name);
        while (current != NULL)
        {
            if (psc->GetBuiltin(ASBuiltin___proto__).CompareBuiltIn_CaseInsensitive_Unchecked(name))
            {   
                Object* pproto = current->Get__proto__();
                if (pproto) 
                    val->SetAsObject(pproto);
                else
                    val->SetUndefined();
                return true;
            }
            else if (psc->GetBuiltin(ASBuiltin___resolve).CompareBuiltIn_CaseInsensitive_Unchecked(name))
            {   
                if (!current->ResolveHandler.IsNull())
                    val->SetAsFunction(current->ResolveHandler);
                else
                    val->SetUndefined();
                return true;
            }

            // Find member case-insensitively.
            MemberHash::Iterator im;

            im = current->Members.FindAlt(ikey);

            if (im != current->Members.End())
            {
                const Value& memval = im->Second.GetMemberValue();
                // if member's value is unset and current is not equal to "this" (to avoid
                // indefinite recursion, then we need to call virtual method GetMemberRaw 
                // to get the value.
                if (!memval.IsSet() && current != this)
                    return current->GetMemberRaw(psc, name, val);
                *val = memval;
                return true;
            }

            if (!resolveHandlerSet && !current->ResolveHandler.IsNull())
            {
                // if not found and we have ResolveHandler - put it into the value, thus
                // we will be able to invoke it later in Execute.
                val->SetAsResolveHandler(current->ResolveHandler);
                resolveHandlerSet = true;
            }
            current = current->Get__proto__();
        }
    }
/*
    if (im != Members.End())
    {
        *val = im->Second.GetMemberValue();
        return true;
    }
  */  
    // MA: Could optimize this a bit by unrolling recursion and thus avoiding
    // nested '__proto__' and '__resolve' checks.
    /*Object* pproto = Get__proto__();
    if (pproto)
        return pproto->GetMemberRaw(psc, name, val);*/

    return false;
}

bool    Object::FindMember(ASStringContext *psc, const ASString& name, Member* pmember)
{
    //printf("GET MEMBER: %s at %p for object %p\n", name.C_str(), val, this);
    SF_ASSERT(pmember != NULL);
    return Members.GetCaseCheck(name, pmember, psc->IsCaseSensitive());
}

bool    Object::DeleteMember(ASStringContext *psc, const ASString& name)
{
    // Find member.
    MemberHash::ConstIterator it = Members.FindCaseCheck(name, psc->IsCaseSensitive());
    // Member must exist and be available for deletion.
    if (it == Members.End())
        return false;
    if (it->Second.GetMemberFlags().GetDontDelete())
        return false;
    if (name == psc->GetBuiltin(ASBuiltin_onEnterFrame))
    {
        // Invalidate optimized advance list.
        psc->InvalidateOptAdvanceList();
    }
    Members.Remove(name);
    return true;
}

bool    Object::SetMemberFlags(ASStringContext *psc, const ASString& name, const UByte flags)
{
    Member member;
    if (FindMember(psc, name, &member))
    {
        PropFlags f = member.GetMemberFlags();
        f.SetFlags(flags);
        member.SetMemberFlags(f);

        Members.Set(name, member);
        return true;
    }
    return false;
}

void    Object::VisitMembers(ASStringContext *psc, 
                             MemberVisitor *pvisitor, 
                             unsigned visitFlags, 
                             const ObjectInterface* instance) const
{
    MemberHash::ConstIterator it = Members.Begin();
    while(it != Members.End())
    {   
        const PropFlags& memberFlags = it->Second.GetMemberFlags();
        if (!memberFlags.GetDontEnum() || (visitFlags & VisitMember_DontEnum))
        {
            const Value& value = it->Second.GetMemberValue();
            if (value.IsSet())
                pvisitor->Visit(it->First, value, memberFlags.Flags);
            else
            {
                // If value is not set - get it by GetMember
                Value palValue;
                if (!(visitFlags & ObjectInterface::VisitMember_NamesOnly))
                {
                    //!AB: basically, we need just a name here. We need to use original instance 
                    // pointer to call GetMemberRaw (if it is not NULL). This will help to get
                    // correct value for members, whose names are stored in prototypes.
                    ObjectInterface* pobj = 
                        const_cast<ObjectInterface*>((instance) ? instance : 
                        static_cast<const ObjectInterface*>(this));
                    pobj->GetMemberRaw(psc, it->First, &palValue);
                }
                pvisitor->Visit(it->First, palValue, memberFlags.Flags);
            }
        }
        ++it;
    }

    if ((visitFlags & VisitMember_Prototype) && pProto)
        pProto->VisitMembers(psc, pvisitor, visitFlags, (instance) ? instance : this);
}

bool    Object::HasMember(ASStringContext *psc, const ASString& name, bool inclPrototypes)
{
    Member member;
    if (Members.Get(name, &member))
    {
        return true;
    }
    else if (inclPrototypes && pProto)
    {
        return pProto->HasMember(psc, name, true);
    }
    return false;
}

bool Object::InstanceOf(Environment* penv, const Object* prototype, bool inclInterfaces) const
{
    for (const Object* proto = this; proto != 0; proto = const_cast<Object*>(proto)->Get__proto__())
    {
        if (inclInterfaces)
        {
            if (proto->DoesImplement(penv, prototype)) return true;
        }
        else
        {
            if (proto == prototype) return true;
        }
    }
    return false;
}

bool Object::Watch(ASStringContext *psc, const ASString& prop, const FunctionRef& callback, const Value& userData)
{
    Watchpoint wp;
    wp.Callback = callback;
    wp.UserData = userData;
    if (pWatchpoints == NULL)
    {
        pWatchpoints = SF_HEAP_NEW(psc->GetHeap()) WatchpointHash;
    }
    // Need to do a case check on set too, to differentiate overwrite behavior. 
    pWatchpoints->SetCaseCheck(prop, wp, psc->IsCaseSensitive());
    return true;
}

bool Object::Unwatch(ASStringContext *psc, const ASString& prop)
{
    if (pWatchpoints != NULL)
    {
        if (pWatchpoints->GetCaseCheck(prop, psc->IsCaseSensitive()) != 0)
        {
            pWatchpoints->RemoveCaseCheck(prop, psc->IsCaseSensitive());
            if (pWatchpoints->GetSize() == 0)
            {
                delete pWatchpoints;
                pWatchpoints = 0;
            }
            return true;
        }
    }
    return false;
}

bool Object::InvokeWatchpoint(Environment* penv, const ASString& prop, const Value& newVal, Value* resultVal)
{
    Value oldVal;
    // if property doesn't exist at this point, the "oldVal" value will be "undefined"
    GetMember(penv, prop, &oldVal);

    SF_ASSERT(resultVal);

    Value result;
    const Watchpoint* wp = pWatchpoints->GetCaseCheck(prop, penv->IsCaseSensitive());
    if (wp && penv && pWatchpoints)
    {
        penv->Push(wp->UserData);
        penv->Push(newVal);
        penv->Push(oldVal);
        penv->Push(Value(prop));
        
        // check, is the object 'this' came from InteractiveObject. If so,
        // use original InteractiveObject as this.
        Ptr<InteractiveObject> paschar = GetASCharacter();

        if (paschar)
            wp->Callback.Invoke(FnCall(&result, GFx::AS2::ToAvmCharacter(paschar), penv, 4, penv->GetTopIndex()));
        else
            wp->Callback.Invoke(FnCall(&result, this, penv, 4, penv->GetTopIndex()));
        penv->Drop(4);
        *resultVal = result;
        return true;
    }
    return false;
}

Object* Object::GetPrototype(GlobalContext* pgc, ASBuiltinType classNameId)
{
    return pgc->GetPrototype(classNameId);
}

///////////////////////
GASPrototypeBase::~GASPrototypeBase()
{
    if (pInterfaces)
        delete pInterfaces;
}

void GASPrototypeBase::InitFunctionMembers(
    Object* pthis,
    ASStringContext *psc, 
    const NameFunction* funcTable, 
    const PropFlags& flags)
{
    Ptr<Object> pfuncProto = Object::GetPrototype(psc->pContext, ASBuiltin_Function);
    for(int i = 0; funcTable[i].Name; i++)
    {
        pthis->SetMemberRaw(psc, psc->CreateConstString(funcTable[i].Name), 
            FunctionRef(
            *SF_HEAP_NEW(psc->GetHeap()) CFunctionObject(psc, pfuncProto, funcTable[i].Function)),
            flags);
    }
}

// 'Init' method is used from the Prototype ancestor ctor to init the GASPrototypeBase instance.
// It is actually a part of GASPrototypeBase ctor, but due to GASPrototypeBase is not Object we need
// to do this in separate function to avoid warning ("usage of 'this' in initializer list).
void GASPrototypeBase::Init(Object* pthis, ASStringContext *psc, const FunctionRef& constructor)
{
    SetConstructor(pthis, psc, constructor); 
    Constructor->SetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_prototype), Value(pthis),
        PropFlags::PropFlag_DontEnum | PropFlags::PropFlag_DontDelete);
}

bool GASPrototypeBase::SetConstructor(Object* pthis, ASStringContext *psc, const Value& ctor) 
{ 
    Constructor = ctor.ToFunction(NULL);
    pthis->SetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_constructor), Value(Value::UNSET),
        PropFlags::PropFlag_DontDelete | PropFlags::PropFlag_DontEnum);
    return true; 
}

void GASPrototypeBase::AddInterface(ASStringContext *psc, int index, FunctionObject* pinterface)
{
    if (pInterfaces == 0 && pinterface == 0)
    {
        pInterfaces = SF_HEAP_NEW(psc->GetHeap()) InterfacesArray(index);
        return;
    }
    SF_ASSERT(pinterface);
    Value prototypeVal;
    if (pinterface->GetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_prototype), &prototypeVal))
    {
        (*pInterfaces)[index] = Ptr<Object>(prototypeVal.ToObject(NULL));
    }
}

bool GASPrototypeBase::DoesImplement(Environment* penv, const Object* prototype) const 
{ 
    if (pInterfaces)
    {
        for(unsigned i = 0, n = unsigned(pInterfaces->GetSize()); i < n; ++i)
        {
            Ptr<Object> intf = (*pInterfaces)[i];
            if (intf && intf->InstanceOf(penv, prototype))
                return true;
        }
    }
    return false;
}

bool GASPrototypeBase::GetMemberRawConstructor(Object* pthis, ASStringContext *psc, const ASString& name, Value* val, bool isConstructor2)
{
    Member m;
    Value lval(Value::UNSET);

    // if "constructor" is set as a member - return it.
    if (pthis->Object::FindMember(psc, name, &m))
    {
        lval = m.GetMemberValue();
    }
    if (lval.IsSet())
        *val = lval;
    else
    {
        FunctionRef ctor;
        if (isConstructor2)
            ctor = __Constructor__;
        else
            ctor = GetConstructor ();
        if (ctor.IsNull())
        {
            val->SetUndefined();
            Object* pproto = pthis->Get__proto__();
            // check __proto__
            if (pproto)
                return pproto->GetMemberRaw(psc, name, val);

        }
        else
            val->SetAsFunction(ctor);
    }
    return true;
}

///////////////////////
static const NameFunction GAS_ObjectFunctionTable[] = 
{
    { "addProperty",            &ObjectProto::AddProperty },
    { "hasOwnProperty",         &ObjectProto::HasOwnProperty },
    { "watch",                  &ObjectProto::Watch_ },
    { "unwatch",                &ObjectProto::Unwatch_ },
    { "isPropertyEnumerable",   &ObjectProto::IsPropertyEnumerable },
    { "isPrototypeOf",          &ObjectProto::IsPrototypeOf },
    { "toString",               &ObjectProto::ToString },
    { "valueOf",                &ObjectProto::ValueOf },
    { 0, 0 }
};

ObjectProto::ObjectProto(ASStringContext *psc, Object* pprototype, const FunctionRef& constructor)
    : Prototype<Object>(psc, pprototype, constructor) 
{
    InitFunctionMembers(psc, GAS_ObjectFunctionTable);
}

// This ctor is used by 'extends' operation and does NOT 
// initialize the function table
ObjectProto::ObjectProto(ASStringContext *psc, Object* pprototype)
    : Prototype<Object>(psc, pprototype) 
{
}

ObjectProto::ObjectProto(ASStringContext *psc, const FunctionRef& constructor)
    : Prototype<Object>(psc, constructor) 
{
    InitFunctionMembers(psc, GAS_ObjectFunctionTable);
}

void ObjectProto::AddProperty(const FnCall& fn)
{
    SF_ASSERT(fn.ThisPtr);

    if (fn.NArgs >= 2)
    {
        ASString       propName(fn.Arg(0).ToString(fn.Env));
        FunctionRef  getter = fn.Arg(1).ToFunction(fn.Env);

        if (!getter.IsNull())
        {
            FunctionRef setter;
            if (fn.NArgs >= 3 && fn.Arg(2).IsFunction())
                setter = fn.Arg(2).ToFunction(fn.Env);
            Value val(getter, setter, fn.Env->GetHeap(), fn.Env->GetCollector());
            fn.ThisPtr->SetMemberRaw(fn.Env->GetSC(), propName, val);
            fn.Result->SetBool(true);
        }
        else
            fn.Result->SetBool(false);
    }
    else
        fn.Result->SetBool(false);
}

void ObjectProto::HasOwnProperty(const FnCall& fn)
{
    SF_ASSERT(fn.ThisPtr);

    fn.Result->SetBool(fn.ThisPtr->HasMember(fn.Env->GetSC(), fn.Arg(0).ToString(fn.Env), false));
}

void ObjectProto::Watch_(const FnCall& fn)
{
    SF_ASSERT(fn.ThisPtr);

    if (fn.NArgs >= 2)
    {        
        FunctionRef callback = fn.Arg(1).ToFunction(fn.Env);
        if (!callback.IsNull())
        {
            Value userData;
            if (fn.NArgs >= 3)
            {
                userData = fn.Arg(2);
            }
            fn.Result->SetBool(fn.ThisPtr->Watch(fn.Env->GetSC(), fn.Arg(0).ToString(fn.Env), callback, userData));
        }
        else
            fn.Result->SetBool(false);
    }
    else
        fn.Result->SetBool(false);
}

void ObjectProto::Unwatch_(const FnCall& fn)
{
    SF_ASSERT(fn.ThisPtr);

    if (fn.NArgs >= 1)
    {        
        fn.Result->SetBool(fn.ThisPtr->Unwatch(fn.Env->GetSC(), fn.Arg(0).ToString(fn.Env)));
    }
    else
        fn.Result->SetBool(false);
}

void ObjectProto::IsPropertyEnumerable(const FnCall& fn)
{
    SF_ASSERT(fn.ThisPtr);

    if (fn.NArgs >= 1)
    {
        ASString prop(fn.Arg(0).ToString(fn.Env));
        bool rv = fn.ThisPtr->HasMember(fn.Env->GetSC(), prop, false);
        if (rv)
        {
            Member m;
            fn.ThisPtr->FindMember(fn.Env->GetSC(), prop, &m);
            if (m.GetMemberFlags().GetDontEnum())
                rv = false;
        }
        fn.Result->SetBool(rv);
    }
    else
        fn.Result->SetBool(false);
}

void ObjectProto::IsPrototypeOf(const FnCall& fn)
{
    SF_ASSERT(fn.ThisPtr);

    if (fn.NArgs >= 1 && !fn.ThisPtr->IsASCharacter())
    {
        Object* pthis = (Object*)fn.ThisPtr;
        ObjectInterface* iobj = fn.Arg(0).ToObjectInterface(fn.Env);
        if (iobj)
        {
            fn.Result->SetBool(iobj->InstanceOf(fn.Env, pthis, false));
            return;
        }
    }
    fn.Result->SetBool(false);
}

void ObjectProto::ToString(const FnCall& fn)
{
    if (fn.ThisPtr->IsFunction()) // "[type Function]"
        fn.Result->SetString(fn.Env->GetBuiltin(ASBuiltin_typeFunction_));
    else if (fn.ThisPtr->IsASCharacter())
        fn.Result->SetString(Value(fn.ThisPtr->ToCharacter()).GetCharacterNamePath(fn.Env));
    else // "[object Object]"
        fn.Result->SetString(fn.Env->GetBuiltin(ASBuiltin_objectObject_));
}

void ObjectProto::ValueOf(const FnCall& fn)
{
    SF_ASSERT(fn.ThisPtr);

    fn.Result->SetAsObjectInterface(fn.ThisPtr);
}

// Constructor for ActionScript class Object.
void ObjectProto::GlobalCtor(const FnCall& fn)
{
    if (fn.NArgs >= 1)
    {
        //!AB, special undocumented(?) case: if Object's ctor is invoked
        // with a parameter of type:
        // 1. number - it will create instance of Number;
        // 2. boolean - it will create instance of Boolean;
        // 3. array - it will create instance of Array;
        // 4. string - it will create instance of String;
        // 5. object - it will just return the same object.
        // "null" and "undefined" are ignored.
        const Value& arg0 = fn.Arg(0);
        Value res;
        if (arg0.IsNumber())
            res = arg0.ToNumber(fn.Env);
        else if (arg0.IsBoolean())
            res = arg0.ToBool(fn.Env);
        else if (arg0.IsString())
            res = arg0.ToString(fn.Env);
        else if (arg0.IsObject() || arg0.IsCharacter())
        {
            res = arg0;
        }
        if (!res.IsUndefined())
        {
            *fn.Result = res;
            return;
        }
    }
    Ptr<Object> o;
    if (fn.ThisPtr)
        o = static_cast<Object*>(fn.ThisPtr);
    else
        o = *SF_HEAP_NEW(fn.Env->GetHeap()) Object(fn.Env);

    // for some reasons Object always contains "constructor" property instead of __constructor__
    FunctionRef ctor = fn.Env->GetConstructor(ASBuiltin_Object);
    o->Set_constructor(fn.Env->GetSC(), ctor);

    fn.Result->SetAsObject(o);
}

//////////////////
const NameFunction ObjectCtorFunction::StaticFunctionTable[] = 
{
    { "registerClass",       &ObjectCtorFunction::RegisterClass },
    { 0, 0 }
};

ObjectCtorFunction::ObjectCtorFunction(ASStringContext *psc) :
    CFunctionObject(psc, ObjectProto::GlobalCtor)
{
    NameFunction::AddConstMembers(
        this, psc, StaticFunctionTable, 
        PropFlags::PropFlag_ReadOnly | PropFlags::PropFlag_DontDelete | PropFlags::PropFlag_DontEnum);
    /*
    for(int i = 0; StaticFunctionTable[i].Name; i++)
    {
        SetMemberRaw(psc, psc->CreateConstString(StaticFunctionTable[i].Name),
                     Value(StaticFunctionTable[i].Function), 
                     PropFlags::PropFlag_ReadOnly | PropFlags::PropFlag_DontDelete |
                     PropFlags::PropFlag_DontEnum);
    }
    */
}

void ObjectCtorFunction::RegisterClass(const FnCall& fn)
{
    fn.Result->SetBool(false);
    if (fn.NArgs < 2)
    {
        fn.Env->LogScriptError ("Error: Too few parameters for Object.registerClass (%d)", fn.NArgs);   
        return;
    }

    GlobalContext* pgctxt = fn.Env->GetGC();
    ASString         classname(fn.Arg(0).ToString(fn.Env));
    SF_ASSERT(pgctxt);
    if (fn.Arg(1).IsFunction())
    {
        // Registration
        FunctionRef func = fn.Arg(1).ToFunction(fn.Env);        
        fn.Result->SetBool(pgctxt->RegisterClass(fn.Env->GetSC(), classname, func));
    }
    else if (fn.Arg(1).IsNull())
    {
        // De-registration
        fn.Result->SetBool(pgctxt->UnregisterClass(fn.Env->GetSC(), classname));
    }
    else
    {
        ASString a1(fn.Arg(1).ToString(fn.Env));
        fn.Env->LogScriptError("Second parameter of Object.registerClass(%s, %s) should be function or null", 
                               classname.ToCStr(), a1.ToCStr());
    }
}

Object* ObjectCtorFunction::CreateNewObject(Environment* penv) const 
{
    return SF_HEAP_NEW(penv->GetHeap()) Object(penv);
}


Object* FunctionCtorFunction::CreateNewObject(Environment* penv) const 
{ 
    return SF_HEAP_NEW(penv->GetHeap()) CFunctionObject(penv->GetSC()); 
}


}}} // SF::GFx::AS2
