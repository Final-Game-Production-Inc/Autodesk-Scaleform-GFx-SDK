//##protect##"disclaimer"
/**********************************************************************

Filename    :   .cpp
Content     :   
Created     :   Mar, 2013
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2013 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**********************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_External_ExtensionContext.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#ifdef SF_ENABLE_ANE
#include "GFx/AS3/AS3_MovieRoot.h"
#endif
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_external::ExtensionContext, Instances::fl_external::ExtensionContext::mid_actionScriptDataGet, SPtr<Instances::fl::Object> > TFunc_Instances_ExtensionContext_actionScriptDataGet;
typedef ThunkFunc1<Instances::fl_external::ExtensionContext, Instances::fl_external::ExtensionContext::mid_actionScriptDataSet, const Value, const Value&> TFunc_Instances_ExtensionContext_actionScriptDataSet;
typedef ThunkFunc2<Instances::fl_external::ExtensionContext, Instances::fl_external::ExtensionContext::mid_call, Value, unsigned, const Value*> TFunc_Instances_ExtensionContext_call;
typedef ThunkFunc0<Instances::fl_external::ExtensionContext, Instances::fl_external::ExtensionContext::mid_dispose, const Value> TFunc_Instances_ExtensionContext_dispose;

template <> const TFunc_Instances_ExtensionContext_actionScriptDataGet::TMethod TFunc_Instances_ExtensionContext_actionScriptDataGet::Method = &Instances::fl_external::ExtensionContext::actionScriptDataGet;
template <> const TFunc_Instances_ExtensionContext_actionScriptDataSet::TMethod TFunc_Instances_ExtensionContext_actionScriptDataSet::Method = &Instances::fl_external::ExtensionContext::actionScriptDataSet;
template <> const TFunc_Instances_ExtensionContext_call::TMethod TFunc_Instances_ExtensionContext_call::Method = &Instances::fl_external::ExtensionContext::call;
template <> const TFunc_Instances_ExtensionContext_dispose::TMethod TFunc_Instances_ExtensionContext_dispose::Method = &Instances::fl_external::ExtensionContext::dispose;

namespace Instances { namespace fl_external
{
    ExtensionContext::ExtensionContext(InstanceTraits::Traits& t)
    : Instances::fl_events::EventDispatcher(t)
//##protect##"instance::ExtensionContext::ExtensionContext()$data"
	, ExtensionID(GetStringManager().GetBuiltin(AS3Builtin_empty_)), ContextID(GetStringManager().GetBuiltin(AS3Builtin_empty_))
//##protect##"instance::ExtensionContext::ExtensionContext()$data"
    {
//##protect##"instance::ExtensionContext::ExtensionContext()$code"
//##protect##"instance::ExtensionContext::ExtensionContext()$code"
    }

    void ExtensionContext::actionScriptDataGet(SPtr<Instances::fl::Object>& result)
    {
//##protect##"instance::ExtensionContext::actionScriptDataGet()"
		SF_UNUSED(result);
#ifdef SF_ENABLE_ANE
		ASVM& asvm = static_cast<ASVM&>(VMRef);
		MovieRoot* pMovieRoot = asvm.GetMovieRoot();
		if (!pMovieRoot)
			return;
		MovieImpl* pmovieImpl = pMovieRoot->GetMovieImpl();
		SF_ASSERT(pmovieImpl);

		if (pmovieImpl)
		{
			GFx::Value* pASData = pmovieImpl->GetActionScriptData(ExtensionID.ToCStr(), ContextID.ToCStr());
			Value* pResult = new Value();
			if (pASData)
				pMovieRoot->GFxValue2ASValue(*pASData, pResult);
            if (pResult && pResult->IsObject())
			result = static_cast<Instances::fl::Object*>(pResult->GetObject());
		}
#endif
//##protect##"instance::ExtensionContext::actionScriptDataGet()"
    }
    void ExtensionContext::actionScriptDataSet(const Value& result, const Value& value)
    {
//##protect##"instance::ExtensionContext::actionScriptDataSet()"
		SF_UNUSED2(result, value);
#ifdef SF_ENABLE_ANE					
		ASVM& asvm = static_cast<ASVM&>(VMRef);
		MovieRoot* pMovieRoot = asvm.GetMovieRoot();
		MovieImpl* pmovieImpl = pMovieRoot->GetMovieImpl();
		SF_ASSERT(pmovieImpl);

		if (pmovieImpl)
		{
			GFx::Value* pdestVal = new GFx::Value();
			pMovieRoot->ASValue2GFxValue(value, pdestVal);

			SF_DEBUG_MESSAGE1(1, "AS ExtensionContext::SetActionScriptData %p \n", pdestVal);
			pmovieImpl->SetActionScriptData(ExtensionID.ToCStr(), ContextID.ToCStr(), pdestVal);
		}
#endif
//##protect##"instance::ExtensionContext::actionScriptDataSet()"
    }
    void ExtensionContext::call(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::ExtensionContext::call()"
		SF_UNUSED3(result, argc, argv);
#ifdef SF_ENABLE_ANE
        ASVM& asvm = static_cast<ASVM&>(VMRef);
        MovieRoot* proot = asvm.GetMovieRoot();
        SF_ASSERT(proot);
        MovieImpl* pmovie = proot->GetMovieImpl();
		SF_ASSERT(pmovie);
	    
		unsigned nArgs = 0;
		ASString methodName(GetStringManager().GetBuiltin(AS3Builtin_empty_));
		if (argc >= 1)
		{
			if (argv[0].Convert2String(methodName) == false)
				return;
			nArgs = argc - 1;
		}

        // convert arguments into GFxValue, starting from index 1
		enum { NumValuesOnStack = 10 };
		void* argArrayOnStack[NumValuesOnStack*((sizeof(GFx::Value)+sizeof(void*)-1)/sizeof(void*))];
		GFx::Value* pargArray;
		if (nArgs > NumValuesOnStack)
			pargArray = (GFx::Value*)SF_HEAP_AUTO_ALLOC_ID(this, sizeof(GFx::Value) * nArgs, GFx::StatMV_ActionScript_Mem);
		else
			pargArray = (GFx::Value*)argArrayOnStack;

        // convert params to GFxValue array
		for (unsigned i = 0, n = nArgs; i < n; ++i)
		{
			const Value& val = argv[i + 1];
			GFx::Value* pdestVal = Scaleform::Construct<GFx::Value>(&pargArray[i]);
			proot->ASValue2GFxValue(val, pdestVal);
		}

        /* Call method ot the extension.*/
		GFx::Value* pResult = new GFx::Value();//(GFx::Value*)SF_HEAP_AUTO_ALLOC_ID(this, sizeof(GFx::Value), GFx::StatMV_ActionScript_Mem);
		pmovie->Call(ExtensionID.ToCStr(), ContextID.ToCStr(), methodName.ToCStr(), nArgs, pargArray, pResult);

		// convert result
		proot->GFxValue2ASValue(*pResult, &result);

        // destruct elements in GFxValue array
        for (unsigned i = 0, n = nArgs; i < n; ++i)
            pargArray[i].~Value();
        if (nArgs > NumValuesOnStack)
            SF_FREE(pargArray);

		SF_DEBUG_MESSAGE(1, "ExtensionContext::call done");

		if (pResult)
			delete pResult;
#endif
//##protect##"instance::ExtensionContext::call()"
    }
    void ExtensionContext::dispose(const Value& result)
    {
//##protect##"instance::ExtensionContext::dispose()"
        SF_UNUSED1(result);
#ifdef SF_ENABLE_ANE
		ASVM& asvm = static_cast<ASVM&>(VMRef);

		if (!VMRef.IsInAS3VMDestructor())
        {
			MovieRoot* pMovieRoot = asvm.GetMovieRoot();
			if (pMovieRoot)
				pMovieRoot->RemoveFromExtensionContexts(this);

			MovieImpl* pMovieImpl = asvm.GetMovieImpl();
			if (pMovieImpl)
				pMovieImpl->FinalizeExtensionContext(ExtensionID.ToCStr(), ContextID.ToCStr());
		}
#endif
//##protect##"instance::ExtensionContext::dispose()"
    }

//##protect##"instance$methods"

	ExtensionContext::~ExtensionContext()
	{
		Value result;
		this->dispose(result);
	}

//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_external
{
    // const UInt16 ExtensionContext::tito[ExtensionContext::ThunkInfoNum] = {
    //    0, 1, 3, 5, 
    // };
    const TypeInfo* ExtensionContext::tit[6] = {
        &AS3::fl::ObjectTI, 
        NULL, &AS3::fl::ObjectTI, 
        &AS3::fl::ObjectTI, &AS3::fl::StringTI, 
        NULL, 
    };
    const ThunkInfo ExtensionContext::ti[ExtensionContext::ThunkInfoNum] = {
        {TFunc_Instances_ExtensionContext_actionScriptDataGet::Func, &ExtensionContext::tit[0], "actionScriptData", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_ExtensionContext_actionScriptDataSet::Func, &ExtensionContext::tit[1], "actionScriptData", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_ExtensionContext_call::Func, &ExtensionContext::tit[3], "call", NULL, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1, 0, NULL},
        {TFunc_Instances_ExtensionContext_dispose::Func, &ExtensionContext::tit[5], "dispose", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    ExtensionContext::ExtensionContext(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"InstanceTraits::ExtensionContext::ExtensionContext()"
//##protect##"InstanceTraits::ExtensionContext::ExtensionContext()"

    }

    void ExtensionContext::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<ExtensionContext&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl_external
{
    ExtensionContext::ExtensionContext(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::ExtensionContext::ExtensionContext()"
//##protect##"class_::ExtensionContext::ExtensionContext()"
    }
    void ExtensionContext::createExtensionContext(SPtr<Instances::fl_external::ExtensionContext>& result, const ASString& extensionID, const ASString& contextType)
    {
//##protect##"class_::ExtensionContext::createExtensionContext()"
        SF_UNUSED3(result, extensionID, contextType);
#ifdef SF_ENABLE_ANE
		SF_DEBUG_MESSAGE1(1, "AS ExtensionContext::createExtensionContext %s \n", extensionID.ToCStr());
		ASVM& asvm = static_cast<ASVM&>(GetVM());

		UPInt i = asvm.GetMovieRoot()->FindExtensionContexts(extensionID, contextType);
		if (i == SF_MAX_UPINT)
		{
			InstanceTraits::fl_external::ExtensionContext& itr = static_cast<InstanceTraits::fl_external::ExtensionContext&>(GetInstanceTraits());
			Pickable<Instances::fl_external::ExtensionContext> ec = itr.MakeInstance(itr);

			ec->ExtensionID = extensionID;
			ec->ContextID = contextType;

			asvm.GetMovieImpl()->InitializeExtensionContext(ec->ExtensionID.ToCStr(), ec->ContextID.ToCStr());
			asvm.GetMovieRoot()->AddToExtensionContexts(ec.GetPtr());
		
			result = ec;
		}
#endif
//##protect##"class_::ExtensionContext::createExtensionContext()"
    }
    void ExtensionContext::getExtensionDirectory(ASString& result, const ASString& extensionID)
    {
//##protect##"class_::ExtensionContext::getExtensionDirectory()"
        SF_UNUSED2(result, extensionID);
#ifdef SF_ENABLE_ANE
		ASVM& asvm = static_cast<ASVM&>(GetVM());
		
		Scaleform::String strResult = asvm.GetMovieImpl()->GetExtensionDirectory(extensionID.ToCStr());
		result = GetStringManager().CreateString(strResult.ToCStr());
#endif
//##protect##"class_::ExtensionContext::getExtensionDirectory()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes

typedef ThunkFunc2<Classes::fl_external::ExtensionContext, Classes::fl_external::ExtensionContext::mid_createExtensionContext, SPtr<Instances::fl_external::ExtensionContext>, const ASString&, const ASString&> TFunc_Classes_ExtensionContext_createExtensionContext;
typedef ThunkFunc1<Classes::fl_external::ExtensionContext, Classes::fl_external::ExtensionContext::mid_getExtensionDirectory, ASString, const ASString&> TFunc_Classes_ExtensionContext_getExtensionDirectory;

template <> const TFunc_Classes_ExtensionContext_createExtensionContext::TMethod TFunc_Classes_ExtensionContext_createExtensionContext::Method = &Classes::fl_external::ExtensionContext::createExtensionContext;
template <> const TFunc_Classes_ExtensionContext_getExtensionDirectory::TMethod TFunc_Classes_ExtensionContext_getExtensionDirectory::Method = &Classes::fl_external::ExtensionContext::getExtensionDirectory;

namespace ClassTraits { namespace fl_external
{
    // const UInt16 ExtensionContext::tito[ExtensionContext::ThunkInfoNum] = {
    //    0, 3, 
    // };
    const TypeInfo* ExtensionContext::tit[5] = {
        &AS3::fl_external::ExtensionContextTI, &AS3::fl::StringTI, &AS3::fl::StringTI, 
        &AS3::fl::StringTI, &AS3::fl::StringTI, 
    };
    const ThunkInfo ExtensionContext::ti[ExtensionContext::ThunkInfoNum] = {
        {TFunc_Classes_ExtensionContext_createExtensionContext::Func, &ExtensionContext::tit[0], "createExtensionContext", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Classes_ExtensionContext_getExtensionDirectory::Func, &ExtensionContext::tit[3], "getExtensionDirectory", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
    };

    ExtensionContext::ExtensionContext(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"ClassTraits::ExtensionContext::ExtensionContext()"
//##protect##"ClassTraits::ExtensionContext::ExtensionContext()"

    }

    Pickable<Traits> ExtensionContext::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) ExtensionContext(vm, AS3::fl_external::ExtensionContextCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_external::ExtensionContextCI));
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
    const TypeInfo ExtensionContextTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_external::ExtensionContext::InstanceType),
        ClassTraits::fl_external::ExtensionContext::ThunkInfoNum,
        0,
        InstanceTraits::fl_external::ExtensionContext::ThunkInfoNum,
        0,
        "ExtensionContext", "flash.external", &fl_events::EventDispatcherTI,
        TypeInfo::None
    };

    const ClassInfo ExtensionContextCI = {
        &ExtensionContextTI,
        ClassTraits::fl_external::ExtensionContext::MakeClassTraits,
        ClassTraits::fl_external::ExtensionContext::ti,
        NULL,
        InstanceTraits::fl_external::ExtensionContext::ti,
        NULL,
    };
}; // namespace fl_external


}}} // namespace Scaleform { namespace GFx { namespace AS3

