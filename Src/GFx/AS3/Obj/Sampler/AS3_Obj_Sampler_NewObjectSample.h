//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Sampler_NewObjectSample.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Sampler_NewObjectSample_H
#define INC_AS3_Obj_Sampler_NewObjectSample_H

#include "AS3_Obj_Sampler_Sample.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_sampler
{
    extern const TypeInfo NewObjectSampleTI;
    extern const ClassInfo NewObjectSampleCI;
} // namespace fl_sampler

namespace ClassTraits { namespace fl_sampler
{
    class NewObjectSample;
}}

namespace InstanceTraits { namespace fl_sampler
{
    class NewObjectSample;
}}

namespace Classes { namespace fl_sampler
{
    class NewObjectSample;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_sampler
{
    class NewObjectSample : public Instances::fl_sampler::Sample
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_sampler::NewObjectSample;
        
    public:
        typedef NewObjectSample SelfType;
        typedef Classes::fl_sampler::NewObjectSample ClassType;
        typedef InstanceTraits::fl_sampler::NewObjectSample TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_sampler::NewObjectSampleTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_sampler::NewObjectSample"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        NewObjectSample(InstanceTraits::Traits& t);

//##protect##"instance$methods"
//##protect##"instance$methods"

    public:

    public:
        // AS3 API members.
        const Value::Number id;
        SPtr<Class> type;

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_sampler
{
    class NewObjectSample : public fl_sampler::Sample
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::NewObjectSample"; }
#endif
    public:
        typedef Instances::fl_sampler::NewObjectSample InstanceType;

    public:
        NewObjectSample(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_sampler::NewObjectSample> MakeInstance(NewObjectSample& t)
        {
            return Pickable<Instances::fl_sampler::NewObjectSample>(new(t.Alloc()) Instances::fl_sampler::NewObjectSample(t));
        }
        SPtr<Instances::fl_sampler::NewObjectSample> MakeInstanceS(NewObjectSample& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { MemberInfoNum = 2 };
        static const MemberInfo mi[MemberInfoNum];
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
    
namespace ClassTraits { namespace fl_sampler
{
    class NewObjectSample : public fl_sampler::Sample
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::NewObjectSample"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_sampler::NewObjectSample InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        NewObjectSample(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

