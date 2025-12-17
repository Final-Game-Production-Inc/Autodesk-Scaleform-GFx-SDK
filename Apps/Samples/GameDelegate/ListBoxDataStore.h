/**************************************************************************

Filename    :   ListBoxDataStore.h
Content     :   Data storage for the listboxes in GameDelegateDemo.swf
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_ListBoxDataStore_H
#define INC_ListBoxDataStore_H

#include "FxGameDelegate.h"

GString CreateRandomString();

//
// Data stores for the listboxes in Resources/CLIK/demos/GameDelegateDemo.swf
//


class SimpleDataStore : public FxDelegateHandler
{
public:
    SimpleDataStore();

    static void         RequestItemRange(const FxDelegateArgs& pparams);
    static void         RequestLength(const FxDelegateArgs& pparams);
    
    void                Accept(CallbackProcessor* cbreg);

private:
    GArray<GString>     NameData;
};

//////////////////////////////////////////////////////////////////////////

class ComplexDataStore : public FxDelegateHandler
{
public:
    ComplexDataStore();

    static void         RequestItemRange(const FxDelegateArgs& pparams);
    static void         RequestLength(const FxDelegateArgs& pparams);
    
    void                Accept(CallbackProcessor* cbreg);

private:
    GArray<UPInt>       IDData;
    GArray<GString>     NameData;
};



#endif  // INC_ListBoxDataStore_H
