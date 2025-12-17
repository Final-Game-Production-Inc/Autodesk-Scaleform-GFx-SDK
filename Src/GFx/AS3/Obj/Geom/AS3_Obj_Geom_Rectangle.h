//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Geom_Rectangle.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Geom_Rectangle_H
#define INC_AS3_Obj_Geom_Rectangle_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_geom
{
    extern const TypeInfo RectangleTI;
    extern const ClassInfo RectangleCI;
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
    class Rectangle;
}}

namespace InstanceTraits { namespace fl_geom
{
    class Rectangle;
}}

namespace Classes { namespace fl_geom
{
    class Rectangle;
}}

//##protect##"forward_declaration"
namespace Instances
{
    namespace fl_geom
    {
        class Point;
    }
}
//##protect##"forward_declaration"

namespace Instances { namespace fl_geom
{
    class Rectangle : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_geom::Rectangle;
        
    public:
        typedef Rectangle SelfType;
        typedef Classes::fl_geom::Rectangle ClassType;
        typedef InstanceTraits::fl_geom::Rectangle TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_geom::RectangleTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_geom::Rectangle"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        Rectangle(InstanceTraits::Traits& t);

//##protect##"instance$methods"
		virtual void AS3Constructor(unsigned argc, const Value* argv);
    public:
        Value::Number GetX() const { return x; }
        Value::Number GetY() const { return y; }
        Value::Number GetWidth() const { return width; }
        Value::Number GetHeight() const { return height; }
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_bottomGet, 
            mid_bottomSet, 
            mid_bottomRightGet, 
            mid_bottomRightSet, 
            mid_leftGet, 
            mid_leftSet, 
            mid_rightGet, 
            mid_rightSet, 
            mid_sizeGet, 
            mid_sizeSet, 
            mid_topGet, 
            mid_topSet, 
            mid_topLeftGet, 
            mid_topLeftSet, 
            mid_clone, 
            mid_contains, 
            mid_containsPoint, 
            mid_containsRect, 
            mid_equals, 
            mid_inflate, 
            mid_inflatePoint, 
            mid_intersection, 
            mid_intersects, 
            mid_isEmpty, 
            mid_offset, 
            mid_offsetPoint, 
            mid_setEmpty, 
            mid_toString, 
            mid_union_, 
        };
        void bottomGet(Value::Number& result);
        void bottomSet(const Value& result, Value::Number value);
        void bottomRightGet(SPtr<Instances::fl_geom::Point>& result);
        void bottomRightSet(const Value& result, Instances::fl_geom::Point* value);
        void leftGet(Value::Number& result);
        void leftSet(const Value& result, Value::Number value);
        void rightGet(Value::Number& result);
        void rightSet(const Value& result, Value::Number value);
        void sizeGet(SPtr<Instances::fl_geom::Point>& result);
        void sizeSet(const Value& result, Instances::fl_geom::Point* value);
        void topGet(Value::Number& result);
        void topSet(const Value& result, Value::Number value);
        void topLeftGet(SPtr<Instances::fl_geom::Point>& result);
        void topLeftSet(const Value& result, Instances::fl_geom::Point* value);
        void clone(SPtr<Instances::fl_geom::Rectangle>& result);
        void contains(bool& result, Value::Number x, Value::Number y);
        void containsPoint(bool& result, Instances::fl_geom::Point* point);
        void containsRect(bool& result, Instances::fl_geom::Rectangle* rect);
        void equals(bool& result, Instances::fl_geom::Rectangle* toCompare);
        void inflate(const Value& result, Value::Number dx, Value::Number dy);
        void inflatePoint(const Value& result, Instances::fl_geom::Point* point);
        void intersection(SPtr<Instances::fl_geom::Rectangle>& result, Instances::fl_geom::Rectangle* toIntersect);
        void intersects(bool& result, Instances::fl_geom::Rectangle* toIntersect);
        void isEmpty(bool& result);
        void offset(const Value& result, Value::Number dx, Value::Number dy);
        void offsetPoint(const Value& result, Instances::fl_geom::Point* point);
        void setEmpty(const Value& result);
        void toString(ASString& result);
        void union_(SPtr<Instances::fl_geom::Rectangle>& result, Instances::fl_geom::Rectangle* toUnion);

        // C++ friendly wrappers for AS3 methods.
        Value::Number bottomGet()
        {
            Value::Number result;
            bottomGet(result);
            return result;
        }
        void bottomSet(Value::Number value)
        {
            bottomSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_geom::Point> bottomRightGet();
        void bottomRightSet(Instances::fl_geom::Point* value)
        {
            bottomRightSet(Value::GetUndefined(), value);
        }
        Value::Number leftGet()
        {
            Value::Number result;
            leftGet(result);
            return result;
        }
        void leftSet(Value::Number value)
        {
            leftSet(Value::GetUndefined(), value);
        }
        Value::Number rightGet()
        {
            Value::Number result;
            rightGet(result);
            return result;
        }
        void rightSet(Value::Number value)
        {
            rightSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_geom::Point> sizeGet();
        void sizeSet(Instances::fl_geom::Point* value)
        {
            sizeSet(Value::GetUndefined(), value);
        }
        Value::Number topGet()
        {
            Value::Number result;
            topGet(result);
            return result;
        }
        void topSet(Value::Number value)
        {
            topSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_geom::Point> topLeftGet();
        void topLeftSet(Instances::fl_geom::Point* value)
        {
            topLeftSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_geom::Rectangle> clone()
        {
            SPtr<Instances::fl_geom::Rectangle> result;
            clone(result);
            return result;
        }
        bool contains(Value::Number vnx, Value::Number vny)
        {
            bool result;
            contains(result, vnx, vny);
            return result;
        }
        bool containsPoint(Instances::fl_geom::Point* point)
        {
            bool result;
            containsPoint(result, point);
            return result;
        }
        bool containsRect(Instances::fl_geom::Rectangle* rect)
        {
            bool result;
            containsRect(result, rect);
            return result;
        }
        bool equals(Instances::fl_geom::Rectangle* toCompare)
        {
            bool result;
            equals(result, toCompare);
            return result;
        }
        void inflate(Value::Number dx, Value::Number dy)
        {
            inflate(Value::GetUndefined(), dx, dy);
        }
        void inflatePoint(Instances::fl_geom::Point* point)
        {
            inflatePoint(Value::GetUndefined(), point);
        }
        SPtr<Instances::fl_geom::Rectangle> intersection(Instances::fl_geom::Rectangle* toIntersect)
        {
            SPtr<Instances::fl_geom::Rectangle> result;
            intersection(result, toIntersect);
            return result;
        }
        bool intersects(Instances::fl_geom::Rectangle* toIntersect)
        {
            bool result;
            intersects(result, toIntersect);
            return result;
        }
        bool isEmpty()
        {
            bool result;
            isEmpty(result);
            return result;
        }
        void offset(Value::Number dx, Value::Number dy)
        {
            offset(Value::GetUndefined(), dx, dy);
        }
        void offsetPoint(Instances::fl_geom::Point* point)
        {
            offsetPoint(Value::GetUndefined(), point);
        }
        void setEmpty()
        {
            setEmpty(Value::GetUndefined());
        }
        ASString toString()
        {
            ASString result(GetStringManager().CreateEmptyString());
            toString(result);
            return result;
        }
        SPtr<Instances::fl_geom::Rectangle> union_(Instances::fl_geom::Rectangle* toUnion)
        {
            SPtr<Instances::fl_geom::Rectangle> result;
            union_(result, toUnion);
            return result;
        }

    public:
        // AS3 API members.
        Value::Number height;
        Value::Number width;
        Value::Number x;
        Value::Number y;

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_geom
{
    class Rectangle : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::Rectangle"; }
#endif
    public:
        typedef Instances::fl_geom::Rectangle InstanceType;

    public:
        Rectangle(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_geom::Rectangle> MakeInstance(Rectangle& t)
        {
            return Pickable<Instances::fl_geom::Rectangle>(new(t.Alloc()) Instances::fl_geom::Rectangle(t));
        }
        SPtr<Instances::fl_geom::Rectangle> MakeInstanceS(Rectangle& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { MemberInfoNum = 4 };
        static const MemberInfo mi[MemberInfoNum];
        enum { ThunkInfoNum = 29 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[50];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_geom
{
    class Rectangle : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Rectangle"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_geom::Rectangle InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Rectangle(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

