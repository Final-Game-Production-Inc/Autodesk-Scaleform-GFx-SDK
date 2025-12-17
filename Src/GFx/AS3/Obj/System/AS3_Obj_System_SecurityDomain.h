//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_System_SecurityDomain.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_System_SecurityDomain_H
#define INC_AS3_Obj_System_SecurityDomain_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_system
{
    extern const TypeInfo SecurityDomainTI;
    extern const ClassInfo SecurityDomainCI;
} // namespace fl_system

namespace ClassTraits { namespace fl_system
{
    class SecurityDomain;
}}

namespace InstanceTraits { namespace fl_system
{
    class SecurityDomain;
}}

namespace Classes { namespace fl_system
{
    class SecurityDomain;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_system
{
    class SecurityDomain : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_system::SecurityDomain;
        
    public:
        typedef SecurityDomain SelfType;
        typedef Classes::fl_system::SecurityDomain ClassType;
        typedef InstanceTraits::fl_system::SecurityDomain TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_system::SecurityDomainTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_system::SecurityDomain"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        SecurityDomain(InstanceTraits::Traits& t);

//##protect##"instance$methods"
//##protect##"instance$methods"

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_system
{
    class SecurityDomain : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::SecurityDomain"; }
#endif
    public:
        typedef Instances::fl_system::SecurityDomain InstanceType;

    public:
        SecurityDomain(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_system::SecurityDomain> MakeInstance(SecurityDomain& t)
        {
            return Pickable<Instances::fl_system::SecurityDomain>(new(t.Alloc()) Instances::fl_system::SecurityDomain(t));
        }
        SPtr<Instances::fl_system::SecurityDomain> MakeInstanceS(SecurityDomain& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_system
{
    class SecurityDomain : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::SecurityDomain"; }
#endif
    public:
        typedef Classes::fl_system::SecurityDomain ClassType;
        typedef InstanceTraits::fl_system::SecurityDomain InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        SecurityDomain(VM& vm, const ClassInfo& ci);
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
    class SecurityDomain : public Class
    {
        friend class ClassTraits::fl_system::SecurityDomain;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_system::SecurityDomainTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::SecurityDomain"; }
#endif
    public:
        typedef SecurityDomain SelfType;
        typedef SecurityDomain ClassType;
        
    private:
        SecurityDomain(ClassTraits::Traits& t);
       
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
        void currentDomainGet(SPtr<Instances::fl_system::SecurityDomain>& result);

        // C++ friendly wrappers for AS3 methods.
        SPtr<Instances::fl_system::SecurityDomain> currentDomainGet()
        {
            SPtr<Instances::fl_system::SecurityDomain> result;
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

