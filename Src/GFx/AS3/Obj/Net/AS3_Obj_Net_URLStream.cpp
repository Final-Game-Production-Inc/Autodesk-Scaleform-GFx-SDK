//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Net_URLStream.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Net_URLStream.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "AS3_Obj_Net_URLRequest.h"
#include "../Utils/AS3_Obj_Utils_ByteArray.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace InstanceTraits { namespace fl_net
{
    // const UInt16 URLStream_tito[22] = {
    //    0, 1, 2, 3, 5, 6, 8, 9, 11, 12, 13, 17, 18, 19, 20, 23, 24, 25, 26, 27, 28, 29, 
    // };
    const TypeInfo* URLStream_tit[31] = {
        &AS3::fl::uintTI, 
        &AS3::fl::BooleanTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl::uintTI, 
        NULL, &AS3::fl::uintTI, 
        NULL, 
        NULL, &AS3::fl_net::URLRequestTI, 
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
    const ThunkInfo URLStream_ti[22] = {
        {ThunkInfo::EmptyFunc, &URLStream_tit[0], "bytesAvailable", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &URLStream_tit[1], "connected", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &URLStream_tit[2], "endian", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &URLStream_tit[3], "endian", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &URLStream_tit[5], "objectEncoding", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &URLStream_tit[6], "objectEncoding", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &URLStream_tit[8], "close", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &URLStream_tit[9], "load", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &URLStream_tit[11], "readBoolean", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &URLStream_tit[12], "readByte", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &URLStream_tit[13], "readBytes", NULL, Abc::NS_Public, CT_Method, 1, 3, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &URLStream_tit[17], "readDouble", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &URLStream_tit[18], "readFloat", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &URLStream_tit[19], "readInt", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &URLStream_tit[20], "readMultiByte", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &URLStream_tit[23], "readObject", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &URLStream_tit[24], "readShort", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &URLStream_tit[25], "readUnsignedByte", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &URLStream_tit[26], "readUnsignedInt", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &URLStream_tit[27], "readUnsignedShort", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &URLStream_tit[28], "readUTF", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &URLStream_tit[29], "readUTFBytes", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
    };

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_net
{

    URLStream::URLStream(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"ClassTraits::URLStream::URLStream()"
//##protect##"ClassTraits::URLStream::URLStream()"

    }

    Pickable<Traits> URLStream::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) URLStream(vm, AS3::fl_net::URLStreamCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_net::URLStreamCI));
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
    const TypeInfo* URLStreamImplements[] = {
        &fl_utils::IDataInputTI,
        NULL
    };

    const TypeInfo URLStreamTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_net::URLStream::InstanceType),
        0,
        0,
        22,
        0,
        "URLStream", "flash.net", &fl_events::EventDispatcherTI,
        URLStreamImplements
    };

    const ClassInfo URLStreamCI = {
        &URLStreamTI,
        ClassTraits::fl_net::URLStream::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_net::URLStream_ti,
        NULL,
    };
}; // namespace fl_net


}}} // namespace Scaleform { namespace GFx { namespace AS3

