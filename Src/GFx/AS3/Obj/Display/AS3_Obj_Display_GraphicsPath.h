//##protect##"disclaimer"
/**********************************************************************

Filename    :   AS3_Obj_Display_GraphicsPath.h
Content     :   
Created     :   Apr, 2013
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2013 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**********************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Display_GraphicsPath_H
#define INC_AS3_Obj_Display_GraphicsPath_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
#include "../Vec/AS3_Obj_Vec_Vector_int.h"
#include "../Vec/AS3_Obj_Vec_Vector_double.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_display
{
    extern const TypeInfo GraphicsPathTI;
    extern const ClassInfo GraphicsPathCI;
    extern const TypeInfo IGraphicsPathTI;
    extern const ClassInfo IGraphicsPathCI;
    extern const TypeInfo IGraphicsDataTI;
    extern const ClassInfo IGraphicsDataCI;
} // namespace fl_display
namespace fl
{
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
} // namespace fl

namespace ClassTraits { namespace fl_display
{
    class GraphicsPath;
}}

namespace InstanceTraits { namespace fl_display
{
    class GraphicsPath;
}}

namespace Classes { namespace fl_display
{
    class GraphicsPath;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_display
{
    class GraphicsPath : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_display::GraphicsPath;
        
    public:
        typedef GraphicsPath SelfType;
        typedef Classes::fl_display::GraphicsPath ClassType;
        typedef InstanceTraits::fl_display::GraphicsPath TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_display::GraphicsPathTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_display::GraphicsPath"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        GraphicsPath(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        void AS3Constructor(unsigned argc, const Value* argv);
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_windingGet, 
            mid_windingSet, 
            mid_cubicCurveTo, 
            mid_curveTo, 
            mid_lineTo, 
            mid_moveTo, 
            mid_wideLineTo, 
            mid_wideMoveTo, 
        };
        void windingGet(ASString& result);
        void windingSet(const Value& result, const ASString& value);
        void cubicCurveTo(const Value& result, Value::Number controlX1, Value::Number controlY1, Value::Number controlX2, Value::Number controlY2, Value::Number anchorX, Value::Number anchorY);
        void curveTo(const Value& result, Value::Number controlX, Value::Number controlY, Value::Number anchorX, Value::Number anchorY);
        void lineTo(const Value& result, Value::Number x, Value::Number y);
        void moveTo(const Value& result, Value::Number x, Value::Number y);
        void wideLineTo(const Value& result, Value::Number x, Value::Number y);
        void wideMoveTo(const Value& result, Value::Number x, Value::Number y);

        // C++ friendly wrappers for AS3 methods.
        ASString windingGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            windingGet(result);
            return result;
        }
        void windingSet(const ASString& value)
        {
            windingSet(Value::GetUndefined(), value);
        }
        void cubicCurveTo(Value::Number controlX1, Value::Number controlY1, Value::Number controlX2, Value::Number controlY2, Value::Number anchorX, Value::Number anchorY)
        {
            cubicCurveTo(Value::GetUndefined(), controlX1, controlY1, controlX2, controlY2, anchorX, anchorY);
        }
        void curveTo(Value::Number controlX, Value::Number controlY, Value::Number anchorX, Value::Number anchorY)
        {
            curveTo(Value::GetUndefined(), controlX, controlY, anchorX, anchorY);
        }
        void lineTo(Value::Number x, Value::Number y)
        {
            lineTo(Value::GetUndefined(), x, y);
        }
        void moveTo(Value::Number x, Value::Number y)
        {
            moveTo(Value::GetUndefined(), x, y);
        }
        void wideLineTo(Value::Number x, Value::Number y)
        {
            wideLineTo(Value::GetUndefined(), x, y);
        }
        void wideMoveTo(Value::Number x, Value::Number y)
        {
            wideMoveTo(Value::GetUndefined(), x, y);
        }

    public:
        // AS3 API members.
        SPtr<Instances::fl_vec::Vector_int> commands;
        SPtr<Instances::fl_vec::Vector_double> data;

//##protect##"instance$data"
        ASString Winding;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{
    class GraphicsPath : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::GraphicsPath"; }
#endif
    public:
        typedef Instances::fl_display::GraphicsPath InstanceType;

    public:
        GraphicsPath(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_display::GraphicsPath> MakeInstance(GraphicsPath& t)
        {
            return Pickable<Instances::fl_display::GraphicsPath>(new(t.Alloc()) Instances::fl_display::GraphicsPath(t));
        }
        SPtr<Instances::fl_display::GraphicsPath> MakeInstanceS(GraphicsPath& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { MemberInfoNum = 2 };
        static const MemberInfo mi[MemberInfoNum];
        enum { ThunkInfoNum = 8 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[27];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_display
{
    class GraphicsPath : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::GraphicsPath"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_display::GraphicsPath InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        GraphicsPath(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

