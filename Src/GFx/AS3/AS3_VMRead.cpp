/**************************************************************************

Filename    :   AS3_VMRead.cpp
Content     :   
Created     :   September, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "AS3_VMRead.h"

namespace Scaleform { namespace GFx { namespace AS3 
{
    ///////////////////////////////////////////////////////////////////////////
    void StackReader::Read(Multiname& obj)
    {
        const Abc::MultinameKind k = obj.GetKind();

        switch (k)
        {
        case Abc::MN_QName:
        case Abc::MN_QNameA:
            // QName is not read from stack; both name and namespase known at compile-time.
            break;
        case Abc::MN_RTQName:
        case Abc::MN_RTQNameA:
            // Name is known at compile time, Namespace is resolved at runtime.

            // For maniacs.
            CheckObject(Back());
            if (GetVM().IsException())
                break;

            obj.SetRTNamespace(
                const_cast<Instances::fl::Namespace&>(Back().AsNamespace()));
            PopBack();

            break;
        case Abc::MN_RTQNameL:
        case Abc::MN_RTQNameLA:
            // Both name and namespace are resolved at runtime.
            obj.PickRTNameUnsafe(GetOpStack());

            // For maniacs.
            CheckObject(Back());
            if (GetVM().IsException())
                break;

            obj.SetRTNamespace(
                const_cast<Instances::fl::Namespace&>(Back().AsNamespace()));
            PopBack();

            break;
        case Abc::MN_Multiname:
        case Abc::MN_MultinameA:
            // MInfo is not supposed to be read from stack.
            break;
        case Abc::MN_MultinameL:
        case Abc::MN_MultinameLA:
            // Namespace set is known at compile time, Name is resolved at runtime.        
            // name_ind is a string, and we will get an int or an uint as an index instead;
            obj.PickRTNameUnsafe(GetOpStack());
            break;
        case Abc::MN_Typename:
            // Recursive!!!?
            GetVM().GetUI().Output(FlashUI::Output_Warning, "Reading chained multiname in itself.");
            Read(obj);
            break;
        default:
            SF_ASSERT(false);
            break;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    ReadArgs::ReadArgs( VM& vm, UInt32 arg_count ) : StackReader(vm)
        , ArgNum(arg_count)
        , CallArgs(vm.GetMemoryHeap())
    {
        if (arg_count)
        {
            VM::OpStackType& opstack = GetOpStack();

            if (arg_count <= eFixedArrSize)
            {
                memcpy(FixedArr, &opstack.TopN(static_cast<UInt16>(arg_count - 1)), arg_count * sizeof(Value));
            }
            else
            {
                CallArgs.Resize(arg_count);
                memcpy(CallArgs.GetDataPtr(), &opstack.TopN(static_cast<UInt16>(arg_count - 1)), arg_count * sizeof(Value));
            }

            opstack.ReleaseN(arg_count);
        }
    }


}}} // namespace Scaleform { namespace GFx { namespace AS3 {
