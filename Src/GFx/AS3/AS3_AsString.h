/**************************************************************************

Filename    :   AS3_AsString.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_AS3_AsString_H
#define INC_AS3_AsString_H

#include "Kernel/SF_MsgFormat.h"
#include "AS3_VM.h"
#include "AS3_Value.h"
#include "AS3_VTable.h"
#include "AS3_FlashUI.h"
#include "Abc/AS3_Abc.h"

namespace Scaleform { namespace GFx { namespace AS3 
{

///////////////////////////////////////////////////////////////////////////
// Forward declaration.
class Object;
class Traits;
class VM;
class SlotInfo;
class Multiname;
class NamespaceSet;
class CallFrame;
class Namespace;

///////////////////////////////////////////////////////////////////////////
enum NamespaceDisplayType 
{
    dComplete,
    dDeclarationOnly,
    dShort
};

///////////////////////////////////////////////////////////////////////////
const char* AsString(const Abc::NamespaceKind kind);
// This function is required by Array.
ASString AsString(const Value& value, StringManager& sm);

#if defined(GFX_AS3_VERBOSE) || defined(SF_AS3_AOTC)
String AsString(const Value& value);
#endif

#if defined(GFX_AS3_VERBOSE)

class Tracer;
class FlashUI;
class ValueStack;

void MakeOffset(FlashUI& ui, UPInt offset);

String AsString(const ValueArray& values);

void Output(FlashUI& ui, SPInt v);
void Output(FlashUI& ui, UPInt v, bool as_hex = false);
void Output(FlashUI& ui, Value::Number v);
void Output(const VM& vm, FlashUI& ui, const ValueStack& values, UPInt from = 0);
void Output(const VM& vm, FlashUI& ui, const ArrayDH<Value>& values, UPInt from = 0);
void Output(const VM& vm, FlashUI& ui, const Value& value);
void Output(const VM& vm, FlashUI& ui, const ValueRegisterFile& values);
#ifdef GFX_AS3_TRACE
void Output(FlashUI& ui, Abc::TCodeOffset offset, const CallFrame& cf);
void TraceOpCode(FlashUI& ui, const VM& vm, Abc::TCodeOffset offset);
void TraceOpCode(Tracer& tr, const StringDataPtr& code, Abc::TCodeOffset offset);
#endif

String AsString(const ValueArrayDH& values);

#ifdef GFX_AS3_TRACE
void TraceVM(const VM& vm, const char* prefix = "");
void TraceVM(Tracer& tr, const char* prefix = "");
#endif

String AsString(const Abc::NamespaceInfo& info, NamespaceDisplayType ndt = dShort);

String AsString(const Abc::ConstPool& cp, const Abc::NamespaceSetInfo& info, NamespaceDisplayType ndt = dComplete);

String AsString(const Abc::ConstPool& cp, const Abc::Multiname& info);
String AsString(const Multiname& mn, NamespaceDisplayType ndt = dComplete);

String AsString(const Abc::ConstPool& cp, const Abc::ValueDetail& detail);
String AsString(const Abc::ConstPool& cp, const Abc::MethodInfo& info);

String AsStringFlag(const Abc::Instance& info);
String AsString(const Abc::ConstPool& cp, const Abc::ClassTable& as3_class, int ind);

String AsString(const Namespace& ns, NamespaceDisplayType ndt);
String AsString(const NamespaceSet& nss);

#ifdef GFX_AS3_TRACE
void OutputMethodName(const VM& vm, FlashUI& ui, int index, int arg_count);
void OutputSlotName(const VM& vm, FlashUI& ui, int index, Abc::Code::OpCode opcode);
void OutputGlobalSlotName(const VM& vm, FlashUI& ui, int index);
void OutputAbsSlotName(const VM& vm, FlashUI& ui, int index, Abc::Code::OpCode opcode);

template <typename T>
void Output(const VM& vm, FlashUI& ui, const Abc::File& file, const T* code, Abc::TCodeOffset offset, const char* cf_name)
{
    using namespace Abc;

    const Abc::ConstPool& cp = file.GetConstPool();
    const Code::OpCode opcode = static_cast<Code::OpCode>(Read8(code, offset));

    Output(ui, UPInt(offset - 1));
    ui.Output(FlashUI::Output_Action, " ");
    ui.Output(FlashUI::Output_Action, Code::GetOpCodeInfo(opcode).name);
    ui.Output(FlashUI::Output_Action, " ");

    switch (opcode)
    {
    case Code::op_getabsobject:
        ui.Output(FlashUI::Output_Action, "obj: ");
        Output(vm, ui, GetAbsObject(ReadU30(code, offset)));
        break;
    case Code::op_getsuper:
    case Code::op_setsuper:
        ui.Output(FlashUI::Output_Action, AS3::AsString(cp, cp.GetMultiname(ReadU30(code, offset))));
        break;
    case Code::op_dxns:
        {
            const StringDataPtr& s = cp.GetString(AbsoluteIndex(ReadU30(code, offset)));
            const String str(s.ToCStr(), s.GetSize());
            ui.Output(FlashUI::Output_Action, str);
        }
        break;
    case Code::op_inclocal:
    case Code::op_declocal:
    case Code::op_inclocal_i:
    case Code::op_declocal_i:
    case Code::op_inclocal_ti:
    case Code::op_declocal_ti:
    case Code::op_kill:
        ui.Output(FlashUI::Output_Action, "reg ");
        ui.Output(FlashUI::Output_Action, Scaleform::AsString(ReadU30(code, offset)));
        break;
    case Code::op_ifnlt:
    case Code::op_ifnle:
    case Code::op_ifngt:
    case Code::op_ifnge:

    case Code::op_ifnlt_ti:
    case Code::op_ifnle_ti:
    case Code::op_ifngt_ti:
    case Code::op_ifnge_ti:

    case Code::op_jump:

    case Code::op_iftrue:
    case Code::op_iffalse:

    case Code::op_iftrue_tb:
    case Code::op_iffalse_tb:

    case Code::op_ifeq:
    case Code::op_ifge:
    case Code::op_ifgt:
    case Code::op_ifle:
    case Code::op_iflt:
    case Code::op_ifne:

    case Code::op_ifeq_ti:
    case Code::op_ifge_ti:
    case Code::op_ifgt_ti:
    case Code::op_ifle_ti:
    case Code::op_iflt_ti:
    case Code::op_ifne_ti:

    case Code::op_ifstricteq:
    case Code::op_ifstrictne:
        {
            const Abc::TCodeOffset ot = offset;
            ui.Output(FlashUI::Output_Action, Scaleform::AsString(ot + ReadS24(code, offset) + JumpOffsetCorrection<UInt8>()));
        }
        break;
    case Code::op_debugfile:
        {
            // We do not store strings in byte code in case of op_debugfile.
            const StringDataPtr& s = cp.GetString(AbsoluteIndex(ReadU30(code, offset)));
            const String str(s.ToCStr(), s.GetSize());

            ui.Output(FlashUI::Output_Action, "'");
            ui.Output(FlashUI::Output_Action, str);
            ui.Output(FlashUI::Output_Action, "'");
        }
        break;
    case Code::op_pushstring:
        {
            const int ind = ReadU30(code, offset);

            ui.Output(FlashUI::Output_Action, "'");

            if (sizeof(T) == 1)
            {
                const StringDataPtr& s = cp.GetString(AbsoluteIndex(ind));
                String str(s.ToCStr(), s.GetSize());
                ui.Output(FlashUI::Output_Action, str);
            }
            else
            {
#if defined(SF_AS3_STORE_PRIMITIVE_VALUES_IN_BYTECODE)
                ASStringNode* sn = reinterpret_cast<ASStringNode*>((uintptr_t)ind);
                ui.Output(FlashUI::Output_Action, sn->pData);
#else
                const StringDataPtr& s = cp.GetString(AbsoluteIndex(ind));
                String str(s.ToCStr(), s.GetSize());
                ui.Output(FlashUI::Output_Action, str);
#endif
            }

            ui.Output(FlashUI::Output_Action, "'");
        }
        break;
    case Code::op_pushbyte:
        ui.Output(FlashUI::Output_Action, Scaleform::AsString(Read8(code, offset)));
        break;
    case Code::op_pushint:
#ifdef SF_AS3_STORE_PRIMITIVE_VALUES_IN_BYTECODE
        ui.Output(FlashUI::Output_Action, Scaleform::AsString(ReadU30(code, offset)));
#else
        ui.Output(FlashUI::Output_Action, Scaleform::AsString(cp.GetInt(ReadU30(code, offset))));
#endif
        break;
    case Code::op_pushuint:
#ifdef SF_AS3_STORE_PRIMITIVE_VALUES_IN_BYTECODE
        ui.Output(FlashUI::Output_Action, Scaleform::AsString(static_cast<UInt32>(ReadU30(code, offset))));
#else
        ui.Output(FlashUI::Output_Action, Scaleform::AsString(cp.GetUInt(ReadU30(code, offset))));
#endif
        break;
    case Code::op_pushdouble:
#ifdef SF_AS3_STORE_PRIMITIVE_VALUES_IN_BYTECODE
        if (sizeof(T) == 1)
            ui.Output(FlashUI::Output_Action, Scaleform::AsString(cp.GetDouble(ReadU30(code, offset))));
        else if (sizeof(Abc::TOpCode::ValueType) == sizeof(AS3::Value::Number))
        {
            // This case handles 64-bit platforms and SF_NO_DOUBLE.
            union
            {
                TOpCode::ValueType  I;
                Value::Number       D;
            } u;

            u.I = ReadUPInt(code, offset);
            ui.Output(FlashUI::Output_Action, Scaleform::AsString(u.D));
        }
        else
        {
            union
            {
                UInt32  I[2];
                double  D;
            } u;

            u.I[0] = ReadU30(code, offset);
            u.I[1] = ReadU30(code, offset);
            ui.Output(FlashUI::Output_Action, Scaleform::AsString(u.D));
        }
#else
        ui.Output(FlashUI::Output_Action, Scaleform::AsString(cp.GetDouble(ReadU30(code, offset))));
#endif
        break;
    case Code::op_pushnamespace:
        ui.Output(FlashUI::Output_Action, AS3::AsString(cp.GetNamespace(ReadU30(code, offset))));
        break;
    case Code::op_hasnext2:
        {
            UInt32 object_reg = ReadU30(code, offset);
            UInt32 index_reg = ReadU30(code, offset);

            ui.Output(FlashUI::Output_Action, "object_reg: ");
            ui.Output(FlashUI::Output_Action, Scaleform::AsString(object_reg));
            ui.Output(FlashUI::Output_Action, " index_reg: ");
            ui.Output(FlashUI::Output_Action, Scaleform::AsString(index_reg));
        }
        break;
    case Code::op_newfunction:
        ui.Output(FlashUI::Output_Action, AS3::AsString(cp, file.GetMethods().Get(Abc::MiInd(ReadU30(code, offset)))));
        break;
    case Code::op_callmethod:
    case Code::op_callsupermethod:
        // It is not clear how op_callgetter/op_callsupergetter will work in this case.
//     case Code::op_callgetter:
//     case Code::op_callsupergetter:
        {
            int index = ReadU30(code, offset);
            int arg_count = ReadU30(code, offset);

            ui.Output(FlashUI::Output_Action, "ind: ");
            ui.Output(FlashUI::Output_Action, Scaleform::AsString(index));
            ui.Output(FlashUI::Output_Action, " argc=");
            ui.Output(FlashUI::Output_Action, Scaleform::AsString(arg_count));

            // Try to get a method name.
            if (sizeof(T) > 1)
                // This is wordcode. We are not tracing.
                OutputMethodName(vm, ui, index, arg_count);
        }
        break;
    case Code::op_callstatic:
        {
            Abc::MiInd index(ReadU30(code, offset));
            int arg_count = ReadU30(code, offset);

            ui.Output(FlashUI::Output_Action, AS3::AsString(cp, file.GetMethods().Get(index)));
            ui.Output(FlashUI::Output_Action, " argc=");
            ui.Output(FlashUI::Output_Action, Scaleform::AsString(arg_count));
        }
        break;
    case Code::op_callproperty:
    case Code::op_callpropvoid:
    case Code::op_callproplex:
    case Code::op_callsuper:
    case Code::op_callsupervoid:
    case Code::op_constructprop:
        {
            int index = ReadU30(code, offset);
            int arg_count = ReadU30(code, offset);

            ui.Output(FlashUI::Output_Action, AS3::AsString(cp, cp.GetMultiname(index)));
            ui.Output(FlashUI::Output_Action, " argc=");
            ui.Output(FlashUI::Output_Action, Scaleform::AsString(arg_count));
        }
        break;
    case Code::op_call:
    case Code::op_construct:
    case Code::op_constructsuper:
    case Code::op_newobject:
    case Code::op_newarray:
        ui.Output(FlashUI::Output_Action, "argc=");
        ui.Output(FlashUI::Output_Action, Scaleform::AsString(ReadU30(code, offset)));
        break;
    case Code::op_newclass:
        ui.Output(FlashUI::Output_Action, AS3::AsString(cp, file.GetClasses(), ReadU30(code, offset)));
        break;
    case Code::op_getdescendants:
    case Code::op_getproperty:
    case Code::op_setproperty:
    case Code::op_initproperty:
    case Code::op_findpropstrict:
    case Code::op_findproperty:
    case Code::op_deleteproperty:
    case Code::op_istype:
    case Code::op_coerce:
    case Code::op_astype:
    case Code::op_getlex:
        ui.Output(FlashUI::Output_Action, AS3::AsString(cp, cp.GetMultiname(ReadU30(code, offset))));
        break;
    case Code::op_getslot:
    case Code::op_setslot:
        {
            int index = ReadU30(code, offset);
            ui.Output(FlashUI::Output_Action, Scaleform::AsString(index));

            // Try to get a slot name.
            if (sizeof(T) > 1)
                // This is wordcode. We are not tracing.
                OutputSlotName(vm, ui, index, opcode);
        }
        break;
    case Code::op_getabsslot:
    case Code::op_setabsslot:
    case Code::op_initabsslot:
        {
            int index = ReadU30(code, offset);

            ui.Output(FlashUI::Output_Action, Scaleform::AsString(index));

            // Try to get a slot name.
            if (sizeof(T) > 1)
                OutputAbsSlotName(vm, ui, index, opcode);
        }
        break;
    case Code::op_getglobalslot:
    case Code::op_setglobalslot:
        {
            int index = ReadU30(code, offset);

            ui.Output(FlashUI::Output_Action, Scaleform::AsString(index));

            // Try to get a slot name.
            if (sizeof(T) > 1)
                // This is wordcode. We are not tracing.
                OutputGlobalSlotName(vm, ui, index);
        }
        break;
    case Code::op_getlocal:
    case Code::op_setlocal:
        ui.Output(FlashUI::Output_Action, "reg: ");
        ui.Output(FlashUI::Output_Action, Scaleform::AsString(ReadU30(code, offset)));
        break;
        // case op_newcatch:
        // bc.ReadU30();
        // // index is a u30 that must be an index of an exception_info structure for this method.
        // // result << AsString(cp, cp.GetMultiname(bc.ReadU30()));
        // break;
    case Code::op_lookupswitch:
        {
            int default_offset = ReadS24(code, offset);
            int case_count = ReadU30(code, offset);
            //int case_offset;

            ui.Output(FlashUI::Output_Action, "default: ");
            ui.Output(FlashUI::Output_Action, Scaleform::AsString(default_offset));
            ui.Output(FlashUI::Output_Action, " maxcase: ");
            ui.Output(FlashUI::Output_Action, Scaleform::AsString(case_count));
            for(int i = 0; i <= case_count; ++i)
            {
                ui.Output(FlashUI::Output_Action, " ");
                ui.Output(FlashUI::Output_Action, Scaleform::AsString(ReadS24(code, offset)));
            }
        }
        break;
    case Code::op_debug:
        {
            const UInt8 debug_type = static_cast<UInt8>(Read8(code, offset));
            const int name_ind = ReadU30(code, offset);
            const UInt8 dreg = static_cast<UInt8>(Read8(code, offset));
            const int extra = ReadU30(code, offset);
            SF_UNUSED3(debug_type, extra, dreg);
            const StringDataPtr& s = cp.GetString(AbsoluteIndex(name_ind));
            const String str(s.ToCStr(), s.GetSize());

            ui.Output(FlashUI::Output_Action, "name: ");
            ui.Output(FlashUI::Output_Action, str);
        }
        break;
    case Code::op_returnvalue:
    case Code::op_returnvoid:
        if (cf_name)
        {
            ui.Output(FlashUI::Output_Action, "// ");
            ui.Output(FlashUI::Output_Action, cf_name);
        }
        break;
        /*
    case Code::op_setslot_str:
    case Code::op_setslot_num:
    case Code::op_setslot_uint:
    case Code::op_setslot_sint:
    case Code::op_setslot_bool:
    case Code::op_setslot_value:
    case Code::op_setslot_obj_as:
    case Code::op_setslot_obj_cpp:
        break;
        */
    default:
        switch (Code::GetOpCodeInfo(opcode).operandCount)
        {
        case 1:
            ui.Output(FlashUI::Output_Action, Scaleform::AsString(ReadU30(code, offset)));
            break;
        case 2:
            {
                int _1 = ReadU30(code, offset);
                int _2 = ReadU30(code, offset);

                ui.Output(FlashUI::Output_Action, Scaleform::AsString(_1));
                ui.Output(FlashUI::Output_Action, " ");
                ui.Output(FlashUI::Output_Action, Scaleform::AsString(_2));
            }
            break;
        default:
            break;
        }
    }

    if (offset == 1 && cf_name)
    {
        ui.Output(FlashUI::Output_Action, "// ");
        ui.Output(FlashUI::Output_Action, cf_name);
    }
}
#endif // GFX_AS3_TRACE

#endif

}}} // namespace Scaleform { namespace GFx { namespace AS3 {


#endif // INC_AS3_AsString_H

