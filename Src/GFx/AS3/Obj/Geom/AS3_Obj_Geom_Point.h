//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Geom_Point.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Geom_Point_H
#define INC_AS3_Obj_Geom_Point_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_geom
{
    extern const TypeInfo PointTI;
    extern const ClassInfo PointCI;
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
    class Point;
}}

namespace InstanceTraits { namespace fl_geom
{
    class Point;
}}

namespace Classes { namespace fl_geom
{
    class Point;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_geom
{
    class Point : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_geom::Point;
        
    public:
        typedef Point SelfType;
        typedef Classes::fl_geom::Point ClassType;
        typedef InstanceTraits::fl_geom::Point TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_geom::PointTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_geom::Point"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        Point(InstanceTraits::Traits& t);

//##protect##"instance$methods"
	virtual void AS3Constructor(unsigned argc, const Value* argv);
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_lengthGet, 
            mid_add, 
            mid_clone, 
            mid_equals, 
            mid_normalize, 
            mid_offset, 
            mid_subtract, 
            mid_toString, 
        };
        void lengthGet(Value::Number& result);
        void add(SPtr<Instances::fl_geom::Point>& result, Instances::fl_geom::Point* v);
        void clone(SPtr<Instances::fl_geom::Point>& result);
        void equals(bool& result, Instances::fl_geom::Point* toCompare);
        void normalize(const Value& result, Value::Number thickness);
        void offset(const Value& result, Value::Number dx, Value::Number dy);
        void subtract(SPtr<Instances::fl_geom::Point>& result, Instances::fl_geom::Point* v);
        void toString(ASString& result);

        // C++ friendly wrappers for AS3 methods.
        Value::Number lengthGet()
        {
            Value::Number result;
            lengthGet(result);
            return result;
        }
        SPtr<Instances::fl_geom::Point> add(Instances::fl_geom::Point* v)
        {
            SPtr<Instances::fl_geom::Point> result;
            add(result, v);
            return result;
        }
        SPtr<Instances::fl_geom::Point> clone()
        {
            SPtr<Instances::fl_geom::Point> result;
            clone(result);
            return result;
        }
        bool equals(Instances::fl_geom::Point* toCompare)
        {
            bool result;
            equals(result, toCompare);
            return result;
        }
        void normalize(Value::Number thickness)
        {
            normalize(Value::GetUndefined(), thickness);
        }
        void offset(Value::Number dx, Value::Number dy)
        {
            offset(Value::GetUndefined(), dx, dy);
        }
        SPtr<Instances::fl_geom::Point> subtract(Instances::fl_geom::Point* v)
        {
            SPtr<Instances::fl_geom::Point> result;
            subtract(result, v);
            return result;
        }
        ASString toString()
        {
            ASString result(GetStringManager().CreateEmptyString());
            toString(result);
            return result;
        }

    public:
        // AS3 API members.
        Value::Number x;
        Value::Number y;

//##protect##"instance$data"
    public:
        Value::Number GetX() const {return x;};
        Value::Number GetY() const {return y;};
        void SetX(Value::Number value) { x = value; }
        void SetY(Value::Number value) { y = value; }
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_geom
{
    class Point : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::Point"; }
#endif
    public:
        typedef Instances::fl_geom::Point InstanceType;

    public:
        Point(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_geom::Point> MakeInstance(Point& t)
        {
            return Pickable<Instances::fl_geom::Point>(new(t.Alloc()) Instances::fl_geom::Point(t));
        }
        SPtr<Instances::fl_geom::Point> MakeInstanceS(Point& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { MemberInfoNum = 2 };
        static const MemberInfo mi[MemberInfoNum];
        enum { ThunkInfoNum = 8 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[14];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_geom
{
    class Point : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Point"; }
#endif
    public:
        typedef Classes::fl_geom::Point ClassType;
        typedef InstanceTraits::fl_geom::Point InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Point(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { ThunkInfoNum = 3 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[10];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_geom
{
    class Point : public Class
    {
        friend class ClassTraits::fl_geom::Point;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_geom::PointTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::Point"; }
#endif
    public:
        typedef Point SelfType;
        typedef Point ClassType;
        
    private:
        Point(ClassTraits::Traits& t);
       
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
            mid_distance, 
            mid_interpolate, 
            mid_polar, 
        };
        void distance(Value::Number& result, Instances::fl_geom::Point* pt1, Instances::fl_geom::Point* pt2);
        void interpolate(SPtr<Instances::fl_geom::Point>& result, Instances::fl_geom::Point* pt1, Instances::fl_geom::Point* pt2, Value::Number f);
        void polar(SPtr<Instances::fl_geom::Point>& result, Value::Number len, Value::Number angle);

        // C++ friendly wrappers for AS3 methods.
        Value::Number distance(Instances::fl_geom::Point* pt1, Instances::fl_geom::Point* pt2)
        {
            Value::Number result;
            distance(result, pt1, pt2);
            return result;
        }
        SPtr<Instances::fl_geom::Point> interpolate(Instances::fl_geom::Point* pt1, Instances::fl_geom::Point* pt2, Value::Number f)
        {
            SPtr<Instances::fl_geom::Point> result;
            interpolate(result, pt1, pt2, f);
            return result;
        }
        SPtr<Instances::fl_geom::Point> polar(Value::Number len, Value::Number angle)
        {
            SPtr<Instances::fl_geom::Point> result;
            polar(result, len, angle);
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

