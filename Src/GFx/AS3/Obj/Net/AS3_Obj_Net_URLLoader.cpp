//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Net_URLLoader.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Net_URLLoader.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "AS3_Obj_Net_URLRequest.h"
#include "GFx/AS3/AS3_MovieRoot.h"
#include "AS3_Obj_Net_URLVariables.h"
#include "../Events/AS3_Obj_Events_ProgressEvent.h"
#include "../Events/AS3_Obj_Events_IOErrorEvent.h"
#include "../Utils/AS3_Obj_Utils_ByteArray.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc5<Instances::fl_net::URLLoader, Instances::fl_net::URLLoader::mid_addEventListener, const Value, const ASString&, const Value&, bool, SInt32, bool> TFunc_Instances_URLLoader_addEventListener;
typedef ThunkFunc0<Instances::fl_net::URLLoader, Instances::fl_net::URLLoader::mid_close, const Value> TFunc_Instances_URLLoader_close;
typedef ThunkFunc1<Instances::fl_net::URLLoader, Instances::fl_net::URLLoader::mid_load, const Value, Instances::fl_net::URLRequest*> TFunc_Instances_URLLoader_load;

template <> const TFunc_Instances_URLLoader_addEventListener::TMethod TFunc_Instances_URLLoader_addEventListener::Method = &Instances::fl_net::URLLoader::addEventListener;
template <> const TFunc_Instances_URLLoader_close::TMethod TFunc_Instances_URLLoader_close::Method = &Instances::fl_net::URLLoader::close;
template <> const TFunc_Instances_URLLoader_load::TMethod TFunc_Instances_URLLoader_load::Method = &Instances::fl_net::URLLoader::load;

namespace Instances { namespace fl_net
{
    URLLoader::URLLoader(InstanceTraits::Traits& t)
    : Instances::fl_events::EventDispatcher(t)
    , bytesLoaded(0)
    , bytesTotal(0)
    , data()
    , dataFormat(AS3::DefaultValue<ASString>(GetStringManager()))
//##protect##"instance::URLLoader::URLLoader()$data"
//##protect##"instance::URLLoader::URLLoader()$data"
    {
//##protect##"instance::URLLoader::URLLoader()$code"
        dataFormat = "text";
//##protect##"instance::URLLoader::URLLoader()$code"
    }

    void URLLoader::addEventListener(const Value& result, const ASString& type, const Value& listener, bool useCapture, SInt32 priority, bool useWeakReference)
    {
//##protect##"instance::URLLoader::addEventListener()"
        SF_UNUSED6(result, type, listener, useCapture, priority, useWeakReference);
        EventDispatcher::addEventListener(result, type, listener, useCapture, priority, useWeakReference);
//##protect##"instance::URLLoader::addEventListener()"
    }
    void URLLoader::close(const Value& result)
    {
//##protect##"instance::URLLoader::close()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("URLLoader::close()");
//##protect##"instance::URLLoader::close()"
    }
    void URLLoader::load(const Value& result, Instances::fl_net::URLRequest* request)
    {
//##protect##"instance::URLLoader::load()"
        SF_UNUSED2(result, request);
        ASVM& vm = static_cast<ASVM&>(GetVM());
        MovieRoot* root = vm.GetMovieRoot();

        LoadQueueEntry::LoadMethod method = LoadQueueEntry::LM_None;
        ASString requestMethod = request->methodGet().ToUpper();
        if (requestMethod == "GET")
        {
            method = LoadQueueEntry::LM_Get;
        }
        else if (requestMethod == "POST")
        {
            method = LoadQueueEntry::LM_Post;
        }
        else if (requestMethod == "PUT")
        {
            method = LoadQueueEntry::LM_Put;
        }
        else if (requestMethod == "DELETE")
        {
            method = LoadQueueEntry::LM_Delete;
        }
        root->AddNewLoadQueueEntry(request, this, method);
//##protect##"instance::URLLoader::load()"
    }

//##protect##"instance$methods"
    void URLLoader::AS3Constructor(unsigned argc, const Value* argv)
    {
        if (argc >= 1 && !argv[0].IsNullOrUndefined())
        {
            VM& vm = GetVM();
            Value result;
            const Multiname prop_name(vm.GetPublicNamespace(), vm.GetStringManager().CreateConstString("load"));

            // We do not check for exceptions here because this is the last statement.
            ExecutePropertyUnsafe(prop_name, result, argc, argv).DoNotCheck();
        }
    }

    bool URLLoader::IsLoadingVariables() const
    {
        return (dataFormat == "variables");
    }

    bool URLLoader::IsLoadingText() const
    {
        return (dataFormat == "text");
    }

    bool URLLoader::IsLoadingBinary() const
    {
        return (dataFormat == "binary");
    }

    void URLLoader::ExecuteProgressEvent(UInt32 bytesLoadedEvent, UInt32 totalBytes)
    {
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        ASString evtName(asvm.GetBuiltin(AS3Builtin_progress));
        if (HasEventHandler(evtName, false))
        {
            this->bytesLoaded = bytesLoadedEvent;
            this->bytesTotal  = totalBytes;
            SPtr<fl_events::ProgressEvent> efe = CreateProgressEventObject(evtName);
            efe->Target  = this;
            efe->SetBytesLoaded(bytesLoadedEvent);
            efe->SetBytesTotal(totalBytes);
            DispatchSingleEvent(efe, false);
        }
    }

    void URLLoader::ExecuteOpenEvent()
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

    void URLLoader::ExecuteCompleteEvent()
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

    void URLLoader::ExecuteErrorEvent(const char* errorStr)
    {
        SPtr<fl_events::IOErrorEvent> efe = CreateIOErrorEventObject(errorStr);
        efe->Target  = this;
        DispatchSingleEvent(efe, false);
    }

    void URLLoader::SetVariablesDataString(const char* pdata)
    {
        // create URLVariables object    
        ASVM& vm        = static_cast<ASVM&>(GetVM());

        // create a LoaderInfo object
        SPtr<AS3::Object> _class = vm.GetClass("flash.net.URLVariables", vm.GetCurrentAppDomain());
        if (_class.GetPtr() == NULL)  
            SF_ASSERT(0);

        ASString dataStr(GetVM().GetStringManager().CreateString(pdata));
        Value dataParams(dataStr);
        SPtr<URLVariables> varObj;
        if (vm.ConstructInstance(varObj, _class, 1, &dataParams))
        {
            data = varObj.GetPtr();
        }
        else
            SF_ASSERT(0);
    }

    void URLLoader::SetTextString(const char* pdata)
    {
        ASString dataStr(GetVM().GetStringManager().CreateString(pdata));
        data = Value(dataStr);
    }

    void URLLoader::SetBinaryData(const ArrayPOD<UByte>& binaryData)
    {
        // create URLVariables object    
        ASVM& vm        = static_cast<ASVM&>(GetVM());

        // create a LoaderInfo object
        
        SPtr<fl_utils::ByteArray> arrObj = vm.MakeByteArray();
        arrObj->Set(binaryData.GetDataPtr(), binaryData.GetSize());
        data = arrObj.GetPtr();
    }

//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_net
{
    // const UInt16 URLLoader::tito[URLLoader::ThunkInfoNum] = {
    //    0, 6, 7, 
    // };
    const TypeInfo* URLLoader::tit[9] = {
        NULL, &AS3::fl::StringTI, &AS3::fl::FunctionTI, &AS3::fl::BooleanTI, &AS3::fl::int_TI, &AS3::fl::BooleanTI, 
        NULL, 
        NULL, &AS3::fl_net::URLRequestTI, 
    };
    const ThunkInfo URLLoader::ti[URLLoader::ThunkInfoNum] = {
        {TFunc_Instances_URLLoader_addEventListener::Func, &URLLoader::tit[0], "addEventListener", NULL, Abc::NS_Public, CT_Method, 2, 5, 0, 0, NULL},
        {TFunc_Instances_URLLoader_close::Func, &URLLoader::tit[6], "close", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_URLLoader_load::Func, &URLLoader::tit[7], "load", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
    };
    const MemberInfo URLLoader::mi[URLLoader::MemberInfoNum] = {
        {"bytesLoaded", NULL, OFFSETOF(Instances::fl_net::URLLoader, bytesLoaded), Abc::NS_Public, SlotInfo::BT_UInt, 0},
        {"bytesTotal", NULL, OFFSETOF(Instances::fl_net::URLLoader, bytesTotal), Abc::NS_Public, SlotInfo::BT_UInt, 0},
        {"data", NULL, OFFSETOF(Instances::fl_net::URLLoader, data), Abc::NS_Public, SlotInfo::BT_Value, 0},
        {"dataFormat", NULL, OFFSETOF(Instances::fl_net::URLLoader, dataFormat), Abc::NS_Public, SlotInfo::BT_String, 0},
    };


    URLLoader::URLLoader(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"InstanceTraits::URLLoader::URLLoader()"
//##protect##"InstanceTraits::URLLoader::URLLoader()"

    }

    void URLLoader::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<URLLoader&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_net
{

    URLLoader::URLLoader(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"ClassTraits::URLLoader::URLLoader()"
//##protect##"ClassTraits::URLLoader::URLLoader()"

    }

    Pickable<Traits> URLLoader::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) URLLoader(vm, AS3::fl_net::URLLoaderCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_net::URLLoaderCI));
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
    const TypeInfo URLLoaderTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_net::URLLoader::InstanceType),
        0,
        0,
        InstanceTraits::fl_net::URLLoader::ThunkInfoNum,
        InstanceTraits::fl_net::URLLoader::MemberInfoNum,
        "URLLoader", "flash.net", &fl_events::EventDispatcherTI,
        TypeInfo::None
    };

    const ClassInfo URLLoaderCI = {
        &URLLoaderTI,
        ClassTraits::fl_net::URLLoader::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_net::URLLoader::ti,
        InstanceTraits::fl_net::URLLoader::mi,
    };
}; // namespace fl_net


}}} // namespace Scaleform { namespace GFx { namespace AS3

