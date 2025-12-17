//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Gfx_TextEventEx.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Gfx_TextEventEx_H
#define INC_AS3_Obj_Gfx_TextEventEx_H

#include "../Events/AS3_Obj_Events_TextEvent.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_gfx
{
    extern const TypeInfo TextEventExTI;
    extern const ClassInfo TextEventExCI;
} // namespace fl_gfx

namespace ClassTraits { namespace fl_gfx
{
    class TextEventEx;
}}

namespace InstanceTraits { namespace fl_gfx
{
    class TextEventEx;
}}

namespace Classes { namespace fl_gfx
{
    class TextEventEx;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_gfx
{
    class TextEventEx : public Instances::fl_events::TextEvent
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_gfx::TextEventEx;
        
    public:
        typedef TextEventEx SelfType;
        typedef Classes::fl_gfx::TextEventEx ClassType;
        typedef InstanceTraits::fl_gfx::TextEventEx TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_gfx::TextEventExTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_gfx::TextEventEx"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        TextEventEx(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        virtual AS3::Object* GetEventClass() const;
        virtual SPtr<Instances::fl_events::Event> Clone() const;
//##protect##"instance$methods"

    public:
        // AS3 API members.
        UInt32 controllerIdx;
        UInt32 buttonIdx;

//##protect##"instance$data"
    public:
        void    SetControllerIdx(unsigned idx)  { controllerIdx = idx; }
        void    SetButtonIdx(unsigned idx) { buttonIdx = idx; }
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_gfx
{
    class TextEventEx : public fl_events::TextEvent
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::TextEventEx"; }
#endif
    public:
        typedef Instances::fl_gfx::TextEventEx InstanceType;

    public:
        TextEventEx(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_gfx::TextEventEx> MakeInstance(TextEventEx& t)
        {
            return Pickable<Instances::fl_gfx::TextEventEx>(new(t.Alloc()) Instances::fl_gfx::TextEventEx(t));
        }
        SPtr<Instances::fl_gfx::TextEventEx> MakeInstanceS(TextEventEx& t)
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
    class TextEventEx : public fl_events::TextEvent
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::TextEventEx"; }
#endif
    public:
        typedef Classes::fl_gfx::TextEventEx ClassType;
        typedef InstanceTraits::fl_gfx::TextEventEx InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        TextEventEx(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 2 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_gfx
{
    class TextEventEx : public Class
    {
        friend class ClassTraits::fl_gfx::TextEventEx;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_gfx::TextEventExTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::TextEventEx"; }
#endif
    public:
        typedef TextEventEx SelfType;
        typedef TextEventEx ClassType;
        
    private:
        TextEventEx(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* LINK_MOUSE_OVER;
        const char* LINK_MOUSE_OUT;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

