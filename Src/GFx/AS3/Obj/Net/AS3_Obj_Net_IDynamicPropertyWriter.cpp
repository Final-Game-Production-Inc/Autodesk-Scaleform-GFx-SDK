//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Net_IDynamicPropertyWriter.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Net_IDynamicPropertyWriter.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "AS3_Obj_Net_IDynamicPropertyOutput.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace InstanceTraits { namespace fl_net
{
    // const UInt16 IDynamicPropertyWriter_tito[1] = {
    //    0, 
    // };
    const TypeInfo* IDynamicPropertyWriter_tit[3] = {
        NULL, &AS3::fl::ObjectTI, &AS3::fl_net::IDynamicPropertyOutputTI, 
    };
    const ThunkInfo IDynamicPropertyWriter_ti[1] = {
        {ThunkInfo::EmptyFunc, &IDynamicPropertyWriter_tit[0], "writeDynamicProperties", "flash.net:IDynamicPropertyWriter", Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
    };

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_net
{

    IDynamicPropertyWriter::IDynamicPropertyWriter(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::IDynamicPropertyWriter::IDynamicPropertyWriter()"
//##protect##"ClassTraits::IDynamicPropertyWriter::IDynamicPropertyWriter()"

    }

    Pickable<Traits> IDynamicPropertyWriter::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) IDynamicPropertyWriter(vm, AS3::fl_net::IDynamicPropertyWriterCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_net::IDynamicPropertyWriterCI));
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
    const TypeInfo IDynamicPropertyWriterTI = {
        TypeInfo::CompileTime | TypeInfo::TypeInterface,
        sizeof(ClassTraits::fl_net::IDynamicPropertyWriter::InstanceType),
        0,
        0,
        1,
        0,
        "IDynamicPropertyWriter", "flash.net", NULL,
        TypeInfo::None
    };

    const ClassInfo IDynamicPropertyWriterCI = {
        &IDynamicPropertyWriterTI,
        ClassTraits::fl_net::IDynamicPropertyWriter::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_net::IDynamicPropertyWriter_ti,
        NULL,
    };
}; // namespace fl_net


}}} // namespace Scaleform { namespace GFx { namespace AS3

