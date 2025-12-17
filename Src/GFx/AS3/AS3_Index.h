/**************************************************************************

Filename    :   AS3_Index.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_AS3_Index_H
#define INC_AS3_Index_H


#include "Kernel/SF_Types.h"

///////////////////////////////////////////////////////////////////////////
#include "Abc/AS3_Abc_Type.h"


namespace Scaleform { namespace GFx { namespace AS3 
{

///////////////////////////////////////////////////////////////////////////
class Traits;

// Begins with ZERO ...
// AbsoluteIndex is used as a dynamic slot index and/or object data item
// offset in SlotInfo.

class AbsoluteIndex
{
public:
	typedef AbsoluteIndex SelfType;

    explicit AbsoluteIndex(SPInt ind)
    : Index(ind)
    {
    }
    AbsoluteIndex(class RelativeIndex ind, const Traits& t);
	explicit AbsoluteIndex(class SlotIndex ind, const Traits& t);
    explicit AbsoluteIndex(class GlobalSlotIndex ind);
    
public:
    SelfType& operator =(const SelfType& other)
    {
        Index = other.Index;
        return *this;
    }

    //
    bool operator <(const SelfType& other)
    {
        return Index < other.Index;
    }
    bool operator <(UPInt other)
    {
        return Index < 0 ? true : (UPInt)Index < other;
    }

    //
    bool operator >(const SelfType& other)
    {
        return Index > other.Index;
    }
    bool operator >(UPInt other)
    {
        return Index < 0 ? false : (UPInt)Index > other;
    }

    ///
    void operator +=(const SelfType& other)
    {
        Index += other.Index;
    }
    void operator +=(SPInt num)
    {
        Index += num;
    }
    SelfType operator +(UPInt v)
    {
        return SelfType(Index + v);
    }
    SelfType operator +(const SelfType& other)
    {
        return SelfType(Index + other.Index);
    }
    SelfType& operator ++()
    {
        ++Index;
        return *this;
    }
    SelfType operator ++(int)
    {
        SPInt tmp = Index;
        ++Index;
        return SelfType(tmp);
    }

    ///
    void operator -=(const SelfType& other)
    {
        Index -= other.Index;
    }
    void operator -=(SPInt num)
    {
        Index -= num;
    }
    SelfType operator -(UPInt v)
    {
        return SelfType(Index - v);
    }
    SelfType operator -(const SelfType& other)
    {
        return SelfType(Index - other.Index);
    }
    SelfType& operator --()
    {
        --Index;
        return *this;
    }
    SelfType operator --(int)
    {
        SPInt tmp = Index;
        --Index;
        return SelfType(tmp);
    }
    
public:
    UInt32 Get() const
    {
        return static_cast<UInt32>(Index);
    }
    SPInt GetRaw() const
    {
        return Index;
    }
    bool IsValid() const
    {
        return Index >= 0;
    }
    
protected:
    SPInt Index;
};

///////////////////////////////////////////////////////////////////////////
// Begins with ZERO ...
class RelativeIndex
{
public:
	typedef RelativeIndex SelfType;

    explicit RelativeIndex(SPInt ind)
    : Index(ind)
    {
    }
    explicit RelativeIndex(class SlotIndex ind);
    RelativeIndex(AbsoluteIndex ind, const Traits& t);
    
public:
    SelfType& operator =(const SelfType& other)
    {
        Index = other.Index;
        return *this;
    }
    void operator +=(const SelfType& other)
    {
        Index += other.Index;
    }
    bool operator <(const SelfType& other)
    {
        return Index < other.Index;
    }
    bool operator <(UPInt other)
    {
        return Index < 0 ? true : (UPInt)Index < other;
    }
    SelfType operator +(UPInt v)
    {
        return SelfType(Index + v);
    }
    SelfType operator +(const SelfType& other)
    {
        return SelfType(Index + other.Index);
    }
    SelfType& operator ++()
    {
        ++Index;
        return *this;
    }
    SelfType operator ++(int)
    {
        SPInt tmp = Index;
        ++Index;
        return SelfType(tmp);
    }
    
public:
    UInt32 Get() const
    {
        return static_cast<UInt32>(Index);
    }
    bool IsValid() const
    {
        return Index >= 0;
    }
    
protected:
    SPInt Index;
};

///////////////////////////////////////////////////////////////////////////
// Begins with ONE ...
// It is a relative index + one.
class SlotIndex
{
public:
	typedef SlotIndex SelfType;

    explicit SlotIndex(UInt32 ind)
    : Index(ind)
    {
    }
    explicit SlotIndex(RelativeIndex ind)
    : Index(ind.Get() + 1)
    {
        SF_ASSERT(ind.IsValid());
    }
	SlotIndex(class GlobalSlotIndex ind, const Traits& t);

public:
    SelfType& operator =(const SelfType& other)
    {
        Index = other.Index;
        return *this;
    }

	//
	bool operator <(UInt32 other)
	{
		return Index < other;
	}
    bool operator <(const SelfType& other)
    {
        return Index < other.Index;
    }

	//
	bool operator >(UInt32 other)
	{
		return Index > other;
	}
	bool operator >(const SelfType& other)
	{
		return Index > other.Index;
	}

	//
	SelfType operator -(UInt32 v)
	{
		SF_ASSERT(Index >= v);
		return SelfType(Index - v);
	}
	SelfType operator -(const SelfType& other)
	{
		SF_ASSERT(Index >= other.Index);
		return SelfType(Index - other.Index);
	}

	//
    SelfType operator +(UInt32 v)
    {
        return SelfType(Index + v);
    }
    SelfType operator +(const SelfType& other)
    {
        return SelfType(Index + other.Index);
    }
    SelfType& operator ++()
    {
        ++Index;
        return *this;
    }
    SelfType operator ++(int)
    {
        UInt32 tmp = Index;
        ++Index;
        return SelfType(tmp);
    }
	void operator +=(const SelfType& other)
	{
		Index += other.Index;
	}
    
public:
    UInt32 Get() const
    {
        return Index;
    }
    bool IsValid() const
    {
        return Index > 0;
    }
    
protected:
    UInt32 Index;
};

///////////////////////////////////////////////////////////////////////////
// Begins with ONE ...
// It is an absolute index + one.
class GlobalSlotIndex
{
public:
	typedef GlobalSlotIndex SelfType;

	explicit GlobalSlotIndex(UInt32 ind)
		: Index(ind)
	{
	}
	explicit GlobalSlotIndex(AbsoluteIndex ind)
		: Index(ind.Get() + 1)
	{
		SF_ASSERT(ind.IsValid());
	}

public:
	SelfType& operator =(const SelfType& other)
	{
		Index = other.Index;
		return *this;
	}

	//
	bool operator <(UInt32 other)
	{
		return Index < other;
	}
	bool operator <(const SelfType& other)
	{
		return Index < other.Index;
	}

	//
	bool operator >(UInt32 other)
	{
		return Index > other;
	}
	bool operator >(const SelfType& other)
	{
		return Index > other.Index;
	}

	//
	SelfType operator -(UInt32 v)
	{
		SF_ASSERT(Index >= v);
		return SelfType(Index - v);
	}
	SelfType operator -(const SelfType& other)
	{
		SF_ASSERT(Index >= other.Index);
		return SelfType(Index - other.Index);
	}
    SelfType& operator --()
    {
        --Index;
        return *this;
    }
    SelfType operator --(int)
    {
        UInt32 tmp = Index;
        --Index;
        return SelfType(tmp);
    }
    void operator -=(UInt32 n)
    {
        Index -= n;
    }
    void operator -=(const SelfType& other)
    {
        Index -= other.Index;
    }

	//
	SelfType operator +(UInt32 v)
	{
		return SelfType(Index + v);
	}
	SelfType operator +(const SelfType& other)
	{
		return SelfType(Index + other.Get());
	}
	SelfType& operator ++()
	{
		++Index;
		return *this;
	}
	SelfType operator ++(int)
	{
		UInt32 tmp = Index;
		++Index;
		return SelfType(tmp);
	}
    void operator +=(UInt32 n)
    {
        Index += n;
    }
	void operator +=(const SelfType& other)
	{
		Index += other.Index;
	}

public:
	UInt32 Get() const
	{
		return Index;
	}
	bool IsValid() const
	{
		return Index > 0;
	}

protected:
	UInt32 Index;
};

inline
RelativeIndex::RelativeIndex(SlotIndex ind)
: Index(ind.Get() - 1)
{
}

}}} // namespace Scaleform { namespace GFx { namespace AS3 {

#endif // INC_AS3_Index_H

