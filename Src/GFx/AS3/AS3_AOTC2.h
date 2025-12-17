/**************************************************************************

Filename    :   AS3_AOTC2.h
Content     :   
Created     :   Aug, 2012
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_AS3_AOTC2_H
#define INC_AS3_AOTC2_H

#include "AS3_Value.h"

namespace Scaleform
{ 
    //////////////////////////////////////////////////////////////////////////////
    inline
    StringBuffer& operator <<(StringBuffer& sb, StringBuffer& /*sb2*/)
    {
        // sb.AppendString(sb2.ToCStr(), sb2.GetSize());
        return sb;
    }

    StringBuffer& operator <<(StringBuffer& sb, int v);

    StringBuffer& operator <<(StringBuffer& sb, unsigned v);

#if defined(SF_CC_MSVC)
    StringBuffer& operator <<(StringBuffer& sb, SInt32 v);
    StringBuffer& operator <<(StringBuffer& sb, UInt32 v);
#endif

#if !defined(SF_64BIT_POINTERS) || defined(SF_OS_WIN32)
    StringBuffer& operator <<(StringBuffer& sb, SInt64 v);
    StringBuffer& operator <<(StringBuffer& sb, UInt64 v);
#endif

    inline
    StringBuffer& operator <<(StringBuffer& sb, const char* v)
    {
        sb.AppendString(v);
        return sb;
    }

    inline
    StringBuffer& operator <<(StringBuffer& sb, const String& v)
    {
        sb.AppendString(v.ToCStr(), v.GetSize());
        return sb;
    }

    inline
    StringBuffer& operator <<(StringBuffer& sb, const GFx::ASString& v)
    {
        sb.AppendString(v.ToCStr(), v.GetSize());
        return sb;
    }

    inline
    StringBuffer& operator <<(StringBuffer& sb, const StringDataPtr& v)
    {
        sb.AppendString(v.ToCStr(), v.GetSize());
        return sb;
    }

    StringBuffer& operator <<(StringBuffer& sb, const GFx::AS3::Value& v);
}

#if defined(SF_AS3_AOTC2)

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

namespace Abc
{
    class File;
}

///
// This is intrusive list node.
class SNode : public NewOverrideBase<Mem_Stat>
{
    friend class AOT::RefCountVisitor;

public:
    enum EType {
        tSentinel,
        tBlock,
        tOC,
        tSwitchOC,
        tIF,
        tAbrupt,
        tStr,
        tValue,
        tVar,
        tMbrAcc,
        t1,
        t2,
        tN,
    };

public:
    SNode(EType t SF_DEBUG_ARG(UPInt addr))
        : RefCount(1)
        , RefCountAST(0)
        , Type(t)
        , pPrev(NULL)
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
    virtual void Accept(AOT::Visitor& v);

    //
    static SPtr<SNode> RetrieveNode(const Value& v);

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

public:
    //
    SNode* GetPrev() const { return pPrev; }
    SNode* GetNext() const { return pNext; }

    //
    void SetPrev(SNode* other) { pPrev = other; }

    //
    void SetNext(SNode* other) { pNext = other; }
    void SetNext(const SPtr<SNode>& other) { pNext = other; }
    void SetNext(Pickable<SNode> other) { pNext = other; }

    //
    void    RemoveNode()
    {
        SPtr<SNode> guard(this);
        pPrev->pNext = pNext;
        pNext->pPrev = pPrev;
    }

    //
    void    ReplaceNodeWith(SNode* first, SNode* last)
    {
        SPtr<SNode> guard(this);
        pPrev->pNext = first;
        pNext->pPrev = last;

        first->pPrev = pPrev;
        last->pNext  = pNext;
    }
    void    ReplaceNodeWith(Pickable<SNode> first, SNode* last)
    {
        SPtr<SNode> guard(this);
        pPrev->pNext = first;
        pNext->pPrev = last;

        first->pPrev = pPrev;
        last->pNext  = pNext;
    }
    void    ReplaceNodeWith(SNode* other)
    {
        ReplaceNodeWith(other, other);
    }
    void    ReplaceNodeWith(Pickable<SNode> other)
    {
        ReplaceNodeWith(other, other.GetPtr());
    }

    //
    void    InsertNodeAfter(SNode* first, SNode* last)
    {
        first->pPrev    = pNext->pPrev;
        last->pNext     = pNext;

        pNext->pPrev    = last;
        pNext           = first;
    }
    void    InsertNodeAfter(Pickable<SNode> first, SNode* last)
    {
        first->pPrev    = pNext->pPrev;
        last->pNext     = pNext;

        pNext->pPrev    = last;
        pNext           = first;
    }
    void    InsertNodeAfter(SNode* other)
    {
        InsertNodeAfter(other, other);
    }
    void    InsertNodeAfter(Pickable<SNode> other)
    {
        InsertNodeAfter(other, other.GetPtr());
    }

    //
    void    InsertNodeBefore(SNode* first, SNode* last)
    {
        last->pNext     = pNext->pPrev;
        first->pPrev    = pPrev;

        SPtr<SNode> guard(this);
        pPrev->pNext    = first;
        pPrev           = last;
    }
    void    InsertNodeBefore(Pickable<SNode> first, SNode* last)
    {
        last->pNext     = pNext->pPrev;
        first->pPrev    = pPrev;

        SPtr<SNode> guard(this);
        pPrev->pNext    = first;
        pPrev           = last;
    }
    void    InsertNodeBefore(SNode* other)
    {
        InsertNodeBefore(other, other);
    }
    void    InsertNodeBefore(Pickable<SNode> other)
    {
        InsertNodeBefore(other, other.GetPtr());
    }

private:
    // Total reference count.
    UInt32      RefCount;
    // Reference count in AST.
    UInt32      RefCountAST;
    EType       Type;
    SNode*      pPrev;
    SPtr<SNode> pNext;
    SF_DEBUG_CODE(UPInt   Addr;)
}; // class SNode

// Double-linked SNode list with sentinel.
class SNodeList
{
public:
    SNodeList();
    ~SNodeList();

    void Clear();
    // Complexity: O(n)
    UPInt GetSize() const;

    //
    SNode* GetFirst() const { return Sentinel.GetNext(); }
    SNode* GetLast() const { return Sentinel.GetPrev(); }
    SNode* Back() const { return Sentinel.GetPrev(); }

    SNode* GetEnd() { return &Sentinel; }
    const SNode* GetEnd() const { return &Sentinel; }

    //
    bool IsEmpty() const { return Sentinel.GetNext() == &Sentinel; }
    bool IsFirst(const SNode* node) const { return node == Sentinel.GetNext(); }
    bool IsLast(const SNode* node) const { return node == Sentinel.GetPrev(); }
    bool IsNull(const SNode* node) const { return node == &Sentinel; }

    // Checks to see if this SNodeList contains some statements.
    // If this list only contains comment string nodes, then we
    // claim the list has no statements.
    // Complexity: O(n)
    bool HasStatements() const;

    //
    void PushFront(SNode* first, SNode* last)
    {
        GetFirst()->InsertNodeBefore(first, last);
    }
    void PushFront(Pickable<SNode> first, SNode* last)
    {
        GetFirst()->InsertNodeBefore(first, last);
    }
    void PushFront(SNodeList& other)
    {
        if (!other.IsEmpty())
        {
            PushFront(other.GetFirst(), other.GetLast());
            other.Clear();
        }
    }
    void PushFront(SNode* node)
    {
        PushFront(node, node);
    }
    void PushFront(Pickable<SNode> node)
    {
        PushFront(node, node.GetPtr());
    }

    //
    void PushBack(SNode* first, SNode* last)
    {
        GetLast()->InsertNodeAfter(first, last);
    }
    void PushBack(Pickable<SNode> first, SNode* last)
    {
        GetLast()->InsertNodeAfter(first, last);
    }
    void PushBack(SNodeList& other)
    {
        if (!other.IsEmpty())
        {
            PushBack(other.GetFirst(), other.GetLast());
            other.Clear();
        }
    }
    void PushBack(SNode* node)
    {
        PushBack(node, node);
    }
    void PushBack(Pickable<SNode> node)
    {
        PushBack(node, node.GetPtr());
    }

    //
    void PopBack()
    {
        RemoveNode(GetLast());
    }

    //
    static void RemoveNode(SNode* node)
    {
        node->RemoveNode();
    }

    //
    void BringNodeToFront(SNode* node)
    {
        RemoveNode(node);
        PushFront(node);
    }

    //
    void BringNodeToBack(SNode* node)
    {
        RemoveNode(node);
        PushBack(node);
    }

private:
    SNode   Sentinel;
}; // SNodeList

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

    //
    struct SNodeStr : public SNode
    {
        enum OP {tName, tComment};

        SNodeStr(const String& v, OP op = tName) 
            : SNode(SNode::tStr SF_DEBUG_ARG(0))
            , Op(op)
            , V(v) 
        {
        }
        SNodeStr(const SNodeStr& other) 
            : SNode(SNode::tStr SF_DEBUG_ARG(0))
            , Op(other.Op)
            , V(other.V)
        {
        }

        virtual void Accept(AOT::Visitor& v);

        EType GetType() const { return SNode::tStr; }
        bool operator ==(const SNodeStr& other) const
        {
            return Op == other.Op && V == other.V;
        }

        bool Is(OP op) const
        {
            return Op == op;
        }

        const OP        Op;
        const String    V;

    private:
        SNodeStr& operator=(const SNodeStr& other);
    }; // struct SNodeStr

    // Constant. It is not supposed to be used as lvalue.
    struct SNodeValue : public SNode
    {
        enum OP {tValue, tAbsObject};

        SNodeValue(const Value& v, OP op = tValue);
        SNodeValue& operator=(const SNodeValue&);

        virtual void Accept(AOT::Visitor& v);

        EType GetType() const { return SNode::tValue; }
        bool operator ==(const SNodeValue& other) const
        {
            return V == other.V;
        }

        bool Is(OP op) const
        {
            return Op == op;
        }

        const OP    Op;
        Value       V;
    }; // struct SNodeValue

    // Variable/Subscript with type.
    struct SNodeVar : public SNode
    {
        enum OP {tVar, tSubscript};

        SNodeVar(const Value& n, const SPtr<const ClassTraits::Traits>& t, OP op = tVar);
        SNodeVar& operator=(const SNodeVar&);

        virtual void Accept(AOT::Visitor& v);

        EType GetType() const { return SNode::tVar; }
        bool operator ==(const SNodeVar& other) const
        {
            return Name == other.Name && Type == other.Type;
        }

        bool Is(OP op) const
        {
            return Op == op;
        }

        const OP            Op;
        SPtr<const ClassTraits::Traits> Type;
        Value               Name;
    }; // struct SNodeVar

    // Member access.
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

        EType GetType() const { return SNode::tMbrAcc; }
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

        SNode1(OP op , const Value& v SF_DEBUG_ARG(UPInt addr))
            : SNode(SNode::t1 SF_DEBUG_ARG(addr))
            , Op(op)
            , V(RetrieveNode(v))
        {
        }
        SNode1(OP op , const SPtr<SNode>& v SF_DEBUG_ARG(UPInt addr))
            : SNode(SNode::t1 SF_DEBUG_ARG(addr))
            , Op(op)
            , V(v)
        {
        }

        virtual void Accept(AOT::Visitor& v);

        EType GetType() const { return SNode::t1; }
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

        SNode2(OP op, const Value& l, const Value& r SF_DEBUG_ARG(UPInt addr));
        SNode2(OP op, const SPtr<SNode>& l, const Value& r SF_DEBUG_ARG(UPInt addr));
        SNode2(OP op, const Value& l, const SPtr<SNode>& r SF_DEBUG_ARG(UPInt addr));
        SNode2(OP op, const SPtr<SNode>& l, const SPtr<SNode>& r SF_DEBUG_ARG(UPInt addr));

        virtual void Accept(AOT::Visitor& v);

        EType GetType() const { return SNode::t2; }
        bool Is(OP op) const
        {
            return Op == op;
        }

        OP          Op;
        SPtr<SNode> L;
        SPtr<SNode> R;
    }; // struct SNode2

    // Basic Block.
    // Any number of entries, one exit.
    struct SNodeBlock : public SNode
    {
        typedef SNodeList   TElems;

        SNodeBlock(UInt32 addr)
            : SNode(SNode::tBlock SF_DEBUG_ARG(addr))
            , Artificial(false)
            , ForceOutline(false)
            , Addr(addr)
        {
        }

        virtual void Accept(AOT::Visitor& v);

        EType GetType() const { return SNode::tBlock; }

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
        UInt32 GetAddr() const
        {
            return Addr;
        }

    private:
        SNodeBlock& operator=(const SNodeBlock&);

    private:
        bool            Artificial:1;
        bool            ForceOutline:1;
        const UInt32    Addr;

    public:
        TElems          Elems;
    }; // struct SNodeBlock

    //
    struct SNodeOC : public SNode
    {
        enum OP {
            op_throw,
            op_getsuper, // 1
            op_setsuper, // 1
            op_dxns, // 1
            op_dxnslate,
            op_kill, // 1
            op_label, // 1
            op_inclocal_ti, // 1
            op_declocal_ti, // 1
            op_pushwith,
            op_popscope,
            op_nextname,
            op_hasnext,
            op_pushnull,
            op_pushundefined,
            op_not_tb,
            op_nextvalue,
            op_pushbyte, // 1
            op_pushshort, // 1
            op_pushtrue,
            op_pushfalse,
            op_pushnan,
            op_pop,
            op_dup,
            op_swap,
            op_pushstring, // 1
            op_pushint, // 1
            op_pushuint, // 1
            op_pushdouble, // 1
            op_pushscope,
            op_pushnamespace, // 1
            op_hasnext2, // 2
            op_increment_tu,
            op_decrement_tu,
            op_inclocal_tu, // 1
            op_declocal_tu, // 1
            op_negate_ti,
            op_newfunction, // 1
            op_call, // 1
            op_construct, // 1
            op_callmethod, // 2
            op_callstatic, // 2
            op_callsuper, // 2
            op_callproperty, // 2
            op_constructsuper, // 1
            op_constructprop, // 2
            op_callproplex, // 2
            op_callsupervoid, // 2
            op_callpropvoid, // 2
            op_applytype, // 1
            op_negate_td,
            op_newobject, // 1
            op_newarray, // 1
            op_newactivation,
            op_getdescendants, // 1
            op_newcatch, // 1
            op_findpropstrict, // 1
            op_findproperty, // 1
            op_getlex, // 1
            op_setproperty, // 1
            op_getarg, // 1 // same as op_getlocal, but operates on orguments
            op_getlocal, // 1
            op_setlocal, // 1
            op_getglobalscope,
            op_getscopeobject, // 1
            op_getproperty, // 1
            op_getouterscope, // 1 // My own
            op_initproperty, // 1
            op_dup_nrc, // My own ... (Not ref-counted version of dup)
            op_deleteproperty, // 1
            op_pop_nrc, // My own ... (Not ref-counted version of pop)
            op_getslot, // 1
            op_setslot, // 1
            op_getglobalslot, // 1
            op_setglobalslot, // 1
            op_convert_s,
            op_esc_xelem,
            op_esc_xattr,
            op_convert_i,
            op_convert_u,
            op_convert_d,
            op_convert_b,
            op_convert_o,
            op_checkfilter,

            op_add_ti,
            op_subtract_ti,
            op_multiply_ti,
            op_add_td,
            op_subtract_td,
            op_multiply_td,
            op_divide_td,

            op_coerce, // 1
            op_coerce_a,
            op_coerce_s,
            op_astype, // 1
            op_astypelate,

            op_negate,
            op_increment,
            op_inclocal, // 1
            op_decrement,
            op_declocal, // 1
            op_typeof,
            op_not,
            op_bitnot,
            op_increment_ti,
            op_decrement_ti,
            op_add_d,

            op_add,
            op_subtract,
            op_multiply,
            op_divide,
            op_modulo,
            op_lshift,
            op_rshift,
            op_urshift,
            op_bitand,
            op_bitor,
            op_bitxor,
            op_equals,
            op_strictequals,
            op_lessthan,
            op_lessequals,
            op_greaterthan,
            op_greaterequals,
            op_instanceof,
            op_istype, // 1
            op_istypelate,
            op_in,

            op_getabsslot, // 1 // My own ...
            op_setabsslot, // 1 // My own ...
            op_initabsslot, // 1 // My own ...
            op_callsupermethod, // 1 // My own ...
            op_callgetter, // 2 // My own ...
            op_callsupergetter, // 2 // My own ...

            op_increment_i,
            op_decrement_i,
            op_inclocal_i, // 1
            op_declocal_i, // 1
            op_negate_i,
            op_add_i,
            op_subtract_i,
            op_multiply_i,

            op_callobject, // 1
            opNum
        };

        SNodeOC(OP op, SInt32 v1 = 0, SInt32 v2 = 0 SF_DEBUG_ARG(UPInt addr = 0))
            : SNode(SNode::tOC SF_DEBUG_ARG(addr))
            , Op(op)
            , Val1(v1)
            , Val2(v2)
        {
        }

        virtual void Accept(AOT::Visitor& v);

        EType GetType() const { return SNode::tOC; }

        OP GetOP() const
        {
            return Op;
        }
        SInt32 GetVal1() const
        {
            return Val1;
        }
        SInt32 GetVal2() const
        {
            return Val2;
        }

        // We do not want to include Abc header here.
        static OP GetOP(int abc_op);

        bool Is(OP op) const
        {
            return Op == op;
        }

    public:
        struct Info
        {
            const char* VMFunct;
            int ArgNum;
            int AbcOpCode;
        };
        static const Info OCInfo[SNodeOC::opNum];

    private:
        OP      Op;
        SInt32  Val1;
        SInt32  Val2;
    }; // struct SNodeOC

    //
    struct SNodeSwitchOC : public SNode
    {
        friend class CleanupVisitor;

        SNodeSwitchOC(const SPtr<SNodeBlock>& def SF_DEBUG_ARG(UPInt addr = 0))
            : SNode(SNode::tSwitchOC SF_DEBUG_ARG(addr))
            , Default(def)
        {
        }

        virtual void Accept(AOT::Visitor& v);

        void PushCase(const SPtr<SNodeBlock>& c)
        {
            Cases.PushBack(c);
        }

        const SPtr<SNodeBlock>& GetDefault() const
        {
            return Default;
        }
        UPInt GetNumOfCases() const
        {
            return Cases.GetSize();
        }
        const SPtr<SNodeBlock>& GetCase(UPInt ind) const
        {
            return Cases[ind];
        }

    private:
        typedef ArrayLH<SPtr<SNodeBlock> >  TCases;

        SPtr<SNodeBlock>    Default;
        TCases              Cases;
    }; // struct SNodeSwitchOC

    // IF operation.
    struct SNodeIF : public SNode
    {
        friend class CleanupVisitor;

        enum OP {
            op_ifnlt, // 1
            op_ifnle, // 1
            op_ifngt, // 1
            op_ifnge, // 1
            op_iftrue, // 1
            op_iffalse, // 1
            op_ifeq, // 1
            op_ifne, // 1
            op_iflt, // 1
            op_ifle, // 1
            op_ifgt, // 1
            op_ifge, // 1
            op_ifstricteq, // 1
            op_ifstrictne, // 1

            op_iftrue_tb, // 1
            op_iffalse_tb, // 1

            op_ifnlt_ti, // 1
            op_ifnle_ti, // 1
            op_ifngt_ti, // 1
            op_ifnge_ti, // 1
            op_ifeq_ti, // 1
            op_ifge_ti, // 1
            op_ifgt_ti, // 1
            op_ifle_ti, // 1
            op_iflt_ti, // 1
            op_ifne_ti, // 1

            op_ifgt_td, // 1
            op_ifle_td, // 1
            op_iflt_td, // 1
            op_ifne_td, // 1
            op_ifnlt_td, // 1
            op_ifnle_td, // 1
            op_ifngt_td, // 1
            op_ifnge_td, // 1
            op_ifeq_td, // 1
            op_ifge_td, // 1
            opNum
        };

        // False should be initialized later.
        SNodeIF(OP op, const SPtr<SNodeBlock>& t SF_DEBUG_ARG(UPInt addr))
            : SNode(SNode::tIF SF_DEBUG_ARG(addr))
            , Op(op)
            , True(t)
        {
        }
        SNodeIF(OP op, const SPtr<SNodeBlock>& t, const SPtr<SNodeBlock>& f SF_DEBUG_ARG(UPInt addr))
            : SNode(SNode::tIF SF_DEBUG_ARG(addr))
            , Op(op)
            , True(t)
            , False(f)
        {
        }

        virtual void Accept(AOT::Visitor& v);

        EType GetType() const { return SNode::tIF; }

        //
        const SPtr<SNodeBlock>& GetTrue() const
        {
            return True;
        }
        SPtr<SNodeBlock>& GetTrue()
        {
            return True;
        }
        void SetTrue(const SPtr<SNode>& n)
        {
            SF_ASSERT(!n || n->Is(SNode::tBlock));
            if (n && n->Is(SNode::tBlock))
                True = static_cast<SNodeBlock*>(n.GetPtr());
            else
                True = NULL;
        }

        //
        const SPtr<SNodeBlock>& GetFalse() const
        {
            return False;
        }
        SPtr<SNodeBlock>& GetFalse()
        {
            return False;
        }
        void SetFalse(const SPtr<SNode>& n)
        {
            SF_ASSERT(!n || n->Is(SNode::tBlock));
            if (n && n->Is(SNode::tBlock))
                False = static_cast<SNodeBlock*>(n.GetPtr());
            else
                False = NULL;
        }

        //
        void SwapTrueFalse()
        {
            SF_ASSERT(False.GetPtr());
            Alg::Swap(True, False);
        }

        bool Is(OP op) const
        {
            return Op == op;
        }

        OP GetOP() const
        {
            return Op;
        }

    private:
        SNodeIF& operator=(const SNodeIF&);

    private:
        OP          Op;
        // SPtr<SNode> Expr;
        SPtr<SNodeBlock> True;
        SPtr<SNodeBlock> False;
    }; // struct SNodeIF

    //
    struct SNodeAbrupt : public SNode
    {
        enum OP {opReturn, opReturnValue, opGoto, opBreak, opContinue};

        SNodeAbrupt(OP op SF_DEBUG_ARG(UPInt addr))
            : SNode(SNode::tAbrupt SF_DEBUG_ARG(addr))
            , Op(op)
        {
        }
        SNodeAbrupt(OP op, const SPtr<SNodeBlock>& d SF_DEBUG_ARG(UPInt addr))
            : SNode(SNode::tAbrupt SF_DEBUG_ARG(addr))
            , Op(op)
            , Dest(d)
        {
        }

        virtual void Accept(AOT::Visitor& v);

        EType GetType() const { return SNode::tAbrupt; }

        bool Is(OP op) const
        {
            return Op == op;
        }

    private:
        SNodeAbrupt& operator=(const SNodeAbrupt&);

    public:
        OP                  Op;
        SPtr<SNodeBlock>    Dest;
        SPtr<SNode>         ReturnValue;
    }; // struct SNodeAbrupt

    // Functor with name and N arguments.
    struct SNodeN : public SNode
    {
        typedef ArrayLH<SPtr<SNode> >   TArgs;
        enum OP {opCall};

        SNodeN(OP op, const String& name SF_DEBUG_ARG(UPInt addr));
        SNodeN(OP op, const String& name, UInt32 argc, const Value* argv SF_DEBUG_ARG(UPInt addr));

        virtual void Accept(AOT::Visitor& v);

        EType GetType() const { return SNode::tN; }

        void ReleaseASTChildren();

        bool Is(OP op) const
        {
            return Op == op;
        }

    private:
        SNodeN& operator =(const SNodeN&);

    public:
        OP					Op;
        String			    Name;
        // Result type.
        SPtr<const Traits>  RType;
        TArgs				Args;
    }; // struct SNodeN

    ///////////////////////////////////////////////////////////////////////////
    // Default implementation won't visit children.
    class Visitor
    {
    public:
        // To get rid of warnings.
        virtual ~Visitor();

    public:
        virtual void VisitSNodeStr(SNodeStr& n);
        virtual void VisitSNodeValue(SNodeValue& n);
        virtual void VisitSNodeVar(SNodeVar& n);
        virtual void VisitSNodeMbrAcc(SNodeMbrAcc& n);
        virtual void VisitSNode1(SNode1& n);
        virtual void VisitSNode2(SNode2& n);
        virtual void VisitSNodeBlock(SNodeBlock& n);
        virtual void VisitSNodeOC(SNodeOC& n);
        virtual void VisitSNodeSwitchOC(SNodeSwitchOC& n);
        virtual void VisitSNodeIF(SNodeIF& n);
        virtual void VisitSNodeAbrupt(SNodeAbrupt& n);
        virtual void VisitSNodeN(SNodeN& n);
    };

    ///////////////////////////////////////////////////////////////////////////
    // Unique MbiInd
    struct UMbiInd
    {
        UMbiInd(const Abc::File& f, Abc::MbiInd m) 
            : pFile(&f)
            , Method(m.Get()) 
        {
        }

        bool operator == (const UMbiInd& other) const
        { return (pFile == other.pFile) && (Method == other.Method); }

        const Abc::File& GetFile() const
        {
            return *pFile;
        }
        Abc::MbiInd GetMethod() const
        {
            return Abc::MbiInd(static_cast<Abc::SInd>(Method));
        }

    private:
        // Make sure the member variables stay with no padding
        // Padding breaks the hash function that uses this struct as a key
        const Abc::File*    pFile;
        // Use pointer-size member to prevent padding.
        SPInt               Method;
    };

    ///////////////////////////////////////////////////////////////////////////
    struct InfoCollector
    {
        // typedef HashSetDH<UInt32> TInt32HashSet;
        // Set of used functions.
        typedef HashSetDH<SPtr<InstanceTraits::Function> > TFunctHashSet;
        // Map Method Body Index into name.
        typedef HashDH<UMbiInd, String> TFunctNameHash;
        // Store ClassTraits because getting of InstanceTraits from ClassTraits is easy.
        typedef HashSetDH<SPtr<const ClassTraits::Traits> > TTraitsHash;
        // Map Method Body Info (of a constructor, Function, or Activation Object)
        // into HashSet of Traits.
        typedef HashDH<UMbiInd, TTraitsHash*> TUsedTraitsHash;

        InfoCollector(MemoryHeap* heap);
        ~InfoCollector();

        void MakeFunctionName(StringBuffer& buf, const InstanceTraits::Function& tr) const;
        String MakeFunctionName(const InstanceTraits::Function& tr) const
        {
            StringBuffer buf;
            MakeFunctionName(buf, tr);
            return buf;
        }

        void AddUsedTraits(const Abc::File& file, const Abc::MbiInd mbi_ind, const ClassTraits::Traits& ctr);
        const TUsedTraitsHash& GetUTH() const
        {
            return UTH;
        }

        TFunctHashSet   UsedFunctions;
        TFunctNameHash  FunctNameHash;

    private:
        TUsedTraitsHash UTH;
    };

    ///////////////////////////////////////////////////////////////////////////
    // Functions.
    // We cannot implement GetClassTraits because constructor of InstanceTraits
    // is not completely initialized.
    InstanceTraits::Traits& GetInstanceTraits(Traits& tr);
    const InstanceTraits::Traits& GetInstanceTraits(const Traits& tr);

} // namespace AOT

}}} // namespace Scaleform { namespace GFx { namespace AS3 

#endif // SF_AS3_AOTC2

#endif // INC_AS3_AOTC2_H


