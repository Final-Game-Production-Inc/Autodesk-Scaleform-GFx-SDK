/**************************************************************************

Filename    :   AS3_AsString.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "AS3_AsString.h"
#include "Obj/AS3_Obj_Namespace.h"
#include "Obj/AS3_Obj_Global.h"
#include "AS3_Tracer.h"

// #ifdef SF_AS3_AOTC
//     #include "../../../Apps/Tools/as3/as3aotc/aotc_impl.h"
// #endif

#define SF_AS3_OLD_GETSLOTNAME

namespace Scaleform
{

///////////////////////////////////////////////////////////////////////////
// Specialization for formatting of ASString.

class ASStrFormatter : public StrFormatter
{
public:
    ASStrFormatter(MsgFormat& f, const GFx::ASStringNode* v)
        : StrFormatter(f, StringDataPtr(v->pData, v->Size))
    {
    }
    ASStrFormatter(MsgFormat& f, const GFx::ASString& v)
        : StrFormatter(f, StringDataPtr(v.ToCStr(), v.GetSize()))
    {
    }
};

template <>
struct FmtInfo<const GFx::ASStringNode*>
{
    typedef ASStrFormatter formatter;
};

template <>
struct FmtInfo<GFx::ASStringNode*>
{
    typedef ASStrFormatter formatter;
};

template <>
struct FmtInfo<const GFx::ASString>
{
    typedef ASStrFormatter formatter;
};

template <>
struct FmtInfo<GFx::ASString>
{
    typedef ASStrFormatter formatter;
};

}


namespace Scaleform { namespace GFx { namespace AS3 
{

///////////////////////////////////////////////////////////////////////////
const char* AsString(const Abc::NamespaceKind kind)
{
    using namespace Abc;

    switch (kind)
    {
    case NS_Public:
        return "public";
    case NS_Protected:
        return "protected";
    case NS_StaticProtected:
        return "static protected";
    case NS_Private:
        return "private";
    case NS_Explicit:
        return "explicit";
    case NS_PackageInternal:
        return "package internal";
    default:
        break;
    }

    SF_ASSERT(false);
    return "Invalid Namespace type";
}

///////////////////////////////////////////////////////////////////////////
ASString AsString(const Value& value, StringManager& sm)
{
    switch(value.GetKind())
    {
//     case Value::kUndefined:
//         return sm.CreateConstString("undefined");
//     case Value::kNull:
//         return sm.CreateConstString("null");
//     case Value::kBoolean:
//         return sm.CreateConstString(value.AsBool() ? "true" : "false");
//     case Value::kInt:
//         return sm.CreateString(Scaleform::AsString(value.AsInt()));
//     case Value::kUInt:
//         return sm.CreateString(Scaleform::AsString(value.AsUInt()));
//     case Value::kNumber:
//         return sm.CreateString(Scaleform::AsString(value.AsNumber()));
//     case Value::kString:
//         return value.AsString();

    case Value::kUndefined:
    case Value::kBoolean:
    case Value::kInt:
    case Value::kUInt:
    case Value::kNumber:
    case Value::kString:
        {
            ASString r = sm.CreateEmptyString();
            value.Convert2String(r).DoNotCheck();
            return r;
        }
    case Value::kThunk:
        return sm.CreateConstString("thunk");
    case Value::kThunkFunction:
        return sm.CreateConstString("thunk function");
    case Value::kVTableInd:
        return sm.CreateString("VTable ind: " + Scaleform::AsString(value.GetVTableInd()));
    case Value::kVTableIndClosure:
        return sm.CreateConstString("VTableIndClosure");
        // !!! Code below is a hack ...
        // We should call commented out code instead.
    case Value::kFunction:
        return sm.CreateConstString("function Function() {}");
    case Value::kObject:
        if (value.GetObject())
            return sm.CreateConstString("[object ") + value.GetObject()->GetTraits().GetName() + "]";
        else
            return sm.GetBuiltin(AS3Builtin_null);
    case Value::kClass:
        if (value.GetObject())
            return sm.CreateConstString("[class ") + value.GetObject()->GetTraits().GetName() + "]";
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
// DO NOT delete this code.
//      case Value::kFunction:
//      case Value::kXML:
//      case Value::kObject:
//      case Value::kClass:
//          return AsString(value.AsObject()) + " (" + AsString(value.GetKind()) + ")";
    case Value::kThunkClosure:
        return sm.CreateConstString("ThunkClosure");
    default:
        SF_ASSERT(false);
        break;
    }

    SF_ASSERT(false);
    return sm.CreateEmptyString();
}

#if defined(GFX_AS3_VERBOSE) || defined(SF_AS3_AOTC)

///////////////////////////////////////////////////////////////////////////
String AsString(const Value& value)
{
    if (value.IsNull())
        return "null";

    Value::KindType kind = value.GetKind();
    switch(kind)
    {
    case Value::kUndefined:
        return "undefined";
    case Value::kBoolean:
        return value.AsBool() ? "true" : "false";
    case Value::kInt:
        return Scaleform::AsString(value.AsInt());
    case Value::kUInt:
        return Scaleform::AsString(value.AsUInt());
    case Value::kNumber:
        if (value.IsNaN())
            return "NaN";
        if (value.IsPOSITIVE_ZERO() || value.IsNEGATIVE_ZERO())
            return "0";
        if (value.IsPOSITIVE_INFINITY() || value.IsNEGATIVE_INFINITY())
            return "0";

        return Scaleform::AsString(value.AsNumber());
    case Value::kString:
        if (value.GetStringNode())
            return "\"" + String(value.AsString().ToCStr()) + "\"";
        else
            return "null";
    case Value::kThunk:
        return "thunk";
    case Value::kThunkFunction:
        return "thunk function";
    case Value::kVTableInd:
        return "VTable ind: " + Scaleform::AsString(value.GetVTableInd());
    case Value::kVTableIndClosure:
        return "VTableIndClosure";
        // !!! Code below is a hack ...
        // We should call commented out code instead.
    case Value::kFunction:
        return "function Function() {}";
    case Value::kObject:
        if (value.GetObject())
            return "[object " + value.GetObject()->GetTraits().GetName() + "]";
        else
            return "null";
    case Value::kClass:
        if (value.GetObject())
            return "[class " + value.GetObject()->GetTraits().GetName() + "]";
        else
            return "null";
    case Value::kNamespace:
        {
            const Instances::fl::Namespace& ns = value.AsNamespace();
            String result(AS3::AsString(ns.GetKind()));

            if (!ns.GetUri().IsEmpty())
            {
                result += " ";
                result += ns.GetUri().ToCStr();
            }
            return result;
        }
        // DO NOT delete this code.
        //      case Value::kFunction:
        //      case Value::kXML:
        //      case Value::kObject:
        //      case Value::kClass:
        //          return AsString(value.AsObject()) + " (" + AsString(value.GetKind()) + ")";
    case Value::kThunkClosure:
        return "ThunkClosure";
    case Value::kInstanceTraits:
    case Value::kClassTraits:
        if (value.GetTraceNullType() == Value::Null)
            return "null";
    default:
        SF_ASSERT(false);
        break;
    }

    SF_ASSERT(false);
    return String();
}

#endif

#if defined(GFX_AS3_VERBOSE)

///////////////////////////////////////////////////////////////////////////
String AsString(const ValueArray& values)
{
    String result;
    
    for(UPInt i = 0; i < values.GetSize(); ++i)
    {
        if (i > 0)
            result.AppendString(", ");
        
        result.AppendString(AS3::AsString(values[i]));
    }
    
    return result;
}

///////////////////////////////////////////////////////////////////////////
void Output(FlashUI& ui, SPInt v)
{
    LongFormatter f(v);

    f.Convert();
    ui.Output(FlashUI::Output_Action, f.ToCStr());
}

void Output(FlashUI& ui, UPInt v, bool as_hex)
{
    LongFormatter f(v);

    if (as_hex)
        f.SetBase(16);

    f.Convert();
    ui.Output(FlashUI::Output_Action, f.ToCStr());
}

void Output(FlashUI& ui, Value::Number v)
{
    DoubleFormatter f(v);

    f.Convert();
    ui.Output(FlashUI::Output_Action, f.ToCStr());
}

///////////////////////////////////////////////////////////////////////////
void Output(const VM& vm, FlashUI& ui, const ValueStack& values, UPInt from)
{
    SF_UNUSED1(from);
    const Value* first = values.GetFirst();
    const Value* last = values.GetCurrent();
    for (const Value* p = first; p <= last; ++p)
    {
        if (p > first)
            ui.Output(FlashUI::Output_Action, ", ");

        Output(vm, ui, *p);
    }
}

///////////////////////////////////////////////////////////////////////////
void Output(const VM& vm, FlashUI& ui, const ValueRegisterFile& values)
{
    const UInt16 size = values.GetSize();

    for(UInt16 i = 0; i < size; ++i)
    {
        if (i > 0)
            ui.Output(FlashUI::Output_Action, ", ");

        Output(vm, ui, values[i]);
    }
}

///////////////////////////////////////////////////////////////////////////
void Output(const VM& vm, FlashUI& ui, const ArrayDH<Value>& values, UPInt from)
{
    const UPInt size = values.GetSize();

    for(UPInt i = from; i < size; ++i)
    {
        if (i > from)
            ui.Output(FlashUI::Output_Action, ", ");

        Output(vm, ui, values[i]);
    }
}

///////////////////////////////////////////////////////////////////////////
void Output(const VM& vm, FlashUI& ui, const Value& value)
{
    switch(value.GetKind())
    {
    case Value::kUndefined:
        ui.Output(FlashUI::Output_Action, "undefined");
        break;
    case Value::kBoolean:
        ui.Output(FlashUI::Output_Action, value.AsBool() ? "true" : "false");
        break;
    case Value::kInt:
        Output(ui, static_cast<SPInt>(value.AsInt()));
        break;
    case Value::kUInt:
        Output(ui, static_cast<UPInt>(value.AsUInt()));
        break;
    case Value::kNumber:
        if (value.IsNaN())
        {
            ui.Output(FlashUI::Output_Action, "NaN");
            break;
        }

        if (value.IsPOSITIVE_ZERO())
        {
            ui.Output(FlashUI::Output_Action, "+0");
            break;
        }

        if (value.IsNEGATIVE_ZERO())
        {
            ui.Output(FlashUI::Output_Action, "-0");
            break;
        }

        if (value.IsPOSITIVE_INFINITY())
        {
            ui.Output(FlashUI::Output_Action, "+Infinity");
            break;
        }

        if (value.IsNEGATIVE_INFINITY())
        {
            ui.Output(FlashUI::Output_Action, "-Infinity");
            break;
        }

        Output(ui, value.AsNumber());
        break;
    case Value::kString:
        if (value.GetStringNode())
        {
            ui.Output(FlashUI::Output_Action, "\"");
            ui.Output(FlashUI::Output_Action, value.AsString().ToCStr());
            ui.Output(FlashUI::Output_Action, "\"");
        }
        else
            ui.Output(FlashUI::Output_Action, "null");
        break;
    case Value::kThunk:
        ui.Output(FlashUI::Output_Action, "thunk@");
        Output(ui, reinterpret_cast<UPInt>(&value.AsThunk()), true);
        break;
    case Value::kThunkFunction:
        if (value.IsNull())
            ui.Output(FlashUI::Output_Action, "null");
        else
        {
            ui.Output(FlashUI::Output_Action, "thunk function@");
            ui.Output(FlashUI::Output_Action, value.GetObject()->GetName().ToCStr());
            Output(ui, reinterpret_cast<UPInt>(&value.AsThunkFunction()), true);
        }
        break;
    case Value::kVTableInd:
        ui.Output(FlashUI::Output_Action, "VTable ind: ");
        Output(ui, static_cast<SPInt>(value.GetVTableInd()));
        break;
    case Value::kVTableIndClosure:
        if (value.GetClosure())
        {
            ui.Output(FlashUI::Output_Action, "VTableIndClosure@");
            ui.Output(FlashUI::Output_Action, value.GetClosure()->GetName().ToCStr());
            ui.Output(FlashUI::Output_Action, "@");
            Output(ui, UPInt(value.GetClosure()), true);
            ui.Output(FlashUI::Output_Action, "@");
            Output(ui, value.GetVTableInd(), true);
        }
        else
            ui.Output(FlashUI::Output_Action, "null");
        break;
        
        // !!! Code below is a hack ...
        // We should call commented out code instead.
    case Value::kFunction:
        if (value.IsNull())
            ui.Output(FlashUI::Output_Action, "null");
        else
        {
            ui.Output(FlashUI::Output_Action, "function@");
            Output(ui, reinterpret_cast<UPInt>(&value.AsFunction()), true);
        }
        break;
    case Value::kObject:
        if (value.IsNull())
            ui.Output(FlashUI::Output_Action, "null");
        else
        {
            const Object* obj = value.GetObject();
            const Traits& tr = obj->GetTraits();

            if (tr.IsGlobal())
            {
                if (obj == &vm.GetGlobalObjectCPP())
                    ui.Output(FlashUI::Output_Action, "globalCPP");
                else
                {
                    // It is an Global Object Script.
                    const Instances::fl::GlobalObjectScript* script = static_cast<const Instances::fl::GlobalObjectScript*>(obj);
                    const Abc::ScriptInfo& si = script->GetScript();
                    const VMAbcFile& file = script->GetFile();
                    const StringDataPtr fn(file.GetAbcFile().GetSource());

                    ui.Output(FlashUI::Output_Action, "script_");
                    LongFormatter f(si.GetMethodInfoInd().Get());
                    f.Convert();
                    ui.Output(FlashUI::Output_Action, f.GetResult().ToCStr());
                    ui.Output(FlashUI::Output_Action, "@");

                    SPInt pos = fn.FindLastChar('\\');
                    if (pos < 0)
                        pos = fn.FindLastChar('/');

                    ++pos;
                    ui.Output(FlashUI::Output_Action, fn.GetTrimLeft(pos).ToCStr());
                }
            }
            else
            {
                ui.Output(FlashUI::Output_Action, obj->GetName().ToCStr());
                ui.Output(FlashUI::Output_Action, "@");
                Output(ui, reinterpret_cast<UPInt>(obj), true);
            }
        }
        break;
    case Value::kClass:
        if (value.GetObject())
        {
            ui.Output(FlashUI::Output_Action, value.GetObject()->GetName().ToCStr());
            ui.Output(FlashUI::Output_Action, "$$");
        }
        else
            ui.Output(FlashUI::Output_Action, "null");
        break;
    case Value::kNamespace:
        {
            const Instances::fl::Namespace& ns = value.AsNamespace();

            ui.Output(FlashUI::Output_Action, AS3::AsString(ns.GetKind()));
            if (!ns.GetUri().IsEmpty())
            {
                ui.Output(FlashUI::Output_Action, " ");
                ui.Output(FlashUI::Output_Action, ns.GetUri().ToCStr());
            }
        }
        break;
    // DO NOT delete this code.
    //      case Value::kFunction:
    //      case Value::kXML:
    //      case Value::kObject:
    //      case Value::kClass:
    //          return AsString(value.GetObject()) + " (" + AsString(value.GetKind()) + ")";
    case Value::kThunkClosure:
        if (value.GetClosure())
        {
            ui.Output(FlashUI::Output_Action, "ThunkClosure@");
            ui.Output(FlashUI::Output_Action, value.GetClosure()->GetName().ToCStr());
            ui.Output(FlashUI::Output_Action, "@");
            Output(ui, reinterpret_cast<UPInt>(value.GetClosure()), true);
            ui.Output(FlashUI::Output_Action, "@");
            Output(ui, reinterpret_cast<UPInt>(&value.AsThunk()), true);
        }
        else
            ui.Output(FlashUI::Output_Action, "null");
        break;
    case Value::kInstanceTraits:
        {
            InstanceTraits::Traits* itr = &value.GetInstanceTraits();

            if (itr == &vm.GetITraitsVoid())
                ui.Output(FlashUI::Output_Action, "void");
            else if (itr == &vm.GetITraitsNull())
                ui.Output(FlashUI::Output_Action, "null");
            else
            {
                switch (value.GetTraceNullType())
                {
                case Value::NotNull:
                    ui.Output(FlashUI::Output_Action, "nnull:");
                    break;
                case Value::Null:
                    ui.Output(FlashUI::Output_Action, "null:");
                    break;
                case Value::NullOrNot:
                    ui.Output(FlashUI::Output_Action, "nullon:");
                    break;
                }

                // This is trace-related info.
                ui.Output(FlashUI::Output_Action, value.GetInstanceTraits().GetName().ToCStr());
                ui.Output(FlashUI::Output_Action, "$");
            }
        }
        break;
    case Value::kClassTraits:
        {
            // This is trace-related info.
            ui.Output(FlashUI::Output_Action, value.GetClassTraits().GetName().ToCStr());
            ui.Output(FlashUI::Output_Action, "$$");
        }
        break;
    case Value::kMethodInd:
        break;
#if defined(SF_AS3_AOTC) // || defined(SF_AS3_AOTC2)
    case Value::kSNodeIT: // SNode + InstanceTraits
        {
            SNode& sn = value.GetSNode();
            AOT::TraceILVisitor2 v(vm);

            sn.Accept(v);
            ui.Output(FlashUI::Output_Action, v.GetResult().ToCStr());
            ui.Output(FlashUI::Output_Action, ":");
            Output(vm, ui, Value(value.GetInstanceTraits(), value.GetTraceNullType()));
        }
        break;
    case Value::kSNodeCT: // SNode + kClassTraits
        {
            SNode& sn = value.GetSNode();
            AOT::TraceILVisitor2 v(vm);

            sn.Accept(v);
            ui.Output(FlashUI::Output_Action, v.GetResult().ToCStr());
            ui.Output(FlashUI::Output_Action, ":");
            Output(vm, ui, Value(value.GetClassTraits(), value.GetTraceNullType()));
        }
        break;
#else
    default:
        break;
#endif
    }
}

///////////////////////////////////////////////////////////////////////////
#ifdef GFX_AS3_TRACE
void Output(FlashUI& ui, Abc::TCodeOffset offset, const CallFrame& cf)
{
    using namespace Abc;

//     const NamespaceDisplayType mdt = dComplete;
    const File& file = cf.GetFile().GetAbcFile();

    const TOpCode& oc = cf.GetCode();
    const TOpCode::ValueType* code = static_cast<const TOpCode::ValueType*>(oc.GetDataPtr());

    Output(cf.GetVM(), ui, file, code, offset, cf.GetName()->pData);
}
#endif

///////////////////////////////////////////////////////////////////////////
String AsString(const ValueArrayDH& values)
{
    String result;

    for(UPInt i = 0; i < values.GetSize(); ++i)
    {
        if (i > 0)
            result.AppendString(", ");

        result.AppendString(AS3::AsString(values[i]));
    }

    return result;
}

String AsString(const Instances::fl::Namespace& ns, NamespaceDisplayType ndt)
{
    ASString str = ns.GetUri();
    String Uri(str.ToCStr(), str.GetSize());

    using namespace Abc;

    switch(ndt)
    {
    case dComplete:
        return String(AS3::AsString(ns.GetKind())) + (Uri.IsEmpty() ? String() : String(" " + Uri));
    case dDeclarationOnly:
    case dShort:
        switch(ns.GetKind())
        {
        case NS_Public:
        case NS_Explicit:
            // User-defined namespace ...
            return Uri;
            //        case NamespaceInfo::NS_Package: // This is treated as NS_Public
            //            return empty_string;
        case NS_PackageInternal:
            return "internal";
        case NS_Protected:
            return "protected";
        case NS_StaticProtected:
            return "static protected";
        case NS_Private:
            return "private";
        default:
            SF_ASSERT(false);
            break;
        }

        return Uri;
    }

    return String();
}

String AsString(const NamespaceSet& nss)
{
    StringBuffer buf;
    const NamespaceSet::TContainer& n = nss.GetNamespaces();
    const UPInt size = n.GetSize();

    buf.AppendChar('[');

    for (UPInt i = 0; i < size; ++i)
        buf += AsString(*n[i], dShort);

    buf.AppendChar(']');

    return buf;
}

#endif

#ifdef GFX_AS3_VERBOSE

///////////////////////////////////////////////////////////////////////////
String AsString(const Abc::NamespaceInfo& info, NamespaceDisplayType ndt)
{
    using namespace Abc;

    static const String empty_string;

    switch(ndt)
    {
    case dComplete:
        {
            const StringDataPtr& uri = info.GetNameURI();
            return String(AS3::AsString(info.GetKind())) + (uri.IsEmpty() ? String() : " " + String(uri.ToCStr(), uri.GetSize()));
        }
    case dDeclarationOnly:
        switch(info.GetKind())
        {
        case NS_Public:
        case NS_Explicit:
            // User-defined namespace ...
            break;
//        case NamespaceInfo::NS_Package: // This is treated as NS_Public
//            return empty_string;
        case NS_PackageInternal:
            return "internal";
        case NS_Protected:
            return "protected";
        case NS_StaticProtected:
            return "static protected";
        case NS_Private:
            return "private";
        default:
            SF_ASSERT(false);
            break;
        }

        return String(info.GetNameURI().ToCStr(), info.GetNameURI().GetSize());
    case dShort:
        // ???
        if (info.HasKind())
        {
            if (info.IsUserDefined())
            {
                const StringDataPtr& uri = info.GetNameURI();
                return String(uri.ToCStr(), uri.GetSize());
            }
        }
        else
            return "noname";

        break;
    }

    return empty_string;
}

///////////////////////////////////////////////////////////////////////////
String AsString(const Abc::ConstPool& cp, const Abc::NamespaceSetInfo& info, NamespaceDisplayType ndt)
{
    String result("[");

    const UInt8* ptr = info.GetData();
    UInt32 size;
    UInt32 ns_ind;

    Abc::Read(ptr, size);
    for(UInt32 i = 0; i < size; ++i)
    {
        // Optimization for empty strings ...
        Abc::Read(ptr, ns_ind);

        const String tmp = AS3::AsString(cp.GetNamespace(ns_ind), ndt);

        if(i > 0 && !tmp.IsEmpty() && result.GetSize() > 1)
            result += ", ";

        if (!tmp.IsEmpty())
            result += tmp;
    }

    return result + "]";
}

///////////////////////////////////////////////////////////////////////////
String AsString(const Abc::ConstPool& cp, const Abc::Multiname& info)
{
    switch(info.GetKind())
    {
    case Abc::MN_QName:
    case Abc::MN_QNameA:
        {
            String result;

            result += AS3::AsString(info.GetNamespace(cp), dComplete);

            if (!result.IsEmpty())
                result += "::";

            const StringDataPtr& name = info.GetName(cp);
            result.AppendString(name.ToCStr(), name.GetSize());

            return result;
        }
        break;
    case Abc::MN_RTQName:
    case Abc::MN_RTQNameA:
        {
            const StringDataPtr& name = info.GetName(cp);
            return String(name.ToCStr(), name.GetSize());
        }
    case Abc::MN_RTQNameL:
    case Abc::MN_RTQNameLA:
        return "";
    case Abc::MN_Multiname:
    case Abc::MN_MultinameA:
        {
            String result;

            result = AS3::AsString(cp, info.GetNamespaceSetInfo(cp));

            if (!result.IsEmpty())
                result += "::";

            const StringDataPtr& name = info.GetName(cp);
            result.AppendString(name.ToCStr(), name.GetSize());

            return result;
        }
        break;
    case Abc::MN_MultinameL:
    case Abc::MN_MultinameLA:
        return AS3::AsString(cp, info.GetNamespaceSetInfo(cp));
    default:
	break;
    }

    SF_ASSERT(false);

    // To avoid compiler warnings ...
    return String();
}

///////////////////////////////////////////////////////////////////////////
String AsString(const Abc::ConstPool& cp, const Abc::ValueDetail& detail)
{
    using namespace Abc;

    const int vindex = detail.GetIndex();

    switch(detail.GetKind())
    {
    case CONSTANT_Int:
        return Scaleform::AsString(cp.GetInt(vindex));
    case CONSTANT_UInt:
        return Scaleform::AsString((unsigned int)cp.GetUInt(vindex));
    case CONSTANT_Double:
        return Scaleform::AsString(cp.GetDouble(vindex));
    case CONSTANT_Utf8:
        {
            const StringDataPtr& str = cp.GetString(AbsoluteIndex(vindex));
            return String(str.ToCStr(), str.GetSize());
        }
    case CONSTANT_True:
        return "true";
    case CONSTANT_False:
        return "false";
    case CONSTANT_Null:
        return "null";
    case CONSTANT_Undefined:
        return "Undefined";
    case CONSTANT_Namespace:
    case CONSTANT_PackageNamespace:
    case CONSTANT_PackageInternalNs:
    case CONSTANT_ProtectedNamespace:
    case CONSTANT_ExplicitNamespace:
    case CONSTANT_StaticProtectedNs:
    case CONSTANT_PrivateNs:
        return AS3::AsString(cp.GetNamespace(vindex));
    }

    return String();
}

///////////////////////////////////////////////////////////////////////////
String AsString(const Abc::ConstPool& cp, const Abc::MethodInfo& info)
{
    String result;

    //result += info.GetName();
    result += "(";

    for(UPInt i = 0; i < info.GetParamCount(); ++i)
    {
        if(i > 0)
            result += ", ";

        if(info.HasParamNames())
        {
#ifndef SF_AS3_NO_PARAMNAMES
            const StringDataPtr pn = info.GetParamName(cp, i);
            result.AppendString(pn.ToCStr(), pn.GetSize());
            result += ":";
#endif
        }

        result += AS3::AsString(cp, info.GetParamType(cp, i));

        if(info.HasOptionalParams() && i >= (info.GetParamCount() - info.GetOptionalParamCount()))
            result += " = " + AS3::AsString(cp, info.GetOptionalParam(i - (info.GetParamCount() - info.GetOptionalParamCount())));
    }

    result += ") : ";
    result += AS3::AsString(cp, info.GetReturnType(cp));

    return result;
}

///////////////////////////////////////////////////////////////////////////
String AsStringFlag(const Abc::Instance& info)
{
    String result;

    if (!info.IsSealed())
        result += "dynamic ";

    if (info.IsFinal())
        result += "final ";

    if (info.IsInterface())
        result += "interface ";
    else
        result += "class ";

    // It is almost always presented ...		
#if 0
    if (info.HasProtectedNamespace())
        result += "(has protected namespace) ";
#endif

    return result;
}

///////////////////////////////////////////////////////////////////////////
String AsString(const Abc::ConstPool& cp, const Abc::ClassTable& as3_class, int ind)
{
    String str;
    const Abc::Instance& inst = as3_class.Get(ind).GetInstanceInfo();

    // Build a header ...
    str += AsStringFlag(inst) + AS3::AsString(cp, inst.GetName(cp));

    if (inst.HasSuperClass())
        str += " extends " + AS3::AsString(cp, inst.GetSuperClassName(cp));

    if (inst.GetInterfaces().GetSize() > 0)
    {
        str += " implements ";

        for(AbsoluteIndex i(0); i.Get() < inst.GetInterfaces().GetSize(); ++i)
        {
            if (i.Get() > 0)
                str += ", ";

            str += AS3::AsString(cp, inst.GetInterfaces().GetName(cp, i));
        }
    }

    // !!! Do not display traits and an initializer ...
#if 0
    // Static info ...
    const ClassInfo& info = as3_class.GetInfo(ind);
    str += AS3::AsString(info);

    // Part of Instance info is already displayed ...
    str += AS3::AsString(inst);
#endif

    return str;
}

///////////////////////////////////////////////////////////////////////////
String AsString(const Multiname& mn, NamespaceDisplayType ndt)
{
    switch(mn.GetKind())
    {
    case Abc::MN_QName:
    case Abc::MN_QNameA:
        {
            String result;

            result += AS3::AsString(mn.GetNamespace(), ndt);

            if (!result.IsEmpty())
                result += "::";

            result += AS3::AsString(mn.GetName());

            return result;
        }
        break;
    case Abc::MN_RTQName:
    case Abc::MN_RTQNameA:
        return AS3::AsString(mn.GetName());
    case Abc::MN_RTQNameL:
    case Abc::MN_RTQNameLA:
        return "";
    case Abc::MN_Multiname:
    case Abc::MN_MultinameA:
        {
            String result;

            if (ndt != dShort)
                result = AS3::AsString(mn.GetNamespaceSet());

            if (!result.IsEmpty())
                result += "::";

            result += AS3::AsString(mn.GetName());

            return result;
        }
        break;
    case Abc::MN_MultinameL:
    case Abc::MN_MultinameLA:
        return AS3::AsString(mn.GetNamespaceSet());
    default:
	break;
    }

    SF_ASSERT(false);
    return String();

//     return AsString(mn.GetName());
}

///////////////////////////////////////////////////////////////////////////
#ifdef GFX_AS3_TRACE
static const char* offsets[11] = {
	"",
	"\t",
	"\t\t",
	"\t\t\t",
	"\t\t\t\t",
	"\t\t\t\t\t",
	"\t\t\t\t\t\t",
	"\t\t\t\t\t\t\t",
	"\t\t\t\t\t\t\t\t",
	"\t\t\t\t\t\t\t\t\t",
	"\t\t\t\t\t\t\t\t\t\t",
};

void MakeOffset(FlashUI& ui, UPInt offset)
{
    while (offset)
    {
        const UPInt n = Alg::Min<UPInt>(offset, 10);
        ui.Output(FlashUI::Output_Action, offsets[n]);
        offset -= n;
    }
}

// static void TraceF(FlashUI& ui, int offset, const char* prefix, const char* name, const TR::ValueArrayDH& values, UPInt from = 0)
// {
//     ui.Output(FlashUI::Output_Action, prefix);
//     MakeOffset(ui, offset);
//     ui.Output(FlashUI::Output_Action, name);
//     ui.Output(FlashUI::Output_Action, ": ");
//     Output(ui, values, from);
//     ui.Output(FlashUI::Output_Action, "\n");
// }

static void TraceF(const VM& vm, FlashUI& ui, UPInt offset, const char* prefix, const char* name, const ValueStack& values, UPInt from = 0)
{
    ui.Output(FlashUI::Output_Action, prefix);
    MakeOffset(ui, offset);
    ui.Output(FlashUI::Output_Action, name);
    ui.Output(FlashUI::Output_Action, ": ");
    Output(vm, ui, values, from);
    ui.Output(FlashUI::Output_Action, "\n");
}

static void TraceF(const VM& vm, FlashUI& ui, UPInt offset, const char* prefix, const char* name, const ArrayDH<Value>& values, UPInt from = 0)
{
    ui.Output(FlashUI::Output_Action, prefix);
    MakeOffset(ui, offset);
    ui.Output(FlashUI::Output_Action, name);
    ui.Output(FlashUI::Output_Action, ": ");
    Output(vm, ui, values, from);
    ui.Output(FlashUI::Output_Action, "\n");
}

static void TraceF(const VM& vm, FlashUI& ui, UPInt offset, const char* prefix, const char* name, const ValueRegisterFile& values, UPInt from = 0)
{
    SF_UNUSED1(from);
    ui.Output(FlashUI::Output_Action, prefix);
    MakeOffset(ui, offset);
    ui.Output(FlashUI::Output_Action, name);
    ui.Output(FlashUI::Output_Action, ": ");
    Output(vm, ui, values);
    ui.Output(FlashUI::Output_Action, "\n");
}

void TraceVM(Tracer& tr, const char* prefix)
{
    FlashUI& ui = tr.GetUI();
    VM& vm = tr.GetVM();
    const Tracer& ctr = const_cast<const Tracer&>(tr);
    const TR::Block& block = ctr.GetCurrBlock();
    const TR::State& st = ctr.GetState(block);
    const UPInt offset = tr.GetPrintOffset();

    TraceF(vm, ui, offset, prefix, "stack", st.GetOpStack());
    TraceF(vm, ui, offset, prefix, "scope", st.GetScopeStack());
    TraceF(vm, ui, offset, prefix, "local", st.GetRegisters());
    if (!tr.GetSavedScope().IsEmpty())
        TraceF(vm, ui, offset, prefix, "stored scope", tr.GetSavedScope());
}

void TraceOpCode(Tracer& tr, const StringDataPtr& code, Abc::TCodeOffset offset)
{
    FlashUI& ui = tr.GetUI();
    const Abc::File& file = tr.GetFile().GetAbcFile();

    MakeOffset(ui, tr.GetPrintOffset());
    // We do not know the name at this point.
    AS3::Output(tr.GetVM(), ui, file, reinterpret_cast<const UInt8*>(code.ToCStr()), offset, NULL);
    ui.Output(FlashUI::Output_Action, "\n");
}

void TraceVM(const VM& vm, const char* prefix)
{
    FlashUI& ui = vm.GetUI();
    UPInt offset = vm.GetCallStack().GetSize();
    if (offset)
	    --offset;

    TraceF(vm, ui, offset, prefix, "stack", vm.GetOpStack(), 0);

    TraceF(vm, ui, offset, prefix, "scope", vm.GetScopeStack(), vm.GetCurrCallFrame().GetScopeStackBaseInd());

    TraceF(vm, ui, offset, prefix, "local", vm.GetRegisters(), 0);

    if (!vm.GetCurrCallFrame().GetSavedScope().IsEmpty())
	    TraceF(vm, ui, offset, prefix, "stored scope", vm.GetCurrCallFrame().GetSavedScope());
    //if (vm.GetCurrCallFrame().GetOriginationTraits())
	//TraceF(vm.GetUI(), offset, prefix, "stored traits", AsStringDebug(Value((Class*)&vm.GetCurrCallFrame().GetOriginationTraits()->GetClass())));
}

void TraceOpCode(FlashUI& ui, const VM& vm, Abc::TCodeOffset offset)
{
    if (vm.GetCallStack().GetSize() == 0)
        return;

    MakeOffset(ui, vm.GetCallStack().GetSize() - 1);
#ifdef SF_AS3_ENABLE_CALLFRAME_CACHE
    Output(ui, offset, *vm.GetCallStack().Back());
#else
    Output(ui, offset, vm.GetCallStack().Back());
#endif
    ui.Output(FlashUI::Output_Action, "\n");
}

void OutputMethodName(const VM& vm, FlashUI& ui, int index, int arg_count)
{
    const AS3::VM::OpStackType& st = vm.GetOpStack();
    const Value* last = &st.Top0();

    // Get object.
    const Value& obj = *(last - arg_count);

    if (obj.IsNullOrUndefined())
        return;

    const Traits& tr = vm.GetValueTraits(obj);
    const VTable& vt = tr.GetVT();

    // const Value funct(ndex, vt, Value::kVTableInd);
    const Value& real_func = vt.GetRaw(AbsoluteIndex(index));

    ui.Output(FlashUI::Output_Action, " // ");

    // VTable stores MethodInd or Thunk.
    if (real_func.GetKind() == Value::kMethodInd)
    {
        //const SInt32 method_ind = real_func.GetMethodInd();
        const VTable& real_vt = real_func.GetTraits().GetVT();
        const ASString& name = real_vt.GetName(AbsoluteIndex(index));

        ui.Output(FlashUI::Output_Action, name.ToCStr());
    }
    else
    {
        // Thunk.
        const ThunkInfo& thunk = real_func.AsThunk();

        ui.Output(FlashUI::Output_Action, thunk.Name);
    }

    ui.Output(FlashUI::Output_Action, "()");
}

void OutputSlotName(const VM& vm, FlashUI& ui, int index, Abc::Code::OpCode opcode)
{
    using namespace Abc;

    const VM::OpStackType& st = vm.GetOpStack();
    if ((opcode == Code::op_getslot) || (opcode == Code::op_setslot))
    {
        const Value& v = opcode == Code::op_getslot ? st.Top0() : st.Top1();
        const Traits& tr = vm.GetValueTraits(v);
        // This line is different from op_getabsslot/op_setabsslot.
#ifdef SF_AS3_OLD_GETSLOTNAME
        const SlotInfo& si = tr.GetSlotInfo(AbsoluteIndex(SlotIndex(index), tr));

        ui.Output(FlashUI::Output_Action, " // ");
        ui.Output(FlashUI::Output_Action, si.GetQualifiedName().ToCStr());
#else
        const ASString name = tr.GetSlotName(AbsoluteIndex(SlotIndex(index), tr));

        ui.Output(FlashUI::Output_Action, " // ");
        ui.Output(FlashUI::Output_Action, name.ToCStr());
#endif
    }
}

void OutputGlobalSlotName(const VM& vm, FlashUI& ui, int index)
{
    const Traits& tr = vm.GetGlobalObject()->GetTraits();

#ifdef SF_AS3_OLD_GETSLOTNAME
    const SlotInfo& si = tr.GetSlotInfo(AbsoluteIndex(SlotIndex(index), tr));

    ui.Output(FlashUI::Output_Action, " // ");
    ui.Output(FlashUI::Output_Action, si.GetQualifiedName().ToCStr());
#else
    const ASString name = tr.GetSlotName(AbsoluteIndex(SlotIndex(index), tr));

    ui.Output(FlashUI::Output_Action, " // ");
    ui.Output(FlashUI::Output_Action, name.ToCStr());
#endif
}

void OutputAbsSlotName(const VM& vm, FlashUI& ui, int index, Abc::Code::OpCode opcode)
{
    using namespace Abc;

    const VM::OpStackType& st = vm.GetOpStack();
    if ((opcode == Code::op_getabsslot) || ((opcode == Code::op_setabsslot || opcode == Code::op_initabsslot)))
    {
        const Value& v = opcode == Code::op_getabsslot ? st.Top0() : st.Top1();
        const Traits& tr = vm.GetValueTraits(v);
        // This line is different from op_getslot/op_setslot.
#ifdef SF_AS3_OLD_GETSLOTNAME
        const SlotInfo& si = tr.GetSlotInfo(AbsoluteIndex(index - 1));

        ui.Output(FlashUI::Output_Action, " // ");
        ui.Output(FlashUI::Output_Action, si.GetQualifiedName().ToCStr());
#else
        const ASString name = tr.GetSlotName(AbsoluteIndex(SlotIndex(index), tr));

        ui.Output(FlashUI::Output_Action, " // ");
        ui.Output(FlashUI::Output_Action, name.ToCStr());
#endif
    }
}

#endif // GFX_AS3_TRACE
#endif // GFX_AS3_VERBOSE

}}} // namespace Scaleform { namespace GFx { namespace AS3 {


