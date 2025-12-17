//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_System_ApplicationDomain.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_System_ApplicationDomain_H
#define INC_AS3_Obj_System_ApplicationDomain_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
#include "../../AS3_VM.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_system
{
    extern const TypeInfo ApplicationDomainTI;
    extern const ClassInfo ApplicationDomainCI;
} // namespace fl_system
namespace fl
{
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
} // namespace fl

namespace ClassTraits { namespace fl_system
{
    class ApplicationDomain;
}}

namespace InstanceTraits { namespace fl_system
{
    class ApplicationDomain;
}}

namespace Classes { namespace fl_system
{
    class ApplicationDomain;
}}

//##protect##"forward_declaration"
namespace Instances
{
    class Namespace;
}
//##protect##"forward_declaration"

namespace Instances { namespace fl_system
{
    class ApplicationDomain : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_system::ApplicationDomain;
        
    public:
        typedef ApplicationDomain SelfType;
        typedef Classes::fl_system::ApplicationDomain ClassType;
        typedef InstanceTraits::fl_system::ApplicationDomain TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_system::ApplicationDomainTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_system::ApplicationDomain"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        ApplicationDomain(InstanceTraits::Traits& t);

//##protect##"instance$methods"
    public:
        virtual void AS3Constructor(unsigned argc, const Value* argv);
        VMAppDomain& GetAppDomain() const
        {
            SF_ASSERT(VMDomain);
            return *VMDomain;
        }
        void SetAppDomain(VMAppDomain& appDomain);
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_parentDomainGet, 
            mid_getDefinition, 
            mid_hasDefinition, 
        };
        void parentDomainGet(SPtr<Instances::fl_system::ApplicationDomain>& result);
        void getDefinition(Value& result, const ASString& name);
        void hasDefinition(bool& result, const ASString& name);

        // C++ friendly wrappers for AS3 methods.
        SPtr<Instances::fl_system::ApplicationDomain> parentDomainGet()
        {
            SPtr<Instances::fl_system::ApplicationDomain> result;
            parentDomainGet(result);
            return result;
        }
        Value getDefinition(const ASString& name)
        {
            Value result;
            getDefinition(result, name);
            return result;
        }
        bool hasDefinition(const ASString& name)
        {
            bool result;
            hasDefinition(result, name);
            return result;
        }

//##protect##"instance$data"
        SPtr<VMAppDomain> VMDomain;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_system
{
    class ApplicationDomain : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::ApplicationDomain"; }
#endif
    public:
        typedef Instances::fl_system::ApplicationDomain InstanceType;

    public:
        ApplicationDomain(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_system::ApplicationDomain> MakeInstance(ApplicationDomain& t)
        {
            return Pickable<Instances::fl_system::ApplicationDomain>(new(t.Alloc()) Instances::fl_system::ApplicationDomain(t));
        }
        SPtr<Instances::fl_system::ApplicationDomain> MakeInstanceS(ApplicationDomain& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 3 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[5];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_system
{
    class ApplicationDomain : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::ApplicationDomain"; }
#endif
    public:
        typedef Classes::fl_system::ApplicationDomain ClassType;
        typedef InstanceTraits::fl_system::ApplicationDomain InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        ApplicationDomain(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { ThunkInfoNum = 1 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[1];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_system
{
    class ApplicationDomain : public Class
    {
        friend class ClassTraits::fl_system::ApplicationDomain;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_system::ApplicationDomainTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::ApplicationDomain"; }
#endif
    public:
        typedef ApplicationDomain SelfType;
        typedef ApplicationDomain ClassType;
        
    private:
        ApplicationDomain(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_currentDomainGet, 
        };
        void currentDomainGet(SPtr<Instances::fl_system::ApplicationDomain>& result);

        // C++ friendly wrappers for AS3 methods.
        SPtr<Instances::fl_system::ApplicationDomain> currentDomainGet()
        {
            SPtr<Instances::fl_system::ApplicationDomain> result;
            currentDomainGet(result);
            return result;
        }

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

