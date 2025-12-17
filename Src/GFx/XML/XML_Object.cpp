/**************************************************************************

Filename    :   XML_Object.cpp
Content     :   XML objects support
Created     :   March 17, 2008
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFxConfig.h"

#ifdef GFX_ENABLE_XML

#include "GFx/XML/XML_Object.h"
#include "GFx/XML/XML_Document.h"

#include "GFx/GFx_PlayerImpl.h"

namespace Scaleform { namespace GFx { namespace XML {

#ifdef SF_BUILD_DEBUG
//
// Trace XML object reference counts in debug output
// (Warning: This generates a lot of messages)
//
// #define SF_XML_STRING_REFCOUNTING_TRACE
//
#endif  // SF_BUILD_DEBUG

//
// Frees the node whose refCount has reached 0.
//
void   DOMStringNode::ReleaseNode()
{
#ifdef SF_XML_STRING_REFCOUNTING_TRACE
    SF_DEBUG_MESSAGE1(1, "GFxXMLStringNode::~ReleaseNode : '%.16s'", pData);
#endif

    SF_ASSERT(RefCount == 0);

    pManager->StringSet.Remove(this);
    pManager->FreeStringNode(this);
}


DOMString::DOMString(DOMStringNode *pnode)
{
    pNode = pnode;
    pNode->AddRef();
}

DOMString::DOMString(const DOMString& src)
{
    pNode = src.pNode;
    pNode->AddRef();
}
DOMString::~DOMString()
{        
    pNode->Release();
}

void    DOMString::AssignNode(DOMStringNode *pnode)
{
    pnode->AddRef();
    pNode->Release();
    pNode = pnode;
}

//
// Static hash function calculation routine
//
UInt32  DOMString::HashFunction(const char *pchar, UPInt length)
{
    return (UInt32)String::BernsteinHashFunction(pchar, length) & DOMStringNode::Flag_HashMask;
}


DOMStringManager::DOMStringManager()
{
    pHeap = Memory::GetHeapByAddress(this);

    // Allocate initial data.    
    pStringNodePages = 0;
    pFreeStringNodes = 0;

    pFreeTextBuffers = 0;
    pTextBufferPages = 0;

    // Empty data - refcount 1, so never released.
    EmptyStringNode.RefCount    = 1;
    EmptyStringNode.Size        = 0;
    EmptyStringNode.HashFlags   = DOMString::HashFunction("", 0);
    EmptyStringNode.pData       =  "";
    EmptyStringNode.pManager    = this;

    StringSet.Add(&EmptyStringNode);
}

DOMStringManager::~DOMStringManager()
{    
    // Release string nodes.
    while(pStringNodePages)
    {
        StringNodePage* ppage = pStringNodePages;
        pStringNodePages = ppage->pNext;

        // Free text in all active XMLString objects. Technically this is a bug, since 
        // all XMLString should have already died before XMLStringManager is destroyed.
        // However, if ActionScript leaks due to circular references, this will at least
        // release all of the string content involved.

        // In the future, once we have GC this may be changed to throw asserts if
        // unreleased data / RefCount is detected at this point.

        for (unsigned i=0; i< ppage->StringNodeCount; i++)
        {
            if (ppage->Nodes[i].pData)
                FreeTextBuffer(const_cast<char*>(ppage->Nodes[i].pData), ppage->Nodes[i].Size);
        }
        SF_FREE(ppage);
    }

    // Free text pages.
    while(pTextBufferPages)
    {
        void *pmem = pTextBufferPages->pMem;
        pTextBufferPages = pTextBufferPages->pNext;        
        SF_FREE(pmem);
    }
}


void    DOMStringManager::AllocateStringNodes()
{
    StringNodePage* ppage = (StringNodePage*) SF_HEAP_ALLOC(pHeap, sizeof(StringNodePage), StatMV_XML_Mem);

    if (ppage)
    {
        ppage->pNext = pStringNodePages;        
        pStringNodePages = ppage;
        // Add nodes to free pool.
        for (unsigned i=0; i < StringNodePage::StringNodeCount; i++)
        {
            DOMStringNode *pnode = &ppage->Nodes[i];
            // Clear data so that release detection fails on it.
            pnode->pData    = 0;           
            // Insert into free list.
            pnode->pNextAlloc = pFreeStringNodes;
            pFreeStringNodes  = pnode;
        }
    }
    else
    {
        // Not enough memory for string nodes!!
        SF_ASSERT(1);
    }
}

void   DOMStringManager::AllocateTextBuffers()
{
    // Align data (some of our allocation routines add size counters
    // that may case this to be misaligned at a multiple of 4 instead).
    // Seems to make a difference on P4 chips.
    void*     pp    = SF_HEAP_ALLOC(pHeap, sizeof(TextPage) + 8, StatMV_XML_Mem);    
    TextPage* ppage = (TextPage*) ((((UPInt)pp)+7) & ~7);

    if (ppage)
    {
        ppage->pMem = pp;

        ppage->pNext = pTextBufferPages;
        pTextBufferPages = ppage;
        // Add nodes to free pool.
        for (unsigned i=0; i < TextPage::BuffCount; i++)
        {
            TextPage::Entry *pe = ppage->Entries + i;
            pe->pNextAlloc = pFreeTextBuffers;
            pFreeTextBuffers  = pe;
        }
    }
    else
    {
        // Not enough memory for text entries!!
        SF_ASSERT(1);
    }
}


// Get buffer for text (adds 0 to length).
char*   DOMStringManager::AllocTextBuffer(UPInt length)
{   
    if (length < TextPage::BuffSize)
    {
        if (!pFreeTextBuffers)
            AllocateTextBuffers();

        char *pbuff = 0;        
        if (pFreeTextBuffers)
        {
            pbuff = pFreeTextBuffers->Buff;
            pFreeTextBuffers = pFreeTextBuffers->pNextAlloc;
        }
        return pbuff;
    }
    // If bigger then the size of built-in buffer entry, allocate separately.
    return (char*)SF_HEAP_ALLOC(pHeap, length + 1, StatMV_XML_Mem);
}

char*   DOMStringManager::AllocTextBuffer(const char* pbuffer, UPInt length)
{
    char *pbuff = AllocTextBuffer(length);
    if (pbuff)
    {
        memcpy(pbuff, pbuffer, length);
        pbuff[length] = 0;
    }
    return pbuff;
}

void    DOMStringManager::FreeTextBuffer(char* pbuffer, UPInt length)
{
    // This is never called with null pbuffer.
    SF_ASSERT(pbuffer);

    if (length < TextPage::BuffSize)
    {
        // delta should be 0, but just in case of a strange compiler...
        ptrdiff_t        delta  = &reinterpret_cast<TextPage::Entry*>(0)->Buff[0] - (char*)0;
        TextPage::Entry* pentry = reinterpret_cast<TextPage::Entry*>(pbuffer - delta);

        pentry->pNextAlloc = pFreeTextBuffers;
        pFreeTextBuffers = pentry;
        return;
    }

    SF_FREE(pbuffer);
}


bool operator == (DOMStringNode* pnode, const DOMStringKey &str)
{
    if (pnode->Size != str.Length)
        return 0;
    return strncmp(pnode->pData, str.pStr, str.Length) == 0;
}


DOMStringNode*  DOMStringManager::CreateStringNode(const char* pstr, UPInt length)
{
    DOMStringNode*  pnode = 0;
    DOMStringKey    key(pstr, DOMString::HashFunction(pstr, length), length);

    if (StringSet.GetAlt(key, &pnode))
        return pnode;

    if (length == 0)
        return &EmptyStringNode;

    if ((pnode = AllocStringNode()) != 0)
    { 
        pnode->pData = AllocTextBuffer(pstr, length);

        if (pnode->pData)
        {
            pnode->RefCount = 0;
            pnode->Size     = (unsigned)length;
            pnode->HashFlags= (UInt32)key.HashValue;

            StringSet.Add(pnode);
            return pnode;
        }
        else
        {
            FreeStringNode(pnode);
        }
    }

    return &EmptyStringNode;
}


// These functions also perform string concatenation.
DOMStringNode*  DOMStringManager::CreateStringNode(const char* pstr1, UPInt l1,
                                                               const char* pstr2, UPInt l2)
{
    DOMStringNode*  pnode = 0;
    UPInt           length = l1 + l2;    
    char*           pbuffer = AllocTextBuffer(length);

    if (pbuffer)
    {
        if (l1 > 0) memcpy(pbuffer, pstr1, l1);
        if (l2 > 0) memcpy(pbuffer + l1, pstr2, l2);
        pbuffer[length] = 0;

        DOMStringKey   key(pbuffer, DOMString::HashFunction(pbuffer, length), length);

        // If already exists, no need for node.
        if (StringSet.GetAlt(key, &pnode))
        {
            FreeTextBuffer(pbuffer, length);
            return pnode;
        }

        // Create node.
        if ((pnode = AllocStringNode()) != 0)
        {             
            pnode->RefCount = 0;
            pnode->Size     = (unsigned)length;
            pnode->pData    = pbuffer;
            pnode->HashFlags= (UInt32)key.HashValue;

            StringSet.Add(pnode);
            return pnode;
        }
        else
        {
            FreeTextBuffer(pbuffer, length);
        }
    }

    return &EmptyStringNode;
}


// --------------------------------------------------------------------

//
// Ctor
//
ObjectManager::ObjectManager(MemoryHeap* pheap, MovieImpl* powner)
: ExternalLibPtr(powner), pHeap(pheap)
{
}
ObjectManager::ObjectManager(MovieImpl* powner)
: ExternalLibPtr(powner)
{
    pHeap = Memory::GetHeapByAddress(this);
}


//
// Dtor
//
ObjectManager::~ObjectManager()
{
    // Unregister from owner (if one exists)
    if (pOwner)
        pOwner->pXMLObjectManager = NULL;

}

//
// Thread safe object allocation 
//
Document* ObjectManager::CreateDocument()
{
    return SF_HEAP_NEW(pHeap) Document(this);
}
ElementNode* ObjectManager::CreateElementNode(DOMString value)
{
    return SF_HEAP_NEW(pHeap) ElementNode(this, value);
}
TextNode* ObjectManager::CreateTextNode(DOMString value)
{
    return SF_HEAP_NEW(pHeap) TextNode(this, value);
}
Attribute* ObjectManager::CreateAttribute(DOMString name, DOMString value)
{
    return SF_HEAP_NEW(pHeap) Attribute(name, value);
}
Prefix* ObjectManager::CreatePrefix(DOMString name, DOMString value)
{
    return SF_HEAP_NEW(pHeap) Prefix(name, value);
}
RootNode* ObjectManager::CreateRootNode(Node* pdom)
{
    return SF_HEAP_NEW(pHeap) RootNode(pdom);
}

}}} // namespace SF::GFx::XML

#endif // #ifdef GFX_ENABLE_XML 
