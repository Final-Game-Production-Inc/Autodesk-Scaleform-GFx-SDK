//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Geom_Matrix3D.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Geom_Matrix3D_H
#define INC_AS3_Obj_Geom_Matrix3D_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
#include "GFx/GFx_DisplayObject.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_geom
{
    extern const TypeInfo Matrix3DTI;
    extern const ClassInfo Matrix3DCI;
    extern const TypeInfo Vector3DTI;
    extern const ClassInfo Vector3DCI;
} // namespace fl_geom
namespace fl
{
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
} // namespace fl
namespace fl_vec
{
    extern const TypeInfo Vector_doubleTI;
    extern const ClassInfo Vector_doubleCI;
    extern const TypeInfo Vector_objectTI;
    extern const ClassInfo Vector_objectCI;
} // namespace fl_vec

namespace ClassTraits { namespace fl_geom
{
    class Matrix3D;
}}

namespace InstanceTraits { namespace fl_geom
{
    class Matrix3D;
}}

namespace Classes { namespace fl_geom
{
    class Matrix3D;
}}

//##protect##"forward_declaration"
    typedef Scaleform::Render::Matrix4x4<Double> Matrix4DDouble;
    namespace Instances
    {
        namespace fl_vec
        {
            class Vector_double;
            class Vector_object;
        }
        namespace fl_geom
        {
            class Vector3D;
        }
    }
//##protect##"forward_declaration"

namespace Instances { namespace fl_geom
{
    class Matrix3D : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_geom::Matrix3D;
        
    public:
        typedef Matrix3D SelfType;
        typedef Classes::fl_geom::Matrix3D ClassType;
        typedef InstanceTraits::fl_geom::Matrix3D TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_geom::Matrix3DTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_geom::Matrix3D"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        Matrix3D(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        virtual void AS3Constructor(unsigned argc, const Value* argv);
    public:
        void SetMatrix3D(const Matrix4DDouble& m)   { mat4 = m; }
        const Matrix4DDouble &GetMatrix3D() const   { return mat4; }
        Matrix4DDouble &GetMatrix3D()               { return mat4; }
        Matrix4F  GetMatrix3DF() const;
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_determinantGet, 
            mid_positionGet, 
            mid_positionSet, 
            mid_rawDataGet, 
            mid_rawDataSet, 
            mid_append, 
            mid_appendRotation, 
            mid_appendScale, 
            mid_appendTranslation, 
            mid_clone, 
            mid_decompose, 
            mid_deltaTransformVector, 
            mid_identity, 
            mid_interpolate, 
            mid_interpolateTo, 
            mid_invert, 
            mid_pointAt, 
            mid_prepend, 
            mid_prependRotation, 
            mid_prependScale, 
            mid_prependTranslation, 
            mid_recompose, 
            mid_transformVector, 
            mid_transformVectors, 
            mid_transpose, 
        };
        void determinantGet(Value::Number& result);
        void positionGet(SPtr<Instances::fl_geom::Vector3D>& result);
        void positionSet(const Value& result, Instances::fl_geom::Vector3D* value);
        void rawDataGet(SPtr<Instances::fl_vec::Vector_double>& result);
        void rawDataSet(const Value& result, Instances::fl_vec::Vector_double* value);
        void append(const Value& result, Instances::fl_geom::Matrix3D* lhs);
        void appendRotation(const Value& result, Value::Number degrees, Instances::fl_geom::Vector3D* axis, Instances::fl_geom::Vector3D* pivotPoint = NULL);
        void appendScale(const Value& result, Value::Number xScale, Value::Number yScale, Value::Number zScale);
        void appendTranslation(const Value& result, Value::Number x, Value::Number y, Value::Number z);
        void clone(SPtr<Instances::fl_geom::Matrix3D>& result);
        void decompose(SPtr<Instances::fl_vec::Vector_object>& result, const ASString& orientationStyle);
        void deltaTransformVector(SPtr<Instances::fl_geom::Vector3D>& result, Instances::fl_geom::Vector3D* v);
        void identity(const Value& result);
        void interpolate(SPtr<Instances::fl_geom::Matrix3D>& result, Instances::fl_geom::Matrix3D* thisMat, Instances::fl_geom::Matrix3D* toMat, Value::Number percent);
        void interpolateTo(const Value& result, Instances::fl_geom::Matrix3D* toMat, Value::Number percent);
        void invert(bool& result);
        void pointAt(const Value& result, Instances::fl_geom::Vector3D* pos, Instances::fl_geom::Vector3D* at = NULL, Instances::fl_geom::Vector3D* up = NULL);
        void prepend(const Value& result, Instances::fl_geom::Matrix3D* rhs);
        void prependRotation(const Value& result, Value::Number degrees, Instances::fl_geom::Vector3D* axis, Instances::fl_geom::Vector3D* pivotPoint = NULL);
        void prependScale(const Value& result, Value::Number xScale, Value::Number yScale, Value::Number zScale);
        void prependTranslation(const Value& result, Value::Number x, Value::Number y, Value::Number z);
        void recompose(bool& result, Instances::fl_vec::Vector_object* components, const ASString& orientationStyle);
        void transformVector(SPtr<Instances::fl_geom::Vector3D>& result, Instances::fl_geom::Vector3D* v);
        void transformVectors(const Value& result, Instances::fl_vec::Vector_double* vin, Instances::fl_vec::Vector_double* vout);
        void transpose(const Value& result);

        // C++ friendly wrappers for AS3 methods.
        Value::Number determinantGet()
        {
            Value::Number result;
            determinantGet(result);
            return result;
        }
        SPtr<Instances::fl_geom::Vector3D> positionGet();
        void positionSet(Instances::fl_geom::Vector3D* value)
        {
            positionSet(Value::GetUndefined(), value);
        }
        SPtr<Instances::fl_vec::Vector_double> rawDataGet();
        void rawDataSet(Instances::fl_vec::Vector_double* value)
        {
            rawDataSet(Value::GetUndefined(), value);
        }
        void append(Instances::fl_geom::Matrix3D* lhs)
        {
            append(Value::GetUndefined(), lhs);
        }
        void appendRotation(Value::Number degrees, Instances::fl_geom::Vector3D* axis, Instances::fl_geom::Vector3D* pivotPoint = NULL)
        {
            appendRotation(Value::GetUndefined(), degrees, axis, pivotPoint);
        }
        void appendScale(Value::Number xScale, Value::Number yScale, Value::Number zScale)
        {
            appendScale(Value::GetUndefined(), xScale, yScale, zScale);
        }
        void appendTranslation(Value::Number x, Value::Number y, Value::Number z)
        {
            appendTranslation(Value::GetUndefined(), x, y, z);
        }
        SPtr<Instances::fl_geom::Matrix3D> clone()
        {
            SPtr<Instances::fl_geom::Matrix3D> result;
            clone(result);
            return result;
        }
        SPtr<Instances::fl_vec::Vector_object> decompose(const ASString& orientationStyle);
        SPtr<Instances::fl_geom::Vector3D> deltaTransformVector(Instances::fl_geom::Vector3D* v);
        void identity()
        {
            identity(Value::GetUndefined());
        }
        SPtr<Instances::fl_geom::Matrix3D> interpolate(Instances::fl_geom::Matrix3D* thisMat, Instances::fl_geom::Matrix3D* toMat, Value::Number percent)
        {
            SPtr<Instances::fl_geom::Matrix3D> result;
            interpolate(result, thisMat, toMat, percent);
            return result;
        }
        void interpolateTo(Instances::fl_geom::Matrix3D* toMat, Value::Number percent)
        {
            interpolateTo(Value::GetUndefined(), toMat, percent);
        }
        bool invert()
        {
            bool result;
            invert(result);
            return result;
        }
        void pointAt(Instances::fl_geom::Vector3D* pos, Instances::fl_geom::Vector3D* at = NULL, Instances::fl_geom::Vector3D* up = NULL)
        {
            pointAt(Value::GetUndefined(), pos, at, up);
        }
        void prepend(Instances::fl_geom::Matrix3D* rhs)
        {
            prepend(Value::GetUndefined(), rhs);
        }
        void prependRotation(Value::Number degrees, Instances::fl_geom::Vector3D* axis, Instances::fl_geom::Vector3D* pivotPoint = NULL)
        {
            prependRotation(Value::GetUndefined(), degrees, axis, pivotPoint);
        }
        void prependScale(Value::Number xScale, Value::Number yScale, Value::Number zScale)
        {
            prependScale(Value::GetUndefined(), xScale, yScale, zScale);
        }
        void prependTranslation(Value::Number x, Value::Number y, Value::Number z)
        {
            prependTranslation(Value::GetUndefined(), x, y, z);
        }
        bool recompose(Instances::fl_vec::Vector_object* components, const ASString& orientationStyle)
        {
            bool result;
            recompose(result, components, orientationStyle);
            return result;
        }
        SPtr<Instances::fl_geom::Vector3D> transformVector(Instances::fl_geom::Vector3D* v);
        void transformVectors(Instances::fl_vec::Vector_double* vin, Instances::fl_vec::Vector_double* vout)
        {
            transformVectors(Value::GetUndefined(), vin, vout);
        }
        void transpose()
        {
            transpose(Value::GetUndefined());
        }

//##protect##"instance$data"
        Matrix4DDouble mat4;    // kept in GFx format, which is transposed from Flash
        GFx::DisplayObject* pDispObj;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_geom
{
    class Matrix3D : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::Matrix3D"; }
#endif
    public:
        typedef Instances::fl_geom::Matrix3D InstanceType;

    public:
        Matrix3D(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_geom::Matrix3D> MakeInstance(Matrix3D& t)
        {
            return Pickable<Instances::fl_geom::Matrix3D>(new(t.Alloc()) Instances::fl_geom::Matrix3D(t));
        }
        SPtr<Instances::fl_geom::Matrix3D> MakeInstanceS(Matrix3D& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 25 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[62];
        static const Abc::ConstValue dva[2];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_geom
{
    class Matrix3D : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Matrix3D"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_geom::Matrix3D InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Matrix3D(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

