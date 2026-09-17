/**************************************************************************

Filename    :   AS3_AOTC2Execute.cpp
Content     :   
Created     :   Aug, 2012
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "AS3_AOTC2Execute.h"

namespace Scaleform { namespace GFx { namespace AS3 
{

///////////////////////////////////////////////////////////////////////
namespace AOT
{   
    const AOReg* GetAOReg(const UInt32 ind, const AOReg* t, const UInt32 size)
    {
        // Binary search.
        const AOT::AOReg* result = NULL;
        UInt32 min = 0;
        UInt32 max = size - 1;

        while (min <= max)
        {
            const UInt32 mid = (min + max) >> 1;
            const UInt32 curValue = t[mid].MBInd;

            if (curValue == ind) 
            {
                min = mid;
                break;
            } 
            else if (ind < curValue) 
                max = mid - 1;
            else 
                min = mid + 1;
        }

        const UInt32 ident = t[min].MBInd;

        if (ident == ind)
        {
            const UInt32 index = min;
            result = &t[index];
        }

        return result;
    }

#if defined(SF_AS3_AOTC2)
    ///////////////////////////////////////////////////////////////////////
    File::File(VM& vm, VMAppDomain& appDomain, const ConstPool& cp)
        : VMFile(vm, appDomain)
        , CP(&cp)
    {
    }

    const Abc::Multiname& File::GetMultiname(UPInt ind) const
    {
        SF_ASSERT(ind < CP->NumMN);
        return CP->MN[ind];
    }

    Pickable<Instances::fl::Namespace> File::MakeInternedNamespace(const Abc::Multiname& mn) const
    {
        SF_ASSERT(static_cast<UInt32>(mn.GetNamespaceInd()) < CP->NumNI);
        const NamespaceInfo& info = CP->NI[mn.GetNamespaceInd()];
        const char* uri = info.NameURI;
        VM& vm = GetVM();
        StringManager& sm = vm.GetStringManager();
        ASString URI = sm.CreateConstString(uri);

        if (mn.GetNextMultinameIndex() >= 0)
        {
            SF_ASSERT(static_cast<UInt32>(mn.GetNextMultinameIndex()) < CP->NumMN);
            const Abc::Multiname& next_mn = CP->MN[mn.GetNextMultinameIndex()];
            const NamespaceInfo& next_info = CP->NI[next_mn.GetNamespaceInd()];
            const char* next_uri = next_info.NameURI;

            URI += "$";
            URI += sm.CreateConstString(next_uri);
        }

        return vm.MakeInternedNamespace(info.GetKind(), URI);
    }

    Pickable<Instances::fl::Namespace> File::MakeInternedNamespace(UPInt nsIndex) const
    {
        SF_ASSERT(nsIndex < CP->NumNI);
        const NamespaceInfo& info = CP->NI[nsIndex];
        VM& vm = GetVM();
        const ASString URI = vm.GetStringManager().CreateConstString(info.GetNameURI());
        return vm.MakeInternedNamespace(info.GetKind(), URI);
    }

    Pickable<NamespaceSet> File::MakeInternedNamespaceSet(UPInt nsSetIndex)
    {
        SF_ASSERT(nsSetIndex > 0 && nsSetIndex <= CP->NumNSSOffset);
        --nsSetIndex;
        const Abc::UInd offset = CP->NSSOffset[nsSetIndex];
        const Abc::UInd size = CP->NSS[offset];

        Pickable<NamespaceSet> nss(SF_HEAP_NEW_ID(GetVM().GetMemoryHeap(), StatMV_VM_VMAbcFile_Mem) NamespaceSet(GetVM()));

        // Fill it up with data.
        for(Abc::UInd i = 1; i <= size; ++i)
        {
            const Abc::UInd ns_ind = CP->NSS[offset + i];
            nss->Add(GetInternedNamespace(ns_ind));
        }

        return nss;
    }

    Pickable<InstanceTraits::Traits> File::MakeActivationInstanceTraits(const Abc::MbiInd mbi_ind SF_DEBUG_ARG(const ASString& name))
    {
        SF_DEBUG_CODE(SF_UNUSED1(name));
        const AOT::AOReg* reg = AOT::GetAOReg(mbi_ind.Get(), CP->AORegTable, CP->NumAORegTable);
        SF_ASSERT(reg);
        return Pickable<InstanceTraits::Traits>(SF_NEW InstanceTraits::TraitsAO(GetVM(), reg->MI, reg->MINum)); 
    }

    ASString File::MakeInternedString(UPInt strIndex) const
    {
        SF_ASSERT(strIndex < CP->NumStrings);
        return GetVM().GetStringManager().CreateConstString(CP->Strings[strIndex]);
    }
#endif
}

    void VM::aot_newclass(Class& cl)
    {
        ClassTraits::Traits& ctr = cl.GetClassTraits();
        InstanceTraits::Traits& itr = ctr.GetInstanceTraits();

        // Alter ClassTraits to store scope stack.
        ctr.StoreScopeStack(GetScopeStackBaseInd(), GetScopeStack());

        // Alter InstanceTraits to store scope stack.
        itr.StoreScopeStack(GetScopeStackBaseInd(), GetScopeStack());

        Value& value = OpStack.Top0();
        value = &cl;

        // Magic logic.
        // Should be called only for instance traits.
        itr.Add2StoredScopeStack(value);
    }

}}} // namespace Scaleform { namespace GFx { namespace AS3 


