//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_DisplayObjectContainer.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Display_DisplayObjectContainer.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../AS3_Obj_Array.h"
#include "../Geom/AS3_Obj_Geom_Point.h"
#include "../Text/AS3_Obj_Text_TextSnapshot.h"
#include "GFx/AS3/AS3_AvmSprite.h"
#include "GFx/AS3/AS3_MovieRoot.h"
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
    SInt32 GetMethodDefArg<Instances::fl_display::DisplayObjectContainer, Instances::fl_display::DisplayObjectContainer::mid_removeChildren, 1, SInt32>(AS3::StringManager&)
    {
        return 0x7fffffff;
    }

} // namespace Impl
#endif // SF_AS3_EMIT_DEF_ARGS

typedef ThunkFunc0<Instances::fl_display::DisplayObjectContainer, Instances::fl_display::DisplayObjectContainer::mid_mouseChildrenGet, bool> TFunc_Instances_DisplayObjectContainer_mouseChildrenGet;
typedef ThunkFunc1<Instances::fl_display::DisplayObjectContainer, Instances::fl_display::DisplayObjectContainer::mid_mouseChildrenSet, const Value, bool> TFunc_Instances_DisplayObjectContainer_mouseChildrenSet;
typedef ThunkFunc0<Instances::fl_display::DisplayObjectContainer, Instances::fl_display::DisplayObjectContainer::mid_numChildrenGet, SInt32> TFunc_Instances_DisplayObjectContainer_numChildrenGet;
typedef ThunkFunc0<Instances::fl_display::DisplayObjectContainer, Instances::fl_display::DisplayObjectContainer::mid_tabChildrenGet, bool> TFunc_Instances_DisplayObjectContainer_tabChildrenGet;
typedef ThunkFunc1<Instances::fl_display::DisplayObjectContainer, Instances::fl_display::DisplayObjectContainer::mid_tabChildrenSet, const Value, bool> TFunc_Instances_DisplayObjectContainer_tabChildrenSet;
typedef ThunkFunc0<Instances::fl_display::DisplayObjectContainer, Instances::fl_display::DisplayObjectContainer::mid_textSnapshotGet, SPtr<Instances::fl_text::TextSnapshot> > TFunc_Instances_DisplayObjectContainer_textSnapshotGet;
typedef ThunkFunc1<Instances::fl_display::DisplayObjectContainer, Instances::fl_display::DisplayObjectContainer::mid_addChild, SPtr<Instances::fl_display::DisplayObject>, Instances::fl_display::DisplayObject*> TFunc_Instances_DisplayObjectContainer_addChild;
typedef ThunkFunc2<Instances::fl_display::DisplayObjectContainer, Instances::fl_display::DisplayObjectContainer::mid_addChildAt, SPtr<Instances::fl_display::DisplayObject>, Instances::fl_display::DisplayObject*, SInt32> TFunc_Instances_DisplayObjectContainer_addChildAt;
typedef ThunkFunc1<Instances::fl_display::DisplayObjectContainer, Instances::fl_display::DisplayObjectContainer::mid_areInaccessibleObjectsUnderPoint, bool, Instances::fl_geom::Point*> TFunc_Instances_DisplayObjectContainer_areInaccessibleObjectsUnderPoint;
typedef ThunkFunc1<Instances::fl_display::DisplayObjectContainer, Instances::fl_display::DisplayObjectContainer::mid_contains, bool, Instances::fl_display::DisplayObject*> TFunc_Instances_DisplayObjectContainer_contains;
typedef ThunkFunc1<Instances::fl_display::DisplayObjectContainer, Instances::fl_display::DisplayObjectContainer::mid_getChildAt, SPtr<Instances::fl_display::DisplayObject>, SInt32> TFunc_Instances_DisplayObjectContainer_getChildAt;
typedef ThunkFunc1<Instances::fl_display::DisplayObjectContainer, Instances::fl_display::DisplayObjectContainer::mid_getChildByName, SPtr<Instances::fl_display::DisplayObject>, const ASString&> TFunc_Instances_DisplayObjectContainer_getChildByName;
typedef ThunkFunc1<Instances::fl_display::DisplayObjectContainer, Instances::fl_display::DisplayObjectContainer::mid_getChildIndex, SInt32, Instances::fl_display::DisplayObject*> TFunc_Instances_DisplayObjectContainer_getChildIndex;
typedef ThunkFunc1<Instances::fl_display::DisplayObjectContainer, Instances::fl_display::DisplayObjectContainer::mid_getObjectsUnderPoint, SPtr<Instances::fl::Array>, Instances::fl_geom::Point*> TFunc_Instances_DisplayObjectContainer_getObjectsUnderPoint;
typedef ThunkFunc2<Instances::fl_display::DisplayObjectContainer, Instances::fl_display::DisplayObjectContainer::mid_removeChild, Value, unsigned, const Value*> TFunc_Instances_DisplayObjectContainer_removeChild;
typedef ThunkFunc1<Instances::fl_display::DisplayObjectContainer, Instances::fl_display::DisplayObjectContainer::mid_removeChildAt, SPtr<Instances::fl_display::DisplayObject>, SInt32> TFunc_Instances_DisplayObjectContainer_removeChildAt;
typedef ThunkFunc2<Instances::fl_display::DisplayObjectContainer, Instances::fl_display::DisplayObjectContainer::mid_removeChildren, const Value, SInt32, SInt32> TFunc_Instances_DisplayObjectContainer_removeChildren;
typedef ThunkFunc2<Instances::fl_display::DisplayObjectContainer, Instances::fl_display::DisplayObjectContainer::mid_setChildIndex, const Value, Instances::fl_display::DisplayObject*, SInt32> TFunc_Instances_DisplayObjectContainer_setChildIndex;
typedef ThunkFunc2<Instances::fl_display::DisplayObjectContainer, Instances::fl_display::DisplayObjectContainer::mid_swapChildren, const Value, Instances::fl_display::DisplayObject*, Instances::fl_display::DisplayObject*> TFunc_Instances_DisplayObjectContainer_swapChildren;
typedef ThunkFunc2<Instances::fl_display::DisplayObjectContainer, Instances::fl_display::DisplayObjectContainer::mid_swapChildrenAt, const Value, SInt32, SInt32> TFunc_Instances_DisplayObjectContainer_swapChildrenAt;

template <> const TFunc_Instances_DisplayObjectContainer_mouseChildrenGet::TMethod TFunc_Instances_DisplayObjectContainer_mouseChildrenGet::Method = &Instances::fl_display::DisplayObjectContainer::mouseChildrenGet;
template <> const TFunc_Instances_DisplayObjectContainer_mouseChildrenSet::TMethod TFunc_Instances_DisplayObjectContainer_mouseChildrenSet::Method = &Instances::fl_display::DisplayObjectContainer::mouseChildrenSet;
template <> const TFunc_Instances_DisplayObjectContainer_numChildrenGet::TMethod TFunc_Instances_DisplayObjectContainer_numChildrenGet::Method = &Instances::fl_display::DisplayObjectContainer::numChildrenGet;
template <> const TFunc_Instances_DisplayObjectContainer_tabChildrenGet::TMethod TFunc_Instances_DisplayObjectContainer_tabChildrenGet::Method = &Instances::fl_display::DisplayObjectContainer::tabChildrenGet;
template <> const TFunc_Instances_DisplayObjectContainer_tabChildrenSet::TMethod TFunc_Instances_DisplayObjectContainer_tabChildrenSet::Method = &Instances::fl_display::DisplayObjectContainer::tabChildrenSet;
template <> const TFunc_Instances_DisplayObjectContainer_textSnapshotGet::TMethod TFunc_Instances_DisplayObjectContainer_textSnapshotGet::Method = &Instances::fl_display::DisplayObjectContainer::textSnapshotGet;
template <> const TFunc_Instances_DisplayObjectContainer_addChild::TMethod TFunc_Instances_DisplayObjectContainer_addChild::Method = &Instances::fl_display::DisplayObjectContainer::addChild;
template <> const TFunc_Instances_DisplayObjectContainer_addChildAt::TMethod TFunc_Instances_DisplayObjectContainer_addChildAt::Method = &Instances::fl_display::DisplayObjectContainer::addChildAt;
template <> const TFunc_Instances_DisplayObjectContainer_areInaccessibleObjectsUnderPoint::TMethod TFunc_Instances_DisplayObjectContainer_areInaccessibleObjectsUnderPoint::Method = &Instances::fl_display::DisplayObjectContainer::areInaccessibleObjectsUnderPoint;
template <> const TFunc_Instances_DisplayObjectContainer_contains::TMethod TFunc_Instances_DisplayObjectContainer_contains::Method = &Instances::fl_display::DisplayObjectContainer::contains;
template <> const TFunc_Instances_DisplayObjectContainer_getChildAt::TMethod TFunc_Instances_DisplayObjectContainer_getChildAt::Method = &Instances::fl_display::DisplayObjectContainer::getChildAt;
template <> const TFunc_Instances_DisplayObjectContainer_getChildByName::TMethod TFunc_Instances_DisplayObjectContainer_getChildByName::Method = &Instances::fl_display::DisplayObjectContainer::getChildByName;
template <> const TFunc_Instances_DisplayObjectContainer_getChildIndex::TMethod TFunc_Instances_DisplayObjectContainer_getChildIndex::Method = &Instances::fl_display::DisplayObjectContainer::getChildIndex;
template <> const TFunc_Instances_DisplayObjectContainer_getObjectsUnderPoint::TMethod TFunc_Instances_DisplayObjectContainer_getObjectsUnderPoint::Method = &Instances::fl_display::DisplayObjectContainer::getObjectsUnderPoint;
template <> const TFunc_Instances_DisplayObjectContainer_removeChild::TMethod TFunc_Instances_DisplayObjectContainer_removeChild::Method = &Instances::fl_display::DisplayObjectContainer::removeChild;
template <> const TFunc_Instances_DisplayObjectContainer_removeChildAt::TMethod TFunc_Instances_DisplayObjectContainer_removeChildAt::Method = &Instances::fl_display::DisplayObjectContainer::removeChildAt;
template <> const TFunc_Instances_DisplayObjectContainer_removeChildren::TMethod TFunc_Instances_DisplayObjectContainer_removeChildren::Method = &Instances::fl_display::DisplayObjectContainer::removeChildren;
template <> const TFunc_Instances_DisplayObjectContainer_setChildIndex::TMethod TFunc_Instances_DisplayObjectContainer_setChildIndex::Method = &Instances::fl_display::DisplayObjectContainer::setChildIndex;
template <> const TFunc_Instances_DisplayObjectContainer_swapChildren::TMethod TFunc_Instances_DisplayObjectContainer_swapChildren::Method = &Instances::fl_display::DisplayObjectContainer::swapChildren;
template <> const TFunc_Instances_DisplayObjectContainer_swapChildrenAt::TMethod TFunc_Instances_DisplayObjectContainer_swapChildrenAt::Method = &Instances::fl_display::DisplayObjectContainer::swapChildrenAt;

namespace Instances { namespace fl_display
{
    DisplayObjectContainer::DisplayObjectContainer(InstanceTraits::Traits& t)
    : Instances::fl_display::InteractiveObject(t)
//##protect##"instance::DisplayObjectContainer::DisplayObjectContainer()$data"
//##protect##"instance::DisplayObjectContainer::DisplayObjectContainer()$data"
    {
//##protect##"instance::DisplayObjectContainer::DisplayObjectContainer()$code"
//##protect##"instance::DisplayObjectContainer::DisplayObjectContainer()$code"
    }

    void DisplayObjectContainer::mouseChildrenGet(bool& result)
    {
//##protect##"instance::DisplayObjectContainer::mouseChildrenGet()"
        SF_UNUSED1(result);
        result = !GetDisplayObjContainer()->IsMouseChildrenDisabledFlagSet();
//##protect##"instance::DisplayObjectContainer::mouseChildrenGet()"
    }
    void DisplayObjectContainer::mouseChildrenSet(const Value& result, bool value)
    {
//##protect##"instance::DisplayObjectContainer::mouseChildrenSet()"
        SF_UNUSED2(result, value);
        GetDisplayObjContainer()->SetMouseChildrenDisabledFlag(!value);
//##protect##"instance::DisplayObjectContainer::mouseChildrenSet()"
    }
    void DisplayObjectContainer::numChildrenGet(SInt32& result)
    {
//##protect##"instance::DisplayObjectContainer::numChildrenGet()"
        SF_ASSERT(pDispObj && pDispObj->IsDisplayObjContainer());
        GFx::DisplayObjContainer* spr = pDispObj->CharToDisplayObjContainer_Unsafe();
        SF_ASSERT(spr);
        result = (int)spr->GetNumChildren();
//##protect##"instance::DisplayObjectContainer::numChildrenGet()"
    }
    void DisplayObjectContainer::tabChildrenGet(bool& result)
    {
//##protect##"instance::DisplayObjectContainer::tabChildrenGet()"
        SF_UNUSED1(result);
        result = !GetDisplayObjContainer()->IsTabChildrenDisabledFlagSet();
//##protect##"instance::DisplayObjectContainer::tabChildrenGet()"
    }
    void DisplayObjectContainer::tabChildrenSet(const Value& result, bool value)
    {
//##protect##"instance::DisplayObjectContainer::tabChildrenSet()"
        SF_UNUSED2(result, value);
        GetDisplayObjContainer()->SetTabChildrenDisabledFlag(!value);

        // fire tabChildrenChange event
        SPtr<fl_events::Event> e = 
            CreateEventObject(GetVM().GetStringManager().CreateConstString("tabChildrenChange"), true, false);
        Dispatch(e, pDispObj);

//##protect##"instance::DisplayObjectContainer::tabChildrenSet()"
    }
    void DisplayObjectContainer::textSnapshotGet(SPtr<Instances::fl_text::TextSnapshot>& result)
    {
//##protect##"instance::DisplayObjectContainer::textSnapshotGet()"
        SF_UNUSED(result);
#ifdef GFX_AS_ENABLE_TEXTSNAPSHOT    
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        SPtr<fl_text::TextSnapshot> ts;

        SPtr<AS3::Object> obj = asvm.GetClass("flash.text.TextSnapshot", asvm.GetCurrentAppDomain());
        asvm.ConstructInstance(ts, obj);
        SF_ASSERT(ts);

        SF_ASSERT(pDispObj && pDispObj->IsSprite());
        GFx::Sprite* psprite = pDispObj->CharToSprite_Unsafe();
        ts->ProcessSnapshot(psprite);
        result = ts;
#endif // #ifdef GFX_AS_ENABLE_TEXTSNAPSHOT    
//##protect##"instance::DisplayObjectContainer::textSnapshotGet()"
    }
    void DisplayObjectContainer::addChild(SPtr<Instances::fl_display::DisplayObject>& result, Instances::fl_display::DisplayObject* child)
    {
//##protect##"instance::DisplayObjectContainer::addChild()"
        SF_UNUSED2(result, child);
        if (!child)
            return GetVM().ThrowTypeError(VM::Error(VM::eNullPointerError, GetVM() SF_DEBUG_ARG("child")));
        if (child == this)
            return GetVM().ThrowArgumentError(VM::Error(VM::eAddObjectItselfError, GetVM()));

        SF_ASSERT(pDispObj && pDispObj->IsDisplayObjContainer());
        GFx::DisplayObjContainer* container = pDispObj->CharToDisplayObjContainer_Unsafe();
        SF_ASSERT(container);
        
        // make sure child's stage obj exists
        if (!child->pDispObj)
            child->CreateStageObject();
        SF_ASSERT(child->pDispObj);

        ToAvmDisplayObjContainer(container)->AddChild(child->pDispObj);
        result = child;
//##protect##"instance::DisplayObjectContainer::addChild()"
    }
    void DisplayObjectContainer::addChildAt(SPtr<Instances::fl_display::DisplayObject>& result, Instances::fl_display::DisplayObject* child, SInt32 index)
    {
//##protect##"instance::DisplayObjectContainer::addChildAt()"
        SF_UNUSED2(result, child);
        if (!child)
            return GetVM().ThrowTypeError(VM::Error(VM::eNullPointerError, GetVM() SF_DEBUG_ARG("child")));
        if (child == this)
            return GetVM().ThrowArgumentError(VM::Error(VM::eAddObjectItselfError, GetVM()));

        SF_ASSERT(pDispObj && pDispObj->IsDisplayObjContainer());
        GFx::DisplayObjContainer* container = pDispObj->CharToDisplayObjContainer_Unsafe();
        SF_ASSERT(container);

        // make sure child's stage obj exists
        if (!child->pDispObj)
            child->CreateStageObject();
        SF_ASSERT(child->pDispObj);

        result = NULL;
        if (index < 0 || index > (SInt32)container->GetNumChildren())
            GetVM().ThrowRangeError(VM::Error(VM::eParamRangeError, GetVM()));
        else
        {
            ToAvmDisplayObjContainer(container)->AddChildAt(child->pDispObj, index);
            result = child;
        }
//##protect##"instance::DisplayObjectContainer::addChildAt()"
    }
    void DisplayObjectContainer::areInaccessibleObjectsUnderPoint(bool& result, Instances::fl_geom::Point* point)
    {
//##protect##"instance::DisplayObjectContainer::areInaccessibleObjectsUnderPoint()"
        SF_UNUSED2(result, point);
        WARN_NOT_IMPLEMENTED("DisplayObjectContainer::areInaccessibleObjectsUnderPoint()");
//##protect##"instance::DisplayObjectContainer::areInaccessibleObjectsUnderPoint()"
    }
    void DisplayObjectContainer::contains(bool& result, Instances::fl_display::DisplayObject* child)
    {
//##protect##"instance::DisplayObjectContainer::contains()"
        SF_UNUSED2(result, child);
        if (!child)
            return GetVM().ThrowTypeError(VM::Error(VM::eNullPointerError, GetVM() SF_DEBUG_ARG("child")));

        SF_ASSERT(pDispObj && pDispObj->IsDisplayObjContainer());
        GFx::DisplayObjContainer* container = pDispObj->CharToDisplayObjContainer_Unsafe();

        result = container->Contains(child->pDispObj);
//##protect##"instance::DisplayObjectContainer::contains()"
    }
    void DisplayObjectContainer::getChildAt(SPtr<Instances::fl_display::DisplayObject>& result, SInt32 index)
    {
//##protect##"instance::DisplayObjectContainer::getChildAt()"
        SF_ASSERT(pDispObj && pDispObj->IsDisplayObjContainer());
        GFx::DisplayObjContainer* container = pDispObj->CharToDisplayObjContainer_Unsafe();

        SPtr<DisplayObject> obj = ToAvmDisplayObjContainer(container)->GetAS3ChildAt(index);
        if (!obj)
            GetVM().ThrowRangeError(VM::Error(VM::eParamRangeError, GetVM()));
        else
            result = obj;
//##protect##"instance::DisplayObjectContainer::getChildAt()"
    }
    void DisplayObjectContainer::getChildByName(SPtr<Instances::fl_display::DisplayObject>& result, const ASString& name)
    {
//##protect##"instance::DisplayObjectContainer::getChildByName()"
        SF_ASSERT(pDispObj && pDispObj->IsDisplayObjContainer());
        GFx::DisplayObjContainer* container = pDispObj->CharToDisplayObjContainer_Unsafe();

        SPtr<DisplayObject> obj = ToAvmDisplayObjContainer(container)->GetAS3ChildByName(name);
        if (!obj)
            result = NULL;
        else
            result = obj;
//##protect##"instance::DisplayObjectContainer::getChildByName()"
    }
    void DisplayObjectContainer::getChildIndex(SInt32& result, Instances::fl_display::DisplayObject* child)
    {
//##protect##"instance::DisplayObjectContainer::getChildIndex()"
        SF_UNUSED2(result, child);
        if (!child)
            return GetVM().ThrowTypeError(VM::Error(VM::eNullPointerError, GetVM() SF_DEBUG_ARG("child")));

        SF_ASSERT(pDispObj && pDispObj->IsDisplayObjContainer());
        GFx::DisplayObjContainer* container = pDispObj->CharToDisplayObjContainer_Unsafe();
        
        SPInt idx = container->GetChildIndex(child->pDispObj);
        if (idx < 0)
            return GetVM().ThrowArgumentError(VM::Error(VM::eMustBeChildError, GetVM() SF_DEBUG_ARG("DisplayObject")));
        
        result = (SInt32)idx;
//##protect##"instance::DisplayObjectContainer::getChildIndex()"
    }
    void DisplayObjectContainer::getObjectsUnderPoint(SPtr<Instances::fl::Array>& result, Instances::fl_geom::Point* point)
    {
//##protect##"instance::DisplayObjectContainer::getObjectsUnderPoint()"
        SF_UNUSED2(result, point);
        
        SF_ASSERT(pDispObj && pDispObj->IsDisplayObjContainer());
        GFx::DisplayObjContainer* container = pDispObj->CharToDisplayObjContainer_Unsafe();

        // convert the point from stage coord space to conttainer's space
        GFx::DisplayObject::Matrix m = container->GetWorldMatrix();
        PointF pt((float)PixelsToTwips(point->GetX()), (float)PixelsToTwips(point->GetY()));
        Render::PointF p = m.TransformByInverse(pt);
        ArrayDH<Ptr<DisplayObjectBase> > arr(GetVM().GetMemoryHeap());
        ToAvmDisplayObjContainer(container)->GetObjectsUnderPoint(&arr, p);

        // Create an array
        SPtr<Instances::fl::Array> asArray = GetVM().MakeArray();

        for (UPInt i = 0, n = arr.GetSize(); i < n; ++i)
        {
            if (arr[i]->IsScriptableObject())
            {
                AvmDisplayObj* avobj = ToAvmDisplayObj(arr[i]->CharToScriptableObject_Unsafe());
                if (!avobj->GetAS3Obj())
                    avobj->CreateASInstance(true); // make sure AS3 instance exists
                asArray->PushBack(Value(avobj->GetAS3Obj()));
            }
        }
        result = asArray;
//##protect##"instance::DisplayObjectContainer::getObjectsUnderPoint()"
    }
    void DisplayObjectContainer::removeChild(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::DisplayObjectContainer::removeChild()"
        SF_UNUSED(result);
        if (argc < 1 || argv[0].IsNullOrUndefined() || !argv[0].IsObject())
            return GetVM().ThrowTypeError(VM::Error(VM::eNullPointerError, GetVM() SF_DEBUG_ARG("child")));

        //@TODO: revise:        
        // 1) do we need to throw exception if type is wrong?
        AS3::Object* object = argv[0].GetObject();
        if (!object || !AreDisplayObjectTraits(object))
            return result.SetUndefined();
        SF_ASSERT(GetVM().IsOfType(argv[0], "flash.display.DisplayObject", GetVM().GetCurrentAppDomain()));

        Instances::fl_display::DisplayObject* child = static_cast<Instances::fl_display::DisplayObject*>(object);
        SF_ASSERT(pDispObj && pDispObj->IsDisplayObjContainer());
        GFx::DisplayObjContainer* container = pDispObj->CharToDisplayObjContainer_Unsafe();
        SF_ASSERT(container);

        // make sure child's stage obj exists
        if (child->pDispObj)
        {
            AvmDisplayObjContainer* avmObj = ToAvmDisplayObjContainer(container);
            avmObj->RemoveChild(child->pDispObj);
        }
        // do we need to addref child here??? (!AB) 
        result = child;
//##protect##"instance::DisplayObjectContainer::removeChild()"
    }
    void DisplayObjectContainer::removeChildAt(SPtr<Instances::fl_display::DisplayObject>& result, SInt32 index)
    {
//##protect##"instance::DisplayObjectContainer::removeChildAt()"
        SF_UNUSED(result);
        SF_ASSERT(pDispObj && pDispObj->IsDisplayObjContainer());
        GFx::DisplayObjContainer* container = pDispObj->CharToDisplayObjContainer_Unsafe();
        SF_ASSERT(container);

        result = NULL;

        // make sure child's stage obj exists
        AvmDisplayObjContainer* avmObj = ToAvmDisplayObjContainer(container);
        
        GFx::DisplayObjectBase* dobj_ = container->GetChildAt(index);
        if (dobj_)
        {
            GFx::DisplayObject* dobj = static_cast<GFx::DisplayObject*>(dobj_);
            //ToAvmDisplayObj(dobj)->CreateASInstance(true);
            //SF_ASSERT(ToAvmDisplayObj(dobj)->GetAS3Obj());
            if (ToAvmDisplayObj(dobj)->GetAS3Obj()) //?AB
                // do we need to addref child here??? (!AB) 
                result = ToAvmDisplayObj(dobj)->GetAS3Obj();
        }
        else
        {
            GetVM().ThrowRangeError(VM::Error(VM::eParamRangeError, GetVM()));
            return;
        }
        avmObj->RemoveChildAt(index);

//##protect##"instance::DisplayObjectContainer::removeChildAt()"
    }
    void DisplayObjectContainer::removeChildren(const Value& result, SInt32 beginIndex, SInt32 endIndex)
    {
//##protect##"instance::DisplayObjectContainer::removeChildren()"
        SF_UNUSED3(result, beginIndex, endIndex);
        SF_ASSERT(pDispObj && pDispObj->IsDisplayObjContainer());
        GFx::DisplayObjContainer* container = pDispObj->CharToDisplayObjContainer_Unsafe();

        int numCh = int(container->GetNumChildren());
        if (beginIndex < 0 || beginIndex > numCh || endIndex < 0)
            return GetVM().ThrowRangeError(VM::Error(VM::eParamRangeError, GetVM()));

        AvmDisplayObjContainer* avmObj = ToAvmDisplayObjContainer(container);
        for (int i = beginIndex; i < endIndex && i < numCh; ++i)
            avmObj->RemoveChildAt(unsigned(beginIndex));

        WARN_NOT_IMPLEMENTED("instance::DisplayObjectContainer::removeChildren()");
//##protect##"instance::DisplayObjectContainer::removeChildren()"
    }
    void DisplayObjectContainer::setChildIndex(const Value& result, Instances::fl_display::DisplayObject* child, SInt32 index)
    {
//##protect##"instance::DisplayObjectContainer::setChildIndex()"
        SF_UNUSED3(result, child, index);

        if (!child)
            return GetVM().ThrowTypeError(VM::Error(VM::eNullPointerError, GetVM() SF_DEBUG_ARG("child")));

        SF_ASSERT(pDispObj && pDispObj->IsDisplayObjContainer());
        GFx::DisplayObjContainer* container = pDispObj->CharToDisplayObjContainer_Unsafe();
        
        if (index < 0 || index >= (SInt32)container->GetNumChildren())
            return GetVM().ThrowRangeError(VM::Error(VM::eParamRangeError, GetVM()));
       
        if (child->pDispObj)
        {
            if (index < 0) index = 0; //?
            AvmDisplayObjContainer* avmObj = ToAvmDisplayObjContainer(container);
            avmObj->SetChildIndex(child->pDispObj, (unsigned)index);
        }
//##protect##"instance::DisplayObjectContainer::setChildIndex()"
    }
    void DisplayObjectContainer::swapChildren(const Value& result, Instances::fl_display::DisplayObject* child1, Instances::fl_display::DisplayObject* child2)
    {
//##protect##"instance::DisplayObjectContainer::swapChildren()"
        SF_UNUSED3(result, child1, child2);
        SF_ASSERT(pDispObj && pDispObj->IsDisplayObjContainer());
        GFx::DisplayObjContainer* container = pDispObj->CharToDisplayObjContainer_Unsafe();

        if (!child1)
            return GetVM().ThrowTypeError(VM::Error(VM::eNullPointerError, GetVM() SF_DEBUG_ARG("child1")));
        if (!child2)
            return GetVM().ThrowTypeError(VM::Error(VM::eNullPointerError, GetVM() SF_DEBUG_ARG("child2")));

        if (child1->pDispObj && child2->pDispObj)
        {
            AvmDisplayObjContainer* avmObj = ToAvmDisplayObjContainer(container);
            avmObj->SwapChildren(child1->pDispObj, child2->pDispObj);
        }
//##protect##"instance::DisplayObjectContainer::swapChildren()"
    }
    void DisplayObjectContainer::swapChildrenAt(const Value& result, SInt32 index1, SInt32 index2)
    {
//##protect##"instance::DisplayObjectContainer::swapChildrenAt()"
        SF_UNUSED3(result, index1, index2);
        SF_ASSERT(pDispObj && pDispObj->IsDisplayObjContainer());
        GFx::DisplayObjContainer* container = pDispObj->CharToDisplayObjContainer_Unsafe();

        if (index1 < 0 || index1 >= (SInt32)container->GetNumChildren())
            return GetVM().ThrowRangeError(VM::Error(VM::eParamRangeError, GetVM()));
        if (index2 < 0 || index2 >= (SInt32)container->GetNumChildren())
            return GetVM().ThrowRangeError(VM::Error(VM::eParamRangeError, GetVM()));
        AvmDisplayObjContainer* avmObj = ToAvmDisplayObjContainer(container);
        avmObj->SwapChildrenAt((unsigned)index1, (unsigned)index2);
//##protect##"instance::DisplayObjectContainer::swapChildrenAt()"
    }

    SPtr<Instances::fl_text::TextSnapshot> DisplayObjectContainer::textSnapshotGet()
    {
        SPtr<Instances::fl_text::TextSnapshot> result;
        textSnapshotGet(result);
        return result;
    }
    SPtr<Instances::fl_display::DisplayObject> DisplayObjectContainer::addChild(Instances::fl_display::DisplayObject* child)
    {
        SPtr<Instances::fl_display::DisplayObject> result;
        addChild(result, child);
        return result;
    }
    SPtr<Instances::fl_display::DisplayObject> DisplayObjectContainer::addChildAt(Instances::fl_display::DisplayObject* child, SInt32 index)
    {
        SPtr<Instances::fl_display::DisplayObject> result;
        addChildAt(result, child, index);
        return result;
    }
    SPtr<Instances::fl_display::DisplayObject> DisplayObjectContainer::getChildAt(SInt32 index)
    {
        SPtr<Instances::fl_display::DisplayObject> result;
        getChildAt(result, index);
        return result;
    }
    SPtr<Instances::fl_display::DisplayObject> DisplayObjectContainer::getChildByName(const ASString& name)
    {
        SPtr<Instances::fl_display::DisplayObject> result;
        getChildByName(result, name);
        return result;
    }
    SPtr<Instances::fl::Array> DisplayObjectContainer::getObjectsUnderPoint(Instances::fl_geom::Point* point)
    {
        SPtr<Instances::fl::Array> result;
        getObjectsUnderPoint(result, point);
        return result;
    }
    SPtr<Instances::fl_display::DisplayObject> DisplayObjectContainer::removeChildAt(SInt32 index)
    {
        SPtr<Instances::fl_display::DisplayObject> result;
        removeChildAt(result, index);
        return result;
    }
//##protect##"instance$methods"
    DisplayObjectContainer::~DisplayObjectContainer()
    {
    }

    void DisplayObjectContainer::ForEachChild_GC(Collector* prcc, GcOp op) const
    {
        InteractiveObject::ForEachChild_GC(prcc, op);
        if (HasDisplayObject())
        {
            GFx::DisplayObjContainer* dobj = GetDisplayObjContainer();
            unsigned n = dobj->GetNumChildren();
            for (unsigned i = 0; i < n; ++i)
            {
                GFx::DisplayObjectBase* chb = dobj->GetChildAt(i);
                GFx::DisplayObject* ch = chb->CharToScriptableObject();
                if (ch)
                {
                    SPtr<Instances::fl_display::DisplayObject>* pas3 = ToAvmDisplayObj(ch)->GetCollectibleAS3Ptr();
                    //!AB: we don't need to "release" or "disable" the collectible pointer we are getting
                    // from 'ch'. The only reason to iterate through it here is to get its children during
                    // 'mark' or 'scan' operations.
                    if (pas3 && (prcc->IsInCollect() || (!Collector::IsDisableOp(op) && !Collector::IsReleaseOp(op))))
                    {
                        AS3::ForEachChild_GC<DisplayObject, Mem_Stat>(prcc, *pas3, op SF_DEBUG_ARG(*this));
                    }
                }
            }
        }
    }
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{
    // const UInt16 DisplayObjectContainer::tito[DisplayObjectContainer::ThunkInfoNum] = {
    //    0, 1, 3, 4, 5, 7, 8, 10, 13, 15, 17, 19, 21, 23, 25, 26, 28, 31, 34, 37, 
    // };
    const TypeInfo* DisplayObjectContainer::tit[40] = {
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::int_TI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl_text::TextSnapshotTI, 
        &AS3::fl_display::DisplayObjectTI, &AS3::fl_display::DisplayObjectTI, 
        &AS3::fl_display::DisplayObjectTI, &AS3::fl_display::DisplayObjectTI, &AS3::fl::int_TI, 
        &AS3::fl::BooleanTI, &AS3::fl_geom::PointTI, 
        &AS3::fl::BooleanTI, &AS3::fl_display::DisplayObjectTI, 
        &AS3::fl_display::DisplayObjectTI, &AS3::fl::int_TI, 
        &AS3::fl_display::DisplayObjectTI, &AS3::fl::StringTI, 
        &AS3::fl::int_TI, &AS3::fl_display::DisplayObjectTI, 
        &AS3::fl::ArrayTI, &AS3::fl_geom::PointTI, 
        &AS3::fl_display::DisplayObjectTI, 
        &AS3::fl_display::DisplayObjectTI, &AS3::fl::int_TI, 
        NULL, &AS3::fl::int_TI, &AS3::fl::int_TI, 
        NULL, &AS3::fl_display::DisplayObjectTI, &AS3::fl::int_TI, 
        NULL, &AS3::fl_display::DisplayObjectTI, &AS3::fl_display::DisplayObjectTI, 
        NULL, &AS3::fl::int_TI, &AS3::fl::int_TI, 
    };
    const Abc::ConstValue DisplayObjectContainer::dva[1] = {
        {Abc::CONSTANT_Int, 1}, 
    };
    const ThunkInfo DisplayObjectContainer::ti[DisplayObjectContainer::ThunkInfoNum] = {
        {TFunc_Instances_DisplayObjectContainer_mouseChildrenGet::Func, &DisplayObjectContainer::tit[0], "mouseChildren", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplayObjectContainer_mouseChildrenSet::Func, &DisplayObjectContainer::tit[1], "mouseChildren", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObjectContainer_numChildrenGet::Func, &DisplayObjectContainer::tit[3], "numChildren", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplayObjectContainer_tabChildrenGet::Func, &DisplayObjectContainer::tit[4], "tabChildren", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplayObjectContainer_tabChildrenSet::Func, &DisplayObjectContainer::tit[5], "tabChildren", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObjectContainer_textSnapshotGet::Func, &DisplayObjectContainer::tit[7], "textSnapshot", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_DisplayObjectContainer_addChild::Func, &DisplayObjectContainer::tit[8], "addChild", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObjectContainer_addChildAt::Func, &DisplayObjectContainer::tit[10], "addChildAt", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Instances_DisplayObjectContainer_areInaccessibleObjectsUnderPoint::Func, &DisplayObjectContainer::tit[13], "areInaccessibleObjectsUnderPoint", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObjectContainer_contains::Func, &DisplayObjectContainer::tit[15], "contains", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObjectContainer_getChildAt::Func, &DisplayObjectContainer::tit[17], "getChildAt", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObjectContainer_getChildByName::Func, &DisplayObjectContainer::tit[19], "getChildByName", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObjectContainer_getChildIndex::Func, &DisplayObjectContainer::tit[21], "getChildIndex", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObjectContainer_getObjectsUnderPoint::Func, &DisplayObjectContainer::tit[23], "getObjectsUnderPoint", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObjectContainer_removeChild::Func, &DisplayObjectContainer::tit[25], "removeChild", NULL, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1, 0, NULL},
        {TFunc_Instances_DisplayObjectContainer_removeChildAt::Func, &DisplayObjectContainer::tit[26], "removeChildAt", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_DisplayObjectContainer_removeChildren::Func, &DisplayObjectContainer::tit[28], "removeChildren", NULL, Abc::NS_Public, CT_Method, 0, 2, 0, 1, &DisplayObjectContainer::dva[0]},
        {TFunc_Instances_DisplayObjectContainer_setChildIndex::Func, &DisplayObjectContainer::tit[31], "setChildIndex", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Instances_DisplayObjectContainer_swapChildren::Func, &DisplayObjectContainer::tit[34], "swapChildren", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Instances_DisplayObjectContainer_swapChildrenAt::Func, &DisplayObjectContainer::tit[37], "swapChildrenAt", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
    };

    DisplayObjectContainer::DisplayObjectContainer(VM& vm, const ClassInfo& ci)
    : fl_display::InteractiveObject(vm, ci)
    {
//##protect##"InstanceTraits::DisplayObjectContainer::DisplayObjectContainer()"
        SetTraitsType(Traits_DisplayObjectContainer);
//##protect##"InstanceTraits::DisplayObjectContainer::DisplayObjectContainer()"

    }

    void DisplayObjectContainer::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<DisplayObjectContainer&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_display
{

    DisplayObjectContainer::DisplayObjectContainer(VM& vm, const ClassInfo& ci)
    : fl_display::InteractiveObject(vm, ci)
    {
//##protect##"ClassTraits::DisplayObjectContainer::DisplayObjectContainer()"
        SetTraitsType(Traits_DisplayObjectContainer);
//##protect##"ClassTraits::DisplayObjectContainer::DisplayObjectContainer()"

    }

    Pickable<Traits> DisplayObjectContainer::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) DisplayObjectContainer(vm, AS3::fl_display::DisplayObjectContainerCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::DisplayObjectContainerCI));
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
    const TypeInfo DisplayObjectContainerTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_display::DisplayObjectContainer::InstanceType),
        0,
        0,
        InstanceTraits::fl_display::DisplayObjectContainer::ThunkInfoNum,
        0,
        "DisplayObjectContainer", "flash.display", &fl_display::InteractiveObjectTI,
        TypeInfo::None
    };

    const ClassInfo DisplayObjectContainerCI = {
        &DisplayObjectContainerTI,
        ClassTraits::fl_display::DisplayObjectContainer::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_display::DisplayObjectContainer::ti,
        NULL,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

