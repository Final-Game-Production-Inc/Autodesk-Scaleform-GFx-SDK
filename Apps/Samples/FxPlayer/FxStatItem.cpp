/**************************************************************************

Filename    :   FxStatItem.cpp
Content     :   Resource statistics tracking
Created     :   October 23, 2008
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "FxStatItem.h"

#include "GFx_Kernel.h"


// ***** FxStatItemChildArray

UPInt       FxStatItemChildArray::GetChildCount() const
{
    UPInt count = 0;        
    for (unsigned i = 0; i< Elements.GetSize(); i++)
        count += Elements[i].GetSize();
    return count;
}

FxStatItem* FxStatItemChildArray::GetChild(UPInt index) const
{
    UPInt j = index;
    for (unsigned i = 0; i< Elements.GetSize(); i++)
    {
        if (Elements[i].IsItem())
        {
            if (j == 0) return Elements[i].GetItem();
            j--;
        }
        else
        {
            SF_ASSERT(Elements[i].IsArrayList());
            UPInt arraySize = Elements[i].GetArray().GetSize();
            if (j< arraySize)
                return Elements[i].GetArray().At(j);
            j-= arraySize;
        }
    }
    SF_ASSERT(0);
    return 0;
}


void FxStatItem::Accept( Visitor& v )
{
    v.Visit_FxStatItem(*this);
}
