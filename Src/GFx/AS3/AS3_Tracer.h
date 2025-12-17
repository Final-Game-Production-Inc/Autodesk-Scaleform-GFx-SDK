/**************************************************************************

Filename    :   AS3_Tracer.h
Content     :   
Created     :   Sep, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_AS3_TRACER_H
#define INC_AS3_TRACER_H

#include "Kernel/SF_List.h"
#include "Kernel/SF_BitSet.h"

#include "AS3_VM.h"
#include "AS3_FlashUI.h"

#if defined(SF_AS3_AOTC)
    #include "AS3_AOTC.h"
#endif

#if defined(SF_AS3_AOTC2)
    #include "AS3_AOTC2.h"
#endif

// Not finished yet.
// #define SF_AS3_TR_UINT_AS_SINT

namespace Scaleform { namespace GFx { namespace AS3
{

///////////////////////////////////////////////////////////////////////////
// Forward declaration.
class Tracer;

namespace InstanceTraits
{
    namespace fl
    {
        class Catch;
    }

    class Function;
}

namespace TR
{
    enum MergeSet {msOpStack, msScopeStack, msRegisterFile};

    ///////////////////////////////////////////////////////////////////////////
    // Moveable.
    // Not moveable in case of SF_AS3_AOTC
    class Block : public NewOverrideBase<StatMV_VM_Tracer_Mem>, public ListNode<Block>
    {
        friend class AS3::Tracer;

        enum ETypeOffset {
            tDeadOffset             = 0,
            tExcTryOffset           = 1,
            tExcCatchOffset         = 2,
            tExcFinallyOffset       = 3,
            tLoopBodyOffset         = 4,
            tLoopConditionOffset    = 5,
            tSwitchCaseOffset       = 6,
        };

    public:
        enum EType {
            tUnknown        = 0,
            tDead           = 1 << tDeadOffset,
            tExcTry         = 1 << tExcTryOffset,
            tExcCatch       = 1 << tExcCatchOffset,
            tExcFinally     = 1 << tExcFinallyOffset,
            tLoopBody       = 1 << tLoopBodyOffset,
            tLoopCondition  = 1 << tLoopConditionOffset,
            tSwitchCase     = 1 << tSwitchCaseOffset,
        };

    public:
        explicit Block(Abc::TCodeOffset f);
        Block(TR::State& st, Abc::TCodeOffset f, EType t);
        ~Block();

    public:
        //
        void SetType(EType t)
        {
            Type |= t;
        }

        //
        bool IsInitializedBlock() const
        {
            return IsInitialized;
        }

        //
        bool IsCatchBlock() const
        {
            return (Type & tExcCatch) != 0;
        }
        void SetCatchBlock()
        {
            CurOpStackSize = 1;
            SetType(tExcCatch);
        }

        //
        bool IsTryBlock() const
        {
            return (Type & tExcTry) != 0;
        }
        void SetTryBlock()
        {
            SetType(tExcTry);
        }

        //
        bool IsExcTryBlock() const
        {
            return (Type & tExcTry) != 0;
        }
        bool IsExcCatchBlock() const
        {
            return (Type & tExcCatch) != 0;
        }
        bool IsExcFinallyBlock() const
        {
            return (Type & tExcFinally) != 0;
        }

        //
        bool IsSwitchCaseBlock() const
        {
            return (Type & tSwitchCase) != 0;
        }

        //
        bool IsLoopBodyBlock() const
        {
            return (Type & tLoopBody) != 0;
        }
        bool IsLoopConditionBlock() const
        {
            return (Type & tLoopCondition) != 0;
        }

        //
        bool IsDeadBlock() const
        {
            return (Type & tDead) != 0;
        }
        void SetDeadBlock(bool flag = true)
        {
            Type = (Type & ~tDead) | (static_cast<UInt32>(flag) << tDeadOffset);
        }

        //
        Abc::TCodeOffset GetFrom() const
        {
            return From;
        }

        //
        TR::State& GetState() const
        {
            SF_ASSERT(State);
            return *State;
        }
        TR::State* GetStatePtr() const
        {
            return State;
        }

        //
#ifdef SF_AS3_AOTC
        void AddTarget(SPtr<SNode>& node)
        {
//             SF_ASSERT(Nodes.GetPtr());
//             Nodes->Elems.PushBack(node);;

            SF_ASSERT(node.GetPtr() == NULL);
            node = Nodes;

//             Targets.PushBack(&node);
        }

        //
        Abc::TCodeOffset GetSwitchOffset() const
        {
            return SwitchOffset;
        }
        void SetSwitchOffset(Abc::TCodeOffset offset)
        {
            SwitchOffset = offset;
        }

        //
        Abc::TCodeOffset GetTryOffset() const
        {
            return TryOffset;
        }
        void SetTryOffset(Abc::TCodeOffset offset)
        {
            TryOffset = offset;
        }
#endif

#if defined(SF_AS3_AOTC) || defined(SF_AS3_AOTC2)
        //
        void PushSNode(Pickable<SNode> n) const
        {
            Nodes->Elems.PushBack(n);
        }
        void PushSNode(const SPtr<SNode>& n) const
        {
            Nodes->Elems.PushBack(n);
        }

        //
        void PopSNode() const
        {
            Nodes->Elems.PopBack();
        }

        //
        AOT::SNodeBlock& GetNodes() const
        {
            return *Nodes;
        }
#endif

    private:
        void SetInitialized(bool flag = true)
        {
            IsInitialized = flag;
        }
        Block* GetThis() { return this; }

    private:
        UInt32                  IsInitialized:1;

        UInt32                  Type;
        TR::State*              State;
        Abc::TCodeOffset        From; // Inclusive
        UPInt                   CurOpStackSize;

#if defined(SF_AS3_AOTC) || defined(SF_AS3_AOTC2)
        Abc::TCodeOffset        SwitchOffset;
        Abc::TCodeOffset        TryOffset;
        SPtr<AOT::SNodeBlock>   Nodes;
#endif

#if defined(SF_AS3_AOTC)
        SPtr<AOT::SNodeCatch>   Catch;
        ArrayLH<Block*>         Catches;
#endif
    }; // class Block

    ///////////////////////////////////////////////////////////////////////////

    class StackReader;

    class State : public NewOverrideBase<Stat_Default_Mem>
    {
        friend class AS3::Tracer;
        friend class StackReader;

    public:
        State(Tracer& tr, Abc::TCodeOffset cp);

    public:
        enum ScopeType {stScopeStack, stStoredScope, stGlobalObject};

    public:
        const ValueArrayDH& GetRegisters() const
        {
            return Registers;
        }
        const ValueArrayDH& GetOpStack() const
        {
            return OpStack;
        }
        const ValueArrayDH& GetScopeStack() const
        {
            return ScopeStack;
        }

        ValueArrayDH& GetValueArray(MergeSet ms);
        const ValueArrayDH& GetValueArray(MergeSet ms) const;

    public:
        ///
        const Value& GetOpValue(UPInt ind) const
        {
            SF_ASSERT(ind < OpStack.GetSize());
            return OpStack[ind];
        }

        ///
        const Value& BackOpValue() const
        {
            return OpStack.Back();
        }
        const Value& BackOp2ndValue() const
        {
            return OpStack[OpStack.GetSize() - 2];
        }

        ///
        Value& BackOpValue()
        {
            return OpStack.Back();
        }
        Value& BackOp2ndValue()
        {
            return OpStack[OpStack.GetSize() - 2];
        }

        ///
        void SetBackOpValue(const Value& v)
        {
            OpStack.Back().Assign(v);
        }
        void SetBackOp2ndValue(const Value& v)
        {
            OpStack[OpStack.GetSize() - 2].Assign(v);
        }

        ///
        void SetBackOpValueUnsafe(const Value& v)
        {
            OpStack.Back().AssignUnsafe(v);
        }
        void SetBackOp2ndValueUnsafe(const Value& v)
        {
            OpStack[OpStack.GetSize() - 2].AssignUnsafe(v);
        }

        ///
        Value PopOpValue()
        {
            return OpStack.Pop();
        }
        void PopOp(UPInt count = 1)
        {
            OpStack.PopBack(count);
        }
        void PushOp(const Value& v)
        {
            OpStack.PushBack(v);
        }
        void PushOp(InstanceTraits::Traits& tr, Value::TraceNullType isNull)
        {
            OpStack.PushBack(Value(tr, isNull));
        }
        void PushOp(ClassTraits::Traits& tr, Value::TraceNullType isNull)
        {
            OpStack.PushBack(Value(tr, isNull));
        }
        void ConvertOpTo(InstanceTraits::Traits& tr, Value::TraceNullType isNull);
#ifdef SF_AS3_AOTC
        void ConvertOpTo(InstanceTraits::Traits& tr, Value::TraceNullType isNull, SNode& n)
        {
            OpStack.Back() = Value(tr, isNull, n);
        }
#endif
        void SwapOp();

        ///
        const Value& GetScopeValue(UPInt ind)
        {
            SF_ASSERT(ind < ScopeStack.GetSize());
            return ScopeStack[ind];
        }
        void PopScope(UPInt count = 1)
        {
            ScopeStack.PopBack(count);
        }
        void PushScope(const Value& v)
        {
            ScopeStack.PushBack(v);
        }

        ///
        bool RegisterIsAlive(AbsoluteIndex index) const
        {
            return RegistersAlive.IsSet(index.Get());
        }
        void SetRegister(AbsoluteIndex index, const Value& v)
        {
            Registers[index.Get()] = v;
            RegistersAlive.Set(index.Get());
        }
        Value& GetRegister(AbsoluteIndex index)
        {
            SF_ASSERT(RegisterIsAlive(index));
            return Registers[index.Get()];
        }
        const Value& GetRegister(AbsoluteIndex index) const
        {
            SF_ASSERT(RegisterIsAlive(index));
            return Registers[index.Get()];
        }
        void ConvertRegisterTo(AbsoluteIndex index, InstanceTraits::Traits& tr, Value::TraceNullType isNull);
		const Value& GetThis() const
		{
			return GetRegister(AbsoluteIndex(0));
		}

    public:
        Tracer& GetTracer() const
        {
            return *pTracer;
        }
        Abc::TCodeOffset GetCP() const
        {
            return BCP;
        }

        ValueArrayDH& GetRegisters()
        {
            return Registers;
        }
        ValueArrayDH& GetOpStack()
        {
            return OpStack;
        }
        ValueArrayDH& GetScopeStack()
        {
            return ScopeStack;
        }

        bool NeedToDiscardResult() const;

    public:

        void exec_throw(Abc::TCodeOffset bcp);
        void exec_dxns(UInt32 index);
        void exec_dxnslate();

        void exec_pushwith();
        void exec_popscope();
        void exec_nextname();
        void exec_hasnext();
        void exec_pushundefined();
        void exec_nextvalue();

        void exec_pushbyte(SInt32 v);
        void exec_pushshort(SInt32 v);
        void exec_pop();
        void exec_dup();
        void exec_swap(bool simulate = false);
        void exec_pushstring(SInt32 v);
        void exec_pushint(SInt32 v);
        void exec_pushuint(SInt32 v);
        void exec_pushdouble(SInt32 v);
        void exec_pushscope();
        void exec_pushnamespace(UInt32 v);
        void exec_hasnext2(UInt32 object_reg, UInt32 index_reg);
        void exec_li8();
        void exec_li16();
        void exec_li32();
        void exec_lf32();
        void exec_lf64();
        void exec_si8();
        void exec_si16();
        void exec_si32();
        void exec_sf32();
        void exec_sf64();
        void exec_newfunction(UInt32 method_ind);
        void exec_call(UInt32 arg_count);
        void exec_construct(UInt32 arg_count);
        void exec_constructsuper(UInt32 arg_count);
        void exec_sxi1();
        void exec_sxi8();
        void exec_sxi16();
        void exec_applytype(UInt32 arg_count);
        void exec_newobject(UInt32 arg_count);
        void exec_newarray(UInt32 arr_size);
        void exec_newactivation();
        void exec_newclass(UInt32 v);
        void exec_getdescendants(UInt32 index);
        void exec_newcatch(UInt32 v);
        void exec_findpropstrict(UInt32 mn_index);
        void exec_findproperty(UInt32 mn_index);
        void exec_getlex(UInt32 mn_index);
        void exec_getabsobject(Abc::Code::OpCode opcode, InstanceTraits::Traits& tr);
        void exec_getlocal(UInt32 reg_ind);
        void exec_setlocal(UInt32 rnum);
        void exec_getglobalscope(bool simulate = false);
        void exec_getscopeobject(UInt32 scope_index);
        void exec_deleteproperty(UInt32 mn_index);
        void exec_getslot(UInt32 slot_index, bool simulate = false);
        void exec_setslot(UInt32 slot_index, bool simulate = false);
        void exec_getglobalslot(UInt32 slot_index);
        void exec_setglobalslot(UInt32 slot_index);
        void exec_esc_xelem();
        void exec_esc_xattr();
        void exec_convert_i();
        // Artificial command.
        void exec_convert_reg_i(UInt32 reg_num);
        void exec_convert_u();
        // Artificial command.
        void exec_convert_reg_u(UInt32 reg_num);
        void exec_convert_d();
        // Artificial command.
        void exec_convert_reg_d(UInt32 reg_num);
        void exec_convert_b();
        void exec_convert_o();
        void exec_checkfilter();
        void exec_coerce_s();
        void exec_astype(UInt32 mn_index);
        void exec_inclocal(UInt32 v);
        void exec_declocal(UInt32 v);
        void exec_not();
        void exec_negate();
        void exec_increment_i();
        void exec_decrement_i();
        void exec_negate_i();

        void exec_add_d();
        void exec_add_i();
        void exec_subtract();
        void exec_subtract_i();
        void exec_multiply();
        void exec_multiply_i();
        void exec_divide();

        void exec_1OpSInt(SF_AOTC_CODE(AOT::SNode1::OP op) SF_AOTC2_CODE(AOT::SNodeOC::OP op));
        void exec_1OpNumber(SF_AOTC_CODE(AOT::SNode1::OP op) SF_AOTC2_CODE(AOT::SNodeOC::OP op));
        void exec_1OpString(SF_AOTC_CODE(AOT::SNode1::OP op) SF_AOTC2_CODE(AOT::SNodeOC::OP op));

        void exec_2OpNumber(SF_AOTC_CODE(AOT::SNode2::OP op));
        void exec_2OpSInt(SF_AOTC_CODE(AOT::SNode2::OP op));
        void exec_2OpUInt(SF_AOTC_CODE(AOT::SNode2::OP op));
        void exec_2OpBoolean(SF_AOTC_CODE(AOT::SNodeBoolExpr::OP op));

        void exec_istype(UInt32 mn_index);
        void exec_inclocal_i(UInt32 v);
        void exec_declocal_i(UInt32 v);
        void exec_getlocal0();
        void exec_getlocal1();
        void exec_getlocal2();
        void exec_getlocal3();
        void exec_setlocal0();
        void exec_setlocal1();
        void exec_setlocal2();
        void exec_setlocal3();

        void exec_if(Abc::TCodeOffset& bcp, const Abc::Code::OpCode opcode);
        void exec_if_boolean(Abc::TCodeOffset& bcp, const Abc::Code::OpCode opcode);
        void exec_jump(Abc::TCodeOffset& bcp);
        void exec_switch(Abc::TCodeOffset& bcp);

        void exec_opcode(const Abc::Code::OpCode opcode, Abc::TCodeOffset& bcp);

    public:
#ifdef SF_AS3_AOTC
        SPtr<SNode> AotGetSlot(const Value& obj, AbsoluteIndex slot_index, bool _this = false) const;
        SPtr<SNode> AotGetProperty(const Value& v, const Multiname& mn, bool _this = false) const;
        SPtr<SNode> AotRetrieveClassName(const Value& v, const Multiname& mn) const;
        void AotUpdateRegisterValue(UInt32 rnum, Value& value) const;
        void aot_reg_op(Value& reg, AOT::SNode1::OP op) const;
        static bool IsInternalVMType(const Traits& tr);
#endif

    private:
        InstanceTraits::Traits& GetObjectType() const;
        InstanceTraits::Traits& GetClassType() const;
        InstanceTraits::Traits& GetFunctionType() const;
        InstanceTraits::Traits& GetArrayType() const;
        InstanceTraits::Traits& GetStringType() const;
        InstanceTraits::Traits& GetNumberType() const;
        InstanceTraits::Traits& GetSIntType() const;
        InstanceTraits::Traits& GetUIntType() const;
        InstanceTraits::Traits& GetBooleanType() const;
        InstanceTraits::Traits& GetNamespaceType() const;
        InstanceTraits::Traits& GetXMLListType() const;
        InstanceTraits::Traits& GetNullType() const;
        InstanceTraits::Traits& GetVoidType() const;

        InstanceTraits::Traits& GetFunctType(const Value& value) const;

        Value::TraceNullType CanBeNull(const Value& v) const;
        Value::TraceNullType CanBeNull(const InstanceTraits::Traits& tr) const;

        // This method will throw VerifyError if type is not found.
        CheckResult GetPropertyType(const Traits& obj_traits, const SlotInfo& si, Value& type SF_AOTC_ARG(SNode& sn)) const;

    private:
        VMAbcFile& GetFile() const;
        VM& GetVM() const;

    private:
        void PushNewOpCodeArg0(UInt32 arg) const;
        void PushNewOpCodeArgNumber0(Value::Number arg) const;

//         void PushNewOpCodeArg(UInt32 arg) const;
//         void PushNewOpCodeArgNumber(Value::Number arg) const;

#if defined(SF_AS3_AOTC) || defined(SF_AS3_AOTC2)
        void PushSNode(Pickable<SNode> n) const;
        void PushSNode(const SPtr<SNode>& n) const;
#endif

#if defined(SF_AS3_AOTC2)
        void PopSNodeOC() const;
        void PushSNodeOC(AOT::SNodeOC::OP op, SInt32 v1 = 0, SInt32 v2 = 0) const;

        void PushSNodeAbrupt(AOT::SNodeAbrupt::OP op) const;
        void PushSNodeAbrupt(AOT::SNodeAbrupt::OP op, const Block& b) const;

        void PushSNodeIF(AOT::SNodeIF::OP op, const Block& t) const;
        void PushSNodeIF(AOT::SNodeIF::OP op, const Block& t, const Block& f) const;

        AOT::SNodeSwitchOC& PushSNodeSwitch(const Block& def) const;

        void PushSNodeGetLocal(UInt32 reg_ind) const;

        void PushSNodeComment(const String& str) const;
#endif

        const Traits* GetValueTraits(const Value& v) const;
        const ScopeStackType& GetSavedScope() const;
        void FindProp(PropRef& result, const Multiname& mn, ScopeType& stype, UPInt& scope_index);

        // Refine opcode having one argument on stack.
        void RefineOpCodeStack1(Abc::Code::OpCode opcode, InstanceTraits::Traits& type, Abc::Code::OpCode op SF_AOTC_ARG(AOT::SNode1::OP sn_op));
        // Refine opcode having two arguments on stack.
        void RefineOpCodeStack2(Abc::Code::OpCode opcode, InstanceTraits::Traits& type, Abc::Code::OpCode op SF_AOTC_ARG(AOT::SNode2::OP sn_op));

        // Refine opcode having one argument in register.
        void RefineOpCodeReg1(Abc::Code::OpCode opcode, InstanceTraits::Traits& type, Abc::Code::OpCode op, int reg_num);

    private:
        State& operator =(const State& other);

    private:
        Tracer*                             pTracer;
        Abc::TCodeOffset                    BCP;
        Abc::TCodeOffset                    OpcodeCP;
        ValueArrayDH                        Registers;
        ValueArrayDH                        OpStack;
        ValueArrayDH                        ScopeStack;
        FixedBitSetDH<StatMV_VM_Tracer_Mem> RegistersAlive;
    }; // class State

    ///////////////////////////////////////////////////////////////////////////
    // This class is very similar to one, which reads real values.
    class StackReader
    {
    public:
        typedef StackReader SelfType;

        SF_INLINE
        StackReader(VM& vm, State& s)
            : VMRef(vm)
            , StateRef(s)
            , Num(0)
        {
        }

    protected:
        // How many extra records we have read ...
        void HaveRead(int _n)
        {
            Num += _n;
        }
        int GetRead() const
        {
            return Num;
        }

    public:
        VM& GetVM() const
        {
            return VMRef;
        }
        State& GetState() const
        {
            return StateRef;
        }

    protected:
        ValueArrayDH& GetOpStack() const
        {
            return GetState().OpStack;
        }
        Value PopOpValue()
        {
            return GetState().PopOpValue();
        }

        void CheckObject(const Value& v) const;

        int Read(Multiname& obj) const;

    private:
        SelfType& operator =(const SelfType&);

    private:
        VM&     VMRef;
        State&  StateRef;
        int     Num;
    };

    class MnReader
    {
    public:
        SF_INLINE
        MnReader(VMAbcFile& file)
            : File(file)
        {
        }

    public:
        VMAbcFile& GetFile() const
        {
            return File;
        }
        const Abc::Multiname& GetMultiname(UPInt ind) const
        {
            return GetFile().GetConstPool().GetMultiname(ind);
        }

    private:
        MnReader& operator =(const MnReader&);

    private:
        VMAbcFile& File;
    };

    // ReadArgs classes are used to read arguments from stack in various call opcodes,
    // including callproperty, callmethod, callsuper, construct, etc.

    struct ReadArgs : public StackReader
    {
        typedef ReadArgs SelfType;

        ReadArgs(VM& vm, State& s, UInt32 arg_count);

        const Value* GetCallArgs() const
        {
            if (ArgNum > CallArgs.GetSize())
                return FixedArr;
            else
                return CallArgs.GetDataPtr();
        }
        unsigned GetCallArgsNum() const
        {
            return ArgNum;
        }

    private:
        SelfType& operator =(const SelfType&);

    private:
        enum {eFixedArrSize = 8};

        const unsigned  ArgNum;
        ValueArrayDH    CallArgs;
        Value           FixedArr[eFixedArrSize];
    };

    struct ReadArgsObject : public ReadArgs
    {
        typedef ReadArgsObject SelfType;

        SF_INLINE
        ReadArgsObject(VM& vm, State& s, UInt32 arg_count)
            : ReadArgs(vm, s, arg_count)
            , ArgObject(s.PopOpValue())
        {
            HaveRead(1);

            CheckObject(ArgObject);
        }

        Value ArgObject;

    private:
        SelfType& operator =(const SelfType&);
    };

    struct ReadArgsObjectValue : public ReadArgsObject
    {
        typedef ReadArgsObjectValue SelfType;

        SF_INLINE
        ReadArgsObjectValue(VM& vm, State& s, UInt32 arg_count)
            : ReadArgsObject(vm, s, arg_count)
            , value(s.PopOpValue())
        {
            HaveRead(1);
        }

        const Value value;
    };

    struct ReadArgsMn : public ReadArgs, public MnReader
    {
        typedef ReadArgsMn SelfType;

        SF_INLINE
        ReadArgsMn(VMAbcFile& file, State& s, UInt32 arg_count, UInt32 mn_index)
            : ReadArgs(file.GetVM(), s, arg_count)
            , MnReader(file)
            , ArgMN(file, GetMultiname(mn_index))
        {
            HaveRead(Read(ArgMN));
        }

        Multiname ArgMN;

    private:
        SelfType& operator =(const SelfType&);
    };

    struct ReadArgsMnObject : public ReadArgsMn
    {
        typedef ReadArgsMnObject SelfType;

        SF_INLINE
        ReadArgsMnObject(VMAbcFile& file, State& s, UInt32 arg_count, UInt32 mn_index)
            : ReadArgsMn(file, s, arg_count, mn_index)
            , ArgObject(s.PopOpValue())
        {
            HaveRead(1);

            CheckObject(ArgObject);
        }

        const Value ArgObject;

    private:
        SelfType& operator =(const SelfType&);
    };

    struct ReadMn : public StackReader, public MnReader
    {
        typedef ReadMn SelfType;

        SF_INLINE
        ReadMn(VMAbcFile& file, State& s, UInt32 mn_index)
            : StackReader(file.GetVM(), s)
            , MnReader(file)
            , ArgMN(file, GetMultiname(mn_index))
        {
            HaveRead(Read(ArgMN));
        }

        Multiname ArgMN;

    private:
        SelfType& operator =(const SelfType&);
    };

    // Read compile-time multiname ...
    struct ReadMnCt : public StackReader, public MnReader
    {
        typedef ReadMnCt SelfType;

        SF_INLINE
        ReadMnCt(VMAbcFile& file, State& s, UInt32 mn_index)
            : StackReader(file.GetVM(), s)
            , MnReader(file)
            , ArgMN(GetMultiname(mn_index))
        {
            if (!ArgMN.IsCompileTime())
            {
                // The multiname at index must not be a runtime multiname.
                SF_ASSERT(false);
            }
        }

        const Abc::Multiname& ArgMN;

    private:
        SelfType& operator =(const SelfType&);
    };

    struct ReadValue : public StackReader
    {
        typedef ReadValue SelfType;

        SF_INLINE
        ReadValue(VM& vm, State& s)
            : StackReader(vm, s)
            , ArgValue(s.PopOpValue())
        {
            HaveRead(1);
        }

        const Value ArgValue;

    private:
        SelfType& operator =(const SelfType&);
    };

    struct ReadObject : public StackReader
    {
        typedef ReadObject SelfType;

        SF_INLINE
        ReadObject(VM& vm, State& s)
            : StackReader(vm, s)
            , ArgObject(s.PopOpValue())
        {
            HaveRead(1);

            CheckObject(ArgObject);
        }

        const Value ArgObject;

    private:
        SelfType& operator =(const SelfType&);
    };

    struct ReadMnCtValue : public ReadMnCt
    {
        typedef ReadMnCtValue SelfType;

        SF_INLINE
        ReadMnCtValue(VMAbcFile& file, State& s, UInt32 mn_index)
            : ReadMnCt(file, s, mn_index)
            , value(s.PopOpValue())
        {
            HaveRead(1);
        }

        const Value value;

    private:
        SelfType& operator =(const SelfType&);
    };

    struct ReadValueObject : public ReadValue
    {
        typedef ReadValueObject SelfType;

        SF_INLINE
        ReadValueObject(VM& vm, State& s)
            : ReadValue(vm, s)
            , ArgObject(s.PopOpValue())
        {
            HaveRead(1);

            CheckObject(ArgObject);
        }

        Value ArgObject;

    private:
        SelfType& operator =(const SelfType&);
    };

    struct ReadValueMn : public ReadValue, public MnReader
    {
        typedef ReadValueMn SelfType;

        SF_INLINE
        ReadValueMn(VMAbcFile& file, State& s, UInt32 mn_index)
            : ReadValue(file.GetVM(), s)
            , MnReader(file)
            , ArgMN(file, GetMultiname(mn_index))
        {
            HaveRead(Read(ArgMN));
        }

        Multiname ArgMN;

    private:
        SelfType& operator =(const SelfType&);
    };

    struct ReadValueMnObject : public ReadValueMn
    {
        typedef ReadValueMnObject SelfType;

        SF_INLINE
        ReadValueMnObject(VMAbcFile& file, State& s, UInt32 mn_index)
            : ReadValueMn(file, s, mn_index)
            , ArgObject(s.PopOpValue())
        {
            HaveRead(1);

            CheckObject(ArgObject);
        }

        const Value ArgObject;

    private:
        SelfType& operator =(const SelfType&);
    };

    struct ReadMnObject : public ReadMn
    {
        typedef ReadMnObject SelfType;

        SF_INLINE
        ReadMnObject(VMAbcFile& file, State& s, UInt32 mn_index)
            : ReadMn(file, s, mn_index)
            , ArgObject(s.PopOpValue())
        {
            HaveRead(1);

            CheckObject(ArgObject);
        }

        const Value ArgObject;

    private:
        SelfType& operator =(const SelfType&);
    };

} // namespace TR

// Tracer is similar to the VM in the way that it tries to "execute" opcode.
// The main difference is that it doesn't care about real values. It does 
// care only about value's types.
// As a side effect it generates optimized byte-code (it is actually word-code).

class Tracer : FlashUI
{
    friend class TR::State;

public:
    Tracer(
        MemoryHeap* heap, 
        const CallFrame& cf, 
        MethodInfo& mi
        SF_AOTC_ARG(AOT::InfoCollector* ic)
        SF_AOTC2_ARG(AOT::InfoCollector* ic)
        );
    virtual ~Tracer();

public:
    // Can throw exceptions.
    void EmitCode();

public:
    //
    MemoryHeap* GetHeap() const
    {
        return Heap;
    }

    //
    const TR::Block& GetFirstBlock() const
    {
        SF_ASSERT(!Blocks.IsEmpty());
        return *Blocks.GetFirst();
    }
    const TR::Block& GetCurrBlock() const
    {
        SF_ASSERT(CurrBlock);
        return *CurrBlock;
    }

    //
    VMAbcFile& GetFile() const
    {
        return CF.GetFile();
    }

#ifdef SF_AS3_ENABLE_EXPLICIT_GO
    //
    Instances::fl::GlobalObjectScript& GetGlobalObjectScript() const
    {
        return CF.GetGlobalObjectScript();
    }
#endif

    //
    const Abc::MethodBodyInfo::Exception& GetException() const
    {
        return CF.GetException();
    }

    //
    Abc::MbiInd GetMethodBodyInd() const
    {
        return CF.GetMethodBodyInd();
    }

    //
    const Abc::MethodBodyInfo& GetMethodBodyInfo() const
    {
        return CF.GetMethodBodyInfo();
    }

    //
    const Abc::MethodInfo& GetMethodInfo() const
    {
        return GetMethodBodyInfo().GetMethodInfo(GetFile().GetMethods());
    }

    bool NeedToDiscardResult() const
    {
        return CF.NeedToDiscardResult();
    }

    const ScopeStackType& GetSavedScope() const
    {
        return CF.GetSavedScope();
    }
    const Traits& GetOriginationTraits() const
    {
        return CF.GetOriginationTraits();
    }

    const Abc::File& GetAbcFile() const
    {
        return GetFile().GetAbcFile();
    }

    VM& GetVM() const
    {
        return GetFile().GetVM();
    }

    const Traits* GetValueTraits(const Value& v, const bool super_tr = false) const;
    const InstanceTraits::Traits& GetInstanceTraits(const Value& v) const;
    // This doesn't seem to be possible to implement.
    //const ClassTraits::Traits* GetClassTraits(const Value& v) const;

    UPInt GetPrintOffset() const
    {
        return PrintOffset;
    }

    const TR::State& GetState(const TR::Block& b) const
    {
        return b.GetState();
    }

    const Value& GetThis() const
    {
        SF_ASSERT(!States.IsEmpty());
        return States[0]->GetThis();
    }

    SF_DEBUG_CODE(ASString GetName() const { return ASString(CF.GetName());} )

public:
    void InitializeBlock(TR::Block& to, const TR::Block& from);
    // Return false in case of an exception.
    CheckResult MergeBlock(TR::Block& to, const TR::Block& from);

private:
    // Methods in this section deal with ORIGINAL Abc byte code.
    // They MAY NOT be used with newly generated byte code.

    int GetNextU30(UPInt opcode_offset = 0) const;

    void SkipOrigOpCode(Abc::TCodeOffset& opcode_cp, Abc::TCodeOffset new_cp);
    void RegisterOrigOpCode(Abc::TCodeOffset opcode_cp);

    Abc::Code::OpCode GetCurrOrigOpCode(Abc::TCodeOffset opcode_offset) const
    {
        using namespace Abc;
        TCodeOffset ccp = opcode_offset;
        return static_cast<Code::OpCode>(Read8(pCode, ccp));
    }
    Abc::Code::OpCode GetNextOrigOpCode(Abc::TCodeOffset bcp) const
    {
        using namespace Abc;
        TCodeOffset ccp = bcp;
        return static_cast<Code::OpCode>(Read8(pCode, ccp));
    }
    Abc::Code::OpCode GetOrigPrevOpCode(UPInt opcode_num_diff = 1) const;
    // offset - absolute byte offset.
    Abc::Code::OpCode GetOrigPrevOpCode2(Abc::TCodeOffset offset) const;

    Abc::Code::OpCode GetOrigValueConsumer(Abc::TCodeOffset bcp) const;
    Abc::TCodeOffset GetNextSwitchStmt(Abc::TCodeOffset& bcp) const;
    static bool IsOrigPropGetter(Abc::Code::OpCode opcode) 
    {
        using namespace Abc;
        return opcode == Code::op_getproperty;
    }
    static bool IsOrigPropSetter(Abc::Code::OpCode opcode) 
    {
        using namespace Abc;
        return opcode == Code::op_setproperty || opcode == Code::op_initproperty;
    }
    static bool IsOrigCall(Abc::Code::OpCode opcode);
    static bool IsOrigConstruct(Abc::Code::OpCode opcode);

private:
    // It will create a block if necessary.
    void StoreOffset(Abc::TCodeOffset bcp, const TR::State& st, SInt32 offset, int base SF_AOTC_ARG(SPtr<SNode>& target));

    Abc::Code::OpCode GetNewTopOpCode(UPInt num = 0) const;

    // Return true if no further processing is needed.
    // Can throw exceptions.
    bool SubstituteOpCode(const Abc::Code::OpCode opcode, Abc::TCodeOffset& bcp, TR::State& st);
    // Substitute op_getlocalXXX.
    bool SubstituteGetlocal(Abc::TCodeOffset& bcp, Abc::TCodeOffset ccp, TR::State& st, int src_reg_num);
    bool EmitFindProperty(TR::State& st, const int mn_index, bool get_prop = false, const Abc::Code::OpCode consumer = Abc::Code::op_nop SF_DEBUG_ARG(bool show_getprop = false));

    bool EmitGetClassTraits(TR::State& st, AS3::Multiname as3_mn, bool objOnStack = false, bool altered = false);

    void EmitGetAbsSlot(TR::State& st, const UPInt index SF_AOTC2_ARG(bool simulate = false));
    void EmitSetAbsSlot(TR::State& st, const SlotInfo& si, const Traits* val_tr, const UPInt index);
    void EmitInitAbsSlot(TR::State& st, const UPInt index);
    // For classes and global objects.
    bool EmitGetAbsObject(TR::State& st, const Value& value, bool objOnStack SF_AOTC2_ARG(bool simulate = false));
    // For regular objects.
    void EmitGetAbsObject2(TR::State& st, const Value& value SF_AOTC2_ARG(bool simulate = false));

    bool EmitGetSlot(TR::State& st, const Value& value, const UPInt index, bool objOnStack = false);

    void EmitGetOuterScope(TR::State& st, const UPInt index);

    // EmitGetProperty() always returns true.
    bool EmitGetProperty(const Abc::Code::OpCode opcode, TR::State& st, const TR::ReadMnObject& args, const int mn_index);
    bool EmitSetProperty(const Abc::Code::OpCode opcode, const TR::ReadValueMnObject& args, const int mn_index);
    bool EmitCall(const Abc::Code::OpCode opcode, TR::State& st, const TR::ReadArgsMnObject& args, const UInt32 mn_index);
    UPInt EmitDefaultArgs(const Value& func, const unsigned argc, TR::State& st);

    // Emit code, which should pop result of previous opcode.
    void EmitPopPrevResult(TR::State& st);

    // Translate offset into a block.
    TR::Block* GetBlock(Abc::TCodeOffset offset);
    CheckResult UpdateBlock(const Abc::TCodeOffset bcp SF_AOTC_ARG(Abc::Code::OpCode opcode) SF_AOTC2_ARG(Abc::Code::OpCode opcode));
    void MarkAllBlocks();
    // Create and mark blocks related to switch statements.
#if defined(SF_AS3_AOTC) || defined(SF_AS3_AOTC2)
    static bool IsTerminatingOpCode(Abc::Code::OpCode opcode);
#endif
#ifdef SF_AS3_AOTC
    void MarkSwitchStatements(Abc::TCodeOffset bcp);
#endif
    void SkipDeadCode(Abc::TCodeOffset& bcp);

    // Can throw exceptions.
    void TraceBlock(Abc::TCodeOffset bcp, const TR::Block& initBlock);

private:
    InstanceTraits::Traits& GetObjectType() const
    {
        return GetVM().GetClassTraitsObject().GetInstanceTraits();
    }
    InstanceTraits::Traits& GetClassType() const
    {
        return GetVM().GetClassTraitsClassClass().GetInstanceTraits();
    }
    InstanceTraits::Traits& GetFunctionType() const
    {
        return GetVM().GetClassTraitsFunction().GetInstanceTraits();
    }
    InstanceTraits::Traits& GetArrayType() const
    {
        return GetVM().GetITraitsArray();
    }
    InstanceTraits::Traits& GetStringType() const
    {
        return GetVM().GetITraitsString();
    }
    InstanceTraits::Traits& GetNumberType() const
    {
        return GetVM().GetITraitsNumber();
    }
    InstanceTraits::Traits& GetSIntType() const
    {
        return GetVM().GetITraitsSInt();
    }
    InstanceTraits::Traits& GetUIntType() const
    {
        return GetVM().GetITraitsUInt();
    }
    InstanceTraits::Traits& GetBooleanType() const
    {
        return GetVM().GetITraitsBoolean();
    }
    InstanceTraits::Traits& GetNamespaceType() const
    {
        return GetVM().GetITraitsNamespace();
    }

    InstanceTraits::Traits& GetXMLListType() const
    {
        return *GetVM().GetXMLSupport().GetITraitsXMLList();
    }

    InstanceTraits::Traits& GetNullType() const
    {
        return GetVM().GetITraitsNull();
    }
    InstanceTraits::Traits& GetVoidType() const
    {
        return GetVM().GetITraitsVoid();
    }

public:
    // Theoretically, we should pass InstanceTraits::Traits here.
    // But just Traits work fine. 
    bool IsAnyType(const Traits* tr) const;
    bool IsPrimitiveType(const Traits* tr) const;
    bool IsNumericType(const Traits* tr) const;
    bool IsNotRefCountedType(const Traits* tr) const;
    bool IsNotObjectType(const Traits* tr) const;
    bool IsNotNullableType(const InstanceTraits::Traits* tr) const;

    // Null is of Object type.
    bool IsStringType(const Traits* tr) const
    {
        return tr == &GetStringType();
    }
    bool IsStringType(const Value& v) const
    {
        return  IsStringType(GetValueTraits(v));
    }

    //
    bool IsBooleanType(const Traits* tr) const
    {
        return tr == &GetBooleanType();
    }
    bool IsBooleanType(const Value& v) const
    {
        return  IsBooleanType(GetValueTraits(v));
    }

    //
#ifdef SF_AS3_TR_UINT_AS_SINT
    bool IsSIntType(const Traits* tr) const
    {
        return tr == &GetSIntType() || tr == &GetUIntType();
    }
#else
    bool IsSIntType(const Traits* tr) const
    {
        return tr == &GetSIntType();
    }
#endif
    bool IsSIntType(const Value& v) const
    {
        return  IsSIntType(GetValueTraits(v));
    }

    //
#ifdef SF_AS3_TR_UINT_AS_SINT
    bool IsUIntType(const Traits* tr) const
    {
        return tr == &GetUIntType() || tr == &GetSIntType();
    }
#else
    bool IsUIntType(const Traits* tr) const
    {
        return tr == &GetUIntType();
    }
#endif
    bool IsUIntType(const Value& v) const
    {
        return IsUIntType(GetValueTraits(v));
    }

    //
    bool IsNumberType(const Traits* tr) const
    {
        return tr == &GetNumberType();
    }
    bool IsNumberType(const Value& v) const
    {
        return IsNumberType(GetValueTraits(v));
    }

    //
    bool IsNamespaceType(const Traits* tr) const
    {
        return tr == &GetNamespaceType();
    }
    bool IsNamespaceType(const Value& v) const
    {
        return IsNamespaceType(GetValueTraits(v));
    }

    bool IsNullType(const Traits* tr) const
    {
        return tr == &GetNullType();
    }

    bool ValueIsOfType(const Value& value, const ClassTraits::Traits& type) const;
    bool ValueIsOfType(const Value& value, const InstanceTraits::Traits& type) const
    {
        const InstanceTraits::Traits& itr = GetInstanceTraits(value);
        return itr.IsOfType(type);
    }

    Value::TraceNullType CanBeNull(const Value& v) const;
    Value::TraceNullType CanBeNull(const InstanceTraits::Traits& tr) const
    {
        return IsNotNullableType(&tr) ? Value::NotNull : Value::NullOrNot;
    }

private:
    CheckResult MergeValues(UPInt ind, TR::State& to, const TR::State& from, TR::MergeSet ms);
	CheckResult MergeLists(TR::State& to, const TR::State& from, bool check_size, TR::MergeSet ms);
    void ThrowMergeTypeError(const Traits& to_tr, const Traits& from_tr) const;

    // No assignment.
    void JoinSNodesNoAssignment(UPInt rnum, Value& to, const Value& from, TR::MergeSet ms);
    void JoinSNodesAssign(Value& to, const Value& from) const;
    void JoinSNodesUpdateType(Value& to, const Value& from, InstanceTraits::Traits& result_type) const;
    void JoinSNodesUpdateType(Value& to, const Value& from, ClassTraits::Traits& result_type) const;

private:
    Tracer& operator =(const Tracer&);

private:
    struct Recalculate
    {
        Recalculate(Abc::TCodeOffset p, int b = -1)
            : pos(p)
            , base(b)
        {
            SF_ASSERT(pos > 0);
            //SF_ASSERT(base <= 0);
        }
        Abc::TCodeOffset pos;
        // offset of a command base relative to "pos".
        // "base" uses target coordinates.
        int base;
    };

    TR::State& GetState(const TR::Block& b)
    {
        return b.GetState();
    }
    TR::State* GetStatePtr(const TR::Block& b) const
    {
        return b.GetStatePtr();
    }

private:
    void PushNewOpCode(Abc::Code::OpCode opcode);
    void PushNewOpCode(Abc::Code::OpCode opcode, UPInt arg1);
    void PushNewOpCodeNumber(Abc::Code::OpCode opcode, Value::Number arg1);
    void PushNewOpCode(Abc::Code::OpCode opcode, UPInt arg1, UPInt arg2);

    void PushNewOpCodeArg0(Abc::TOpCode::ValueType arg);
    void PushNewOpCodeArgNumber0(Value::Number arg);

//     void PushNewOpCodeArg(Abc::TOpCode::ValueType arg);
//     void PushNewOpCodeArgNumber(Value::Number arg);

    // PopNewOpCode will pop opcode itself and its arguments.
    void PopNewOpCode();

    Abc::TOpCode::ValueType GetNewOpCode() const;

    void CalcOpStackSize(Abc::Code::OpCode opcode, UPInt arg1 = 0, UPInt arg2 = 0);

private:
    bool IsException() const
    {
        return GetVM().IsException();
    }

    Value GetGlobalObject(const TR::State& st) const;
    static const ClassTraits::Traits* GetSlotCTraits(const Traits& tr, SlotIndex ind);

    // Return NULL if a valid block cannot be created.
    TR::Block* AddBlock(
        const TR::State& st, 
        Abc::TCodeOffset from, 
        TR::Block::EType type, 
        bool checkOpCode 
        );
    void AddBlock(Abc::TCodeOffset from);

public:
    // UI related stuff.

    FlashUI& GetUI() { return *this; }

    virtual void Output(OutputMessageType type, const char* msg);  
    virtual bool OnOpCode(Abc::TCodeOffset offset);

#if defined(SF_AS3_AOTC) || defined(SF_AS3_AOTC2)
public:
    AOT::SNodeBlock& GetFirstNodeBlock() const
    {
        SF_ASSERT(FirstNodeBlock.GetPtr());
        return *FirstNodeBlock;
    }
    AOT::InfoCollector& GetInfoCollector() const
    {
        SF_ASSERT(pIC);
        return *pIC;
    }

    void TraceAST(SNode& ast);

    // Optimization.
    // this_ctr in this case is ClassTraits of AS3 class.
    SPtr<SNode> Rewrite(const ClassTraits::Traits* this_ctr, const Abc::MbiInd cmbi_ind, bool add_prolog, const String& file_ns);

private:
    SPtr<SNode> MakeCallName(const Abc::Code::OpCode opcode, const Value& obj, UPInt slot_index) const;
#endif

#if defined(SF_AS3_AOTC)
public:
    SPtr<AOT::SNodeRT>& GetSNodeRT(UInt32 n, const SPtr<const Traits>& t SF_DEBUG_ARG(UPInt addr));
#endif

private:
    bool                            Done;
    MemoryHeap*                     Heap;
    const CallFrame&                CF;
    MethodInfo&                     MI;
    Abc::TCodeOffset                CurrOffset;
    const StringDataPtr             BCode;
    const UInt8*                    pCode;
    const Abc::TCodeOffset          CodeEnd;
    UPInt                           PrintOffset;
    TR::Block*                      CurrBlock;
    // Original opcode positions
    ArrayDH_POD<Abc::TCodeOffset, Mem_Stat> OrigOpcodePos;
    // New opcode positions
    ArrayDH_POD<Abc::TCodeOffset, Mem_Stat> NewOpcodePos;
    // Positions, which require postponed recalculation.
    ArrayDH_POD<Recalculate, Mem_Stat>      PosToRecalculate;
    // Map of original position to new (long) position.
    ArrayDH_POD<Abc::TCodeOffset, Mem_Stat> Orig2newPosMap;

    ArrayDH<TR::State*, Mem_Stat>           States;
    List<TR::Block>                         Blocks;
    ArrayDH<SPtr<InstanceTraits::fl::Catch> >   CatchTraits;

#if defined(SF_AS3_AOTC) || defined(SF_AS3_AOTC2)
    struct RTKey
    {
        RTKey(UInt32 n, const Traits* t) : Num(n), Type(t) {}

        bool operator==(const RTKey& other) const
        {
            return Num == other.Num && Type == other.Type;
        }

        struct HashFunctor
        {
            UPInt operator()(const RTKey& key) const
            { return key.Num + reinterpret_cast<UPInt>(key.Type); }
        };

        UInt32          Num;
        const Traits*   Type;
    };

    AOT::InfoCollector*         pIC;

    SPtr<AOT::SNodeBlock>       FirstNodeBlock;
#endif

#if defined(SF_AS3_AOTC)
    HashSetDH<Abc::TCodeOffset> HandledSwitchStmt;

    // This should be HashSetDH instead.
    typedef HashDH<RTKey, SPtr<AOT::SNodeRT>, RTKey::HashFunctor> RTHashType;

    // RTHash stands for Register Type Hash.
    // It allows us to search among AOT::SNodeRT nodes.
    RTHashType                  RTHash;
#endif
}; // class Tracer

namespace TR
{
    inline
    InstanceTraits::Traits& State::GetObjectType() const
    {
        return GetTracer().GetObjectType();
    }

    inline
    InstanceTraits::Traits& State::GetClassType() const
    {
        return GetTracer().GetClassType();
    }

    inline
    InstanceTraits::Traits& State::GetFunctionType() const
    {
        return GetTracer().GetFunctionType();
    }

    inline
    InstanceTraits::Traits& State::GetArrayType() const
    {
        return GetTracer().GetArrayType();
    }

    inline
    InstanceTraits::Traits& State::GetStringType() const
    {
        return GetTracer().GetStringType();
    }

    inline
    InstanceTraits::Traits& State::GetNumberType() const
    {
        return GetTracer().GetNumberType();
    }

    inline
    InstanceTraits::Traits& State::GetSIntType() const
    {
        return GetTracer().GetSIntType();
    }

    inline
    InstanceTraits::Traits& State::GetUIntType() const
    {
        return GetTracer().GetUIntType();
    }

    inline
    InstanceTraits::Traits& State::GetBooleanType() const
    {
        return GetTracer().GetBooleanType();
    }

    inline
    InstanceTraits::Traits& State::GetNamespaceType() const
    {
        return GetTracer().GetNamespaceType();
    }

    inline
    InstanceTraits::Traits& State::GetXMLListType() const
    {
        return GetTracer().GetXMLListType();
    }

    inline
    InstanceTraits::Traits& State::GetNullType() const
    {
        return GetTracer().GetNullType();
    }

    inline
    InstanceTraits::Traits& State::GetVoidType() const
    {
        return GetTracer().GetVoidType();
    }

    inline
    void State::exec_getglobalscope(bool simulate) 
    {
        /* We do not need to do this.
#if defined(SF_AS3_AOTC2)
        PushSNodeOC(AOT::SNodeOC::op_getglobalscope);
#endif
        */

        if (!simulate)
            GetTracer().PushNewOpCode(Abc::Code::op_getglobalscope);

        PushOp(GetTracer().GetGlobalObject(*this));
    }

    inline
    void State::PushNewOpCodeArg0(UInt32 arg) const
    {
        pTracer->PushNewOpCodeArg0(arg);
    }
    inline
    void State::PushNewOpCodeArgNumber0(Value::Number arg) const
    {
        pTracer->PushNewOpCodeArgNumber0(arg);
    }
//     inline
//     void State::PushNewOpCodeArg(UInt32 arg) const
//     {
//         pTracer->PushNewOpCodeArg(arg);
//     }
//     inline
//     void State::PushNewOpCodeArgNumber(Value::Number arg) const
//     {
//         pTracer->PushNewOpCodeArgNumber(arg);
//     }

    inline
    const Traits* State::GetValueTraits(const Value& v) const
    {
        return GetTracer().GetValueTraits(v);
    }

    inline
    const ScopeStackType& State::GetSavedScope() const
    {
        return GetTracer().GetSavedScope();
    }

    inline
    VMAbcFile& State::GetFile() const
    {
        return GetTracer().GetFile();
    }

    inline
    VM& State::GetVM() const
    {
        return GetTracer().GetVM();
    }

    inline
    Value::TraceNullType State::CanBeNull(const Value& v) const
    {
        return GetTracer().CanBeNull(v);
    }

    inline
    Value::TraceNullType State::CanBeNull(const InstanceTraits::Traits& tr) const
    {
        return GetTracer().CanBeNull(tr);
    }

    inline
    bool State::NeedToDiscardResult() const
    {
        return GetTracer().NeedToDiscardResult();
    }

} // namespace TR

}}} // namespace Scaleform { namespace GFx { namespace AS3 {

#endif // INC_AS3_TRACER_H

