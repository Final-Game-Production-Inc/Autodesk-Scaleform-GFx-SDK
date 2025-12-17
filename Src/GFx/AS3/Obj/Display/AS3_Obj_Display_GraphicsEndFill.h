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

#ifndef INC_AS3_Obj_Display_GraphicsEndFill_H
#define INC_AS3_Obj_Display_GraphicsEndFill_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_display
{
    extern const TypeInfo GraphicsEndFillTI;
    extern const ClassInfo GraphicsEndFillCI;
    extern const TypeInfo IGraphicsFillTI;
    extern const ClassInfo IGraphicsFillCI;
    extern const TypeInfo IGraphicsDataTI;
    extern const ClassInfo IGraphicsDataCI;
} // namespace fl_display

namespace ClassTraits { namespace fl_display
{
    class GraphicsEndFill;
}}

namespace InstanceTraits { namespace fl_display
{
    class GraphicsEndFill;
}}

namespace Classes { namespace fl_display
{
    class GraphicsEndFill;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_display
{
    class GraphicsEndFill : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_display::GraphicsEndFill;
        
    public:
        typedef GraphicsEndFill SelfType;
        typedef Classes::fl_display::GraphicsEndFill ClassType;
        typedef InstanceTraits::fl_display::GraphicsEndFill TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_display::GraphicsEndFillTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_display::GraphicsEndFill"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        GraphicsEndFill(InstanceTraits::Traits& t);

//##protect##"instance$methods"
//##protect##"instance$methods"

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{
    class GraphicsEndFill : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::GraphicsEndFill"; }
#endif
    public:
        typedef Instances::fl_display::GraphicsEndFill InstanceType;

    public:
        GraphicsEndFill(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_display::GraphicsEndFill> MakeInstance(GraphicsEndFill& t)
        {
            return Pickable<Instances::fl_display::GraphicsEndFill>(new(t.Alloc()) Instances::fl_display::GraphicsEndFill(t));
        }
        SPtr<Instances::fl_display::GraphicsEndFill> MakeInstanceS(GraphicsEndFill& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_display
{
    class GraphicsEndFill : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::GraphicsEndFill"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_display::GraphicsEndFill InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        GraphicsEndFill(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

