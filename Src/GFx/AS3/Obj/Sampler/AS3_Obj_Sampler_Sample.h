//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Sampler_Sample.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Sampler_Sample_H
#define INC_AS3_Obj_Sampler_Sample_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
#include "../AS3_Obj_Array.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_sampler
{
    extern const TypeInfo SampleTI;
    extern const ClassInfo SampleCI;
} // namespace fl_sampler

namespace ClassTraits { namespace fl_sampler
{
    class Sample;
}}

namespace InstanceTraits { namespace fl_sampler
{
    class Sample;
}}

namespace Classes { namespace fl_sampler
{
    class Sample;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_sampler
{
    class Sample : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_sampler::Sample;
        
    public:
        typedef Sample SelfType;
        typedef Classes::fl_sampler::Sample ClassType;
        typedef InstanceTraits::fl_sampler::Sample TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_sampler::SampleTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_sampler::Sample"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        Sample(InstanceTraits::Traits& t);

//##protect##"instance$methods"
//##protect##"instance$methods"

    public:
        // AS3 API members.
        SPtr<Instances::fl::Array> stack;
        const Value::Number time;

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_sampler
{
    class Sample : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::Sample"; }
#endif
    public:
        typedef Instances::fl_sampler::Sample InstanceType;

    public:
        Sample(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_sampler::Sample> MakeInstance(Sample& t)
        {
            return Pickable<Instances::fl_sampler::Sample>(new(t.Alloc()) Instances::fl_sampler::Sample(t));
        }
        SPtr<Instances::fl_sampler::Sample> MakeInstanceS(Sample& t)
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
    
namespace ClassTraits { namespace fl_sampler
{
    class Sample : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Sample"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_sampler::Sample InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Sample(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

