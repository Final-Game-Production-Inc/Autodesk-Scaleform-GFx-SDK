//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Text_TextLineMetrics.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Text_TextLineMetrics_H
#define INC_AS3_Obj_Text_TextLineMetrics_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_text
{
    extern const TypeInfo TextLineMetricsTI;
    extern const ClassInfo TextLineMetricsCI;
} // namespace fl_text

namespace ClassTraits { namespace fl_text
{
    class TextLineMetrics;
}}

namespace InstanceTraits { namespace fl_text
{
    class TextLineMetrics;
}}

namespace Classes { namespace fl_text
{
    class TextLineMetrics;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_text
{
    class TextLineMetrics : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_text::TextLineMetrics;
        
    public:
        typedef TextLineMetrics SelfType;
        typedef Classes::fl_text::TextLineMetrics ClassType;
        typedef InstanceTraits::fl_text::TextLineMetrics TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_text::TextLineMetricsTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_text::TextLineMetrics"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        TextLineMetrics(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        virtual void AS3Constructor(unsigned argc, const Value* argv);
//##protect##"instance$methods"

    public:
        // AS3 API members.
        Value::Number ascent;
        Value::Number descent;
        Value::Number height;
        Value::Number leading;
        Value::Number width;
        Value::Number x;

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_text
{
    class TextLineMetrics : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::TextLineMetrics"; }
#endif
    public:
        typedef Instances::fl_text::TextLineMetrics InstanceType;

    public:
        TextLineMetrics(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_text::TextLineMetrics> MakeInstance(TextLineMetrics& t)
        {
            return Pickable<Instances::fl_text::TextLineMetrics>(new(t.Alloc()) Instances::fl_text::TextLineMetrics(t));
        }
        SPtr<Instances::fl_text::TextLineMetrics> MakeInstanceS(TextLineMetrics& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { MemberInfoNum = 6 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_text
{
    class TextLineMetrics : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::TextLineMetrics"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_text::TextLineMetrics InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        TextLineMetrics(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

