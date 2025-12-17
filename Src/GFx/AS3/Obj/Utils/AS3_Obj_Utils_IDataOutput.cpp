//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Utils_IDataOutput.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Utils_IDataOutput.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "AS3_Obj_Utils_ByteArray.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace InstanceTraits { namespace fl_utils
{
    // const UInt16 IDataOutput_tito[16] = {
    //    0, 1, 3, 4, 6, 8, 10, 14, 16, 18, 20, 23, 25, 27, 29, 31, 
    // };
    const TypeInfo* IDataOutput_tit[33] = {
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl::uintTI, 
        NULL, &AS3::fl::uintTI, 
        NULL, &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::int_TI, 
        NULL, &AS3::fl_utils::ByteArrayTI, &AS3::fl::uintTI, &AS3::fl::uintTI, 
        NULL, &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        NULL, &AS3::fl::int_TI, 
        NULL, &AS3::fl::StringTI, &AS3::fl::StringTI, 
        NULL, NULL, 
        NULL, &AS3::fl::int_TI, 
        NULL, &AS3::fl::uintTI, 
        NULL, &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
    };
    const ThunkInfo IDataOutput_ti[16] = {
        {ThunkInfo::EmptyFunc, &IDataOutput_tit[0], "endian", "flash.utils:IDataOutput", Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &IDataOutput_tit[1], "endian", "flash.utils:IDataOutput", Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &IDataOutput_tit[3], "objectEncoding", "flash.utils:IDataOutput", Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &IDataOutput_tit[4], "objectEncoding", "flash.utils:IDataOutput", Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &IDataOutput_tit[6], "writeBoolean", "flash.utils:IDataOutput", Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &IDataOutput_tit[8], "writeByte", "flash.utils:IDataOutput", Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &IDataOutput_tit[10], "writeBytes", "flash.utils:IDataOutput", Abc::NS_Public, CT_Method, 1, 3, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &IDataOutput_tit[14], "writeDouble", "flash.utils:IDataOutput", Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &IDataOutput_tit[16], "writeFloat", "flash.utils:IDataOutput", Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &IDataOutput_tit[18], "writeInt", "flash.utils:IDataOutput", Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &IDataOutput_tit[20], "writeMultiByte", "flash.utils:IDataOutput", Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &IDataOutput_tit[23], "writeObject", "flash.utils:IDataOutput", Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &IDataOutput_tit[25], "writeShort", "flash.utils:IDataOutput", Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &IDataOutput_tit[27], "writeUnsignedInt", "flash.utils:IDataOutput", Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &IDataOutput_tit[29], "writeUTF", "flash.utils:IDataOutput", Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &IDataOutput_tit[31], "writeUTFBytes", "flash.utils:IDataOutput", Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
    };

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_utils
{

    IDataOutput::IDataOutput(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::IDataOutput::IDataOutput()"
//##protect##"ClassTraits::IDataOutput::IDataOutput()"

    }

    Pickable<Traits> IDataOutput::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) IDataOutput(vm, AS3::fl_utils::IDataOutputCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_utils::IDataOutputCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_utils
{
    const TypeInfo IDataOutputTI = {
        TypeInfo::CompileTime | TypeInfo::TypeInterface,
        sizeof(ClassTraits::fl_utils::IDataOutput::InstanceType),
        0,
        0,
        16,
        0,
        "IDataOutput", "flash.utils", NULL,
        TypeInfo::None
    };

    const ClassInfo IDataOutputCI = {
        &IDataOutputTI,
        ClassTraits::fl_utils::IDataOutput::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_utils::IDataOutput_ti,
        NULL,
    };
}; // namespace fl_utils


}}} // namespace Scaleform { namespace GFx { namespace AS3

