/**************************************************************************

PublicHeader:   None
Filename    :   SF_BitSet.h
Content     :   Template implementation for a simple BitSet
Created     :   Feb, 2010
Authors     :   Artem Bolgar, Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_BitSet_H
#define INC_SF_Kernel_BitSet_H

#include "SF_Allocator.h"

#undef new

namespace Scaleform {

// A simple non-expandable basic bitset class
template <class Allocator>
class FixedBitSetBase
{
public:
    FixedBitSetBase(const void* pheapAddr, unsigned bitsCount)
        : BitsCount(bitsCount)
    {
        unsigned dataLen = (BitsCount + 7)/8;
        pData = (UByte*)Allocator::Alloc(pheapAddr, dataLen, __FILE__, __LINE__);
        memset(pData, 0, dataLen);
    }
    FixedBitSetBase(const FixedBitSetBase& other)
        : BitsCount(other.BitsCount)
    {
        unsigned dataLen = (BitsCount + 7)/8;
        pData = (UByte*)Allocator::Alloc(Memory::GetHeapByAddress(other.pData), dataLen, __FILE__, __LINE__);
        memcpy(pData, other.pData, dataLen);
    }
    ~FixedBitSetBase()
    {
        Allocator::Free(pData);
    }

    FixedBitSetBase& operator =(const FixedBitSetBase& other)
    {
        if (this != &other)
        {
            Allocator::Free(pData);

            BitsCount = other.BitsCount;
            unsigned dataLen = (BitsCount + 7)/8;
            pData = (UByte*)Allocator::Alloc(Memory::GetHeapByAddress(other.pData), dataLen, __FILE__, __LINE__);
            memcpy(pData, other.pData, dataLen);
        }

        return *this;
    }

    void Set(unsigned bitIndex) 
    { 
        SF_ASSERT(bitIndex < BitsCount);
        pData[bitIndex >> 3] |= (1 << (bitIndex & 7));
    }

    void Set(unsigned bitIndex, bool s) 
    { 
        (s) ? Set(bitIndex) : Clear(bitIndex);
    }

    void Clear(unsigned bitIndex) 
    { 
        SF_ASSERT(bitIndex < BitsCount);
        pData[bitIndex >> 3] &= ~(1 << (bitIndex & 7));
    }

    bool IsSet(unsigned bitIndex) const
    { 
        SF_ASSERT(bitIndex < BitsCount);
        return (pData[bitIndex >> 3] & (1 << (bitIndex & 7))) != 0;
    }
    bool operator[](unsigned bitIndex) const { return IsSet(bitIndex); }

protected:
    UByte*      pData;
    unsigned    BitsCount;
};

template<int SID=Stat_Default_Mem>
class FixedBitSetLH : public FixedBitSetBase<AllocatorLH<UByte, SID> >
{
public:
    typedef FixedBitSetBase<AllocatorLH<UByte, SID> >     BaseType;

protected:
    void* getThis() { return this; }
public:
    FixedBitSetLH(unsigned bitsCount) : BaseType(getThis(), bitsCount) {}
};

template<int SID=Stat_Default_Mem>
class FixedBitSetDH : public FixedBitSetBase<AllocatorDH<UByte, SID> >
{
public:
    typedef FixedBitSetBase<AllocatorDH<UByte, SID> >     BaseType;

    FixedBitSetDH(MemoryHeap* pheap, unsigned bitsCount) : BaseType(pheap, bitsCount) {}
};

template<int SID=Stat_Default_Mem>
class FixedBitSetGH : public FixedBitSetBase<AllocatorGH<UByte, SID> >
{
public:
    typedef FixedBitSetBase<AllocatorGH<UByte, SID> >     BaseType;

    FixedBitSetGH(unsigned bitsCount) : BaseType(Memory::GetGlobalHeap(), bitsCount) {}
};

} // Scaleform

#endif // INC_SF_Kernel_BitSet_H
