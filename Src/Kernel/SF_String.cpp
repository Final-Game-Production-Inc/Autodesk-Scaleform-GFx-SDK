/**************************************************************************

Filename    :   SF_String.cpp
Content     :   String UTF8 string implementation with copy-on
                write semantics (thread-safe for assignment but not
                modification).
Created     :   April 27, 2007
Authors     :   Ankur Mohan, Michael Antonov, Sergey, Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "SF_String.h"
#include "SF_Debug.h"

#include <stdlib.h>
#include <ctype.h>

#ifdef SF_OS_QNX
# include <strings.h>
#endif

namespace Scaleform {

#define String_LengthIsSize (UPInt(1) << String::Flag_LengthIsSizeShift)

String::DataDesc String::NullData = {String_LengthIsSize, 1, {0} };


String::String()
{
    pData = &NullData;
    pData->AddRef();
};

String::String(const char* pdata)
{
    // Obtain length in bytes; it doesn't matter if _data is UTF8.
    UPInt size = pdata ? SFstrlen(pdata) : 0; 
    pData = AllocDataCopy1(Memory::GetGlobalHeap(), size, 0, pdata, size);
};

String::String(const char* pdata1, const char* pdata2, const char* pdata3)
{
    // Obtain length in bytes; it doesn't matter if _data is UTF8.
    UPInt size1 = pdata1 ? SFstrlen(pdata1) : 0; 
    UPInt size2 = pdata2 ? SFstrlen(pdata2) : 0; 
    UPInt size3 = pdata3 ? SFstrlen(pdata3) : 0; 

    DataDesc *pdataDesc = AllocDataCopy2(Memory::GetGlobalHeap(),
                                         size1 + size2 + size3, 0,
                                         pdata1, size1, pdata2, size2);
    memcpy(pdataDesc->Data + size1 + size2, pdata3, size3);   
    pData = pdataDesc;    
}

String::String(const char* pdata, UPInt size)
{
    SF_ASSERT((size == 0) || (pdata != 0));
    pData = AllocDataCopy1(Memory::GetGlobalHeap(), size, 0, pdata, size);
};


String::String(const InitStruct& src, UPInt size)
{
    pData = AllocData(Memory::GetGlobalHeap(), size, 0);
    src.InitString(GetData()->Data, size);
}

String::String(const String& src)
{
    DataDesc* pdata = src.GetData();   

    // We can only AddRef if string belongs to the same (global) heap.
    // This constructor should not be called for derived (non-global heap) classes.
    if (src.GetHeap() == Memory::GetGlobalHeap())
    {
        pData = pdata;
        pData->AddRef();
    }
    else
    {
        UPInt size = pdata->GetSize();
        pData = AllocDataCopy1(Memory::GetGlobalHeap(),size,pdata->GetLengthFlag(),
                               pdata->Data, size);
    }
}

String::String(const StringDataPtr src)
{
    pData = AllocDataCopy1(Memory::GetGlobalHeap(), src.GetSize(), 0, src.ToCStr(), src.GetSize());
}

String::String(const StringBuffer& src)
{
    pData = AllocDataCopy1(Memory::GetGlobalHeap(), src.GetSize(), 0, src.ToCStr(), src.GetSize());
}

String::String(const wchar_t* data)
{
    pData = &NullData;
    pData->AddRef();
    // Simplified logic for wchar_t constructor.
    if (data)    
        *this = data;    
}


String::DataDesc* String::AllocData(MemoryHeap* pheap, UPInt size, UPInt lengthIsSize)
{
    String::DataDesc* pdesc;

    if (size == 0)
    {
        pdesc = &NullData;
        pdesc->AddRef();
        return pdesc;
    }

    pdesc = (DataDesc*)SF_HEAP_ALLOC(pheap, sizeof(DataDesc)+ size, Stat_String_Mem);  
    pdesc->Data[size] = 0;
    pdesc->RefCount = 1;
    pdesc->Size     = size | lengthIsSize;  
    return pdesc;
}


String::DataDesc* String::AllocDataCopy1(MemoryHeap* pheap, UPInt size, UPInt lengthIsSize,
                                         const char* pdata, UPInt copySize)
{
    String::DataDesc* pdesc = AllocData(pheap, size, lengthIsSize);
    memcpy(pdesc->Data, pdata, copySize);
    return pdesc;
}

String::DataDesc* String::AllocDataCopy2(MemoryHeap* pheap, UPInt size, UPInt lengthIsSize,
                                         const char* pdata1, UPInt copySize1,
                                         const char* pdata2, UPInt copySize2)
{
    String::DataDesc* pdesc = AllocData(pheap, size, lengthIsSize);
    memcpy(pdesc->Data, pdata1, copySize1);
    memcpy(pdesc->Data + copySize1, pdata2, copySize2);
    return pdesc;
}



MemoryHeap*    String::GetHeap() const
{
    MemoryHeap* pheap = 0;

    switch(GetHeapType())
    {
    case HT_Global:
        pheap = Memory::GetGlobalHeap();
        break;
    case HT_Local:
        pheap = Memory::GetHeapByAddress(this);
        break;
    case HT_Dynamic:
        pheap = ((const StringDH*)this)->GetHeap();
        break;
    default:;
    }

    return pheap;
}



UPInt String::GetLength() const 
{
    // Optimize length accesses for non-UTF8 character strings. 
    DataDesc* pdata = GetData();
    UPInt     length, size = pdata->GetSize();
    
    if (pdata->LengthIsSize())
        return size;    
    
    length = (UPInt)UTF8Util::GetLength(pdata->Data, (UPInt)size);
    
    if (length == size)
        pdata->Size |= String_LengthIsSize;
    
    return length;
}


//static UInt32 String_CharSearch(const char* buf, )


UInt32 String::GetCharAt(UPInt index) const 
{  
    SPInt       i = (SPInt) index;
    DataDesc*   pdata = GetData();
    const char* buf = pdata->Data;
    UInt32      c;
    
    if (pdata->LengthIsSize())
    {
        SF_ASSERT(index < pdata->GetSize());
        buf += i;
        return UTF8Util::DecodeNextChar_Advance0(&buf);
    }

    c = UTF8Util::GetCharAt(index, buf, pdata->GetSize());
    return c;
}

UInt32 String::GetFirstCharAt(UPInt index, const char** offset) const
{
    DataDesc*   pdata = GetData();
    SPInt       i = (SPInt) index;
    const char* buf = pdata->Data;
    const char* end = buf + pdata->GetSize();
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

UInt32 String::GetNextChar(const char** offset) const
{
    return UTF8Util::DecodeNextChar(offset);
}



void String::AppendChar(UInt32 ch)
{
    DataDesc*   pdata = GetData();
    UPInt       size = pdata->GetSize();
    char        buff[8];
    SPInt       encodeSize = 0;

    // Converts ch into UTF8 string and fills it into buff.   
    UTF8Util::EncodeCharSafe(buff, 8, &encodeSize, ch);
    SF_ASSERT(encodeSize >= 0);

    SetData(AllocDataCopy2(GetHeap(), size + (UPInt)encodeSize, 0,
                           pdata->Data, size, buff, (UPInt)encodeSize));
    pdata->Release();
}


void String::AppendString(const wchar_t* pstr, SPInt len)
{
    if (!pstr)
        return;

    DataDesc*   pdata = GetData();
    const UPInt       oldSize = pdata->GetSize();    
    const UPInt       encodeSize = (UPInt)UTF8Util::GetEncodeStringSize(pstr, len);

    DataDesc*   pnewData = AllocDataCopy1(GetHeap(), oldSize + (UPInt)encodeSize, 0,
                                          pdata->Data, oldSize);
    // DataDesc includes data for NULL termination. Pass full size to UTF8Util
    UTF8Util::EncodeStringSafe(pnewData->Data + oldSize,  oldSize + (UPInt)encodeSize + 1, pstr, len);

    SetData(pnewData);
    pdata->Release();
}


void String::AppendString(const char* putf8str, SPInt utf8StrSz)
{
    if (!putf8str || !utf8StrSz)
        return;
    if (utf8StrSz == -1)
        utf8StrSz = (SPInt)SFstrlen(putf8str);

    DataDesc*   pdata = GetData();
    UPInt       oldSize = pdata->GetSize();

    SetData(AllocDataCopy2(GetHeap(), oldSize + (UPInt)utf8StrSz, 0,
                           pdata->Data, oldSize, putf8str, (UPInt)utf8StrSz));
    pdata->Release();
}

void    String::AssignString(const InitStruct& src, UPInt size)
{
    DataDesc*   poldData = GetData();
    DataDesc*   pnewData = AllocData(GetHeap(), size, 0);
    src.InitString(pnewData->Data, size);
    SetData(pnewData);
    poldData->Release();
}

void    String::AssignString(const char* putf8str, UPInt size)
{
    DataDesc* poldData = GetData();
    SetData(AllocDataCopy1(GetHeap(), size, 0, putf8str, size));
    poldData->Release();
}

void    String::operator = (const char* pstr)
{
    AssignString(pstr, pstr ? SFstrlen(pstr) : 0);
}

void    String::operator = (const wchar_t* pwstr)
{
    DataDesc*   poldData = GetData();
    const UPInt size = pwstr ? (UPInt)UTF8Util::GetEncodeStringSize(pwstr) : 0;

    DataDesc*   pnewData = AllocData(GetHeap(), size, 0);
    // DataDesc includes data for NULL termination. Pass full size to UTF8Util
    UTF8Util::EncodeStringSafe(pnewData->Data, size + 1, pwstr);
    SetData(pnewData);
    poldData->Release();
}


void    String::operator = (const String& src)
{ 
    MemoryHeap* pheap = GetHeap();
    DataDesc*    psdata = src.GetData();
    DataDesc*    pdata = GetData();    

    if (pheap == src.GetHeap())
    {        
        SetData(psdata);
        psdata->AddRef();
    }
    else
    {
        SetData(AllocDataCopy1(pheap, psdata->GetSize(), psdata->GetLengthFlag(),
                               psdata->Data, psdata->GetSize()));
    }

    pdata->Release();
}


void    String::operator = (const StringBuffer& src)
{ 
    DataDesc* polddata = GetData();    
    SetData(AllocDataCopy1(GetHeap(), src.GetSize(), 0, src.ToCStr(), src.GetSize()));
    polddata->Release();
}

void    String::operator += (const String& src)
{
    DataDesc   *pourData = GetData(),
               *psrcData = src.GetData();
    UPInt       ourSize  = pourData->GetSize(),
                srcSize  = psrcData->GetSize();
    UPInt       lflag    = pourData->GetLengthFlag() & psrcData->GetLengthFlag();

    SetData(AllocDataCopy2(GetHeap(), ourSize + srcSize, lflag,
                            pourData->Data, ourSize, psrcData->Data, srcSize));
    pourData->Release();
}


String   String::operator + (const char* str) const
{   
    String tmp1(*this);
    tmp1 += (str ? str : "");
    return tmp1;
}

String   String::operator + (const String& src) const
{ 
    String tmp1(*this);
    tmp1 += src;
    return tmp1;
}

void    String::Remove(UPInt posAt, SPInt removeLength)
{
    DataDesc*   pdata = GetData();
    UPInt       oldSize = pdata->GetSize();    
    // Length indicates the number of characters to remove. 
    UPInt       length = GetLength();

    // If index is past the string, nothing to remove.
    if (posAt >= length)
        return;
    // Otherwise, cap removeLength to the length of the string.
    if ((posAt + removeLength) > length)
        removeLength = length - posAt;

    // Get the byte position of the UTF8 char at position posAt.
    SPInt bytePos    = UTF8Util::GetByteIndex(posAt, pdata->Data, oldSize);
    SPInt removeSize = UTF8Util::GetByteIndex(removeLength, pdata->Data + bytePos, oldSize-bytePos);

    SetData(AllocDataCopy2(GetHeap(), oldSize - removeSize, pdata->GetLengthFlag(),
                           pdata->Data, bytePos,
                           pData->Data + bytePos + removeSize, (oldSize - bytePos - removeSize)));
    pdata->Release();
}


String   String::Substring(UPInt start, UPInt end) const
{
    UPInt length = GetLength();
    if ((start >= length) || (start >= end))
        return String();   

    DataDesc* pdata = GetData();
    
    // If size matches, we know the exact index range.
    if (pdata->LengthIsSize())
        return String(pdata->Data + start, end - start);
    
    // Get position of starting character.
    SPInt byteStart = UTF8Util::GetByteIndex(start, pdata->Data, pdata->GetSize());
    SPInt byteSize  = UTF8Util::GetByteIndex(end - start, pdata->Data + byteStart, pdata->GetSize()-byteStart);
    return String(pdata->Data + byteStart, (UPInt)byteSize);
}

void String::Clear()
{   
    NullData.AddRef();
    GetData()->Release();
    SetData(&NullData);
}


String   String::ToUpper() const 
{       
    UInt32      c;
    const char* psource = GetData()->Data;
    const char* pend = psource + GetData()->GetSize();
    String      str;
    SPInt       bufferOffset = 0;
    char        buffer[512];
    
    while(psource < pend)
    {
        do {            
            c = UTF8Util::DecodeNextChar_Advance0(&psource);
            UTF8Util::EncodeCharSafe(buffer, 512, &bufferOffset, SFtowupper(wchar_t(c)));
        } while ((psource < pend) && (bufferOffset < SPInt(sizeof(buffer)-8)));

        // Append string a piece at a time.
        str.AppendString(buffer, bufferOffset);
        bufferOffset = 0;
    }

    return str;
}

String   String::ToLower() const 
{
    UInt32      c;
    const char* psource = GetData()->Data;
    const char* pend = psource + GetData()->GetSize();
    String      str;
    SPInt       bufferOffset = 0;
    char        buffer[512];

    while(psource < pend)
    {
        do {
            c = UTF8Util::DecodeNextChar_Advance0(&psource);
            UTF8Util::EncodeCharSafe(buffer, 512, &bufferOffset, SFtowlower(wchar_t(c)));
        } while ((psource < pend) && (bufferOffset < SPInt(sizeof(buffer)-8)));

        // Append string a piece at a time.
        str.AppendString(buffer, bufferOffset);
        bufferOffset = 0;
    }

    return str;
}



String& String::Insert(const char* substr, UPInt posAt, SPInt strSize)
{
    DataDesc* poldData   = GetData();
    UPInt     oldSize    = poldData->GetSize();
    UPInt     insertSize = (strSize < 0) ? SFstrlen(substr) : (UPInt)strSize;    
    UPInt     byteIndex  =  (poldData->LengthIsSize()) ?
                            posAt : (UPInt)UTF8Util::GetByteIndex(posAt, poldData->Data, oldSize);

    SF_ASSERT(byteIndex <= oldSize);
    
    DataDesc* pnewData = AllocDataCopy2(GetHeap(), oldSize + insertSize, 0,
                                        poldData->Data, byteIndex, substr, insertSize);
    memcpy(pnewData->Data + byteIndex + insertSize,
           poldData->Data + byteIndex, oldSize - byteIndex);
    SetData(pnewData);
    poldData->Release();
    return *this;
}

/*
String& String::Insert(const UInt32* substr, UPInt posAt, SPInt len)
{
    for (SPInt i = 0; i < len; ++i)
    {
        UPInt charw = InsertCharAt(substr[i], posAt);
        posAt += charw;
    }
    return *this;
}
*/

UPInt String::InsertCharAt(UInt32 c, UPInt posAt)
{
    char    buf[8];
    SPInt   index = 0;
    UTF8Util::EncodeCharSafe(buf, 8, &index, c);
    SF_ASSERT(index >= 0 && index < 8);
    buf[(UPInt)index] = 0;

    Insert(buf, posAt, index);
    return (UPInt)index;
}


int String::CompareNoCase(const char* a, const char* b)
{
    return SFstricmp(a, b);
}

int String::CompareNoCase(const char* a, const char* b, SPInt len)
{
    if (len)
    {
        SPInt f,l;
        SPInt slen = len;
        const char *s = b;
        do {
            f = (SPInt)SFtolower((int)(*(a++)));
            l = (SPInt)SFtolower((int)(*(b++)));
        } while (--len && f && (f == l) && *b != 0);

        if (f == l && (len != 0 || *b != 0))
        {
            f = (SPInt)slen;
            l = (SPInt)SFstrlen(s);
            return int(f - l);
        }

        return int(f - l);
    }
    else
        return (0-(int)SFstrlen(b));
}

// ***** Implement hash static functions

// Hash function
UPInt String::BernsteinHashFunction(const void* pdataIn, UPInt size, UPInt seed)
{
    const UByte*    pdata   = (const UByte*) pdataIn;
    UPInt           h       = seed;
    while (size > 0)
    {
        size--;
        h = ((h << 5) + h) ^ (unsigned) pdata[size];
    }

    return h;
}

// Hash function, case-insensitive
UPInt String::BernsteinHashFunctionCIS(const void* pdataIn, UPInt size, UPInt seed)
{
    const UByte*    pdata = (const UByte*) pdataIn;
    UPInt           h = seed;
    while (size > 0)
    {
        size--;
        h = ((h << 5) + h) ^ SFtolower(pdata[size]);
    }

    // Alternative: "sdbm" hash function, suggested at same web page above.
    // h = 0;
    // for bytes { h = (h << 16) + (h << 6) - hash + *p; }
    return h;
}


void String::EscapeSpecialHTML(const char* psrc, UPInt length, String* pescapedStr)
{
    UInt32       ch;
    StringBuffer buf;
    const char*  end = psrc + length;

    while(ch = UTF8Util::DecodeNextChar_Advance0(&psrc), (psrc < end))
    {
        if (ch == '<')
        {
            buf.AppendString("&lt;", 4);
        }
        else if (ch == '>')
        {
            buf.AppendString("&gt;", 4);
        }
        else if (ch == '\"')
        {
            buf.AppendString("&quot;", 6);
        }
        else if (ch == '\'')
        {
            buf.AppendString("&apos;", 6);
        }
        else if (ch == '&')
        {
            buf.AppendString("&amp;", 5);
        }
        else
        {
            buf.AppendChar(ch);
        }
    }
    *pescapedStr = buf;
}

void String::UnescapeSpecialHTML(const char* psrc, UPInt length, String* punescapedStr)
{
    UInt32       ch;
    StringBuffer buf;
    const char*  end = psrc + length;

    while (ch = UTF8Util::DecodeNextChar_Advance0(&psrc), (psrc < end))
    {
        if (ch == '&')
        {
            if (strncmp(psrc, "quot;", 5) == 0)
            {
                buf.AppendChar('\"');
                psrc += 5;
                break;
            }
            else if (strncmp(psrc, "apos;", 5) == 0)
            {
                buf.AppendChar('\'');
                psrc += 5;
                break;
            }
            else if (strncmp(psrc, "amp;", 4) == 0)
            {
                buf.AppendChar('&');
                psrc += 4;
                break;
            }
            else if (strncmp(psrc, "lt;", 3) == 0)
            {
                buf.AppendChar('<');
                psrc += 3;
                break;
            }
            else if (strncmp(psrc, "gt;", 3) == 0)
            {
                buf.AppendChar('>');
                psrc += 3;
                break;
            }
            else
                buf.AppendChar(ch);
        }
        else
            buf.AppendChar(ch);
    }
    *punescapedStr = buf;
}



// ***** Local String - StringLH

StringLH::StringLH() : String(String::NoConstructor())
{
    NullData.AddRef();
    SetDataLcl(&NullData);
};

StringLH::StringLH(const char* pdata) : String(String::NoConstructor())
{
    // Obtain length in bytes; it doesn't matter if _data is UTF8.
    UPInt size = pdata ? SFstrlen(pdata) : 0; 
    SetDataLcl(AllocDataCopy1(Memory::GetHeapByAddress(this), size, 0, pdata, size));
};

StringLH::StringLH(const char* pdata, UPInt size) : String(String::NoConstructor())
{
    SF_ASSERT((size == 0) || (pdata != 0));
    SetDataLcl(AllocDataCopy1(Memory::GetHeapByAddress(this), size, 0, pdata, size));
};

StringLH::StringLH(const InitStruct& src, UPInt size) : String(String::NoConstructor())
{
    SetDataLcl(AllocData(Memory::GetHeapByAddress(this), size, 0));
    src.InitString(GetData()->Data, size);
}

// Called for both Copy constructor and String initializer.
void StringLH::CopyConstructHelper(const String& src)
{
    DataDesc*       pdata = src.GetData();   
    MemoryHeap*     pheap = Memory::GetHeapByAddress(this);

    // We can only AddRef if string belongs to the same local heap.    
    if (src.GetHeap() == pheap)
    {   
        pdata->AddRef();
        SetDataLcl(pdata);
    }
    else
    {
        UPInt size = pdata->GetSize();
        SetDataLcl(AllocDataCopy1(pheap,size,pdata->GetLengthFlag(),
                                  pdata->Data, size));
    }
}

StringLH::StringLH(const wchar_t* pdata)
{
    // Allow BASE constructor for data.
    // NullData.AddRef();
    // SetDataLcl(&NullData);    
    if (pdata)
        *this = pdata;
}


// ***** Dynamic heap string - StringDH

StringDH::StringDH(MemoryHeap* pheap)
    : String(String::NoConstructor())
{
    pHeap = pheap;
    NullData.AddRef();
    SetDataLcl(&NullData);    
};

StringDH::StringDH(MemoryHeap* pheap, const char* pdata)
    : String(String::NoConstructor())
{
    // Obtain length in bytes; it doesn't matter if _data is UTF8.
    UPInt size = pdata ? SFstrlen(pdata) : 0; 
    pHeap = pheap;
    SetDataLcl(AllocDataCopy1(pheap, size, 0, pdata, size));
};

StringDH::StringDH(MemoryHeap* pheap,
                   const char* pdata1, const char* pdata2, const char* pdata3)
{
     // Obtain length in bytes; it doesn't matter if _data is UTF8.
    UPInt size1 = pdata1 ? SFstrlen(pdata1) : 0; 
    UPInt size2 = pdata2 ? SFstrlen(pdata2) : 0; 
    UPInt size3 = pdata3 ? SFstrlen(pdata3) : 0; 

    DataDesc *pdataDesc = AllocDataCopy2(pheap,
                                         size1 + size2 + size3, 0,
                                         pdata1, size1, pdata2, size2);
    memcpy(pdataDesc->Data + size1 + size2, pdata3, size3);
    pHeap = pheap;
    SetDataLcl(pdataDesc);    
}

StringDH::StringDH(MemoryHeap* pheap, const char* pdata, UPInt size)
     : String(String::NoConstructor())
{
    SF_ASSERT((size == 0) || (pdata != 0));
    pHeap = pheap;
    SetDataLcl(AllocDataCopy1(pheap, size, 0, pdata, size));
};

StringDH::StringDH(MemoryHeap* pheap, const InitStruct& src, UPInt size)
     : String(String::NoConstructor())
{
    pHeap = pheap;
    SetDataLcl(AllocData(pheap, size, 0));
    src.InitString(GetData()->Data, size);
}

// Called for both Copy constructor and String initializer.
void StringDH::CopyConstructHelper(const String& src, MemoryHeap* pheap)
{
    DataDesc*     pdata = src.GetData();
    MemoryHeap*   psrcHeap = src.GetHeap();

    // We allow null here to indicate heap copy.
    if (!pheap)
        pheap = psrcHeap;
    pHeap = pheap;

    // We can only AddRef if string belongs to the same local heap.    
    if (psrcHeap == pheap)
    {   
        pdata->AddRef();
        SetDataLcl(pdata);
    }
    else
    {
        UPInt size = pdata->GetSize();
        SetDataLcl(AllocDataCopy1(pheap,size,pdata->GetLengthFlag(),
                                  pdata->Data, size));
    }
}

StringDH::StringDH(MemoryHeap* pheap, const wchar_t* pdata)
{
    // Allow BASE constructor for data.
    // NullData.AddRef();
    // SetDataLcl(&NullData);
    pHeap = pheap;
    if (pdata)
        *this = pdata;
}



// ***** String Buffer used for Building Strings


#define DEFAULD_GROW_SIZE 512
// Constructors / Destructor.
StringBuffer::StringBuffer(MemoryHeap* pheap)
    : pData(NULL), Size(0), BufferSize(0), GrowSize(DEFAULD_GROW_SIZE), LengthIsSize(false), pHeap(pheap)
{
}

StringBuffer::StringBuffer(UPInt growSize, MemoryHeap* pheap)
    : pData(NULL), Size(0), BufferSize(0), GrowSize(DEFAULD_GROW_SIZE), LengthIsSize(false), pHeap(pheap)
{
    SetGrowSize(growSize);
}

StringBuffer::StringBuffer(const char* data, MemoryHeap* pheap)
    : pData(NULL), Size(0), BufferSize(0), GrowSize(DEFAULD_GROW_SIZE), LengthIsSize(false), pHeap(pheap)
{
    *this = data;
}

StringBuffer::StringBuffer(const char* data, UPInt dataSize, MemoryHeap* pheap)
    : pData(NULL), Size(0), BufferSize(0), GrowSize(DEFAULD_GROW_SIZE), LengthIsSize(false), pHeap(pheap)
{
    AppendString(data, dataSize);
}

StringBuffer::StringBuffer(const String& src, MemoryHeap* pheap)
    : pData(NULL), Size(0), BufferSize(0), GrowSize(DEFAULD_GROW_SIZE), LengthIsSize(false), pHeap(pheap)
{
    AppendString(src.ToCStr(), src.GetSize());
}

StringBuffer::StringBuffer(const StringBuffer& src, MemoryHeap* pheap)
    : pData(NULL), Size(0), BufferSize(0), GrowSize(DEFAULD_GROW_SIZE), LengthIsSize(false), pHeap(pheap)
{
    AppendString(src.ToCStr(), src.GetSize());
    LengthIsSize = src.LengthIsSize;
}

StringBuffer::StringBuffer(const wchar_t* data, MemoryHeap* pheap)
    : pData(NULL), Size(0), BufferSize(0), GrowSize(DEFAULD_GROW_SIZE), LengthIsSize(false), pHeap(pheap)
{
    *this = data;
}

StringBuffer::~StringBuffer()
{
    if (pData)
        SF_HEAP_FREE(pHeap, pData);
}
void StringBuffer::SetGrowSize(UPInt growSize) 
{ 
    if (growSize <= 16)
        GrowSize = 16;
    else
    {
        UInt8 bits = Alg::UpperBit(UInt32(growSize-1));
        UPInt size = 1ULL<<bits;
        GrowSize = size == growSize ? growSize : size;
    }
}

UPInt StringBuffer::GetLength() const
{
    UPInt length, size = GetSize();
    if (LengthIsSize)
        return size;

    length = (UPInt)UTF8Util::GetLength(pData, (UPInt)GetSize());

    if (length == GetSize())
        LengthIsSize = true;
    return length;
}

void    StringBuffer::Reserve(UPInt _size)
{
    if (_size >= BufferSize) // >= because of trailing zero! (!AB)
    {
        BufferSize = (_size + 1 + GrowSize - 1)& ~(GrowSize-1);
        if (!pData)
            pData = (char*)SF_HEAP_ALLOC(pHeap, BufferSize,  Stat_String_Mem);
        else 
            pData = (char*)SF_REALLOC(pData, BufferSize, Stat_String_Mem);
    }
}
void    StringBuffer::Resize(UPInt _size)
{
    Reserve(_size);
    LengthIsSize = false;
    Size = _size;
    if (pData)
        pData[Size] = 0;
}

void StringBuffer::Clear()
{
    Resize(0);
    /*
    if (pData != pEmptyNullData)
    {
        SF_HEAP_FREE(pHeap, pData);
        pData = pEmptyNullData;
        Size = BufferSize = 0;
        LengthIsSize = false;
    }
    */
}
// Appends a character
void     StringBuffer::AppendChar(UInt32 ch)
{
    char    buff[8];
    UPInt   origSize = GetSize();

    // Converts ch into UTF8 string and fills it into buff. Also increments index according to the number of bytes
    // in the UTF8 string.
    SPInt   srcSize = 0;
    UTF8Util::EncodeCharSafe(buff, 8, &srcSize, ch);
    SF_ASSERT(srcSize >= 0);
    
    UPInt size = origSize + srcSize;
    Resize(size);
    memcpy(pData + origSize, buff, srcSize);
}

// Append a string
void     StringBuffer::AppendString(const wchar_t* pstr, SPInt len)
{
    if (!pstr)
        return;

    SPInt   srcSize     = UTF8Util::GetEncodeStringSize(pstr, len);
    UPInt   origSize    = GetSize();
    UPInt   size        = srcSize + origSize;

    Resize(size);
    UTF8Util::EncodeStringSafe(pData + origSize, srcSize + 1, pstr, len);
}

void      StringBuffer::AppendString(const char* putf8str, SPInt utf8StrSz)
{
    if (!putf8str || !utf8StrSz)
        return;
    if (utf8StrSz == -1)
        utf8StrSz = (SPInt)SFstrlen(putf8str);

    UPInt   origSize    = GetSize();
    UPInt   size        = utf8StrSz + origSize;

    Resize(size);
    memcpy(pData + origSize, putf8str, utf8StrSz);
}


void      StringBuffer::operator = (const char* pstr)
{
    pstr = pstr ? pstr : "";
    UPInt size = SFstrlen(pstr);
    Resize(size);
    memcpy(pData, pstr, size);
}

void      StringBuffer::operator = (const wchar_t* pstr)
{
    pstr = pstr ? pstr : L"";
    UPInt size = (UPInt)UTF8Util::GetEncodeStringSize(pstr);
    Resize(size);
    UTF8Util::EncodeStringSafe(pData, size + 1, pstr);
}

void      StringBuffer::operator = (const String& src)
{
    Resize(src.GetSize());
    memcpy(pData, src.ToCStr(), src.GetSize());
}

void      StringBuffer::operator = (const StringBuffer& src)
{
    pHeap = src.pHeap;
    GrowSize = src.GrowSize;
    Resize(src.GetSize());
    memcpy(pData, src.ToCStr(), src.GetSize());
    LengthIsSize = src.LengthIsSize;
}


// Inserts substr at posAt
void      StringBuffer::Insert(const char* substr, UPInt posAt, SPInt len)
{
    UPInt     oldSize    = Size;
    UPInt     insertSize = (len < 0) ? SFstrlen(substr) : (UPInt)len;    
    UPInt     byteIndex  = LengthIsSize ? posAt : 
                           (UPInt)UTF8Util::GetByteIndex(posAt, pData, (SPInt)Size);

    SF_ASSERT(byteIndex <= oldSize);
    Reserve(oldSize + insertSize);

    memmove(pData + byteIndex + insertSize, pData + byteIndex, oldSize - byteIndex + 1);
    memcpy (pData + byteIndex, substr, insertSize);
    LengthIsSize = false;
    Size = oldSize + insertSize;
    pData[Size] = 0;
}

// Inserts character at posAt
UPInt     StringBuffer::InsertCharAt(UInt32 c, UPInt posAt)
{
    char    buf[8];
    SPInt   len = 0;
    UTF8Util::EncodeCharSafe(buf, 8, &len, c);
    SF_ASSERT(len >= 0);
    buf[(UPInt)len] = 0;

    Insert(buf, posAt, len);
    return (UPInt)len;
}

StringDataPtr StringDataPtr::Null("", 0);

//////////////////////////////////////////////////////////////////////////////
bool StringDataPtr::IsWhiteSpace(UInt32 c)
{
    return (c == ' ' ||  c == '\n' || c == '\r' || c =='\t' || c == '\f' || c == '\v' ||
        (c >= 0x2000 && c <=0x200b) || c == 0x2028 || c == 0x2029 || c == 0x205f || c == 0x3000 );
}

StringDataPtr StringDataPtr::GetTruncateWhitespace() const
{
    const char* s = ToCStr();
    const char* b = s;
    const char* str = s;
    const char* e = s + GetSize();
    bool found_nws = false;

    // Skip front whitespace.
    while (b < e)
    {
        str = b;
        UInt32 c = UTF8Util::DecodeNextChar_Advance0(&b);

        if (!IsWhiteSpace(c))
        {
            found_nws = true;
            break;
        }
    }

    if (b == e && !found_nws)
        return StringDataPtr();

    // Find last not whitespace character.
    const char* l = b;
    const char* enw = l;
    while (l < e)
    {
        UInt32 c = UTF8Util::DecodeNextChar_Advance0(&l);

        if (!IsWhiteSpace(c))
            enw = l;
    }

    if (str == ToCStr() && enw == e)
        // String doesn't have any whitespace.
        return *this;

    return StringDataPtr(str, enw - str);
}

SPInt StringDataPtr::FindLastChar(char c, UPInt init_ind) const
{
    if (init_ind == (UPInt)~0 || init_ind > GetSize())
        init_ind = GetSize();
    else
        ++init_ind;

    for (UPInt i = init_ind; i > 0; --i)
        if (pStr[i - 1] == c)
            return static_cast<SPInt>(i - 1);

    return -1;
}

SPInt StringDataPtr::FindSubstring(const StringDataPtr& s, UPInt init_ind) const
{
    if (s.IsEmpty())
        return -1;

    const UPInt last_si = s.GetSize() - 1;
    UPInt si = 0;
    char c = s[si];
    for (UPInt i = init_ind; i < GetSize(); ++i)
    {
        if (pStr[i] == c)
        {
            if (si == last_si)
                return static_cast<SPInt>(i - s.GetSize());
            else
                c = s[++si];
        }
        else if (si > 0)
        {
            si = 0;
            c = s[si];
        }
    }

    return -1;
}

StringDataPtr StringDataPtr::GetNextToken(char separator) const
{
    UPInt cur_pos = 0;
    const char* cur_str = ToCStr();

    for (; cur_pos < GetSize() && cur_str[cur_pos]; ++cur_pos)
    {
        if (cur_str[cur_pos] == separator)
        {
            break;
        }
    }

    return StringDataPtr(ToCStr(), cur_pos);
}

} // Scaleform
