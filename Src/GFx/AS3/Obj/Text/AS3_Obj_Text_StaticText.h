//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Text_StaticText.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Text_StaticText_H
#define INC_AS3_Obj_Text_StaticText_H

#include "../Display/AS3_Obj_Display_DisplayObject.h"
//##protect##"includes"
#include "GFx/GFx_StaticText.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_text
{
    extern const TypeInfo StaticTextTI;
    extern const ClassInfo StaticTextCI;
} // namespace fl_text
namespace fl
{
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl

namespace ClassTraits { namespace fl_text
{
    class StaticText;
}}

namespace InstanceTraits { namespace fl_text
{
    class StaticText;
}}

namespace Classes { namespace fl_text
{
    class StaticText;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_text
{
    class StaticText : public Instances::fl_display::DisplayObject
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_text::StaticText;
        
    public:
        typedef StaticText SelfType;
        typedef Classes::fl_text::StaticText ClassType;
        typedef InstanceTraits::fl_text::StaticText TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_text::StaticTextTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_text::StaticText"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        StaticText(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        GFx::StaticTextCharacter* GetStaticTextChar() const 
        { 
            SF_ASSERT(pDispObj && pDispObj->GetType() == CharacterDef::StaticText);
            return static_cast<GFx::StaticTextCharacter*>(pDispObj.GetPtr()); 
        }
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_textGet, 
        };
        void textGet(ASString& result);

        // C++ friendly wrappers for AS3 methods.
        ASString textGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            textGet(result);
            return result;
        }

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_text
{
    class StaticText : public fl_display::DisplayObject
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::StaticText"; }
#endif
    public:
        typedef Instances::fl_text::StaticText InstanceType;

    public:
        StaticText(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_text::StaticText> MakeInstance(StaticText& t)
        {
            return Pickable<Instances::fl_text::StaticText>(new(t.Alloc()) Instances::fl_text::StaticText(t));
        }
        SPtr<Instances::fl_text::StaticText> MakeInstanceS(StaticText& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 1 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[1];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_text
{
    class StaticText : public fl_display::DisplayObject
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::StaticText"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_text::StaticText InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        StaticText(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

