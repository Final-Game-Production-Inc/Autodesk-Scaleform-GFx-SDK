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

#include "AS3_AOTC.h"
#include "Kernel/SF_HeapNew.h"
#include "GFx/AS3/AS3_Traits.h"
#include "GFx/AS3/AS3_Tracer.h"
#include "GFx/AS3/AS3_AsString.h"

#if defined(SF_AS3_AOTC)

namespace Scaleform { namespace GFx { namespace AS3 
{

    ///////////////////////////////////////////////////////////////////////////
    SPtr<AOT::SNodeRT>& Tracer::GetSNodeRT(UInt32 n, const SPtr<const Traits>& t SF_DEBUG_ARG(UPInt addr))
    {
        RTHashType::ValueType* pV;

        while ((pV = RTHash.Get(RTKey(n, t))) == NULL)
        {
            SPtr<AOT::SNodeRT> v;
            v.Pick(SF_NEW AOT::SNodeRT(n, t SF_DEBUG_ARG(addr)));

            RTHash.Add(RTKey(n, t), v);
        }

        return *pV;
    }

    ///////////////////////////////////////////////////////////////////////////
    SNode::~SNode()
    {
    }

    SPtr<SNode> SNode::RetrieveNode(const Value& v)
    {
        if (v.IsSNode())
            return &v.GetSNode();

        return Pickable<SNode>(SF_NEW AOT::SNodeValue(v));
    }

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

        bool IsAnyType(const VM& vm, SNode& n)
        {
            bool result = true;
            const Traits* tr = GetTraits(vm, n);

            if (tr)
                result = IsAnyType(*tr);

            return result;
        }

        // SInt32 or UInt32
        bool IsFixedNumType(const VM& vm, SNode& n)
        {
            bool result = false;
            const Traits* tr = GetTraits(vm, n);

            if (tr)
                result = vm.IsFixedNumType(*tr);

            return result;
        }

        // SInt32, UInt32, or Number.
        bool IsNumericType(const VM& vm, SNode& n)
        {
            bool result = false;
            const Traits* tr = GetTraits(vm, n);

            if (tr)
                result = vm.IsNumericType(*tr);

            return result;
        }

        // Number.
        bool IsNumberType(const VM& vm, SNode& n)
        {
            bool result = false;
            const Traits* tr = GetTraits(vm, n);

            if (tr)
                result = (tr == &vm.GetITraitsNumber());

            return result;
        }

        // String.
        bool IsStringType(const VM& vm, SNode& n)
        {
            bool result = false;
            const Traits* tr = GetTraits(vm, n);

            if (tr)
                result = (tr == &vm.GetITraitsString());

            return result;
        }

        //////////////////////////////////////////////////////////////////////////
        // http://en.wikipedia.org/wiki/Operators_in_C_and_C%2B%2B

        UInt8 GetPrecedence(SNode1::OP op)
        {
            UInt8 result = 0;

            switch (op)
            {
            case SNode1::opNot:
            case SNode1::opNegate:
                result = 3;
                break;
            case SNode1::opIncr:
            case SNode1::opDecr:
                result = 2;
                break;
            case SNode1::opBitNot:
                result = 3;
                break;
            default:
                break;
            }

            return result;
        }

        UInt8 GetPrecedence(SNode2::OP op)
        {
            UInt8 result = 0;

            switch (op)
            {
            case SNode2::opAssign:
                result = 16;
                break;
            case SNode2::opAdd:
            case SNode2::opSub:
                result = 6;
                break;
            case SNode2::opMul:
            case SNode2::opDiv:
            case SNode2::opModulo:
                result = 5;
                break;
            case SNode2::opBitOr:
                result = 12;
                break;
            case SNode2::opBitXOr:
                result = 11;
                break;
            case SNode2::opBitAnd:
                result = 10;
                break;
            case SNode2::opLSchift:
            case SNode2::opRSchift:
            case SNode2::opURSchift:
                result = 7;
                break;
            default:
                break;
            }

            return result;
        }

        UInt8 GetPrecedence(SNodeBoolExpr::OP op)
        {
            UInt8 result = 0;

            switch (op)
            {
            case SNodeBoolExpr::opAnd:
                result = 13;
                break;
            case SNodeBoolExpr::opOr:
                result = 14;
                break;
            case SNodeBoolExpr::opNLT:
            case SNodeBoolExpr::opNLE:
            case SNodeBoolExpr::opNGT:
            case SNodeBoolExpr::opNGE:
            case SNodeBoolExpr::opGE:
            case SNodeBoolExpr::opGT:
            case SNodeBoolExpr::opLE:
            case SNodeBoolExpr::opLT:
                result = 8;
                break;
            case SNodeBoolExpr::opEQ:
            case SNodeBoolExpr::opNE:
            case SNodeBoolExpr::opStrictEQ:
            case SNodeBoolExpr::opStrictNE:
                result = 9;
                break;
            default:
                break;
            }

            return result;
        }

        UInt8 GetPrecedence(const SNode& n)
        {
            UInt8 result = 0;

            switch (n.GetType())
            {
            case SNode::tN1:
                result = GetPrecedence(static_cast<const SNode1&>(n).Op);
                break;
            case SNode::tN2:
                result = GetPrecedence(static_cast<const SNode2&>(n).Op);
                break;
            case SNode::tBoolExpr:
                result = GetPrecedence(static_cast<const SNodeBoolExpr&>(n).Op);
                break;
            default:
                break;
            }

            return result;
        }

        //////////////////////////////////////////////////////////////////////////
        // Check if a node is a "this" variable.
        bool IsThisVar(const SNode& n)
        {
            bool result = false;

            if (n.GetType() == SNode::tVar)
                result = static_cast<const SNodeVar&>(n).IsThis();

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

        const Traits* GetTraitsSNode2(const VM& vm, const SNode2& n)
        {
            const Traits* result = NULL;
            if (!n.L || !n.R)
                return result;

            const Traits* ltr = GetTraits(vm, *n.L);
            const Traits* rtr = GetTraits(vm, *n.R);

            if (ltr == NULL || rtr == NULL)
                return result;

            switch (n.Op)
            {
            case SNode2::opAdd:
                if (ltr == &vm.GetITraitsString() || rtr == &vm.GetITraitsString())
                    result = &vm.GetITraitsString();
                else
                    result = &vm.GetITraitsNumber();
                break;
            case SNode2::opSub:
            case SNode2::opMul:
            case SNode2::opDiv:
                if (ltr == &vm.GetITraitsUInt() && rtr == &vm.GetITraitsUInt())
                    result = &vm.GetITraitsUInt();
                else if (vm.IsFixedNumType(*ltr) && vm.IsFixedNumType(*rtr))
                    result = &vm.GetITraitsSInt();
                else
                    result = &vm.GetITraitsNumber();
                break;
            case SNode2::opModulo:
                result = &vm.GetITraitsNumber();
                break;
            case SNode2::opBitOr:
            case SNode2::opBitXOr:
            case SNode2::opBitAnd:
            case SNode2::opLSchift:
            case SNode2::opRSchift:
                result = &vm.GetITraitsSInt();
                break;
            case SNode2::opURSchift:
                result = &vm.GetITraitsUInt();
                break;
            /*
            case SNode2::opAssign:
            case SNode2::opNextValue:
            case SNode2::opAsType:
            case SNode2::opAsTypeLate:
            */
            default:
                break;
            }

            return result;
        }

        const Traits* GetTraits(const VM& vm, const SNode& n)
        {
            const Traits* result = NULL;
            const SNode::EType type = n.GetType();

            switch (type)
            {
            case SNode::tValue:
                result = &vm.GetValueTraits(static_cast<const SNodeValue&>(n).V);
                break;
            case SNode::tVar:
                result = static_cast<const SNodeVar&>(n).Type;
                break;
            case SNode::tN2:
                result = GetTraitsSNode2(vm, static_cast<const SNode2&>(n));
                break;
            case SNode::tN:
                {
                    const SNodeN& nn = static_cast<const SNodeN&>(n);
                    if (nn.IsCall() && nn.RType)
                        result = nn.RType;
                    else
                        result = GetTraits(vm, *static_cast<const SNodeN&>(n).Obj);
                }
                break;
            case SNode::tRT:
                result = static_cast<const SNodeRT&>(n).Type;
                break;
            default:
                break;
            }

            return result;
        }

        //////////////////////////////////////////////////////////////////////////
        SNodeValue::~SNodeValue()
        {
        }

        void SNodeValue::Accept(AOT::Visitor& v)
        {
            v.VisitSNodeValue(*this);
        }

        SNodeValue::SNodeValue(const Value& v) : SNode(SNode::tValue SF_DEBUG_ARG(0)), V(v)
        {
            //         SF_ASSERT(v.GetTraceNullType() == Value::Null || !v.IsTraitsStrict());
        }

        //////////////////////////////////////////////////////////////////////////
        void SNodeName::Accept(AOT::Visitor& v)
        {
            v.VisitSNodeName(*this);
        }

        //////////////////////////////////////////////////////////////////////////
        void SNodeComment::Accept(AOT::Visitor& v)
        {
            v.VisitSNodeComment(*this);
        }

        //////////////////////////////////////////////////////////////////////////
        SNodeVar::SNodeVar(const Value& n, const SPtr<const Traits>& t, bool _this) 
            : SNode(SNode::tVar SF_DEBUG_ARG(0))
            , This(_this)
            , Dynamic(false)
            , Name(n)
            , Type(t)
        {
        }
        SNodeVar::SNodeVar(const SPtr<SNode>& pref, const Value& n, const SPtr<const Traits>& t) 
            : SNode(SNode::tVar SF_DEBUG_ARG(0))
            , This(false)
            , Dynamic(false)
            , Prefix(pref)
            , Name(n)
            , Type(t)
        {
        }

        SNodeVar::~SNodeVar()
        {
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
        SNode1::~SNode1()
        {
        }

        void SNode1::Accept(AOT::Visitor& v)
        {
            v.VisitSNode1(*this);
        }

        //////////////////////////////////////////////////////////////////////////
        SNode2::SNode2(const Value& l, OP op, const Value& r SF_DEBUG_ARG(UPInt addr))
            : SNode(SNode::tN2 SF_DEBUG_ARG(addr))
            , Op(op)
            , L(RetrieveNode(l))
            , R(RetrieveNode(r))
        {
            SF_ASSERT(!l.IsTraitsStrict());
            SF_ASSERT(!r.IsTraitsStrict());
        }

        SNode2::SNode2(const SPtr<SNode>& l, OP op, const Value& r SF_DEBUG_ARG(UPInt addr))
            : SNode(SNode::tN2 SF_DEBUG_ARG(addr))
            , Op(op)
            , L(l)
            , R(RetrieveNode(r))
        {
            /* For debugging.
            Value::KindType rk = r.GetKind();
            rk = rk;
            bool b = r.IsTraitsStrict();
            b = b;
            */
            SF_ASSERT(r.GetTraceNullType() == Value::Null || !r.IsTraitsStrict());
        }

        SNode2::SNode2(const Value& l, OP op, const SPtr<SNode>& r SF_DEBUG_ARG(UPInt addr))
            : SNode(SNode::tN2 SF_DEBUG_ARG(addr))
            , Op(op)
            , L(RetrieveNode(l))
            , R(r)
        {
            SF_ASSERT(!l.IsTraitsStrict());
        }

        SNode2::SNode2(const SPtr<SNode>& l, OP op, const SPtr<SNode>& r SF_DEBUG_ARG(UPInt addr))
            : SNode(SNode::tN2 SF_DEBUG_ARG(addr))
            , Op(op)
            , L(l)
            , R(r)
        {
        }

        SNode2::~SNode2()
        {
        }

        void SNode2::Accept(AOT::Visitor& v)
        {
            v.VisitSNode2(*this);
        }

        //////////////////////////////////////////////////////////////////////////
        SNodeBoolExpr::SNodeBoolExpr(OP op, const Value& l, const Value& r SF_DEBUG_ARG(UPInt addr))
            : SNode(SNode::tBoolExpr SF_DEBUG_ARG(addr))
            , Op(op)
            , L(RetrieveNode(l))
            , R(RetrieveNode(r))
        {
        }

        SNodeBoolExpr::SNodeBoolExpr(OP op, const SPtr<SNode>& l, const SPtr<SNode>& r SF_DEBUG_ARG(UPInt addr))
            : SNode(SNode::tBoolExpr SF_DEBUG_ARG(addr))
            , Op(op)
            , L(l)
            , R(r)
        {
        }

        SNodeBoolExpr::SNodeBoolExpr(OP op, const SPtr<SNode>& l, const Value& r SF_DEBUG_ARG(UPInt addr))
            : SNode(SNode::tBoolExpr SF_DEBUG_ARG(addr))
            , Op(op)
            , L(l)
            , R(RetrieveNode(r))
        {
        }

        SNodeBoolExpr::SNodeBoolExpr(OP op, const Value& l, const SPtr<SNode>& r SF_DEBUG_ARG(UPInt addr))
            : SNode(SNode::tBoolExpr SF_DEBUG_ARG(addr))
            , Op(op)
            , L(RetrieveNode(l))
            , R(r)
        {
        }

        SNodeBoolExpr::OP SNodeBoolExpr::GetNegation(OP op)
        {
            OP result = opLT;

            switch (op)
            {
            case opNLT:
                result = opLT;
                break;
            case opNLE:
                result = opLE;
                break;
            case opNGT:
                result = opGT;
                break;
            case opNGE:
                result = opGE;
                break;
            case opEQ:
                result = opNE;
                break;

            case opGE:
                result = opLT;
                break;
            case opGT:
                result = opLE;
                break;
            case opLE:
                result = opGT;
                break;
            case opLT:
                result = opGE;
                break;
            case opNE:
                result = opEQ;
                break;

            case opStrictEQ:
                result = opStrictNE;
                break;
            case opStrictNE:
                result = opStrictEQ;
                break;
            default:
                break;
            }

            return result;
        }

        void SNodeBoolExpr::Accept(AOT::Visitor& v)
        {
            v.VisitSNodeBoolExpr(*this);
        }

        //////////////////////////////////////////////////////////////////////////
        void SNodeIF::Accept(AOT::Visitor& v)
        {
            v.VisitSNodeIF(*this);
        }

        //////////////////////////////////////////////////////////////////////////
        void SNodeSwitch::Accept(AOT::Visitor& v)
        {
            v.VisitSNodeSwitch(*this);
        }

        //////////////////////////////////////////////////////////////////////////
        void SNodeLoop::Accept(AOT::Visitor& v)
        {
            v.VisitSNodeLoop(*this);
        }

        //////////////////////////////////////////////////////////////////////////
        void SNodeCatch::Accept(AOT::Visitor& v)
        {
            v.VisitSNodeCatch(*this);
        }

        //////////////////////////////////////////////////////////////////////////
        void SNodeAbrupt::Accept(AOT::Visitor& v)
        {
            v.VisitSNodeAbrupt(*this);
        }

        //////////////////////////////////////////////////////////////////////////
        void SNodeException::Accept(AOT::Visitor& v)
        {
            v.VisitSNodeException(*this);
        }

        //////////////////////////////////////////////////////////////////////////
        void SNodeBlock::Accept(AOT::Visitor& v)
        {
            v.VisitSNodeBlock(*this);
        }

        Scaleform::UInt32 SNodeBlock::GetLabel() const
        {
            return GetBlock().GetFrom();
        }

        //////////////////////////////////////////////////////////////////////////
        SNodeArgs::SNodeArgs(OP op, const SPtr<InstanceTraits::Function>& fitr SF_DEBUG_ARG(UPInt addr))
        : SNode(SNode::tArgs SF_DEBUG_ARG(addr))
        , Op(op)
        , FITR(fitr)
        {
            // No args.
        }

        SNodeArgs::SNodeArgs(OP op, UInt32 argc, const Value* argv SF_DEBUG_ARG(UPInt addr))
        : SNode(SNode::tArgs SF_DEBUG_ARG(addr))
        , Op(op)
        {
            for (UPInt i = 0; i < argc; ++i)
                Args.PushBack(RetrieveNode(argv[i]));
        }

        SNodeArgs::~SNodeArgs()
        {
        }

        void SNodeArgs::Accept(AOT::Visitor& v)
        {
            v.VisitSNodeArgs(*this);
        }

        //////////////////////////////////////////////////////////////////////////
        SNodeN::SNodeN(const Value& _this, OP op, const SPtr<SNode>& o SF_DEBUG_ARG(UPInt addr))
            : SNode(SNode::tN SF_DEBUG_ARG(addr))
            , Op(op)
            , Obj(o)
            , RType(NULL)
			, This(_this)
        {
        }

        SNodeN::SNodeN(const Value& _this, OP op, const SPtr<SNode>& o, UInt32 argc, const Value* argv SF_DEBUG_ARG(UPInt addr))
            : SNode(SNode::tN SF_DEBUG_ARG(addr))
            , Op(op)
            , Obj(o)
            , RType(NULL)
			, This(_this)
        {
            for (UPInt i = 0; i < argc; ++i)
                Args.PushBack(RetrieveNode(argv[i]));
        }

        SNodeN::SNodeN(const Value& _this, OP op, const Value& o, UInt32 argc, const Value* argv SF_DEBUG_ARG(UPInt addr))
            : SNode(SNode::tN SF_DEBUG_ARG(addr))
            , Op(op)
            , Obj(RetrieveNode(o))
            , RType(NULL)
			, This(_this)
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
            if (Obj)
                Obj->ReleaseAST();

            const UPInt size = Args.GetSize();
            for (UPInt i = 0; i < size; ++i)
            {
                if (Args[i])
                    Args[i]->ReleaseAST();
            }
        }

        //////////////////////////////////////////////////////////////////////////
        void SNodeRT::Accept(AOT::Visitor& v)
        {
            v.VisitSNodeRT(*this);
        }

        //////////////////////////////////////////////////////////////////////////
        SNodeRTV::~SNodeRTV()
        {
        }

        void SNodeRTV::Accept(AOT::Visitor& v)
        {
            v.VisitSNodeRTV(*this);
        }

        void SNodeRTV::ReleaseASTChildren()
        {
            if (RT)
                RT->ReleaseAST();

            if (V)
                V->ReleaseAST();
        }

        //////////////////////////////////////////////////////////////////////////
        SNodePhi::SNodePhi(const SPtr<SNode>& n SF_DEBUG_ARG(UPInt addr))
            : SNode(SNode::tPhi SF_DEBUG_ARG(addr))
        {
            SF_ASSERT(n);
            Elems.PushBack(n);
        }

        SNodePhi::SNodePhi(const SPtr<SNode>& n1, const SPtr<SNode>& n2 SF_DEBUG_ARG(UPInt addr))
            : SNode(SNode::tPhi SF_DEBUG_ARG(addr))
        {
            if (n1.GetPtr() != n2.GetPtr())
            {
                SF_ASSERT(n1);
                Elems.PushBack(n1);
                SF_ASSERT(n2);
                Elems.PushBack(n2);
            }
            else
                // There is no reason to create a new Phi-node if values are equal.
                SF_ASSERT(false);
        }

        void SNodePhi::Accept(AOT::Visitor& v)
        {
            v.VisitSNodePhi(*this);
        }

        void SNodePhi::AddSNode(const SPtr<SNode>& n)
        {
            SNode* p = n.GetPtr();
            const UPInt size = Elems.GetSize();

            for (UPInt i = 0; i < size; ++i)
            {
                if (p == Elems[i].GetPtr())
                    // We already have this node in a list.
                    return;
            }

            Elems.PushBack(n);
        }

        //////////////////////////////////////////////////////////////////////////
        Visitor::~Visitor()
        {
        }

        void Visitor::VisitSNodeValue(SNodeValue& n)
        {
            SF_UNUSED1(n);
        }

        void Visitor::VisitSNodeName(SNodeName& n)
        {
            SF_UNUSED1(n);
        }

        void Visitor::VisitSNodeComment(SNodeComment& n)
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

        void Visitor::VisitSNodeBoolExpr(SNodeBoolExpr& n)
        {
            SF_UNUSED1(n);
        }

        void Visitor::VisitSNodeIF(SNodeIF& n)
        {
            SF_UNUSED1(n);
        }

        void Visitor::VisitSNodeSwitch(SNodeSwitch& n)
        {
            SF_UNUSED1(n);
        }

        void Visitor::VisitSNodeLoop(SNodeLoop& n)
        {
            SF_UNUSED1(n);
        }

        void Visitor::VisitSNodeException(SNodeException& n)
        {
            SF_UNUSED1(n);
        }

        void Visitor::VisitSNodeBlock(SNodeBlock& n)
        {
            SF_UNUSED1(n);
        }

        void Visitor::VisitSNodeArgs(SNodeArgs& n)
        {
            SF_UNUSED1(n);
        }

        void Visitor::VisitSNodeN(SNodeN& n)
        {
            SF_UNUSED1(n);
        }

        void Visitor::VisitSNodeCatch(SNodeCatch& n)
        {
            SF_UNUSED1(n);
        }

        void Visitor::VisitSNodeAbrupt(SNodeAbrupt& n)
        {
            SF_UNUSED1(n);
        }

        void Visitor::VisitSNodeRT(SNodeRT& n)
        {
            SF_UNUSED1(n);
        }

        void Visitor::VisitSNodeRTV(SNodeRTV& n)
        {
            SF_UNUSED1(n);
        }

        void Visitor::VisitSNodePhi(SNodePhi& n)
        {
            SF_UNUSED1(n);
        }

        ///////////////////////////////////////////////////////////////////////////
        void SetLoopCondVisitor::VisitSNodeLoop(SNodeLoop& n)
        {
            n.Expr = Expr;

            // Set parent of this expression.
            Expr->SetParent(&n);
        }

        void SetLoopCondVisitor::VisitSNodeBlock(SNodeBlock& n)
        {
            if (n.IsArtificial() && !n.Elems.IsEmpty())
                n.Elems[0]->Accept(*this);
        }

        ///////////////////////////////////////////////////////////////////////////
        TraceILVisitor2::TraceILVisitor2(const VM& vm, UInt32 level)
        : Plain(false)
        , Level(level)
        , VMRef(vm)
        {
            Buf.SetGrowSize(4 * 1024);
        }

        void TraceILVisitor2::VisitSNodeValue(SNodeValue& n)
        {
            if (n.V.IsInstanceTraits())
            {
                Buf += "Instance Traits: ";
                Buf += n.V.GetInstanceTraits().GetName().ToCStr();
            }
            else if (n.V.IsClassTraits())
            {
                Buf += "Class Traits: ";
                Buf += n.V.GetClassTraits().GetName().ToCStr();
            }
            else
                Buf += AS3::AsString(n.V);
        }

        void TraceILVisitor2::VisitSNodeName(SNodeName& n)
        {
            Buf.AppendString(n.Name.ToCStr(), n.Name.GetSize());
        }

        void TraceILVisitor2::VisitSNodeVar(SNodeVar& n)
        {
            if (n.IsDynamic())
            {
                if (!n.Name.Convert2String(Buf))
                    // Exception.
                    // This should never happen.
                    SF_ASSERT(false);

                return;
            }

            if (n.Prefix && !IsThisVar(*n.Prefix))
            {
                n.Prefix->Accept(*this);
                Buf.AppendString("->");
            }

            if (n.Name.IsPrimitive())
            {
                if (!n.Name.Convert2String(Buf))
                    // Exception.
                    // This should never happen.
                    SF_ASSERT(false);
            }
            else
            {
#if 0
                const Traits& tr = GetVM().GetValueTraits(n.Name);

                Buf.AppendString("SF_HEAP_AUTO_NEW(this) ");

                if (tr.IsClassTraits())
                    Buf.AppendString("ClassTraits::");
                else
                    Buf.AppendString("InstanceTraits::");

                const ASString name = GetVM().GetValueTraits(n.Name).GetName();

                Buf.AppendString(name.ToCStr(), name.GetSize());
#else
                const ASString name = GetVM().GetValueTraits(n.Name).GetName();

                Buf.AppendString("GetITraits");
                Buf.AppendString(name.ToCStr(), name.GetSize());
                Buf.AppendString("().MakeInstance(");
                Buf.AppendString("GetITraits");
                Buf.AppendString(name.ToCStr(), name.GetSize());
                Buf.AppendString("())");
#endif
            }
        }

        void TraceILVisitor2::VisitSNodeMbrAcc(SNodeMbrAcc& n)
        {
            if (n.Obj && !IsThisVar(*n.Obj))
            {
                n.Obj->Accept(*this);
                Buf.AppendString("->");
            }

            if (n.Mbr)
                n.Mbr->Accept(*this);
        }

        void TraceILVisitor2::VisitSNode1(SNode1& n)
        {
            const char* op = NULL;;

            switch (n.Op)
            {
            case SNode1::opThrow:
                op = "throw ";
                break;
            case SNode1::opNot:
                op = "!";
                break;
            case SNode1::opNegate:
                op = "-";
                break;
            case SNode1::opIncr:
                op = "++";
                break;
            case SNode1::opDecr:
                op = "--";
                break;
            case SNode1::opConvertS:
                op = "ConvertS";
                break;
            case SNode1::opCoerceS:
                op = "CoerceS";
                break;
            case SNode1::opTypeOf: 
                op = "TypeOf";
                break;
            case SNode1::opBitNot:
                op = "opBitNot";
                break;
            case SNode1::opDelete:
                op = "delete";
                break;
            case SNode1::opNewClass:
                op = "new class ";
                break;
            default:
                SF_ASSERT(false);
                break;
            };

            SF_ASSERT(op);
            Buf.AppendString(op);
            if (n.V)
            {
                // There is no need to check precedence of n.V because
                // precedence of n.Op will always be lower.

                const bool cexpr = GetPrecedence(*n.V) > GetPrecedence(n.Op);
                //                 const bool cexpr = IsCompoundExpr(*n.V);

                if (cexpr)
                    Buf.AppendChar('(');
                n.V->Accept(*this);
                if (cexpr)
                    Buf.AppendChar(')');
            }
        }

        void TraceILVisitor2::VisitSNode2(SNode2& n)
        {
            const char* op = NULL;

            const bool cexprL = GetPrecedence(*n.L) > GetPrecedence(n.Op);

            if (cexprL)
                Buf.AppendChar('(');
            n.L->Accept(*this);
            if (cexprL)
                Buf.AppendChar(')');

            switch (n.Op)
            {
            case SNode2::opAssign:
                op = "=";
                break;
            case SNode2::opAdd:
                op = "+";
                break;
            case SNode2::opSub:
                op = "-";
                break;
            case SNode2::opMul:
                op = "*";
                break;
            case SNode2::opDiv:
                op = "/";
                break;
            case SNode2::opModulo:
                op = "%";
                break;
            case SNode2::opBitOr:
                op = "|";
                break;
            case SNode2::opBitXOr:
                op = "^";
                break;
            case SNode2::opBitAnd:
                op = "&";
                break;
            case SNode2::opLSchift:
                op = "<<";
                break;
            case SNode2::opRSchift:
                op = ">>";
                break;
            case SNode2::opURSchift:
                op = ">>";
                break;
            case SNode2::opNextValue:
                op = "NextValue";
                break;
            case SNode2::opAsType:
                op = "opAsType";
                break;
            case SNode2::opAsTypeLate:
                op = "opAsTypeLate";
                break;
            default:
                SF_ASSERT(false);
                break;
            }

            Buf.AppendChar(' ');
            SF_ASSERT(op);
            Buf.AppendString(op);
            Buf.AppendChar(' ');

            const bool cexprR = GetPrecedence(*n.R) > GetPrecedence(n.Op);

            if (cexprR)
                Buf.AppendChar('(');
            n.R->Accept(*this);
            if (cexprR)
                Buf.AppendChar(')');
        }

        void TraceILVisitor2::VisitSNodeBoolExpr(SNodeBoolExpr& n)
        {
            const bool cexprL = GetPrecedence(*n.L) > GetPrecedence(n.Op);

            if (cexprL)
                Buf.AppendChar('(');
            n.L->Accept(*this);
            if (cexprL)
                Buf.AppendChar(')');

            Buf.AppendChar(' ');

            const char* op = NULL;
            switch (n.Op)
            {
            case SNodeBoolExpr::opAnd:
                op = "&&";
                break;
            case SNodeBoolExpr::opOr:
                op = "||";
                break;
            case SNodeBoolExpr::opNLT:
                op = ">=";
                break;
            case SNodeBoolExpr::opNLE:
                op = ">";
                break;
            case SNodeBoolExpr::opNGT:
                op = "<=";
                break;
            case SNodeBoolExpr::opNGE:
                op = "<";
                break;
            case SNodeBoolExpr::opEQ:
                op = "==";
                break;
            case SNodeBoolExpr::opGE:
                op = ">=";
                break;
            case SNodeBoolExpr::opGT:
                op = ">";
                break;
            case SNodeBoolExpr::opLE:
                op = "<=";
                break;
            case SNodeBoolExpr::opLT:
                op = "<";
                break;
            case SNodeBoolExpr::opNE:
                op = "!=";
                break;
            case SNodeBoolExpr::opStrictEQ:
                op = "==";
                break;
            case SNodeBoolExpr::opStrictNE:
                op = "!=";
                break;
            case SNodeBoolExpr::opInstanceOf:
                op = "InstanceOf";
                break;
            case SNodeBoolExpr::opIn:
                op = "In";
                break;
            case SNodeBoolExpr::opIsTypeLate:
                op = "IsTypeLate";
                break;
            }

            SF_ASSERT(op != NULL);
            Buf.AppendString(op);

            Buf.AppendChar(' ');

            const bool cexprR = GetPrecedence(*n.R) > GetPrecedence(n.Op);

            if (cexprR)
                Buf.AppendChar('(');
            n.R->Accept(*this);
            if (cexprR)
                Buf.AppendChar(')');
        }

        void TraceILVisitor2::VisitSNodeIF(SNodeIF& n)
        {
            Buf.AppendString(" if");

            if (n.GetTrue())
                Buf.AppendString(" stmt");

            if (n.GetFalse())
                Buf.AppendString(" else stmt");
        }

        void TraceILVisitor2::VisitSNodeSwitch(SNodeSwitch& n)
        {
            SF_UNUSED1(n);
            Buf.AppendString(" switch stmt");
        }

        void TraceILVisitor2::VisitSNodeLoop(SNodeLoop& n)
        {
            switch (n.Op)
            {
            case SNodeLoop::opWhile:
                Buf.AppendString(" while loop");
                break;
            case SNodeLoop::opDoWhile:
                Buf.AppendString(" do-while loop");
                break;
            default:
                Buf.AppendString(" loop");
                break;
            }
        }

        void TraceILVisitor2::VisitSNodeException(SNodeException& n)
        {
            SF_UNUSED1(n);
            Buf.AppendString(" try catch");
        }

        void TraceILVisitor2::VisitSNodeBlock(SNodeBlock& n)
        {
            SF_UNUSED1(n);
            Buf.AppendString(" block");
        }

        void TraceILVisitor2::VisitSNodeArgs(SNodeArgs& n)
        {
            const char* name = NULL;
            switch (n.Op)
            {
            case SNodeArgs::opNewArray:
                name = "new Array";
                break;
            case SNodeArgs::opNewObject:
                name = "new Object";
                break;
            case SNodeArgs::opNewFunction:
                name = "new Function";
                break;
            default:
                break;
            }

            Buf.AppendString(name);
        }

        void TraceILVisitor2::VisitSNodeN(SNodeN& n)
        {
            switch (n.Op)
            {
            case SNodeN::opConstructProp:
                Buf.AppendString("new ");
                n.Obj->Accept(*this);
                break;
            case SNodeN::opCallType:
            case SNodeN::opCall:
            case SNodeN::opCallGet:
            case SNodeN::opCallSet:
                Buf.AppendChar(' ');
                n.Obj->Accept(*this);
                break;
            default:
                break;
            }

            Buf.AppendString("()");
        }

        void TraceILVisitor2::VisitSNodeAbrupt(SNodeAbrupt& n)
        {
            switch (n.Op)
            {
            case SNodeAbrupt::opReturn:
                Buf.AppendString(" return");
                break;
            case SNodeAbrupt::opGoto:
                if (n.Dest)
                {
                    LongFormatter f(n.Label);
                    f.Convert();
                    const StringDataPtr r = f.GetResult();

                    Buf.AppendString(" goto label_");
                    Buf.AppendString(r.ToCStr(), r.GetSize());
                }
                break;
            case SNodeAbrupt::opBreak:
                Buf.AppendString(" break");
                break;
            case SNodeAbrupt::opContinue:
                Buf.AppendString(" continue");
                break;
            }
        }

        void TraceILVisitor2::VisitSNodeRT(SNodeRT& n)
        {
            Format(Buf, "reg{0}", n.Num);
        }

        void TraceILVisitor2::VisitSNodeRTV(SNodeRTV& n)
        {
            if (n.GetRT())
                n.GetRT()->Accept(*this);

            Buf.AppendString(" = ");

            if (n.GetValue())
                n.GetValue()->Accept(*this);
        }

    } // namespace AOT

}}} // namespace Scaleform { namespace GFx { namespace AS3 

#endif // SF_AS3_AOTC
