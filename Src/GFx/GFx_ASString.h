/**************************************************************************

PublicHeader:   None
Filename    :   GFx_ASString.h
Content     :   String manager implementation for Action Script
Created     :   November 7, 2006
Authors     :   Michael Antonov, Sergey Sikorskiy

Notes       :    Implements optimized GASString class, which acts as a
hash key for strings allocated from GASStringManager.

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_ASSTRING_H
#define INC_SF_GFX_ASSTRING_H

#include "Kernel/SF_Types.h"
#include "Kernel/SF_Array.h"
#include "Kernel/SF_Hash.h"
#include "Kernel/SF_RefCount.h"

#include "Kernel/SF_UTF8Util.h"
#include "GFx/GFx_String.h"

#include "GFx/GFx_PlayerStats.h"

#include <string.h>
#ifdef SF_OS_PS3
# include <wctype.h>
#endif // SF_OS_PS3

namespace Scaleform { namespace GFx {

// ***** Classes Declared

class   ASString;
class   ASStringManager;
struct  ASStringNode;

// Log forward declaration - for leak report
class   LogState;

// String node - stored in the manager table.

struct ASStringNode
{        

    const char*         pData;
    ASStringManager*    pManager;
    union {
        ASStringNode*   pLower;
        ASStringNode*   pNextAlloc;
    };
    UInt32              RefCount;
    UInt32              HashFlags;
    unsigned            Size;


    // *** Utility functions

    ASStringManager*   GetManager() const
    {
        return pManager;
    }
    inline UInt32  GetHashCode()
    {
        return HashFlags & 0x00FFFFFF;
    }

    void    AddRef()
    {
        RefCount++;
    }
    void    Release()
    {
        if (--RefCount == 0)
            ReleaseNode();
    }

    // Releases the node to the manager.
    void    ReleaseNode();    
    // Resolves pLower to a valid value.    
    void    ResolveLowercase_Impl();

    void    ResolveLowercase()
    {
        // This should not be called if lowercase was already resolved,
        // or we would overwrite it.
        SF_ASSERT(pLower == 0);
        ResolveLowercase_Impl();
    }

    bool    IsNull() const;
};




// ***** GASString - ActionScript string implementation.

// GASString is represented as a pointer to a unique node so that
// comparisons can be implemented very fast. Nodes are allocated by
// and stored within GASStringManager.
// GASString objects can be created only by string manager; default
// constructor is not implemented.

struct ASStringNodeHolder
{
    ASStringNodeHolder(ASStringNode* pnode = NULL): pNode(pnode) {}
    ASStringNodeHolder(const ASStringNodeHolder& other): pNode(other.pNode) {}

    ASStringNode*  pNode;
};

class ASString;

// ASConstString is a helper class, which provides extra functionality
// over a raw ASStringNode*.
// Warning: this class doesn't use reference counting in order to reduce 
// performance overhead.
// The main property of ASConstString is that data it holds cannot be changed.

// Do not derive from GNewOverrideBase and do not new!
class ASConstString : protected ASStringNodeHolder
{
public:
    explicit ASConstString(ASStringNode *pnode)
    : ASStringNodeHolder(pnode)
    {
        SF_ASSERT(pNode);
        SF_ASSERT(pNode->pData);
    }
    explicit ASConstString(const ASString& other);
    ASConstString(const ASConstString& other)
    : ASStringNodeHolder(other)
    {
        SF_ASSERT(pNode);
        SF_ASSERT(pNode->pData);
    }

public:
    enum FlagConstants
    {
        Flag_HashMask       = 0x00FFFFFF,
        Flag_FlagMask       = 0xFF000000,
        // Flags
        Flag_Builtin        = 0x80000000,
        Flag_ConstData      = 0x40000000,
        Flag_StandardMember = 0x20000000,
        Flag_CaseInsensitive= 0x10000000, // used with StandardMembers

        // This flag is set if GetLength() == GetSize() for a string.
        // Avoid extra scanning is Substring and indexing logic.
        Flag_LengthIsSize   = 0x08000000,

        // Determining whether a string a path is pricey, so we cache this information
        // in a string node. Flag_PathCheck is set if a check was made for a path.
        // Flag_IsNotPath is set if we have determined that this string is not to be a path.
        // If a check was not made yet, Flag_IsNotPath is always cleared.
        Flag_PathCheck      = 0x04000000,
        Flag_IsNotPath      = 0x02000000
    };

public:
    // Pointer to raw buffer.
    const char* ToCStr() const          { return pNode->pData; }
    const char* GetBuffer() const       { return pNode->pData; }

    // Size of string characters without 0. Raw count, not UTF8.
    unsigned    GetSize() const         { return pNode->Size; }
    bool        IsEmpty() const         { return pNode->Size == 0; }

    bool        IsNull() const;

    unsigned    GetHashFlags() const    { return pNode->HashFlags; }
    unsigned    GetHash() const         { return GetHashFlags() & Flag_HashMask; }

    ASStringNode*       GetNode() const    { return pNode; }
    ASStringManager*    GetManager() const { return pNode->GetManager(); }

    static UInt32   HashFunction(const char *pchar, UPInt length);

    // *** UTF8 Aware functions.
    // The rest of the functions here operate in UTF8. For example,    

    // Returns length in characters.
    unsigned        GetLength() const;
    // GetCharAt() allows to have zero characters inside of a string.
    UInt32          GetCharAt(unsigned index) const;
    UInt32          GetFirstCharAt(UPInt index, const char** offset) const;
    UInt32          GetNextChar(const char** offset) const;

    ASStringNode*   AppendCharNode(UInt32 ch) const;
    ASStringNode*   AppendStringNode(const char* str) const;
    ASStringNode*   AppendStringNode(const ASConstString& str) const;

    // Returns a String that's a substring of this.
    //  -start is the index of the first UTF8 character you want to include.    
    //  -end is the index one past the last UTF8 character you want to include.
    ASStringNode*   SubstringNode(int start, int end) const;

    // Returns a String with truncated whitespace.
    ASStringNode*   TruncateWhitespaceNode() const;

    // Case-converted strings.
    ASStringNode*   ToUpperNode() const; 
    ASStringNode*   ToLowerNode() const;

public:
    // *** Operators.

    // Accesses raw bytes returned by GetSize.
    char operator [] (unsigned index) const
    {
        SF_ASSERT(index < pNode->Size);
        return pNode->pData[index];
    }
    ASConstString operator + (const char* str) const
    {
        return ASConstString(AppendStringNode(str));
    }
    ASConstString operator + (const ASConstString& str) const
    {
        return ASConstString(AppendStringNode(str));
    }

    // Comparison.
    bool    operator == (const ASConstString& str) const
    {
        return pNode == str.pNode;
    }    
    bool    operator != (const ASConstString& str) const
    {
        return pNode != str.pNode;
    }
    bool    operator == (const char* str) const
    {
        return SFstrcmp(pNode->pData, str) == 0;
    }
    bool    operator != (const char* str) const
    {
        return !operator == (str);
    }

    // Compares provide the same ordering for UTF8 due to
    // the nature of data.
    bool    operator < (const char* pstr) const
    {
        return SFstrcmp(ToCStr(), pstr) < 0;
    }
    bool    operator<(const ASConstString& str) const
    {
        return *this < str.ToCStr();
    }
    bool    operator > (const char* pstr) const
    {
        return SFstrcmp(ToCStr(), pstr) > 0;
    }
    bool    operator > (const ASConstString& str) const
    {
        return *this > str.ToCStr();
    }

public:
    // These functions compare strings with using current locale. They might be used
    // for sorting arrays of strings. By default, strings are sorted by Unicode order,
    // however this is not always acceptable. For example, in French, sorting of array
    // ['a','A','Z','z','e','ê','é'] should give the result ['a','A','e','é','ê','z','Z'], 
    // if locale is used rather than just Unicode order.
    int LocaleCompare_CaseCheck(const char* pstr, UPInt len, bool caseSensitive) const;
    int LocaleCompare_CaseCheck(const ASConstString& str, bool caseSensitive) const
    {
        return LocaleCompare_CaseCheck(str.ToCStr(), str.GetLength(), caseSensitive);
    }

    int LocaleCompare(const char* pstr, UPInt len = SF_MAX_UPINT) const
    {
        return LocaleCompare_CaseCheck(pstr, len, true);
    }
    int LocaleCompare_CaseInsensitive(const char* pstr, UPInt len = SF_MAX_UPINT) const
    {
        return LocaleCompare_CaseCheck(pstr, len, false);
    }

    bool    Compare_CaseCheck(const ASConstString &str, bool caseSensitive) const
    {
        if (caseSensitive)
            return pNode == str.pNode;
        // For case in-sensitive strings we need to resolve lowercase.
        ResolveLowercase();
        str.ResolveLowercase();
        return pNode->pLower == str.pNode->pLower;
    }

    // Compares constants, assumes that 
    bool    CompareBuiltIn_CaseCheck(const ASConstString &str, bool caseSensitive) const
    {
        SF_ASSERT((pNode->HashFlags & Flag_Builtin) != 0);
        if (caseSensitive)
            return pNode == str.pNode;
        // For case in-sensitive strings we need to resolve lowercase.
        str.ResolveLowercase();
        return pNode->pLower == str.pNode->pLower;
    }


    // Compares constants case-insensitively
    bool    CompareBuiltIn_CaseInsensitive(const ASConstString& str) const
    {
        SF_ASSERT((pNode->HashFlags & Flag_Builtin) != 0);        
        // For case in-sensitive strings we need to resolve lowercase.
        str.ResolveLowercase();        
        return pNode->pLower == str.pNode->pLower;
    }

    // Compares constants case-insensitively
    bool    Compare_CaseInsensitive_Resolved(const ASConstString &str) const
    {
        SF_ASSERT(pNode->pLower != 0);
        // For case in-sensitive strings we need to resolve lowercase.
        str.ResolveLowercase();        
        return pNode->pLower == str.pNode->pLower;
    }

    // Compares constants case-insensitively.
    // Assumes that pLower MUST be resolved in both strings.
    bool    CompareBuiltIn_CaseInsensitive_Unchecked(const ASConstString &str) const
    {
        SF_ASSERT((pNode->HashFlags & Flag_Builtin) != 0);
        SF_ASSERT(str.pNode->pLower != 0);
        return pNode->pLower == str.pNode->pLower;
    }

public:
    // *** Utils.
    void ResolveLowercase() const
    {
        if (pNode->pLower == 0)
            pNode->ResolveLowercase();
    }

    // ***** Custom Methods

    // Path flag access inlines.
    bool    IsNotPath() const                 { return (GetHashFlags() & ASConstString::Flag_IsNotPath) != 0;  }
    void    SetPathFlags(UInt32 flags) const  { pNode->HashFlags |= flags; }

    // Determines if this string is a built-in.
    bool    IsBuiltin() const           { return (GetHashFlags() & ASConstString::Flag_Builtin) != 0;  }
    // Determines if this string is a standard member.
    bool    IsStandardMember() const    { return (GetHashFlags() & ASConstString::Flag_StandardMember) != 0;  }

    bool    IsCaseInsensitive() const    
    { 
        return (GetHashFlags() & Flag_CaseInsensitive) != 0;  
    }
};

// Do not derive from GNewOverrideBase and do not new!
class ASString : public ASConstString
{
    friend class ASStringManager;

public:

    // *** Create/Destroy: can

    explicit ASString(ASStringNode *pnode)
    : ASConstString(pnode)
    {
        pNode->AddRef();
    }

    explicit ASString(const ASConstString& src)
    : ASConstString(src)
    {
        pNode->AddRef();
    }

    ASString(const ASString& src)
    : ASConstString(src)
    {
        pNode->AddRef();
    }
    ~ASString()
    {        
        pNode->Release();
    }

    // *** General Functions

    void        Clear();

    void        SetNull();

    // *** UTF8 Aware functions.

    // The rest of the functions here operate in UTF8. For example,    
    ASString    AppendChar(UInt32 ch) const { return ASString(AppendCharNode(ch)); }

    void        Append(const char* str, UPInt len);
    void        Append(const char* str) { Append(str, strlen(str)); }
    void        Append(const ASString& str);

    /* No Insert or Remove for now - not necessary.

    GASString& Insert(const GASString& substr, int posAt, int len = -1);
    String&   Insert(const char* substr, int posAt, int len = -1);
    String&   Insert(char ch, int posAt);

    void        Remove(int posAt, int len = 1);
    */  

    // Returns a String that's a substring of this.
    //  -start is the index of the first UTF8 character you want to include.    
    //  -end is the index one past the last UTF8 character you want to include.
    ASString    Substring(int start, int end) const { return ASString(SubstringNode(start, end)); }

    // Returns a String with truncated whitespace.
    ASString    TruncateWhitespace() const { return ASString(TruncateWhitespaceNode()); }

    // Case-converted strings.
    ASString    ToUpper() const { return ASString(ToUpperNode()); }
    ASString    ToLower() const { return ASString(ToLowerNode()); }

    // *** Operators

    // Assignment.
    template <typename T>
    void        operator = (const T&);
    void        operator = (const char* str);
    void        operator = (const ASString& src)
    {
        AssignNode(src.pNode);
    }

    // Concatenation of string / UTF8.
    void        operator += (const char* str) { Append(str); }
    void        operator += (const ASString& str) { Append(str); }
    ASString    operator + (const char* str) const;
    ASString    operator + (const ASString& str) const;

    // Comparison.
    bool        operator == (const ASString& str) const
    {
        return pNode == str.pNode;
    }    
    bool        operator != (const ASString& str) const
    {
        return pNode != str.pNode;
    }
    bool        operator == (const char* str) const
    {
        return SFstrcmp(pNode->pData, str) == 0;
    }
    bool        operator != (const char* str) const
    {
        return !operator == (str);
    }

    // Compares provide the same ordering for UTF8 due to
    // the nature of data.
    bool        operator<(const char* pstr) const
    {
        return SFstrcmp(ToCStr(), pstr) < 0;
    }
    bool        operator>(const char* pstr) const
    {
        return SFstrcmp(ToCStr(), pstr) > 0;
    }

    // Operator '<' needs to consider possibility of character '\0' in a string,
    // so it can't use strcmp. Mush match ECMA-262 section 11.8.5.16 - 20.
    bool        operator<(const ASString& str) const;
    bool        operator>(const ASString& str) const
    {
        if (pNode == str.pNode)
            return false;
        return !(operator < (str));
    }

    // These functions compare strings with using current locale. They might be used
    // for sorting arrays of strings. By default, strings are sorted by Unicode order,
    // however this is not always acceptable. For example, in French, sorting of array
    // ['a','A','Z','z','e','ê','é'] should give the result ['a','A','e','é','ê','z','Z'], 
    // if locale is used rather than just Unicode order.
    int LocaleCompare_CaseCheck(const ASString& str, bool caseSensitive) const
    {
        return ASConstString::LocaleCompare_CaseCheck(str.ToCStr(), str.GetLength(), caseSensitive);
    }
    static int LocaleCompare_CaseCheck(const ASString& s1, const ASString& s2, bool caseSensitive)
    {
        return s1.LocaleCompare_CaseCheck(s2, caseSensitive);
    }

    int LocaleCompare(const ASString& str) const
    {
        return ASConstString::LocaleCompare(str.ToCStr(), str.GetLength());
    }
    static int LocaleCompare(const ASString& s1, const ASString& s2)
    {
        return s1.LocaleCompare(s2);
    }
    int LocaleCompare_CaseInsensitive(const ASString& str) const
    {
        return ASConstString::LocaleCompare_CaseInsensitive(str.ToCStr(), str.GetLength());
    }
    static int LocaleCompare_CaseInsensitive(const ASString& s1, const ASString& s2)
    {
        return s1.LocaleCompare_CaseInsensitive(s2);
    }


    SF_INLINE void    AssignNode(ASStringNode *pnode)
    {
        SF_ASSERT(pnode);
        SF_ASSERT(pnode->pData);
        pnode->AddRef();
        pNode->Release();
        pNode = pnode;
    }


    bool    Compare_CaseCheck(const ASString &str, bool caseSensitive) const
    {
        return ASConstString::Compare_CaseCheck(str, caseSensitive);
    }

    // Compares constants, assumes that 
    bool    CompareBuiltIn_CaseCheck(const ASString &str, bool caseSensitive) const
    {
        return ASConstString::CompareBuiltIn_CaseCheck(str, caseSensitive);
    }


    // Compares constants case-insensitively
    bool    CompareBuiltIn_CaseInsensitive(const ASString &str) const
    {
        return ASConstString::CompareBuiltIn_CaseInsensitive(str);
    }

    // Compares constants case-insensitively
    bool    Compare_CaseInsensitive_Resolved(const ASString &str) const
    {
        return ASConstString::Compare_CaseInsensitive_Resolved(str);
    }

    // Compares constants case-insensitively.
    // Assumes that pLower MUST be resolved in both strings.
    bool    CompareBuiltIn_CaseInsensitive_Unchecked(const ASString &str) const
    {
        return ASConstString::CompareBuiltIn_CaseInsensitive_Unchecked(str);
    }


    // ***** Case Insensitive wrapper support

    // Case insensitive keys are used to look up insensitive string in hash tables
    // for SWF files with version before SWF 7.
    struct NoCaseKey
    {
        const ASString *pStr;        
        NoCaseKey(const ASString &str) : pStr(&str)
        {
            str.ResolveLowercase();
        }
    };

    bool    operator == (const NoCaseKey& strKey) const
    {
        return strKey.pStr->Compare_CaseInsensitive_Resolved(*this);
    }    
    bool    operator != (const NoCaseKey& strKey) const
    {
        return !strKey.pStr->Compare_CaseInsensitive_Resolved(*this);
    }

    // Raw compare is used to look up a string is extremely rare cases when no 
    // string context is available for GASString creation.    
    struct RawCompareKey
    {
        const char *pStr;
        UPInt       Hash;

        RawCompareKey(const char *pstr, UPInt length)
        {
            pStr = pstr;
            Hash = ASString::HashFunction(pstr, length);            
        }
    };

    bool    operator == (const RawCompareKey& strKey) const
    {
        return operator == (strKey.pStr);
    }    
    bool    operator != (const RawCompareKey& strKey) const
    {
        return operator != (strKey.pStr);
    }

};

inline
ASConstString::ASConstString(const ASString& other)
: ASStringNodeHolder(other.GetNode())
{
    SF_ASSERT(pNode);
}

// ***** String Manager implementation

struct ASStringKey
{
    const char* pStr;
    UPInt       HashValue;
    UPInt       Length;

    ASStringKey(const ASStringKey &src)
        : pStr(src.pStr), HashValue(src.HashValue), Length(src.Length)
    { }
    ASStringKey(const char* pstr, UPInt hashValue, UPInt length)
        : pStr(pstr), HashValue(hashValue), Length(length)
    { }
};


template<class C>
class ASStringNodeHashFunc
{
public:
    typedef C ValueType;

    // Hash code is stored right in the node
    UPInt  operator() (const C& data) const
    {
        return data->HashFlags;
    }

    UPInt  operator() (const ASStringKey &str) const
    {
        return str.HashValue;
    }

    // Hash update - unused.
    static UPInt    GetCachedHash(const C& data)                { return data->HashFlags; }
    static void     SetCachedHash(C& data, UPInt hashValue)     { SF_UNUSED2(data, hashValue); }
    // Value access.
    static C&       GetValue(C& data)                           { return data; }
    static const C& GetValue(const C& data)                     { return data; }

};

// String node hash set - keeps track of all strings currently existing in the manager.
typedef HashSetUncachedLH<ASStringNode*,
                           ASStringNodeHashFunc<ASStringNode*>,
                           ASStringNodeHashFunc<ASStringNode*>,
                           StatMV_ASString_Mem> ASStringNodeHash;




class ASStringManager : public RefCountBase<ASStringManager, StatMV_ASString_Mem>
{    
    friend class ASString;  
    friend struct ASStringNode;

    ASStringNodeHash StringSet;
    MemoryHeap*      pHeap;


    // Allocation Page structures, used to avoid many small allocations.
    struct StringNodePage
    {
        enum { StringNodeCount = 127 };     
        // Node array starts here.
        ASStringNode       Nodes[StringNodeCount];
        // Next allocated page; save so that it can be released.
        StringNodePage  *   pNext;
    };

    // Strings text is also allocated in pages, to make small string management efficient.
    struct TextPage
    {
        // The size of buffer is usually 12 or 16, depending on platform.
        enum  {
            BuffSize   = (sizeof(void*) <= 4) ? (sizeof(void*) * 3) : (sizeof(void*) * 2),
            // Use -2 because we do custom alignment and we want to fit in allocator block.
            BuffCount  = (2048 / BuffSize) - 2
        };

        struct Entry
        {
            union
            {   // Entry for free node list.
                Entry*  pNextAlloc;
                char    Buff[BuffSize];
            };
        };

        Entry       Entries[BuffCount];
        TextPage*   pNext;
        void*       pMem;
    };


    // Free string nodes that can be used.
    ASStringNode*   pFreeStringNodes;
    // List of allocated string node pages, so that they can be released.
    // Note that these are allocated for the duration of GASStringManager lifetime.
    StringNodePage* pStringNodePages;

    // Free string buffers that can be used, together with their owner nodes.
    TextPage::Entry* pFreeTextBuffers;
    TextPage*       pTextBufferPages;

    // Pointer to the available string node.
    ASStringNode    EmptyStringNode;

    ASStringNode    NullStringNode;

    // Log object used for reporting AS leaks.
    Ptr<LogState>   pLog;
    StringLH        FileName;


    void            AllocateStringNodes();
    void            AllocateTextBuffers();

    ASStringNode*  AllocStringNode()
    {
        if (!pFreeStringNodes)
            AllocateStringNodes();
        // Grab next node from list.
        ASStringNode* pnode = pFreeStringNodes;
        if (pFreeStringNodes)
            pFreeStringNodes = pnode->pNextAlloc;
        return pnode;
    }

    void            FreeStringNode(ASStringNode* pnode)
    {
        if (pnode->pData)
        {
            if (!(pnode->HashFlags & ASString::Flag_ConstData))
                FreeTextBuffer(const_cast<char*>(pnode->pData), pnode->Size);
            pnode->pData = 0;
        }
        // Insert into free list.
        pnode->pNextAlloc = pFreeStringNodes;
        pFreeStringNodes  = pnode;
    }

    // Get buffer for text (adds 0 to length).
    char*           AllocTextBuffer(UPInt length);
    // Allocates text buffer and copies length characters into it. Appends 0.
    char*           AllocTextBuffer(const char* pbuffer, UPInt length);
    void            FreeTextBuffer(char* pbuffer, UPInt length);

public:

    ASStringManager(MemoryHeap* pheap);
    ~ASStringManager();

    MemoryHeap* GetHeap() const { return pHeap; }

    // Sets the log that will be used to report leaks during destructor.    
    void        SetLeakReportLog(LogState *plog, const char *pfilename);

    ASString   CreateEmptyString()
    {
        return ASString(GetEmptyStringNode());
    }

    // Create a string from a buffer.
    ASString   CreateString(const char *pstr)
    {
        return ASString(CreateStringNode(pstr));
    } 
    ASString   CreateString(const char *pstr, UPInt length)
    {
        return ASString(CreateStringNode(pstr, length));
    }     
    ASString   CreateString(const String& str)
    {
        return ASString(CreateStringNode(str.ToCStr(), str.GetSize()));
    }
    ASString   CreateConstString(const char *pstr, UPInt length, UInt32 stringFlags = 0)
    {
        return ASString(CreateConstStringNode(pstr, length, stringFlags));
    }
    ASString   CreateConstString(const char *pstr)
    {
        return ASString(CreateConstStringNode(pstr, SFstrlen(pstr), 0));
    }
    // Wide character; use special type of counting.
    ASString  CreateString(const wchar_t* pwstr, SPInt len = -1)
    {
        return ASString(CreateStringNode(pwstr, len));
    }  

    // Various functions for creating string nodes.
    // Returns a node copy/reference to text in question.
    // All lengths specified must be exact and not include trailing characters.

    ASStringNode*  CreateConstStringNode(const char* pstr, UPInt length, UInt32 stringFlags);
    // Allocates node owning buffer.
    ASStringNode*  CreateStringNode(const char* pstr);
    ASStringNode*  CreateStringNode(const char* pstr, UPInt length);
    // These functions also perform string concatenation.
    ASStringNode*  CreateStringNode(const char* pstr1, UPInt l1,
                                    const char* pstr2, UPInt l2);
    ASStringNode*  CreateStringNode(const char* pstr1, UPInt l1,
                                    const char* pstr2, UPInt l2,
                                    const char* pstr3, UPInt l3);
    // Wide character; use special type of counting.
    ASStringNode*  CreateStringNode(const wchar_t* pwstr, SPInt len = -1);  

    ASStringNode* GetEmptyStringNode() { return &EmptyStringNode;  }

    ASStringNode* GetNullStringNode() { return &NullStringNode;  }

    // Shared-code Helpers to be used by StringManager
    void          InitBuiltinArray(ASStringNodeHolder* nodes, const char** strings,
                                   unsigned count);
    void          ReleaseBuiltinArray(ASStringNodeHolder* nodes, unsigned count);
};

inline
bool ASStringNode::IsNull() const
{
    return this == pManager->GetNullStringNode();
}

inline
bool ASConstString::IsNull() const
{
    return pNode == GetManager()->GetNullStringNode();
}

inline
void ASString::SetNull()
{
    AssignNode(GetManager()->GetNullStringNode());
}

template <>
inline
void ASString::operator = <String>(const String& str)
{
    AssignNode(GetManager()->CreateStringNode(str.ToCStr(), str.GetSize()));
}

template <>
inline
void ASString::operator = <StringDH>(const StringDH& str)
{
    AssignNode(GetManager()->CreateStringNode(str.ToCStr(), str.GetSize()));
}

template <>
inline
void ASString::operator = <Ptr<ASStringNode> >(const Ptr<ASStringNode>& str)
{
    if (str)
        AssignNode(str.GetPtr());
    else
        SetNull();
}

// ***** StringManager Template

// StringManager maintains a set of pre-registered string efficiently
// indexable by Builtin_Type, which is typically an enumeration. AS2 and AS3
// built-in string managers are separated since their required string sets are different.
// Different BuiltinStringManagers can, however, share the same ASStringManager.

template<class Builtin_Type, unsigned Builtin_Count>
class ASStringBuiltinManagerT
{    
    ASStringNodeHolder  Builtins[Builtin_Count];
    ASStringManager*    pStringManager;
    const char**        pStaticStrings;
public:

    ASStringBuiltinManagerT(ASStringManager* stringManager, const char** textStrings)
        : pStringManager(stringManager), pStaticStrings(textStrings)
    {
        pStringManager->InitBuiltinArray(Builtins, textStrings, Builtin_Count);
    }
    ~ASStringBuiltinManagerT()
    {
        pStringManager->ReleaseBuiltinArray(Builtins, Builtin_Count);
    }

    MemoryHeap*    GetHeap() const { return pStringManager->GetHeap(); }

    // String access and use.   
    const ASStringNodeHolder& GetBuiltinNodeHolder(Builtin_Type btype) const { return Builtins[btype]; }
    const ASString& GetBuiltin(Builtin_Type btype) const            { return (const ASString&)GetBuiltinNodeHolder(btype); }
    ASString        CreateConstString(const char *pstr) const       { return pStringManager->CreateConstString(pstr); }
    ASString        CreateConstString(const char *pstr, UPInt length) const { return pStringManager->CreateConstString(pstr, length); }
    ASString        CreateString(const char *pstr) const            { return pStringManager->CreateString(pstr); }
    ASString        CreateString(const wchar_t *pwstr, SPInt len = -1) const { return pStringManager->CreateString(pwstr, len); }
    ASString        CreateString(const char *pstr, UPInt length) const  { return pStringManager->CreateString(pstr, length); }
    ASString        CreateString(const String& str) const           { return pStringManager->CreateString(str); }
    ASStringNode*   GetEmptyStringNode() const                      { return pStringManager->GetEmptyStringNode();  }
    ASString        CreateEmptyString() const                       { return ASString(pStringManager->GetEmptyStringNode()); }
    ASStringNode*   GetNullStringNode() const                       { return pStringManager->GetNullStringNode();  }
    ASString        CreateNullString() const                        { return ASString(pStringManager->GetNullStringNode()); }

    ASStringManager* GetStringManager() const { return pStringManager; }

    inline const char* GetBuiltinCStr(Builtin_Type btype)
    {        
        SF_ASSERT(btype < Builtin_Count);
        return pStaticStrings[btype];
    }
};


// ***** Hash table used for member Strings

struct ASStringHashFunctor
{
    inline UPInt  operator() (const ASString& str) const
    {
        // No need to mask out hashFlags since flags are in high bits that
        // will be masked out automatically by the hash table.
        return str.GetHashFlags();
    }

    // Hash lookup for custom keys.
    inline UPInt  operator() (const ASString::NoCaseKey& ikey) const
    {        
        return ikey.pStr->GetHashFlags();
    }
    inline UPInt  operator() (const ASString::RawCompareKey& ikey) const
    {
        return ikey.Hash;
    }
};

template<class C, class HashF>
class HashsetNodeEntry_GC
{
public:
    // Internal chaining for collisions.
    SPInt NextInChain;
    C     Value;

    HashsetNodeEntry_GC()
        : NextInChain(-2) { }
    HashsetNodeEntry_GC(const HashsetNodeEntry_GC& e)
        : NextInChain(e.NextInChain), Value(e.Value) { }
    HashsetNodeEntry_GC(const C& key, SPInt next)
        : NextInChain(next), Value(key) { }    
    HashsetNodeEntry_GC(const typename C::NodeRef& keyRef, SPInt next)
        : NextInChain(next), Value(keyRef) { }

    bool    IsEmpty() const             { return NextInChain == -2;  }
    bool    IsEndOfChain() const        { return NextInChain == -1;  }
    UPInt   GetCachedHash(UPInt maskValue) const  { return HashF()(Value) & maskValue; }
    void    SetCachedHash(UPInt hashValue)        { SF_UNUSED(hashValue); }

    void    Clear()
    {        
        Value.~C(); // placement delete
        NextInChain = -2;
    }
    void    Free()
    {   
        typedef typename C::FirstType FirstDtor;
        //typedef typename C::SecondType SecondDtor;
        Value.First.~FirstDtor();
        Value.Second.Finalize_GC(); // placement delete
        //Value.Second.~SecondDtor(); // placement delete

        //Value.~C();
        NextInChain = -2;
    }
};

template<class C, class U, class HashF = FixedSizeHash<C>, int StatId = Stat_Default_Mem>
class HashUncachedLH_GC
    : public Hash<C, U, HashF, AllocatorLH<C, StatId>, HashNode<C,U,HashF>,
    HashsetNodeEntry_GC<HashNode<C,U,HashF>, typename HashNode<C,U,HashF>::NodeHashF> >
{
public:
    typedef HashUncachedLH_GC<C, U, HashF, StatId> SelfType;
    typedef Hash<C, U, HashF, AllocatorLH<C, StatId>, HashNode<C,U,HashF>,
        HashsetNodeEntry_GC<HashNode<C,U,HashF>, typename HashNode<C,U,HashF>::NodeHashF> >  BaseType;
    //GHashsetEntry<C, HashF> > BaseType;

    // Delegated constructors.
    HashUncachedLH_GC()                                        { }
    HashUncachedLH_GC(int sizeHint) : BaseType(sizeHint)       { }
    HashUncachedLH_GC(const SelfType& src) : BaseType(src)     { }
    ~HashUncachedLH_GC()                                       
    { 
        //FreeTable();
    }
    void operator = (const SelfType& src)                    { BaseType::operator = (src); }
};

// Case-insensitive string hash.
template<class U, class BaseType>
class ASStringHashBase : public BaseType
{
    typedef ASStringHashBase<U, BaseType>  SelfType;

public:
    // Delegated constructors.
    ASStringHashBase()                                     { }
    ASStringHashBase(int sizeHint) : BaseType(sizeHint)    { }
    ASStringHashBase(const SelfType& src) : BaseType(src)  { }
    ~ASStringHashBase()                                    { }

    void    operator = (const SelfType& src)               { BaseType::operator = (src); }


    // *** Case-Insensitive / Case-Selectable 'get' lookups.

    bool    GetCaseInsensitive(const ASString& key, U* pvalue) const
    {
        ASString::NoCaseKey ikey(key);
        return BaseType::GetAlt(ikey, pvalue);
    }
    // Pointer-returning get variety.
    const U* GetCaseInsensitive(const ASString& key) const   
    {
        ASString::NoCaseKey ikey(key);
        return BaseType::GetAlt(ikey);
    }
    U*  GetCaseInsensitive(const ASString& key)
    {
        ASString::NoCaseKey ikey(key);
        return BaseType::GetAlt(ikey);
    }

    // Case-checking based on argument.
    bool    GetCaseCheck(const ASString& key, U* pvalue, bool caseSensitive) const
    {
        return caseSensitive ? BaseType::Get(key, pvalue) : GetCaseInsensitive(key, pvalue);
    }
    const U* GetCaseCheck(const ASString& key, bool caseSensitive) const   
    {
        return caseSensitive ? BaseType::Get(key) : GetCaseInsensitive(key);
    }
    U*      GetCaseCheck(const ASString& key, bool caseSensitive)
    {
        return caseSensitive ? BaseType::Get(key) : GetCaseInsensitive(key);
    }


    // *** Case-Insensitive / Case-Selectable find.

    typedef typename BaseType::ConstIterator  ConstIterator;
    typedef typename BaseType::Iterator       Iterator;

    Iterator    FindCaseInsensitive(const ASString& key)
    {
        ASString::NoCaseKey ikey(key);
        return BaseType::FindAlt(ikey);
    }
    Iterator    FindCaseCheck(const ASString& key, bool caseSensitive)
    {
        return caseSensitive ? BaseType::Find(key) : FindCaseInsensitive(key);
    }


    // *** Case-Selectable set.

    // Set just uses a find and assigns value if found.
    // The key is not modified - this behavior is identical to Flash string variable assignment.    
    void    SetCaseCheck(const ASString& key, const U& value, bool caseSensitive)
    {
        Iterator it = FindCaseCheck(key, caseSensitive);
        if (it != BaseType::End())
        {
            it->Second = value;
        }
        else
        {
            BaseType::Add(key, value);
        }
    } 


    // *** Case-Insensitive / Case-Selectable remove.

    void     RemoveCaseInsensitive(const ASString& key)
    {   
        ASString::NoCaseKey ikey(key);
        BaseType::RemoveAlt(ikey);
    }

    void     RemoveCaseCheck(const ASString& key, bool caseSensitive)
    {   
        if (caseSensitive)
            BaseType::Remove(key);
        else
            RemoveCaseInsensitive(key);
    }
};

template <class U>
class ASStringHash : public ASStringHashBase<U, HashUncachedLH<ASString, U, ASStringHashFunctor, StatMV_ASString_Mem> >
{
    typedef ASStringHash<U>                                                                  SelfType;
    typedef ASStringHashBase<U, HashUncachedLH<ASString, U, ASStringHashFunctor, StatMV_ASString_Mem> >   BaseType;

};

#ifdef GFX_AS_ENABLE_GC
// special version for GRefCountCollection
template <class U>
class ASStringHash_GC : public ASStringHashBase<U, HashUncachedLH_GC<ASString, U, ASStringHashFunctor, StatMV_ASString_Mem> >
{
    typedef ASStringHash_GC<U>                                                               SelfType;
    typedef ASStringHashBase<U, HashUncachedLH_GC<ASString, U, ASStringHashFunctor, StatMV_ASString_Mem> >   BaseType;

};
#else
#define ASStringHash_GC ASStringHash
#endif // !SF_NO_GC

}} // Scaleform::GFx

#endif
