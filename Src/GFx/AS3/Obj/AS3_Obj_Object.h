//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Object.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Object_H
#define INC_AS3_Obj_Object_H

#include "../AS3_Instance.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl
{
    extern const TypeInfo ObjectTI;
    extern const ClassInfo ObjectCI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl

namespace ClassTraits { namespace fl
{
    class Object;
}}

namespace InstanceTraits { namespace fl
{
    class Object;
}}

namespace Classes { namespace fl
{
    class Object;
}}

//##protect##"forward_declaration"
namespace InstanceTraits
{
    class Interface;
}
//##protect##"forward_declaration"

namespace Instances { namespace fl
{
    class Object : public Instance
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl::Object;
        
    public:
        typedef Object SelfType;
        typedef Classes::fl::Object ClassType;
        typedef InstanceTraits::fl::Object TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl::ObjectTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl::Object"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        Object(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        friend class Instances::fl::Namespace;
        friend class InstanceTraits::Traits;
        friend class InstanceTraits::Interface;

    private:
        Object(VM& vm);

    public:
        //static void toString(Object& _this, Value& result, unsigned argc, const Value* argv);
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_AS3isPrototypeOf, 
            mid_AS3hasOwnProperty, 
            mid_AS3propertyIsEnumerable, 
            mid_hasOwnPropertyProto, 
            mid_propertyIsEnumerableProto, 
            mid_setPropertyIsEnumerableProto, 
            mid_isPrototypeOfProto, 
            mid_toStringProto, 
            mid_toLocaleStringProto, 
            mid_valueOfProto, 
        };
        static void AS3isPrototypeOf(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);
        static void AS3hasOwnProperty(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);
        static void AS3propertyIsEnumerable(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);

        // C++ friendly wrappers for AS3 methods.
        static void hasOwnPropertyProto(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);
        static void propertyIsEnumerableProto(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);
        static void setPropertyIsEnumerableProto(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);
        static void isPrototypeOfProto(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);
        static void toStringProto(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);
        static void toLocaleStringProto(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);
        static void valueOfProto(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl
{
    class Object : public CTraits
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::Object"; }
#endif
    public:
        typedef Instances::fl::Object InstanceType;

    public:
        Object(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl::Object> MakeInstance(Object& t)
        {
            return Pickable<Instances::fl::Object>(new(t.Alloc()) Instances::fl::Object(t));
        }
        SPtr<Instances::fl::Object> MakeInstanceS(Object& t)
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
    
namespace ClassTraits { namespace fl
{
    class Object : public Traits
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Object"; }
#endif
    public:
        typedef Classes::fl::Object ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Object(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl
{
    class Object : public Class
    {
        friend class ClassTraits::fl::Object;
        static const TypeInfo& GetTypeInfo() { return AS3::fl::ObjectTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::Object"; }
#endif
    public:
        typedef Object SelfType;
        typedef Object ClassType;
        
    private:
        Object(ClassTraits::Traits& t);
    public:
        virtual void InitPrototype(AS3::Object& obj) const;
        enum { ThunkInfoNum = 7 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[11];
        static const Abc::ConstValue dva[1];
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
        virtual void Call(const Value& _this, Value& result, unsigned argc, const Value* const argv);
        // Implementation of Construct for Object differs from the default one.
        virtual void Construct(Value& result, unsigned argc, const Value* argv, bool extCall = false);
        // Set up values of slots ...
        virtual void SetupSlotValues(Object& for_obj) const;

        void toStringProto(ASString& result);
//##protect##"class_$methods"

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
namespace InstanceTraits
{
    class Interface : public CTraits
    {
    public:
        typedef Instances::fl::Object InstanceType;

    public:
        Interface(VM& vm, const ClassInfo& ci);

    public: 
        Pickable<Instances::fl::Object> MakeInstance(Traits& t);

        virtual void MakeObject(Value& result, Traits& t);
    };
}
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

