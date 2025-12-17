//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_Loader.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Display_Loader.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../Utils/AS3_Obj_Utils_ByteArray.h"
#include "AS3_Obj_Display_LoaderInfo.h"
#include "../System/AS3_Obj_System_LoaderContext.h"
#include "GFx/AS3/AS3_MovieRoot.h"
#include "GFx/AS3/AS3_AvmLoader.h"
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
    bool GetMethodDefArg<Instances::fl_display::Loader, Instances::fl_display::Loader::mid_unloadAndStop, 0, bool>(AS3::StringManager&)
    {
        return true;
    }

} // namespace Impl
#endif // SF_AS3_EMIT_DEF_ARGS

typedef ThunkFunc0<Instances::fl_display::Loader, Instances::fl_display::Loader::mid_contentGet, SPtr<Instances::fl_display::DisplayObject> > TFunc_Instances_Loader_contentGet;
typedef ThunkFunc0<Instances::fl_display::Loader, Instances::fl_display::Loader::mid_contentLoaderInfoGet, SPtr<Instances::fl_display::LoaderInfo> > TFunc_Instances_Loader_contentLoaderInfoGet;
typedef ThunkFunc0<Instances::fl_display::Loader, Instances::fl_display::Loader::mid_close, const Value> TFunc_Instances_Loader_close;
typedef ThunkFunc2<Instances::fl_display::Loader, Instances::fl_display::Loader::mid_load, const Value, Instances::fl_net::URLRequest*, Instances::fl_system::LoaderContext*> TFunc_Instances_Loader_load;
typedef ThunkFunc2<Instances::fl_display::Loader, Instances::fl_display::Loader::mid_loadBytes, const Value, Instances::fl_utils::ByteArray*, Instances::fl_system::LoaderContext*> TFunc_Instances_Loader_loadBytes;
typedef ThunkFunc0<Instances::fl_display::Loader, Instances::fl_display::Loader::mid_unload, const Value> TFunc_Instances_Loader_unload;
typedef ThunkFunc1<Instances::fl_display::Loader, Instances::fl_display::Loader::mid_unloadAndStop, const Value, bool> TFunc_Instances_Loader_unloadAndStop;

template <> const TFunc_Instances_Loader_contentGet::TMethod TFunc_Instances_Loader_contentGet::Method = &Instances::fl_display::Loader::contentGet;
template <> const TFunc_Instances_Loader_contentLoaderInfoGet::TMethod TFunc_Instances_Loader_contentLoaderInfoGet::Method = &Instances::fl_display::Loader::contentLoaderInfoGet;
template <> const TFunc_Instances_Loader_close::TMethod TFunc_Instances_Loader_close::Method = &Instances::fl_display::Loader::close;
template <> const TFunc_Instances_Loader_load::TMethod TFunc_Instances_Loader_load::Method = &Instances::fl_display::Loader::load;
template <> const TFunc_Instances_Loader_loadBytes::TMethod TFunc_Instances_Loader_loadBytes::Method = &Instances::fl_display::Loader::loadBytes;
template <> const TFunc_Instances_Loader_unload::TMethod TFunc_Instances_Loader_unload::Method = &Instances::fl_display::Loader::unload;
template <> const TFunc_Instances_Loader_unloadAndStop::TMethod TFunc_Instances_Loader_unloadAndStop::Method = &Instances::fl_display::Loader::unloadAndStop;

namespace Instances { namespace fl_display
{
    Loader::Loader(InstanceTraits::Traits& t)
    : Instances::fl_display::DisplayObjectContainer(t)
//##protect##"instance::Loader::Loader()$data"
//##protect##"instance::Loader::Loader()$data"
    {
//##protect##"instance::Loader::Loader()$code"
//##protect##"instance::Loader::Loader()$code"
    }

    void Loader::contentGet(SPtr<Instances::fl_display::DisplayObject>& result)
    {
//##protect##"instance::Loader::contentGet()"
        SF_UNUSED1(result);
        if (GetDisplayObjContainer()->GetNumChildren() > 0)
            getChildAt(result, 0); //?
        else
            result = NULL;
//##protect##"instance::Loader::contentGet()"
    }
    void Loader::contentLoaderInfoGet(SPtr<Instances::fl_display::LoaderInfo>& result)
    {
//##protect##"instance::Loader::contentLoaderInfoGet()"
        SF_UNUSED1(result);
        result = pContentLoaderInfo;
//##protect##"instance::Loader::contentLoaderInfoGet()"
    }
    void Loader::close(const Value& result)
    {
//##protect##"instance::Loader::close()"
        SF_UNUSED1(result);

        ASVM& vm = static_cast<ASVM&>(GetVM());
        MovieRoot* root = vm.GetMovieRoot();
        root->CancelMovieLoading(this);
//##protect##"instance::Loader::close()"
    }
    void Loader::load(const Value& result, Instances::fl_net::URLRequest* request, Instances::fl_system::LoaderContext* context)
    {
//##protect##"instance::Loader::load()"
        SF_UNUSED(result);
        ASVM& vm = static_cast<ASVM&>(GetVM());
        MovieRoot* root = vm.GetMovieRoot();
        if (pContentLoaderInfo)
        {
            pContentLoaderInfo->SetAppDomain(context != NULL ? context->applicationDomain : NULL);
        }

        // check if we need to unload already loaded movie first.
        // !AB: Flash doesn't unload previously loaded movie (in the case when
        // the same Loader is used for multiple loadings, see 
        // Test\AS3\SWF\Loader\test_mult_load_same_loader).
        //if (pContentLoaderInfo && pContentLoaderInfo->GetContentDispObj())
        //    root->UnloadMovie(this);

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
//##protect##"instance::Loader::load()"
    }
    void Loader::loadBytes(const Value& result, Instances::fl_utils::ByteArray* bytes, Instances::fl_system::LoaderContext* context)
    {
//##protect##"instance::Loader::loadBytes()"
        SF_UNUSED(result);

        ASVM& vm = static_cast<ASVM&>(GetVM());
        MovieRoot* root = vm.GetMovieRoot();
        if (pContentLoaderInfo)
        {
            pContentLoaderInfo->SetAppDomain(context != NULL ? context->applicationDomain : NULL);
        }

        // check if we need to unload already loaded movie first.
        //if (pContentLoaderInfo && pContentLoaderInfo->GetContentDispObj())
        //    root->UnloadMovie(this);

        root->AddNewLoadQueueEntry(bytes, this);
//##protect##"instance::Loader::loadBytes()"
    }
    void Loader::unload(const Value& result)
    {
//##protect##"instance::Loader::unload()"
        SF_UNUSED1(result);

        ASVM& vm = static_cast<ASVM&>(GetVM());
        MovieRoot* root = vm.GetMovieRoot();
        root->UnloadMovie(this);
//##protect##"instance::Loader::unload()"
    }
    void Loader::unloadAndStop(const Value& result, bool gc)
    {
//##protect##"instance::Loader::unloadAndStop()"
        SF_UNUSED2(result, gc);

        // Tries to unload a child SWF and stops the execution from loaded SWFs. 
        // Tries to remove references to EventDispatcher, NetConnection, Timer, Sound, 
        // or Video objects of the child SWF file. The following occurs for the child SWF 
        // file and the child SWF file's display list:
        //    Sounds are stopped.
        //    Stage event listeners are removed.
        //    Event listeners for enterFrame, frameConstructed, exitFrame, activate and deactivate are removed.
        //    Timers are stopped.
        //    Camera and Microphone instances are detached (n/a)
        //    Movie clips are stopped.
        ASVM& vm = static_cast<ASVM&>(GetVM());
        MovieRoot* root = vm.GetMovieRoot();
        root->UnloadMovie(this, true, gc);
//##protect##"instance::Loader::unloadAndStop()"
    }

    SPtr<Instances::fl_display::DisplayObject> Loader::contentGet()
    {
        SPtr<Instances::fl_display::DisplayObject> result;
        contentGet(result);
        return result;
    }
    SPtr<Instances::fl_display::LoaderInfo> Loader::contentLoaderInfoGet()
    {
        SPtr<Instances::fl_display::LoaderInfo> result;
        contentLoaderInfoGet(result);
        return result;
    }
//##protect##"instance$methods"
    Loader::~Loader()
    {
    }

    void Loader::ForEachChild_GC(Collector* prcc, GcOp op) const
    {
        DisplayObjectContainer::ForEachChild_GC(prcc, op);
        AS3::ForEachChild_GC<LoaderInfo, Mem_Stat>(prcc, pContentLoaderInfo, op SF_DEBUG_ARG(*this));
    }

    void Loader::InitInstance(bool extCall)
    {
        ASVM& vm        = static_cast<ASVM&>(GetVM());

        if (!extCall)
        {
            // create stage display object if the movieclip was created by
            // "new".
            // create a LoaderInfo object
            SPtr<AS3::Object> _class = vm.GetClass("flash.display.LoaderInfo", vm.GetCurrentAppDomain());
            if (_class.GetPtr() == NULL)  
                SF_ASSERT(0);

            if (vm.ConstructInstance(pContentLoaderInfo, _class))
            {
            }
            else
                SF_ASSERT(0); //@TODO error handling

            pContentLoaderInfo->SetLoader(this);

            Loader::CreateStageObject();
            SF_ASSERT(pDispObj);
        }
        else
        {
            if (pDispObj)
            {
                Instances::fl_display::DisplayObject* as3dobj = ToAvmDisplayObj(pDispObj)->GetAS3Obj();
                if (as3dobj && as3dobj->pLoaderInfo)
                    as3dobj->pLoaderInfo->SetLoader(this);
            }
        }
    }
    GFx::DisplayObject* Loader::CreateStageObject()
    {
        if (!pDispObj)
        {
            MovieRoot* proot = static_cast<const ASVM&>(GetVM()).GetMovieRoot();

            pDispObj = *SF_HEAP_NEW(proot->GetMovieHeap()) 
                AS3::AvmLoader(proot, proot->GetMovieImpl()->GetMovieDefImpl(), NULL, ResourceId());

            AvmDisplayObj* pAvmObj = ToAvmDisplayObj(pDispObj);
            pAvmObj->AssignAS3Obj(this);
            pAvmObj->SetAppDomain(GetInstanceTraits().GetAppDomain());
            proot->AddScriptableMovieClip(GetDisplayObjContainer()); 
        }
        return pDispObj;
    }

    void Loader::ResetContent() 
    { 
        if (pContentLoaderInfo)
            pContentLoaderInfo->ResetContent(); 
    }

    void Loader::ExecuteProgressEvent(UPInt bytesLoaded, UPInt totalBytes)
    {
        if (pContentLoaderInfo)
            pContentLoaderInfo->ExecuteProgressEvent(bytesLoaded, totalBytes);
    }

    void Loader::ExecuteCompleteEvent()
    {
        if (pContentLoaderInfo)
            pContentLoaderInfo->ExecuteCompleteEvent();
    }
    static void ExecuteCompleteEventCFunc(const MovieRoot::ActionEntry& ae)
    {
        SF_ASSERT(ae.pAS3Obj);
        Loader* loader = static_cast<Loader*>(ae.pAS3Obj.GetPtr());
        loader->ExecuteCompleteEvent();
    }

    void Loader::QueueCompleteEvent()
    {
        if (pContentLoaderInfo)
        {
            MovieRoot* proot = static_cast<const ASVM&>(GetVM()).GetMovieRoot();
            MovieRoot::ActionEntry* ae = proot->InsertEmptyAction(MovieRoot::AL_Manual);
            if (ae)
            {
                ae->SetAction(pDispObj, this, ExecuteCompleteEventCFunc);
            }
        }
    }

    static void ExecuteInitEventCFunc(const MovieRoot::ActionEntry& ae)
    {
        SF_ASSERT(ae.pAS3Obj);
        Loader* loader = static_cast<Loader*>(ae.pAS3Obj.GetPtr());
        loader->ExecuteInitEvent(ae.pCharacter);
		if (ae.pNLoadInitCL)
			ae.pNLoadInitCL->InitEventCallback();
    }
    void Loader::QueueInitEvent(GFx::DisplayObject* obj, Ptr<NotifyLoadInitC> pnLoadInitCL )
    {
        if (pContentLoaderInfo)
        {
            MovieRoot* proot = static_cast<const ASVM&>(GetVM()).GetMovieRoot();
            MovieRoot::ActionEntry* ae = proot->InsertEmptyAction(MovieRoot::AL_Manual);
            if (ae)
            {
                ae->SetAction(pDispObj, this, ExecuteInitEventCFunc);
                ae->pCharacter		= obj;
				ae->pNLoadInitCL	= pnLoadInitCL;
            }
        }
    }
    void Loader::ExecuteInitEvent(GFx::DisplayObject* obj)
    {
        if (pContentLoaderInfo)
            pContentLoaderInfo->ExecuteInitEvent(obj);
    }
    void Loader::ExecuteErrorEvent(const char* url)
    {
        if (pContentLoaderInfo)
        {
            char buf[1024];
            SFsprintf(buf, sizeof(buf), "Error reading %s", url);
            pContentLoaderInfo->ExecuteErrorEvent(buf);
        }
    }

    void Loader::ExecuteOpenEvent()
    {
        if (pContentLoaderInfo)
            pContentLoaderInfo->ExecuteOpenEvent();
    }


    LoaderInfo* Loader::GetContentLoaderInfo() const
    {
        return pContentLoaderInfo;
    }
    AvmLoader* Loader::GetAvmLoader() const { return static_cast<AvmLoader*>(pDispObj.GetPtr()); }

//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{
    // const UInt16 Loader::tito[Loader::ThunkInfoNum] = {
    //    0, 1, 2, 3, 6, 9, 10, 
    // };
    const TypeInfo* Loader::tit[12] = {
        &AS3::fl_display::DisplayObjectTI, 
        &AS3::fl_display::LoaderInfoTI, 
        NULL, 
        NULL, &AS3::fl_net::URLRequestTI, &AS3::fl_system::LoaderContextTI, 
        NULL, &AS3::fl_utils::ByteArrayTI, &AS3::fl_system::LoaderContextTI, 
        NULL, 
        NULL, &AS3::fl::BooleanTI, 
    };
    const Abc::ConstValue Loader::dva[1] = {
        {Abc::CONSTANT_True, 0}, 
    };
    const ThunkInfo Loader::ti[Loader::ThunkInfoNum] = {
        {TFunc_Instances_Loader_contentGet::Func, &Loader::tit[0], "content", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Loader_contentLoaderInfoGet::Func, &Loader::tit[1], "contentLoaderInfo", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Loader_close::Func, &Loader::tit[2], "close", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Loader_load::Func, &Loader::tit[3], "load", NULL, Abc::NS_Public, CT_Method, 1, 2, 0, 0, NULL},
        {TFunc_Instances_Loader_loadBytes::Func, &Loader::tit[6], "loadBytes", NULL, Abc::NS_Public, CT_Method, 1, 2, 0, 0, NULL},
        {TFunc_Instances_Loader_unload::Func, &Loader::tit[9], "unload", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Loader_unloadAndStop::Func, &Loader::tit[10], "unloadAndStop", NULL, Abc::NS_Public, CT_Method, 0, 1, 0, 1, &Loader::dva[0]},
    };

    Loader::Loader(VM& vm, const ClassInfo& ci)
    : fl_display::DisplayObjectContainer(vm, ci)
    {
//##protect##"InstanceTraits::Loader::Loader()"
        SetTraitsType(Traits_Loader);
//##protect##"InstanceTraits::Loader::Loader()"

    }

    void Loader::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<Loader&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_display
{

    Loader::Loader(VM& vm, const ClassInfo& ci)
    : fl_display::DisplayObjectContainer(vm, ci)
    {
//##protect##"ClassTraits::Loader::Loader()"
        SetTraitsType(Traits_Loader);
//##protect##"ClassTraits::Loader::Loader()"

    }

    Pickable<Traits> Loader::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) Loader(vm, AS3::fl_display::LoaderCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::LoaderCI));
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
    const TypeInfo LoaderTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_display::Loader::InstanceType),
        0,
        0,
        InstanceTraits::fl_display::Loader::ThunkInfoNum,
        0,
        "Loader", "flash.display", &fl_display::DisplayObjectContainerTI,
        TypeInfo::None
    };

    const ClassInfo LoaderCI = {
        &LoaderTI,
        ClassTraits::fl_display::Loader::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_display::Loader::ti,
        NULL,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

