//##protect##"disclaimer"
/**********************************************************************

Filename    :   .h
Content     :   
Created     :   Nov, 2013
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2013 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**********************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Display_GraphicsGradientFill_H
#define INC_AS3_Obj_Display_GraphicsGradientFill_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
#include "../Geom/AS3_Obj_Geom_Matrix.h"
#include "../AS3_Obj_Array.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_display
{
    extern const TypeInfo GraphicsGradientFillTI;
    extern const ClassInfo GraphicsGradientFillCI;
    extern const TypeInfo IGraphicsFillTI;
    extern const ClassInfo IGraphicsFillCI;
    extern const TypeInfo IGraphicsDataTI;
    extern const ClassInfo IGraphicsDataCI;
} // namespace fl_display

namespace ClassTraits { namespace fl_display
{
    class GraphicsGradientFill;
}}

namespace InstanceTraits { namespace fl_display
{
    class GraphicsGradientFill;
}}

namespace Classes { namespace fl_display
{
    class GraphicsGradientFill;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_display
{
    class GraphicsGradientFill : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_display::GraphicsGradientFill;
        
    public:
        typedef GraphicsGradientFill SelfType;
        typedef Classes::fl_display::GraphicsGradientFill ClassType;
        typedef InstanceTraits::fl_display::GraphicsGradientFill TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_display::GraphicsGradientFillTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_display::GraphicsGradientFill"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        GraphicsGradientFill(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        virtual void AS3Constructor(unsigned argc, const Value* argv);
//##protect##"instance$methods"

    public:
        // AS3 API members.
        SPtr<Instances::fl::Array> alphas;
        SPtr<Instances::fl::Array> colors;
        Value::Number focalPointRatio;
        ASString interpolationMethod;
        SPtr<Instances::fl_geom::Matrix> matrix;
        SPtr<Instances::fl::Array> ratios;
        ASString spreadMethod;
        ASString type;

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{
    class GraphicsGradientFill : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::GraphicsGradientFill"; }
#endif
    public:
        typedef Instances::fl_display::GraphicsGradientFill InstanceType;

    public:
        GraphicsGradientFill(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_display::GraphicsGradientFill> MakeInstance(GraphicsGradientFill& t)
        {
            return Pickable<Instances::fl_display::GraphicsGradientFill>(new(t.Alloc()) Instances::fl_display::GraphicsGradientFill(t));
        }
        SPtr<Instances::fl_display::GraphicsGradientFill> MakeInstanceS(GraphicsGradientFill& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { MemberInfoNum = 8 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_display
{
    class GraphicsGradientFill : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::GraphicsGradientFill"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_display::GraphicsGradientFill InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        GraphicsGradientFill(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

