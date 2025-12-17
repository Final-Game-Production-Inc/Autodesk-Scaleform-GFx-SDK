//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_External_ExternalInterface.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy, Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_External_ExternalInterface.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "Kernel/SF_Allocator.h"
#include "../../AS3_MovieRoot.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Classes { namespace fl_external
{
    ExternalInterface::ExternalInterface(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::ExternalInterface::ExternalInterface()"
//##protect##"class_::ExternalInterface::ExternalInterface()"
    }
    void ExternalInterface::availableGet(bool& result)
    {
//##protect##"class_::ExternalInterface::availableGet()"
      
        // [static] [read-only] Indicates whether this player is in a container that offers an 
        // external interface.

        // PPS: Since there is no concept of a container, we change the semantics of this 
        //      property to return true if an external interface handler exists.

        ASVM& vm = static_cast<ASVM&>(GetVM());

        MovieRoot* proot = vm.GetMovieRoot();
        SF_ASSERT(proot);
        MovieImpl* pmovie = proot->GetMovieImpl();

        result = (pmovie->pExtIntfHandler.GetPtr() != NULL);

//##protect##"class_::ExternalInterface::availableGet()"
    }
    void ExternalInterface::marshallExceptionsGet(bool& result)
    {
//##protect##"class_::ExternalInterface::marshallExceptionsGet()"

        // [static] Indicates whether the external interface should attempt to pass 
        // ActionScript exceptions to the current browser and JavaScript exceptions to Flash 
        // Player.

        // PPS: The semantics are not defined for GFx. We return false always.
        result = false;

        GFx::LogState* plog = static_cast<ASVM&>(GetVM()).GetLogState();
        if (plog) plog->LogWarning("ExternalInterface::marshallExceptions is not supported.");

//##protect##"class_::ExternalInterface::marshallExceptionsGet()"
    }
    void ExternalInterface::marshallExceptionsSet(const Value& result, bool value)
    {
//##protect##"class_::ExternalInterface::marshallExceptionsSet()"

        // [static] Indicates whether the external interface should attempt to pass 
        // ActionScript exceptions to the current browser and JavaScript exceptions to Flash 
        // Player.

        // PPS: The semantics are not defined for GFx. We disregard setting this property.
        SF_UNUSED2(result, value);

        GFx::LogState* plog = static_cast<ASVM&>(GetVM()).GetLogState();
        if (plog) plog->LogWarning("ExternalInterface::marshallExceptions is not supported.");

//##protect##"class_::ExternalInterface::marshallExceptionsSet()"
    }
    void ExternalInterface::objectIDGet(ASString& result)
    {
//##protect##"class_::ExternalInterface::objectIDGet()"

        // [static] [read-only] Returns the id attribute of the object tab in Internet 
        // Explorer, or the name attribute of the embed tab in Netscape.

        // PPS: The semantics are not defined for GFx. We return an empty string.
        result = GetVM().GetStringManager().GetBuiltin(AS3Builtin_null);

        GFx::LogState* plog = static_cast<ASVM&>(GetVM()).GetLogState();
        if (plog) plog->LogWarning("ExternalInterface::objectID is not supported.");

//##protect##"class_::ExternalInterface::objectIDGet()"
    }
    void ExternalInterface::addCallback(const Value& result, const ASString& functionName, const Value& closure)
    {
//##protect##"class_::ExternalInterface::addCallback()"

        // [static] Registers an ActionScript method as callbale form the container.

        SF_UNUSED1(result);

        ASVM& vm = static_cast<ASVM&>(GetVM());
        MovieRoot* proot = vm.GetMovieRoot();
        SF_ASSERT(proot);

        proot->AddInvokeAlias(functionName, closure);

//##protect##"class_::ExternalInterface::addCallback()"
    }
    void ExternalInterface::call(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"class_::ExternalInterface::call()"

        // [static] Calls a function exposed by the Flash Player container, passing zero or 
        // more arguments.

        ASVM& vm = static_cast<ASVM&>(GetVM());
        MovieRoot* proot = vm.GetMovieRoot();
        SF_ASSERT(proot);
        MovieImpl* pmovie = proot->GetMovieImpl();

        if (pmovie->pExtIntfHandler)
        {
            unsigned nArgs = 0;
            ASString methodName(GetStringManager().GetBuiltin(AS3Builtin_empty_));
            if (argc >= 1)
            {
                if (!argv[0].Convert2String(methodName))
                    // Exception.
                    return;

                nArgs = argc - 1;
            }

            // AMP
#ifdef SF_AMP_SERVER
            ASString fullMethodName(GetStringManager().GetBuiltin(AS3Builtin_empty_));
            fullMethodName = "ExternalInterface::call(";
            fullMethodName += methodName;
            fullMethodName += ")";
            SF_AMP_SCOPE_TIMER(pmovie->AdvanceStats, fullMethodName.ToCStr(), Amp_Profile_Level_Medium);
#endif

            // convert arguments into GFxValue, starting from index 1
            enum { NumValuesOnStack = 10 };
            void* argArrayOnStack[NumValuesOnStack*((sizeof(GFx::Value)+sizeof(void*)-1)/sizeof(void*))];
            GFx::Value* pargArray;
            if (nArgs > NumValuesOnStack)
                pargArray = (GFx::Value*)
                SF_HEAP_AUTO_ALLOC_ID(this, sizeof(GFx::Value) * nArgs, GFx::StatMV_ActionScript_Mem);
            else
                pargArray = (GFx::Value*)argArrayOnStack;

            // convert params to GFxValue array
            for (unsigned i = 0, n = nArgs; i < n; ++i)
            {
                const Value& val = argv[i + 1];
                GFx::Value* pdestVal = Scaleform::Construct<GFx::Value>(&pargArray[i]);
                proot->ASValue2GFxValue(val, pdestVal);
            }

            proot->ExternalIntfRetVal.SetUndefined();

            pmovie->pExtIntfHandler->Callback
                (pmovie, (methodName.IsEmpty()) ? NULL : methodName.ToCStr(), pargArray, nArgs);

            result = proot->ExternalIntfRetVal;

            // destruct elements in GFxValue array
            for (unsigned i = 0, n = nArgs; i < n; ++i)
            {
                pargArray[i].~Value();
            }
            if (nArgs > NumValuesOnStack)
            {
                SF_FREE(pargArray);
            }
        }
        else
        {
            vm.GetUI().Output(FlashUI::Output_Warning, "Warning: ExternalInterface.call - handler is not installed.\n");
            result.SetUndefined();
        }

//##protect##"class_::ExternalInterface::call()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes

typedef ThunkFunc0<Classes::fl_external::ExternalInterface, Classes::fl_external::ExternalInterface::mid_availableGet, bool> TFunc_Classes_ExternalInterface_availableGet;
typedef ThunkFunc0<Classes::fl_external::ExternalInterface, Classes::fl_external::ExternalInterface::mid_marshallExceptionsGet, bool> TFunc_Classes_ExternalInterface_marshallExceptionsGet;
typedef ThunkFunc1<Classes::fl_external::ExternalInterface, Classes::fl_external::ExternalInterface::mid_marshallExceptionsSet, const Value, bool> TFunc_Classes_ExternalInterface_marshallExceptionsSet;
typedef ThunkFunc0<Classes::fl_external::ExternalInterface, Classes::fl_external::ExternalInterface::mid_objectIDGet, ASString> TFunc_Classes_ExternalInterface_objectIDGet;
typedef ThunkFunc2<Classes::fl_external::ExternalInterface, Classes::fl_external::ExternalInterface::mid_addCallback, const Value, const ASString&, const Value&> TFunc_Classes_ExternalInterface_addCallback;
typedef ThunkFunc2<Classes::fl_external::ExternalInterface, Classes::fl_external::ExternalInterface::mid_call, Value, unsigned, const Value*> TFunc_Classes_ExternalInterface_call;

template <> const TFunc_Classes_ExternalInterface_availableGet::TMethod TFunc_Classes_ExternalInterface_availableGet::Method = &Classes::fl_external::ExternalInterface::availableGet;
template <> const TFunc_Classes_ExternalInterface_marshallExceptionsGet::TMethod TFunc_Classes_ExternalInterface_marshallExceptionsGet::Method = &Classes::fl_external::ExternalInterface::marshallExceptionsGet;
template <> const TFunc_Classes_ExternalInterface_marshallExceptionsSet::TMethod TFunc_Classes_ExternalInterface_marshallExceptionsSet::Method = &Classes::fl_external::ExternalInterface::marshallExceptionsSet;
template <> const TFunc_Classes_ExternalInterface_objectIDGet::TMethod TFunc_Classes_ExternalInterface_objectIDGet::Method = &Classes::fl_external::ExternalInterface::objectIDGet;
template <> const TFunc_Classes_ExternalInterface_addCallback::TMethod TFunc_Classes_ExternalInterface_addCallback::Method = &Classes::fl_external::ExternalInterface::addCallback;
template <> const TFunc_Classes_ExternalInterface_call::TMethod TFunc_Classes_ExternalInterface_call::Method = &Classes::fl_external::ExternalInterface::call;

namespace ClassTraits { namespace fl_external
{
    // const UInt16 ExternalInterface::tito[ExternalInterface::ThunkInfoNum] = {
    //    0, 1, 2, 4, 5, 8, 
    // };
    const TypeInfo* ExternalInterface::tit[10] = {
        &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::BooleanTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, &AS3::fl::FunctionTI, 
        NULL, &AS3::fl::StringTI, 
    };
    const ThunkInfo ExternalInterface::ti[ExternalInterface::ThunkInfoNum] = {
        {TFunc_Classes_ExternalInterface_availableGet::Func, &ExternalInterface::tit[0], "available", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Classes_ExternalInterface_marshallExceptionsGet::Func, &ExternalInterface::tit[1], "marshallExceptions", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Classes_ExternalInterface_marshallExceptionsSet::Func, &ExternalInterface::tit[2], "marshallExceptions", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Classes_ExternalInterface_objectIDGet::Func, &ExternalInterface::tit[4], "objectID", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Classes_ExternalInterface_addCallback::Func, &ExternalInterface::tit[5], "addCallback", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Classes_ExternalInterface_call::Func, &ExternalInterface::tit[8], "call", NULL, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1, 0, NULL},
    };

    ExternalInterface::ExternalInterface(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::ExternalInterface::ExternalInterface()"
//##protect##"ClassTraits::ExternalInterface::ExternalInterface()"

    }

    Pickable<Traits> ExternalInterface::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) ExternalInterface(vm, AS3::fl_external::ExternalInterfaceCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_external::ExternalInterfaceCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_external
{
    const TypeInfo ExternalInterfaceTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_external::ExternalInterface::InstanceType),
        ClassTraits::fl_external::ExternalInterface::ThunkInfoNum,
        0,
        0,
        0,
        "ExternalInterface", "flash.external", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo ExternalInterfaceCI = {
        &ExternalInterfaceTI,
        ClassTraits::fl_external::ExternalInterface::MakeClassTraits,
        ClassTraits::fl_external::ExternalInterface::ti,
        NULL,
        NULL,
        NULL,
    };
}; // namespace fl_external


}}} // namespace Scaleform { namespace GFx { namespace AS3

