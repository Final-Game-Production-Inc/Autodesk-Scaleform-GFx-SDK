//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Gfx_FocusEventEx.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Gfx_FocusEventEx_H
#define INC_AS3_Obj_Gfx_FocusEventEx_H

#include "../Events/AS3_Obj_Events_FocusEvent.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_gfx
{
    extern const TypeInfo FocusEventExTI;
    extern const ClassInfo FocusEventExCI;
} // namespace fl_gfx

namespace ClassTraits { namespace fl_gfx
{
    class FocusEventEx;
}}

namespace InstanceTraits { namespace fl_gfx
{
    class FocusEventEx;
}}

namespace Classes { namespace fl_gfx
{
    class FocusEventEx;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_gfx
{
    class FocusEventEx : public Instances::fl_events::FocusEvent
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_gfx::FocusEventEx;
        
    public:
        typedef FocusEventEx SelfType;
        typedef Classes::fl_gfx::FocusEventEx ClassType;
        typedef InstanceTraits::fl_gfx::FocusEventEx TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_gfx::FocusEventExTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_gfx::FocusEventEx"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        FocusEventEx(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        virtual AS3::Object* GetEventClass() const;
        virtual SPtr<Instances::fl_events::Event> Clone() const;
//##protect##"instance$methods"

    public:
        // AS3 API members.
        UInt32 controllerIdx;

//##protect##"instance$data"
    public:
        void    SetControllerIdx(unsigned idx)  { controllerIdx = idx; }
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_gfx
{
    class FocusEventEx : public fl_events::FocusEvent
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::FocusEventEx"; }
#endif
    public:
        typedef Instances::fl_gfx::FocusEventEx InstanceType;

    public:
        FocusEventEx(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_gfx::FocusEventEx> MakeInstance(FocusEventEx& t)
        {
            return Pickable<Instances::fl_gfx::FocusEventEx>(new(t.Alloc()) Instances::fl_gfx::FocusEventEx(t));
        }
        SPtr<Instances::fl_gfx::FocusEventEx> MakeInstanceS(FocusEventEx& t)
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
    class FocusEventEx : public fl_events::FocusEvent
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::FocusEventEx"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_gfx::FocusEventEx InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        FocusEventEx(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

