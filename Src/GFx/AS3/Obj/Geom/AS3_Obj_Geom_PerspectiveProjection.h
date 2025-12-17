//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Geom_PerspectiveProjection.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Geom_PerspectiveProjection_H
#define INC_AS3_Obj_Geom_PerspectiveProjection_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
#include "GFx/GFx_DisplayObject.h"

#define DEFAULT_STAGE_WIDTH 500.f
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_geom
{
    extern const TypeInfo PerspectiveProjectionTI;
    extern const ClassInfo PerspectiveProjectionCI;
    extern const TypeInfo PointTI;
    extern const ClassInfo PointCI;
    extern const TypeInfo Matrix3DTI;
    extern const ClassInfo Matrix3DCI;
} // namespace fl_geom
namespace fl
{
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
} // namespace fl

namespace ClassTraits { namespace fl_geom
{
    class PerspectiveProjection;
}}

namespace InstanceTraits { namespace fl_geom
{
    class PerspectiveProjection;
}}

namespace Classes { namespace fl_geom
{
    class PerspectiveProjection;
}}

//##protect##"forward_declaration"
	namespace Instances
	{
        namespace fl_geom
        {
            class Point;
            class Matrix3D;
        }
	}
//##protect##"forward_declaration"

namespace Instances { namespace fl_geom
{
    class PerspectiveProjection : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_geom::PerspectiveProjection;
        
    public:
        typedef PerspectiveProjection SelfType;
        typedef Classes::fl_geom::PerspectiveProjection ClassType;
        typedef InstanceTraits::fl_geom::PerspectiveProjection TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_geom::PerspectiveProjectionTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_geom::PerspectiveProjection"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        PerspectiveProjection(InstanceTraits::Traits& t);

//##protect##"instance$methods"
		virtual void AS3Constructor(unsigned argc, const Value* argv); 
    public:
        Double CalculateFocalLength(float stageWidth);
        Double CalculateFOV(float stageWidth);
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_projectionCenterGet, 
            mid_projectionCenterSet, 
            mid_focalLengthGet, 
            mid_focalLengthSet, 
            mid_fieldOfViewGet, 
            mid_fieldOfViewSet, 
            mid_toMatrix3D, 
        };
        void projectionCenterGet(SPtr<Instances::fl_geom::Point>& result);
        void projectionCenterSet(const Value& result, Instances::fl_geom::Point* value);
        void focalLengthGet(Value::Number& result);
        void focalLengthSet(const Value& result, Value::Number value);
        void fieldOfViewGet(Value::Number& result);
        void fieldOfViewSet(const Value& result, Value::Number value);
        void toMatrix3D(SPtr<Instances::fl_geom::Matrix3D>& result);

        // C++ friendly wrappers for AS3 methods.
        SPtr<Instances::fl_geom::Point> projectionCenterGet();
        void projectionCenterSet(Instances::fl_geom::Point* value)
        {
            projectionCenterSet(Value::GetUndefined(), value);
        }
        Value::Number focalLengthGet()
        {
            Value::Number result;
            focalLengthGet(result);
            return result;
        }
        void focalLengthSet(Value::Number value)
        {
            focalLengthSet(Value::GetUndefined(), value);
        }
        Value::Number fieldOfViewGet()
        {
            Value::Number result;
            fieldOfViewGet(result);
            return result;
        }
        void fieldOfViewSet(Value::Number value)
        {
            fieldOfViewSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_geom::Matrix3D> toMatrix3D();

//##protect##"instance$data"
        PointF projectionCenter;
        Double focalLength;
        Double fieldOfView;
    public:
        GFx::DisplayObject* pDispObj;
        PointF &GetProjectionCenter()   { return projectionCenter; }

        Double GetFOV() const           { return fieldOfView; }
        void SetFOV(Double f)           { fieldOfView = f; }

        Double GetFocalLength() const   { return focalLength; }
        void SetFocalLength(Double f)   { focalLength = f; }

//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_geom
{
    class PerspectiveProjection : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::PerspectiveProjection"; }
#endif
    public:
        typedef Instances::fl_geom::PerspectiveProjection InstanceType;

    public:
        PerspectiveProjection(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_geom::PerspectiveProjection> MakeInstance(PerspectiveProjection& t)
        {
            return Pickable<Instances::fl_geom::PerspectiveProjection>(new(t.Alloc()) Instances::fl_geom::PerspectiveProjection(t));
        }
        SPtr<Instances::fl_geom::PerspectiveProjection> MakeInstanceS(PerspectiveProjection& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 7 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[10];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_geom
{
    class PerspectiveProjection : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::PerspectiveProjection"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_geom::PerspectiveProjection InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        PerspectiveProjection(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

