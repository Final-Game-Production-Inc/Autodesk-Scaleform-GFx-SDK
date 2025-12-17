//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_LoaderInfo.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Display_LoaderInfo.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "AS3_Obj_Display_Loader.h"
#include "GFx/AS3/AS3_MovieRoot.h"
#include "GFx/AS3/AS3_AvmLoader.h"
#include "../Utils/AS3_Obj_Utils_ByteArray.h"
#include "../Events/AS3_Obj_Events_ProgressEvent.h"
#include "../Events/AS3_Obj_Events_IOErrorEvent.h"
#include "../System/AS3_Obj_System_ApplicationDomain.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
namespace Classes
{
    class ApplicationDomain;
    class ByteArray;
}
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_display::LoaderInfo, Instances::fl_display::LoaderInfo::mid_actionScriptVersionGet, UInt32> TFunc_Instances_LoaderInfo_actionScriptVersionGet;
typedef ThunkFunc0<Instances::fl_display::LoaderInfo, Instances::fl_display::LoaderInfo::mid_applicationDomainGet, SPtr<Instances::fl_system::ApplicationDomain> > TFunc_Instances_LoaderInfo_applicationDomainGet;
typedef ThunkFunc0<Instances::fl_display::LoaderInfo, Instances::fl_display::LoaderInfo::mid_bytesGet, SPtr<Instances::fl_utils::ByteArray> > TFunc_Instances_LoaderInfo_bytesGet;
typedef ThunkFunc0<Instances::fl_display::LoaderInfo, Instances::fl_display::LoaderInfo::mid_bytesLoadedGet, UInt32> TFunc_Instances_LoaderInfo_bytesLoadedGet;
typedef ThunkFunc0<Instances::fl_display::LoaderInfo, Instances::fl_display::LoaderInfo::mid_bytesTotalGet, UInt32> TFunc_Instances_LoaderInfo_bytesTotalGet;
typedef ThunkFunc0<Instances::fl_display::LoaderInfo, Instances::fl_display::LoaderInfo::mid_childAllowsParentGet, bool> TFunc_Instances_LoaderInfo_childAllowsParentGet;
typedef ThunkFunc0<Instances::fl_display::LoaderInfo, Instances::fl_display::LoaderInfo::mid_childSandboxBridgeGet, SPtr<Instances::fl::Object> > TFunc_Instances_LoaderInfo_childSandboxBridgeGet;
typedef ThunkFunc1<Instances::fl_display::LoaderInfo, Instances::fl_display::LoaderInfo::mid_childSandboxBridgeSet, const Value, const Value&> TFunc_Instances_LoaderInfo_childSandboxBridgeSet;
typedef ThunkFunc0<Instances::fl_display::LoaderInfo, Instances::fl_display::LoaderInfo::mid_contentGet, SPtr<Instances::fl_display::DisplayObject> > TFunc_Instances_LoaderInfo_contentGet;
typedef ThunkFunc0<Instances::fl_display::LoaderInfo, Instances::fl_display::LoaderInfo::mid_contentTypeGet, ASString> TFunc_Instances_LoaderInfo_contentTypeGet;
typedef ThunkFunc0<Instances::fl_display::LoaderInfo, Instances::fl_display::LoaderInfo::mid_frameRateGet, Value::Number> TFunc_Instances_LoaderInfo_frameRateGet;
typedef ThunkFunc0<Instances::fl_display::LoaderInfo, Instances::fl_display::LoaderInfo::mid_heightGet, SInt32> TFunc_Instances_LoaderInfo_heightGet;
typedef ThunkFunc0<Instances::fl_display::LoaderInfo, Instances::fl_display::LoaderInfo::mid_loaderGet, SPtr<Instances::fl_display::Loader> > TFunc_Instances_LoaderInfo_loaderGet;
typedef ThunkFunc0<Instances::fl_display::LoaderInfo, Instances::fl_display::LoaderInfo::mid_loaderURLGet, ASString> TFunc_Instances_LoaderInfo_loaderURLGet;
typedef ThunkFunc0<Instances::fl_display::LoaderInfo, Instances::fl_display::LoaderInfo::mid_parametersGet, SPtr<Instances::fl::Object> > TFunc_Instances_LoaderInfo_parametersGet;
typedef ThunkFunc0<Instances::fl_display::LoaderInfo, Instances::fl_display::LoaderInfo::mid_parentAllowsChildGet, bool> TFunc_Instances_LoaderInfo_parentAllowsChildGet;
typedef ThunkFunc0<Instances::fl_display::LoaderInfo, Instances::fl_display::LoaderInfo::mid_parentSandboxBridgeGet, SPtr<Instances::fl::Object> > TFunc_Instances_LoaderInfo_parentSandboxBridgeGet;
typedef ThunkFunc1<Instances::fl_display::LoaderInfo, Instances::fl_display::LoaderInfo::mid_parentSandboxBridgeSet, const Value, const Value&> TFunc_Instances_LoaderInfo_parentSandboxBridgeSet;
typedef ThunkFunc0<Instances::fl_display::LoaderInfo, Instances::fl_display::LoaderInfo::mid_sameDomainGet, bool> TFunc_Instances_LoaderInfo_sameDomainGet;
typedef ThunkFunc0<Instances::fl_display::LoaderInfo, Instances::fl_display::LoaderInfo::mid_sharedEventsGet, SPtr<Instances::fl_events::EventDispatcher> > TFunc_Instances_LoaderInfo_sharedEventsGet;
typedef ThunkFunc0<Instances::fl_display::LoaderInfo, Instances::fl_display::LoaderInfo::mid_swfVersionGet, UInt32> TFunc_Instances_LoaderInfo_swfVersionGet;
typedef ThunkFunc0<Instances::fl_display::LoaderInfo, Instances::fl_display::LoaderInfo::mid_urlGet, ASString> TFunc_Instances_LoaderInfo_urlGet;
typedef ThunkFunc0<Instances::fl_display::LoaderInfo, Instances::fl_display::LoaderInfo::mid_widthGet, SInt32> TFunc_Instances_LoaderInfo_widthGet;

template <> const TFunc_Instances_LoaderInfo_actionScriptVersionGet::TMethod TFunc_Instances_LoaderInfo_actionScriptVersionGet::Method = &Instances::fl_display::LoaderInfo::actionScriptVersionGet;
template <> const TFunc_Instances_LoaderInfo_applicationDomainGet::TMethod TFunc_Instances_LoaderInfo_applicationDomainGet::Method = &Instances::fl_display::LoaderInfo::applicationDomainGet;
template <> const TFunc_Instances_LoaderInfo_bytesGet::TMethod TFunc_Instances_LoaderInfo_bytesGet::Method = &Instances::fl_display::LoaderInfo::bytesGet;
template <> const TFunc_Instances_LoaderInfo_bytesLoadedGet::TMethod TFunc_Instances_LoaderInfo_bytesLoadedGet::Method = &Instances::fl_display::LoaderInfo::bytesLoadedGet;
template <> const TFunc_Instances_LoaderInfo_bytesTotalGet::TMethod TFunc_Instances_LoaderInfo_bytesTotalGet::Method = &Instances::fl_display::LoaderInfo::bytesTotalGet;
template <> const TFunc_Instances_LoaderInfo_childAllowsParentGet::TMethod TFunc_Instances_LoaderInfo_childAllowsParentGet::Method = &Instances::fl_display::LoaderInfo::childAllowsParentGet;
template <> const TFunc_Instances_LoaderInfo_childSandboxBridgeGet::TMethod TFunc_Instances_LoaderInfo_childSandboxBridgeGet::Method = &Instances::fl_display::LoaderInfo::childSandboxBridgeGet;
template <> const TFunc_Instances_LoaderInfo_childSandboxBridgeSet::TMethod TFunc_Instances_LoaderInfo_childSandboxBridgeSet::Method = &Instances::fl_display::LoaderInfo::childSandboxBridgeSet;
template <> const TFunc_Instances_LoaderInfo_contentGet::TMethod TFunc_Instances_LoaderInfo_contentGet::Method = &Instances::fl_display::LoaderInfo::contentGet;
template <> const TFunc_Instances_LoaderInfo_contentTypeGet::TMethod TFunc_Instances_LoaderInfo_contentTypeGet::Method = &Instances::fl_display::LoaderInfo::contentTypeGet;
template <> const TFunc_Instances_LoaderInfo_frameRateGet::TMethod TFunc_Instances_LoaderInfo_frameRateGet::Method = &Instances::fl_display::LoaderInfo::frameRateGet;
template <> const TFunc_Instances_LoaderInfo_heightGet::TMethod TFunc_Instances_LoaderInfo_heightGet::Method = &Instances::fl_display::LoaderInfo::heightGet;
template <> const TFunc_Instances_LoaderInfo_loaderGet::TMethod TFunc_Instances_LoaderInfo_loaderGet::Method = &Instances::fl_display::LoaderInfo::loaderGet;
template <> const TFunc_Instances_LoaderInfo_loaderURLGet::TMethod TFunc_Instances_LoaderInfo_loaderURLGet::Method = &Instances::fl_display::LoaderInfo::loaderURLGet;
template <> const TFunc_Instances_LoaderInfo_parametersGet::TMethod TFunc_Instances_LoaderInfo_parametersGet::Method = &Instances::fl_display::LoaderInfo::parametersGet;
template <> const TFunc_Instances_LoaderInfo_parentAllowsChildGet::TMethod TFunc_Instances_LoaderInfo_parentAllowsChildGet::Method = &Instances::fl_display::LoaderInfo::parentAllowsChildGet;
template <> const TFunc_Instances_LoaderInfo_parentSandboxBridgeGet::TMethod TFunc_Instances_LoaderInfo_parentSandboxBridgeGet::Method = &Instances::fl_display::LoaderInfo::parentSandboxBridgeGet;
template <> const TFunc_Instances_LoaderInfo_parentSandboxBridgeSet::TMethod TFunc_Instances_LoaderInfo_parentSandboxBridgeSet::Method = &Instances::fl_display::LoaderInfo::parentSandboxBridgeSet;
template <> const TFunc_Instances_LoaderInfo_sameDomainGet::TMethod TFunc_Instances_LoaderInfo_sameDomainGet::Method = &Instances::fl_display::LoaderInfo::sameDomainGet;
template <> const TFunc_Instances_LoaderInfo_sharedEventsGet::TMethod TFunc_Instances_LoaderInfo_sharedEventsGet::Method = &Instances::fl_display::LoaderInfo::sharedEventsGet;
template <> const TFunc_Instances_LoaderInfo_swfVersionGet::TMethod TFunc_Instances_LoaderInfo_swfVersionGet::Method = &Instances::fl_display::LoaderInfo::swfVersionGet;
template <> const TFunc_Instances_LoaderInfo_urlGet::TMethod TFunc_Instances_LoaderInfo_urlGet::Method = &Instances::fl_display::LoaderInfo::urlGet;
template <> const TFunc_Instances_LoaderInfo_widthGet::TMethod TFunc_Instances_LoaderInfo_widthGet::Method = &Instances::fl_display::LoaderInfo::widthGet;

namespace Instances { namespace fl_display
{
    LoaderInfo::LoaderInfo(InstanceTraits::Traits& t)
    : Instances::fl_events::EventDispatcher(t)
//##protect##"instance::LoaderInfo::LoaderInfo()$data"
    , BytesLoaded(0)
    , BytesTotal(0)
    , AppDomain(&GetVM().GetFrameAppDomain())
//##protect##"instance::LoaderInfo::LoaderInfo()$data"
    {
//##protect##"instance::LoaderInfo::LoaderInfo()$code"
//##protect##"instance::LoaderInfo::LoaderInfo()$code"
    }

    void LoaderInfo::actionScriptVersionGet(UInt32& result)
    {
//##protect##"instance::LoaderInfo::actionScriptVersionGet()"
        SF_UNUSED1(result);
        result = 3; // only as3 is supported for now.
//##protect##"instance::LoaderInfo::actionScriptVersionGet()"
    }
    void LoaderInfo::applicationDomainGet(SPtr<Instances::fl_system::ApplicationDomain>& result)
    {
//##protect##"instance::LoaderInfo::applicationDomainGet()"
        InstanceTraits::fl_system::ApplicationDomain& tr = static_cast<InstanceTraits::fl_system::ApplicationDomain&>(GetVM().GetITraitsApplicationDomain());
        result = tr.MakeInstance(tr);
        SF_ASSERT(AppDomain);
        result->SetAppDomain(*AppDomain);
//##protect##"instance::LoaderInfo::applicationDomainGet()"
    }
    void LoaderInfo::bytesGet(SPtr<Instances::fl_utils::ByteArray>& result)
    {
//##protect##"instance::LoaderInfo::bytesGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("LoaderInfo::bytesGet()");
//##protect##"instance::LoaderInfo::bytesGet()"
    }
    void LoaderInfo::bytesLoadedGet(UInt32& result)
    {
//##protect##"instance::LoaderInfo::bytesLoadedGet()"
        SF_UNUSED1(result);
        if (GetContentDispObj())
            result = GetContentDispObj()->GetResourceMovieDef()->GetBytesLoaded();
        else
            result = (UInt32) BytesLoaded;
//##protect##"instance::LoaderInfo::bytesLoadedGet()"
    }
    void LoaderInfo::bytesTotalGet(UInt32& result)
    {
//##protect##"instance::LoaderInfo::bytesTotalGet()"
        SF_UNUSED1(result);
        if (GetContentDispObj())
            result = GetContentDispObj()->GetResourceMovieDef()->GetFileBytes();
        else
            result = (UInt32) BytesTotal;
//##protect##"instance::LoaderInfo::bytesTotalGet()"
    }
    void LoaderInfo::childAllowsParentGet(bool& result)
    {
//##protect##"instance::LoaderInfo::childAllowsParentGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("LoaderInfo::childAllowsParentGet()");
//##protect##"instance::LoaderInfo::childAllowsParentGet()"
    }
    void LoaderInfo::childSandboxBridgeGet(SPtr<Instances::fl::Object>& result)
    {
//##protect##"instance::LoaderInfo::childSandboxBridgeGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("LoaderInfo::childSandboxBridgeGet()");
//##protect##"instance::LoaderInfo::childSandboxBridgeGet()"
    }
    void LoaderInfo::childSandboxBridgeSet(const Value& result, const Value& value)
    {
//##protect##"instance::LoaderInfo::childSandboxBridgeSet()"
        SF_UNUSED2(result, value);
        WARN_NOT_IMPLEMENTED("LoaderInfo::childSandboxBridgeSet()");
//##protect##"instance::LoaderInfo::childSandboxBridgeSet()"
    }
    void LoaderInfo::contentGet(SPtr<Instances::fl_display::DisplayObject>& result)
    {
//##protect##"instance::LoaderInfo::contentGet()"
        result = Content;
//##protect##"instance::LoaderInfo::contentGet()"
    }
    void LoaderInfo::contentTypeGet(ASString& result)
    {
//##protect##"instance::LoaderInfo::contentTypeGet()"
        SF_UNUSED1(result);
        if (Content)
        {
            MovieDefImpl* def = GetContentDispObj()->GetResourceMovieDef();
            if (def->GetDataDef()->MovieType == MovieDataDef::MT_Flash)
                result = "application/x-shockwave-flash";
            else if (def->GetDataDef()->MovieType == MovieDataDef::MT_Image)
                result = "image";
            else
                result = "unknown";
        }
        else
            result = "unknown"; //?
//##protect##"instance::LoaderInfo::contentTypeGet()"
    }
    void LoaderInfo::frameRateGet(Value::Number& result)
    {
//##protect##"instance::LoaderInfo::frameRateGet()"
        SF_UNUSED1(result);
        if (Content)
        {
            result = (Value::Number)GetContentDispObj()->GetResourceMovieDef()->GetFrameRate();
        }
        else
            result = 0;
//##protect##"instance::LoaderInfo::frameRateGet()"
    }
    void LoaderInfo::heightGet(SInt32& result)
    {
//##protect##"instance::LoaderInfo::heightGet()"
        SF_UNUSED1(result);
        if (Content)
        {
            result = (SInt32)GetContentDispObj()->GetResourceMovieDef()->GetHeight();
        }
        else
            result = 0;
//##protect##"instance::LoaderInfo::heightGet()"
    }
    void LoaderInfo::loaderGet(SPtr<Instances::fl_display::Loader>& result)
    {
//##protect##"instance::LoaderInfo::loaderGet()"
        result = pLoader;
//##protect##"instance::LoaderInfo::loaderGet()"
    }
    void LoaderInfo::loaderURLGet(ASString& result)
    {
//##protect##"instance::LoaderInfo::loaderURLGet()"
        SF_UNUSED1(result);
        if (pLoader)
        {
            result = GetVM().GetStringManager().CreateString(pLoader->GetAvmLoader()->GetResourceMovieDef()->GetFileURL());
        }
        else
            result = GetVM().GetStringManager().CreateEmptyString();
//##protect##"instance::LoaderInfo::loaderURLGet()"
    }
    void LoaderInfo::parametersGet(SPtr<Instances::fl::Object>& result)
    {
//##protect##"instance::LoaderInfo::parametersGet()"
        result = GetVM().MakeObject();
        WARN_NOT_IMPLEMENTED("LoaderInfo::parametersGet()");
//##protect##"instance::LoaderInfo::parametersGet()"
    }
    void LoaderInfo::parentAllowsChildGet(bool& result)
    {
//##protect##"instance::LoaderInfo::parentAllowsChildGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("LoaderInfo::parentAllowsChildGet()");
//##protect##"instance::LoaderInfo::parentAllowsChildGet()"
    }
    void LoaderInfo::parentSandboxBridgeGet(SPtr<Instances::fl::Object>& result)
    {
//##protect##"instance::LoaderInfo::parentSandboxBridgeGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("LoaderInfo::parentSandboxBridgeGet()");
//##protect##"instance::LoaderInfo::parentSandboxBridgeGet()"
    }
    void LoaderInfo::parentSandboxBridgeSet(const Value& result, const Value& value)
    {
//##protect##"instance::LoaderInfo::parentSandboxBridgeSet()"
        SF_UNUSED2(result, value);
        WARN_NOT_IMPLEMENTED("LoaderInfo::parentSandboxBridgeSet()");
//##protect##"instance::LoaderInfo::parentSandboxBridgeSet()"
    }
    void LoaderInfo::sameDomainGet(bool& result)
    {
//##protect##"instance::LoaderInfo::sameDomainGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("LoaderInfo::sameDomainGet()");
//##protect##"instance::LoaderInfo::sameDomainGet()"
    }
    void LoaderInfo::sharedEventsGet(SPtr<Instances::fl_events::EventDispatcher>& result)
    {
//##protect##"instance::LoaderInfo::sharedEventsGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("LoaderInfo::sharedEventsGet()");
//##protect##"instance::LoaderInfo::sharedEventsGet()"
    }
    void LoaderInfo::swfVersionGet(UInt32& result)
    {
//##protect##"instance::LoaderInfo::swfVersionGet()"
        SF_UNUSED1(result);
        if (Content)
        {
            result = (UInt32)GetContentDispObj()->GetResourceMovieDef()->GetVersion();
        }
        else
            result = 0;
//##protect##"instance::LoaderInfo::swfVersionGet()"
    }
    void LoaderInfo::urlGet(ASString& result)
    {
//##protect##"instance::LoaderInfo::urlGet()"
        SF_UNUSED1(result);
        if (Content)
        {
            result = GetVM().GetStringManager().CreateString(
                GetContentDispObj()->GetResourceMovieDef()->GetFileURL());
        }
        else
            result = GetVM().GetStringManager().CreateEmptyString();
//##protect##"instance::LoaderInfo::urlGet()"
    }
    void LoaderInfo::widthGet(SInt32& result)
    {
//##protect##"instance::LoaderInfo::widthGet()"
        SF_UNUSED1(result);
        if (Content)
        {
            result = (SInt32)GetContentDispObj()->GetResourceMovieDef()->GetWidth();
        }
        else
            result = 0;
//##protect##"instance::LoaderInfo::widthGet()"
    }

    SPtr<Instances::fl_system::ApplicationDomain> LoaderInfo::applicationDomainGet()
    {
        SPtr<Instances::fl_system::ApplicationDomain> result;
        applicationDomainGet(result);
        return result;
    }
    SPtr<Instances::fl_utils::ByteArray> LoaderInfo::bytesGet()
    {
        SPtr<Instances::fl_utils::ByteArray> result;
        bytesGet(result);
        return result;
    }
    SPtr<Instances::fl_display::DisplayObject> LoaderInfo::contentGet()
    {
        SPtr<Instances::fl_display::DisplayObject> result;
        contentGet(result);
        return result;
    }
    SPtr<Instances::fl_display::Loader> LoaderInfo::loaderGet()
    {
        SPtr<Instances::fl_display::Loader> result;
        loaderGet(result);
        return result;
    }
    SPtr<Instances::fl_events::EventDispatcher> LoaderInfo::sharedEventsGet()
    {
        SPtr<Instances::fl_events::EventDispatcher> result;
        sharedEventsGet(result);
        return result;
    }
//##protect##"instance$methods"
    void LoaderInfo::ExecuteProgressEvent(UPInt bytesLoaded, UPInt totalBytes)
    {
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        ASString evtName(asvm.GetBuiltin(AS3Builtin_progress));
        if (HasEventHandler(evtName, false))
        {
            SPtr<fl_events::ProgressEvent> efe = CreateProgressEventObject(evtName);
            efe->Target  = this;
            BytesLoaded = static_cast<UInt32>(bytesLoaded);
            BytesTotal  = static_cast<UInt32>(totalBytes);
            efe->SetBytesLoaded(bytesLoaded);
            efe->SetBytesTotal(totalBytes);
            DispatchSingleEvent(efe, false);
        }
    }

    void LoaderInfo::ExecuteCompleteEvent()
    {
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        ASString evtName(asvm.GetBuiltin(AS3Builtin_complete));
        if (HasEventHandler(evtName, false))
        {
            SPtr<fl_events::Event> efe = CreateEventObject(evtName);
            efe->Target  = this;
            DispatchSingleEvent(efe, false);
        }
    }

    void LoaderInfo::ExecuteErrorEvent(const char* errorStr)
    {
        SPtr<fl_events::IOErrorEvent> efe = CreateIOErrorEventObject(errorStr);
        efe->Target  = this;
        DispatchSingleEvent(efe, false);
    }

    void LoaderInfo::ExecuteOpenEvent()
    {
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        ASString evtName(asvm.GetBuiltin(AS3Builtin_open));
        if (HasEventHandler(evtName, false))
        {
            SPtr<fl_events::Event> efe = CreateEventObject(evtName);
            efe->Target  = this;
            DispatchSingleEvent(efe, false);
        }
    }

    void LoaderInfo::ExecuteInitEvent(GFx::DisplayObject* obj)
    {
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        ASString evtName(asvm.GetBuiltin(AS3Builtin_init));
        if (obj)
        {
            ToAvmDisplayObj(obj)->CreateASInstance(true); 
            Content = ToAvmDisplayObj(obj)->GetAS3Obj();
        }
        if (HasEventHandler(evtName, false))
        {
            SPtr<fl_events::Event> efe = CreateEventObject(evtName);
            efe->Target  = this;
            DispatchSingleEvent(efe, false);
        }
    }

    void LoaderInfo::SetAppDomain(Instances::fl_system::ApplicationDomain* appDomain)
    {
        if (appDomain != NULL)
        {
            AppDomain = &appDomain->GetAppDomain();
        }
        else
        {
            AppDomain = GetVM().GetFrameAppDomain().AddNewChild(GetVM());
        }
    }

    void LoaderInfo::VerifyAppDomain()
    {
        if (AppDomain->IsEmpty())
        {
            GetVM().RemoveAppDomain(*AppDomain);
        }

    }

    void LoaderInfo::ForEachChild_GC(Collector* prcc, GcOp op) const
    {
        Instances::fl_events::EventDispatcher::ForEachChild_GC(prcc, op);
        AS3::ForEachChild_GC<DisplayObject, Mem_Stat>(prcc, Content, op SF_DEBUG_ARG(*this));
        AS3::ForEachChild_GC<Loader, Mem_Stat>(prcc, pLoader, op SF_DEBUG_ARG(*this));
    }

//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{
    // const UInt16 LoaderInfo::tito[LoaderInfo::ThunkInfoNum] = {
    //    0, 1, 2, 3, 4, 5, 6, 7, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 20, 21, 22, 23, 24, 
    // };
    const TypeInfo* LoaderInfo::tit[25] = {
        &AS3::fl::uintTI, 
        &AS3::fl_system::ApplicationDomainTI, 
        &AS3::fl_utils::ByteArrayTI, 
        &AS3::fl::uintTI, 
        &AS3::fl::uintTI, 
        &AS3::fl::BooleanTI, 
        &AS3::fl::ObjectTI, 
        NULL, &AS3::fl::ObjectTI, 
        &AS3::fl_display::DisplayObjectTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::int_TI, 
        &AS3::fl_display::LoaderTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::ObjectTI, 
        &AS3::fl::BooleanTI, 
        &AS3::fl::ObjectTI, 
        NULL, &AS3::fl::ObjectTI, 
        &AS3::fl::BooleanTI, 
        &AS3::fl_events::EventDispatcherTI, 
        &AS3::fl::uintTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::int_TI, 
    };
    const ThunkInfo LoaderInfo::ti[LoaderInfo::ThunkInfoNum] = {
        {TFunc_Instances_LoaderInfo_actionScriptVersionGet::Func, &LoaderInfo::tit[0], "actionScriptVersion", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_LoaderInfo_applicationDomainGet::Func, &LoaderInfo::tit[1], "applicationDomain", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_LoaderInfo_bytesGet::Func, &LoaderInfo::tit[2], "bytes", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_LoaderInfo_bytesLoadedGet::Func, &LoaderInfo::tit[3], "bytesLoaded", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_LoaderInfo_bytesTotalGet::Func, &LoaderInfo::tit[4], "bytesTotal", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_LoaderInfo_childAllowsParentGet::Func, &LoaderInfo::tit[5], "childAllowsParent", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_LoaderInfo_childSandboxBridgeGet::Func, &LoaderInfo::tit[6], "childSandboxBridge", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_LoaderInfo_childSandboxBridgeSet::Func, &LoaderInfo::tit[7], "childSandboxBridge", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_LoaderInfo_contentGet::Func, &LoaderInfo::tit[9], "content", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_LoaderInfo_contentTypeGet::Func, &LoaderInfo::tit[10], "contentType", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_LoaderInfo_frameRateGet::Func, &LoaderInfo::tit[11], "frameRate", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_LoaderInfo_heightGet::Func, &LoaderInfo::tit[12], "height", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_LoaderInfo_loaderGet::Func, &LoaderInfo::tit[13], "loader", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_LoaderInfo_loaderURLGet::Func, &LoaderInfo::tit[14], "loaderURL", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_LoaderInfo_parametersGet::Func, &LoaderInfo::tit[15], "parameters", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_LoaderInfo_parentAllowsChildGet::Func, &LoaderInfo::tit[16], "parentAllowsChild", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_LoaderInfo_parentSandboxBridgeGet::Func, &LoaderInfo::tit[17], "parentSandboxBridge", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_LoaderInfo_parentSandboxBridgeSet::Func, &LoaderInfo::tit[18], "parentSandboxBridge", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_LoaderInfo_sameDomainGet::Func, &LoaderInfo::tit[20], "sameDomain", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_LoaderInfo_sharedEventsGet::Func, &LoaderInfo::tit[21], "sharedEvents", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_LoaderInfo_swfVersionGet::Func, &LoaderInfo::tit[22], "swfVersion", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_LoaderInfo_urlGet::Func, &LoaderInfo::tit[23], "url", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_LoaderInfo_widthGet::Func, &LoaderInfo::tit[24], "width", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
    };

    LoaderInfo::LoaderInfo(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"InstanceTraits::LoaderInfo::LoaderInfo()"
//##protect##"InstanceTraits::LoaderInfo::LoaderInfo()"

    }

    void LoaderInfo::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<LoaderInfo&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl_display
{
    LoaderInfo::LoaderInfo(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::LoaderInfo::LoaderInfo()"
//##protect##"class_::LoaderInfo::LoaderInfo()"
    }
    void LoaderInfo::getLoaderInfoByDefinition(SPtr<Instances::fl_display::LoaderInfo>& result, const Value& object)
    {
//##protect##"class_::LoaderInfo::getLoaderInfoByDefinition()"
        SF_UNUSED2(result, object);
        WARN_NOT_IMPLEMENTED("class_::LoaderInfo::getLoaderInfoByDefinition()");
//##protect##"class_::LoaderInfo::getLoaderInfoByDefinition()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes

typedef ThunkFunc1<Classes::fl_display::LoaderInfo, Classes::fl_display::LoaderInfo::mid_getLoaderInfoByDefinition, SPtr<Instances::fl_display::LoaderInfo>, const Value&> TFunc_Classes_LoaderInfo_getLoaderInfoByDefinition;

template <> const TFunc_Classes_LoaderInfo_getLoaderInfoByDefinition::TMethod TFunc_Classes_LoaderInfo_getLoaderInfoByDefinition::Method = &Classes::fl_display::LoaderInfo::getLoaderInfoByDefinition;

namespace ClassTraits { namespace fl_display
{
    // const UInt16 LoaderInfo::tito[LoaderInfo::ThunkInfoNum] = {
    //    0, 
    // };
    const TypeInfo* LoaderInfo::tit[2] = {
        &AS3::fl_display::LoaderInfoTI, &AS3::fl::ObjectTI, 
    };
    const ThunkInfo LoaderInfo::ti[LoaderInfo::ThunkInfoNum] = {
        {TFunc_Classes_LoaderInfo_getLoaderInfoByDefinition::Func, &LoaderInfo::tit[0], "getLoaderInfoByDefinition", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
    };

    LoaderInfo::LoaderInfo(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"ClassTraits::LoaderInfo::LoaderInfo()"
//##protect##"ClassTraits::LoaderInfo::LoaderInfo()"

    }

    Pickable<Traits> LoaderInfo::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) LoaderInfo(vm, AS3::fl_display::LoaderInfoCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::LoaderInfoCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_display
{
    const TypeInfo LoaderInfoTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_display::LoaderInfo::InstanceType),
        ClassTraits::fl_display::LoaderInfo::ThunkInfoNum,
        0,
        InstanceTraits::fl_display::LoaderInfo::ThunkInfoNum,
        0,
        "LoaderInfo", "flash.display", &fl_events::EventDispatcherTI,
        TypeInfo::None
    };

    const ClassInfo LoaderInfoCI = {
        &LoaderInfoTI,
        ClassTraits::fl_display::LoaderInfo::MakeClassTraits,
        ClassTraits::fl_display::LoaderInfo::ti,
        NULL,
        InstanceTraits::fl_display::LoaderInfo::ti,
        NULL,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

