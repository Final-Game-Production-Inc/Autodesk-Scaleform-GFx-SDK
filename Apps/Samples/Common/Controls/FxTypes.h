/**************************************************************************

Filename    :   FxTypes.h
Content     :   Custom GFx types for samples
Created     :   9/4/2007
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_FXTYPES_H
#define INC_FXTYPES_H

#include "GString.h"

struct gstring_key
{
    const char *pChar;

    gstring_key(const char *pc)
    {
        pChar = pc;
    }
    bool operator == (const gstring_key& c) const
    {
        return strcmp(pChar, c.pChar) == 0;
    }
    bool operator != (const gstring_key& c) const
    {
        return strcmp(pChar, c.pChar) != 0;
    }
};

class gstring_hashfn
{
public:

    size_t  operator()(const gstring_key& data) const
    { 
        size_t  size = strlen(data.pChar);
        return GString::BernsteinHashFunction(data.pChar, size);
    }
};

template <class N>
class GConstStringHash : public GTL::ghash<gstring_key, N, gstring_hashfn> {};

#endif//INC_FXTYPES_H
