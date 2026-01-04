/**************************************************************************

Filename    :   XML_Object.h
Content     :   XML objects support
Created     :   March 17, 2008
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_XMLObject_H
#define INC_SF_GFX_XMLObject_H

#include <GFxConfig.h>

#ifdef GFX_ENABLE_XML

#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_Array.h"
#include "GFx/GFx_Player.h"

namespace Scaleform { namespace GFx { namespace XML {
//
// This file declares a DOM string manager built for XML file processing
// and DOM tree creation. The string manager manages unique strings
// by hashing them. All strings are expected to be in UTF-8.
//

// Forward declarations
class   DOMString;
class   DOMStringManager;
struct  DOMStringNode;

struct Node;
struct Prefix;
struct Attribute;
struct ElementNode;
struct TextNode;
struct Document;
struct ShadowRef;
struct RootNode;

struct Object;
class ObjectManager;


//
// A wrapper class to compare a char array with an XML DOM string
//
struct DOMStringCompareWrapper
{
    // Char array
    const char*     pData;

    // Char array byte size
    UPInt           Length;

    DOMStringCompareWrapper(const char* pdata, UPInt len)
        : pData(pdata), Length(len) {}
};

// 
// String node - stored in the manager table.
//
struct DOMStringNode
{        
    enum FlagConstants
    {
        Flag_HashMask       = 0x00FFFFFF,
    };

    const char*         pData;
    union
    {
        DOMStringManager*   pManager;
        DOMStringNode*      pNextAlloc;
    };
    UInt32              HashFlags;
    unsigned            Size;

    SInt32          RefCount;

    // *** Utility functions

    DOMStringManager*  GetManager() const
    {
        return pManager;
    }
    inline UInt32  GetHashCode()
    {
        return HashFlags & Flag_HashMask;
    }

    void    AddRef()
    {
        RefCount++;
    }
    void    Release()
    {
        if (--RefCount == 0)
        {  
            // Refcount is 0
            // Objects should clean themselves
            ReleaseNode();
        }
    }

    // Releases the node to the manager.
    void    ReleaseNode();    
};


// 
// XMLString - XML DOM string implementation.

// XMLString is represented as a pointer to a unique node so that
// comparisons can be implemented very fast. Nodes are allocated by
// and stored within XMLStringManager.
// XMLString objects can be created only by string manager; default
// constructor is not implemented.

struct DOMStringNodeHolder
{
    DOMStringNode*  pNode;
};

// Do not derive from GNewOverrideBase and do not new!
class DOMString : public DOMStringNodeHolder
{
    friend class DOMStringManager;

public:

    // *** Create/Destroy: can

    explicit DOMString(DOMStringNode *pnode);

    DOMString(const DOMString& src);

    ~DOMString();

    // *** General Functions

    void        Clear();

    // Pointer to raw buffer.
    const char* ToCStr() const          { return pNode->pData; }
    const char* GetBuffer() const       { return pNode->pData; }

    // Size of string characters without 0. Raw count, not UTF8.
    unsigned    GetSize() const         { return pNode->Size; }
    bool        IsEmpty() const         { return pNode->Size == 0; }

    unsigned    GetHashFlags() const    { return pNode->HashFlags; }
    unsigned    GetHash() const         { return GetHashFlags() & DOMStringNode::Flag_HashMask; }

    DOMStringNode* GetNode() const      { return pNode; }

    static UInt32   HashFunction(const char *pchar, UPInt length);

    void    operator = (const DOMString& src)
    {
        AssignNode(src.pNode);
    }

    // Comparison.
    bool    operator == (const DOMString& str) const
    {
        return pNode == str.pNode;
    }    
    bool    operator != (const DOMString& str) const
    {
        return pNode != str.pNode;
    }
    bool    operator == (const DOMStringCompareWrapper& str)
    {
        return (strncmp(pNode->pData, str.pData, str.Length) == 0);
    }

    void    AssignNode(DOMStringNode *pnode);
};

//
// Hash key 
//
struct DOMStringKey
{
    const char* pStr;
    UPInt       HashValue;
    UPInt       Length;

    DOMStringKey(const DOMStringKey &src)
        : pStr(src.pStr), HashValue(src.HashValue), Length(src.Length)
    { }
    DOMStringKey(const char* pstr, UPInt hashValue, UPInt length)
        : pStr(pstr), HashValue(hashValue), Length(length)
    { }
};


//
// Hash functions
//
template<class C>
class DOMStringNodeHashFunc
{
public:
    typedef C ValueType;

    // Hash code is stored right in the node
    UPInt  operator() (const C& data) const
    {
        return data->HashFlags;
    }

    UPInt  operator() (const DOMStringKey &str) const
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

//
// String node hash set 
//
// Keeps track of all strings currently existing in the manager.
//
typedef HashSetUncachedLH<DOMStringNode*, 
DOMStringNodeHashFunc<DOMStringNode*>, DOMStringNodeHashFunc<DOMStringNode*>, 
StatMV_XML_Mem>  XMLDOMStringNodeHash;



// 
// XML string manager
//
class DOMStringManager
{    
    friend class DOMString;  
    friend struct DOMStringNode;

    XMLDOMStringNodeHash StringSet;

    //
    // Allocation Page structures, used to avoid many small allocations.
    //
    struct StringNodePage
    {
        enum { StringNodeCount = 127 };     
        // Node array starts here.
        DOMStringNode       Nodes[StringNodeCount];
        // Next allocated page; save so that it can be released.
        StringNodePage  *   pNext;
    };

    //
    // Strings text is also allocated in pages, to make small string 
    // management efficient.
    //
    struct TextPage
    {
        // The size of buffer is usually 12 or 16, depending on platform.
        enum  {
            BuffSize   = (sizeof(void*) <= 4) ? (sizeof(void*) * 3) 
            : (sizeof(void*) * 2),
            // Use -2 because we do custom alignment and we want to fit in 
            // allocator block.
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


    //
    // Free string nodes that can be used.
    //
    DOMStringNode*      pFreeStringNodes;

    //
    // List of allocated string node pages, so that they can be released.
    // Note that these are allocated for the duration of XMLStringManager lifetime.
    //
    StringNodePage*     pStringNodePages;

    //
    // Free string buffers that can be used, together with their owner nodes.
    //
    TextPage::Entry*    pFreeTextBuffers;
    TextPage*           pTextBufferPages;

    //
    // Heap used for string allocation
    //
    MemoryHeap*        pHeap;

    //
    // Pointer to the available string node.
    //
    DOMStringNode       EmptyStringNode;

    void            AllocateStringNodes();
    void            AllocateTextBuffers();

    DOMStringNode*  AllocStringNode()
    {
        if (!pFreeStringNodes)
            AllocateStringNodes();
        // Grab next node from list.
        DOMStringNode* pnode = pFreeStringNodes;
        if (pFreeStringNodes)
            pFreeStringNodes = pnode->pNextAlloc;      
        // Assign manager
        pnode->pManager = this;       
        return pnode;
    }

    void            FreeStringNode(DOMStringNode* pnode)
    {
        if (pnode->pData)
        {
            FreeTextBuffer(const_cast<char*>(pnode->pData), pnode->Size);
            pnode->pData = 0;
        }
        // Insert into free list.
        pnode->pNextAlloc = pFreeStringNodes;
        pFreeStringNodes  = pnode;
    }

    //
    // Get buffer for text (adds 0 to length).
    //
    char*           AllocTextBuffer(UPInt length);

    //
    // Allocates text buffer and copies length characters into it. Appends 0.
    //
    char*           AllocTextBuffer(const char* pbuffer, UPInt length);
    void            FreeTextBuffer(char* pbuffer, UPInt length);


    // Various functions for creating string nodes.
    // Returns a node copy/reference to text in question.
    // All lengths specified must be exact and not include trailing characters.

    //
    // Allocates node owning buffer.
    //
    DOMStringNode*    CreateStringNode(const char* pstr, UPInt length);

    //
    // These functions also perform string concatenation.
    //
    DOMStringNode*    CreateStringNode(const char* pstr1, UPInt l1,
        const char* pstr2, UPInt l2);

    DOMStringNode*    GetEmptyStringNode() { return &EmptyStringNode;  }

public:

    DOMStringManager();
    ~DOMStringManager();

    //
    // Return the instance of empty string
    //
    DOMString   CreateEmptyString()
    {
        return DOMString(GetEmptyStringNode());
    }

    //
    // Create a string from a buffer.
    //
    DOMString   CreateString(const char *pstr, UPInt length)
    {
        return DOMString(CreateStringNode(pstr, length));
    }     
    void   AppendString(DOMString& str1, const char* str2, UPInt length)
    {
        str1.AssignNode(CreateStringNode(str1.pNode->pData, str1.pNode->Size,
            str2, length));
    }
};


// --------------------------------------------------------------------


// 
// The heterogeneous object manager
// 
// Wraps all object allocations into a single interface
//
class ObjectManager : public RefCountBaseNTS<ObjectManager, StatMV_XML_Mem>, public ExternalLibPtr
{
    //
    // Hashed string pool that maintains only one copy 
    // of a string
    //
    DOMStringManager         StringPool; 

    //
    // The heap used for object allocation. This heap is
    // the same as the heap used to allocate the object manager
    // unless a different one is explicitly specified.
    //
    MemoryHeap*                    pHeap;

public:
    ObjectManager(MemoryHeap* pheap, MovieImpl* powner = NULL);
    ObjectManager(MovieImpl* powner = NULL);
    virtual ~ObjectManager();

    MemoryHeap*        GetHeap()   { return pHeap; }

    //
    // Object creation methods
    //

    DOMString CreateString(const char* str, UPInt len)
    {        
        return StringPool.CreateString(str, len);
    }
    template<size_t N>
    DOMString CreateString(const char (&str)[N])
    {        
        return StringPool.CreateString(str, N-1);
    }
    DOMString EmptyString()
    {
        return StringPool.CreateEmptyString();
    }
    void   AppendString(DOMString& str1, const char* str2, UPInt len)
    {
        StringPool.AppendString(str1, str2, len);
    }

    Document*     CreateDocument();
    ElementNode*  CreateElementNode(DOMString value);
    TextNode*     CreateTextNode(DOMString value);
    Attribute*    CreateAttribute(DOMString name, DOMString value);
    Prefix*       CreatePrefix(DOMString name, DOMString value);
    RootNode*     CreateRootNode(Node* pdom);
};

}}} // namespace SF::GFx::XML

#endif  // #ifdef GFX_ENABLE_XML

#endif // INC_SF_GFX_XMLObject_H
