//##protect##"disclaimer"
/**********************************************************************

Filename    :   .h
Content     :   
Created     :   Sep, 2013
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2013 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**********************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Domain_H
#define INC_AS3_Obj_Domain_H

#include "AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl
{
    extern const TypeInfo DomainTI;
    extern const ClassInfo DomainCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
} // namespace fl
namespace fl_utils
{
    extern const TypeInfo ByteArrayTI;
    extern const ClassInfo ByteArrayCI;
} // namespace fl_utils

namespace ClassTraits { namespace fl
{
    class Domain;
}}

namespace InstanceTraits { namespace fl
{
    class Domain;
}}

namespace Classes { namespace fl
{
    class Domain;
}}

//##protect##"forward_declaration"
namespace Instances { namespace fl_utils
{ 
    class ByteArray;
}} // namespace Instances { namespace fl_utils
//##protect##"forward_declaration"

namespace Instances { namespace fl
{
    class Domain : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl::Domain;
        
    public:
        typedef Domain SelfType;
        typedef Classes::fl::Domain ClassType;
        typedef InstanceTraits::fl::Domain TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl::DomainTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl::Domain"; }
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
        virtual void ForEachChild_GC(Collector* prcc, GcOp op) const;

        const String& GetPath() const
        {
            return Path;
        }
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_loadBytes, 
            mid_parentDomainGet, 
            mid_getClass, 
            mid_load, 
        };
        void loadBytes(const Value& result, Instances::fl_utils::ByteArray* byteArray);
        void parentDomainGet(SPtr<Instances::fl::Domain>& result);
        void getClass(Value& result, const ASString& name);
        void load(bool& result, const ASString& fileName);

        // C++ friendly wrappers for AS3 methods.
        void loadBytes(Instances::fl_utils::ByteArray* byteArray)
        {
            loadBytes(Value::GetUndefined(), byteArray);
        }
        SPtr<Instances::fl::Domain> parentDomainGet()
        {
            SPtr<Instances::fl::Domain> result;
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
        SPtr<VMAppDomain>           VMDomain;
        String                      Path;
        // ABC code should be kept in memory.
        ArrayLH<UInt8, Mem_Stat>    FileData;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl
{
    class Domain : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::Domain"; }
#endif
    public:
        typedef Instances::fl::Domain InstanceType;

    public:
        Domain(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl::Domain> MakeInstance(Domain& t)
        {
            return Pickable<Instances::fl::Domain>(new(t.Alloc()) Instances::fl::Domain(t));
        }
        SPtr<Instances::fl::Domain> MakeInstanceS(Domain& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 4 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[7];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl
{
    class Domain : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Domain"; }
#endif
    public:
        typedef Classes::fl::Domain ClassType;
        typedef InstanceTraits::fl::Domain InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Domain(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { ThunkInfoNum = 1 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[1];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl
{
    class Domain : public Class
    {
        friend class ClassTraits::fl::Domain;
        static const TypeInfo& GetTypeInfo() { return AS3::fl::DomainTI; }
        
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
        void currentDomainGet(SPtr<Instances::fl::Domain>& result);

        // C++ friendly wrappers for AS3 methods.
        SPtr<Instances::fl::Domain> currentDomainGet()
        {
            SPtr<Instances::fl::Domain> result;
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

