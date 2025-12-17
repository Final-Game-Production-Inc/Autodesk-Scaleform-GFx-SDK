//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_MorphShape.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Display_MorphShape_H
#define INC_AS3_Obj_Display_MorphShape_H

#include "AS3_Obj_Display_DisplayObject.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_display
{
    extern const TypeInfo MorphShapeTI;
    extern const ClassInfo MorphShapeCI;
} // namespace fl_display

namespace ClassTraits { namespace fl_display
{
    class MorphShape;
}}

namespace InstanceTraits { namespace fl_display
{
    class MorphShape;
}}

namespace Classes { namespace fl_display
{
    class MorphShape;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"

namespace Instances { namespace fl_display
{
    class MorphShape : public Instances::fl_display::DisplayObject
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_display::MorphShape;
        
    public:
        typedef MorphShape SelfType;
        typedef Classes::fl_display::MorphShape ClassType;
        typedef InstanceTraits::fl_display::MorphShape TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_display::MorphShapeTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_display::MorphShape"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        MorphShape(InstanceTraits::Traits& t);

//##protect##"instance$methods"
//##protect##"instance$methods"

//##protect##"instance$data"
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_display
{
    class MorphShape : public fl_display::DisplayObject
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::MorphShape"; }
#endif
    public:
        typedef Instances::fl_display::MorphShape InstanceType;

    public:
        MorphShape(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_display::MorphShape> MakeInstance(MorphShape& t)
        {
            return Pickable<Instances::fl_display::MorphShape>(new(t.Alloc()) Instances::fl_display::MorphShape(t));
        }
        SPtr<Instances::fl_display::MorphShape> MakeInstanceS(MorphShape& t)
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
    class MorphShape : public fl_display::DisplayObject
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::MorphShape"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_display::MorphShape InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        MorphShape(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

