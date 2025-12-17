//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_StageScaleMode.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Display_StageScaleMode_H
#define INC_AS3_Obj_Display_StageScaleMode_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_display
{
    extern const TypeInfo StageScaleModeTI;
    extern const ClassInfo StageScaleModeCI;
} // namespace fl_display

namespace ClassTraits { namespace fl_display
{
    class StageScaleMode;
}}

namespace InstanceTraits { namespace fl_display
{
    class StageScaleMode;
}}

namespace Classes { namespace fl_display
{
    class StageScaleMode;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_display
{
    class StageScaleMode : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::StageScaleMode"; }
#endif
    public:
        typedef Classes::fl_display::StageScaleMode ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        StageScaleMode(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 4 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_display
{
    class StageScaleMode : public Class
    {
        friend class ClassTraits::fl_display::StageScaleMode;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_display::StageScaleModeTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::StageScaleMode"; }
#endif
    public:
        typedef StageScaleMode SelfType;
        typedef StageScaleMode ClassType;
        
    private:
        StageScaleMode(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* EXACT_FIT;
        const char* NO_BORDER;
        const char* NO_SCALE;
        const char* SHOW_ALL;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

