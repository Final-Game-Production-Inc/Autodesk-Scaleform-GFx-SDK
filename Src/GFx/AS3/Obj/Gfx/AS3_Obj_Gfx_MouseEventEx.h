//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Gfx_MouseEventEx.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Gfx_MouseEventEx_H
#define INC_AS3_Obj_Gfx_MouseEventEx_H

#include "../Events/AS3_Obj_Events_MouseEvent.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_gfx
{
    extern const TypeInfo MouseEventExTI;
    extern const ClassInfo MouseEventExCI;
} // namespace fl_gfx

namespace ClassTraits { namespace fl_gfx
{
    class MouseEventEx;
}}

namespace InstanceTraits { namespace fl_gfx
{
    class MouseEventEx;
}}

namespace Classes { namespace fl_gfx
{
    class MouseEventEx;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_gfx
{
    class MouseEventEx : public Instances::fl_events::MouseEvent
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_gfx::MouseEventEx;
        
    public:
        typedef MouseEventEx SelfType;
        typedef Classes::fl_gfx::MouseEventEx ClassType;
        typedef InstanceTraits::fl_gfx::MouseEventEx TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_gfx::MouseEventExTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_gfx::MouseEventEx"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        MouseEventEx(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        virtual AS3::Object* GetEventClass() const;
        virtual SPtr<Instances::fl_events::Event> Clone() const;
//##protect##"instance$methods"

    public:
        // AS3 API members.
        UInt32 mouseIdx;
        UInt32 buttonIdx;
        UInt32 nestingIdx;

//##protect##"instance$data"
    public:
        void    SetMouseIdx(unsigned idx) { mouseIdx = idx; }
        void    SetButtonIdx(unsigned idx) { buttonIdx = idx; }
        void    SetNestingIdx(unsigned idx) { nestingIdx = idx; }
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_gfx
{
    class MouseEventEx : public fl_events::MouseEvent
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::MouseEventEx"; }
#endif
    public:
        typedef Instances::fl_gfx::MouseEventEx InstanceType;

    public:
        MouseEventEx(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_gfx::MouseEventEx> MakeInstance(MouseEventEx& t)
        {
            return Pickable<Instances::fl_gfx::MouseEventEx>(new(t.Alloc()) Instances::fl_gfx::MouseEventEx(t));
        }
        SPtr<Instances::fl_gfx::MouseEventEx> MakeInstanceS(MouseEventEx& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { MemberInfoNum = 3 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_gfx
{
    class MouseEventEx : public fl_events::MouseEvent
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::MouseEventEx"; }
#endif
    public:
        typedef Classes::fl_gfx::MouseEventEx ClassType;
        typedef InstanceTraits::fl_gfx::MouseEventEx InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        MouseEventEx(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 3 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_gfx
{
    class MouseEventEx : public Class
    {
        friend class ClassTraits::fl_gfx::MouseEventEx;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_gfx::MouseEventExTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::MouseEventEx"; }
#endif
    public:
        typedef MouseEventEx SelfType;
        typedef MouseEventEx ClassType;
        
    private:
        MouseEventEx(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const UInt32 LEFT_BUTTON;
        const UInt32 RIGHT_BUTTON;
        const UInt32 MIDDLE_BUTTON;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

