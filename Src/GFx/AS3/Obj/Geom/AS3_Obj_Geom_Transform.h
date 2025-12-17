//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Geom_Transform.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Geom_Transform_H
#define INC_AS3_Obj_Geom_Transform_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
#include "GFx/GFx_DisplayObject.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_geom
{
    extern const TypeInfo TransformTI;
    extern const ClassInfo TransformCI;
    extern const TypeInfo ColorTransformTI;
    extern const ClassInfo ColorTransformCI;
    extern const TypeInfo MatrixTI;
    extern const ClassInfo MatrixCI;
    extern const TypeInfo Matrix3DTI;
    extern const ClassInfo Matrix3DCI;
    extern const TypeInfo PerspectiveProjectionTI;
    extern const ClassInfo PerspectiveProjectionCI;
    extern const TypeInfo RectangleTI;
    extern const ClassInfo RectangleCI;
} // namespace fl_geom
namespace fl_display
{
    extern const TypeInfo DisplayObjectTI;
    extern const ClassInfo DisplayObjectCI;
} // namespace fl_display

namespace ClassTraits { namespace fl_geom
{
    class Transform;
}}

namespace InstanceTraits { namespace fl_geom
{
    class Transform;
}}

namespace Classes { namespace fl_geom
{
    class Transform;
}}

//##protect##"forward_declaration"
namespace Instances { namespace fl_geom
{
    class Rectangle;
    class Matrix;
    class Matrix3D;
    class Vector3D;
    class PerspectiveProjection;
    class ColorTransform;
}}

namespace Instances { namespace fl_display
{
    class DisplayObject;
}}
//##protect##"forward_declaration"

namespace Instances { namespace fl_geom
{
    class Transform : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_geom::Transform;
        
    public:
        typedef Transform SelfType;
        typedef Classes::fl_geom::Transform ClassType;
        typedef InstanceTraits::fl_geom::Transform TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_geom::TransformTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_geom::Transform"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        Transform(InstanceTraits::Traits& t);

//##protect##"instance$methods"
		virtual void AS3Constructor(unsigned argc, const Value* argv);
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_colorTransformGet, 
            mid_colorTransformSet, 
            mid_concatenatedColorTransformGet, 
            mid_concatenatedMatrixGet, 
            mid_matrixGet, 
            mid_matrixSet, 
            mid_matrix3DGet, 
            mid_matrix3DSet, 
            mid_perspectiveProjectionGet, 
            mid_perspectiveProjectionSet, 
            mid_pixelBoundsGet, 
            mid_getRelativeMatrix3D, 
        };
        void colorTransformGet(SPtr<Instances::fl_geom::ColorTransform>& result);
        void colorTransformSet(const Value& result, Instances::fl_geom::ColorTransform* value);
        void concatenatedColorTransformGet(SPtr<Instances::fl_geom::ColorTransform>& result);
        void concatenatedMatrixGet(SPtr<Instances::fl_geom::Matrix>& result);
        void matrixGet(SPtr<Instances::fl_geom::Matrix>& result);
        void matrixSet(const Value& result, Instances::fl_geom::Matrix* value);
        void matrix3DGet(SPtr<Instances::fl_geom::Matrix3D>& result);
        void matrix3DSet(const Value& result, Instances::fl_geom::Matrix3D* value);
        void perspectiveProjectionGet(SPtr<Instances::fl_geom::PerspectiveProjection>& result);
        void perspectiveProjectionSet(const Value& result, Instances::fl_geom::PerspectiveProjection* value);
        void pixelBoundsGet(SPtr<Instances::fl_geom::Rectangle>& result);
        void getRelativeMatrix3D(SPtr<Instances::fl_geom::Matrix3D>& result, Instances::fl_display::DisplayObject* relativeTo);

        // C++ friendly wrappers for AS3 methods.
        SPtr<Instances::fl_geom::ColorTransform> colorTransformGet();
        void colorTransformSet(Instances::fl_geom::ColorTransform* value)
        {
            colorTransformSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_geom::ColorTransform> concatenatedColorTransformGet();
        SPtr<Instances::fl_geom::Matrix> concatenatedMatrixGet();
        SPtr<Instances::fl_geom::Matrix> matrixGet();
        void matrixSet(Instances::fl_geom::Matrix* value)
        {
            matrixSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_geom::Matrix3D> matrix3DGet();
        void matrix3DSet(Instances::fl_geom::Matrix3D* value)
        {
            matrix3DSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_geom::PerspectiveProjection> perspectiveProjectionGet();
        void perspectiveProjectionSet(Instances::fl_geom::PerspectiveProjection* value)
        {
            perspectiveProjectionSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_geom::Rectangle> pixelBoundsGet();
        SPtr<Instances::fl_geom::Matrix3D> getRelativeMatrix3D(Instances::fl_display::DisplayObject* relativeTo);

//##protect##"instance$data"
	public:
		GFx::DisplayObject* pDispObj;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_geom
{
    class Transform : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::Transform"; }
#endif
    public:
        typedef Instances::fl_geom::Transform InstanceType;

    public:
        Transform(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_geom::Transform> MakeInstance(Transform& t)
        {
            return Pickable<Instances::fl_geom::Transform>(new(t.Alloc()) Instances::fl_geom::Transform(t));
        }
        SPtr<Instances::fl_geom::Transform> MakeInstanceS(Transform& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 12 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[17];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_geom
{
    class Transform : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Transform"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_geom::Transform InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Transform(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

