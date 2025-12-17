/**************************************************************************

Filename    :   AS3_AOTC.cpp
Content     :   
Created     :   Oct, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "AS3_AOTC2.h"
#include "Kernel/SF_HeapNew.h"
#include "GFx/AS3/AS3_Traits.h"
#include "GFx/AS3/AS3_Tracer.h"
#include "GFx/AS3/AS3_AsString.h"

namespace Scaleform
{
    StringBuffer& operator <<(StringBuffer& sb, int v)
    {
        LongFormatter fmt(v);

        fmt.Convert();
        const StringDataPtr r = fmt.GetResult();
        sb.AppendString(r.ToCStr(), r.GetSize());

        return sb;
    }

    StringBuffer& operator <<(StringBuffer& sb, unsigned v)
    {
        LongFormatter fmt(v);

        fmt.Convert();
        const StringDataPtr r = fmt.GetResult();
        sb.AppendString(r.ToCStr(), r.GetSize());

        return sb;
    }

#if defined(SF_CC_MSVC)
    StringBuffer& operator <<(StringBuffer& sb, SInt32 v)
    {
        LongFormatter fmt(v);

        fmt.Convert();
        const StringDataPtr r = fmt.GetResult();
        sb.AppendString(r.ToCStr(), r.GetSize());

        return sb;
    }

    StringBuffer& operator <<(StringBuffer& sb, UInt32 v)
    {
        LongFormatter fmt(v);

        fmt.Convert();
        const StringDataPtr r = fmt.GetResult();
        sb.AppendString(r.ToCStr(), r.GetSize());

        return sb;
    }
#endif

#if !defined(SF_64BIT_POINTERS) || defined(SF_OS_WIN32)
    StringBuffer& operator <<(StringBuffer& sb, SInt64 v)
    {
        LongFormatter fmt(v);

        fmt.Convert();
        const StringDataPtr r = fmt.GetResult();
        sb.AppendString(r.ToCStr(), r.GetSize());

        return sb;
    }

    StringBuffer& operator <<(StringBuffer& sb, UInt64 v)
    {
        LongFormatter fmt(v);

        fmt.Convert();
        const StringDataPtr r = fmt.GetResult();
        sb.AppendString(r.ToCStr(), r.GetSize());

        return sb;
    }
#endif

    StringBuffer& operator <<(StringBuffer& sb, const GFx::AS3::Value& v)
    {
        using namespace GFx::AS3;

        switch (v.GetKind())
        {
        case Value::kBoolean:
            sb << (v.AsBool() ? "true" : "false");
            break;
        case Value::kInt:
            sb << AsString(v.AsInt());
            break;
        case Value::kUInt:
            sb << AsString(v.AsUInt());
            break;
        case Value::kNumber:
            sb << AsString(v.AsNumber());
            break;
        case Value::kString:
            sb << v.AsString();
            break;
        default:
            break;
        }

        return sb;
    }
}

#if defined(SF_AS3_AOTC2)

namespace Scaleform { namespace GFx { namespace AS3 
{

    ///////////////////////////////////////////////////////////////////////////
    SNode::~SNode()
    {
    }

    void SNode::Accept(AOT::Visitor& v)
    {
        SF_UNUSED1(v);
        SF_ASSERT(false);
    }

    SPtr<SNode> SNode::RetrieveNode(const Value& v)
    {
        if (v.IsSNode())
            return &v.GetSNode();

        return Pickable<SNode>(SF_NEW AOT::SNodeValue(v));
    }

    ///////////////////////////////////////////////////////////////////////////
    SNodeList::SNodeList()
        : Sentinel(SNode::tSentinel SF_DEBUG_ARG(0))
    {
        Clear();
    }

    SNodeList::~SNodeList()
    {
        while (!IsEmpty())
            RemoveNode(GetFirst());
    }

    void SNodeList::Clear()
    {
        Sentinel.SetPrev(&Sentinel);
        Sentinel.SetNext(&Sentinel);
    }

    UPInt SNodeList::GetSize() const
    {
        UPInt result = 0;

        for (SNode* n = GetFirst(); !IsNull(n); n = n->GetNext())
            ++result;

        return result;
    }

    // Checks to see if this SNodeList contains some statements.
    // If this list only contains comment string nodes, then we
    // claim the list has no statements.  Note that we consider
    // name string nodes to be statements.
    bool SNodeList::HasStatements() const
    {
        if (IsEmpty())
            return false;

        bool status = true;
        SNode* en = GetFirst();  // We should be able to assert(!IsNull(GetFirst()));
        if (!IsNull(en) && en->Is(SNode::tStr))
        {
            status = false;
            for ( ; !IsNull(en); en = en->GetNext())
            {
                if (en->Is(SNode::tStr))  // Make certain this is a comment node
                    if (static_cast<const AOT::SNodeStr*>(en)->Is(AOT::SNodeStr::tComment))
                        continue;
                status = true;
                break;
            }
        }

        return status;
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace AOT
    {

        ///////////////////////////////////////////////////////////////////////////
        bool IsAnyType(const Traits& tr)
        {
            bool result = false;
            const VM& vm = tr.GetVM();

            if (tr.IsClassTraits())
            {
                const ClassTraits::Traits* ctr = static_cast<const ClassTraits::Traits*>(&tr);
                result = (ctr == &vm.GetClassTraitsClassClass());
            }
            else
            {
                const InstanceTraits::Traits* itr = static_cast<const InstanceTraits::Traits*>(&tr);
                result = (itr == &vm.GetITraitsObject());
            }

            return result;
        }

        //////////////////////////////////////////////////////////////////////////
        InstanceTraits::Traits& GetInstanceTraits(Traits& tr)
        {
            return (tr.IsClassTraits() ?
                static_cast<ClassTraits::Traits&>(tr).GetInstanceTraits() :
            static_cast<InstanceTraits::Traits&>(tr));
        }

        const InstanceTraits::Traits& GetInstanceTraits(const Traits& tr)
        {
            return (tr.IsClassTraits() ?
                static_cast<const ClassTraits::Traits&>(tr).GetInstanceTraits() :
            static_cast<const InstanceTraits::Traits&>(tr));
        }

        //////////////////////////////////////////////////////////////////////////
        void SNodeStr::Accept(AOT::Visitor& v)
        {
            v.VisitSNodeStr(*this);
        }

        //////////////////////////////////////////////////////////////////////////
        SNodeValue::SNodeValue(const Value& v, OP op) 
            : SNode(SNode::tValue SF_DEBUG_ARG(0))
            , Op(op)
            , V(v)
        {
            // SF_ASSERT(v.GetTraceNullType() == Value::Null || !v.IsTraitsStrict());
        }

        void SNodeValue::Accept(AOT::Visitor& v)
        {
            v.VisitSNodeValue(*this);
        }

        //////////////////////////////////////////////////////////////////////////
        SNodeVar::SNodeVar(const Value& n, const SPtr<const ClassTraits::Traits>& t, OP op) 
            : SNode(SNode::tVar SF_DEBUG_ARG(0))
            , Op(op)
            , Type(t)
            , Name(n)
        {
            SF_ASSERT(n.GetTraceNullType() == Value::Null || !n.IsTraitsStrict());
        }

        void SNodeVar::Accept(AOT::Visitor& v)
        {
            v.VisitSNodeVar(*this);
        }

        //////////////////////////////////////////////////////////////////////////
        void SNodeMbrAcc::Accept(AOT::Visitor& v)
        {
            v.VisitSNodeMbrAcc(*this);
        }

        //////////////////////////////////////////////////////////////////////////
        void SNode1::Accept(AOT::Visitor& v)
        {
            v.VisitSNode1(*this);
        }

        //////////////////////////////////////////////////////////////////////////
        SNode2::SNode2(OP op, const Value& l, const Value& r SF_DEBUG_ARG(UPInt addr))
            : SNode(SNode::t2 SF_DEBUG_ARG(addr))
            , Op(op)
            , L(RetrieveNode(l))
            , R(RetrieveNode(r))
        {
        }
        SNode2::SNode2(OP op, const SPtr<SNode>& l, const Value& r SF_DEBUG_ARG(UPInt addr))
            : SNode(SNode::t2 SF_DEBUG_ARG(addr))
            , Op(op)
            , L(l)
            , R(RetrieveNode(r))
        {
        }
        SNode2::SNode2(OP op, const Value& l, const SPtr<SNode>& r SF_DEBUG_ARG(UPInt addr))
            : SNode(SNode::t2 SF_DEBUG_ARG(addr))
            , Op(op)
            , L(RetrieveNode(l))
            , R(r)
        {
        }
        SNode2::SNode2(OP op, const SPtr<SNode>& l, const SPtr<SNode>& r SF_DEBUG_ARG(UPInt addr))
            : SNode(SNode::t2 SF_DEBUG_ARG(addr))
            , Op(op)
            , L(l)
            , R(r)
        {
        }

        void SNode2::Accept(AOT::Visitor& v)
        {
            v.VisitSNode2(*this);
        }

        //////////////////////////////////////////////////////////////////////////
        void SNodeBlock::Accept(AOT::Visitor& v)
        {
            v.VisitSNodeBlock(*this);
        }

        //////////////////////////////////////////////////////////////////////////
        void SNodeOC::Accept(AOT::Visitor& v)
        {
            v.VisitSNodeOC(*this);
        }

        const SNodeOC::Info SNodeOC::OCInfo[SNodeOC::opNum] = {
            {"aot_throw",            0, Abc::Code::op_throw},
            {"exec_getsuper",        1, Abc::Code::op_getsuper},
            {"exec_setsuper",        1, Abc::Code::op_setsuper},
            {"aot_dxns",             1, Abc::Code::op_dxns},
            {"exec_dxnslate",        0, Abc::Code::op_dxnslate},
            {"exec_kill",            1, Abc::Code::op_kill},
            {"exec_label",           1, Abc::Code::op_label},
            {"exec_inclocal_ti",     1, Abc::Code::op_inclocal_ti},
            {"exec_declocal_ti",     1, Abc::Code::op_declocal_ti},
            {"exec_pushwith",        0, Abc::Code::op_pushwith},
            {"exec_popscope",        0, Abc::Code::op_popscope},
            {"exec_nextname",        0, Abc::Code::op_nextname},
            {"exec_hasnext",         0, Abc::Code::op_hasnext},
            {"exec_pushnull",        0, Abc::Code::op_pushnull},
            {"exec_pushundefined",   0, Abc::Code::op_pushundefined},
            {"exec_not_tb",          0, Abc::Code::op_not_tb},
            {"exec_nextvalue",       0, Abc::Code::op_nextvalue},
            {"exec_pushbyte",        1, Abc::Code::op_pushbyte},
            {"exec_pushshort",       1, Abc::Code::op_pushshort},
            {"exec_pushtrue",        0, Abc::Code::op_pushtrue},
            {"exec_pushfalse",       0, Abc::Code::op_pushfalse},
            {"exec_pushnan",         0, Abc::Code::op_pushnan},
            {"exec_pop",             0, Abc::Code::op_pop},
            {"exec_dup",             0, Abc::Code::op_dup},
            {"exec_swap",            0, Abc::Code::op_swap},
            {"exec_pushstring",      1, Abc::Code::op_pushstring},
            {"exec_pushint",         1, Abc::Code::op_pushint},
            {"exec_pushuint",        1, Abc::Code::op_pushuint},
            {"exec_pushdouble",      1, Abc::Code::op_pushdouble},
            {"exec_pushscope",       0, Abc::Code::op_pushscope},
            {"exec_pushnamespace",   1, Abc::Code::op_pushnamespace},
            {"exec_hasnext2",        2, Abc::Code::op_hasnext2},
            {"exec_increment_tu",    0, Abc::Code::op_increment_tu},
            {"exec_decrement_tu",    0, Abc::Code::op_decrement_tu},
            {"exec_inclocal_tu",     1, Abc::Code::op_inclocal_tu},
            {"exec_declocal_tu",     1, Abc::Code::op_declocal_tu},
            {"exec_negate_ti",       0, Abc::Code::op_negate_ti},
            {"exec_newfunction",     1, Abc::Code::op_newfunction},
            {"exec_call",            1, Abc::Code::op_call},
            {"exec_construct",       1, Abc::Code::op_construct},
            {"exec_callmethod",      2, Abc::Code::op_callmethod},
            {"exec_callstatic",      2, Abc::Code::op_callstatic},
            {"exec_callsuper",       2, Abc::Code::op_callsuper},
            {"exec_callproperty",    2, Abc::Code::op_callproperty},
            {"aot_constructsuper",   1, Abc::Code::op_constructsuper},
            {"exec_constructprop",   2, Abc::Code::op_constructprop},
            {"exec_callproplex",     2, Abc::Code::op_callproplex},
            {"exec_callsupervoid",   2, Abc::Code::op_callsupervoid},
            {"exec_callpropvoid",    2, Abc::Code::op_callpropvoid},
            {"exec_applytype",       1, Abc::Code::op_applytype},
            {"exec_negate_td",       0, Abc::Code::op_negate_td},
            {"exec_newobject",       1, Abc::Code::op_newobject},
            {"exec_newarray",        1, Abc::Code::op_newarray},
            {"exec_newactivation",   0, Abc::Code::op_newactivation},
            {"exec_getdescendants",  1, Abc::Code::op_getdescendants},
            {"exec_newcatch",        1, Abc::Code::op_newcatch},
            {"exec_findpropstrict",  1, Abc::Code::op_findpropstrict},
            {"exec_findproperty",    1, Abc::Code::op_findproperty},
            {"exec_getlex",          1, Abc::Code::op_getlex},
            {"exec_setproperty",     1, Abc::Code::op_setproperty},
            {"aot_getarg",           1, Abc::Code::op_getlocal},
            {"exec_getlocal",        1, Abc::Code::op_getlocal},
            {"exec_setlocal",        1, Abc::Code::op_setlocal},
            {"exec_getglobalscope",  0, Abc::Code::op_getglobalscope},
            {"exec_getscopeobject",  1, Abc::Code::op_getscopeobject},
            {"exec_getproperty",     1, Abc::Code::op_getproperty},
            {"exec_getouterscope",   1, Abc::Code::op_getouterscope},
            {"exec_initproperty",    1, Abc::Code::op_initproperty},
            {"exec_dup_nrc",         0, Abc::Code::op_dup_nrc},
            {"exec_deleteproperty",  1, Abc::Code::op_deleteproperty},
            {"exec_pop_nrc",         0, Abc::Code::op_pop_nrc},
            {"exec_getslot",         1, Abc::Code::op_getslot},
            {"exec_setslot",         1, Abc::Code::op_setslot},
            {"aot_getglobalslot",    1, Abc::Code::op_getglobalslot},
            {"aot_setglobalslot",    1, Abc::Code::op_setglobalslot},
            {"exec_convert_s",       0, Abc::Code::op_convert_s},
            {"exec_esc_xelem",       0, Abc::Code::op_esc_xelem},
            {"exec_esc_xattr",       0, Abc::Code::op_esc_xattr},
            {"exec_convert_i",       0, Abc::Code::op_convert_i},
            {"exec_convert_u",       0, Abc::Code::op_convert_u},
            {"exec_convert_d",       0, Abc::Code::op_convert_d},
            {"exec_convert_b",       0, Abc::Code::op_convert_b},
            {"exec_convert_o",       0, Abc::Code::op_convert_o},
            {"exec_checkfilter",     0, Abc::Code::op_checkfilter},

            {"exec_add_ti",          0, Abc::Code::op_add_ti},
            {"exec_subtract_ti",     0, Abc::Code::op_subtract_ti},
            {"exec_multiply_ti",     0, Abc::Code::op_multiply_ti},
            {"exec_add_td",          0, Abc::Code::op_add_td},
            {"exec_subtract_td",     0, Abc::Code::op_subtract_td},
            {"exec_multiply_td",     0, Abc::Code::op_multiply_td},
            {"exec_divide_td",       0, Abc::Code::op_divide_td},

            {"exec_coerce",          1, Abc::Code::op_coerce},
            {"exec_coerce_a",        0, Abc::Code::op_coerce_a},
            {"exec_coerce_s",        0, Abc::Code::op_coerce_s},
            {"exec_astype",          1, Abc::Code::op_astype},
            {"exec_astypelate",      0, Abc::Code::op_astypelate},

            {"exec_negate",          0, Abc::Code::op_negate},
            {"exec_increment",       0, Abc::Code::op_increment},
            {"exec_inclocal",        1, Abc::Code::op_inclocal},
            {"exec_decrement",       0, Abc::Code::op_decrement},
            {"exec_declocal",        1, Abc::Code::op_declocal},
            {"exec_typeof",          0, Abc::Code::op_typeof},
            {"exec_not",             0, Abc::Code::op_not},
            {"exec_bitnot",          0, Abc::Code::op_bitnot},
            {"exec_increment_ti",    0, Abc::Code::op_increment_ti},
            {"exec_decrement_ti",    0, Abc::Code::op_decrement_ti},
            {"exec_add_d",           0, Abc::Code::op_add_d},

            {"exec_add",             0, Abc::Code::op_add},
            {"exec_subtract",        0, Abc::Code::op_subtract},
            {"exec_multiply",        0, Abc::Code::op_multiply},
            {"exec_divide",          0, Abc::Code::op_divide},
            {"exec_modulo",          0, Abc::Code::op_modulo},
            {"exec_lshift",          0, Abc::Code::op_lshift},
            {"exec_rshift",          0, Abc::Code::op_rshift},
            {"exec_urshift",         0, Abc::Code::op_urshift},
            {"exec_bitand",          0, Abc::Code::op_bitand},
            {"exec_bitor",           0, Abc::Code::op_bitor},
            {"exec_bitxor",          0, Abc::Code::op_bitxor},
            {"exec_equals",          0, Abc::Code::op_equals},
            {"exec_strictequals",    0, Abc::Code::op_strictequals},
            {"exec_lessthan",        0, Abc::Code::op_lessthan},
            {"exec_lessequals",      0, Abc::Code::op_lessequals},
            {"exec_greaterthan",     0, Abc::Code::op_greaterthan},
            {"exec_greaterequals",   0, Abc::Code::op_greaterequals},
            {"exec_instanceof",      0, Abc::Code::op_instanceof},
            {"exec_istype",          1, Abc::Code::op_istype},
            {"exec_istypelate",      0, Abc::Code::op_istypelate},
            {"exec_in",              0, Abc::Code::op_in},

            {"exec_getabsslot",      1, Abc::Code::op_getabsslot},
            {"exec_setabsslot",      1, Abc::Code::op_setabsslot},
            {"exec_initabsslot",     1, Abc::Code::op_initabsslot},
            {"exec_callsupermethod", 2, Abc::Code::op_callsupermethod},
            {"exec_callgetter",      2, Abc::Code::op_callgetter},
            {"exec_callsupergetter", 2, Abc::Code::op_callsupergetter},

            {"exec_increment_i",     0, Abc::Code::op_increment_i},
            {"exec_decrement_i",     0, Abc::Code::op_decrement_i},
            {"exec_inclocal_i",      1, Abc::Code::op_inclocal_i},
            {"exec_declocal_i",      1, Abc::Code::op_declocal_i},
            {"exec_negate_i",        0, Abc::Code::op_negate_i},
            {"exec_add_i",           0, Abc::Code::op_add_i},
            {"exec_subtract_i",      0, Abc::Code::op_subtract_i},
            {"exec_multiply_i",      0, Abc::Code::op_multiply_i},
                                                                       
            {"exec_callobject",      1, Abc::Code::op_callobject},      
        };

        SNodeOC::OP SNodeOC::GetOP(int abc_op)
        {
            OP op = op_throw;
            
            switch (static_cast<Abc::Code::OpCode>(abc_op))
            {
            case Abc::Code::op_throw:
                op = op_throw;
                break;
            case Abc::Code::op_getsuper:
                op = op_getsuper;
                break;
            case Abc::Code::op_setsuper:
                op = op_setsuper;
                break;
            case Abc::Code::op_dxns:
                op = op_dxns;
                break;
            case Abc::Code::op_dxnslate:
                op = op_dxnslate;
                break;
            case Abc::Code::op_kill:
                op = op_kill;
                break;
            case Abc::Code::op_label:
                op = op_label;
                break;
            case Abc::Code::op_inclocal_ti:
                op = op_inclocal_ti;
                break;
            case Abc::Code::op_declocal_ti:
                op = op_declocal_ti;
                break;
            case Abc::Code::op_pushwith:
                op = op_pushwith;
                break;
            case Abc::Code::op_popscope:
                op = op_popscope;
                break;
            case Abc::Code::op_nextname:
                op = op_nextname;
                break;
            case Abc::Code::op_hasnext:
                op = op_hasnext;
                break;
            case Abc::Code::op_pushnull:
                op = op_pushnull;
                break;
            case Abc::Code::op_pushundefined:
                op = op_pushundefined;
                break;
            case Abc::Code::op_not_tb:
                op = op_not_tb;
                break;
            case Abc::Code::op_nextvalue:
                op = op_nextvalue;
                break;
            case Abc::Code::op_pushbyte:
                op = op_pushbyte;
                break;
            case Abc::Code::op_pushshort:
                op = op_pushshort;
                break;
            case Abc::Code::op_pushtrue:
                op = op_pushtrue;
                break;
            case Abc::Code::op_pushfalse:
                op = op_pushfalse;
                break;
            case Abc::Code::op_pushnan:
                op = op_pushnan;
                break;
            case Abc::Code::op_pop:
                op = op_pop;
                break;
            case Abc::Code::op_dup:
                op = op_dup;
                break;
            case Abc::Code::op_swap:
                op = op_swap;
                break;
            case Abc::Code::op_pushstring:
                op = op_pushstring;
                break;
            case Abc::Code::op_pushint:
                op = op_pushint;
                break;
            case Abc::Code::op_pushuint:
                op = op_pushuint;
                break;
            case Abc::Code::op_pushdouble:
                op = op_pushdouble;
                break;
            case Abc::Code::op_pushscope:
                op = op_pushscope;
                break;
            case Abc::Code::op_pushnamespace:
                op = op_pushnamespace;
                break;
            case Abc::Code::op_hasnext2:
                op = op_hasnext2;
                break;
            case Abc::Code::op_increment_tu:
                op = op_increment_tu;
                break;
            case Abc::Code::op_decrement_tu:
                op = op_decrement_tu;
                break;
            case Abc::Code::op_inclocal_tu:
                op = op_inclocal_tu;
                break;
            case Abc::Code::op_declocal_tu:
                op = op_declocal_tu;
                break;
            case Abc::Code::op_negate_ti:
                op = op_negate_ti;
                break;
            case Abc::Code::op_newfunction:
                op = op_newfunction;
                break;
            case Abc::Code::op_call:
                op = op_call;
                break;
            case Abc::Code::op_construct:
                op = op_construct;
                break;
            case Abc::Code::op_callmethod:
                op = op_callmethod;
                break;
            case Abc::Code::op_callstatic:
                op = op_callstatic;
                break;
            case Abc::Code::op_callsuper:
                op = op_callsuper;
                break;
            case Abc::Code::op_callproperty:
                op = op_callproperty;
                break;
            case Abc::Code::op_constructsuper:
                op = op_constructsuper;
                break;
            case Abc::Code::op_constructprop:
                op = op_constructprop;
                break;
            case Abc::Code::op_callproplex:
                op = op_callproplex;
                break;
            case Abc::Code::op_callsupervoid:
                op = op_callsupervoid;
                break;
            case Abc::Code::op_callpropvoid:
                op = op_callpropvoid;
                break;
            case Abc::Code::op_applytype:
                op = op_applytype;
                break;
            case Abc::Code::op_negate_td:
                op = op_negate_td;
                break;
            case Abc::Code::op_newobject:
                op = op_newobject;
                break;
            case Abc::Code::op_newarray:
                op = op_newarray;
                break;
            case Abc::Code::op_newactivation:
                op = op_newactivation;
                break;
            case Abc::Code::op_getdescendants:
                op = op_getdescendants;
                break;
            case Abc::Code::op_newcatch:
                op = op_newcatch;
                break;
            case Abc::Code::op_findpropstrict:
                op = op_findpropstrict;
                break;
            case Abc::Code::op_findproperty:
                op = op_findproperty;
                break;
            case Abc::Code::op_getlex:
                op = op_getlex;
                break;
            case Abc::Code::op_setproperty:
                op = op_setproperty;
                break;
            case Abc::Code::op_getlocal:
                op = op_getlocal;
                break;
            case Abc::Code::op_setlocal:
                op = op_setlocal;
                break;
            case Abc::Code::op_getglobalscope:
                op = op_getglobalscope;
                break;
            case Abc::Code::op_getscopeobject:
                op = op_getscopeobject;
                break;
            case Abc::Code::op_getproperty:
                op = op_getproperty;
                break;
            case Abc::Code::op_getouterscope:
                op = op_getouterscope;
                break;
            case Abc::Code::op_initproperty:
                op = op_initproperty;
                break;
            case Abc::Code::op_dup_nrc:
                op = op_dup_nrc;
                break;
            case Abc::Code::op_deleteproperty:
                op = op_deleteproperty;
                break;
            case Abc::Code::op_pop_nrc:
                op = op_pop_nrc;
                break;
            case Abc::Code::op_getslot:
                op = op_getslot;
                break;
            case Abc::Code::op_setslot:
                op = op_setslot;
                break;
            case Abc::Code::op_getglobalslot:
                op = op_getglobalslot;
                break;
            case Abc::Code::op_setglobalslot:
                op = op_setglobalslot;
                break;
            case Abc::Code::op_convert_s:
                op = op_convert_s;
                break;
            case Abc::Code::op_esc_xelem:
                op = op_esc_xelem;
                break;
            case Abc::Code::op_esc_xattr:
                op = op_esc_xattr;
                break;
            case Abc::Code::op_convert_i:
                op = op_convert_i;
                break;
            case Abc::Code::op_convert_u:
                op = op_convert_u;
                break;
            case Abc::Code::op_convert_d:
                op = op_convert_d;
                break;
            case Abc::Code::op_convert_b:
                op = op_convert_b;
                break;
            case Abc::Code::op_convert_o:
                op = op_convert_o;
                break;
            case Abc::Code::op_checkfilter:
                op = op_checkfilter;
                break;

            case Abc::Code::op_add_ti:
                op = op_add_ti;
                break;
            case Abc::Code::op_subtract_ti:
                op = op_subtract_ti;
                break;
            case Abc::Code::op_multiply_ti:
                op = op_multiply_ti;
                break;
            case Abc::Code::op_add_td:
                op = op_add_td;
                break;
            case Abc::Code::op_subtract_td:
                op = op_subtract_td;
                break;
            case Abc::Code::op_multiply_td:
                op = op_multiply_td;
                break;
            case Abc::Code::op_divide_td:
                op = op_divide_td;
                break;

            case Abc::Code::op_coerce:
                op = op_coerce;
                break;
            case Abc::Code::op_coerce_a:
                op = op_coerce_a;
                break;
            case Abc::Code::op_coerce_s:
                op = op_coerce_s;
                break;
            case Abc::Code::op_astype:
                op = op_astype;
                break;
            case Abc::Code::op_astypelate:
                op = op_astypelate;
                break;

            case Abc::Code::op_negate:
                op = op_negate;
                break;
            case Abc::Code::op_increment:
                op = op_increment;
                break;
            case Abc::Code::op_inclocal:
                op = op_inclocal;
                break;
            case Abc::Code::op_decrement:
                op = op_decrement;
                break;
            case Abc::Code::op_declocal:
                op = op_declocal;
                break;
            case Abc::Code::op_typeof:
                op = op_typeof;
                break;
            case Abc::Code::op_not:
                op = op_not;
                break;
            case Abc::Code::op_bitnot:
                op = op_bitnot;
                break;
            case Abc::Code::op_increment_ti:
                op = op_increment_ti;
                break;
            case Abc::Code::op_decrement_ti:
                op = op_decrement_ti;
                break;
            case Abc::Code::op_add_d:
                op = op_add_d;
                break;

            case Abc::Code::op_add:
                op = op_add;
                break;
            case Abc::Code::op_subtract:
                op = op_subtract;
                break;
            case Abc::Code::op_multiply:
                op = op_multiply;
                break;
            case Abc::Code::op_divide:
                op = op_divide;
                break;
            case Abc::Code::op_modulo:
                op = op_modulo;
                break;
            case Abc::Code::op_lshift:
                op = op_lshift;
                break;
            case Abc::Code::op_rshift:
                op = op_rshift;
                break;
            case Abc::Code::op_urshift:
                op = op_urshift;
                break;
            case Abc::Code::op_bitand:
                op = op_bitand;
                break;
            case Abc::Code::op_bitor:
                op = op_bitor;
                break;
            case Abc::Code::op_bitxor:
                op = op_bitxor;
                break;
            case Abc::Code::op_equals:
                op = op_equals;
                break;
            case Abc::Code::op_strictequals:
                op = op_strictequals;
                break;
            case Abc::Code::op_lessthan:
                op = op_lessthan;
                break;
            case Abc::Code::op_lessequals:
                op = op_lessequals;
                break;
            case Abc::Code::op_greaterthan:
                op = op_greaterthan;
                break;
            case Abc::Code::op_greaterequals:
                op = op_greaterequals;
                break;
            case Abc::Code::op_instanceof:
                op = op_instanceof;
                break;
            case Abc::Code::op_istype:
                op = op_istype;
                break;
            case Abc::Code::op_istypelate:
                op = op_istypelate;
                break;
            case Abc::Code::op_in:
                op = op_in;
                break;

            case Abc::Code::op_getabsslot:
                op = op_getabsslot;
                break;
            case Abc::Code::op_setabsslot:
                op = op_setabsslot;
                break;
            case Abc::Code::op_initabsslot:
                op = op_initabsslot;
                break;
            case Abc::Code::op_callsupermethod:
                op = op_callsupermethod;
                break;
            case Abc::Code::op_callgetter:
                op = op_callgetter;
                break;
            case Abc::Code::op_callsupergetter:
                op = op_callsupergetter;
                break;

            case Abc::Code::op_increment_i:
                op = op_increment_i;
                break;
            case Abc::Code::op_decrement_i:
                op = op_decrement_i;
                break;
            case Abc::Code::op_inclocal_i:
                op = op_inclocal_i;
                break;
            case Abc::Code::op_declocal_i:
                op = op_declocal_i;
                break;
            case Abc::Code::op_negate_i:
                op = op_negate_i;
                break;
            case Abc::Code::op_add_i:
                op = op_add_i;
                break;
            case Abc::Code::op_subtract_i:
                op = op_subtract_i;
                break;
            case Abc::Code::op_multiply_i:
                op = op_multiply_i;
                break;

            case Abc::Code::op_callobject:
                op = op_callobject;
                break;
            default:
                break;
            }

            return op;
        }


        //////////////////////////////////////////////////////////////////////////
        void SNodeSwitchOC::Accept(AOT::Visitor& v)
        {
            v.VisitSNodeSwitchOC(*this);
        }

        //////////////////////////////////////////////////////////////////////////
        void SNodeIF::Accept(AOT::Visitor& v)
        {
            v.VisitSNodeIF(*this);
        }

        //////////////////////////////////////////////////////////////////////////
        void SNodeAbrupt::Accept(AOT::Visitor& v)
        {
            v.VisitSNodeAbrupt(*this);
        }

        //////////////////////////////////////////////////////////////////////////
        SNodeN::SNodeN(OP op, const String& name SF_DEBUG_ARG(UPInt addr))
            : SNode(SNode::tN SF_DEBUG_ARG(addr))
            , Op(op)
            , Name(name)
        {
        }

        SNodeN::SNodeN(OP op, const String& name, UInt32 argc, const Value* argv SF_DEBUG_ARG(UPInt addr))
            : SNode(SNode::tN SF_DEBUG_ARG(addr))
            , Op(op)
            , Name(name)
        {
            for (UPInt i = 0; i < argc; ++i)
                Args.PushBack(RetrieveNode(argv[i]));
        }

        void SNodeN::Accept(AOT::Visitor& v)
        {
            v.VisitSNodeN(*this);
        }

        void SNodeN::ReleaseASTChildren()
        {
            const UPInt size = Args.GetSize();
            for (UPInt i = 0; i < size; ++i)
            {
                if (Args[i])
                    Args[i]->ReleaseAST();
            }
        }

        //////////////////////////////////////////////////////////////////////////
        Visitor::~Visitor()
        {
        }

        void Visitor::VisitSNodeStr(SNodeStr& n)
        {
            SF_UNUSED1(n);
        }

        void Visitor::VisitSNodeValue(SNodeValue& n)
        {
            SF_UNUSED1(n);
        }

        void Visitor::VisitSNodeVar(SNodeVar& n)
        {
            SF_UNUSED1(n);
        }

        void Visitor::VisitSNodeMbrAcc(SNodeMbrAcc& n)
        {
            SF_UNUSED1(n);
        }

        void Visitor::VisitSNode1(SNode1& n)
        {
            SF_UNUSED1(n);
        }

        void Visitor::VisitSNode2(SNode2& n)
        {
            SF_UNUSED1(n);
        }

        void Visitor::VisitSNodeBlock(SNodeBlock& n)
        {
            SF_UNUSED1(n);
        }

        void Visitor::VisitSNodeOC(SNodeOC& n)
        {
            SF_UNUSED1(n);
        }

        void Visitor::VisitSNodeSwitchOC(SNodeSwitchOC& n)
        {
            SF_UNUSED1(n);
        }

        void Visitor::VisitSNodeIF(SNodeIF& n)
        {
            SF_UNUSED1(n);
        }

        void Visitor::VisitSNodeAbrupt(SNodeAbrupt& n)
        {
            SF_UNUSED1(n);
        }

        void Visitor::VisitSNodeN(SNodeN& n)
        {
            SF_UNUSED1(n);
        }

    } // namespace AOT

}}} // namespace Scaleform { namespace GFx { namespace AS3 

#endif // SF_AS3_AOTC
