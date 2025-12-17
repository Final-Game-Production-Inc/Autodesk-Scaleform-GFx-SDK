//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Net_LocalConnection.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Net_LocalConnection.h"
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
    // const UInt16 LocalConnection_tito[8] = {
    //    0, 1, 3, 4, 5, 6, 7, 9, 
    // };
    const TypeInfo* LocalConnection_tit[12] = {
        &AS3::fl::ObjectTI, 
        NULL, &AS3::fl::ObjectTI, 
        &AS3::fl::StringTI, 
        NULL, 
        NULL, 
        NULL, 
        NULL, &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, &AS3::fl::StringTI, 
    };
    const ThunkInfo LocalConnection_ti[8] = {
        {ThunkInfo::EmptyFunc, &LocalConnection_tit[0], "client", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &LocalConnection_tit[1], "client", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &LocalConnection_tit[3], "domain", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &LocalConnection_tit[4], "allowDomain", NULL, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1, 0, NULL},
        {ThunkInfo::EmptyFunc, &LocalConnection_tit[5], "allowInsecureDomain", NULL, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1, 0, NULL},
        {ThunkInfo::EmptyFunc, &LocalConnection_tit[6], "close", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &LocalConnection_tit[7], "connect", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &LocalConnection_tit[9], "send", NULL, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1, 0, NULL},
    };

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_net
{

    LocalConnection::LocalConnection(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"ClassTraits::LocalConnection::LocalConnection()"
//##protect##"ClassTraits::LocalConnection::LocalConnection()"

    }

    Pickable<Traits> LocalConnection::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) LocalConnection(vm, AS3::fl_net::LocalConnectionCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_net::LocalConnectionCI));
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
    const TypeInfo LocalConnectionTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_net::LocalConnection::InstanceType),
        0,
        0,
        8,
        0,
        "LocalConnection", "flash.net", &fl_events::EventDispatcherTI,
        TypeInfo::None
    };

    const ClassInfo LocalConnectionCI = {
        &LocalConnectionTI,
        ClassTraits::fl_net::LocalConnection::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_net::LocalConnection_ti,
        NULL,
    };
}; // namespace fl_net


}}} // namespace Scaleform { namespace GFx { namespace AS3

