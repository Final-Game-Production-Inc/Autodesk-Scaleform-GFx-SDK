//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Gfx_FocusManager.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy, Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Gfx_FocusManager.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "GFx/AS3/AS3_MovieRoot.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Classes { namespace fl_gfx
{
    FocusManager::FocusManager(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::FocusManager::FocusManager()"
//##protect##"class_::FocusManager::FocusManager()"
    }
    void FocusManager::alwaysEnableArrowKeysSet(const Value& result, bool enable)
    {
//##protect##"class_::FocusManager::alwaysEnableArrowKeysSet()"

        SF_UNUSED(result);
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        if (asvm.ExtensionsEnabled)
            asvm.GetMovieImpl()->SetAlwaysEnableFocusArrowKeys(enable);

//##protect##"class_::FocusManager::alwaysEnableArrowKeysSet()"
    }
    void FocusManager::alwaysEnableArrowKeysGet(bool& result)
    {
//##protect##"class_::FocusManager::alwaysEnableArrowKeysGet()"

        ASVM& asvm = static_cast<ASVM&>(GetVM());
        if (!asvm.ExtensionsEnabled) return;
        MovieImpl* proot = asvm.GetMovieImpl();
        if (proot->IsAlwaysEnableFocusArrowKeysSet())
            result = proot->IsAlwaysEnableFocusArrowKeys();
        else
            result = false;

//##protect##"class_::FocusManager::alwaysEnableArrowKeysGet()"
    }
    void FocusManager::disableFocusKeysSet(const Value& result, bool disable)
    {
//##protect##"class_::FocusManager::disableFocusKeysSet()"

        SF_UNUSED(result);
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        if (asvm.ExtensionsEnabled)
            asvm.GetMovieImpl()->SetDisableFocusKeys(disable);

//##protect##"class_::FocusManager::disableFocusKeysSet()"
    }
    void FocusManager::disableFocusKeysGet(bool& result)
    {
//##protect##"class_::FocusManager::disableFocusKeysGet()"

        ASVM& asvm = static_cast<ASVM&>(GetVM());
        if (!asvm.ExtensionsEnabled) return;
        MovieImpl* proot = asvm.GetMovieImpl();
        if (proot->IsDisableFocusKeysSet())
            result = proot->IsDisableFocusKeys();
        else
            result = false;

//##protect##"class_::FocusManager::disableFocusKeysGet()"
    }
    void FocusManager::moveFocus(SPtr<Instances::fl_display::InteractiveObject>& result, const ASString& keyToSimulate, Instances::fl_display::InteractiveObject* startFromMovie, bool includeFocusEnabledChars, UInt32 controllerIdx)
    {
//##protect##"class_::FocusManager::moveFocus()"

        ASVM& asvm = static_cast<ASVM&>(GetVM());
        if (asvm.ExtensionsEnabled)
        {
//             result.SetUndefined();            

            MovieImpl* proot = asvm.GetMovieImpl();
            Ptr<GFx::InteractiveObject> startChar;
            if (startFromMovie)
            {
                startChar = startFromMovie->GetIntObj();
            }
            else
                startChar = proot->GetFocusedCharacter(controllerIdx).GetPtr();

            short keycode;
            KeyModifiers kmods;
            if (keyToSimulate == "up")
                keycode = Key::Up;
            else if (keyToSimulate == "down")
                keycode = Key::Down;
            else if (keyToSimulate == "left")
                keycode = Key::Left;
            else if (keyToSimulate == "right")
                keycode = Key::Right;
            else if (keyToSimulate == "tab")
                keycode = Key::Tab;
            else if (keyToSimulate == "shifttab")
            {
                keycode = Key::Tab;
                kmods.SetShiftPressed(true);
            }
            else
            {                
                if (asvm.GetLog())
                    asvm.GetLog()->LogWarning("moveFocus - invalid string id for key: '%s'",
                    keyToSimulate.ToCStr());
                return;
            }
            ProcessFocusKeyInfo focusInfo;
            InputEventsQueue::QueueEntry::KeyEntry ke;
            ke.KeyboardIndex            = (UInt8)controllerIdx;
            ke.Code                     = keycode;
            ke.KeysState                = kmods.States;
            proot->InitFocusKeyInfo(&focusInfo, ke, includeFocusEnabledChars);
            focusInfo.CurFocused        = startChar;
            focusInfo.ManualFocus       = true;
            proot->ProcessFocusKey(GFx::Event::KeyDown, ke, &focusInfo);
            proot->FinalizeProcessFocusKey(&focusInfo);

            result = ToAvmInteractiveObj(focusInfo.CurFocused)->GetAS3IntObj();
        }

//##protect##"class_::FocusManager::moveFocus()"
    }
    void FocusManager::findFocus(SPtr<Instances::fl_display::InteractiveObject>& result, const ASString& keyToSimulate, Instances::fl_display::DisplayObjectContainer* parentMovie, bool loop, Instances::fl_display::InteractiveObject* startFromMovie, bool includeFocusEnabledChars, UInt32 controllerIdx)
    {
//##protect##"class_::FocusManager::findFocus()"

    ASVM& asvm = static_cast<ASVM&>(GetVM());
    if (asvm.ExtensionsEnabled)
    {
//         result.SetUndefined();

        MovieImpl* proot = asvm.GetMovieImpl();
        Ptr<GFx::InteractiveObject> startChar;
        if (startFromMovie)
        {
            startChar = startFromMovie->GetIntObj();
        }
        else
            startChar = proot->GetFocusedCharacter(controllerIdx).GetPtr();

        short keycode;
        KeyModifiers kmods;
        if (keyToSimulate == "up")
            keycode = Key::Up;
        else if (keyToSimulate == "down")
            keycode = Key::Down;
        else if (keyToSimulate == "left")
            keycode = Key::Left;
        else if (keyToSimulate == "right")
            keycode = Key::Right;
        else if (keyToSimulate == "tab")
            keycode = Key::Tab;
        else if (keyToSimulate == "shifttab")
        {
            keycode = Key::Tab;
            kmods.SetShiftPressed(true);
        }
        else
        {
            /* // PPS: Warning causes unnecessary spew in log with CLIK. CLIK's FocusHandler
               // passes all unhandled key events to findFocus.
            if (fn.Env->GetLog())
                fn.Env->GetLog()->LogWarning("findFocus - invalid string id for key: '%s'",
                directionStr.ToCStr());
            */
            return;
        }

        Ptr<GFx::InteractiveObject> panelChar;
        if (parentMovie)
        {
            panelChar = parentMovie->GetIntObj();
        }
        else
        {
            // take a modal clip as panelChar, if the parameter is "null"
            panelChar = proot->GetModalClip(controllerIdx);
        }

        ProcessFocusKeyInfo focusInfo;
        InputEventsQueue::QueueEntry::KeyEntry ke;
        ke.KeyboardIndex            = (UInt8)controllerIdx;
        ke.Code                     = keycode;
        ke.KeysState                = kmods.States;
        FocusGroupDescr focusGroup(asvm.GetHeap());
        focusGroup.ModalClip        = (panelChar) ? panelChar->GetCharacterHandle() : NULL;
        focusGroup.LastFocused      = startChar;
        proot->InitFocusKeyInfo(&focusInfo, ke, includeFocusEnabledChars, &focusGroup);
        focusInfo.ManualFocus       = true;
        proot->ProcessFocusKey(GFx::Event::KeyDown, ke, &focusInfo);
        if (focusInfo.CurFocused && focusInfo.CurFocused != startChar)
        {
            result = ToAvmInteractiveObj(focusInfo.CurFocused)->GetAS3IntObj();
        }
        else
        {
            if (!loop || focusGroup.TabableArray.GetSize() == 0)
            {
                result = NULL;
            }
            else
            {
                if (keycode == Key::Tab)
                {
                    if (!kmods.IsShiftPressed())
                    {
                        result = ToAvmInteractiveObj(focusGroup.TabableArray[0])->GetAS3IntObj();
                    }
                    else
                    {
                        result = ToAvmInteractiveObj(focusGroup.TabableArray[focusGroup.TabableArray.GetSize()-1])->GetAS3IntObj();
                    }
                }
                else
                {
                    float lastV = SF_MIN_FLOAT;
                    UPInt index = 0;
                    for (UPInt i = 0, n = focusGroup.TabableArray.GetSize(); i < n; ++i)
                    {
                        Ptr<GFx::InteractiveObject> b = focusGroup.TabableArray[i];
                        if ((!focusInfo.InclFocusEnabled && !b->IsTabable()) ||
                            !b->IsFocusAllowed(proot, focusInfo.KeyboardIndex))
                        {
                            // If this is not for manual focus and not tabable - ignore.
                            continue;
                        }
                        GFx::DisplayObject::Matrix mb = b->GetLevelMatrix();
                        RectF bRect  = mb.EncloseTransform(b->GetFocusRect());
                        switch (keycode)
                        {
                        case Key::Down: 
                            // find top
                            if (lastV == SF_MIN_FLOAT || bRect.y1 < lastV)
                            {
                                lastV = bRect.y1;
                                index = i;
                            }
                            break; //
                        case Key::Up: 
                            // find bottom
                            if (lastV == SF_MIN_FLOAT || bRect.y2 > lastV)
                            {
                                lastV = bRect.y2;
                                index = i;
                            }
                            break; //
                        case Key::Left: 
                            // find most right
                            if (lastV == SF_MIN_FLOAT || bRect.x2 > lastV)
                            {
                                lastV = bRect.x2;
                                index = i;
                            }
                            break; //
                        case Key::Right: 
                            // find most left
                            if (lastV == SF_MIN_FLOAT || bRect.x1 < lastV)
                            {
                                lastV = bRect.x1;
                                index = i;
                            }
                            break; //
                        default: SF_ASSERT(0);
                        }
                    }
                    result = ToAvmInteractiveObj(focusGroup.TabableArray[index])->GetAS3IntObj();
                }
            }
        }
    }

//##protect##"class_::FocusManager::findFocus()"
    }
    void FocusManager::setFocus(const Value& result, Instances::fl_display::InteractiveObject* obj, UInt32 controllerIdx)
    {
//##protect##"class_::FocusManager::setFocus()"

        SF_UNUSED(result);
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        Ptr<GFx::InteractiveObject> newFocus;
        if (obj)
            newFocus = obj->GetIntObj();
        Ptr<GFx::InteractiveObject> focused = asvm.GetMovieImpl()->GetFocusedCharacter(controllerIdx);
        if (focused != newFocus)
            asvm.GetMovieImpl()->SetKeyboardFocusTo(newFocus, controllerIdx, GFx_FocusMovedByAS); 

//##protect##"class_::FocusManager::setFocus()"
    }
    void FocusManager::getFocus(SPtr<Instances::fl_display::InteractiveObject>& result, UInt32 controllerIdx)
    {
//##protect##"class_::FocusManager::getFocus()"

        ASVM& asvm = static_cast<ASVM&>(GetVM());
        Ptr<GFx::InteractiveObject> focused = asvm.GetMovieImpl()->GetFocusedCharacter(controllerIdx);
        if (!focused)
            result = NULL;
        else
        {
            Instances::fl_display::DisplayObject* ido = ToAvmInteractiveObj(focused)->GetAS3Obj();

            if (ido && IsInteractiveObject(ido->GetTraitsType()))
                result = static_cast<Instances::fl_display::InteractiveObject*>(ido);
            else
                result = NULL;
        }

//##protect##"class_::FocusManager::getFocus()"
    }
    void FocusManager::numFocusGroupsGet(UInt32& result)
    {
//##protect##"class_::FocusManager::numFocusGroupsGet()"

        ASVM& asvm = static_cast<ASVM&>(GetVM());
        MovieImpl* proot = asvm.GetMovieImpl();
        result = proot->GetFocusGroupCount();

//##protect##"class_::FocusManager::numFocusGroupsGet()"
    }
    void FocusManager::setFocusGroupMask(const Value& result, Instances::fl_display::InteractiveObject* obj, UInt32 mask)
    {
//##protect##"class_::FocusManager::setFocusGroupMask()"

        SF_UNUSED(result);
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        if (asvm.ExtensionsEnabled)
        {
            if (obj) 
                obj->GetIntObj()->PropagateFocusGroupMask(mask);
        }

//##protect##"class_::FocusManager::setFocusGroupMask()"
    }
    void FocusManager::getFocusGroupMask(UInt32& result, Instances::fl_display::InteractiveObject* obj)
    {
//##protect##"class_::FocusManager::getFocusGroupMask()"

        ASVM& asvm = static_cast<ASVM&>(GetVM());
        if (asvm.ExtensionsEnabled)
        {
            result = obj->GetIntObj()->GetFocusGroupMask();
        }

//##protect##"class_::FocusManager::getFocusGroupMask()"
    }
    void FocusManager::setControllerFocusGroup(bool& result, UInt32 controllerIdx, UInt32 focusGroupIdx)
    {
//##protect##"class_::FocusManager::setControllerFocusGroup()"

        ASVM& asvm = static_cast<ASVM&>(GetVM());
        if (asvm.ExtensionsEnabled)
        {
            MovieImpl* proot = asvm.GetMovieImpl();
            result = proot->SetControllerFocusGroup(controllerIdx, focusGroupIdx);
        }
        else 
            result = false;

//##protect##"class_::FocusManager::setControllerFocusGroup()"
    }
    void FocusManager::getControllerFocusGroup(UInt32& result, UInt32 controllerIdx)
    {
//##protect##"class_::FocusManager::getControllerFocusGroup()"

        ASVM& asvm = static_cast<ASVM&>(GetVM());
        if (asvm.ExtensionsEnabled)
        {
            MovieImpl* proot = asvm.GetMovieImpl();
            result = proot->GetControllerFocusGroup(controllerIdx);
        }
        else
            result = 0;

//##protect##"class_::FocusManager::getControllerFocusGroup()"
    }
    void FocusManager::getControllerMaskByFocusGroup(UInt32& result, UInt32 focusGroupIdx)
    {
//##protect##"class_::FocusManager::getControllerMaskByFocusGroup()"
		
		ASVM& asvm = static_cast<ASVM&>(GetVM());		
		if (asvm.ExtensionsEnabled)
		{
			MovieImpl* proot = asvm.GetMovieImpl();
			result = proot->GetControllerMaskByFocusGroup(focusGroupIdx);
		}
		else 
		{
			result = 0;
		}
//##protect##"class_::FocusManager::getControllerMaskByFocusGroup()"
    }
    void FocusManager::getModalClip(SPtr<Instances::fl_display::Sprite>& result, UInt32 controllerIdx)
    {
//##protect##"class_::FocusManager::getModalClip()"

        ASVM& asvm = static_cast<ASVM&>(GetVM());
        if (asvm.ExtensionsEnabled)
        {
            MovieImpl* proot = asvm.GetMovieImpl();
            GFx::Sprite* pmodalClip = proot->GetModalClip(controllerIdx);
            if (pmodalClip)
            {
                Instances::fl_display::DisplayObject* ido = ToAvmSprite(pmodalClip)->GetAS3Obj();

                if (ido && IsSprite(ido->GetTraitsType()))
                    result = static_cast<Instances::fl_display::Sprite*>(ido);
                else
                    result = NULL;
            }
        }

//##protect##"class_::FocusManager::getModalClip()"
    }
    void FocusManager::setModalClip(const Value& result, Instances::fl_display::Sprite* mc, UInt32 controllerIdx)
    {
//##protect##"class_::FocusManager::setModalClip()"

        SF_UNUSED(result);
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        if (asvm.ExtensionsEnabled)
        {
            MovieImpl* proot = asvm.GetMovieImpl();
            if (mc)
            {
                proot->SetModalClip(static_cast<GFx::Sprite*>(mc->pDispObj.GetPtr()), controllerIdx);
            }
            else
                proot->SetModalClip(NULL, controllerIdx);
        }

//##protect##"class_::FocusManager::setModalClip()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes

typedef ThunkFunc1<Classes::fl_gfx::FocusManager, Classes::fl_gfx::FocusManager::mid_alwaysEnableArrowKeysSet, const Value, bool> TFunc_Classes_FocusManager_alwaysEnableArrowKeysSet;
typedef ThunkFunc0<Classes::fl_gfx::FocusManager, Classes::fl_gfx::FocusManager::mid_alwaysEnableArrowKeysGet, bool> TFunc_Classes_FocusManager_alwaysEnableArrowKeysGet;
typedef ThunkFunc1<Classes::fl_gfx::FocusManager, Classes::fl_gfx::FocusManager::mid_disableFocusKeysSet, const Value, bool> TFunc_Classes_FocusManager_disableFocusKeysSet;
typedef ThunkFunc0<Classes::fl_gfx::FocusManager, Classes::fl_gfx::FocusManager::mid_disableFocusKeysGet, bool> TFunc_Classes_FocusManager_disableFocusKeysGet;
typedef ThunkFunc4<Classes::fl_gfx::FocusManager, Classes::fl_gfx::FocusManager::mid_moveFocus, SPtr<Instances::fl_display::InteractiveObject>, const ASString&, Instances::fl_display::InteractiveObject*, bool, UInt32> TFunc_Classes_FocusManager_moveFocus;
typedef ThunkFunc6<Classes::fl_gfx::FocusManager, Classes::fl_gfx::FocusManager::mid_findFocus, SPtr<Instances::fl_display::InteractiveObject>, const ASString&, Instances::fl_display::DisplayObjectContainer*, bool, Instances::fl_display::InteractiveObject*, bool, UInt32> TFunc_Classes_FocusManager_findFocus;
typedef ThunkFunc2<Classes::fl_gfx::FocusManager, Classes::fl_gfx::FocusManager::mid_setFocus, const Value, Instances::fl_display::InteractiveObject*, UInt32> TFunc_Classes_FocusManager_setFocus;
typedef ThunkFunc1<Classes::fl_gfx::FocusManager, Classes::fl_gfx::FocusManager::mid_getFocus, SPtr<Instances::fl_display::InteractiveObject>, UInt32> TFunc_Classes_FocusManager_getFocus;
typedef ThunkFunc0<Classes::fl_gfx::FocusManager, Classes::fl_gfx::FocusManager::mid_numFocusGroupsGet, UInt32> TFunc_Classes_FocusManager_numFocusGroupsGet;
typedef ThunkFunc2<Classes::fl_gfx::FocusManager, Classes::fl_gfx::FocusManager::mid_setFocusGroupMask, const Value, Instances::fl_display::InteractiveObject*, UInt32> TFunc_Classes_FocusManager_setFocusGroupMask;
typedef ThunkFunc1<Classes::fl_gfx::FocusManager, Classes::fl_gfx::FocusManager::mid_getFocusGroupMask, UInt32, Instances::fl_display::InteractiveObject*> TFunc_Classes_FocusManager_getFocusGroupMask;
typedef ThunkFunc2<Classes::fl_gfx::FocusManager, Classes::fl_gfx::FocusManager::mid_setControllerFocusGroup, bool, UInt32, UInt32> TFunc_Classes_FocusManager_setControllerFocusGroup;
typedef ThunkFunc1<Classes::fl_gfx::FocusManager, Classes::fl_gfx::FocusManager::mid_getControllerFocusGroup, UInt32, UInt32> TFunc_Classes_FocusManager_getControllerFocusGroup;
typedef ThunkFunc1<Classes::fl_gfx::FocusManager, Classes::fl_gfx::FocusManager::mid_getControllerMaskByFocusGroup, UInt32, UInt32> TFunc_Classes_FocusManager_getControllerMaskByFocusGroup;
typedef ThunkFunc1<Classes::fl_gfx::FocusManager, Classes::fl_gfx::FocusManager::mid_getModalClip, SPtr<Instances::fl_display::Sprite>, UInt32> TFunc_Classes_FocusManager_getModalClip;
typedef ThunkFunc2<Classes::fl_gfx::FocusManager, Classes::fl_gfx::FocusManager::mid_setModalClip, const Value, Instances::fl_display::Sprite*, UInt32> TFunc_Classes_FocusManager_setModalClip;

template <> const TFunc_Classes_FocusManager_alwaysEnableArrowKeysSet::TMethod TFunc_Classes_FocusManager_alwaysEnableArrowKeysSet::Method = &Classes::fl_gfx::FocusManager::alwaysEnableArrowKeysSet;
template <> const TFunc_Classes_FocusManager_alwaysEnableArrowKeysGet::TMethod TFunc_Classes_FocusManager_alwaysEnableArrowKeysGet::Method = &Classes::fl_gfx::FocusManager::alwaysEnableArrowKeysGet;
template <> const TFunc_Classes_FocusManager_disableFocusKeysSet::TMethod TFunc_Classes_FocusManager_disableFocusKeysSet::Method = &Classes::fl_gfx::FocusManager::disableFocusKeysSet;
template <> const TFunc_Classes_FocusManager_disableFocusKeysGet::TMethod TFunc_Classes_FocusManager_disableFocusKeysGet::Method = &Classes::fl_gfx::FocusManager::disableFocusKeysGet;
template <> const TFunc_Classes_FocusManager_moveFocus::TMethod TFunc_Classes_FocusManager_moveFocus::Method = &Classes::fl_gfx::FocusManager::moveFocus;
template <> const TFunc_Classes_FocusManager_findFocus::TMethod TFunc_Classes_FocusManager_findFocus::Method = &Classes::fl_gfx::FocusManager::findFocus;
template <> const TFunc_Classes_FocusManager_setFocus::TMethod TFunc_Classes_FocusManager_setFocus::Method = &Classes::fl_gfx::FocusManager::setFocus;
template <> const TFunc_Classes_FocusManager_getFocus::TMethod TFunc_Classes_FocusManager_getFocus::Method = &Classes::fl_gfx::FocusManager::getFocus;
template <> const TFunc_Classes_FocusManager_numFocusGroupsGet::TMethod TFunc_Classes_FocusManager_numFocusGroupsGet::Method = &Classes::fl_gfx::FocusManager::numFocusGroupsGet;
template <> const TFunc_Classes_FocusManager_setFocusGroupMask::TMethod TFunc_Classes_FocusManager_setFocusGroupMask::Method = &Classes::fl_gfx::FocusManager::setFocusGroupMask;
template <> const TFunc_Classes_FocusManager_getFocusGroupMask::TMethod TFunc_Classes_FocusManager_getFocusGroupMask::Method = &Classes::fl_gfx::FocusManager::getFocusGroupMask;
template <> const TFunc_Classes_FocusManager_setControllerFocusGroup::TMethod TFunc_Classes_FocusManager_setControllerFocusGroup::Method = &Classes::fl_gfx::FocusManager::setControllerFocusGroup;
template <> const TFunc_Classes_FocusManager_getControllerFocusGroup::TMethod TFunc_Classes_FocusManager_getControllerFocusGroup::Method = &Classes::fl_gfx::FocusManager::getControllerFocusGroup;
template <> const TFunc_Classes_FocusManager_getControllerMaskByFocusGroup::TMethod TFunc_Classes_FocusManager_getControllerMaskByFocusGroup::Method = &Classes::fl_gfx::FocusManager::getControllerMaskByFocusGroup;
template <> const TFunc_Classes_FocusManager_getModalClip::TMethod TFunc_Classes_FocusManager_getModalClip::Method = &Classes::fl_gfx::FocusManager::getModalClip;
template <> const TFunc_Classes_FocusManager_setModalClip::TMethod TFunc_Classes_FocusManager_setModalClip::Method = &Classes::fl_gfx::FocusManager::setModalClip;

namespace ClassTraits { namespace fl_gfx
{
    // const UInt16 FocusManager::tito[FocusManager::ThunkInfoNum] = {
    //    0, 2, 3, 5, 6, 11, 18, 21, 23, 24, 27, 29, 32, 34, 36, 38, 
    // };
    const TypeInfo* FocusManager::tit[41] = {
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        &AS3::fl_display::InteractiveObjectTI, &AS3::fl::StringTI, &AS3::fl_display::InteractiveObjectTI, &AS3::fl::BooleanTI, &AS3::fl::uintTI, 
        &AS3::fl_display::InteractiveObjectTI, &AS3::fl::StringTI, &AS3::fl_display::DisplayObjectContainerTI, &AS3::fl::BooleanTI, &AS3::fl_display::InteractiveObjectTI, &AS3::fl::BooleanTI, &AS3::fl::uintTI, 
        NULL, &AS3::fl_display::InteractiveObjectTI, &AS3::fl::uintTI, 
        &AS3::fl_display::InteractiveObjectTI, &AS3::fl::uintTI, 
        &AS3::fl::uintTI, 
        NULL, &AS3::fl_display::InteractiveObjectTI, &AS3::fl::uintTI, 
        &AS3::fl::uintTI, &AS3::fl_display::InteractiveObjectTI, 
        &AS3::fl::BooleanTI, &AS3::fl::uintTI, &AS3::fl::uintTI, 
        &AS3::fl::uintTI, &AS3::fl::uintTI, 
        &AS3::fl::uintTI, &AS3::fl::uintTI, 
        &AS3::fl_display::SpriteTI, &AS3::fl::uintTI, 
        NULL, &AS3::fl_display::SpriteTI, &AS3::fl::uintTI, 
    };
    const ThunkInfo FocusManager::ti[FocusManager::ThunkInfoNum] = {
        {TFunc_Classes_FocusManager_alwaysEnableArrowKeysSet::Func, &FocusManager::tit[0], "alwaysEnableArrowKeys", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Classes_FocusManager_alwaysEnableArrowKeysGet::Func, &FocusManager::tit[2], "alwaysEnableArrowKeys", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Classes_FocusManager_disableFocusKeysSet::Func, &FocusManager::tit[3], "disableFocusKeys", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Classes_FocusManager_disableFocusKeysGet::Func, &FocusManager::tit[5], "disableFocusKeys", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Classes_FocusManager_moveFocus::Func, &FocusManager::tit[6], "moveFocus", NULL, Abc::NS_Public, CT_Method, 1, 4, 0, 0, NULL},
        {TFunc_Classes_FocusManager_findFocus::Func, &FocusManager::tit[11], "findFocus", NULL, Abc::NS_Public, CT_Method, 1, 6, 0, 0, NULL},
        {TFunc_Classes_FocusManager_setFocus::Func, &FocusManager::tit[18], "setFocus", NULL, Abc::NS_Public, CT_Method, 1, 2, 0, 0, NULL},
        {TFunc_Classes_FocusManager_getFocus::Func, &FocusManager::tit[21], "getFocus", NULL, Abc::NS_Public, CT_Method, 0, 1, 0, 0, NULL},
        {TFunc_Classes_FocusManager_numFocusGroupsGet::Func, &FocusManager::tit[23], "numFocusGroups", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Classes_FocusManager_setFocusGroupMask::Func, &FocusManager::tit[24], "setFocusGroupMask", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Classes_FocusManager_getFocusGroupMask::Func, &FocusManager::tit[27], "getFocusGroupMask", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Classes_FocusManager_setControllerFocusGroup::Func, &FocusManager::tit[29], "setControllerFocusGroup", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Classes_FocusManager_getControllerFocusGroup::Func, &FocusManager::tit[32], "getControllerFocusGroup", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Classes_FocusManager_getControllerMaskByFocusGroup::Func, &FocusManager::tit[34], "getControllerMaskByFocusGroup", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Classes_FocusManager_getModalClip::Func, &FocusManager::tit[36], "getModalClip", NULL, Abc::NS_Public, CT_Method, 0, 1, 0, 0, NULL},
        {TFunc_Classes_FocusManager_setModalClip::Func, &FocusManager::tit[38], "setModalClip", NULL, Abc::NS_Public, CT_Method, 1, 2, 0, 0, NULL},
    };

    FocusManager::FocusManager(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::FocusManager::FocusManager()"
//##protect##"ClassTraits::FocusManager::FocusManager()"

    }

    Pickable<Traits> FocusManager::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) FocusManager(vm, AS3::fl_gfx::FocusManagerCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_gfx::FocusManagerCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_gfx
{
    const TypeInfo FocusManagerTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_gfx::FocusManager::InstanceType),
        ClassTraits::fl_gfx::FocusManager::ThunkInfoNum,
        0,
        0,
        0,
        "FocusManager", "scaleform.gfx", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo FocusManagerCI = {
        &FocusManagerTI,
        ClassTraits::fl_gfx::FocusManager::MakeClassTraits,
        ClassTraits::fl_gfx::FocusManager::ti,
        NULL,
        NULL,
        NULL,
    };
}; // namespace fl_gfx


}}} // namespace Scaleform { namespace GFx { namespace AS3

