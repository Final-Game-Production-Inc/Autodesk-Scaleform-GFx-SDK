//##protect##"disclaimer"
/**********************************************************************

Filename    :   .h
Content     :   
Created     :   Jul, 2012
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**********************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_System_Domain_H
#define INC_AS3_Obj_System_Domain_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
#include "../../AS3_VM.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_system
{
    extern const TypeInfo DomainTI;
    extern const ClassInfo DomainCI;
} // namespace fl_system
namespace fl
{
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
} // namespace fl

namespace ClassTraits { namespace fl_system
{
    class Domain;
}}

namespace InstanceTraits { namespace fl_system
{
    class Domain;
}}

namespace Classes { namespace fl_system
{
    class Domain;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_system
{
    class Domain : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_system::Domain;
        
    public:
        typedef Domain SelfType;
        typedef Classes::fl_system::Domain ClassType;
        typedef InstanceTraits::fl_system::Domain TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_system::DomainTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_system::Domain"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        Domain(InstanceTraits::Traits& t);

//##protect##"instance$methods"
    public:
        virtual void AS3Constructor(unsigned argc, const Value* argv);
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_parentDomainGet, 
            mid_getClass, 
            mid_load, 
        };
        void parentDomainGet(SPtr<Instances::fl_system::Domain>& result);
        void getClass(Value& result, const ASString& name);
        void load(bool& result, const ASString& fileName);

        // C++ friendly wrappers for AS3 methods.
        SPtr<Instances::fl_system::Domain> parentDomainGet()
        {
            SPtr<Instances::fl_system::Domain> result;
            parentDomainGet(result);
            return result;
        }
        Value getClass(const ASString& name)
        {
            Value result;
            getClass(result, name);
            return result;
        }
        bool load(const ASString& fileName)
        {
            bool result;
            load(result, fileName);
            return result;
        }

//##protect##"instance$data"
        VMAppDomain* VMDomain;
        ArrayLH<SPtr<VMAbcFile> > Files; 
        ArrayLH<UInt8, Mem_Stat> FileData;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_system
{
    class Domain : public CTraits
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::Domain"; }
#endif
    public:
        typedef Instances::fl_system::Domain InstanceType;

    public:
        Domain(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_system::Domain> MakeInstance(Domain& t)
        {
            return Pickable<Instances::fl_system::Domain>(new(t.Alloc()) Instances::fl_system::Domain(t));
        }
        SPtr<Instances::fl_system::Domain> MakeInstanceS(Domain& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 3 };
        static const ThunkInfo ti[ThunkInfoNum];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_system
{
    class Domain : public Traits
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Domain"; }
#endif
    public:
        typedef Classes::fl_system::Domain ClassType;

    public:
        Domain(VM& vm);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { ThunkInfoNum = 1 };
        static const ThunkInfo ti[ThunkInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_system
{
    class Domain : public Class
    {
        friend class ClassTraits::fl_system::Domain;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_system::DomainTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::Domain"; }
#endif
    public:
        typedef Domain SelfType;
        typedef Domain ClassType;
        
    private:
        Domain(ClassTraits::Traits& t);
       
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
        void currentDomainGet(SPtr<Instances::fl_system::Domain>& result);

        // C++ friendly wrappers for AS3 methods.
        SPtr<Instances::fl_system::Domain> currentDomainGet()
        {
            SPtr<Instances::fl_system::Domain> result;
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

