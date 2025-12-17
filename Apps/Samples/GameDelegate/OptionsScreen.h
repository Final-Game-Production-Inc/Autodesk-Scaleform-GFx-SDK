/**************************************************************************

Filename    :   OptionsScreen.h
Content     :   Data storage for the options screen in 
                GameDelegateDemo.swf
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_OptionsScreen_H
#define INC_OptionsScreen_H

#include "FxGameDelegate.h"

//
// Data for the options screen
//


class OptionsScreen : public FxDelegateHandler
{
public:    
    enum AAModeType
    {
        AA_Unknown      = -1,
        AA_Edge         = 0,
        AA_HWAA         = 1,
    };

    OptionsScreen();    

    static void         LoadDifficulties(const FxDelegateArgs& pparams);
    static void         LoadOptionValues(const FxDelegateArgs& pparams);    
    static void         Apply(const FxDelegateArgs& pparams);

    void                Accept(CallbackProcessor* cbreg);

private:
    UPInt               DifficultyChoice;
    bool                bBloom;
    bool                bAA;
    AAModeType          AAMode;
    Float               Volume;

    GArray<GString>     DifficultyData;
};


#endif	// INC_OptionsScreen_H
