//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Text_TextRenderer.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Text_TextRenderer_H
#define INC_AS3_Obj_Text_TextRenderer_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_text
{
    extern const TypeInfo TextRendererTI;
    extern const ClassInfo TextRendererCI;
} // namespace fl_text
namespace fl
{
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
    extern const TypeInfo ArrayTI;
    extern const ClassInfo ArrayCI;
} // namespace fl

namespace ClassTraits { namespace fl_text
{
    class TextRenderer;
}}

namespace InstanceTraits { namespace fl_text
{
    class TextRenderer;
}}

namespace Classes { namespace fl_text
{
    class TextRenderer;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_text
{
    class TextRenderer : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::TextRenderer"; }
#endif
    public:
        typedef Classes::fl_text::TextRenderer ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        TextRenderer(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_text
{
    class TextRenderer : public Class
    {
        friend class ClassTraits::fl_text::TextRenderer;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_text::TextRendererTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::TextRenderer"; }
#endif
    public:
        typedef TextRenderer SelfType;
        typedef TextRenderer ClassType;
        
    private:
        TextRenderer(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

