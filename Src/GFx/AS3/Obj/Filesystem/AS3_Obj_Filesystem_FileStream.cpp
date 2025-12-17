//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Filesystem_FileStream.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Filesystem_FileStream.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "AS3_Obj_Filesystem_File.h"
#include "../Utils/AS3_Obj_Utils_ByteArray.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace InstanceTraits { namespace fl_filesystem
{
    // const UInt16 FileStream_tito[39] = {
    //    0, 1, 2, 4, 5, 7, 8, 10, 11, 13, 14, 17, 20, 21, 22, 26, 27, 28, 29, 32, 33, 34, 35, 36, 37, 38, 40, 41, 43, 45, 49, 51, 53, 55, 58, 60, 62, 64, 66, 
    // };
    const TypeInfo* FileStream_tit[68] = {
        &AS3::fl::uintTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl::uintTI, 
        NULL, &AS3::fl::uintTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        NULL, 
        NULL, &AS3::fl_filesystem::FileTI, &AS3::fl::StringTI, 
        NULL, &AS3::fl_filesystem::FileTI, &AS3::fl::StringTI, 
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
        NULL, 
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
    const ThunkInfo FileStream_ti[39] = {
        {ThunkInfo::EmptyFunc, &FileStream_tit[0], "bytesAvailable", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileStream_tit[1], "endian", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileStream_tit[2], "endian", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileStream_tit[4], "objectEncoding", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileStream_tit[5], "objectEncoding", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileStream_tit[7], "position", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileStream_tit[8], "position", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileStream_tit[10], "readAhead", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileStream_tit[11], "readAhead", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileStream_tit[13], "close", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileStream_tit[14], "open", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileStream_tit[17], "openAsync", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileStream_tit[20], "readBoolean", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileStream_tit[21], "readByte", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileStream_tit[22], "readBytes", NULL, Abc::NS_Public, CT_Method, 1, 3, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileStream_tit[26], "readDouble", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileStream_tit[27], "readFloat", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileStream_tit[28], "readInt", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileStream_tit[29], "readMultiByte", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileStream_tit[32], "readObject", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileStream_tit[33], "readShort", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileStream_tit[34], "readUnsignedByte", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileStream_tit[35], "readUnsignedInt", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileStream_tit[36], "readUnsignedShort", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileStream_tit[37], "readUTF", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileStream_tit[38], "readUTFBytes", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileStream_tit[40], "truncate", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileStream_tit[41], "writeBoolean", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileStream_tit[43], "writeByte", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileStream_tit[45], "writeBytes", NULL, Abc::NS_Public, CT_Method, 1, 3, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileStream_tit[49], "writeDouble", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileStream_tit[51], "writeFloat", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileStream_tit[53], "writeInt", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileStream_tit[55], "writeMultiByte", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileStream_tit[58], "writeObject", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileStream_tit[60], "writeShort", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileStream_tit[62], "writeUnsignedInt", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileStream_tit[64], "writeUTF", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &FileStream_tit[66], "writeUTFBytes", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
    };

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_filesystem
{

    FileStream::FileStream(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"ClassTraits::FileStream::FileStream()"
//##protect##"ClassTraits::FileStream::FileStream()"

    }

    Pickable<Traits> FileStream::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) FileStream(vm, AS3::fl_filesystem::FileStreamCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_filesystem::FileStreamCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_filesystem
{
    const TypeInfo* FileStreamImplements[] = {
        &fl_utils::IDataInputTI,
        &fl_utils::IDataOutputTI,
        NULL
    };

    const TypeInfo FileStreamTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_filesystem::FileStream::InstanceType),
        0,
        0,
        39,
        0,
        "FileStream", "flash.filesystem", &fl_events::EventDispatcherTI,
        FileStreamImplements
    };

    const ClassInfo FileStreamCI = {
        &FileStreamTI,
        ClassTraits::fl_filesystem::FileStream::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_filesystem::FileStream_ti,
        NULL,
    };
}; // namespace fl_filesystem


}}} // namespace Scaleform { namespace GFx { namespace AS3

