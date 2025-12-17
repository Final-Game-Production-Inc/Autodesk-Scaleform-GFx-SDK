//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_SimpleButton.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Display_SimpleButton.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../Media/AS3_Obj_Media_SoundTransform.h"
#include "GFx/AS3/AS3_MovieRoot.h"
#include "GFx/GFx_CharacterDef.h"
#include "GFx/GFx_Resource.h"
#include "GFx/AS3/AS3_AvmButton.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_display::SimpleButton, Instances::fl_display::SimpleButton::mid_downStateGet, SPtr<Instances::fl_display::DisplayObject> > TFunc_Instances_SimpleButton_downStateGet;
typedef ThunkFunc1<Instances::fl_display::SimpleButton, Instances::fl_display::SimpleButton::mid_downStateSet, const Value, Instances::fl_display::DisplayObject*> TFunc_Instances_SimpleButton_downStateSet;
typedef ThunkFunc0<Instances::fl_display::SimpleButton, Instances::fl_display::SimpleButton::mid_enabledGet, bool> TFunc_Instances_SimpleButton_enabledGet;
typedef ThunkFunc1<Instances::fl_display::SimpleButton, Instances::fl_display::SimpleButton::mid_enabledSet, const Value, bool> TFunc_Instances_SimpleButton_enabledSet;
typedef ThunkFunc0<Instances::fl_display::SimpleButton, Instances::fl_display::SimpleButton::mid_hitTestStateGet, SPtr<Instances::fl_display::DisplayObject> > TFunc_Instances_SimpleButton_hitTestStateGet;
typedef ThunkFunc1<Instances::fl_display::SimpleButton, Instances::fl_display::SimpleButton::mid_hitTestStateSet, const Value, Instances::fl_display::DisplayObject*> TFunc_Instances_SimpleButton_hitTestStateSet;
typedef ThunkFunc0<Instances::fl_display::SimpleButton, Instances::fl_display::SimpleButton::mid_overStateGet, SPtr<Instances::fl_display::DisplayObject> > TFunc_Instances_SimpleButton_overStateGet;
typedef ThunkFunc1<Instances::fl_display::SimpleButton, Instances::fl_display::SimpleButton::mid_overStateSet, const Value, Instances::fl_display::DisplayObject*> TFunc_Instances_SimpleButton_overStateSet;
typedef ThunkFunc0<Instances::fl_display::SimpleButton, Instances::fl_display::SimpleButton::mid_soundTransformGet, SPtr<Instances::fl_media::SoundTransform> > TFunc_Instances_SimpleButton_soundTransformGet;
typedef ThunkFunc1<Instances::fl_display::SimpleButton, Instances::fl_display::SimpleButton::mid_soundTransformSet, const Value, Instances::fl_media::SoundTransform*> TFunc_Instances_SimpleButton_soundTransformSet;
typedef ThunkFunc0<Instances::fl_display::SimpleButton, Instances::fl_display::SimpleButton::mid_trackAsMenuGet, bool> TFunc_Instances_SimpleButton_trackAsMenuGet;
typedef ThunkFunc1<Instances::fl_display::SimpleButton, Instances::fl_display::SimpleButton::mid_trackAsMenuSet, const Value, bool> TFunc_Instances_SimpleButton_trackAsMenuSet;
typedef ThunkFunc0<Instances::fl_display::SimpleButton, Instances::fl_display::SimpleButton::mid_upStateGet, SPtr<Instances::fl_display::DisplayObject> > TFunc_Instances_SimpleButton_upStateGet;
typedef ThunkFunc1<Instances::fl_display::SimpleButton, Instances::fl_display::SimpleButton::mid_upStateSet, const Value, Instances::fl_display::DisplayObject*> TFunc_Instances_SimpleButton_upStateSet;
typedef ThunkFunc0<Instances::fl_display::SimpleButton, Instances::fl_display::SimpleButton::mid_useHandCursorGet, bool> TFunc_Instances_SimpleButton_useHandCursorGet;
typedef ThunkFunc1<Instances::fl_display::SimpleButton, Instances::fl_display::SimpleButton::mid_useHandCursorSet, const Value, bool> TFunc_Instances_SimpleButton_useHandCursorSet;

template <> const TFunc_Instances_SimpleButton_downStateGet::TMethod TFunc_Instances_SimpleButton_downStateGet::Method = &Instances::fl_display::SimpleButton::downStateGet;
template <> const TFunc_Instances_SimpleButton_downStateSet::TMethod TFunc_Instances_SimpleButton_downStateSet::Method = &Instances::fl_display::SimpleButton::downStateSet;
template <> const TFunc_Instances_SimpleButton_enabledGet::TMethod TFunc_Instances_SimpleButton_enabledGet::Method = &Instances::fl_display::SimpleButton::enabledGet;
template <> const TFunc_Instances_SimpleButton_enabledSet::TMethod TFunc_Instances_SimpleButton_enabledSet::Method = &Instances::fl_display::SimpleButton::enabledSet;
template <> const TFunc_Instances_SimpleButton_hitTestStateGet::TMethod TFunc_Instances_SimpleButton_hitTestStateGet::Method = &Instances::fl_display::SimpleButton::hitTestStateGet;
template <> const TFunc_Instances_SimpleButton_hitTestStateSet::TMethod TFunc_Instances_SimpleButton_hitTestStateSet::Method = &Instances::fl_display::SimpleButton::hitTestStateSet;
template <> const TFunc_Instances_SimpleButton_overStateGet::TMethod TFunc_Instances_SimpleButton_overStateGet::Method = &Instances::fl_display::SimpleButton::overStateGet;
template <> const TFunc_Instances_SimpleButton_overStateSet::TMethod TFunc_Instances_SimpleButton_overStateSet::Method = &Instances::fl_display::SimpleButton::overStateSet;
template <> const TFunc_Instances_SimpleButton_soundTransformGet::TMethod TFunc_Instances_SimpleButton_soundTransformGet::Method = &Instances::fl_display::SimpleButton::soundTransformGet;
template <> const TFunc_Instances_SimpleButton_soundTransformSet::TMethod TFunc_Instances_SimpleButton_soundTransformSet::Method = &Instances::fl_display::SimpleButton::soundTransformSet;
template <> const TFunc_Instances_SimpleButton_trackAsMenuGet::TMethod TFunc_Instances_SimpleButton_trackAsMenuGet::Method = &Instances::fl_display::SimpleButton::trackAsMenuGet;
template <> const TFunc_Instances_SimpleButton_trackAsMenuSet::TMethod TFunc_Instances_SimpleButton_trackAsMenuSet::Method = &Instances::fl_display::SimpleButton::trackAsMenuSet;
template <> const TFunc_Instances_SimpleButton_upStateGet::TMethod TFunc_Instances_SimpleButton_upStateGet::Method = &Instances::fl_display::SimpleButton::upStateGet;
template <> const TFunc_Instances_SimpleButton_upStateSet::TMethod TFunc_Instances_SimpleButton_upStateSet::Method = &Instances::fl_display::SimpleButton::upStateSet;
template <> const TFunc_Instances_SimpleButton_useHandCursorGet::TMethod TFunc_Instances_SimpleButton_useHandCursorGet::Method = &Instances::fl_display::SimpleButton::useHandCursorGet;
template <> const TFunc_Instances_SimpleButton_useHandCursorSet::TMethod TFunc_Instances_SimpleButton_useHandCursorSet::Method = &Instances::fl_display::SimpleButton::useHandCursorSet;

namespace Instances { namespace fl_display
{
    SimpleButton::SimpleButton(InstanceTraits::Traits& t)
    : Instances::fl_display::InteractiveObject(t)
//##protect##"instance::SimpleButton::SimpleButton()$data"
//##protect##"instance::SimpleButton::SimpleButton()$data"
    {
//##protect##"instance::SimpleButton::SimpleButton()$code"
//##protect##"instance::SimpleButton::SimpleButton()$code"
    }

    void SimpleButton::downStateGet(SPtr<Instances::fl_display::DisplayObject>& result)
    {
//##protect##"instance::SimpleButton::downStateGet()"
        result = DownStateObject;
//##protect##"instance::SimpleButton::downStateGet()"
    }
    void SimpleButton::downStateSet(const Value& result, Instances::fl_display::DisplayObject* value)
    {
//##protect##"instance::SimpleButton::downStateSet()"
        SF_UNUSED(result);
        GetAvmButton()->SetDownStateObject(value->pDispObj);
        DownStateObject = value;
//##protect##"instance::SimpleButton::downStateSet()"
    }
    void SimpleButton::enabledGet(bool& result)
    {
//##protect##"instance::SimpleButton::enabledGet()"
        SF_ASSERT(pDispObj && pDispObj->IsInteractiveObject());
        result = pDispObj->CharToInteractiveObject_Unsafe()->IsEnabledFlagSet();
//##protect##"instance::SimpleButton::enabledGet()"
    }
    void SimpleButton::enabledSet(const Value& result, bool value)
    {
//##protect##"instance::SimpleButton::enabledSet()"
        SF_UNUSED1(result);
        SF_ASSERT(pDispObj && pDispObj->IsInteractiveObject());

        pDispObj->CharToInteractiveObject_Unsafe()->SetEnabledFlag(value);
//##protect##"instance::SimpleButton::enabledSet()"
    }
    void SimpleButton::hitTestStateGet(SPtr<Instances::fl_display::DisplayObject>& result)
    {
//##protect##"instance::SimpleButton::hitTestStateGet()"
        result = HitTestStateObject;
//##protect##"instance::SimpleButton::hitTestStateGet()"
    }
    void SimpleButton::hitTestStateSet(const Value& result, Instances::fl_display::DisplayObject* value)
    {
//##protect##"instance::SimpleButton::hitTestStateSet()"
        SF_UNUSED(result);
        GetAvmButton()->SetHitStateObject(value->pDispObj);
        HitTestStateObject = value;
//##protect##"instance::SimpleButton::hitTestStateSet()"
    }
    void SimpleButton::overStateGet(SPtr<Instances::fl_display::DisplayObject>& result)
    {
//##protect##"instance::SimpleButton::overStateGet()"
        SF_UNUSED1(result);
        result = OverStateObject;
//##protect##"instance::SimpleButton::overStateGet()"
    }
    void SimpleButton::overStateSet(const Value& result, Instances::fl_display::DisplayObject* value)
    {
//##protect##"instance::SimpleButton::overStateSet()"
        SF_UNUSED(result);
        GetAvmButton()->SetOverStateObject(value->pDispObj);
        OverStateObject = value;
//##protect##"instance::SimpleButton::overStateSet()"
    }
    void SimpleButton::soundTransformGet(SPtr<Instances::fl_media::SoundTransform>& result)
    {
//##protect##"instance::SimpleButton::soundTransformGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("SimpleButton::soundTransformGet()");
//##protect##"instance::SimpleButton::soundTransformGet()"
    }
    void SimpleButton::soundTransformSet(const Value& result, Instances::fl_media::SoundTransform* value)
    {
//##protect##"instance::SimpleButton::soundTransformSet()"
        SF_UNUSED2(result, value);
        WARN_NOT_IMPLEMENTED("SimpleButton::soundTransformSet()");
//##protect##"instance::SimpleButton::soundTransformSet()"
    }
    void SimpleButton::trackAsMenuGet(bool& result)
    {
//##protect##"instance::SimpleButton::trackAsMenuGet()"
        SF_UNUSED1(result);
        SF_ASSERT(pDispObj && pDispObj->IsInteractiveObject());
        GFx::InteractiveObject* spr = pDispObj->CharToInteractiveObject_Unsafe();
        SF_ASSERT(spr);
        result = spr->GetTrackAsMenu();
//##protect##"instance::SimpleButton::trackAsMenuGet()"
    }
    void SimpleButton::trackAsMenuSet(const Value& result, bool value)
    {
//##protect##"instance::SimpleButton::trackAsMenuSet()"
        SF_UNUSED1(result);
        SF_ASSERT(pDispObj && pDispObj->IsInteractiveObject());
        GFx::InteractiveObject* spr = pDispObj->CharToInteractiveObject_Unsafe();
        SF_ASSERT(spr);
        spr->SetTrackAsMenuFlag(value);
//##protect##"instance::SimpleButton::trackAsMenuSet()"
    }
    void SimpleButton::upStateGet(SPtr<Instances::fl_display::DisplayObject>& result)
    {
//##protect##"instance::SimpleButton::upStateGet()"
        result = UpStateObject;
//##protect##"instance::SimpleButton::upStateGet()"
    }
    void SimpleButton::upStateSet(const Value& result, Instances::fl_display::DisplayObject* value)
    {
//##protect##"instance::SimpleButton::upStateSet()"
        SF_UNUSED(result);
        GetAvmButton()->SetUpStateObject(value->pDispObj);
        UpStateObject = value;
//##protect##"instance::SimpleButton::upStateSet()"
    }
    void SimpleButton::useHandCursorGet(bool& result)
    {
//##protect##"instance::SimpleButton::useHandCursorGet()"
        SF_UNUSED1(result);
        SF_ASSERT(pDispObj && pDispObj->IsInteractiveObject());
        result = pDispObj->CharToInteractiveObject_Unsafe()->IsUseHandCursorFlagTrue();
//##protect##"instance::SimpleButton::useHandCursorGet()"
    }
    void SimpleButton::useHandCursorSet(const Value& result, bool value)
    {
//##protect##"instance::SimpleButton::useHandCursorSet()"
        SF_UNUSED2(result, value);
        SF_ASSERT(pDispObj && pDispObj->IsInteractiveObject());
        pDispObj->CharToInteractiveObject_Unsafe()->SetUseHandCursorFlag(value);
//##protect##"instance::SimpleButton::useHandCursorSet()"
    }

    SPtr<Instances::fl_display::DisplayObject> SimpleButton::downStateGet()
    {
        SPtr<Instances::fl_display::DisplayObject> result;
        downStateGet(result);
        return result;
    }
    SPtr<Instances::fl_display::DisplayObject> SimpleButton::hitTestStateGet()
    {
        SPtr<Instances::fl_display::DisplayObject> result;
        hitTestStateGet(result);
        return result;
    }
    SPtr<Instances::fl_display::DisplayObject> SimpleButton::overStateGet()
    {
        SPtr<Instances::fl_display::DisplayObject> result;
        overStateGet(result);
        return result;
    }
    SPtr<Instances::fl_media::SoundTransform> SimpleButton::soundTransformGet()
    {
        SPtr<Instances::fl_media::SoundTransform> result;
        soundTransformGet(result);
        return result;
    }
    SPtr<Instances::fl_display::DisplayObject> SimpleButton::upStateGet()
    {
        SPtr<Instances::fl_display::DisplayObject> result;
        upStateGet(result);
        return result;
    }
//##protect##"instance$methods"
    AvmButton* SimpleButton::GetAvmButton() const
    {
        SF_ASSERT(pDispObj);
        return static_cast<AvmButton*>(ToAvmDisplayObj(pDispObj));
    }

    void SimpleButton::AS3Constructor(unsigned argc, const Value* argv)
    {
        if (argc >= 1)
        {
            if (GetVM().IsOfType(argv[0], "flash.display.DisplayObject", GetVM().GetCurrentAppDomain()))
            {
                DisplayObject* value = static_cast<DisplayObject*>(argv[0].GetObject());
                GetAvmButton()->SetUpStateObject(value->pDispObj);
            }
        }
        if (argc >= 2)
        {
            if (GetVM().IsOfType(argv[1], "flash.display.DisplayObject", GetVM().GetCurrentAppDomain()))
            {
                DisplayObject* value = static_cast<DisplayObject*>(argv[1].GetObject());
                GetAvmButton()->SetOverStateObject(value->pDispObj);
            }
        }
        if (argc >= 3)
        {
            if (GetVM().IsOfType(argv[2], "flash.display.DisplayObject", GetVM().GetCurrentAppDomain()))
            {
                DisplayObject* value = static_cast<DisplayObject*>(argv[2].GetObject());
                GetAvmButton()->SetDownStateObject(value->pDispObj);
            }
        }
        if (argc >= 4)
        {
            if (GetVM().IsOfType(argv[3], "flash.display.DisplayObject", GetVM().GetCurrentAppDomain()))
            {
                DisplayObject* value = static_cast<DisplayObject*>(argv[3].GetObject());
                GetAvmButton()->SetHitStateObject(value->pDispObj);
            }
        }
    }

    void SimpleButton::InitInstance(bool extCall)
    {
        if (!extCall)
            SimpleButton::CreateStageObject();
    }

    GFx::DisplayObject* SimpleButton::CreateStageObject()
    {
        if (!pDispObj)
        {
            ASVM& asvm = static_cast<ASVM&>(GetVM());
            MovieRoot* proot = static_cast<const ASVM&>(GetVM()).GetMovieRoot();
            MovieDefImpl* pdefImpl = asvm.GetResourceMovieDef(this);

            if (pdefImpl)
            {
                // it is possible that there is no corresponding stage object
                // for the class; use empty button then.
                CharacterCreateInfo ccinfo;
                ccinfo.pCharDef     = NULL;

                FindLibarySymbol(&ccinfo, pdefImpl);

                if (!ccinfo.pCharDef)
                {
                    ccinfo = pdefImpl->GetCharacterCreateInfo(ResourceId(CharacterDef::CharId_EmptyButton));
                    
                    if (GetVM().GetCallStack().GetSize() > 0)
                    {
                        // figure out MovieDefImpl from current call frame.
                        const VMAbcFile& abcFile = GetVM().GetCurrCallFrame().GetFile();
                        ccinfo.pBindDefImpl = static_cast<const ASVM::AbcFileWithMovieDef&>(abcFile.GetAbcFile()).pDefImpl;
                    }
                    else
                        ccinfo.pBindDefImpl = pdefImpl;
                }

                SF_ASSERT(ccinfo.pCharDef);
                pDispObj = *static_cast<GFx::DisplayObject*>(proot->GetASSupport()->CreateCharacterInstance(
                    proot->GetMovieImpl(), ccinfo, NULL, 
                    ResourceId(), CharacterDef::Button));
                AvmDisplayObj* pAvmObj = ToAvmDisplayObj(pDispObj);
                pAvmObj->AssignAS3Obj(this);
                pAvmObj->SetAppDomain(GetInstanceTraits().GetAppDomain());
                static_cast<Button*>(pDispObj.GetPtr())->CreateCharacters();
            }
        }
        return pDispObj;
    }
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{
    // const UInt16 SimpleButton::tito[SimpleButton::ThunkInfoNum] = {
    //    0, 1, 3, 4, 6, 7, 9, 10, 12, 13, 15, 16, 18, 19, 21, 22, 
    // };
    const TypeInfo* SimpleButton::tit[24] = {
        &AS3::fl_display::DisplayObjectTI, 
        NULL, &AS3::fl_display::DisplayObjectTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl_display::DisplayObjectTI, 
        NULL, &AS3::fl_display::DisplayObjectTI, 
        &AS3::fl_display::DisplayObjectTI, 
        NULL, &AS3::fl_display::DisplayObjectTI, 
        &AS3::fl_media::SoundTransformTI, 
        NULL, &AS3::fl_media::SoundTransformTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl_display::DisplayObjectTI, 
        NULL, &AS3::fl_display::DisplayObjectTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
    };
    const ThunkInfo SimpleButton::ti[SimpleButton::ThunkInfoNum] = {
        {TFunc_Instances_SimpleButton_downStateGet::Func, &SimpleButton::tit[0], "downState", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_SimpleButton_downStateSet::Func, &SimpleButton::tit[1], "downState", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_SimpleButton_enabledGet::Func, &SimpleButton::tit[3], "enabled", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_SimpleButton_enabledSet::Func, &SimpleButton::tit[4], "enabled", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_SimpleButton_hitTestStateGet::Func, &SimpleButton::tit[6], "hitTestState", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_SimpleButton_hitTestStateSet::Func, &SimpleButton::tit[7], "hitTestState", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_SimpleButton_overStateGet::Func, &SimpleButton::tit[9], "overState", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_SimpleButton_overStateSet::Func, &SimpleButton::tit[10], "overState", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_SimpleButton_soundTransformGet::Func, &SimpleButton::tit[12], "soundTransform", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_SimpleButton_soundTransformSet::Func, &SimpleButton::tit[13], "soundTransform", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_SimpleButton_trackAsMenuGet::Func, &SimpleButton::tit[15], "trackAsMenu", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_SimpleButton_trackAsMenuSet::Func, &SimpleButton::tit[16], "trackAsMenu", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_SimpleButton_upStateGet::Func, &SimpleButton::tit[18], "upState", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_SimpleButton_upStateSet::Func, &SimpleButton::tit[19], "upState", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_SimpleButton_useHandCursorGet::Func, &SimpleButton::tit[21], "useHandCursor", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_SimpleButton_useHandCursorSet::Func, &SimpleButton::tit[22], "useHandCursor", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
    };

    SimpleButton::SimpleButton(VM& vm, const ClassInfo& ci)
    : fl_display::InteractiveObject(vm, ci)
    {
//##protect##"InstanceTraits::SimpleButton::SimpleButton()"
        SetTraitsType(Traits_SimpleButton);
//##protect##"InstanceTraits::SimpleButton::SimpleButton()"

    }

    void SimpleButton::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<SimpleButton&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_display
{

    SimpleButton::SimpleButton(VM& vm, const ClassInfo& ci)
    : fl_display::InteractiveObject(vm, ci)
    {
//##protect##"ClassTraits::SimpleButton::SimpleButton()"
        SetTraitsType(Traits_SimpleButton);
//##protect##"ClassTraits::SimpleButton::SimpleButton()"

    }

    Pickable<Traits> SimpleButton::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) SimpleButton(vm, AS3::fl_display::SimpleButtonCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::SimpleButtonCI));
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
    const TypeInfo SimpleButtonTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_display::SimpleButton::InstanceType),
        0,
        0,
        InstanceTraits::fl_display::SimpleButton::ThunkInfoNum,
        0,
        "SimpleButton", "flash.display", &fl_display::InteractiveObjectTI,
        TypeInfo::None
    };

    const ClassInfo SimpleButtonCI = {
        &SimpleButtonTI,
        ClassTraits::fl_display::SimpleButton::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_display::SimpleButton::ti,
        NULL,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

