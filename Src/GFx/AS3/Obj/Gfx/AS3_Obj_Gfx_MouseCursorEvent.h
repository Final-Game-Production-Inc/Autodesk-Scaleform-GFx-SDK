//##protect##"disclaimer"
/**********************************************************************

Filename    :   .h
Content     :   
Created     :   Jul, 2011
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**********************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Gfx_MouseCursorEvent_H
#define INC_AS3_Obj_Gfx_MouseCursorEvent_H

#include "../Events/AS3_Obj_Events_Event.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_gfx
{
    extern const TypeInfo MouseCursorEventTI;
    extern const ClassInfo MouseCursorEventCI;
} // namespace fl_gfx

namespace ClassTraits { namespace fl_gfx
{
    class MouseCursorEvent;
}}

namespace InstanceTraits { namespace fl_gfx
{
    class MouseCursorEvent;
}}

namespace Classes { namespace fl_gfx
{
    class MouseCursorEvent;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_gfx
{
    class MouseCursorEvent : public Instances::fl_events::Event
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_gfx::MouseCursorEvent;
        
    public:
        typedef MouseCursorEvent SelfType;
        typedef Classes::fl_gfx::MouseCursorEvent ClassType;
        typedef InstanceTraits::fl_gfx::MouseCursorEvent TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_gfx::MouseCursorEventTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_gfx::MouseCursorEvent"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        MouseCursorEvent(InstanceTraits::Traits& t);

//##protect##"instance$methods"
    public:
        void SetControllerIdx(UInt32 mi)  { mouseIdx = mi; }
        void SetCursor(const ASString& c) { cursor = c; }
//##protect##"instance$methods"

    public:
        // AS3 API members.
        ASString cursor;
        UInt32 mouseIdx;

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_gfx
{
    class MouseCursorEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::MouseCursorEvent"; }
#endif
    public:
        typedef Instances::fl_gfx::MouseCursorEvent InstanceType;

    public:
        MouseCursorEvent(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_gfx::MouseCursorEvent> MakeInstance(MouseCursorEvent& t)
        {
            return Pickable<Instances::fl_gfx::MouseCursorEvent>(new(t.Alloc()) Instances::fl_gfx::MouseCursorEvent(t));
        }
        SPtr<Instances::fl_gfx::MouseCursorEvent> MakeInstanceS(MouseCursorEvent& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { MemberInfoNum = 2 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_gfx
{
    class MouseCursorEvent : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::MouseCursorEvent"; }
#endif
    public:
        typedef Classes::fl_gfx::MouseCursorEvent ClassType;
        typedef InstanceTraits::fl_gfx::MouseCursorEvent InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        MouseCursorEvent(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 1 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_gfx
{
    class MouseCursorEvent : public Class
    {
        friend class ClassTraits::fl_gfx::MouseCursorEvent;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_gfx::MouseCursorEventTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::MouseCursorEvent"; }
#endif
    public:
        typedef MouseCursorEvent SelfType;
        typedef MouseCursorEvent ClassType;
        
    private:
        MouseCursorEvent(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* CURSOR_CHANGE;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

