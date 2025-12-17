//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Net_NetConnection.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Net_NetConnection.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

#ifndef SF_AS3_EMIT_DEF_ARGS
// Values of default arguments.
namespace Impl
{

    template <>
    SF_INLINE
    Value GetMethodDefArg<Instances::fl_net::NetConnection, Instances::fl_net::NetConnection::mid_addHeader, 2, const Value&>(AS3::StringManager&)
    {
        return Value::GetNull();
    }

} // namespace Impl
#endif // SF_AS3_EMIT_DEF_ARGS

typedef ThunkFunc0<Instances::fl_net::NetConnection, Instances::fl_net::NetConnection::mid_clientGet, SPtr<Instances::fl::Object> > TFunc_Instances_NetConnection_clientGet;
typedef ThunkFunc1<Instances::fl_net::NetConnection, Instances::fl_net::NetConnection::mid_clientSet, const Value, const Value&> TFunc_Instances_NetConnection_clientSet;
typedef ThunkFunc0<Instances::fl_net::NetConnection, Instances::fl_net::NetConnection::mid_connectedGet, bool> TFunc_Instances_NetConnection_connectedGet;
typedef ThunkFunc0<Instances::fl_net::NetConnection, Instances::fl_net::NetConnection::mid_connectedProxyTypeGet, ASString> TFunc_Instances_NetConnection_connectedProxyTypeGet;
typedef ThunkFunc1<Instances::fl_net::NetConnection, Instances::fl_net::NetConnection::mid_defaultObjectEncodingSet, const Value, UInt32> TFunc_Instances_NetConnection_defaultObjectEncodingSet;
typedef ThunkFunc0<Instances::fl_net::NetConnection, Instances::fl_net::NetConnection::mid_objectEncodingGet, UInt32> TFunc_Instances_NetConnection_objectEncodingGet;
typedef ThunkFunc1<Instances::fl_net::NetConnection, Instances::fl_net::NetConnection::mid_objectEncodingSet, const Value, UInt32> TFunc_Instances_NetConnection_objectEncodingSet;
typedef ThunkFunc0<Instances::fl_net::NetConnection, Instances::fl_net::NetConnection::mid_proxyTypeGet, ASString> TFunc_Instances_NetConnection_proxyTypeGet;
typedef ThunkFunc1<Instances::fl_net::NetConnection, Instances::fl_net::NetConnection::mid_proxyTypeSet, const Value, const ASString&> TFunc_Instances_NetConnection_proxyTypeSet;
typedef ThunkFunc0<Instances::fl_net::NetConnection, Instances::fl_net::NetConnection::mid_uriGet, ASString> TFunc_Instances_NetConnection_uriGet;
typedef ThunkFunc0<Instances::fl_net::NetConnection, Instances::fl_net::NetConnection::mid_usingTLSGet, bool> TFunc_Instances_NetConnection_usingTLSGet;
typedef ThunkFunc3<Instances::fl_net::NetConnection, Instances::fl_net::NetConnection::mid_addHeader, const Value, const ASString&, bool, const Value&> TFunc_Instances_NetConnection_addHeader;
typedef ThunkFunc2<Instances::fl_net::NetConnection, Instances::fl_net::NetConnection::mid_call, Value, unsigned, const Value*> TFunc_Instances_NetConnection_call;
typedef ThunkFunc0<Instances::fl_net::NetConnection, Instances::fl_net::NetConnection::mid_close, const Value> TFunc_Instances_NetConnection_close;
typedef ThunkFunc2<Instances::fl_net::NetConnection, Instances::fl_net::NetConnection::mid_connect, Value, unsigned, const Value*> TFunc_Instances_NetConnection_connect;

template <> const TFunc_Instances_NetConnection_clientGet::TMethod TFunc_Instances_NetConnection_clientGet::Method = &Instances::fl_net::NetConnection::clientGet;
template <> const TFunc_Instances_NetConnection_clientSet::TMethod TFunc_Instances_NetConnection_clientSet::Method = &Instances::fl_net::NetConnection::clientSet;
template <> const TFunc_Instances_NetConnection_connectedGet::TMethod TFunc_Instances_NetConnection_connectedGet::Method = &Instances::fl_net::NetConnection::connectedGet;
template <> const TFunc_Instances_NetConnection_connectedProxyTypeGet::TMethod TFunc_Instances_NetConnection_connectedProxyTypeGet::Method = &Instances::fl_net::NetConnection::connectedProxyTypeGet;
template <> const TFunc_Instances_NetConnection_defaultObjectEncodingSet::TMethod TFunc_Instances_NetConnection_defaultObjectEncodingSet::Method = &Instances::fl_net::NetConnection::defaultObjectEncodingSet;
template <> const TFunc_Instances_NetConnection_objectEncodingGet::TMethod TFunc_Instances_NetConnection_objectEncodingGet::Method = &Instances::fl_net::NetConnection::objectEncodingGet;
template <> const TFunc_Instances_NetConnection_objectEncodingSet::TMethod TFunc_Instances_NetConnection_objectEncodingSet::Method = &Instances::fl_net::NetConnection::objectEncodingSet;
template <> const TFunc_Instances_NetConnection_proxyTypeGet::TMethod TFunc_Instances_NetConnection_proxyTypeGet::Method = &Instances::fl_net::NetConnection::proxyTypeGet;
template <> const TFunc_Instances_NetConnection_proxyTypeSet::TMethod TFunc_Instances_NetConnection_proxyTypeSet::Method = &Instances::fl_net::NetConnection::proxyTypeSet;
template <> const TFunc_Instances_NetConnection_uriGet::TMethod TFunc_Instances_NetConnection_uriGet::Method = &Instances::fl_net::NetConnection::uriGet;
template <> const TFunc_Instances_NetConnection_usingTLSGet::TMethod TFunc_Instances_NetConnection_usingTLSGet::Method = &Instances::fl_net::NetConnection::usingTLSGet;
template <> const TFunc_Instances_NetConnection_addHeader::TMethod TFunc_Instances_NetConnection_addHeader::Method = &Instances::fl_net::NetConnection::addHeader;
template <> const TFunc_Instances_NetConnection_call::TMethod TFunc_Instances_NetConnection_call::Method = &Instances::fl_net::NetConnection::call;
template <> const TFunc_Instances_NetConnection_close::TMethod TFunc_Instances_NetConnection_close::Method = &Instances::fl_net::NetConnection::close;
template <> const TFunc_Instances_NetConnection_connect::TMethod TFunc_Instances_NetConnection_connect::Method = &Instances::fl_net::NetConnection::connect;

namespace Instances { namespace fl_net
{
    NetConnection::NetConnection(InstanceTraits::Traits& t)
    : Instances::fl_events::EventDispatcher(t)
//##protect##"instance::NetConnection::NetConnection()$data"
//##protect##"instance::NetConnection::NetConnection()$data"
    {
//##protect##"instance::NetConnection::NetConnection()$code"
//##protect##"instance::NetConnection::NetConnection()$code"
    }

    void NetConnection::clientGet(SPtr<Instances::fl::Object>& result)
    {
//##protect##"instance::NetConnection::clientGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("instance::NetConnection::clientGet()");
//##protect##"instance::NetConnection::clientGet()"
    }
    void NetConnection::clientSet(const Value& result, const Value& value)
    {
//##protect##"instance::NetConnection::clientSet()"
        SF_UNUSED2(result, value);
        WARN_NOT_IMPLEMENTED("instance::NetConnection::clientSet()");
//##protect##"instance::NetConnection::clientSet()"
    }
    void NetConnection::connectedGet(bool& result)
    {
//##protect##"instance::NetConnection::connectedGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("instance::NetConnection::connectedGet()");
//##protect##"instance::NetConnection::connectedGet()"
    }
    void NetConnection::connectedProxyTypeGet(ASString& result)
    {
//##protect##"instance::NetConnection::connectedProxyTypeGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("instance::NetConnection::connectedProxyTypeGet()");
//##protect##"instance::NetConnection::connectedProxyTypeGet()"
    }
    void NetConnection::defaultObjectEncodingSet(const Value& result, UInt32 value)
    {
//##protect##"instance::NetConnection::defaultObjectEncodingSet()"
        SF_UNUSED2(result, value);
        WARN_NOT_IMPLEMENTED("instance::NetConnection::defaultObjectEncodingSet()");
//##protect##"instance::NetConnection::defaultObjectEncodingSet()"
    }
    void NetConnection::objectEncodingGet(UInt32& result)
    {
//##protect##"instance::NetConnection::objectEncodingGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("instance::NetConnection::objectEncodingGet()");
//##protect##"instance::NetConnection::objectEncodingGet()"
    }
    void NetConnection::objectEncodingSet(const Value& result, UInt32 value)
    {
//##protect##"instance::NetConnection::objectEncodingSet()"
        SF_UNUSED2(result, value);
        WARN_NOT_IMPLEMENTED("instance::NetConnection::objectEncodingSet()");
//##protect##"instance::NetConnection::objectEncodingSet()"
    }
    void NetConnection::proxyTypeGet(ASString& result)
    {
//##protect##"instance::NetConnection::proxyTypeGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("instance::NetConnection::proxyTypeGet()");
//##protect##"instance::NetConnection::proxyTypeGet()"
    }
    void NetConnection::proxyTypeSet(const Value& result, const ASString& value)
    {
//##protect##"instance::NetConnection::proxyTypeSet()"
        SF_UNUSED2(result, value);
        WARN_NOT_IMPLEMENTED("instance::NetConnection::proxyTypeSet()");
//##protect##"instance::NetConnection::proxyTypeSet()"
    }
    void NetConnection::uriGet(ASString& result)
    {
//##protect##"instance::NetConnection::uriGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("instance::NetConnection::uriGet()");
//##protect##"instance::NetConnection::uriGet()"
    }
    void NetConnection::usingTLSGet(bool& result)
    {
//##protect##"instance::NetConnection::usingTLSGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("instance::NetConnection::usingTLSGet()");
//##protect##"instance::NetConnection::usingTLSGet()"
    }
    void NetConnection::addHeader(const Value& result, const ASString& operation, bool mustUnderstand, const Value& param)
    {
//##protect##"instance::NetConnection::addHeader()"
        SF_UNUSED4(result, operation, mustUnderstand, param);
        WARN_NOT_IMPLEMENTED("instance::NetConnection::addHeader()");
//##protect##"instance::NetConnection::addHeader()"
    }
    void NetConnection::call(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::NetConnection::call()"
        SF_UNUSED3(result, argc, argv);
        WARN_NOT_IMPLEMENTED("instance::NetConnection::call()");
//##protect##"instance::NetConnection::call()"
    }
    void NetConnection::close(const Value& result)
    {
//##protect##"instance::NetConnection::close()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("instance::NetConnection::close()");
//##protect##"instance::NetConnection::close()"
    }
    void NetConnection::connect(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::NetConnection::connect()"
        SF_UNUSED3(result, argc, argv);
#ifndef GFX_ENABLE_VIDEO
        WARN_NOT_IMPLEMENTED("instance::NetConnection::connect()");
#endif
//##protect##"instance::NetConnection::connect()"
    }

//##protect##"instance$methods"
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_net
{
    // const UInt16 NetConnection::tito[NetConnection::ThunkInfoNum] = {
    //    0, 1, 3, 4, 5, 7, 8, 10, 11, 13, 14, 15, 19, 22, 23, 
    // };
    const TypeInfo* NetConnection::tit[25] = {
        &AS3::fl::ObjectTI, 
        NULL, &AS3::fl::ObjectTI, 
        &AS3::fl::BooleanTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::uintTI, 
        &AS3::fl::uintTI, 
        NULL, &AS3::fl::uintTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::StringTI, &AS3::fl::BooleanTI, &AS3::fl::ObjectTI, 
        NULL, &AS3::fl::StringTI, &AS3::fl_net::ResponderTI, 
        NULL, 
        NULL, &AS3::fl::StringTI, 
    };
    const ThunkInfo NetConnection::ti[NetConnection::ThunkInfoNum] = {
        {TFunc_Instances_NetConnection_clientGet::Func, &NetConnection::tit[0], "client", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_NetConnection_clientSet::Func, &NetConnection::tit[1], "client", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_NetConnection_connectedGet::Func, &NetConnection::tit[3], "connected", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_NetConnection_connectedProxyTypeGet::Func, &NetConnection::tit[4], "connectedProxyType", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_NetConnection_defaultObjectEncodingSet::Func, &NetConnection::tit[5], "defaultObjectEncoding", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_NetConnection_objectEncodingGet::Func, &NetConnection::tit[7], "objectEncoding", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_NetConnection_objectEncodingSet::Func, &NetConnection::tit[8], "objectEncoding", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_NetConnection_proxyTypeGet::Func, &NetConnection::tit[10], "proxyType", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_NetConnection_proxyTypeSet::Func, &NetConnection::tit[11], "proxyType", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_NetConnection_uriGet::Func, &NetConnection::tit[13], "uri", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_NetConnection_usingTLSGet::Func, &NetConnection::tit[14], "usingTLS", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_NetConnection_addHeader::Func, &NetConnection::tit[15], "addHeader", NULL, Abc::NS_Public, CT_Method, 1, 3, 0, 0, NULL},
        {TFunc_Instances_NetConnection_call::Func, &NetConnection::tit[19], "call", NULL, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1, 0, NULL},
        {TFunc_Instances_NetConnection_close::Func, &NetConnection::tit[22], "close", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_NetConnection_connect::Func, &NetConnection::tit[23], "connect", NULL, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1, 0, NULL},
    };

    NetConnection::NetConnection(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"InstanceTraits::NetConnection::NetConnection()"
//##protect##"InstanceTraits::NetConnection::NetConnection()"

    }

    void NetConnection::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<NetConnection&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_net
{

    NetConnection::NetConnection(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"ClassTraits::NetConnection::NetConnection()"
//##protect##"ClassTraits::NetConnection::NetConnection()"

    }

    Pickable<Traits> NetConnection::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) NetConnection(vm, AS3::fl_net::NetConnectionCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_net::NetConnectionCI));
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
    const TypeInfo NetConnectionTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_net::NetConnection::InstanceType),
        0,
        0,
        InstanceTraits::fl_net::NetConnection::ThunkInfoNum,
        0,
        "NetConnection", "flash.net", &fl_events::EventDispatcherTI,
        TypeInfo::None
    };

    const ClassInfo NetConnectionCI = {
        &NetConnectionTI,
        ClassTraits::fl_net::NetConnection::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_net::NetConnection::ti,
        NULL,
    };
}; // namespace fl_net


}}} // namespace Scaleform { namespace GFx { namespace AS3

