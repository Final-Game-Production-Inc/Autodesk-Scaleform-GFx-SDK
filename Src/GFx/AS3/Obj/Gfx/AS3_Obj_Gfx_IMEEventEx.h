//##protect##"disclaimer"
/**********************************************************************

Filename    :   AS3_Obj_Gfx_IMEEventEx.h
Content     :   
Created     :   Jan, 2011
Authors     :   Sergey Sikorskiy

Copyright   :   (c) 2011 Scaleform Corp. All Rights Reserved.

Notes       :    

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Gfx_IMEEventEx_H
#define INC_AS3_Obj_Gfx_IMEEventEx_H

#include "../Events/AS3_Obj_Events_Event.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_gfx
{
    extern const TypeInfo IMEEventExTI;
    extern const ClassInfo IMEEventExCI;
} // namespace fl_gfx

namespace ClassTraits { namespace fl_gfx
{
    class IMEEventEx;
}}

namespace InstanceTraits { namespace fl_gfx
{
    class IMEEventEx;
}}

namespace Classes { namespace fl_gfx
{
    class IMEEventEx;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_gfx
{
    class IMEEventEx : public Instances::fl_events::Event
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_gfx::IMEEventEx;
        
    public:
        typedef IMEEventEx SelfType;
        typedef Classes::fl_gfx::IMEEventEx ClassType;
        typedef InstanceTraits::fl_gfx::IMEEventEx TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_gfx::IMEEventExTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_gfx::IMEEventEx"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        IMEEventEx(InstanceTraits::Traits& t);

//##protect##"instance$methods"
	public: 
		void SetMessage(const char* messageName) { message = messageName;}
//##protect##"instance$methods"

    public:
        // AS3 API members.
        ASString message;

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_gfx
{
    class IMEEventEx : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::IMEEventEx"; }
#endif
    public:
        typedef Instances::fl_gfx::IMEEventEx InstanceType;

    public:
        IMEEventEx(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_gfx::IMEEventEx> MakeInstance(IMEEventEx& t)
        {
            return Pickable<Instances::fl_gfx::IMEEventEx>(new(t.Alloc()) Instances::fl_gfx::IMEEventEx(t));
        }
        SPtr<Instances::fl_gfx::IMEEventEx> MakeInstanceS(IMEEventEx& t)
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
    class IMEEventEx : public fl_events::Event
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::IMEEventEx"; }
#endif
    public:
        typedef Classes::fl_gfx::IMEEventEx ClassType;
        typedef InstanceTraits::fl_gfx::IMEEventEx InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        IMEEventEx(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 8 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_gfx
{
    class IMEEventEx : public Class
    {
        friend class ClassTraits::fl_gfx::IMEEventEx;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_gfx::IMEEventExTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::IMEEventEx"; }
#endif
    public:
        typedef IMEEventEx SelfType;
        typedef IMEEventEx ClassType;
        
    private:
        IMEEventEx(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* SET_CURRENT_INPUT_LANGUAGE;
        const char* SET_SUPPORTED_LANGUAGES;
        const char* SET_SUPPORTED_IMENAMES;
        const char* SET_IMENAME;
        const char* SET_CURRENT_LANGUAGE;
        const char* REMOVE_STATUS_WINDOW;
        const char* DISPLAY_STATUS_WINDOW;
        const char* SET_CONVERSION_STATUS;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

