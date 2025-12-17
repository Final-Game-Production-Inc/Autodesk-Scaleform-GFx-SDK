//##protect##"disclaimer"
/**********************************************************************

Filename    :   AS3_Obj_Display_Shader.h
Content     :   
Created     :   Sep, 2011
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**********************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Display_Shader_H
#define INC_AS3_Obj_Display_Shader_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
#include "../Utils/AS3_Obj_Utils_ByteArray.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_display
{
    extern const TypeInfo ShaderTI;
    extern const ClassInfo ShaderCI;
    extern const TypeInfo ShaderDataTI;
    extern const ClassInfo ShaderDataCI;
} // namespace fl_display
namespace fl_utils
{
    extern const TypeInfo ByteArrayTI;
    extern const ClassInfo ByteArrayCI;
} // namespace fl_utils
namespace fl
{
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl

namespace ClassTraits { namespace fl_display
{
    class Shader;
}}

namespace InstanceTraits { namespace fl_display
{
    class Shader;
}}

namespace Classes { namespace fl_display
{
    class Shader;
}}

//##protect##"forward_declaration"
namespace Instances
{
    class ShaderData;
}
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_display
{
    class Shader : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Shader"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Shader(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

