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

#ifndef INC_AS3_Obj_Display_GraphicsSolidFill_H
#define INC_AS3_Obj_Display_GraphicsSolidFill_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_display
{
    extern const TypeInfo GraphicsSolidFillTI;
    extern const ClassInfo GraphicsSolidFillCI;
    extern const TypeInfo IGraphicsFillTI;
    extern const ClassInfo IGraphicsFillCI;
    extern const TypeInfo IGraphicsDataTI;
    extern const ClassInfo IGraphicsDataCI;
} // namespace fl_display

namespace ClassTraits { namespace fl_display
{
    class GraphicsSolidFill;
}}

namespace InstanceTraits { namespace fl_display
{
    class GraphicsSolidFill;
}}

namespace Classes { namespace fl_display
{
    class GraphicsSolidFill;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_display
{
    class GraphicsSolidFill : public Instances::fl::Object
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_display::GraphicsSolidFill;
        
    public:
        typedef GraphicsSolidFill SelfType;
        typedef Classes::fl_display::GraphicsSolidFill ClassType;
        typedef InstanceTraits::fl_display::GraphicsSolidFill TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_display::GraphicsSolidFillTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_display::GraphicsSolidFill"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        GraphicsSolidFill(InstanceTraits::Traits& t);

//##protect##"instance$methods"
        virtual void AS3Constructor(unsigned argc, const Value* argv);
//##protect##"instance$methods"

    public:
        // AS3 API members.
        Value::Number alpha;
        UInt32 color;

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{
    class GraphicsSolidFill : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::GraphicsSolidFill"; }
#endif
    public:
        typedef Instances::fl_display::GraphicsSolidFill InstanceType;

    public:
        GraphicsSolidFill(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_display::GraphicsSolidFill> MakeInstance(GraphicsSolidFill& t)
        {
            return Pickable<Instances::fl_display::GraphicsSolidFill>(new(t.Alloc()) Instances::fl_display::GraphicsSolidFill(t));
        }
        SPtr<Instances::fl_display::GraphicsSolidFill> MakeInstanceS(GraphicsSolidFill& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { MemberInfoNum = 2 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_display
{
    class GraphicsSolidFill : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::GraphicsSolidFill"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_display::GraphicsSolidFill InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        GraphicsSolidFill(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

