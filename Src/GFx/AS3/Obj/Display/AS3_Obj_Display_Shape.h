//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_Shape.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Display_Shape_H
#define INC_AS3_Obj_Display_Shape_H

#include "AS3_Obj_Display_DisplayObject.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_display
{
    extern const TypeInfo ShapeTI;
    extern const ClassInfo ShapeCI;
    extern const TypeInfo GraphicsTI;
    extern const ClassInfo GraphicsCI;
} // namespace fl_display

namespace ClassTraits { namespace fl_display
{
    class Shape;
}}

namespace InstanceTraits { namespace fl_display
{
    class Shape;
}}

namespace Classes { namespace fl_display
{
    class Shape;
}}

//##protect##"forward_declaration"
namespace Instances
{
    namespace fl_display
    {
        class Graphics;
    }
}
//##protect##"forward_declaration"

namespace Instances { namespace fl_display
{
    class Shape : public Instances::fl_display::DisplayObject
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_display::Shape;
        
    public:
        typedef Shape SelfType;
        typedef Classes::fl_display::Shape ClassType;
        typedef InstanceTraits::fl_display::Shape TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_display::ShapeTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_display::Shape"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        Shape(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        virtual void ForEachChild_GC(Collector* prcc, GcOp op) const;
        virtual void InitInstance(bool extCall);
        virtual GFx::DisplayObject* CreateStageObject();

        ~Shape();
//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_graphicsGet, 
        };
        void graphicsGet(SPtr<Instances::fl_display::Graphics>& result);

        // C++ friendly wrappers for AS3 methods.
        SPtr<Instances::fl_display::Graphics> graphicsGet();

//##protect##"instance$data"
        SPtr<Graphics> pGraphics;
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{
    class Shape : public fl_display::DisplayObject
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::Shape"; }
#endif
    public:
        typedef Instances::fl_display::Shape InstanceType;

    public:
        Shape(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_display::Shape> MakeInstance(Shape& t)
        {
            return Pickable<Instances::fl_display::Shape>(new(t.Alloc()) Instances::fl_display::Shape(t));
        }
        SPtr<Instances::fl_display::Shape> MakeInstanceS(Shape& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 1 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[1];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_display
{
    class Shape : public fl_display::DisplayObject
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Shape"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_display::Shape InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Shape(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

