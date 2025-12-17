//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Geom_Matrix.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Geom_Matrix_H
#define INC_AS3_Obj_Geom_Matrix_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_geom
{
    extern const TypeInfo MatrixTI;
    extern const ClassInfo MatrixCI;
    extern const TypeInfo PointTI;
    extern const ClassInfo PointCI;
} // namespace fl_geom
namespace fl
{
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl

namespace ClassTraits { namespace fl_geom
{
    class Matrix;
}}

namespace InstanceTraits { namespace fl_geom
{
    class Matrix;
}}

namespace Classes { namespace fl_geom
{
    class Matrix;
}}

//##protect##"forward_declaration"
typedef Scaleform::Render::Matrix2x4<Double> Matrix2DDouble;
typedef Scaleform::Render::Matrix2F          Matrix2DFloat;
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
    class Matrix : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_geom::Matrix;
        
    public:
        typedef Matrix SelfType;
        typedef Classes::fl_geom::Matrix ClassType;
        typedef InstanceTraits::fl_geom::Matrix TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_geom::MatrixTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_geom::Matrix"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        Matrix(InstanceTraits::Traits& t);

//##protect##"instance$methods"
		virtual void AS3Constructor(unsigned argc, const Value* argv);
    public:
        void SetMatrix(const Matrix2DDouble& m);
        Matrix2DDouble GetMatrix() const;
        Matrix2DFloat  GetMatrixF() const;
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_clone, 
            mid_concat, 
            mid_createBox, 
            mid_createGradientBox, 
            mid_deltaTransformPoint, 
            mid_identity, 
            mid_invert, 
            mid_rotate, 
            mid_scale, 
            mid_toString, 
            mid_transformPoint, 
            mid_translate, 
        };
        void clone(SPtr<Instances::fl_geom::Matrix>& result);
        void concat(const Value& result, Instances::fl_geom::Matrix* m);
        void createBox(const Value& result, Value::Number scaleX, Value::Number scaleY, Value::Number rotation = 0.0, Value::Number tx = 0.0, Value::Number ty = 0.0);
        void createGradientBox(const Value& result, Value::Number width, Value::Number height, Value::Number rotation = 0.0, Value::Number tx = 0.0, Value::Number ty = 0.0);
        void deltaTransformPoint(SPtr<Instances::fl_geom::Point>& result, Instances::fl_geom::Point* point);
        void identity(const Value& result);
        void invert(const Value& result);
        void rotate(const Value& result, Value::Number angle);
        void scale(const Value& result, Value::Number sx, Value::Number sy);
        void toString(ASString& result);
        void transformPoint(SPtr<Instances::fl_geom::Point>& result, Instances::fl_geom::Point* point);
        void translate(const Value& result, Value::Number dx, Value::Number dy);

        // C++ friendly wrappers for AS3 methods.
        SPtr<Instances::fl_geom::Matrix> clone()
        {
            SPtr<Instances::fl_geom::Matrix> result;
            clone(result);
            return result;
        }
        void concat(Instances::fl_geom::Matrix* m)
        {
            concat(Value::GetUndefined(), m);
        }
        void createBox(Value::Number scaleX, Value::Number scaleY, Value::Number rotation = 0.0, Value::Number ttx = 0.0, Value::Number tty = 0.0)
        {
            createBox(Value::GetUndefined(), scaleX, scaleY, rotation, ttx, tty);
        }
        void createGradientBox(Value::Number width, Value::Number height, Value::Number rotation = 0.0, Value::Number ttx = 0.0, Value::Number tty = 0.0)
        {
            createGradientBox(Value::GetUndefined(), width, height, rotation, ttx, tty);
        }
        SPtr<Instances::fl_geom::Point> deltaTransformPoint(Instances::fl_geom::Point* point);
        void identity()
        {
            identity(Value::GetUndefined());
        }
        void invert()
        {
            invert(Value::GetUndefined());
        }
        void rotate(Value::Number angle)
        {
            rotate(Value::GetUndefined(), angle);
        }
        void scale(Value::Number sx, Value::Number sy)
        {
            scale(Value::GetUndefined(), sx, sy);
        }
        ASString toString()
        {
            ASString result(GetStringManager().CreateEmptyString());
            toString(result);
            return result;
        }
        SPtr<Instances::fl_geom::Point> transformPoint(Instances::fl_geom::Point* point);
        void translate(Value::Number dx, Value::Number dy)
        {
            translate(Value::GetUndefined(), dx, dy);
        }

    public:
        // AS3 API members.
        Value::Number a;
        Value::Number b;
        Value::Number c;
        Value::Number d;
        Value::Number tx;
        Value::Number ty;

//##protect##"instance$data"
    public:
        const static float GradientBoxMagicNumber;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_geom
{
    class Matrix : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::Matrix"; }
#endif
    public:
        typedef Instances::fl_geom::Matrix InstanceType;

    public:
        Matrix(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_geom::Matrix> MakeInstance(Matrix& t)
        {
            return Pickable<Instances::fl_geom::Matrix>(new(t.Alloc()) Instances::fl_geom::Matrix(t));
        }
        SPtr<Instances::fl_geom::Matrix> MakeInstanceS(Matrix& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { MemberInfoNum = 6 };
        static const MemberInfo mi[MemberInfoNum];
        enum { ThunkInfoNum = 12 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[30];
        static const Abc::ConstValue dva[6];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_geom
{
    class Matrix : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Matrix"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_geom::Matrix InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Matrix(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

