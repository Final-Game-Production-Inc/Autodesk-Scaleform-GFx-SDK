//##protect##"disclaimer"
/**********************************************************************

Filename    :   .cpp
Content     :   
Created     :   Jul, 2012
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**********************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_System_Domain.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../../../../Kernel/SF_SysFile.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

// Values of default arguments.
namespace Impl
{

} // namespace Impl
typedef ThunkFunc0<Instances::fl_system::Domain, Instances::fl_system::Domain::mid_parentDomainGet, SPtr<Instances::fl_system::Domain> > TFunc_Instances_Domain_parentDomainGet;
typedef ThunkFunc1<Instances::fl_system::Domain, Instances::fl_system::Domain::mid_getClass, Value, const ASString&> TFunc_Instances_Domain_getClass;
typedef ThunkFunc1<Instances::fl_system::Domain, Instances::fl_system::Domain::mid_load, bool, const ASString&> TFunc_Instances_Domain_load;

template <> const TFunc_Instances_Domain_parentDomainGet::TMethod TFunc_Instances_Domain_parentDomainGet::Method = &Instances::fl_system::Domain::parentDomainGet;
template <> const TFunc_Instances_Domain_getClass::TMethod TFunc_Instances_Domain_getClass::Method = &Instances::fl_system::Domain::getClass;
template <> const TFunc_Instances_Domain_load::TMethod TFunc_Instances_Domain_load::Method = &Instances::fl_system::Domain::load;

namespace Instances { namespace fl_system
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

    void Domain::parentDomainGet(SPtr<Instances::fl_system::Domain>& result)
    {
//##protect##"instance::Domain::parentDomainGet()"
        VMAppDomain* parentDomain = VMDomain->GetParent();
        if (parentDomain == NULL)
        {
            result = NULL;
        }
        else
        {
            InstanceTraits::fl_system::Domain& tr = static_cast<InstanceTraits::fl_system::Domain&>(GetInstanceTraits());
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
        {
            result = &(*classTraits)->GetInstanceTraits().GetClass();
        }
//##protect##"instance::Domain::getClass()"
    }
    void Domain::load(bool& result, const ASString& fileName)
    {
//##protect##"instance::Domain::load()"
        result = false;
        SysFile file;

        if (file.Open(fileName.ToCStr()))
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
                {
                    VM& vm = GetVM();
                    vm.LoadFile(pfile, *VMDomain);
                    vm.AddFile(pfile);
                }
            } 
        }
        else
        {
            GetVM().ThrowError(VM::Error(VM::eFileOpenError, GetVM() SF_DEBUG_ARG("Unable to load file")));
        }
//##protect##"instance::Domain::load()"
    }

//##protect##"instance$methods"
    void Domain::AS3Constructor(unsigned argc, const Value* argv)
    {
        VM& vm = GetVM();
        if (argc == 0 || argv[0].IsNullOrUndefined())
        {
            VMDomain = &vm.GetFrameAppDomain().AddNewChild(vm);
        }
        else
        {
            Domain& parentDomain = static_cast<Domain&>(*argv[0].GetObject());
            VMDomain = &parentDomain.VMDomain->AddNewChild(vm);
        }
    }
//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_system
{
    const ThunkInfo Domain::ti[Domain::ThunkInfoNum] = {
        {TFunc_Instances_Domain_parentDomainGet::Func, &AS3::fl_system::DomainTI, "parentDomain", NULL, Abc::NS_Public, CT_Get, 0, 0},
        {TFunc_Instances_Domain_getClass::Func, NULL, "getClass", NULL, Abc::NS_Public, CT_Method, 1, 1},
        {TFunc_Instances_Domain_load::Func, &AS3::fl::BooleanTI, "load", NULL, Abc::NS_Public, CT_Method, 1, 1},
    };

    Domain::Domain(VM& vm, const ClassInfo& ci)
    : CTraits(vm, ci)
    {
//##protect##"InstanceTraits::Domain::Domain()"
//##protect##"InstanceTraits::Domain::Domain()"
        SetMemSize(sizeof(Instances::fl_system::Domain));

    }

    void Domain::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<Domain&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl_system
{
    Domain::Domain(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::Domain::Domain()"
//##protect##"class_::Domain::Domain()"
    }
    void Domain::currentDomainGet(SPtr<Instances::fl_system::Domain>& result)
    {
//##protect##"class_::Domain::currentDomainGet()"
        InstanceTraits::fl_system::Domain& tr = static_cast<InstanceTraits::fl_system::Domain&>(GetInstanceTraits());
        result = tr.MakeInstance(tr);
        result->VMDomain = &GetVM().GetCurrentAppDomain();
//##protect##"class_::Domain::currentDomainGet()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes

typedef ThunkFunc0<Classes::fl_system::Domain, Classes::fl_system::Domain::mid_currentDomainGet, SPtr<Instances::fl_system::Domain> > TFunc_Classes_Domain_currentDomainGet;

template <> const TFunc_Classes_Domain_currentDomainGet::TMethod TFunc_Classes_Domain_currentDomainGet::Method = &Classes::fl_system::Domain::currentDomainGet;

namespace ClassTraits { namespace fl_system
{
    const ThunkInfo Domain::ti[Domain::ThunkInfoNum] = {
        {TFunc_Classes_Domain_currentDomainGet::Func, &AS3::fl_system::DomainTI, "currentDomain", NULL, Abc::NS_Public, CT_Get, 0, 0},
    };
    Domain::Domain(VM& vm)
    : Traits(vm, AS3::fl_system::DomainCI)
    {
//##protect##"ClassTraits::Domain::Domain()"
//##protect##"ClassTraits::Domain::Domain()"
        MemoryHeap* mh = vm.GetMemoryHeap();

        Pickable<InstanceTraits::Traits> it(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraits::fl_system::Domain(vm, AS3::fl_system::DomainCI));
        SetInstanceTraits(it);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) Classes::fl_system::Domain(*this));

    }

    Pickable<Traits> Domain::MakeClassTraits(VM& vm)
    {
        return Pickable<Traits>(SF_HEAP_NEW_ID(vm.GetMemoryHeap(), StatMV_VM_CTraits_Mem) Domain(vm));
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_system
{
    const TypeInfo DomainTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        "Domain", "flash.system", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo DomainCI = {
        &DomainTI,
        ClassTraits::fl_system::Domain::MakeClassTraits,
        ClassTraits::fl_system::Domain::ThunkInfoNum,
        0,
        InstanceTraits::fl_system::Domain::ThunkInfoNum,
        0,
        ClassTraits::fl_system::Domain::ti,
        NULL,
        InstanceTraits::fl_system::Domain::ti,
        NULL,
    };
}; // namespace fl_system


}}} // namespace Scaleform { namespace GFx { namespace AS3

