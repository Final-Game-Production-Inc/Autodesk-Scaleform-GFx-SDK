//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_LoaderInfo.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Display_LoaderInfo_H
#define INC_AS3_Obj_Display_LoaderInfo_H

#include "../Events/AS3_Obj_Events_EventDispatcher.h"
//##protect##"includes"
#include "AS3_Obj_Display_Loader.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_display
{
    extern const TypeInfo LoaderInfoTI;
    extern const ClassInfo LoaderInfoCI;
    extern const TypeInfo DisplayObjectTI;
    extern const ClassInfo DisplayObjectCI;
    extern const TypeInfo LoaderTI;
    extern const ClassInfo LoaderCI;
} // namespace fl_display
namespace fl
{
    extern const TypeInfo uintTI;
    extern const ClassInfo uintCI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo ObjectTI;
    extern const ClassInfo ObjectCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
} // namespace fl
namespace fl_system
{
    extern const TypeInfo ApplicationDomainTI;
    extern const ClassInfo ApplicationDomainCI;
} // namespace fl_system
namespace fl_utils
{
    extern const TypeInfo ByteArrayTI;
    extern const ClassInfo ByteArrayCI;
} // namespace fl_utils
namespace fl_events
{
    extern const TypeInfo EventDispatcherTI;
    extern const ClassInfo EventDispatcherCI;
} // namespace fl_events

namespace ClassTraits { namespace fl_display
{
    class LoaderInfo;
}}

namespace InstanceTraits { namespace fl_display
{
    class LoaderInfo;
}}

namespace Classes { namespace fl_display
{
    class LoaderInfo;
}}

//##protect##"forward_declaration"
namespace Instances { 
    namespace fl_system
    {
        class ApplicationDomain;
    }
    namespace fl_utils
    {
        class ByteArray;
    }
    namespace fl_display
    {
        class Loader;
    }
}
//##protect##"forward_declaration"

namespace Instances { namespace fl_display
{
    class LoaderInfo : public Instances::fl_events::EventDispatcher
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_display::LoaderInfo;
        
    public:
        typedef LoaderInfo SelfType;
        typedef Classes::fl_display::LoaderInfo ClassType;
        typedef InstanceTraits::fl_display::LoaderInfo TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_display::LoaderInfoTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_display::LoaderInfo"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        LoaderInfo(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        void ForEachChild_GC(Collector* prcc, GcOp op) const;

    public:
        void ExecuteProgressEvent(UPInt bytesLoaded, UPInt totalBytes);
        void ExecuteCompleteEvent();
        void ExecuteErrorEvent(const char* errorStr);
        void ExecuteOpenEvent();
        void ExecuteInitEvent(GFx::DisplayObject* obj);
        void SetLoader(Loader* ploader) { pLoader = ploader; }
        void ResetContent() { Content = NULL; }
        void SetAppDomain(Instances::fl_system::ApplicationDomain* appDomain);
        void VerifyAppDomain();
        
        GFx::DisplayObject* GetContentDispObj() const { return (Content) ? Content->pDispObj : 0; }
        void SetContent(DisplayObject* c) { Content = c; }
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_actionScriptVersionGet, 
            mid_applicationDomainGet, 
            mid_bytesGet, 
            mid_bytesLoadedGet, 
            mid_bytesTotalGet, 
            mid_childAllowsParentGet, 
            mid_childSandboxBridgeGet, 
            mid_childSandboxBridgeSet, 
            mid_contentGet, 
            mid_contentTypeGet, 
            mid_frameRateGet, 
            mid_heightGet, 
            mid_loaderGet, 
            mid_loaderURLGet, 
            mid_parametersGet, 
            mid_parentAllowsChildGet, 
            mid_parentSandboxBridgeGet, 
            mid_parentSandboxBridgeSet, 
            mid_sameDomainGet, 
            mid_sharedEventsGet, 
            mid_swfVersionGet, 
            mid_urlGet, 
            mid_widthGet, 
        };
        void actionScriptVersionGet(UInt32& result);
        void applicationDomainGet(SPtr<Instances::fl_system::ApplicationDomain>& result);
        void bytesGet(SPtr<Instances::fl_utils::ByteArray>& result);
        void bytesLoadedGet(UInt32& result);
        void bytesTotalGet(UInt32& result);
        void childAllowsParentGet(bool& result);
        void childSandboxBridgeGet(SPtr<Instances::fl::Object>& result);
        void childSandboxBridgeSet(const Value& result, const Value& value);
        void contentGet(SPtr<Instances::fl_display::DisplayObject>& result);
        void contentTypeGet(ASString& result);
        void frameRateGet(Value::Number& result);
        void heightGet(SInt32& result);
        void loaderGet(SPtr<Instances::fl_display::Loader>& result);
        void loaderURLGet(ASString& result);
        void parametersGet(SPtr<Instances::fl::Object>& result);
        void parentAllowsChildGet(bool& result);
        void parentSandboxBridgeGet(SPtr<Instances::fl::Object>& result);
        void parentSandboxBridgeSet(const Value& result, const Value& value);
        void sameDomainGet(bool& result);
        void sharedEventsGet(SPtr<Instances::fl_events::EventDispatcher>& result);
        void swfVersionGet(UInt32& result);
        void urlGet(ASString& result);
        void widthGet(SInt32& result);

        // C++ friendly wrappers for AS3 methods.
        UInt32 actionScriptVersionGet()
        {
            UInt32 result;
            actionScriptVersionGet(result);
            return result;
        }
        SPtr<Instances::fl_system::ApplicationDomain> applicationDomainGet();
        SPtr<Instances::fl_utils::ByteArray> bytesGet();
        UInt32 bytesLoadedGet()
        {
            UInt32 result;
            bytesLoadedGet(result);
            return result;
        }
        UInt32 bytesTotalGet()
        {
            UInt32 result;
            bytesTotalGet(result);
            return result;
        }
        bool childAllowsParentGet()
        {
            bool result;
            childAllowsParentGet(result);
            return result;
        }
        SPtr<Instances::fl::Object> childSandboxBridgeGet()
        {
            SPtr<Instances::fl::Object> result;
            childSandboxBridgeGet(result);
            return result;
        }
        void childSandboxBridgeSet(const Value& value)
        {
            childSandboxBridgeSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_display::DisplayObject> contentGet();
        ASString contentTypeGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            contentTypeGet(result);
            return result;
        }
        Value::Number frameRateGet()
        {
            Value::Number result;
            frameRateGet(result);
            return result;
        }
        SInt32 heightGet()
        {
            SInt32 result;
            heightGet(result);
            return result;
        }
        SPtr<Instances::fl_display::Loader> loaderGet();
        ASString loaderURLGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            loaderURLGet(result);
            return result;
        }
        SPtr<Instances::fl::Object> parametersGet()
        {
            SPtr<Instances::fl::Object> result;
            parametersGet(result);
            return result;
        }
        bool parentAllowsChildGet()
        {
            bool result;
            parentAllowsChildGet(result);
            return result;
        }
        SPtr<Instances::fl::Object> parentSandboxBridgeGet()
        {
            SPtr<Instances::fl::Object> result;
            parentSandboxBridgeGet(result);
            return result;
        }
        void parentSandboxBridgeSet(const Value& value)
        {
            parentSandboxBridgeSet(Value::GetUndefined(), value);
        }
        bool sameDomainGet()
        {
            bool result;
            sameDomainGet(result);
            return result;
        }
        SPtr<Instances::fl_events::EventDispatcher> sharedEventsGet();
        UInt32 swfVersionGet()
        {
            UInt32 result;
            swfVersionGet(result);
            return result;
        }
        ASString urlGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            urlGet(result);
            return result;
        }
        SInt32 widthGet()
        {
            SInt32 result;
            widthGet(result);
            return result;
        }

//##protect##"instance$data"
    protected:
        UInt32 BytesLoaded;
        UInt32 BytesTotal;
        SPtr<DisplayObject> Content;
        SPtr<Loader> pLoader;
        SPtr<VMAppDomain> AppDomain;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{
    class LoaderInfo : public fl_events::EventDispatcher
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::LoaderInfo"; }
#endif
    public:
        typedef Instances::fl_display::LoaderInfo InstanceType;

    public:
        LoaderInfo(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_display::LoaderInfo> MakeInstance(LoaderInfo& t)
        {
            return Pickable<Instances::fl_display::LoaderInfo>(new(t.Alloc()) Instances::fl_display::LoaderInfo(t));
        }
        SPtr<Instances::fl_display::LoaderInfo> MakeInstanceS(LoaderInfo& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 23 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[25];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_display
{
    class LoaderInfo : public fl_events::EventDispatcher
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::LoaderInfo"; }
#endif
    public:
        typedef Classes::fl_display::LoaderInfo ClassType;
        typedef InstanceTraits::fl_display::LoaderInfo InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        LoaderInfo(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { ThunkInfoNum = 1 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[2];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_display
{
    class LoaderInfo : public Class
    {
        friend class ClassTraits::fl_display::LoaderInfo;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_display::LoaderInfoTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::LoaderInfo"; }
#endif
    public:
        typedef LoaderInfo SelfType;
        typedef LoaderInfo ClassType;
        
    private:
        LoaderInfo(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_getLoaderInfoByDefinition, 
        };
        void getLoaderInfoByDefinition(SPtr<Instances::fl_display::LoaderInfo>& result, const Value& object);

        // C++ friendly wrappers for AS3 methods.
        SPtr<Instances::fl_display::LoaderInfo> getLoaderInfoByDefinition(const Value& object)
        {
            SPtr<Instances::fl_display::LoaderInfo> result;
            getLoaderInfoByDefinition(result, object);
            return result;
        }

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

