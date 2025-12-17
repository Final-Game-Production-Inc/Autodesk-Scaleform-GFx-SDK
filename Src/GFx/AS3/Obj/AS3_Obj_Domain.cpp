//##protect##"disclaimer"
/**********************************************************************

Filename    :   .cpp
Content     :   
Created     :   Sep, 2013
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2013 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**********************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Domain.h"
#include "../AS3_VM.h"
#include "../AS3_Marshalling.h"
//##protect##"includes"
#include "../../../Kernel/SF_SysFile.h"
#include "Utils/AS3_Obj_Utils_ByteArray.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc1<Instances::fl::Domain, Instances::fl::Domain::mid_loadBytes, const Value, Instances::fl_utils::ByteArray*> TFunc_Instances_Domain_loadBytes;
typedef ThunkFunc0<Instances::fl::Domain, Instances::fl::Domain::mid_parentDomainGet, SPtr<Instances::fl::Domain> > TFunc_Instances_Domain_parentDomainGet;
typedef ThunkFunc1<Instances::fl::Domain, Instances::fl::Domain::mid_getClass, Value, const ASString&> TFunc_Instances_Domain_getClass;
typedef ThunkFunc1<Instances::fl::Domain, Instances::fl::Domain::mid_load, bool, const ASString&> TFunc_Instances_Domain_load;

template <> const TFunc_Instances_Domain_loadBytes::TMethod TFunc_Instances_Domain_loadBytes::Method = &Instances::fl::Domain::loadBytes;
template <> const TFunc_Instances_Domain_parentDomainGet::TMethod TFunc_Instances_Domain_parentDomainGet::Method = &Instances::fl::Domain::parentDomainGet;
template <> const TFunc_Instances_Domain_getClass::TMethod TFunc_Instances_Domain_getClass::Method = &Instances::fl::Domain::getClass;
template <> const TFunc_Instances_Domain_load::TMethod TFunc_Instances_Domain_load::Method = &Instances::fl::Domain::load;

namespace Instances { namespace fl
{
    Domain::Domain(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
//##protect##"instance::Domain::Domain()$data"
    , VMDomain(&GetVM().GetFrameAppDomain())
//##protect##"instance::Domain::Domain()$data"
    {
//##protect##"instance::Domain::Domain()$code"
//##protect##"instance::Domain::Domain()$code"
    }

    void Domain::loadBytes(const Value& result, Instances::fl_utils::ByteArray* byteArray)
    {
//##protect##"instance::Domain::loadBytes()"
        SF_UNUSED1(result);
        // Reader should be created on heap.
        UInt32 size = byteArray->GetLength();
        FileData.Resize(size);
        memcpy(FileData.GetDataPtr(), byteArray->GetDataPtr(), size);
        AutoPtr<Abc::Reader> reader(new Abc::Reader(static_cast<const UInt8*>(FileData.GetDataPtr()), size));
        Ptr<Abc::File> pfile = *SF_HEAP_AUTO_NEW(this) Abc::File();

        pfile->SetSource("ByteArray");
        pfile->SetDataSize(size);

        const bool ok = reader->Read(*pfile);
        if (ok)
            GetVM().LoadFile(pfile, *VMDomain);
        else
            FileData.Clear();
//##protect##"instance::Domain::loadBytes()"
    }
    void Domain::parentDomainGet(SPtr<Instances::fl::Domain>& result)
    {
//##protect##"instance::Domain::parentDomainGet()"
        VMAppDomain* parentDomain = VMDomain->GetParent();
        if (parentDomain == NULL)
            result = NULL;
        else
        {
            InstanceTraits::fl::Domain& tr = static_cast<InstanceTraits::fl::Domain&>(GetInstanceTraits());
            result = tr.MakeInstance(tr);
            result->VMDomain = parentDomain;
        }
//##protect##"instance::Domain::parentDomainGet()"
    }
    void Domain::getClass(Value& result, const ASString& name)
    {
//##protect##"instance::Domain::getClass()"
        ClassTraits::Traits** classTraits = VMDomain->GetClassTrait(Multiname(GetVM(), name.GetBuffer()));
        result.SetNull();
        if (classTraits != NULL)
            result = &(*classTraits)->GetInstanceTraits().GetClass();
//##protect##"instance::Domain::getClass()"
    }
    void Domain::load(bool& result, const ASString& fileName)
    {
//##protect##"instance::Domain::load()"
        result = false;
        SysFile file;
        VM& vm = GetVM();

        if (fileName.IsNull())
            return GetVM().ThrowTypeError(VM::Error(VM::eNullArgumentError, vm SF_DEBUG_ARG("filename")));

        String name(String(fileName.ToCStr(), fileName.GetLength()));

        if (!name.HasAbsolutePath())
            name = GetPath() + name;

        if (file.Open(name))
        {
            const int size = file.GetLength();

            FileData.Resize(size);
            const int read = file.Read((UByte*)&FileData[0], size);

            if (read == size)
            {
                // Reader should be created on heap.
                AutoPtr<Abc::Reader> reader(new Abc::Reader(&FileData[0], size));
                Ptr<Abc::File> pfile = *SF_HEAP_AUTO_NEW(this) Abc::File();

                pfile->SetSource(fileName.ToCStr());
                pfile->SetDataSize(size);

                result = reader->Read(*pfile);
                if (result)
                    vm.LoadFile(pfile, *VMDomain);
            } 

            if (!result)
                FileData.Clear();
        }
        else
            vm.ThrowError(VM::Error(VM::eFileOpenError, GetVM() SF_DEBUG_ARG(fileName)));
//##protect##"instance::Domain::load()"
    }

//##protect##"instance$methods"
    void Domain::AS3Constructor(unsigned argc, const Value* argv)
    {
        VM& vm = GetVM();

        if (argc > 0 && argv[0].IsObject() && argv[0].GetObject())
        {
            // Argument is an Object.
            AS3::Object* obj = argv[0].GetObject();
            // Domain is a final class.
            if (&GetTraits() == &obj->GetTraits())
            {
                // Argument is of type Domain.
                Domain* parentDomain = static_cast<Domain*>(obj);
                VMDomain = parentDomain->VMDomain->AddNewChild(vm);

                const String& parenPath = parentDomain->GetPath();
                if (parenPath.IsEmpty())
                {
                    if (vm.GetCallStack().GetSize() > 0)
                    {
                        const String& fn = vm.GetCurrCallFrame().GetFile().GetAbcFile().GetSource();
                        Path = fn.GetPath();
                    }
                }
                else
                    // Inherit path from the parent domain.
                    Path = parenPath;

                return;
            }
        }

        // Fallback. There is no parent domain.
        VMDomain = vm.GetFrameAppDomain().AddNewChild(vm);

        if (vm.GetCallStack().GetSize() > 0)
        {
            const String& fn = vm.GetCurrCallFrame().GetFile().GetAbcFile().GetSource();
            Path = fn.GetPath();
        }
    }

    void Domain::ForEachChild_GC(Collector* prcc, GcOp op) const
    {
        Instances::fl::Object::ForEachChild_GC(prcc, op);
        AS3::ForEachChild_GC<VMAppDomain, Mem_Stat>(prcc, VMDomain, op SF_DEBUG_ARG(*this));
    }
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl
{
    // const UInt16 Domain::tito[Domain::ThunkInfoNum] = {
    //    0, 2, 3, 5, 
    // };
    const TypeInfo* Domain::tit[7] = {
        NULL, &AS3::fl_utils::ByteArrayTI, 
        &AS3::fl::DomainTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl::BooleanTI, &AS3::fl::StringTI, 
    };
    const ThunkInfo Domain::ti[Domain::ThunkInfoNum] = {
        {TFunc_Instances_Domain_loadBytes::Func, &Domain::tit[0], "loadBytes", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Domain_parentDomainGet::Func, &Domain::tit[2], "parentDomain", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Domain_getClass::Func, &Domain::tit[3], "getClass", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Domain_load::Func, &Domain::tit[5], "load", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
    };

    Domain::Domain(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::Domain::Domain()"
//##protect##"InstanceTraits::Domain::Domain()"

    }

    void Domain::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<Domain&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl
{
    Domain::Domain(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::Domain::Domain()"
//##protect##"class_::Domain::Domain()"
    }
    void Domain::currentDomainGet(SPtr<Instances::fl::Domain>& result)
    {
//##protect##"class_::Domain::currentDomainGet()"
        InstanceTraits::fl::Domain& itr = static_cast<InstanceTraits::fl::Domain&>(GetInstanceTraits());
        result = itr.MakeInstance(itr);
        result->VMDomain = &GetVM().GetCurrentAppDomain();
//##protect##"class_::Domain::currentDomainGet()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes

typedef ThunkFunc0<Classes::fl::Domain, Classes::fl::Domain::mid_currentDomainGet, SPtr<Instances::fl::Domain> > TFunc_Classes_Domain_currentDomainGet;

template <> const TFunc_Classes_Domain_currentDomainGet::TMethod TFunc_Classes_Domain_currentDomainGet::Method = &Classes::fl::Domain::currentDomainGet;

namespace ClassTraits { namespace fl
{
    // const UInt16 Domain::tito[Domain::ThunkInfoNum] = {
    //    0, 
    // };
    const TypeInfo* Domain::tit[1] = {
        &AS3::fl::DomainTI, 
    };
    const ThunkInfo Domain::ti[Domain::ThunkInfoNum] = {
        {TFunc_Classes_Domain_currentDomainGet::Func, &Domain::tit[0], "currentDomain", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
    };

    Domain::Domain(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::Domain::Domain()"
//##protect##"ClassTraits::Domain::Domain()"

    }

    Pickable<Traits> Domain::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) Domain(vm, AS3::fl::DomainCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl::DomainCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl
{
    const TypeInfo DomainTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl::Domain::InstanceType),
        ClassTraits::fl::Domain::ThunkInfoNum,
        0,
        InstanceTraits::fl::Domain::ThunkInfoNum,
        0,
        "Domain", "", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo DomainCI = {
        &DomainTI,
        ClassTraits::fl::Domain::MakeClassTraits,
        ClassTraits::fl::Domain::ti,
        NULL,
        InstanceTraits::fl::Domain::ti,
        NULL,
    };
}; // namespace fl


}}} // namespace Scaleform { namespace GFx { namespace AS3

