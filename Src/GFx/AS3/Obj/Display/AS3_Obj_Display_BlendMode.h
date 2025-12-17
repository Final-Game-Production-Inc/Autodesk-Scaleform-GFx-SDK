//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Display_BlendMode.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Display_BlendMode_H
#define INC_AS3_Obj_Display_BlendMode_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
#include "Render/Render_States.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_display
{
    extern const TypeInfo BlendModeTI;
    extern const ClassInfo BlendModeCI;
} // namespace fl_display

namespace ClassTraits { namespace fl_display
{
    class BlendMode;
}}

namespace InstanceTraits { namespace fl_display
{
    class BlendMode;
}}

namespace Classes { namespace fl_display
{
    class BlendMode;
}}

//##protect##"forward_declaration"
#ifdef DIFFERENCE
#undef DIFFERENCE
#endif
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_display
{
    class BlendMode : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::BlendMode"; }
#endif
    public:
        typedef Classes::fl_display::BlendMode ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        BlendMode(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 14 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_display
{
    class BlendMode : public Class
    {
        friend class ClassTraits::fl_display::BlendMode;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_display::BlendModeTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::BlendMode"; }
#endif
    public:
        typedef BlendMode SelfType;
        typedef BlendMode ClassType;
        
    private:
        BlendMode(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
    public:
        static Render::BlendMode GetBlendMode(const ASString& value);
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* ADD;
        const char* ALPHA;
        const char* DARKEN;
        const char* DIFFERENCE;
        const char* ERASE;
        const char* HARDLIGHT;
        const char* INVERT;
        const char* LAYER;
        const char* LIGHTEN;
        const char* MULTIPLY;
        const char* NORMAL;
        const char* OVERLAY;
        const char* SCREEN;
        const char* SUBTRACT;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

