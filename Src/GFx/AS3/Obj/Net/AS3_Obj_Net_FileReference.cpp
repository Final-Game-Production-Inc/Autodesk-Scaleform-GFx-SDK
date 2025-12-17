//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Net_FileReference.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Net_FileReference.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "AS3_Obj_Net_URLRequest.h"
#include "../AS3_Obj_Date.h"
#include "../AS3_Obj_Array.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace InstanceTraits { namespace fl_net
{
    // const UInt16 FileReference_tito[12] = {
    //    0, 1, 2, 3, 4, 5, 6, 7, 9, 10, 13, 17, 
    // };
    const TypeInfo* FileReference_tit[19] = {
        &AS3::fl::DateTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::DateTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::BooleanTI, &AS3::fl::ArrayTI, 
        NULL, 
        NULL, &AS3::fl_net::URLRequestTI, &AS3::fl::StringTI, 
        NULL, &AS3::fl_net::URLRequestTI, &AS3::fl::StringTI, &AS3::fl::BooleanTI, 
        NULL, &AS3::fl_net::URLRequestTI, 
    };
    const Abc::ConstValue FileReference_dva[3] = {
        {Abc::CONSTANT_Null, 0}, 
        {Abc::CONSTANT_Utf8, 10}, {Abc::CONSTANT_False, 0}, 
    };
    const ThunkInfo FileReference_ti[12] = {
        {ThunkInfo::EmptyFunc, &FileReference_tit[0], "creationDate", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileReference_tit[1], "creator", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileReference_tit[2], "extension", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileReference_tit[3], "modificationDate", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileReference_tit[4], "name", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileReference_tit[5], "size", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileReference_tit[6], "type", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileReference_tit[7], "browse", NULL, Abc::NS_Public, CT_Method, 0, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileReference_tit[9], "cancel", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileReference_tit[10], "download", NULL, Abc::NS_Public, CT_Method, 1, 2, 0, 1, &FileReference_dva[0]},
        {ThunkInfo::EmptyFunc, &FileReference_tit[13], "upload", NULL, Abc::NS_Public, CT_Method, 1, 3, 0, 2, &FileReference_dva[1]},
        {ThunkInfo::EmptyFunc, &FileReference_tit[17], "uploadUnencoded", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
    };

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_net
{

    FileReference::FileReference(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"ClassTraits::FileReference::FileReference()"
//##protect##"ClassTraits::FileReference::FileReference()"

    }

    Pickable<Traits> FileReference::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) FileReference(vm, AS3::fl_net::FileReferenceCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_net::FileReferenceCI));
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
    const TypeInfo FileReferenceTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_net::FileReference::InstanceType),
        0,
        0,
        12,
        0,
        "FileReference", "flash.net", &fl_events::EventDispatcherTI,
        TypeInfo::None
    };

    const ClassInfo FileReferenceCI = {
        &FileReferenceTI,
        ClassTraits::fl_net::FileReference::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_net::FileReference_ti,
        NULL,
    };
}; // namespace fl_net


}}} // namespace Scaleform { namespace GFx { namespace AS3

