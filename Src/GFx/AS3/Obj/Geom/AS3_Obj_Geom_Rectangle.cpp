//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Geom_Rectangle.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Geom_Rectangle.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "AS3_Obj_Geom_Point.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_geom::Rectangle, Instances::fl_geom::Rectangle::mid_bottomGet, Value::Number> TFunc_Instances_Rectangle_bottomGet;
typedef ThunkFunc1<Instances::fl_geom::Rectangle, Instances::fl_geom::Rectangle::mid_bottomSet, const Value, Value::Number> TFunc_Instances_Rectangle_bottomSet;
typedef ThunkFunc0<Instances::fl_geom::Rectangle, Instances::fl_geom::Rectangle::mid_bottomRightGet, SPtr<Instances::fl_geom::Point> > TFunc_Instances_Rectangle_bottomRightGet;
typedef ThunkFunc1<Instances::fl_geom::Rectangle, Instances::fl_geom::Rectangle::mid_bottomRightSet, const Value, Instances::fl_geom::Point*> TFunc_Instances_Rectangle_bottomRightSet;
typedef ThunkFunc0<Instances::fl_geom::Rectangle, Instances::fl_geom::Rectangle::mid_leftGet, Value::Number> TFunc_Instances_Rectangle_leftGet;
typedef ThunkFunc1<Instances::fl_geom::Rectangle, Instances::fl_geom::Rectangle::mid_leftSet, const Value, Value::Number> TFunc_Instances_Rectangle_leftSet;
typedef ThunkFunc0<Instances::fl_geom::Rectangle, Instances::fl_geom::Rectangle::mid_rightGet, Value::Number> TFunc_Instances_Rectangle_rightGet;
typedef ThunkFunc1<Instances::fl_geom::Rectangle, Instances::fl_geom::Rectangle::mid_rightSet, const Value, Value::Number> TFunc_Instances_Rectangle_rightSet;
typedef ThunkFunc0<Instances::fl_geom::Rectangle, Instances::fl_geom::Rectangle::mid_sizeGet, SPtr<Instances::fl_geom::Point> > TFunc_Instances_Rectangle_sizeGet;
typedef ThunkFunc1<Instances::fl_geom::Rectangle, Instances::fl_geom::Rectangle::mid_sizeSet, const Value, Instances::fl_geom::Point*> TFunc_Instances_Rectangle_sizeSet;
typedef ThunkFunc0<Instances::fl_geom::Rectangle, Instances::fl_geom::Rectangle::mid_topGet, Value::Number> TFunc_Instances_Rectangle_topGet;
typedef ThunkFunc1<Instances::fl_geom::Rectangle, Instances::fl_geom::Rectangle::mid_topSet, const Value, Value::Number> TFunc_Instances_Rectangle_topSet;
typedef ThunkFunc0<Instances::fl_geom::Rectangle, Instances::fl_geom::Rectangle::mid_topLeftGet, SPtr<Instances::fl_geom::Point> > TFunc_Instances_Rectangle_topLeftGet;
typedef ThunkFunc1<Instances::fl_geom::Rectangle, Instances::fl_geom::Rectangle::mid_topLeftSet, const Value, Instances::fl_geom::Point*> TFunc_Instances_Rectangle_topLeftSet;
typedef ThunkFunc0<Instances::fl_geom::Rectangle, Instances::fl_geom::Rectangle::mid_clone, SPtr<Instances::fl_geom::Rectangle> > TFunc_Instances_Rectangle_clone;
typedef ThunkFunc2<Instances::fl_geom::Rectangle, Instances::fl_geom::Rectangle::mid_contains, bool, Value::Number, Value::Number> TFunc_Instances_Rectangle_contains;
typedef ThunkFunc1<Instances::fl_geom::Rectangle, Instances::fl_geom::Rectangle::mid_containsPoint, bool, Instances::fl_geom::Point*> TFunc_Instances_Rectangle_containsPoint;
typedef ThunkFunc1<Instances::fl_geom::Rectangle, Instances::fl_geom::Rectangle::mid_containsRect, bool, Instances::fl_geom::Rectangle*> TFunc_Instances_Rectangle_containsRect;
typedef ThunkFunc1<Instances::fl_geom::Rectangle, Instances::fl_geom::Rectangle::mid_equals, bool, Instances::fl_geom::Rectangle*> TFunc_Instances_Rectangle_equals;
typedef ThunkFunc2<Instances::fl_geom::Rectangle, Instances::fl_geom::Rectangle::mid_inflate, const Value, Value::Number, Value::Number> TFunc_Instances_Rectangle_inflate;
typedef ThunkFunc1<Instances::fl_geom::Rectangle, Instances::fl_geom::Rectangle::mid_inflatePoint, const Value, Instances::fl_geom::Point*> TFunc_Instances_Rectangle_inflatePoint;
typedef ThunkFunc1<Instances::fl_geom::Rectangle, Instances::fl_geom::Rectangle::mid_intersection, SPtr<Instances::fl_geom::Rectangle>, Instances::fl_geom::Rectangle*> TFunc_Instances_Rectangle_intersection;
typedef ThunkFunc1<Instances::fl_geom::Rectangle, Instances::fl_geom::Rectangle::mid_intersects, bool, Instances::fl_geom::Rectangle*> TFunc_Instances_Rectangle_intersects;
typedef ThunkFunc0<Instances::fl_geom::Rectangle, Instances::fl_geom::Rectangle::mid_isEmpty, bool> TFunc_Instances_Rectangle_isEmpty;
typedef ThunkFunc2<Instances::fl_geom::Rectangle, Instances::fl_geom::Rectangle::mid_offset, const Value, Value::Number, Value::Number> TFunc_Instances_Rectangle_offset;
typedef ThunkFunc1<Instances::fl_geom::Rectangle, Instances::fl_geom::Rectangle::mid_offsetPoint, const Value, Instances::fl_geom::Point*> TFunc_Instances_Rectangle_offsetPoint;
typedef ThunkFunc0<Instances::fl_geom::Rectangle, Instances::fl_geom::Rectangle::mid_setEmpty, const Value> TFunc_Instances_Rectangle_setEmpty;
typedef ThunkFunc0<Instances::fl_geom::Rectangle, Instances::fl_geom::Rectangle::mid_toString, ASString> TFunc_Instances_Rectangle_toString;
typedef ThunkFunc1<Instances::fl_geom::Rectangle, Instances::fl_geom::Rectangle::mid_union_, SPtr<Instances::fl_geom::Rectangle>, Instances::fl_geom::Rectangle*> TFunc_Instances_Rectangle_union_;

template <> const TFunc_Instances_Rectangle_bottomGet::TMethod TFunc_Instances_Rectangle_bottomGet::Method = &Instances::fl_geom::Rectangle::bottomGet;
template <> const TFunc_Instances_Rectangle_bottomSet::TMethod TFunc_Instances_Rectangle_bottomSet::Method = &Instances::fl_geom::Rectangle::bottomSet;
template <> const TFunc_Instances_Rectangle_bottomRightGet::TMethod TFunc_Instances_Rectangle_bottomRightGet::Method = &Instances::fl_geom::Rectangle::bottomRightGet;
template <> const TFunc_Instances_Rectangle_bottomRightSet::TMethod TFunc_Instances_Rectangle_bottomRightSet::Method = &Instances::fl_geom::Rectangle::bottomRightSet;
template <> const TFunc_Instances_Rectangle_leftGet::TMethod TFunc_Instances_Rectangle_leftGet::Method = &Instances::fl_geom::Rectangle::leftGet;
template <> const TFunc_Instances_Rectangle_leftSet::TMethod TFunc_Instances_Rectangle_leftSet::Method = &Instances::fl_geom::Rectangle::leftSet;
template <> const TFunc_Instances_Rectangle_rightGet::TMethod TFunc_Instances_Rectangle_rightGet::Method = &Instances::fl_geom::Rectangle::rightGet;
template <> const TFunc_Instances_Rectangle_rightSet::TMethod TFunc_Instances_Rectangle_rightSet::Method = &Instances::fl_geom::Rectangle::rightSet;
template <> const TFunc_Instances_Rectangle_sizeGet::TMethod TFunc_Instances_Rectangle_sizeGet::Method = &Instances::fl_geom::Rectangle::sizeGet;
template <> const TFunc_Instances_Rectangle_sizeSet::TMethod TFunc_Instances_Rectangle_sizeSet::Method = &Instances::fl_geom::Rectangle::sizeSet;
template <> const TFunc_Instances_Rectangle_topGet::TMethod TFunc_Instances_Rectangle_topGet::Method = &Instances::fl_geom::Rectangle::topGet;
template <> const TFunc_Instances_Rectangle_topSet::TMethod TFunc_Instances_Rectangle_topSet::Method = &Instances::fl_geom::Rectangle::topSet;
template <> const TFunc_Instances_Rectangle_topLeftGet::TMethod TFunc_Instances_Rectangle_topLeftGet::Method = &Instances::fl_geom::Rectangle::topLeftGet;
template <> const TFunc_Instances_Rectangle_topLeftSet::TMethod TFunc_Instances_Rectangle_topLeftSet::Method = &Instances::fl_geom::Rectangle::topLeftSet;
template <> const TFunc_Instances_Rectangle_clone::TMethod TFunc_Instances_Rectangle_clone::Method = &Instances::fl_geom::Rectangle::clone;
template <> const TFunc_Instances_Rectangle_contains::TMethod TFunc_Instances_Rectangle_contains::Method = &Instances::fl_geom::Rectangle::contains;
template <> const TFunc_Instances_Rectangle_containsPoint::TMethod TFunc_Instances_Rectangle_containsPoint::Method = &Instances::fl_geom::Rectangle::containsPoint;
template <> const TFunc_Instances_Rectangle_containsRect::TMethod TFunc_Instances_Rectangle_containsRect::Method = &Instances::fl_geom::Rectangle::containsRect;
template <> const TFunc_Instances_Rectangle_equals::TMethod TFunc_Instances_Rectangle_equals::Method = &Instances::fl_geom::Rectangle::equals;
template <> const TFunc_Instances_Rectangle_inflate::TMethod TFunc_Instances_Rectangle_inflate::Method = &Instances::fl_geom::Rectangle::inflate;
template <> const TFunc_Instances_Rectangle_inflatePoint::TMethod TFunc_Instances_Rectangle_inflatePoint::Method = &Instances::fl_geom::Rectangle::inflatePoint;
template <> const TFunc_Instances_Rectangle_intersection::TMethod TFunc_Instances_Rectangle_intersection::Method = &Instances::fl_geom::Rectangle::intersection;
template <> const TFunc_Instances_Rectangle_intersects::TMethod TFunc_Instances_Rectangle_intersects::Method = &Instances::fl_geom::Rectangle::intersects;
template <> const TFunc_Instances_Rectangle_isEmpty::TMethod TFunc_Instances_Rectangle_isEmpty::Method = &Instances::fl_geom::Rectangle::isEmpty;
template <> const TFunc_Instances_Rectangle_offset::TMethod TFunc_Instances_Rectangle_offset::Method = &Instances::fl_geom::Rectangle::offset;
template <> const TFunc_Instances_Rectangle_offsetPoint::TMethod TFunc_Instances_Rectangle_offsetPoint::Method = &Instances::fl_geom::Rectangle::offsetPoint;
template <> const TFunc_Instances_Rectangle_setEmpty::TMethod TFunc_Instances_Rectangle_setEmpty::Method = &Instances::fl_geom::Rectangle::setEmpty;
template <> const TFunc_Instances_Rectangle_toString::TMethod TFunc_Instances_Rectangle_toString::Method = &Instances::fl_geom::Rectangle::toString;
template <> const TFunc_Instances_Rectangle_union_::TMethod TFunc_Instances_Rectangle_union_::Method = &Instances::fl_geom::Rectangle::union_;

namespace Instances { namespace fl_geom
{
    Rectangle::Rectangle(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
    , height()
    , width()
    , x()
    , y()
//##protect##"instance::Rectangle::Rectangle()$data"
//##protect##"instance::Rectangle::Rectangle()$data"
    {
//##protect##"instance::Rectangle::Rectangle()$code"
//##protect##"instance::Rectangle::Rectangle()$code"
    }

    void Rectangle::bottomGet(Value::Number& result)
    {
//##protect##"instance::Rectangle::bottomGet()"
        result = y+height;
//##protect##"instance::Rectangle::bottomGet()"
    }
    void Rectangle::bottomSet(const Value& result, Value::Number value)
    {
//##protect##"instance::Rectangle::bottomSet()"
        SF_UNUSED1(result);
		height = value - y;
//##protect##"instance::Rectangle::bottomSet()"
    }
    void Rectangle::bottomRightGet(SPtr<Instances::fl_geom::Point>& result)
    {
//##protect##"instance::Rectangle::bottomRightGet()"
		Value args[] = {Value(x + width), Value(y + height)};
		GetVM().ConstructBuiltinObject(result, "flash.geom.Point", 2, args).DoNotCheck();
//##protect##"instance::Rectangle::bottomRightGet()"
    }
    void Rectangle::bottomRightSet(const Value& result, Instances::fl_geom::Point* value)
    {
//##protect##"instance::Rectangle::bottomRightSet()"
		if (!value)
		{
            return (GetVM().ThrowTypeError(VM::Error(VM::eConvertNullToObjectError, GetVM())));
		}
        width = value->GetX() - x;
		height = value->GetY() - y;
        SF_UNUSED1(result);
//##protect##"instance::Rectangle::bottomRightSet()"
    }
    void Rectangle::leftGet(Value::Number& result)
    {
//##protect##"instance::Rectangle::leftGet()"
        result = x;
//##protect##"instance::Rectangle::leftGet()"
    }
    void Rectangle::leftSet(const Value& result, Value::Number value)
    {
//##protect##"instance::Rectangle::leftSet()"
		width += (x-value);
        x = value;
        SF_UNUSED1(result);
		 
//##protect##"instance::Rectangle::leftSet()"
    }
    void Rectangle::rightGet(Value::Number& result)
    {
//##protect##"instance::Rectangle::rightGet()"
       result = x + width;
//##protect##"instance::Rectangle::rightGet()"
    }
    void Rectangle::rightSet(const Value& result, Value::Number value)
    {
//##protect##"instance::Rectangle::rightSet()"
        width = value - x;
        SF_UNUSED1(result);
//##protect##"instance::Rectangle::rightSet()"
    }
    void Rectangle::sizeGet(SPtr<Instances::fl_geom::Point>& result)
    {
//##protect##"instance::Rectangle::sizeGet()"
		Value args[] = {Value(width), Value(height)};
		GetVM().ConstructBuiltinObject(result, "flash.geom.Point", 2, args).DoNotCheck();
//##protect##"instance::Rectangle::sizeGet()"
    }
    void Rectangle::sizeSet(const Value& result, Instances::fl_geom::Point* value)
    {
//##protect##"instance::Rectangle::sizeSet()"
		if (!value)
		{
            return (GetVM().ThrowTypeError(VM::Error(VM::eConvertNullToObjectError, GetVM())));
		}
        width = value->GetX();
		height = value->GetY();
        SF_UNUSED1(result);
//##protect##"instance::Rectangle::sizeSet()"
    }
    void Rectangle::topGet(Value::Number& result)
    {
//##protect##"instance::Rectangle::topGet()"
        result = y;
//##protect##"instance::Rectangle::topGet()"
    }
    void Rectangle::topSet(const Value& result, Value::Number value)
    {
//##protect##"instance::Rectangle::topSet()"
        height += y - value;
		y = value;
        SF_UNUSED1(result);
//##protect##"instance::Rectangle::topSet()"
    }
    void Rectangle::topLeftGet(SPtr<Instances::fl_geom::Point>& result)
    {
//##protect##"instance::Rectangle::topLeftGet()"
		Value args[] = {Value(x), Value(y)};
		GetVM().ConstructBuiltinObject(result, "flash.geom.Point", 2, args).DoNotCheck();
//##protect##"instance::Rectangle::topLeftGet()"
    }
    void Rectangle::topLeftSet(const Value& result, Instances::fl_geom::Point* value)
    {
//##protect##"instance::Rectangle::topLeftSet()"
		if (!value)
		{
            return (GetVM().ThrowTypeError(VM::Error(VM::eConvertNullToObjectError, GetVM())));
		}
        width += x - value->GetX();
		height += y - value->GetY();
		x = value->GetX();
		y = value->GetY();
        SF_UNUSED1(result);
//##protect##"instance::Rectangle::topLeftSet()"
    }
    void Rectangle::clone(SPtr<Instances::fl_geom::Rectangle>& result)
    {
//##protect##"instance::Rectangle::clone()"
		Value args[] = {Value(x), Value(y), Value(width), Value(height)};
		GetVM().ConstructBuiltinObject(result, "flash.geom.Rectangle", 4, args).DoNotCheck();
//##protect##"instance::Rectangle::clone()"
    }
    void Rectangle::contains(bool& result, Value::Number vnx, Value::Number vny)
    {
//##protect##"instance::Rectangle::contains()"
		// note that bottom and right edges are not contained.
		Value::Number right = this->x + width;
		Value::Number bottom = this->y + height;
		result = ((vnx < right) && (vnx >= this->x) && (vny < bottom) && (vny >= this->y));
//##protect##"instance::Rectangle::contains()"
    }
    void Rectangle::containsPoint(bool& result, Instances::fl_geom::Point* point)
    {
//##protect##"instance::Rectangle::containsPoint()"
		if (!point)
		{
            return (GetVM().ThrowTypeError(VM::Error(VM::eConvertNullToObjectError, GetVM())));
		}
		Value::Number px = point->GetX();
		Value::Number py = point->GetY();
		return contains(result, px, py);
//##protect##"instance::Rectangle::containsPoint()"
    }
    void Rectangle::containsRect(bool& result, Instances::fl_geom::Rectangle* rect)
    {
//##protect##"instance::Rectangle::containsRect()"
		// special case for 0 width/height rectangles. 
		if (!rect)
		{
            return (GetVM().ThrowTypeError(VM::Error(VM::eConvertNullToObjectError, GetVM())));
		}
		if ((rect->GetHeight() == 0) && (rect->GetWidth() == 0))
		{
			if (x < rect->GetX() && y < rect->GetY())
			{
				result = true;
				return;
			}
			result = false;
			return;
		}
		Value::Number x2 = rect->GetX();
		Value::Number y2 = rect->GetY();
		Value::Number right2 = rect->GetWidth() + x2;
		Value::Number bottom2 = rect->GetHeight() + y2;

		Value::Number x1 = x;
		Value::Number y1 = y;
		Value::Number right1 = width + x1;
		Value::Number bottom1 = height + y1;

		result = ((right1 >= right2) && (bottom1 >= bottom2) && (x1 <= x2) && (y1 <= y2));
//##protect##"instance::Rectangle::containsRect()"
    }
    void Rectangle::equals(bool& result, Instances::fl_geom::Rectangle* toCompare)
    {
//##protect##"instance::Rectangle::equals()"
		if (!toCompare)
		{
            return (GetVM().ThrowTypeError(VM::Error(VM::eConvertNullToObjectError, GetVM())));
		}
       result = ((x == toCompare->GetX()) && (y == toCompare->GetY()) && (width == toCompare->GetWidth()) 
		   && (height == toCompare->GetHeight()));
//##protect##"instance::Rectangle::equals()"
    }
    void Rectangle::inflate(const Value& result, Value::Number dx, Value::Number dy)
    {
//##protect##"instance::Rectangle::inflate()"
		SF_UNUSED1(result);
        x = x - dx; width = width + 2*dx;
		y = y - dy; height = height + 2*dy;
//##protect##"instance::Rectangle::inflate()"
    }
    void Rectangle::inflatePoint(const Value& result, Instances::fl_geom::Point* point)
    {
//##protect##"instance::Rectangle::inflatePoint()"
        SF_UNUSED1(result);
		if (!point)
		{
            return (GetVM().ThrowTypeError(VM::Error(VM::eConvertNullToObjectError, GetVM())));
		}
		Value::Number dx = point->GetX();
		Value::Number dy = point->GetY();

		x = x - dx; width = width + 2*dx;
		y = y - dy; height = height + 2*dy;
//##protect##"instance::Rectangle::inflatePoint()"
    }
    void Rectangle::intersection(SPtr<Instances::fl_geom::Rectangle>& result, Instances::fl_geom::Rectangle* toIntersect)
    {
//##protect##"instance::Rectangle::intersection()"
		bool res1;
		if (!toIntersect)
		{
            return (GetVM().ThrowTypeError(VM::Error(VM::eConvertNullToObjectError, GetVM())));
		}
		intersects(res1, toIntersect);
		if (!res1)
		{
			const Value::Number zero = MakeValueNumber(0.0);
			Value args[] = {Value(zero), Value(zero), Value(zero), Value(zero)};
			return GetVM().ConstructBuiltinObject(result, "flash.geom.Rectangle", 4, args).DoNotCheck();
		}

		Value::Number x1		= toIntersect->GetX();
		Value::Number y1		= toIntersect->GetY();
		Value::Number right1	= x1 + toIntersect->GetWidth();
		Value::Number bottom1	= y1 + toIntersect->GetHeight();

		x1 = x > x1? x: x1;
		right1 = x + width > right1? right1: x+width;
		y1 = y > y1? y: y1;
		bottom1 = y + height > bottom1? bottom1: y + height;

		Value args[] = {Value(x1), Value(y1), Value(right1 - x1), Value(bottom1 - y1)};
		GetVM().ConstructBuiltinObject(result, "flash.geom.Rectangle", 4, args).DoNotCheck();

//##protect##"instance::Rectangle::intersection()"
    }
    void Rectangle::intersects(bool& result, Instances::fl_geom::Rectangle* toIntersect)
    {
//##protect##"instance::Rectangle::intersects()"
		// If either rectangle is empty, return false;
		if (!toIntersect)
		{
            return (GetVM().ThrowTypeError(VM::Error(VM::eConvertNullToObjectError, GetVM())));
		}
		bool res;
		toIntersect->isEmpty(res);
		if (res)
		{
			result = false;
			return;
		}

		isEmpty(res);
		if (res)
		{
			result = false;
			return;
		}

		Value::Number bottom = y + height;
		Value::Number right = x + width;

		Value::Number x2 = toIntersect->GetX();
		Value::Number y2 = toIntersect->GetY();
		Value::Number bottom2 = toIntersect->GetY() + toIntersect->GetHeight();
		Value::Number right2 = toIntersect->GetX() + toIntersect->GetWidth();


		if ( (bottom >= y2)&&(bottom2 >= y) )
			if ( (right2 >= x)&&(right >= x2) )
			{
				result = true;
				return;
			}
		result = false;

//##protect##"instance::Rectangle::intersects()"
    }
    void Rectangle::isEmpty(bool& result)
    {
//##protect##"instance::Rectangle::isEmpty()"
        if (width <= 0 || height <= 0)
		{
			result = true;
			return;
		}
		result = false;
//##protect##"instance::Rectangle::isEmpty()"
    }
    void Rectangle::offset(const Value& result, Value::Number dx, Value::Number dy)
    {
//##protect##"instance::Rectangle::offset()"
        SF_UNUSED1(result);
		x += dx;
		y += dy;
//##protect##"instance::Rectangle::offset()"
    }
    void Rectangle::offsetPoint(const Value& result, Instances::fl_geom::Point* point)
    {
//##protect##"instance::Rectangle::offsetPoint()"
        SF_UNUSED1(result);
		if (!point)
		{
            return (GetVM().ThrowTypeError(VM::Error(VM::eConvertNullToObjectError, GetVM())));
		}
		x += point->GetX();
		y += point->GetY();
//##protect##"instance::Rectangle::offsetPoint()"
    }
    void Rectangle::setEmpty(const Value& result)
    {
//##protect##"instance::Rectangle::setEmpty()"
        SF_UNUSED1(result);
        // !!! This is not correct implementation!
        bool res;
		isEmpty(res);
// 		result.SetBool(res);
//##protect##"instance::Rectangle::setEmpty()"
    }
    void Rectangle::toString(ASString& result)
    {
//##protect##"instance::Rectangle::toString()"
		result += GetStringManager().CreateConstString("(x=") + GetVM().AsString(Value(x)) + 
			GetStringManager().CreateConstString(", ") + 
			GetStringManager().CreateConstString("y=") + 
			GetVM().AsString(Value(y)) +
			GetStringManager().CreateConstString(", ") + 
			GetStringManager().CreateConstString("w=") + 
			GetVM().AsString(Value(width)) +
			GetStringManager().CreateConstString(", ") + 
			GetStringManager().CreateConstString("h=") + 
			GetVM().AsString(Value(height)) +
			GetStringManager().CreateConstString(")");
//##protect##"instance::Rectangle::toString()"
    }
    void Rectangle::union_(SPtr<Instances::fl_geom::Rectangle>& result, Instances::fl_geom::Rectangle* toUnion)
    {
//##protect##"instance::Rectangle::union_()"
		// if either of the rectangles is empty, return the other rectangle
		if (!toUnion)
		{
            return (GetVM().ThrowTypeError(VM::Error(VM::eConvertNullToObjectError, GetVM())));
		}
		bool res;
		toUnion->isEmpty(res);
		if (res)
		{
			Value args[] = {Value(x), Value(y), Value(width), Value(height)};
			return(GetVM().ConstructBuiltinObject(result, "flash.geom.Rectangle", 4, args).DoNotCheck());
		}

		isEmpty(res);
		if (res)
		{
			Value args[] = {Value(toUnion->GetX()), Value(toUnion->GetY()), Value(toUnion->GetWidth()), Value(toUnion->GetHeight())};
			return(GetVM().ConstructBuiltinObject(result, "flash.geom.Rectangle", 4, args).DoNotCheck());
		}

		Value::Number x1 = x; Value::Number y1 = y; Value::Number right1 = x + width; Value::Number bottom1 = y + height;
		Value::Number x2 = toUnion->GetX(); Value::Number y2 = toUnion->GetY(); 
		Value::Number right2 = x2 + toUnion->GetWidth(); Value::Number bottom2 = y2 + toUnion->GetHeight();

		Value::Number x3,y3, bottom3, right3;

		x3		= x1 > x2? x2:x1;
		right3	= right1 > right2? right1: right2;
		y3		= y1 > y2? y2:y1;
		bottom3	= bottom1 > bottom2? bottom1: bottom2;

		Value args[] = {Value(x3), Value(y3), Value(right3 - x3), Value(bottom3 - y3)};
		GetVM().ConstructBuiltinObject(result, "flash.geom.Rectangle", 4, args).DoNotCheck();
//##protect##"instance::Rectangle::union_()"
    }

    SPtr<Instances::fl_geom::Point> Rectangle::bottomRightGet()
    {
        SPtr<Instances::fl_geom::Point> result;
        bottomRightGet(result);
        return result;
    }
    SPtr<Instances::fl_geom::Point> Rectangle::sizeGet()
    {
        SPtr<Instances::fl_geom::Point> result;
        sizeGet(result);
        return result;
    }
    SPtr<Instances::fl_geom::Point> Rectangle::topLeftGet()
    {
        SPtr<Instances::fl_geom::Point> result;
        topLeftGet(result);
        return result;
    }
//##protect##"instance$methods"
    void Rectangle::AS3Constructor(unsigned argc, const Value* argv)
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
                if (argc > 2)
                {
                    if (!argv[2].Convert2Number(width))
                    {
                        return;
                    }
                    if (argc > 3)
                    {
                        if (!argv[3].Convert2Number(height))
                        {
                            return;
                        }
                    }
                }
            }
        }
    }
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_geom
{
    // const UInt16 Rectangle::tito[Rectangle::ThunkInfoNum] = {
    //    0, 1, 3, 4, 6, 7, 9, 10, 12, 13, 15, 16, 18, 19, 21, 22, 25, 27, 29, 31, 34, 36, 38, 40, 41, 44, 46, 47, 48, 
    // };
    const TypeInfo* Rectangle::tit[50] = {
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl_geom::PointTI, 
        NULL, &AS3::fl_geom::PointTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl_geom::PointTI, 
        NULL, &AS3::fl_geom::PointTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl_geom::PointTI, 
        NULL, &AS3::fl_geom::PointTI, 
        &AS3::fl_geom::RectangleTI, 
        &AS3::fl::BooleanTI, &AS3::fl::NumberTI, &AS3::fl::NumberTI, 
        &AS3::fl::BooleanTI, &AS3::fl_geom::PointTI, 
        &AS3::fl::BooleanTI, &AS3::fl_geom::RectangleTI, 
        &AS3::fl::BooleanTI, &AS3::fl_geom::RectangleTI, 
        NULL, &AS3::fl::NumberTI, &AS3::fl::NumberTI, 
        NULL, &AS3::fl_geom::PointTI, 
        &AS3::fl_geom::RectangleTI, &AS3::fl_geom::RectangleTI, 
        &AS3::fl::BooleanTI, &AS3::fl_geom::RectangleTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::NumberTI, &AS3::fl::NumberTI, 
        NULL, &AS3::fl_geom::PointTI, 
        NULL, 
        &AS3::fl::StringTI, 
        &AS3::fl_geom::RectangleTI, &AS3::fl_geom::RectangleTI, 
    };
    const ThunkInfo Rectangle::ti[Rectangle::ThunkInfoNum] = {
        {TFunc_Instances_Rectangle_bottomGet::Func, &Rectangle::tit[0], "bottom", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Rectangle_bottomSet::Func, &Rectangle::tit[1], "bottom", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Rectangle_bottomRightGet::Func, &Rectangle::tit[3], "bottomRight", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Rectangle_bottomRightSet::Func, &Rectangle::tit[4], "bottomRight", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Rectangle_leftGet::Func, &Rectangle::tit[6], "left", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Rectangle_leftSet::Func, &Rectangle::tit[7], "left", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Rectangle_rightGet::Func, &Rectangle::tit[9], "right", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Rectangle_rightSet::Func, &Rectangle::tit[10], "right", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Rectangle_sizeGet::Func, &Rectangle::tit[12], "size", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Rectangle_sizeSet::Func, &Rectangle::tit[13], "size", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Rectangle_topGet::Func, &Rectangle::tit[15], "top", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Rectangle_topSet::Func, &Rectangle::tit[16], "top", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Rectangle_topLeftGet::Func, &Rectangle::tit[18], "topLeft", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Rectangle_topLeftSet::Func, &Rectangle::tit[19], "topLeft", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Rectangle_clone::Func, &Rectangle::tit[21], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Rectangle_contains::Func, &Rectangle::tit[22], "contains", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Instances_Rectangle_containsPoint::Func, &Rectangle::tit[25], "containsPoint", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Rectangle_containsRect::Func, &Rectangle::tit[27], "containsRect", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Rectangle_equals::Func, &Rectangle::tit[29], "equals", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Rectangle_inflate::Func, &Rectangle::tit[31], "inflate", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Instances_Rectangle_inflatePoint::Func, &Rectangle::tit[34], "inflatePoint", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Rectangle_intersection::Func, &Rectangle::tit[36], "intersection", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Rectangle_intersects::Func, &Rectangle::tit[38], "intersects", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Rectangle_isEmpty::Func, &Rectangle::tit[40], "isEmpty", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Rectangle_offset::Func, &Rectangle::tit[41], "offset", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Instances_Rectangle_offsetPoint::Func, &Rectangle::tit[44], "offsetPoint", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Rectangle_setEmpty::Func, &Rectangle::tit[46], "setEmpty", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Rectangle_toString::Func, &Rectangle::tit[47], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Rectangle_union_::Func, &Rectangle::tit[48], "union", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
    };
    const MemberInfo Rectangle::mi[Rectangle::MemberInfoNum] = {
        {"height", NULL, OFFSETOF(Instances::fl_geom::Rectangle, height), Abc::NS_Public, SlotInfo::BT_Number, 0},
        {"width", NULL, OFFSETOF(Instances::fl_geom::Rectangle, width), Abc::NS_Public, SlotInfo::BT_Number, 0},
        {"x", NULL, OFFSETOF(Instances::fl_geom::Rectangle, x), Abc::NS_Public, SlotInfo::BT_Number, 0},
        {"y", NULL, OFFSETOF(Instances::fl_geom::Rectangle, y), Abc::NS_Public, SlotInfo::BT_Number, 0},
    };


    Rectangle::Rectangle(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::Rectangle::Rectangle()"
//##protect##"InstanceTraits::Rectangle::Rectangle()"

    }

    void Rectangle::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<Rectangle&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_geom
{

    Rectangle::Rectangle(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::Rectangle::Rectangle()"
//##protect##"ClassTraits::Rectangle::Rectangle()"

    }

    Pickable<Traits> Rectangle::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) Rectangle(vm, AS3::fl_geom::RectangleCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_geom::RectangleCI));
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
    const TypeInfo RectangleTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_geom::Rectangle::InstanceType),
        0,
        0,
        InstanceTraits::fl_geom::Rectangle::ThunkInfoNum,
        InstanceTraits::fl_geom::Rectangle::MemberInfoNum,
        "Rectangle", "flash.geom", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo RectangleCI = {
        &RectangleTI,
        ClassTraits::fl_geom::Rectangle::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_geom::Rectangle::ti,
        InstanceTraits::fl_geom::Rectangle::mi,
    };
}; // namespace fl_geom


}}} // namespace Scaleform { namespace GFx { namespace AS3

