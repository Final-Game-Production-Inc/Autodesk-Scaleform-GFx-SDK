/**************************************************************************

Filename    :   OptionsScreen.cpp
Content     :   Data storage for the options screen in 
                GameDelegateDemo.swf
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "OptionsScreen.h"

OptionsScreen::OptionsScreen()
{
    DifficultyData.PushBack("Easy");
    DifficultyData.PushBack("Medium");
    DifficultyData.PushBack("Hard");
    DifficultyData.PushBack("Very Hard");
    DifficultyData.PushBack("Insane");
    DifficultyChoice = 1;
    bBloom = true;
    bAA = false;
    AAMode = AA_Unknown;
    Volume = 0.5f;

    GFC_DEBUG_MESSAGE(1, "OptionsScreen init done");
}

void OptionsScreen::Accept(CallbackProcessor* cbreg)
{
    cbreg->Process( "Options.loadDifficulties", &OptionsScreen::LoadDifficulties );
    cbreg->Process( "Options.loadOptionValues", &OptionsScreen::LoadOptionValues );
    cbreg->Process( "Options.apply", &OptionsScreen::Apply );
}

void OptionsScreen::LoadDifficulties(const FxDelegateArgs& pparams)
{
    GFC_DEBUG_MESSAGE(1, "OptionsScreen::LoadDifficulties");
    OptionsScreen* prt = (OptionsScreen*)pparams.GetHandler();

    FxResponseArgsList rargs;
    for (UPInt i = 0; i < prt->DifficultyData.GetSize(); i++)
    {
        rargs.Add( prt->DifficultyData[i].ToCStr() );
    }
    pparams.Respond(rargs);
}

void OptionsScreen::LoadOptionValues(const FxDelegateArgs& pparams)
{
    GFC_DEBUG_MESSAGE(1, "OptionsScreen::LoadOptionValues");
    OptionsScreen* prt = (OptionsScreen*)pparams.GetHandler();

    FxResponseArgs<5> rargs;
    rargs.Add( (Double)prt->DifficultyChoice);
    rargs.Add( prt->bBloom );
    rargs.Add( prt->bAA );
    rargs.Add( (Double)prt->AAMode );
    rargs.Add( (Double)prt->Volume );
    pparams.Respond(rargs);
}

void OptionsScreen::Apply(const FxDelegateArgs& pparams)
{
    GFC_DEBUG_MESSAGE(1, "OptionsScreen::Apply");
    OptionsScreen* prt = (OptionsScreen*)pparams.GetHandler();

    GASSERT(pparams.GetArgCount() > 4);
    prt->DifficultyChoice = (UPInt)pparams[0].GetNumber();
    prt->bBloom = pparams[1].GetBool();
    prt->bAA = pparams[2].GetBool();
    prt->AAMode = (AAModeType)(int)pparams[3].GetNumber();
    prt->Volume = (Float)pparams[4].GetNumber();
    // No response
}

