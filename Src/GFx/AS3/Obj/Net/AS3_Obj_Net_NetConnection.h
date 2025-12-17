//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Net_NetConnection.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Net_NetConnection_H
#define INC_AS3_Obj_Net_NetConnection_H

#include "../Events/AS3_Obj_Events_EventDispatcher.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_net
{
    extern const TypeInfo NetConnectionTI;
    extern const ClassInfo NetConnectionCI;
    extern const TypeInfo ResponderTI;
    extern const ClassInfo ResponderCI;
} // namespace fl_net
namespace fl
{
    extern const TypeInfo ObjectTI;
    extern const ClassInfo ObjectCI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo uintTI;
    extern const ClassInfo uintCI;
} // namespace fl

namespace ClassTraits { namespace fl_net
{
    class NetConnection;
}}

namespace InstanceTraits { namespace fl_net
{
    class NetConnection;
}}

namespace Classes { namespace fl_net
{
    class NetConnection;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_net
{
    class NetConnection : public Instances::fl_events::EventDispatcher
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_net::NetConnection;
        
    public:
        typedef NetConnection SelfType;
        typedef Classes::fl_net::NetConnection ClassType;
        typedef InstanceTraits::fl_net::NetConnection TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_net::NetConnectionTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_net::NetConnection"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        NetConnection(InstanceTraits::Traits& t);

//##protect##"instance$methods"
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_clientGet, 
            mid_clientSet, 
            mid_connectedGet, 
            mid_connectedProxyTypeGet, 
            mid_defaultObjectEncodingSet, 
            mid_objectEncodingGet, 
            mid_objectEncodingSet, 
            mid_proxyTypeGet, 
            mid_proxyTypeSet, 
            mid_uriGet, 
            mid_usingTLSGet, 
            mid_addHeader, 
            mid_call, 
            mid_close, 
            mid_connect, 
        };
        void clientGet(SPtr<Instances::fl::Object>& result);
        void clientSet(const Value& result, const Value& value);
        void connectedGet(bool& result);
        void connectedProxyTypeGet(ASString& result);
        void defaultObjectEncodingSet(const Value& result, UInt32 value);
        void objectEncodingGet(UInt32& result);
        void objectEncodingSet(const Value& result, UInt32 value);
        void proxyTypeGet(ASString& result);
        void proxyTypeSet(const Value& result, const ASString& value);
        void uriGet(ASString& result);
        void usingTLSGet(bool& result);
        void addHeader(const Value& result, const ASString& operation, bool mustUnderstand, const Value& param);
        void call(Value& result, unsigned argc, const Value* const argv);
        void close(const Value& result);
        void connect(Value& result, unsigned argc, const Value* const argv);

        // C++ friendly wrappers for AS3 methods.
        SPtr<Instances::fl::Object> clientGet()
        {
            SPtr<Instances::fl::Object> result;
            clientGet(result);
            return result;
        }
        void clientSet(const Value& value)
        {
            clientSet(Value::GetUndefined(), value);
        }
        bool connectedGet()
        {
            bool result;
            connectedGet(result);
            return result;
        }
        ASString connectedProxyTypeGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            connectedProxyTypeGet(result);
            return result;
        }
        void defaultObjectEncodingSet(UInt32 value)
        {
            defaultObjectEncodingSet(Value::GetUndefined(), value);
        }
        UInt32 objectEncodingGet()
        {
            UInt32 result;
            objectEncodingGet(result);
            return result;
        }
        void objectEncodingSet(UInt32 value)
        {
            objectEncodingSet(Value::GetUndefined(), value);
        }
        ASString proxyTypeGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            proxyTypeGet(result);
            return result;
        }
        void proxyTypeSet(const ASString& value)
        {
            proxyTypeSet(Value::GetUndefined(), value);
        }
        ASString uriGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            uriGet(result);
            return result;
        }
        bool usingTLSGet()
        {
            bool result;
            usingTLSGet(result);
            return result;
        }
        void addHeader(const ASString& operation, bool mustUnderstand, const Value& param)
        {
            addHeader(Value::GetUndefined(), operation, mustUnderstand, param);
        }
        void close()
        {
            close(Value::GetUndefined());
        }

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_net
{
    class NetConnection : public fl_events::EventDispatcher
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::NetConnection"; }
#endif
    public:
        typedef Instances::fl_net::NetConnection InstanceType;

    public:
        NetConnection(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_net::NetConnection> MakeInstance(NetConnection& t)
        {
            return Pickable<Instances::fl_net::NetConnection>(new(t.Alloc()) Instances::fl_net::NetConnection(t));
        }
        SPtr<Instances::fl_net::NetConnection> MakeInstanceS(NetConnection& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 15 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[25];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_net
{
    class NetConnection : public fl_events::EventDispatcher
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::NetConnection"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_net::NetConnection InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        NetConnection(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

