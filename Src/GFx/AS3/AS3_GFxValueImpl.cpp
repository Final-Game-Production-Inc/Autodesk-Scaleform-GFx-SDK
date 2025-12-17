/**************************************************************************

Filename    :   AS3_GFxValueImpl.cpp
Content     :   Complex Objects API Implementation
Created     :   
Authors     :   Prasad Silva, Michael Antonov

Notes       :   Contains MovieImpl, Value::ObjectInterface and 
                Value method definitions.

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/GFx_PlayerImpl.h"
#include "GFx/GFx_TextField.h"
#include "Obj/AS3_Obj_Array.h"
#include "GFx/AS3/AS3_MovieRoot.h"
#include "AS3_Slot.h"
#include "AS3_MovieRoot.h"

#include "AS3_AvmSprite.h"
#include "AS3_AvmInteractiveObj.h"
#include "AS3_AvmTextField.h"
#include "AS3_AvmDisplayObjContainer.h"
#include "Obj/Display/AS3_Obj_Display_DisplayObjectContainer.h"
#include "Obj/AS3_Obj_Function.h"
#include "Obj/Utils/AS3_Obj_Utils_ByteArray.h"
#include "Kernel/SF_MsgFormat.h"

//#include "Render/Render_Renderer.h"


namespace Scaleform { namespace GFx {

#define GFX_VALUE_OFFSETOF(st, m) ((size_t) ( (char *)&((st *)(16))->m - (char *)16 ))

// ***** Custom AS2 function object

namespace AS3 {

#define CLOSURE_TYPE_BIT    0x2

// Custom FunctionObject that wraps the function context as well as
// the user data.
class UserDefinedFunction : public Instances::FunctionBase
{
protected:
    mutable Ptr<FunctionHandler>    pContext;
    void*                           pUserData; 

public:
    UserDefinedFunction(InstanceTraits::Traits& t, FunctionHandler* pcontext, void* puserData) 
        : FunctionBase(t)
    {
        pContext = pcontext;
        pUserData = puserData;
    }

    // *argv* does not include *this* ...
    // These Execute() methods should be const. It is reasonable.
    // Function call doesn't change function itself.
    virtual void ExecuteUnsafe(const AS3::Value& _this, AS3::Value& result, unsigned argc, const AS3::Value* argv)
    {
        ExecuteImpl(_this, result, argc, argv);
    }
    virtual void Execute(const AS3::Value& _this, unsigned argc, const AS3::Value* argv, bool discard_result = false)
    {
        AS3::Value result;
        ExecuteImpl(_this, result, argc, argv);
        if (!discard_result)
            PushResult(result);
    }

    virtual UInt32 GetMaxParamNum() const { return SF_AS3_VARARGNUM; }

private:
    void    ExecuteImpl(const AS3::Value& _this, AS3::Value& result, unsigned argc, const AS3::Value* argv) const;
};


void UserDefinedFunction::ExecuteImpl(const AS3::Value& _this, AS3::Value& result, 
                                      unsigned argc, const AS3::Value* argv) const
{
    ArrayCPP<GFx::Value> args;
    GFx::Value thisVal, retVal;

    ASVM& vm = (ASVM&)(GetVM());
    MovieRoot* pmovieRoot = vm.GetMovieRoot();
    SF_ASSERT(pmovieRoot);

    pmovieRoot->ASValue2GFxValue(_this, &thisVal);
    args.PushBack(thisVal); // Used to pack 'this' on callback

    for (unsigned i = 0; i < argc; i++)
    {
        GFx::Value arg;
        pmovieRoot->ASValue2GFxValue(argv[i], &arg);
        args.PushBack(arg);
    }

    FunctionHandler::Params params;
    params.pMovie               = vm.GetMovieImpl();
    params.pRetVal              = &retVal;
    params.pThis                = &thisVal;
    params.pArgs                = argc > 0 ? &args[1] : NULL;
    params.pArgsWithThisRef     = &args[0];
    params.ArgCount             = (unsigned)args.GetSize() - 1;
    params.pUserData            = pUserData;
    pContext->Call(params);

    if (!retVal.IsUndefined()) 
        pmovieRoot->GFxValue2ASValue(retVal, &result);
}

} // namespace AS3



// If both VMs are supported, Value::Object interface is implemented
// through virtual functions; if not, it is statically linked.

#if defined(GFX_AS2_SUPPORT) && defined(GFX_AS3_SUPPORT)

class AS3ValueObjectInterface : public MovieImpl::ValueObjectInterface
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
    AS3ValueObjectInterface(MovieImpl* pmovieRoot)
        : MovieImpl::ValueObjectInterface(pmovieRoot) { }

};
#define GFX_Value_ObjectInterface_CLASS AS3ValueObjectInterface

#else
#define GFX_Value_ObjectInterface_CLASS GFx::Value::ObjectInterface
#endif


#ifdef GFX_AS3_SUPPORT

// ***** GFxValue::ObjectInterface or AS3ValueObjectInterface definitions


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
            size_t memberOffset = GFX_VALUE_OFFSETOF(ASStringNode, pData);
            ASStringNode* ps = (ASStringNode*)(((UByte*)pobj) - memberOffset);
            ps->AddRef();
        }
        break;
    case Value::VT_StringW:
        {
            // pobj points to const wchar_t*
            size_t memberOffset = GFX_VALUE_OFFSETOF(MovieImpl::WideStringStorage, pData);
            MovieImpl::WideStringStorage* pws = (MovieImpl::WideStringStorage*)(((UByte*)pobj) - memberOffset);
            pws->AddRef();
        }
        break;
    case Value::VT_DisplayObject:
    case Value::VT_Object:
    case Value::VT_Array:
        ((AS3::Object*)pobj)->AddRef_Unsafe();
        break;

#ifdef GFX_AS3_SUPPORT
    case Value::VT_Closure:
        ((AS3::Object*)((UPInt)pobj & ~CLOSURE_TYPE_BIT))->AddRef_Unsafe();
        break;
#endif
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
            size_t memberOffset = GFX_VALUE_OFFSETOF(ASStringNode, pData);
            ASStringNode* ps = (ASStringNode*)(((UByte*)pobj) - memberOffset);
            ps->Release();
        }
        break;
    case Value::VT_StringW:
        {
            // pobj points to const wchar_t*
            size_t memberOffset = GFX_VALUE_OFFSETOF(MovieImpl::WideStringStorage, pData);
            MovieImpl::WideStringStorage* pws = 
                (MovieImpl::WideStringStorage*)(((UByte*)pobj) - memberOffset);
            pws->Release();
        }
        break;
    case Value::VT_DisplayObject:
    case Value::VT_Object:
    case Value::VT_Array:    
        ((AS3::Object*)pobj)->Release_Unsafe();
        break;

    case Value::VT_Closure:
        ((AS3::Object*)((UPInt)pobj & ~CLOSURE_TYPE_BIT))->Release_Unsafe();
        break;

    default:
        SF_ASSERT(0);
    }

#if defined(SF_BUILD_DEBUG) || defined(GFX_AS_ENABLE_GFXVALUE_CLEANUP)
    ExternalObjRefs.Remove(val);
#endif
}


bool GFX_Value_ObjectInterface_CLASS::HasMember(void* pdata, const char* name,
                                                bool isdobj) const 
{        
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::HasMember", Amp_Native_Function_Id_ObjectInterface_HasMember);
    SF_UNUSED(isdobj);
    AS3::MovieRoot* root = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot.GetPtr());
    AS3::VM*        vm   = root->GetAVM();
    AS3::Object*    obj  = (AS3::Object*)pdata;
    
    AS3::Multiname mn(root->GetAVM()->GetPublicNamespace(),
                      root->GetStringManager()->CreateString(name));
    AS3::PropRef   prop;

    obj->FindProperty(prop, mn);
    SF_ASSERT(!vm->IsException());
    
    if (!prop)
    {
        // Check if object is a display object container
        if (AreDisplayObjectContainerTraits(obj))
        {
            AS3::AvmDisplayObjContainer* pcurr = 
                AS3::ToAvmDisplayObjContainer(static_cast<AS3::Instances::fl_display::DisplayObjectContainer*>(
                obj)->pDispObj->CharToDisplayObjContainer());
            ASString propName = root->GetStringManager()->CreateString(name);
            AS3::Instances::fl_display::DisplayObject* pchild = pcurr->GetAS3ChildByName(propName);
            if (pchild)
            {
                SF_ASSERT(pchild->pDispObj);
                return true;
            }
            else
            {
                if (vm->IsException()) vm->IgnoreException();
                return false;
            }
        }
    }

    return (bool)prop;
}


bool GFX_Value_ObjectInterface_CLASS::GetMember(void* pdata, const char* name,
                                                Value* pval, bool isdobj) const
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::GetMember", Amp_Native_Function_Id_ObjectInterface_GetMember);
    SF_UNUSED(isdobj);
    AS3::MovieRoot* root = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot.GetPtr());
    AS3::VM*        vm   = root->GetAVM();
    AS3::Object*    obj  = (AS3::Object*)pdata;

    AS3::Multiname mn(vm->GetPublicNamespace(),
        root->GetStringManager()->CreateString(name));

    AS3::Value asval;
    if (!obj->GetProperty(mn, asval))
    {
        // Check if object is a display object container
        if (AreDisplayObjectContainerTraits(obj))
        {
            AS3::AvmDisplayObjContainer* pcurr = 
                AS3::ToAvmDisplayObjContainer(static_cast<AS3::Instances::fl_display::DisplayObjectContainer*>(
                obj)->pDispObj->CharToDisplayObjContainer());
            ASString propName = root->GetStringManager()->CreateString(name);
            AS3::Instances::fl_display::DisplayObject* pchild = pcurr->GetAS3ChildByName(propName);
            if (pchild)
            {
                SF_ASSERT(pchild->pDispObj);
                asval = AS3::Value(pchild);
                root->ASValue2GFxValue(asval, pval);
                return true;
            }
            else
            {
                if (vm->IsException()) vm->IgnoreException();
                pval->SetUndefined();
                return false;
            }
        }

        if (vm->IsException())
            vm->OutputAndIgnoreException();
        pval->SetUndefined();
        return false;
    }

    root->ASValue2GFxValue(asval, pval);

    if (vm->IsException())
    {
        vm->OutputAndIgnoreException();
        return false;
    }

    return true;
}


bool GFX_Value_ObjectInterface_CLASS::SetMember(void* pdata, const char* name,
                                                const Value& value, bool isdobj)
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::SetMember", Amp_Native_Function_Id_ObjectInterface_SetMember);
    SF_UNUSED(isdobj);
    AS3::MovieRoot* root = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot.GetPtr());
    AS3::VM*        vm   = root->GetAVM();
    AS3::Object*    obj  = (AS3::Object*)pdata;

    AS3::Multiname  mn(vm->GetPublicNamespace(),
                       root->GetStringManager()->CreateString(name));

    // Check if display list contains a child with same name, and bail if that is the case
    // This is done for DAPI consistency
    if (AreDisplayObjectContainerTraits(obj))
    {
        AS3::AvmDisplayObjContainer* pcurr = 
            AS3::ToAvmDisplayObjContainer(static_cast<AS3::Instances::fl_display::DisplayObjectContainer*>(
            obj)->pDispObj->CharToDisplayObjContainer());
        ASString propName = root->GetStringManager()->CreateString(name);
        AS3::Instances::fl_display::DisplayObject* pchild = pcurr->GetAS3ChildByName(propName);
        if (pchild)
        {
            SF_ASSERT(pchild->pDispObj);
            String errMsg;
            Format(errMsg, "Property '{0}' already exists as a DisplayObject. SetMember aborted.", 
                name);
            root->Output(AS3::FlashUI::Output_Error, errMsg);
            return false;
        }
    }

    AS3::Value asval;
    root->GFxValue2ASValue(value, &asval);
    if (!obj->SetProperty(mn, asval))
    {
        if (vm->IsException())
            vm->OutputAndIgnoreException();
        return false;
    }

    SF_ASSERT(!vm->IsException());
    return true;
}


bool GFX_Value_ObjectInterface_CLASS::Invoke(void* pdata, GFx::Value* presult, const char* name,
                                             const GFx::Value* pargs, UPInt nargs, bool isdobj)
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::Invoke", Amp_Native_Function_Id_ObjectInterface_Invoke);
    SF_AMP_SCOPE_TIMER(GetAdvanceStats(), name, Amp_Profile_Level_ActionScript);
    SF_UNUSED(isdobj);
    AS3::MovieRoot* root = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot.GetPtr());
    AS3::VM*        vm   = root->GetAVM();

    AS3::Multiname  mn(vm->GetPublicNamespace(),
                       root->GetStringManager()->CreateString(name));
    AS3::PropRef    prop;

    ((AS3::Object*)pdata)->FindProperty(prop, mn);
    
    if (!prop)
        return false;
   
    AS3::Value asfn;
    AS3::Value asresult;

    if (!prop.GetSlotValueUnsafe(*vm, asfn))
    {
        vm->OutputAndIgnoreException();
        return false;
    }
    
    if (nargs)
    {
        Array<AS3::Value> args(static_cast<int>(nargs));
        for (UPInt i = 0; i< nargs; i++)
            root->GFxValue2ASValue(pargs[i], &args[i]);
        vm->ExecuteUnsafe(asfn, AS3::Value((AS3::Object*)pdata), asresult, static_cast<unsigned>(nargs), &args[0]);
    }
    else
    {
        vm->ExecuteUnsafe(asfn, AS3::Value((AS3::Object*)pdata), asresult, 0, 0);
    }
       
    if (vm->IsException())
    {
        vm->OutputAndIgnoreException();
        return false;
    }

    if (presult)
        root->ASValue2GFxValue(asresult, presult);

	if (vm->IsException())
	{
		vm->OutputAndIgnoreException();
		return false;
	}

	return true;
}


bool GFX_Value_ObjectInterface_CLASS::InvokeClosure(void* pdata, UPInt dataAux, Value* presult, 
                                                    const Value* pargs, UPInt nargs)
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::InvokeClosure", Amp_Native_Function_Id_ObjectInterface_InvokeClosure);
    AS3::MovieRoot* root = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot.GetPtr());
    AS3::VM*        vm   = root->GetAVM();

    AS3::Value asfn;
    AS3::Value asresult;
    
    // Get closure reference
    int closureType = ((UPInt)pdata & CLOSURE_TYPE_BIT) ? 1 : 0;
    AS3::Object* pobj = (AS3::Object*)((UPInt)pdata & ~CLOSURE_TYPE_BIT);
    if (closureType)
        // VTableIndClosure
        asfn = AS3::Value(pobj, SInt32(dataAux), false);
    else
        // ThunkClosure
        asfn = AS3::Value(pobj, *(const AS3::ThunkInfo*)dataAux);

    if (nargs)
    {
        Array<AS3::Value> args(static_cast<int>(nargs));
        for (UPInt i = 0; i< nargs; i++)
            root->GFxValue2ASValue(pargs[i], &args[i]);
        vm->ExecuteUnsafe(asfn, AS3::Value(pobj), asresult, static_cast<unsigned>(nargs), &args[0]);
    }
    else
    {
        vm->ExecuteUnsafe(asfn, AS3::Value(pobj), asresult, 0, 0);
    }

    if (vm->IsException())
    {
        vm->OutputAndIgnoreException();
        return false;
    }

    if (presult)
        root->ASValue2GFxValue(asresult, presult);
    SF_ASSERT(!vm->IsException());
    return true;
}

bool GFX_Value_ObjectInterface_CLASS::DeleteMember(void* pdata, const char* name, bool isdobj)
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::DeleteMember", Amp_Native_Function_Id_ObjectInterface_DeleteMember);
    SF_UNUSED(isdobj);
    AS3::MovieRoot* root = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot.GetPtr());
    AS3::VM*        vm   = root->GetAVM();

    AS3::Multiname  mn(vm->GetPublicNamespace(),
                       root->GetStringManager()->CreateString(name));
    
    SF_ASSERT(!vm->IsException());
    return ((AS3::Object*)pdata)->DeleteProperty(mn);
}


void GFX_Value_ObjectInterface_CLASS::VisitMembers(void* pdata, Value::ObjectVisitor* visitor,
                                                   bool isdobj) const
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::VisitMembers", Amp_Native_Function_Id_ObjectInterface_VisitMembers);
    SF_UNUSED(isdobj);
    AS3::MovieRoot* root = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot.GetPtr());        

    AS3::Object*    obj  = (AS3::Object*)pdata;

    // Iterate through all dynamic slots
    const AS3::Object::DynAttrsType* dynAttrs = obj->GetDynamicAttrs();
    if (dynAttrs)
    {
        AS3::Object::DynAttrsType::ConstIterator it = dynAttrs->Begin();
        for (; !it.IsEnd(); ++it)
        {
            GFx::Value val;
            // Include "DontEnum" properties.
            const ASString& name = it->First.GetName();
            const AS3::Value& asval = it->Second;

            root->ASValue2GFxValue(asval, &val);
            visitor->Visit(name.ToCStr(), val);
        }
    }
    
    // !PPS Iterating through fixed slots is not comparable to an AS3 iteration, as the AS3
    // version only looks at dyn slots. The describeType() method - that dumps XML - is used
    // to reflect on the type/fixed traits.
    
    // !PPS However, retrieving public members may be useful in some case (UE3 integration use of
    // Event types. We use Value::ObjectVisitor::IncludeAS3PublicMembers() to look through
    // static slots with public scope (members only).
    if (visitor->IncludeAS3PublicMembers())
    {
        // TODO
        const AS3::Traits &t = obj->GetTraits();
        AS3::Traits::CIterator it = t.Begin();
        for (; !it.IsEnd(); ++it)
        {
            // PPS: We skip inherited properties.. or not. They may be useful
            //if (it.GetCurrentSlotIndex() < t.GetFirstOwnSlotIndex())
            //    continue;

            GFx::Value val;
            ASString name(it.GetSlotName());
            const AS3::SlotInfo& slotInfo = it.GetSlotInfo();

            // PPS: We skip methods and setters
            if (slotInfo.IsMethod() || slotInfo.IsSetter())
                continue;

            // PPS: We skip non-public members
            if (slotInfo.GetNamespace().GetKind() != AS3::Abc::NS_Public)
                continue;

            AS3::Value asval;
            slotInfo.GetSlotValueUnsafe(asval, obj).DoNotCheck();

            root->ASValue2GFxValue(asval, &val);
            visitor->Visit(name.ToCStr(), val);
        }
    }

    // Iterate through all child movieclips if obj is a DisplayObjectContainer
    if (AreDisplayObjectContainerTraits(obj))
    {
        AS3::AvmDisplayObjContainer* pcurr = 
            AS3::ToAvmDisplayObjContainer(static_cast<AS3::Instances::fl_display::DisplayObjectContainer*>(
            obj)->pDispObj->CharToDisplayObjContainer());
        for (unsigned i = 0; i < pcurr->GetNumChildren(); i++) 
        {
            AS3::Instances::fl_display::DisplayObject* pchild = pcurr->GetAS3ChildAt(i);
            SF_ASSERT(pchild->pDispObj);            
            // PPS: This check seems unnecessary
            //if (!pchild->pDispObj->IsTimelineObjectFlagSet())
            {
                ASString name = pchild->pDispObj->GetName();
                AS3::Value asval(pchild);
                GFx::Value val;
                root->ASValue2GFxValue(asval, &val);
                visitor->Visit(name.ToCStr(), val);
            }
        }
    }

    SF_ASSERT(!root->GetAVM()->IsException());
}

bool GFX_Value_ObjectInterface_CLASS::IsInstanceOf(void* pdata, const char* className) const
{
	SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::IsInstanceOf", Amp_Native_Function_Id_ObjectInterface_IsInstanceOf);

	AS3::MovieRoot* root = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot.GetPtr());        
    AS3::Object*    obj  = (AS3::Object*)pdata;
    AS3::VM*        vm   = root->GetAVM();

    const AS3::Multiname mn(*vm, className);
    const AS3::ClassTraits::Traits* ctr = vm->Resolve2ClassTraits(mn, vm->GetCurrentAppDomain());
    if (!ctr || !obj->GetTraits().GetClass().GetClassTraits().IsOfType(*ctr))
	{
		SF_DEBUG_MESSAGE1(1, "IsInstanceOf returned false. %p\n", ctr);
        return false;
	}

    return true;
}

bool GFX_Value_ObjectInterface_CLASS::IsByteArray(void* pdata) const
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::IsByteArray", Amp_Native_Function_Id_ObjectInterface_IsByteArray);

    AS3::MovieRoot* root = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot.GetPtr());        
    AS3::Object*    obj  = (AS3::Object*)pdata;
    AS3::VM*        vm   = root->GetAVM();

    const AS3::Multiname mn(*vm, "flash.utils.ByteArray");
    const AS3::ClassTraits::Traits* ctr = vm->Resolve2ClassTraits(mn, vm->GetCurrentAppDomain());
    if (!ctr || !obj->GetTraits().GetClass().GetClassTraits().IsOfType(*ctr))
	{
		SF_DEBUG_MESSAGE1(1, "IsByteArray returned false. %p\n", ctr);
        return false;
	}

    //AS3::Instances::fl_utils::ByteArray *arr = (AS3::Instances::fl_utils::ByteArray*) (AS3::Object*) pdata;
    return true;
}

unsigned GFX_Value_ObjectInterface_CLASS::GetByteArraySize(void* pdata) const
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::GetByteArraySize", Amp_Native_Function_Id_ObjectInterface_GetByteArraySize);

    AS3::MovieRoot* root = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot.GetPtr());        
    AS3::Object*    obj  = (AS3::Object*)pdata;
    AS3::VM*        vm   = root->GetAVM();

    const AS3::Multiname mn(*vm, "flash.utils.ByteArray");
    const AS3::ClassTraits::Traits* ctr = vm->Resolve2ClassTraits(mn, vm->GetCurrentAppDomain());
    if (!ctr || !obj->GetTraits().GetClass().GetClassTraits().IsOfType(*ctr)) 
        return 0;

    AS3::Instances::fl_utils::ByteArray *arr = (AS3::Instances::fl_utils::ByteArray*) (AS3::Object*) pdata;
    return arr->GetLength();
}

void GFX_Value_ObjectInterface_CLASS::SetByteArraySize(void* pdata, UPInt setSize)
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::SetByteArraySize", Amp_Native_Function_Id_ObjectInterface_SetByteArraySize);

    AS3::MovieRoot* root = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot.GetPtr());        
    AS3::Object*    obj  = (AS3::Object*)pdata;
    AS3::VM*        vm   = root->GetAVM();

    const AS3::Multiname mn(*vm, "flash.utils.ByteArray");
    const AS3::ClassTraits::Traits* ctr = vm->Resolve2ClassTraits(mn, vm->GetCurrentAppDomain());
    if (!ctr || !obj->GetTraits().GetClass().GetClassTraits().IsOfType(*ctr)) 
        return;

    AS3::Instances::fl_utils::ByteArray *arr = (AS3::Instances::fl_utils::ByteArray*) (AS3::Object*) pdata;
    arr->lengthSet(static_cast<Scaleform::UInt32>(setSize));
}

bool GFX_Value_ObjectInterface_CLASS::ReadFromByteArray(void* pdata, UByte *destBuff, UPInt destBuffSz) const
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::ReadFromByteArray", Amp_Native_Function_Id_ObjectInterface_ReadFromByteArray);

    AS3::MovieRoot* root = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot.GetPtr());        
    AS3::Object*    obj  = (AS3::Object*)pdata;
    AS3::VM*        vm   = root->GetAVM();

    const AS3::Multiname mn(*vm, "flash.utils.ByteArray");
    const AS3::ClassTraits::Traits* ctr = vm->Resolve2ClassTraits(mn, vm->GetCurrentAppDomain());
    if (!ctr || !obj->GetTraits().GetClass().GetClassTraits().IsOfType(*ctr)) 
        return false;

    AS3::Instances::fl_utils::ByteArray *arr = (AS3::Instances::fl_utils::ByteArray*) (AS3::Object*) pdata;
    arr->Get(destBuff, destBuffSz);
    return true;
}

bool GFX_Value_ObjectInterface_CLASS::WriteToByteArray(void* pdata, const UByte *srcBuff, UPInt writeSize)
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::WriteToByteArray", Amp_Native_Function_Id_ObjectInterface_WriteToByteArray);

    AS3::MovieRoot* root = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot.GetPtr());        
    AS3::Object*    obj  = (AS3::Object*)pdata;
    AS3::VM*        vm   = root->GetAVM();

    const AS3::Multiname mn(*vm, "flash.utils.ByteArray");
    const AS3::ClassTraits::Traits* ctr = vm->Resolve2ClassTraits(mn, vm->GetCurrentAppDomain());
    if (!ctr || !obj->GetTraits().GetClass().GetClassTraits().IsOfType(*ctr)) 
        return false;

    AS3::Instances::fl_utils::ByteArray *arr = (AS3::Instances::fl_utils::ByteArray*) (AS3::Object*) pdata;
    arr->Set(srcBuff, writeSize);
    return true;
}

void* GFX_Value_ObjectInterface_CLASS::GetRawDataPtr(void* pdata)
{
   SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::GetRawDataPtr", Amp_Native_Function_Id_ObjectInterface_GetRawDataPtr);

    AS3::MovieRoot* root = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot.GetPtr());        
    AS3::Object*    obj  = (AS3::Object*)pdata;
    AS3::VM*        vm   = root->GetAVM();

    const AS3::Multiname mn(*vm, "flash.utils.ByteArray");
    const AS3::ClassTraits::Traits* ctr = vm->Resolve2ClassTraits(mn, vm->GetCurrentAppDomain());
    if (!ctr || !obj->GetTraits().GetClass().GetClassTraits().IsOfType(*ctr)) 
        return NULL;

    AS3::Instances::fl_utils::ByteArray *arr = (AS3::Instances::fl_utils::ByteArray*) (AS3::Object*) pdata;
    return arr->GetDataPtr();
}

unsigned GFX_Value_ObjectInterface_CLASS::GetArraySize(void* pdata) const
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::GetArraySize", Amp_Native_Function_Id_ObjectInterface_GetArraySize);
    AS3::Instances::fl::Array* arr = (AS3::Instances::fl::Array*) (AS3::Object*) pdata;
    return (unsigned)arr->GetSize();
}


bool GFX_Value_ObjectInterface_CLASS::SetArraySize(void* pdata, unsigned sz)
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::SetArraySize", Amp_Native_Function_Id_ObjectInterface_SetArraySize);
    AS3::Instances::fl::Array* arr = (AS3::Instances::fl::Array*) (AS3::Object*) pdata;
    arr->Resize(sz);
    return true;
}


bool GFX_Value_ObjectInterface_CLASS::GetElement(void* pdata, unsigned idx, Value *pval) const
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::GetElement", Amp_Native_Function_Id_ObjectInterface_GetElement);
    AS3::Instances::fl::Array* arr = (AS3::Instances::fl::Array*) (AS3::Object*) pdata;
        
    pval->SetUndefined();
    if (idx >= (unsigned)arr->GetSize())
        return false;

    AS3::MovieRoot*   root = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot.GetPtr());
    const AS3::Value& asval = arr->At(idx);
    root->ASValue2GFxValue(asval, pval);
    return true;
}


bool GFX_Value_ObjectInterface_CLASS::SetElement(void* pdata, unsigned idx, const Value& value)
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::SetElement", Amp_Native_Function_Id_ObjectInterface_SetElement);
    AS3::Instances::fl::Array* arr  = (AS3::Instances::fl::Array*) (AS3::Object*) pdata;
    AS3::MovieRoot*        root = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot.GetPtr());
    
    AS3::Value asval;
    root->GFxValue2ASValue(value, &asval);
    arr->Set(idx, asval);
    return true;
}


void GFX_Value_ObjectInterface_CLASS::VisitElements(void* pdata, Value::ArrayVisitor* visitor,
                                                    unsigned idx, int count) const
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::VisitElements", Amp_Native_Function_Id_ObjectInterface_VisitElements);
    AS3::Instances::fl::Array* arr  = (AS3::Instances::fl::Array*) (AS3::Object*) pdata;
    AS3::MovieRoot*        root = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot.GetPtr());
    
    unsigned sz = (unsigned)arr->GetSize();
    if (idx >= sz)
        return;
    if (count < 0)
        count = (sz - idx);

    GFx::Value val;
    unsigned   eidx = Alg::Min(sz, idx + count);

    for (unsigned i=idx; i < eidx; i++)
    {
        const AS3::Value& asval = arr->At(i);
        root->ASValue2GFxValue(asval, &val);
        visitor->Visit(i, val);
    }
}


bool GFX_Value_ObjectInterface_CLASS::PushBack(void *pdata, const Value &value)
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::PushBack", Amp_Native_Function_Id_ObjectInterface_PushBack);
    AS3::Instances::fl::Array* arr  = (AS3::Instances::fl::Array*) (AS3::Object*) pdata;
    AS3::MovieRoot*        root = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot.GetPtr());

    AS3::Value asval;
    root->GFxValue2ASValue(value, &asval);
    arr->PushBack(asval);
    return true;
}


bool GFX_Value_ObjectInterface_CLASS::PopBack(void* pdata, Value* pval)
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::PopBack", Amp_Native_Function_Id_ObjectInterface_PopBack);
    AS3::Instances::fl::Array* arr  = (AS3::Instances::fl::Array*) (AS3::Object*) pdata;
    AS3::MovieRoot*        root = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot.GetPtr());

    int sz = static_cast<int>(arr->GetSize());
    if (sz <= 0) 
    {
        if (pval) pval->SetUndefined();
        return false;
    }
    if (pval)
    {
        const AS3::Value& asval = arr->At(sz-1);
        root->ASValue2GFxValue(asval, pval);
    }
    arr->Resize(sz-1);
    return true;
}


bool GFX_Value_ObjectInterface_CLASS::RemoveElements(void *pdata, unsigned idx, int count)
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::RemoveElements", Amp_Native_Function_Id_ObjectInterface_RemoveElements);
    AS3::Instances::fl::Array* arr  = (AS3::Instances::fl::Array*) (AS3::Object*) pdata; 

    unsigned sz = (unsigned)arr->GetSize();
    if (idx >= sz) return false;
    if (count < 0) { count = (sz - idx); }

    arr->CutMultipleAt(idx, Alg::Min(sz - idx, (unsigned)count));
    return true;
}

bool GFX_Value_ObjectInterface_CLASS::GetWorldMatrix(void* pdata, Render::Matrix2F* pmat) const
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::GetWorldMatrix", Amp_Native_Function_Id_ObjectInterface_GetWorldMatrix);
    AS3::Object*    obj  = (AS3::Object*)pdata;
    if (!AreDisplayObjectTraits(obj)) return false;
    DisplayObject* pd = static_cast<AS3::Instances::fl_display::DisplayObject*>(obj)->pDispObj;
    SF_ASSERT(pd);

    Render::Matrix2F m = pd->GetWorldMatrix();
    m.Tx() = TwipsToPixels(m.Tx());
    m.Ty() = TwipsToPixels(m.Ty());
    *pmat = m;

    return true;
}

bool GFX_Value_ObjectInterface_CLASS::GetDisplayMatrix(void* pdata, Render::Matrix2F* pmat) const
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::GetDisplayMatrix", Amp_Native_Function_Id_ObjectInterface_GetDisplayMatrix);
    AS3::Object*    obj  = (AS3::Object*)pdata;
    if (!AreDisplayObjectTraits(obj)) return false;
    DisplayObject* pd = static_cast<AS3::Instances::fl_display::DisplayObject*>(obj)->pDispObj;
    SF_ASSERT(pd);

    Render::Matrix2F m = pd->GetMatrix();
    m.Tx() = TwipsToPixels(m.Tx());
    m.Ty() = TwipsToPixels(m.Ty());
    *pmat = m;

    return true;
}

bool GFX_Value_ObjectInterface_CLASS::SetDisplayMatrix(void* pdata, const Render::Matrix2F& mat)
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::SetDisplayMatrix", Amp_Native_Function_Id_ObjectInterface_SetDisplayMatrix);
    AS3::Object*    obj  = (AS3::Object*)pdata;
    if (!AreDisplayObjectTraits(obj)) return false;
    DisplayObject* pd = static_cast<AS3::Instances::fl_display::DisplayObject*>(obj)->pDispObj;
    SF_ASSERT(pd);

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
    AS3::Object*    obj  = (AS3::Object*)pdata;
    if (!AreDisplayObjectTraits(obj)) return false;
    DisplayObject* pd = static_cast<AS3::Instances::fl_display::DisplayObject*>(obj)->pDispObj; 
    SF_ASSERT(pd);

    Render::Matrix3F m = pd->GetMatrix3D();
    m.Tx() = TwipsToPixels(m.Tx());
    m.Ty() = TwipsToPixels(m.Ty());
    m.Tz() = TwipsToPixels(m.Tz());
    *pmat = m;
    return true;
}

bool GFX_Value_ObjectInterface_CLASS::SetMatrix3D(void* pdata, const Render::Matrix3F& mat)
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::SetMatrix3D", Amp_Native_Function_Id_ObjectInterface_SetMatrix3D);
    AS3::Object*    obj  = (AS3::Object*)pdata;
    if (!AreDisplayObjectTraits(obj)) return false;
    DisplayObject* pd = static_cast<AS3::Instances::fl_display::DisplayObject*>(obj)->pDispObj;
    SF_ASSERT(pd);

    // Extra checks
    if (!mat.IsValid())
    {
        SF_DEBUG_WARNING(1, "GFxValue::SetDisplayMatrix called with an invalid matrix.\n"
            "At least one of the matrix values is invalid (non-finite).\n");
        return false;
    }

    Render::Matrix3F m = mat;
    m.Tx() = PixelsToTwips(m.Tx());
    m.Ty() = PixelsToTwips(m.Ty());
    m.Tz() = PixelsToTwips(m.Tz());
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
    AS3::Object*    obj = (AS3::Object*)pdata;
    if (!AreDisplayObjectTraits(obj)) return false;
    DisplayObject* pd = static_cast<AS3::Instances::fl_display::DisplayObject*>(obj)->pDispObj;    
    return pd != NULL;
}

bool    GFX_Value_ObjectInterface_CLASS::GetParent(void* pdata, Value* pparent) const
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::GetParent", Amp_Native_Function_Id_ObjectInterface_GetParent);

    AS3::Object* obj = (AS3::Object*)pdata;
    if (!AreDisplayObjectTraits(obj)) return false;
    DisplayObject* pd = static_cast<AS3::Instances::fl_display::DisplayObject*>(obj)->pDispObj;    
    if(pd == NULL) return false;

    DisplayObject* pdp = pd->GetParent();
    if(pdp != NULL)
    {
        AS3::MovieRoot* proot = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot.GetPtr());
        AS3::Object*    pas3obj = AS3::ToAvmDisplayObj(pdp)->GetAS3Obj();
        return proot->CreateObjectValue(pparent, proot->GetMovieImpl()->pObjectInterface, pas3obj, true);
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
    AS3::Object*    obj  = (AS3::Object*)pdata;
    if (!AreDisplayObjectTraits(obj)) return false;
    DisplayObject* pd = static_cast<AS3::Instances::fl_display::DisplayObject*>(obj)->pDispObj;
    SF_ASSERT(pd);
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
        Double z = TwipsToPixels(geomData.Z);
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
    AS3::Object*    obj  = (AS3::Object*)pdata;
    if (!AreDisplayObjectTraits(obj)) return false;
    DisplayObject* pd = static_cast<AS3::Instances::fl_display::DisplayObject*>(obj)->pDispObj;
    SF_ASSERT(pd);

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

			float alpha = static_cast<float>(cinfo.GetAlpha() / 100.);
			if( cx.M[0][3] != alpha )
			{
				cx.M[0][3] = alpha;
				pd->SetCxform(cx);
				pd->SetAcceptAnimMoves(0);
			}
        }
    }

    if (cinfo.IsFlagSet(DisplayInfo::V_visible))
    {
        pd->SetVisible(cinfo.GetVisible());
    }


    if (cinfo.IsFlagSet(DisplayInfo::V_z))
    {
        Double zval = PixelsToTwips(cinfo.GetZ());
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

		Double r = fmod((Double)rval, (Double)360.);
		if (r > 180)
			r -= 360;
		else if (r < -180)
			r += 360;
		if (r != tfgeomData->XRotation)
		{       
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

		Double r = fmod((Double)rval, (Double)360.);
		if (r > 180)
			r -= 360;
		else if (r < -180)
			r += 360;
		if (r != tfgeomData->YRotation)
		{      
            tfgeomData->YRotation= r;
            Update3DTransforms = true;
        }
    }

    if (Update3DTransforms)
        pd->UpdateTransform3D();

    if (cinfo.IsFlagSet(DisplayInfo::V_FOV))
    {
        Double rval = cinfo.GetFOV();
		Double r = fmod((Double)rval, (Double)180.);
		if (r != pd->GetFOV())
		{       
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

		bool updateMatrix = false;

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
				if (tfgeomData->Rotation != newRotation)
				{
					tfgeomData->Rotation = newRotation;
					updateMatrix = true;
				}
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
				updateMatrix = true;
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
				updateMatrix = true;
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
			float x = (float) tfgeomData->X;
			if (m.Tx() != x)
			{
				m.Tx() = x;
				updateMatrix = true;
			}
        }
        // _y
        Double yval = cinfo.IsFlagSet(DisplayInfo::V_y) ? Double(pt.y) : NumberUtil::NaN();
        if (!NumberUtil::IsNaN(yval))
        {
            if (NumberUtil::IsNEGATIVE_INFINITY(yval) || NumberUtil::IsPOSITIVE_INFINITY(yval))
                yval = 0;
            tfgeomData->Y = int(floor(PixelsToTwips(yval)));
			float y = (float) tfgeomData->Y;
			if (m.Ty() != y)
			{
				m.Ty() = y;
				updateMatrix = true;
			}
        }

        if (updateMatrix && m.IsValid())
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
    AS3::Object*    obj  = (AS3::Object*)pdata;
    if (!AreDisplayObjectTraits(obj)) return false;
    DisplayObject* pd = static_cast<AS3::Instances::fl_display::DisplayObject*>(obj)->pDispObj;
    SF_ASSERT(pd);

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
    AS3::Object*    obj  = (AS3::Object*)pdata;
    if (!AreDisplayObjectTraits(obj)) return false;
    DisplayObject* pd = static_cast<AS3::Instances::fl_display::DisplayObject*>(obj)->pDispObj;
    SF_ASSERT(pd);

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
    AS3::MovieRoot*     root = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot.GetPtr());
    AS3::Object*        obj  = (AS3::Object*)pdata;
    if (!AreDisplayObjectTraits(obj)) return false;
    DisplayObject* pd = static_cast<AS3::Instances::fl_display::DisplayObject*>(obj)->pDispObj;
    SF_ASSERT(pd);

    if (pd->GetType() != CharacterDef::TextField)
        return GetMember(pdata, reqHtml ? "htmlText" : "text", pval, true);
    else
    {
        TextField* ptf = static_cast<TextField*>(pd);
        ASString text = (reqHtml) ? ptf->GetHtml() : ptf->GetText(false);
        root->ASValue2GFxValue(text, pval);
        return true;
    }
}

bool GFX_Value_ObjectInterface_CLASS::GotoAndPlay(void *pdata, const char *frame, bool stop)
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::GotoAndPlay", Amp_Native_Function_Id_ObjectInterface_GotoAndPlay);
    AS3::MovieRoot*     root = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot.GetPtr());
    AS3::Object*    obj  = (AS3::Object*)pdata;
    if (!AreDisplayObjectTraits(obj)) return false;
    DisplayObject* pd = static_cast<AS3::Instances::fl_display::DisplayObject*>(obj)->pDispObj;
    SF_ASSERT(pd);

    if (!pd->IsSprite()) return false;

    unsigned frameNum;
    Sprite* pspr = pd->CharToSprite_Unsafe();
    if (pspr->GetLabeledFrame(frame, &frameNum))
    {
        pspr->GotoFrame(frameNum);
        pspr->SetPlayState(stop ? State_Stopped : State_Playing);
        // execute all events right here
        root->QueueFrameActions();

        // execute highest/high/frame actions.
        // Do not execute enterframe and lower levels (render events and such)
        root->ExecuteActionQueue(AS3::MovieRoot::AL_Highest);
        root->ExecuteActionQueue(AS3::MovieRoot::AL_High);
        root->ExecuteActionQueue(AS3::MovieRoot::AL_Frame);
        return true;
    }
    return false;
}

bool GFX_Value_ObjectInterface_CLASS::GotoAndPlay(void *pdata, unsigned frame, bool stop)
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::GotoAndPlay", Amp_Native_Function_Id_ObjectInterface_GotoAndPlay);
    AS3::MovieRoot*     root = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot.GetPtr());
    AS3::Object*    obj  = (AS3::Object*)pdata;
    if (!AreDisplayObjectTraits(obj)) return false;
    DisplayObject* pd = static_cast<AS3::Instances::fl_display::DisplayObject*>(obj)->pDispObj;
    SF_ASSERT(pd);

    if (!pd->IsSprite()) return false;

    // Incoming frame number is 1-based, but internally GFx is 0 based.
    Sprite* pspr = pd->CharToSprite_Unsafe();
    pspr->GotoFrame(frame-1);
    pspr->SetPlayState(stop ? State_Stopped : State_Playing);
    // execute all events right here
    root->QueueFrameActions();

    // execute highest/high/frame actions.
    // Do not execute enterframe and lower levels (render events and such)
    root->ExecuteActionQueue(AS3::MovieRoot::AL_Highest);
    root->ExecuteActionQueue(AS3::MovieRoot::AL_High);
    root->ExecuteActionQueue(AS3::MovieRoot::AL_Frame);
    return true;
}

bool GFX_Value_ObjectInterface_CLASS::GetCxform(void *pdata, Render::Cxform *pcx) const
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::GetCxform", Amp_Native_Function_Id_ObjectInterface_GetCxform);
    AS3::Object*    obj  = (AS3::Object*)pdata;
    if (!AreDisplayObjectTraits(obj)) return false;
    DisplayObject* pd = static_cast<AS3::Instances::fl_display::DisplayObject*>(obj)->pDispObj;
    SF_ASSERT(pd);

    *pcx = pd->GetCxform();
    return true;
}

bool GFX_Value_ObjectInterface_CLASS::SetCxform(void *pdata, const Render::Cxform &cx)
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::SetCxform", Amp_Native_Function_Id_ObjectInterface_SetCxform);
    AS3::Object*    obj  = (AS3::Object*)pdata;
    if (!AreDisplayObjectTraits(obj)) return false;
    DisplayObject* pd = static_cast<AS3::Instances::fl_display::DisplayObject*>(obj)->pDispObj;
    SF_ASSERT(pd);

    pd->SetCxform(cx);
    pd->SetAcceptAnimMoves(false);
    return true;
}

static DisplayObjContainer* AS3ToGFxDisplayObjContainer(AS3::Object* po)
{
    return static_cast<DisplayObjContainer*>(
        static_cast<AS3::Instances::fl_display::DisplayObjectContainer*>(po)->pDispObj.GetPtr()
        );
}

bool GFX_Value_ObjectInterface_CLASS::CreateEmptyMovieClip(void* pdata, GFx::Value* pmc, 
                                                     const char* instanceName, SInt32 depth)
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::CreateEmptyMovieClip", Amp_Native_Function_Id_ObjectInterface_CreateEmptyMovieClip);
    AS3::MovieRoot*        root = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot.GetPtr());
    AS3::Object*    parentObj  = (AS3::Object*)pdata;
    if (!AreDisplayObjectContainerTraits(parentObj)) return false;
    DisplayObjContainer* parentDispObj = AS3ToGFxDisplayObjContainer(parentObj);
    SF_ASSERT(parentDispObj);

    AS3::Value asObj;
    AS3::ASVM* pAVM = root->GetAVM();
    SF_ASSERT(pAVM);

    if (!pAVM->CreateObject("flash.display.Sprite", pAVM->GetCurrentAppDomain(), asObj))
        return false;

    SF_ASSERT(asObj.IsObject());
    if (!AreDisplayObjectTraits(asObj.GetObject())) return false;
    AS3::Instances::fl_display::DisplayObject* pmcObj = static_cast<AS3::Instances::fl_display::DisplayObject*>(asObj.GetObject());
    AS3::Value undefVal;
    pmcObj->nameSet(undefVal, root->GetStringManager()->CreateString(instanceName));

    // Add new display obj to parent
    AS3::AvmDisplayObjContainer* container = AS3::ToAvmDisplayObjContainer(parentDispObj);
    unsigned index = (depth < 0) ? container->GetNumChildren() : depth;
    container->AddChildAt(pmcObj->pDispObj, index);

    root->ASValue2GFxValue(asObj, pmc);

    return true;
}

bool GFX_Value_ObjectInterface_CLASS::AttachMovie(void* pdata, GFx::Value* pmc, 
                                            const char* symbolName, const char* instanceName, 
                                            SInt32 depth, const MemberValueSet* initArgs)
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::AttachMovie", Amp_Native_Function_Id_ObjectInterface_AttachMovie);
    AS3::MovieRoot*        root = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot.GetPtr());
    AS3::Object*    parentObj  = (AS3::Object*)pdata;
    if (!AreDisplayObjectContainerTraits(parentObj)) return false;
    DisplayObjContainer* parentDispObj = AS3ToGFxDisplayObjContainer(parentObj);
    SF_ASSERT(parentDispObj);

    AS3::Value asObj;
    AS3::ASVM* pAVM = root->GetAVM();
    SF_ASSERT(pAVM);

    AS3::VMAppDomain& ad = AS3::ToAvmDisplayObjContainer(parentDispObj)->GetAppDomain();
    if (!pAVM->CreateObject(symbolName, ad, asObj))
    {
        LogState* log = root->GetLogState();
        if (log)
            log->LogScriptWarning("attachMovie() failed - export name \"%s\" is not found.", symbolName);
        pAVM->IgnoreException();

        return false;
    }

    // ** Simulate attachMovie
    SF_ASSERT(asObj.IsObject());
    if (!AreDisplayObjectTraits(asObj.GetObject())) return false;
    AS3::Instances::fl_display::DisplayObject* pmcObj = static_cast<AS3::Instances::fl_display::DisplayObject*>(asObj.GetObject());
    AS3::Value undefVal;
    pmcObj->nameSet(undefVal, root->GetStringManager()->CreateString(instanceName));
    
    // Load initArgs into obj
    if (initArgs)
    {
        for (unsigned i = 0; i < initArgs->GetSize(); i++)
        {
            const MemberValue& mv = (*initArgs)[i];
            AS3::Multiname propname(root->GetAVM()->GetPublicNamespace(),
                root->GetStringManager()->CreateString(mv.Key));
            AS3::Value propval;
            root->GFxValue2ASValue(mv.mValue, &propval);
            pmcObj->SetProperty(propname, propval).DoNotCheck();
        }
    }

    // Add new display obj to parent
    AS3::AvmDisplayObjContainer* container = AS3::ToAvmDisplayObjContainer(parentDispObj);
    if (depth > (int)container->GetNumChildren())
    {
        LogState* log = root->GetLogState();
        if (log)
            log->LogScriptWarning("DAPI AttachMovie() depth requested (%d) for symbol \"%s\" "
                                  "is too large. Using next highest index (%d) instead.", 
                                  depth, symbolName, container->GetNumChildren());
    }
    unsigned index = (depth < 0 || depth > (int)container->GetNumChildren()) ? container->GetNumChildren() : depth;
    container->AddChildAt(pmcObj->pDispObj, index);

    root->ASValue2GFxValue(asObj, pmc);

    return true;
}

void GFX_Value_ObjectInterface_CLASS::ToString(String* pstr, const GFx::Value& thisVal) const
{
    SF_AMP_SCOPE_TIMER_ID(GetAdvanceStats(), "ObjectInterface::ToString", Amp_Native_Function_Id_ObjectInterface_ToString);
    AS3::MovieRoot*       proot= static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot.GetPtr());

    AS3::Value asVal;
    proot->GFxValue2ASValue(thisVal, &asVal);

    AS3::StringManager* sm = proot->GetStringManager();
    ASString asStr = sm->CreateEmptyString();
    asVal.Convert2String(asStr).DoNotCheck();
    pstr->AssignString(asStr.ToCStr(), asStr.GetSize());
}


#ifdef GFX_AS_ENABLE_USERDATA

ASUserData*  GFX_Value_ObjectInterface_CLASS::GetUserData(void* pdata, bool isdobj) const 
{
    SF_UNUSED(isdobj);
    AS3::Object*    obj  = (AS3::Object*)pdata;

    if (obj) return obj->GetUserData();
    return NULL;
}

void    GFX_Value_ObjectInterface_CLASS::SetUserData(void* pdata, ASUserData* puserdata, bool isdobj) 
{
    SF_UNUSED(isdobj);
    AS3::Object*    obj  = (AS3::Object*)pdata;

    if (obj) return obj->SetUserData(GetMovieImpl(), puserdata, isdobj);
}

bool    GFX_Value_ObjectInterface_CLASS::CreateObjectValue(Value* pval, void* pdata, bool isdobj)
{
    AS3::MovieRoot* root = static_cast<AS3::MovieRoot*>(GetMovieImpl()->pASMovieRoot.GetPtr());

    return root->CreateObjectValue(pval, this, pdata, isdobj);
}

#endif  // GFX_AS_ENABLE_USERDATA

#endif // GFX_AS3_SUPPORT


// ***** MovieRoot definitions

namespace AS3 {

bool    MovieRoot::CreateObjectValue(GFx::Value* pval, GFx::Value::ObjectInterface* pobjifc, void* pdata, bool isdobj)
{
    SF_UNUSED(isdobj);

    GFx::Value::ValueType type = GFx::Value::VT_Object;
    AS3::Object*    object  = (AS3::Object*)pdata;

    if (AreDisplayObjectTraits(object))
        type = GFx::Value::VT_DisplayObject;
    else if (object->GetTraitsType() == Traits_Array)
        type = GFx::Value::VT_Array;

    // Clear existing managed ref first
    if (pval->IsManagedValue()) pval->ReleaseManagedValue();
    pval->Type = GFx::Value::ValueType(type | GFx::Value::VTC_ManagedBit);
    pval->mValue.pData = object;
    pval->pObjectInterface = pobjifc;
    // Make sure to add-ref before returning
    pobjifc->ObjectAddRef(pval, pdata);

    return true;
}

// Value conversion for external communication
void MovieRoot::GFxValue2ASValue(const GFx::Value& gfxVal, Value* pdestVal)
{
    
    SF_ASSERT(pdestVal);
    switch(gfxVal.GetType())
    {
    case GFx::Value::VT_Undefined: pdestVal->SetUndefined(); break;
    case GFx::Value::VT_Boolean:   pdestVal->SetBool(gfxVal.GetBool()); break;
    case GFx::Value::VT_Null:      pdestVal->SetNull(); break;
    case GFx::Value::VT_Int:       pdestVal->SetSInt32(gfxVal.GetInt()); break;
    case GFx::Value::VT_UInt:      pdestVal->SetUInt32(gfxVal.GetUInt()); break;
    case GFx::Value::VT_Number:    pdestVal->SetNumber(gfxVal.GetNumber()); break;
    case GFx::Value::VT_String:    
        {
            if (gfxVal.IsManagedValue())
            {
                size_t memberOffset = OFFSETOF(ASStringNode, pData);
                // Must be an assignment of a string from the same movie root
                SF_ASSERT(gfxVal.pObjectInterface->IsSameContext(pMovieImpl->pObjectInterface));
                ASStringNode* strnode = (ASStringNode*)(((UByte*)gfxVal.mValue.pStringManaged) - memberOffset);                
                *pdestVal = strnode;                
            }
            else
                *pdestVal = GetStringManager()->CreateString(gfxVal.GetString());
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
                *pdestVal = str;
            }
            else 
                *pdestVal = GetStringManager()->CreateString(gfxVal.GetStringW());
        }
        break;
    
    case GFx::Value::VT_Array:
    case GFx::Value::VT_Object:
    case GFx::Value::VT_DisplayObject:
        {            
            // Must be an assignment of an object from the same movie root
            SF_ASSERT(gfxVal.pObjectInterface->IsSameContext(pMovieImpl->pObjectInterface));
            Object* obj = (Object*)gfxVal.mValue.pData;
            if (obj)
            {
                const Traits& tr = obj->GetTraits();
                if (tr.GetTraitsType() == Traits_Function && tr.IsInstanceTraits())
                {
                    // Try to recreate Value::kFunction tag. It can be handy.
                    *pdestVal = (Instances::Function*)obj;
                    break;
                }
            }

            *pdestVal = obj;
        }
        break;    

#ifdef GFX_AS3_SUPPORT
    case GFx::Value::VT_Closure:
        {
            SF_ASSERT(gfxVal.pObjectInterface->IsSameContext(pMovieImpl->pObjectInterface));
            int closureType = ((UPInt)gfxVal.mValue.pData & CLOSURE_TYPE_BIT) ? 1 : 0;
            Object* pobj = (Object*)((UPInt)gfxVal.mValue.pData & ~CLOSURE_TYPE_BIT);
            if (closureType)
                // VTableIndClosure
                *pdestVal = AS3::Value(pobj, SInt32(gfxVal.DataAux), false);
            else
                // ThunkClosure
                *pdestVal = AS3::Value(pobj, *(const ThunkInfo*)gfxVal.DataAux);
        }
        break;
#endif
    default:
        break;
    }    
}

void MovieRoot::ASValue2GFxValue(const Value& value, GFx::Value* pdestVal) const
{
    SF_ASSERT(pdestVal);
    int toType;
    int closureType = 0;
    if (pdestVal->GetType() & GFx::Value::VTC_ConvertBit)
        toType = pdestVal->GetType() & (~GFx::Value::VTC_ConvertBit);
    else
    {
        switch(value.GetKind())
        {
        case Value::kUndefined: toType = GFx::Value::VT_Undefined; break;
        case Value::kBoolean:   toType = GFx::Value::VT_Boolean; break;
        case Value::kInt:       toType = GFx::Value::VT_Int; break;
        case Value::kUInt:      toType = GFx::Value::VT_UInt; break;
        case Value::kNumber:    toType = GFx::Value::VT_Number; break;

            // All treated as objects:
        case Value::kObject:
        case Value::kClass:
        case Value::kFunction:
        case Value::kThunkFunction:           
                toType = value.GetObject() ? GFx::Value::VT_Object : GFx::Value::VT_Null;
            break;       

        case Value::kThunkClosure:
                closureType = 0;
#ifdef GFX_AS3_SUPPORT
                toType = value.GetClosure() ? GFx::Value::VT_Closure : GFx::Value::VT_Null;
#else
                toType = GFx::Value::VT_Null;
#endif
            break;
        case Value::kVTableIndClosure:
                closureType = 1;
#ifdef GFX_AS3_SUPPORT
                toType = value.GetClosure() ? GFx::Value::VT_Closure : GFx::Value::VT_Null;
#else
                toType = GFx::Value::VT_Null;
#endif
            break;  

        // All other types are treated as strings.
        default:
            toType = GFx::Value::VT_String; break;
        }
        // Special cases
        if (value.IsNull()) toType = GFx::Value::VT_Null;
    }

    // Clear existing ref
    if (pdestVal->IsManagedValue())
        pdestVal->ReleaseManagedValue();

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
            pdestVal->mValue.BValue = value.AsBool();
        }
        break;
    case GFx::Value::VT_Int:
        {
            pdestVal->Type = GFx::Value::ValueType(toType);            
            if (!value.Convert2Int32(pdestVal->mValue.IValue))
            {
                pAVM->OutputAndIgnoreException();
                // No NaN type for Int32
                pdestVal->mValue.NValue = NumberUtil::NaN();
            } 
        }
        break;
    case GFx::Value::VT_UInt:
        {
            pdestVal->Type = GFx::Value::ValueType(toType);            
            if (!value.Convert2UInt32(pdestVal->mValue.UIValue))
            {
                pAVM->OutputAndIgnoreException();
                // No NaN type for SInt32
                pdestVal->mValue.NValue = NumberUtil::NaN();
            } 
        }
        break;
    case GFx::Value::VT_Number:
        {
            pdestVal->Type = GFx::Value::ValueType(toType);
            Value::Number v;
            if (!value.Convert2Number(v))
            {
				pAVM->OutputAndIgnoreException();
                pdestVal->mValue.NValue = NumberUtil::NaN();
            }
            else
                pdestVal->mValue.NValue = v;
        }
        break;
        
    case GFx::Value::VT_String:    
        {
            ASString str(pAVM->GetStringManager().GetBuiltin(AS3Builtin_empty_));
            if (!value.Convert2String(str))
                pAVM->OutputAndIgnoreException();

            size_t        memberOffset = GFX_VALUE_OFFSETOF(ASStringNode, pData);                        
            ASStringNode* strNode = str.GetNode();
            pdestVal->Type = GFx::Value::ValueType(GFx::Value::VT_String | GFx::Value::VTC_ManagedBit);
            pdestVal->mValue.pStringManaged = (const char**)(((UByte*)strNode) + memberOffset);
            pdestVal->pObjectInterface = pMovieImpl->pObjectInterface;
			pMovieImpl->pObjectInterface->ObjectAddRef(pdestVal, pdestVal->mValue.pData);
        }
        break;        
    case GFx::Value::VT_StringW:    
        {               
            ASString str(pAVM->GetStringManager().GetBuiltin(AS3Builtin_empty_));
            if (!value.Convert2String(str))
                pAVM->OutputAndIgnoreException();
            
            size_t memberOffset = GFX_VALUE_OFFSETOF(MovieImpl::WideStringStorage, pData);
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
            Object* object = value.GetObject();
            
             if (AreDisplayObjectTraits(object))
                type = GFx::Value::VT_DisplayObject;
             else if (object->GetTraitsType() == Traits_Array && object->GetTraits().IsInstanceTraits())
                type = GFx::Value::VT_Array;

             pdestVal->Type = GFx::Value::ValueType(type | GFx::Value::VTC_ManagedBit);
             pdestVal->mValue.pData = object;
             pdestVal->pObjectInterface = pMovieImpl->pObjectInterface;
             pMovieImpl->pObjectInterface->ObjectAddRef(pdestVal, pdestVal->mValue.pData);
        }
        break;

#ifdef GFX_AS3_SUPPORT
    case GFx::Value::VT_Closure:
        {
            Object* object = value.GetClosure();
            pdestVal->Type = GFx::Value::ValueType(toType | GFx::Value::VTC_ManagedBit);
            pdestVal->mValue.pData = (void*)((UPInt)object | (closureType ? CLOSURE_TYPE_BIT : 0x0));
            pdestVal->DataAux = closureType ? (UPInt)value.GetVTableInd() : (UPInt)&value.GetThunkFunct();
            pdestVal->pObjectInterface = pMovieImpl->pObjectInterface;
            pMovieImpl->pObjectInterface->ObjectAddRef(pdestVal, pdestVal->mValue.pData);
        }
        break;
#endif
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
    ASString asstr = GetStringManager()->CreateString(pstring);
    ASValue2GFxValue(asstr, pvalue);
}

void MovieRoot::CreateStringW(GFx::Value* pvalue, const wchar_t* pstring)
{
    ASString asstr = GetStringManager()->CreateString(pstring);
    ASValue2GFxValue(asstr, pvalue);
}

void MovieRoot::CreateObject(GFx::Value* pvalue, const char* className,
                             const GFx::Value* pargs, unsigned nargs)
{
    // Setup invoke parameters
    // ** Create storage
    enum { NumValuesOnStack = 10 };
    void* argArrayOnStack[NumValuesOnStack*((sizeof(GFx::Value)+sizeof(void*))/sizeof(void*))];
    Value* pargArray;
    if (nargs > NumValuesOnStack)
        pargArray = (Value*)
        SF_HEAP_AUTO_ALLOC_ID(this, sizeof(Value) * nargs, GFx::StatMV_ActionScript_Mem);
    else
        pargArray = (Value*)argArrayOnStack;
    // ** Convert to AS3::Value
    for (unsigned i = 0, n = nargs; i < n; ++i)
    {
        const GFx::Value& val = pargs[i];
        Value* pdestVal = Scaleform::Construct<Value>(&pargArray[i]);
        GFxValue2ASValue(val, pdestVal);
    }

    Value obj;
    const bool need2execute = pAVM->Construct(className ? className : "Object", pAVM->GetFrameAppDomain(), obj, nargs, pargArray, false); 
    if (!pAVM->IsException()) {
        if (need2execute)
            pAVM->ExecuteCode();
    } else {
        printf("Exception in CreateObject(\"%s\"):\n\t", className);
        pAVM->OutputAndIgnoreException();
    }
    ASValue2GFxValue(obj, pvalue);

    // Destruct elements in AS3::Value params array
    for (unsigned i = 0, n = nargs; i < n; ++i)
        pargArray[i].~Value();
    if (nargs > NumValuesOnStack)
        SF_FREE(pargArray);
}

void MovieRoot::CreateArray(GFx::Value* pvalue)
{
    Value arr(pAVM->MakeArray());

    ASValue2GFxValue(arr, pvalue);
}

void MovieRoot::CreateFunction(GFx::Value* pvalue, GFx::FunctionHandler* pfc, 
                               void* puserData /* = NULL */)
{
    // Hard way.
    unsigned size = sizeof(UserDefinedFunction);
    void* addr = Memory::AllocInHeap(pAVM->GetMemoryHeap(), size);

    // Placement new.
    AS3::Value asval(MakePickable(new(addr) 
        UserDefinedFunction(pAVM->GetITraitsFunction(), pfc, puserData))
        );
    ASValue2GFxValue(asval, pvalue);
}

} // namespace AS3


// In case someone decides to combine sources
#undef GFX_Value_ObjectInterface_CLASS

}} // Scaleform::GFx
