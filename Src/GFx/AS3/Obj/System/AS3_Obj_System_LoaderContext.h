//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_System_LoaderContext.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_System_LoaderContext_H
#define INC_AS3_Obj_System_LoaderContext_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
#include "AS3_Obj_System_SecurityDomain.h"
#include "AS3_Obj_System_ApplicationDomain.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_system
{
    extern const TypeInfo LoaderContextTI;
    extern const ClassInfo LoaderContextCI;
} // namespace fl_system

namespace ClassTraits { namespace fl_system
{
    class LoaderContext;
}}

namespace InstanceTraits { namespace fl_system
{
    class LoaderContext;
}}

namespace Classes { namespace fl_system
{
    class LoaderContext;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_system
{
    class LoaderContext : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_system::LoaderContext;
        
    public:
        typedef LoaderContext SelfType;
        typedef Classes::fl_system::LoaderContext ClassType;
        typedef InstanceTraits::fl_system::LoaderContext TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_system::LoaderContextTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_system::LoaderContext"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        LoaderContext(InstanceTraits::Traits& t);

//##protect##"instance$methods"
    public:
        virtual void AS3Constructor(unsigned argc, const Value* argv);
//##protect##"instance$methods"

    public:
        // AS3 API members.
        bool allowLoadBytesCodeExecution;
        SPtr<Instances::fl_system::ApplicationDomain> applicationDomain;
        bool checkPolicyFile;
        SPtr<Instances::fl_system::SecurityDomain> securityDomain;

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_system
{
    class LoaderContext : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::LoaderContext"; }
#endif
    public:
        typedef Instances::fl_system::LoaderContext InstanceType;

    public:
        LoaderContext(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_system::LoaderContext> MakeInstance(LoaderContext& t)
        {
            return Pickable<Instances::fl_system::LoaderContext>(new(t.Alloc()) Instances::fl_system::LoaderContext(t));
        }
        SPtr<Instances::fl_system::LoaderContext> MakeInstanceS(LoaderContext& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { MemberInfoNum = 4 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_system
{
    class LoaderContext : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::LoaderContext"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_system::LoaderContext InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        LoaderContext(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

