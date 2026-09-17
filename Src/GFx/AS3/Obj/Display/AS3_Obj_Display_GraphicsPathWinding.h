//##protect##"disclaimer"
/**********************************************************************

Filename    :   .h
Content     :   
Created     :   Apr, 2013
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2013 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**********************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Display_GraphicsPathWinding_H
#define INC_AS3_Obj_Display_GraphicsPathWinding_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_display
{
    extern const TypeInfo GraphicsPathWindingTI;
    extern const ClassInfo GraphicsPathWindingCI;
} // namespace fl_display

namespace ClassTraits { namespace fl_display
{
    class GraphicsPathWinding;
}}

namespace InstanceTraits { namespace fl_display
{
    class GraphicsPathWinding;
}}

namespace Classes { namespace fl_display
{
    class GraphicsPathWinding;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_display
{
    class GraphicsPathWinding : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::GraphicsPathWinding"; }
#endif
    public:
        typedef Classes::fl_display::GraphicsPathWinding ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        GraphicsPathWinding(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 2 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_display
{
    class GraphicsPathWinding : public Class
    {
        friend class ClassTraits::fl_display::GraphicsPathWinding;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_display::GraphicsPathWindingTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::GraphicsPathWinding"; }
#endif
    public:
        typedef GraphicsPathWinding SelfType;
        typedef GraphicsPathWinding ClassType;
        
    private:
        GraphicsPathWinding(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* EVEN_ODD;
        const char* NON_ZERO;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

