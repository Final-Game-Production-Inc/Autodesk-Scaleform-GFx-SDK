//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Html_HTMLHost.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Html_HTMLHost.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../Geom/AS3_Obj_Geom_Rectangle.h"
#include "AS3_Obj_Html_HTMLWindowCreateOptions.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
namespace Classes
{
    class HTMLLoader;
}
//##protect##"methods"

namespace InstanceTraits { namespace fl_html
{
    // const UInt16 HTMLHost_tito[10] = {
    //    0, 1, 2, 4, 6, 8, 10, 12, 13, 14, 
    // };
    const TypeInfo* HTMLHost_tit[15] = {
        &AS3::fl_html::HTMLLoaderTI, 
        &AS3::fl_geom::RectangleTI, 
        NULL, &AS3::fl_geom::RectangleTI, 
        &AS3::fl_html::HTMLLoaderTI, &AS3::fl_html::HTMLWindowCreateOptionsTI, 
        NULL, &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        NULL, 
        NULL, 
        NULL, 
    };
    const ThunkInfo HTMLHost_ti[10] = {
        {ThunkInfo::EmptyFunc, &HTMLHost_tit[0], "htmlLoader", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLHost_tit[1], "windowRect", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLHost_tit[2], "windowRect", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLHost_tit[4], "createWindow", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLHost_tit[6], "updateLocation", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLHost_tit[8], "updateStatus", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLHost_tit[10], "updateTitle", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLHost_tit[12], "windowBlur", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLHost_tit[13], "windowClose", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLHost_tit[14], "windowFocus", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_html
{

    HTMLHost::HTMLHost(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::HTMLHost::HTMLHost()"
//##protect##"ClassTraits::HTMLHost::HTMLHost()"

    }

    Pickable<Traits> HTMLHost::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) HTMLHost(vm, AS3::fl_html::HTMLHostCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_html::HTMLHostCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_html
{
    const TypeInfo HTMLHostTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_html::HTMLHost::InstanceType),
        0,
        0,
        10,
        0,
        "HTMLHost", "flash.html", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo HTMLHostCI = {
        &HTMLHostTI,
        ClassTraits::fl_html::HTMLHost::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_html::HTMLHost_ti,
        NULL,
    };
}; // namespace fl_html


}}} // namespace Scaleform { namespace GFx { namespace AS3

