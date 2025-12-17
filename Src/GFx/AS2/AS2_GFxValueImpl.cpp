/**************************************************************************

Filename    :   AS2_GFxValueImpl.cpp
Content     :   Complex Objects API Implementation
Created     :   
Authors     :   Prasad Silva

Notes       :   Contains MovieImpl, Value::ObjectInterface and 
                Value method definitions.

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/GFx_PlayerImpl.h"
#include "GFx/GFx_TextField.h"
#include "GFx/AS2/AS2_ArrayObject.h"
#include "GFx/AS2/AS2_MovieRoot.h"

#include "GFx/AS2/AS2_AvmSprite.h"
#include "GFx/AS2/AS2_AvmCharacter.h"
#include "GFx/AS2/AS2_AvmTextField.h"
#include "Kernel/SF_MsgFormat.h"

//#include "Render/Render_Renderer.h"


namespace Scaleform { namespace GFx {


#define OFFSETOF(st, m) ((size_t) ( (char *)&((st *)(16))->m - (char *)16 ))


// ***** Custom AS2 function object

namespace AS2 {

// Custom FunctionObject that wraps the function context as well as
// the user data.
class UserDefinedFunctionObject : public FunctionObject
{
    friend class FunctionProto;
    friend class FunctionCtorFunction;
    friend class CustomFunctionObject;
    friend class FunctionRefBase;
protected:
    Ptr<FunctionHandler>            pContext;
    void*                           pUserData;

    UserDefinedFunctionObject(ASStringContext* psc) : FunctionObject(psc) { SF_UNUSED(psc); }
    UserDefinedFunctionObject(Environment* penv) : FunctionObject(penv) { SF_UNUSED(penv); }

#ifdef GFX_AS_ENABLE_GC
    
    // PPS: We may need to link up ForEachChild_GC via a visitor interface
    //      later on to somewhat automate the cleaning up of any GFxValues 
    //      held by the function context.
    
    virtual void Finalize_GC()
    {
        pContext = NULL;
        FunctionObject::Finalize_GC();
    }
#endif // GFX_AS_ENABLE_GC
public:
    UserDefinedFunctionObject(ASStringContext* psc, FunctionHandler* pcontext, void* puserData) 
        : FunctionObject(psc), pContext(pcontext), pUserData(puserData)
    {
        Set__proto__(psc, psc->pContext->GetPrototype(ASBuiltin_Function));
    }
    virtual ~UserDefinedFunctionObject() {}

    virtual Environment* GetEnvironment(const FnCall& fn, Ptr<InteractiveObject>* ptargetCh) 
    {
        SF_UNUSED(ptargetCh);
        return fn.Env;
    }
    virtual void Invoke (const FnCall& fn, LocalFrame*, const char* pmethodName);

    virtual bool IsNull () const { return 0 == pContext; }
    virtual bool IsEqual(const FunctionObject& f) const 
    { 
        SF_UNUSED(f); 
        return false;   // AB wants it returning false
    }

    // returns number of arguments expected by this function;
    // returns -1 if number of arguments is unknown (for C functions)
    int     GetNumArgs() const { return -1; }

private:
    void    InvokeImpl(const FnCall& fn);
};


void UserDefinedFunctionObject::Invoke(const FnCall& fn, LocalFrame*, const char*) 
{    
    if (0 != pContext) {
        ObjectInterface* pthis   = fn.ThisPtr;
        if (pthis && pthis->IsSuper())
        {
            ObjectInterface* prealThis = static_cast<SuperObject*>(pthis)->GetRealThis();
            FnCall fn2(fn.Result, prealThis, fn.Env, fn.NArgs, fn.FirstArgBottomIndex);
            InvokeImpl(fn2);
            static_cast<SuperObject*>(pthis)->ResetAltProto(); // reset alternative proto, if set
        }
        else
            InvokeImpl(fn);
    }
}

void UserDefinedFunctionObject::InvokeImpl(const FnCall& fn)
{
    Environment* penv = fn.Env;
    SF_ASSERT(penv);
    ArrayCPP<GFx::Value> args;

    GFx::Value thisVal, retVal;
    AS2::Value thisAS;
    if (fn.ThisPtr) thisAS.SetAsObjectInterface(fn.ThisPtr);
    else thisAS.SetNull();
    penv->GetAS2Root()->ASValue2Value(penv, thisAS, &thisVal);
    args.PushBack(thisVal);  // Used to pack 'this' on callback

    for (int i=0; i < fn.NArgs; i++)
    {
        GFx::Value arg;
        penv->GetAS2Root()->ASValue2Value(penv, fn.Arg(i), &arg);
        args.PushBack(arg);
    }

    FunctionHandler::Params params;
    params.pMovie               = penv->GetAS2Root()->GetMovieImpl();
    params.pRetVal              = &retVal;
    params.pThis                = &thisVal;
    params.pArgs                = fn.NArgs > 0 ? &args[1] : NULL;
    params.pArgsWithThisRef     = &args[0];
    params.ArgCount             = (unsigned)args.GetSize() - 1;
    params.pUserData            = pUserData;
    pContext->Call(params);

    if (!retVal.IsUndefined()) 
        penv->GetAS2Root()->Value2ASValue(retVal, fn.Result);
}

} // namespace AS2


// ***** Value::ObjectInterface definitions

// Value_AS2ObjectData is a simple helper used by Value::ObjectInterface to
// decode 'pdata' value in Value and interpret it as object.
struct Value_AS2ObjectData
{
    AS2::ObjectInterface* pObject;
    AS2::Environment*     pEnv;
    AS2::MovieRoot*       pRoot;

    Value_AS2ObjectData(const Value::ObjectInterface* i, void* pdata, bool isdobj);   
};

Value_AS2ObjectData::Value_AS2ObjectData(const Value::ObjectInterface* i, void* pdata, bool isdobj)
{
    MovieImpl* pmovieImpl = i->GetMovieImpl();    
    pRoot   = static_cast<AS2::MovieRoot*>(pmovieImpl->pASMovieRoot.GetPtr());
    pEnv    = AS2::ToAvmSprite(pRoot->GetLevel0Movie())->GetASEnvironment();
    pObject = static_cast<AS2::ObjectInterface*>(pdata);
    if (isdobj)
    {
        CharacterHandle* pch = static_cast<CharacterHandle*>(pdata);
        DisplayObject* d = pch->ResolveCharacter(pmovieImpl);
        if (!d)
        {
            pObject = 0;
            return;
        }
        pObject = AS2::ToAvmCharacter(d->CharToInteractiveObject_Unsafe());
    }
}


// If both VMs are supported, Value::Object interface is implemented
// through virtual functions; if not, it is statically linked.

#if defined(GFX_AS2_SUPPORT) && defined(GFX_AS3_SUPPORT)

class AS2ValueObjectInterface : public MovieImpl::ValueObjectInterface
{
    typedef Value::DisplayInfo DisplayInfo;

    // Implementation of Value::ObjectInterface
    void    ObjectAddRef(Value* val, void* pobj);
    void    ObjectRelease(Value* val, void* pobj);

    bool    HasMember(void* pdata, const char* name, bool isdobj) const;
    bool    GetMember(void* pdata, const char* name, Value* pval, bool isdobj) const;
    bool    SetMember(void* pdata, const char* name, const Value& value, bool isdobj);
    bool    Invoke(void* pdata, Value* presult, const char* name,
                   const Value* pargs, UPInt nargs, bool isdobj);
    bool    InvokeClosure(void* pdata, UPInt dataAux, Value* presult,
                   const Value* pargs, UPInt nargs);

    bool    DeleteMember(void* pdata, const char* name, bool isdobj);
    void    VisitMembers(void* pdata, ObjVisitor* visitor, bool isdobj) const;

    unsigned GetArraySize(void* pdata) const;
    bool    SetArraySize(void* pdata, unsigned sz);
    bool    GetElement(void* pdata, unsigned idx, Value *pval) const;
    bool    SetElement(void* pdata, unsigned idx, const Value& value);
    void    VisitElements(void* pdata, ArrVisitor* visitor,
                         unsigned idx, int count) const;
    bool    PushBack(void* pdata, const Value& value);
    bool    PopBack(void* pdata, Value* pval);
    bool    RemoveElements(void* pdata, unsigned idx, int count);

	bool	IsInstanceOf(void* pdata, const char* className) const;

    bool    IsByteArray(void* pdata) const;
    unsigned GetByteArraySize(void* pdata) const;
	void	SetByteArraySize(void* pdata, UPInt setSize);
    bool    ReadFromByteArray(void* pdata, UByte *destBuff, UPInt destBuffSz) const;
    bool    WriteToByteArray(void* pdata, const UByte *srcBuff, UPInt writeSize);
	void*   GetRawDataPtr(void* pdata);

    bool    IsDisplayObjectActive(void* pdata) const;
    bool    GetParent(void* pdata, Value* pparent) const;
    bool    GetDisplayInfo(void* pdata, DisplayInfo* pinfo) const;
    bool    SetDisplayInfo(void* pdata, const DisplayInfo& info);
    bool    GetWorldMatrix(void* pdata, Render::Matrix2F* pmat) const;
    bool    GetDisplayMatrix(void* pdata, Render::Matrix2F* pmat) const;
    bool    SetDisplayMatrix(void* pdata, const Render::Matrix2F& mat);
    bool    GetMatrix3D(void* pdata, Render::Matrix3F* pmat) const;
    bool    SetMatrix3D(void* pdata, const Render::Matrix3F& mat);

    bool    GetCxform(void* pdata, Render::Cxform* pcx) const;
    bool    SetCxform(void* pdata, const Render::Cxform& cx);

    bool    GetText(void* pdata, Value* pval, bool ishtml) const;
    bool    SetText(void* pdata, const char* ptext, bool ishtml);
    bool    SetText(void* pdata, const wchar_t*, bool ishtml);
    bool    GotoAndPlay(void* pdata, const char* frame, bool stop);
    bool    GotoAndPlay(void* pdata, unsigned frame, bool stop);

    bool    CreateEmptyMovieClip(void* pdata, GFx::Value* pmc, 
                                    const char* instanceName, SInt32 depth);
    bool    AttachMovie(void* pdata, GFx::Value* pmc, 
                            const char* symbolName, const char* instanceName, 
                            SInt32 depth, const MemberValueSet* initArgs);

    void    ToString(String* pstr, const GFx::Value& thisVal) const;

#ifdef GFX_AS_ENABLE_USERDATA

    void    SetUserData(void* pdata, ASUserData* puserdata, bool isdobj);
    ASUserData* GetUserData(void* pdata, bool isdobj) const;
    bool    CreateObjectValue(Value* pval, void* pdata, bool isdobj);

#endif  // GFX_AS_ENABLE_USERDATA

    AmpStats* GetAdvanceStats() const;

public:
    AS2ValueObjectInterface(MovieImpl* pmovieRoot)
        : MovieImpl::ValueObjectInterface(pmovieRoot) { }

};
#define GFX_Value_ObjectInterface_CLASS AS2ValueObjectInterface

#else
#define GFX_Value_ObjectInterface_CLASS GFx::Value::ObjectInterface
#endif

// ***** GFxValue::ObjectInterface definitions

#ifdef GFX_AS2_SUPPORT

AmpStats* GFX_Value_ObjectInterface_CLASS::GetAdvanceStats() const
{
    return GetMovieImpl()->pASMovieRoot->GetMovieImpl()->AdvanceStats; 
}


void GFX_Value_ObjectInterface_CLASS::ObjectAddRef(Value* val, void* pobj)
{
    // Addref complex object
    switch (val->GetType())
    {
    case Value::VT_String:
        {
            // pobj points to const char**
            size_t memberOffset = OFFSETOF(ASStringNode, pData);
            ASStringNode* ps = (ASStringNode*)(((UByte*)pobj) - memberOffset);
            ps->AddRef();            
        }
        break;
    case Value::VT_StringW:
        {
            // pobj points to const wchar_t*
            size_t memberOffset = OFFSETOF(MovieImpl::WideStringStorage, pData);
            MovieImpl::WideStringStorage* pws = (MovieImpl::WideStringStorage*)(((UByte*)pobj) - memberOffset);
            pws->AddRef();            
        }
        break;
    case Value::VT_DisplayObject:
        {
            CharacterHandle* pifc = (CharacterHandle*)pobj;
            pifc->AddRef();
        }
        break;
    case Value::VT_Object:
    case Value::VT_Array:
        {
            AS2::ObjectInterface* pifc = reinterpret_cast<AS2::ObjectInterface*>(pobj);
            pifc->ToASObject()->AddRef();
        }
        break;
    default:
        SF_ASSERT(0);
    }

#if defined(SF_BUILD_DEBUG) || defined(GFX_AS_ENABLE_GFXVALUE_CLEANUP)
    ExternalObjRefs.PushBack(val);
#endif
}

void GFX_Value_ObjectInterface_CLASS::ObjectRelease(Value* val, void* pobj)
{
    // NOTE: The next statements may cause an access violation exception
    // if the GFxMovie containing the AS object has been released. To
    // avoid this issue, make sure to release any GFx::Values holding
    // references to AS objects before the movie that holds those objects
    // is released.

    switch (val->GetType())
    {
    case Value::VT_String:
        {
            // pobj points to const char**
            size_t memberOffset = OFFSETOF(ASStringNode, pData);
            ASStringNode* ps = (ASStringNode*)(((UByte*)pobj) - memberOffset);
            ps->Release();
        }
        break;
    case Value::VT_StringW:
        {
            // pobj points to const wchar_t*
            size_t memberOffset = OFFSETOF(MovieImpl::WideStringStorage, pData);
            MovieImpl::WideStringStorage* pws = 
                (MovieImpl::WideStringStorage*)(((UByte*)pobj) - memberOffset);
            pws->Release();
        }
        break;
    case Value::VT_DisplayObject:
        {
            CharacterHandle* pifc = (CharacterHandle*)pobj;
            pifc->Release();
        }
        break;
    case Value::VT_Object:
    case Value::VT_Array:
        {
            AS2::ObjectInterface* pifc = reinterpret_cast<AS2::ObjectInterface*>(pobj);
            pifc->ToASObject()->Release();
        }
        break;
    default:
        SF_ASSERT(0);
    }

#if defined(SF_BUILD_DEBUG) || defined(GFX_AS_ENABLE_GFXVALUE_CLEANUP)
    ExternalObjRefs.Remove(val);
#endif
}


bool GFX_Value_ObjectInterface_CLASS::HasMember(void* pdata, const char* name, bool isdobj) const 
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::HasMember", Amp_Native_Function_Id_ObjectInterface_HasMember);

    Value_AS2ObjectData o(this, pdata, isdobj);
    if (!o.pObject)
        return false;

    // Look for member    
    AS2::Value member;
    // PPS: We are using GetMember instead of FindMember because certain GASObjectInterface
    //      subclasses do not implement FindMember (such as Sprite). GetMember has guaranteed
    //      behavior.
    if (!o.pObject->GetMember(o.pEnv, o.pEnv->CreateConstString(name), &member))
    {
        // Member not found
        return false;
    }
    return true;
}

bool GFX_Value_ObjectInterface_CLASS::GetMember(void* pdata, const char* name, Value* pval, bool isdobj) const
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::GetMember", Amp_Native_Function_Id_ObjectInterface_GetMember);

    Value_AS2ObjectData o(this, pdata, isdobj);
    if (!o.pObject)
    {
        if (pval) pval->SetUndefined();
        return false;
    }
      
    // Look for member
    AS2::Value asval;
    if (!o.pObject->GetMember(o.pEnv, o.pEnv->CreateString(name), &asval))
    {
        // Member not found
        if (pval) pval->SetUndefined();
        return false;
    }
    
    // Resolve property if required
    if (asval.IsProperty())
    {
        AS2::ObjectInterface* pobj = NULL;
        AS2::AvmCharacter* paschar = NULL;
        if (o.pObject->IsASObject())
            pobj = o.pObject->ToASObject();
        if (o.pObject->IsASCharacter())
            paschar = o.pObject->ToAvmCharacter();
        asval.GetPropertyValue(o.pEnv, paschar ? (AS2::ObjectInterface*)paschar : pobj, &asval);
    }

    // Found the member
    o.pRoot->ASValue2Value(o.pEnv, asval, pval);
    return true;
}

bool GFX_Value_ObjectInterface_CLASS::SetMember(void* pdata, const char* name, const Value& value, bool isdobj)
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::SetMember", Amp_Native_Function_Id_ObjectInterface_SetMember);

    Value_AS2ObjectData o(this, pdata, isdobj);
    if (!o.pObject)
        return false;

    AS2::Value asval;
    o.pRoot->Value2ASValue(value, &asval);
    // Call SetMember to resolve properties
    return o.pObject->SetMember(o.pEnv, o.pEnv->CreateString(name), asval);
}

bool GFX_Value_ObjectInterface_CLASS::Invoke(void* pdata, GFx::Value* presult, const char* name,
                                    const GFx::Value* pargs, UPInt nargs, bool isdobj)
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::Invoke", Amp_Native_Function_Id_ObjectInterface_Invoke);

    Value_AS2ObjectData o(this, pdata, isdobj);
    if (!o.pObject)
        return false;

    AS2::Value member, result;

    // Look for member
    if (!o.pObject->GetConstMemberRaw(o.pEnv->GetSC(), name, &member))
    {
        // Warn?
        return false;
    }

    AS2::Value asArg;
    for (int i=(int)nargs-1; i > -1; i--)
    {
        o.pRoot->Value2ASValue(pargs[i], &asArg);
        o.pEnv->Push(asArg);
    }
    bool retVal = GAS_Invoke(member, &result, o.pObject, o.pEnv, (int)nargs, o.pEnv->GetTopIndex(), NULL);
    o.pEnv->Drop((unsigned)nargs);

    // Process the return value if needed
    if (presult)
        o.pRoot->ASValue2Value(o.pEnv, result, presult);

    return retVal;
}

bool GFX_Value_ObjectInterface_CLASS::InvokeClosure(void* pdata, UPInt dataAux, Value* presult, 
                                                    const Value* pargs, UPInt nargs)
{
    SF_UNUSED5(pdata, dataAux, presult, pargs, nargs);
    SF_DEBUG_ERROR(1, "The AS2 VM does not support closures!\n");
    SF_ASSERT(0);
    return false;
}

bool GFX_Value_ObjectInterface_CLASS::DeleteMember(void* pdata, const char* name, bool isdobj)
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::DeleteMember", Amp_Native_Function_Id_ObjectInterface_DeleteMember);
    Value_AS2ObjectData o(this, pdata, isdobj);
    if (!o.pObject)
        return false;
    return o.pObject->DeleteMember(o.pEnv->GetSC(), ASString(o.pEnv->CreateConstString(name)));
}

void GFX_Value_ObjectInterface_CLASS::VisitMembers(void* pdata, Value::ObjectVisitor* visitor, bool isdobj) const
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::VisitMembers", Amp_Native_Function_Id_ObjectInterface_VisitMembers);

    Value_AS2ObjectData   o(this, pdata, isdobj);
    AS2::ASStringContext* psc = o.pEnv->GetSC();

    class VisitorProxy : public AS2::ObjectInterface::MemberVisitor
    {
    public:
        VisitorProxy(AS2::MovieRoot* root, AS2::Environment* env, Value::ObjectVisitor* visitor) 
            : pMovieRoot(root), pEnv(env), pVisitor(visitor) 
        { }
        void    Visit(const ASString& name, const AS2::Value& val, UByte flags)
        {
            SF_UNUSED(flags);
            Value v;
            pMovieRoot->ASValue2Value(pEnv, val, &v);
            pVisitor->Visit(name.ToCStr(), v);
        };
    private:
        AS2::MovieRoot*        pMovieRoot;
        AS2::Environment*      pEnv;
        Value::ObjectVisitor*  pVisitor;
    } visitorProxy(o.pRoot, o.pEnv, visitor);

    o.pObject->VisitMembers(psc, &visitorProxy,
        AS2::ObjectInterface::VisitMember_Prototype | AS2::ObjectInterface::VisitMember_ChildClips);
}


bool GFX_Value_ObjectInterface_CLASS::IsInstanceOf(void* pdata, const char* className) const
{
	SF_UNUSED2(pdata, className);
    SF_DEBUG_ERROR(1, "Not implemented in AS2!\n");
    SF_ASSERT(0);
    return false;
}


bool GFX_Value_ObjectInterface_CLASS::IsByteArray(void* pdata) const
{
    SF_UNUSED(pdata);
    SF_DEBUG_ERROR(1, "The AS2 VM does not support ByteArray!\n");
    SF_ASSERT(0);
    return false;
}

unsigned GFX_Value_ObjectInterface_CLASS::GetByteArraySize(void* pdata) const
{
    SF_UNUSED(pdata);
    SF_DEBUG_ERROR(1, "The AS2 VM does not support ByteArray!\n");
    SF_ASSERT(0);
    return 0;
}

void GFX_Value_ObjectInterface_CLASS::SetByteArraySize(void* pdata, UPInt setSize)
{
	SF_UNUSED2(pdata, setSize);
    SF_DEBUG_ERROR(1, "The AS2 VM does not support ByteArray!\n");
    SF_ASSERT(0);
}

bool GFX_Value_ObjectInterface_CLASS::ReadFromByteArray(void* pdata, UByte *destBuff, UPInt destBuffSz) const
{
    SF_UNUSED3(pdata, destBuff, destBuffSz);
    SF_DEBUG_ERROR(1, "The AS2 VM does not support ByteArray!\n");
    SF_ASSERT(0);
    return false;
}

bool GFX_Value_ObjectInterface_CLASS::WriteToByteArray(void* pdata, const UByte *srcBuff, UPInt writeSize)
{
    SF_UNUSED3(pdata, srcBuff, writeSize);
    SF_DEBUG_ERROR(1, "The AS2 VM does not support ByteArray!\n");
    SF_ASSERT(0);
    return false;
}

void* GFX_Value_ObjectInterface_CLASS::GetRawDataPtr(void* pdata)
{
    SF_UNUSED(pdata);
    SF_DEBUG_ERROR(1, "The AS2 VM does not support ByteArray!\n");
    SF_ASSERT(0);
    return NULL;
}

unsigned GFX_Value_ObjectInterface_CLASS::GetArraySize(void* pdata) const
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::GetArraySize", Amp_Native_Function_Id_ObjectInterface_GetArraySize);
    AS2::ObjectInterface*   pobj = static_cast<AS2::ObjectInterface*>(pdata);
    const AS2::ArrayObject* parr = static_cast<const AS2::ArrayObject*>(pobj);
    return (unsigned)parr->GetSize();
}

bool GFX_Value_ObjectInterface_CLASS::SetArraySize(void* pdata, unsigned sz)
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::SetArraySize", Amp_Native_Function_Id_ObjectInterface_SetArraySize);
    AS2::ObjectInterface* pobj = static_cast<AS2::ObjectInterface*>(pdata);
    AS2::ArrayObject*     parr = static_cast<AS2::ArrayObject*>(pobj);
    parr->Resize(sz);
    return true;
}

bool GFX_Value_ObjectInterface_CLASS::GetElement(void* pdata, unsigned idx, Value *pval) const
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::GetElement", Amp_Native_Function_Id_ObjectInterface_GetElement);
    AS2::ObjectInterface*   pobj = static_cast<AS2::ObjectInterface*>(pdata);
    const AS2::ArrayObject* parr = static_cast<const AS2::ArrayObject*>(pobj);

    pval->SetUndefined();
    if (idx >= (unsigned)parr->GetSize()) return false;

    const AS2::Value* pelem = parr->GetElementPtr(idx);
    if (!pelem) return false;

    AS2::MovieRoot*   proot = static_cast<AS2::MovieRoot*>(GetMovieImpl()->pASMovieRoot.GetPtr());
    AS2::Environment* penv  = AS2::ToAvmSprite(proot->GetLevel0Movie())->GetASEnvironment();
    proot->ASValue2Value(penv, *pelem, pval);
    return true;
}

bool GFX_Value_ObjectInterface_CLASS::SetElement(void* pdata, unsigned idx, const Value& value)
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::SetElement", Amp_Native_Function_Id_ObjectInterface_SetElement);
    AS2::ObjectInterface* pobj = static_cast<AS2::ObjectInterface*>(pdata);
    AS2::ArrayObject*     parr = static_cast<AS2::ArrayObject*>(pobj);

    AS2::MovieRoot* proot = static_cast<AS2::MovieRoot*>(GetMovieImpl()->pASMovieRoot.GetPtr());    
    AS2::Value asval;
    proot->Value2ASValue(value, &asval);
    parr->SetElementSafe(idx, asval);
    return true;
}

void GFX_Value_ObjectInterface_CLASS::VisitElements(void* pdata, Value::ArrayVisitor* visitor, unsigned idx, int count) const
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::VisitElements", Amp_Native_Function_Id_ObjectInterface_VisitElements);
    AS2::ObjectInterface* pobj = static_cast<AS2::ObjectInterface*>(pdata);
    AS2::ArrayObject*     parr = static_cast<AS2::ArrayObject*>(pobj);
    AS2::MovieRoot*       proot= static_cast<AS2::MovieRoot*>(GetMovieImpl()->pASMovieRoot.GetPtr());
    AS2::Environment*     penv = AS2::ToAvmSprite(proot->GetLevel0Movie())->GetASEnvironment();
    Value val;

    unsigned sz = (unsigned)parr->GetSize();
    if (idx >= sz) return;
    if (count < 0) { count = (sz - idx); }

    unsigned eidx = Alg::Min(sz, idx + count);
    for (unsigned i=idx; i < eidx; i++)
    {
        const AS2::Value* pelem = parr->GetElementPtr(i);
        if (pelem)
            proot->ASValue2Value(penv, *pelem, &val);
        else
            val.SetUndefined();
        visitor->Visit(i, val);
    }
}

bool GFX_Value_ObjectInterface_CLASS::PushBack(void *pdata, const Value &value)
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::PushBack", Amp_Native_Function_Id_ObjectInterface_PushBack);
    AS2::ObjectInterface* pobj = static_cast<AS2::ObjectInterface*>(pdata);
    AS2::ArrayObject*     parr = static_cast<AS2::ArrayObject*>(pobj);
    AS2::MovieRoot*       proot= static_cast<AS2::MovieRoot*>(GetMovieImpl()->pASMovieRoot.GetPtr());

    AS2::Value asval;
    proot->Value2ASValue(value, &asval);
    parr->PushBack(asval);
    return true;
}

bool GFX_Value_ObjectInterface_CLASS::PopBack(void* pdata, Value* pval)
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::PopBack", Amp_Native_Function_Id_ObjectInterface_PopBack);
    AS2::ObjectInterface* pobj = static_cast<AS2::ObjectInterface*>(pdata);
    AS2::ArrayObject*     parr = static_cast<AS2::ArrayObject*>(pobj);
    AS2::MovieRoot*       proot= static_cast<AS2::MovieRoot*>(GetMovieImpl()->pASMovieRoot.GetPtr());
    AS2::Environment*     penv = AS2::ToAvmSprite(proot->GetLevel0Movie())->GetASEnvironment();

    int sz = parr->GetSize();
    if (sz <= 0) 
    {
        if (pval) pval->SetUndefined();
        return false;
    }

    if (pval)
    {
        AS2::Value* pasval = parr->GetElementPtr(sz - 1);
        proot->ASValue2Value(penv, *pasval, pval);
    }

    parr->PopBack();
    return true;
}

bool GFX_Value_ObjectInterface_CLASS::RemoveElements(void *pdata, unsigned idx, int count)
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::RemoveElements", Amp_Native_Function_Id_ObjectInterface_RemoveElements);
    AS2::ObjectInterface* pobj = static_cast<AS2::ObjectInterface*>(pdata);
    AS2::ArrayObject*     parr = static_cast<AS2::ArrayObject*>(pobj);

    unsigned sz = (unsigned)parr->GetSize();
    if (idx >= sz) return false;
    if (count < 0) { count = (sz - idx); }

    parr->RemoveElements(idx, Alg::Min(sz - idx, (unsigned)count));
    return true;
}

bool GFX_Value_ObjectInterface_CLASS::GetDisplayMatrix(void* pdata, Render::Matrix2F* pmat) const
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::GetDisplayMatrix", Amp_Native_Function_Id_ObjectInterface_GetDisplayMatrix);
    CharacterHandle* pch = static_cast<CharacterHandle*>(pdata);
    DisplayObject*   pd = pch->ResolveCharacter(GetMovieImpl());
    if (!pd) return false;

    Render::Matrix2F m = pd->GetMatrix();
    m.Tx() = TwipsToPixels(m.Tx());
    m.Ty() = TwipsToPixels(m.Ty());
    *pmat = m;

    return true;
}

bool GFX_Value_ObjectInterface_CLASS::GetWorldMatrix(void* pdata, Render::Matrix2F* pmat) const
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::GetWorldMatrix", Amp_Native_Function_Id_ObjectInterface_GetWorldMatrix);
    CharacterHandle* pch = static_cast<CharacterHandle*>(pdata);
    DisplayObject*   pd = pch->ResolveCharacter(GetMovieImpl());
    if (!pd) return false;

    Render::Matrix2F m = pd->GetWorldMatrix();
    m.Tx() = TwipsToPixels(m.Tx());
    m.Ty() = TwipsToPixels(m.Ty());
    *pmat = m;

    return true;
}

bool GFX_Value_ObjectInterface_CLASS::SetDisplayMatrix(void* pdata, const Render::Matrix2F& mat)
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::SetDisplayMatrix", Amp_Native_Function_Id_ObjectInterface_SetDisplayMatrix);
    CharacterHandle* pch = static_cast<CharacterHandle*>(pdata);
    DisplayObject*   pd = pch->ResolveCharacter(GetMovieImpl());
    if (!pd) return false;

    // Extra checks
    if (!mat.IsValid())
    {
        SF_DEBUG_WARNING(1, "GFxValue::SetDisplayMatrix called with an invalid matrix.\n"
            "At least one of the matrix values is invalid (non-finite).\n");
        return false;
    }

    Render::Matrix2F m = mat;
    m.Tx() = PixelsToTwips(m.Tx());
    m.Ty() = PixelsToTwips(m.Ty());
    pd->SetMatrix(m);

    // Also set the geom data structure to reflect changes for the runtime
    DisplayObject::GeomDataType geomData;
    pd->GetGeomData(geomData);
    geomData.X = int(mat.Tx());
    geomData.Y = int(mat.Ty());
    geomData.XScale = mat.GetXScaleDouble()*(Double)100.;
    geomData.YScale = mat.GetYScaleDouble()*(Double)100.;
    geomData.Rotation = (mat.GetRotationDouble()*(Double)180.)/SF_MATH_PI;
    pd->SetGeomData(geomData);

    return true;
}

bool GFX_Value_ObjectInterface_CLASS::GetMatrix3D(void* pdata, Render::Matrix3F* pmat) const
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::GetMatrix3D", Amp_Native_Function_Id_ObjectInterface_GetMatrix3D);
    CharacterHandle* pch = static_cast<CharacterHandle*>(pdata);
    DisplayObject*   pd = pch->ResolveCharacter(GetMovieImpl());
    if (!pd) return false;

    Render::Matrix3F m = pd->GetMatrix3D();
    // Z is not stored in TWIPS in AS2
    m.Tx() = TwipsToPixels(m.Tx());
    m.Ty() = TwipsToPixels(m.Ty());
    *pmat = m;

    return true;
}


bool GFX_Value_ObjectInterface_CLASS::SetMatrix3D(void* pdata, const Render::Matrix3F& mat)
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::SetMatrix3D", Amp_Native_Function_Id_ObjectInterface_SetMatrix3D);
    CharacterHandle* pch = static_cast<CharacterHandle*>(pdata);
    DisplayObject*   pd = pch->ResolveCharacter(GetMovieImpl());
    if (!pd) return false;

    // Extra checks
    if (!mat.IsValid())
    { 
        SF_DEBUG_WARNING(1, "GFxValue::SetDisplayMatrix called with an invalid matrix.\n"
            "At least one of the matrix values is invalid (non-finite).\n");
        return false;
    }

    Render::Matrix3F m = mat;
    // Z is not stored in TWIPS in AS2
    m.Tx() = PixelsToTwips(m.Tx());
    m.Ty() = PixelsToTwips(m.Ty());
    pd->SetMatrix3D(m);

    // Also set the geom data structure to reflect changes for the runtime
    DisplayObject::GeomDataType geomData;
    pd->GetGeomData(geomData);
    geomData.Z = mat.Tz();
    geomData.ZScale = mat.GetZScale()*(Double)100.;

    float xr, yr;
    mat.GetRotation(&xr, &yr, NULL);
    geomData.XRotation = SF_RADTODEG(xr);
    geomData.YRotation = SF_RADTODEG(yr);

    pd->SetGeomData(geomData);

    return true;
}

bool    GFX_Value_ObjectInterface_CLASS::IsDisplayObjectActive(void* pdata) const
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::IsDisplayObjectActive", Amp_Native_Function_Id_ObjectInterface_IsDisplayObjectActive);

    CharacterHandle* pch = static_cast<CharacterHandle*>(pdata);
    DisplayObject*   pd = pch->ResolveCharacter(GetMovieImpl());
    return pd != NULL;
}

bool    GFX_Value_ObjectInterface_CLASS::GetParent(void* pdata, Value* pparent) const
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::GetParent", Amp_Native_Function_Id_ObjectInterface_GetParent);

    CharacterHandle* pch = static_cast<CharacterHandle*>(pdata);
    DisplayObject*   pd = pch->ResolveCharacter(GetMovieImpl());

    if(pd == NULL) return false;

    DisplayObject*    pdp = pd->GetParent();
    if(pdp != NULL)
    {
        AS2::MovieRoot*   proot= static_cast<AS2::MovieRoot*>(GetMovieImpl()->pASMovieRoot.GetPtr());
        return proot->CreateObjectValue(pparent, proot->GetMovieImpl()->pObjectInterface, pdp->GetCharacterHandle(), true);
    }
    else
    {
        pparent->SetNull();
        return true;
    }
}

bool GFX_Value_ObjectInterface_CLASS::GetDisplayInfo(void* pdata, Value::DisplayInfo* pinfo) const
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::GetDisplayInfo", Amp_Native_Function_Id_ObjectInterface_GetDisplayInfo);
    CharacterHandle* pch = static_cast<CharacterHandle*>(pdata);
    DisplayObject*   pd = pch->ResolveCharacter(GetMovieImpl());
    if (!pd) return false;

    DisplayObject::GeomDataType geomData;
    pd->GetGeomData(geomData);
    if (pd->GetType() == CharacterDef::TextField)
    {
        // Special case for TextField
        ((TextField*)pd)->GetPosition(pinfo);
    }
    else
    {
        Double x = TwipsToPixels(Double(geomData.X));
        Double y = TwipsToPixels(Double(geomData.Y));
        Double rotation = geomData.Rotation;
        Double xscale = geomData.XScale;
        Double yscale = geomData.YScale;
        Double alpha = pd->GetCxform().M[0][3] * 100.F;
        bool visible = pd->GetVisible();
        Double xrotation = geomData.XRotation;
        Double yrotation = geomData.YRotation;
        Double zscale = geomData.ZScale;
        Double z = geomData.Z;
        pinfo->Set(x, y, rotation, xscale, yscale, alpha, visible, z, xrotation, yrotation, zscale);
    }

    pinfo->SetFOV(pd->GetFOV());        
    Matrix4F matProj;
    if (pd->GetProjectionMatrix3D(&matProj))
        pinfo->SetProjectionMatrix3D(&matProj);

    Matrix3F matView;
    if (pd->GetViewMatrix3D(&matView))
        pinfo->SetViewMatrix3D(&matView);

    Render::EdgeAAMode edgeaaMode = pd->GetRenderNode()->GetEdgeAAMode();
    pinfo->SetEdgeAAMode(edgeaaMode);

    return true;
}

bool GFX_Value_ObjectInterface_CLASS::SetDisplayInfo(void *pdata, const Value::DisplayInfo &cinfo)
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::SetDisplayInfo", Amp_Native_Function_Id_ObjectInterface_SetDisplayInfo);
    // The getter code was lifted from InteractiveObject::SetStandardMember, however
    // some logic was compressed for efficiency (e.g.: set the matrix once, instead
    // of 5 different times - worst case)

    CharacterHandle* pch = static_cast<CharacterHandle*>(pdata);
    DisplayObject*   pd = pch->ResolveCharacter(GetMovieImpl());
    if (!pd) return false;

    bool Update3DTransforms = false;

    // Check for TextField special case:
    bool istf = (pd->GetType() == CharacterDef::TextField);
    TextField* ptf = static_cast<TextField*>(pd);

    if (cinfo.IsFlagSet(DisplayInfo::V_edgeaaMode))
    {
        pd->GetRenderNode()->SetEdgeAAMode(cinfo.GetEdgeAAMode());
    }

    if (cinfo.IsFlagSet(DisplayInfo::V_alpha))
    {
        if (!NumberUtil::IsNaN(cinfo.GetAlpha()))
        {
            // Set alpha modulate, in percent.
            InteractiveObject::Cxform  cx = pd->GetCxform();
            cx.M[0][3] = float(cinfo.GetAlpha() / 100.);
            pd->SetCxform(cx);
            pd->SetAcceptAnimMoves(0);
        }
    }
    if (cinfo.IsFlagSet(DisplayInfo::V_visible))
    {
        pd->SetVisible(cinfo.GetVisible());
    }

    if (cinfo.IsFlagSet(DisplayInfo::V_z))
    {
        Double zval = cinfo.GetZ();
        if (NumberUtil::IsNaN(zval))
            zval = 0;
        if (NumberUtil::IsNEGATIVE_INFINITY(zval) || NumberUtil::IsPOSITIVE_INFINITY(zval))
            zval = 0;

        pd->EnsureGeomDataCreated(); // let timeline anim continue
        DisplayObjectBase::GeomDataType* tfgeomData = pd->GetGeomDataPtr();
        SF_ASSERT(tfgeomData);

        if (zval != tfgeomData->Z)
        {
            tfgeomData->Z = zval;
            Update3DTransforms = true;
        }
    }

    if (cinfo.IsFlagSet(DisplayInfo::V_zscale))
    {
        Double newZScale = cinfo.GetZScale();
        if (NumberUtil::IsNaN(newZScale) ||
            NumberUtil::IsNEGATIVE_INFINITY(newZScale) || NumberUtil::IsPOSITIVE_INFINITY(newZScale))
        {
            newZScale = 100.;
        }

        pd->EnsureGeomDataCreated(); // let timeline anim continue
        DisplayObjectBase::GeomDataType* tfgeomData = pd->GetGeomDataPtr();
        SF_ASSERT(tfgeomData);

        if (tfgeomData->ZScale != newZScale)
        {
            tfgeomData->ZScale = newZScale;
            Update3DTransforms = true;
        }
    }

    if (cinfo.IsFlagSet(DisplayInfo::V_xrotation))
    {
        Double rval = cinfo.GetXRotation();

        pd->EnsureGeomDataCreated(); // let timeline anim continue
        DisplayObjectBase::GeomDataType* tfgeomData = pd->GetGeomDataPtr();
        SF_ASSERT(tfgeomData);

        if (rval != tfgeomData->XRotation)
        {       
            Double r = fmod((Double)rval, (Double)360.);
            if (r > 180)
                r -= 360;
            else if (r < -180)
                r += 360;

            tfgeomData->XRotation = r;
            Update3DTransforms = true;
        }
    }

    if (cinfo.IsFlagSet(DisplayInfo::V_yrotation))
    {
        Double rval = cinfo.GetYRotation();

        pd->EnsureGeomDataCreated(); // let timeline anim continue
        DisplayObjectBase::GeomDataType* tfgeomData = pd->GetGeomDataPtr();
        SF_ASSERT(tfgeomData);

        if (rval != tfgeomData->YRotation)
        {      
            Double r = fmod((Double)rval, (Double)360.);
            if (r > 180)
                r -= 360;
            else if (r < -180)
                r += 360;

            tfgeomData->YRotation= r;
            Update3DTransforms = true;
        }
    }

    if (Update3DTransforms)
        pd->UpdateTransform3D();

    if (cinfo.IsFlagSet(DisplayInfo::V_FOV))
    {
        Double rval = cinfo.GetFOV();
        if (rval != pd->GetFOV())
        {       
            Double r = fmod((Double)rval, (Double)180.);
            pd->SetFOV(r);
        }
    }
    else
    {
        // don't set view and projection if FOV was set, since that also sets view and projection
        if (cinfo.IsFlagSet(DisplayInfo::V_projMatrix3D))
        {
            pd->SetProjectionMatrix3D(*cinfo.GetProjectionMatrix3D());
        }

        if (cinfo.IsFlagSet(DisplayInfo::V_viewMatrix3D))
        {
            pd->SetViewMatrix3D(*cinfo.GetViewMatrix3D());
        }
    }

    if (cinfo.IsFlagSet(DisplayInfo::V_x|DisplayInfo::V_y|DisplayInfo::V_rotation|
                       DisplayInfo::V_xscale|DisplayInfo::V_yscale))
    {       
        // Special case for TextField:
        if (istf)
            ptf->SetNeedUpdateGeomData();

        pd->SetAcceptAnimMoves(0);
        DisplayObjectBase::GeomDataType* tfgeomData = pd->GetGeomDataPtr();
        SF_ASSERT(tfgeomData);

        InteractiveObject::Matrix m = pd->GetMatrix();

        Render::PointD pt;
        // Special case for TextField:
        if (istf && cinfo.IsFlagSet(DisplayInfo::V_x|DisplayInfo::V_y))
        {
            Render::PointF tpt = ptf->TransformToTextRectSpace(cinfo);
            pt.x = tpt.x;
            pt.y = tpt.y;
        }
        else
        {
            if (cinfo.IsFlagSet(DisplayInfo::V_x))
                pt.x = cinfo.GetX();
            if (cinfo.IsFlagSet(DisplayInfo::V_y))
                pt.y = cinfo.GetY();
        }

        // Perform scale/rotation before x/y is set.
        // If order should be changed, call this method twice in expected order.

        if (cinfo.IsFlagSet(DisplayInfo::V_rotation|DisplayInfo::V_xscale|DisplayInfo::V_yscale))
        {
            InteractiveObject::Matrix om = tfgeomData->OrigMatrix;
            om.Tx() = m.Tx();
            om.Ty() = m.Ty();

            Double origRotation = om.GetRotationDouble();
            Double origXScale = om.GetXScaleDouble();
            Double origYScale = om.GetYScaleDouble();
            
            Double newXScale = tfgeomData->XScale / 100;
            Double newYScale = tfgeomData->YScale / 100;
            Double newRotation = SF_DEGTORAD(tfgeomData->Rotation);

            // _rotation
            Double rval = cinfo.IsFlagSet(DisplayInfo::V_rotation) ? cinfo.GetRotation() : NumberUtil::NaN();
            if (!NumberUtil::IsNaN(rval))
            {
                newRotation = fmod((Double)rval, (Double)360.);
                if (newRotation > 180)
                    newRotation -= 360;
                else if (newRotation < -180)
                    newRotation += 360;
                tfgeomData->Rotation = newRotation;
                newRotation = SF_DEGTORAD(newRotation); // Convert to rads
            }
            // _xscale
            Double nx = cinfo.IsFlagSet(DisplayInfo::V_xscale) ? (cinfo.GetXScale()/100) : NumberUtil::NaN();
            if ((nx != newXScale) && !NumberUtil::IsNaNOrInfinity(nx))
            {
                tfgeomData->XScale = cinfo.GetXScale();
                if (origXScale == 0 || nx > 1E+16)
                {
                    nx = 0;
                    origXScale = 1;
                }
                newXScale = nx;
            }
            // _yscale
            Double ny = cinfo.IsFlagSet(DisplayInfo::V_yscale) ? (cinfo.GetYScale()/100) : NumberUtil::NaN();
            if ((ny != newYScale) && !NumberUtil::IsNaNOrInfinity(ny))
            {                
                tfgeomData->YScale = cinfo.GetYScale();
                if (origYScale == 0 || ny > 1E+16)
                {
                    ny = 0;
                    origYScale = 1;
                }
                newYScale = ny;
            }              

            // remove old rotation by rotate back and add new one

            ASCharacter_MatrixScaleAndRotate2x2(om,
                float(newXScale/origXScale), 
                float(newYScale/origYScale),
                float(newRotation - origRotation));

            m = om;
        }

        // _x
        Double xval = cinfo.IsFlagSet(DisplayInfo::V_x) ? Double(pt.x) : NumberUtil::NaN();
        if (!NumberUtil::IsNaN(xval))
        {
            if (NumberUtil::IsNEGATIVE_INFINITY(xval) || NumberUtil::IsPOSITIVE_INFINITY(xval))
                xval = 0;
            tfgeomData->X = int(floor(PixelsToTwips(xval)));
            m.Tx() = (float) tfgeomData->X;
        }
        // _y
        Double yval = cinfo.IsFlagSet(DisplayInfo::V_y) ? Double(pt.y) : NumberUtil::NaN();
        if (!NumberUtil::IsNaN(yval))
        {
            if (NumberUtil::IsNEGATIVE_INFINITY(yval) || NumberUtil::IsPOSITIVE_INFINITY(yval))
                yval = 0;
            tfgeomData->Y = int(floor(PixelsToTwips(yval)));
            m.Ty() = (float) tfgeomData->Y;
        }

        if (m.IsValid())
        {
            if (pd->Is3D())     // update the 3D transform so we don't change the object into a 2D object
                pd->UpdateTransform3D();
            else
                pd->SetMatrix(m);
        }

        // Special case for TextField:
        if (istf) 
        {
            if (cinfo.IsFlagSet(Value::DisplayInfo::V_x))
                tfgeomData->X = Alg::IRound(PixelsToTwips(pt.x));
            if (cinfo.IsFlagSet(Value::DisplayInfo::V_y))
                tfgeomData->Y = Alg::IRound(PixelsToTwips(pt.y));
        }
    }

    return true;
}

bool GFX_Value_ObjectInterface_CLASS::SetText(void *pdata, const char *ptext, bool reqHtml)
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::SetText", Amp_Native_Function_Id_ObjectInterface_SetText);
    CharacterHandle* pch = static_cast<CharacterHandle*>(pdata);
    DisplayObject*   pd = pch->ResolveCharacter(GetMovieImpl());
    if (!pd) return false;

    if (pd->GetType() != CharacterDef::TextField)
        return SetMember(pdata, (reqHtml) ? "htmlText" : "text", ptext, true);
    else 
    {
        TextField* ptf = static_cast<TextField*>(pd);
        ptf->SetText(ptext, reqHtml);
        return true;
    }
}

bool GFX_Value_ObjectInterface_CLASS::SetText(void *pdata, const wchar_t *ptext, bool reqHtml)
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::SetText", Amp_Native_Function_Id_ObjectInterface_SetText);
    CharacterHandle* pch = static_cast<CharacterHandle*>(pdata);
    DisplayObject* pd = pch->ResolveCharacter(GetMovieImpl());
    if (!pd) return false;

    if (pd->GetType() != CharacterDef::TextField)
        return SetMember(pdata, (reqHtml) ? "htmlText" : "text", ptext, true);
    else 
    {
        TextField* ptf = static_cast<TextField*>(pd);
        ptf->SetText(ptext, reqHtml);
        return true;
    }
}

bool GFX_Value_ObjectInterface_CLASS::GetText(void* pdata, Value* pval, bool reqHtml) const
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::GetText", Amp_Native_Function_Id_ObjectInterface_GetText);
    CharacterHandle* pch = static_cast<CharacterHandle*>(pdata);
    DisplayObject*   pd = pch->ResolveCharacter(GetMovieImpl());
    if (!pd) return false;

    if (pd->GetType() != CharacterDef::TextField)
        return GetMember(pdata, (reqHtml) ? "htmlText" : "text", pval, true);
    else 
    {
        AS2::MovieRoot*   proot= static_cast<AS2::MovieRoot*>(GetMovieImpl()->pASMovieRoot.GetPtr());
        AS2::Environment* penv = AS2::ToAvmSprite(proot->GetLevel0Movie())->GetASEnvironment();
        
        TextField* ptf = static_cast<TextField*>(pd);
        ASString astext = ptf->GetText(reqHtml);
        proot->ASValue2Value(penv, astext, pval);
        return true;
    }
}


bool GFX_Value_ObjectInterface_CLASS::GotoAndPlay(void *pdata, const char *frame, bool stop)
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::GotoAndPlay", Amp_Native_Function_Id_ObjectInterface_GotoAndPlay);
    CharacterHandle* pch = static_cast<CharacterHandle*>(pdata);
    DisplayObject*   pd = pch->ResolveCharacter(GetMovieImpl());
    if (!pd || !pd->IsSprite())
        return false;

    unsigned frameNum;
    Sprite* pspr = pd->CharToSprite_Unsafe();
    if (pspr->GetLabeledFrame(frame, &frameNum))
    {
        pspr->GotoFrame(frameNum);
        pspr->SetPlayState(stop ? State_Stopped : State_Playing);
        return true;
    }
    return false;
}

bool GFX_Value_ObjectInterface_CLASS::GotoAndPlay(void *pdata, unsigned frame, bool stop)
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::GotoAndPlay", Amp_Native_Function_Id_ObjectInterface_GotoAndPlay);
    CharacterHandle* pch = static_cast<CharacterHandle*>(pdata);
    DisplayObject*   pd = pch->ResolveCharacter(GetMovieImpl());
    if (!pd || !pd->IsSprite())
        return false;

    // Incoming frame number is 1-based, but internally GFx is 0 based.
    Sprite* pspr = pd->CharToSprite_Unsafe();
    pspr->GotoFrame(frame-1);
    pspr->SetPlayState(stop ? State_Stopped : State_Playing);
    return true;
}

bool GFX_Value_ObjectInterface_CLASS::GetCxform(void *pdata, Render::Cxform *pcx) const
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::GetCxform", Amp_Native_Function_Id_ObjectInterface_GetCxform);

    CharacterHandle* pch = static_cast<CharacterHandle*>(pdata);
    DisplayObject*   pd = pch->ResolveCharacter(GetMovieImpl());
    if (!pd) return false;
    bool istf = (pd->GetType() == CharacterDef::TextField);
    if (!istf && !pd->IsSprite()) return false;

    *pcx = pd->GetCxform();
    return true;
}

bool GFX_Value_ObjectInterface_CLASS::SetCxform(void *pdata, const Render::Cxform &cx)
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::SetCxform", Amp_Native_Function_Id_ObjectInterface_SetCxform);

    CharacterHandle* pch = static_cast<CharacterHandle*>(pdata);
    DisplayObject*   pd = pch->ResolveCharacter(GetMovieImpl());
    if (!pd) return false;
    bool istf = (pd->GetType() == CharacterDef::TextField);
    if (!istf && !pd->IsSprite()) return false;

    pd->SetCxform(cx);
    pd->SetAcceptAnimMoves(false);

    return true;
}

bool GFX_Value_ObjectInterface_CLASS::CreateEmptyMovieClip(void* pdata, GFx::Value* pmc, 
                                                     const char* instanceName, SInt32 depth)
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::CreateEmptyMovieClip", Amp_Native_Function_Id_ObjectInterface_CreateEmptyMovieClip);

    CharacterHandle* pch = static_cast<CharacterHandle*>(pdata);
    DisplayObject*   pd = pch->ResolveCharacter(GetMovieImpl());
    if (!pd || !pd->IsSprite())
        return false;

    Sprite* psprite = pd->CharToSprite_Unsafe();
    AS2::MovieRoot*   proot= static_cast<AS2::MovieRoot*>(GetMovieImpl()->pASMovieRoot.GetPtr());
    AS2::Environment* penv = AS2::ToAvmSprite(proot->GetLevel0Movie())->GetASEnvironment();

    // Check for specified depth; else get next highest depth
    if (depth < 0)
        depth = Alg::Max<int>(0, psprite->GetLargestDepthInUse() - 16384 + 1);

    // Create a new object and add it.
    CharPosInfo pos = CharPosInfo( ResourceId(CharacterDef::CharId_EmptyMovieClip),
        depth + 16384,
        1, Render::Cxform::Identity, 1, Matrix2F::Identity);

    // Bounds check depth.
    if ((pos.Depth < 0) || (pos.Depth > (2130690045 + 16384)))
        return false;

    Ptr<DisplayObjectBase> newCh = psprite->AddDisplayObject(
        pos, penv->CreateString(instanceName), NULL, NULL, SF_MAX_UINT, 
        DisplayList::Flags_ReplaceIfDepthIsOccupied);  

    if (newCh)
    {
        newCh->SetAcceptAnimMoves(false);

        // Return newly created clip.
        InteractiveObject* pspriteCh = newCh->CharToInteractiveObject_Unsafe();
        SF_ASSERT (pspriteCh != 0);
        AS2::Value asval(pspriteCh);
        proot->ASValue2Value(penv, asval, pmc);
    }
    return true;
}

bool GFX_Value_ObjectInterface_CLASS::AttachMovie(void* pdata, GFx::Value* pmc, 
                                            const char* symbolName, const char* instanceName, 
                                            SInt32 depth, const MemberValueSet* initArgs)
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::AttachMovie", Amp_Native_Function_Id_ObjectInterface_AttachMovie);

    CharacterHandle* pch = static_cast<CharacterHandle*>(pdata);
    DisplayObject*   pd = pch->ResolveCharacter(GetMovieImpl());
    if (!pd || !pd->IsSprite())
        return false;

    Sprite* psprite = pd->CharToSprite_Unsafe();
    AS2::MovieRoot*   proot= static_cast<AS2::MovieRoot*>(GetMovieImpl()->pASMovieRoot.GetPtr());
    AS2::Environment* penv = AS2::ToAvmSprite(proot->GetLevel0Movie())->GetASEnvironment();

    ResourceBindData resBindData;
    if (!psprite->GetMovieImpl()->FindExportedResource(psprite->GetResourceMovieDef(), &resBindData, symbolName))
    {
        psprite->LogScriptWarning("%s.attachMovie() failed - export name \"%s\" is not found.",
            psprite->GetCharacterHandle()->GetNamePath().ToCStr(), symbolName);
        return false;
    }

    SF_ASSERT(resBindData.pResource.GetPtr() != 0); // MA TBD: Could this be true?     
    if (!(resBindData.pResource->GetResourceType() & Resource::RT_CharacterDef_Bit))
    {
        psprite->LogScriptWarning("%s.attachMovie() failed - \"%s\" is not a movieclip.",
            psprite->GetCharacterHandle()->GetNamePath().ToCStr(), symbolName);
        return false;
    }

    CharacterCreateInfo ccinfo;
    ccinfo.pCharDef     = (CharacterDef*) resBindData.pResource.GetPtr();
    ccinfo.pBindDefImpl = resBindData.pBinding->GetOwnerDefImpl();

    // Check for specified depth; else get next highest depth
    if (depth < 0)
        depth = Alg::Max<int>(0, psprite->GetLargestDepthInUse() - 16384 + 1);

    // Create a new object and add it.
    CharPosInfo pos( ccinfo.pCharDef->GetId(), depth + 16384,
        1, Render::Cxform::Identity, 1, Matrix2F::Identity);
    // Bounds check depth.
    if ((pos.Depth < 0) || (pos.Depth > (2130690045 + 16384)))
    {
        psprite->LogScriptWarning("%s.attachMovie(\"%s\") failed - depth (%d) must be >= 0",
            psprite->GetCharacterHandle()->GetNamePath().ToCStr(), symbolName, pos.Depth);
        return false;
    }

    // Create an init obj
    Ptr<AS2::Object> pinitobj = NULL;
    if (initArgs)
    {
        pinitobj = *penv->OperatorNew(penv->GetBuiltin(AS2::ASBuiltin_Object));
        for (unsigned i = 0; i < initArgs->GetSize(); i++)
        {
            const MemberValue& mv = (*initArgs)[i];
            ASString propname = penv->CreateString(mv.Key);
            AS2::Value propval;
            proot->Value2ASValue(mv.mValue, &propval);
            pinitobj->SetMember(penv, propname, propval);
        }
    }

    // We pass pchDef to make sure that symbols from nested imports use
    // the right definition scope, which might be different from that of psprite.
    Ptr<DisplayObjectBase> newCh = psprite->AddDisplayObject(
        pos, penv->CreateString(instanceName), 0,
        (AS2::ObjectInterface*)pinitobj.GetPtr(), SF_MAX_UINT, 
        DisplayList::Flags_ReplaceIfDepthIsOccupied, &ccinfo);
    if (newCh)
    {
        newCh->SetAcceptAnimMoves(false);

        //!AB: attachMovie in Flash 6 and above should return newly created clip
        if (psprite->GetVersion() >= 6)
        {
            InteractiveObject* pspriteCh = newCh->CharToInteractiveObject_Unsafe(); 
            SF_ASSERT (pspriteCh != 0);
            AS2::Value asval(pspriteCh);
            proot->ASValue2Value(penv, asval, pmc);
        }
    }
    return true;
}

void GFX_Value_ObjectInterface_CLASS::ToString(String* pstr, const GFx::Value& thisVal) const
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::ToString", Amp_Native_Function_Id_ObjectInterface_ToString);
    AS2::MovieRoot*       proot= static_cast<AS2::MovieRoot*>(GetMovieImpl()->pASMovieRoot.GetPtr());
    AS2::Environment*     penv = AS2::ToAvmSprite(proot->GetLevel0Movie())->GetASEnvironment();

    AS2::Value asVal;
    proot->Value2ASValue(thisVal, &asVal);

    *pstr = String(asVal.ToString(penv).ToCStr());
}

#ifdef GFX_AS_ENABLE_USERDATA

ASUserData*  GFX_Value_ObjectInterface_CLASS::GetUserData(void* pdata, bool isdobj) const 
{
    Value_AS2ObjectData v(this, pdata, isdobj);
    if (v.pObject) return v.pObject->GetUserData();
    return NULL;
}

void    GFX_Value_ObjectInterface_CLASS::SetUserData(void* pdata, ASUserData* puserdata, bool isdobj) 
{
    Value_AS2ObjectData v(this, pdata, isdobj);
    if (v.pObject) v.pObject->SetUserData(pMovieRoot, puserdata, isdobj);
}

bool    GFX_Value_ObjectInterface_CLASS::CreateObjectValue(Value* pval, void* pdata, bool isdobj)
{
    AS2::MovieRoot*       proot= static_cast<AS2::MovieRoot*>(GetMovieImpl()->pASMovieRoot.GetPtr());
    return proot->CreateObjectValue(pval, this, pdata, isdobj);
}

#endif  // GFX_AS2_SUPPORT

#endif  // GFX_AS_ENABLE_USERDATA


// ***** MovieRoot definitions

namespace AS2 {

bool    MovieRoot::CreateObjectValue(GFx::Value* pval, GFx::Value::ObjectInterface* pobjifc, void* pdata, bool isdobj)
{
    GFx::Value::ValueType type = GFx::Value::VT_Object;
    if (isdobj)
    {
        type = GFx::Value::VT_DisplayObject;
        CharacterHandle* pch = static_cast<CharacterHandle*>(pdata);
        MovieImpl* pmovieImpl = GetMovieImpl();
        SF_ASSERT(pmovieImpl);
        DisplayObject* pd = pch->ResolveCharacter(pmovieImpl);
        if (!pd || (!pd->IsSprite() && !pd->IsInteractiveObject()))
        {
            return false;
        }
    }
    else
    {
        Object* pobj = static_cast<ObjectInterface*>(pdata)->ToASObject();
        if (pobj->GetObjectType() == AS2::ObjectInterface::Object_Array)
        {
            type = GFx::Value::VT_Array;
        }
    }

    // Clear existing managed ref first
    if (pval->IsManagedValue()) pval->ReleaseManagedValue();
    pval->Type = GFx::Value::ValueType(type | GFx::Value::VTC_ManagedBit);
    pval->mValue.pData = pdata;
    pval->pObjectInterface = pobjifc;
    // Make sure to add-ref before returning
    pobjifc->ObjectAddRef(pval, pdata);

    return true;
}


void MovieRoot::Value2ASValue(const GFx::Value& gfxVal, Value* pdestVal)
{   
	SF_ASSERT(pdestVal);
	switch(gfxVal.GetType())
	{
	case GFx::Value::VT_Undefined: pdestVal->SetUndefined(); break;
	case GFx::Value::VT_Null:      pdestVal->SetNull(); break;
    case GFx::Value::VT_Boolean:   pdestVal->SetBool(gfxVal.GetBool()); break;
    case GFx::Value::VT_Int:       pdestVal->SetInt(gfxVal.GetInt()); break;
    case GFx::Value::VT_UInt:      pdestVal->SetUInt(gfxVal.GetUInt()); break;
	case GFx::Value::VT_Number:    pdestVal->SetNumber(gfxVal.GetNumber()); break;
	case GFx::Value::VT_String:    
		{
			if (gfxVal.IsManagedValue())
			{
				size_t memberOffset = OFFSETOF(ASStringNode, pData);
				// Must be an assignment of a string from the same movie root
				SF_ASSERT(gfxVal.pObjectInterface->IsSameContext(pMovieImpl->pObjectInterface));
				ASStringNode* strnode = (ASStringNode*)(((UByte*)gfxVal.mValue.pStringManaged) - memberOffset);
				ASString str(strnode);
				pdestVal->SetString(str);
			}
			else
				pdestVal->SetString(GetStringManager()->CreateString(gfxVal.GetString()));
		}
		break;
	case GFx::Value::VT_StringW:    
		{
			if (gfxVal.IsManagedValue())
			{                
				size_t memberOffset = OFFSETOF(MovieImpl::WideStringStorage, pData);
				// Must be an assignment of a string from the same movie root
				SF_ASSERT(gfxVal.pObjectInterface->IsSameContext(pMovieImpl->pObjectInterface));
				MovieImpl::WideStringStorage* wstr = 
					(MovieImpl::WideStringStorage*)(((UByte*)gfxVal.mValue.pStringW) - memberOffset);
				ASString str(wstr->pNode);
				pdestVal->SetString(str);
			}
			else 
				pdestVal->SetString(GetStringManager()->CreateString(gfxVal.GetStringW()));
		}
		break;
	case GFx::Value::VT_Array:
	case GFx::Value::VT_Object:
		{
			// Must be an assignment of an object from the same movie root
			SF_ASSERT(gfxVal.pObjectInterface->IsSameContext(pMovieImpl->pObjectInterface));
			ObjectInterface* pifc = (ObjectInterface*)gfxVal.mValue.pData;
			pdestVal->SetAsObject((Object*) pifc);
		}
		break;
	case GFx::Value::VT_DisplayObject:
		{
			// Must be an assignment of an object from the same movie root
			SF_ASSERT(gfxVal.pObjectInterface->IsSameContext(pMovieImpl->pObjectInterface));
			CharacterHandle* pifc = (CharacterHandle*)gfxVal.mValue.pData;
			pdestVal->SetAsCharacterHandle(pifc);
		}
		break;
	default:
		break;
	}

}

void MovieRoot::ASValue2Value(Environment* penv, const Value& value, GFx::Value* pdestVal) const
{
	SF_ASSERT(pdestVal);
	int toType;
	if (pdestVal->GetType() & GFx::Value::VTC_ConvertBit)
		toType = pdestVal->GetType() & (~GFx::Value::VTC_ConvertBit);
	else
	{
		switch(value.GetType())
		{
		case Value::UNDEFINED:    toType = GFx::Value::VT_Undefined; break;
		case Value::BOOLEAN:      toType = GFx::Value::VT_Boolean; break;
		case Value::NULLTYPE:     toType = GFx::Value::VT_Null; break;
        case Value::INTEGER:      
		case Value::NUMBER:       toType = GFx::Value::VT_Number; break;
		case Value::OBJECT:       
        case Value::FUNCTION:
                                  toType = GFx::Value::VT_Object; break;
		case Value::CHARACTER:    toType = GFx::Value::VT_DisplayObject; break;
			// Value::PROPERTY is returned as a string 
		default:                  toType = GFx::Value::VT_String; break;
		}
	}

	// Clear existing ref
	if (pdestVal->IsManagedValue()) pdestVal->ReleaseManagedValue();

	switch(toType)
	{
	case GFx::Value::VT_Undefined:
	case GFx::Value::VT_Null:
		{
			pdestVal->Type = GFx::Value::ValueType(toType);
		}
		break;
	case GFx::Value::VT_Boolean:      
		{
			pdestVal->Type = GFx::Value::ValueType(toType);
			pdestVal->mValue.BValue = value.ToBool(penv);
		}
		break;
    case GFx::Value::VT_Int:
        {
            pdestVal->Type = GFx::Value::ValueType(toType);
            pdestVal->mValue.IValue = (SInt32)value.ToNumber(penv);
        }
        break;
    case GFx::Value::VT_UInt:
        {
            pdestVal->Type = GFx::Value::ValueType(toType);
            pdestVal->mValue.UIValue = (UInt32)value.ToNumber(penv);
        }
        break;
	case GFx::Value::VT_Number:
		{
			pdestVal->Type = GFx::Value::ValueType(toType);
			pdestVal->mValue.NValue = value.ToNumber(penv);
		}
		break;
	case GFx::Value::VT_String:    
		{
			size_t memberOffset = OFFSETOF(ASStringNode, pData);
			ASString str = value.ToString(penv);
			ASStringNode* strNode = str.GetNode();
			pdestVal->Type = GFx::Value::ValueType(GFx::Value::VT_String | GFx::Value::VTC_ManagedBit);
			pdestVal->mValue.pStringManaged = (const char**)(((UByte*)strNode) + memberOffset);
			pdestVal->pObjectInterface = pMovieImpl->pObjectInterface;
			pMovieImpl->pObjectInterface->ObjectAddRef(pdestVal, pdestVal->mValue.pData);
		}
		break;
	case GFx::Value::VT_StringW:    
		{   
			size_t memberOffset = OFFSETOF(MovieImpl::WideStringStorage, pData);
			ASString str = value.ToString(penv);
			ASStringNode* strNode = str.GetNode();
			unsigned len = str.GetLength() + 1;
			void* pdata = SF_HEAP_ALLOC(GetMovieHeap(), sizeof(MovieImpl::WideStringStorage) - sizeof(UByte) + (sizeof(wchar_t) * len), StatMV_Other_Mem);
			pdestVal->Type = GFx::Value::ValueType(GFx::Value::VT_StringW | GFx::Value::VTC_ManagedBit);
			Ptr<MovieImpl::WideStringStorage> wstr = *::new(pdata) MovieImpl::WideStringStorage(strNode, len);
			pdestVal->mValue.pStringW = (const wchar_t*)(((UByte*)wstr.GetPtr()) + memberOffset);
			pdestVal->pObjectInterface = pMovieImpl->pObjectInterface;            
			pMovieImpl->pObjectInterface->ObjectAddRef(pdestVal, pdestVal->mValue.pData);
		}
		break;
	case GFx::Value::VT_Object:
		{
			GFx::Value::ValueType type = GFx::Value::VT_Object;
			ObjectInterface* pifc = (ObjectInterface*)value.ToObjectInterface(penv);
			if (pifc->IsASObject())
			{
				Object* pobj = pifc->ToASObject();
				if (pobj->GetObjectType() == ObjectInterface::Object_Array)
					type = GFx::Value::VT_Array;
			}
			pdestVal->Type = GFx::Value::ValueType(type | GFx::Value::VTC_ManagedBit);
			pdestVal->mValue.pData = pifc;
			pdestVal->pObjectInterface = pMovieImpl->pObjectInterface;
		    pMovieImpl->pObjectInterface->ObjectAddRef(pdestVal, pdestVal->mValue.pData);
		}
		break;
	case GFx::Value::VT_DisplayObject:
		{
			ObjectInterface* pifc = (ObjectInterface*)value.ToObjectInterface(penv);
            if (!pifc) 
            {
                pdestVal->Type = GFx::Value::VT_Undefined;
                return;
            }
			InteractiveObject* aschar = pifc->ToCharacter();
			pdestVal->Type = GFx::Value::ValueType(GFx::Value::VT_DisplayObject | GFx::Value::VTC_ManagedBit);
			pdestVal->mValue.pData = aschar->GetCharacterHandle();
			pdestVal->pObjectInterface = pMovieImpl->pObjectInterface;
			pMovieImpl->pObjectInterface->ObjectAddRef(pdestVal, pdestVal->mValue.pData);
		}
	}
}

void MovieRoot::CreateObjectInterface(MovieImpl* movie)
{
    SF_ASSERT(!movie->pObjectInterface);
    movie->pObjectInterface = 
        SF_HEAP_NEW(GetMovieHeap()) GFX_Value_ObjectInterface_CLASS(movie);
}

void MovieRoot::CreateString(GFx::Value* pvalue, const char* pstring)
{
    Environment* penv = ToAvmSprite(GetLevel0Movie())->GetASEnvironment();
    ASString asstr = penv->CreateString(pstring);
    ASValue2Value(penv, asstr, pvalue);
}

void MovieRoot::CreateStringW(GFx::Value* pvalue, const wchar_t* pstring)
{
    Environment* penv = ToAvmSprite(GetLevel0Movie())->GetASEnvironment();
    ASString asstr = penv->CreateString(pstring);
    pvalue->SetConvertStringW();
    ASValue2Value(penv, asstr, pvalue);
}

void MovieRoot::CreateObject(GFx::Value* pvalue, const char* className,
                             const GFx::Value* pargs, unsigned nargs)
{
    Environment* penv = ToAvmSprite(GetLevel0Movie())->GetASEnvironment();
    Ptr<Object> pobj;

    if (className)
    {
        // Push params to stack
        if (nargs > 0)
        {
            for (int i=(nargs-1); i > -1; --i)
            {
                Value asval;
                Value2ASValue(pargs[i], &asval);
                penv->Push(asval);
            }
        }

        // Check if the className is part of a package (has '.')
        const char* p = strchr(className, '.');
        if (p)
        {
            // For package resolution
            ASStringContext* psc = penv->GetSC();
            char            buf[256];
            const char*     pname = className;
            Ptr<Object> parent = penv->GetGC()->pGlobal;

            while(pname)
            {
                const char* pal = strchr(pname, '.');
                UPInt sz;
                if (pal)
                    sz = pal++ - pname + 1;
                else
                    break;  // class name found

                SF_ASSERT(sz <= sizeof(buf));
                if (sz > sizeof(buf)) 
                    sz = sizeof(buf);

                memcpy(buf, pname, sz-1);
                buf[sz-1] = '\0';

                pname = pal;

                Value        pkgObjVal;
                Ptr<Object> pkgObj;
                ASString       memberName(psc->CreateString(buf));

                if (parent->GetMemberRaw(psc, memberName, &pkgObjVal))
                    pkgObj = pkgObjVal.ToObject(NULL);
                else
                {
                    pvalue->SetUndefined();
                    return; // Package in path not found; bail
                }

                parent = pkgObj;
            }
            
            pobj = *penv->OperatorNew(parent, penv->CreateString(pname), nargs);
        }
        else
            pobj = *penv->OperatorNew(penv->CreateString(className), nargs);
        
        if (pobj)
            ASValue2Value(penv, Value(pobj), pvalue);
        else
            pvalue->SetUndefined();

        // Cleanup; pop params from stack
        if (nargs > 0)
            penv->Drop(nargs);
    }
    else
    {
        pobj = *penv->OperatorNew(penv->GetBuiltin(ASBuiltin_Object));
        ASValue2Value(penv, Value(pobj), pvalue);
    }    
}

void AS2::MovieRoot::CreateArray(GFx::Value* pvalue)
{
    Environment* penv = ToAvmSprite(GetLevel0Movie())->GetASEnvironment();
    Ptr<ArrayObject> parr = 
        *static_cast<ArrayObject*>(penv->OperatorNew(penv->GetBuiltin(ASBuiltin_Array)));
    Value asval(parr);
    ASValue2Value(penv, asval, pvalue);
}

} // namespace AS2


void AS2::MovieRoot::CreateFunction(GFx::Value* pvalue, GFx::FunctionHandler* pfc, 
                                    void* puserData /* = NULL */)
{
    Environment* penv = ToAvmSprite(GetLevel0Movie())->GetASEnvironment();
    ASStringContext* psc = penv->GetSC();
    AS2::Value asval;
    Ptr<UserDefinedFunctionObject> pfuncObj = *SF_HEAP_NEW(psc->GetHeap()) UserDefinedFunctionObject(psc, pfc, puserData);
    asval.SetAsFunction( FunctionRef(pfuncObj.GetPtr()) );
    ASValue2Value(penv, asval, pvalue);
}


// In case someone decides to combine sources
#undef GFX_Value_ObjectInterface_CLASS

}} // Scaleform::GFx
