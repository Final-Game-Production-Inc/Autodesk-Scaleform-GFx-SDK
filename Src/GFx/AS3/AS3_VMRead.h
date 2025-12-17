/**************************************************************************

Filename    :   AS3_VMRead.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_AS3_VMRead_H
#define INC_AS3_VMRead_H

#include "AS3_VM.h"


namespace Scaleform { namespace GFx { namespace AS3 
{

///////////////////////////////////////////////////////////////////////////
class StackReader
{
public:
    typedef StackReader SelfType;

    StackReader(VM& vm)
    : VMRef(vm)
    , OpStack(vm.OpStack)
    {
    }
    
public:
    void CheckObject(const Value& v) const
    {
        if (v.IsNullOrUndefined())
            GetVM().ThrowTypeError(VM::Error(
                v.IsNull() ? VM::eConvertNullToObjectError : VM::eConvertUndefinedToObjectError,
                GetVM()
            ));
    }

protected:
    VM& GetVM() const
    {
        return VMRef;
    }

    //
    VM::OpStackType& GetOpStack()
    {
        return OpStack;
    }
    const VM::OpStackType& GetOpStack() const
    {
        return OpStack;
    }

    //
    SF_INLINE
    void PopBack()
    {
        GetOpStack().PopBack();
    }
    SF_INLINE
    void PopBack2()
    {
        GetOpStack().PopBack2();
    }
    SF_INLINE
    void PopBack3()
    {
        GetOpStack().PopBack3();
    }
    SF_INLINE
    void PopBack(UPInt n)
    {
        GetOpStack().PopBack(static_cast<unsigned>(n));
    }

    //
    SF_INLINE
    Value& Back()
    {
        return GetOpStack().Top0();
    }
    SF_INLINE
    const Value& Back() const
    {
        return GetOpStack().Top0();
    }

protected:
    void Read(Multiname& obj);

private:
    SelfType& operator =(const SelfType&);

private:
    VM& VMRef;
    VM::OpStackType& OpStack;
};

// This is just a label at this time.
class MnReader
{
    /*
public:
    SF_INLINE
    MnReader(VMFile& file)
        : File(file)
    {
    }

public:
    VMFile& GetFile() const
    {
        return File;
    }
//     SF_INLINE
//     const Abc::Multiname& GetMultiname(UPInt ind) const
//     {
//         return GetFile().GetConstPool().GetMultiname(ind);
//     }

private:
    MnReader& operator =(const MnReader&);

private:
    VMFile& File;
    */
};

// ReadArgs classes are used to read arguments from stack in various call opcodes,
// including callproperty, callmethod, callsuper, construct, etc.

struct ReadArgs : public StackReader
{
    typedef ReadArgs SelfType;

    ReadArgs(VM& vm, UInt32 arg_count);
    
    SF_INLINE
    const Value* GetCallArgs() const
    {
        if (ArgNum <= eFixedArrSize)
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

    const unsigned ArgNum;
    Value FixedArr[eFixedArrSize];
    ValueArrayDH CallArgs;
};

struct ReadArgsObject : public ReadArgs
{
    typedef ReadArgsObject SelfType;

    SF_INLINE
    ReadArgsObject(VM& vm, UInt32 arg_count)
    : ReadArgs(vm, arg_count)
    , ArgObject(GetOpStack())
    {
        CheckObject(ArgObject);
    }
    
    Value ArgObject;

private:
    SelfType& operator =(const SelfType&);
};

struct ReadArgsObjectRef : public ReadArgs
{
    typedef ReadArgsObjectRef SelfType;

    SF_INLINE
    ReadArgsObjectRef(VM& vm, UInt32 arg_count)
        : ReadArgs(vm, arg_count)
        , ArgObject(Back())
    {
        CheckObject(ArgObject);
    }

    Value& ArgObject;

private:
    SelfType& operator =(const SelfType&);
};

struct ReadArgsObjectValue : public ReadArgsObject
{
    typedef ReadArgsObjectValue SelfType;

    SF_INLINE
    ReadArgsObjectValue(VM& vm, UInt32 arg_count)
    : ReadArgsObject(vm, arg_count)
    , value(GetOpStack())
    {
    }
    
    const Value value;
};

struct ReadArgsMn : public ReadArgs
{
    typedef ReadArgsMn SelfType;

    SF_INLINE
    ReadArgsMn(VMFile& file, UInt32 arg_count, const Abc::Multiname& mn)
    : ReadArgs(file.GetVM(), arg_count)
    , ArgMN(file, mn)
    {
        Read(ArgMN);
    }
    
    Multiname   ArgMN;

private:
    SelfType& operator =(const SelfType&);
};

struct ReadArgsMnObject : public ReadArgsMn
{
    typedef ReadArgsMnObject SelfType;

    SF_INLINE
    ReadArgsMnObject(VMFile& file, UInt32 arg_count, const Abc::Multiname& mn)
    : ReadArgsMn(file, arg_count, mn)
    , ArgObject(GetOpStack())
    {
        CheckObject(ArgObject);
    }
    
    const Value ArgObject;

private:
    SelfType& operator =(const SelfType&);
};

struct ReadArgsMnObjectRef : public ReadArgsMn
{
    typedef ReadArgsMnObjectRef SelfType;

    SF_INLINE
    ReadArgsMnObjectRef(VMFile& file, UInt32 arg_count, const Abc::Multiname& mn)
        : ReadArgsMn(file, arg_count, mn)
        , ArgObject(Back())
    {
        CheckObject(ArgObject);
    }

    Value& ArgObject;

private:
    SelfType& operator =(const SelfType&);
};

struct ReadMn : public StackReader, public MnReader
{
    typedef ReadMn SelfType;

    SF_INLINE
    ReadMn(VMFile& file, const Abc::Multiname& mn)
    : StackReader(file.GetVM())
    , ArgMN(file, mn)
    {
        Read(ArgMN);
    }
    
    Multiname   ArgMN;

private:
    SelfType& operator =(const SelfType&);
};

struct ReadValue : public StackReader
{
    typedef ReadValue SelfType;

    SF_INLINE
    ReadValue(VM& vm)
    : StackReader(vm)
    , ArgValue(GetOpStack())
    {
    }
    
    Value ArgValue;

private:
    SelfType& operator =(const SelfType&);
};

struct ReadObject : public StackReader
{
    typedef ReadObject SelfType;

    SF_INLINE
    ReadObject(VM& vm)
    : StackReader(vm)
    , ArgObject(GetOpStack())
    {
        CheckObject(ArgObject);
    }

    Value ArgObject;

private:
    SelfType& operator =(const SelfType&);
};

// Check ArgObject.
// Can throw exceptions.
struct ReadObjectRef : public StackReader
{
    typedef ReadObjectRef SelfType;

    SF_INLINE
    ReadObjectRef(VM& vm)
        : StackReader(vm)
        , ArgObject(Back())
    {
        CheckObject(ArgObject);
    }

    Value& ArgObject;

private:
    SelfType& operator =(const SelfType&);
};

// DO NOT check value.
// Doesn't throw exceptions.
struct ReadValueRef : public StackReader
{
    typedef ReadValueRef SelfType;

    SF_INLINE
    ReadValueRef(VM& vm)
        : StackReader(vm)
        , value(Back())
    {
    }

    Value& value;

private:
    SelfType& operator =(const SelfType&);
};

struct ReadValueObject : public ReadValue
{
    typedef ReadValueObject SelfType;

    SF_INLINE
    ReadValueObject(VM& vm)
    : ReadValue(vm)
    , ArgObject(GetOpStack())
    {
        CheckObject(ArgObject);
    }
    
    const Value ArgObject;

private:
    SelfType& operator =(const SelfType&);
};

struct ReadValueMn : public ReadValue, public MnReader
{
    typedef ReadValueMn SelfType;

    SF_INLINE
    ReadValueMn(VMFile& file, const Abc::Multiname& mn)
    : ReadValue(file.GetVM())
    , ArgMN(file, mn)
    {
        Read(ArgMN);
    }
    
    Multiname   ArgMN;

private:
    SelfType& operator =(const SelfType&);
};

struct ReadValueMnObject : public ReadValueMn
{
    typedef ReadValueMnObject SelfType;

    SF_INLINE
    ReadValueMnObject(VMFile& file, const Abc::Multiname& mn)
    : ReadValueMn(file, mn)
    , ArgObject(GetOpStack())
    {
        CheckObject(ArgObject);
    }
    
    const Value ArgObject;

private:
    SelfType& operator =(const SelfType&);
};

struct ReadMnObjectRef : public ReadMn
{
    typedef ReadMnObjectRef SelfType;

    SF_INLINE
    ReadMnObjectRef(VMFile& file, const Abc::Multiname& mn)
    : ReadMn(file, mn)
    , ArgObject(Back())
    {
        CheckObject(ArgObject);
    }
    
    Value& ArgObject;

private:
    SelfType& operator =(const SelfType&);
};
    
}}} // namespace Scaleform { namespace GFx { namespace AS3 {

#endif // INC_AS3_VMRead_H
