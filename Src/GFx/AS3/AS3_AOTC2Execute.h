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
        const Abc::UInd*        NSSOffset;
        const Abc::UInd*        NSS;
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

    inline
    void VM::aot_throw() 
    { 
        // Get exception object from the stack ...
        SF_ASSERT(OpStack.GetSize() > 0);
        ExceptionObj.Pick(OpStack);
    }

    inline
    void VM::aot_dxns(VM::ResourceGuard& cf, Instances::fl::Namespace& ns) 
    {
        // Sets the default XML namespace.

        SF_UNUSED1(cf);

        // index is a u30 that must be an index into the string constant pool. The string at index is used
        // as the uri for the default XML namespace for this method.
        SetDefXMLNamespace(&ns);
    }

    inline
    bool VM::aot_nlt() 
    {
        SH2<2> stack(OpStack);
        // No exceptions at this point.
        Boolean3 result;

        return (AbstractLessThan(result, stack._1, stack._2) && result != true3);
    }

    inline
    bool VM::aot_nlt_ti() 
    {
        TSH2<2, SInt32, SInt32> stack(OpStack);

        return (stack._1 >= stack._2);
    }

    inline
    bool VM::aot_nlt_td() 
    {
        TSH2<2, Value::Number, Value::Number> stack(OpStack);

        return (stack._1 >= stack._2);
    }

    inline
    bool VM::aot_nle() 
    {
        SH2<2> stack(OpStack);
        // No exceptions at this point.
        Boolean3 result;

        return (AbstractLessThan(result, stack._2, stack._1) && result != false3);
    }

    inline
    bool VM::aot_nle_ti() 
    {
        TSH2<2, SInt32, SInt32> stack(OpStack);

        return (stack._1 > stack._2);
    }

    inline
    bool VM::aot_nle_td() 
    {
        TSH2<2, Value::Number, Value::Number> stack(OpStack);

        return (stack._1 > stack._2);
    }

    inline
    bool VM::aot_ngt() 
    {
        SH2<2> stack(OpStack);
        // No exceptions at this point.
        Boolean3 result;

        return (AbstractLessThan(result, stack._2, stack._1) && result != true3);
    }

    inline
    bool VM::aot_ngt_ti() 
    {
        TSH2<2, SInt32, SInt32> stack(OpStack);

        return (stack._1 <= stack._2);
    }

    inline
    bool VM::aot_ngt_td() 
    {
        TSH2<2, Value::Number, Value::Number> stack(OpStack);

        return (stack._1 <= stack._2);
    }

    inline
    bool VM::aot_nge() 
    {
        SH2<2> stack(OpStack);
        // No exceptions at this point.
        Boolean3 result;

        return (AbstractLessThan(result, stack._1, stack._2) && result != false3);
    }

    inline
    bool VM::aot_nge_ti() 
    {
        TSH2<2, SInt32, SInt32> stack(OpStack);

        return (stack._1 < stack._2);
    }

    inline
    bool VM::aot_nge_td() 
    {
        TSH2<2, Value::Number, Value::Number> stack(OpStack);

        return (stack._1 < stack._2);
    }

    inline
    bool VM::aot_true() 
    {
        SH1<1, bool> stack(OpStack);

        return (stack.IsOK() && stack._1 == true);
    }

    inline
    bool VM::aot_true_tb() 
    {
        TSH1<1, bool> stack(OpStack);

        return (stack._1 == true);
    }

    inline
    bool VM::aot_false()
    {
        SH1<1, bool> stack(OpStack);

        return (stack.IsOK() && stack._1 == false);
    }

    inline
    bool VM::aot_false_tb()
    {
        TSH1<1, bool> stack(OpStack);

        return (stack._1 == false);
    }

    inline
    bool VM::aot_eq() 
    {
        SH2<2> stack(OpStack);
        // No exceptions at this point.
        bool result;

        // Order of parameter evaluation is not important here ...
        return (AbstractEqual(result, stack._1, stack._2) && result);
    }

    inline
    bool VM::aot_eq_ti() 
    {
        TSH2<2, SInt32, SInt32> stack(OpStack);

        // Order of parameter evaluation is not important here ...
        return (stack._1 == stack._2);
    }

    inline
    bool VM::aot_eq_td() 
    {
        TSH2<2, Value::Number, Value::Number> stack(OpStack);

        // Order of parameter evaluation is not important here ...
        return (stack._1 == stack._2);
    }

    inline
    bool VM::aot_ne() 
    {
        SH2<2> stack(OpStack);
        // No exceptions at this point.
        bool result;

        // Order of parameter evaluation is not important here ...
        return (AbstractEqual(result, stack._1, stack._2) && !result);
    }

    inline
    bool VM::aot_ne_ti() 
    {
        TSH2<2, SInt32, SInt32> stack(OpStack);

        // Order of parameter evaluation is not important here ...
        return (stack._1 != stack._2);
    }

    inline
    bool VM::aot_ne_td() 
    {
        TSH2<2, Value::Number, Value::Number> stack(OpStack);

        // Order of parameter evaluation is not important here ...
        return (stack._1 != stack._2);
    }

    inline
    bool VM::aot_lt()
    {
        SH2<2> stack(OpStack);
        // No exceptions at this point.
        bool result;

        return (LessThan(result, stack._1, stack._2) && result);
    }

    inline
    bool VM::aot_lt_ti()
    {
        TSH2<2, SInt32, SInt32> stack(OpStack);

        return (stack._1 < stack._2);
    }

    inline
    bool VM::aot_lt_td()
    {
        TSH2<2, Value::Number, Value::Number> stack(OpStack);

        return (stack._1 < stack._2);
    }

    inline
    bool VM::aot_le()
    {
        SH2<2> stack(OpStack);
        // No exceptions at this point.
        bool result;

        return (LessThanOrEqual(result, stack._1, stack._2) && result);
    }

    inline
    bool VM::aot_le_ti()
    {
        TSH2<2, SInt32, SInt32> stack(OpStack);

        return (stack._1 <= stack._2);
    }

    inline
    bool VM::aot_le_td()
    {
        TSH2<2, Value::Number, Value::Number> stack(OpStack);

        return (stack._1 <= stack._2);
    }

    inline
    bool VM::aot_gt() 
    {
        SH2<2> stack(OpStack);
        // No exceptions at this point.
        bool result;

        return (GreaterThan(result, stack._1, stack._2) && result);
    }

    inline
    bool VM::aot_gt_ti()
    {
        TSH2<2, SInt32, SInt32> stack(OpStack);

        return (stack._1 > stack._2);
    }

    inline
    bool VM::aot_gt_td()
    {
        TSH2<2, Value::Number, Value::Number> stack(OpStack);

        return (stack._1 > stack._2);
    }

    inline
    bool VM::aot_ge() 
    {
        SH2<2> stack(OpStack);
        // No exceptions at this point.
        bool result;

        return (GreaterThanOrEqual(result, stack._1, stack._2) && result);
    }

    inline
    bool VM::aot_ge_ti()
    {
        TSH2<2, SInt32, SInt32> stack(OpStack);

        return (stack._1 >= stack._2);
    }

    inline
    bool VM::aot_ge_td()
    {
        TSH2<2, Value::Number, Value::Number> stack(OpStack);

        return (stack._1 >= stack._2);
    }

    inline
    bool VM::aot_stricteq() 
    {
        SH2<2> stack(OpStack);

        // Order of parameter evaluation is not important here ...
        return (StrictEqual(stack._1, stack._2));
    }

    inline
    bool VM::aot_strictne() 
    {
        SH2<2> stack(OpStack);

        // Order of parameter evaluation is not important here ...
        return (!StrictEqual(stack._1, stack._2));
    }

    inline
    void VM::aot_getglobalslot(Instances::fl::GlobalObject& go, UInt32 slot_index) 
    {
        // Get the value of a slot on the global scope.

        OpStack.PushBack(Value::GetUndefined());
        go.GetSlotValueUnsafe(SlotIndex(slot_index), OpStack.Top0()).DoNotCheck();
    }

    inline
    void VM::aot_setglobalslot(Instances::fl::GlobalObject& go, UInt32 slot_index) 
    {
        // Set the value of a slot on the global scope.

        go.SetSlotValue(SlotIndex(slot_index), OpStack.Top0()).DoNotCheck();
        // We will need to pop this value even in case of an exception.
        OpStack.PopBack();
    }

}}} // namespace Scaleform { namespace GFx { namespace AS3 {

#endif // INC_AS3_AOTC2EXECUTE_H

