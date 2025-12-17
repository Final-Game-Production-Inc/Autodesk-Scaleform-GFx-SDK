/**************************************************************************

Filename    :   AS3_Obj_Global.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "AS3_Obj_Global.h"

#include "../AS3_VM.h"
#include "../AS3_Marshalling.h"
#include "../AS3_AsString.h" // For logging.

#include "GFx/AS3/AS3_MovieRoot.h"
#include "GFx/AS3/AS3_IntervalTimer.h"

#include "AS3_Obj_Number.h"
#include "AS3_Obj_int.h"
#include "AS3_Obj_uint.h"
#include "AS3_Obj_Boolean.h"
#include "AS3_Obj_String.h"
#include "AS3_Obj_Array.h"
#include "AS3_Obj_QName.h"
#ifdef GFX_ENABLE_XML
    #include "AS3_Obj_XMLList.h"
#endif
#include "AS3_Obj_UserDefined.h" // Because of GlobalObjectScript::InitUserDefinedClassTraits()
#include "System/AS3_Obj_System_ApplicationDomain.h"

namespace Scaleform { namespace GFx { namespace AS3 
{

void escapeMultiByteInternal(VM& vm, ASString& result, const ASString& value);
void unescapeMultiByteInternal(VM& vm, ASString& result, const ASString& value);

//##markup##"obj_global_cpp$package_methods_initialization"
//##begin##"obj_global_cpp$package_methods_initialization"
typedef ThunkFunc1<Instances::fl::GlobalObjectCPP, __LINE__, ASString, const ASString&> TFunc_Instances_GlobalObjectCPP_decodeURI;
typedef ThunkFunc1<Instances::fl::GlobalObjectCPP, __LINE__, ASString, const ASString&> TFunc_Instances_GlobalObjectCPP_decodeURIComponent;
typedef ThunkFunc1<Instances::fl::GlobalObjectCPP, __LINE__, ASString, const ASString&> TFunc_Instances_GlobalObjectCPP_encodeURI;
typedef ThunkFunc1<Instances::fl::GlobalObjectCPP, __LINE__, ASString, const ASString&> TFunc_Instances_GlobalObjectCPP_encodeURIComponent;
typedef ThunkFunc1<Instances::fl::GlobalObjectCPP, __LINE__, bool, Value::Number> TFunc_Instances_GlobalObjectCPP_isNaN;
typedef ThunkFunc1<Instances::fl::GlobalObjectCPP, __LINE__, bool, Value::Number> TFunc_Instances_GlobalObjectCPP_isFinite;
typedef ThunkFunc2<Instances::fl::GlobalObjectCPP, __LINE__, Value, unsigned, const Value*> TFunc_Instances_GlobalObjectCPP_parseInt;
typedef ThunkFunc1<Instances::fl::GlobalObjectCPP, __LINE__, Value::Number, const ASString&> TFunc_Instances_GlobalObjectCPP_parseFloat;
typedef ThunkFunc2<Instances::fl::GlobalObjectCPP, __LINE__, Value, unsigned, const Value*> TFunc_Instances_GlobalObjectCPP_escape;
typedef ThunkFunc2<Instances::fl::GlobalObjectCPP, __LINE__, Value, unsigned, const Value*> TFunc_Instances_GlobalObjectCPP_unescape;
typedef ThunkFunc1<Instances::fl::GlobalObjectCPP, __LINE__, bool, const Value&> TFunc_Instances_GlobalObjectCPP_isXMLName;
typedef ThunkFunc2<Instances::fl::GlobalObjectCPP, __LINE__, Value, unsigned, const Value*> TFunc_Instances_GlobalObjectCPP_trace;
typedef ThunkFunc2<Instances::fl::GlobalObjectCPP, __LINE__, Value, unsigned, const Value*> TFunc_Instances_GlobalObjectCPP_print;

template <> const TFunc_Instances_GlobalObjectCPP_decodeURI::TMethod TFunc_Instances_GlobalObjectCPP_decodeURI::Method = &Instances::fl::GlobalObjectCPP::decodeURI;
template <> const TFunc_Instances_GlobalObjectCPP_decodeURIComponent::TMethod TFunc_Instances_GlobalObjectCPP_decodeURIComponent::Method = &Instances::fl::GlobalObjectCPP::decodeURIComponent;
template <> const TFunc_Instances_GlobalObjectCPP_encodeURI::TMethod TFunc_Instances_GlobalObjectCPP_encodeURI::Method = &Instances::fl::GlobalObjectCPP::encodeURI;
template <> const TFunc_Instances_GlobalObjectCPP_encodeURIComponent::TMethod TFunc_Instances_GlobalObjectCPP_encodeURIComponent::Method = &Instances::fl::GlobalObjectCPP::encodeURIComponent;
template <> const TFunc_Instances_GlobalObjectCPP_isNaN::TMethod TFunc_Instances_GlobalObjectCPP_isNaN::Method = &Instances::fl::GlobalObjectCPP::isNaN;
template <> const TFunc_Instances_GlobalObjectCPP_isFinite::TMethod TFunc_Instances_GlobalObjectCPP_isFinite::Method = &Instances::fl::GlobalObjectCPP::isFinite;
template <> const TFunc_Instances_GlobalObjectCPP_parseInt::TMethod TFunc_Instances_GlobalObjectCPP_parseInt::Method = &Instances::fl::GlobalObjectCPP::parseInt;
template <> const TFunc_Instances_GlobalObjectCPP_parseFloat::TMethod TFunc_Instances_GlobalObjectCPP_parseFloat::Method = &Instances::fl::GlobalObjectCPP::parseFloat;
template <> const TFunc_Instances_GlobalObjectCPP_escape::TMethod TFunc_Instances_GlobalObjectCPP_escape::Method = &Instances::fl::GlobalObjectCPP::escape;
template <> const TFunc_Instances_GlobalObjectCPP_unescape::TMethod TFunc_Instances_GlobalObjectCPP_unescape::Method = &Instances::fl::GlobalObjectCPP::unescape;
template <> const TFunc_Instances_GlobalObjectCPP_isXMLName::TMethod TFunc_Instances_GlobalObjectCPP_isXMLName::Method = &Instances::fl::GlobalObjectCPP::isXMLName;
template <> const TFunc_Instances_GlobalObjectCPP_trace::TMethod TFunc_Instances_GlobalObjectCPP_trace::Method = &Instances::fl::GlobalObjectCPP::trace;
template <> const TFunc_Instances_GlobalObjectCPP_print::TMethod TFunc_Instances_GlobalObjectCPP_print::Method = &Instances::fl::GlobalObjectCPP::print;
typedef ThunkFunc2<Instances::fl::GlobalObjectCPP, __LINE__, const Value, Instances::fl_net::URLRequest*, const ASString&> TFunc_Instances_GlobalObjectCPP_navigateToURL;
typedef ThunkFunc1<Instances::fl::GlobalObjectCPP, __LINE__, SPtr<Class>, const ASString&> TFunc_Instances_GlobalObjectCPP_getClassByAlias;
typedef ThunkFunc2<Instances::fl::GlobalObjectCPP, __LINE__, const Value, const ASString&, Class*> TFunc_Instances_GlobalObjectCPP_registerClassAlias;

template <> const TFunc_Instances_GlobalObjectCPP_navigateToURL::TMethod TFunc_Instances_GlobalObjectCPP_navigateToURL::Method = &Instances::fl::GlobalObjectCPP::navigateToURL;
template <> const TFunc_Instances_GlobalObjectCPP_getClassByAlias::TMethod TFunc_Instances_GlobalObjectCPP_getClassByAlias::Method = &Instances::fl::GlobalObjectCPP::getClassByAlias;
template <> const TFunc_Instances_GlobalObjectCPP_registerClassAlias::TMethod TFunc_Instances_GlobalObjectCPP_registerClassAlias::Method = &Instances::fl::GlobalObjectCPP::registerClassAlias;
typedef ThunkFunc2<Instances::fl::GlobalObjectCPP, __LINE__, Value, unsigned, const Value*> TFunc_Instances_GlobalObjectCPP_fscommand;

template <> const TFunc_Instances_GlobalObjectCPP_fscommand::TMethod TFunc_Instances_GlobalObjectCPP_fscommand::Method = &Instances::fl::GlobalObjectCPP::fscommand;
typedef ThunkFunc1<Instances::fl::GlobalObjectCPP, __LINE__, const Value, UInt32> TFunc_Instances_GlobalObjectCPP_clearInterval;
typedef ThunkFunc1<Instances::fl::GlobalObjectCPP, __LINE__, const Value, UInt32> TFunc_Instances_GlobalObjectCPP_clearTimeout;
typedef ThunkFunc1<Instances::fl::GlobalObjectCPP, __LINE__, SPtr<Instances::fl::XML>, const Value&> TFunc_Instances_GlobalObjectCPP_describeType;
typedef ThunkFunc1<Instances::fl::GlobalObjectCPP, __LINE__, ASString, const ASString&> TFunc_Instances_GlobalObjectCPP_escapeMultiByte;
typedef ThunkFunc1<Instances::fl::GlobalObjectCPP, __LINE__, Value, const ASString&> TFunc_Instances_GlobalObjectCPP_getDefinitionByName;
typedef ThunkFunc1<Instances::fl::GlobalObjectCPP, __LINE__, ASString, const Value&> TFunc_Instances_GlobalObjectCPP_getQualifiedClassName;
typedef ThunkFunc1<Instances::fl::GlobalObjectCPP, __LINE__, Value, const Value&> TFunc_Instances_GlobalObjectCPP_getQualifiedSuperclassName;
typedef ThunkFunc0<Instances::fl::GlobalObjectCPP, __LINE__, SInt32> TFunc_Instances_GlobalObjectCPP_getTimer;
typedef ThunkFunc2<Instances::fl::GlobalObjectCPP, __LINE__, Value, unsigned, const Value*> TFunc_Instances_GlobalObjectCPP_setInterval;
typedef ThunkFunc2<Instances::fl::GlobalObjectCPP, __LINE__, Value, unsigned, const Value*> TFunc_Instances_GlobalObjectCPP_setTimeout;
typedef ThunkFunc1<Instances::fl::GlobalObjectCPP, __LINE__, ASString, const ASString&> TFunc_Instances_GlobalObjectCPP_unescapeMultiByte;

template <> const TFunc_Instances_GlobalObjectCPP_clearInterval::TMethod TFunc_Instances_GlobalObjectCPP_clearInterval::Method = &Instances::fl::GlobalObjectCPP::clearInterval;
template <> const TFunc_Instances_GlobalObjectCPP_clearTimeout::TMethod TFunc_Instances_GlobalObjectCPP_clearTimeout::Method = &Instances::fl::GlobalObjectCPP::clearTimeout;
template <> const TFunc_Instances_GlobalObjectCPP_describeType::TMethod TFunc_Instances_GlobalObjectCPP_describeType::Method = &Instances::fl::GlobalObjectCPP::describeType;
template <> const TFunc_Instances_GlobalObjectCPP_escapeMultiByte::TMethod TFunc_Instances_GlobalObjectCPP_escapeMultiByte::Method = &Instances::fl::GlobalObjectCPP::escapeMultiByte;
template <> const TFunc_Instances_GlobalObjectCPP_getDefinitionByName::TMethod TFunc_Instances_GlobalObjectCPP_getDefinitionByName::Method = &Instances::fl::GlobalObjectCPP::getDefinitionByName;
template <> const TFunc_Instances_GlobalObjectCPP_getQualifiedClassName::TMethod TFunc_Instances_GlobalObjectCPP_getQualifiedClassName::Method = &Instances::fl::GlobalObjectCPP::getQualifiedClassName;
template <> const TFunc_Instances_GlobalObjectCPP_getQualifiedSuperclassName::TMethod TFunc_Instances_GlobalObjectCPP_getQualifiedSuperclassName::Method = &Instances::fl::GlobalObjectCPP::getQualifiedSuperclassName;
template <> const TFunc_Instances_GlobalObjectCPP_getTimer::TMethod TFunc_Instances_GlobalObjectCPP_getTimer::Method = &Instances::fl::GlobalObjectCPP::getTimer;
template <> const TFunc_Instances_GlobalObjectCPP_setInterval::TMethod TFunc_Instances_GlobalObjectCPP_setInterval::Method = &Instances::fl::GlobalObjectCPP::setInterval;
template <> const TFunc_Instances_GlobalObjectCPP_setTimeout::TMethod TFunc_Instances_GlobalObjectCPP_setTimeout::Method = &Instances::fl::GlobalObjectCPP::setTimeout;
template <> const TFunc_Instances_GlobalObjectCPP_unescapeMultiByte::TMethod TFunc_Instances_GlobalObjectCPP_unescapeMultiByte::Method = &Instances::fl::GlobalObjectCPP::unescapeMultiByte;

//##end##"obj_global_cpp$package_methods_initialization"

namespace Instances { namespace fl
{
    ///////////////////////////////////////////////////////////////////////
    GlobalObject::GlobalObject(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
    {
    }

    void GlobalObject::Construct(Value& _this, unsigned argc, const Value* argv, bool extCall)
    {
        SF_UNUSED4(_this, argc, argv, extCall);
        GetVM().ThrowTypeError(VM::Error(VM::eConstructOfNonFunctionError, GetVM()));
    }

    void GlobalObject::Call(const Value& _this, Value& result, unsigned argc, const Value* const argv)
    {
        SF_UNUSED4(result, _this, argc, argv);
        GetVM().ThrowTypeError(VM::Error(VM::eCallOfNonFunctionError, GetVM()
            SF_DEBUG_ARG(GetVM().GetValueTraits(_this).GetName().ToCStr())
            ));
    }

    GlobalObject::~GlobalObject()
    {
    }

    void GlobalObject::Execute()
    {
        ; // Do nothing.
    }

    ///////////////////////////////////////////////////////////////////////
    GlobalObjectScript::GlobalObjectScript(InstanceTraits::fl::GlobalObjectScript& t)
    : GlobalObject(t)
    , Initialized(false)
    {
    }

    GlobalObjectScript::~GlobalObjectScript()
    {
    }

    void GlobalObjectScript::Execute()
    {
        // Do not call it twice.
        if (Initialized)
            return;

        // Set up slot values ...
        if (!GetTraits().SetupSlotValues(GetFile(), GetScript(), *this))
            // Exception.
            return;

        const Abc::MbiInd mbi_ind = GetFile().GetMethodBodyInfoInd(GetScript().GetMethodInfoInd());


        // We do not need to pass arguments to a constructor. It is a Global Object ...
        SF_DEBUG_CODE(static const char* name = "Global Object constructor. Method ind: ";)

        GetVM().AddFrame(
            Value(this), // GlobalObjectScript are already tracked.
            GetFile(),
            mbi_ind,
            Value(this), 
            0, // No args ...
            NULL,
            true, // discard_result
#ifdef SF_AS3_ENABLE_EXPLICIT_GO
            *this,
#endif
#if 0
            NULL, // No saved scope ...
#else
            GetTraits().GetStoredScopeStack(),
#endif
            GetTraits() // That seems to be correct because this function is originated in this class. No inheritance involved.
            SF_DEBUG_ARG(GetStringManager().CreateConstString(name) + Scaleform::AsString(mbi_ind.Get()))
            ); 

        if (GetVM().IsException())
            // Oops. Exception.
            return;

        // We assume that the CallFrame above will be executed.
        Initialized = true;
    }

    void GlobalObjectScript::InitUserDefinedClassTraits()
    {
        // This method will set up Script Object with InstanceTraits::UserDefined.
        // It does nothing else.

        using namespace Abc;

        const Abc::ScriptInfo& si = GetScript();
        VMAbcFile& file = GetFile();
        const TraitTable& tt = file.GetTraits();

        for (UPInt i = 0; i < si.GetTraitsCount(); ++i)
        {
            const TraitInfo& ti = si.GetTraitInfo(tt, AbsoluteIndex(i));

            if (ti.GetType() != TraitInfo::tClass)
                continue;

            const Abc::Multiname& name = ti.GetTypeName(file.GetAbcFile());
            const Instances::fl::Namespace& ns = file.GetInternedNamespace(name.GetNamespaceInd());
            ASString str_name = file.GetInternedString(name.GetNameInd());

            // Skip Scaleform classes.
            if (IsScaleformGFx(ns))
                continue;

            const ClassTraits::UserDefined* cud = static_cast<const ClassTraits::UserDefined*>(GetVM().GetRegisteredClassTraits(str_name, ns, file.GetAppDomain()));

            // Filter out classes, which are defined in other files. This happens.
            if (cud && (&cud->GetFile() == &file))
            {
                InstanceTraits::UserDefined& iud = static_cast<InstanceTraits::UserDefined&>(cud->GetInstanceTraits());

                if (!iud.HasScriptSetUp())
                    iud.SetScript(*this);
//                 else
//                     SF_ASSERT(false);
            }
//             else
//                 SF_ASSERT(false);
        }
    }

    const SlotInfo* GlobalObjectScript::InitializeOnDemand(
        const SlotInfo* si,
        const ASString&,
        const Namespace&,
        UPInt&)
    {
        if (si)
            InitOnDemand();

        return si;
    }

    void GlobalObjectScript::InitOnDemand()
    {
        if (!Initialized)
        {
            // Oops. We need to execute the script.

            // InitOnDemand() will execute code. We shouldn't do that in AOTC.
#if !defined(SF_AS3_AOTC) && !defined(SF_AS3_AOTC2)
            // Put a CallFrame.
            Execute();

            if (GetVM().IsException())
                return;

            // Execute just this one CallFrame.
            GetVM().ExecuteCode();

            // This is not necessary. It will be done by Init().
            // Initialized = true;
#else
            Initialized = true;
#endif
        }
    }

#ifdef GFX_AS3_VERBOSE
    Scaleform::String GlobalObjectScript::GetAS3ObjectName() const 
    { 
        return Scaleform::String(GetFile().GetAbcFile().GetName()); 
    }
#endif

    ///////////////////////////////////////////////////////////////////////
    MemberInfo GlobalObjectCPP::mi[GlobalObjectCPP::MemberInfoNum] = {
        {"Infinity", NULL, OFFSETOF(Instances::fl::GlobalObjectCPP, PositiveInfinity), Abc::NS_Public, SlotInfo::BT_Number, 1},
        {"-Infinity", NULL, OFFSETOF(Instances::fl::GlobalObjectCPP, NegativeInfinity), Abc::NS_Public, SlotInfo::BT_Number, 1},
        {"NaN", NULL, OFFSETOF(Instances::fl::GlobalObjectCPP, NaN), Abc::NS_Public, SlotInfo::BT_Number, 1},
        {"undefined", NULL, OFFSETOF(Instances::fl::GlobalObjectCPP, undefined), Abc::NS_Public, SlotInfo::BT_Value, 1},
        {"HIDE_NSURI_METHODS", NULL, OFFSETOF(Instances::fl::GlobalObjectCPP, HIDE_NSURI_METHODS), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"INCLUDE_BASES", NULL, OFFSETOF(Instances::fl::GlobalObjectCPP, INCLUDE_BASES), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"INCLUDE_INTERFACES", NULL, OFFSETOF(Instances::fl::GlobalObjectCPP, INCLUDE_INTERFACES), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"INCLUDE_VARIABLES", NULL, OFFSETOF(Instances::fl::GlobalObjectCPP, INCLUDE_VARIABLES), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"INCLUDE_ACCESSORS", NULL, OFFSETOF(Instances::fl::GlobalObjectCPP, INCLUDE_ACCESSORS), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"INCLUDE_METHODS", NULL, OFFSETOF(Instances::fl::GlobalObjectCPP, INCLUDE_METHODS), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"INCLUDE_METADATA", NULL, OFFSETOF(Instances::fl::GlobalObjectCPP, INCLUDE_METADATA), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"INCLUDE_CONSTRUCTOR", NULL, OFFSETOF(Instances::fl::GlobalObjectCPP, INCLUDE_CONSTRUCTOR), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"INCLUDE_TRAITS", NULL, OFFSETOF(Instances::fl::GlobalObjectCPP, INCLUDE_TRAITS), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"USE_ITRAITS", NULL, OFFSETOF(Instances::fl::GlobalObjectCPP, USE_ITRAITS), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"HIDE_OBJECT", NULL, OFFSETOF(Instances::fl::GlobalObjectCPP, HIDE_OBJECT), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"FLASH10_FLAGS", NULL, OFFSETOF(Instances::fl::GlobalObjectCPP, FLASH10_FLAGS), Abc::NS_Public, SlotInfo::BT_UInt, 1},
    };

    GlobalObjectCPP::GlobalObjectCPP(
        VM& vm,
        InstanceTraits::Traits& t
        )
    : GlobalObject(t)
    , PositiveInfinity(NumberUtil::POSITIVE_INFINITY())
    , NegativeInfinity(NumberUtil::NEGATIVE_INFINITY())
    , NaN(NumberUtil::NaN())
    , undefined() // Value is undefined by default.
    , HIDE_NSURI_METHODS(0x0001)
    , INCLUDE_BASES(0x0002)
    , INCLUDE_INTERFACES(0x0004)
    , INCLUDE_VARIABLES(0x0008)
    , INCLUDE_ACCESSORS(0x0010)
    , INCLUDE_METHODS(0x0020)
    , INCLUDE_METADATA(0x0040)
    , INCLUDE_CONSTRUCTOR(0x0080)
    , INCLUDE_TRAITS(0x0100)
    , USE_ITRAITS(0x0200)
    , HIDE_OBJECT(0x0400)
    , FLASH10_FLAGS(INCLUDE_BASES |
        INCLUDE_INTERFACES |
        INCLUDE_VARIABLES |
        INCLUDE_ACCESSORS |
        INCLUDE_METHODS |
        INCLUDE_METADATA |
        INCLUDE_CONSTRUCTOR |
        INCLUDE_TRAITS |
        HIDE_NSURI_METHODS |
        HIDE_OBJECT
        )
    {
#ifdef GFX_AS3_SUPPORT
        RegisterClassInfoTable(Classes::ClassRegistrationTable);
#endif

#ifdef SF_AS3_CLASS_AS_SLOT
        AddFixedSlot(vm.GetClassObject());
        AddFixedSlot(vm.GetClassClass());
        AddFixedSlot(vm.GetClassNamespace());
        AddFixedSlot(vm.GetClassFunction());
        AddFixedSlot(vm.GetClassBoolean());
        AddFixedSlot(vm.GetClassNumber());
        AddFixedSlot(vm.GetClassSInt());
        AddFixedSlot(vm.GetClassUInt());
        AddFixedSlot(vm.GetClassString());
        AddFixedSlot(vm.GetClassArray());
        AddFixedSlot(vm.GetClassQName());
#ifdef GFX_ENABLE_XML
        const XMLSupport& xmls = vm.GetXMLSupport();

        if (xmls.IsEnabled())
        {
            AddFixedSlot(xmls.GetITraitsXML()->GetClass());
            AddFixedSlot(xmls.GetITraitsXMLList()->GetClass());
        }
#endif
#else
        SF_UNUSED1(vm);
#endif
        
//##markup##"obj_global_cpp$package_methods_registration"
//##begin##"obj_global_cpp$package_methods_registration"
        {
            const TypeInfo TInfo = {TypeInfo::CompileTime, 0, 0, 0, 0, 0, "", "", NULL};
            const ClassInfo CInfo = {&TInfo, NULL};
            static const TypeInfo* tit[] = {
                &AS3::fl::StringTI, &AS3::fl::StringTI, 
                &AS3::fl::StringTI, &AS3::fl::StringTI, 
                &AS3::fl::StringTI, &AS3::fl::StringTI, 
                &AS3::fl::StringTI, &AS3::fl::StringTI, 
                &AS3::fl::BooleanTI, &AS3::fl::NumberTI, 
                &AS3::fl::BooleanTI, &AS3::fl::NumberTI, 
                &AS3::fl::NumberTI, 
                &AS3::fl::NumberTI, &AS3::fl::StringTI, 
                &AS3::fl::StringTI, 
                &AS3::fl::StringTI, 
                &AS3::fl::BooleanTI, NULL, 
                NULL, 
                NULL, 
            };
            static const ThunkInfo f[] = {
                {TFunc_Instances_GlobalObjectCPP_decodeURI::Func, &tit[0], "decodeURI", NULL, Abc::NS_Public, CT_Method, 0, 1, 0},
                {TFunc_Instances_GlobalObjectCPP_decodeURIComponent::Func, &tit[2], "decodeURIComponent", NULL, Abc::NS_Public, CT_Method, 0, 1, 0},
                {TFunc_Instances_GlobalObjectCPP_encodeURI::Func, &tit[4], "encodeURI", NULL, Abc::NS_Public, CT_Method, 0, 1, 0},
                {TFunc_Instances_GlobalObjectCPP_encodeURIComponent::Func, &tit[6], "encodeURIComponent", NULL, Abc::NS_Public, CT_Method, 0, 1, 0},
                {TFunc_Instances_GlobalObjectCPP_isNaN::Func, &tit[8], "isNaN", NULL, Abc::NS_Public, CT_Method, 0, 1, 0},
                {TFunc_Instances_GlobalObjectCPP_isFinite::Func, &tit[10], "isFinite", NULL, Abc::NS_Public, CT_Method, 0, 1, 0},
                {TFunc_Instances_GlobalObjectCPP_parseInt::Func, &tit[12], "parseInt", NULL, Abc::NS_Public, CT_Method, 0, 2, 1},
                {TFunc_Instances_GlobalObjectCPP_parseFloat::Func, &tit[13], "parseFloat", NULL, Abc::NS_Public, CT_Method, 0, 1, 0},
                {TFunc_Instances_GlobalObjectCPP_escape::Func, &tit[15], "escape", NULL, Abc::NS_Public, CT_Method, 0, 1, 1},
                {TFunc_Instances_GlobalObjectCPP_unescape::Func, &tit[16], "unescape", NULL, Abc::NS_Public, CT_Method, 0, 1, 1},
                {TFunc_Instances_GlobalObjectCPP_isXMLName::Func, &tit[17], "isXMLName", NULL, Abc::NS_Public, CT_Method, 0, 1, 0},
                {TFunc_Instances_GlobalObjectCPP_trace::Func, &tit[19], "trace", NULL, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1},
                {TFunc_Instances_GlobalObjectCPP_print::Func, &tit[20], "print", NULL, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1},
            };
            for (unsigned i = 0; i < NUMBEROF(f); ++i)
                Add2VT(CInfo, f[i]);
        }
#ifndef SF_AS3_NO_FLASH
        {
            const TypeInfo TInfo = {TypeInfo::CompileTime, 0, 0, 0, 0, 0, "", "flash.net", NULL};
            const ClassInfo CInfo = {&TInfo, NULL};
            static const TypeInfo* tit[] = {
                NULL, &AS3::fl_net::URLRequestTI, &AS3::fl::StringTI, 
                &AS3::fl::ClassTI, &AS3::fl::StringTI, 
                NULL, &AS3::fl::StringTI, &AS3::fl::ClassTI, 
            };
            static const ThunkInfo f[] = {
                {TFunc_Instances_GlobalObjectCPP_navigateToURL::Func, &tit[0], "navigateToURL", NULL, Abc::NS_Public, CT_Method, 1, 2, 0},
                {TFunc_Instances_GlobalObjectCPP_getClassByAlias::Func, &tit[3], "getClassByAlias", NULL, Abc::NS_Public, CT_Method, 1, 1, 0},
                {TFunc_Instances_GlobalObjectCPP_registerClassAlias::Func, &tit[5], "registerClassAlias", NULL, Abc::NS_Public, CT_Method, 2, 2, 0},
            };
            for (unsigned i = 0; i < NUMBEROF(f); ++i)
                Add2VT(CInfo, f[i]);
        }
#endif
#ifndef SF_AS3_NO_FLASH
        {
            const TypeInfo TInfo = {TypeInfo::CompileTime, 0, 0, 0, 0, 0, "", "flash.system", NULL};
            const ClassInfo CInfo = {&TInfo, NULL};
            static const TypeInfo* tit[] = {
                NULL, &AS3::fl::StringTI, 
            };
            static const ThunkInfo f[] = {
                {TFunc_Instances_GlobalObjectCPP_fscommand::Func, &tit[0], "fscommand", NULL, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1},
            };
            for (unsigned i = 0; i < NUMBEROF(f); ++i)
                Add2VT(CInfo, f[i]);
        }
#endif
#ifndef SF_AS3_NO_FLASH
        {
            const TypeInfo TInfo = {TypeInfo::CompileTime, 0, 0, 0, 0, 0, "", "flash.utils", NULL};
            const ClassInfo CInfo = {&TInfo, NULL};
            static const TypeInfo* tit[] = {
                NULL, &AS3::fl::uintTI, 
                NULL, &AS3::fl::uintTI, 
                &AS3::fl::XMLTI, NULL, 
                &AS3::fl::StringTI, &AS3::fl::StringTI, 
                NULL, &AS3::fl::StringTI, 
                &AS3::fl::StringTI, NULL, 
                NULL, NULL, 
                &AS3::fl::int_TI, 
                &AS3::fl::uintTI, &AS3::fl::FunctionTI, &AS3::fl::NumberTI, 
                &AS3::fl::uintTI, &AS3::fl::FunctionTI, &AS3::fl::NumberTI, 
                &AS3::fl::StringTI, &AS3::fl::StringTI, 
            };
            static const ThunkInfo f[] = {
                {TFunc_Instances_GlobalObjectCPP_clearInterval::Func, &tit[0], "clearInterval", NULL, Abc::NS_Public, CT_Method, 1, 1, 0},
                {TFunc_Instances_GlobalObjectCPP_clearTimeout::Func, &tit[2], "clearTimeout", NULL, Abc::NS_Public, CT_Method, 1, 1, 0},
                {TFunc_Instances_GlobalObjectCPP_describeType::Func, &tit[4], "describeType", NULL, Abc::NS_Public, CT_Method, 1, 1, 0},
                {TFunc_Instances_GlobalObjectCPP_escapeMultiByte::Func, &tit[6], "escapeMultiByte", NULL, Abc::NS_Public, CT_Method, 1, 1, 0},
                {TFunc_Instances_GlobalObjectCPP_getDefinitionByName::Func, &tit[8], "getDefinitionByName", NULL, Abc::NS_Public, CT_Method, 1, 1, 0},
                {TFunc_Instances_GlobalObjectCPP_getQualifiedClassName::Func, &tit[10], "getQualifiedClassName", NULL, Abc::NS_Public, CT_Method, 1, 1, 0},
                {TFunc_Instances_GlobalObjectCPP_getQualifiedSuperclassName::Func, &tit[12], "getQualifiedSuperclassName", NULL, Abc::NS_Public, CT_Method, 1, 1, 0},
                {TFunc_Instances_GlobalObjectCPP_getTimer::Func, &tit[14], "getTimer", NULL, Abc::NS_Public, CT_Method, 0, 0, 0},
                {TFunc_Instances_GlobalObjectCPP_setInterval::Func, &tit[15], "setInterval", NULL, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1},
                {TFunc_Instances_GlobalObjectCPP_setTimeout::Func, &tit[18], "setTimeout", NULL, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1},
                {TFunc_Instances_GlobalObjectCPP_unescapeMultiByte::Func, &tit[21], "unescapeMultiByte", NULL, Abc::NS_Public, CT_Method, 1, 1, 0},
            };
            for (unsigned i = 0; i < NUMBEROF(f); ++i)
                Add2VT(CInfo, f[i]);
        }
#endif

//##end##"obj_global_cpp$package_methods_registration"

        for (unsigned i = 0; i < GlobalObjectCPP::MemberInfoNum; ++i)
            t.AddSlot(mi[i]);

        // avmplus.
        {
            const TypeInfo TInfo = {TypeInfo::CompileTime, 0, 0, 0, 0, 0, "", "avmplus", NULL};
            const ClassInfo CInfo = {&TInfo, NULL};
            static const TypeInfo* tit[5] = {
                &AS3::fl::XMLTI,
                &AS3::fl::StringTI, NULL,
                NULL, NULL,
            };
            // "describeType" in "avmplus" has different signature comparing to "flash.utils".
            static const ThunkInfo f[] = {
                {TFunc_Instances_GlobalObjectCPP_describeType::Func, &tit[0], "describeType", NULL, Abc::NS_Public, CT_Method, 0, SF_AS3_VARARGNUM, 1},
                {TFunc_Instances_GlobalObjectCPP_getQualifiedClassName::Func, &tit[1], "getQualifiedClassName", NULL, Abc::NS_Public, CT_Method, 1, 1},
                {TFunc_Instances_GlobalObjectCPP_getQualifiedSuperclassName::Func, &tit[3], "getQualifiedSuperclassName", NULL, Abc::NS_Public, CT_Method, 1, 1},
            };
            for (unsigned i = 0; i < NUMBEROF(f); ++i)
                Add2VT(CInfo, f[i]);
        }
    }
    
    GlobalObjectCPP::~GlobalObjectCPP()
    {
    }

    void GlobalObjectCPP::RegisterClassInfoTable(const ClassInfo* table[])
    {
        for (unsigned i = 0; table[i] != NULL; ++i)
            CIRegistrationHash.Add(table[i]->GetName(), table[i]);
    }

    void GlobalObjectCPP::ForEachChild_GC(Collector* prcc, GcOp op) const
    {
        GlobalObject::ForEachChild_GC(prcc, op);

        // All values are registered with Traits and will be visited automatically.
        // AS3::ForEachChild_GC(prcc, Values, op);

        UPInt size = CTraits.GetSize();
        for (UPInt i = 0; i < size; ++i)
            AS3::ForEachChild_GC(prcc, CTraits[i], op SF_DEBUG_ARG(*this));
    }
     
    ClassTraits::Traits* GlobalObjectCPP::GetClassTraits(
        const ASString& name,
        const Namespace& ns
        )
    {
        if (ns.GetKind() != Abc::NS_Public)
            return NULL;

        const ClassInfo* inh = NULL;
        if (CIRegistrationHash.Get(name.ToCStr(), &inh) && inh && ns.GetUri() == inh->GetPkgName())
        {
            Pickable<ClassTraits::Traits> tr;

            // Create ClassTraits.
            tr = (*inh->Factory)(GetVM());
            // It should be registered outside of this method.
            //GetVM().RegisterClassTraits(name, ns, *tr);

            CTraits.PushBack(tr);

            return tr.GetPtr();
        }

        // That shouldn't happen.
        // It actually can happen if we are searching in a wrong namespace.
        //SF_ASSERT(false);

        return NULL;
    }

#ifdef SF_AS3_CLASS_AS_SLOT
    const SlotInfo* GlobalObjectCPP::InitializeOnDemand(
        const SlotInfo* si,
        const ASString& name,
        const Namespace& ns,
        UPInt& index
        )
    {
        if (si || ns.GetKind() != Abc::NS_Public)
            return si;

        const ClassTraits::Traits* tr = GetVM().Resolve2ClassTraits(name, ns, GetVM().GetFrameAppDomain());
        if (tr)
        {
            Class& class_ = tr->GetInstanceTraits().GetClass();

            // Add Class to slots.
            si = &AddFixedSlot(class_, Pickable<const Namespace>(&ns, PickValue), index);
        }

        return si;
    }
#endif

    //////////////////////////////////////////////////////////////////////
//##markup##"obj_global_cpp$package_methods"
//##begin##"obj_global_cpp$package_methods"
    void GlobalObjectCPP::decodeURI(ASString& result, const ASString& uri)
    {
//##protect##"instance::::decodeURI()"
        StringBuffer b;

        if (!ASUtils::AS3::DecodeURI(uri.ToCStr(), uri.GetSize(), b))
        {
            VM& vm = GetVM();
            vm.ThrowURIError(VM::Error(VM::eInvalidURIError, vm
                SF_DEBUG_ARG("decodeURI")
                ));
        }
        else
            result = GetVM().GetStringManager().CreateString(b.ToCStr(), b.GetSize());
//##protect##"instance::::decodeURI()"
    }
    void GlobalObjectCPP::decodeURIComponent(ASString& result, const ASString& uri)
    {
//##protect##"instance::::decodeURIComponent()"
        StringBuffer b;

        if (!ASUtils::AS3::DecodeURI(uri.ToCStr(), uri.GetSize(), b, true))
        {
            VM& vm = GetVM();
            vm.ThrowURIError(VM::Error(VM::eInvalidURIError, vm
                SF_DEBUG_ARG("decodeURIComponent")
                ));
        }
        else
            result = GetVM().GetStringManager().CreateString(b.ToCStr(), b.GetSize());
//##protect##"instance::::decodeURIComponent()"
    }
    void GlobalObjectCPP::encodeURI(ASString& result, const ASString& uri)
    {
//##protect##"instance::::encodeURI()"
        StringBuffer b;
        
        if (!ASUtils::AS3::EncodeURI(uri.ToCStr(), uri.GetSize(), b))
        {
            VM& vm = GetVM();
            vm.ThrowURIError(VM::Error(VM::eInvalidURIError, vm
                SF_DEBUG_ARG("encodeURI")
                ));
        }
        else
            result = GetVM().GetStringManager().CreateString(b.ToCStr(), b.GetSize());
//##protect##"instance::::encodeURI()"
    }
    void GlobalObjectCPP::encodeURIComponent(ASString& result, const ASString& uri)
    {
//##protect##"instance::::encodeURIComponent()"
        StringBuffer b;

        if (!ASUtils::AS3::EncodeURI(uri.ToCStr(), uri.GetSize(), b, true))
        {
            VM& vm = GetVM();
            vm.ThrowURIError(VM::Error(VM::eInvalidURIError, vm
                SF_DEBUG_ARG("encodeURIComponent")
                ));
        }
        else
            result = GetVM().GetStringManager().CreateString(b.ToCStr(), b.GetSize());
//##protect##"instance::::encodeURIComponent()"
    }
    void GlobalObjectCPP::isNaN(bool& result, Value::Number n)
    {
//##protect##"instance::::isNaN()"
        result = NumberUtil::IsNaN(n);
//##protect##"instance::::isNaN()"
    }
    void GlobalObjectCPP::isFinite(bool& result, Value::Number n)
    {
//##protect##"instance::::isFinite()"
        result = !NumberUtil::IsNaNOrInfinity(n);
//##protect##"instance::::isFinite()"
    }
    void GlobalObjectCPP::parseInt(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::::parseInt()"
        ASString str = GetStringManager().GetBuiltin(AS3Builtin_NaN);

        if (argc >= 1)
            if (!argv[0].Convert2String(str))
                return;

        SInt32      radix = 0;
        UInt32      len = str.GetSize();
        UInt32      offset = 0;
        
        if (!len)
        {
            result = Value(NumberUtil::NaN());
            return;
        }

        if (argc >= 2)
            if (!argv[1].Convert2Int32(radix))
                return;

        result = Value(NumberUtil::StringToInt(str.ToCStr(), len, radix, &offset));

//##protect##"instance::::parseInt()"
    }
    void GlobalObjectCPP::parseFloat(Value::Number& result, const ASString& str)
    {
//##protect##"instance::::parseFloat()"
        UInt32    len = str.GetSize();
        UInt32    offset = 0;
        const char* cstr = str.ToCStr();

        offset = static_cast<UInt32>(UTF8Util::GetByteIndex(ASUtils::SkipWhiteSpace(cstr), cstr, len));
        const char* numstr = cstr + offset;

        if (!strncmp(numstr, "0x", 2) || !strncmp(numstr, "0X", 2))
            // Ignore heximal.
            result = 0.0;
        else
            result = NumberUtil::StringToDouble(numstr, len - offset, &offset);
//##protect##"instance::::parseFloat()"
    }
    void GlobalObjectCPP::escape(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::::escape()"
        if (argc > 0)
        {
            const Value& v = argv[0];

            if (v.IsNullOrUndefined())
                result = GetVM().GetStringManager().GetBuiltin(AS3Builtin_null);
            else
            {
                ASString s = GetStringManager().CreateEmptyString();

                if (v.Convert2String(s))
                {
                    StringBuffer b;
                    ASUtils::AS3::Escape(s.ToCStr(), s.GetSize(), b);
                    result = GetVM().GetStringManager().CreateString(b.ToCStr(), b.GetSize());
                }
            }
        }
        else
            result = GetVM().GetStringManager().GetBuiltin(AS3Builtin_undefined);
//##protect##"instance::::escape()"
    }
    void GlobalObjectCPP::unescape(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::::unescape()"
        //NOT_IMPLEMENTED("::unescape()");

        if (argc > 0)
        {
            const Value& v = argv[0];

            if (v.IsNullOrUndefined())
                result = GetVM().GetStringManager().GetBuiltin(AS3Builtin_null);
            else
            {
                ASString s = GetStringManager().CreateEmptyString();

                if (v.Convert2String(s))
                {
                    StringBuffer b;
                    ASUtils::AS3::Unescape(s.ToCStr(), s.GetSize(), b);
                    result = GetVM().GetStringManager().CreateString(b.ToCStr(), b.GetSize());
                }
            }
        }
        else
            result = GetVM().GetStringManager().GetBuiltin(AS3Builtin_undefined);
//##protect##"instance::::unescape()"
    }
    void GlobalObjectCPP::isXMLName(bool& result, const Value& str)
    {
//##protect##"instance::::isXMLName()"
        if (str.IsNullOrUndefined())
        {
            result = false;
            return;
        }

        ASString name = GetVM().GetStringManager().CreateEmptyString();
        if(!str.Convert2String(name))
            GetVM().ThrowArgumentError(VM::Error(VM::eConvertToPrimitiveError, GetVM()));

        if (name.IsEmpty())
        {
            result = false;
            return;
        }

        wchar_t ch = (wchar_t)name.GetCharAt(0);
        if (!SFiswalpha(ch) && ch != '_')
        {
            result = false;
            return;
        }

        for (unsigned i = 1; i < name.GetLength(); i++)
        {
            wchar_t wch = (wchar_t)name.GetCharAt(i);
            if (SFiswdigit(wch) || SFiswalpha(wch) || (wch == '.') || (wch == '-') || (wch == '_'))
                continue;
            else
            {
                result = false;
                return;
            }
        }

        result = true;
//##protect##"instance::::isXMLName()"
    }
    void GlobalObjectCPP::trace(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::::trace()"
        // Redirect call to the *System*. Do it the same way as in AVMPlus.
        // !!! There is no trace in flash.system.
        //ClassSystem->trace(result, argc, argv);

        SF_UNUSED1(result);
        StringBuffer r;
        VM& vm = GetVM();

        for(unsigned i = 0; i < argc; ++i)
        {
            if (i > 0)
                r.AppendChar(' ');

#if 0 // DO NOT delete this code.
            // For some reason this code has problems.
            if (!argv[i].Convert2String(r))
                // Oops. Exception.
                return;
#else
            ASString tmp(GetStringManager().CreateEmptyString());
            if (!argv[i].Convert2String(tmp))
                // Oops. Exception.
                return;

            r.AppendString(String(tmp.ToCStr(), tmp.GetSize()));
#endif
        }

        r.AppendString("\n");

        // Output buffer by MaxMsgSize size pieces to prevent truncation.
        {
            // MovieRoot::Output() is using a buffer of size 2000. The rest is truncated.
            enum {MaxMsgSize = 2000};
            char tmp[MaxMsgSize];
            UPInt done = 0;
            UPInt left = r.GetSize();
            UPInt chunk_size;

            while (left)
            {
                chunk_size = Alg::Min(left, static_cast<UPInt>(MaxMsgSize) - 1);

                memcpy(tmp, r.ToCStr() + done, chunk_size);
                tmp[chunk_size] = 0;

                vm.GetUI().Output(FlashUI::Output_Message, tmp);

                done += chunk_size;
                left -= chunk_size;
            }
        }
//##protect##"instance::::trace()"
    }
    void GlobalObjectCPP::print(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::::print()"
        trace(result, argc, argv);
//##protect##"instance::::print()"
    }
#ifndef SF_AS3_NO_FLASH
    void GlobalObjectCPP::navigateToURL(const Value& result, Instances::fl_net::URLRequest* request, const ASString& window)
    {
//##protect##"instance::::navigateToURL()"

        SF_UNUSED2(result, window);

        MovieImpl* proot = static_cast<const ASVM&>(GetVM()).GetMovieImpl();
        UrlNavigator *urlnav = proot->GetUrlNavigator();
        if (!urlnav)
        {
            GFx::LogState* plog = static_cast<ASVM&>(GetVM()).GetLogState();
            if (plog) plog->LogWarning("navigateToURL failed! UrlNavigator state is not installed.");
            return;
        }

        SF_ASSERT(request);
        ASString url = request->urlGet();
        urlnav->NavigateToUrl(String(url.ToCStr(), url.GetSize()));

//##protect##"instance::::navigateToURL()"
    }
    void GlobalObjectCPP::getClassByAlias(SPtr<Class>& result, const ASString& aliasName)
    {
//##protect##"instance::::getClassByAlias()"
        VM& vm = GetVM();
        Class* cl = vm.GetClassByAlias(aliasName);

        if (cl)
            result = cl;
        else
        {
            vm.ThrowReferenceError(VM::Error(VM::eClassNotFoundError, vm
                SF_DEBUG_ARG(aliasName)
                ));
        }
//##protect##"instance::::getClassByAlias()"
    }
    void GlobalObjectCPP::registerClassAlias(const Value& result, const ASString& aliasName, Class* classObject)
    {
//##protect##"instance::::registerClassAlias()"
        SF_UNUSED1(result);

        if (classObject)
        {
            VM& vm = GetVM();
            vm.SetClassAlias(aliasName, *classObject);
        }
//##protect##"instance::::registerClassAlias()"
    }
    void GlobalObjectCPP::fscommand(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::::fscommand()"
        SF_UNUSED(result);
        MovieImpl* proot = static_cast<const ASVM&>(GetVM()).GetMovieImpl();
        GFx::FSCommandHandler *phandler = proot->pFSCommandHandler;
        if (phandler && (argc > 0))
        {
            ASString command(argv[0].AsString());
            ASString args((argc > 1) ? argv[1].AsString() :
                                       GetVM().GetStringManager().GetBuiltin(AS3Builtin_empty_));
            phandler->Callback(proot, command.ToCStr(), args.ToCStr());
        }
//##protect##"instance::::fscommand()"
    }
    void GlobalObjectCPP::clearInterval(const Value& result, UInt32 id)
    {
//##protect##"instance::::clearInterval()"
        SF_UNUSED2(result, id);
        MovieImpl* proot = static_cast<const ASVM&>(GetVM()).GetMovieImpl();
        proot->ClearIntervalTimer(int(id));
//##protect##"instance::::clearInterval()"
    }
    void GlobalObjectCPP::clearTimeout(const Value& result, UInt32 id)
    {
//##protect##"instance::::clearTimeout()"
        SF_UNUSED1(result);
        MovieImpl* proot = static_cast<const ASVM&>(GetVM()).GetMovieImpl();
        proot->ClearIntervalTimer(int(id));
//##protect##"instance::::clearTimeout()"
    }
    void GlobalObjectCPP::describeType(SPtr<Instances::fl::XML>& result, const Value& value)
    {
//##protect##"instance::::describeType()"
        VM& vm = GetVM();
#ifdef GFX_ENABLE_XML
        vm.GetXMLSupport().DescribeType(vm, result, value);
#else
        SF_UNUSED2(result, value);
        vm.ThrowVerifyError(VM::Error(VM::eNotImplementedError, GetVM() SF_DEBUG_ARG("describeType")));
#endif
//##protect##"instance::::describeType()"
    }
    void GlobalObjectCPP::escapeMultiByte(ASString& result, const ASString& value)
    {
//##protect##"instance::::escapeMultiByte()"
        escapeMultiByteInternal(GetVM(), result, value);
//##protect##"instance::::escapeMultiByte()"
    }
    void GlobalObjectCPP::getDefinitionByName(Value& result, const ASString& name)
    {
//##protect##"instance::::getDefinitionByName()"
#if 0 // DO NOT delete this code.
        // This code still has problems.
        // !!! We HAVE TO return "undefined" in case when name cannot be resolved.
        // Otherwise a lot of code won't work.
        Class* class_ = GetVM().GetClass(name.ToCStr(), GetVM().GetFrameAppDomain());
        if (class_)
            result = class_;
        else
            result.SetUndefined();
#else
        Value _class;
        
        // !!! GetVM().GetClass() shouldn't be used here because definition
        // can be located in a script and require initialization on demand.
        if (GetVM().GetClassUnsafe(name.ToCStr(), GetVM().GetFrameAppDomain(), _class))
            _class.Swap(result);
#endif
//##protect##"instance::::getDefinitionByName()"
    }
    void GlobalObjectCPP::getQualifiedClassName(ASString& result, const Value& value)
    {
//##protect##"instance::::getQualifiedClassName()"
        const InstanceTraits::Traits& itr = GetVM().GetInstanceTraits(value);
        result = itr.GetQualifiedName();
//##protect##"instance::::getQualifiedClassName()"
    }
    void GlobalObjectCPP::getQualifiedSuperclassName(Value& result, const Value& value)
    {
//##protect##"instance::::getQualifiedSuperclassName()"
        const InstanceTraits::Traits& itr = GetVM().GetInstanceTraits(value);

        if (itr.GetParent())
            result = itr.GetParent()->GetQualifiedName();
        else
            result.SetNull();
//##protect##"instance::::getQualifiedSuperclassName()"
    }
    void GlobalObjectCPP::getTimer(SInt32& result)
    {
//##protect##"instance::::getTimer()"
        MovieImpl* proot = static_cast<const ASVM&>(GetVM()).GetMovieImpl();
        result = (SInt32)proot->GetASTimerMs();

        // For testing of as3vm.
        //result = (SInt32)(Scaleform::Timer::GetTicks() % 100000000) / 1000;
//##protect##"instance::::getTimer()"
    }
    void GlobalObjectCPP::setInterval(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::::setInterval()"
        if (argc < 2)
            return GetVM().ThrowArgumentError(VM::Error(VM::eWrongArgumentCountError, GetVM() SF_DEBUG_ARG("GlobalObjectCPP::setInterval") SF_DEBUG_ARG(2) SF_DEBUG_ARG(SF_AS3_VARARGNUM) SF_DEBUG_ARG(argc)));

        UInt32 delay;
        if (!argv[1].Convert2UInt32(delay))
            return;
        Ptr<IntervalTimer> timer = *SF_HEAP_NEW(GetVM().GetMemoryHeap()) IntervalTimer(argv[0], delay);
        if (argc > 2)
            timer->SetArguments(argc - 2, &argv[2]);
        MovieImpl* proot = static_cast<const ASVM&>(GetVM()).GetMovieImpl();

        result = Value((UInt32)proot->AddIntervalTimer(timer));
        timer->Start(proot);
//##protect##"instance::::setInterval()"
    }
    void GlobalObjectCPP::setTimeout(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::::setTimeout()"
        SF_UNUSED3(result, argc, argv);
        if (argc < 2)
            return GetVM().ThrowArgumentError(VM::Error(VM::eWrongArgumentCountError, GetVM() SF_DEBUG_ARG("GlobalObjectCPP::setTimeout") SF_DEBUG_ARG(2) SF_DEBUG_ARG(SF_AS3_VARARGNUM) SF_DEBUG_ARG(argc)));

        UInt32 delay;
        if (!argv[1].Convert2UInt32(delay))
            return;
        Ptr<IntervalTimer> timer = *SF_HEAP_NEW(GetVM().GetMemoryHeap()) IntervalTimer(argv[0], delay, true);
        if (argc > 2)
            timer->SetArguments(argc - 2, &argv[2]);
        MovieImpl* proot = static_cast<const ASVM&>(GetVM()).GetMovieImpl();

        result = Value((UInt32)proot->AddIntervalTimer(timer));
        timer->Start(proot);
//##protect##"instance::::setTimeout()"
    }
    void GlobalObjectCPP::unescapeMultiByte(ASString& result, const ASString& value)
    {
//##protect##"instance::::unescapeMultiByte()"
        unescapeMultiByteInternal(GetVM(), result, value);
//##protect##"instance::::unescapeMultiByte()"
    }

#endif
//##end##"obj_global_cpp$package_methods"
    
//##markup##"obj_global_cpp$ConstPool"
//##begin##"obj_global_cpp$ConstPool"

    SInt32 GlobalObjectCPP::Ints[5] = {
        0,
        0x7fffffff,
        16777215,
        2000,
        -1,

    };

    UInt32 GlobalObjectCPP::UInts[8] = {
        0,
        10,
        1,
        255,
        7,
        0xFFFFFFFF,
        0xFFFF00,
        0xB,

    };

    Value::Number GlobalObjectCPP::Numbers[9] = {
        0,
        10,
        0x7FFFFFFF,
        1,
        NumberUtil::NaN(),
        NumberUtil::NEGATIVE_INFINITY(),
        NumberUtil::POSITIVE_INFINITY(),
        1.0,
        3,

    };

    const char* GlobalObjectCPP::Strings[12] = {
        "",
        ",",
        "originalPreferred",
        "informational",
        "null",
        "pad",
        "rgb",
        "evenOdd",
        "normal",
        "eulerAngles",
        "Filedata",
        "default",

    };

//##end##"obj_global_cpp$ConstPool"

    ///////////////////////////////////////////////////////////////////////////
#ifdef SF_AS3_CLASS_AS_SLOT
    void GlobalObjectCPP::AddFixedSlot(Class& cl)
    {
        UPInt index = 0;
        ASString name = cl.GetName();
        VM& vm = GetVM();
        ClassTraits::Traits* ctr = &cl.GetClassTraits();

        AddFixedSlotValuePair(
            name, 
            vm.MakePublicNamespace(),
            ctr, 
            Value(&cl),
            index
            );
    }

    const SlotInfo& GlobalObjectCPP::AddFixedSlotValuePair(const ASString& prop_name, Pickable<const Instances::fl::Namespace> ns,
        const ClassTraits::Traits* ctr, const Value& v, UPInt& index)
    {
        Values.PushBack(v);
        Traits& tr = GetTraits();

        // Add to Traits and bind ...
        tr.AddSlotValueCPP(prop_name, ns, ctr);
        // No dynamic slots should be added at this point.
        SF_ASSERT(Values.GetSize() == tr.GetFixedValueSlotNumber());

        // We MAY NOT use Resolve2ClassTraits here because this method can 
        // be called from GlobalObjectCPP.
        // if (GetVM().ClassTraitsSet.Get(prop_name, *ns) == NULL)
        GetVM().GetCurrentAppDomain().AddClassTrait(prop_name, *ns, const_cast<ClassTraits::Traits*>(ctr));

        index = tr.GetSlotInfoNum() - 1;
        return tr.GetSlotInfo(AbsoluteIndex(index));

    }
#endif

}} // namespace Instances

namespace InstanceTraits { namespace fl
{
    GlobalObject::GlobalObject(VM& vm)
    : CTraits(vm, CInfo)
    {
        SetConstructor(vm.GetClassObject());

        // Magic logic.
        // Global objects do not have stored scope.
        ClearInitScope();
    }

    void GlobalObject::MakeObject(Value& result, Traits& t)
    {
        SF_UNUSED2(result, t);
    }

    bool GlobalObject::IsGlobal() const
    {
        return true;
    }

    // Let's not set Abc flag up.
    const TypeInfo GlobalObject::TInfo = {
        TypeInfo::CompileTime | TypeInfo::DynamicObject,
        sizeof(GlobalObject::InstanceType),
        0, 0, 0, 0,
        "global", "", NULL,
        TypeInfo::None
    };
    const ClassInfo GlobalObject::CInfo = {&GlobalObject::TInfo, NULL};

    ///////////////////////////////////////////////////////////////////////////
    GlobalObjectScript::GlobalObjectScript(
        VMAbcFile& file, 
        VM& vm, 
        const Abc::ScriptInfo& script
        ) 
        : GlobalObject(vm)
        , File(&file)
        , Script(script)
    {
        // Instance size will be set by AddSlots().

        const Abc::HasTraits& object_traits = script;

        // MemSize doesn't depend on parent.
        if (!AddSlots(object_traits, file, GetFixedMemSize()))
            return;
    }

    void GlobalObjectScript::ForEachChild_GC(Collector* prcc, RefCountBaseGC<Mem_Stat>::GcOp op) const
    {
        GlobalObject::ForEachChild_GC(prcc, op);

        AS3::ForEachChild_GC<VMAbcFile, Mem_Stat>(prcc, File, op SF_DEBUG_ARG(*this));
    }

    Pickable<Instances::fl::GlobalObjectScript> GlobalObjectScript::MakeInstance()
    {
        // Placement new.
        return Pickable<Instances::fl::GlobalObjectScript>(new(Alloc()) Instances::fl::GlobalObjectScript(*this));
    }

    unsigned GlobalObjectScript::GetFixedMemSize() const
    {
        return sizeof(Instances::fl::GlobalObjectScript);
    }

    VMAbcFile* GlobalObjectScript::GetFilePtr() const
    {
        return File;
    }


}} // InstanceTraits

///////////////////////////////////////////////////////////////////////////
ASStringNode* VM::CreateConstStringNode(const char* msg)
{
    ASStringNode* snode;

    if (msg == NULL || SFstrlen(msg) == 0)
        snode = GetStringManager().GetEmptyStringNode();
    else
        snode = GetStringManager().GetStringManager()->
                    CreateConstStringNode(msg, SFstrlen(msg), 0);

    return snode;
}

}}} // namespace Scaleform { namespace GFx { namespace AS3
