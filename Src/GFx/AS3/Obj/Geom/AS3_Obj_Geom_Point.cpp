//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Geom_Point.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Geom_Point.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_geom::Point, Instances::fl_geom::Point::mid_lengthGet, Value::Number> TFunc_Instances_Point_lengthGet;
typedef ThunkFunc1<Instances::fl_geom::Point, Instances::fl_geom::Point::mid_add, SPtr<Instances::fl_geom::Point>, Instances::fl_geom::Point*> TFunc_Instances_Point_add;
typedef ThunkFunc0<Instances::fl_geom::Point, Instances::fl_geom::Point::mid_clone, SPtr<Instances::fl_geom::Point> > TFunc_Instances_Point_clone;
typedef ThunkFunc1<Instances::fl_geom::Point, Instances::fl_geom::Point::mid_equals, bool, Instances::fl_geom::Point*> TFunc_Instances_Point_equals;
typedef ThunkFunc1<Instances::fl_geom::Point, Instances::fl_geom::Point::mid_normalize, const Value, Value::Number> TFunc_Instances_Point_normalize;
typedef ThunkFunc2<Instances::fl_geom::Point, Instances::fl_geom::Point::mid_offset, const Value, Value::Number, Value::Number> TFunc_Instances_Point_offset;
typedef ThunkFunc1<Instances::fl_geom::Point, Instances::fl_geom::Point::mid_subtract, SPtr<Instances::fl_geom::Point>, Instances::fl_geom::Point*> TFunc_Instances_Point_subtract;
typedef ThunkFunc0<Instances::fl_geom::Point, Instances::fl_geom::Point::mid_toString, ASString> TFunc_Instances_Point_toString;

template <> const TFunc_Instances_Point_lengthGet::TMethod TFunc_Instances_Point_lengthGet::Method = &Instances::fl_geom::Point::lengthGet;
template <> const TFunc_Instances_Point_add::TMethod TFunc_Instances_Point_add::Method = &Instances::fl_geom::Point::add;
template <> const TFunc_Instances_Point_clone::TMethod TFunc_Instances_Point_clone::Method = &Instances::fl_geom::Point::clone;
template <> const TFunc_Instances_Point_equals::TMethod TFunc_Instances_Point_equals::Method = &Instances::fl_geom::Point::equals;
template <> const TFunc_Instances_Point_normalize::TMethod TFunc_Instances_Point_normalize::Method = &Instances::fl_geom::Point::normalize;
template <> const TFunc_Instances_Point_offset::TMethod TFunc_Instances_Point_offset::Method = &Instances::fl_geom::Point::offset;
template <> const TFunc_Instances_Point_subtract::TMethod TFunc_Instances_Point_subtract::Method = &Instances::fl_geom::Point::subtract;
template <> const TFunc_Instances_Point_toString::TMethod TFunc_Instances_Point_toString::Method = &Instances::fl_geom::Point::toString;

namespace Instances { namespace fl_geom
{
    Point::Point(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
    , x()
    , y()
//##protect##"instance::Point::Point()$data"
//##protect##"instance::Point::Point()$data"
    {
//##protect##"instance::Point::Point()$code"
//##protect##"instance::Point::Point()$code"
    }

    void Point::lengthGet(Value::Number& result)
    {
//##protect##"instance::Point::lengthGet()"
        result = sqrt(x*x + y*y);
//##protect##"instance::Point::lengthGet()"
    }
    void Point::add(SPtr<Instances::fl_geom::Point>& result, Instances::fl_geom::Point* v)
    {
//##protect##"instance::Point::add()"
		if (!v)
		{
            return (GetVM().ThrowTypeError(VM::Error(VM::eConvertNullToObjectError, GetVM())));
		}
		Value args[] = {Value(x + v->x), Value(y + v->y)};
		// by calling NoNotCheck, we avoid throwing an exception if the return value of 
		// ConstructBuiltinValue is not compared to anything. Safe since this is the last 
		// line in the code.
		GetVM().ConstructBuiltinObject(result, "flash.geom.Point", 2, args).DoNotCheck();
//##protect##"instance::Point::add()"
    }
    void Point::clone(SPtr<Instances::fl_geom::Point>& result)
    {
//##protect##"instance::Point::clone()"
		Value args[] = {Value(x), Value(y)};
		GetVM().ConstructBuiltinObject(result, "flash.geom.Point", 2, args).DoNotCheck();
//##protect##"instance::Point::clone()"
    }
    void Point::equals(bool& result, Instances::fl_geom::Point* toCompare)
    {
//##protect##"instance::Point::equals()"
		if (!toCompare)
		{
			return (GetVM().ThrowTypeError(VM::Error(VM::eConvertNullToObjectError, GetVM())));
		}
        result = ((x == toCompare->x) && (y == toCompare->y));
//##protect##"instance::Point::equals()"
    }
    void Point::normalize(const Value& result, Value::Number thickness)
    {
//##protect##"instance::Point::normalize()"
        SF_UNUSED1(result);
		// Modifies the original point, don't create a new one!
		if (x == 0 && y == 0)
		{
			// Replicate flash player's behavior in this case.
			x = 0;
			y = 0;
			return;
		}
		double tmp = x*thickness/sqrt(x*x+y*y);
		y = y*thickness/sqrt(x*x+y*y);
		x = tmp;
//##protect##"instance::Point::normalize()"
    }
    void Point::offset(const Value& result, Value::Number dx, Value::Number dy)
    {
//##protect##"instance::Point::offset()"
        SF_UNUSED1(result);
		// Modifies the original point, don't create a new one!
		x += dx;
		y += dy;
//##protect##"instance::Point::offset()"
    }
    void Point::subtract(SPtr<Instances::fl_geom::Point>& result, Instances::fl_geom::Point* v)
    {
//##protect##"instance::Point::subtract()"
		// Creates new point
		if (!v)
		{
			return (GetVM().ThrowTypeError(VM::Error(VM::eConvertNullToObjectError, GetVM())));
		}
		Value args[] = {Value(x - v->x), Value(y - v->y)};
		GetVM().ConstructBuiltinObject(result, "flash.geom.Point", 2, args).DoNotCheck();
//##protect##"instance::Point::subtract()"
    }
    void Point::toString(ASString& result)
    {
//##protect##"instance::Point::toString()"
		result += GetStringManager().CreateConstString("(x=") + GetVM().AsString(Value(x)) + 
			GetStringManager().CreateConstString(", ") + 
			GetStringManager().CreateConstString("y=") + 
			GetVM().AsString(Value(y)) +
			GetStringManager().CreateConstString(")");
//##protect##"instance::Point::toString()"
    }

//##protect##"instance$methods"
    void Point::AS3Constructor(unsigned argc, const Value* argv)
    {

        if (argc > 0)
        {
            // According to doc, x will not be modified if there is an exception
            if (!argv[0].Convert2Number(x))
            {
                return;
            }
            if (argc > 1)
            {
                if (!argv[1].Convert2Number(y))
                {
                    return;
                }
            }
        }
    }

//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_geom
{
    // const UInt16 Point::tito[Point::ThunkInfoNum] = {
    //    0, 1, 3, 4, 6, 8, 11, 13, 
    // };
    const TypeInfo* Point::tit[14] = {
        &AS3::fl::NumberTI, 
        &AS3::fl_geom::PointTI, &AS3::fl_geom::PointTI, 
        &AS3::fl_geom::PointTI, 
        &AS3::fl::BooleanTI, &AS3::fl_geom::PointTI, 
        NULL, &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, &AS3::fl::NumberTI, 
        &AS3::fl_geom::PointTI, &AS3::fl_geom::PointTI, 
        &AS3::fl::StringTI, 
    };
    const ThunkInfo Point::ti[Point::ThunkInfoNum] = {
        {TFunc_Instances_Point_lengthGet::Func, &Point::tit[0], "length", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Point_add::Func, &Point::tit[1], "add", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Point_clone::Func, &Point::tit[3], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Point_equals::Func, &Point::tit[4], "equals", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Point_normalize::Func, &Point::tit[6], "normalize", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Point_offset::Func, &Point::tit[8], "offset", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Instances_Point_subtract::Func, &Point::tit[11], "subtract", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Point_toString::Func, &Point::tit[13], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };
    const MemberInfo Point::mi[Point::MemberInfoNum] = {
        {"x", NULL, OFFSETOF(Instances::fl_geom::Point, x), Abc::NS_Public, SlotInfo::BT_Number, 0},
        {"y", NULL, OFFSETOF(Instances::fl_geom::Point, y), Abc::NS_Public, SlotInfo::BT_Number, 0},
    };


    Point::Point(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::Point::Point()"
//##protect##"InstanceTraits::Point::Point()"

    }

    void Point::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<Point&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl_geom
{
    Point::Point(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::Point::Point()"
//##protect##"class_::Point::Point()"
    }
    void Point::distance(Value::Number& result, Instances::fl_geom::Point* pt1, Instances::fl_geom::Point* pt2)
    {
//##protect##"class_::Point::distance()"
        result = Value(MakeValueNumber(sqrt((pt1->GetX() - pt2->GetX())*(pt1->GetX() - pt2->GetX()) + 
			(pt1->GetY() - pt2->GetY())*(pt1->GetY() - pt2->GetY()))));
//##protect##"class_::Point::distance()"
    }
    void Point::interpolate(SPtr<Instances::fl_geom::Point>& result, Instances::fl_geom::Point* pt1, Instances::fl_geom::Point* pt2, Value::Number f)
    {
//##protect##"class_::Point::interpolate()"
		Value args[] = {Value((pt1->GetX() - pt2->GetX())*f + pt2->GetX()), Value((pt1->GetY() - pt2->GetY())*f + pt2->GetY())};
		GetVM().ConstructBuiltinObject(result, "flash.geom.Point", 2, args).DoNotCheck();
//##protect##"class_::Point::interpolate()"
    }
    void Point::polar(SPtr<Instances::fl_geom::Point>& result, Value::Number len, Value::Number angle)
    {
//##protect##"class_::Point::polar()"
		Value args[] = {Value(MakeValueNumber(len*cos(angle))), Value(MakeValueNumber(len*sin(angle)))};
		GetVM().ConstructBuiltinObject(result, "flash.geom.Point", 2, args).DoNotCheck();
//##protect##"class_::Point::polar()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes

typedef ThunkFunc2<Classes::fl_geom::Point, Classes::fl_geom::Point::mid_distance, Value::Number, Instances::fl_geom::Point*, Instances::fl_geom::Point*> TFunc_Classes_Point_distance;
typedef ThunkFunc3<Classes::fl_geom::Point, Classes::fl_geom::Point::mid_interpolate, SPtr<Instances::fl_geom::Point>, Instances::fl_geom::Point*, Instances::fl_geom::Point*, Value::Number> TFunc_Classes_Point_interpolate;
typedef ThunkFunc2<Classes::fl_geom::Point, Classes::fl_geom::Point::mid_polar, SPtr<Instances::fl_geom::Point>, Value::Number, Value::Number> TFunc_Classes_Point_polar;

template <> const TFunc_Classes_Point_distance::TMethod TFunc_Classes_Point_distance::Method = &Classes::fl_geom::Point::distance;
template <> const TFunc_Classes_Point_interpolate::TMethod TFunc_Classes_Point_interpolate::Method = &Classes::fl_geom::Point::interpolate;
template <> const TFunc_Classes_Point_polar::TMethod TFunc_Classes_Point_polar::Method = &Classes::fl_geom::Point::polar;

namespace ClassTraits { namespace fl_geom
{
    // const UInt16 Point::tito[Point::ThunkInfoNum] = {
    //    0, 3, 7, 
    // };
    const TypeInfo* Point::tit[10] = {
        &AS3::fl::NumberTI, &AS3::fl_geom::PointTI, &AS3::fl_geom::PointTI, 
        &AS3::fl_geom::PointTI, &AS3::fl_geom::PointTI, &AS3::fl_geom::PointTI, &AS3::fl::NumberTI, 
        &AS3::fl_geom::PointTI, &AS3::fl::NumberTI, &AS3::fl::NumberTI, 
    };
    const ThunkInfo Point::ti[Point::ThunkInfoNum] = {
        {TFunc_Classes_Point_distance::Func, &Point::tit[0], "distance", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Classes_Point_interpolate::Func, &Point::tit[3], "interpolate", NULL, Abc::NS_Public, CT_Method, 3, 3, 0, 0, NULL},
        {TFunc_Classes_Point_polar::Func, &Point::tit[7], "polar", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
    };

    Point::Point(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::Point::Point()"
//##protect##"ClassTraits::Point::Point()"

    }

    Pickable<Traits> Point::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) Point(vm, AS3::fl_geom::PointCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_geom::PointCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_geom
{
    const TypeInfo PointTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_geom::Point::InstanceType),
        ClassTraits::fl_geom::Point::ThunkInfoNum,
        0,
        InstanceTraits::fl_geom::Point::ThunkInfoNum,
        InstanceTraits::fl_geom::Point::MemberInfoNum,
        "Point", "flash.geom", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo PointCI = {
        &PointTI,
        ClassTraits::fl_geom::Point::MakeClassTraits,
        ClassTraits::fl_geom::Point::ti,
        NULL,
        InstanceTraits::fl_geom::Point::ti,
        InstanceTraits::fl_geom::Point::mi,
    };
}; // namespace fl_geom


}}} // namespace Scaleform { namespace GFx { namespace AS3

