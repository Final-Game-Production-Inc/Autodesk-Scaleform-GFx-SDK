//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Media_Video.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Media_Video.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "AS3_Obj_Media_Camera.h"
#include "../Net/AS3_Obj_Net_NetStream.h"
#include "GFx/AS3/AS3_MovieRoot.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_media::Video, Instances::fl_media::Video::mid_deblockingGet, SInt32> TFunc_Instances_Video_deblockingGet;
typedef ThunkFunc1<Instances::fl_media::Video, Instances::fl_media::Video::mid_deblockingSet, const Value, SInt32> TFunc_Instances_Video_deblockingSet;
typedef ThunkFunc0<Instances::fl_media::Video, Instances::fl_media::Video::mid_smoothingGet, bool> TFunc_Instances_Video_smoothingGet;
typedef ThunkFunc1<Instances::fl_media::Video, Instances::fl_media::Video::mid_smoothingSet, const Value, bool> TFunc_Instances_Video_smoothingSet;
typedef ThunkFunc0<Instances::fl_media::Video, Instances::fl_media::Video::mid_videoHeightGet, SInt32> TFunc_Instances_Video_videoHeightGet;
typedef ThunkFunc0<Instances::fl_media::Video, Instances::fl_media::Video::mid_videoWidthGet, SInt32> TFunc_Instances_Video_videoWidthGet;
typedef ThunkFunc1<Instances::fl_media::Video, Instances::fl_media::Video::mid_attachCamera, const Value, Instances::fl_events::EventDispatcher*> TFunc_Instances_Video_attachCamera;
typedef ThunkFunc1<Instances::fl_media::Video, Instances::fl_media::Video::mid_attachNetStream, const Value, Instances::fl_net::NetStream*> TFunc_Instances_Video_attachNetStream;
typedef ThunkFunc0<Instances::fl_media::Video, Instances::fl_media::Video::mid_clear, const Value> TFunc_Instances_Video_clear;

template <> const TFunc_Instances_Video_deblockingGet::TMethod TFunc_Instances_Video_deblockingGet::Method = &Instances::fl_media::Video::deblockingGet;
template <> const TFunc_Instances_Video_deblockingSet::TMethod TFunc_Instances_Video_deblockingSet::Method = &Instances::fl_media::Video::deblockingSet;
template <> const TFunc_Instances_Video_smoothingGet::TMethod TFunc_Instances_Video_smoothingGet::Method = &Instances::fl_media::Video::smoothingGet;
template <> const TFunc_Instances_Video_smoothingSet::TMethod TFunc_Instances_Video_smoothingSet::Method = &Instances::fl_media::Video::smoothingSet;
template <> const TFunc_Instances_Video_videoHeightGet::TMethod TFunc_Instances_Video_videoHeightGet::Method = &Instances::fl_media::Video::videoHeightGet;
template <> const TFunc_Instances_Video_videoWidthGet::TMethod TFunc_Instances_Video_videoWidthGet::Method = &Instances::fl_media::Video::videoWidthGet;
template <> const TFunc_Instances_Video_attachCamera::TMethod TFunc_Instances_Video_attachCamera::Method = &Instances::fl_media::Video::attachCamera;
template <> const TFunc_Instances_Video_attachNetStream::TMethod TFunc_Instances_Video_attachNetStream::Method = &Instances::fl_media::Video::attachNetStream;
template <> const TFunc_Instances_Video_clear::TMethod TFunc_Instances_Video_clear::Method = &Instances::fl_media::Video::clear;

namespace Instances { namespace fl_media
{
    Video::Video(InstanceTraits::Traits& t)
    : Instances::fl_display::DisplayObject(t)
//##protect##"instance::Video::Video()$data"
//##protect##"instance::Video::Video()$data"
    {
//##protect##"instance::Video::Video()$code"
#ifdef GFX_ENABLE_VIDEO
        Width  = 320;
        Height = 240;
#endif
//##protect##"instance::Video::Video()$code"
    }

    void Video::deblockingGet(SInt32& result)
    {
//##protect##"instance::Video::deblockingGet()"
#ifdef GFX_ENABLE_VIDEO
        const AvmVideoCharacterBase* pavm = GetVideoCharacter()->GetAvmVideoCharacter();
        result = pavm->Deblocking;
#else
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("Video::deblockingGet()");
#endif
//##protect##"instance::Video::deblockingGet()"
    }
    void Video::deblockingSet(const Value& result, SInt32 value)
    {
//##protect##"instance::Video::deblockingSet()"
#ifdef GFX_ENABLE_VIDEO
        SF_UNUSED(result);
        AvmVideoCharacterBase* pavm = GetVideoCharacter()->GetAvmVideoCharacter();
        pavm->Deblocking = value;
#else
        SF_UNUSED2(result, value);
        WARN_NOT_IMPLEMENTED("Video::deblockingSet()");
#endif
//##protect##"instance::Video::deblockingSet()"
    }
    void Video::smoothingGet(bool& result)
    {
//##protect##"instance::Video::smoothingGet()"
#ifdef GFX_ENABLE_VIDEO
        const AvmVideoCharacterBase* pavm = GetVideoCharacter()->GetAvmVideoCharacter();
        result = pavm->Smoothing;
#else
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("Video::smoothingGet()");
#endif
//##protect##"instance::Video::smoothingGet()"
    }
    void Video::smoothingSet(const Value& result, bool value)
    {
//##protect##"instance::Video::smoothingSet()"
#ifdef GFX_ENABLE_VIDEO
        SF_UNUSED(result);
        AvmVideoCharacterBase* pavm = GetVideoCharacter()->GetAvmVideoCharacter();
        pavm->Smoothing = value;
#else
        SF_UNUSED2(result, value);
        WARN_NOT_IMPLEMENTED("Video::smoothingSet()");
#endif
//##protect##"instance::Video::smoothingSet()"
    }
    void Video::videoHeightGet(SInt32& result)
    {
//##protect##"instance::Video::videoHeightGet()"
#ifdef GFX_ENABLE_VIDEO
        result = (SInt32)Height;
#else
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("Video::videoHeightGet()");
#endif
//##protect##"instance::Video::videoHeightGet()"
    }
    void Video::videoWidthGet(SInt32& result)
    {
//##protect##"instance::Video::videoWidthGet()"
#ifdef GFX_ENABLE_VIDEO
        result = (SInt32)Width;
#else
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("Video::videoWidthGet()");
#endif
//##protect##"instance::Video::videoWidthGet()"
    }
    void Video::attachCamera(const Value& result, Instances::fl_events::EventDispatcher* camera)
    {
//##protect##"instance::Video::attachCamera()"
        SF_UNUSED2(result, camera);
        WARN_NOT_IMPLEMENTED("Video::attachCamera()");
//##protect##"instance::Video::attachCamera()"
    }
    void Video::attachNetStream(const Value& result, Instances::fl_net::NetStream* netStream)
    {
//##protect##"instance::Video::attachNetStream()"
#ifdef GFX_ENABLE_VIDEO
        SF_UNUSED(result);

        GFx::Video::VideoProvider* pprovider = NULL;
        if (netStream)
            pprovider = netStream->GetVideoProvider();

        GFx::Video::VideoCharacter* pvideo = GetVideoCharacter();
        if (pvideo)
        {
            pvideo->AttachVideoProvider(pprovider);
            if (pprovider)
                pprovider->RegisterVideoCharacter(pvideo);
        }
#else
        SF_UNUSED2(result, netStream);
        WARN_NOT_IMPLEMENTED("Video::attachNetStream()");
#endif
//##protect##"instance::Video::attachNetStream()"
    }
    void Video::clear(const Value& result)
    {
//##protect##"instance::Video::clear()"
#ifdef GFX_ENABLE_VIDEO
        SF_UNUSED(result);
        GetVideoCharacter()->ReleaseTexture();
#else
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("Video::clear()");
#endif
//##protect##"instance::Video::clear()"
    }

//##protect##"instance$methods"
#ifdef GFX_ENABLE_VIDEO
    void Video::AS3Constructor(unsigned argc, const Value* argv)
    {
        if (argc >= 2)
        {
            argv[0].Convert2Number(Width).DoNotCheck();
            argv[1].Convert2Number(Height).DoNotCheck();
            GetVideoCharacter()->ViewRect = Render::RectF(0, 0,
                PixelsToTwips((float)Width), PixelsToTwips((float)Height));
        }
    }

    void Video::InitInstance(bool extCall)
    {
        if (!extCall)
        {
            CreateStageObject();
            GetVideoCharacter()->ViewRect = Render::RectF(0, 0,
                PixelsToTwips((float)Width), PixelsToTwips((float)Height));
        }
    }

    GFx::DisplayObject* Video::CreateStageObject()
    {
        if (!pDispObj)
        {
            ASVM& asvm = static_cast<ASVM&>(GetVM());
            MovieRoot* proot = asvm.GetMovieRoot();

            MovieDefImpl* pdefImpl = asvm.GetResourceMovieDef(this);
            if (pdefImpl)
            {
                CharacterCreateInfo ccinfo;
                ccinfo.pCharDef = NULL;

                FindLibarySymbol(&ccinfo, pdefImpl);
                if (!ccinfo.pCharDef)
                    ccinfo = pdefImpl->GetCharacterCreateInfo(ResourceId(CharacterDef::CharId_EmptyVideo));
                SF_ASSERT(ccinfo.pCharDef);

                pDispObj = *static_cast<GFx::DisplayObject*>(proot->GetASSupport()->CreateCharacterInstance(
                    proot->GetMovieImpl(), ccinfo, NULL, 
                    ResourceId(), CharacterDef::Video));
                AvmDisplayObj* pAvmObj = ToAvmDisplayObj(pDispObj);
                pAvmObj->AssignAS3Obj(this);
                pAvmObj->SetAppDomain(GetInstanceTraits().GetAppDomain());
            }
        }
        return pDispObj;
    }
#endif
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_media
{
    // const UInt16 Video::tito[Video::ThunkInfoNum] = {
    //    0, 1, 3, 4, 6, 7, 8, 10, 12, 
    // };
    const TypeInfo* Video::tit[13] = {
        &AS3::fl::int_TI, 
        NULL, &AS3::fl::int_TI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::int_TI, 
        &AS3::fl::int_TI, 
        NULL, &AS3::fl_media::CameraTI, 
        NULL, &AS3::fl_net::NetStreamTI, 
        NULL, 
    };
    const ThunkInfo Video::ti[Video::ThunkInfoNum] = {
        {TFunc_Instances_Video_deblockingGet::Func, &Video::tit[0], "deblocking", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Video_deblockingSet::Func, &Video::tit[1], "deblocking", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Video_smoothingGet::Func, &Video::tit[3], "smoothing", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Video_smoothingSet::Func, &Video::tit[4], "smoothing", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Video_videoHeightGet::Func, &Video::tit[6], "videoHeight", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Video_videoWidthGet::Func, &Video::tit[7], "videoWidth", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Video_attachCamera::Func, &Video::tit[8], "attachCamera", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Video_attachNetStream::Func, &Video::tit[10], "attachNetStream", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Video_clear::Func, &Video::tit[12], "clear", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    Video::Video(VM& vm, const ClassInfo& ci)
    : fl_display::DisplayObject(vm, ci)
    {
//##protect##"InstanceTraits::Video::Video()"
//##protect##"InstanceTraits::Video::Video()"

    }

    void Video::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<Video&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_media
{

    Video::Video(VM& vm, const ClassInfo& ci)
    : fl_display::DisplayObject(vm, ci)
    {
//##protect##"ClassTraits::Video::Video()"
//##protect##"ClassTraits::Video::Video()"

    }

    Pickable<Traits> Video::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) Video(vm, AS3::fl_media::VideoCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_media::VideoCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_media
{
    const TypeInfo VideoTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_media::Video::InstanceType),
        0,
        0,
        InstanceTraits::fl_media::Video::ThunkInfoNum,
        0,
        "Video", "flash.media", &fl_display::DisplayObjectTI,
        TypeInfo::None
    };

    const ClassInfo VideoCI = {
        &VideoTI,
        ClassTraits::fl_media::Video::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_media::Video::ti,
        NULL,
    };
}; // namespace fl_media


}}} // namespace Scaleform { namespace GFx { namespace AS3

