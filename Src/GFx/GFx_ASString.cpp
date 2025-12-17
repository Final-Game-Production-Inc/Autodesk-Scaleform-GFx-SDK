/**************************************************************************

Filename    :   GFx_ASString.cpp
Content     :   String manager implementation for Action Script
Created     :   November 7, 2006
Authors     :   Michael Antonov, Sergey Sikorskiy

Notes       :   Implements optimized GASString class, which acts as a
                hash key for strings allocated from GASStringManager.

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/GFx_ASString.h"
#include "GFx/GFx_ASUtils.h"

// Log for leak reports
#include "GFx/GFx_Log.h"

namespace Scaleform { namespace GFx {

// ***** GASString implementation

// Static hash function calculation routine
UInt32  ASConstString::HashFunction(const char *pchar, UPInt length)
{
    return (UInt32)String::BernsteinHashFunctionCIS(pchar, length) & Flag_HashMask;
}


void    ASString::Clear()
{
    AssignNode(GetManager()->GetEmptyStringNode());  
}

bool    ASString::operator<(const ASString& str) const
{
    if (pNode == str.pNode)
        return false;
    unsigned sz = Alg::Min(GetSize(), str.GetSize());
    int      result = memcmp(ToCStr(), str.ToCStr(), sz);
    if (result != 0)
        return result < 0;
    return GetSize() < str.GetSize();
}

// Returns length in characters.
unsigned    ASConstString::GetLength() const
{
    unsigned length, size = GetSize();    
   
    // Optimize length accesses for non-UTF8 character strings.
    if (GetHashFlags() & Flag_LengthIsSize)
        return size;    
    length = (unsigned)UTF8Util::GetLength(GetBuffer(), GetSize());
    if (length == size)
        pNode->HashFlags |= Flag_LengthIsSize;
    return length;
}

UInt32  ASConstString::GetCharAt(unsigned index) const
{
    if (GetHashFlags() & Flag_LengthIsSize)
    {
        return (UInt32) operator [](index);
    }

    int   i = (int) index;
    const char* buf = GetBuffer();
    UInt32  c;

    do 
    {
        // We must advance in case of zero character.
        c = UTF8Util::DecodeNextChar_Advance0(&buf);
        i--;

#if 0
        // Actually, we may have zero character inside of a string.
        if (c == 0)
        {
            // We've hit the end of the string; don't go further.
            SF_ASSERT(i == 0);
            return c;
        }
#endif
    } while (i >= 0);    

    return c;
}

UInt32 ASConstString::GetFirstCharAt(UPInt index, const char** offset) const
{
    SPInt       i = (SPInt) index;
    const char* buf = ToCStr();
    const char* end = buf + GetSize();
    UInt32      c;

    do 
    {
        c = UTF8Util::DecodeNextChar_Advance0(&buf);
        i--;

        if (buf >= end)
        {
            // We've hit the end of the string; don't go further.
            SF_ASSERT(i == 0);
            return c;
        }
    } while (i >= 0);

    *offset = buf;

    return c;
}

UInt32 ASConstString::GetNextChar(const char** offset) const
{
    // Optimize char accesses for non-UTF8 character strings.
    if (GetHashFlags() & Flag_LengthIsSize)
    {
        UInt32 result = **offset;
        ++(*offset);
        return result;
    }

    return UTF8Util::DecodeNextChar(offset);
}

// The rest of the functions here operate in UTF8. For example,    
ASStringNode* ASConstString::AppendCharNode(UInt32 ch) const
{
    char    buff[8];
    SPInt   index = 0;
    UTF8Util::EncodeCharSafe(buff, 8, &index, ch);
    // Assign us a concatenated node.
    ASStringNode* node = GetManager()->CreateStringNode(pNode->pData, pNode->Size, buff, index);

    if ((pNode->HashFlags & Flag_LengthIsSize) && ch < 128)
        // Inherit Flag_LengthIsSize.
        node->HashFlags |= Flag_LengthIsSize;

    return node;
}

/*
void    GASString::Remove(int posAt, int len)
{
    SF_UNUSED2(posAt, len);
    SF_ASSERT(0);
} */

// Returns a String that's a substring of this.
//  -start is the index of the first UTF8 character you want to include.    
//  -end is the index one past the last UTF8 character you want to include.
ASStringNode* ASConstString::SubstringNode(int start, int end) const
{
    SF_ASSERT(start <= end);

    // Special case, always return empty string.
    if (start == end)
        return GetManager()->GetEmptyStringNode();

    int         index = 0;
    const char* p = GetBuffer();   
    const char* start_pointer = p;

    const char* end_pointer = p;

    for (;;)
    {
        if (index == start)
        {
            start_pointer = p;
        }

        UInt32  c = UTF8Util::DecodeNextChar(&p);
        index++;

        if (index == end)
        {
            end_pointer = p;
            break;
        }

        if (c == 0)
        {
            //!AB if the string is shorter than "end" param - then just
            // return a whole string as a substring.
            if (index < end)
            {
                end_pointer = p;
            }
            break;
        }
    }

    if (end_pointer < start_pointer)
    {
        end_pointer = start_pointer;
    }

    return GetManager()->CreateStringNode(start_pointer, (int) (end_pointer - start_pointer));
}

ASStringNode* ASConstString::TruncateWhitespaceNode() const
{
    const StringDataPtr s(ToCStr(), GetSize());
    StringDataPtr ts = s.GetTruncateWhitespace();

    if (ts == s)
        return GetNode();

    return GetManager()->CreateStringNode(ts.ToCStr(), ts.GetSize());
}

// Case-converted strings.
ASStringNode* ASConstString::ToUpperNode() const
{
    String str = String(pNode->pData, GetSize()).ToUpper();
    return GetManager()->CreateStringNode(str, str.GetSize());
}

ASStringNode* ASConstString::ToLowerNode() const
{
    ResolveLowercase();
    return pNode->pLower ? pNode->pLower : GetManager()->GetEmptyStringNode();
}


// *** Operations

void ASString::operator = (const char* pstr)
{
    AssignNode(GetManager()->CreateStringNode(pstr, strlen(pstr)));
}

void ASString::Append(const char* str, UPInt len)
{
    AssignNode(GetManager()->CreateStringNode(pNode->pData, pNode->Size,
        str, len));
}

void ASString::Append(const ASString& str)
{
    AssignNode(GetManager()->CreateStringNode(pNode->pData, pNode->Size,
        str.pNode->pData, str.pNode->Size));
}

ASString   ASString::operator + (const char* pstr) const
{
    return ASString(GetManager()->CreateStringNode(pNode->pData, pNode->Size,
        pstr, strlen(pstr)));
}

ASString   ASString::operator + (const ASString& str) const
{
    return ASString(GetManager()->CreateStringNode(pNode->pData, pNode->Size,
        str.pNode->pData, str.pNode->Size));
}

ASStringNode* ASConstString::AppendStringNode(const ASConstString& str) const
{
    ASStringNode* node = GetManager()->CreateStringNode(
        pNode->pData, 
        pNode->Size,
        str.pNode->pData, 
        str.pNode->Size
        );

    if ((pNode->HashFlags & Flag_LengthIsSize) && (str.GetNode()->HashFlags & Flag_LengthIsSize))
        // Inherit Flag_LengthIsSize.
        node->HashFlags |= Flag_LengthIsSize;

    return node;
}

int ASConstString::LocaleCompare_CaseCheck(const char* pstr, UPInt len, bool caseSensitive) const
{
    if (len == SF_MAX_UPINT)
        len = SFstrlen(pstr);

    wchar_t buf1[250], buf2[250];
    wchar_t *pwstra = NULL, *pwstrb = NULL;
    
    UPInt la = GetLength();
    if (la >= sizeof(buf1)/sizeof(buf1[0]))
        pwstra = (wchar_t*)SF_ALLOC(sizeof(wchar_t) * (la + 1), Stat_Default_Mem);
    else
        pwstra = buf1;

    if (len >= sizeof(buf2)/sizeof(buf2[0]))
        pwstrb = (wchar_t*)SF_ALLOC(sizeof(wchar_t) * (len + 1), Stat_Default_Mem);
    else
        pwstrb = buf2;

    UTF8Util::DecodeStringSafe(pwstra, la + 1, ToCStr(), GetSize());
    UTF8Util::DecodeStringSafe(pwstrb, len + 1, pstr, len);

    int res;
    if (caseSensitive)
        res = SFwcscoll(pwstra, pwstrb);
    else
        res = SFwcsicoll(pwstra, pwstrb);
    if (pwstra != buf1)
        SF_FREE(pwstra);
    if (pwstrb != buf2)
        SF_FREE(pwstrb);
    return res;
}

// ***** Node implementation


// Resolves pLower to a valid value.
void    ASStringNode::ResolveLowercase_Impl()
{   
    String        str = String(pData, Size).ToLower();
    ASStringNode* pnode = pManager->CreateStringNode(str, str.GetSize());
    
    if (pnode != pManager->GetEmptyStringNode())
    {
        pLower = pnode;
        if (pnode != this)
            pnode->AddRef();
    }    
}

// Frees the node whose refCount has reached 0.
void   ASStringNode::ReleaseNode()
{
    SF_ASSERT(RefCount == 0);

    // If we had a lowercase version, release its ref count.
    if ((pLower != this) && pLower)
        pLower->Release();
    pManager->StringSet.Remove(this);
    pManager->FreeStringNode(this);
}




// ***** GASStringManager

ASStringManager::ASStringManager(MemoryHeap* pheap)
    : pHeap(pheap)
{
    // Allocate initial data.    
    pStringNodePages = 0;
    pFreeStringNodes = 0;

    pFreeTextBuffers = 0;
    pTextBufferPages = 0;

    // Empty data - refcount 1, so never released.
    EmptyStringNode.RefCount    = 1;
    EmptyStringNode.Size        = 0;
    EmptyStringNode.HashFlags   = ASString::HashFunction("", 0) |
                                  ASString::Flag_ConstData | ASString::Flag_Builtin | ASString::Flag_LengthIsSize;
    EmptyStringNode.pData       = "";
    EmptyStringNode.pManager    = this;
    EmptyStringNode.pLower      = &EmptyStringNode;

    StringSet.Add(&EmptyStringNode);

    NullStringNode              = EmptyStringNode;
    NullStringNode.pLower       = &NullStringNode;
}

ASStringManager::~ASStringManager()
{    
    // Release string nodes.
    StringBuffer    leakReport;
    unsigned        ileaks = 0;

    while(pStringNodePages)
    {
        StringNodePage* ppage = pStringNodePages;
        pStringNodePages = ppage->pNext;

        // Free text in all active GASString objects. Technically this is a bug, since 
        // all GASString should have already died before GASStringManager is destroyed.
        // However, if ActionScript leaks due to circular references, this will at least
        // release all of the string content involved.

        // In the future, once we have GC this may be changed to throw asserts if
        // unreleased data / RefCount is detected at this point.

        for (unsigned i=0; i< ppage->StringNodeCount; i++)
        {
            if (ppage->Nodes[i].pData)
            {   
                if (ileaks < 16)
                {
                    leakReport += (ileaks > 0) ? ", '" : "'";
                    leakReport += ppage->Nodes[i].pData;
                    leakReport += "'";
                }
                ileaks++;

                if (!(ppage->Nodes[i].HashFlags & ASString::Flag_ConstData))
                {
                    FreeTextBuffer(const_cast<char*>(ppage->Nodes[i].pData), ppage->Nodes[i].Size);
                }
            }
        }
        SF_HEAP_FREE(pHeap, ppage);
    }

    // Free text pages.
    while(pTextBufferPages)
    {
        void *pmem = pTextBufferPages->pMem;
        pTextBufferPages = pTextBufferPages->pNext;        
        SF_HEAP_FREE(pHeap, pmem);
    }


    if (ileaks && pLog.GetPtr())
    {
        pLog->LogScriptError("ActionScript Memory leaks in movie '%s', including %d string nodes",
                             FileName.ToCStr(), ileaks);
        pLog->LogScriptError("Leaked string content: %s\n", leakReport.ToCStr());
    }    
}


// Sets the log that will be used to report leaks during destructor.    
void    ASStringManager::SetLeakReportLog(LogState *plog, const char *pfilename)
{
    pLog    = plog;
    FileName= pfilename ? pfilename : "";
}

void    ASStringManager::AllocateStringNodes()
{
    StringNodePage* ppage = (StringNodePage*)
        SF_HEAP_ALLOC(pHeap, sizeof(StringNodePage), StatMV_ASString_Mem);

    if (ppage)
    {
        ppage->pNext = pStringNodePages;        
        pStringNodePages = ppage;
        // Add nodes to free pool.
        for (unsigned i=0; i < StringNodePage::StringNodeCount; i++)
        {
            ASStringNode *pnode = &ppage->Nodes[i];
            // Clear data so that release detection fails on it.
            pnode->pData    = 0;
            // Assign manager so that it doesn't need to be re-assigned.
            pnode->pManager = this;
            
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

void   ASStringManager::AllocateTextBuffers()
{
    // Align data (some of our allocation routines add size counters
    // that may case this to be misaligned at a multiple of 4 instead).
    // Seems to make a difference on P4 chips.
    void*     pp    = SF_HEAP_ALLOC(pHeap, sizeof(TextPage) + 8, StatMV_ASString_Mem);    
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
char*   ASStringManager::AllocTextBuffer(UPInt length)
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
    return (char*)SF_HEAP_ALLOC(pHeap, length + 1, StatMV_ASString_Mem);
}

void    ASStringManager::FreeTextBuffer(char* pbuffer, UPInt length)
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
    
    SF_HEAP_FREE(pHeap, pbuffer);
}

char*   ASStringManager::AllocTextBuffer(const char* pbuffer, UPInt length)
{
    char *pbuff = AllocTextBuffer(length);
    if (pbuff)
    {
        memcpy(pbuff, pbuffer, length);
        pbuff[length] = 0;
    }
    return pbuff;
}



// *** String node management


bool operator == (ASStringNode* pnode, const ASStringKey &str)
{
    if (pnode->Size != str.Length)
        return 0;
    return strncmp(pnode->pData, str.pStr, str.Length) == 0;
}


ASStringNode*  ASStringManager::CreateConstStringNode(const char* pstr, UPInt length, UInt32 stringFlags)
{    
    ASStringNode*  pnode;
    ASStringKey    key(pstr, ASString::HashFunction(pstr, length), length);

    if (StringSet.GetAlt(key, &pnode))
    {
        // Must set flags, otherwise setting could be lost quietly for strings
        // that are already registered, such as built-ins.
        pnode->HashFlags |= stringFlags;
        return pnode;
    }

    if ((pnode = AllocStringNode()) != 0)
    {        
        pnode->RefCount = 0;
        pnode->Size     = (unsigned)length;
        pnode->pData    = pstr;
        pnode->HashFlags= (UInt32)(key.HashValue | ASString::Flag_ConstData | stringFlags);
        pnode->pLower   = 0;
        
        StringSet.Add(pnode);
        return pnode;
    }

    // Return dummy string node.
    return &EmptyStringNode;
}


ASStringNode*  ASStringManager::CreateStringNode(const char* pstr, UPInt length)
{
    if (!pstr || length == 0)
        return &EmptyStringNode;

    ASStringNode*  pnode;
    ASStringKey    key(pstr, ASString::HashFunction(pstr, length), length);

    if (StringSet.GetAlt(key, &pnode))
        return pnode;

    if ((pnode = AllocStringNode()) != 0)
    { 
        pnode->pData = AllocTextBuffer(pstr, length);

        if (pnode->pData)
        {
            pnode->RefCount = 0;
            pnode->Size     = (unsigned)length;
            pnode->HashFlags= (UInt32)key.HashValue;
            pnode->pLower   = 0;

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

ASStringNode*  ASStringManager::CreateStringNode(const char* pstr)
{
    if (!pstr)
        return &EmptyStringNode;
    return CreateStringNode(pstr, strlen(pstr));
}

// Wide character; use special type of counting.
ASStringNode*  ASStringManager::CreateStringNode(const wchar_t* pwstr, SPInt len)
{
//    String strBuff;
//    String::EncodeUTF8FromWChar(&strBuff, pwstr);
    String strBuff;
    strBuff.AppendString(pwstr, len);
    return CreateStringNode(strBuff.ToCStr(), strBuff.GetSize());
}


// These functions also perform string concatenation.
ASStringNode*  ASStringManager::CreateStringNode(const char* pstr1, UPInt l1,
                                                 const char* pstr2, UPInt l2)
{
    ASStringNode*   pnode;
    UPInt           length = l1 + l2;    
    char*           pbuffer = AllocTextBuffer(length);
    
    if (pbuffer)
    {
        if (pstr1 && l1 > 0) memcpy(pbuffer, pstr1, l1);
        if (pstr2 && l2 > 0) memcpy(pbuffer + l1, pstr2, l2);
        pbuffer[length] = 0;

        ASStringKey   key(pbuffer, ASString::HashFunction(pbuffer, length), length);

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
            pnode->pLower   = 0;

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


ASStringNode*  ASStringManager::CreateStringNode(const char* pstr1, UPInt l1,
                                                 const char* pstr2, UPInt l2,
                                                 const char* pstr3, UPInt l3)
{
    ASStringNode*   pnode;
    UPInt           length = l1 + l2 + l3;
    char*           pbuffer = AllocTextBuffer(length);

    if (pbuffer)
    {
        if (pstr1 && l1 > 0) memcpy(pbuffer, pstr1, l1);
        if (pstr2 && l2 > 0) memcpy(pbuffer + l1, pstr2, l2);
        if (pstr3 && l3 > 0) memcpy(pbuffer + l1 + l2, pstr3, l3);
        pbuffer[length] = 0;

        ASStringKey   key(pbuffer, ASString::HashFunction(pbuffer, length), length);
        
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
            pnode->pLower   = 0;

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

// Shared-code Helpers to be used by ASStringBuiltinManager
void  ASStringManager::InitBuiltinArray(ASStringNodeHolder* nodes, const char** strings,
                                        unsigned count)
{
    for (unsigned i = 0; i < count; i++)
    {
        ASString str(CreateConstString(strings[i],
                     strlen(strings[i]),
                     (UInt32)ASString::Flag_Builtin) );
        nodes[i].pNode = str.GetNode();
        nodes[i].pNode->AddRef();
        nodes[i].pNode->ResolveLowercase_Impl();
    }
}

void  ASStringManager::ReleaseBuiltinArray(ASStringNodeHolder* nodes, unsigned count)
{
    for (unsigned i = 0; i < count; i++)
    {  
        nodes[i].pNode->Release();
        // For safety - these should never be accessed after release.
        nodes[i].pNode = 0; 
    }
}




}} // Scaleform::GFx


