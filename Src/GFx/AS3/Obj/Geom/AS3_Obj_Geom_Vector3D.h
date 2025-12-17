//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Geom_Vector3D.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Geom_Vector3D_H
#define INC_AS3_Obj_Geom_Vector3D_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_geom
{
    extern const TypeInfo Vector3DTI;
    extern const ClassInfo Vector3DCI;
} // namespace fl_geom
namespace fl
{
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl

namespace ClassTraits { namespace fl_geom
{
    class Vector3D;
}}

namespace InstanceTraits { namespace fl_geom
{
    class Vector3D;
}}

namespace Classes { namespace fl_geom
{
    class Vector3D;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_geom
{
    class Vector3D : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_geom::Vector3D;
        
    public:
        typedef Vector3D SelfType;
        typedef Classes::fl_geom::Vector3D ClassType;
        typedef InstanceTraits::fl_geom::Vector3D TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_geom::Vector3DTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_geom::Vector3D"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        Vector3D(InstanceTraits::Traits& t);

//##protect##"instance$methods"
		virtual void AS3Constructor(unsigned argc, const Value* argv);
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_lengthGet, 
            mid_lengthSquaredGet, 
            mid_xSet, 
            mid_xGet, 
            mid_ySet, 
            mid_yGet, 
            mid_zSet, 
            mid_zGet, 
            mid_wSet, 
            mid_wGet, 
            mid_add, 
            mid_clone, 
            mid_crossProduct, 
            mid_decrementBy, 
            mid_dotProduct, 
            mid_equals, 
            mid_incrementBy, 
            mid_nearEquals, 
            mid_negate, 
            mid_normalize, 
            mid_project, 
            mid_scaleBy, 
            mid_subtract, 
            mid_toString, 
        };
        void lengthGet(Value::Number& result);
        void lengthSquaredGet(Value::Number& result);
        void xSet(const Value& result, Value::Number value);
        void xGet(Value::Number& result);
        void ySet(const Value& result, Value::Number value);
        void yGet(Value::Number& result);
        void zSet(const Value& result, Value::Number value);
        void zGet(Value::Number& result);
        void wSet(const Value& result, Value::Number value);
        void wGet(Value::Number& result);
        void add(SPtr<Instances::fl_geom::Vector3D>& result, Instances::fl_geom::Vector3D* v);
        void clone(SPtr<Instances::fl_geom::Vector3D>& result);
        void crossProduct(SPtr<Instances::fl_geom::Vector3D>& result, Instances::fl_geom::Vector3D* v);
        void decrementBy(const Value& result, Instances::fl_geom::Vector3D* v);
        void dotProduct(Value::Number& result, Instances::fl_geom::Vector3D* a);
        void equals(bool& result, Instances::fl_geom::Vector3D* toCompare, bool allFour = false);
        void incrementBy(const Value& result, Instances::fl_geom::Vector3D* v);
        void nearEquals(bool& result, Instances::fl_geom::Vector3D* toCompare, Value::Number tolerance, bool allFour = false);
        void negate(const Value& result);
        void normalize(Value::Number& result);
        void project(const Value& result);
        void scaleBy(const Value& result, Value::Number s);
        void subtract(SPtr<Instances::fl_geom::Vector3D>& result, Instances::fl_geom::Vector3D* v);
        void toString(ASString& result);

        // C++ friendly wrappers for AS3 methods.
        Value::Number lengthGet()
        {
            Value::Number result;
            lengthGet(result);
            return result;
        }
        Value::Number lengthSquaredGet()
        {
            Value::Number result;
            lengthSquaredGet(result);
            return result;
        }
        void xSet(Value::Number value)
        {
            xSet(Value::GetUndefined(), value);
        }
        Value::Number xGet()
        {
            Value::Number result;
            xGet(result);
            return result;
        }
        void ySet(Value::Number value)
        {
            ySet(Value::GetUndefined(), value);
        }
        Value::Number yGet()
        {
            Value::Number result;
            yGet(result);
            return result;
        }
        void zSet(Value::Number value)
        {
            zSet(Value::GetUndefined(), value);
        }
        Value::Number zGet()
        {
            Value::Number result;
            zGet(result);
            return result;
        }
        void wSet(Value::Number value)
        {
            wSet(Value::GetUndefined(), value);
        }
        Value::Number wGet()
        {
            Value::Number result;
            wGet(result);
            return result;
        }
        SPtr<Instances::fl_geom::Vector3D> add(Instances::fl_geom::Vector3D* v)
        {
            SPtr<Instances::fl_geom::Vector3D> result;
            add(result, v);
            return result;
        }
        SPtr<Instances::fl_geom::Vector3D> clone()
        {
            SPtr<Instances::fl_geom::Vector3D> result;
            clone(result);
            return result;
        }
        SPtr<Instances::fl_geom::Vector3D> crossProduct(Instances::fl_geom::Vector3D* v)
        {
            SPtr<Instances::fl_geom::Vector3D> result;
            crossProduct(result, v);
            return result;
        }
        void decrementBy(Instances::fl_geom::Vector3D* v)
        {
            decrementBy(Value::GetUndefined(), v);
        }
        Value::Number dotProduct(Instances::fl_geom::Vector3D* a)
        {
            Value::Number result;
            dotProduct(result, a);
            return result;
        }
        bool equals(Instances::fl_geom::Vector3D* toCompare, bool allFour = false)
        {
            bool result;
            equals(result, toCompare, allFour);
            return result;
        }
        void incrementBy(Instances::fl_geom::Vector3D* v)
        {
            incrementBy(Value::GetUndefined(), v);
        }
        bool nearEquals(Instances::fl_geom::Vector3D* toCompare, Value::Number tolerance, bool allFour = false)
        {
            bool result;
            nearEquals(result, toCompare, tolerance, allFour);
            return result;
        }
        void negate()
        {
            negate(Value::GetUndefined());
        }
        Value::Number normalize()
        {
            Value::Number result;
            normalize(result);
            return result;
        }
        void project()
        {
            project(Value::GetUndefined());
        }
        void scaleBy(Value::Number s)
        {
            scaleBy(Value::GetUndefined(), s);
        }
        SPtr<Instances::fl_geom::Vector3D> subtract(Instances::fl_geom::Vector3D* v)
        {
            SPtr<Instances::fl_geom::Vector3D> result;
            subtract(result, v);
            return result;
        }
        ASString toString()
        {
            ASString result(GetStringManager().CreateEmptyString());
            toString(result);
            return result;
        }

//##protect##"instance$data"
        Value::Number x;
        Value::Number y;
        Value::Number z;
        Value::Number w;

        Value::Number GetX() {return x;};
        Value::Number GetY() {return y;};
        Value::Number GetZ() {return z;};
		Value::Number GetW() {return w;};
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_geom
{
    class Vector3D : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::Vector3D"; }
#endif
    public:
        typedef Instances::fl_geom::Vector3D InstanceType;

    public:
        Vector3D(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_geom::Vector3D> MakeInstance(Vector3D& t)
        {
            return Pickable<Instances::fl_geom::Vector3D>(new(t.Alloc()) Instances::fl_geom::Vector3D(t));
        }
        SPtr<Instances::fl_geom::Vector3D> MakeInstanceS(Vector3D& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 24 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[40];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_geom
{
    class Vector3D : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Vector3D"; }
#endif
    public:
        typedef Classes::fl_geom::Vector3D ClassType;
        typedef InstanceTraits::fl_geom::Vector3D InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Vector3D(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 3 };
        static const MemberInfo mi[MemberInfoNum];
        enum { ThunkInfoNum = 2 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[6];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_geom
{
    class Vector3D : public Class
    {
        friend class ClassTraits::fl_geom::Vector3D;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_geom::Vector3DTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::Vector3D"; }
#endif
    public:
        typedef Vector3D SelfType;
        typedef Vector3D ClassType;
        
    private:
        Vector3D(ClassTraits::Traits& t);
       
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
            mid_angleBetween, 
            mid_distance, 
        };
        void angleBetween(Value::Number& result, Instances::fl_geom::Vector3D* a, Instances::fl_geom::Vector3D* b);
        void distance(Value::Number& result, Instances::fl_geom::Vector3D* pt1, Instances::fl_geom::Vector3D* pt2);

        // C++ friendly wrappers for AS3 methods.
        Value::Number angleBetween(Instances::fl_geom::Vector3D* a, Instances::fl_geom::Vector3D* b)
        {
            Value::Number result;
            angleBetween(result, a, b);
            return result;
        }
        Value::Number distance(Instances::fl_geom::Vector3D* pt1, Instances::fl_geom::Vector3D* pt2)
        {
            Value::Number result;
            distance(result, pt1, pt2);
            return result;
        }

    public:
        // AS3 API members.
        SPtr<Instances::fl_geom::Vector3D> X_AXIS;
        SPtr<Instances::fl_geom::Vector3D> Y_AXIS;
        SPtr<Instances::fl_geom::Vector3D> Z_AXIS;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

