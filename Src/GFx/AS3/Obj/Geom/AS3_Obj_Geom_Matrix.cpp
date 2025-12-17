//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Geom_Matrix.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Geom_Matrix.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "AS3_Obj_Geom_Point.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

#ifndef SF_AS3_EMIT_DEF_ARGS
// Values of default arguments.
namespace Impl
{

    template <>
    SF_INLINE
    Value::Number GetMethodDefArg<Instances::fl_geom::Matrix, Instances::fl_geom::Matrix::mid_createBox, 2, Value::Number>(AS3::StringManager&)
    {
        return 0.0;
    }

    template <>
    SF_INLINE
    Value::Number GetMethodDefArg<Instances::fl_geom::Matrix, Instances::fl_geom::Matrix::mid_createBox, 3, Value::Number>(AS3::StringManager&)
    {
        return 0.0;
    }

    template <>
    SF_INLINE
    Value::Number GetMethodDefArg<Instances::fl_geom::Matrix, Instances::fl_geom::Matrix::mid_createBox, 4, Value::Number>(AS3::StringManager&)
    {
        return 0.0;
    }

    template <>
    SF_INLINE
    Value::Number GetMethodDefArg<Instances::fl_geom::Matrix, Instances::fl_geom::Matrix::mid_createGradientBox, 2, Value::Number>(AS3::StringManager&)
    {
        return 0.0;
    }

    template <>
    SF_INLINE
    Value::Number GetMethodDefArg<Instances::fl_geom::Matrix, Instances::fl_geom::Matrix::mid_createGradientBox, 3, Value::Number>(AS3::StringManager&)
    {
        return 0.0;
    }

    template <>
    SF_INLINE
    Value::Number GetMethodDefArg<Instances::fl_geom::Matrix, Instances::fl_geom::Matrix::mid_createGradientBox, 4, Value::Number>(AS3::StringManager&)
    {
        return 0.0;
    }

} // namespace Impl
#endif // SF_AS3_EMIT_DEF_ARGS

typedef ThunkFunc0<Instances::fl_geom::Matrix, Instances::fl_geom::Matrix::mid_clone, SPtr<Instances::fl_geom::Matrix> > TFunc_Instances_Matrix_clone;
typedef ThunkFunc1<Instances::fl_geom::Matrix, Instances::fl_geom::Matrix::mid_concat, const Value, Instances::fl_geom::Matrix*> TFunc_Instances_Matrix_concat;
typedef ThunkFunc5<Instances::fl_geom::Matrix, Instances::fl_geom::Matrix::mid_createBox, const Value, Value::Number, Value::Number, Value::Number, Value::Number, Value::Number> TFunc_Instances_Matrix_createBox;
typedef ThunkFunc5<Instances::fl_geom::Matrix, Instances::fl_geom::Matrix::mid_createGradientBox, const Value, Value::Number, Value::Number, Value::Number, Value::Number, Value::Number> TFunc_Instances_Matrix_createGradientBox;
typedef ThunkFunc1<Instances::fl_geom::Matrix, Instances::fl_geom::Matrix::mid_deltaTransformPoint, SPtr<Instances::fl_geom::Point>, Instances::fl_geom::Point*> TFunc_Instances_Matrix_deltaTransformPoint;
typedef ThunkFunc0<Instances::fl_geom::Matrix, Instances::fl_geom::Matrix::mid_identity, const Value> TFunc_Instances_Matrix_identity;
typedef ThunkFunc0<Instances::fl_geom::Matrix, Instances::fl_geom::Matrix::mid_invert, const Value> TFunc_Instances_Matrix_invert;
typedef ThunkFunc1<Instances::fl_geom::Matrix, Instances::fl_geom::Matrix::mid_rotate, const Value, Value::Number> TFunc_Instances_Matrix_rotate;
typedef ThunkFunc2<Instances::fl_geom::Matrix, Instances::fl_geom::Matrix::mid_scale, const Value, Value::Number, Value::Number> TFunc_Instances_Matrix_scale;
typedef ThunkFunc0<Instances::fl_geom::Matrix, Instances::fl_geom::Matrix::mid_toString, ASString> TFunc_Instances_Matrix_toString;
typedef ThunkFunc1<Instances::fl_geom::Matrix, Instances::fl_geom::Matrix::mid_transformPoint, SPtr<Instances::fl_geom::Point>, Instances::fl_geom::Point*> TFunc_Instances_Matrix_transformPoint;
typedef ThunkFunc2<Instances::fl_geom::Matrix, Instances::fl_geom::Matrix::mid_translate, const Value, Value::Number, Value::Number> TFunc_Instances_Matrix_translate;

template <> const TFunc_Instances_Matrix_clone::TMethod TFunc_Instances_Matrix_clone::Method = &Instances::fl_geom::Matrix::clone;
template <> const TFunc_Instances_Matrix_concat::TMethod TFunc_Instances_Matrix_concat::Method = &Instances::fl_geom::Matrix::concat;
template <> const TFunc_Instances_Matrix_createBox::TMethod TFunc_Instances_Matrix_createBox::Method = &Instances::fl_geom::Matrix::createBox;
template <> const TFunc_Instances_Matrix_createGradientBox::TMethod TFunc_Instances_Matrix_createGradientBox::Method = &Instances::fl_geom::Matrix::createGradientBox;
template <> const TFunc_Instances_Matrix_deltaTransformPoint::TMethod TFunc_Instances_Matrix_deltaTransformPoint::Method = &Instances::fl_geom::Matrix::deltaTransformPoint;
template <> const TFunc_Instances_Matrix_identity::TMethod TFunc_Instances_Matrix_identity::Method = &Instances::fl_geom::Matrix::identity;
template <> const TFunc_Instances_Matrix_invert::TMethod TFunc_Instances_Matrix_invert::Method = &Instances::fl_geom::Matrix::invert;
template <> const TFunc_Instances_Matrix_rotate::TMethod TFunc_Instances_Matrix_rotate::Method = &Instances::fl_geom::Matrix::rotate;
template <> const TFunc_Instances_Matrix_scale::TMethod TFunc_Instances_Matrix_scale::Method = &Instances::fl_geom::Matrix::scale;
template <> const TFunc_Instances_Matrix_toString::TMethod TFunc_Instances_Matrix_toString::Method = &Instances::fl_geom::Matrix::toString;
template <> const TFunc_Instances_Matrix_transformPoint::TMethod TFunc_Instances_Matrix_transformPoint::Method = &Instances::fl_geom::Matrix::transformPoint;
template <> const TFunc_Instances_Matrix_translate::TMethod TFunc_Instances_Matrix_translate::Method = &Instances::fl_geom::Matrix::translate;

namespace Instances { namespace fl_geom
{
    Matrix::Matrix(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
    , a()
    , b()
    , c()
    , d()
    , tx()
    , ty()
//##protect##"instance::Matrix::Matrix()$data"
//##protect##"instance::Matrix::Matrix()$data"
    {
//##protect##"instance::Matrix::Matrix()$code"
//##protect##"instance::Matrix::Matrix()$code"
    }

    void Matrix::clone(SPtr<Instances::fl_geom::Matrix>& result)
    {
//##protect##"instance::Matrix::clone()"
		Value args[] = {Value(a), Value(b), Value(c), Value(d), Value(tx), Value(ty)};
		GetVM().ConstructBuiltinObject(result, "flash.geom.Matrix", 6, args).DoNotCheck();
//##protect##"instance::Matrix::clone()"
    }
    void Matrix::concat(const Value& result, Instances::fl_geom::Matrix* m)
    {
//##protect##"instance::Matrix::concat()"
        SF_UNUSED1(result);
		if (!m)
		{
            return (GetVM().ThrowTypeError(VM::Error(VM::eConvertNullToObjectError, GetVM())));
		}
		Matrix2DDouble m2d = GetMatrix();
		m2d.Append(m->GetMatrix());
		SetMatrix(m2d);
//##protect##"instance::Matrix::concat()"
    }
    void Matrix::createBox(const Value& result, Value::Number scaleX, Value::Number scaleY, Value::Number rotation, Value::Number ttx, Value::Number tty)
    {
//##protect##"instance::Matrix::createBox()"
        Matrix2DDouble m = GetMatrix();
		m.AppendRotation(rotation);
		m.AppendScaling(scaleX, scaleY);
		m.AppendTranslation(ttx, tty);
		SetMatrix(m);
        SF_UNUSED1(result);
//##protect##"instance::Matrix::createBox()"
    }
    void Matrix::createGradientBox(const Value& result, Value::Number width, Value::Number height, Value::Number rotation, Value::Number ttx, Value::Number tty)
    {
//##protect##"instance::Matrix::createGradientBox()"
        SF_UNUSED1(result);
        Matrix2DDouble m;
        float w = float(width);
        float h = float(height);
        float r = float(rotation);
        float txn = (w * 0.5f) + float(ttx);
        float tyn = (h * 0.5f) + float(tty);

        w *= GradientBoxMagicNumber; 
        h *= GradientBoxMagicNumber;
        m.AppendRotation(r);
        m.AppendScaling(w, h);
        m.AppendTranslation(txn, tyn);

        SetMatrix(m);
//##protect##"instance::Matrix::createGradientBox()"
    }
    void Matrix::deltaTransformPoint(SPtr<Instances::fl_geom::Point>& result, Instances::fl_geom::Point* point)
    {
//##protect##"instance::Matrix::deltaTransformPoint()"
		// The difference between this function and transformPoint is that here we don't add the translation
		if (!point)
		{
            return (GetVM().ThrowTypeError(VM::Error(VM::eConvertNullToObjectError, GetVM())));
		}
		Value::Number px = point->GetX();
		Value::Number py = point->GetY();

		Value args[] = {Value(a*px + c*py), Value(b*px + d*py)};
		GetVM().ConstructBuiltinObject(result, "flash.geom.Point", 2, args).DoNotCheck();
//##protect##"instance::Matrix::deltaTransformPoint()"
    }
    void Matrix::identity(const Value& result)
    {
//##protect##"instance::Matrix::identity()"
		SetMatrix(Matrix2DDouble());
        SF_UNUSED1(result);
//##protect##"instance::Matrix::identity()"
    }
    void Matrix::invert(const Value& result)
    {
//##protect##"instance::Matrix::invert()"
        SF_UNUSED1(result);
		Matrix2DDouble m(a,c,tx,b,d,ty);
		m.Invert();
		SetMatrix(m);
//##protect##"instance::Matrix::invert()"
    }
    void Matrix::rotate(const Value& result, Value::Number angle)
    {
//##protect##"instance::Matrix::rotate()"
        SF_UNUSED1(result);
		Matrix2DDouble m(a,c,tx,b,d,ty);
	    m.AppendRotation(angle);
		SetMatrix(m);
//##protect##"instance::Matrix::rotate()"
    }
    void Matrix::scale(const Value& result, Value::Number sx, Value::Number sy)
    {
//##protect##"instance::Matrix::scale()"
        SF_UNUSED1(result);
		Matrix2DDouble m(a,c,tx,b,d,ty);
		m.AppendScaling(sx, sy);
		SetMatrix(m);
//##protect##"instance::Matrix::scale()"
    }
    void Matrix::toString(ASString& result)
    {
//##protect##"instance::Matrix::toString()"
		result += GetStringManager().CreateConstString("(a=") + GetVM().AsString(Value(a)) + 
			GetStringManager().CreateConstString(", ") + 
			GetStringManager().CreateConstString("b=") + 
			GetVM().AsString(Value(b)) +
			GetStringManager().CreateConstString(", ") + 
			GetStringManager().CreateConstString("c=") + 
			GetVM().AsString(Value(c)) +
			GetStringManager().CreateConstString(", ") + 
			GetStringManager().CreateConstString("d=") + 
			GetVM().AsString(Value(d)) +
			GetStringManager().CreateConstString(", ") + 
			GetStringManager().CreateConstString("tx=") + 
			GetVM().AsString(Value(tx)) +
			GetStringManager().CreateConstString(", ") + 
			GetStringManager().CreateConstString("ty=") + 
			GetVM().AsString(Value(ty)) +
			GetStringManager().CreateConstString(")");
//##protect##"instance::Matrix::toString()"
    }
    void Matrix::transformPoint(SPtr<Instances::fl_geom::Point>& result, Instances::fl_geom::Point* point)
    {
//##protect##"instance::Matrix::transformPoint()"
		if (!point)
		{
            return (GetVM().ThrowTypeError(VM::Error(VM::eConvertNullToObjectError, GetVM())));
			
		}
		Value::Number px = point->GetX();
		Value::Number py = point->GetY();

		Value args[] = {Value(a*px + c*py + tx), Value(b*px + d*py + ty)};
		GetVM().ConstructBuiltinObject(result, "flash.geom.Point", 2, args).DoNotCheck();
//##protect##"instance::Matrix::transformPoint()"
    }
    void Matrix::translate(const Value& result, Value::Number dx, Value::Number dy)
    {
//##protect##"instance::Matrix::translate()"
        SF_UNUSED1(result);
		Matrix2DDouble m(a,c,tx,b,d,ty);
		m.AppendTranslation(dx, dy);
		SetMatrix(m);
//##protect##"instance::Matrix::translate()"
    }

    SPtr<Instances::fl_geom::Point> Matrix::deltaTransformPoint(Instances::fl_geom::Point* point)
    {
        SPtr<Instances::fl_geom::Point> result;
        deltaTransformPoint(result, point);
        return result;
    }
    SPtr<Instances::fl_geom::Point> Matrix::transformPoint(Instances::fl_geom::Point* point)
    {
        SPtr<Instances::fl_geom::Point> result;
        transformPoint(result, point);
        return result;
    }
//##protect##"instance$methods"
    // Some magic number used for scaling coefficients in CreateGradientBox.
    // This number was deduced empirically; in reality it may be more complex
    // and depend on something else
    const float Matrix::GradientBoxMagicNumber = 1.0f/1638.4f;

    void Matrix::SetMatrix(const Matrix2DDouble& m)
    {
        a = m.Sx();
        b = m.Shy();
        c = m.Shx();
        d = m.Sy();
        tx = m.Tx();
        ty = m.Ty();
    }

	Matrix2DDouble Matrix::GetMatrix() const
	{
		Matrix2DDouble m;

		m.Sx() = a;
		m.Shy() = b;
		m.Shx() = c;
		m.Sy() = d;
		m.Tx() = tx;
		m.Ty() = ty;

		return m;
	}

    Matrix2DFloat Matrix::GetMatrixF() const
    {
        Matrix2DFloat m;

        m.Sx() = float(a);
        m.Shy() = float(b);
        m.Shx() = float(c);
        m.Sy() = float(d);
        m.Tx() = float(tx);
        m.Ty() = float(ty);

        return m;
    }

	void Matrix::AS3Constructor(unsigned argc, const Value* argv)
	{
		// Initialize to default values
		a = 1; b = 0; c = 0; d = 1; tx = 0; ty = 0;
		if (argc > 0)
		{
			// According to doc, x will not be modified if there is an exception
			if (!argv[0].Convert2Number(a))
			{
				return;
			}
			if (argc > 1)
			{
				if (!argv[1].Convert2Number(b))
				{
					return;
				}
				if (argc > 2)
				{
					if (!argv[2].Convert2Number(c))
					{
						return;
					}
					if (argc > 3)
					{
						if (!argv[3].Convert2Number(d))
						{
							return;
						}
						if (argc > 4)
						{
							if (!argv[4].Convert2Number(tx))
							{
								return;
							}
							if (argc > 5)
							{
								if (!argv[5].Convert2Number(ty))
								{
									return;
								}
							}
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
    // const UInt16 Matrix::tito[Matrix::ThunkInfoNum] = {
    //    0, 1, 3, 9, 15, 17, 18, 19, 21, 24, 25, 27, 
    // };
    const TypeInfo* Matrix::tit[30] = {
        &AS3::fl_geom::MatrixTI, 
        NULL, &AS3::fl_geom::MatrixTI, 
        NULL, &AS3::fl::NumberTI, &AS3::fl::NumberTI, &AS3::fl::NumberTI, &AS3::fl::NumberTI, &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, &AS3::fl::NumberTI, &AS3::fl::NumberTI, &AS3::fl::NumberTI, &AS3::fl::NumberTI, 
        &AS3::fl_geom::PointTI, &AS3::fl_geom::PointTI, 
        NULL, 
        NULL, 
        NULL, &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, &AS3::fl::NumberTI, 
        &AS3::fl::StringTI, 
        &AS3::fl_geom::PointTI, &AS3::fl_geom::PointTI, 
        NULL, &AS3::fl::NumberTI, &AS3::fl::NumberTI, 
    };
    const Abc::ConstValue Matrix::dva[6] = {
        {Abc::CONSTANT_Double, 0}, {Abc::CONSTANT_Double, 0}, {Abc::CONSTANT_Double, 0}, 
        {Abc::CONSTANT_Double, 0}, {Abc::CONSTANT_Double, 0}, {Abc::CONSTANT_Double, 0}, 
    };
    const ThunkInfo Matrix::ti[Matrix::ThunkInfoNum] = {
        {TFunc_Instances_Matrix_clone::Func, &Matrix::tit[0], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Matrix_concat::Func, &Matrix::tit[1], "concat", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Matrix_createBox::Func, &Matrix::tit[3], "createBox", NULL, Abc::NS_Public, CT_Method, 2, 5, 0, 3, &Matrix::dva[0]},
        {TFunc_Instances_Matrix_createGradientBox::Func, &Matrix::tit[9], "createGradientBox", NULL, Abc::NS_Public, CT_Method, 2, 5, 0, 3, &Matrix::dva[3]},
        {TFunc_Instances_Matrix_deltaTransformPoint::Func, &Matrix::tit[15], "deltaTransformPoint", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Matrix_identity::Func, &Matrix::tit[17], "identity", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Matrix_invert::Func, &Matrix::tit[18], "invert", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Matrix_rotate::Func, &Matrix::tit[19], "rotate", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Matrix_scale::Func, &Matrix::tit[21], "scale", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Instances_Matrix_toString::Func, &Matrix::tit[24], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Matrix_transformPoint::Func, &Matrix::tit[25], "transformPoint", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Matrix_translate::Func, &Matrix::tit[27], "translate", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
    };
    const MemberInfo Matrix::mi[Matrix::MemberInfoNum] = {
        {"a", NULL, OFFSETOF(Instances::fl_geom::Matrix, a), Abc::NS_Public, SlotInfo::BT_Number, 0},
        {"b", NULL, OFFSETOF(Instances::fl_geom::Matrix, b), Abc::NS_Public, SlotInfo::BT_Number, 0},
        {"c", NULL, OFFSETOF(Instances::fl_geom::Matrix, c), Abc::NS_Public, SlotInfo::BT_Number, 0},
        {"d", NULL, OFFSETOF(Instances::fl_geom::Matrix, d), Abc::NS_Public, SlotInfo::BT_Number, 0},
        {"tx", NULL, OFFSETOF(Instances::fl_geom::Matrix, tx), Abc::NS_Public, SlotInfo::BT_Number, 0},
        {"ty", NULL, OFFSETOF(Instances::fl_geom::Matrix, ty), Abc::NS_Public, SlotInfo::BT_Number, 0},
    };


    Matrix::Matrix(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::Matrix::Matrix()"
//##protect##"InstanceTraits::Matrix::Matrix()"

    }

    void Matrix::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<Matrix&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_geom
{

    Matrix::Matrix(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::Matrix::Matrix()"
//##protect##"ClassTraits::Matrix::Matrix()"

    }

    Pickable<Traits> Matrix::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) Matrix(vm, AS3::fl_geom::MatrixCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_geom::MatrixCI));
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
    const TypeInfo MatrixTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_geom::Matrix::InstanceType),
        0,
        0,
        InstanceTraits::fl_geom::Matrix::ThunkInfoNum,
        InstanceTraits::fl_geom::Matrix::MemberInfoNum,
        "Matrix", "flash.geom", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo MatrixCI = {
        &MatrixTI,
        ClassTraits::fl_geom::Matrix::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_geom::Matrix::ti,
        InstanceTraits::fl_geom::Matrix::mi,
    };
}; // namespace fl_geom


}}} // namespace Scaleform { namespace GFx { namespace AS3

