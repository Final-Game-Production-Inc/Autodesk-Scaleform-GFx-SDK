/**************************************************************************

Filename    :   AS3_VM.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "AS3_VM.h"
#include "AS3_AsString.h"
#include "AS3_VMRead.h"
#include "AS3_VTable.h"
#include "AS3_Tracer.h"

// For VM::FormatErrorMessage
#include "Kernel/SF_MsgFormat.h" 
#include "Kernel/SF_Debug.h" 

#ifdef GFX_ENABLE_XML
    #include "Obj/AS3_Obj_XMLList.h"
#endif
#include "Obj/AS3_Obj_Function.h"
#include "Obj/AS3_Obj_Activation.h"
#include "Obj/AS3_Obj_Catch.h"
#include "Obj/AS3_Obj_Boolean.h"
#include "Obj/AS3_Obj_Number.h"
#include "Obj/AS3_Obj_int.h"
#include "Obj/AS3_Obj_uint.h"
#include "Obj/AS3_Obj_String.h"
#include "Obj/AS3_Obj_UserDefined.h"
#include "Obj/AS3_Obj_Array.h"
#include "Obj/AS3_Obj_QName.h"
#include "Obj/AS3_Obj_Namespace.h"
#include "Obj/AS3_Obj_Error.h"
#include "Obj/Vec/AS3_Obj_Vec_Vector.h"
#include "Obj/Vec/AS3_Obj_Vec_Vector_int.h"
#include "Obj/Vec/AS3_Obj_Vec_Vector_uint.h"
#include "Obj/Vec/AS3_Obj_Vec_Vector_double.h"
#include "Obj/Vec/AS3_Obj_Vec_Vector_String.h"
#include "Obj/Vec/AS3_Obj_Vec_Vector_object.h"
#include "Obj/AS3_Obj_Global.h"
#include "Obj/AS3_Obj_EvalError.h"
#include "Obj/AS3_Obj_RangeError.h"
#include "Obj/AS3_Obj_ReferenceError.h"
#include "Obj/AS3_Obj_SecurityError.h"
#include "Obj/AS3_Obj_SyntaxError.h"
#include "Obj/AS3_Obj_TypeError.h"
#include "Obj/AS3_Obj_URIError.h"
#include "Obj/AS3_Obj_VerifyError.h"
#include "Obj/AS3_Obj_UninitializedError.h"
#include "Obj/AS3_Obj_ArgumentError.h"
#include "Obj/AS3_Obj_Global.h"
#include "Obj/AS3_Obj_Domain.h"
#include "Obj/Errors/AS3_Obj_Errors_MemoryError.h"
#include "Obj/System/AS3_Obj_System_ApplicationDomain.h"
#include "Obj/Utils/AS3_Obj_Utils_ByteArray.h"

// Define SF_TRACE_FILE_LOADING if you want to trace file loading/unloading process.
// #define SF_TRACE_FILE_LOADING

namespace Scaleform { namespace GFx { namespace AS3 
{

#if defined(GFX_AS3_VERBOSE)

    static int errorMappingTable[] =
    {
        1000,
        1001,
        1002,
        1003,
        1004,
        1005,
        1006,
        1007,
        1008,
        1009,
        1010,
        1011,
        1012,
        1013,
        1014,
        1015,
        1016,
        1017,
        1018,
        1019,
        1020,
        1021,
        1022,
        1023,
        1024,
        1025,
        1026,
        1027,
        1028,
        1029,
        1030,
        1031,
        1032,
        1033,
        1034,
        1035,
        1037,
        1038,
        1039,
        1040,
        1041,
        1042,
        1043,
        1044,
        1045,
        1046,
        1047,
        1049,
        1050,
        1051,
        1052,
        1053,
        1054,
        1056,
        1057,
        1058,
        1059,
        1060,
        1061,
        1063,
        1064,
        1065,
        1066,
        1067,
        1068,
        1069,
        1070,
        1071,
        1072,
        1073,
        1074,
        1075,
        1076,
        1077,
        1078,
        1079,
        1080,
        1081,
        1082,
        1083,
        1084,
        1085,
        1086,
        1087,
        1088,
        1089,
        1090,
        1091,
        1092,
        1093,
        1094,
        1095,
        1096,
        1097,
        1098,
        1100,
        1101,
        1102,
        1103,
        1104,
        1107,
        1108,
        1109,
        1110,
        1111,
        1112,
        1113,
        1114,
        1115,
        1116,
        1117,
        1118,
        1119,
        1120,
        1121,
        1122,
        1123,
        1124,
        1125,
        1126,
        1127,
        1128,
        1500,
        1501,
        1502,
        1503,
        1504,
        1505,
        1506,
        1507,
        1508,
        1509,
        1510,

        2006,
        2007,
        2008,
        2012,
        2015,
        2024,
        2025,
        2027,
        2030,
        2058,

        2077,

        2088,
        2089,
        2090,
        2091,
        2092,
        2093,
        2105,
        2106,
        2107,

        2108,
        2109,

        2136,
    };

    enum { eNumErrorConstants = sizeof(errorMappingTable) / sizeof(int) };

    static const char* errorConstants[eNumErrorConstants] =
    { 
        /* 1000,0*/ "The system is out of memory.",
        /* 1001,1*/ "The method {0} is not implemented.",
        /* 1002,2*/ "Number.toPrecision has a range of 1 to 21. Number.toFixed and Number.toExponential have a range of 0 to 20. Specified value is not within expected range.",
        /* 1003,3*/ "The radix argument must be between 2 and 36; got {0}.",
        /* 1004,4*/ "Method {0} was invoked on an incompatible object.",
        /* 1005,5*/ "Array index is not a positive integer ({0}).",
        /* 1006,6*/ "{0} is not a function.",
        /* 1007,7*/ "Instantiation attempted on a non-constructor.",
        /* 1008,8*/ "{0} is ambiguous; Found more than one matching binding.",
        /* 1009,9*/ "Cannot access a property or method of a null object reference.",
        /* 1010,10*/ "A term is undefined and has no properties.",
        /* 1011,11*/ "Method {0} contained illegal opcode {1} at offset {2}.",
        /* 1012,12*/ "The last instruction exceeded code size.",
        /* 1013,13*/ "Cannot call OP_findproperty when scopeDepth is 0.",
        /* 1014,14*/ "Class {0} could not be found.",
        /* 1015,15*/ "Method {0} cannot set default xml namespace",
        /* 1016,16*/ "Descendants operator (..) not supported on type {0}.",
        /* 1017,17*/ "Scope stack overflow occurred.",
        /* 1018,18*/ "Scope stack underflow occurred.",
        /* 1019,19*/ "Getscopeobject {0} is out of bounds.",
        /* 1020,20*/ "Code cannot fall off the end of a method.",
        /* 1021,21*/ "At least one branch target was not on a valid instruction in the method.",
        /* 1022,22*/ "Type void may only be used as a function return type.",
        /* 1023,23*/ "Stack overflow occurred.",
        /* 1024,24*/ "Stack underflow occurred.",
        /* 1025,25*/ "An invalid register {0} was accessed.",
        /* 1026,26*/ "Slot {0} exceeds slotCount={1} of {2}.",
        /* 1027,27*/ "Method_info {0} exceeds method_count={1}.",
        /* 1028,28*/ "Disp_id {0} exceeds max_disp_id={1} of {2}.",
        /* 1029,29*/ "Disp_id {0} is undefined on {1}.",
        /* 1030,30*/ "Stack depth is unbalanced. {0} != {1}.",
        /* 1031,31*/ "Scope depth is unbalanced. {0} != {1}.",
        /* 1032,32*/ "Cpool index {0} is out of range {1}.",
        /* 1033,33*/ "Cpool entry {0} is wrong type.",
        /* 1034,34*/ "Type Coercion failed: cannot convert {0} to {1}.",
        /* 1035,35*/ "Illegal super expression found in method {0}.",
        /* 1037,36*/ "Cannot assign to a method {0} on {1}.",
        /* 1038,37*/ "{0} is already defined.",
        /* 1039,38*/ "Cannot verify method until it is referenced.",
        /* 1040,39*/ "The right-hand side of instanceof must be a class or function.",
        /* 1041,40*/ "The right-hand side of operator must be a class.",
        /* 1042,41*/ "Not an ABC file.  major_version={0} minor_version={1}.",
        /* 1043,42*/ "Invalid code_length={0}.",
        /* 1044,43*/ "MethodInfo-{0} unsupported flags={1}.",
        /* 1045,44*/ "Unsupported traits kind={0}.",
        /* 1046,45*/ "MethodInfo-{0} referenced before definition.",
        /* 1047,46*/ "No entry point was found.",
        /* 1049,47*/ "Prototype objects must be vanilla Objects.",
        /* 1050,48*/ "Cannot convert {0} to primitive.",
        /* 1051,49*/ "Illegal early binding access to {0}.",
        /* 1052,50*/ "Invalid URI passed to {0} function.",
        /* 1053,51*/ "Illegal override of {0} in {1}.",
        /* 1054,52*/ "Illegal range or target offsets in exception handler.",
        /* 1056,53*/ "Cannot create property {0} on {1}.",
        /* 1057,54*/ "{0} can only contain methods.",
        /* 1058,55*/ "Illegal operand type: {0} must be {1}.",
        /* 1059,56*/ "ClassInfo-{0} is referenced before definition.",
        /* 1060,57*/ "ClassInfo {0} exceeds class_count={1}.",
        /* 1061,58*/ "The value {0} cannot be converted to {1} without losing precision.",
        /* 1063,59*/ "Argument count mismatch on {0}. Expected from {1} to {2}, got {3}.",
        /* 1064,60*/ "Cannot call method {0} as constructor.",
        /* 1065,61*/ "Variable {0} is not defined.",
        /* 1066,62*/ "The form function('function body') is not supported.",
        /* 1067,63*/ "Native method {0} has illegal method body.",
        /* 1068,64*/ "{0} and {1} cannot be reconciled.",
        /* 1069,65*/ "Property {0} not found on {1} and there is no default value.",
        /* 1070,66*/ "Method {0} not found on {1}",
        /* 1071,67*/ "Function {0} has already been bound to {1}.",
        /* 1072,68*/ "Disp_id 0 is illegal.",
        /* 1073,69*/ "Non-override method {0} replaced because of duplicate disp_id {1}.",
        /* 1074,70*/ "Illegal write to read-only property {0} on {1}.",
        /* 1075,71*/ "Math is not a function.",
        /* 1076,72*/ "Math is not a constructor.",
        /* 1077,73*/ "Illegal read of write-only property {0} on {1}.",
        /* 1078,74*/ "Illegal opcode/multiname combination: {0}<{1}>.",
        /* 1079,75*/ "Native methods are not allowed in loaded code.",
        /* 1080,76*/ "Illegal value for namespace.",
        /* 1081,77*/ "Property {0} not found on {1} and there is no default value.",
        /* 1082,78*/ "No default namespace has been set.",
        /* 1083,79*/ "The prefix \"{0}\" for element \"{1}\" is not bound.",
        /* 1084,80*/ "Element or attribute (\"{0}\") does not match QName production: QName::=(NCName':')?NCName.",
        /* 1085,81*/ "The element type \"{0}\" must be terminated by the matching end-tag \"</{1}>\".",
        /* 1086,82*/ "The {0} method only works on lists containing one item.",
        /* 1087,83*/ "Assignment to indexed XML is not allowed.",
        /* 1088,84*/ "The markup in the document following the root element must be well-formed.",
        /* 1089,85*/ "Assignment to lists with more than one item is not supported.",
        /* 1090,86*/ "XML parser failure: element is malformed.",
        /* 1091,87*/ "XML parser failure: Unterminated CDATA section.",
        /* 1092,88*/ "XML parser failure: Unterminated XML declaration.",
        /* 1093,89*/ "XML parser failure: Unterminated DOCTYPE declaration.",
        /* 1094,90*/ "XML parser failure: Unterminated comment.",
        /* 1095,91*/ "XML parser failure: Unterminated attribute.",
        /* 1096,92*/ "XML parser failure: Unterminated element.",
        /* 1097,93*/ "XML parser failure: Unterminated processing instruction.",
        /* 1098,94*/ "Illegal prefix {0} for no namespace.",
        /* 1100,95*/ "Cannot supply flags when constructing one RegExp from another.",
        /* 1101,96*/ "Cannot verify method {0} with unknown scope.",
        /* 1102,97*/ "Illegal default value for type {0}.",
        /* 1103,98*/ "Class {0} cannot extend final base class.",
        /* 1104,99*/ "Attribute \"{0}\" was already specified for element \"{1}\".",
        /* 1107,100*/ "The ABC data is corrupt, attempt to read out of bounds.",
        /* 1108,101*/ "The OP_newclass opcode was used with the incorrect base class.",
        /* 1109,102*/ "Attempt to directly call unbound function {0} from method {1}.",
        /* 1110,103*/ "{0} cannot extend {1}.",
        /* 1111,104*/ "{0} cannot implement {1}.",
        /* 1112,105*/ "Argument count mismatch on class coercion.  Expected 1, got {0}.",
        /* 1113,106*/ "OP_newactivation used in method without NEED_ACTIVATION flag.",
        /* 1114,107*/ "OP_getglobalslot or OP_setglobalslot used with no global scope.",
        /* 1115,108*/ "{0} is not a constructor.",
        /* 1116,109*/ "second argument to Function.prototype.apply must be an array.",
        /* 1117,110*/ "Invalid XML name: {0}.",
        /* 1118,111*/ "Illegal cyclical loop between nodes.",
        /* 1119,112*/ "Delete operator is not supported with operand of type {0}.",
        /* 1120,113*/ "Cannot delete property {0} on {1}.",
        /* 1121,114*/ "Method {0} has a duplicate method body.",
        /* 1122,115*/ "Interface method {0} has illegal method body.",
        /* 1123,116*/ "Filter operator not supported on type {0}.",
        /* 1124,117*/ "OP_hasnext2 requires object and index to be distinct registers.",
        /* 1125,118*/ "The index {0} is out of range {1}.",
        /* 1126,119*/ "Cannot change the length of a fixed Vector.",
        /* 1127,120*/ "Type application attempted on a non-parameterized type.",
        /* 1128,121*/ "Incorrect number of type parameters for {0}. Expected {1}, got {2}.",
        /* 1500,122*/ "Error occurred opening file {0}.",
        /* 1501,123*/ "Error occurred writing to file {0}.",
        /* 1502,124*/ "A script has executed for longer than the default timeout period of 15 seconds.",
        /* 1503,125*/ "A script failed to exit after 30 seconds and was terminated.",
        /* 1504,126*/ "End of file.",
        /* 1505,127*/ "The string index {0} is out of bounds; must be in range {1} to {2}.",
        /* 1506,128*/ "The specified range is invalid.",
        /* 1507,129*/ "Argument {0} cannot be null.",
        /* 1508,130*/ "The value specified for argument {0} is invalid.",
        /* 1509,131*/ "There was an error decompressing the data.",
        /* 1510,132*/ "When the callback argument is a method of a class, the optional this argument must be null.",

        /* 2006,133*/ "The supplied index is out of bounds.",
        /* 2007,134*/ "Parameter {0} must be non-null.",
        /* 2008,135*/ "Parameter {0} must be one of the accepted values.",
        /* 2012,136*/ "{0} class cannot be instantiated.",
        /* 2015,137*/ "Argument error: {0}.",
        /* 2024,138*/ "An object cannot be added as a child of itself.",
        /* 2025,139*/ "The supplied {0} must be a child of the caller.",
        /* 2027,140*/ "{0} must be non negative.",
        /* 2030,141*/ "End of file was encountered.",
        /* 2058,142*/ "There was an error decompressing the data.",
        /* 2077,143*/ "Illegal operation: {0}.",

        /* 2088,144*/ "The Proxy class does not implement getProperty. It must be overridden by a subclass.",
        /* 2089,145*/ "The Proxy class does not implement setProperty. It must be overridden by a subclass.",
        /* 2090,146*/ "The Proxy class does not implement callProperty. It must be overridden by a subclass.",
        /* 2091,147*/ "The Proxy class does not implement hasProperty. It must be overridden by a subclass.",
        /* 2092,148*/ "The Proxy class does not implement deleteProperty. It must be overridden by a subclass.",
        /* 2093,149*/ "The Proxy class does not implement getDescendants. It must be overridden by a subclass.",
        /* 2105,150*/ "The Proxy class does not implement nextNameIndex. It must be overridden by a subclass.",
        /* 2106,151*/ "The Proxy class does not implement nextName. It must be overridden by a subclass.",
        /* 2107,152*/ "The Proxy class does not implement nextValue. It must be overridden by a subclass.",

        /* 2108,153*/ "Scene {0} was not found.",
        /* 2109,154*/ "Frame label {0} was not found in scene {1}.",

        /* 2136,155*/ "The SWF file {0} contains invalid data.",
    };

    const char* GetErrorMsg(const int ind)
    {
        // Do binary search.
        int min = 0;
        int max = eNumErrorConstants - 1;

        while (min <= max)
        {
            const int mid = (min + max) >> 1;
            const int curValue = errorMappingTable[mid];

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

        int ident = errorMappingTable[min];

        if (ident == ind)
        {
			int index = min;
            return errorConstants[index];
        }

        return NULL;
    }
#endif


///////////////////////////////////////////////////////////////////////////
VMFile::VMFile(VM& vm, VMAppDomain& appDomain)
    : GASRefCountBase(&vm.GetGC())
    , VMRef(vm)
    , AppDomain(&appDomain)
{
    // String with index zero is empty.
    IntStrings.PushBack(vm.GetStringManager().GetEmptyStringNode());
}

VMFile::~VMFile()
{
}

void VMFile::ForEachChild_GC(Collector* prcc, GcOp op) const
{
    IntNamespaces.ForEachChild_GC(prcc, op SF_DEBUG_ARG(*this));

    // IntNamespaceSets
    {
        UPInt size = IntNamespaceSets.GetSize();
        for (UPInt i = 0; i < size; ++i)
            AS3::ForEachChild_GC<NamespaceSet, Mem_Stat>(prcc, IntNamespaceSets[i], op SF_DEBUG_ARG(*this));
    }

    // ActivationTraitsCache
    {
        TActivationTraitsCache::ConstIterator it = ActivationTraitsCache.Begin();
        for (; !it.IsEnd(); ++it)
            AS3::ForEachChild_GC<InstanceTraits::Traits, Mem_Stat>(prcc, it->Second, op SF_DEBUG_ARG(*this));
    }

    // LoadedClasses
    {
        UPInt size = LoadedClasses.GetSize();
        for (UPInt i = 0; i < size; ++i)
            AS3::ForEachChild_GC<ClassTraits::Traits, Mem_Stat>(prcc, LoadedClasses[i], op SF_DEBUG_ARG(*this));
    }
    AS3::ForEachChild_GC<VMAppDomain, Mem_Stat>(prcc, AppDomain, op SF_DEBUG_ARG(*this));
}

Instances::fl::Namespace& VMFile::GetInternedNamespace(const Abc::Multiname& mn)
{
    SPtr<Instances::fl::Namespace>* pNs = IntNamespaces.Get(mn);

    if (pNs == NULL)
    {
        IntNamespaces.Add(mn, MakeInternedNamespace(mn));
        pNs = IntNamespaces.Get(mn);
        SF_ASSERT(pNs);
    }

    return **pNs;
}

Instances::fl::Namespace& VMFile::GetInternedNamespace(UPInt nsIndex)
{
    SPtr<Instances::fl::Namespace>* pNs = IntNamespaces.Get(nsIndex);

    if (pNs == NULL)
    {
        IntNamespaces.Add(nsIndex, MakeInternedNamespace(nsIndex));
        pNs = IntNamespaces.Get(nsIndex);
        SF_ASSERT(pNs);
    }

    return **pNs;
}

NamespaceSet& VMFile::GetInternedNamespaceSet(UPInt nsSetIndex)
{
    if (nsSetIndex >= IntNamespaceSets.GetSize())
        IntNamespaceSets.Resize(nsSetIndex + 1);

    // Initialize.
    if (!IntNamespaceSets[nsSetIndex])
        IntNamespaceSets[nsSetIndex] = MakeInternedNamespaceSet(nsSetIndex);

    return *IntNamespaceSets[nsSetIndex];
}

ASString VMFile::GetInternedString(UPInt strIndex)
{
    if (strIndex >= IntStrings.GetSize())
        IntStrings.Resize(strIndex + 1);

    if (IntStrings[strIndex].GetPtr() == NULL)
        IntStrings[strIndex].SetPtr(MakeInternedString(strIndex).GetNode());

    return ASString(IntStrings[strIndex]);
}

InstanceTraits::Traits& VMFile::GetActivationInstanceTraits(const Abc::MbiInd mbi_ind SF_DEBUG_ARG(const ASString& name))
{
    TActivationTraitsCache::ValueType* ptr = ActivationTraitsCache.Get(mbi_ind);

    if (ptr == NULL)
    {
        Pickable<InstanceTraits::Traits> t = MakeActivationInstanceTraits(mbi_ind SF_DEBUG_ARG(name));

        ActivationTraitsCache.Add(mbi_ind, t);
        return *t;
    }

    return **ptr;
}

void VMFile::Clear()
{
    IntNamespaces.Clear();
    IntNamespaceSets.Clear();
    IntStrings.Clear();
    ActivationTraitsCache.Clear();
    ASStringNodeSet.Clear();
}

///////////////////////////////////////////////////////////////////////////

// ***** VMAbcFile Implementation

VMAbcFile::VMAbcFile(
    VM& vm, 
    const Ptr<Abc::File>& file, 
    VMAppDomain& appDomain 
    SF_AOTC_ARG(AOT::InfoCollector* ic)
    SF_AOTC2_ARG(AOT::InfoCollector* ic)
    )
    : VMFile(vm, appDomain)
    , File(file)
    SF_AOTC_ARG(pIC(ic))
    SF_AOTC2_ARG(pIC(ic))
{
    SetHasFinalize();
    SF_ASSERT(File.GetPtr() != NULL);

    // We HAVE TO resize OpCodeArray and Exceptions here because we need to use
    // reference to an element. (It will be invalidated during resizing of Array)
    const UPInt size = file->GetMethodBodyTable().GetSize();
    MethodInfoArray.Resize(size);

    vm.AddVMAbcFileWeak(this);
}

VMAbcFile::~VMAbcFile()
{
    VM& vm = GetVM();

    VM::AliasClassType::Iterator ait;

    for (TAliasClass::ConstIterator it = AliasClass.Begin(); !it.IsEnd(); ++it)
    {
        ait = vm.AliasClass.Find(*it);

        if (ait.IsEnd())
            continue;

        vm.AliasClass.Remove(*it);
        vm.ClassAlias.Remove(ait->Second);
    }

    // SF_DEBUG_MESSAGE2(GFX_UNLOAD_TRACE_ENABLED, "~VMAbcFile %s %p\n", File->GetName().ToCStr(), this);
    vm.RemoveVMAbcFileWeak(this);
}

void VMAbcFile::Finalize_GC()
{
    // SF_DEBUG_MESSAGE2(GFX_UNLOAD_TRACE_ENABLED, "VMAbcFile::Finalize %s %p\n", File->GetName().ToCStr(), this);
    UnRegister();
}

Pickable<Instances::fl::Namespace> VMAbcFile::MakeInternedNamespace(const Abc::Multiname& mn) const
{
    const Abc::ConstPool& cp = GetAbcFile().GetConstPool();
    const Abc::NamespaceInfo& info = mn.GetNamespace(cp);
    const StringDataPtr& uri = info.GetNameURI();
    StringManager& sm = GetVM().GetStringManager();
    ASString URI = sm.CreateString(uri.ToCStr(), uri.GetSize());

    if (mn.GetNextMultinameIndex() >= 0)
    {
        const Abc::Multiname& next_mn = mn.GetNextMultiname(cp);
        const Abc::NamespaceInfo& next_info = next_mn.GetNamespace(cp);
        const StringDataPtr& next_uri = next_info.GetNameURI();

        URI += "$";
        URI += sm.CreateString(next_uri.ToCStr(), next_uri.GetSize());
    }

    return GetVM().MakeInternedNamespace(info.GetKind(), URI);
}

Pickable<Instances::fl::Namespace> VMAbcFile::MakeInternedNamespace(UPInt nsIndex) const
{
    const Abc::NamespaceInfo& info = GetAbcFile().GetConstPool().GetNamespace(nsIndex);
    return GetVM().MakeInternedNamespace(info.GetKind(), info.GetNameURI());
}

Pickable<NamespaceSet> VMAbcFile::MakeInternedNamespaceSet(UPInt nsSetIndex)
{
    Pickable<NamespaceSet> nss(SF_HEAP_NEW_ID(GetVM().GetMemoryHeap(), StatMV_VM_VMAbcFile_Mem) NamespaceSet(GetVM()));
    const Abc::NamespaceSetInfo& info = GetAbcFile().GetConstPool().GetNamespaceSet(nsSetIndex);

    // Fill it up with data.
    const UInt8* ptr = info.GetData();
    UInt32 size;
    UInt32 ns_ind;

    Abc::Read(ptr, size);
    for(UInt32 i = 0; i < size; ++i)
    {
        Abc::Read(ptr, ns_ind);
        nss->Add(GetInternedNamespace(ns_ind));
    }

    return nss;
}

Pickable<InstanceTraits::Traits> VMAbcFile::MakeActivationInstanceTraits(const Abc::MbiInd mbi_ind SF_DEBUG_ARG(const ASString& name))
{
    VM& vm = GetVM();
    const Abc::MethodBodyInfo& mbi = GetMethodBody().Get(mbi_ind);

    return Pickable<InstanceTraits::Traits>(SF_HEAP_NEW_ID(vm.GetMemoryHeap(), StatMV_VM_ITraits_Mem) InstanceTraits::Activation(*this, vm, mbi SF_DEBUG_ARG(name)));
}

ASString VMAbcFile::MakeInternedString(UPInt strIndex) const
{
    const StringDataPtr str = GetAbcFile().GetConstPool().GetString(AbsoluteIndex(strIndex));
    // !!! This should be CreateConstString().
    return GetVM().GetStringManager().CreateString(str.ToCStr(), str.GetSize());
}

void VMAbcFile::ForEachChild_GC(Collector* prcc, GcOp op) const
{
    VMFile::ForEachChild_GC(prcc, op);

    UPInt i = 0;
    UPInt size = 0;

#ifdef SF_AS3_ENABLE_GETABSOBJECT
    // AbsObjects
    {
        HashSetLH<SPtr<GASRefCountBase> >::ConstIterator it = AbsObjects.Begin();
        for (; !it.IsEnd(); ++it)
            AS3::ForEachChild_GC<GASRefCountBase, Mem_Stat>(prcc, *it, op SF_DEBUG_ARG(*this));
    }
#endif

    // GlobalObjects
    {
        HashSetLH<SPtr<Instances::fl::GlobalObjectScript> >::ConstIterator it = GlobalObjects.Begin();
        for (; !it.IsEnd(); ++it)
            AS3::ForEachChild_GC<Instances::fl::GlobalObjectScript, Mem_Stat>(prcc, *it, op SF_DEBUG_ARG(*this));
    }

    // Children
    {
        size = Children.GetSize();
        for (i = 0; i < size; ++i)
            AS3::ForEachChild_GC<VMAbcFile, Mem_Stat>(prcc, Children[i], op SF_DEBUG_ARG(*this));
    }

    // FunctionTraitsCache
    {
        TFunctionTraitsCache::ConstIterator it = FunctionTraitsCache.Begin();
        for (; !it.IsEnd(); ++it)
            AS3::ForEachChild_GC<InstanceTraits::Function, Mem_Stat>(prcc, it->Second, op SF_DEBUG_ARG(*this));
    }
}

bool VMAbcFile::RegisterUserDefinedClassTraits()
{
    const Abc::ClassTable& classes = GetClasses();
    UPInt numOfClasses = 0;
    VM& vm = GetVM();

#ifdef SF_TRACE_FILE_LOADING
    FlashUI& ui = vm.GetUI();
#endif

    // Filter out scripts, which
    // 1) initialize classes, which are not registered;
    // 2) duplicate identical scripts from other files.

    // Register uninitialized traits,
    const UPInt csize = classes.GetSize();
    for (UPInt i = 0; i < csize; ++i)
    {
        const Abc::ClassInfo& class_info = classes.Get(i);
        const Abc::Multiname& name = class_info.GetName(GetConstPool());
        const ASString str_name = GetInternedString(name.GetNameInd());
        const Instances::fl::Namespace& ns = GetInternedNamespace(name.GetNamespaceInd());

        // check for *scaleform extension* packages.
        if (IsScaleformGFx(ns))
            // Skip it.
            continue;

        const ClassTraits::Traits* ctr = vm.Resolve2ClassTraits(str_name, ns, GetAppDomain());
        if (ctr == NULL)
        {
            // Class is not registered yet.
            Pickable<ClassTraits::Traits> ud(SF_HEAP_NEW_ID(vm.GetMemoryHeap(), StatMV_VM_CTraits_Mem) ClassTraits::UserDefined(*this, vm, class_info));
            AddClassTrait(str_name, ns, ud.GetPtr());
            ++numOfClasses;
            RegisterLoadedClass(ud);

#ifdef SF_TRACE_FILE_LOADING
            ui.Output(FlashUI::Output_Action, "Registering class ");
            if (!ns.GetUri().IsEmpty())
            {
                ui.Output(FlashUI::Output_Action, ns.GetUri().ToCStr());
                ui.Output(FlashUI::Output_Action, "::");
            }
            ui.Output(FlashUI::Output_Action, str_name.ToCStr());
            ui.Output(FlashUI::Output_Action, "\n");
#endif
        }
        else
		{
//             if (vm.AppDomainSupported)
//             {
//                 // Replace current traits by new ones, as per flash behavior (not documentation)
//                 Pickable<ClassTraits::Traits> ud(SF_HEAP_NEW_ID(vm.GetMemoryHeap(), StatMV_VM_CTraits_Mem) ClassTraits::UserDefined(*this, vm, class_info));
//                 AppDomain->SetClassTrait(str_name, ns, ud.GetPtr());
//                 ++numOfClasses;
//                 LoadedClasses.PushBack(ud);
//             }

            // This class is already loaded from another file. Skip it.
#ifdef SF_TRACE_FILE_LOADING
            ui.Output(FlashUI::Output_Action, "Class ");
            if (!ns.GetUri().IsEmpty())
            {
                ui.Output(FlashUI::Output_Action, ns.GetUri().ToCStr());
                ui.Output(FlashUI::Output_Action, "::");
            }
            ui.Output(FlashUI::Output_Action, str_name.ToCStr());
            ui.Output(FlashUI::Output_Action, " is already registered with another file (");
            ui.Output(FlashUI::Output_Action, ctr->GetFilePtr()->GetAbcFile().GetName().ToCStr());
            ui.Output(FlashUI::Output_Action, ").\n");
#endif

            // Add file, which defines this class to the list of children.
            VMAbcFile* file = ctr->GetFilePtr();
            if (file)
            {
                bool alreadyDependsOn = false;
                const UPInt jsize = Children.GetSize();
                for (UPInt j = 0; j < jsize; ++j)
                {
                    if (Children[j].GetPtr() == file)
                    {
                        alreadyDependsOn = true;
                        break;
                    }
                }

                if (!alreadyDependsOn)
                    Children.PushBack(file);
            }
        }
    }

    if (numOfClasses == 0 && csize != 0)
        return false;

    bool uninitialize = false;

    // Initialize uninitialized traits,
    for (UPInt i = 0; i < csize; ++i)
    {
        const Abc::ClassInfo& class_info = classes.Get(i);
        const Abc::Multiname& name = class_info.GetName(GetConstPool());
        const ASString str_name = GetInternedString(name.GetNameInd());
        const Instances::fl::Namespace& ns = GetInternedNamespace(name.GetNamespaceInd());

        if (IsScaleformGFx(ns))
            // Skip it.
            continue;

        /* Another way of doing the same thing.
        const ClassTraits::UserDefined* ud = static_cast<const ClassTraits::UserDefined*>(Resolve2ClassTraits(str_name, ns));

        // Filter out classes, which are defined in other files. This happens.
        if (ud && (&ud->GetFile() == &file))
            ud->Initialize();
        */

        ClassTraits::UserDefined& ud = vm.GetUserDefinedTraits(*this, class_info);

        // Filter out classes, which are defined in other files. This happens.
        if (&ud.GetFile() != this)
		{
#ifdef SF_TRACE_FILE_LOADING
            ui.Output(FlashUI::Output_Action, "Class ");
            if (!ns.GetUri().IsEmpty())
            {
                ui.Output(FlashUI::Output_Action, ns.GetUri().ToCStr());
                ui.Output(FlashUI::Output_Action, "::");
            }
            ui.Output(FlashUI::Output_Action, str_name.ToCStr());
            ui.Output(FlashUI::Output_Action, " is already registered with another file.\n");
#endif
            continue;
		}

        // Initialize() can throw exceptions.
        if (!ud.Initialize())
        {
            if (vm.IsException())
                vm.OutputAndIgnoreException();

            // Delete ClassTraits.
            RemoveClassTrait(str_name, ns);
            --numOfClasses;
            uninitialize = true;
            break;
        }
    }

    // UnRegister whole file.
    if (uninitialize)
    {
        // Remove traits, which may be not completely initialized because of an exception,
        UnregisterUserDefinedClassTraits();
        return false;
    }

    return (csize == 0 || (csize != 0 && numOfClasses != 0));
}

void VMAbcFile::UnregisterUserDefinedClassTraits()
{
#ifdef SF_TRACE_FILE_LOADING
    VM& vm = GetVM();
    FlashUI& ui = vm.GetUI();
    ui.Output(FlashUI::Output_Action, "\nFile ");
    ui.Output(FlashUI::Output_Action, File->GetSource().ToCStr());
    ui.Output(FlashUI::Output_Action, " will be completely unregistered.\n");
#endif

    const UPInt size = GetNumOfLoadedClasses();
    for (UPInt i = 0; i < size; ++i)
    {
        ClassTraits::Traits& ctr = *GetLoadedClass(i);

        if (ctr.IsValid())
        {
            // Fast way.
            InstanceTraits::Traits& itr = ctr.GetInstanceTraits();
            const ASString str_name = itr.GetName();
            const Instances::fl::Namespace& ns = itr.GetNamespace();

#ifdef SF_TRACE_FILE_LOADING
            ui.Output(FlashUI::Output_Action, "Unregistering class ");
            if (!ns.GetUri().IsEmpty())
            {
                ui.Output(FlashUI::Output_Action, ns.GetUri().ToCStr());
                ui.Output(FlashUI::Output_Action, "::");
            }
            ui.Output(FlashUI::Output_Action, str_name.ToCStr());
            ui.Output(FlashUI::Output_Action, "\n");
#endif

            RemoveClassTrait(str_name, ns);
        }
        else
        {
            // Slow way.
            RemoveClassTraitValue(&ctr);
        }
    }

    ClearLoadedClasses();
}

bool VMAbcFile::RegisterScrips(bool to_execute)
{
    VM& vm = GetVM();

    // Register scripts.
    const Abc::File& file = GetAbcFile();
    const UPInt n = file.GetScripts().GetSize();
    for (UPInt i = 0; i < n; ++i)
    {
        const Abc::ScriptInfo& script = file.GetScripts().Get(i);

        SPtr<InstanceTraits::fl::GlobalObjectScript> it;

        // Can throw exceptions.
        it.Pick(SF_HEAP_NEW_ID(vm.GetMemoryHeap(), StatMV_VM_ITraits_Mem) InstanceTraits::fl::GlobalObjectScript(
            *this,
            vm, 
            script
            ));

        if (vm.IsException())
            return false;

        SPtr<Instances::fl::GlobalObjectScript> inst = it->MakeInstance();

        if (vm.IsException())
            return false;

        vm.RegisterGlobalObject(*inst);

        // Register this global object with its file.
        GlobalObjects.Set(inst);

        inst->InitUserDefinedClassTraits();
    }

#if defined(SF_AS3_AOTC2)
    SF_UNUSED1(to_execute);
#else
    // Execute last script if necessary.
    if (to_execute && n > 0)
        vm.GlobalObjects.Back()->Execute();
#endif

    return !vm.IsException();
}

CheckResult VMAbcFile::Load(bool to_execute)
{
#ifdef SF_TRACE_FILE_LOADING
    FlashUI& ui = GetVM().GetUI();
    ui.Output(FlashUI::Output_Action, "\nRegistering file: ");
    ui.Output(FlashUI::Output_Action, File->GetSource().ToCStr());
    ui.Output(FlashUI::Output_Action, "\n");
#endif

    // Register class traits.
    // Register scripts.
    return RegisterUserDefinedClassTraits() && RegisterScrips(to_execute);
}

void VMAbcFile::UnRegister()
{
    if (!GetVM().RemoveVMAbcFileWeak(this))
        return; // already unregistered

    // Get rid of class traits.
    UnregisterUserDefinedClassTraits();

    // Destroy generated opcode because it holds pointers to objects.
    MethodInfoArray.Clear();

#ifdef SF_AS3_ENABLE_GETABSOBJECT
    // Clear AbsObjects because there is no optimized opcode any more.
    AbsObjects.Clear();
#endif

    // Get rid of Global Objects.
    HashSetLH<SPtr<Instances::fl::GlobalObjectScript> >::Iterator it = GlobalObjects.Begin();
    for (; !it.IsEnd(); ++it)
    {
        Instances::fl::GlobalObjectScript* o_ptr = *it;
        SF_ASSERT(o_ptr);
        GetVM().UnregisterGlobalObject(*o_ptr);
    }
    GlobalObjects.Clear();

    // Clear the rest.
    
    // LoadedClasses is cleared by UnregisterUserDefinedClassTraits().
    // ??? Children should not be cleared.
    Children.Clear();
    FunctionTraitsCache.Clear();
    VMFile::Clear();

    if (GetAppDomain().IsEmpty())
    {
        bool removed = GetVM().RemoveAppDomain(GetAppDomain());
        SF_UNUSED(removed);
    }
}

const Abc::TOpCode& VMAbcFile::GetOpCode(const CallFrame& cf)
{
    VM& vm = GetVM();
    const Abc::MbiInd ind = cf.GetMethodBodyInd();
    SF_ASSERT(ind.IsValid() && static_cast<UPInt>(ind.Get()) < MethodInfoArray.GetSize());
    MethodInfo& mi = MethodInfoArray[ind.Get()];

    if (mi.OpCode.IsEmpty())
    {
        Tracer tracer(
            vm.GetMemoryHeap(), 
            cf, 
            mi
            SF_AOTC_ARG(pIC)
            SF_AOTC2_ARG(pIC)
        );

        if (!vm.IsException())
            tracer.EmitCode();
    }

#ifdef SF_AS3_EMIT_DEF_ARGS
    if (!cf.OpStackIsAllocated)
    {
        VM::OpStackType& stack = vm.OpStack;

        // !!! We need this PLUS ONE at this time because return and returnvoid
        // put result on method's stack.
        stack.Reserve(static_cast<UInt16>(mi.MaxOpStackSize + 1));

        cf.OpStackIsAllocated = true;
    }
#endif

    return mi.OpCode;
}

Value VMAbcFile::GetDetailValue(const Abc::ValueDetail& d)
{
    const int value_ind = d.GetIndex();

    // If we have a value ...
    if (value_ind > 0)
    {
        switch(d.GetKind())
        {
        case Abc::CONSTANT_Int:
            return Value(GetConstPool().GetInt(value_ind));
        case Abc::CONSTANT_UInt:
            return Value(GetConstPool().GetUInt(value_ind));
        case Abc::CONSTANT_Double:
            return Value(MakeValueNumber(GetConstPool().GetDouble(value_ind)));
        case Abc::CONSTANT_Utf8:
            return Value(GetInternedString(value_ind));
        case Abc::CONSTANT_True:
            return Value(true);
        case Abc::CONSTANT_False:
            return Value(false);
        case Abc::CONSTANT_Null:
            return Value::GetNull();
        case Abc::CONSTANT_Undefined:
            return Value::GetUndefined();
        case Abc::CONSTANT_Namespace:
        case Abc::CONSTANT_PackageNamespace:
        case Abc::CONSTANT_PackageInternalNs:
        case Abc::CONSTANT_ProtectedNamespace:
        case Abc::CONSTANT_ExplicitNamespace:
        case Abc::CONSTANT_StaticProtectedNs:
        case Abc::CONSTANT_PrivateNs:
            return Value(&GetInternedNamespace(d.GetIndex()));
        default:
            SF_ASSERT(false);
            break;
        }
    }

    return Value::GetUndefined();
}

namespace fl
{
    extern const ClassInfo FunctionCI;
    extern const ClassInfo FunctionCICpp;
}

InstanceTraits::Function&
VMAbcFile::GetFunctionInstanceTraits(Instances::fl::GlobalObjectScript& gos, UInt32 method_ind)
{
#if 0
    VM& vm = GetVM();
    const Abc::MethodBodyInfo& mbi = GetMethodBodyInfo(method_ind);
    if (mbi.GetTraitsCount())
    {
        TFunctionTraitsCache::ValueType* ptr = FunctionTraitsCache.Get(method_ind);

        if (ptr == NULL)
        {
            Pickable<InstanceTraits::Function> t(SF_HEAP_NEW_ID(vm.GetMemoryHeap(), StatMV_VM_ITraits_Mem) InstanceTraits::Function(*this, vm, FunctionCI, mbi));

            if (vm.IsException())
                return *vm.NoFunctionTraits;

            FunctionTraitsCache.Add(method_ind, t);

            return *t;
        }

        return **ptr;
    }

    return *vm.NoFunctionTraits;
#else
    TFunctionTraitsCache::ValueType* ptr = FunctionTraitsCache.Get(method_ind);

    if (ptr == NULL)
    {
        VM& vm = GetVM();
        Pickable<InstanceTraits::Function> t(SF_HEAP_NEW_ID(vm.GetMemoryHeap(), StatMV_VM_ITraits_Mem) InstanceTraits::Function(*this, AS3::fl::FunctionCI, method_ind, gos));

        if (vm.IsException())
            return *vm.NoFunctionTraits;

        FunctionTraitsCache.Add(method_ind, t);

        return *t;
    }

    return **ptr;
#endif
}


const Abc::Multiname& VMAbcFile::GetMultiname(UPInt ind) const
{
    return GetConstPool().GetMultiname(ind);
}

///////////////////////////////////////////////////////////////////////////
XMLSupport::~XMLSupport()
{
}

void XMLSupport::ForEachChild_GC(Collector* prcc, RefCountBaseGC<Mem_Stat>::GcOp op) const
{
    SF_UNUSED2(prcc, op);
}

CheckResult XMLSupport::EqualsXML(bool& result, Instances::fl::XML& /*l*/, Instances::fl::XML& /*r*/) const
{
    result = false;
    return false;
}

CheckResult XMLSupport::EqualsXML(bool& stop, bool& result, Instances::fl::XML& /*l*/, const Value& /*r*/) const
{
    stop = true;
    result = false;
    return false;
}

CheckResult XMLSupport::EqualsXMLList(bool& result, Instances::fl::XMLList& /*l*/, const Value& /*v*/) const
{
    result = false;
    return false;
}

CheckResult XMLSupport::EqualsQName(bool& result, Instances::fl::QName& /*l*/, Instances::fl::QName& /*r*/) const
{
    result = false;
    return false;
}

bool XMLSupport::Add(Value& result, Object& l, Object& r) const
{
    SF_UNUSED3(result, l, r);

    return false;
}

CheckResult XMLSupport::ToXMLString(VM& vm, Value& v) const
{
    SF_UNUSED2(vm, v);

    return false;
}

void XMLSupport::DescribeType(VM& vm, SPtr<Instances::fl::XML>& result, const Value& value) const
{
    SF_UNUSED3(vm, result, value);
}

const ClassTraits::Traits* XMLSupport::GetClassTraitsXML() const
{
    return NULL;
}

InstanceTraits::Traits* XMLSupport::GetITraitsXML() const
{
    return NULL;
}

const ClassTraits::Traits* XMLSupport::GetClassTraitsXMLList() const
{
    return NULL;
}

InstanceTraits::Traits* XMLSupport::GetITraitsXMLList() const
{
    return NULL;
}

///////////////////////////////////////////////////////////////////////////
// Original isdigit() may assert if c > 127.
inline bool IsDigit(char c)
{
    return ('0' <= c && c <= '9');
}

///////////////////////////////////////////////////////////////////////////
CheckResult GetArrayInd(ASStringNode* sn, UInt32& ind)
{
    const unsigned ssize = sn->Size;

    if (ssize > 0)
    {
        const char* sdata = sn->pData;
        char c = sdata[0];

        if (c == '0')
        {
            ind = 0;
            return ssize == 1;
        }

        if (IsDigit(c))
        {
            for (unsigned i = 1; i < ssize; ++i)
            {
                if (!IsDigit(sdata[i]))
                    return false;
            }

            char* end;
            Value::Number num = strtod(sdata, &end);

            if (num <= SF_MAX_UINT32)
            {
                ind = (UInt32)num;
                return true;
            }
        } 
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////
CheckResult GetArrayInd(const Multiname& prop_name, UInt32& ind)
{
    bool result = false;
    const Value& name = prop_name.GetName();

    if (name.IsString())
        result = GetArrayInd(name.GetStringNode(), ind);
    else if (name.IsNumeric())
        result = name.Convert2UInt32(ind);

    return result;
}

///////////////////////////////////////////////////////////////////////////
CheckResult GetVectorInd(ASStringNode* sn, UInt32& ind)
{
    bool result = false;

    Value::Number num;
    if (GetStrNumber(sn, num))
    {
        if (num <= SF_MAX_UINT32)
        {
            ind = (UInt32)num;
            result = true;
        }
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////
CheckResult GetVectorInd(const Multiname& prop_name, UInt32& ind)
{
    bool result = false;
    const Value& name = prop_name.GetName();

    /* Old version, which is copied over from GetArrayInd().
    if (name.IsString())
        result = GetArrayInd(name.GetStringNode(), ind);
    else if (name.IsNumeric())
        result = name.Convert2UInt32(ind);
    */

    switch (name.GetKind())
    {
    case Value::kString:
        result = GetVectorInd(name.GetStringNode(), ind);
        break;
    case Value::kUInt:
        ind = name.AsUInt();
        result = true;
        break;
    case Value::kInt:
        if (name.AsInt() >= 0)
        {
            ind = static_cast<UInt32>(name.AsInt());
            result = true;
        }
        break;
    case Value::kNumber:
        {
            Value::Number n;
            const Value::Number num = name.AsNumber();

            if (num >= 0 && ::modf(num, reinterpret_cast<double*>(&n)) == 0.0 && num <= SF_MAX_UINT32)
            {
                ind = static_cast<UInt32>(num);
                result = true;
            }
        }
        break;
    default:
        break;
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////
CheckResult GetStrNumber(ASStringNode* sn, Value::Number& result)
{
    if (sn == NULL)
        return false;

    const unsigned ssize = sn->Size;

    if (ssize > 0)
    {
        const char* sdata = sn->pData;
        char c = sdata[0];
        if (IsDigit(c))
        {
            for (unsigned i = 1; i < ssize; ++i)
            {
                c = sdata[i];
                if (!IsDigit(c))
                {
                    if (c == '.')
                    {
                        for (unsigned j = i + 1; j < ssize; ++j)
                        {
                            if (sdata[j] != '0')
                                return false;
                        }

                        // Everything is fine.
                        break;
                    }

                    return false;
                }
            }

            char* end;
            result = strtod(sdata, &end);

            return true;
        } 
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////
CallFrame::CallFrame(MemoryHeap* heap)
: DiscardResult(false)
#ifdef SF_AS3_EMIT_DEF_ARGS
, OpStackIsAllocated(false)
#endif
#ifndef SF_AS3_ENABLE_CALLFRAME_CACHE
, ACopy(false)
#endif
SF_DEBUG_ARG(RegisteredFunction(false))
, ScopeStackBaseInd(0)
, CP(0)
#ifdef GFX_AS3_TRACE
, OCPtr(NULL)
#endif
, pRegisterFile(NULL)
, pHeap(heap)
, pFile(NULL)
, MBIIndex(0)
, pSavedScope(NULL)
, OriginationTraits(NULL)
, pScopeStack(NULL)
, PrevInitialStackPos(NULL)
#ifdef SF_AS3_ENABLE_EXPLICIT_GO
, GOS(NULL)
#endif
#ifdef GFX_ENABLE_XML
, DefXMLNamespace(NULL)
#endif
, PrevFirstStackPos(NULL)
SF_DEBUG_ARG(PrevReservedNum(0))
SF_DEBUG_ARG(CurrFileInd(0))
SF_DEBUG_ARG(CurrLineNumber(0))
SF_DEBUG_ARG(StartTicks(0))
{
    // VM is not available at this time.
    // SF_ASSERT(GetVM().OpStack.IsValid());
}

#ifdef SF_AS3_ENABLE_CALLFRAME_CACHE

CallFrame::~CallFrame()
{
    SF_ASSERT(GetVM().OpStack.IsValid());
}

#else

CallFrame::CallFrame(const CallFrame& other)
: DiscardResult(other.DiscardResult)
#ifdef SF_AS3_EMIT_DEF_ARGS
, OpStackIsAllocated(other.OpStackIsAllocated)
#endif
, ACopy(true) // ACopy must be set to true unconditionally.
SF_DEBUG_ARG(RegisteredFunction(other.RegisteredFunction))
, ScopeStackBaseInd(other.ScopeStackBaseInd)
, CP(other.CP)
#ifdef GFX_AS3_TRACE
, OCPtr(other.OCPtr)
#endif
, pRegisterFile(other.pRegisterFile)
, pHeap(other.pHeap)
, pFile(other.pFile)
, MBIIndex(other.MBIIndex)
, pSavedScope(other.pSavedScope)
, OriginationTraits(other.OriginationTraits)
, pScopeStack(other.pScopeStack)
, PrevInitialStackPos(other.PrevInitialStackPos)
#ifdef SF_AS3_ENABLE_EXPLICIT_GO
, GOS(other.GOS)
#endif
#ifdef GFX_ENABLE_XML
, DefXMLNamespace(other.DefXMLNamespace)
#endif
, PrevFirstStackPos(other.PrevFirstStackPos)
SF_DEBUG_ARG(PrevReservedNum(other.PrevReservedNum))
SF_DEBUG_ARG(Name(other.Name))
SF_DEBUG_ARG(CurrFileInd(0))
SF_DEBUG_ARG(CurrLineNumber(0))
SF_DEBUG_ARG(StartTicks(other.StartTicks))
, Invoker(other.Invoker)
{
    // VM is not available at this time.
    // SF_ASSERT(GetVM().OpStack.IsValid());
}

CallFrame::~CallFrame()
{
    if (pRegisterFile && pScopeStack && ACopy)
    {
        VM::OpStackType& stack = GetVM().OpStack;
        stack.ReleaseReserved(PrevFirstStackPos SF_DEBUG_ARG(PrevReservedNum));
        pRegisterFile->ReleaseReserved(static_cast<UInt16>(GetMethodBodyInfo().GetMaxLocalRegisterIndex()));
        ClearScopeStack();

#ifdef GFX_ENABLE_XML
        GetVM().SetDefXMLNamespace(GetDefXMLNamespace());
#endif

#ifdef SF_BUILD_DEBUG
        SF_ASSERT(stack.IsValid());
#endif
    }
}

CallFrame& CallFrame::operator =(const CallFrame& other)
{
    if (this != &other)
    {
        DiscardResult = other.DiscardResult;
#ifdef SF_AS3_EMIT_DEF_ARGS
        OpStackIsAllocated = other.OpStackIsAllocated;
#endif
        ACopy = true;
        SF_DEBUG_CODE(RegisteredFunction = other.RegisteredFunction;)
        ScopeStackBaseInd = other.ScopeStackBaseInd;
        CP = other.CP;
#ifdef GFX_AS3_TRACE
        OCPtr = other.OCPtr;
#endif
        pRegisterFile = other.pRegisterFile;
        pHeap = other.pHeap;
        pFile = other.pFile;
        MBIIndex = other.MBIIndex;
        pSavedScope = other.pSavedScope;
        OriginationTraits = other.OriginationTraits;
        pScopeStack = other.pScopeStack;
        PrevInitialStackPos = other.PrevInitialStackPos;
#ifdef SF_AS3_ENABLE_EXPLICIT_GO
        GOS = other.GOS;
#endif
#ifdef GFX_ENABLE_XML
        DefXMLNamespace = other.DefXMLNamespace;
#endif
        PrevFirstStackPos = other.PrevFirstStackPos;
        SF_DEBUG_CODE(PrevReservedNum = other.PrevReservedNum;)
        SF_DEBUG_CODE(Name = other.Name;)
        SF_DEBUG_CODE(CurrFileInd = other.CurrFileInd;)
        SF_DEBUG_CODE(CurrLineNumber = other.CurrLineNumber;)
        SF_DEBUG_CODE(StartTicks = other.StartTicks;)
        Invoker = other.Invoker;
    }

#ifdef SF_BUILD_DEBUG
    SF_ASSERT(GetVM().OpStack.IsValid());
#endif

    return *this;
}
#endif

Abc::TOpCodeVTP CallFrame::GetCP() const
{
    if (CP == NULL)
        CP = GetCode().GetDataPtr();

    return CP;
}

void CallFrame::SetupRegisters(const Abc::MethodInfo& mi, const Value& _this, unsigned argc, const Value* argv)
{
    // Register 0 holds the "this" object. This value is never null.
    int reg = 0;
    SetRegister(reg++, (_this.IsNullOrUndefined() ? Value(GetGlobalObject()) : _this));
    
    const UPInt param_count = mi.GetParamCount();
    const UPInt first_opt_param_num = param_count - mi.GetOptionalParamCount();
    
    // Registers 1 through method_info.param_count hold parameter values coerced to the declared types
    // of the parameters. If fewer than method_body_info.local_count values are supplied to the call then
    // the remaining values are either the values provided by default value declarations (optional arguments)
    // or the value undefined.
    
    VM& vm = GetVM();
    for(unsigned i = 0; i < param_count; ++i)
    {
        // Coerce to the argument's type ...

        // Resolve multiname ...
        const Abc::Multiname& mn = mi.GetParamType(GetFile().GetConstPool(), i);
        const ClassTraits::Traits* to = vm.Resolve2ClassTraits(GetFile(), mn);

        if (to == NULL)
        {
            return vm.ThrowReferenceError(VM::Error(VM::eClassNotFoundError, vm
                SF_DEBUG_ARG(mn.GetName(GetFile().GetConstPool()))
                ));
        }

        Value result = Value::GetUndefined();

        if (i < argc)
        {
            if (!to->Coerce(argv[i], result))
            {
                // A TypeError is thrown if value cannot be coerced to the specified type.
                return vm.ThrowTypeError(VM::Error(VM::eCheckTypeFailedError, vm
                    SF_DEBUG_ARG(argv[i])
                    SF_DEBUG_ARG(to->GetName().ToCStr())
                    ));
            }
        } else
        {
            // No more arguments from stack ...
            if(mi.HasOptionalParams() && i >= first_opt_param_num)
            {
                if (!to->Coerce(GetFile().GetDetailValue(mi.GetOptionalParam(i - first_opt_param_num)), result))
                {
                    // A TypeError is thrown if value cannot be coerced to the specified type.
                    return vm.ThrowTypeError(VM::Error(VM::eCheckTypeFailedError, vm
                        SF_DEBUG_ARG(GetFile().GetDetailValue(mi.GetOptionalParam(i - first_opt_param_num)))
                        SF_DEBUG_ARG(to->GetName().ToCStr())
                        ));
                }
            }
            else if (to != &vm.GetClassTraitsClassClass())
            {
                return vm.ThrowArgumentError(VM::Error(VM::eWrongArgumentCountError, vm
                    SF_DEBUG_ARG(Name->pData)
                    SF_DEBUG_ARG(static_cast<int>(first_opt_param_num))
                    SF_DEBUG_ARG(static_cast<int>(param_count))
                    SF_DEBUG_ARG(argc)
                    ));
            }
        }

        // Store value ...
        SetRegister(reg++, result);
    }

    if (mi.NeedRest())
    {
        // If NEED_REST is set in method_info.flags, the method_info.param_count+1 register is set up to
        // reference an array that holds the superfluous arguments.

        Pickable<Instances::fl::Array> arr = vm.MakeArray();
        // Store arr ASAP to prevent problems with GC in case of exception.
        SetRegister(reg++, arr);

        if (argc > param_count)
            arr->Append((unsigned)(argc - param_count), argv + param_count);
    } else if (mi.NeedArguments())
    {
        // If NEED_ARGUMENTS is set in method_info.flags, the method_info.param_count+1 register is set up
        // to reference an "arguments" object that holds all the actual arguments: see ECMA-262 for more
        // information. (The AVM2 is not strictly compatible with ECMA-262; it creates an Array object for
        // the "arguments" object, whereas ECMA-262 requires a plain Object.)

        // "arguments" is a regular Array. There is no an explicit "arguments" class.
        Pickable<Instances::fl::Array> arr = vm.MakeArray();
        // Store arr ASAP to prevent problems with GC in case of exception.
        SetRegister(reg++, arr);

        // Two registers are taken by "this" and "array of arguments".
        // Everything in between can be copied into arr.
        const unsigned avail_regn = reg - 2;

        // Populate arr with data.
        for (unsigned i = 0; i < argc; ++i)
        {
            if (i < avail_regn)
                arr->PushBack(GetRegister(i + 1));
            else
                // We have more arguments than declared types.
                arr->PushBack(argv[i]);
        }

        // Add property "callee".
        {
            Value callee;

            Value::KindType vk = Invoker.GetKind();
            switch (vk)
            {
            case Value::kVTableInd:
                SF_ASSERT(_this.GetObject());
                // Make a MethodIndClosure.
                callee = Value(_this.GetObject(), Invoker.GetVTableInd(), false);
                break;
            case Value::kFunction:
#if 0
                // DO NOT delete code below.
                // Code below is not correct.
                // We shouldn't create a MethodClosure for a Function.
                // Make a method closure by storing "this".
                callee = vm.GetClassFunction().MakeInstance2(Invoker, _this);
#else
                callee = Invoker;
#endif
                break;
            default:
                callee = Invoker;
            }

            arr->AddDynamicSlotValuePair(
                vm.GetStringManager().CreateConstString("callee"), 
                callee
                );
        }
    }
}

void CallFrame::ClearOpStack()
{
    // At the end OpStack should contain zero values in case of 
    // DiscardResult == true and one value otherwise. Not normalized stack
    // occurs in case of try {...} finally { return value; }

    VM& vm = GetVM();
    VM::OpStackType& stack = vm.OpStack;

    stack.PopReserved(PrevInitialStackPos, !DiscardResult && !vm.IsException());
}

#ifdef SF_AS3_ENABLE_CALLFRAME_CACHE

void CallFrame::Clear()
{
    SF_ASSERT(pRegisterFile);
    SF_ASSERT(pScopeStack);

    Invoker.SetUndefined();
    VM::OpStackType& stack = GetVM().OpStack;
    stack.ReleaseReserved(SF_DEBUG_CODE(PrevFirstStackPos) SF_DEBUG_ARG(PrevReservedNum));
    const Abc::MethodBodyInfo& mbi = GetMethodBodyInfo();
    pRegisterFile->ReleaseReserved(static_cast<UInt16>(mbi.GetMaxLocalRegisterIndex()));
    ClearScopeStack();
}

///////////////////////////////////////////////////////////////////////////
CallFrameCache::CallFrameCache(MemoryHeap* heap) : Heap(heap), CallFrames(heap) 
{
}

CallFrameCache::~CallFrameCache()
{
    const UPInt size = CallFrames.GetSize();
    for (UPInt i = 0; i < size; ++i)
        delete CallFrames[i];
}

CallFrame* CallFrameCache::MakeCallFrame()
{
    CallFrame* result = NULL;
    if (CallFrames.IsEmpty())
        result = SF_HEAP_NEW(Heap) CallFrame(Heap);
    else
    {
        result = CallFrames.Back();
        CallFrames.PopBack();
    }

    return result;
}

void CallFrameCache::DestroyCallFrame(CallFrame* cf)
{
    SF_ASSERT(cf);
    cf->Clear();
    CallFrames.PushBack(cf);
}

#endif

Instances::fl::GlobalObjectScript* CallFrame::GetGlobalObject() const
{
#ifdef SF_AS3_ENABLE_EXPLICIT_GO
    return &GetGlobalObjectScript();
#else
    const ScopeStackType& sss = GetSavedScope();

    if (!sss.IsEmpty())
        return static_cast<Instances::fl::GlobalObjectScript*>(sss[0].GetObject());

    const Value& reg0 = GetRegister(0);
    if (reg0.IsObject())
        return static_cast<Instances::fl::GlobalObjectScript*>(reg0.GetObject());

    return NULL;
#endif
}

///////////////////////////////////////////////////////////////////////////
#ifdef SF_ENABLE_APP_DOMAIN
bool VMAppDomain::Enabled = true;
#else
bool VMAppDomain::Enabled = false;
#endif

VMAppDomain::VMAppDomain(VM& vm, VMAppDomain* parentDomain) 
    : GASRefCountBase(&vm.GetGC())
    , ParentDomain(NULL)
    , ClassTraitsSet(vm.GetMemoryHeap())
{
    if (parentDomain != NULL)
        parentDomain->AddChild(*this);
}

VMAppDomain::~VMAppDomain()
{
}

void VMAppDomain::AddChild(VMAppDomain& childDomain)
{
    SF_ASSERT(IsEnabled());  // Should not be adding children if App Domain is not enabled
    ChildDomains.PushBack(&childDomain);
    childDomain.ParentDomain = this;
}
SPtr<VMAppDomain> VMAppDomain::AddNewChild(VM& vm)
{
    if (IsEnabled())
        return MakePickable(SF_HEAP_NEW_ID(vm.GetMemoryHeap(), StatMV_VM_VM_Mem) VMAppDomain(vm, this));
    else
        return this;
}

VMAppDomain* VMAppDomain::GetParent() const
{
    return ParentDomain;
}

bool VMAppDomain::RemoveChild(VMAppDomain& childDomain)
{
    for (UPInt i = 0; i < ChildDomains.GetSize(); ++i)
    {
        if (ChildDomains[i] == &childDomain)
        {
            ChildDomains.RemoveAt(i);
            return true;
        }

        if (ChildDomains[i]->RemoveChild(childDomain))
            return true;
    }
    return false;
}


ClassTraits::Traits** VMAppDomain::GetClassTrait(const ASString& name, const Instances::fl::Namespace& ns)
{
    ClassTraits::Traits** classTrait = NULL;

    // Search from Root to Child. ???
    if (ParentDomain != NULL)
        classTrait = ParentDomain->GetClassTrait(name, ns);

    if (classTrait == NULL)
        classTrait = ClassTraitsSet.Get(name, ns);

    return classTrait;
}

ClassTraits::Traits** VMAppDomain::GetClassTraitNoParent(const ASString& name, const Instances::fl::Namespace& ns)
{
    return ClassTraitsSet.Get(name, ns);
}

ClassTraits::Traits** VMAppDomain::GetClassTrait(const Multiname& mn)
{
    ClassTraits::Traits** classTrait = NULL;

    // Search from Root to Child. ???
    if (ParentDomain != NULL)
        classTrait = ParentDomain->GetClassTrait(mn);

    if (classTrait == NULL)
        classTrait = ClassTraitsSet.Get(mn);

    return classTrait;
}

void VMAppDomain::AddClassTrait(const ASString& name, const Instances::fl::Namespace& ns, ClassTraits::Traits* val)
{
    ClassTraitsSet.Add(name, ns, val);
}

void VMAppDomain::SetClassTrait(const ASString& name, const Instances::fl::Namespace& ns, ClassTraits::Traits* val)
{
    ClassTraitsSet.Set(name, ns, val);
}

void VMAppDomain::RemoveClassTrait(const ASString& name, const Instances::fl::Namespace& ns)
{
    ClassTraitsSet.Remove(name, ns);
}

void VMAppDomain::RemoveClassTraitValue(ClassTraits::Traits* val)
{
    ClassTraitsSet.RemoveValue(val);
}

void VMAppDomain::GetMultinames(Array<SPtr<const Instances::fl::Namespace> >& multinames) const
{
    ClassTraitsSet.GetMultinames(multinames);
    for (UPInt i = 0; i < ChildDomains.GetSize(); ++i)
        ChildDomains[i]->GetMultinames(multinames);
}

bool VMAppDomain::IsEmpty() const
{
    if (!ClassTraitsSet.IsEmpty())
        return false;

    if (!ChildDomains.IsEmpty())
        return false;

    return true;
}

void VMAppDomain::ForEachChild_GC(Collector* prcc, GcOp op) const
{
    for (UPInt i = 0; i < ChildDomains.GetSize(); ++i)
        ChildDomains[i]->ForEachChild_GC(prcc, op);
}

///////////////////////////////////////////////////////////////////////////
VM::VM(FlashUI& _ui, StringManager& sm, ASRefCountCollector& gc SF_AOTC_ARG(AOT::InfoCollector* ic) SF_AOTC2_ARG(AOT::InfoCollector* ic))
: Initialized(false)
, InDestructor(false)
, LoadingAbcFile(false)
, StringManagerRef(sm)
, GC(gc)
, UI(_ui)
, InInitializer(0)
, MHeap(Memory::GetHeapByAddress(&GetSelf()))
, XMLSupport_(Pickable<XMLSupport>(SF_HEAP_NEW(GetMemoryHeap()) XMLSupport(GetSelf())))
#ifdef SF_AS3_ENABLE_CALLFRAME_CACHE
, CFCache(GetMemoryHeap())
#endif
, ScopeStack(GetMemoryHeap())
, HandleException(false)
, GlobalObjects()
, CallStack(GetMemoryHeap())

#ifdef SF_AMP_SERVER
, ActiveLineTimestamp(0)
#endif
, SystemDomain(MakePickable(SF_HEAP_NEW_ID(GetMemoryHeap(), StatMV_VM_VM_Mem) VMAppDomain(GetSelf(), NULL)))
, CurrentDomain(VMAppDomain::IsEnabled() ? MakePickable(SF_HEAP_NEW_ID(GetMemoryHeap(), StatMV_VM_VM_Mem) VMAppDomain(GetSelf(), SystemDomain)) : SystemDomain)

, PublicNamespace(MakePickable(SF_HEAP_NEW(GetMemoryHeap()) Instances::fl::Namespace(GetSelf(), Abc::NS_Public, "")))
, AS3Namespace(MakePickable(SF_HEAP_NEW(GetMemoryHeap()) Instances::fl::Namespace(GetSelf(), Abc::NS_Public, NS_AS3)))
, VectorNamespace(MakePickable(SF_HEAP_NEW(GetMemoryHeap()) Instances::fl::Namespace(GetSelf(), Abc::NS_Public, NS_Vector)))
, ProxyNamespace(MakePickable(SF_HEAP_NEW(GetMemoryHeap()) Instances::fl::Namespace(GetSelf(), Abc::NS_Public, NS_flash_proxy)))
, XMLNamespace(MakePickable(SF_HEAP_NEW(GetMemoryHeap()) Instances::fl::Namespace(GetSelf(), Abc::NS_Public, NS_XML)))

, TraitsClassClass(PrepareClassTraits(SF_HEAP_NEW_ID(GetMemoryHeap(), StatMV_VM_CTraits_Mem) ClassTraits::ClassClass(GetSelf())))
, TraitsObject(PrepareClassTraits(static_cast<ClassTraits::fl::Object*>(ClassTraits::fl::Object::MakeClassTraits(GetSelf()).GetPtr())))
, TraitsNamespace(PrepareClassTraits(static_cast<ClassTraits::fl::Namespace*>(ClassTraits::fl::Namespace::MakeClassTraits(GetSelf()).GetPtr())))
, TraitsFunction(PrepareClassTraits(static_cast<ClassTraits::Function*>(ClassTraits::Function::MakeClassTraits(GetSelf()).GetPtr())))
, TraitsBoolean(PrepareClassTraits(static_cast<ClassTraits::fl::Boolean*>(ClassTraits::fl::Boolean::MakeClassTraits(GetSelf()).GetPtr())))
, TraitsNumber(PrepareClassTraits(static_cast<ClassTraits::fl::Number*>(ClassTraits::fl::Number::MakeClassTraits(GetSelf()).GetPtr())))
, TraitsInt(PrepareClassTraits(static_cast<ClassTraits::fl::int_*>(ClassTraits::fl::int_::MakeClassTraits(GetSelf()).GetPtr())))
, TraitsUint(PrepareClassTraits(static_cast<ClassTraits::fl::uint*>(ClassTraits::fl::uint::MakeClassTraits(GetSelf()).GetPtr())))
, TraitsString(PrepareClassTraits(static_cast<ClassTraits::fl::String*>(ClassTraits::fl::String::MakeClassTraits(GetSelf()).GetPtr())))
, TraitsArray(PrepareClassTraits(static_cast<ClassTraits::fl::Array*>(ClassTraits::fl::Array::MakeClassTraits(GetSelf()).GetPtr())))
, TraitsQName(PrepareClassTraits(static_cast<ClassTraits::fl::QName*>(ClassTraits::fl::QName::MakeClassTraits(GetSelf()).GetPtr())))
, TraitsCatch(PrepareClassTraits(static_cast<ClassTraits::fl::Catch*>(ClassTraits::fl::Catch::MakeClassTraits(GetSelf()).GetPtr())))
, TraitsVector(PrepareClassTraits(static_cast<ClassTraits::fl_vec::Vector*>(ClassTraits::fl_vec::Vector::MakeClassTraits(GetSelf()).GetPtr())))
, TraitsVector_int(PrepareClassTraits(static_cast<ClassTraits::fl_vec::Vector_int*>(ClassTraits::fl_vec::Vector_int::MakeClassTraits(GetSelf()).GetPtr())))
, TraitsVector_uint(PrepareClassTraits(static_cast<ClassTraits::fl_vec::Vector_uint*>(ClassTraits::fl_vec::Vector_uint::MakeClassTraits(GetSelf()).GetPtr())))
, TraitsVector_Number(PrepareClassTraits(static_cast<ClassTraits::fl_vec::Vector_double*>(ClassTraits::fl_vec::Vector_double::MakeClassTraits(GetSelf()).GetPtr())))
, TraitsVector_String(PrepareClassTraits(static_cast<ClassTraits::fl_vec::Vector_String*>(ClassTraits::fl_vec::Vector_String::MakeClassTraits(GetSelf()).GetPtr())))
, TraitsApplicationDomain(PrepareClassTraits(static_cast<ClassTraits::fl_system::ApplicationDomain*>(ClassTraits::fl_system::ApplicationDomain::MakeClassTraits(GetSelf()).GetPtr())))
, TraitsDomain(PrepareClassTraits(static_cast<ClassTraits::fl::Domain*>(ClassTraits::fl::Domain::MakeClassTraits(GetSelf()).GetPtr())))

, TraitsNull(MakePickable(SF_HEAP_NEW_ID(GetMemoryHeap(), StatMV_VM_ITraits_Mem) InstanceTraits::Anonimous(GetSelf())))
, TraitsVoid(MakePickable(SF_HEAP_NEW_ID(GetMemoryHeap(), StatMV_VM_ITraits_Mem) InstanceTraits::Void(GetSelf())))

, NoFunctionTraits(MakePickable(SF_HEAP_NEW_ID(GetMemoryHeap(), StatMV_VM_ITraits_Mem) InstanceTraits::Function(GetSelf(), AS3::fl::FunctionCICpp)))

, TraitaGlobalObject(MakePickable(SF_HEAP_NEW_ID(GetMemoryHeap(), StatMV_VM_ITraits_Mem) InstanceTraits::fl::GlobalObject(GetSelf())))
, GlobalObject(MakePickable(new(Memory::AllocInHeap(GetMemoryHeap(), sizeof(Instances::fl::GlobalObjectCPP))) Instances::fl::GlobalObjectCPP(GetSelf(), *TraitaGlobalObject)))
, GlobalObjectValue(GlobalObject)
, TraitsByteArray(PrepareClassTraits(static_cast<ClassTraits::fl_utils::ByteArray*>(ClassTraits::fl_utils::ByteArray::MakeClassTraits(GetSelf()).GetPtr())))
SF_AOTC_ARG(pIC(ic))
SF_AOTC2_ARG(pIC(ic))
{
#ifdef GFX_ENABLE_XML
    EnableXMLSupport();
#endif

    NoFunctionTraits->SetConstructor(Pickable<Class>(&GetClassFunction(), PickValue));

    Initialized = true;

    GlobalObjects.PushBack(GlobalObject.GetPtr());

    // Delayed initialization to prevent circular dependencies.
    {
        // Set up traits of the public namespace, which was created uninitialized.
        // !!! For some reason that causes memory leaks.
        //PublicNamespace->pTraits = &ClassNamespace->GetInstanceTraits();
        //AS3Namespace->pTraits = &ClassNamespace->GetInstanceTraits();

        // ClassClass delayed initialization.
        {
            // ClassClass is a child of Object (like everything else).
            TraitsClassClass->SetParentTraits(TraitsObject.GetPtr());

            TraitsClassClass->RegisterSlots();

            // Prototype of ClassClass cannot be set in constructor because
            // ClassObject is not initialized at this time.
            GetClassClass().SetupPrototype();
        }

        TraitsObject->RegisterSlots();
        TraitsNamespace->RegisterSlots();
    }

#ifndef SF_AS3_CLASS_AS_SLOT
    Instances::fl::Namespace& ns = GetPublicNamespace();
    ClassTraits::Traits* ctr;

    ctr = TraitsObject;
    SystemDomain->AddClassTrait(ctr->GetName(), ns, ctr);

    ctr = TraitsClassClass;
    SystemDomain->AddClassTrait(ctr->GetName(), ns, ctr);

    ctr = TraitsNamespace;
    SystemDomain->AddClassTrait(ctr->GetName(), ns, ctr);

    ctr = TraitsFunction;
    SystemDomain->AddClassTrait(ctr->GetName(), ns, ctr);

    ctr = TraitsBoolean;
    SystemDomain->AddClassTrait(ctr->GetName(), ns, ctr);

    ctr = TraitsNumber;
    SystemDomain->AddClassTrait(ctr->GetName(), ns, ctr);

    ctr = TraitsInt;
    SystemDomain->AddClassTrait(ctr->GetName(), ns, ctr);

    ctr = TraitsUint;
    SystemDomain->AddClassTrait(ctr->GetName(), ns, ctr);

    ctr = TraitsString;
    SystemDomain->AddClassTrait(ctr->GetName(), ns, ctr);

    ctr = TraitsArray;
    SystemDomain->AddClassTrait(ctr->GetName(), ns, ctr);

    ctr = TraitsByteArray;
    SystemDomain->AddClassTrait(ctr->GetName(), ns, ctr);

#ifdef GFX_ENABLE_XML
    ctr = TraitsQName;
    SystemDomain->AddClassTrait(ctr->GetName(), ns, ctr);

    ctr = TraitsXML;
    SystemDomain->AddClassTrait(ctr->GetName(), ns, ctr);

    ctr = TraitsXMLList;
    SystemDomain->AddClassTrait(ctr->GetName(), ns, ctr);
#endif
#endif
}

VM::~VM()
{
    InDestructor = true;

    // This situation should be handled by the garbage col  lector.
    // Because of dependencies.
    //ClassClass->ReleasePrototype();
    UnregisterAllAbcFiles();
}

VMAppDomain& VM::GetFrameAppDomain() const
{
    if (CallStack.GetSize() == 0 || !VMAppDomain::IsEnabled())
        return GetCurrentAppDomain();
    return GetCurrCallFrame().GetFile().GetAppDomain();
}

bool VM::RemoveAppDomain(VMAppDomain& appDomain)
{
    if (!LoadingAbcFile)
    {
        return SystemDomain->RemoveChild(appDomain);
    }
    return false;
}

void VM::UnregisterAllAbcFiles()
{
    bool oi = InDestructor;
    InDestructor = true;
    for (UPInt i = 0, n = VMAbcFilesWeak.GetSize(); i < n; ++i)
    {
        VMAbcFilesWeak[i]->UnRegister();
    }
    VMAbcFilesWeak.Clear();
    InDestructor = oi;
}

SPtr<VMAbcFile> VM::FindVMAbcFileWeak(const char* name, VMAppDomain& appDomain) const 
{
    for (UPInt i = 0, n = VMAbcFilesWeak.GetSize(); i < n; ++i)
    {
        VMAbcFile* abcFile = VMAbcFilesWeak[i];
        if (abcFile->GetAbcFile().GetName() == name && &abcFile->GetAppDomain() == &appDomain)
        {
            return SPtr<VMAbcFile>(VMAbcFilesWeak[i]);
        }
    }

    return SPtr<VMAbcFile>(NULL);
}

void VM::AddVMAbcFileWeak(VMAbcFile* f)
{
    if (!InDestructor)
    {
        VMAbcFilesWeak.PushBack(f);
    }
}

bool VM::RemoveVMAbcFileWeak(VMAbcFile* f)
{
    if (!InDestructor)
    {
        for (UPInt i = 0, n = VMAbcFilesWeak.GetSize(); i < n; ++i)
        {
            if (VMAbcFilesWeak[i] == f)
            {
                VMAbcFilesWeak.RemoveAt(i);
                return true;
            }
        }
    }
    return false;
}

UPInt VM::GetAllLoadedAbcFiles(Scaleform::Array<SPtr<VMAbcFile> >* destArray) const
{
    UPInt i = 0;
    if (!InDestructor)
    {
        for (UPInt n = VMAbcFilesWeak.GetSize(); i < n; ++i)
        {
            destArray->PushBack(VMAbcFilesWeak[i]);
        }
    }
    return i;    
}

void VM::RegisterClassTraits(ClassTraits::Traits& tr, VMAppDomain& appDomain)
{
    const InstanceTraits::Traits& itr = tr.GetInstanceTraits();

    RegisterClassTraits(itr.GetName(), itr.GetNamespace(), tr, appDomain);
}

bool IsScaleformGFx(const Instances::fl::Namespace& ns)
{
    static const StringDataPtr scaleform_gfx("scaleform.gfx");
    const ASString& uri = ns.GetUri();
    const StringDataPtr str(uri.ToCStr(), uri.GetSize());

    return (str.GetSize() >= scaleform_gfx.GetSize() && SFstrncmp(str.ToCStr(), scaleform_gfx.ToCStr(), scaleform_gfx.GetSize()) == 0);
}

void VM::RegisterClassTraitsVector(ClassTraits::Traits& tr, VMAppDomain& appDomain)
{
    const Instances::fl::Namespace& ns = GetInternedNamespace(Abc::NS_Public, NS_Vector);

    RegisterClassTraits(tr.GetInstanceTraits().GetClass().GetName(), ns, tr, appDomain);
}

Pickable<Instances::fl::Object> VM::MakeObject() const
{
    InstanceTraits::fl::Object& itr = static_cast<InstanceTraits::fl::Object&>(GetClassTraitsObject().GetInstanceTraits());
    return itr.MakeInstance(itr);
}

Pickable<Instances::fl::Array> VM::MakeArray() const
{
    InstanceTraits::fl::Array& itr = static_cast<InstanceTraits::fl::Array&>(GetClassTraitsArray().GetInstanceTraits());
    return itr.MakeInstance(itr);
}

Pickable<Instances::fl_utils::ByteArray> VM::MakeByteArray() const
{
    InstanceTraits::fl_utils::ByteArray& itr = static_cast<InstanceTraits::fl_utils::ByteArray&>(GetClassTraitsByteArray().GetInstanceTraits());
    return itr.MakeInstance(itr);
}

ASString VM::AsString(const Value& value)
{
    StringManager& sm = GetStringManager();
    ASString result = sm.CreateEmptyString();

    if (!value.Convert2String(result))
    {
        // Exception.
        // Do nothing about it.
        ;
    }
    
    return result;
}

const ClassTraits::Traits* VM::GetRegisteredClassTraits(const Multiname& mn, VMAppDomain& appDomain)
{
    const ClassTraits::Traits* result = NULL;

    // Both branches seem to work fine.
    // First branch seems to result in better performance.
#if 1
    if (mn.IsAnyType())
        // Code below worked fine.
        // But using GetClassTraitsObject() in Resolve2ClassTraits() causes several dozen of tests to fail.
        //result = &GetClassTraitsObject();
        result = &GetClassTraitsClassClass();
    else
    {
        ClassTraits::Traits** pptraits = appDomain.GetClassTrait(mn);
        result = pptraits ? *pptraits : NULL;
    }
#else
    ClassTraits::Traits** pptraits = appDomain.GetClassTrait(mn);
    result = pptraits ? *pptraits : NULL;
#endif

    return result;
}

const ClassTraits::Traits* VM::GetRegisteredClassTraits(const ASString& name, const Instances::fl::Namespace& ns, VMAppDomain& appDomain)
{
    ClassTraits::Traits** r = appDomain.GetClassTrait(name, ns);
    return r ? *r : NULL;
}

const ClassTraits::Traits* VM::Resolve2ClassTraits(const ASString& name, const Instances::fl::Namespace& ns, VMAppDomain& appDomain)
{
    const ClassTraits::Traits* result = GetRegisteredClassTraits(name, ns, appDomain);

    if (result == NULL)
    {
        ClassTraits::Traits* r = GetGlobalObjectCPP().GetClassTraits(name, ns);
        result = r;

        if (result)
            GetSystemAppDomain().AddClassTrait(name, ns, r);
    }

    return result;
}

const ClassTraits::Traits* VM::Resolve2ClassTraits(const Multiname& mn, VMAppDomain& appDomain)
{
    const ClassTraits::Traits* result = GetRegisteredClassTraits(mn, appDomain);

    if (result == NULL)
    {
        StringManager& sm = GetStringManager();
        ASString name = sm.CreateEmptyString();

        if (!mn.GetName().Convert2String(name))
            // Error
            return result;

        ClassTraits::Traits* r = GetGlobalObjectCPP().GetClassTraits(name, mn.GetNamespace());
        result = r;

        if (result)
            GetSystemAppDomain().AddClassTrait(name, mn.GetNamespace(), r);
    }

    return result;
}

const ClassTraits::Traits* VM::Resolve2ClassTraits(VMFile& file, const Abc::Multiname& mn)
{
    if (mn.IsAnyType())
    {
        // !!! ATTENSION !!!
        // This is a fundamental problem.
        // The major difference between Class and Object is that Class leaves
        // *undefined* unchanged, in opposite, Object will convert *undefined*
        // to *null*.

        // Object can represent any type.
        // Using code below will cause several dozen of tests to fail.
        //return &GetClassTraitsObject();
        return &GetClassTraitsClassClass();
    }

    const ASString name = file.GetInternedString(mn.GetNameInd());

    if (name == GetStringManager().GetBuiltin(AS3Builtin_void))
        // Should this be Object?
        return &GetClassTraitsObject();

    SF_ASSERT(mn.IsQName());
    const Instances::fl::Namespace& ns = file.GetInternedNamespace(mn.GetNamespaceInd());
    const ClassTraits::Traits* ctr = Resolve2ClassTraits(name, ns, file.GetAppDomain());

    // Handle Vector<>.
    if (ctr == &GetClassTraitsVector())
    {
        const ClassTraits::Traits* elem_ctr = NULL;

        // Sometimes Vector multinames do not have enclosed type (mn.GetNextMultinameIndex() == -1).
        if (mn.GetNextMultinameIndex() <= 0)
            elem_ctr = &GetClassTraitsObject();
        else
        {
            const Abc::Multiname& next = file.GetMultiname(mn.GetNextMultinameIndex());
            elem_ctr = Resolve2ClassTraits(file, next);
        }

        if (elem_ctr)
        {
            const BuiltinTraitsType tt = elem_ctr->GetTraitsType();
            switch (tt)
            {
            case Traits_SInt:
                ctr = &GetClassTraitsVectorSInt();
                break;
            case Traits_UInt:
                ctr = &GetClassTraitsVectorUInt();
                break;
            case Traits_Number:
                ctr = &GetClassTraitsVectorNumber();
                break;
            case Traits_String:
                ctr = &GetClassTraitsVectorString();
                break;
            default:
                // Condition below should be removed at some point. 
                if (elem_ctr->IsValid())
                {
                    const ClassTraits::Traits& vctr = static_cast<const Classes::fl_vec::Vector&>(ctr->GetInstanceTraits().GetClass()).Resolve2Vector(*elem_ctr);

                    ctr = &vctr;
                }
                break;
            }
        }
    }

    return ctr;
}

const ClassTraits::Traits* VM::Resolve2ClassTraits(const TypeInfo& ti)
{
    StringManager& sm = GetStringManager();
    const ASString name = sm.CreateConstString(ti.Name);
    const ASString ns_name = sm.CreateConstString(ti.PkgName);
    SPtr<Instances::fl::Namespace> ns = MakeInternedNamespace(Abc::NS_Public, ns_name);

    return Resolve2ClassTraits(name, *ns, GetSystemAppDomain());
}

void VM::UnregisterGlobalObject(Instances::fl::GlobalObject& go)
{
    const UPInt size = GlobalObjects.GetSize();

    for (UPInt j = 0; j < size; ++j)
    {
        Instances::fl::GlobalObject* v = GlobalObjects[j];
        if (v == &go)
        {
            GlobalObjects.RemoveAt(j);
            break;
        }
    }
}

void VM::SetClassAlias(const ASString& alias, Class& cl)
{
    Class* result = NULL;

    if (AliasClass.Get(alias, &result))
    {
        if (result == &cl)
            // The same class.
            return;

        // Unregister old class with file.
        {
            VMAbcFile* file = result->GetTraits().GetFilePtr();

            if (file)
                // Register with file.
                file->AliasClass.Remove(alias);
        }

        ClassAlias.Remove(result);

        // Register new class.
        {
            VMAbcFile* file = cl.GetTraits().GetFilePtr();

            if (file)
                // Register with file.
                file->AliasClass.Add(alias);

            AliasClass.Set(alias, &cl);
            ClassAlias.Set(&cl, alias);
        }
    }
    else
    {
        VMAbcFile* file = cl.GetTraits().GetFilePtr();

        if (file)
            // Register with file.
            file->AliasClass.Add(alias);

        AliasClass.Add(alias, &cl);
        ClassAlias.Set(&cl, alias);
    }
}

Class* VM::GetClassByAlias(const ASString& alias) const
{
    Class* result = NULL;
    AliasClass.Get(alias, &result);
    return result;
}

ASString VM::GetAliasByClass(Class& cl) const
{
    ASString result = GetStringManager().CreateEmptyString();

    ClassAlias.Get(&cl, &result);
    return result;
}

bool VM::IsOfType(const Value& v, const char* type_name, VMAppDomain& appDomain)
{
    const Multiname mn(*this, type_name);

    const ClassTraits::Traits* ctr = Resolve2ClassTraits(mn, appDomain);
    return (ctr && IsOfType(v, *ctr));
}

inline
Value::Number GetFractionalPortion(Value::Number v)
{
    double r; // We cannot use Value::Number here. 

    return modf(v, &r);
}

bool VM::IsOfType(const Value& v, const ClassTraits::Traits& ctr) const
{
    const BuiltinTraitsType tt = ctr.GetTraitsType();
    const Value::KindType k = v.GetKind();

    switch (k)
    {
    case Value::kUndefined:
        return false;
    case Value::kBoolean:
        // Boolean is final.
        return tt == Traits_Boolean || IsObject(ctr);
    case Value::kInt:
        if (tt == Traits_UInt)
            return v.AsInt() >= 0; // It is not correct to use SF_MIN_UINT32 here.
        else if (tt == Traits_Number)
            return true;

        // int is final.
        return tt == Traits_SInt || IsObject(ctr);
    case Value::kUInt:
        if (tt == Traits_SInt)
            return v.AsUInt() <= SF_MAX_SINT32;
        else if (tt == Traits_Number)
            return true;

        // uint is final.
        return tt == Traits_UInt || IsObject(ctr);
    case Value::kNumber:
        {
            double r; // We cannot use Value::Number here.

            if (modf(v.AsNumber(), &r) == 0)
            {
                if (tt == Traits_UInt)
                    return r >= SF_MIN_UINT32 && r <= SF_MAX_UINT32;
                else if (tt == Traits_SInt)
                    return r >= SF_MIN_SINT32 && r <= SF_MAX_SINT32;
            }
        }

        // Number is final.
        return tt == Traits_Number || IsObject(ctr);
    case Value::kString:
        if (v.GetStringNode() == NULL)
            return false;
        else
            // String is final.
            return tt == Traits_String || IsObject(ctr);
    case Value::kObject:
        if (v.GetObject() == NULL)
            return false;
        // fall through.    
    case Value::kFunction:
    case Value::kNamespace:
    case Value::kThunkFunction:        
    case Value::kThunkClosure:        
    case Value::kVTableIndClosure:        
        return GetClassTraits(v).IsOfType(ctr);
    case Value::kClass:
        if (v.GetObject() == NULL)
            return false;
        return v.GetObject()->GetTraits().GetClass().GetClassTraits().IsOfType(ctr);
    default:
        SF_ASSERT(false);
        break;
    }

    return false;
}

int VM::OnException(Abc::TCodeOffset offset, CallFrame& cf)
{
    int position = -1;
    const Abc::MethodBodyInfo::Exception& e = cf.GetException();
    UPInt handler_num = 0;

    while (e.FindExceptionInfo(offset, handler_num))
    {
        const Abc::MethodBodyInfo::ExceptionInfo& ei = e.Get(handler_num);
        const int et_ind = ei.GetExceptionTypeInd();
        
        if (et_ind == 0)
        {
            // This handler can handle any type of exceptions ...
            
            // The value and scope stacks in the handling method are cleared before the handler is entered.
            // If a handler is found then the stack is cleared, the exception object is pushed onto the stack.
            cf.ClearOpStack();
            OpStack.PushBack(ExceptionObj);
            // Clean *exception* value ...
            ExceptionObj.Clean();
            cf.ClearScopeStack();
            
            position = ei.GetTargetPos();
            break;
        } else
        {
            // Undefined ExceptionObj should be ignored.
            if (!ExceptionObj.IsUndefined())
            {
                //const String& en = GetConstPool().GetString(Abc::StringID(et_ind));
                const ClassTraits::Traits& exc_ctr = GetClassTraits(ExceptionObj);

                HandleException = false;
                const ClassTraits::Traits* handler_tr = Resolve2ClassTraits(cf.GetFile(), cf.GetFile().GetConstPool().GetMultiname(et_ind));
                if (IsException())
                {
                    ; // Oops. Exception during exception handling.
                }
                HandleException = true;

                if (handler_tr && exc_ctr.IsOfType(*handler_tr))
                {
                    // This handler can handle this type of exceptions ...

                    // The value and scope stacks in the handling method are cleared before the handler is entered.
                    // If a handler is found then the stack is cleared, the exception object is pushed onto the stack.
                    cf.ClearOpStack();
                    OpStack.PushBack(ExceptionObj);
                    // Clean *exception* value ...
                    ExceptionObj.Clean();
                    cf.ClearScopeStack();

                    position = ei.GetTargetPos();
                    break;
                }
            }
        }

        ++handler_num;
    }
    
    HandleException = (position < 0);
        
    return position;
}

void VM::exec_getsuper(VMFile& file, const Traits& ot, const Abc::Multiname& mn)
{

    ReadMnObjectRef args(file, mn);

    if (IsException())
        return;

    // MA: eReadSealedError or eWriteOplyError possible in property
    const Value value(args.ArgObject, PickValue);
    if (!GetSuperProperty(*this, &ot, args.ArgObject, value, args.ArgMN))
        return ThrowReferenceError(VM::Error(VM::eIllegalSuperCallError, *this
            SF_DEBUG_ARG(args.ArgMN.GetName())
            ));
}

void VM::exec_setsuper(VMFile& file, const Traits& ot, const Abc::Multiname& mn) 
{
    ReadValueMnObject args(file, mn);
    
    if (IsException())
        return;

    // Semantic of this operation is not very clear to me ...
    // Let's do it this way ...
    
    if (!SetSuperProperty(*this, &ot, args.ArgObject, args.ArgMN, args.ArgValue))
        return ThrowReferenceError(VM::Error(VM::eIllegalSuperCallError, *this
            SF_DEBUG_ARG(args.ArgMN.GetName())
            ));
}

void VM::exec_setproperty(VMFile& file, const Abc::Multiname& mn) 
{
    // Set a property.
    
    // Read arguments ...
    ReadValueMnObject args(file, mn);
    
    if (IsException())
        return;
    
    SetProperty(*this, args.ArgObject, args.ArgMN, args.ArgValue).DoNotCheck();
}

void VM::exec_callproperty(VMFile& file, const Abc::Multiname& mn, UInt32 arg_count)
{
    // Call a property.
    
    // Read arguments ...
    ReadArgsMnObject args(file, arg_count, mn);
    
    if (IsException())
        return;
    
    // Resolve property ...
    PropRef prop;

    FindObjProperty(prop, *this, args.ArgObject, args.ArgMN, FindCall);
    
    if (prop)
    {
        Value func;
        
        if (!prop.GetSlotValueUnsafe(*this, func, SlotInfo::valExecute))
            return;
        
        // A TypeError is thrown if the property specified by the
        // multiname is null or undefined.
        if (func.IsNullOrUndefined())
        {
            return ThrowTypeError(VM::Error(VM::eCallOfNonFunctionError, *this
                SF_DEBUG_ARG(args.ArgMN.GetName())
                ));
        }
        
        // The [[Call]] property is invoked on the value of the resolved property with the arguments 
        // obj, arg1, ..., argn. 
        Execute(func, args.ArgObject, arg_count, args.GetCallArgs());
    } 
    else
    {
        // No property ...
        if (GetValueTraits(args.ArgObject).IsDynamic() || args.ArgObject.IsPrimitive())
        {
            // In case of a dynamic object property might be created.
            // But we do not create it for the time being. We just throw
            // an exception.
            ThrowTypeError(VM::Error(VM::eCallOfNonFunctionError, *this
                SF_DEBUG_ARG(args.ArgMN.GetName())
                ));
            return;
        }
        else
        {
            return ThrowReferenceError(VM::Error(VM::ePropertyNotFoundError, *this
                    SF_DEBUG_ARG(args.ArgMN.GetName())
                    SF_DEBUG_ARG(args.ArgObject)
                    ));
        }
    }
}

void VM::exec_returnvoid()
{
    // Return from a method.

    // Return from the currently executing method. This returns the value undefined. If the
    // method has a return type, then undefined is coerced to that type and then returned.

    /* DO NOT delete this code. It was original behavior.
    OpStack.PushBack(Value::GetUndefined());
    */
    if (!GetCurrCallFrame().NeedToDiscardResult())
        OpStack.PushBack(Value::GetUndefined());

    /* DO NOT delete this code !!!
    Commented out because of a performance reason.
    There was no problem with this so far.
    if (!GetCurrCallFrame().NeedToDiscardResult())
    // Can throw exceptions.
    Coerce2ReturnType(OpStack.Back(), OpStack.Back());
    */
}

void VM::exec_returnvalue()
{
    // Return a value from a method.

    // return_value is popped off of the stack, and coerced to the expected return type of the
    // method. The coerced value is what is actually returned from the method.

    // It is returned on stack, so, if we do not want to coerce it (for the time being), we can
    // just leave it there.

    // A TypeError is thrown if return_value cannot be coerced to the expected return type of the
    // executing method.

    /* DO NOT delete this code. It was original behavior.
    // Coerce to the argument's type ...
    // This "if" statements is a little bit tricky.
    if (!GetCurrCallFrame().NeedToDiscardResult())
    // Can throw exceptions.
    Coerce2ReturnType(OpStack.Back(), OpStack.Back());
    */

    if (GetCurrCallFrame().NeedToDiscardResult())
    {
	    // Below is the old code.
//         if (OpStack.Back().IsPrimitive())
//             // Primitive value cannot be cast to an object, so, just get rid of it.
//             OpStack.Pop();
//         else
//         {
//             // Can throw exceptions.
//             Coerce2ReturnType(OpStack.Back(), OpStack.Back());
//             if (IsException())
//                 return;
// 
//             if (OpStack.GetSize() > 1)
//             {
//                 Value v = OpStack.Pop();
//                 OpStack.Pop();
//                 OpStack.PushBack(v);
//             }
//         }

	/* Commenting out code below will result in a memory leak with the /Tamarin/acceptance/ecma3/ExecutionContexts/e10_1_6.abc test

        {
            if (OpStack.GetSize() > 0)
            {
		// Can throw exceptions.
		Coerce2ReturnType(OpStack.Back(), OpStack.Back());
		if (IsException())
                    return;

		if (OpStack.GetSize() > 1)
		{
			UPInt size = OpStack.GetSize();

			Alg::Swap(OpStack[size - 1], OpStack[size - 2]);
			OpStack.PopBack();
		}
            }
        }
	*/

        OpStack.PopBack();
    }
    else
        Coerce2ReturnType(OpStack.Top0(), OpStack.Top0());
}

void VM::exec_callproplex(VMFile& file, const Abc::Multiname& mn, UInt32 arg_count)
{
    // Call a property.
    
    // Read arguments ...
    ReadArgsMnObject args(file, arg_count, mn);
    
    if (IsException())
        return;
    
    // The property specified by the multiname at index is resolved on the object obj. The [[Call]]
    // property is invoked on the value of the resolved property with the arguments null, arg1, ...,
    // argn. The result of the call is pushed onto the stack.
    
    // Resolve a property ...
    
    PropRef prop;

    FindObjProperty(prop, *this, args.ArgObject, args.ArgMN, FindCall);

    if (prop)
    {
        Value func;
        
        if (!prop.GetSlotValueUnsafe(*this, func, SlotInfo::valExecute))
            return;
        
        // A TypeError is thrown if the property specified by the
        // multiname is null or undefined.
        if (func.IsNullOrUndefined())
        {
            ThrowTypeError(VM::Error(VM::eCallOfNonFunctionError, *this
                SF_DEBUG_ARG(args.ArgMN.GetName())
                ));
            return;
        }

        // The [[Call]]
        // property is invoked on the value of the resolved property with the arguments null, arg1, ...,
        // argn. The result of the call is pushed onto the stack.

        // This is the only difference from *callproperty* I could find so far ...
        Execute(func, Value::GetUndefined(), arg_count, args.GetCallArgs());
    }
    else
    {
        // No property ...
        if (GetValueTraits(args.ArgObject).IsDynamic())
        {
            // In case of a dynamic object property might be created.
            // But we do not create it for the time being. We just throw
            // an exception.
            ThrowTypeError(VM::Error(VM::eCallOfNonFunctionError, *this
                SF_DEBUG_ARG(args.ArgMN.GetName())
                ));
            return;
        }
        else
        {
            return ThrowReferenceError(VM::Error(VM::ePropertyNotFoundError, *this
                SF_DEBUG_ARG(args.ArgMN.GetName())
                SF_DEBUG_ARG(args.ArgObject)
                ));
        }
    }
}

void VM::exec_callpropvoid(VMFile& file, const Abc::Multiname& mn, UInt32 arg_count)
{
    // Call a property, discarding the return value.
    
    // Read arguments ...
    ReadArgsMnObject args(file, arg_count, mn);
    
    if (IsException())
        return;
    
    // Resolve property ...
    
    PropRef prop;

    FindObjProperty(prop, *this, args.ArgObject, args.ArgMN, FindCall);

    if (prop)
    {
        Value value;
        
        if (!prop.GetSlotValueUnsafe(*this, value, SlotInfo::valExecute))
            return;
        
        // A TypeError is thrown if the property specified by the
        // multiname is null or undefined.
        if (value.IsNullOrUndefined())
        {
            ThrowTypeError(VM::Error(VM::eCallOfNonFunctionError, *this
                SF_DEBUG_ARG(args.ArgMN.GetName())
                ));
            return;
        }

        // The [[Call]] property is invoked on the value of the resolved property with the arguments 
        // obj, arg1, ..., argn. 

        // We MUST discard result here.
        Execute(value, args.ArgObject, arg_count, args.GetCallArgs(), true);
    }
    else
    {
        // No property ...
        if (GetValueTraits(args.ArgObject).IsDynamic())
        {
            // In case of a dynamic object property might be created.
            // But we do not create it for the time being. We just throw
            // an exception.
            ThrowTypeError(VM::Error(VM::eCallOfNonFunctionError, *this
                SF_DEBUG_ARG(args.ArgMN.GetName())
                ));
            return;
        }
        else
        {
            return ThrowReferenceError(VM::Error(VM::ePropertyNotFoundError, *this
                SF_DEBUG_ARG(args.ArgMN.GetName())
                SF_DEBUG_ARG(args.ArgObject)
                ));
        }
    }
}

void VM::exec_callmethod(UInt32 method_index, UInt32 arg_count)
{
    // Call a method identified by index in the object's method table (virtual table).
    // This opcode cannot be generated by a compiler. Veryfier must throw
    // exception in this case.
    
    // Read arguments ...
    ReadArgsObject args(*this, arg_count);
    
    if (IsException())
        return;
    
    // method_index is the index of the method to invoke on receiver. arg_count is a u30 that is
    // the number of arguments present on the stack. receiver is the object to invoke the method on.
    
#if 0
    const Value funct(method_index, vt, Value::kVTableInd);

    // The [[Call]] property is invoked on the value of the resolved property with the arguments 
    // obj, arg1, ..., argn.
    Execute(funct, args.ArgObject, arg_count, args.GetCallArgs());
#else
    const Traits& tr = GetValueTraits(args.ArgObject);

    ExecuteVTableIndUnsafe(method_index, tr, args.ArgObject, arg_count, args.GetCallArgs());
#endif
}

void VM::exec_callsupermethod(const Traits& ot, UInt32 method_index, UInt32 arg_count)
{
    // Call a method identified by index in the object's method table (virtual table).
    // This opcode cannot be generated by a compiler. Verifier must throw
    // exception in this case.

    // Read arguments ...
    ReadArgsObject args(*this, arg_count);

    if (IsException())
        return;

    // method_index is the index of the method to invoke on receiver. arg_count is a u30 that is
    // the number of arguments present on the stack. receiver is the object to invoke the method on.

    const Traits* parent_tr = ot.GetParent();
    if (parent_tr == NULL)
        return ThrowReferenceError(VM::Error(VM::eIllegalSuperCallError, *this
        SF_DEBUG_ARG("Couldn't find parent property")
        ));

#if 0
    const Value funct(method_index, *parent_tr, Value::kVTableInd);

    // The [[Call]] property is invoked on the value of the resolved property with the arguments 
    // obj, arg1, ..., argn. 
    Execute(funct, args.ArgObject, arg_count, args.GetCallArgs());
#else
    ExecuteVTableIndUnsafe(method_index, *parent_tr, args.ArgObject, arg_count, args.GetCallArgs());
#endif
}

void VM::exec_callgetter(UInt32 method_index, UInt32 arg_count)
{
    // Call a getter identified by index in the object's method table (virtual table).
    // This opcode cannot be generated by a compiler. Veryfier must throw
    // exception in this case.

    // Read arguments ...
    ReadArgsObject args(*this, arg_count);

    if (IsException())
        return;

    // method_index is the index of the method to invoke on receiver. arg_count is a u30 that is
    // the number of arguments present on the stack. receiver is the object to invoke the method on.

    const Traits& tr = GetValueTraits(args.ArgObject);
    const VTable& vt = tr.GetVT();

#ifdef USE_VTABLE_IND
    const Value funct_getter(method_index, vt, Value::kVTableInd);
#else
    const Value& funct_getter = vt.GetRaw(AbsoluteIndex(method_index));
#endif

    // Retrieve function.
    Value funct;
    ExecuteUnsafe(funct_getter, args.ArgObject, funct, 0, NULL);

    if (IsException())
        return;

    // The [[Call]] property is invoked on the value of the resolved property with the arguments 
    // obj, arg1, ..., argn.
    Execute(funct, args.ArgObject, arg_count, args.GetCallArgs());
}

void VM::exec_callsupergetter(const Traits& ot, UInt32 method_index, UInt32 arg_count)
{
    // Call a getter identified by index in the object's method table (virtual table).
    // This opcode cannot be generated by a compiler. Verifier must throw
    // exception in this case.

    // Read arguments ...
    ReadArgsObject args(*this, arg_count);

    if (IsException())
        return;

    // method_index is the index of the method to invoke on receiver. arg_count is a u30 that is
    // the number of arguments present on the stack. receiver is the object to invoke the method on.

    const Traits* parent_tr = ot.GetParent();
    if (parent_tr == NULL)
        return ThrowReferenceError(VM::Error(VM::eIllegalSuperCallError, *this
        SF_DEBUG_ARG("Couldn't find parent property")
        ));

    const Value funct_getter(method_index, *parent_tr, Value::kVTableInd);

    // Retrieve function.
    Value funct;
    ExecuteUnsafe(funct_getter, args.ArgObject, funct, 0, NULL);

    if (IsException())
        return;

    // The [[Call]] property is invoked on the value of the resolved property with the arguments 
    // obj, arg1, ..., argn. 
    Execute(funct, args.ArgObject, arg_count, args.GetCallArgs());
}

void VM::exec_callstatic(VMAbcFile& file, Abc::MiInd ind, UInt32 arg_count)
{
    // Call a method identified by index in the abcFile method table.
    
    // Read arguments ...
    ReadArgsObject args(*this, arg_count);
    
    if (IsException())
        return;
    
    // method_index is the index of the method_info of the method to invoke.
            
    // The receiver will be used as the "this" value for the method. 
    // The result of the method is pushed onto the stack.

    {
        const Abc::MbiInd mbi_ind = file.GetMethodBodyInfoInd(ind);
        
#if 0
        AddFrame(
            Value::GetUndefined(), // There is no object to track its life time.
            file,
            mbi_ind,
            args.ArgObject,
            NULL, // ??? No saved scope ???
            arg_count,
            args.GetCallArgs(),
            false, // discard_result
            NULL
            SF_DEBUG_ARG(args.ArgObject.GetObject()->GetName() + " callstatic")
            );
#else
        const Traits& tr = args.ArgObject.GetObject()->GetTraits();

#ifdef SF_AS3_ENABLE_EXPLICIT_GO
        SF_ASSERT(tr.GetGlobalObjectScript());
        Instances::fl::GlobalObjectScript& gos = *tr.GetGlobalObjectScript();
#endif

        AddFrame(
            Value::GetUndefined(), // There is no object to track its life time.
            file,
            mbi_ind,
            args.ArgObject,
            arg_count,
            args.GetCallArgs(),
            false, // discard_result
#ifdef SF_AS3_ENABLE_EXPLICIT_GO
            gos,
#endif
            tr.GetStoredScopeStack(),
            tr
            SF_DEBUG_ARG(args.ArgObject.GetObject()->GetName() + " callstatic")
            );
#endif
    }
}

void VM::exec_newfunction(const CallFrame& cf, UInt32 method_ind)
{
    // When creating the new function object the scope stack used is the current scope stack when
    // this instruction is executed, and the body is the method_body entry that references the
    // specified method_info entry.

    // No exceptions in this method;
    SF_ASSERT(GetGlobalObject());
    Pickable<Instances::Function> funct = GetClassFunction().MakeInstance(
        // It is OK to take file from a current call frame.
        cf.GetFile(), 
        method_ind, 
        cf.GetSavedScope(), 
        *GetGlobalObject()
        );

    OpStack.PushBack(funct);

    // No exceptions in this method;
    // Tamarin throws exceptions in this opcode.
}

void VM::exec_call(UInt32 arg_count)
{
    // Call a closure.
    
    // Read arguments ...
    ReadArgsObjectValue args(*this, arg_count);
    
    if (IsException())
        return;

    // function is the closure that is being called. receiver is the object to use for the "this" value. This will
    // invoke the [[Call]] property on function with the arguments receiver, arg1, ..., argn. 
    // The result of invoking the [[Call]] property will be pushed onto the stack.

    Execute(args.value, args.ArgObject, arg_count, args.GetCallArgs());
}

void VM::exec_callobject(UInt32 arg_count)
{
    // Read arguments ...
    ReadArgsObjectRef args(*this, arg_count);

    if (IsException())
        return;

#if 0
    ExecuteInternalUnsafe(args.ArgObject, Value::GetUndefined(), args.ArgObject, arg_count, args.GetCallArgs());
#else
    //SF_ASSERT(args.ArgObject.IsClass());
    args.ArgObject.GetObject()->Call(Value::GetUndefined(), args.ArgObject, arg_count, args.GetCallArgs());
#endif
}

void VM::exec_callsuper(VMFile& file, const Traits& ot, const Abc::Multiname& mn, UInt32 arg_count)
{
    // Call a method on a base class.
    
    // Read arguments ...
    ReadArgsMnObject args(file, arg_count, mn);
    
    if (IsException())
        return;
    
    // The base class of receiver is determined and the method indicated by the multiname is
    // resolved in the declared traits of the base class.

    Value func;
    if (!GetSuperProperty(*this, &ot, func, args.ArgObject, args.ArgMN, SlotInfo::valExecute))
        // Not confirmed ...
        return ThrowReferenceError(VM::Error(VM::eCallNotFoundError, *this
            SF_DEBUG_ARG(args.ArgMN.GetName())
            ));

    // The [[Call]] property is invoked on the value of the resolved property with the arguments 
    // obj, arg1, ..., argn. 
    // The first argument will be used as the "this" value for the method.
    // The result of the method call is pushed onto the stack.
    
    Execute(func, args.ArgObject, arg_count, args.GetCallArgs());
}

void VM::exec_callsupervoid(VMFile& file, const Traits& ot, const Abc::Multiname& mn, UInt32 arg_count)
{
    // Call a method on a base class, discarding the return value.
    
    // Read arguments ...
    ReadArgsMnObject args(file, arg_count, mn);
    
    if (IsException())
        return;
    
    // The base class of receiver is determined and the method indicated by the multiname is
    // resolved in the declared traits of the base class.

    Value value;
    if (!GetSuperProperty(*this, &ot, value, args.ArgObject, args.ArgMN, SlotInfo::valExecute))
        // Not confirmed ...
        return ThrowReferenceError(VM::Error(VM::eCallNotFoundError, *this
            SF_DEBUG_ARG(args.ArgMN.GetName())
            ));

    // The [[Call]] property is invoked on the value of the resolved property with the arguments 
    // obj, arg1, ..., argn. 
    // The first argument will be used as the "this" value for the method.
    // The result of the method call is pushed onto the stack.
    
    // We MUST discard result here.
    Execute(value, args.ArgObject, arg_count, args.GetCallArgs(), true);
}

void VM::exec_construct(UInt32 arg_count)
{
    // Construct an instance.
    
    // Read arguments ...
    // ReadArgsObjectRef doesn't work here.
    ReadArgsObjectRef args(*this, arg_count);
    
    if (IsException())
        return;
    
    // In case we can use more efficient code here.
    SF_ASSERT(!args.ArgObject.IsTraits());

    // "object" is the function that is being constructed. This will invoke the [[Construct]] property on object with
    // the given arguments. The new instance generated by invoking [[Construct]] will be pushed onto the stack.
    
    // TODO:
    // !!! Look up for a "Construct" property ...
    //A TypeError is thrown if object does not implement the [[Construct]] property.
    
    if (args.ArgObject.IsNullOrUndefined())
        return ThrowTypeError(VM::Error(VM::eConvertNullToObjectError, *this));

    const Value::KindType kind = args.ArgObject.GetKind();
    if (kind == Value::kVTableInd || kind == Value::kVTableIndClosure)
        return ThrowTypeError(VM::Error(VM::eCannotCallMethodAsConstructor, *this
        SF_DEBUG_ARG(GetValueTraits(args.ArgObject).GetName())
        ));

    if (!args.ArgObject.IsObject())
        return ThrowTypeError(VM::Error(VM::eNotConstructorError, *this
        SF_DEBUG_ARG(args.ArgObject)
        ));

    args.ArgObject.GetObject()->Construct(args.ArgObject, arg_count, args.GetCallArgs());
}

void VM::exec_constructsuper(const Traits& ot, UInt32 arg_count)
{
    // Construct an instance of the base class.
    // This will invoke the constructor on the base class of object with 
    // the given arguments.
    
    // Read arguments ...
    ReadArgsObject args(*this, arg_count);
    
    if (IsException())
        return;
    
    /* Old code.
    // Do not delete. Keep for reference.
    const Class& cl = ot->GetClass();

    SF_ASSERT(cl.GetParentClass());
    if (cl.GetParentClass())
    {
        Class& parent_class = *cl.GetParentClass();
        
        // This will invoke the constructor on the base class of object with the given arguments.
        parent_class.GetNewObjectTraits().Super(*ot, args.ArgObject, arg_count, args.GetCallArgs());
    }
    */

    if (ot.GetParent())
    {
        // Dirty trick.
        // constructsuper can be called only for instances, so we can cast
        // Traits to InstanceTraits.
        const InstanceTraits::Traits* ptr = static_cast<const InstanceTraits::Traits*>(ot.GetParent());

        ptr->AS3Constructor(ot, args.ArgObject, arg_count, args.GetCallArgs());
    }
}

void VM::exec_constructprop(VMFile& file, const Abc::Multiname& mn, UInt32 arg_count)
{
    // Construct a property.

    // The [[Construct]] property is invoked on the value of the resolved property with the arguments
    // obj, arg1, ..., argn. 
    // The new instance generated by invoking [[Construct]] will be pushed onto the stack.          
    
    // Read arguments ...
    ReadArgsMnObjectRef args(file, arg_count, mn);
    
    if (IsException())
        return;

    if (args.ArgMN.IsQName())
    {
        const ClassTraits::Traits* ctr = Resolve2ClassTraits(args.ArgMN, file.GetAppDomain());

        if (ctr)
        {
            InstanceTraits::Traits& itr = ctr->GetInstanceTraits();

            itr.GetClass().Construct(args.ArgObject, arg_count, args.GetCallArgs());
            return;
        }
    }

    // Fall back to the regular version.

    PropRef prop;

    FindObjProperty(prop, *this, args.ArgObject, args.ArgMN);

    if (prop)
    {
        Value value;

        if (!prop.GetSlotValueUnsafe(*this, value))
            return;

        if (value.IsNullOrUndefined())
            return ThrowTypeError(VM::Error(VM::eConvertNullToObjectError, *this));

        const Value::KindType kind = value.GetKind();
        if (kind == Value::kVTableInd || kind == Value::kVTableIndClosure)
            return ThrowTypeError(VM::Error(VM::eCannotCallMethodAsConstructor, *this
            SF_DEBUG_ARG(args.ArgMN.GetName())
            ));

        // Sometimes people try to call operator new on regular functions. It happens.
        if (!Value::IsObject(kind))
            return ThrowReferenceError(VM::Error(VM::eUndefinedVarError, *this
            SF_DEBUG_ARG(args.ArgMN.GetName())
            ));

        // TODO:
        // !!! Look up for a "Construct" property ...
        //A TypeError is thrown if object does not implement the [[Construct]] property.

        // We may not use Construct() from Traits here because value can be 
        // a constructor function. And constructor functions have to be objects.
        value.GetObject()->Construct(args.ArgObject, arg_count, args.GetCallArgs());
    } 
    else
    {
        return ThrowReferenceError(VM::Error(VM::ePropertyNotFoundError, *this
            SF_DEBUG_ARG(args.ArgMN.GetName())
            SF_DEBUG_ARG(args.ArgObject)
            ));
    }
}

void VM::exec_getdescendants(VMFile& file, const Abc::Multiname& mn) 
{
    // XML - related ...
    
#ifdef GFX_ENABLE_XML
    // Read arguments ...
    ReadMnObjectRef args(file, mn);

    if (IsException())
        return;

    const XMLSupport& xmls = GetXMLSupport();

    if (!xmls.IsEnabled())
        return ThrowVerifyError(VM::Error(VM::eNotImplementedError, *this SF_DEBUG_ARG("getdescendants")));

    if (args.ArgObject.IsObject() && args.ArgObject.GetObject())
    {
        Object* obj = args.ArgObject.GetObject();
        InstanceTraits::fl::XMLList& itr = static_cast<InstanceTraits::fl::XMLList&>(*xmls.GetITraitsXMLList());
        Pickable<Instances::fl::XMLList> list = itr.MakeInstance(itr);
        // Assign Pickable immediately to prevent memory leaks.
        args.ArgObject = list;
        obj->GetDescendants(*list, args.ArgMN);
    }
    else
    {
        // PopBack() because we haven't remove Object from stack.
        OpStack.PopBack();

        // A TypeError is thrown if obj is not of type XML or XMLList.
        ThrowTypeError(VM::Error(VM::eDescendentsError, *this
            SF_DEBUG_ARG(GetValueTraits(args.ArgObject).GetName().ToCStr())
            ));
    }
#else
    SF_UNUSED2(file, mn);
    ThrowVerifyError(VM::Error(VM::eNotImplementedError, *this SF_DEBUG_ARG("getdescendants")));
#endif
}

void VM::exec_newcatch(VMAbcFile& file, const Abc::MethodBodyInfo::ExceptionInfo& e)
{
    SF_ASSERT(TraitsCatch.GetPtr());
    Classes::fl::Catch& cl = (Classes::fl::Catch&)TraitsCatch->GetInstanceTraits().GetClass();
    // This MakeInstance() should not necessary be implemented in Class.
    OpStack.PushBack(cl.MakeInstance(file, e));

    // No exceptions in this method;
    // Tamarin throws exceptions in this opcode.
}

void VM::FindProperty(PropRef& result, const Multiname& mn, const ScopeStackType& ss, VMAppDomain& appDomain)
{
    FindScopeProperty(result, *this, GetScopeStackBaseInd(), GetScopeStack(), mn);

    // Look in Saved Scope.
    if (!result && !ss.IsEmpty())
        FindScopeProperty(result, *this, 0, ss, mn);

    // Look in Global Objects. This is correct behavior.
    if (!result)
    {
        const ClassTraits::Traits* ctr = GetRegisteredClassTraits(mn, appDomain);

#if 0 // Future development.
        if (ctr && ctr->IsValid() && ctr->GetInstanceTraits().HasConstructorSetup())
        {
            result = PropRef(&ctr->GetInstanceTraits().GetClass(), NULL, 0);
            return;
        }
#endif

        FindGOProperty(result, *this, GetGlobalObjects(), mn, ctr);
    }
}

void VM::exec_findpropstrict(VMFile& file, const Abc::Multiname& mn, const ScopeStackType& ss) 
{
    ReadMn args(file, mn);
    
    if (IsException())
        return;

    // This searches the scope stack, and then the saved scope in the current method closure, for a
    // property with the name specified by the multiname at mn_index.
    
    // Find a property.
    PropRef prop;
    FindProperty(prop, args.ArgMN, ss, file.GetAppDomain());

    if (prop)
    {
        // Push found object ...
        OpStack.PushBack(prop.GetThis());
    } else
    {
        // A ReferenceError is thrown if the property is not resolved in any object on the scope stack.
        // return ThrowReferenceError("The property is not resolved in any object on the scope stack");

        return ThrowReferenceError(VM::Error(VM::eUndefinedVarError, *this
            SF_DEBUG_ARG(args.ArgMN.GetName())
            ));
    }
}

void VM::exec_findproperty(VMFile& file, const Abc::Multiname& mn, const ScopeStackType& ss, Instances::fl::GlobalObject& go) 
{
    // Search the scope stack for a property.
    
    ReadMn args(file, mn);

    if (IsException())
        return;

    // This searches the scope stack, and then the saved scope in the current method closure, for a
    // property with the name specified by the multiname at mn_index.
    
    // If the property is resolved then the object it was resolved in is pushed onto the stack. If the
    // property is unresolved in all objects on the scope stack then the global object is pushed onto
    // the stack.

#if 0
    // We shouldn't put a Class on stack. We should put a global object responsible for this class instead.
    const ClassTraits::Traits* ctr = FindClassTraits(*this, args.ArgMN);
    if (ctr && ctr->IsValid() && ctr->GetInstanceTraits().HasConstructorSetup())
        // Push found object ...
        return OpStack.PushBack(Value(&ctr->GetInstanceTraits().GetClass()));
#endif

    PropRef prop;
    FindProperty(prop, args.ArgMN, ss, file.GetAppDomain());

    if (prop)
    {
        // Push found object ...
        OpStack.PushBack(prop.GetThis());
    } else
    {
        // Push GlobalObject ...
        OpStack.PushBack(Value(&go));
    }
}

void VM::exec_getlex(VMFile& file, const Abc::Multiname& mn, const ScopeStackType& ss) 
{
    // Find and get a property.
    
    if (!mn.IsCompileTime())
        // The multiname at index must not be a runtime multiname.
        return ThrowTypeError(Error(eIllegalOpMultinameError, *this SF_DEBUG_ARG(file.GetInternedString(mn.GetNameInd())) SF_DEBUG_ARG("compile time")));
    
    // This is the equivalent of doing a "findpropstict" followed by a "getproperty". It will find the
    // object on the scope stack that contains the property, and then will get the value from that object.

    // Find a property.
    // The same as "findpropstict" ...

#if 0
    // There is no need to do this because case with classes will be handled by Tracer.
    // This should be safe because Class is a value we want to get.
    const ClassTraits::Traits* ctr = FindClassTraits(*this, mn);
    if (ctr && ctr->IsValid() && ctr->GetInstanceTraits().HasConstructorSetup())
        // Push found object ...
        return OpStack.PushBack(Value(&ctr->GetInstanceTraits().GetClass()));
#endif

    PropRef prop;
    FindProperty(prop, Multiname(file, mn), ss, file.GetAppDomain());

    if (prop)
    {
        // ... and then get the value from this object.

        // Efficient version.
//         OpStack.PushBack(Value::GetUndefined());
//         prop.GetValue(OpStack.Back());
//         if (IsException())
//             return OpStack.PopBack();

        // Current version.
        Value value;

        if (!prop.GetSlotValueUnsafe(*this, value))
            return;

        // DO NOT delete this code.
//         // That should be already handled by GetValue().
//         switch (value.GetKind())
//         {
//         case Value::kFunction:
//             value = GetClassFunction().MakeInstance2(value.AsFunction(), prop.GetObject() SF_DEBUG_ARG(value.AsFunction().GetFunctionName()));
//             break;
//         case Value::kThunk:
//             value = GetClassFunction().MakeInstance(
//                 (InstanceTraits::Traits&)prop.GetObject()->GetValueTraits(),
//                 *prop.GetObject(), 
//                 value.AsThunk()
//                 );
//             break;
//         default:
//             break;
//         }

        OpStack.PickPushBack(value);
    } else
    {
        // A ReferenceError is thrown if the property is not resolved in any object on the scope stack.
        return ThrowReferenceError(VM::Error(VM::ePropertyNotFoundError, *this
            SF_DEBUG_ARG(file.GetInternedString(mn.GetNameInd()))
            SF_DEBUG_ARG("any object on the scope stack")
            ));
    }
}

void VM::exec_getproperty(VMFile& file, const Abc::Multiname& mn) 
{
    // Get a property.
    
    // Read arguments ...
    ReadMnObjectRef args(file, mn);
    
    if (IsException())
        return;
    
    // The property with the name specified by the multiname will be resolved in object, and the
    // value of that property will be pushed onto the stack. If the property is unresolved,
    // undefined is pushed onto the stack.
    
    Value _this(args.ArgObject, PickValue);
    if(!AS3::GetPropertyUnsafe(*this, _this, args.ArgMN, args.ArgObject))
    {
        if (IsException())
            return;

        // This logic seems to be specific to Tamarin.
        if (_this.IsPrimitive())
        {
            return ThrowReferenceError(VM::Error(VM::ePropertyNotFoundError, *this
                SF_DEBUG_ARG(args.ArgMN.GetName())
                SF_DEBUG_ARG(_this)
                ));
        }
    }
}

void VM::exec_initproperty(VMFile& file, const Abc::Multiname& mn) 
{
    // Initialize a property.
    
    // Read arguments ...
    ReadValueMnObject args(file, mn);
    
    if (IsException())
        return;
    
    InitializerGuard __(*this);

    AS3::SetProperty(*this, args.ArgObject, args.ArgMN, args.ArgValue).DoNotCheck();
}

void VM::exec_deleteproperty(VMFile& file, const Abc::Multiname& mn) 
{
    // Delete a property.
    
    // Read arguments ...
    ReadMnObjectRef args(file, mn);

    // TODO:
    // A ReferenceError is thrown if object is null or undefined.
    // It is TypeError for now.

    if (IsException())
        return;

    // This will invoke the [[Delete]] method on object with the name specified by the multiname.
    // If object is not dynamic or the property is a fixed property then nothing happens, and false
    // is pushed onto the stack. If object is dynamic and the property is not a fixed property, it is
    // removed from object and true is pushed onto the stack.
    bool result = false;

#ifdef GFX_ENABLE_XML
    if (GetXMLSupport().IsEnabled() && IsXMLListObject(args.ArgMN.GetName()))
    {
        return ThrowTypeError(VM::Error(VM::eDeleteTypeError, *this
            SF_DEBUG_ARG(GetValueTraits(args.ArgMN.GetName()).GetName().ToCStr())
            ));
    } else
#endif 
    if (args.ArgObject.IsPrimitive())
    {
        // We want to bypass a thunk here.
        // This logic seems to be Tamarin-specific.
        return ThrowReferenceError(VM::Error(VM::eDeleteSealedError, *this
                SF_DEBUG_ARG(args.ArgMN.GetName())
                SF_DEBUG_ARG(args.ArgObject)
                ));
    }
    else if (args.ArgObject.IsObject())
    {
        Object& obj = *args.ArgObject.GetObject();

        if (obj.IsDynamic())
            result = obj.DeleteProperty(args.ArgMN);

        if (IsException())
            return;
    }

    // All primitive values are also sealed classes.

    args.ArgObject.SetBool(result);
}

void VM::exec_getslot(UInt32 slot_index) 
{
    // Get the value of a slot.
    
    // Read arguments ...
    ReadObjectRef args(*this);
    
    if (IsException())
        return;
    
    SF_ASSERT(args.ArgObject.IsObject());
    const Value _this(args.ArgObject, PickValue);
    _this.GetObject()->GetSlotValueUnsafe(SlotIndex(slot_index), args.ArgObject).DoNotCheck();
}

void VM::exec_setabsslot(UPInt slot_ind)
{
    // Set the value of a slot.
    // slot_ind is ONE based.
    // slot_ind - 1 is an index in a slot table.

    // Read arguments ...
    ReadValueObject args(*this);

    if (IsException())
        return;

    SF_ASSERT(args.ArgObject.IsObject());
    Object& obj = *args.ArgObject.GetObject();

    obj.GetTraits().SetSlotValue(AbsoluteIndex(slot_ind - 1), args.ArgValue, &obj).DoNotCheck();
}

void VM::exec_initabsslot(UPInt slot_ind)
{
    // Set the value of a slot.
    // slot_ind is ONE based.
    // slot_ind - 1 is an index in a slot table.

    // Read arguments ...
    ReadValueObject args(*this);

    if (IsException())
        return;

    SF_ASSERT(args.ArgObject.IsObject());
    Object& obj = *args.ArgObject.GetObject();

    InitializerGuard __(*this);

    obj.GetTraits().SetSlotValue(AbsoluteIndex(slot_ind - 1), args.ArgValue, &obj).DoNotCheck();
}

void VM::exec_setslot(UInt32 slot_index) 
{
    // Set the value of a slot.
    
    // Read arguments ...
    ReadValueObject args(*this);
    
    if (IsException())
        return;
    
    args.ArgObject.GetObject()->SetSlotValue(SlotIndex(slot_index), args.ArgValue).DoNotCheck();
}

void VM::exec_coerce(VMFile& file, const Abc::Multiname& mn) 
{
    // Coerce a value to a specified type.
    
    if (!mn.IsCompileTime())
        // The multiname at index must not be a runtime multiname.
        return ThrowTypeError(Error(eIllegalOpMultinameError, *this SF_DEBUG_ARG(file.GetInternedString(mn.GetNameInd())) SF_DEBUG_ARG("compile time")));

    // Read arguments ...
    ReadValueRef args(*this);

    // No exceptions at this point.
    
    // The type specified by the multiname is resolved, and value is coerced to that type. The
    // resulting value is pushed onto the stack. If any of value�s base classes, or implemented
    // interfaces matches the type specified by the multiname, then the conversion succeeds and the
    // result is pushed onto the stack.
    
    // Resolve multiname ...
    const ClassTraits::Traits* to = Resolve2ClassTraits(file, mn);
    
    if (to)
    {
#if 0
        // We MAY NOT use args.value as a source and a target (aliased).
        // Coerce() expects these values to be separate values.
        Value result;
        if (to->Coerce(args.value, result))
        {
            result.Swap(args.value);
            return;
        }
#else
        // Aliasing seems to be fine for the time being.
        if (to->Coerce(args.value, args.value))
            return;
#endif
        else
        {
            // A TypeError is thrown if value cannot be coerced to the specified type.
            return ThrowTypeError(VM::Error(VM::eCheckTypeFailedError, *this
                SF_DEBUG_ARG(args.value)
                SF_DEBUG_ARG(to->GetName().ToCStr())
                ));
        }
    }
    
    return ThrowReferenceError(VM::Error(VM::ePropertyNotFoundError, *this
        SF_DEBUG_ARG(file.GetInternedString(mn.GetNameInd()))
        SF_DEBUG_ARG("any object on the scope stack")
        ));
}

void VM::exec_newobject(UInt32 arg_count) 
{
    Pickable<Instances::fl::Object> o = MakeObject();
    
    for (UInt32 i = 0; i < arg_count; ++i)
    {
        Value value(OpStack);
        Value name(OpStack);

        // All object properties are dynamic.
        o->AddDynamicSlotValuePair(name.AsString(), value);
    }
    
    OpStack.PushBack(o);

    // It looks like this function doesn't throw exceptions.
    // Tamarin throws exceptions in this opcode.
}

void VM::exec_newclass(VMAbcFile& file, UInt32 v) 
{
    const Abc::ClassInfo& class_info = file.GetClasses().Get(v);
    
    Value& value = OpStack.Top0();
    
    // TBD:
    // The base_type argument must be coerced to Class and then must be a base class 
    // of class_info. This means that there is no automatic creation of base classes,
    // instead compiler is responsible for creating base class instance ahead of time
    // and passing it to us.
    // NOTE: newclass is also called for interfaces,
    // in which case base_type is typically null.

    if (value.IsNullOrUndefined())
    {
        if (class_info.HasSuperClass())
            return ThrowTypeError(VM::Error(VM::eConvertNullToObjectError, *this));
    }
    else
    {
        if (!value.IsClass())
            // Is this a correct exception type and a message?
            return ThrowTypeError(VM::Error(VM::eConvertNullToObjectError, *this));
    }

    // check for *scaleform extension* packages.
    const Abc::Multiname& name = class_info.GetName(file.GetConstPool());
    Instances::fl::Namespace& t_ns = file.GetInternedNamespace(name.GetNamespaceInd());
    const ASString& t_uri = t_ns.GetUri();
    const StringDataPtr t_str(t_uri.ToCStr(), t_uri.GetSize());
    const StringDataPtr scaleform_gfx("scaleform.gfx");

    if (t_str.GetSize() >= scaleform_gfx.GetSize() && SFstrncmp(t_str.ToCStr(), scaleform_gfx.ToCStr(), scaleform_gfx.GetSize()) == 0)
    {
        // This is *scaleform.gfx extension* package.
        const ASString t_name = file.GetInternedString(name.GetNameInd());

        if (!GetGlobalObjectCPP().GetProperty(Multiname(t_ns, t_name), value))
        {
            return ThrowReferenceError(VM::Error(VM::ePropertyNotFoundError, *this
                SF_DEBUG_ARG(t_name.ToCStr())
                SF_DEBUG_ARG(scaleform_gfx.ToCStr())
                ));
        }
    }
    else
    {
        // When this instruction is executed, the scope stack must contain all the scopes of all base
        // classes, as the scope stack is saved by the created ClassClosure.

        ClassTraits::UserDefined& cud = GetUserDefinedTraits(file, class_info);

        if (cud.GetInstanceTraits().HasConstructorSetup())
            // This class was already created by another script. It happens.
            value = &cud.GetInstanceTraits().GetConstructor();
        else
            value = cud.MakeClass();
    }

}

void VM::exec_astype(VMFile& file, const Abc::Multiname& mn) 
{
    // Return the same value, or null if not of the specified type.
    
    // Read arguments ...
    if (!mn.IsCompileTime())
        // The multiname at index must not be a runtime multiname.
        return ThrowTypeError(Error(eIllegalOpMultinameError, *this SF_DEBUG_ARG(file.GetInternedString(mn.GetNameInd())) SF_DEBUG_ARG("compile time")));

    ReadValueRef args(*this);

    // No exceptions at this point.
    
    // Pop value off of the stack. If value is of the type specified by the multiname, push value back
    // onto the stack. If value is not of the type specified by the multiname, then push null onto
    // the stack.
    
    const ClassTraits::Traits* tr = Resolve2ClassTraits(file, mn);
    
    // and must be the name of a type ...
    if (tr)
    {
        const ClassTraits::Traits& value_tr = GetClassTraits(args.value);
        
        if (tr != &value_tr)
        {
            args.value.SetNull();
            return;
        }
    }
    
    return ThrowTypeError(VM::Error(VM::eClassNotFoundError, *this
        SF_DEBUG_ARG(file.GetInternedString(mn.GetNameInd()))
        ));
}

void VM::exec_instanceof() 
{
    // Check the prototype chain of an object for the existence of a type.

    Value type(OpStack);

    if (type.IsFunction() || type.IsClass())
    {
        Value& value = OpStack.Top0();
        Value& result = value;

        if (value.IsNullOrUndefined())
            return result.SetBool(false);

        const Traits* cur_tr = &GetClassTraits(value);
        const Traits& type_tr = GetValueTraits(type);

        if (type.IsClass())
        {
            // Check parents.
            while (cur_tr)
            {
                if (cur_tr == &type_tr)
                    return result.SetBool(true);

                cur_tr = cur_tr->GetParent();
            }
        }
        else
        {
            // Check prototype chain.
            const Class&  type_cl = type_tr.GetClass();

            while (cur_tr)
            {
                AS3::Class& cur_cl = const_cast<Traits*>(cur_tr)->GetClass();
                const AS3::Object& proto = cur_cl.GetPrototype();

                // "type" can be a primitive value.
                if (&proto == &type_cl)
                    return result.SetBool(true);

                cur_tr = cur_tr->GetParent();
            }
        }

        result.SetBool(false);
    }
    else
        ThrowTypeError(VM::Error(VM::eCantUseInstanceofOnNonObjectError, *this));
}

void VM::exec_istype(VMFile& file, const Abc::Multiname& mn) 
{
    // Check whether an Object is of a certain type.
    
    // Read arguments ...
    if (!mn.IsCompileTime())
        // The multiname at index must not be a runtime multiname.
        return ThrowTypeError(Error(eIllegalOpMultinameError, *this SF_DEBUG_ARG(file.GetInternedString(mn.GetNameInd())) SF_DEBUG_ARG("compile time")));

    ReadValueRef args(*this);

    // No exceptions at this point.
    
    const ClassTraits::Traits* type = Resolve2ClassTraits(file, mn);
    
    if (type == NULL)
        // Unresolved multiname ...
        return ThrowTypeError(VM::Error(VM::eClassNotFoundError, *this
            SF_DEBUG_ARG(file.GetInternedString(mn.GetNameInd()))
            ));
    
    // Resolve the type specified by the multiname. Let indexType refer to that type. Compute the
    // type of value, and let valueType refer to that type. If valueType is the same as indexType, result
    // is true. If indexType is a base type of valueType, or an implemented interface of valueType,
    // then result is true. Otherwise result is set to false. Push result onto the stack.
    
    args.value.SetBool(IsOfType(args.value, *type));
}

void VM::exec_debugline(CallFrame& cf, UInt32 v)
{
#ifdef SF_AMP_SERVER
    AMP::ViewStats* stats = GetAdvanceStats();
    if (stats != NULL && 
        AmpServer::GetInstance().IsProfiling() && 
        AmpServer::GetInstance().GetProfileLevel() >= Amp_Profile_Level_ActionScript)
    {
        SetActiveLine(v);
        UInt32 fileId = cf.GetFile().GetAbcFile().GetFileHandle() + (cf.GetMethodBodyInfo().GetMethodInfoInd().Get() << 16);
        stats->RegisterScriptFunction(fileId, cf.GetFile().GetAbcFile().GetSwfFileOffset(), cf.Name->pData, 0, 3, true);
        if (stats->IsDebugPaused())
        {
            AmpServer::GetInstance().SendCurrentState();
        }
        stats->DebugWait();
    }

    cf.CurrLineNumber = v;
#else
    SF_UNUSED2(cf, v);
#endif
}

void VM::exec_debugfile(CallFrame& cf, UInt32 v)
{
#ifdef SF_AMP_SERVER
    VMAbcFile& file = cf.GetFile();

    AMP::ViewStats* stats = GetAdvanceStats();
    if (stats != NULL && 
        AmpServer::GetInstance().IsProfiling() && 
        AmpServer::GetInstance().GetProfileLevel() >= Amp_Profile_Level_ActionScript)
    {
        stats->RegisterSourceFile(file.GetAbcFile().GetFileHandle(), v, file.GetInternedString(v).ToCStr());

        UInt64 fileId = file.GetAbcFile().GetFileHandle();
        fileId <<= 32;
        fileId += v;
        SetActiveFile(fileId);
    }

    cf.CurrFileInd = v;
#else
    SF_UNUSED2(cf, v);
#endif
}

void VM::exec_astypelate() 
{
    // Return the same value, or null if not of the specified type.
    // (Similar to "astype") ...
    
    // Pop class and value off of the stack. class should be an object of type Class. If value is of the
    // type specified by class, push value back onto the stack. If value is not of the type specified by
    // class, then push null onto the stack.
    
    const Value vclass(OpStack);
    
    if (!vclass.IsClass())
    {
        // A TypeError is thrown if class is not of type Class.
        if (vclass.IsNullOrUndefined())
        {
            return ThrowTypeError(VM::Error(
                vclass.IsNull() ? eConvertNullToObjectError :  eConvertUndefinedToObjectError,
                *this
                ));
        }

        return ThrowTypeError(VM::Error(VM::eIsTypeMustBeClassError, *this));
    }
    
    const ClassTraits::Traits& tr = vclass.AsClass().GetClassTraits();
    Value& value = OpStack.Top0();
    
    if (!IsOfType(value, tr))
        value.SetNull();
}

void VM::exec_newarray(UInt32 arr_size) 
{
    Pickable<Instances::fl::Array> a = MakeArray();
    
    a->Pick(OpStack, arr_size);
    
    OpStack.PushBack(a);

    // No exceptions in this method;
    // Tamarin throws exceptions in this opcode.
}

void VM::exec_hasnext2(UInt32 object_reg, UInt32 index_reg) 
{
    AbsoluteIndex reg_ind(index_reg);
    AbsoluteIndex obj_ind(object_reg);
    Value& obj_value = GetRegister(obj_ind);

    if (reg_ind.Get() == obj_ind.Get())
        return ThrowTypeError(VM::Error(VM::eInvalidHasNextError, *this));

    // Check for null/undefined
    if (obj_value.IsNullOrUndefined())
        // Push false.
        return OpStack.PushBack(Value(false));

    UInt32 v = 0;
    if (!GetRegister(reg_ind).Convert2UInt32(v))
        // Exception
        return;

    GlobalSlotIndex slot_ind(v);

    if (obj_value.IsObject())
    {
        Object& obj = obj_value;
        
        GlobalSlotIndex ind = obj.GetNextDynPropIndex(slot_ind);
        Object* cur_obj = &obj;

        const Traits* cur_tr = &obj.GetTraits();
        // Go inside if we have no more attributes to enumerate.
        while (cur_tr && !ind.IsValid())
        {
            // Try to enumerate prototype.
            cur_obj = &const_cast<Class&>(cur_tr->GetClass()).GetPrototype();

            ind = cur_obj->GetNextDynPropIndex(GlobalSlotIndex(0));
            // Check for a parent.
            cur_tr = cur_tr->GetParent();

            // If there is no more properties to enumerate, and there is no
            // parent, then stop.
        }

        // Set object.
        if (ind.IsValid())
        {
            if (cur_obj != &obj)
                SetRegister(obj_ind, Value(cur_obj));
        }
        else
            // Set Object to NULL.
            SetRegister(obj_ind, Value::GetNull());

        // Set index.
        SetRegister(reg_ind, Value(ind.Get()));

        // Store boolean result on stack.
        OpStack.PushBack(Value(ind.IsValid()));
    }
    else
    {
        // Primitive values.

#if 0
        Traits& t = GetValueTraits(obj_value);
        GlobalSlotIndex ind = t.GetNextDynPropIndex(slot_ind);

        Object* cur_obj = NULL;
        const Traits* cur_tr = &t;
        while (cur_tr && !ind.IsValid())
        {
            cur_obj = &const_cast<Class&>(cur_tr->GetClass()).GetPrototype();

            ind = cur_obj->GetNextDynPropIndex(GlobalSlotIndex(0));
            cur_tr = cur_tr->GetParent();
        }

        // Set object.
        if (ind.IsValid())
        {
            if (cur_obj != NULL)
                // Prototype object was found.
                SetRegister(obj_ind, Value(cur_obj));
        }
        else
            // Set Object to NULL.
            SetRegister(obj_ind, Value::GetNull());

        // Set index.
        SetRegister(reg_ind, Value(ind.Get()));

        // Store boolean result on stack.
        OpStack.PushBack(Value(ind.IsValid()));
#endif

        if (obj_value.IsNamespace())
        {
            GlobalSlotIndex ind = obj_value.GetNamespace()->GetNextPropIndex(slot_ind);
            // Set index.
            SetRegister(reg_ind, Value(ind.Get()));
            // Store boolean result on stack.
            OpStack.PushBack(Value(ind.IsValid()));
            return;
        }

        // Traits only.
        GlobalSlotIndex ind = slot_ind;

        const Traits* cur_tr = &GetValueTraits(obj_value);
        Object* cur_obj = NULL;

        if (cur_tr->GetTraitsType() == Traits_Namespace && cur_tr->IsInstanceTraits())
        {
            switch (ind.Get())
            {
            case 0:
            case 1:
                // Set index.
                SetRegister(reg_ind, Value(ind.Get() + 1));
                // Store boolean result on stack.
                OpStack.PushBack(Value(true));
                return;
            default:
                break;
            }
        }

        // Go inside if we have no more attributes to enumerate.
        do {
            // Try to enumerate prototype.
            cur_obj = &const_cast<Class&>(cur_tr->GetClass()).GetPrototype();

            ind = cur_obj->GetNextDynPropIndex(GlobalSlotIndex(0));
            // Check for a parent.
            cur_tr = cur_tr->GetParent();

            // If there is no more properties to enumerate, and there is no
            // parent, then stop.
        }
        while (cur_tr && !ind.IsValid());

        // Set object.
        if (ind.IsValid())
        {
            SetRegister(obj_ind, Value(cur_obj));
        }
        else
            // Set Object to NULL.
            SetRegister(obj_ind, Value::GetNull());

        // Set index.
        SetRegister(reg_ind, Value(ind.Get()));

        // Store boolean result on stack.
        OpStack.PushBack(Value(ind.IsValid()));
    }
}

void VM::exec_applytype(UInt32 arg_count) 
{
    // Read arguments ...
    ReadArgsObjectRef args(*this, arg_count);

    if (IsException())
        return;

    if (!args.ArgObject.IsClass())
        return ThrowTypeError(VM::Error(eTypeAppOfNonParamType, *this));

    args.ArgObject = &args.ArgObject.AsClass().ApplyTypeArgs(args.GetCallArgsNum(), args.GetCallArgs()).GetInstanceTraits().GetClass();
}

void VM::ExecuteInternalUnsafe(const Value& func, const Value& _this, Value& result, unsigned argc, const Value* argv, bool result_on_stack)
{
    SIMD::IS::PrefetchObj(&func);
    SIMD::IS::PrefetchObj(&_this);

    const Value::KindType kind = func.GetKind();
    switch(kind )
    {
    case Value::kFunction:
        {
            Instances::Function& f = func;
            
            // The [[Call]] property is invoked on the value of the resolved property with the arguments 

            if (result_on_stack)
                // Result will be left on stack.
           
				f.Execute(_this, argc, argv);
            else
                f.ExecuteUnsafe(_this, result, argc, argv);
        }
        break;
    case Value::kClass:
    case Value::kObject:
        {
            // It doesn't look like a function ...
            // Just call a "Call" method ...
            
            if (func.GetObject() == NULL)
            {
                return ThrowTypeError(VM::Error(VM::eCallOfNonFunctionError, *this
                    SF_DEBUG_ARG(func)
                    ));
            }

#ifdef SF_FPE_ENABLE
            FPEGuard fpe_guard;
#endif
            func.GetObject()->Call(_this, result, argc, argv);

            // C++. We need to handle exceptions ...
            if (IsException())
                return;
            
            if (result_on_stack)
                OpStack.PickPushBack(result);
        }
        break;
    case Value::kThunk:
        {
            SIMD::IS::PrefetchObj(&func.AsThunk());
            const ThunkInfo& thunk = func.AsThunk();

            ExecuteThunkUnsafe(thunk, _this, result, argc, argv, result_on_stack);
        }
        break;
    case Value::kThunkFunction:
        {
            // ThunkClosure behaves similar to MethodClosure.
            Instances::ThunkFunction& f = func;
            
            f.ExecuteUnsafe(_this, result, argc, argv);
            
            // C++. We need to handle exceptions ...
            if (IsException())
                return;
            
            if (result_on_stack)
                OpStack.PickPushBack(result);
        }
        break;
    case Value::kThunkClosure:
        {
            SIMD::IS::PrefetchObj(&func.AsThunk());
            const ThunkInfo& thunk = func.AsThunk();
            const Value _thisValue(func.GetClosure());

            ExecuteThunkUnsafe(thunk, _thisValue, result, argc, argv, result_on_stack);
        }
        break;
    case Value::kVTableInd:
        {
            const VTable& vt = func.GetTraits().GetVT();
            const SInt32 ind = func.GetVTableInd();
            const Value& real_func = vt.GetRaw(AbsoluteIndex(ind));
            SIMD::IS::PrefetchObj(&real_func);

            // VTable stores MethodInd or Thunk.
            if (real_func.GetKind() == Value::kMethodInd)
            {
                // MethodInd.
                ExecuteMethodIndUnsafe(func, real_func, _this, result, argc, argv, result_on_stack SF_DEBUG_ARG(ind));
            }
            else
            {
                // Thunk.

                SIMD::IS::PrefetchObj(&real_func.AsThunk());
                const ThunkInfo& thunk = real_func.AsThunk();

                ExecuteThunkUnsafe(thunk, _this, result, argc, argv, result_on_stack);
            }
        }
        break;
    case Value::kVTableIndClosure:
        {
            const VTable& vt = func.IsSuperCall() ? func.GetClosure()->GetTraits().GetParent()->GetVT() : func.GetClosure()->GetVT();
            const SInt32 ind = func.GetVTableInd();
            const Value& real_func = vt.GetRaw(AbsoluteIndex(ind));
            SIMD::IS::PrefetchObj(&real_func);
            const Value _thisValue(func.GetClosure());

            // VTable stores MethodInd or Thunk.
            if (real_func.GetKind() == Value::kMethodInd)
            {
                // MethodInd.
                ExecuteMethodIndUnsafe(func, real_func, _thisValue, result, argc, argv, result_on_stack SF_DEBUG_ARG(ind));
            }
            else
            {
                // Thunk.

                SIMD::IS::PrefetchObj(&real_func.AsThunk());
                const ThunkInfo& thunk = real_func.AsThunk();

                ExecuteThunkUnsafe(thunk, _thisValue, result, argc, argv, result_on_stack);
            }
        }
        break;
    default:
        ThrowTypeError(VM::Error(VM::eCallOfNonFunctionError, *this
            SF_DEBUG_ARG(func)
            ));
        break;
    }
}

void VM::ExecuteVTableIndUnsafe(UInt32 ind, const Traits& tr, Value& _this, unsigned argc, const Value* argv)
{
    const Value& real_func = tr.GetVT().GetRaw(AbsoluteIndex(ind));
    SIMD::IS::PrefetchObj(&real_func);

#if 0
    // It is not correct to convert Closure to Object here.
    // No explanation for the time being.
    if (_this.IsClosure())
        _this.Assign(_this.GetClosure()) ;
#endif

    // VTable stores MethodInd or Thunk.
    if (real_func.GetKind() == Value::kMethodInd)
    {
        // MethodInd.
        // We keep this code inlined because we do not want to create temporary
        // Value result.
        const Value func(ind, tr, Value::kVTableInd);

        const Abc::MiInd method_ind(real_func.GetMethodInfoInd());
        const AS3::Traits& real_tr = real_func.GetTraits();
        SF_ASSERT(real_tr.GetFilePtr());
        VMAbcFile& file = *real_tr.GetFilePtr();
        const Abc::MbiInd mbi_ind = file.GetMethodBodyInfoInd(method_ind);

#ifdef SF_AS3_ENABLE_EXPLICIT_GO
//         SF_ASSERT(!real_tr.IsGlobal());
//         SF_ASSERT(real_tr.GetGlobalObjectScript());
        Instances::fl::GlobalObjectScript* gos = real_tr.GetGlobalObjectScript();
        if (gos == NULL)
        {
//             SF_ASSERT(GetValueTraits(real_tr.GetStoredScopeStack()[0]).IsGlobal());
//             gos = static_cast<Instances::fl::GlobalObjectScript*>(real_tr.GetStoredScopeStack()[0].GetObject());

            SF_ASSERT(GetValueTraits(_this).IsGlobal());
            gos = static_cast<Instances::fl::GlobalObjectScript*>(_this.GetObject());
        }

        SF_ASSERT(gos);
#endif

        AddFrame(
            func, // We have to pass real value. Undefined won't work.
            file,
            mbi_ind,
            _this,
            argc,
            argv,
            false, // discard_result
#ifdef SF_AS3_ENABLE_EXPLICIT_GO
            *gos,
#endif
            real_tr.GetStoredScopeStack(),
            real_tr
            SF_DEBUG_ARG(real_tr.GetVT().GetName(AbsoluteIndex(ind)))
            );
    }
    else
    {
        // Thunk.

        SIMD::IS::PrefetchObj(&real_func.AsThunk());
        const ThunkInfo& thunk = real_func.AsThunk();
        Value result;

        ExecuteThunkUnsafe(thunk, _this, result, argc, argv, true);
    }
}

SF_INLINE
void VM::ExecuteThunkUnsafe(const ThunkInfo& thunk, const Value& _this, Value& result, unsigned argc, const Value* argv, bool result_on_stack)
{
    // Check number of arguments.
    const unsigned max_arg_num = static_cast<unsigned>(thunk.GetMaxArgNum());
    // thunk.HasEllipsis() means "variable number of arguments", but we still may specify max_arg_num (class Date).
    // In this case we may pass any number of arguments, but only max_arg_num arguments will be handled by a method.
    if (argc < static_cast<unsigned>(thunk.GetMinArgNum()) || (!thunk.HasEllipsis() && argc > max_arg_num))
        return ThrowArgumentError(
            VM::Error(
                VM::eWrongArgumentCountError, 
                *this
                SF_DEBUG_ARG(thunk.Name)
                SF_DEBUG_ARG(static_cast<int>(thunk.GetMinArgNum()))
                SF_DEBUG_ARG(static_cast<int>(thunk.GetMaxArgNum()))
                SF_DEBUG_ARG(argc)
                )
            );

#ifdef SF_FPE_ENABLE
    FPEGuard fpe_guard;
#endif

#ifdef SF_AS3_EMIT_DEF_ARGS
    if (thunk.HasEllipsis() || argc >= max_arg_num)
        // All arguments are in place.
        // Run method.
        (*thunk.Method)(thunk, *this, _this, result, argc, argv);
    else
    {
        ValueArrayDH args(GetMemoryHeap());
        args.Reserve(max_arg_num);
        UPInt i = 0;

        // Move existing values.
        for (; i < argc; ++i)
            args.PushBack(argv[i]);

        const UPInt first_opt_param_num = max_arg_num - thunk.GetDefArgNum();

        // Generic default value.
        for (; i < first_opt_param_num; ++i)
        {
            // ArgType[0] is type of return value.
            const TypeInfo* arg_type = thunk.ArgType[i + 1];

            if (arg_type == NULL)
                args.PushBack(Value::GetUndefined());
            else if (arg_type->IsBoolean())
                args.PushBack(Value(false));
            else if (arg_type->IsInt())
                args.PushBack(Value(SInt32(0)));
            else if (arg_type->IsUInt())
                args.PushBack(Value(UInt32(0)));
            else if (arg_type->IsString())
                args.PushBack(Value(GetStringManager().CreateEmptyString()));
            else if (arg_type->IsNumber())
                args.PushBack(Value(NumberUtil::NaN()));
            else
                args.PushBack(Value((Object*)NULL));
        }

        // Stored default value.
        for (; i < max_arg_num; ++i)
        {
            const Abc::ConstValue& cv = thunk.DefArgValue[i - first_opt_param_num];
            const Abc::ValueKind k = cv.GetKind();

            switch (k)
            {
            case Abc::CONSTANT_Undefined:
                args.PushBack(Value::GetUndefined());
                break;
            case Abc::CONSTANT_Utf8:
                args.PushBack(Value(GetStringManager().CreateConstString(Instances::fl::GlobalObjectCPP::Strings[cv.GetIndex()])));
                break;
            case Abc::CONSTANT_Int:
                args.PushBack(Value(Instances::fl::GlobalObjectCPP::Ints[cv.GetIndex()]));
                break;
            case Abc::CONSTANT_UInt:
                args.PushBack(Value(Instances::fl::GlobalObjectCPP::UInts[cv.GetIndex()]));
                break;
            case Abc::CONSTANT_Double:
                args.PushBack(Value(Instances::fl::GlobalObjectCPP::Numbers[cv.GetIndex()]));
                break;
            case Abc::CONSTANT_True:
                args.PushBack(Value(true));
                break;
            case Abc::CONSTANT_False:
                args.PushBack(Value(false));
                break;
            case Abc::CONSTANT_Null:
                args.PushBack(Value((Object*)NULL));
                break;
            default:
                break;
            }
        }

        // Finally run method.
        (*thunk.Method)(thunk, *this, _this, result, max_arg_num, &args[0]);
    }
#else

    (*thunk.Method)(thunk, *this, _this, result, argc, argv);

#endif

    // C++. We need to handle exceptions ...
    if (result_on_stack && !IsException())
        OpStack.PickPushBack(result);
}

SF_INLINE
void VM::ExecuteMethodIndUnsafe(const Value& func, const Value& real_func, const Value& _this, Value& result, unsigned argc, const Value* argv, bool result_on_stack SF_DEBUG_ARG(const SInt32 ind))
{
    const Abc::MiInd method_ind(real_func.GetMethodInfoInd());
    const AS3::Traits& real_tr = real_func.GetTraits();
    SF_ASSERT(real_tr.GetFilePtr());
    VMAbcFile& file = *real_tr.GetFilePtr();
    const Abc::MbiInd mbi_ind = file.GetMethodBodyInfoInd(method_ind);

#ifdef SF_AS3_ENABLE_EXPLICIT_GO
//     SF_ASSERT(real_tr.GetGlobalObjectScript());
//     Instances::fl::GlobalObjectScript& gos = *real_tr.GetGlobalObjectScript();

    Instances::fl::GlobalObjectScript* gos = real_tr.GetGlobalObjectScript();
    if (gos == NULL)
    {
        SF_ASSERT(GetValueTraits(_this).IsGlobal());
        gos = static_cast<Instances::fl::GlobalObjectScript*>(_this.GetObject());
    }

    SF_ASSERT(gos);
#endif

    AddFrame(
        func,
        file,
        mbi_ind,
        _this,
        argc,
        argv,
        false, // discard_result
#ifdef SF_AS3_ENABLE_EXPLICIT_GO
        *gos,
#endif
        real_tr.GetStoredScopeStack(),
        real_tr
        SF_DEBUG_ARG(real_tr.GetVT().GetName(AbsoluteIndex(ind)))
        );

    if (!result_on_stack)
        ExecuteAndRetrieveResult(result);
}

void VM::Execute_IgnoreExceptionUnsafe(const Value& func, const Value& _this,
                                       Value& result, unsigned argc, const Value* argv)
{
    ExecuteUnsafe(func, _this, result, argc, argv);
    if (IsException())    
        OutputAndIgnoreException();
}


void VM::Coerce2ReturnType(const Value& value, Value& result)
{
    // Resolve multiname ...
    const CallFrame& cf = GetCurrCallFrame();
    const Abc::Multiname& mn = cf.GetReturnType();
    const ClassTraits::Traits* to = Resolve2ClassTraits(cf.GetFile(), mn);

    if (to)
    {
        if (!to->Coerce(value, result))
        {
            // A TypeError is thrown if return_value cannot be coerced to the expected return type of the
            // executing method.
            return ThrowTypeError(VM::Error(VM::eCheckTypeFailedError, *this
                SF_DEBUG_ARG(value)
                SF_DEBUG_ARG(to->GetName().ToCStr())
                ));
        }
    } else
    {
        //SF_ASSERT2(false, "Couldn't resolve multiname " + AsString(mn));
        return ThrowTypeError(VM::Error(VM::eClassNotFoundError, *this
            SF_DEBUG_ARG(mn.GetName(cf.GetFile().GetConstPool()).ToCStr())
            ));
    }
}

ClassTraits::Traits* VM::GetClassTraits(const ASString& name, const Instances::fl::Namespace& ns, VMAppDomain& appDomain)
{
    ClassTraits::Traits** ptr = appDomain.GetClassTrait(name, ns);

    if (ptr == NULL)
        return NULL;

    return *ptr;
}

ClassTraits::UserDefined& VM::GetUserDefinedTraits(VMFile& file, const Abc::ClassInfo& ci)
{
    const Abc::Multiname& name = file.GetMultiname(ci.GetNameInd());
    const ASString t_name = file.GetInternedString(name.GetNameInd());
    const Instances::fl::Namespace& t_ns = file.GetInternedNamespace(name.GetNamespaceInd());

    ClassTraits::Traits** ptr = file.GetAppDomain().GetClassTrait(t_name, t_ns);

    if (ptr == NULL)
    {
        // check for *scaleform extension* packages.
        const ASString& t_uri = t_ns.GetUri();
        const StringDataPtr t_str(t_uri.ToCStr(), t_uri.GetSize());
        const StringDataPtr scaleform_gfx("scaleform.gfx");

        if (t_str.GetSize() >= scaleform_gfx.GetSize() && SFstrncmp(t_str.ToCStr(), scaleform_gfx.ToCStr(), scaleform_gfx.GetSize()) == 0)
        {
            // This is *scaleform.gfx extension* package.
            return const_cast<ClassTraits::UserDefined&>(
                static_cast<const ClassTraits::UserDefined&>(
                    *Resolve2ClassTraits(file, name)
                    )
                );
        }

        SF_ASSERT(false);
    }

    return static_cast<ClassTraits::UserDefined&>(**ptr);
}

void VM::OutputError(const Value& e)
{
    Value r;
    StringManager& sm = GetStringManager();
    ASString errorString = sm.CreateEmptyString();

    // Stack trace is stored in an Exception object. The easiest way to retrieve it is to call
    // the "getStackTrace" method.
    const Multiname prop_name(GetPublicNamespace(), GetStringManager().CreateConstString("getStackTrace"));
    if (!e.IsNullOrUndefined() && AS3::ExecutePropertyUnsafe(*this, prop_name, e, r, 0, NULL))
    {
        if (!r.Convert2String(errorString))
            return;
    }

    if (e.IsNullOrUndefined() || errorString.IsEmpty())
    {
        // It looks like there is no getStackTrace() method.
        if (!e.Convert2String(errorString))
            return;
    }

    UI.Output(FlashUI::Output_Error, errorString.ToCStr());
}

Pickable<Instances::fl::Namespace> VM::MakeNamespace(Abc::NamespaceKind kind,
                                        const ASString& uri, const Value& prefix) const
{
    SF_UNUSED(uri);
    return static_cast<InstanceTraits::fl::Namespace&>(GetClassTraitsNamespace().GetInstanceTraits()).MakeInstance(kind, uri, prefix);
}

Pickable<Instances::fl::Namespace> VM::MakeInternedNamespace(Abc::NamespaceKind kind,
                                               const ASString& uri) const
{
    if (uri.IsEmpty() && kind == Abc::NS_Public)
        return Pickable<Instances::fl::Namespace>(&GetPublicNamespace(), PickValue);

    InstanceTraits::fl::Namespace& nsInstanceTraits = static_cast<InstanceTraits::fl::Namespace&>(GetClassTraitsNamespace().GetInstanceTraits());
    return nsInstanceTraits.MakeInternedInstance(kind, uri, Value::GetUndefined());
}

Pickable<Instances::fl::Namespace> VM::MakeInternedNamespace(Abc::NamespaceKind kind,
                                               const char* name) const
{
    if (!name) name = "";
    return MakeInternedNamespace(kind, GetStringManager().CreateString(name));    
}

Pickable<Instances::fl::Namespace> VM::MakeInternedNamespace(Abc::NamespaceKind kind, const StringDataPtr& uri) const
{
    return MakeInternedNamespace(kind, GetStringManager().CreateString(uri.ToCStr(), uri.GetSize()));    
}

Instances::fl::Namespace& VM::GetInternedNamespace(Abc::NamespaceKind kind,
                                               const ASString& uri) const
{
    if (uri.IsEmpty() && kind == Abc::NS_Public)
        return GetPublicNamespace();

    InstanceTraits::fl::Namespace& nsInstanceTraits = static_cast<InstanceTraits::fl::Namespace&>(GetClassTraitsNamespace().GetInstanceTraits());
    return nsInstanceTraits.GetInternedInstance(kind, uri, Value::GetUndefined());
}

Instances::fl::Namespace& VM::GetInternedNamespace(Abc::NamespaceKind kind,
                                               const char* name) const
{
    if (!name) name = "";
    return GetInternedNamespace(kind, GetStringManager().CreateString(name));    
}

Instances::fl::Namespace& VM::GetInternedNamespace(Abc::NamespaceKind kind, const StringDataPtr& uri) const
{
    return GetInternedNamespace(kind, GetStringManager().CreateString(uri.ToCStr(), uri.GetSize()));    
}

SPtr<VMAbcFile> VM::LoadFile(const Ptr<Abc::File>& file, VMAppDomain& appDomain, bool to_execute)
{
    LoadingAbcFile = true;
    {
        // We need pCurrentFile to be able to initialize InstanceTraits.
        SPtr<VMAbcFile> vmfile(MakePickable(SF_HEAP_NEW_ID(GetMemoryHeap(), StatMV_VM_VMAbcFile_Mem) VMAbcFile(*this, file, appDomain SF_AOTC_ARG(pIC) SF_AOTC2_ARG(pIC))));

        if (vmfile->Load(to_execute))
        {
            // Register file.
            //Files.PushBack(vmfile);
            LoadingAbcFile = false;
            return vmfile;
        }
    }
    LoadingAbcFile = false;
    return NULL;
}

Class* VM::GetClass(const StringDataPtr& gname, VMAppDomain& appDomain)
{
    Class* _class = NULL;

    // This check for empty name slightly improves performance.
    if (!gname.IsEmpty())
    {
        static const StringDataPtr vecPref("Vector.<", 8);

        if (gname.GetSize() > vecPref.GetSize() && gname.GetTrimRight(gname.GetSize() - vecPref.GetSize()) == vecPref)
        {
            SPInt end = gname.FindLastChar('>');
            if (end > 0)
            {
                const StringDataPtr subtype_name(gname.GetTrimLeft(vecPref.GetSize()).GetTrimRight(gname.GetSize() - end));
                Class* subtype_class = GetClass(subtype_name, appDomain);

                if (subtype_class)
                {
                    const Value subtype_value(subtype_class);
                    _class = &GetClassVector().ApplyTypeArgs(1, &subtype_value).GetInstanceTraits().GetClass();
                }
            }
        }
        else
        {
            // Create a parsed multiname
            const Multiname mn(*this, gname);

            // Check for a Class name.
            const ClassTraits::Traits* ctr = Resolve2ClassTraits(mn, appDomain);

            if (ctr)
            {
                ctr->InitOnDemand();

                if (!IsException())
                    _class = &ctr->GetInstanceTraits().GetClass();
            }
        }
    }

    return _class;
}

bool VM::GetClassUnsafe(const StringDataPtr& gname, VMAppDomain& appDomain, Value& result)
{
    Class* _class = GetClass(gname, appDomain);

    if (_class)
    {
        result.AssignUnsafe(_class) ;
        return true;
    }

    if (IsException())
        return false;

    ThrowReferenceError(VM::Error(VM::eUndefinedVarError, *this
        SF_DEBUG_ARG(gname.ToCStr())
        ));

    return false;
}

bool VM::Construct(Class& cl, Value& result, unsigned argc, const Value* argv, bool extCall)
{
    const UPInt original_stack_size = CallStack.GetSize();

    // Create instance.
    cl.Construct(result, argc, argv, extCall);

    if (IsException())
        return false;

    return CallStack.GetSize() > original_stack_size;
}

bool VM::Construct(const char* gname, VMAppDomain& appDomain, Value& result, unsigned argc, const Value* argv, bool extCall)
{
    // Get class.
    Value value;
    // !!! GetClass() MAY NOT be used here because of initialization on demand.
    if (!GetClassUnsafe(gname, appDomain, value))
        return false;

    // A TypeError is thrown if object does not implement the [[Construct]] property.

    if (value.IsNullOrUndefined())
    {
        ThrowTypeError(VM::Error(VM::eConvertNullToObjectError, *this));
        return false;
    }

    // Create instance.
    return Construct(value.AsClass(), result, argc, argv, extCall);
}

// Private helper to ConstructBuiltinObject
CheckResult VM::constructBuiltinObject(SPtr<Object> &pobj, const char* gname,
                                unsigned argc, const Value* argv)
{
    bool result = false;
    Value v;
    const bool need2execute = Construct(gname, GetCurrentAppDomain(), v, argc, argv);

    SF_UNUSED(need2execute);
    SF_ASSERT(!need2execute);

    if (!IsException() && !v.IsNullOrUndefined())
    {
        pobj = v.GetObject();
        result = true;
    }
    else
    {
        pobj = NULL;
        if (IsException())
            IgnoreException();
    }

    return result;
}

Traits& VM::GetValueTraits(const Value& v) const
{
    switch (v.GetKind())
    {
    case Value::kUndefined:
        break;
    case Value::kBoolean:
        return GetClassTraitsBoolean().GetInstanceTraits();
    case Value::kInt:
        return GetClassTraitsSInt().GetInstanceTraits();
    case Value::kUInt:
        return GetClassTraitsUInt().GetInstanceTraits();
    case Value::kNumber:
        return GetClassTraitsNumber().GetInstanceTraits();
    case Value::kString:
        if (v.GetStringNode())
            return GetClassTraitsString().GetInstanceTraits();
        else
            return GetClassTraitsObject().GetInstanceTraits();
    case Value::kNamespace:
        return GetClassTraitsNamespace().GetInstanceTraits();
    case Value::kFunction:
    case Value::kObject:
    case Value::kClass:
    // ThunkFunction is a real Object. It just wraps a Thunk.
    case Value::kThunkFunction:
        if (v.IsNull())
            return GetClassTraitsObject().GetInstanceTraits();
        else
            return v.GetObject()->GetTraits();
    case Value::kThunk:
    case Value::kThunkClosure:
        return GetClassTraitsFunction().GetThunkTraits();
#if 1 
    // We MAY NOT return different traits here until we fix class Tracer to 
    // return the same Traits for these cases.
    case Value::kVTableInd:
    case Value::kVTableIndClosure:
        return GetClassTraitsFunction().GetVTableTraits();
#else
#if 0
    case Value::kVTableInd:
        {
            const VTable& vt = v.GetVTable();
            const SInt32 ind = v.GetVTableInd();
            const Value& real_func = vt.GetRaw(AbsoluteIndex(ind));

            // VTable stores MethodInd or Thunk.
            if (real_func.GetKind() == Value::kMethodInd)
            {
                // MethodInd.
                const VTable& real_vt = real_func.GetVTable();
                return real_vt.GetTraits();
            }
            else
                // Thunk.
                return GetClassTraitsFunction().GetThunkTraits();
        }
        break;
    case Value::kVTableIndClosure:
        {
            const VTable& vt = v.IsSuperCall() ? v.GetClosure()->GetTraits().GetParent()->GetVT() : v.GetClosure()->GetVT();
            const SInt32 ind = v.GetVTableInd();
            const Value& real_func = vt.GetRaw(AbsoluteIndex(ind));

            // VTable stores MethodInd or Thunk.
            if (real_func.GetKind() == Value::kMethodInd)
            {
                // MethodInd.
                const VTable& real_vt = real_func.GetVTable();
                return real_vt.GetTraits();
            }
            else
                // Thunk.
                return GetClassTraitsFunction().GetThunkTraits();
        }
        break;
#else
    case Value::kVTableInd:
        {
            const VTable& vt = v.GetVTable();
            return vt.GetTraits();
        }
        break;
    case Value::kVTableIndClosure:
        return v.GetClosure()->GetTraits();
#endif
#endif
#if defined(SF_AS3_AOTC) || defined(SF_AS3_AOTC2)
    case Value::kSNodeIT:
#endif
    case Value::kInstanceTraits:
        return v.GetInstanceTraits();
#if defined(SF_AS3_AOTC) || defined(SF_AS3_AOTC2)
    case Value::kSNodeCT:
#endif
    case Value::kClassTraits:
        return v.GetClassTraits();
    case Value::kMethodInd:
        break;
    }

    return GetITraitsVoid();
}

const ClassTraits::Traits& VM::GetClassTraits(const Value& v) const
{
    const Value::KindType k = v.GetKind();
    switch(k)
    {
    case Value::kBoolean:
        return GetClassTraitsBoolean();
    case Value::kInt:
        return GetClassTraitsSInt();
    case Value::kUInt:
        return GetClassTraitsUInt();
    case Value::kNumber:
        return GetClassTraitsNumber();
    case Value::kString:
        if (v.GetStringNode())
            // Not null.
            return GetClassTraitsString();
        else
            // Return ClassTraits::Object for null.
            return GetClassTraitsObject();
    case Value::kClass:
        return v.AsClass().GetClassTraits();
    case Value::kNamespace:
        return GetClassTraitsNamespace();
    case Value::kThunk:
    case Value::kThunkFunction:
    case Value::kFunction:
    case Value::kThunkClosure:
    case Value::kVTableInd:
    case Value::kVTableIndClosure:
        // This should work in case of null.
        return *TraitsFunction;
#if defined(SF_AS3_AOTC) || defined(SF_AS3_AOTC2)
    case Value::kSNodeCT:
#endif
    case Value::kClassTraits:
        return v.GetClassTraits();
    default:
        break;
    }

    if (v.GetObject())
        return v.GetObject()->GetClass().GetClassTraits();
    else
        // Return ClassTraits::Object for null.
        return GetClassTraitsObject();
}

InstanceTraits::Traits& VM::GetInstanceTraits(const Value& v) const
{
    const Value::KindType k = v.GetKind();
    switch(k)
    {
    case Value::kUndefined:
        break;
    case Value::kBoolean:
        return GetClassTraitsBoolean().GetInstanceTraits();
    case Value::kInt:
        return GetClassTraitsSInt().GetInstanceTraits();
    case Value::kUInt:
        return GetClassTraitsUInt().GetInstanceTraits();
    case Value::kNumber:
        return GetClassTraitsNumber().GetInstanceTraits();
    case Value::kString:
        if (v.GetStringNode())
            // Not null.
            return GetClassTraitsString().GetInstanceTraits();
        else
            return GetClassTraitsObject().GetInstanceTraits();
    case Value::kNamespace:
        return GetClassTraitsNamespace().GetInstanceTraits();
    case Value::kFunction:
    case Value::kObject:
        // ThunkFunction is a real Object. It just wraps a Thunk.
    case Value::kThunkFunction:
        if (v.IsNull())
            return GetClassTraitsObject().GetInstanceTraits();
        else
            return static_cast<InstanceTraits::Traits&>(v.GetObject()->GetTraits());
    case Value::kClass:
        if (v.IsNull())
            // ??? Object?
            return GetClassTraitsObject().GetInstanceTraits();
        else
            return v.AsClass().GetClassTraits().GetInstanceTraits();
    case Value::kThunk:
    case Value::kThunkClosure:
        return GetClassTraitsFunction().GetThunkTraits();
    case Value::kVTableInd:
    case Value::kVTableIndClosure:
        return GetClassTraitsFunction().GetVTableTraits();
#if defined(SF_AS3_AOTC) || defined(SF_AS3_AOTC2)
    case Value::kSNodeIT:
#endif
    case Value::kInstanceTraits:
        return v.GetInstanceTraits();
#if defined(SF_AS3_AOTC) || defined(SF_AS3_AOTC2)
    case Value::kSNodeCT:
#endif
    case Value::kClassTraits:
        {
            const ClassTraits::Traits& ctr = v.GetClassTraits();
            if (ctr.IsValid())
                return ctr.GetInstanceTraits();
        }
    case Value::kMethodInd:
        break;
    }

    return GetITraitsVoid();
}

Instances::fl::GlobalObjectScript* VM::GetGlobalObject() const
{
    if (GetCallStack().GetSize() != 0)
        return GetCurrCallFrame().GetGlobalObject();
    
    // Do we have a better solution?
//     return &const_cast<Instances::fl::GlobalObjectCPP&>(GetGlobalObjectCPP());
    SF_ASSERT(false);
    return NULL;
}

bool VM::IsVMCachedType(const InstanceTraits::Traits& tr) const
{
    const InstanceTraits::Traits* p = &tr;
    bool result = false;

    if (IsPrimitiveType(tr))
        result = true;
    else if (p == &GetITraitsClass())
        result = true;
    else if (p == &GetITraitsObject())
        result = true;
    else if (p == &GetITraitsNamespace())
        result = true;
    else if (p == &GetITraitsFunction())
        result = true;
    else if (p == &GetITraitsArray())
        result = true;
    else if (p == &GetITraitsByteArray())
        result = true;
    else if (p == &GetITraitsVectorSInt())
        result = true;
    else if (p == &GetITraitsVectorUInt())
        result = true;
    else if (p == &GetITraitsVectorNumber())
        result = true;
    else if (p == &GetITraitsVectorString())
        result = true;
    else if (p == &GetClassTraitsQName().GetInstanceTraits())
        result = true;
#if 0
    else if (GetXMLSupport().IsEnabled())
    {
        const XMLSupport& xmls = GetXMLSupport();
        if (p == xmls.GetITraitsXML())
            result = true;
        else if (p == xmls.GetITraitsXMLList())
            result = true;
    }
#endif

    return result;
}

bool VM::IsVMCachedType(const ClassTraits::Traits& tr) const
{
    const ClassTraits::Traits* p = &tr;
    bool result = false;

    if (IsPrimitiveType(tr))
        result = true;
    else if (p == &GetClassTraitsClassClass())
        result = true;
    else if (p == &GetClassTraitsObject())
        result = true;
    else if (p == &GetClassTraitsNamespace())
        result = true;
    else if (p == &GetClassTraitsFunction())
        result = true;
    else if (p == &GetClassTraitsArray())
        result = true;
    else if (p == &GetClassTraitsByteArray())
        result = true;
    else if (p == &GetClassTraitsVectorSInt())
        result = true;
    else if (p == &GetClassTraitsVectorUInt())
        result = true;
    else if (p == &GetClassTraitsVectorNumber())
        result = true;
    else if (p == &GetClassTraitsVectorString())
        result = true;
    else if (p == &GetClassTraitsQName())
        result = true;
#if 0
    else if (GetXMLSupport().IsEnabled())
    {
        const XMLSupport& xmls = GetXMLSupport();
        if (p == xmls.GetClassTraitsXML())
            result = true;
        else if (p == xmls.GetClassTraitsXMLList())
            result = true;
    }
#endif

    return result;
}

bool VM::IsVMCachedType(const Traits& tr) const
{
    bool result = false;

    if (tr.IsInstanceTraits())
        result = IsVMCachedType(static_cast<const InstanceTraits::Traits&>(tr));
    else
        result = IsVMCachedType(static_cast<const ClassTraits::Traits&>(tr));

    return result;
}

Value VM::GetDefaultValue(const ClassTraits::Traits& ctr) const
{
    const BuiltinTraitsType tt = ctr.GetTraitsType();

    switch (tt)
    {
    case Traits_Boolean:
        return Value(false);
    case Traits_SInt:
        return Value(SInt32(0));
    case Traits_UInt:
        return Value(UInt32(0));
    case Traits_Number:
        return Value(NumberUtil::NaN());
        // Do not delete code below.
        // !!! We MAY NOT return empty string as a default value.
        // case Traits_String:
        //    return GetStringManager().CreateEmptyString();
    default:
        if (&ctr == &GetClassTraitsClassClass())
            return Value::GetUndefined();
        break;
    }

    return Value::GetNull();
}

Value VM::GetDefaultValue(VMAbcFile& file, const Abc::Multiname& mn)
{
    if (!mn.IsAnyType())
    {
        // AnyType gets converted to *undefined*.
        const ClassTraits::Traits* ctr = Resolve2ClassTraits(file, mn);

        if (ctr)
            return GetDefaultValue(*ctr);
    }

    return Value::GetUndefined();
}

InstanceTraits::Traits& VM::GetFunctReturnType(const ThunkInfo& thunk, VMAppDomain& appDomain)
{
    const TypeInfo* rt = thunk.GetResultType();

    if (rt)
    {
        StringManager& sm = GetStringManager();
        const ASString name = sm.CreateConstString(rt->GetName());
        const SPtr<Instances::fl::Namespace> ns = MakeInternedNamespace(Abc::NS_Public, rt->GetPkgName());

        const ClassTraits::Traits* ctr = Resolve2ClassTraits(name, *ns, appDomain);

        if (ctr)
            return ctr->GetInstanceTraits();
    }

    return GetITraitsObject();
}

InstanceTraits::Traits& VM::GetFunctReturnType(const Value& value, VMAppDomain& appDomain)
{
    const Value::KindType kind = value.GetKind();

    switch (kind)
    {
    case Value::kFunction:
        {
            const Instances::Function& f = value.AsFunction();
            const ClassTraits::Traits* ctr = Resolve2ClassTraits(f.GetFile(), f.GetReturnType());

            if (ctr)
                return ctr->GetInstanceTraits();
        }
        break;
    case Value::kVTableInd:
    case Value::kVTableIndClosure:
        {
            const VTable& vt = value.GetTraits().GetVT();
            const SInt32 ind = value.GetVTableInd();
            const Value& real_func = vt.GetRaw(AbsoluteIndex(ind));

            // VTable stores MethodInd or Thunk.
            if (real_func.GetKind() == Value::kMethodInd)
            {
                // MethodInd.

                const Abc::MiInd method_ind(real_func.GetMethodInfoInd());
                const Traits& real_tr = real_func.GetTraits();
                SF_ASSERT(real_tr.GetFilePtr());
                VMAbcFile& file = *real_tr.GetFilePtr();
                //const Abc::MethodBodyInfo& mbi = file.GetMethodBodyInfo(method_ind);
                const Abc::MethodInfo& mi = file.GetMethodInfo(method_ind);
                const Abc::Multiname& rt = mi.GetReturnType(file.GetConstPool());
                const ClassTraits::Traits* ctr = Resolve2ClassTraits(file, rt);

                if (ctr)
                    return ctr->GetInstanceTraits();
            }
            else
            {
                // Thunk.
                switch (kind)
                {
                case Value::kVTableInd:
                    return GetFunctReturnType(real_func.AsThunk(), appDomain);
                    break;
                case Value::kVTableIndClosure:
                    return GetFunctReturnType(real_func.AsThunk(), appDomain);
                    break;
                default:
                    break;
                }
            }
        }
        break;
    case Value::kMethodInd:
        {
            // MethodInd.

            const Abc::MiInd method_ind(value.GetMethodInfoInd());
            const Traits& real_tr = value.GetTraits();
            SF_ASSERT(real_tr.GetFilePtr());
            VMAbcFile& file = *real_tr.GetFilePtr();
            //const Abc::MethodBodyInfo& mbi = file.GetMethodBodyInfo(method_ind);
            const Abc::MethodInfo& mi = file.GetMethodInfo(method_ind);
            const Abc::Multiname& rt = mi.GetReturnType(file.GetConstPool());
            const ClassTraits::Traits* ctr = Resolve2ClassTraits(file, rt);

            if (ctr)
                return ctr->GetInstanceTraits();
        }
        break;
    case Value::kThunk:
        return GetFunctReturnType(value.AsThunk(), appDomain);
    case Value::kThunkFunction:
        return GetFunctReturnType(value.AsThunkFunction().GetThunk(), appDomain);
    case Value::kThunkClosure:
        return GetFunctReturnType(value.AsThunk(), appDomain);
    default:
        break;
    }

    return GetITraitsObject();
}

bool VM::CheckObject(const Value& v)
{
    bool result = true;

    if (v.IsNullOrUndefined())
    {
        ThrowTypeError(VM::Error(v.IsNull() ? VM::eConvertNullToObjectError : VM::eConvertUndefinedToObjectError, *this));
        result = false;
    }

    return result;
}

CheckResult VM::ConstructBuiltinValue(Value& v, const TypeInfo& ti, unsigned argc, const Value* argv)
{
    bool result = false;
    const ClassTraits::Traits* ctr = Resolve2ClassTraits(
        GetStringManager().CreateConstString(ti.GetName()),
        //GetPublicNamespace()
        GetInternedNamespace(Abc::NS_Public, ti.GetPkgName()), 
        GetCurrentAppDomain()
        );

    if (ctr)
    {
        InstanceTraits::Traits& itr = ctr->GetInstanceTraits();

        itr.MakeObject(v, itr);
        v.GetObject()->AS3Constructor(argc, argv);
        result = true;
    }

    return result;
}

void VM::RegisterClassInfoTable(const ClassInfo* table[])
{
    GetGlobalObjectCPP().RegisterClassInfoTable(table);
}

void VM::GetStackTraceASString(ASString& result, const char* line_pref) const
{
#ifdef GFX_AS3_VERBOSE
    const CallStackType& cs = GetCallStack();

    for (UPInt i = cs.GetSize(), j = 0; i > 0; --i, ++j)
    {
#ifdef SF_AS3_ENABLE_CALLFRAME_CACHE
        const CallFrame& cf = *cs[i - 1];
#else
        const CallFrame& cf = cs[i - 1];
#endif

        if (j > 0)
            result += "\n";

        result += line_pref;
        result += "at ";
        result += ASString(cf.GetName());
        result += "()";
        if (cf.GetCurrFileInd() != 0)
        {
            result += "[";
            result += cf.GetCurrFileName();
            result += ":";
            result += Scaleform::AsString(cf.GetCurrLineNumber());
            result += "]";
        }
    }
#else
    SF_UNUSED(line_pref);
    result = GetStringManager().GetBuiltin(AS3Builtin_null);
#endif
}

void VM::GetCodeFileName(ASString& result) const
{
    const CallFrame& cf = GetCurrCallFrame();
    const Abc::File& file = cf.GetFile().GetAbcFile();
    result = file.GetSource();
}

#ifdef GFX_AS3_VERBOSE
static
ASString GetThunkName(ASString base, const ThunkInfo& thunk, bool isClosure)
{
    if (!base.IsEmpty())
    {
        if (!base.IsEmpty())
            base += "/";

        if (isClosure && thunk.NamespaceName && *thunk.NamespaceName != 0)
        {
            if (strcmp(thunk.NamespaceName, NS_Vector) != 0)
            {
                if (strcmp(thunk.NamespaceName, NS_AS3) == 0)
                    base += "AS3";
                else if (strcmp(thunk.NamespaceName, NS_flash_proxy) == 0)
                    base += "flash_proxy";
                else
                    base += thunk.NamespaceName;

                base += "::";
            }
        }
    }

    if (isClosure)
        base = "MethodClosure " + base;
    else
        base = "Function " + base;

    base += thunk.Name;
    base += "()";

    return base;
}

static
ASString GetVTableIndName(ASString base, const VTable& vt, SInt32 ind, bool isClosure)
{
    const Value& real_func = vt.GetRaw(AbsoluteIndex(ind));

    // VTable stores MethodInd or Thunk.
    if (real_func.GetKind() == Value::kMethodInd)
    {
        // MethodInd.

        if (isClosure)
            base = "MethodClosure ";
        else
            base = "Function ";

        //return vt.GetTraits().GetQualifiedName() + "::" + vt.GetName(AbsoluteIndex(ind));
        return base + vt.GetName(AbsoluteIndex(ind)) + "()";
    }

    // Thunk.
    return GetThunkName(base, real_func.AsThunk(), isClosure);
}

static
ASString GetNameSkipVectorNS(const Traits& tr)
{
    const InstanceTraits::Traits* itr = NULL;

    if (tr.IsClassTraits())
        itr = &static_cast<const ClassTraits::Traits&>(tr).GetInstanceTraits();
    else
        itr = &static_cast<const InstanceTraits::Traits&>(tr);

    SF_ASSERT(itr);
    if (itr->GetNamespace().GetUri() == NS_Vector)
        return tr.GetName();

    return tr.GetQualifiedName();
}

#endif

ASString VM::describeTypeEx(const Value& value) const
{
    SF_UNUSED1(value);
    StringManager& sm = GetStringManager();
    
#ifdef GFX_AS3_VERBOSE
    switch(value.GetKind())
    {
    case Value::kUndefined:
        return sm.GetBuiltin(AS3Builtin_undefined);
    case Value::kBoolean:
        return sm.GetBuiltin(AS3Builtin_boolean);
    case Value::kInt:
        return sm.CreateConstString("int");
    case Value::kUInt:
        return sm.CreateConstString("uint");
    case Value::kNumber:
        return sm.GetBuiltin(AS3Builtin_number);
    case Value::kString:
        if (value.GetStringNode())
            return sm.GetBuiltin(AS3Builtin_string);
        else
            return sm.GetBuiltin(AS3Builtin_null);
    case Value::kThunk:
        return GetThunkName(sm.GetBuiltin(AS3Builtin_empty_), value.AsThunk(), false);
    case Value::kThunkFunction:
        {
            const Instances::ThunkFunction& tf = value.AsThunkFunction();
            const Traits& otr = tf.GetOriginationTraits();

            return GetThunkName(GetNameSkipVectorNS(otr), tf.GetThunk(), false);
        }
    case Value::kVTableInd:
        return GetVTableIndName(
            sm.GetBuiltin(AS3Builtin_empty_),
            value.GetTraits().GetVT(),
            value.GetVTableInd(),
            false
            );
        break;
    case Value::kVTableIndClosure:
        return GetVTableIndName(
            GetNameSkipVectorNS(value.GetClosure()->GetTraits()),
            value.GetClosure()->GetVT(),
            value.GetVTableInd(),
            true
            );
        break;
    case Value::kFunction:
        {
            const Instances::Function& f = value.AsFunction();
            const Traits& otr = f.GetOriginationTraits();
            
            return GetNameSkipVectorNS(otr) + "::" + f.GetName();
        }
    case Value::kObject:
        if (value.GetObject())
            return GetNameSkipVectorNS(value.GetObject()->GetTraits());
        else
            return sm.GetBuiltin(AS3Builtin_null);
        break; // Just in case.
    case Value::kClass:
        if (value.GetObject())
            return GetNameSkipVectorNS(value.GetObject()->GetTraits()).AppendChar('$');
        else
            return sm.GetBuiltin(AS3Builtin_null);
    case Value::kNamespace:
        {
            const Instances::fl::Namespace& ns = value.AsNamespace();
            ASString result = sm.CreateConstString(AS3::AsString(ns.GetKind()));

            if (!ns.GetUri().IsEmpty())
            {
                result += " ";
                result += ns.GetUri();
            }
            return result;
        }
    case Value::kThunkClosure:
        return GetThunkName(
            GetNameSkipVectorNS(value.GetClosure()->GetTraits()), 
            value.AsThunk(),
            true
            );
    default:
        SF_ASSERT(false);
        break;
    }
#endif

    return sm.GetBuiltin(AS3Builtin_empty_);
}

#ifdef SF_AMP_SERVER
void VM::SetActiveLine(UInt32 lineNumber)
{
    AMP::ViewStats* stats = GetAdvanceStats();
    if (stats != NULL)
    {
        UInt32 activeLine = stats->GetActiveLine();
        if (lineNumber != activeLine)
        {
            // Record time
            UInt64 newTimestamp = Timer::GetRawTicks();
            stats->RecordSourceLineTime(newTimestamp - ActiveLineTimestamp);
            ActiveLineTimestamp = newTimestamp;

            stats->SetActiveLine(lineNumber);
        }    
    }
}

void VM::SetActiveFile(UInt64 fileId)
{
    AMP::ViewStats* stats = GetAdvanceStats();
    if (stats != NULL)
    {
        UInt64 activeFile = stats->GetActiveFile();
        if (fileId != activeFile)
        {
            // Record time
            UInt64 newTimestamp = Timer::GetRawTicks();
            stats->RecordSourceLineTime(newTimestamp - ActiveLineTimestamp);
            ActiveLineTimestamp = newTimestamp;

            stats->SetActiveFile(fileId);
        }    
    }
}
#endif


Classes::fl::Boolean& VM::GetClassBoolean() const
{
    SF_ASSERT(TraitsBoolean.GetPtr());
    return (Classes::fl::Boolean&)TraitsBoolean->GetInstanceTraits().GetClass();
}

InstanceTraits::Traits& VM::GetITraitsBoolean() const
{
    return GetClassTraitsBoolean().GetInstanceTraits();
}

Classes::fl::Number& VM::GetClassNumber() const
{
    SF_ASSERT(TraitsNumber.GetPtr());
    return (Classes::fl::Number&)TraitsNumber->GetInstanceTraits().GetClass();
}

InstanceTraits::Traits& VM::GetITraitsNumber() const
{
    return GetClassTraitsNumber().GetInstanceTraits();
}

Classes::fl::int_& VM::GetClassSInt() const
{
    return (Classes::fl::int_&)GetClassTraitsSInt().GetInstanceTraits().GetClass();
}

InstanceTraits::Traits& VM::GetITraitsSInt() const
{
    return GetClassTraitsSInt().GetInstanceTraits();
}

Classes::fl::uint& VM::GetClassUInt() const
{
    return (Classes::fl::uint&)GetClassTraitsUInt().GetInstanceTraits().GetClass();
}

InstanceTraits::Traits& VM::GetITraitsUInt() const
{
    return GetClassTraitsUInt().GetInstanceTraits();
}

Classes::fl::String& VM::GetClassString() const
{
    return (Classes::fl::String&)GetClassTraitsString().GetInstanceTraits().GetClass();
}

InstanceTraits::Traits& VM::GetITraitsString() const
{
    return GetClassTraitsString().GetInstanceTraits();
}

Classes::fl::Array& VM::GetClassArray() const
{
    return (Classes::fl::Array&)GetClassTraitsArray().GetInstanceTraits().GetClass();
}

InstanceTraits::Traits& VM::GetITraitsArray() const
{
    return GetClassTraitsArray().GetInstanceTraits();
}

Classes::fl_utils::ByteArray& VM::GetClassByteArray() const
{
    return (Classes::fl_utils::ByteArray&)GetClassTraitsByteArray().GetInstanceTraits().GetClass();
}

InstanceTraits::Traits& VM::GetITraitsByteArray() const
{
    return GetClassTraitsByteArray().GetInstanceTraits();
}

Classes::fl::QName& VM::GetClassQName() const
{
    SF_ASSERT(TraitsQName.GetPtr());
    return (Classes::fl::QName&)TraitsQName->GetInstanceTraits().GetClass();
}

Classes::fl_vec::Vector& VM::GetClassVector() const
{
    return (Classes::fl_vec::Vector&)GetClassTraitsVector().GetInstanceTraits().GetClass();
}

Classes::fl_vec::Vector_int& VM::GetClassVectorSInt() const
{
    return (Classes::fl_vec::Vector_int&)GetClassTraitsVectorSInt().GetInstanceTraits().GetClass();
}

InstanceTraits::Traits& VM::GetITraitsVectorSInt() const
{
    return GetClassTraitsVectorSInt().GetInstanceTraits();
}

Classes::fl_vec::Vector_uint& VM::GetClassVectorUInt() const
{
    return (Classes::fl_vec::Vector_uint&)GetClassTraitsVectorUInt().GetInstanceTraits().GetClass();
}

InstanceTraits::Traits& VM::GetITraitsVectorUInt() const
{
    return GetClassTraitsVectorUInt().GetInstanceTraits();
}

Classes::fl_vec::Vector_double& VM::GetClassVectorNumber() const
{
    return (Classes::fl_vec::Vector_double&)GetClassTraitsVectorNumber().GetInstanceTraits().GetClass();
}

InstanceTraits::Traits& VM::GetITraitsVectorNumber() const
{
    return GetClassTraitsVectorNumber().GetInstanceTraits();
}

Classes::fl_vec::Vector_String& VM::GetClassVectorString() const
{
    return (Classes::fl_vec::Vector_String&)GetClassTraitsVectorString().GetInstanceTraits().GetClass();
}

InstanceTraits::Traits& VM::GetITraitsVectorString() const
{
    return GetClassTraitsVectorString().GetInstanceTraits();
}

InstanceTraits::Traits& VM::GetITraitsApplicationDomain() const
{
    return GetClassApplicationDomain().GetInstanceTraits();
}

InstanceTraits::Traits& VM::GetITraitsDomain() const
{
    return GetClassDomain().GetInstanceTraits();
}

void VM::ThrowErrorInternal(const Error& e, const TypeInfo& ti)
{
    const ClassTraits::Traits* ctr = Resolve2ClassTraits(
        GetStringManager().CreateConstString(ti.GetName()),
        //GetPublicNamespace()
        GetInternedNamespace(Abc::NS_Public, ti.GetPkgName()), 
        GetCurrentAppDomain()
        );

    SF_ASSERT(ctr);

    Value obj;
    InstanceTraits::Traits& itr = ctr->GetInstanceTraits();
    itr.MakeObject(obj, itr);
    const Value argv[2] = {Value(e.GetMessage()), Value((SInt32)e.GetErrorID())};
    obj.GetObject()->AS3Constructor(2, argv);
    Throw(obj);
}

void VM::ThrowError(const Error& e)
{
    ThrowErrorInternal(e, fl::ErrorTI);
}

void VM::ThrowEvalError(const Error& e)
{
    ThrowErrorInternal(e, fl::EvalErrorTI);
}

void VM::ThrowRangeError(const Error& e)
{
    ThrowErrorInternal(e, fl::RangeErrorTI);
}

void VM::ThrowReferenceError(const Error& e)
{
    ThrowErrorInternal(e, fl::ReferenceErrorTI);
}

void VM::ThrowSecurityError(const Error& e)
{
    ThrowErrorInternal(e, fl::SecurityErrorTI);
}

void VM::ThrowSyntaxError(const Error& e)
{
    ThrowErrorInternal(e, fl::SyntaxErrorTI);
}

void VM::ThrowTypeError(const Error& e)
{
    ThrowErrorInternal(e, fl::TypeErrorTI);
}

void VM::ThrowURIError(const Error& e)
{
    ThrowErrorInternal(e, fl::URIErrorTI);
}

void VM::ThrowVerifyError(const Error& e)
{
    ThrowErrorInternal(e, fl::VerifyErrorTI);
}

void VM::ThrowUninitializedError(const Error& e)
{
    ThrowErrorInternal(e, fl::UninitializedErrorTI);
}

void VM::ThrowArgumentError(const Error& e)
{
    ThrowErrorInternal(e, fl::ArgumentErrorTI);
}

void VM::ThrowMemoryError(const Error& e)
{
    ThrowErrorInternal(e, fl_errors::MemoryErrorTI);
}

///////////////////////////////////////////////////////////////////////////
#ifdef GFX_AS3_VERBOSE
VM::Error::Error(VM::ErrorID id, VM& vm)
: ID(id), Message(vm.GetStringManager().CreateEmptyString())
{
    String fullMsg;
    const int ind = (int)id;

    const char* builtin_msg = GetErrorMsg(ind);
    Scaleform::Format(fullMsg, "Error #{0}: {1}", ind, builtin_msg);

    Message = vm.GetStringManager().CreateString(fullMsg);
}

VM::Error::Error(VM::ErrorID id, VM& vm, int arg1)
: ID(id), Message(vm.GetStringManager().CreateEmptyString())
{
    String fullMsg;
    String tmpMsg;
    const int ind = (int)id;

    const char* builtin_msg = GetErrorMsg(ind);

    Format(tmpMsg, builtin_msg, arg1);
    Format(fullMsg, "Error #{0}: {1}", ind, tmpMsg);

    Message = vm.GetStringManager().CreateString(fullMsg);
}

VM::Error::Error(VM::ErrorID id, VM& vm, const StringDataPtr arg1)
: ID(id), Message(vm.GetStringManager().CreateEmptyString())
{
    String fullMsg;
    String tmpMsg;
    const int ind = (int)id;

    const char* builtin_msg = GetErrorMsg(ind);

    Format(tmpMsg, builtin_msg, arg1);
    Format(fullMsg, "Error #{0}: {1}", ind, tmpMsg);

    Message = vm.GetStringManager().CreateString(fullMsg);
}

VM::Error::Error(VM::ErrorID id, VM& vm, const Value& arg1)
: ID(id), Message(vm.GetStringManager().CreateEmptyString())
{
    String fullMsg;
    String tmpMsg;
    const int ind = (int)id;
    StringManager& sm = vm.GetStringManager();

    ASString s1 = sm.CreateEmptyString();
    arg1.Convert2String(s1).DoNotCheck();

    const char* builtin_msg = GetErrorMsg(ind);

    Format(tmpMsg, builtin_msg, s1.ToCStr());
    Format(fullMsg, "Error #{0}: {1}", ind, tmpMsg);

    Message = vm.GetStringManager().CreateString(fullMsg);
}

VM::Error::Error(VM::ErrorID id, VM& vm, int arg1, int arg2)
: ID(id), Message(vm.GetStringManager().CreateEmptyString())
{
    String fullMsg;
    String tmpMsg;
    const int ind = (int)id;

    const char* builtin_msg = GetErrorMsg(ind);

    Format(tmpMsg, builtin_msg, arg1, arg2);
    Format(fullMsg, "Error #{0}: {1}", ind, tmpMsg);

    Message = vm.GetStringManager().CreateString(fullMsg);
}

VM::Error::Error(VM::ErrorID id, VM& vm, const StringDataPtr arg1, const StringDataPtr arg2)
: ID(id), Message(vm.GetStringManager().CreateEmptyString())
{
    String fullMsg;
    String tmpMsg;
    const int ind = (int)id;

    const char* builtin_msg = GetErrorMsg(ind);

    Format(tmpMsg, builtin_msg, arg1, arg2);
    Format(fullMsg, "Error #{0}: {1}", ind, tmpMsg);

    Message = vm.GetStringManager().CreateString(fullMsg);
}

VM::Error::Error(VM::ErrorID id, VM& vm, const Value& arg1, const Value& arg2)
: ID(id), Message(vm.GetStringManager().CreateEmptyString())
{
    String fullMsg;
    String tmpMsg;
    const int ind = (int)id;

    StringManager& sm = vm.GetStringManager();

    ASString s1 = sm.CreateEmptyString();
    arg1.Convert2String(s1).DoNotCheck();

    ASString s2 = sm.CreateEmptyString();
    arg2.Convert2String(s2).DoNotCheck();

    const char* builtin_msg = GetErrorMsg(ind);

    Format(tmpMsg, builtin_msg, s1.ToCStr(), s2.ToCStr());
    Format(fullMsg, "Error #{0}: {1}", ind, tmpMsg);

    Message = vm.GetStringManager().CreateString(fullMsg);
}

VM::Error::Error(VM::ErrorID id, VM& vm, const Value& arg1, const StringDataPtr arg2)
: ID(id), Message(vm.GetStringManager().CreateEmptyString())
{
    String fullMsg;
    String tmpMsg;
    const int ind = (int)id;
    StringManager& sm = vm.GetStringManager();

    ASString s1 = sm.CreateEmptyString();
    arg1.Convert2String(s1).DoNotCheck();

    const char* builtin_msg = GetErrorMsg(ind);

    Format(tmpMsg, builtin_msg, s1.ToCStr(), arg2);
    Format(fullMsg, "Error #{0}: {1}", ind, tmpMsg);

    Message = vm.GetStringManager().CreateString(fullMsg);
}

VM::Error::Error(VM::ErrorID id, VM& vm, const StringDataPtr arg1, const Value& arg2)
: ID(id), Message(vm.GetStringManager().CreateEmptyString())
{
    String fullMsg;
    String tmpMsg;
    const int ind = (int)id;
    StringManager& sm = vm.GetStringManager();

    ASString s2 = sm.CreateEmptyString();
    arg2.Convert2String(s2).DoNotCheck();

    const char* builtin_msg = GetErrorMsg(ind);

    Format(tmpMsg, builtin_msg, arg1, s2.ToCStr());
    Format(fullMsg, "Error #{0}: {1}", ind, tmpMsg);

    Message = vm.GetStringManager().CreateString(fullMsg);
}

VM::Error::Error(VM::ErrorID id, VM& vm, const StringDataPtr arg1, int arg2, int arg3, int arg4)
: ID(id), Message(vm.GetStringManager().CreateEmptyString())
{
    String fullMsg;
    String tmpMsg;
    const int ind = (int)id;

    const char* builtin_msg = GetErrorMsg(ind);

    Format(tmpMsg, builtin_msg, arg1, arg2, arg3, arg4);
    Format(fullMsg, "Error #{0}: {1}", ind, tmpMsg);

    Message = vm.GetStringManager().CreateString(fullMsg);
}

#else

VM::Error::Error(VM::ErrorID id, VM& vm)
: ID(id), Message(vm.GetStringManager().CreateEmptyString())
{
    String fullMsg;
    const int ind = (int)id;

    Scaleform::Format(fullMsg, "Error #{0}", ind);

    Message = vm.GetStringManager().CreateString(fullMsg);
}

#endif

VM::ResourceGuard::ResourceGuard(VM& vm, UInt16 osn, UInt16 rn)
: RegNum(rn)
, VMRef(vm)
{
    VM::OpStackType& stack = VMRef.OpStack;
    RegistersType& regs = VMRef.GetRegisters();
    ScopeStackType& ss = VMRef.GetScopeStack();

    SF_DEBUG_CODE(PrevReservedNum = stack.GetNumOfReservedElem();)
        PrevFirstStackPos = stack.GetFirst();
    stack.Reserve(osn);

    regs.Reserve(RegNum);

    ScopeStackBaseInd = ss.GetSize();

    // Store current Default XML Namespace.
    DefXMLNamespace = vm.GetDefXMLNamespace();
    vm.SetDefXMLNamespace(NULL);
}

VM::ResourceGuard::~ResourceGuard()
{
    VM::OpStackType& stack = VMRef.OpStack;
    RegistersType& regs = VMRef.GetRegisters();
    ScopeStackType& ss = VMRef.GetScopeStack();

    stack.ReleaseReserved(PrevFirstStackPos SF_DEBUG_ARG(PrevReservedNum));

    regs.ReleaseReserved(RegNum);

    ss.Resize(ScopeStackBaseInd);

    // Restore stored Default XML Namespace.
    VMRef.SetDefXMLNamespace(DefXMLNamespace);
}

///////////////////////////////////////////////////////////////////////////
void ThunkInfo::EmptyFunc(const ThunkInfo& ti, VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv)
{
#if 0
    SF_UNUSED5(ti, _this, result, argc, argv);
    WARN_NOT_IMPLEMENTED_STATIC("ThunkInfo::EmptyFunc()");
#else
    SF_UNUSED4(_this, result, argc, argv);
    String str("The method ");
    if (ti.NamespaceName)
    {
        str += ti.NamespaceName;
        str += "::";
    }
    if (ti.Name)
    {
        str += ti.Name;
        str += "()";
    }
    str += " is not implemented\n";

    vm.GetUI().Output(FlashUI::Output_Warning, str.ToCStr());
#endif
}

///////////////////////////////////////////////////////////////////////////
// We keep implementation here because of Array.
CheckResult SetProperty(VM& vm, const Value& _this, const Multiname& prop_name, const Value& value)
{
    /* For maniacs
    if (_this.IsNullOrUndefined())
    {
        vm.ThrowTypeError(VM::eConvertUndefinedToObjectError);
        return false;
    }
    */

    if (_this.IsObject() && _this.GetObject())
        return _this.GetObject()->SetProperty(prop_name, value);

    // Find a property.
    // Find + Set shouldn't mutate a prototype object.
    // Search in own properties only. Ignore prototype.
    PropRef prop;
    AS3::FindObjProperty(prop, vm, _this, prop_name, FindSet);

    if (prop)
        return prop.SetSlotValue(vm, value);

    // A ReferenceError is thrown if the property is unresolved and obj is not dynamic.
    vm.ThrowReferenceError(VM::Error(VM::eWriteSealedError, vm
        SF_DEBUG_ARG(prop_name.GetName())
        SF_DEBUG_ARG(vm.GetValueTraits(_this).GetName().ToCStr())
        ));

    return false;
}

CheckResult GetPropertyUnsafe(VM& vm, const Value& _this, const Multiname& prop_name, Value& value)
{
    /* For maniacs
    if (_this.IsNullOrUndefined())
    {
        vm.ThrowTypeError(VM::eConvertUndefinedToObjectError);
        return false;
    }
    */

    if (_this.IsObject() && _this.GetObject())
        return _this.GetObject()->GetProperty(prop_name, value);

    // Find a property.
    PropRef prop;

    FindObjProperty(prop, vm, _this, prop_name);

    if (prop)
        return prop.GetSlotValueUnsafe(vm, value);

    return false;
}

///////////////////////////////////////////////////////////////////////////
// Keep it here for the time being.

CheckResult Object::SetProperty(const Multiname& prop_name, const Value& value)
{
    // Find a property.

    VM& vm = GetVM();

    // Find + Set shouldn't mutate a prototype object.
    PropRef prop;

    AS3::FindObjProperty(prop, GetVM(), Value(this), prop_name, FindSet);

    if (prop)
        return prop.SetSlotValue(vm, value);
    else
    {
        Object& obj = *this;
        
        // Dynamic properties can only be assigned to public namespace.
        // TBD: Add Multiname::AsValidDynamicName()?. Empty names shouldn't be allowed...
        if (obj.IsDynamic() &&
            prop_name.ContainsNamespace(vm.GetPublicNamespace()))
        {
            obj.AddDynamicSlotValuePair(prop_name.GetName(), value);
            return true;
        }
    }

    // A ReferenceError is thrown if the property is unresolved and obj is not dynamic.
    vm.ThrowReferenceError(VM::Error(VM::eWriteSealedError, vm
        SF_DEBUG_ARG(prop_name.GetName())
        SF_DEBUG_ARG(GetTraits().GetName().ToCStr())
        ));

    return false;
}

CheckResult Object::GetProperty(const Multiname& prop_name, Value& value)
{
    VM& vm = GetVM();

    // Find a property.
    PropRef prop;

    AS3::FindObjProperty(prop, vm, Value(this), prop_name);

    if (prop)
    {
        Value v;

        if (prop.GetSlotValueUnsafe(vm, v))
        {
            value.Swap(v);
            return true;
        }
    }

    return false;
}

void Object::GetDynamicProperty(AbsoluteIndex ind, Value& value)
{
    const DynAttrsType::ConstIterator bit = DynAttrs.Begin();
    DynAttrsType::ConstIterator it(bit.GetContainer(), ind .Get());

    value.Assign(it->Second);
}

CheckResult Object::DeleteProperty(const Multiname& prop_name)
{
    if (IsDynamic())
    {
        return DeleteDynamicSlotValuePair(prop_name);
    }

    return false;
}


#if defined(SF_FPE_ENABLE)
FPEGuard::FPEGuard()
{
#if defined(SF_OS_WIN32) || defined(SF_OS_XBOX360)
#if defined(SF_MSVC_SAFESTRING)
    // Enable hardware exceptions and store previous state.
    _controlfp_s(&State, _EM_INEXACT | _EM_UNDERFLOW | _EM_OVERFLOW | _EM_ZERODIVIDE | _EM_INVALID, _MCW_EM);
#else
    // Store previous state.
    State = _controlfp(0, 0);
    // Enable hardware exceptions.
    _controlfp(_EM_INEXACT | _EM_UNDERFLOW | _EM_OVERFLOW | _EM_ZERODIVIDE | _EM_INVALID, _MCW_EM);
#endif
#endif
}

FPEGuard::~FPEGuard()
{
#if defined(SF_OS_WIN32) || defined(SF_OS_XBOX360)
#if defined(SF_MSVC_SAFESTRING)
    unsigned c;

    // Restore previous state.
#if defined(SF_OS_XBOX360) || defined(_DURANGO)
    _controlfp_s(&c, State, _MCW_DN | _MCW_EM | _MCW_RC);
#else
    _controlfp_s(&c, State, _MCW_DN | _MCW_EM | _MCW_IC | _MCW_RC | _MCW_PC);
#endif

#else
    // Restore previous state.
    _controlfp(State, _MCW_DN | _MCW_EM | _MCW_IC | _MCW_RC | _MCW_PC);
#endif
#endif
}
#endif

#if defined(SF_AS3_AOTC) || defined(SF_AS3_AOTC2)
namespace AOT
{
    void InfoCollector::MakeFunctionName(StringBuffer& buf, const InstanceTraits::Function& tr) const
    {
        const String* fun_name = FunctNameHash.Get(UMbiInd(tr.GetFile().GetAbcFile(), tr.GetMethodBodyInfoInd()));

        buf.AppendString("Function_");
        buf.AppendString(fun_name ? *fun_name : Scaleform::AsString(tr.GetMethodInfoInd().Get()));
    }
}
#endif

namespace Impl
{
    void CoerceInternal(VM& vm, const TypeInfo& ti, Value& to, const Value& from)
    {
        const ClassTraits::Traits* ctr = vm.Resolve2ClassTraits(ti);

        if (ctr && ctr->Coerce(from, to))
            return;

        to = from;
    }
}

}}} // namespace Scaleform { namespace GFx { namespace AS3 {

