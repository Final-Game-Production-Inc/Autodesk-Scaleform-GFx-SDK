//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Gfx_KeyboardEventEx.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Gfx_KeyboardEventEx_H
#define INC_AS3_Obj_Gfx_KeyboardEventEx_H

#include "../Events/AS3_Obj_Events_KeyboardEvent.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_gfx
{
    extern const TypeInfo KeyboardEventExTI;
    extern const ClassInfo KeyboardEventExCI;
} // namespace fl_gfx

namespace ClassTraits { namespace fl_gfx
{
    class KeyboardEventEx;
}}

namespace InstanceTraits { namespace fl_gfx
{
    class KeyboardEventEx;
}}

namespace Classes { namespace fl_gfx
{
    class KeyboardEventEx;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_gfx
{
    class KeyboardEventEx : public Instances::fl_events::KeyboardEvent
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_gfx::KeyboardEventEx;
        
    public:
        typedef KeyboardEventEx SelfType;
        typedef Classes::fl_gfx::KeyboardEventEx ClassType;
        typedef InstanceTraits::fl_gfx::KeyboardEventEx TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_gfx::KeyboardEventExTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_gfx::KeyboardEventEx"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        KeyboardEventEx(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        virtual AS3::Object* GetEventClass() const;
        virtual SPtr<Instances::fl_events::Event> Clone() const;
//##protect##"instance$methods"

    public:
        // AS3 API members.
        UInt32 controllerIdx;

//##protect##"instance$data"
    public:
        void    SetControllerIdx(unsigned idx) { controllerIdx = idx; }
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_gfx
{
    class KeyboardEventEx : public fl_events::KeyboardEvent
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::KeyboardEventEx"; }
#endif
    public:
        typedef Instances::fl_gfx::KeyboardEventEx InstanceType;

    public:
        KeyboardEventEx(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_gfx::KeyboardEventEx> MakeInstance(KeyboardEventEx& t)
        {
            return Pickable<Instances::fl_gfx::KeyboardEventEx>(new(t.Alloc()) Instances::fl_gfx::KeyboardEventEx(t));
        }
        SPtr<Instances::fl_gfx::KeyboardEventEx> MakeInstanceS(KeyboardEventEx& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { MemberInfoNum = 1 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_gfx
{
    class KeyboardEventEx : public fl_events::KeyboardEvent
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::KeyboardEventEx"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_gfx::KeyboardEventEx InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        KeyboardEventEx(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

