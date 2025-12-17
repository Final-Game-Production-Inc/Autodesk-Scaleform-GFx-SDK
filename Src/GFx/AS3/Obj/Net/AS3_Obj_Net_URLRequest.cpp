//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Net_URLRequest.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Net_URLRequest.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../AS3_Obj_Array.h"
#include "../Utils/AS3_Obj_Utils_ByteArray.h"
#include "GFx/AS3/AS3_MovieRoot.h"
#include "AS3_Obj_Net_URLVariables.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_net::URLRequest, Instances::fl_net::URLRequest::mid_authenticateGet, bool> TFunc_Instances_URLRequest_authenticateGet;
typedef ThunkFunc1<Instances::fl_net::URLRequest, Instances::fl_net::URLRequest::mid_authenticateSet, const Value, bool> TFunc_Instances_URLRequest_authenticateSet;
typedef ThunkFunc0<Instances::fl_net::URLRequest, Instances::fl_net::URLRequest::mid_cacheResponseGet, bool> TFunc_Instances_URLRequest_cacheResponseGet;
typedef ThunkFunc1<Instances::fl_net::URLRequest, Instances::fl_net::URLRequest::mid_cacheResponseSet, const Value, bool> TFunc_Instances_URLRequest_cacheResponseSet;
typedef ThunkFunc0<Instances::fl_net::URLRequest, Instances::fl_net::URLRequest::mid_contentTypeGet, ASString> TFunc_Instances_URLRequest_contentTypeGet;
typedef ThunkFunc1<Instances::fl_net::URLRequest, Instances::fl_net::URLRequest::mid_contentTypeSet, const Value, const ASString&> TFunc_Instances_URLRequest_contentTypeSet;
typedef ThunkFunc0<Instances::fl_net::URLRequest, Instances::fl_net::URLRequest::mid_dataGet, Value> TFunc_Instances_URLRequest_dataGet;
typedef ThunkFunc1<Instances::fl_net::URLRequest, Instances::fl_net::URLRequest::mid_dataSet, const Value, const Value&> TFunc_Instances_URLRequest_dataSet;
typedef ThunkFunc0<Instances::fl_net::URLRequest, Instances::fl_net::URLRequest::mid_digestGet, ASString> TFunc_Instances_URLRequest_digestGet;
typedef ThunkFunc1<Instances::fl_net::URLRequest, Instances::fl_net::URLRequest::mid_digestSet, const Value, const ASString&> TFunc_Instances_URLRequest_digestSet;
typedef ThunkFunc0<Instances::fl_net::URLRequest, Instances::fl_net::URLRequest::mid_followRedirectsGet, bool> TFunc_Instances_URLRequest_followRedirectsGet;
typedef ThunkFunc1<Instances::fl_net::URLRequest, Instances::fl_net::URLRequest::mid_followRedirectsSet, const Value, bool> TFunc_Instances_URLRequest_followRedirectsSet;
typedef ThunkFunc0<Instances::fl_net::URLRequest, Instances::fl_net::URLRequest::mid_manageCookiesGet, bool> TFunc_Instances_URLRequest_manageCookiesGet;
typedef ThunkFunc1<Instances::fl_net::URLRequest, Instances::fl_net::URLRequest::mid_manageCookiesSet, const Value, bool> TFunc_Instances_URLRequest_manageCookiesSet;
typedef ThunkFunc0<Instances::fl_net::URLRequest, Instances::fl_net::URLRequest::mid_methodGet, ASString> TFunc_Instances_URLRequest_methodGet;
typedef ThunkFunc1<Instances::fl_net::URLRequest, Instances::fl_net::URLRequest::mid_methodSet, const Value, const ASString&> TFunc_Instances_URLRequest_methodSet;
typedef ThunkFunc0<Instances::fl_net::URLRequest, Instances::fl_net::URLRequest::mid_requestHeadersGet, SPtr<Instances::fl::Array> > TFunc_Instances_URLRequest_requestHeadersGet;
typedef ThunkFunc1<Instances::fl_net::URLRequest, Instances::fl_net::URLRequest::mid_requestHeadersSet, const Value, Instances::fl::Array*> TFunc_Instances_URLRequest_requestHeadersSet;
typedef ThunkFunc0<Instances::fl_net::URLRequest, Instances::fl_net::URLRequest::mid_urlGet, ASString> TFunc_Instances_URLRequest_urlGet;
typedef ThunkFunc1<Instances::fl_net::URLRequest, Instances::fl_net::URLRequest::mid_urlSet, const Value, const ASString&> TFunc_Instances_URLRequest_urlSet;
typedef ThunkFunc0<Instances::fl_net::URLRequest, Instances::fl_net::URLRequest::mid_useCacheGet, bool> TFunc_Instances_URLRequest_useCacheGet;
typedef ThunkFunc1<Instances::fl_net::URLRequest, Instances::fl_net::URLRequest::mid_useCacheSet, const Value, bool> TFunc_Instances_URLRequest_useCacheSet;
typedef ThunkFunc0<Instances::fl_net::URLRequest, Instances::fl_net::URLRequest::mid_userAgentGet, ASString> TFunc_Instances_URLRequest_userAgentGet;
typedef ThunkFunc1<Instances::fl_net::URLRequest, Instances::fl_net::URLRequest::mid_userAgentSet, const Value, const ASString&> TFunc_Instances_URLRequest_userAgentSet;

template <> const TFunc_Instances_URLRequest_authenticateGet::TMethod TFunc_Instances_URLRequest_authenticateGet::Method = &Instances::fl_net::URLRequest::authenticateGet;
template <> const TFunc_Instances_URLRequest_authenticateSet::TMethod TFunc_Instances_URLRequest_authenticateSet::Method = &Instances::fl_net::URLRequest::authenticateSet;
template <> const TFunc_Instances_URLRequest_cacheResponseGet::TMethod TFunc_Instances_URLRequest_cacheResponseGet::Method = &Instances::fl_net::URLRequest::cacheResponseGet;
template <> const TFunc_Instances_URLRequest_cacheResponseSet::TMethod TFunc_Instances_URLRequest_cacheResponseSet::Method = &Instances::fl_net::URLRequest::cacheResponseSet;
template <> const TFunc_Instances_URLRequest_contentTypeGet::TMethod TFunc_Instances_URLRequest_contentTypeGet::Method = &Instances::fl_net::URLRequest::contentTypeGet;
template <> const TFunc_Instances_URLRequest_contentTypeSet::TMethod TFunc_Instances_URLRequest_contentTypeSet::Method = &Instances::fl_net::URLRequest::contentTypeSet;
template <> const TFunc_Instances_URLRequest_dataGet::TMethod TFunc_Instances_URLRequest_dataGet::Method = &Instances::fl_net::URLRequest::dataGet;
template <> const TFunc_Instances_URLRequest_dataSet::TMethod TFunc_Instances_URLRequest_dataSet::Method = &Instances::fl_net::URLRequest::dataSet;
template <> const TFunc_Instances_URLRequest_digestGet::TMethod TFunc_Instances_URLRequest_digestGet::Method = &Instances::fl_net::URLRequest::digestGet;
template <> const TFunc_Instances_URLRequest_digestSet::TMethod TFunc_Instances_URLRequest_digestSet::Method = &Instances::fl_net::URLRequest::digestSet;
template <> const TFunc_Instances_URLRequest_followRedirectsGet::TMethod TFunc_Instances_URLRequest_followRedirectsGet::Method = &Instances::fl_net::URLRequest::followRedirectsGet;
template <> const TFunc_Instances_URLRequest_followRedirectsSet::TMethod TFunc_Instances_URLRequest_followRedirectsSet::Method = &Instances::fl_net::URLRequest::followRedirectsSet;
template <> const TFunc_Instances_URLRequest_manageCookiesGet::TMethod TFunc_Instances_URLRequest_manageCookiesGet::Method = &Instances::fl_net::URLRequest::manageCookiesGet;
template <> const TFunc_Instances_URLRequest_manageCookiesSet::TMethod TFunc_Instances_URLRequest_manageCookiesSet::Method = &Instances::fl_net::URLRequest::manageCookiesSet;
template <> const TFunc_Instances_URLRequest_methodGet::TMethod TFunc_Instances_URLRequest_methodGet::Method = &Instances::fl_net::URLRequest::methodGet;
template <> const TFunc_Instances_URLRequest_methodSet::TMethod TFunc_Instances_URLRequest_methodSet::Method = &Instances::fl_net::URLRequest::methodSet;
template <> const TFunc_Instances_URLRequest_requestHeadersGet::TMethod TFunc_Instances_URLRequest_requestHeadersGet::Method = &Instances::fl_net::URLRequest::requestHeadersGet;
template <> const TFunc_Instances_URLRequest_requestHeadersSet::TMethod TFunc_Instances_URLRequest_requestHeadersSet::Method = &Instances::fl_net::URLRequest::requestHeadersSet;
template <> const TFunc_Instances_URLRequest_urlGet::TMethod TFunc_Instances_URLRequest_urlGet::Method = &Instances::fl_net::URLRequest::urlGet;
template <> const TFunc_Instances_URLRequest_urlSet::TMethod TFunc_Instances_URLRequest_urlSet::Method = &Instances::fl_net::URLRequest::urlSet;
template <> const TFunc_Instances_URLRequest_useCacheGet::TMethod TFunc_Instances_URLRequest_useCacheGet::Method = &Instances::fl_net::URLRequest::useCacheGet;
template <> const TFunc_Instances_URLRequest_useCacheSet::TMethod TFunc_Instances_URLRequest_useCacheSet::Method = &Instances::fl_net::URLRequest::useCacheSet;
template <> const TFunc_Instances_URLRequest_userAgentGet::TMethod TFunc_Instances_URLRequest_userAgentGet::Method = &Instances::fl_net::URLRequest::userAgentGet;
template <> const TFunc_Instances_URLRequest_userAgentSet::TMethod TFunc_Instances_URLRequest_userAgentSet::Method = &Instances::fl_net::URLRequest::userAgentSet;

namespace Instances { namespace fl_net
{
    URLRequest::URLRequest(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
//##protect##"instance::URLRequest::URLRequest()$data"
    , Url(GetVM().GetStringManager().CreateEmptyString())
    , Method(GetVM().GetStringManager().CreateString("GET"))
    , ContentType(GetVM().GetStringManager().CreateEmptyString())
    , Headers(GetVM().MakeArray())
//##protect##"instance::URLRequest::URLRequest()$data"
    {
//##protect##"instance::URLRequest::URLRequest()$code"
//##protect##"instance::URLRequest::URLRequest()$code"
    }

    void URLRequest::authenticateGet(bool& result)
    {
//##protect##"instance::URLRequest::authenticateGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("URLRequest::authenticateGet()");
//##protect##"instance::URLRequest::authenticateGet()"
    }
    void URLRequest::authenticateSet(const Value& result, bool value)
    {
//##protect##"instance::URLRequest::authenticateSet()"
        SF_UNUSED2(result, value);
        WARN_NOT_IMPLEMENTED("URLRequest::authenticateSet()");
//##protect##"instance::URLRequest::authenticateSet()"
    }
    void URLRequest::cacheResponseGet(bool& result)
    {
//##protect##"instance::URLRequest::cacheResponseGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("URLRequest::cacheResponseGet()");
//##protect##"instance::URLRequest::cacheResponseGet()"
    }
    void URLRequest::cacheResponseSet(const Value& result, bool value)
    {
//##protect##"instance::URLRequest::cacheResponseSet()"
        SF_UNUSED2(result, value);
        WARN_NOT_IMPLEMENTED("URLRequest::cacheResponseSet()");
//##protect##"instance::URLRequest::cacheResponseSet()"
    }
    void URLRequest::contentTypeGet(ASString& result)
    {
//##protect##"instance::URLRequest::contentTypeGet()"
        result = ContentType;
//##protect##"instance::URLRequest::contentTypeGet()"
    }
    void URLRequest::contentTypeSet(const Value& result, const ASString& value)
    {
//##protect##"instance::URLRequest::contentTypeSet()"
        SF_UNUSED(result);
        ContentType = value;
//##protect##"instance::URLRequest::contentTypeSet()"
    }
    void URLRequest::dataGet(Value& result)
    {
//##protect##"instance::URLRequest::dataGet()"
        result = DataObj;
//##protect##"instance::URLRequest::dataGet()"
    }
    void URLRequest::dataSet(const Value& result, const Value& value)
    {
//##protect##"instance::URLRequest::dataSet()"
        SF_UNUSED(result);
        DataObj = value;
//##protect##"instance::URLRequest::dataSet()"
    }
    void URLRequest::digestGet(ASString& result)
    {
//##protect##"instance::URLRequest::digestGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("URLRequest::digestGet()");
//##protect##"instance::URLRequest::digestGet()"
    }
    void URLRequest::digestSet(const Value& result, const ASString& value)
    {
//##protect##"instance::URLRequest::digestSet()"
        SF_UNUSED2(result, value);
        WARN_NOT_IMPLEMENTED("URLRequest::digestSet()");
//##protect##"instance::URLRequest::digestSet()"
    }
    void URLRequest::followRedirectsGet(bool& result)
    {
//##protect##"instance::URLRequest::followRedirectsGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("URLRequest::followRedirectsGet()");
//##protect##"instance::URLRequest::followRedirectsGet()"
    }
    void URLRequest::followRedirectsSet(const Value& result, bool value)
    {
//##protect##"instance::URLRequest::followRedirectsSet()"
        SF_UNUSED2(result, value);
        WARN_NOT_IMPLEMENTED("URLRequest::followRedirectsSet()");
//##protect##"instance::URLRequest::followRedirectsSet()"
    }
    void URLRequest::manageCookiesGet(bool& result)
    {
//##protect##"instance::URLRequest::manageCookiesGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("URLRequest::manageCookiesGet()");
//##protect##"instance::URLRequest::manageCookiesGet()"
    }
    void URLRequest::manageCookiesSet(const Value& result, bool value)
    {
//##protect##"instance::URLRequest::manageCookiesSet()"
        SF_UNUSED2(result, value);
        WARN_NOT_IMPLEMENTED("URLRequest::manageCookiesSet()");
//##protect##"instance::URLRequest::manageCookiesSet()"
    }
    void URLRequest::methodGet(ASString& result)
    {
//##protect##"instance::URLRequest::methodGet()"
        result = Method;
//##protect##"instance::URLRequest::methodGet()"
    }
    void URLRequest::methodSet(const Value& result, const ASString& value)
    {
//##protect##"instance::URLRequest::methodSet()"
        SF_UNUSED1(result);
        Method = value;
//##protect##"instance::URLRequest::methodSet()"
    }
    void URLRequest::requestHeadersGet(SPtr<Instances::fl::Array>& result)
    {
//##protect##"instance::URLRequest::requestHeadersGet()"
        result = Headers;
//##protect##"instance::URLRequest::requestHeadersGet()"
    }
    void URLRequest::requestHeadersSet(const Value& result, Instances::fl::Array* value)
    {
//##protect##"instance::URLRequest::requestHeadersSet()"
        SF_UNUSED1(result);
        bool success = true;
        if (value)
        {
            for (UPInt i = 0; i < value->GetSize(); ++i)
            {
                if (!GetVM().IsOfType(value->At(i), "flash.net.URLRequestHeader", GetVM().GetCurrentAppDomain()))
                {
                    success = false;
                    break;
                }
            }
        }

        if (success)
        {
            Headers = value;
        }
//##protect##"instance::URLRequest::requestHeadersSet()"
    }
    void URLRequest::urlGet(ASString& result)
    {
//##protect##"instance::URLRequest::urlGet()"
        result = Url;
//##protect##"instance::URLRequest::urlGet()"
    }
    void URLRequest::urlSet(const Value& result, const ASString& value)
    {
//##protect##"instance::URLRequest::urlSet()"
        SF_UNUSED1(result);
        Url = value;
//##protect##"instance::URLRequest::urlSet()"
    }
    void URLRequest::useCacheGet(bool& result)
    {
//##protect##"instance::URLRequest::useCacheGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("URLRequest::useCacheGet()");
//##protect##"instance::URLRequest::useCacheGet()"
    }
    void URLRequest::useCacheSet(const Value& result, bool value)
    {
//##protect##"instance::URLRequest::useCacheSet()"
        SF_UNUSED2(result, value);
        WARN_NOT_IMPLEMENTED("URLRequest::useCacheSet()");
//##protect##"instance::URLRequest::useCacheSet()"
    }
    void URLRequest::userAgentGet(ASString& result)
    {
//##protect##"instance::URLRequest::userAgentGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("URLRequest::userAgentGet()");
//##protect##"instance::URLRequest::userAgentGet()"
    }
    void URLRequest::userAgentSet(const Value& result, const ASString& value)
    {
//##protect##"instance::URLRequest::userAgentSet()"
        SF_UNUSED2(result, value);
        WARN_NOT_IMPLEMENTED("URLRequest::userAgentSet()");
//##protect##"instance::URLRequest::userAgentSet()"
    }

    SPtr<Instances::fl::Array> URLRequest::requestHeadersGet()
    {
        SPtr<Instances::fl::Array> result;
        requestHeadersGet(result);
        return result;
    }
//##protect##"instance$methods"
    void URLRequest::ForEachChild_GC(Collector* prcc, GcOp op) const
    {
        Object::ForEachChild_GC(prcc, op);
        AS3::ForEachChild_GC<Instances::fl::Array, Mem_Stat>(prcc, Headers, op SF_DEBUG_ARG(*this));
    }

    void URLRequest::AS3Constructor(unsigned argc, const Value* argv)
    {
        if (argc >= 1)
        {
            const Value& v = argv[0];

            // There is nothing to check. It it is a last statement.
            v.Convert2String(Url).DoNotCheck();
        }
    }

    const ASString& URLRequest::GetUrl() const
    {
        return Url;
    }

//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_net
{
    // const UInt16 URLRequest::tito[URLRequest::ThunkInfoNum] = {
    //    0, 1, 3, 4, 6, 7, 9, 10, 12, 13, 15, 16, 18, 19, 21, 22, 24, 25, 27, 28, 30, 31, 33, 34, 
    // };
    const TypeInfo* URLRequest::tit[36] = {
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        NULL, 
        NULL, NULL, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl::ArrayTI, 
        NULL, &AS3::fl::ArrayTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
    };
    const ThunkInfo URLRequest::ti[URLRequest::ThunkInfoNum] = {
        {TFunc_Instances_URLRequest_authenticateGet::Func, &URLRequest::tit[0], "authenticate", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_URLRequest_authenticateSet::Func, &URLRequest::tit[1], "authenticate", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_URLRequest_cacheResponseGet::Func, &URLRequest::tit[3], "cacheResponse", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_URLRequest_cacheResponseSet::Func, &URLRequest::tit[4], "cacheResponse", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_URLRequest_contentTypeGet::Func, &URLRequest::tit[6], "contentType", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_URLRequest_contentTypeSet::Func, &URLRequest::tit[7], "contentType", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_URLRequest_dataGet::Func, &URLRequest::tit[9], "data", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_URLRequest_dataSet::Func, &URLRequest::tit[10], "data", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_URLRequest_digestGet::Func, &URLRequest::tit[12], "digest", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_URLRequest_digestSet::Func, &URLRequest::tit[13], "digest", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_URLRequest_followRedirectsGet::Func, &URLRequest::tit[15], "followRedirects", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_URLRequest_followRedirectsSet::Func, &URLRequest::tit[16], "followRedirects", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_URLRequest_manageCookiesGet::Func, &URLRequest::tit[18], "manageCookies", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_URLRequest_manageCookiesSet::Func, &URLRequest::tit[19], "manageCookies", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_URLRequest_methodGet::Func, &URLRequest::tit[21], "method", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_URLRequest_methodSet::Func, &URLRequest::tit[22], "method", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_URLRequest_requestHeadersGet::Func, &URLRequest::tit[24], "requestHeaders", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_URLRequest_requestHeadersSet::Func, &URLRequest::tit[25], "requestHeaders", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_URLRequest_urlGet::Func, &URLRequest::tit[27], "url", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_URLRequest_urlSet::Func, &URLRequest::tit[28], "url", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_URLRequest_useCacheGet::Func, &URLRequest::tit[30], "useCache", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_URLRequest_useCacheSet::Func, &URLRequest::tit[31], "useCache", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_URLRequest_userAgentGet::Func, &URLRequest::tit[33], "userAgent", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_URLRequest_userAgentSet::Func, &URLRequest::tit[34], "userAgent", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
    };

    URLRequest::URLRequest(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::URLRequest::URLRequest()"
//##protect##"InstanceTraits::URLRequest::URLRequest()"

    }

    void URLRequest::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<URLRequest&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_net
{

    URLRequest::URLRequest(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::URLRequest::URLRequest()"
//##protect##"ClassTraits::URLRequest::URLRequest()"

    }

    Pickable<Traits> URLRequest::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) URLRequest(vm, AS3::fl_net::URLRequestCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_net::URLRequestCI));
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
    const TypeInfo URLRequestTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_net::URLRequest::InstanceType),
        0,
        0,
        InstanceTraits::fl_net::URLRequest::ThunkInfoNum,
        0,
        "URLRequest", "flash.net", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo URLRequestCI = {
        &URLRequestTI,
        ClassTraits::fl_net::URLRequest::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_net::URLRequest::ti,
        NULL,
    };
}; // namespace fl_net


}}} // namespace Scaleform { namespace GFx { namespace AS3

