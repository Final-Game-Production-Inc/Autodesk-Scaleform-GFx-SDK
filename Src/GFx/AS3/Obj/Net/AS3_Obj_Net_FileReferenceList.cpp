//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Net_FileReferenceList.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Net_FileReferenceList.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../AS3_Obj_Array.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace InstanceTraits { namespace fl_net
{
    // const UInt16 FileReferenceList_tito[2] = {
    //    0, 1, 
    // };
    const TypeInfo* FileReferenceList_tit[3] = {
        &AS3::fl::ArrayTI, 
        &AS3::fl::BooleanTI, &AS3::fl::ArrayTI, 
    };
    const ThunkInfo FileReferenceList_ti[2] = {
        {ThunkInfo::EmptyFunc, &FileReferenceList_tit[0], "fileList", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileReferenceList_tit[1], "browse", NULL, Abc::NS_Public, CT_Method, 0, 1, 0, 0, NULL},
    };

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_net
{

    FileReferenceList::FileReferenceList(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"ClassTraits::FileReferenceList::FileReferenceList()"
//##protect##"ClassTraits::FileReferenceList::FileReferenceList()"

    }

    Pickable<Traits> FileReferenceList::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) FileReferenceList(vm, AS3::fl_net::FileReferenceListCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_net::FileReferenceListCI));
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
    const TypeInfo FileReferenceListTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_net::FileReferenceList::InstanceType),
        0,
        0,
        2,
        0,
        "FileReferenceList", "flash.net", &fl_events::EventDispatcherTI,
        TypeInfo::None
    };

    const ClassInfo FileReferenceListCI = {
        &FileReferenceListTI,
        ClassTraits::fl_net::FileReferenceList::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_net::FileReferenceList_ti,
        NULL,
    };
}; // namespace fl_net


}}} // namespace Scaleform { namespace GFx { namespace AS3

