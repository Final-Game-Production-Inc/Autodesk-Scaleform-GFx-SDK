//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Filesystem_FileMode.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Filesystem_FileMode.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Classes { namespace fl_filesystem
{
    FileMode::FileMode(ClassTraits::Traits& t)
    : Class(t)
    , APPEND("append")
    , READ("read")
    , UPDATE("update")
    , WRITE("write")
    {
//##protect##"class_::FileMode::FileMode()"
//##protect##"class_::FileMode::FileMode()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_filesystem
{
    const MemberInfo FileMode::mi[FileMode::MemberInfoNum] = {
        {"APPEND", NULL, OFFSETOF(Classes::fl_filesystem::FileMode, APPEND), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"READ", NULL, OFFSETOF(Classes::fl_filesystem::FileMode, READ), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"UPDATE", NULL, OFFSETOF(Classes::fl_filesystem::FileMode, UPDATE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"WRITE", NULL, OFFSETOF(Classes::fl_filesystem::FileMode, WRITE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
    };


    FileMode::FileMode(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::FileMode::FileMode()"
//##protect##"ClassTraits::FileMode::FileMode()"

    }

    Pickable<Traits> FileMode::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) FileMode(vm, AS3::fl_filesystem::FileModeCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_filesystem::FileModeCI));
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
    const TypeInfo FileModeTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_filesystem::FileMode::InstanceType),
        0,
        ClassTraits::fl_filesystem::FileMode::MemberInfoNum,
        0,
        0,
        "FileMode", "flash.filesystem", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo FileModeCI = {
        &FileModeTI,
        ClassTraits::fl_filesystem::FileMode::MakeClassTraits,
        NULL,
        ClassTraits::fl_filesystem::FileMode::mi,
        NULL,
        NULL,
    };
}; // namespace fl_filesystem


}}} // namespace Scaleform { namespace GFx { namespace AS3

