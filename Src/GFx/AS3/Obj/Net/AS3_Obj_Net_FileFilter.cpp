//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Net_FileFilter.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Net_FileFilter.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace InstanceTraits { namespace fl_net
{
    // const UInt16 FileFilter_tito[6] = {
    //    0, 1, 3, 4, 6, 7, 
    // };
    const TypeInfo* FileFilter_tit[9] = {
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
    };
    const ThunkInfo FileFilter_ti[6] = {
        {ThunkInfo::EmptyFunc, &FileFilter_tit[0], "description", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileFilter_tit[1], "description", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileFilter_tit[3], "extension", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileFilter_tit[4], "extension", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileFilter_tit[6], "macType", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileFilter_tit[7], "macType", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
    };

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_net
{

    FileFilter::FileFilter(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::FileFilter::FileFilter()"
//##protect##"ClassTraits::FileFilter::FileFilter()"

    }

    Pickable<Traits> FileFilter::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) FileFilter(vm, AS3::fl_net::FileFilterCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_net::FileFilterCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_net
{
    const TypeInfo FileFilterTI = {
        TypeInfo::CompileTime | TypeInfo::Final | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_net::FileFilter::InstanceType),
        0,
        0,
        6,
        0,
        "FileFilter", "flash.net", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo FileFilterCI = {
        &FileFilterTI,
        ClassTraits::fl_net::FileFilter::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_net::FileFilter_ti,
        NULL,
    };
}; // namespace fl_net


}}} // namespace Scaleform { namespace GFx { namespace AS3

