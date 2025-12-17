/**************************************************************************

Filename    :   SF_WString.cpp
Content     :   Wide String buffer implementation
modification).
Created     :   April 27, 2007
Authors     :   Ankur Mohan, Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "SF_WString.h"

namespace Scaleform {

// ***** GFxWStringBuffer class
WStringBuffer::WStringBuffer(const WStringBuffer& other)
    : pText(0), Length(0), Reserved(0,0)
{
    // Our reserve is 0. It's ok, although not efficient. Should we add
    // a different constructor?
    if (other.pText && Resize(other.Length+1))    
        memcpy(pText, other.pText, (other.Length+1)*sizeof(wchar_t));
}

WStringBuffer::~WStringBuffer()
{
    if ((pText != Reserved.pBuffer) && pText)
        SF_FREE(pText);
}

bool     WStringBuffer::Resize(UPInt size)
{    
    if ((size > Length) && (size >= Reserved.Size))
    {
        wchar_t* palloc = (wchar_t*) SF_ALLOC(sizeof(wchar_t)*(size+1), Stat_Default_Mem);
        if (palloc)
        {
            if (pText)
                memcpy(palloc, pText, (Length+1)*sizeof(wchar_t));
            palloc[size] = 0;

            if ((pText != Reserved.pBuffer) && pText)
                SF_FREE(pText);
            pText  = palloc;
            Length = size;
            return true;
        }
        return false;
    }

    if (pText)
        pText[size] = 0;
    Length = size;
    return true;
}


// Assign buffer data.
WStringBuffer& WStringBuffer::operator = (const WStringBuffer& buff)
{
    SetString(buff.pText, buff.Length);
    return *this;
}

WStringBuffer& WStringBuffer::operator = (const String& str)
{
    const UPInt size = str.GetLength();
    if (Resize(size) && size)    
        UTF8Util::DecodeStringSafe(pText, size + 1, str.ToCStr(), str.GetSize());
    return *this;
}

WStringBuffer& WStringBuffer::operator = (const char* putf8str)
{
    const UPInt size = UTF8Util::GetLength(putf8str);
    if (Resize(size) && size)    
        UTF8Util::DecodeStringSafe(pText, size + 1, putf8str);
    return *this;
}

WStringBuffer& WStringBuffer::operator = (const wchar_t *pstr)
{
    UPInt length = 0;
    for (const wchar_t *pstr2 = pstr; *pstr2 != 0; pstr2++)
        length++;     
    SetString(pstr, length);
    return *this;
}

void WStringBuffer::SetString(const char* putf8str, UPInt utf8Sz)
{
    if (utf8Sz == SF_MAX_UPINT)
        utf8Sz = SFstrlen(putf8str);
    const UPInt numChars = UTF8Util::GetLength(putf8str, utf8Sz);
    if (Resize(numChars) && utf8Sz)
        UTF8Util::DecodeStringSafe(pText, numChars + 1, putf8str, utf8Sz);
}

void WStringBuffer::SetString(const wchar_t* pstr, UPInt length)
{
    if (length == SF_MAX_UPINT)
        length = SFwcslen(pstr);
    if (Resize(length) && length)
        memcpy(pText, pstr, (length+1)*sizeof(wchar_t));    
}

void WStringBuffer::StripTrailingNewLines()
{
    SPInt len = SPInt(Length);
    // check, is the content already null terminated
    if (len > 0 && pText[len -1] == 0)
        --len; //if yes, skip the '\0'
    for (SPInt i = len - 1; i >= 0 && (pText[i] == '\n' || pText[i] == '\r'); --i)
    {
        --Length;
        pText[i] = 0;
    }
}

} // Scaleform
