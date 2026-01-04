/**************************************************************************

Filename    :   AS3_AOTC2Execute.h
Content     :   
Created     :   Aug, 2012
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_AS3_AOTC2EXECUTE_H
#define INC_AS3_AOTC2EXECUTE_H

#include "AS3_VM.h"
#include "AS3_CodeExecute.h"
#include "Obj/AS3_Obj_Global.h"

namespace Scaleform { namespace GFx { namespace AS3
{

namespace AOT
{

    // Abc::NamespaceInfo cannot be initialized with initialization list.
    struct NamespaceInfo
    {
        Abc::NamespaceKind GetKind() const
        {
            return Kind;
        }
        const char* GetNameURI() const
        {
            return NameURI;
        }

        Abc::NamespaceKind  Kind;
        const char*         NameURI;
    };

    struct AOReg
    {
        UInt32              MBInd;
        const MemberInfo*   MI;
        UInt32              MINum;
    };

    const AOReg* GetAOReg(const UInt32 ind, const AOReg* t, const UInt32 size);

    struct ConstPool
    {
        UInt32                  NumStrings;
        UInt32                  NumNI;
        UInt32                  NumNSSOffset;
        UInt32                  NumNSS;
        UInt32                  NumMN;
        UInt32                  NumAORegTable;

        const char**            Strings;
        const NamespaceInfo*    NI;
        const UInt32*           NSSOffset;
        const UInt32*           NSS;
        const Abc::Multiname*   MN;
        const AOT::AOReg*       AORegTable;
    };

#if defined(SF_AS3_AOTC2)
    class File : public VMFile
    {
    public:
        File(VM& vm, VMAppDomain& appDomain, const ConstPool& cp);

    public:
        virtual const Abc::Multiname& GetMultiname(UPInt ind) const;

    protected:
        virtual Pickable<Instances::fl::Namespace> MakeInternedNamespace(const Abc::Multiname& mn) const;
        virtual Pickable<Instances::fl::Namespace> MakeInternedNamespace(UPInt nsIndex) const;
        virtual Pickable<NamespaceSet> MakeInternedNamespaceSet(UPInt nsSetIndex);
        virtual Pickable<InstanceTraits::Traits> MakeActivationInstanceTraits(const Abc::MbiInd mbi_ind SF_DEBUG_ARG(const ASString& name));
        virtual ASString MakeInternedString(UPInt strIndex) const;

    private:
        const ConstPool*    CP;
    };
#endif
}

namespace InstanceTraits { namespace fl
{

    ///////////////////////////////////////////////////////////////////////////
    class GlobalObjectAOT : public GlobalObject
    {
    public:
        GlobalObjectAOT(VM& vm, const SPtr<VMFile>& file, const MemberInfo mi[], UInt32 mi_size) 
            : GlobalObject(vm)
            , File(file)
        {
            // ???? !!!!!! Instance size shouldn't change.

            for (UInt32 i = 0; i < mi_size; ++i)
                AddSlot(mi[i]);
        }

    public:
        virtual void ForEachChild_GC(Collector* prcc, RefCountBaseGC<Mem_Stat>::GcOp op) const
        {
            GlobalObject::ForEachChild_GC(prcc, op);

            AS3::ForEachChild_GC<VMFile, Mem_Stat>(prcc, File, op SF_DEBUG_ARG(*this));
        }

        VMFile& GetFile() const
        {
            return *File;
        }

    private:
        GlobalObjectScript& operator =(const GlobalObjectScript&);

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::fl::GlobalObjectAOT"; }
#endif
    private:
        SPtr<VMFile> File;
    };
}} // namespace InstanceTraits { namespace fl

}}} // namespace Scaleform { namespace GFx { namespace AS3 {

#endif // INC_AS3_AOTC2EXECUTE_H

