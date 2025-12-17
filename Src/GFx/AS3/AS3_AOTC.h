/**************************************************************************

Filename    :   AS3_AOTC.h
Content     :   
Created     :   Oct, 2011
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_AS3_AOTC_H
#define INC_AS3_AOTC_H

#include "AS3_Value.h"

#if defined(SF_AS3_AOTC)

namespace Scaleform { namespace GFx { namespace AS3 
{

// Forward declaration.
//

namespace InstanceTraits
{
    class Function;
}

namespace AOT
{
    class Visitor;
    class RefCountVisitor;
}

namespace TR
{
    class Block;
    class State;
}

///
class SNode : public NewOverrideBase<Mem_Stat>
{
    friend class AOT::RefCountVisitor;

public:
    enum EType {
        tValue, 
        tName, 
        tComment, 
        tVar, 
        tMbrAcc, 
        tN1, 
        tN2, 
        tBoolExpr, 
        tIF, 
        tSwitch, 
        tLoop, 
        tCatch, 
        tException, 
        tBlock,
        tAbrupt,
        tArgs,
        tN,
        tRT,
        tRTV,
        tPhi,
    };

public:
    SNode(EType t SF_DEBUG_ARG(UPInt addr))
        : RefCount(1)
        , RefCountAST(0)
        , Type(t)
        , Parent(NULL)
        SF_DEBUG_ARG(Addr(addr))
    {
    }
    virtual ~SNode();

public:
    void    AddRef()
    {
        RefCount++;
        AddRefAST();
    }
    void    Release()
    {
        ReleaseAST();
        if (--RefCount == 0)
            delete this;
    }

public:
    virtual void Accept(AOT::Visitor& v) = 0;

    //
    static SPtr<SNode> RetrieveNode(const Value& v);

    //
    SNode* GetParent() const
    {
        return Parent;
    }
    void SetParent(SNode* p)
    {
        Parent = p;
    }

    //
    EType GetType() const
    {
        return Type;
    }

    //
    bool Is(EType t) const
    {
        return Type == t;
    }

    //
    UInt32  GetRefCountAST() const
    {
        return RefCountAST;
    }

    //
    SF_DEBUG_CODE(UPInt GetAddr() const { return Addr; } )

public:
    void    AddRefAST()
    {
        if (RefCountAST)
            RefCountAST++;
    }
    void    ReleaseAST()
    {
        if (RefCountAST > 0)
            --RefCountAST;
    }

private:
    // Total reference count.
    UInt32  RefCount;
    // Reference count in AST.
    UInt32  RefCountAST;
    EType   Type;
    SNode*  Parent;
    SF_DEBUG_CODE(UPInt   Addr;)
}; // class SNode

SF_INLINE
void Value::AddRefSNode() const
{
    SF_ASSERT(value.VS._2.VSNode);
    value.VS._2.VSNode->AddRef();
}

SF_INLINE
void Value::ReleaseSNode() const
{
    SF_ASSERT(value.VS._2.VSNode);
    value.VS._2.VSNode->Release();
}


namespace AOT
{
    /////////////////////////////////////////////////////////////////
    struct SNodeValue : public SNode
    {
        SNodeValue(const Value& v);
        virtual ~SNodeValue();

        virtual void Accept(AOT::Visitor& v);

        bool operator ==(const SNodeValue& other) const
        {
            return V == other.V;
        }

        Value   V;
    }; // struct SNodeValue

    struct SNodeName : public SNode
    {
        SNodeName(const String& n) : SNode(SNode::tName SF_DEBUG_ARG(0)), Name(n) {}
        SNodeName(const SNodeName& other) : SNode(SNode::tName SF_DEBUG_ARG(0)), Name(other.Name) {}

        virtual void Accept(AOT::Visitor& v);

        bool operator ==(const SNodeName& other) const
        {
            return Name == other.Name;
        }

        const String    Name;

    private:
        SNodeName& operator=(const SNodeName& other);
    }; // struct SNodeName

    struct SNodeComment : public SNode
    {
        SNodeComment(const String& n) : SNode(SNode::tComment SF_DEBUG_ARG(0)), Txt(n) {}
        SNodeComment(const SNodeComment& other) : SNode(SNode::tComment SF_DEBUG_ARG(0)), Txt(other.Txt) {}

        virtual void Accept(AOT::Visitor& v);

        bool operator ==(const SNodeComment& other) const
        {
            return Txt == other.Txt;
        }

        const String    Txt;

    private:
        SNodeComment& operator=(const SNodeComment& other);
    }; // struct SNodeComment

    class SNodeVar : public SNode
    {
    public:
        SNodeVar(const Value& n, const SPtr<const Traits>& t, bool _this = false);
        SNodeVar(const SPtr<SNode>& pref, const Value& n, const SPtr<const Traits>& t);
        virtual ~SNodeVar();

    public:
        virtual void Accept(AOT::Visitor& v);

        bool operator ==(const SNodeVar& other) const
        {
            // Let's skip Prefix for the time being.
            return This == other.This && Dynamic == other.Dynamic && Name == other.Name;
        }

        //
        void SetThis(bool flag = true)
        {
            This = flag;
        }
        bool IsThis() const
        {
            return This;
        }

        //
        void SetDynamic(bool flag = true)
        {
            Dynamic = flag;
        }
        bool IsDynamic() const
        {
            return Dynamic;
        }

    protected:
        bool                This:1;
        // true if this variable cannot be resolved at compile time.
        bool                Dynamic:1;

    private:
        SNodeVar& operator =(const SNodeVar&);

    public:
        SPtr<SNode>         Prefix;
        Value               Name;
        SPtr<const Traits>  Type;
    }; // struct SNodeVar

    // Member access.
    // This node is very similar to SNode2, 
    // but it is convenient to have a separate node.
    struct SNodeMbrAcc : public SNode
    {
        enum Type {tAS3, tCppRef, tCppPtr};

        SNodeMbrAcc(const SPtr<SNode>& o, const SPtr<SNode>& m, Type t SF_DEBUG_ARG(UPInt addr))
        : SNode(SNode::tMbrAcc SF_DEBUG_ARG(addr))
        , T(t)
        , Obj(o)
        , Mbr(m)
        {
        }

        virtual void Accept(AOT::Visitor& v);

        bool Is(Type t) const
        {
            return T == t;
        }

        Type        T;
        SPtr<SNode> Obj;
        SPtr<SNode> Mbr;
    }; // struct SNodeMbrAcc

    // Unary operation.
    struct SNode1 : public SNode
    {
        enum OP 
        {
            opThrow, 
            opNot, // !!! TBD: This should be moved into SNodeBoolExpr.
            opNegate, 
            opIncr, 
            opDecr, 
            opConvertS, 
            opCoerceS, 
            opTypeOf, 
            opBitNot, 
            opDelete,
            opNewClass,
        };

        SNode1(const Value& v, OP op SF_DEBUG_ARG(UPInt addr))
            : SNode(SNode::tN1 SF_DEBUG_ARG(addr))
            , Op(op)
            , V(RetrieveNode(v))
        {
        }
        SNode1(const SPtr<SNode>& v, OP op SF_DEBUG_ARG(UPInt addr))
            : SNode(SNode::tN1 SF_DEBUG_ARG(addr))
            , Op(op)
            , V(v)
        {
        }

        virtual ~SNode1();
        virtual void Accept(AOT::Visitor& v);

        bool Is(OP op) const
        {
            return Op == op;
        }

        OP          Op;
        SPtr<SNode> V;
    }; // struct SNode1

    // Binary operation.
    struct SNode2 : public SNode
    {
        enum OP 
        {
            opAssign,
            opAssignAdd,
            opAssignSub,
            opAssignMul,
            opAssignDiv,
            opAssignModulo,
            opAdd,
            opSub,
            opMul,
            opDiv,
            opModulo,
            opBitOr,
            opBitXOr,
            opBitAnd,
            opLSchift,
            opRSchift,
            opURSchift,
            opNextValue,
            opAsType,
            opAsTypeLate,
        };

        SNode2(const Value& l, OP op, const Value& r SF_DEBUG_ARG(UPInt addr));
        SNode2(const SPtr<SNode>& l, OP op, const Value& r SF_DEBUG_ARG(UPInt addr));
        SNode2(const Value& l, OP op, const SPtr<SNode>& r SF_DEBUG_ARG(UPInt addr));
        SNode2(const SPtr<SNode>& l, OP op, const SPtr<SNode>& r SF_DEBUG_ARG(UPInt addr));
        virtual ~SNode2();

        virtual void Accept(AOT::Visitor& v);

        bool Is(OP op) const
        {
            return Op == op;
        }

        OP          Op;
        SPtr<SNode> L;
        SPtr<SNode> R;
    }; // struct SNode2

    // Boolean Expression.
    struct SNodeBoolExpr : public SNode
    {
        enum OP 
        {
            opAnd,
            opOr,
            opNLT,
            opNLE,
            opNGT,
            opNGE,
            opEQ,
            opGE,
            opGT,
            opLE,
            opLT,
            opNE,
            opStrictEQ,
            opStrictNE,
            opInstanceOf, 
            opIn,
            opIsTypeLate,
        };

        SNodeBoolExpr(OP op, const Value& l, const Value& r SF_DEBUG_ARG(UPInt addr));
        SNodeBoolExpr(OP op, const SPtr<SNode>& l, const SPtr<SNode>& r SF_DEBUG_ARG(UPInt addr));
        SNodeBoolExpr(OP op, const SPtr<SNode>& l, const Value& r SF_DEBUG_ARG(UPInt addr));
        SNodeBoolExpr(OP op, const Value& l, const SPtr<SNode>& r SF_DEBUG_ARG(UPInt addr));

        static OP GetNegation(OP op);

        virtual void Accept(AOT::Visitor& v);

        bool Is(OP op) const
        {
            return Op == op;
        }

        OP          Op;
        SPtr<SNode> L;
        SPtr<SNode> R;
    }; // struct SNodeBoolExpr

    // IF operation.
    struct SNodeIF : public SNode
    {
        // True/False should be initialized later.
        SNodeIF(const SPtr<SNode>& expr SF_DEBUG_ARG(UPInt addr))
            : SNode(SNode::tIF SF_DEBUG_ARG(addr))
            , LabelEnd(0)
            , Expr(expr) {}

        virtual void Accept(AOT::Visitor& v);

        //
        const SPtr<SNode>& GetTrue() const
        {
            return True;
        }
        SPtr<SNode>& GetTrueNC()
        {
            return True;
        }
        void SetTrue(const SPtr<SNode>& n)
        {
            SF_ASSERT(!n || n->GetType() == SNode::tBlock);
            True = n;
        }

        //
        const SPtr<SNode>& GetFalse() const
        {
            return False;
        }
        SPtr<SNode>& GetFalseNC()
        {
            return False;
        }
        void SetFalse(const SPtr<SNode>& n)
        {
            SF_ASSERT(!n || n->GetType() == SNode::tBlock);
            False = n;
        }

        //
        void SwapTrueFalse()
        {
            Alg::Swap(True, False);
        }

    private:
        SNodeIF& operator=(const SNodeIF&);

    public:
        UInt32      LabelEnd;
        SPtr<SNode> Expr;

    private:
        SPtr<SNode> True;
        SPtr<SNode> False;
    }; // struct SNodeIF

    struct SNodeSwitch : public SNode
    {
        SNodeSwitch(SF_DEBUG_CODE(UPInt addr)) : SNode(SNode::tSwitch SF_DEBUG_ARG(addr)) {}

        struct Record
        {
            SPtr<SNode> CaseCond;
            SPtr<SNode> CaseBody;
        };

        typedef ArrayLH<Record> TCases;

        virtual void Accept(AOT::Visitor& v);

        SPtr<SNode> Cond;
        UInt32      DefaultN;
        TCases      Cases;
    }; // struct SNodeSwitch

    struct SNodeLoop : public SNode
    {
        enum OP {opUnknown, opWhile, opDoWhile, opFor};

        SNodeLoop(UInt32 b, OP op SF_DEBUG_ARG(UPInt addr)) 
            : SNode(SNode::tLoop SF_DEBUG_ARG(addr))
            , Op(op)
            , LabelBegin(b)
            , LabelEnd(0)
        {
        }

        virtual void Accept(AOT::Visitor& v);

        bool Is(OP op) const
        {
            return Op == op;
        }

        OP          Op;
        UInt32      LabelBegin;
        UInt32      LabelEnd;
        SPtr<SNode> Init; // In case of "for" statement.
        SPtr<SNode> Expr;
        // We cannot declare Body as SPtr<SNodeBlock> at the moment.
        SPtr<SNode> Body;
        SPtr<SNode> PostBody; // In case of "for" statement.
    }; // struct SNodeLoop

    struct SNodeCatch : public SNode
    {
        SNodeCatch(SF_DEBUG_CODE(UPInt addr)) : SNode(SNode::tCatch SF_DEBUG_ARG(addr)) {}

        virtual void Accept(AOT::Visitor& v);

        SPtr<SNodeVar>  Var;
        SPtr<SNodeVar>  Type;
        SPtr<SNode>     Body;
    }; // struct SNodeCatch

    struct SNodeException : public SNode
    {
        SNodeException(SF_DEBUG_CODE(UPInt addr)) : SNode(SNode::tException SF_DEBUG_ARG(addr)) {}

        typedef ArrayLH<SPtr<SNodeCatch> >  TCatches;

        virtual void Accept(AOT::Visitor& v);

        SPtr<SNode> Try;
        SPtr<SNode> Finally;
        TCatches    Catches;
    }; // struct SNodeException

    // Basic Block.
    // One entry, one exit.
    struct SNodeBlock : public SNode
    {
        typedef HashSetLH<SNode*>       TPredecessors;
        typedef ArrayLH<SPtr<SNode> >   TElems;

        SNodeBlock()
            : SNode(SNode::tBlock SF_DEBUG_ARG(0))
            , Artificial(false)
            , ForceOutline(false)
            , Block(NULL)
        {
        }
        SNodeBlock(TR::Block& bl SF_DEBUG_ARG(UPInt addr))
            : SNode(SNode::tBlock SF_DEBUG_ARG(addr))
            , Artificial(false)
            , ForceOutline(false)
            , Block(&bl)
        {
        }

        virtual void Accept(AOT::Visitor& v);

    public:
        //
        bool IsArtificial() const
        {
            return Artificial;
        }
        void SetArtificial()
        {
            Artificial = true;
        }

        //
        bool IsForceOutline() const
        {
            return ForceOutline;
        }
        void SetForceOutline()
        {
            ForceOutline = true;
        }

        //
        UInt32 GetLabel() const;

        //
        TR::Block& GetBlock() const
        {
            SF_ASSERT(Block);
            return *Block;
        }

    private:
        SNodeBlock& operator=(const SNodeBlock&);

    private:
        bool        Artificial:1;
        bool        ForceOutline:1;
        TR::Block*  Block;

    public:
        // Nodes, pointing to this block.
        // TPredecessors   Preds;
        TElems      Elems;
    }; // struct SNodeBlock

    struct SNodeAbrupt : public SNode
    {
        enum OP {opReturn, opGoto, opBreak, opContinue};

        SNodeAbrupt(OP op, const SPtr<SNodeBlock>& d, UInt32 l SF_DEBUG_ARG(UPInt addr))
            : SNode(SNode::tAbrupt SF_DEBUG_ARG(addr))
            , Label(l)
            , Op(op)
            , Dest(d)
        {
        }

        virtual void Accept(AOT::Visitor& v);

        bool Is(OP op) const
        {
            return Op == op;
        }

    private:
        SNodeAbrupt& operator=(const SNodeAbrupt&);

    public:
        const UInt32        Label;
        OP                  Op;
        SPtr<SNodeBlock>    Dest;
        SPtr<SNode>         ReturnValue;
    }; // struct SNodeAbrupt

    // Functor with N arguments.
    struct SNodeArgs : public SNode
    {
        typedef ArrayLH<SPtr<SNode> >   TArgs;
        enum OP {opNewArray, opNewObject, opNewFunction};

        SNodeArgs(OP op, const SPtr<InstanceTraits::Function>& fitr SF_DEBUG_ARG(UPInt addr));
        SNodeArgs(OP op, UInt32 argc, const Value* argv SF_DEBUG_ARG(UPInt addr));
        virtual ~SNodeArgs();

        virtual void Accept(AOT::Visitor& v);

        bool Is(OP op) const
        {
            return Op == op;
        }

    private:
        SNodeArgs& operator=(const SNodeArgs&);

    public:
        const OP        Op;
        const SPtr<InstanceTraits::Function> FITR;
        TArgs           Args;
    }; // struct SNodeArgs

    // Functor with object and N arguments.
    struct SNodeN : public SNode
    {
        typedef ArrayLH<SPtr<SNode> >   TArgs;
        // DO NOT reorder.
        enum OP {opConstruct, opConstructProp, opCallType, opCall, opCallGet, opCallSet};

        SNodeN(const Value& _this, OP op, const SPtr<SNode>& o SF_DEBUG_ARG(UPInt addr));
        SNodeN(const Value& _this, OP op, const SPtr<SNode>& o, UInt32 argc, const Value* argv SF_DEBUG_ARG(UPInt addr));
        SNodeN(const Value& _this, OP op, const Value& o, UInt32 argc, const Value* argv SF_DEBUG_ARG(UPInt addr));

        virtual void Accept(AOT::Visitor& v);

        void ReleaseASTChildren();

        bool IsCall() const
        {
            return Op > opCallType;
        }

        bool Is(OP op) const
        {
            return Op == op;
        }

	private:
		SNodeN& operator =(const SNodeN&);

	public:
        OP					Op;
        SPtr<SNode>			Obj;
        // Result type.
        SPtr<const Traits>  RType;
        TArgs				Args;
		Value				This;
    }; // struct SNodeN

    // Register number + type.
    struct SNodeRT : public SNode
    {
        SNodeRT(UInt32 n, const SPtr<const Traits>& t SF_DEBUG_ARG(UPInt addr))
            : SNode(tRT SF_DEBUG_ARG(addr))
            , Declared(false)
            , Num(n)
            , Type(t) 
        {
        }

        virtual void Accept(AOT::Visitor& v);

        bool operator ==(const SNodeRT& other) const
        {
            // Let's skip Prefix for the time being.
            return Num == other.Num && Type == other.Type;
        }

        bool IsDeclared() const
        {
            return Declared;
        }
        void SetDeclared()
        {
            SF_ASSERT(!IsDeclared());
            Declared = true;
        }

    private:
        SNodeRT& operator=(const SNodeRT&);

    private:
        bool    Declared;

    public:
        const UInt32                Num;
        const SPtr<const Traits>    Type;
    }; // struct SNodeRT

    // A triple: register, type, value.
    // The only way to assign a value to a register is to create another instance of the SNodeRTV class.
    struct SNodeRTV : public SNode
    {
        SNodeRTV(const SPtr<SNodeRT>& rt, const SPtr<SNode>& v SF_DEBUG_ARG(UPInt addr)) 
            : SNode(SNode::tRTV SF_DEBUG_ARG(addr)) 
            , CanPropagate_(true)
            , RT(rt)
            , V(v)
        {
        }
        SNodeRTV(const SPtr<SNodeRT>& rt, const Value& v SF_DEBUG_ARG(UPInt addr)) 
            : SNode(SNode::tRTV SF_DEBUG_ARG(addr)) 
            , CanPropagate_(true)
            , RT(rt)
            , V(RetrieveNode(v))
        {
        }
        virtual ~SNodeRTV();

        virtual void Accept(AOT::Visitor& v);

        UInt32 GetRegNum() const
        {
            return RT->Num;
        }
        const SPtr<SNode>& GetValue() const
        {
            return V;
        }
        const SPtr<SNodeRT>& GetRT() const
        {
            return RT;
        }

        void ReleaseASTChildren();

        bool CanPropagate() const
        {
            return CanPropagate_;
        }
        void DoNotPropagate()
        {
            CanPropagate_ = false;
        }

    private:
        SNodeRTV& operator=(const SNodeRTV& other);

    private:
        bool            CanPropagate_;

    public:
        SPtr<SNodeRT>   RT;
        SPtr<SNode>     V;
    }; // struct SNodeRTV

    // Phi node from SSA.
    // Required for data flow analysis.
    struct SNodePhi : public SNode
    {
        typedef ArrayLH<SPtr<SNode> >   TElems;

        SNodePhi(const SPtr<SNode>& n SF_DEBUG_ARG(UPInt addr));
        SNodePhi(const SPtr<SNode>& n1, const SPtr<SNode>& n2 SF_DEBUG_ARG(UPInt addr));

        virtual void Accept(AOT::Visitor& v);

        void AddSNode(const SPtr<SNode>& n);

        const TElems& GetElems() const
        {
            return Elems;
        }

    private:
        TElems  Elems;
    }; // struct SNodePhi


    ///////////////////////////////////////////////////////////////////////////
    template <typename T> struct Type {};
    template <> struct Type<SNodeValue> { enum {value = SNode::tValue }; };
    template <> struct Type<SNodeName> { enum {value = SNode::tName }; };
    template <> struct Type<SNodeComment> { enum {value = SNode::tComment }; };
    template <> struct Type<SNodeVar> { enum {value = SNode::tVar }; };
    template <> struct Type<SNodeMbrAcc> { enum {value = SNode::tVar }; };
    template <> struct Type<SNode1> { enum {value = SNode::tN1 }; };
    template <> struct Type<SNode2> { enum {value = SNode::tN2 }; };
    template <> struct Type<SNodeBoolExpr> { enum {value = SNode::tBoolExpr }; };
    template <> struct Type<SNodeSwitch> { enum {value = SNode::tSwitch}; };
    template <> struct Type<SNodeLoop> { enum {value = SNode::tLoop}; };
    template <> struct Type<SNodeCatch> { enum {value = SNode::tCatch }; };
    template <> struct Type<SNodeException> { enum {value = SNode::tException }; };
    template <> struct Type<SNodeBlock> { enum {value = SNode::tBlock }; };
    template <> struct Type<SNodeAbrupt> { enum {value = SNode::tAbrupt }; };
    template <> struct Type<SNodeArgs> { enum {value = SNode::tArgs }; };
    template <> struct Type<SNodeN> { enum {value = SNode::tN }; };
    template <> struct Type<SNodeRT> { enum {value = SNode::tRT }; };
    template <> struct Type<SNodeRTV> { enum {value = SNode::tRTV }; };
    template <> struct Type<SNodePhi> { enum {value = SNode::tPhi }; };

    ///////////////////////////////////////////////////////////////////////////
    UInt8 GetPrecedence(SNode1::OP op);
    UInt8 GetPrecedence(SNode2::OP op);
    UInt8 GetPrecedence(SNodeBoolExpr::OP op);
    UInt8 GetPrecedence(const SNode& n);

    // Check if a node is a "this" variable.
    bool IsThisVar(const SNode& n);

    const Traits* GetTraits(const VM& vm, const SNode& n);
    InstanceTraits::Traits& GetInstanceTraits(Traits& tr);
    const InstanceTraits::Traits& GetInstanceTraits(const Traits& tr);

    bool IsAnyType(const Traits& tr);
    bool IsAnyType(const VM& vm, SNode& n);

    // SInt32 or UInt32
    bool IsFixedNumType(const VM& vm, SNode& n);
    // SInt32, UInt32, or Number.
    bool IsNumericType(const VM& vm, SNode& n);
    // Number.
    bool IsNumberType(const VM& vm, SNode& n);
    // String.
    bool IsStringType(const VM& vm, SNode& n);

    ///////////////////////////////////////////////////////////////////////////
    // Default implementation won't visit children.
    class Visitor
    {
    public:
        // To get rid of warnings.
        virtual ~Visitor();

    public:
        virtual void VisitSNodeValue(SNodeValue& n);
        virtual void VisitSNodeName(SNodeName& n);
        virtual void VisitSNodeComment(SNodeComment& n);
        virtual void VisitSNodeVar(SNodeVar& n);
        virtual void VisitSNodeMbrAcc(SNodeMbrAcc& n);
        virtual void VisitSNode1(SNode1& n);
        virtual void VisitSNode2(SNode2& n);
        virtual void VisitSNodeBoolExpr(SNodeBoolExpr& n);
        virtual void VisitSNodeIF(SNodeIF& n);
        virtual void VisitSNodeSwitch(SNodeSwitch& n);
        virtual void VisitSNodeLoop(SNodeLoop& n);
        virtual void VisitSNodeException(SNodeException& n);
        virtual void VisitSNodeBlock(SNodeBlock& n);
        virtual void VisitSNodeArgs(SNodeArgs& n);
        virtual void VisitSNodeN(SNodeN& n);
        virtual void VisitSNodeCatch(SNodeCatch& n);
        virtual void VisitSNodeAbrupt(SNodeAbrupt& n);
        virtual void VisitSNodeRT(SNodeRT& n);
        virtual void VisitSNodeRTV(SNodeRTV& n);
        virtual void VisitSNodePhi(SNodePhi& n);
    };

    /////////////////////////////////////////////////////////////////
    // Utility classes.
    class SetLoopCondVisitor : public Visitor
    {
    public:
        SetLoopCondVisitor(SPtr<SNode>& expr) : Expr(expr) {}

    public:
        virtual void VisitSNodeLoop(SNodeLoop& n);
        virtual void VisitSNodeBlock(SNodeBlock& n);

    private:
        SPtr<SNode> Expr;
    };

    // TraceIL stands for "trace in line".
    // This visitor is supposed to trace values and expressions.
    // (Everything that can be printed in one line)
    class TraceILVisitor2 : public Visitor
    {
        typedef Visitor TParent;

    public:
        TraceILVisitor2(const VM& vm, UInt32 level = 0);

    public:
        virtual void VisitSNodeValue(SNodeValue& n);
        virtual void VisitSNodeName(SNodeName& n);
        virtual void VisitSNodeVar(SNodeVar& n);
        virtual void VisitSNodeMbrAcc(SNodeMbrAcc& n);
        virtual void VisitSNode1(SNode1& n);
        virtual void VisitSNode2(SNode2& n);
        virtual void VisitSNodeBoolExpr(SNodeBoolExpr& n);
        virtual void VisitSNodeIF(SNodeIF& n);
        virtual void VisitSNodeSwitch(SNodeSwitch& n);
        virtual void VisitSNodeLoop(SNodeLoop& n);
        virtual void VisitSNodeException(SNodeException& n);
        virtual void VisitSNodeBlock(SNodeBlock& n);
        virtual void VisitSNodeArgs(SNodeArgs& n);
        virtual void VisitSNodeN(SNodeN& n);
        virtual void VisitSNodeAbrupt(SNodeAbrupt& n);
        virtual void VisitSNodeRT(SNodeRT& n);
        virtual void VisitSNodeRTV(SNodeRTV& n);

    public:
        const StringBuffer& GetResult() const { return Buf; }
        const VM& GetVM() const { return VMRef; }

        const Traits* GetTraits(SNode& n) const
        {
            return AOT::GetTraits(GetVM(), n);
        }

    private:
        TraceILVisitor2& operator=(const TraceILVisitor2&);

    protected:
        bool            Plain;
        UInt32          Level;
        const VM&       VMRef;
        StringBuffer    Buf;
    };

    struct InfoCollector
    {
        // typedef HashSetDH<UInt32> TInt32HashSet;
        // Set of used functions.
        typedef HashSetDH<SPtr<InstanceTraits::Function> > TFunctHashSet;
        // Map Method Index to name.
        typedef HashDH<UInt32, String> TFunctNameHash;
        // Map Method Body Info into HashSet of Traits.
        typedef HashDH<SInt32, HashSetDH<SPtr<const Traits> >*> TUsedTraitsHash;

        InfoCollector(MemoryHeap* heap) 
            : UsedFunctions(heap)
            , FunctNameHash(heap)
            , UTH(heap)
        {
        }
        ~InfoCollector();

        void MakeFunctionName(StringBuffer& buf, const InstanceTraits::Function& tr) const;
        String MakeFunctionName(const InstanceTraits::Function& tr) const
        {
            StringBuffer buf;
            MakeFunctionName(buf, tr);
            return buf;
        }

        void AddUsedTraits(const SInt32 mbi_ind, const Traits& tr);

        TFunctHashSet   UsedFunctions;
        TFunctNameHash  FunctNameHash;
        TUsedTraitsHash UTH;
    };

} // namespace AOT

}}} // namespace Scaleform { namespace GFx { namespace AS3 

#endif // SF_AS3_AOTC

#endif // INC_AS3_AOTC_H


