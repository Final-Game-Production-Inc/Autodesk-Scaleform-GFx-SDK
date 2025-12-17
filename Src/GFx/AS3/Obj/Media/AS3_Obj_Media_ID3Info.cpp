//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Media_ID3Info.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Media_ID3Info.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Instances { namespace fl_media
{
    ID3Info::ID3Info(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
    , album(AS3::DefaultValue<ASString>(GetStringManager()))
    , artist(AS3::DefaultValue<ASString>(GetStringManager()))
    , comment(AS3::DefaultValue<ASString>(GetStringManager()))
    , genre(AS3::DefaultValue<ASString>(GetStringManager()))
    , songName(AS3::DefaultValue<ASString>(GetStringManager()))
    , track(AS3::DefaultValue<ASString>(GetStringManager()))
    , year(AS3::DefaultValue<ASString>(GetStringManager()))
//##protect##"instance::ID3Info::ID3Info()$data"
//##protect##"instance::ID3Info::ID3Info()$data"
    {
//##protect##"instance::ID3Info::ID3Info()$code"
//##protect##"instance::ID3Info::ID3Info()$code"
    }


//##protect##"instance$methods"
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_media
{
    const MemberInfo ID3Info::mi[ID3Info::MemberInfoNum] = {
        {"album", NULL, OFFSETOF(Instances::fl_media::ID3Info, album), Abc::NS_Public, SlotInfo::BT_String, 0},
        {"artist", NULL, OFFSETOF(Instances::fl_media::ID3Info, artist), Abc::NS_Public, SlotInfo::BT_String, 0},
        {"comment", NULL, OFFSETOF(Instances::fl_media::ID3Info, comment), Abc::NS_Public, SlotInfo::BT_String, 0},
        {"genre", NULL, OFFSETOF(Instances::fl_media::ID3Info, genre), Abc::NS_Public, SlotInfo::BT_String, 0},
        {"songName", NULL, OFFSETOF(Instances::fl_media::ID3Info, songName), Abc::NS_Public, SlotInfo::BT_String, 0},
        {"track", NULL, OFFSETOF(Instances::fl_media::ID3Info, track), Abc::NS_Public, SlotInfo::BT_String, 0},
        {"year", NULL, OFFSETOF(Instances::fl_media::ID3Info, year), Abc::NS_Public, SlotInfo::BT_String, 0},
    };


    ID3Info::ID3Info(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::ID3Info::ID3Info()"
//##protect##"InstanceTraits::ID3Info::ID3Info()"

    }

    void ID3Info::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<ID3Info&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_media
{

    ID3Info::ID3Info(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::ID3Info::ID3Info()"
//##protect##"ClassTraits::ID3Info::ID3Info()"

    }

    Pickable<Traits> ID3Info::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) ID3Info(vm, AS3::fl_media::ID3InfoCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_media::ID3InfoCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_media
{
    const TypeInfo ID3InfoTI = {
        TypeInfo::CompileTime | TypeInfo::DynamicObject | TypeInfo::Final | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_media::ID3Info::InstanceType),
        0,
        0,
        0,
        InstanceTraits::fl_media::ID3Info::MemberInfoNum,
        "ID3Info", "flash.media", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo ID3InfoCI = {
        &ID3InfoTI,
        ClassTraits::fl_media::ID3Info::MakeClassTraits,
        NULL,
        NULL,
        NULL,
        InstanceTraits::fl_media::ID3Info::mi,
    };
}; // namespace fl_media


}}} // namespace Scaleform { namespace GFx { namespace AS3

