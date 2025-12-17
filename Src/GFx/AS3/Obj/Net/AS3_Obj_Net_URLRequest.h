//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Net_URLRequest.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Net_URLRequest_H
#define INC_AS3_Obj_Net_URLRequest_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_net
{
    extern const TypeInfo URLRequestTI;
    extern const ClassInfo URLRequestCI;
} // namespace fl_net
namespace fl
{
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo anyTI;
    extern const ClassInfo anyCI;
    extern const TypeInfo ArrayTI;
    extern const ClassInfo ArrayCI;
} // namespace fl

namespace ClassTraits { namespace fl_net
{
    class URLRequest;
}}

namespace InstanceTraits { namespace fl_net
{
    class URLRequest;
}}

namespace Classes { namespace fl_net
{
    class URLRequest;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_net
{
    class URLRequest : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_net::URLRequest;
        
    public:
        typedef URLRequest SelfType;
        typedef Classes::fl_net::URLRequest ClassType;
        typedef InstanceTraits::fl_net::URLRequest TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_net::URLRequestTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_net::URLRequest"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        URLRequest(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        virtual void    ForEachChild_GC(Collector* prcc, GcOp op) const;
        virtual void    AS3Constructor(unsigned argc, const Value* argv);
    public:
        const ASString& GetUrl() const;

//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_authenticateGet, 
            mid_authenticateSet, 
            mid_cacheResponseGet, 
            mid_cacheResponseSet, 
            mid_contentTypeGet, 
            mid_contentTypeSet, 
            mid_dataGet, 
            mid_dataSet, 
            mid_digestGet, 
            mid_digestSet, 
            mid_followRedirectsGet, 
            mid_followRedirectsSet, 
            mid_manageCookiesGet, 
            mid_manageCookiesSet, 
            mid_methodGet, 
            mid_methodSet, 
            mid_requestHeadersGet, 
            mid_requestHeadersSet, 
            mid_urlGet, 
            mid_urlSet, 
            mid_useCacheGet, 
            mid_useCacheSet, 
            mid_userAgentGet, 
            mid_userAgentSet, 
        };
        void authenticateGet(bool& result);
        void authenticateSet(const Value& result, bool value);
        void cacheResponseGet(bool& result);
        void cacheResponseSet(const Value& result, bool value);
        void contentTypeGet(ASString& result);
        void contentTypeSet(const Value& result, const ASString& value);
        void dataGet(Value& result);
        void dataSet(const Value& result, const Value& value);
        void digestGet(ASString& result);
        void digestSet(const Value& result, const ASString& value);
        void followRedirectsGet(bool& result);
        void followRedirectsSet(const Value& result, bool value);
        void manageCookiesGet(bool& result);
        void manageCookiesSet(const Value& result, bool value);
        void methodGet(ASString& result);
        void methodSet(const Value& result, const ASString& value);
        void requestHeadersGet(SPtr<Instances::fl::Array>& result);
        void requestHeadersSet(const Value& result, Instances::fl::Array* value);
        void urlGet(ASString& result);
        void urlSet(const Value& result, const ASString& value);
        void useCacheGet(bool& result);
        void useCacheSet(const Value& result, bool value);
        void userAgentGet(ASString& result);
        void userAgentSet(const Value& result, const ASString& value);

        // C++ friendly wrappers for AS3 methods.
        bool authenticateGet()
        {
            bool result;
            authenticateGet(result);
            return result;
        }
        void authenticateSet(bool value)
        {
            authenticateSet(Value::GetUndefined(), value);
        }
        bool cacheResponseGet()
        {
            bool result;
            cacheResponseGet(result);
            return result;
        }
        void cacheResponseSet(bool value)
        {
            cacheResponseSet(Value::GetUndefined(), value);
        }
        ASString contentTypeGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            contentTypeGet(result);
            return result;
        }
        void contentTypeSet(const ASString& value)
        {
            contentTypeSet(Value::GetUndefined(), value);
        }
        Value dataGet()
        {
            Value result;
            dataGet(result);
            return result;
        }
        void dataSet(const Value& value)
        {
            dataSet(Value::GetUndefined(), value);
        }
        ASString digestGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            digestGet(result);
            return result;
        }
        void digestSet(const ASString& value)
        {
            digestSet(Value::GetUndefined(), value);
        }
        bool followRedirectsGet()
        {
            bool result;
            followRedirectsGet(result);
            return result;
        }
        void followRedirectsSet(bool value)
        {
            followRedirectsSet(Value::GetUndefined(), value);
        }
        bool manageCookiesGet()
        {
            bool result;
            manageCookiesGet(result);
            return result;
        }
        void manageCookiesSet(bool value)
        {
            manageCookiesSet(Value::GetUndefined(), value);
        }
        ASString methodGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            methodGet(result);
            return result;
        }
        void methodSet(const ASString& value)
        {
            methodSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl::Array> requestHeadersGet();
        void requestHeadersSet(Instances::fl::Array* value)
        {
            requestHeadersSet(Value::GetUndefined(), value);
        }
        ASString urlGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            urlGet(result);
            return result;
        }
        void urlSet(const ASString& value)
        {
            urlSet(Value::GetUndefined(), value);
        }
        bool useCacheGet()
        {
            bool result;
            useCacheGet(result);
            return result;
        }
        void useCacheSet(bool value)
        {
            useCacheSet(Value::GetUndefined(), value);
        }
        ASString userAgentGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            userAgentGet(result);
            return result;
        }
        void userAgentSet(const ASString& value)
        {
            userAgentSet(Value::GetUndefined(), value);
        }

//##protect##"instance$data"
    protected:
        ASString                    Url;
        Value                       DataObj;
        ASString                    Method;
        ASString                    ContentType;
        SPtr<Instances::fl::Array>  Headers;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_net
{
    class URLRequest : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::URLRequest"; }
#endif
    public:
        typedef Instances::fl_net::URLRequest InstanceType;

    public:
        URLRequest(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_net::URLRequest> MakeInstance(URLRequest& t)
        {
            return Pickable<Instances::fl_net::URLRequest>(new(t.Alloc()) Instances::fl_net::URLRequest(t));
        }
        SPtr<Instances::fl_net::URLRequest> MakeInstanceS(URLRequest& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 24 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[36];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_net
{
    class URLRequest : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::URLRequest"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_net::URLRequest InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        URLRequest(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

