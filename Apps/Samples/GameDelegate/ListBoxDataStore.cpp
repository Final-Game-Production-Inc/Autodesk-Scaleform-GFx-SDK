/**************************************************************************

Filename    :   ListBoxDataStore.cpp
Content     :   Data storage for the listboxes in GameDelegateDemo.swf
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "ListBoxDataStore.h"

GString CreateRandomString()
{
    GString ret;
    UPInt maxLen = 10;
    for (UPInt i=0; i < maxLen; i++)
    {
        ret.AppendChar( (rand() % ('z' - 'a')) + 'a' );
    }
    return ret;
}

//////////////////////////////////////////////////////////////////////////

SimpleDataStore::SimpleDataStore()
{
    for (UPInt i=0; i < 10000; i++)
    {
        NameData.PushBack(CreateRandomString());
    }
    GFC_DEBUG_MESSAGE1(1, "SimpleDataStore init done: %d", NameData.GetSize());
}

void SimpleDataStore::Accept(CallbackProcessor* cbreg)
{
    cbreg->Process( "Simple.requestItemRange", &SimpleDataStore::RequestItemRange );
    cbreg->Process( "Simple.requestLength", &SimpleDataStore::RequestLength );
}

void SimpleDataStore::RequestItemRange(const FxDelegateArgs& pparams)
{
    GASSERT(pparams.GetArgCount() > 1);
    UInt    sidx = (UInt)pparams[0].GetNumber();
    UInt    eidx = (UInt)pparams[1].GetNumber();
    GFC_DEBUG_MESSAGE2(1, "SimpleDataStore::RequestItemRange:%d - %d", sidx, eidx);
    SimpleDataStore* prt = (SimpleDataStore*)pparams.GetHandler();

    FxResponseArgsList rargs;
    SInt    numRet = (eidx - sidx + 1);
    for (SInt i = 0; i < numRet; i++)
    {
        rargs.Add( prt->NameData[sidx+i].ToCStr() );
    }
    pparams.Respond(rargs);
}

void SimpleDataStore::RequestLength(const FxDelegateArgs& pparams)
{
    GFC_DEBUG_MESSAGE(1, "SimpleDataStore::RequestLength");
    SimpleDataStore* prt = (SimpleDataStore*)pparams.GetHandler();

    FxResponseArgs<1> rargs;
    rargs.Add( (Double)prt->NameData.GetSize() );
    pparams.Respond(rargs);
}


//////////////////////////////////////////////////////////////////////////


ComplexDataStore::ComplexDataStore()
{
    for (UPInt i=0; i < 10000; i++)
    {
        IDData.PushBack(i);
        NameData.PushBack(CreateRandomString());
    }
    GFC_DEBUG_MESSAGE(1, "ComplexDataStore init done");
}

void ComplexDataStore::Accept(CallbackProcessor* cbreg)
{
    cbreg->Process( "Complex.requestItemRange", &ComplexDataStore::RequestItemRange );
    cbreg->Process( "Complex.requestLength", &ComplexDataStore::RequestLength );
}

void ComplexDataStore::RequestItemRange(const FxDelegateArgs& pparams)
{
    GASSERT(pparams.GetArgCount() > 1);
    UInt    sidx = (UInt)pparams[0].GetNumber();
    UInt    eidx = (UInt)pparams[1].GetNumber();
    GFC_DEBUG_MESSAGE2(1, "ComplexDataStore::RequestItemRange: %d - %d", sidx, eidx);
    ComplexDataStore* prt = (ComplexDataStore*)pparams.GetHandler();

    FxResponseArgsList rargs;
    UInt    numRet = (eidx - sidx + 1);
    for (UPInt i = 0; i < numRet; i++)
    {
        rargs.Add( (Double)prt->IDData[sidx+i] );
        rargs.Add( prt->NameData[sidx+i].ToCStr() );
    }
    pparams.Respond(rargs);
}

void ComplexDataStore::RequestLength(const FxDelegateArgs& pparams)
{
    GFC_DEBUG_MESSAGE(1, "ComplexDataStore::RequestLength");
    ComplexDataStore* prt = (ComplexDataStore*)pparams.GetHandler();

    FxResponseArgs<1> rargs;
    rargs.Add( (Double)prt->NameData.GetSize() );
    pparams.Respond(rargs);
}
