//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_Graphics.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy, Artem Bolgar, Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Display_Graphics_H
#define INC_AS3_Obj_Display_Graphics_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
#include "GFx/GFx_DrawingContext.h"
#include "../Vec/AS3_Obj_Vec_Vector_int.h"
#include "../Vec/AS3_Obj_Vec_Vector_double.h"
#include "../Vec/AS3_Obj_Vec_Vector_object.h"
#include "GFx/GFx_DisplayObject.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_display
{
    extern const TypeInfo GraphicsTI;
    extern const ClassInfo GraphicsCI;
    extern const TypeInfo BitmapDataTI;
    extern const ClassInfo BitmapDataCI;
} // namespace fl_display
namespace fl_geom
{
    extern const TypeInfo MatrixTI;
    extern const ClassInfo MatrixCI;
} // namespace fl_geom
namespace fl
{
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo uintTI;
    extern const ClassInfo uintCI;
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo ArrayTI;
    extern const ClassInfo ArrayCI;
} // namespace fl
namespace fl_vec
{
    extern const TypeInfo Vector_objectTI;
    extern const ClassInfo Vector_objectCI;
    extern const TypeInfo Vector_intTI;
    extern const ClassInfo Vector_intCI;
    extern const TypeInfo Vector_doubleTI;
    extern const ClassInfo Vector_doubleCI;
} // namespace fl_vec

namespace ClassTraits { namespace fl_display
{
    class Graphics;
}}

namespace InstanceTraits { namespace fl_display
{
    class Graphics;
}}

namespace Classes { namespace fl_display
{
    class Graphics;
}}

//##protect##"forward_declaration"
namespace Instances
{
    namespace fl_display
    {
        class BitmapData;
    }
    namespace fl_geom
    {
        class Matrix;
    }
}
//##protect##"forward_declaration"

namespace Instances { namespace fl_display
{
    class Graphics : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_display::Graphics;
        
    public:
        typedef Graphics SelfType;
        typedef Classes::fl_display::Graphics ClassType;
        typedef InstanceTraits::fl_display::Graphics TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_display::GraphicsTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_display::Graphics"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        Graphics(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        virtual void AS3Constructor(unsigned argc, const Value* argv);
        void AcquirePath(bool newShapeFlag);
        void CreateGradientHelper(unsigned argc, const Value* const argv, Render::ComplexFill* complexFill);
    public:
        static void FillGradientData( Instances::fl::Array* colors, Instances::fl::Array* alphas, Instances::fl::Array* ratios, GradientData* gradientData );
    protected:

        //##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_beginBitmapFill, 
            mid_beginFill, 
            mid_beginGradientFill, 
            mid_clear, 
            mid_curveTo, 
            mid_drawCircle, 
            mid_drawEllipse, 
            mid_drawGraphicsData, 
            mid_drawPath, 
            mid_drawRect, 
            mid_drawRoundRect, 
            mid_drawRoundRectComplex, 
            mid_endFill, 
            mid_lineGradientStyle, 
            mid_lineStyle, 
            mid_lineTo, 
            mid_moveTo, 
        };
        void beginBitmapFill(const Value& result, Instances::fl_display::BitmapData* bitmap, Instances::fl_geom::Matrix* matrix = NULL, bool repeat = true, bool smooth = false);
        void beginFill(const Value& result, UInt32 color, Value::Number alpha = 1.0);
        void beginGradientFill(Value& result, unsigned argc, const Value* const argv);
        void clear(const Value& result);
        void curveTo(const Value& result, Value::Number controlX, Value::Number controlY, Value::Number anchorX, Value::Number anchorY);
        void drawCircle(const Value& result, Value::Number x, Value::Number y, Value::Number radius);
        void drawEllipse(const Value& result, Value::Number x, Value::Number y, Value::Number width, Value::Number height);
        void drawGraphicsData(const Value& result, Instances::fl_vec::Vector_object* graphicsData);
        void drawPath(const Value& result, Instances::fl_vec::Vector_int* commands, Instances::fl_vec::Vector_double* data, const ASString& winding);
        void drawRect(const Value& result, Value::Number x, Value::Number y, Value::Number width, Value::Number height);
        void drawRoundRect(const Value& result, Value::Number x, Value::Number y, Value::Number width, Value::Number height, Value::Number ellipseWidth, Value::Number ellipseHeight = NumberUtil::NaN());
        void drawRoundRectComplex(Value& result, unsigned argc, const Value* const argv);
        void endFill(const Value& result);
        void lineGradientStyle(Value& result, unsigned argc, const Value* const argv);
        void lineStyle(Value& result, unsigned argc, const Value* const argv);
        void lineTo(const Value& result, Value::Number x, Value::Number y);
        void moveTo(const Value& result, Value::Number x, Value::Number y);

        // C++ friendly wrappers for AS3 methods.
        void beginBitmapFill(Instances::fl_display::BitmapData* bitmap, Instances::fl_geom::Matrix* matrix = NULL, bool repeat = true, bool smooth = false)
        {
            beginBitmapFill(Value::GetUndefined(), bitmap, matrix, repeat, smooth);
        }
        void beginFill(UInt32 color, Value::Number alpha = 1.0)
        {
            beginFill(Value::GetUndefined(), color, alpha);
        }
        void clear()
        {
            clear(Value::GetUndefined());
        }
        void curveTo(Value::Number controlX, Value::Number controlY, Value::Number anchorX, Value::Number anchorY)
        {
            curveTo(Value::GetUndefined(), controlX, controlY, anchorX, anchorY);
        }
        void drawCircle(Value::Number x, Value::Number y, Value::Number radius)
        {
            drawCircle(Value::GetUndefined(), x, y, radius);
        }
        void drawEllipse(Value::Number x, Value::Number y, Value::Number width, Value::Number height)
        {
            drawEllipse(Value::GetUndefined(), x, y, width, height);
        }
        void drawGraphicsData(Instances::fl_vec::Vector_object* graphicsData)
        {
            drawGraphicsData(Value::GetUndefined(), graphicsData);
        }
        void drawPath(Instances::fl_vec::Vector_int* commands, Instances::fl_vec::Vector_double* data, const ASString& winding)
        {
            drawPath(Value::GetUndefined(), commands, data, winding);
        }
        void drawRect(Value::Number x, Value::Number y, Value::Number width, Value::Number height)
        {
            drawRect(Value::GetUndefined(), x, y, width, height);
        }
        void drawRoundRect(Value::Number x, Value::Number y, Value::Number width, Value::Number height, Value::Number ellipseWidth, Value::Number ellipseHeight = NumberUtil::NaN())
        {
            drawRoundRect(Value::GetUndefined(), x, y, width, height, ellipseWidth, ellipseHeight);
        }
        void endFill()
        {
            endFill(Value::GetUndefined());
        }
        void lineTo(Value::Number x, Value::Number y)
        {
            lineTo(Value::GetUndefined(), x, y);
        }
        void moveTo(Value::Number x, Value::Number y)
        {
            moveTo(Value::GetUndefined(), x, y);
        }

//##protect##"instance$data"
    public:
        Ptr<DrawingContext> pDrawing;
        GFx::DisplayObject* pDispObj;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{
    class Graphics : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::Graphics"; }
#endif
    public:
        typedef Instances::fl_display::Graphics InstanceType;

    public:
        Graphics(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_display::Graphics> MakeInstance(Graphics& t)
        {
            return Pickable<Instances::fl_display::Graphics>(new(t.Alloc()) Instances::fl_display::Graphics(t));
        }
        SPtr<Instances::fl_display::Graphics> MakeInstanceS(Graphics& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 17 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[84];
        static const Abc::ConstValue dva[19];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_display
{
    class Graphics : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Graphics"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_display::Graphics InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Graphics(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

