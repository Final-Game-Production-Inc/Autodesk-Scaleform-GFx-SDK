//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Catch.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Catch_H
#define INC_AS3_Obj_Catch_H

#include "AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl
{
    extern const TypeInfo CatchTI;
    extern const ClassInfo CatchCI;
} // namespace fl

namespace ClassTraits { namespace fl
{
    class Catch;
}}

namespace InstanceTraits { namespace fl
{
    class Catch;
}}

namespace Classes { namespace fl
{
    class Catch;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl
{
    class Catch : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl::Catch;
        
    public:
        typedef Catch SelfType;
        typedef Classes::fl::Catch ClassType;
        typedef InstanceTraits::fl::Catch TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl::CatchTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl::Catch"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        Catch(InstanceTraits::Traits& t);

//##protect##"instance$methods"
//##protect##"instance$methods"

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl
{
    class Catch : public CTraits
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::Catch"; }
#endif
    public:
        typedef Instances::fl::Catch InstanceType;

    public:
        Catch(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl::Catch> MakeInstance(Catch& t)
        {
            return Pickable<Instances::fl::Catch>(new(t.Alloc()) Instances::fl::Catch(t));
        }
        SPtr<Instances::fl::Catch> MakeInstanceS(Catch& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

//##protect##"instance_traits$methods"
        Catch(VMAbcFile& file, VM& vm, const Abc::MethodBodyInfo::ExceptionInfo& e);
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl
{
    class Catch : public Traits
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Catch"; }
#endif
    public:
        typedef Classes::fl::Catch ClassType;
        typedef InstanceTraits::fl::Catch InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Catch(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl
{
    class Catch : public Class
    {
        friend class ClassTraits::fl::Catch;
        static const TypeInfo& GetTypeInfo() { return AS3::fl::CatchTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::Catch"; }
#endif
    public:
        typedef Catch SelfType;
        typedef Catch ClassType;
        
    private:
        Catch(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
    public:
        SPtr<InstanceTraits::fl::Catch> MakeInstanceTraits(VMAbcFile& file, const Abc::MethodBodyInfo::ExceptionInfo& e);
        Pickable<Instances::fl::Catch> MakeInstance(VMAbcFile& file, const Abc::MethodBodyInfo::ExceptionInfo& e)
        {
            SPtr<InstanceTraits::fl::Catch> tr = MakeInstanceTraits(file, e);
            return tr->MakeInstance(*tr);
        }
//##protect##"class_$methods"

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

