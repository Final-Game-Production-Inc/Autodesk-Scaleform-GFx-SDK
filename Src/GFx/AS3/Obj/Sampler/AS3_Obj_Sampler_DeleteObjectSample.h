//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Sampler_DeleteObjectSample.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Sampler_DeleteObjectSample_H
#define INC_AS3_Obj_Sampler_DeleteObjectSample_H

#include "AS3_Obj_Sampler_Sample.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_sampler
{
    extern const TypeInfo DeleteObjectSampleTI;
    extern const ClassInfo DeleteObjectSampleCI;
} // namespace fl_sampler

namespace ClassTraits { namespace fl_sampler
{
    class DeleteObjectSample;
}}

namespace InstanceTraits { namespace fl_sampler
{
    class DeleteObjectSample;
}}

namespace Classes { namespace fl_sampler
{
    class DeleteObjectSample;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_sampler
{
    class DeleteObjectSample : public Instances::fl_sampler::Sample
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_sampler::DeleteObjectSample;
        
    public:
        typedef DeleteObjectSample SelfType;
        typedef Classes::fl_sampler::DeleteObjectSample ClassType;
        typedef InstanceTraits::fl_sampler::DeleteObjectSample TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_sampler::DeleteObjectSampleTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_sampler::DeleteObjectSample"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        DeleteObjectSample(InstanceTraits::Traits& t);

//##protect##"instance$methods"
//##protect##"instance$methods"

    public:
        // AS3 API members.
        const Value::Number id;
        const Value::Number size;

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_sampler
{
    class DeleteObjectSample : public fl_sampler::Sample
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::DeleteObjectSample"; }
#endif
    public:
        typedef Instances::fl_sampler::DeleteObjectSample InstanceType;

    public:
        DeleteObjectSample(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_sampler::DeleteObjectSample> MakeInstance(DeleteObjectSample& t)
        {
            return Pickable<Instances::fl_sampler::DeleteObjectSample>(new(t.Alloc()) Instances::fl_sampler::DeleteObjectSample(t));
        }
        SPtr<Instances::fl_sampler::DeleteObjectSample> MakeInstanceS(DeleteObjectSample& t)
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
    class DeleteObjectSample : public fl_sampler::Sample
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::DeleteObjectSample"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_sampler::DeleteObjectSample InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        DeleteObjectSample(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

