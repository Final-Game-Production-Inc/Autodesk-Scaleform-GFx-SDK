/**************************************************************************

PublicHeader:   None
Filename    :   SF_StringHash.h
Content     :   String hash table used when optional case-insensitive
                lookup is required.
Created     :
Authors     :

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_StringHash_H
#define INC_SF_Kernel_StringHash_H

#include "SF_String.h"
#include "SF_Hash.h"

namespace Scaleform {

// *** StringHash

// This is a custom string hash table that supports case-insensitive
// searches through special functions such as GetCaseInsensitive, etc.
// This class is used for Flash labels, exports and other case-insensitive tables.

template<class U, class Allocator = AllocatorGH<U> >
class StringHash : public Hash<String, U, String::NoCaseHashFunctor, Allocator>
{
public:
    typedef U                                                        ValueType;
    typedef StringHash<U, Allocator>                                 SelfType;
    typedef Hash<String, U, String::NoCaseHashFunctor, Allocator>    BaseType;

public:    

    void    operator = (const SelfType& src) { BaseType::operator = (src); }

    bool    GetCaseInsensitive(const String& key, U* pvalue) const
    {
        String::NoCaseKey ikey(key);
        return BaseType::GetAlt(ikey, pvalue);
    }
    // Pointer-returning get variety.
    const U* GetCaseInsensitive(const String& key) const   
    {
        String::NoCaseKey ikey(key);
        return BaseType::GetAlt(ikey);
    }
    U*  GetCaseInsensitive(const String& key)
    {
        String::NoCaseKey ikey(key);
        return BaseType::GetAlt(ikey);
    }

    
    typedef typename BaseType::Iterator base_iterator;

    base_iterator    FindCaseInsensitive(const String& key)
    {
        String::NoCaseKey ikey(key);
        return BaseType::FindAlt(ikey);
    }

    // Set just uses a find and assigns value if found. The key is not modified;
    // this behavior is identical to Flash string variable assignment.    
    void    SetCaseInsensitive(const String& key, const U& value)
    {
        base_iterator it = FindCaseInsensitive(key);
        if (it != BaseType::End())
        {
            it->Second = value;
        }
        else
        {
            BaseType::Add(key, value);
        }
    } 
};


// Global version that assigns the id.
template<class U, int SID = Stat_Default_Mem>
class StringHash_sid : public StringHash<U, AllocatorGH<U, SID> >
{
    typedef StringHash_sid<U, SID>                SelfType;
    typedef StringHash<U, AllocatorGH<U, SID> >   BaseType;
public:    

    void    operator = (const SelfType& src) { BaseType::operator = (src); }
};




// ***** StringHashLH: Structure heap local String Hash table

// This hash table must live locally within a heap-allocated structure
// and is also allocates its string from the local heap by using StringLcl.
// 
// This hash table needs to have custom implementation, since all strings
// are passed by String reference, so the locally stored value is not the same.


template<class U, class HashF>
struct StringLH_HashNode
{
    StringLH   First;
    U          Second;

    // NodeRef is used to allow passing of elements into ghash_set
    // without using a temporary object.
    struct NodeRef
    {
        const String*    pFirst;
        const U*         pSecond;

        NodeRef(const String& f, const U& s) : pFirst(&f), pSecond(&s) { }
        NodeRef(const NodeRef& src)          : pFirst(src.pFirst), pSecond(src.pSecond) { }

        // Enable computation of ghash_node_hashf.
        inline UPInt GetHash() const        { return HashF()(*pFirst); } 
        // Necessary conversion to allow ghash_node::operator == to work.
        operator const String& () const    { return *pFirst; }
    };

    // Note: No default constructor is necessary.
    StringLH_HashNode(const StringLH_HashNode& src) : First(src.First), Second(src.Second) { }
    StringLH_HashNode(const NodeRef& src) : First(*src.pFirst), Second(*src.pSecond)  { }
    void operator = (const NodeRef& src)  { First  = *src.pFirst; Second = *src.pSecond; }

#ifdef SF_CC_RENESAS
    bool operator == (const NodeRef& src) const { return (First == *src.pFirst); }
#endif

    template<class K>
    bool operator == (const K& src) const  { return (First == src); }

    template<class K>
    static UPInt SF_STDCALL CalcHash(const K& data)   { return HashF()(data); }
    inline UPInt            GetHash() const           { return HashF()(First); }

    // Hash functors used with this node. A separate functor is used for alternative
    // key lookup so that it does not need to access the '.First' element.    
    struct NodeHashF    
    {    
        template<class K>
        UPInt  operator()(const K& data) const { return data.GetHash(); } 
    };    
    struct NodeAltHashF    
    {
        template<class K>
        UPInt  operator()(const K& data) const { return StringLH_HashNode<U,HashF>::CalcHash(data); }
    };
};


#undef new

template<class U, int SID = Stat_Default_Mem,
         class HashF = String::NoCaseHashFunctor,         
         class HashNode = StringLH_HashNode<U,HashF>,
         class Entry = HashsetCachedNodeEntry<HashNode, typename HashNode::NodeHashF> >
class StringHashLH
{
public:
    SF_MEMORY_REDEFINE_NEW(StringHashLH, SID)

    typedef AllocatorLH<U, SID>                               Allocator;

    // Types used for hash_set.
    typedef U                                                 ValueType;
    typedef StringHashLH<U, SID, HashF, HashNode, Entry>      SelfType;
    typedef typename HashNode::NodeHashF                      HashNodeF;
    typedef HashSet<HashNode, HashNodeF,
                     typename HashNode::NodeAltHashF,
                     Allocator, Entry >                       Container;

    // Actual hash table itself, implemented as hash_set.
    Container   mHash;

public:    

    StringHashLH()                                       { }
    StringHashLH(int sizeHint) : mHash(sizeHint)         { }
    StringHashLH(const SelfType& src) : mHash(src.mHash) { }
    ~StringHashLH()                                      { }

    void    operator = (const SelfType& src)    { mHash = src.mHash; }

    // Remove all entries from the Hash table.
    inline void    Clear()                  { mHash.Clear(); }
    // Returns true if the mHash is empty.
    inline bool    IsEmpty() const          { return mHash.IsEmpty(); }

    // Access (set).
    inline void    Set(const String& key, const U& value)
    {
        typename HashNode::NodeRef e(key, value);
        mHash.Set(e);
    }
    inline void    Add(const String& key, const U& value)
    {
        typename HashNode::NodeRef e(key, value);
        mHash.Add(e);
    }

    // Removes an element by clearing its Entry.
    inline void     Remove(const String& key)
    {   
        mHash.RemoveAlt(key);
    }
    template<class K>
    inline void     RemoveAlt(const K& key)
    {   
        mHash.RemoveAlt(key);
    }

    // Retrieve the value under the given key.    
    //  - If there's no value under the key, then return false and leave *pvalue alone.
    //  - If there is a value, return true, and set *Pvalue to the Entry's value.
    //  - If value == NULL, return true or false according to the presence of the key.    
    bool    Get(const String& key, U* pvalue) const   
    {
        const HashNode* p = mHash.GetAlt(key);
        if (p)
        {
            if (pvalue)
                *pvalue = p->Second;
            return true;
        }
        return false;
    }

    template<class K>
    bool    GetAlt(const K& key, U* pvalue) const   
    {
        const HashNode* p = mHash.GetAlt(key);
        if (p)
        {
            if (pvalue)
                *pvalue = p->Second;
            return true;
        }
        return false;
    }

    // Retrieve the pointer to a value under the given key.    
    //  - If there's no value under the key, then return NULL.    
    //  - If there is a value, return the pointer.    
    inline U*  Get(const String& key)
    {
        HashNode* p = mHash.GetAlt(key);
        return p ? &p->Second : 0;
    }
    inline const U* Get(const String& key) const
    {
        const HashNode* p = mHash.GetAlt(key);
        return p ? &p->Second : 0;
    }

    template<class K>
    inline U*  GetAlt(const K& key)
    {
        HashNode* p = mHash.GetAlt(key);
        return p ? &p->Second : 0;
    }
    template<class K>
    inline const U* GetAlt(const K& key) const
    {
        const HashNode* p = mHash.GetAlt(key);
        return p ? &p->Second : 0;
    }

    // Sizing methods - delegate to Hash.
    inline UPInt   GetSize() const               { return mHash.GetSize(); }    
    inline void    Resize(UPInt n)               { mHash.Resize(n); }
    inline void    SetSapacity(UPInt newSize)    { mHash.RemoveAlt(newSize); }

    // Iterator API, like STL.
    typedef typename Container::ConstIterator   ConstIterator;
    typedef typename Container::Iterator        Iterator;

    inline Iterator        Begin()              { return mHash.Begin(); }
    inline Iterator        End()                { return mHash.End(); }
    inline ConstIterator   Begin() const        { return mHash.Begin(); }
    inline ConstIterator   End() const          { return mHash.End();   }

    Iterator        Find(const String& key)          { return mHash.FindAlt(key);  }
    ConstIterator   Find(const String& key) const    { return mHash.FindAlt(key);  }

    template<class K>
    Iterator        FindAlt(const K& key)       { return mHash.FindAlt(key);  }
    template<class K>
    ConstIterator   FindAlt(const K& key) const { return mHash.FindAlt(key);  }

    
    // **** Case-Insensitive Lookup

    bool    GetCaseInsensitive(const String& key, U* pvalue) const
    {
        String::NoCaseKey ikey(key);
        return GetAlt(ikey, pvalue);
    }
    // Pointer-returning get variety.
    const U* GetCaseInsensitive(const String& key) const   
    {
        String::NoCaseKey ikey(key);
        return GetAlt(ikey);
    }
    U*  GetCaseInsensitive(const String& key)
    {
        String::NoCaseKey ikey(key);
        return GetAlt(ikey);
    }


   // typedef typename BaseType::Iterator base_iterator;

    Iterator    FindCaseInsensitive(const String& key)
    {
        String::NoCaseKey ikey(key);
        return FindAlt(ikey);
    }

    // Set just uses a find and assigns value if found. The key is not modified;
    // this behavior is identical to Flash string variable assignment.    
    void    SetCaseInsensitive(const String& key, const U& value)
    {
        Iterator it = FindCaseInsensitive(key);
        if (it != End())
        {
            it->Second = value;
        }
        else
        {
            Add(key, value);
        }
    } 

};

} // Scaleform 

// Redefine operator 'new' if necessary.
#if defined(SF_DEFINE_NEW)
#define new SF_DEFINE_NEW
#endif

#endif
