//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Events_Event.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Events_Event.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../../AS3_MovieRoot.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_events::Event, Instances::fl_events::Event::mid_bubblesGet, bool> TFunc_Instances_Event_bubblesGet;
typedef ThunkFunc0<Instances::fl_events::Event, Instances::fl_events::Event::mid_cancelableGet, bool> TFunc_Instances_Event_cancelableGet;
typedef ThunkFunc0<Instances::fl_events::Event, Instances::fl_events::Event::mid_currentTargetGet, SPtr<Instances::fl::Object> > TFunc_Instances_Event_currentTargetGet;
typedef ThunkFunc0<Instances::fl_events::Event, Instances::fl_events::Event::mid_eventPhaseGet, UInt32> TFunc_Instances_Event_eventPhaseGet;
typedef ThunkFunc0<Instances::fl_events::Event, Instances::fl_events::Event::mid_targetGet, SPtr<Instances::fl::Object> > TFunc_Instances_Event_targetGet;
typedef ThunkFunc0<Instances::fl_events::Event, Instances::fl_events::Event::mid_typeGet, ASString> TFunc_Instances_Event_typeGet;
typedef ThunkFunc0<Instances::fl_events::Event, Instances::fl_events::Event::mid_clone, SPtr<Instances::fl_events::Event> > TFunc_Instances_Event_clone;
typedef ThunkFunc2<Instances::fl_events::Event, Instances::fl_events::Event::mid_formatToString, Value, unsigned, const Value*> TFunc_Instances_Event_formatToString;
typedef ThunkFunc0<Instances::fl_events::Event, Instances::fl_events::Event::mid_isDefaultPrevented, bool> TFunc_Instances_Event_isDefaultPrevented;
typedef ThunkFunc0<Instances::fl_events::Event, Instances::fl_events::Event::mid_preventDefault, const Value> TFunc_Instances_Event_preventDefault;
typedef ThunkFunc0<Instances::fl_events::Event, Instances::fl_events::Event::mid_stopImmediatePropagation, const Value> TFunc_Instances_Event_stopImmediatePropagation;
typedef ThunkFunc0<Instances::fl_events::Event, Instances::fl_events::Event::mid_stopPropagation, const Value> TFunc_Instances_Event_stopPropagation;
typedef ThunkFunc0<Instances::fl_events::Event, Instances::fl_events::Event::mid_toString, ASString> TFunc_Instances_Event_toString;

template <> const TFunc_Instances_Event_bubblesGet::TMethod TFunc_Instances_Event_bubblesGet::Method = &Instances::fl_events::Event::bubblesGet;
template <> const TFunc_Instances_Event_cancelableGet::TMethod TFunc_Instances_Event_cancelableGet::Method = &Instances::fl_events::Event::cancelableGet;
template <> const TFunc_Instances_Event_currentTargetGet::TMethod TFunc_Instances_Event_currentTargetGet::Method = &Instances::fl_events::Event::currentTargetGet;
template <> const TFunc_Instances_Event_eventPhaseGet::TMethod TFunc_Instances_Event_eventPhaseGet::Method = &Instances::fl_events::Event::eventPhaseGet;
template <> const TFunc_Instances_Event_targetGet::TMethod TFunc_Instances_Event_targetGet::Method = &Instances::fl_events::Event::targetGet;
template <> const TFunc_Instances_Event_typeGet::TMethod TFunc_Instances_Event_typeGet::Method = &Instances::fl_events::Event::typeGet;
template <> const TFunc_Instances_Event_clone::TMethod TFunc_Instances_Event_clone::Method = &Instances::fl_events::Event::clone;
template <> const TFunc_Instances_Event_formatToString::TMethod TFunc_Instances_Event_formatToString::Method = &Instances::fl_events::Event::formatToString;
template <> const TFunc_Instances_Event_isDefaultPrevented::TMethod TFunc_Instances_Event_isDefaultPrevented::Method = &Instances::fl_events::Event::isDefaultPrevented;
template <> const TFunc_Instances_Event_preventDefault::TMethod TFunc_Instances_Event_preventDefault::Method = &Instances::fl_events::Event::preventDefault;
template <> const TFunc_Instances_Event_stopImmediatePropagation::TMethod TFunc_Instances_Event_stopImmediatePropagation::Method = &Instances::fl_events::Event::stopImmediatePropagation;
template <> const TFunc_Instances_Event_stopPropagation::TMethod TFunc_Instances_Event_stopPropagation::Method = &Instances::fl_events::Event::stopPropagation;
template <> const TFunc_Instances_Event_toString::TMethod TFunc_Instances_Event_toString::Method = &Instances::fl_events::Event::toString;

namespace Instances { namespace fl_events
{
    Event::Event(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
//##protect##"instance::Event::Event()$data"
    , Type(GetVM().GetStringManager().CreateEmptyString())
    , Phase(Phase_Target)
    , Bubbles(false)
    , Cancelable(false)
    , DefaultPrevented(false)
    , PropagationStopped(false)
    , ImmPropagationStopped(false)
    , WasDispatched(false)
//##protect##"instance::Event::Event()$data"
    {
//##protect##"instance::Event::Event()$code"
//##protect##"instance::Event::Event()$code"
    }

    void Event::bubblesGet(bool& result)
    {
//##protect##"instance::Event::bubblesGet()"
        result = Bubbles;
//##protect##"instance::Event::bubblesGet()"
    }
    void Event::cancelableGet(bool& result)
    {
//##protect##"instance::Event::cancelableGet()"
        result = Cancelable;
//##protect##"instance::Event::cancelableGet()"
    }
    void Event::currentTargetGet(SPtr<Instances::fl::Object>& result)
    {
//##protect##"instance::Event::currentTargetGet()"
        result = CurrentTarget.GetPtr();
//##protect##"instance::Event::currentTargetGet()"
    }
    void Event::eventPhaseGet(UInt32& result)
    {
//##protect##"instance::Event::eventPhaseGet()"
        result = Phase;
//##protect##"instance::Event::eventPhaseGet()"
    }
    void Event::targetGet(SPtr<Instances::fl::Object>& result)
    {
//##protect##"instance::Event::targetGet()"
        result = Target.GetPtr();
//##protect##"instance::Event::targetGet()"
    }
    void Event::typeGet(ASString& result)
    {
//##protect##"instance::Event::typeGet()"
        result = Type;
//##protect##"instance::Event::typeGet()"
    }
    void Event::clone(SPtr<Instances::fl_events::Event>& result)
    {
//##protect##"instance::Event::clone()"
        SF_UNUSED1(result);
        SPtr<fl_events::Event> e = Clone();
        result = e;
//##protect##"instance::Event::clone()"
    }
    void Event::formatToString(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::Event::formatToString()"
        SF_UNUSED3(result, argc, argv);
        if (argc < 1)
        {
            GetVM().ThrowArgumentError(VM::Error(VM::eWrongArgumentCountError, GetVM() SF_DEBUG_ARG("Event::formatToString") SF_DEBUG_ARG(1) SF_DEBUG_ARG(1) SF_DEBUG_ARG(argc)));
            return;
        }
        StringBuffer sb(GetVM().GetMemoryHeap());
        char str[1024];

        ASVM& vm = static_cast<ASVM&>(GetVM());
        ASString asstr(vm.GetBuiltin(AS3Builtin_empty_));
        argv[0].Convert2String(asstr).DoNotCheck();

        SFsprintf(str, sizeof(str), "[%s", asstr.ToCStr());
        sb.AppendString(str);

        for (unsigned i = 1; i < argc; ++i)
        {
            argv[i].Convert2String(asstr).DoNotCheck();
            AS3::Multiname mn(vm.GetPublicNamespace(), asstr);
            AS3::PropRef   prop;
            
            FindProperty(prop, mn);
            if (!prop)
            {
                SFsprintf(str, sizeof(str), "Property %s not found on flash.events.Event and there is no default value.",
                    asstr.ToCStr());

                vm.ThrowReferenceError(VM::Error(VM::ePropertyNotFoundError, vm SF_DEBUG_ARG(str)));
                return;
            }

            AS3::Value asval;
            if (!prop.GetSlotValueUnsafe(vm, asval))
            {
                return; //?
            }
            
            SFsprintf(str, sizeof(str), " %s=", asstr.ToCStr());
            sb.AppendString(str);

            bool quotes = asval.IsString();

            asval.Convert2String(asstr).DoNotCheck();
            if (quotes)
                sb.AppendChar('"');
            SFsprintf(str, sizeof(str), "%s", asstr.ToCStr());
            sb.AppendString(str);
            if (quotes)
                sb.AppendChar('"');
        }

        sb.AppendString("]", 2); // incl trailing zero
        result = vm.GetStringManager().CreateString(sb.ToCStr());
//##protect##"instance::Event::formatToString()"
    }
    void Event::isDefaultPrevented(bool& result)
    {
//##protect##"instance::Event::isDefaultPrevented()"
        SF_UNUSED1(result);
        result = DefaultPrevented;
//##protect##"instance::Event::isDefaultPrevented()"
    }
    void Event::preventDefault(const Value& result)
    {
//##protect##"instance::Event::preventDefault()"
        SF_UNUSED1(result);
        if (Cancelable)
            DefaultPrevented = true;
//##protect##"instance::Event::preventDefault()"
    }
    void Event::stopImmediatePropagation(const Value& result)
    {
//##protect##"instance::Event::stopImmediatePropagation()"
        SF_UNUSED1(result);
        ImmPropagationStopped = true;
//##protect##"instance::Event::stopImmediatePropagation()"
    }
    void Event::stopPropagation(const Value& result)
    {
//##protect##"instance::Event::stopPropagation()"
        SF_UNUSED1(result);
        PropagationStopped = true;
//##protect##"instance::Event::stopPropagation()"
    }
    void Event::toString(ASString& result)
    {
//##protect##"instance::Event::toString()"
        SF_UNUSED1(result);
        Value res;
        ASVM& vm = static_cast<ASVM&>(GetVM());
        Value params[] = {
            vm.GetStringManager().CreateConstString(GetEventName()),
            vm.GetStringManager().CreateConstString("type"),
            vm.GetStringManager().CreateConstString("bubbles"),
            vm.GetStringManager().CreateConstString("cancelable"),
            vm.GetStringManager().CreateConstString("eventPhase")
        };
        formatToString(res, sizeof(params)/sizeof(params[0]), params);
        res.Convert2String(result).DoNotCheck();
//##protect##"instance::Event::toString()"
    }

//##protect##"instance$methods"
    Event::~Event()
    {
    }

    // AS3 ctor of Event class
    void Event::AS3Constructor(unsigned argc, const Value* argv)
    {
        if (argc < 1)
        {
            GetVM().ThrowArgumentError(VM::Error(VM::eWrongArgumentCountError, GetVM() SF_DEBUG_ARG("Event::AS3Constructor") SF_DEBUG_ARG(1) SF_DEBUG_ARG(1) SF_DEBUG_ARG(argc)));
            return;
        }
        argv[0].Convert2String(Type).DoNotCheck();
        if (argc >= 2)
        {
            Bubbles = argv[1].Convert2Boolean();
        }
        if (argc >= 3)
        {
            Cancelable = argv[2].Convert2Boolean();
        }
    }

    SPtr<Instances::fl_events::Event> Event::Clone() const
    {
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        StringManager& sm = asvm.GetStringManager();

        // Note: properties, such as type, bubbles, cancelable, target, currentTarget
        // and eventPhase might be overriden by a custom user defined class. Therefore,
        // need to take care about copying them correctly via GetProperty call (!AB).

        SPtr<Instances::fl_events::Event> e;
        Value params[] = { Value(Type), Value(Bubbles), Value(Cancelable) };

        if (GetTraits().IsUserDefined())
        {
            // no idea why GetProperty is not const, so using const_cast.... (!AB)
            Instances::fl_events::Event* _this = const_cast<Instances::fl_events::Event*>(this);

            Multiname typePropName(asvm.GetPublicNamespace(), sm.CreateConstString("type"));
            Multiname bubblesPropName(asvm.GetPublicNamespace(), sm.CreateConstString("bubbles"));
            Multiname cancelablePropName(asvm.GetPublicNamespace(), sm.CreateConstString("cancelable"));
            Value typeProp, bubblesProp, cancelableProp;
            if (_this->GetProperty(typePropName, typeProp))
                params[0] = typeProp;
            if (_this->GetProperty(bubblesPropName, bubblesProp))
                params[1] = bubblesProp;
            if (_this->GetProperty(cancelablePropName, cancelableProp))
                params[2] = cancelableProp;
        }

        asvm.ConstructInstance(e, GetEventClass(), 3, params);

        e->CurrentTarget            = CurrentTarget;
        e->Target                   = Target;
        e->DefaultPrevented         = DefaultPrevented;
        e->PropagationStopped       = PropagationStopped;
        e->ImmPropagationStopped    = ImmPropagationStopped;
        e->Phase                    = Phase;

        if (GetTraits().IsUserDefined())
        {
            // no idea why GetProperty is not const, so using const_cast.... (!AB)
            Instances::fl_events::Event* _this = const_cast<Instances::fl_events::Event*>(this);

            Multiname targetPropName(asvm.GetPublicNamespace(), sm.CreateConstString("target"));
            Multiname currentTargetPropName(asvm.GetPublicNamespace(), sm.CreateConstString("currentTarget"));
            Multiname eventPhasePropName(asvm.GetPublicNamespace(), sm.CreateConstString("eventPhase"));
            Value targetProp, currentTargetProp, eventPhaseProp;
            if (_this->GetProperty(targetPropName, targetProp) && targetProp.IsObject())
                e->Target = static_cast<Instances::fl::Object*>(targetProp.GetObject());
            if (_this->GetProperty(currentTargetPropName, currentTargetProp) && currentTargetProp.IsObject())
                e->CurrentTarget = static_cast<Instances::fl::Object*>(currentTargetProp.GetObject());
            if (_this->GetProperty(eventPhasePropName, eventPhaseProp))
            {
                eventPhaseProp.ToUInt32Value().DoNotCheck();
                e->Phase = eventPhaseProp.AsUInt();
            }
        }
        return e;
    }

    bool Event::NeedsCloning() const 
    { 
        if (WasDispatched)
            return true;
        if (GetTraits().IsUserDefined())
        {
            // check if target property is modified. If yes - need to clone event (return true).
            Multiname targetPropName(GetVM().GetPublicNamespace(), GetVM().GetStringManager().CreateConstString("target"));
            Value targetProp;
            // no idea why GetProperty is not const, so using const_cast.... (!AB)
            Instances::fl_events::Event* _this = const_cast<Instances::fl_events::Event*>(this);
            if (_this->GetProperty(targetPropName, targetProp) && targetProp.IsObject())
                return (Target != static_cast<Instances::fl::Object*>(targetProp.GetObject()));
        }
        return false; 
    }

    void Event::ForEachChild_GC(Collector* prcc, GcOp op) const
    {
        Instances::fl::Object::ForEachChild_GC(prcc, op);

        AS3::ForEachChild_GC<Object, Mem_Stat>(prcc, CurrentTarget, op SF_DEBUG_ARG(*this));
        AS3::ForEachChild_GC<Object, Mem_Stat>(prcc, Target, op SF_DEBUG_ARG(*this));
    }

    AS3::Object* Event::GetEventClass() const 
    { 
        return static_cast<ASVM&>(GetVM()).EventClass; 
    }

//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_events
{
    // const UInt16 Event::tito[Event::ThunkInfoNum] = {
    //    0, 1, 2, 3, 4, 5, 6, 7, 9, 10, 11, 12, 13, 
    // };
    const TypeInfo* Event::tit[14] = {
        &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        &AS3::fl::ObjectTI, 
        &AS3::fl::uintTI, 
        &AS3::fl::ObjectTI, 
        &AS3::fl::StringTI, 
        &AS3::fl_events::EventTI, 
        &AS3::fl::StringTI, &AS3::fl::StringTI, 
        &AS3::fl::BooleanTI, 
        NULL, 
        NULL, 
        NULL, 
        &AS3::fl::StringTI, 
    };
    const ThunkInfo Event::ti[Event::ThunkInfoNum] = {
        {TFunc_Instances_Event_bubblesGet::Func, &Event::tit[0], "bubbles", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Event_cancelableGet::Func, &Event::tit[1], "cancelable", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Event_currentTargetGet::Func, &Event::tit[2], "currentTarget", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Event_eventPhaseGet::Func, &Event::tit[3], "eventPhase", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Event_targetGet::Func, &Event::tit[4], "target", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Event_typeGet::Func, &Event::tit[5], "type", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Event_clone::Func, &Event::tit[6], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Event_formatToString::Func, &Event::tit[7], "formatToString", NULL, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1, 0, NULL},
        {TFunc_Instances_Event_isDefaultPrevented::Func, &Event::tit[9], "isDefaultPrevented", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Event_preventDefault::Func, &Event::tit[10], "preventDefault", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Event_stopImmediatePropagation::Func, &Event::tit[11], "stopImmediatePropagation", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Event_stopPropagation::Func, &Event::tit[12], "stopPropagation", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Event_toString::Func, &Event::tit[13], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    Event::Event(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::Event::Event()"
//##protect##"InstanceTraits::Event::Event()"

    }

    void Event::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<Event&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl_events
{
    Event::Event(ClassTraits::Traits& t)
    : Class(t)
    , ACTIVATE("activate")
    , ADDED("added")
    , ADDED_TO_STAGE("addedToStage")
    , CANCEL("cancel")
    , CHANGE("change")
    , CLOSE("close")
    , CLOSING("closing")
    , COMPLETE("complete")
    , CONNECT("connect")
    , DEACTIVATE("deactivate")
    , DISPLAYING("displaying")
    , ENTER_FRAME("enterFrame")
    , EXITING("exiting")
    , EXIT_FRAME("exitFrame")
    , FRAME_CONSTRUCTED("frameConstructed")
    , FULLSCREEN("fullScreen")
    , HTML_BOUNDS_CHANGE("htmlBoundsChange")
    , HTML_DOM_INITIALIZE("htmlDOMInitialize")
    , HTML_RENDER("htmlRender")
    , ID3("id3")
    , INIT("init")
    , LOCATION_CHANGE("locationChange")
    , MOUSE_LEAVE("mouseLeave")
    , NETWORK_CHANGE("networkChange")
    , OPEN("open")
    , REMOVED("removed")
    , REMOVED_FROM_STAGE("removedFromStage")
    , RENDER("render")
    , RESIZE("resize")
    , SCROLL("scroll")
    , SELECT("select")
    , SOUND_COMPLETE("soundComplete")
    , TAB_CHILDREN_CHANGE("tabChildrenChange")
    , TAB_ENABLED_CHANGE("tabEnabledChange")
    , TAB_INDEX_CHANGE("tabIndexChange")
    , UNLOAD("unload")
    , USER_IDLE("userIdle")
    , USER_PRESENT("userPresent")
    {
//##protect##"class_::Event::Event()"
//##protect##"class_::Event::Event()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_events
{
    const MemberInfo Event::mi[Event::MemberInfoNum] = {
        {"ACTIVATE", NULL, OFFSETOF(Classes::fl_events::Event, ACTIVATE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"ADDED", NULL, OFFSETOF(Classes::fl_events::Event, ADDED), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"ADDED_TO_STAGE", NULL, OFFSETOF(Classes::fl_events::Event, ADDED_TO_STAGE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"CANCEL", NULL, OFFSETOF(Classes::fl_events::Event, CANCEL), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"CHANGE", NULL, OFFSETOF(Classes::fl_events::Event, CHANGE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"CLOSE", NULL, OFFSETOF(Classes::fl_events::Event, CLOSE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"CLOSING", NULL, OFFSETOF(Classes::fl_events::Event, CLOSING), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"COMPLETE", NULL, OFFSETOF(Classes::fl_events::Event, COMPLETE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"CONNECT", NULL, OFFSETOF(Classes::fl_events::Event, CONNECT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"DEACTIVATE", NULL, OFFSETOF(Classes::fl_events::Event, DEACTIVATE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"DISPLAYING", NULL, OFFSETOF(Classes::fl_events::Event, DISPLAYING), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"ENTER_FRAME", NULL, OFFSETOF(Classes::fl_events::Event, ENTER_FRAME), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"EXITING", NULL, OFFSETOF(Classes::fl_events::Event, EXITING), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"EXIT_FRAME", NULL, OFFSETOF(Classes::fl_events::Event, EXIT_FRAME), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"FRAME_CONSTRUCTED", NULL, OFFSETOF(Classes::fl_events::Event, FRAME_CONSTRUCTED), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"FULLSCREEN", NULL, OFFSETOF(Classes::fl_events::Event, FULLSCREEN), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"HTML_BOUNDS_CHANGE", NULL, OFFSETOF(Classes::fl_events::Event, HTML_BOUNDS_CHANGE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"HTML_DOM_INITIALIZE", NULL, OFFSETOF(Classes::fl_events::Event, HTML_DOM_INITIALIZE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"HTML_RENDER", NULL, OFFSETOF(Classes::fl_events::Event, HTML_RENDER), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"ID3", NULL, OFFSETOF(Classes::fl_events::Event, ID3), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"INIT", NULL, OFFSETOF(Classes::fl_events::Event, INIT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"LOCATION_CHANGE", NULL, OFFSETOF(Classes::fl_events::Event, LOCATION_CHANGE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"MOUSE_LEAVE", NULL, OFFSETOF(Classes::fl_events::Event, MOUSE_LEAVE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"NETWORK_CHANGE", NULL, OFFSETOF(Classes::fl_events::Event, NETWORK_CHANGE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"OPEN", NULL, OFFSETOF(Classes::fl_events::Event, OPEN), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"REMOVED", NULL, OFFSETOF(Classes::fl_events::Event, REMOVED), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"REMOVED_FROM_STAGE", NULL, OFFSETOF(Classes::fl_events::Event, REMOVED_FROM_STAGE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"RENDER", NULL, OFFSETOF(Classes::fl_events::Event, RENDER), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"RESIZE", NULL, OFFSETOF(Classes::fl_events::Event, RESIZE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"SCROLL", NULL, OFFSETOF(Classes::fl_events::Event, SCROLL), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"SELECT", NULL, OFFSETOF(Classes::fl_events::Event, SELECT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"SOUND_COMPLETE", NULL, OFFSETOF(Classes::fl_events::Event, SOUND_COMPLETE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"TAB_CHILDREN_CHANGE", NULL, OFFSETOF(Classes::fl_events::Event, TAB_CHILDREN_CHANGE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"TAB_ENABLED_CHANGE", NULL, OFFSETOF(Classes::fl_events::Event, TAB_ENABLED_CHANGE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"TAB_INDEX_CHANGE", NULL, OFFSETOF(Classes::fl_events::Event, TAB_INDEX_CHANGE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"UNLOAD", NULL, OFFSETOF(Classes::fl_events::Event, UNLOAD), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"USER_IDLE", NULL, OFFSETOF(Classes::fl_events::Event, USER_IDLE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"USER_PRESENT", NULL, OFFSETOF(Classes::fl_events::Event, USER_PRESENT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    Event::Event(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::Event::Event()"
//##protect##"ClassTraits::Event::Event()"

    }

    Pickable<Traits> Event::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) Event(vm, AS3::fl_events::EventCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_events::EventCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_events
{
    const TypeInfo EventTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_events::Event::InstanceType),
        0,
        ClassTraits::fl_events::Event::MemberInfoNum,
        InstanceTraits::fl_events::Event::ThunkInfoNum,
        0,
        "Event", "flash.events", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo EventCI = {
        &EventTI,
        ClassTraits::fl_events::Event::MakeClassTraits,
        NULL,
        ClassTraits::fl_events::Event::mi,
        InstanceTraits::fl_events::Event::ti,
        NULL,
    };
}; // namespace fl_events


}}} // namespace Scaleform { namespace GFx { namespace AS3

