//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Net_URLLoader.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Net_URLLoader_H
#define INC_AS3_Obj_Net_URLLoader_H

#include "../Events/AS3_Obj_Events_EventDispatcher.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_net
{
    extern const TypeInfo URLLoaderTI;
    extern const ClassInfo URLLoaderCI;
    extern const TypeInfo URLRequestTI;
    extern const ClassInfo URLRequestCI;
} // namespace fl_net
namespace fl
{
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo FunctionTI;
    extern const ClassInfo FunctionCI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
} // namespace fl

namespace ClassTraits { namespace fl_net
{
    class URLLoader;
}}

namespace InstanceTraits { namespace fl_net
{
    class URLLoader;
}}

namespace Classes { namespace fl_net
{
    class URLLoader;
}}

//##protect##"forward_declaration"
namespace Instances
{
    namespace fl_net
    {
        class URLRequest;
    }
}
//##protect##"forward_declaration"

namespace Instances { namespace fl_net
{
    class URLLoader : public Instances::fl_events::EventDispatcher
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_net::URLLoader;
        
    public:
        typedef URLLoader SelfType;
        typedef Classes::fl_net::URLLoader ClassType;
        typedef InstanceTraits::fl_net::URLLoader TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_net::URLLoaderTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_net::URLLoader"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        URLLoader(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        virtual void    AS3Constructor(unsigned argc, const Value* argv);
    public:
        void ExecuteOpenEvent();
        void ExecuteProgressEvent(UInt32 bytesLoaded, UInt32 totalBytes);
        void ExecuteCompleteEvent();
        void ExecuteErrorEvent(const char* errorStr);
        bool IsLoadingVariables() const;
        bool IsLoadingText() const;
        bool IsLoadingBinary() const;
        void SetVariablesDataString(const char* pdata);
        void SetTextString(const char* pdata);
        void SetBinaryData(const ArrayPOD<UByte>& binaryData);
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_addEventListener, 
            mid_close, 
            mid_load, 
        };
        void addEventListener(const Value& result, const ASString& type, const Value& listener, bool useCapture, SInt32 priority, bool useWeakReference);
        void close(const Value& result);
        void load(const Value& result, Instances::fl_net::URLRequest* request);

        // C++ friendly wrappers for AS3 methods.
        void addEventListener(const ASString& type, const Value& listener, bool useCapture, SInt32 priority, bool useWeakReference)
        {
            addEventListener(Value::GetUndefined(), type, listener, useCapture, priority, useWeakReference);
        }
        void close()
        {
            close(Value::GetUndefined());
        }
        void load(Instances::fl_net::URLRequest* request)
        {
            load(Value::GetUndefined(), request);
        }

    public:
        // AS3 API members.
        UInt32 bytesLoaded;
        UInt32 bytesTotal;
        Value data;
        ASString dataFormat;

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_net
{
    class URLLoader : public fl_events::EventDispatcher
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::URLLoader"; }
#endif
    public:
        typedef Instances::fl_net::URLLoader InstanceType;

    public:
        URLLoader(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_net::URLLoader> MakeInstance(URLLoader& t)
        {
            return Pickable<Instances::fl_net::URLLoader>(new(t.Alloc()) Instances::fl_net::URLLoader(t));
        }
        SPtr<Instances::fl_net::URLLoader> MakeInstanceS(URLLoader& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { MemberInfoNum = 4 };
        static const MemberInfo mi[MemberInfoNum];
        enum { ThunkInfoNum = 3 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[9];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_net
{
    class URLLoader : public fl_events::EventDispatcher
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::URLLoader"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_net::URLLoader InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        URLLoader(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

