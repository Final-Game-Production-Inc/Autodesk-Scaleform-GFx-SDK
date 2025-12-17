//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Utils_IDataInput.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Utils_IDataInput.h"
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
    // const UInt16 IDataInput_tito[19] = {
    //    0, 1, 2, 4, 5, 7, 8, 9, 13, 14, 15, 16, 19, 20, 21, 22, 23, 24, 25, 
    // };
    const TypeInfo* IDataInput_tit[27] = {
        &AS3::fl::uintTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl::uintTI, 
        NULL, &AS3::fl::uintTI, 
        &AS3::fl::BooleanTI, 
        &AS3::fl::int_TI, 
        NULL, &AS3::fl_utils::ByteArrayTI, &AS3::fl::uintTI, &AS3::fl::uintTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::int_TI, 
        &AS3::fl::StringTI, &AS3::fl::uintTI, &AS3::fl::StringTI, 
        NULL, 
        &AS3::fl::int_TI, 
        &AS3::fl::uintTI, 
        &AS3::fl::uintTI, 
        &AS3::fl::uintTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::StringTI, &AS3::fl::uintTI, 
    };
    const ThunkInfo IDataInput_ti[19] = {
        {ThunkInfo::EmptyFunc, &IDataInput_tit[0], "bytesAvailable", "flash.utils:IDataInput", Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &IDataInput_tit[1], "endian", "flash.utils:IDataInput", Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &IDataInput_tit[2], "endian", "flash.utils:IDataInput", Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &IDataInput_tit[4], "objectEncoding", "flash.utils:IDataInput", Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &IDataInput_tit[5], "objectEncoding", "flash.utils:IDataInput", Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &IDataInput_tit[7], "readBoolean", "flash.utils:IDataInput", Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &IDataInput_tit[8], "readByte", "flash.utils:IDataInput", Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &IDataInput_tit[9], "readBytes", "flash.utils:IDataInput", Abc::NS_Public, CT_Method, 1, 3, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &IDataInput_tit[13], "readDouble", "flash.utils:IDataInput", Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &IDataInput_tit[14], "readFloat", "flash.utils:IDataInput", Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &IDataInput_tit[15], "readInt", "flash.utils:IDataInput", Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &IDataInput_tit[16], "readMultiByte", "flash.utils:IDataInput", Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &IDataInput_tit[19], "readObject", "flash.utils:IDataInput", Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &IDataInput_tit[20], "readShort", "flash.utils:IDataInput", Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &IDataInput_tit[21], "readUnsignedByte", "flash.utils:IDataInput", Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &IDataInput_tit[22], "readUnsignedInt", "flash.utils:IDataInput", Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &IDataInput_tit[23], "readUnsignedShort", "flash.utils:IDataInput", Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &IDataInput_tit[24], "readUTF", "flash.utils:IDataInput", Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &IDataInput_tit[25], "readUTFBytes", "flash.utils:IDataInput", Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
    };

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_utils
{

    IDataInput::IDataInput(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::IDataInput::IDataInput()"
//##protect##"ClassTraits::IDataInput::IDataInput()"

    }

    Pickable<Traits> IDataInput::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) IDataInput(vm, AS3::fl_utils::IDataInputCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_utils::IDataInputCI));
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
    const TypeInfo IDataInputTI = {
        TypeInfo::CompileTime | TypeInfo::TypeInterface,
        sizeof(ClassTraits::fl_utils::IDataInput::InstanceType),
        0,
        0,
        19,
        0,
        "IDataInput", "flash.utils", NULL,
        TypeInfo::None
    };

    const ClassInfo IDataInputCI = {
        &IDataInputTI,
        ClassTraits::fl_utils::IDataInput::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_utils::IDataInput_ti,
        NULL,
    };
}; // namespace fl_utils


}}} // namespace Scaleform { namespace GFx { namespace AS3

