//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Text_TextRenderer.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Text_TextRenderer.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../AS3_Obj_Array.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace InstanceTraits { namespace fl_text
{
    // const UInt16 TextRenderer_tito[2] = {
    //    0, 2, 
    // };
    const TypeInfo* TextRenderer_tit[4] = {
        NULL, &AS3::fl::StringTI, 
        NULL, &AS3::fl::int_TI, 
    };
    const ThunkInfo TextRenderer_ti[2] = {
        {ThunkInfo::EmptyFunc, &TextRenderer_tit[0], "displayMode", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &TextRenderer_tit[2], "maxLevel", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
    };

}} // namespace InstanceTraits

namespace Classes { namespace fl_text
{
    TextRenderer::TextRenderer(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::TextRenderer::TextRenderer()"
//##protect##"class_::TextRenderer::TextRenderer()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_text
{
    // const UInt16 TextRenderer_tito[1] = {
    //    0, 
    // };
    const TypeInfo* TextRenderer_tit[5] = {
        NULL, &AS3::fl::StringTI, &AS3::fl::StringTI, &AS3::fl::StringTI, &AS3::fl::ArrayTI, 
    };
    const ThunkInfo TextRenderer_ti[1] = {
        {ThunkInfo::EmptyFunc, &TextRenderer_tit[0], "setAdvancedAntiAliasingTable", NULL, Abc::NS_Public, CT_Method, 4, 4, 0, 0, NULL},
    };

    TextRenderer::TextRenderer(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::TextRenderer::TextRenderer()"
//##protect##"ClassTraits::TextRenderer::TextRenderer()"

    }

    Pickable<Traits> TextRenderer::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) TextRenderer(vm, AS3::fl_text::TextRendererCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_text::TextRendererCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_text
{
    const TypeInfo TextRendererTI = {
        TypeInfo::CompileTime | TypeInfo::Final | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_text::TextRenderer::InstanceType),
        1,
        0,
        2,
        0,
        "TextRenderer", "flash.text", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo TextRendererCI = {
        &TextRendererTI,
        ClassTraits::fl_text::TextRenderer::MakeClassTraits,
        ClassTraits::fl_text::TextRenderer_ti,
        NULL,
        InstanceTraits::fl_text::TextRenderer_ti,
        NULL,
    };
}; // namespace fl_text


}}} // namespace Scaleform { namespace GFx { namespace AS3

