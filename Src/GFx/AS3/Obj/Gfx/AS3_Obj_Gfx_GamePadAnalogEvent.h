//##protect##"disclaimer"
/**********************************************************************

Filename    :   AS3_Obj_Gfx_GamePadAnalogEvent.h
Content     :   
Created     :   Jun, 2011
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**********************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Gfx_GamePadAnalogEvent_H
#define INC_AS3_Obj_Gfx_GamePadAnalogEvent_H

#include "../Events/AS3_Obj_Events_Event.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_gfx
{
    extern const TypeInfo GamePadAnalogEventTI;
    extern const ClassInfo GamePadAnalogEventCI;
} // namespace fl_gfx
namespace fl
{
    extern const TypeInfo uintTI;
    extern const ClassInfo uintCI;
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl
namespace fl_events
{
    extern const TypeInfo EventTI;
    extern const ClassInfo EventCI;
} // namespace fl_events

namespace ClassTraits { namespace fl_gfx
{
    class GamePadAnalogEvent;
}}

namespace InstanceTraits { namespace fl_gfx
{
    class GamePadAnalogEvent;
}}

namespace Classes { namespace fl_gfx
{
    class GamePadAnalogEvent;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_gfx
{
    class GamePadAnalogEvent : public Instances::fl_events::Event
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_gfx::GamePadAnalogEvent;
        
    public:
        typedef GamePadAnalogEvent SelfType;
        typedef Classes::fl_gfx::GamePadAnalogEvent ClassType;
        typedef InstanceTraits::fl_gfx::GamePadAnalogEvent TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_gfx::GamePadAnalogEventTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_gfx::GamePadAnalogEvent"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        GamePadAnalogEvent(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        virtual void AS3Constructor(unsigned argc, const Value* argv);
        virtual AS3::Object* GetEventClass() const;
        virtual SPtr<Instances::fl_events::Event> Clone() const;
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_codeGet, 
            mid_codeSet, 
            mid_controllerIdxGet, 
            mid_controllerIdxSet, 
            mid_xvalueGet, 
            mid_xvalueSet, 
            mid_yvalueGet, 
            mid_yvalueSet, 
            mid_clone, 
            mid_toString, 
        };
        void codeGet(UInt32& result);
        void codeSet(const Value& result, UInt32 value);
        void controllerIdxGet(UInt32& result);
        void controllerIdxSet(const Value& result, UInt32 value);
        void xvalueGet(Value::Number& result);
        void xvalueSet(const Value& result, Value::Number value);
        void yvalueGet(Value::Number& result);
        void yvalueSet(const Value& result, Value::Number value);
        void clone(SPtr<Instances::fl_events::Event>& result);
        void toString(ASString& result);

        // C++ friendly wrappers for AS3 methods.
        UInt32 codeGet()
        {
            UInt32 result;
            codeGet(result);
            return result;
        }
        void codeSet(UInt32 value)
        {
            codeSet(Value::GetUndefined(), value);
        }
        UInt32 controllerIdxGet()
        {
            UInt32 result;
            controllerIdxGet(result);
            return result;
        }
        void controllerIdxSet(UInt32 value)
        {
            controllerIdxSet(Value::GetUndefined(), value);
        }
        Value::Number xvalueGet()
        {
            Value::Number result;
            xvalueGet(result);
            return result;
        }
        void xvalueSet(Value::Number value)
        {
            xvalueSet(Value::GetUndefined(), value);
        }
        Value::Number yvalueGet()
        {
            Value::Number result;
            yvalueGet(result);
            return result;
        }
        void yvalueSet(Value::Number value)
        {
            yvalueSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_events::Event> clone();
        ASString toString()
        {
            ASString result(GetStringManager().CreateEmptyString());
            toString(result);
            return result;
        }

//##protect##"instance$data"
    public:
        UInt32  Code;
        UInt32  ControllerIdx;
        Double  XValue, YValue;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_gfx
{
    class GamePadAnalogEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::GamePadAnalogEvent"; }
#endif
    public:
        typedef Instances::fl_gfx::GamePadAnalogEvent InstanceType;

    public:
        GamePadAnalogEvent(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_gfx::GamePadAnalogEvent> MakeInstance(GamePadAnalogEvent& t)
        {
            return Pickable<Instances::fl_gfx::GamePadAnalogEvent>(new(t.Alloc()) Instances::fl_gfx::GamePadAnalogEvent(t));
        }
        SPtr<Instances::fl_gfx::GamePadAnalogEvent> MakeInstanceS(GamePadAnalogEvent& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 10 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[14];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_gfx
{
    class GamePadAnalogEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::GamePadAnalogEvent"; }
#endif
    public:
        typedef Classes::fl_gfx::GamePadAnalogEvent ClassType;
        typedef InstanceTraits::fl_gfx::GamePadAnalogEvent InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        GamePadAnalogEvent(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 1 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_gfx
{
    class GamePadAnalogEvent : public Class
    {
        friend class ClassTraits::fl_gfx::GamePadAnalogEvent;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_gfx::GamePadAnalogEventTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::GamePadAnalogEvent"; }
#endif
    public:
        typedef GamePadAnalogEvent SelfType;
        typedef GamePadAnalogEvent ClassType;
        
    private:
        GamePadAnalogEvent(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* CHANGE;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

