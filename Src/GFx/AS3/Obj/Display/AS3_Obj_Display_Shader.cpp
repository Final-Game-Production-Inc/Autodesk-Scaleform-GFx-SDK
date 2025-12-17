//##protect##"disclaimer"
/**********************************************************************

Filename    :   AS3_Obj_Display_Shader.cpp
Content     :   
Created     :   Sep, 2011
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**********************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Display_Shader.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
namespace Classes
{
    class ShaderData;
}
//##protect##"methods"

namespace InstanceTraits { namespace fl_display
{
    // const UInt16 Shader_tito[5] = {
    //    0, 2, 3, 5, 6, 
    // };
    const TypeInfo* Shader_tit[8] = {
        NULL, &AS3::fl_utils::ByteArrayTI, 
        &AS3::fl_display::ShaderDataTI, 
        NULL, &AS3::fl_display::ShaderDataTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
    };
    const ThunkInfo Shader_ti[5] = {
        {ThunkInfo::EmptyFunc, &Shader_tit[0], "byteCode", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &Shader_tit[2], "data", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &Shader_tit[3], "data", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &Shader_tit[5], "precisionHint", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &Shader_tit[6], "precisionHint", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
    };

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_display
{

    Shader::Shader(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::Shader::Shader()"
//##protect##"ClassTraits::Shader::Shader()"

    }

    Pickable<Traits> Shader::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) Shader(vm, AS3::fl_display::ShaderCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_display::ShaderCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_display
{
    const TypeInfo ShaderTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_display::Shader::InstanceType),
        0,
        0,
        5,
        0,
        "Shader", "flash.display", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo ShaderCI = {
        &ShaderTI,
        ClassTraits::fl_display::Shader::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_display::Shader_ti,
        NULL,
    };
}; // namespace fl_display


}}} // namespace Scaleform { namespace GFx { namespace AS3

