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

#ifndef INC_AS3_Obj_Display_GraphicsStroke_H
#define INC_AS3_Obj_Display_GraphicsStroke_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
#include "AS3_Obj_Display_IGraphicsFill.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_display
{
    extern const TypeInfo GraphicsStrokeTI;
    extern const ClassInfo GraphicsStrokeCI;
    extern const TypeInfo IGraphicsPathTI;
    extern const ClassInfo IGraphicsPathCI;
    extern const TypeInfo IGraphicsDataTI;
    extern const ClassInfo IGraphicsDataCI;
} // namespace fl_display

namespace ClassTraits { namespace fl_display
{
    class GraphicsStroke;
}}

namespace InstanceTraits { namespace fl_display
{
    class GraphicsStroke;
}}

namespace Classes { namespace fl_display
{
    class GraphicsStroke;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_display
{
    class GraphicsStroke : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_display::GraphicsStroke;
        
    public:
        typedef GraphicsStroke SelfType;
        typedef Classes::fl_display::GraphicsStroke ClassType;
        typedef InstanceTraits::fl_display::GraphicsStroke TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_display::GraphicsStrokeTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_display::GraphicsStroke"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        GraphicsStroke(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        virtual void AS3Constructor(unsigned argc, const Value* argv);
//##protect##"instance$methods"

    public:
        // AS3 API members.
        ASString caps;
        SPtr<Instances::fl::Object> fill;
        ASString joints;
        Value::Number miterLimit;
        bool pixelHinting;
        ASString scaleMode;
        Value::Number thickness;

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{
    class GraphicsStroke : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::GraphicsStroke"; }
#endif
    public:
        typedef Instances::fl_display::GraphicsStroke InstanceType;

    public:
        GraphicsStroke(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_display::GraphicsStroke> MakeInstance(GraphicsStroke& t)
        {
            return Pickable<Instances::fl_display::GraphicsStroke>(new(t.Alloc()) Instances::fl_display::GraphicsStroke(t));
        }
        SPtr<Instances::fl_display::GraphicsStroke> MakeInstanceS(GraphicsStroke& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { MemberInfoNum = 7 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_display
{
    class GraphicsStroke : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::GraphicsStroke"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_display::GraphicsStroke InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        GraphicsStroke(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

