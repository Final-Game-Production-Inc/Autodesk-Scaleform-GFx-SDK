//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Sampler_StackFrame.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Sampler_StackFrame_H
#define INC_AS3_Obj_Sampler_StackFrame_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_sampler
{
    extern const TypeInfo StackFrameTI;
    extern const ClassInfo StackFrameCI;
} // namespace fl_sampler
namespace fl
{
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl

namespace ClassTraits { namespace fl_sampler
{
    class StackFrame;
}}

namespace InstanceTraits { namespace fl_sampler
{
    class StackFrame;
}}

namespace Classes { namespace fl_sampler
{
    class StackFrame;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_sampler
{
    class StackFrame : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_sampler::StackFrame;
        
    public:
        typedef StackFrame SelfType;
        typedef Classes::fl_sampler::StackFrame ClassType;
        typedef InstanceTraits::fl_sampler::StackFrame TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_sampler::StackFrameTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_sampler::StackFrame"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        StackFrame(InstanceTraits::Traits& t);

//##protect##"instance$methods"
//##protect##"instance$methods"

    public:

    public:
        // AS3 API members.
        const char* file;
        const UInt32 line;
        const char* name;

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_sampler
{
    class StackFrame : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::StackFrame"; }
#endif
    public:
        typedef Instances::fl_sampler::StackFrame InstanceType;

    public:
        StackFrame(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_sampler::StackFrame> MakeInstance(StackFrame& t)
        {
            return Pickable<Instances::fl_sampler::StackFrame>(new(t.Alloc()) Instances::fl_sampler::StackFrame(t));
        }
        SPtr<Instances::fl_sampler::StackFrame> MakeInstanceS(StackFrame& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { MemberInfoNum = 3 };
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
    class StackFrame : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::StackFrame"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_sampler::StackFrame InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        StackFrame(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

