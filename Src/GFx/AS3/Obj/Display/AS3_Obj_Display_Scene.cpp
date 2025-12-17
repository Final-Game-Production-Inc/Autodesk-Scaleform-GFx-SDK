//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_Scene.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Display_Scene.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../AS3_Obj_Array.h"
#include "GFx/AS3/AS3_MovieRoot.h"
#include "AS3_Obj_Display_FrameLabel.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_display::Scene, Instances::fl_display::Scene::mid_labelsGet, SPtr<Instances::fl::Array> > TFunc_Instances_Scene_labelsGet;
typedef ThunkFunc0<Instances::fl_display::Scene, Instances::fl_display::Scene::mid_nameGet, ASString> TFunc_Instances_Scene_nameGet;
typedef ThunkFunc0<Instances::fl_display::Scene, Instances::fl_display::Scene::mid_numFramesGet, SInt32> TFunc_Instances_Scene_numFramesGet;

template <> const TFunc_Instances_Scene_labelsGet::TMethod TFunc_Instances_Scene_labelsGet::Method = &Instances::fl_display::Scene::labelsGet;
template <> const TFunc_Instances_Scene_nameGet::TMethod TFunc_Instances_Scene_nameGet::Method = &Instances::fl_display::Scene::nameGet;
template <> const TFunc_Instances_Scene_numFramesGet::TMethod TFunc_Instances_Scene_numFramesGet::Method = &Instances::fl_display::Scene::numFramesGet;

namespace Instances { namespace fl_display
{
    Scene::Scene(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
//##protect##"instance::Scene::Scene()$data"
    , SceneInfo(NULL)
//##protect##"instance::Scene::Scene()$data"
    {
//##protect##"instance::Scene::Scene()$code"
//##protect##"instance::Scene::Scene()$code"
    }

    void Scene::labelsGet(SPtr<Instances::fl::Array>& result)
    {
//##protect##"instance::Scene::labelsGet()"
        SF_UNUSED1(result);
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        // Create an array
        result = GetVM().MakeArray();

        Class* frameLabelClassVal = GetVM().GetClass("flash.display.FrameLabel", GetVM().GetCurrentAppDomain());

        if (SceneInfo)
        {
            UPInt cnt = SceneInfo->Labels.GetSize();
            for (UPInt i = 0; i < cnt; ++i)
            {
                // create a FrameLabel object

                SPtr<Instances::fl_display::FrameLabel> frameLabel;
                asvm.ConstructInstance(frameLabel, frameLabelClassVal, 0);

                // populate frameLabel
                frameLabel->Set(SceneInfo->Offset, &SceneInfo->Labels[i]);

                result->PushBack(Value(frameLabel));
            }
        }
        else
        {
            SF_ASSERT(SpriteObj);
            unsigned cnt = SpriteObj->GetFrameCount();
            Scaleform::Array<String> labels;
            for (unsigned i = 0; i < cnt; ++i)
            {
                // not very efficient way of getting labels from the sprite;
                // however, hopefully, it is not critical.
                if (SpriteObj->GetDef()->GetFrameLabels(i, &labels))
                {
                    for (UPInt j = 0, nj = labels.GetSize(); j < nj; ++j)
                    {
                        // create a FrameLabel object
                        SPtr<Instances::fl_display::FrameLabel> frameLabel;
                        asvm.ConstructInstance(frameLabel, frameLabelClassVal, 0);

                        // populate frameLabel
                        frameLabel->Set(i + 1, asvm.GetStringManager().CreateString(labels[j].ToCStr()));

                        result->PushBack(Value(frameLabel));
                    }
                    labels.Clear();
                }
            }
        }
//##protect##"instance::Scene::labelsGet()"
    }
    void Scene::nameGet(ASString& result)
    {
//##protect##"instance::Scene::nameGet()"
        SF_UNUSED1(result);
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        if (SceneInfo)
            result = SceneInfo->Name;
        else
            result = asvm.GetStringManager().CreateEmptyString();
//##protect##"instance::Scene::nameGet()"
    }
    void Scene::numFramesGet(SInt32& result)
    {
//##protect##"instance::Scene::numFramesGet()"
        SF_UNUSED1(result);
        if (SceneInfo)
            result = (SInt32)SceneInfo->NumFrames;
        else
        {
            SF_ASSERT(SpriteObj);
            result = SpriteObj->GetFrameCount();
        }
//##protect##"instance::Scene::numFramesGet()"
    }

    SPtr<Instances::fl::Array> Scene::labelsGet()
    {
        SPtr<Instances::fl::Array> result;
        labelsGet(result);
        return result;
    }
//##protect##"instance$methods"
    void Scene::Set(const MovieDataDef::SceneInfo* sceneInfo)
    {
        SceneInfo = sceneInfo;
    }
    void Scene::Set(GFx::Sprite* spr)
    {
        SpriteObj = spr;
    }

//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{
    // const UInt16 Scene::tito[Scene::ThunkInfoNum] = {
    //    0, 1, 2, 
    // };
    const TypeInfo* Scene::tit[3] = {
        &AS3::fl::ArrayTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::int_TI, 
    };
    const ThunkInfo Scene::ti[Scene::ThunkInfoNum] = {
        {TFunc_Instances_Scene_labelsGet::Func, &Scene::tit[0], "labels", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Scene_nameGet::Func, &Scene::tit[1], "name", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Scene_numFramesGet::Func, &Scene::tit[2], "numFrames", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
    };

    Scene::Scene(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::Scene::Scene()"
//##protect##"InstanceTraits::Scene::Scene()"

    }

    void Scene::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<Scene&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_display
{

    Scene::Scene(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::Scene::Scene()"
//##protect##"ClassTraits::Scene::Scene()"

    }

    Pickable<Traits> Scene::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) Scene(vm, AS3::fl_display::SceneCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::SceneCI));
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
    const TypeInfo SceneTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_display::Scene::InstanceType),
        0,
        0,
        InstanceTraits::fl_display::Scene::ThunkInfoNum,
        0,
        "Scene", "flash.display", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo SceneCI = {
        &SceneTI,
        ClassTraits::fl_display::Scene::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_display::Scene::ti,
        NULL,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

