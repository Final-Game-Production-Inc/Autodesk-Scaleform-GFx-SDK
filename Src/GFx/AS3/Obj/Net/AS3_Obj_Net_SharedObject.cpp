//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Net_SharedObject.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Net_SharedObject.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "AS3_Obj_Net_NetConnection.h"
#include "../../AS3_MovieRoot.h"
#include "Kernel/SF_String.h"
#include "AS3_Obj_Net_SharedObjectFlushStatus.h"
#include "Kernel/SF_MsgFormat.h"
#include "../AS3_Obj_Array.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"

//#define SF_DEBUG_ASSHAREDOBJECTLOADER

#ifdef GFX_AS3_ENABLE_SHAREDOBJECT
//
// Custom visitor for SharedObject that loads data into a 
// Object instance.
//
class ASSharedObjectLoader : public SharedObjectVisitor
{
public:
    ASSharedObjectLoader(VM* pvm, Instances::fl::Object* psobj) 
        : pVM(pvm), pData(psobj), bArrayIsTop(false) {}

    virtual void Begin();
    virtual void PushObject( const String& name );
    virtual void PushArray( const String& name );
    virtual void AddProperty( const String& name, const String& value, GFx::Value::ValueType type);
    virtual void PopObject();
    virtual void PopArray();
    virtual void End();

private:
    VM*                         pVM;
    Instances::fl::Object*          pData;
    Array<Instances::fl::Object*>   ObjectStack;
    bool                        bArrayIsTop;
};

void ASSharedObjectLoader::Begin()
{
#ifdef SF_DEBUG_ASSHAREDOBJECTLOADER
    SF_DEBUG_MESSAGE(1, "ASSharedObjectLoader::Begin");
#endif // SF_DEBUG_ASSHAREDOBJECTLOADER

    ObjectStack.Clear();

    // Initialize root element
    ObjectStack.PushBack(pData);
}


void ASSharedObjectLoader::PushObject( const String& name )
{
#ifdef SF_DEBUG_ASSHAREDOBJECTLOADER
    SF_DEBUG_MESSAGE1(1, "ASSharedObjectLoader::PushObject('%s')", name.ToCStr());
#endif  // SF_DEBUG_ASSHAREDOBJECTLOADER

    SF_ASSERT(ObjectStack.GetSize() > 0);
    Object* ptop = ObjectStack.Back();

    // Push an object to stack
    SPtr<Instances::fl::Object> pobj = pVM->MakeObject();

    if (bArrayIsTop)
    {
        Instances::fl::Array* ptoparr = static_cast<Instances::fl::Array*>(ptop);
        ptoparr->PushBack(Value(pobj));
    }
    else
    {
        ptop->SetProperty(Multiname(*pVM, StringDataPtr(name.ToCStr(), name.GetLength())), Value(pobj)).DoNotCheck();
    }

    bArrayIsTop = false;
    ObjectStack.PushBack(pobj);
}


void ASSharedObjectLoader::PushArray( const String& name )
{
#ifdef SF_DEBUG_ASSHAREDOBJECTLOADER
    SF_DEBUG_MESSAGE1(1, "ASSharedObjectLoader::PushArray('%s')", name.ToCStr());
#endif  // SF_DEBUG_ASSHAREDOBJECTLOADER

    SF_ASSERT(ObjectStack.GetSize() > 0);
    Object* ptop = ObjectStack.Back();

    // Push an array to stack
    SPtr<Instances::fl::Array> parr = pVM->MakeArray();

    if (bArrayIsTop)
    {
        Instances::fl::Array* ptoparr = static_cast<Instances::fl::Array*>(ptop);
        ptoparr->PushBack(Value(parr));
    }
    else
    {
        ptop->SetProperty(Multiname(*pVM, StringDataPtr(name.ToCStr(), name.GetLength())), Value(parr)).DoNotCheck() ;
    }

    bArrayIsTop = true;
    ObjectStack.PushBack(parr);
}


void ASSharedObjectLoader::AddProperty(const String &name, 
                                        const String &value, 
                                        GFx::Value::ValueType type)
{
#ifdef SF_DEBUG_ASSHAREDOBJECTLOADER
    SF_DEBUG_MESSAGE2(1, "ASSharedObjectLoader::PushProperty('%s', '%s')", 
        name.ToCStr(), value.ToCStr());
#endif  // SF_DEBUG_ASSHAREDOBJECTLOADER

    SF_ASSERT(ObjectStack.GetSize() > 0);
    Object* ptop = ObjectStack.Back();

    // Add property to top element
    Value v;
    switch (type)
    {
    case GFx::Value::VT_Int:
        {
            v.SetSInt32(atoi(value.ToCStr()));
            break;
        }
    case GFx::Value::VT_UInt:
        {
            v.SetUInt32(atoi(value.ToCStr()));
            break;
        }
    case GFx::Value::VT_Number:
        {
            v.SetNumber(atof(value.ToCStr()));
            break;
        }
    case GFx::Value::VT_String:
        {
            v.Assign(pVM->GetStringManager().CreateString(value));
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
        Instances::fl::Array* ptoparr = static_cast<Instances::fl::Array*>(ptop);
        ptoparr->PushBack(v);
    }
    else
    {
        ptop->SetProperty(Multiname(*pVM, StringDataPtr(name.ToCStr(), name.GetLength())), v).DoNotCheck();
    }
}


void ASSharedObjectLoader::PopObject()
{
#ifdef SF_DEBUG_ASSHAREDOBJECTLOADER
    SF_DEBUG_MESSAGE(1, "ASSharedObjectLoader::PopObject");
#endif  // SF_DEBUG_ASSHAREDOBJECTLOADER

    // Root should not be popped
    SF_ASSERT(ObjectStack.GetSize() > 1);
    ObjectStack.PopBack();
    // Evaluate current state
    Instances::fl::Object* ptop = ObjectStack.Back();
    
    bArrayIsTop = ptop->GetTraitsType() == Traits_Array && 
        ptop->GetTraits().IsInstanceTraits();
}


void ASSharedObjectLoader::PopArray()
{
#ifdef SF_DEBUG_ASSHAREDOBJECTLOADER
    SF_DEBUG_MESSAGE(1, "ASSharedObjectLoader::PopArray");
#endif  // SF_DEBUG_ASSHAREDOBJECTLOADER

    PopObject();
}


void ASSharedObjectLoader::End()
{
#ifdef SF_DEBUG_ASSHAREDOBJECTLOADER
    SF_DEBUG_MESSAGE(1, "ASSharedObjectLoader::End");
#endif  // SF_DEBUG_ASSHAREDOBJECTLOADER

    // Check stack state; only root element must exist
    SF_ASSERT(bArrayIsTop == false);
    SF_ASSERT(ObjectStack.GetSize() == 1);
}

//////////////////////////////////////////////////////////////////////////

#endif  // GFX_AS3_ENABLE_SHAREDOBJECT

//##protect##"methods"

#ifndef SF_AS3_EMIT_DEF_ARGS
// Values of default arguments.
namespace Impl
{

    template <>
    SF_INLINE
    ASString GetMethodDefArg<Instances::fl_net::SharedObject, Instances::fl_net::SharedObject::mid_connect, 1, const ASString&>(AS3::StringManager& sm)
    {
        return sm.CreateConstString("null");
    }

    template <>
    SF_INLINE
    Value GetMethodDefArg<Instances::fl_net::SharedObject, Instances::fl_net::SharedObject::mid_setProperty, 1, const Value&>(AS3::StringManager&)
    {
        return Value::GetNull();
    }

    template <>
    SF_INLINE
    ASString GetMethodDefArg<Classes::fl_net::SharedObject, Classes::fl_net::SharedObject::mid_getLocal, 1, const ASString&>(AS3::StringManager& sm)
    {
        return sm.CreateConstString("null");
    }

    template <>
    SF_INLINE
    ASString GetMethodDefArg<Classes::fl_net::SharedObject, Classes::fl_net::SharedObject::mid_getRemote, 1, const ASString&>(AS3::StringManager& sm)
    {
        return sm.CreateConstString("null");
    }

    template <>
    SF_INLINE
    Value GetMethodDefArg<Classes::fl_net::SharedObject, Classes::fl_net::SharedObject::mid_getRemote, 2, const Value&>(AS3::StringManager&)
    {
        return Value(false);
    }

} // namespace Impl
#endif // SF_AS3_EMIT_DEF_ARGS

typedef ThunkFunc0<Instances::fl_net::SharedObject, Instances::fl_net::SharedObject::mid_clientGet, SPtr<Instances::fl::Object> > TFunc_Instances_SharedObject_clientGet;
typedef ThunkFunc1<Instances::fl_net::SharedObject, Instances::fl_net::SharedObject::mid_clientSet, const Value, const Value&> TFunc_Instances_SharedObject_clientSet;
typedef ThunkFunc0<Instances::fl_net::SharedObject, Instances::fl_net::SharedObject::mid_dataGet, SPtr<Instances::fl::Object> > TFunc_Instances_SharedObject_dataGet;
typedef ThunkFunc1<Instances::fl_net::SharedObject, Instances::fl_net::SharedObject::mid_defaultObjectEncodingSet, const Value, UInt32> TFunc_Instances_SharedObject_defaultObjectEncodingSet;
typedef ThunkFunc1<Instances::fl_net::SharedObject, Instances::fl_net::SharedObject::mid_fpsSet, const Value, Value::Number> TFunc_Instances_SharedObject_fpsSet;
typedef ThunkFunc0<Instances::fl_net::SharedObject, Instances::fl_net::SharedObject::mid_objectEncodingGet, UInt32> TFunc_Instances_SharedObject_objectEncodingGet;
typedef ThunkFunc1<Instances::fl_net::SharedObject, Instances::fl_net::SharedObject::mid_objectEncodingSet, const Value, UInt32> TFunc_Instances_SharedObject_objectEncodingSet;
typedef ThunkFunc0<Instances::fl_net::SharedObject, Instances::fl_net::SharedObject::mid_sizeGet, UInt32> TFunc_Instances_SharedObject_sizeGet;
typedef ThunkFunc0<Instances::fl_net::SharedObject, Instances::fl_net::SharedObject::mid_clear, const Value> TFunc_Instances_SharedObject_clear;
typedef ThunkFunc0<Instances::fl_net::SharedObject, Instances::fl_net::SharedObject::mid_close, const Value> TFunc_Instances_SharedObject_close;
typedef ThunkFunc2<Instances::fl_net::SharedObject, Instances::fl_net::SharedObject::mid_connect, const Value, Instances::fl_net::NetConnection*, const ASString&> TFunc_Instances_SharedObject_connect;
typedef ThunkFunc1<Instances::fl_net::SharedObject, Instances::fl_net::SharedObject::mid_flush, ASString, SInt32> TFunc_Instances_SharedObject_flush;
typedef ThunkFunc2<Instances::fl_net::SharedObject, Instances::fl_net::SharedObject::mid_send, Value, unsigned, const Value*> TFunc_Instances_SharedObject_send;
typedef ThunkFunc1<Instances::fl_net::SharedObject, Instances::fl_net::SharedObject::mid_setDirty, const Value, const ASString&> TFunc_Instances_SharedObject_setDirty;
typedef ThunkFunc2<Instances::fl_net::SharedObject, Instances::fl_net::SharedObject::mid_setProperty, const Value, const ASString&, const Value&> TFunc_Instances_SharedObject_setProperty;

template <> const TFunc_Instances_SharedObject_clientGet::TMethod TFunc_Instances_SharedObject_clientGet::Method = &Instances::fl_net::SharedObject::clientGet;
template <> const TFunc_Instances_SharedObject_clientSet::TMethod TFunc_Instances_SharedObject_clientSet::Method = &Instances::fl_net::SharedObject::clientSet;
template <> const TFunc_Instances_SharedObject_dataGet::TMethod TFunc_Instances_SharedObject_dataGet::Method = &Instances::fl_net::SharedObject::dataGet;
template <> const TFunc_Instances_SharedObject_defaultObjectEncodingSet::TMethod TFunc_Instances_SharedObject_defaultObjectEncodingSet::Method = &Instances::fl_net::SharedObject::defaultObjectEncodingSet;
template <> const TFunc_Instances_SharedObject_fpsSet::TMethod TFunc_Instances_SharedObject_fpsSet::Method = &Instances::fl_net::SharedObject::fpsSet;
template <> const TFunc_Instances_SharedObject_objectEncodingGet::TMethod TFunc_Instances_SharedObject_objectEncodingGet::Method = &Instances::fl_net::SharedObject::objectEncodingGet;
template <> const TFunc_Instances_SharedObject_objectEncodingSet::TMethod TFunc_Instances_SharedObject_objectEncodingSet::Method = &Instances::fl_net::SharedObject::objectEncodingSet;
template <> const TFunc_Instances_SharedObject_sizeGet::TMethod TFunc_Instances_SharedObject_sizeGet::Method = &Instances::fl_net::SharedObject::sizeGet;
template <> const TFunc_Instances_SharedObject_clear::TMethod TFunc_Instances_SharedObject_clear::Method = &Instances::fl_net::SharedObject::clear;
template <> const TFunc_Instances_SharedObject_close::TMethod TFunc_Instances_SharedObject_close::Method = &Instances::fl_net::SharedObject::close;
template <> const TFunc_Instances_SharedObject_connect::TMethod TFunc_Instances_SharedObject_connect::Method = &Instances::fl_net::SharedObject::connect;
template <> const TFunc_Instances_SharedObject_flush::TMethod TFunc_Instances_SharedObject_flush::Method = &Instances::fl_net::SharedObject::flush;
template <> const TFunc_Instances_SharedObject_send::TMethod TFunc_Instances_SharedObject_send::Method = &Instances::fl_net::SharedObject::send;
template <> const TFunc_Instances_SharedObject_setDirty::TMethod TFunc_Instances_SharedObject_setDirty::Method = &Instances::fl_net::SharedObject::setDirty;
template <> const TFunc_Instances_SharedObject_setProperty::TMethod TFunc_Instances_SharedObject_setProperty::Method = &Instances::fl_net::SharedObject::setProperty;

namespace Instances { namespace fl_net
{
    SharedObject::SharedObject(InstanceTraits::Traits& t)
    : Instances::fl_events::EventDispatcher(t)
//##protect##"instance::SharedObject::SharedObject()$data"
//##protect##"instance::SharedObject::SharedObject()$data"
    {
//##protect##"instance::SharedObject::SharedObject()$code"
//##protect##"instance::SharedObject::SharedObject()$code"
    }

    void SharedObject::clientGet(SPtr<Instances::fl::Object>& result)
    {
//##protect##"instance::SharedObject::clientGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("instance::SharedObject::clientGet()");
//##protect##"instance::SharedObject::clientGet()"
    }
    void SharedObject::clientSet(const Value& result, const Value& value)
    {
//##protect##"instance::SharedObject::clientSet()"
        SF_UNUSED2(result, value);
        WARN_NOT_IMPLEMENTED("instance::SharedObject::clientSet()");
//##protect##"instance::SharedObject::clientSet()"
    }
    void SharedObject::dataGet(SPtr<Instances::fl::Object>& result)
    {
//##protect##"instance::SharedObject::dataGet()"
        result = DataObj;
//##protect##"instance::SharedObject::dataGet()"
    }
    void SharedObject::defaultObjectEncodingSet(const Value& result, UInt32 value)
    {
//##protect##"instance::SharedObject::defaultObjectEncodingSet()"
        SF_UNUSED2(result, value);
        WARN_NOT_IMPLEMENTED("instance::SharedObject::defaultObjectEncodingSet()");
//##protect##"instance::SharedObject::defaultObjectEncodingSet()"
    }
    void SharedObject::fpsSet(const Value& result, Value::Number value)
    {
//##protect##"instance::SharedObject::fpsSet()"
        SF_UNUSED2(result, value);
        WARN_NOT_IMPLEMENTED("instance::SharedObject::fpsSet()");
//##protect##"instance::SharedObject::fpsSet()"
    }
    void SharedObject::objectEncodingGet(UInt32& result)
    {
//##protect##"instance::SharedObject::objectEncodingGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("instance::SharedObject::objectEncodingGet()");
//##protect##"instance::SharedObject::objectEncodingGet()"
    }
    void SharedObject::objectEncodingSet(const Value& result, UInt32 value)
    {
//##protect##"instance::SharedObject::objectEncodingSet()"
        SF_UNUSED2(result, value);
        WARN_NOT_IMPLEMENTED("instance::SharedObject::objectEncodingSet()");
//##protect##"instance::SharedObject::objectEncodingSet()"
    }
    void SharedObject::sizeGet(UInt32& result)
    {
//##protect##"instance::SharedObject::sizeGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("instance::SharedObject::sizeGet()");
//##protect##"instance::SharedObject::sizeGet()"
    }
    void SharedObject::clear(const Value& result)
    {
//##protect##"instance::SharedObject::clear()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("instance::SharedObject::clear()");
//##protect##"instance::SharedObject::clear()"
    }
    void SharedObject::close(const Value& result)
    {
//##protect##"instance::SharedObject::close()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("instance::SharedObject::close()");
//##protect##"instance::SharedObject::close()"
    }
    void SharedObject::connect(const Value& result, Instances::fl_net::NetConnection* myConnection, const ASString& params)
    {
//##protect##"instance::SharedObject::connect()"
        SF_UNUSED3(result, myConnection, params);
        WARN_NOT_IMPLEMENTED("instance::SharedObject::connect()");
//##protect##"instance::SharedObject::connect()"
    }
    void SharedObject::flush(ASString& result, SInt32 minDiskSpace)
    {
//##protect##"instance::SharedObject::flush()"

#ifdef GFX_AS3_ENABLE_SHAREDOBJECT
        SF_UNUSED1(minDiskSpace);

        ASVM& vm = static_cast<ASVM&>(GetVM());
        MovieRoot* proot = vm.GetMovieRoot();
        SF_ASSERT(proot);
        MovieImpl* pmovie = proot->GetMovieImpl();

        bool ok = false;
        Ptr<SharedObjectManagerBase> psoMgr = pmovie->GetSharedObjectManager();
        if (psoMgr)
        {
            Ptr<SharedObjectVisitor> pwriter = *psoMgr->CreateWriter(Name, 
                LocalPath, pmovie->GetFileOpener());
            ok = FlushImpl(pwriter);
            if (!ok)
                vm.ThrowError(VM::Error(VM::eFileWriteError, vm SF_DEBUG_ARG("Unable to flush shared object data!")));
            else
            {
                result = vm.GetStringManager().CreateConstString("flushed");
            }
        }
        else
        {
            vm.ThrowError(VM::Error(VM::eFileWriteError, vm SF_DEBUG_ARG("SharedObjectManager state is not installed!")));
        }
#else
        SF_UNUSED2(result, minDiskSpace);
        WARN_NOT_IMPLEMENTED("instance::SharedObject::flush()");
#endif

//##protect##"instance::SharedObject::flush()"
    }
    void SharedObject::send(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::SharedObject::send()"
        SF_UNUSED3(result, argc, argv);
        WARN_NOT_IMPLEMENTED("instance::SharedObject::send()");
//##protect##"instance::SharedObject::send()"
    }
    void SharedObject::setDirty(const Value& result, const ASString& propertyName)
    {
//##protect##"instance::SharedObject::setDirty()"
        SF_UNUSED2(result, propertyName);
        WARN_NOT_IMPLEMENTED("instance::SharedObject::setDirty()");
//##protect##"instance::SharedObject::setDirty()"
    }
    void SharedObject::setProperty(const Value& result, const ASString& propertyName, const Value& value)
    {
//##protect##"instance::SharedObject::setProperty()"
        SF_UNUSED3(result, propertyName, value);
        WARN_NOT_IMPLEMENTED("instance::SharedObject::setProperty()");
//##protect##"instance::SharedObject::setProperty()"
    }

//##protect##"instance$methods"

    bool    SharedObject::SetNameAndLocalPath(const ASString& name, const ASString& localPath)
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
        Name = name.ToCStr();
        LocalPath = localPath.ToCStr();
        return true;
    }

#ifdef GFX_AS3_ENABLE_SHAREDOBJECT
    bool SharedObject::FlushImpl(SharedObjectVisitor* writer)
    {
        if (!writer)
            return false;

        // Visit all members and notify writer
        bool hasData = DataObj.GetPtr() != NULL;
        SF_ASSERT(hasData);
        SF_UNUSED(hasData);
        Instances::fl::Object* pdataObj = DataObj.GetPtr();
        SF_ASSERT(pdataObj);

        struct DataWriter
        {
            // Hash is required to avoid infinite loops when objects have
            // circular references. If an object has been visited we do 
            // not visit it again.
            HashIdentity<Instances::fl::Object*, Instances::fl::Object*>  VisitedObjects;
            VM                      *pVM;
            SharedObjectVisitor     *pWriter;

            DataWriter(VM *pvm, SharedObjectVisitor* pwriter) : pVM(pvm), pWriter(pwriter) {}

            void    VisitMembers(Instances::fl::Object *pdataObj)
            {
                bool isArray = (pdataObj->GetTraitsType() == Traits_Array && 
                    pdataObj->GetTraits().IsInstanceTraits());

                if (!isArray)
                {
                    // We visit only dynamic attributes
                    const AS3::Object::DynAttrsType* dynAttrs = pdataObj->GetDynamicAttrs();
                    if (dynAttrs)
                    {
                        AS3::Object::DynAttrsType::ConstIterator it = dynAttrs->Begin();
                        for (; !it.IsEnd(); ++it)
                        {
                            // Include "DontEnum" properties.
                            const ASString& name = it->First.GetName();
                            const AS3::Value& asval = it->Second;

                            Visit(String(name.ToCStr(), name.GetLength()), asval);
                        }
                    }
                }
                else
                {
                    Instances::fl::Array* arr  = static_cast<Instances::fl::Array*>(pdataObj);
                    unsigned sz = (unsigned)arr->GetSize();
                    for (unsigned i=0; i < sz; i++)
                    {
                        const AS3::Value& asval = arr->At(i);
                        String name;
                        Format(name, "{0}", i);

                        Visit(name, asval);
                    }
                }
            }

            void Visit(const String& name, const Value& val)
            {
                switch (val.GetKind())
                {
                case Value::kNumber:
                    {
                        // writeout number                    
                        DoubleFormatter f(val.AsNumber());
                        f.Convert();
                        String sval(f, f.GetSize());

                        pWriter->AddProperty( name, sval, 
                            GFx::Value::VT_Number);
                        break;
                    }
                case Value::kInt:
                    {
                        // writeout int
                        LongFormatter f(val.AsInt());
                        f.Convert();
                        String sval(f, f.GetSize());

                        pWriter->AddProperty( name, sval, 
                            GFx::Value::VT_Int);
                        break;
                    }
                case Value::kUInt:
                    {
                        // writeout int
                        LongFormatter f(val.AsInt());
                        f.Convert();
                        String sval(f, f.GetSize());

                        pWriter->AddProperty( name, sval, 
                            GFx::Value::VT_Int);
                        break;
                    }
                case Value::kString:
                    {
                        // writeout string
                        String sval(val.AsString().ToCStr());
                        pWriter->AddProperty( name, sval, 
                            GFx::Value::VT_String);
                        break;
                    }
                case Value::kUndefined:
                    {
                        // writeout undefined
                        pWriter->AddProperty( name, "",
                            GFx::Value::VT_Undefined);
                        break;
                    }
                case Value::kBoolean:
                    {
                        // writeout boolean                    
                        pWriter->AddProperty( name, 
                            val.AsBool() ? "true" : "false", GFx::Value::VT_Boolean);
                        break;
                    }
                case Value::kObject:
                    {
                        if (val.IsNull())
                        {
                            // writeout null
                            pWriter->AddProperty( name, "", GFx::Value::VT_Null);
                            break;
                        }

                        // special case for Array
                        Instances::fl::Object* pobj = (Instances::fl::Object*)val.GetObject();
                        if (VisitedObjects.Get(pobj)) break;
                        VisitedObjects.Add(pobj, pobj);

                        if (pobj->GetTraitsType() == Traits_Array && 
                            pobj->GetTraits().IsInstanceTraits())
                        {
                            // writeout array
                            pWriter->PushArray( name );
                            VisitMembers(pobj);
                            pWriter->PopArray();
                        }
                        else
                        {
                            // writeout object
                            pWriter->PushObject( name );
                            VisitMembers(pobj);
                            pWriter->PopObject();
                        }
                        break;
                    }
                default:;
                }
            }
        } visitor(&GetVM(), writer);

        writer->Begin();
        visitor.VisitMembers(pdataObj);
        writer->End();

        return true;
    }
#endif // GFX_AS3_ENABLE_SHAREDOBJECT

//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_net
{
    // const UInt16 SharedObject::tito[SharedObject::ThunkInfoNum] = {
    //    0, 1, 3, 4, 6, 8, 9, 11, 12, 13, 14, 17, 19, 20, 22, 
    // };
    const TypeInfo* SharedObject::tit[25] = {
        &AS3::fl::ObjectTI, 
        NULL, &AS3::fl::ObjectTI, 
        &AS3::fl::ObjectTI, 
        NULL, &AS3::fl::uintTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::uintTI, 
        NULL, &AS3::fl::uintTI, 
        &AS3::fl::uintTI, 
        NULL, 
        NULL, 
        NULL, &AS3::fl_net::NetConnectionTI, &AS3::fl::StringTI, 
        &AS3::fl::StringTI, &AS3::fl::int_TI, 
        NULL, 
        NULL, &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, &AS3::fl::ObjectTI, 
    };
    const Abc::ConstValue SharedObject::dva[1] = {
        {Abc::CONSTANT_Null, 0}, 
    };
    const ThunkInfo SharedObject::ti[SharedObject::ThunkInfoNum] = {
        {TFunc_Instances_SharedObject_clientGet::Func, &SharedObject::tit[0], "client", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_SharedObject_clientSet::Func, &SharedObject::tit[1], "client", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_SharedObject_dataGet::Func, &SharedObject::tit[3], "data", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_SharedObject_defaultObjectEncodingSet::Func, &SharedObject::tit[4], "defaultObjectEncoding", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_SharedObject_fpsSet::Func, &SharedObject::tit[6], "fps", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_SharedObject_objectEncodingGet::Func, &SharedObject::tit[8], "objectEncoding", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_SharedObject_objectEncodingSet::Func, &SharedObject::tit[9], "objectEncoding", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_SharedObject_sizeGet::Func, &SharedObject::tit[11], "size", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_SharedObject_clear::Func, &SharedObject::tit[12], "clear", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_SharedObject_close::Func, &SharedObject::tit[13], "close", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_SharedObject_connect::Func, &SharedObject::tit[14], "connect", NULL, Abc::NS_Public, CT_Method, 1, 2, 0, 1, &SharedObject::dva[0]},
        {TFunc_Instances_SharedObject_flush::Func, &SharedObject::tit[17], "flush", NULL, Abc::NS_Public, CT_Method, 0, 1, 0, 0, NULL},
        {TFunc_Instances_SharedObject_send::Func, &SharedObject::tit[19], "send", NULL, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1, 0, NULL},
        {TFunc_Instances_SharedObject_setDirty::Func, &SharedObject::tit[20], "setDirty", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_SharedObject_setProperty::Func, &SharedObject::tit[22], "setProperty", NULL, Abc::NS_Public, CT_Method, 1, 2, 0, 0, NULL},
    };

    SharedObject::SharedObject(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"InstanceTraits::SharedObject::SharedObject()"
//##protect##"InstanceTraits::SharedObject::SharedObject()"

    }

    void SharedObject::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<SharedObject&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl_net
{
    SharedObject::SharedObject(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::SharedObject::SharedObject()"
//##protect##"class_::SharedObject::SharedObject()"
    }
    void SharedObject::getLocal(SPtr<Instances::fl_net::SharedObject>& result, const ASString& name, const ASString& localPath, bool secure)
    {
//##protect##"class_::SharedObject::getLocal()"

#ifdef GFX_AS3_ENABLE_SHAREDOBJECT
        SF_UNUSED1(secure);

        ASString strFullPath = GetStringManager().CreateString(localPath.ToCStr());
        strFullPath += ":";
        strFullPath += name;

        SPtr<Instances::fl_net::SharedObject>* psavedObj = SharedObjects.Get(strFullPath);
        if (psavedObj)
        {
            // Found a stored shared obj; return it
            result = *psavedObj;
            return;
        }

        InstanceTraits::fl_net::SharedObject& itr = static_cast<InstanceTraits::fl_net::SharedObject&>(GetInstanceTraits());
        SPtr<Instances::fl_net::SharedObject> pthis = itr.MakeInstance(itr);

        // If name and directory aren't formatted correctly, fail
        if (!pthis->SetNameAndLocalPath(name, localPath))
        {
            result = NULL;
            return;
        }

        ASVM& vm = static_cast<ASVM&>(GetVM());
        MovieRoot* proot = vm.GetMovieRoot();
        SF_ASSERT(proot);
        MovieImpl* pmovie = proot->GetMovieImpl();

        SPtr<Instances::fl::Object> pdataObj = vm.MakeObject();        
        ASSharedObjectLoader   loader(&vm, pdataObj);

        Ptr<SharedObjectManagerBase> psoMgr = pmovie->GetSharedObjectManager();
        FileOpenerBase*  pfileopener = pmovie->GetFileOpener();
        Scaleform::String strName(name.ToCStr());
        Scaleform::String strLocalPath(localPath.ToCStr());
        if (!psoMgr || !psoMgr->LoadSharedObject(strName, strLocalPath, &loader, pfileopener))
        {
            result = NULL;
            return;
        }
        else
        {
            pthis->SetDataObject(pdataObj);
            result = pthis;
            // Store for future lookup
            SharedObjects.Add(strFullPath, pthis);
        }
#else
        SF_UNUSED4(result, name, localPath, secure);
        WARN_NOT_IMPLEMENTED("class_::SharedObject::getLocal()");
#endif  //GFX_AS3_ENABLE_SHAREDOBJECT

//##protect##"class_::SharedObject::getLocal()"
    }
    void SharedObject::getRemote(SPtr<Instances::fl_net::SharedObject>& result, const ASString& name, const ASString& remotePath, const Value& persistence, bool secure)
    {
//##protect##"class_::SharedObject::getRemote()"
        SF_UNUSED5(result, name, remotePath, persistence, secure);
        WARN_NOT_IMPLEMENTED("class_::SharedObject::getRemote()");
//##protect##"class_::SharedObject::getRemote()"
    }
//##protect##"class_$methods"

    void SharedObject::ForEachChild_GC(Collector* prcc, GcOp op) const
    {
        Class::ForEachChild_GC(prcc, op);
        SOLookupType::ConstIterator it = SharedObjects.Begin();
        while(it != SharedObjects.End())
        {   
            const SPtr<Instances::fl_net::SharedObject>& sobj = it->Second;
            AS3::ForEachChild_GC<Instances::fl_net::SharedObject, Mem_Stat>(prcc, sobj, op SF_DEBUG_ARG(*this));
            ++it;
        }
    }

//##protect##"class_$methods"

}} // namespace Classes

typedef ThunkFunc3<Classes::fl_net::SharedObject, Classes::fl_net::SharedObject::mid_getLocal, SPtr<Instances::fl_net::SharedObject>, const ASString&, const ASString&, bool> TFunc_Classes_SharedObject_getLocal;
typedef ThunkFunc4<Classes::fl_net::SharedObject, Classes::fl_net::SharedObject::mid_getRemote, SPtr<Instances::fl_net::SharedObject>, const ASString&, const ASString&, const Value&, bool> TFunc_Classes_SharedObject_getRemote;

template <> const TFunc_Classes_SharedObject_getLocal::TMethod TFunc_Classes_SharedObject_getLocal::Method = &Classes::fl_net::SharedObject::getLocal;
template <> const TFunc_Classes_SharedObject_getRemote::TMethod TFunc_Classes_SharedObject_getRemote::Method = &Classes::fl_net::SharedObject::getRemote;

namespace ClassTraits { namespace fl_net
{
    // const UInt16 SharedObject::tito[SharedObject::ThunkInfoNum] = {
    //    0, 4, 
    // };
    const TypeInfo* SharedObject::tit[9] = {
        &AS3::fl_net::SharedObjectTI, &AS3::fl::StringTI, &AS3::fl::StringTI, &AS3::fl::BooleanTI, 
        &AS3::fl_net::SharedObjectTI, &AS3::fl::StringTI, &AS3::fl::StringTI, &AS3::fl::ObjectTI, &AS3::fl::BooleanTI, 
    };
    const Abc::ConstValue SharedObject::dva[5] = {
        {Abc::CONSTANT_Null, 0}, {Abc::CONSTANT_False, 0}, 
        {Abc::CONSTANT_Null, 0}, {false}, {Abc::CONSTANT_False, 0}, 
    };
    const ThunkInfo SharedObject::ti[SharedObject::ThunkInfoNum] = {
        {TFunc_Classes_SharedObject_getLocal::Func, &SharedObject::tit[0], "getLocal", NULL, Abc::NS_Public, CT_Method, 1, 3, 0, 2, &SharedObject::dva[0]},
        {TFunc_Classes_SharedObject_getRemote::Func, &SharedObject::tit[4], "getRemote", NULL, Abc::NS_Public, CT_Method, 1, 4, 0, 3, &SharedObject::dva[2]},
    };

    SharedObject::SharedObject(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"ClassTraits::SharedObject::SharedObject()"
//##protect##"ClassTraits::SharedObject::SharedObject()"

    }

    Pickable<Traits> SharedObject::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) SharedObject(vm, AS3::fl_net::SharedObjectCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_net::SharedObjectCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_net
{
    const TypeInfo SharedObjectTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_net::SharedObject::InstanceType),
        ClassTraits::fl_net::SharedObject::ThunkInfoNum,
        0,
        InstanceTraits::fl_net::SharedObject::ThunkInfoNum,
        0,
        "SharedObject", "flash.net", &fl_events::EventDispatcherTI,
        TypeInfo::None
    };

    const ClassInfo SharedObjectCI = {
        &SharedObjectTI,
        ClassTraits::fl_net::SharedObject::MakeClassTraits,
        ClassTraits::fl_net::SharedObject::ti,
        NULL,
        InstanceTraits::fl_net::SharedObject::ti,
        NULL,
    };
}; // namespace fl_net


}}} // namespace Scaleform { namespace GFx { namespace AS3

