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

#ifndef INC_AS3_Obj_Display_GraphicsPathCommand_H
#define INC_AS3_Obj_Display_GraphicsPathCommand_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_display
{
    extern const TypeInfo GraphicsPathCommandTI;
    extern const ClassInfo GraphicsPathCommandCI;
} // namespace fl_display

namespace ClassTraits { namespace fl_display
{
    class GraphicsPathCommand;
}}

namespace InstanceTraits { namespace fl_display
{
    class GraphicsPathCommand;
}}

namespace Classes { namespace fl_display
{
    class GraphicsPathCommand;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_display
{
    class GraphicsPathCommand : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::GraphicsPathCommand"; }
#endif
    public:
        typedef Classes::fl_display::GraphicsPathCommand ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        GraphicsPathCommand(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 7 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_display
{
    class GraphicsPathCommand : public Class
    {
        friend class ClassTraits::fl_display::GraphicsPathCommand;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_display::GraphicsPathCommandTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::GraphicsPathCommand"; }
#endif
    public:
        typedef GraphicsPathCommand SelfType;
        typedef GraphicsPathCommand ClassType;
        
    private:
        GraphicsPathCommand(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const SInt32 CUBIC_CURVE_TO;
        const SInt32 CURVE_TO;
        const SInt32 LINE_TO;
        const SInt32 MOVE_TO;
        const SInt32 NO_OP;
        const SInt32 WIDE_LINE_TO;
        const SInt32 WIDE_MOVE_TO;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

