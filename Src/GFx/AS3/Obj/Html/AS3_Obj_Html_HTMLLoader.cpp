//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Html_HTMLLoader.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Html_HTMLLoader.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "AS3_Obj_Html_HTMLHost.h"
#include "../Net/AS3_Obj_Net_URLRequest.h"
#include "../System/AS3_Obj_System_ApplicationDomain.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
namespace Classes
{
    class HTMLHistoryItem;
}
//##protect##"methods"

namespace InstanceTraits { namespace fl_html
{
    // const UInt16 HTMLLoader_tito[49] = {
    //    0, 1, 3, 4, 6, 7, 8, 9, 10, 12, 13, 14, 16, 17, 19, 20, 21, 22, 24, 25, 27, 28, 30, 31, 33, 34, 36, 37, 39, 40, 42, 43, 45, 46, 48, 49, 51, 52, 54, 55, 57, 58, 59, 61, 62, 63, 65, 67, 69, 
    // };
    const TypeInfo* HTMLLoader_tit[70] = {
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::BooleanTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::uintTI, 
        &AS3::fl::uintTI, 
        NULL, &AS3::fl::uintTI, 
        &AS3::fl_html::HTMLHostTI, 
        NULL, &AS3::fl_html::HTMLHostTI, 
        &AS3::fl::BooleanTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl_system::ApplicationDomainTI, 
        NULL, &AS3::fl_system::ApplicationDomainTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::ObjectTI, 
        NULL, 
        &AS3::fl_html::HTMLHistoryItemTI, &AS3::fl::uintTI, 
        NULL, 
        NULL, 
        NULL, &AS3::fl::int_TI, 
        NULL, &AS3::fl_net::URLRequestTI, 
        NULL, &AS3::fl::StringTI, 
        NULL, 
    };
    const ThunkInfo HTMLLoader_ti[49] = {
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[0], "authenticate", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[1], "authenticate", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[3], "cacheResponse", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[4], "cacheResponse", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[6], "contentHeight", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[7], "contentWidth", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[8], "hasFocusableContent", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[9], "height", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[10], "height", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[12], "historyLength", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[13], "historyPosition", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[14], "historyPosition", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[16], "htmlHost", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[17], "htmlHost", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[19], "loaded", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[20], "location", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[21], "manageCookies", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[22], "manageCookies", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[24], "navigateInSystemBrowser", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[25], "navigateInSystemBrowser", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[27], "paintsDefaultBackground", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[28], "paintsDefaultBackground", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[30], "placeLoadStringContentInApplicationSandbox", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[31], "placeLoadStringContentInApplicationSandbox", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[33], "runtimeApplicationDomain", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[34], "runtimeApplicationDomain", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[36], "scrollH", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[37], "scrollH", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[39], "scrollV", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[40], "scrollV", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[42], "textEncodingFallback", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[43], "textEncodingFallback", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[45], "textEncodingOverride", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[46], "textEncodingOverride", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[48], "useCache", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[49], "useCache", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[51], "userAgent", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[52], "userAgent", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[54], "width", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[55], "width", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[57], "window", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[58], "cancelLoad", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[59], "getHistoryAt", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[61], "historyBack", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[62], "historyForward", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[63], "historyGo", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[65], "load", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[67], "loadString", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &HTMLLoader_tit[69], "reload", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_html
{

    HTMLLoader::HTMLLoader(VM& vm, const ClassInfo& ci)
    : fl_display::Sprite(vm, ci)
    {
//##protect##"ClassTraits::HTMLLoader::HTMLLoader()"
//##protect##"ClassTraits::HTMLLoader::HTMLLoader()"

    }

    Pickable<Traits> HTMLLoader::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) HTMLLoader(vm, AS3::fl_html::HTMLLoaderCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_html::HTMLLoaderCI));
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
    const TypeInfo HTMLLoaderTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_html::HTMLLoader::InstanceType),
        0,
        0,
        49,
        0,
        "HTMLLoader", "flash.html", &fl_display::SpriteTI,
        TypeInfo::None
    };

    const ClassInfo HTMLLoaderCI = {
        &HTMLLoaderTI,
        ClassTraits::fl_html::HTMLLoader::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_html::HTMLLoader_ti,
        NULL,
    };
}; // namespace fl_html


}}} // namespace Scaleform { namespace GFx { namespace AS3

