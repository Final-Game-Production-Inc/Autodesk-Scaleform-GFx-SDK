/**************************************************************************

Filename    :   AS3_Abc.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "AS3_Abc.h"

namespace Scaleform { namespace GFx { namespace AS3 { 

    const TypeInfo* TypeInfo::None[] = {NULL};

}}} // namespace Scaleform { namespace GFx { namespace AS3 { 

namespace Scaleform { namespace GFx { namespace AS3 { namespace Abc 
{

///////////////////////////////////////////////////////////////////////////
template <>
int Read16(const UInt8* data, TCodeOffset& cp)
{
    // Let's help CPU parallelize code below. Unroll loop and no data dependencies.
    const int b1 = data[cp + 0];
    const int b2 = data[cp + 1];
    cp += 2;

    return (b2 << 8) | b1;
}

template <>
int ReadS24(const UInt8* data, TCodeOffset& cp)
{
    // Let's help CPU parallelize code below. Unroll loop and no data dependencies.
    const int b1 = data[cp + 0];
    const int b2 = data[cp + 1];
    const int b3 = data[cp + 2];
    cp += 3;

    int r = b3 << 16 | b2 << 8 | b1;
    
    if(b3 & 0x80) 
        r = -1 - ( r ^ 0xffffff );
    
    return r;
}

template <>
int ReadU30(const UInt8* data, TCodeOffset& cp)
{
    UInt32 shift = 0;
    UInt32 s = 0;
    int nr = 0;

    while (true)
    {
        int b = Read8(data, cp);
        ++nr;
        s |= (b & 127) << shift;
        shift += 7;

        if (!(b & 128) || shift >= 32)
            break;
    }

    return s;
}

template <>
Double ReadDouble(const UInt8* data, TCodeOffset& cp)
{
    // The bytes in the abc are little endian.
    union {
        double v;
#if (SF_BYTE_ORDER == SF_LITTLE_ENDIAN)
        struct { UInt32 lo, hi; } w;
#else
        struct { UInt32 hi, lo; } w;
#endif
    };

    // The words in memory can be little endian or big endian.
    // Let's help CPU parallelize code below. Unroll loop and no data dependencies.
    w.lo = data[cp + 0] | data[cp + 1] << 8 | data[cp + 2] << 16 | data[cp + 3] << 24;
    w.hi = data[cp + 4] | data[cp + 5] << 8 | data[cp + 6] << 16 | data[cp + 7] << 24;
    cp += 8;

    return v;
}

template <>
String ReadString(const UInt8* data, TCodeOffset& cp, UPInt size)
{
//     StringBuffer b(size);

    // !!! No checking.
    String b((const char*)(data + cp), size);
    cp += size;
    return b;
}
template <>
StringDataPtr ReadStringPtr(const UInt8* data, TCodeOffset& cp, UPInt size)
{
    // !!! No checking.
    StringDataPtr b((const char*)(data + cp), size);
    cp += size;
    return b;
}

///////////////////////////////////////////////////////////////////////////
template <>
int Read16(const UInt8*& cp)
{
    // Let's help CPU parallelize code below. Unroll loop and no data dependencies.
    const int b1 = *cp;
    const int b2 = *(cp + 1);
    cp += 2;

    return (b2 << 8) | b1;
}

template <>
int ReadS24(const UInt8*& cp)
{
    // Let's help CPU parallelize code below. Unroll loop and no data dependencies.
    const int b1 = *cp;
    const int b2 = *(cp + 1);
    const int b3 = *(cp + 2);
    cp += 3;

    int r = b3 << 16 | b2 << 8 | b1;

    if(b3 & 0x80) 
        r = -1 - ( r ^ 0xffffff );

    return r;
}

template <>
int ReadU30(const UInt8*& cp)
{
    UInt32 shift = 0;
    UInt32 s = 0;
    int nr = 0;

    while (true)
    {
        int b = Read8(cp);
        ++nr;
        s |= (b & 127) << shift;
        shift += 7;

        if (!(b & 128) || shift >= 32)
            break;
    }

    return s;
}

template <>
Double ReadDouble(const UInt8*& cp)
{
    // The bytes in the abc are little endian.
    union {
        double v;
#if (SF_BYTE_ORDER == SF_LITTLE_ENDIAN)
        struct { UInt32 lo, hi; } w;
#else
        struct { UInt32 hi, lo; } w;
#endif
    };

    // The words in memory can be little endian or big endian.
    // Let's help CPU parallelize code below. Unroll loop and no data dependencies.
    w.lo =       *cp | *(cp + 1) << 8 | *(cp + 2) << 16 | *(cp + 3) << 24;
    w.hi = *(cp + 4) | *(cp + 5) << 8 | *(cp + 6) << 16 | *(cp + 7) << 24;
    cp += 8;

    return v;
}

template <>
String ReadString(const UInt8*& cp, UPInt size)
{
    //     StringBuffer b(size);

    // !!! No checking.
    String b((const char*)cp, size);
    cp += size;
    return b;
}
template <>
StringDataPtr ReadStringPtr(const UInt8*& cp, UPInt size)
{
    // !!! No checking.
    StringDataPtr b((const char*)cp, size);
    cp += size;
    return b;
}

///////////////////////////////////////////////////////////////////////////
bool IsValidValueKind(UInt8 vk)
{
    switch(vk)
    {
    case CONSTANT_Int:
    case CONSTANT_UInt:
    case CONSTANT_Double:
    case CONSTANT_Utf8:
    case CONSTANT_True:
    case CONSTANT_False:
    case CONSTANT_Null:
    case CONSTANT_Undefined:
    case CONSTANT_Namespace:
    case CONSTANT_PackageNamespace:
    case CONSTANT_PackageInternalNs:
    case CONSTANT_ProtectedNamespace:
    case CONSTANT_ExplicitNamespace:
    case CONSTANT_StaticProtectedNs:
    case CONSTANT_PrivateNs:
        return true;
    default:
        break;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////
// So far we need operandCount only.
const Code::OpCodeInfo Code::opcode_info[0x100] =
{
    // operandCount  pop   pop_args
    //      canThrow    push pop_mn
    /* 00 */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0x00") },
    /* 01 */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0x01") },
    /* 02 */ {    0,  false,   0, 0, 0, 0    SF_DEBUG_ARG("nop") },
    /* 03 */ {    0,  true,    1, 0, 0, 0    SF_DEBUG_ARG("throw") },
    /* 04 */ {    1,  true,    1, 1, 0, 1    SF_DEBUG_ARG("getsuper") },
    /* 05 */ {    1,  true,    2, 0, 0, 1    SF_DEBUG_ARG("setsuper") },
    /* 06 */ {    1,  true,    0, 0, 0, 0    SF_DEBUG_ARG("dxns") },
    /* 07 */ {    0,  true,    1, 0, 0, 0    SF_DEBUG_ARG("dxnslate") },
    /* 08 */ {    1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("kill") },
    /* 09 */ {    0,  false,   0, 0, 0, 0    SF_DEBUG_ARG("label") },
    
    /* 0A */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0x0A") },
    /* 0A */ {    1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("inclocal_ti") }, // My own ...
    /* 0B */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0x0B") },
    /* 0B */ {    1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("declocal_ti") }, // My own ...
    
    /* 0C */ {    1,  true,    2, 0, 0, 0    SF_DEBUG_ARG("ifnlt") },
    /* 0D */ {    1,  true,    2, 0, 0, 0    SF_DEBUG_ARG("ifnle") },
    /* 0E */ {    1,  true,    2, 0, 0, 0    SF_DEBUG_ARG("ifngt") },
    /* 0F */ {    1,  true,    2, 0, 0, 0    SF_DEBUG_ARG("ifnge") },
    /* 10 */ {    1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("jump") },
    /* 11 */ {    1,  false,   1, 0, 0, 0    SF_DEBUG_ARG("iftrue") }, //
    /* 12 */ {    1,  false,   1, 0, 0, 0    SF_DEBUG_ARG("iffalse") }, //
    /* 13 */ {    1,  true,    2, 0, 0, 0    SF_DEBUG_ARG("ifeq") },
    /* 14 */ {    1,  true,    2, 0, 0, 0    SF_DEBUG_ARG("ifne") },
    /* 15 */ {    1,  true,    2, 0, 0, 0    SF_DEBUG_ARG("iflt") },
    /* 16 */ {    1,  true,    2, 0, 0, 0    SF_DEBUG_ARG("ifle") },
    /* 17 */ {    1,  true,    2, 0, 0, 0    SF_DEBUG_ARG("ifgt") },
    /* 18 */ {    1,  true,    2, 0, 0, 0    SF_DEBUG_ARG("ifge") },
    /* 19 */ {    1,  false,   2, 0, 0, 0    SF_DEBUG_ARG("ifstricteq") },
    /* 1A */ {    1,  false,   2, 0, 0, 0    SF_DEBUG_ARG("ifstrictne") },
    /* 1B */ {    2,  false,   1, 0, 0, 0    SF_DEBUG_ARG("lookupswitch") }, // Number of operands is variable.
    /* 1C */ {    0,  false,   1, 0, 0, 0    SF_DEBUG_ARG("pushwith") }, // Push on a scope stack ...
    /* 1D */ {    0,  false,   0, 0, 0, 0    SF_DEBUG_ARG("popscope") },
    /* 1E */ {    0,  true,    2, 1, 0, 0    SF_DEBUG_ARG("nextname") },
    /* 1F */ {    0,  true,    2, 1, 0, 0    SF_DEBUG_ARG("hasnext") }, // index ...
    /* 20 */ {    0,  false,   0, 1, 0, 0    SF_DEBUG_ARG("pushnull") },
    /* 21 */ {    0,  false,   0, 1, 0, 0    SF_DEBUG_ARG("pushundefined") },

    /* 22 */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0x22") },
    /* 22 */ {    0,  false,   1, 1, 0, 0    SF_DEBUG_ARG("not_tb") }, // My own ...

    /* 23 */ {    0,  true,    2, 1, 0, 0    SF_DEBUG_ARG("nextvalue") },
    /* 24 */ {    1,  false,   0, 1, 0, 0    SF_DEBUG_ARG("pushbyte") },
    /* 25 */ {    1,  false,   0, 1, 0, 0    SF_DEBUG_ARG("pushshort") },
    /* 26 */ {    0,  false,   0, 1, 0, 0    SF_DEBUG_ARG("pushtrue") },
    /* 27 */ {    0,  false,   0, 1, 0, 0    SF_DEBUG_ARG("pushfalse") },
    /* 28 */ {    0,  false,   0, 1, 0, 0    SF_DEBUG_ARG("pushnan") },
    /* 29 */ {    0,  false,   1, 0, 0, 0    SF_DEBUG_ARG("pop") },
    /* 2A */ {    0,  false,   1, 2, 0, 0    SF_DEBUG_ARG("dup") },
    /* 2B */ {    0,  false,   2, 2, 0, 0    SF_DEBUG_ARG("swap") },
    /* 2C */ {    1,  false,   0, 1, 0, 0    SF_DEBUG_ARG("pushstring") },
    /* 2D */ {    1,  false,   0, 1, 0, 0    SF_DEBUG_ARG("pushint") },
    /* 2E */ {    1,  false,   0, 1, 0, 0    SF_DEBUG_ARG("pushuint") },
    /* 2F */ {    1,  false,   0, 1, 0, 0    SF_DEBUG_ARG("pushdouble") },
    /* 30 */ {    0,  false,   1, 0, 0, 0    SF_DEBUG_ARG("pushscope") }, // Push on a scope stack ...
    /* 31 */ {    1,  false,   0, 1, 0, 0    SF_DEBUG_ARG("pushnamespace") },
    /* 32 */ {    2,  true,    0, 1, 0, 0    SF_DEBUG_ARG("hasnext2") },
    
    /* 33 */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0x33") },
    /* 33 */ {    1,  false,   1, 0, 0, 0    SF_DEBUG_ARG("iftrue_tb") }, // My own ...
    /* 34 */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0x34") },
    /* 34 */ {    1,  false,   1, 0, 0, 0    SF_DEBUG_ARG("iffalse_tb") }, // My own ...
    
    /* 35 */ //{   0,   true,    0    SF_DEBUG_ARG("li8") },
    /* 35 */ {    0,  false,   1, 1, 0, 0    SF_DEBUG_ARG("increment_tu") }, // My own ...
    /* 36 */ //{   0,   true,    0    SF_DEBUG_ARG("li16") },
    /* 36 */ {    0,  false,   1, 1, 0, 0    SF_DEBUG_ARG("decrement_tu") }, // My own ...
    /* 37 */ //{   0,   true,    0    SF_DEBUG_ARG("li32") },
    /* 37 */ {    1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("inclocal_tu") }, // My own ...
    /* 38 */ //{   0,   true,    0    SF_DEBUG_ARG("lf32") },
    /* 38 */ {    1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("declocal_tu") }, // My own ...
    /* 39 */ {   0,   true,    1, 1, 0, 0    SF_DEBUG_ARG("lf64") },
    /* 3A */ {   0,   true,    2, 0, 0, 0    SF_DEBUG_ARG("si8") },
    /* 3B */ {   0,   true,    2, 0, 0, 0    SF_DEBUG_ARG("si16") },
    /* 3C */ {   0,   true,    2, 0, 0, 0    SF_DEBUG_ARG("si32") },
    /* 3D */ {   0,   true,    2, 0, 0, 0    SF_DEBUG_ARG("sf32") },
    /* 3E */ {   0,   true,    2, 0, 0, 0    SF_DEBUG_ARG("sf64") },

    /* 3F */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0x3F") },
    /* 3F */ {    0,  false,   1, 1, 0, 0    SF_DEBUG_ARG("negate_ti") }, // My own ...

    /* 40 */ {    1,  false,   0, 1, 0, 0    SF_DEBUG_ARG("newfunction") }, // Tamarin throws exceptions in this opcode.
    /* 41 */ {    1,  true,    2, 1, 1, 0    SF_DEBUG_ARG("call") },
    /* 42 */ {    1,  true,    1, 1, 1, 0    SF_DEBUG_ARG("construct") },
    /* 43 */ {    2,  true,    1, 1, 1, 0    SF_DEBUG_ARG("callmethod") },
    /* 44 */ {    2,  true,    1, 1, 1, 0    SF_DEBUG_ARG("callstatic") },
    /* 45 */ {    2,  true,    1, 1, 1, 1    SF_DEBUG_ARG("callsuper") },
    /* 46 */ {    2,  true,    1, 1, 1, 1    SF_DEBUG_ARG("callproperty") },
    /* 47 */ {    0,  false,   0, 0, 0, 0    SF_DEBUG_ARG("returnvoid") },
    /* 48 */ {    0,  false,   1, 0, 0, 0    SF_DEBUG_ARG("returnvalue") },
    /* 49 */ {    1,  true,    1, 0, 1, 0    SF_DEBUG_ARG("constructsuper") },
    /* 4A */ {    2,  true,    1, 1, 1, 1    SF_DEBUG_ARG("constructprop") },
    /* 4S */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0x4B") },
    /* 4C */ {    2,  true,    1, 1, 1, 1    SF_DEBUG_ARG("callproplex") },
    /* 4D */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0x4D") },
    /* 4E */ {    2,  true,    1, 0, 1, 1    SF_DEBUG_ARG("callsupervoid") },
    /* 4F */ {    2,  true,    1, 0, 1, 1    SF_DEBUG_ARG("callpropvoid") },
    /* 50 */ {    0,  false,   1, 1, 0, 0    SF_DEBUG_ARG("sxi1") },
    /* 51 */ {    0,  false,   1, 1, 0, 0    SF_DEBUG_ARG("sxi8") },
    /* 52 */ {    0,  false,   1, 1, 0, 0    SF_DEBUG_ARG("sxi16") },
    /* 53 */ {    1,  true,    1, 1, 1, 0    SF_DEBUG_ARG("applytype") },

    /* 54 */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0x54") },
    /* 54 */ {    0,  false,   1, 1, 0, 0    SF_DEBUG_ARG("negate_td") }, // My own ...

    /* 55 */ {    1,  false,   0, 1, 1, 0    SF_DEBUG_ARG("newobject") }, // Tamarin throws exceptions in this opcode.
    /* 56 */ {    1,  false,   0, 1, 1, 0    SF_DEBUG_ARG("newarray") }, // Tamarin throws exceptions in this opcode.
    /* 57 */ {    0,  false,   0, 1, 0, 0    SF_DEBUG_ARG("newactivation") }, // Tamarin throws exceptions in this opcode.
    /* 58 */ {    1,  true,    1, 1, 0, 0    SF_DEBUG_ARG("newclass") },
    /* 59 */ {    1,  true,    1, 1, 0, 1    SF_DEBUG_ARG("getdescendants") },
    /* 5A */ {    1,  false,   0, 1, 0, 0    SF_DEBUG_ARG("newcatch") }, // Tamarin throws exceptions in this opcode.
    /* 5B */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0x5B") },
    /* 5C */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0x5C") },
    /* 5D */ {    1,  true,    0, 1, 0, 1    SF_DEBUG_ARG("findpropstrict") },
    /* 5E */ {    1,  true,    0, 1, 0, 1    SF_DEBUG_ARG("findproperty") },
    /* 5F */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0x5F") },
    /* 60 */ {    1,  true,    0, 1, 0, 0    SF_DEBUG_ARG("getlex") },
    /* 61 */ {    1,  true,    2, 0, 0, 1    SF_DEBUG_ARG("setproperty") },
    /* 62 */ {    1,  false,   0, 1, 0, 0    SF_DEBUG_ARG("getlocal") },
    /* 63 */ {    1,  false,   1, 0, 0, 0    SF_DEBUG_ARG("setlocal") },
    /* 64 */ {    0,  false,   0, 1, 0, 0    SF_DEBUG_ARG("getglobalscope") },
    /* 65 */ {    1,  false,   0, 1, 0, 0    SF_DEBUG_ARG("getscopeobject") },
    /* 66 */ {    1,  true,    1, 1, 0, 1    SF_DEBUG_ARG("getproperty") }, // undefined if property not found ...
    /* 67 */ {    1,  false,   0, 1, 0, 0    SF_DEBUG_ARG("getouterscope") }, // My own
    /* 68 */ {    1,  true,    2, 0, 0, 1    SF_DEBUG_ARG("initproperty") },

    /* 69 */ {    0,  false,   1, 2, 0, 0    SF_DEBUG_ARG("dup_nrc") }, // My own ... (Not ref-counted version of dup)
    /* 6A */ {    1,  true,    1, 1, 0, 1    SF_DEBUG_ARG("deleteproperty") },
    /* 6B */ {    0,  false,   1, 0, 0, 0    SF_DEBUG_ARG("pop_nrc") }, // My own ... (Not ref-counted version of pop)

    /* 6C */ {    1,  true,    1, 1, 0, 0    SF_DEBUG_ARG("getslot") },
    /* 6D */ {    1,  true,    2, 0, 0, 0    SF_DEBUG_ARG("setslot") },
    /* 6E */ {    1,  false,   0, 1, 0, 0    SF_DEBUG_ARG("getglobalslot") },
    /* 6F */ {    1,  false,   1, 0, 0, 0    SF_DEBUG_ARG("setglobalslot") },
    /* 70 */ {    0,  true,    1, 1, 0, 0    SF_DEBUG_ARG("convert_s") },
    /* 71 */ {    0,  true,    1, 1, 0, 0    SF_DEBUG_ARG("esc_xelem") },
    /* 72 */ {    0,  true,    1, 1, 0, 0    SF_DEBUG_ARG("esc_xattr") },
    /* 73 */ {    0,  true,    1, 1, 0, 0    SF_DEBUG_ARG("convert_i") },
    /* 74 */ {    0,  true,    1, 1, 0, 0    SF_DEBUG_ARG("convert_u") },
    /* 75 */ {    0,  true,    1, 1, 0, 0    SF_DEBUG_ARG("convert_d") },
    /* 76 */ {    0,  false,   1, 1, 0, 0    SF_DEBUG_ARG("convert_b") }, // Tamarin throws exceptions in this opcode.
    /* 77 */ {    0,  true,    1, 1, 0, 0    SF_DEBUG_ARG("convert_o") },
    /* 78 */ {    0,  true,    1, 1, 0, 0    SF_DEBUG_ARG("checkfilter") },

    /* 79 */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0x79") },
    /* 79 */ {    0,  false,   2, 1, 0, 0    SF_DEBUG_ARG("add_ti") }, // My own ...
    /* 7A */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0x7A") },
    /* 7A */ {    0,  false,   2, 1, 0, 0    SF_DEBUG_ARG("subtract_ti") }, // My own ...
    /* 7B */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0x7B") },
    /* 7B */ {    0,  false,   2, 1, 0, 0    SF_DEBUG_ARG("multiply_ti") }, // My own ...
    /* 7C */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0x7C") },
    /* 7C */ {    0,  false,   2, 1, 0, 0    SF_DEBUG_ARG("add_td") }, // My own ...
    /* 7D */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0x7D") },
    /* 7D */ {    0,  false,   2, 1, 0, 0    SF_DEBUG_ARG("subtract_td") }, // My own ...
    /* 7E */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0x7E") },
    /* 7E */ {    0,  false,   2, 1, 0, 0    SF_DEBUG_ARG("multiply_td") }, // My own ...
    /* 7F */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0x7F") },
    /* 7F */ {    0,  false,   2, 1, 0, 0    SF_DEBUG_ARG("divide_td") }, // My own ...

    /* 80 */ {    1,  true,    1, 1, 0, 0    SF_DEBUG_ARG("coerce") },
    /* 81 */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0x81") },
    /* 82 */ {    0,  true,    1, 1, 0, 0    SF_DEBUG_ARG("coerce_a") },
    /* 83 */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0x83") },
    /* 84 */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0x84") },
    /* 85 */ {    0,  true,    1, 1, 0, 0    SF_DEBUG_ARG("coerce_s") },   // convert_d is the same operation as coerce_d
    /* 86 */ {    1,  true,    1, 1, 0, 0    SF_DEBUG_ARG("astype") },
    /* 87 */ {    0,  true,    2, 1, 0, 0    SF_DEBUG_ARG("astypelate") },
    /* 88 */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0x88") },
    /* 89 */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0x89") },
    
    /* 8A */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0x8A") },
    /* 8A */ {    1,  false,   2, 0, 0, 0    SF_DEBUG_ARG("ifnlt_ti") }, // My own ...
    /* 8B */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0x8B") },
    /* 8B */ {    1,  false,   2, 0, 0, 0    SF_DEBUG_ARG("ifnle_ti") }, // My own ...
    /* 8C */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0x8C") },
    /* 8C */ {    1,  false,   2, 0, 0, 0    SF_DEBUG_ARG("ifngt_ti") }, // My own ...
    /* 8D */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0x8D") },
    /* 8D */ {    1,  false,   2, 0, 0, 0    SF_DEBUG_ARG("ifnge_ti") }, // My own ...
    /* 8E */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0x8E") },
    /* 8E */ {    1,  false,   2, 0, 0, 0    SF_DEBUG_ARG("ifeq_ti") }, // My own ...
    /* 8F */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0x8F") },
    /* 8F */ {    1,  false,   2, 0, 0, 0    SF_DEBUG_ARG("ifge_ti") }, // My own ...
                       
    /* 90 */ {    0,  true,    1, 1, 0, 0    SF_DEBUG_ARG("negate") },
    /* 91 */ {    0,  true,    1, 1, 0, 0    SF_DEBUG_ARG("increment") },
    /* 92 */ {    1,  true,    0, 0, 0, 0    SF_DEBUG_ARG("inclocal") },
    /* 93 */ {    0,  true,    1, 1, 0, 0    SF_DEBUG_ARG("decrement") },
    /* 94 */ {    1,  true,    0, 0, 0, 0    SF_DEBUG_ARG("declocal") },
    /* 95 */ {    0,  false,   1, 1, 0, 0    SF_DEBUG_ARG("typeof") },
    /* 96 */ {    0,  false,   1, 1, 0, 0    SF_DEBUG_ARG("not") },
    /* 97 */ {    0,  true,    1, 1, 0, 0    SF_DEBUG_ARG("bitnot") },
    
    /* 98 */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0x98") },
    /* 98 */ {    0,  false,   1, 1, 0, 0    SF_DEBUG_ARG("increment_ti") }, // My own ...
    /* 99 */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0x99") },
    /* 99 */ {    0,  false,   1, 1, 0, 0    SF_DEBUG_ARG("decrement_ti") }, // My own ...
    
    /* 9A */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0x9A") },
    /* 9B */ {   -1,  true,    2, 1, 0, 0    SF_DEBUG_ARG("add_d") },
    
    /* 9C */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0x9C") },
    /* 9C */ {    1,  false,   2, 0, 0, 0    SF_DEBUG_ARG("ifgt_ti") }, // My own ...
    /* 9D */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0x9D") },
    /* 9D */ {    1,  false,   2, 0, 0, 0    SF_DEBUG_ARG("ifle_ti") }, // My own ...
    /* 9E */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0x9E") },
    /* 9E */ {    1,  false,   2, 0, 0, 0    SF_DEBUG_ARG("iflt_ti") }, // My own ...
    /* 9F */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0x9F") },
    /* 9F */ {    1,  false,   2, 0, 0, 0    SF_DEBUG_ARG("ifne_ti") }, // My own ...
    
    /* A0 */ {    0,  true,    2, 1, 0, 0    SF_DEBUG_ARG("add") },
    /* A1 */ {    0,  true,    2, 1, 0, 0    SF_DEBUG_ARG("subtract") },
    /* A2 */ {    0,  true,    2, 1, 0, 0    SF_DEBUG_ARG("multiply") },
    /* A3 */ {    0,  true,    2, 1, 0, 0    SF_DEBUG_ARG("divide") },
    /* A4 */ {    0,  true,    2, 1, 0, 0    SF_DEBUG_ARG("modulo") },
    /* A5 */ {    0,  true,    2, 1, 0, 0    SF_DEBUG_ARG("lshift") },
    /* A6 */ {    0,  true,    2, 1, 0, 0    SF_DEBUG_ARG("rshift") },
    /* A7 */ {    0,  true,    2, 1, 0, 0    SF_DEBUG_ARG("urshift") },
    /* A8 */ {    0,  true,    2, 1, 0, 0    SF_DEBUG_ARG("bitand") },
    /* A9 */ {    0,  true,    2, 1, 0, 0    SF_DEBUG_ARG("bitor") },
    /* AA */ {    0,  true,    2, 1, 0, 0    SF_DEBUG_ARG("bitxor") },
    /* AB */ {    0,  true,    2, 1, 0, 0    SF_DEBUG_ARG("equals") },
    /* AC */ {    0,  true,    2, 1, 0, 0    SF_DEBUG_ARG("strictequals") },
    /* AD */ {    0,  true,    2, 1, 0, 0    SF_DEBUG_ARG("lessthan") },
    /* AE */ {    0,  true,    2, 1, 0, 0    SF_DEBUG_ARG("lessequals") },
    /* AF */ {    0,  true,    1, 1, 0, 0    SF_DEBUG_ARG("greaterthan") },
    /* B0 */ {    0,  true,    1, 1, 0, 0    SF_DEBUG_ARG("greaterequals") },
    /* B1 */ {    0,  true,    2, 1, 0, 0    SF_DEBUG_ARG("instanceof") },
    /* B2 */ {    1,  true,    1, 1, 0, 0    SF_DEBUG_ARG("istype") },
    /* B3 */ {    0,  true,    2, 1, 0, 0    SF_DEBUG_ARG("istypelate") },
    /* B4 */ {    0,  true,    2, 1, 0, 0    SF_DEBUG_ARG("in") },

    /* B5 */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0xB5") },
    /* B5 */ {    1,  false,   0, 1, 0, 0    SF_DEBUG_ARG("getabsobject") }, // My own ...
    /* B6 */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0xB6") },
    /* B6 */ {    1,  false,   1, 1, 0, 0    SF_DEBUG_ARG("getabsslot") }, // My own ...
    /* B7 */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0xB7") },
    /* B7 */ {    1,  false,    2, 0, 0, 0    SF_DEBUG_ARG("setabsslot") }, // My own ...
    /* B8 */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0xB8") },
    /* B8 */ {    1,  false,    2, 0, 0, 0    SF_DEBUG_ARG("initabsslot") }, // My own ...
    /* B9 */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0xB9") },
    /* B9 */ {    1,  false,    1, 1, 1, 0    SF_DEBUG_ARG("callsupermethod") }, // My own ...
    /* BA */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0xBA") },
    /* BA */ {    2,  false,    1, 1, 1, 0    SF_DEBUG_ARG("callgetter") }, // My own ...
    /* BB */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0xBB") },
    /* BB */ {    2,  false,    1, 1, 1, 0    SF_DEBUG_ARG("callsupergetter") }, // My own ...

    /* BC */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0xBC") },
    /* BC */ {    1,  false,   2, 0, 0, 0    SF_DEBUG_ARG("ifgt_td") }, // My own ...
    /* BD */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0xBD") },
    /* BD */ {    1,  false,   2, 0, 0, 0    SF_DEBUG_ARG("ifle_td") }, // My own ...
    /* BE */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0xBE") },
    /* BE */ {    1,  false,   2, 0, 0, 0    SF_DEBUG_ARG("iflt_td") }, // My own ...
    /* BF */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0xBF") },
    /* BF */ {    1,  false,   2, 0, 0, 0    SF_DEBUG_ARG("ifne_td") }, // My own ...
                       
    /* C0 */ {    0,  true,    1, 1, 0, 0    SF_DEBUG_ARG("increment_i") },
    /* C1 */ {    0,  true,    1, 1, 0, 0    SF_DEBUG_ARG("decrement_i") },
    /* C2 */ {    1,  true,    0, 0, 0, 0    SF_DEBUG_ARG("inclocal_i") },
    /* C3 */ {    1,  true,    0, 0, 0, 0    SF_DEBUG_ARG("declocal_i") },
    /* C4 */ {    0,  true,    1, 1, 0, 0    SF_DEBUG_ARG("negate_i") },
    /* C5 */ {    0,  true,    2, 1, 0, 0    SF_DEBUG_ARG("add_i") },
    /* C6 */ {    0,  true,    2, 1, 0, 0    SF_DEBUG_ARG("subtract_i") },
    /* C7 */ {    0,  true,    2, 1, 0, 0    SF_DEBUG_ARG("multiply_i") },
                       
    /* C8 */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0xC8") },
    /* C8 */ {    1,  false,   2, 0, 0, 0    SF_DEBUG_ARG("ifnlt_td") }, // My own ...
    /* C9 */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0xC9") },
    /* C9 */ {    1,  false,   2, 0, 0, 0    SF_DEBUG_ARG("ifnle_td") }, // My own ...
    /* CA */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0xCA") },
    /* CA */ {    1,  false,   2, 0, 0, 0    SF_DEBUG_ARG("ifngt_td") }, // My own ...
    /* CB */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0xCB") },
    /* CB */ {    1,  false,   2, 0, 0, 0    SF_DEBUG_ARG("ifnge_td") }, // My own ...
    /* CC */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0xCC") },
    /* CC */ {    1,  false,   2, 0, 0, 0    SF_DEBUG_ARG("ifeq_td") }, // My own ...
    /* CD */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0xCD") },
    /* CD */ {    1,  false,   2, 0, 0, 0    SF_DEBUG_ARG("ifge_td") }, // My own ...
    /* CE */ //{   -1,  false,   0    SF_DEBUG_ARG("OP_0xCE") },
    /* CE */ {    1,  false,   1, 1, 1, 0    SF_DEBUG_ARG("callobject") }, // My own ...

    /* CF */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0xCF") },
                       
    /* D0 */ {    0,  false,   0, 1, 0, 0    SF_DEBUG_ARG("getlocal_0") },
    /* D1 */ {    0,  false,   0, 1, 0, 0    SF_DEBUG_ARG("getlocal_1") },
    /* D2 */ {    0,  false,   0, 1, 0, 0    SF_DEBUG_ARG("getlocal_2") },
    /* D3 */ {    0,  false,   0, 1, 0, 0    SF_DEBUG_ARG("getlocal_3") },
    /* D4 */ {    0,  false,   1, 0, 0, 0    SF_DEBUG_ARG("setlocal_0") },
    /* D5 */ {    0,  false,   1, 0, 0, 0    SF_DEBUG_ARG("setlocal_1") },
    /* D6 */ {    0,  false,   1, 0, 0, 0    SF_DEBUG_ARG("setlocal_2") },
    /* D7 */ {    0,  false,   1, 0, 0, 0    SF_DEBUG_ARG("setlocal_3") },
                       
#ifdef ENABLE_STRICT_SETSLOT
    /* D8 */ {    1,  false,   2, 0, 0, 0    SF_DEBUG_ARG("setslot_str") }, // My own ...
    /* D9 */ {    1,  false,   2, 0, 0, 0    SF_DEBUG_ARG("setslot_num") }, // My own ...
    /* DA */ {    1,  false,   2, 0, 0, 0    SF_DEBUG_ARG("setslot_uint") }, // My own ...
    /* DB */ {    1,  false,   2, 0, 0, 0    SF_DEBUG_ARG("setslot_sint") }, // My own ...
    /* DC */ {    1,  false,   2, 0, 0, 0    SF_DEBUG_ARG("setslot_bool") }, // My own ...
    /* DD */ {    1,  false,   2, 0, 0, 0    SF_DEBUG_ARG("setslot_value") }, // My own ...
    /* DE */ {    1,  false,   2, 0, 0, 0    SF_DEBUG_ARG("setslot_obj_as") }, // My own ...
    /* DF */ {    1,  false,   2, 0, 0, 0    SF_DEBUG_ARG("setslot_obj_cpp") }, // My own ...
#else
    /* D8 */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0xD8") },
    /* D9 */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0xD9") },
    /* DA */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0xDA") },
    /* DB */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0xDB") },
    /* DC */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0xDC") },
    /* DD */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0xDD") },
    /* DE */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0xDE") },
    /* DF */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0xDF") },
#endif

    /* E0 */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0xE0") },
    /* E1 */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0xE1") },
    /* E2 */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0xE2") },
    /* E3 */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0xE3") },
    /* E4 */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0xE4") },
    /* E5 */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0xE5") },
    /* E6 */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0xE6") },
    /* E7 */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0xE7") },
    /* E8 */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0xE8") },
    /* E9 */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0xE9") },
    /* EA */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0xEA") },
    /* EB */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0xEB") },
    /* EC */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0xEC") },
    /* ED */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0xED") },
    /* EE */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0xEE") },
                       
    /* EF */ {    4,  true,    0, 0, 0, 0    SF_DEBUG_ARG("debug") }, // Tamarin throws exceptions in this opcode.
    /* F0 */ {    1,  true,    0, 0, 0, 0    SF_DEBUG_ARG("debugline") }, // Tamarin throws exceptions in this opcode.
    /* F1 */ {    1,  true,    0, 0, 0, 0    SF_DEBUG_ARG("debugfile") }, // Tamarin throws exceptions in this opcode.
    /* F2 */ {    1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0xF2") }, // op_0xF2 is used internally by the Tamarin VM.
    /* F3 */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0xF3") },
                       
    /* F4 */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0xF4") },
    /* F5 */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0xF5") },
    /* F6 */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0xF6") },
    /* F7 */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0xF7") },
    /* F8 */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0xF8") },
    /* F9 */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0xF9") },
    /* FA */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0xFA") },
    /* FB */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0xFB") },
    /* FC */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0xFC") },
    /* FD */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0xFD") },
    /* FE */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0xFE") },
    /* FF */ {   -1,  false,   0, 0, 0, 0    SF_DEBUG_ARG("OP_0xFF") },
    // END
};

///////////////////////////////////////////////////////////////////////////
MethodBodyInfo::MethodBodyInfo()
: method_info_ind(-1)
, max_stack(-1)
, local_reg_count(-1)
, init_scope_depth(-1)
, max_scope_depth(-1)
{
}

MethodBodyInfo::~MethodBodyInfo()
{
}

///////////////////////////////////////////////////////////////////////////
MethodBodyInfo::ExceptionInfo::ExceptionInfo()
: from(0)
, to(0)
, target(0)
, exc_type_ind(0)
, var_name_ind(0)
{
}

MethodBodyInfo::ExceptionInfo::ExceptionInfo(UInd _from, UInd _to, UInd _target, UInd _exc_type_ind, UInt32 _var_name_ind)
: from(_from)
, to(_to)
, target(_target)
, exc_type_ind(_exc_type_ind)
, var_name_ind(_var_name_ind)
{
}

///////////////////////////////////////////////////////////////////////////
bool MethodBodyInfo::Exception::FindExceptionInfo(Abc::TCodeOffset offset, UPInt& handler_num) const
{
    const UPInt size = info.GetSize();
    for(; handler_num < size; ++handler_num)
    {
        const ExceptionInfo& ei = Get(handler_num);

        if (offset >= static_cast<TCodeOffset>(ei.GetFrom()) && offset <= static_cast<TCodeOffset>(ei.GetTo()))
            return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////
void File::Clear()
{
    // DataSize = 0; // We do not clear DataSize on purpose.
    MinorVersion = 0;
    MajorVersion = 0;
    Const_Pool.Clear();
    Methods.Clear();
    Metadata.Clear();
    Traits.Clear();
    AS3_Classes.Clear();
    Scripts.Clear();
    MethodBodies.Clear();
}

File::~File()
{
}

///////////////////////////////////////////////////////////////////////////
MethodBodyTable::~MethodBodyTable()
{
    const UPInt size = Info.GetSize();
    for (UPInt i = 0; i < size; ++i)
        delete Info[i];
}

}}}} // namespace Scaleform { namespace GFx { namespace AS3 { namespace Abc {


