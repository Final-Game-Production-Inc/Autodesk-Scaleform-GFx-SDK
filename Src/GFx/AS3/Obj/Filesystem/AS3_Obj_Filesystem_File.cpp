//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Filesystem_File.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Filesystem_File.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../AS3_Obj_Array.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
namespace Classes
{
    class Icon;
}
//##protect##"methods"

namespace InstanceTraits { namespace fl_filesystem
{
    // const UInt16 File_tito[33] = {
    //    0, 1, 2, 3, 4, 5, 6, 7, 9, 10, 11, 13, 15, 18, 21, 23, 24, 25, 26, 29, 32, 33, 35, 37, 38, 39, 40, 41, 44, 47, 50, 51, 52, 
    // };
    const TypeInfo* File_tit[54] = {
        &AS3::fl::BooleanTI, 
        &AS3::fl_desktop::IconTI, 
        &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl_filesystem::FileTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, &AS3::fl::ArrayTI, 
        NULL, &AS3::fl::StringTI, &AS3::fl::ArrayTI, 
        NULL, &AS3::fl::StringTI, 
        NULL, 
        NULL, 
        &AS3::fl_filesystem::FileTI, 
        NULL, &AS3::fl_net::FileReferenceTI, &AS3::fl::BooleanTI, 
        NULL, &AS3::fl_net::FileReferenceTI, &AS3::fl::BooleanTI, 
        NULL, 
        NULL, &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        NULL, 
        NULL, 
        &AS3::fl::ArrayTI, 
        NULL, 
        &AS3::fl::StringTI, &AS3::fl_net::FileReferenceTI, &AS3::fl::BooleanTI, 
        NULL, &AS3::fl_net::FileReferenceTI, &AS3::fl::BooleanTI, 
        NULL, &AS3::fl_net::FileReferenceTI, &AS3::fl::BooleanTI, 
        NULL, 
        NULL, 
        &AS3::fl_filesystem::FileTI, &AS3::fl::StringTI, 
    };
    const ThunkInfo File_ti[33] = {
        {ThunkInfo::EmptyFunc, &File_tit[0], "exists", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &File_tit[1], "icon", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &File_tit[2], "isDirectory", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &File_tit[3], "isHidden", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &File_tit[4], "isPackage", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &File_tit[5], "isSymbolicLink", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &File_tit[6], "nativePath", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &File_tit[7], "nativePath", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &File_tit[9], "parent", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &File_tit[10], "url", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &File_tit[11], "url", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &File_tit[13], "browseForDirectory", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &File_tit[15], "browseForOpen", NULL, Abc::NS_Public, CT_Method, 1, 2, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &File_tit[18], "browseForOpenMultiple", NULL, Abc::NS_Public, CT_Method, 1, 2, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &File_tit[21], "browseForSave", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &File_tit[23], "cancel", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &File_tit[24], "canonicalize", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &File_tit[25], "clone", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &File_tit[26], "copyTo", NULL, Abc::NS_Public, CT_Method, 1, 2, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &File_tit[29], "copyToAsync", NULL, Abc::NS_Public, CT_Method, 1, 2, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &File_tit[32], "createDirectory", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &File_tit[33], "deleteDirectory", NULL, Abc::NS_Public, CT_Method, 0, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &File_tit[35], "deleteDirectoryAsync", NULL, Abc::NS_Public, CT_Method, 0, 1, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &File_tit[37], "deleteFile", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &File_tit[38], "deleteFileAsync", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &File_tit[39], "getDirectoryListing", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &File_tit[40], "getDirectoryListingAsync", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &File_tit[41], "getRelativePath", NULL, Abc::NS_Public, CT_Method, 1, 2, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &File_tit[44], "moveTo", NULL, Abc::NS_Public, CT_Method, 1, 2, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &File_tit[47], "moveToAsync", NULL, Abc::NS_Public, CT_Method, 1, 2, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &File_tit[50], "moveToTrash", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &File_tit[51], "moveToTrashAsync", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &File_tit[52], "resolvePath", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
    };

}} // namespace InstanceTraits

namespace Classes { namespace fl_filesystem
{
    File::File(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::File::File()"
//##protect##"class_::File::File()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes


namespace ClassTraits { namespace fl_filesystem
{
    // const UInt16 File_tito[7] = {
    //    0, 1, 2, 3, 4, 5, 6, 
    // };
    const TypeInfo* File_tit[7] = {
        &AS3::fl_filesystem::FileTI, 
        &AS3::fl_filesystem::FileTI, 
        &AS3::fl_filesystem::FileTI, 
        &AS3::fl_filesystem::FileTI, 
        &AS3::fl_filesystem::FileTI, 
        &AS3::fl_filesystem::FileTI, 
        &AS3::fl::ArrayTI, 
    };
    const ThunkInfo File_ti[7] = {
        {ThunkInfo::EmptyFunc, &File_tit[0], "applicationDirectory", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &File_tit[1], "applicationStorageDirectory", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &File_tit[2], "desktopDirectory", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &File_tit[3], "documentsDirectory", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &File_tit[4], "createTempDirectory", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &File_tit[5], "createTempFile", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &File_tit[6], "getRootDirectories", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    File::File(VM& vm, const ClassInfo& ci)
    : fl_net::FileReference(vm, ci)
    {
//##protect##"ClassTraits::File::File()"
//##protect##"ClassTraits::File::File()"

    }

    Pickable<Traits> File::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) File(vm, AS3::fl_filesystem::FileCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_filesystem::FileCI));
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
    const TypeInfo FileTI = {
        TypeInfo::CompileTime | TypeInfo::NotImplemented,
        sizeof(ClassTraits::fl_filesystem::File::InstanceType),
        7,
        0,
        33,
        0,
        "File", "flash.filesystem", &fl_net::FileReferenceTI,
        TypeInfo::None
    };

    const ClassInfo FileCI = {
        &FileTI,
        ClassTraits::fl_filesystem::File::MakeClassTraits,
        ClassTraits::fl_filesystem::File_ti,
        NULL,
        InstanceTraits::fl_filesystem::File_ti,
        NULL,
    };
}; // namespace fl_filesystem


}}} // namespace Scaleform { namespace GFx { namespace AS3

