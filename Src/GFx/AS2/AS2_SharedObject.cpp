/**************************************************************************

Filename    :   AS2_SharedObject.cpp
Content     :   AS2 Implementation of SharedObject class
Created     :   January 20, 2009
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/AS2/AS2_SharedObject.h"

#if defined(GFX_AS2_ENABLE_SHAREDOBJECT)

#include "GFx/AS2/AS2_Action.h"
#include "Kernel/SF_Types.h"
#include "GFx/AS2/AS2_ArrayObject.h"
#include "Kernel/SF_MsgFormat.h"

namespace Scaleform { namespace GFx { namespace AS2 {

//#define SF_DEBUG_GASSHAREDOBJECTLOADER

//
// Custom visitor for SharedObject that loads data into a 
// Object instance.
//
class GASSharedObjectLoader : public SharedObjectVisitor
{
public:
    GASSharedObjectLoader(Environment* penv, Object* psobj) 
        : pEnv(penv), pData(psobj), bArrayIsTop(false) {}

    void Begin();
    void PushObject( const String& name );
    void PushArray( const String& name );
    void AddProperty( const String& name, const String& value, GFx::Value::ValueType type);
    void PopObject();
    void PopArray();
    void End();

private:
    Environment*     pEnv;
    Object*          pData;
    Array<Object*>  ObjectStack;
    bool                bArrayIsTop;
};


void GASSharedObjectLoader::Begin()
{
#ifdef SF_DEBUG_GASSHAREDOBJECTLOADER
    SF_DEBUG_MESSAGE(1, "GASSharedObjectLoader::Begin");
#endif // SF_DEBUG_GASSHAREDOBJECTLOADER

    ObjectStack.Clear();

    // Initialize root element
    ObjectStack.PushBack(pData);
}


void GASSharedObjectLoader::PushObject( const String& name )
{
#ifdef SF_DEBUG_GASSHAREDOBJECTLOADER
    SF_DEBUG_MESSAGE1(1, "GASSharedObjectLoader::PushObject('%s')", name.ToCStr());
#endif  // SF_DEBUG_GASSHAREDOBJECTLOADER

    SF_ASSERT(ObjectStack.GetSize() > 0);
    Object* ptop = ObjectStack.Back();

    // Push an object to stack
    Ptr<Object> pobj = *pEnv->OperatorNew(pEnv->GetBuiltin(ASBuiltin_Object));

    if (bArrayIsTop)
    {
        ArrayObject* ptoparr = static_cast<ArrayObject*>(ptop);
        ptoparr->PushBack(Value(pobj));
    }
    else
        ptop->SetMember(pEnv, pEnv->CreateString(name), Value(pobj));

    bArrayIsTop = false;
    ObjectStack.PushBack(pobj);
}


void GASSharedObjectLoader::PushArray( const String& name )
{
#ifdef SF_DEBUG_GASSHAREDOBJECTLOADER
    SF_DEBUG_MESSAGE1(1, "GASSharedObjectLoader::PushArray('%s')", name.ToCStr());
#endif  // SF_DEBUG_GASSHAREDOBJECTLOADER

    SF_ASSERT(ObjectStack.GetSize() > 0);
    Object* ptop = ObjectStack.Back();

    // Push an array to stack
    Ptr<Object> parr = *pEnv->OperatorNew(pEnv->GetBuiltin(ASBuiltin_Array));

    if (bArrayIsTop)
    {
        ArrayObject* ptoparr = static_cast<ArrayObject*>(ptop);
        ptoparr->PushBack(Value(parr));
    }
    else
        ptop->SetMember(pEnv, pEnv->CreateString(name), Value(parr));

    bArrayIsTop = true;
    ObjectStack.PushBack(parr);
}


void GASSharedObjectLoader::AddProperty(const String &name, 
                                         const String &value, 
                                         GFx::Value::ValueType type)
{
#ifdef SF_DEBUG_GASSHAREDOBJECTLOADER
    SF_DEBUG_MESSAGE2(1, "GASSharedObjectLoader::PushProperty('%s', '%s')", 
        name.ToCStr(), value.ToCStr());
#endif  // SF_DEBUG_GASSHAREDOBJECTLOADER

    SF_ASSERT(ObjectStack.GetSize() > 0);
    Object* ptop = ObjectStack.Back();

    // Add property to top element
    Value v;
    switch (type)
    {
    case GFx::Value::VT_Number:
        {
            v.SetNumber(atof(value.ToCStr()));
            break;
        }
    case GFx::Value::VT_String:
        {
            v.SetString(pEnv->CreateString(value));
            break;
        }
    case GFx::Value::VT_Boolean:
        {
            v.SetBool( !SFstrncmp(value.ToCStr(), "true", 4) );
            break;
        }
    case GFx::Value::VT_Undefined:
        {
            v.SetUndefined();
            break;
        }
    case GFx::Value::VT_Null:
        {
            v.SetNull();
            break;
        }
    default:
        {
            SF_ASSERT(0); // Unknown case
        }
    }

    if (bArrayIsTop)
    {
        ArrayObject* ptoparr = static_cast<ArrayObject*>(ptop);
        ptoparr->PushBack(v);
    }
    else
        ptop->SetMember(pEnv, pEnv->CreateString(name), v);
}


void GASSharedObjectLoader::PopObject()
{
#ifdef SF_DEBUG_GASSHAREDOBJECTLOADER
    SF_DEBUG_MESSAGE(1, "GASSharedObjectLoader::PopObject");
#endif  // SF_DEBUG_GASSHAREDOBJECTLOADER

    // Root should not be popped
    SF_ASSERT(ObjectStack.GetSize() > 1);
    ObjectStack.PopBack();
    // Evaluate current state
    Object* ptop = ObjectStack.Back();
    bArrayIsTop = (ptop->GetObjectType() == Object::Object_Array);
}


void GASSharedObjectLoader::PopArray()
{
#ifdef SF_DEBUG_GASSHAREDOBJECTLOADER
    SF_DEBUG_MESSAGE(1, "GASSharedObjectLoader::PopArray");
#endif  // SF_DEBUG_GASSHAREDOBJECTLOADER

    PopObject();
}


void GASSharedObjectLoader::End()
{
#ifdef SF_DEBUG_GASSHAREDOBJECTLOADER
    SF_DEBUG_MESSAGE(1, "GASSharedObjectLoader::End");
#endif  // SF_DEBUG_GASSHAREDOBJECTLOADER

    // Check stack state; only root element must exist
    SF_ASSERT(bArrayIsTop == false);
    SF_ASSERT(ObjectStack.GetSize() == 1);
}


///////////////////////////////////////////
//

SharedObject::SharedObject(Environment* penv)
: Object(penv)
{
    Set__proto__ (penv->GetSC(), penv->GetPrototype(ASBuiltin_SharedObject));
}

//
// From Flash Docs:
//
// name:String - A string that represents the name of the object. The 
//               name can include forward slashes (/); for example, 
//               work/addresses is a legal name. Spaces are not allowed 
//               in a shared object name, nor are the following characters: 
//
//               ~ % & \ ; : " ' , < > ? #
//
bool    SharedObject::SetNameAndLocalPath(const String& name, 
                                             const String& localPath )
{  
    const char* pstr;
    UInt32 c = name.GetFirstCharAt(0, &pstr);
    while (c)
    {
        switch (c)
        {
        case '~':
        case '%':
        case '&':
        case '\\':
        case ';':
        case ':':
        case '"':
        case '\'':
        case ',':
        case '<':
        case '>':
        case '?':
        case '#':
            return false;
        }
        // Check whitespace
        if (SFiswspace((wchar_t)c))
            return false;
        c = name.GetNextChar(&pstr);
    }
    Name = name;
    LocalPath = localPath;
    return true;
}


bool SharedObject::SetMember(Environment* penv, const ASString& name, 
                                const Value& val, 
                                const PropFlags& flags /* = PropFlags */)
{
    // No assignments to 'data' member are allowed
    if (name == "data")
        return true;
    return Object::SetMember(penv, name, val, flags);
}


void SharedObject::SetDataObject(Environment* penv, Object* pobj)
{
    Object::SetMember(penv, penv->CreateConstString("data"), Value(pobj));
}

//
// From Flash Docs:
//
// "Flash calculates the size of a shared object by stepping through 
//  all of its data properties; the more data properties the object 
//  has, the longer it takes to estimate its size. Estimating object 
//  size can take significant processing time, so you may want to 
//  avoid using this method unless you have a specific need for it."
//
UPInt SharedObject::ComputeSizeInBytes(Environment* penv)
{
    // Visit all members and accumulate member sizes
    Value val;
    bool hasData = Object::GetMember(penv, penv->CreateConstString("data"), &val);
    SF_ASSERT(hasData);
    SF_UNUSED(hasData);
    Object* pdataObj = val.ToObject(penv);
    SF_ASSERT(pdataObj);

    struct DataSizeEstimator : public ObjectInterface::MemberVisitor
    {
        Environment*             pEnv;
        UPInt                       SizeInBytes;

        DataSizeEstimator(Environment *penv)
        {
            pEnv = penv;
            SizeInBytes = 0;
        }

        virtual void Visit(const ASString& name, const Value& val, UByte flags)
        {
            SF_UNUSED(flags);
            SizeInBytes += name.GetSize();
            switch (val.GetType())
            {
            case Value::NUMBER:
            case Value::INTEGER:
                {
                    SizeInBytes += sizeof(float);
                    break;
                }
            case Value::STRING:
                {
                    SizeInBytes += val.ToString(pEnv).GetSize();
                    break;
                }
            case Value::OBJECT:
                {
                    Object* pobj = val.ToObject(pEnv);
                    pobj->VisitMembers(pEnv->GetSC(), this, 0);
                    break;
                }
            default:;
            }
        }
    } visitor(penv);    
    pdataObj->VisitMembers(penv->GetSC(), &visitor, 0);
    return visitor.SizeInBytes;
}


void SharedObject::Flush(Environment* penv, SharedObjectVisitor* writer)
{
    if (!writer)
        return;

    // Visit all members and notify writer
    Value val;
    bool hasData = Object::GetMember(penv, penv->CreateConstString("data"), &val);
    SF_ASSERT(hasData);
    SF_UNUSED(hasData);
    Object* pdataObj = val.ToObject(penv);
    SF_ASSERT(pdataObj);
    
    struct DataWriter : public ObjectInterface::MemberVisitor
    {
        // Hash is required to avoid infinite loops when objects have
        // circular references. If an object has been visited we do 
        // not visit it again.
        HashIdentity<Object*, Object*>   VisitedObjects;
        Environment*                         pEnv;
        SharedObjectVisitor*                 pWriter;

        DataWriter(Environment *penv, SharedObjectVisitor* pwriter)
        {
            pEnv = penv;
            pWriter = pwriter;
        }

        virtual void Visit(const ASString& name, const Value& val, UByte flags)
        {
            SF_UNUSED(flags);
            String propname(name.ToCStr(), name.GetSize());
            switch (val.GetType())
            {
            case Value::NUMBER:
            case Value::INTEGER:
                {
                    // writeout number                    
                    DoubleFormatter f(val.ToNumber(pEnv));
                    f.Convert();
                    String sval(f, f.GetSize());

                    pWriter->AddProperty( propname, sval, 
                        GFx::Value::VT_Number);
                    break;
                }
            case Value::STRING:
                {
                    // writeout string
                    String sval(val.ToString(pEnv).ToCStr());
                    pWriter->AddProperty( propname, sval, 
                        GFx::Value::VT_String);
                    break;
                }
            case Value::UNDEFINED:
                {
                    // writeout undefined
                    pWriter->AddProperty( propname, "",
                        GFx::Value::VT_Undefined);
                    break;
                }
            case Value::BOOLEAN:
                {
                    // writeout boolean                    
                    pWriter->AddProperty( propname, 
                        val.ToBool(pEnv) ? "true" : "false", GFx::Value::VT_Boolean);
                    break;
                }
            case Value::NULLTYPE:
                {
                    // writeout null
                    pWriter->AddProperty( propname, "", GFx::Value::VT_Null);
                    break;
                }
            case Value::OBJECT:
                {
                    // special case for Array
                    Object* pobj = val.ToObject(pEnv);
                    if (VisitedObjects.Get(pobj)) break;
                    VisitedObjects.Add(pobj, pobj);
                    if (pobj->GetObjectType() == Object::Object_Array)
                    {
                        // writeout array
                        pWriter->PushArray( propname );
                        pobj->VisitMembers(pEnv->GetSC(), this, 0);
                        pWriter->PopArray();
                    }
                    else
                    {
                        // writeout object
                        pWriter->PushObject( propname );
                        pobj->VisitMembers(pEnv->GetSC(), this, 0);
                        pWriter->PopObject();
                    }
                    break;
                }
            default:;
            }
        }
    } visitor(penv, writer);
    writer->Begin();
    pdataObj->VisitMembers(penv->GetSC(), &visitor, 0);
    writer->End();
}


//////////////////////////////////////////
//


const NameFunction SharedObjectProto::FunctionTable[] = 
{
    { "clear",          &SharedObjectProto::Clear },
    { "flush",          &SharedObjectProto::Flush },
    { "getSize",        &SharedObjectProto::GetSize },
    { 0, 0 }
};


SharedObjectProto::SharedObjectProto(ASStringContext* psc, 
                                           Object* pprototype, 
                                           const FunctionRef& constructor) : 
Prototype<SharedObject>(psc, pprototype, constructor)
{
    InitFunctionMembers (psc, FunctionTable);
}


//
// public clear() : Void
//
// Purges all the data from the shared object and deletes the shared object from the disk.
//
void SharedObjectProto::Clear(const FnCall &fn)
{
    CHECK_THIS_PTR(fn, SharedObject);
    SharedObject* pthis = (SharedObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    // Set a new data object (essentially clearing)
    Ptr<Object> pdataObj = *fn.Env->OperatorNew(fn.Env->GetBuiltin(ASBuiltin_Object));
    pthis->SetDataObject(fn.Env, pdataObj);

    // Write out
    Ptr<SharedObjectManagerBase> psoMgr = fn.Env->GetMovieImpl()->GetSharedObjectManager();
    if (psoMgr)
    {
        Ptr<SharedObjectVisitor> pwriter = *psoMgr->CreateWriter(pthis->GetName(), 
            pthis->GetLocalPath(), fn.Env->GetMovieImpl()->GetFileOpener());
       pthis->Flush(fn.Env, pwriter);
    }
}


//
// public flush([minDiskSpace:Number]) : Object
// 
// Immediately writes a locally persistent shared object to a local 
// file. If you don't use this method, Flash writes the shared object 
// to a file when the shared object session ends--that is, when the 
// SWF file is closed, that is when the shared object is 
// garbage-collected because it no longer has any references to it 
// or you call SharedObject.clear(). 
//
// Returns
// 
// Object - A Boolean value: true or false; or a string value of 
// "pending", as described in the following list:
// 
// * If the user has permitted local information storage for objects 
//   from this domain, and the amount of space allotted is sufficient 
//   to store the object, this method returns true. (If you have 
//   passed a value for minimumDiskSpace, the amount of space allotted 
//   must be at least equal to that value for true to be returned).
// * If the user has permitted local information storage for objects 
//   from this domain, but the amount of space allotted is not 
//   sufficient to store the object, this method returns "pending".
// * If the user has permanently denied local information storage for 
//   objects from this domain, or if Flash cannot save the object for 
//   any reason, this method returns false.
//
void SharedObjectProto::Flush(const FnCall &fn)
{
    CHECK_THIS_PTR(fn, SharedObject);
    SharedObject* pthis = (SharedObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    // Write out
    Ptr<SharedObjectManagerBase> psoMgr = fn.Env->GetMovieImpl()->GetSharedObjectManager();
    if (psoMgr)
    {
        Ptr<SharedObjectVisitor> pwriter = *psoMgr->CreateWriter(pthis->GetName(), 
            pthis->GetLocalPath(), fn.Env->GetMovieImpl()->GetFileOpener());
        pthis->Flush(fn.Env, pwriter);
    }
}


//
// public getSize() : Number
// 
// Gets the current size of the shared object, in bytes. 
//
// Returns
// 
// Number - A numeric value specifying the size of the shared object,
// in bytes.
//
void SharedObjectProto::GetSize(const FnCall& fn)
{
    CHECK_THIS_PTR(fn, SharedObject);
    SharedObject* pthis = (SharedObject*)fn.ThisPtr;
    SF_ASSERT(pthis);
    if (!pthis) return;

    fn.Result->SetUInt(unsigned(pthis->ComputeSizeInBytes(fn.Env)));
}


//////////////////


const NameFunction SharedObjectCtorFunction::StaticFunctionTable[] = 
{
    { "getLocal",    &SharedObjectCtorFunction::GetLocal },
    { 0, 0 }
};


SharedObjectCtorFunction::SharedObjectCtorFunction(ASStringContext *psc)
:   CFunctionObject(psc, GlobalCtor)
{
    NameFunction::AddConstMembers(this, psc, StaticFunctionTable, 
        PropFlags::PropFlag_ReadOnly | PropFlags::PropFlag_DontDelete | 
        PropFlags::PropFlag_DontEnum);
}


#ifdef GFX_AS_ENABLE_GC
template <class Functor>
void SharedObjectCtorFunction::ForEachChild_GC(Collector* prcc) const
{
    Object::template ForEachChild_GC<Functor>(prcc);
    SOLookupType::ConstIterator it = SharedObjects.Begin();
    while(it != SharedObjects.End())
    {   
        const SharedObjectPtr& pobj = it->Second;
        pobj.template ForEachChild_GC<Functor>(prcc);
        ++it;
    }
}
SF_GC_PREGEN_FOR_EACH_CHILD(SharedObjectCtorFunction)

void SharedObjectCtorFunction::ExecuteForEachChild_GC(Collector* prcc, OperationGC operation) const
{
    ASRefCountBaseType::CallForEachChild<SharedObjectCtorFunction>(prcc, operation);
}

void SharedObjectCtorFunction::Finalize_GC()
{
    SharedObjects.~SOLookupType();
    Object::Finalize_GC();
}
#endif //SF_NO_GC


//
// public static getLocal(name:String, [localPath:String], [secure:Boolean]) : SharedObject
//
// Returns a reference to a locally persistent shared object that is 
// available only to the current client. If the shared object does 
// not already exist, this method creates one. This method is a static 
// method of the SharedObject class.
//
// Returns
//
// SharedObject - A reference to a shared object that is persistent 
// locally and is available only to the current client. If Flash 
// Player can't create or find the shared object (for example, if 
// localPath was specified but no such directory exists, or if the 
// secure parameter is used incorrectly) this method returns null. 
//
void SharedObjectCtorFunction::GetLocal(const FnCall& fn)
{
    if (fn.NArgs < 1)
        return;

    ASString name = fn.Arg(0).ToString(fn.Env);
    ASString localPath = fn.Env->CreateConstString("");
    if (fn.NArgs > 1)
        localPath = fn.Arg(1).ToString(fn.Env);
    String strName(name.ToCStr());
    String strLocalPath(localPath.ToCStr());

    // Check for stored shared obj
    ASString strFullPath = fn.Env->CreateString(strLocalPath);
    strFullPath += ":";
    strFullPath += strName;
    FunctionRef fref = fn.Env->GetConstructor(ASBuiltin_SharedObject);
    SF_ASSERT(fref.Function);
    SharedObjectCtorFunction* pctorobj = static_cast<SharedObjectCtorFunction*>(fref.Function);
    SharedObjectPtr* psavedObj = pctorobj->SharedObjects.Get(strFullPath);
    if (psavedObj)
    {
        // Found a stored shared obj; return it
        fn.Result->SetAsObject(*psavedObj);
        return;
    }

    Ptr<Object> pobj = *fn.Env->OperatorNew(fn.Env->GetBuiltin(ASBuiltin_SharedObject));
    SharedObject* pthis = static_cast<SharedObject*>(pobj.GetPtr());

    // If name and directory aren't formatted correctly, fail
    if (!pthis->SetNameAndLocalPath(strName, strLocalPath))
    {
        fn.Result->SetNull();
        return;
    }

    Ptr<Object> pdataObj = *fn.Env->OperatorNew(fn.Env->GetBuiltin(ASBuiltin_Object));
    GASSharedObjectLoader   loader(fn.Env, pdataObj);

    Ptr<SharedObjectManagerBase> psoMgr = fn.Env->GetMovieImpl()->GetSharedObjectManager();
    FileOpenerBase*  pfileopener = fn.Env->GetMovieImpl()->GetFileOpener();
    if (!psoMgr || !psoMgr->LoadSharedObject(strName, strLocalPath, &loader, pfileopener))
    {
        fn.Result->SetNull();
        return;
    }
    else
    {
        pthis->SetDataObject(fn.Env, pdataObj);
        fn.Result->SetAsObject(pthis);
        // Store for future lookup
        pctorobj->SharedObjects.Add(strFullPath, pthis);
    }
}


Object* SharedObjectCtorFunction::CreateNewObject(Environment* penv) const 
{
    return SF_HEAP_NEW(penv->GetHeap()) SharedObject(penv);
}


void SharedObjectCtorFunction::GlobalCtor(const FnCall& fn)
{
    Ptr<SharedObject> ab = *SF_HEAP_NEW(fn.Env->GetHeap()) SharedObject(fn.Env);
    fn.Result->SetAsObject(ab.GetPtr());
}


FunctionRef SharedObjectCtorFunction::Register(GlobalContext* pgc)
{
    ASStringContext sc(pgc, 8);
    FunctionRef  ctor(*SF_HEAP_NEW(pgc->GetHeap()) SharedObjectCtorFunction(&sc));
    Ptr<Object> proto = *SF_HEAP_NEW(pgc->GetHeap()) SharedObjectProto(&sc, 
        pgc->GetPrototype(ASBuiltin_Object), ctor);
    pgc->SetPrototype(ASBuiltin_SharedObject, proto);
    pgc->pGlobal->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_SharedObject), Value(ctor));
    return ctor;
}

}}} //SF::GFx::AS2

#endif  // SF_NO_FXPLAYER_AS_SHAREDOBJECT
