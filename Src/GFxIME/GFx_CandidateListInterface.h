/**************************************************************************

Filename    :   GFx_CandidateListInterface.h
Content     :   Character browser interface from GFx (Flash) to engine
Created     :   6/18/2006
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_CHARACTERINTERFACE_H
#define INC_CHARACTERINTERFACE_H

#include "GFx_CandidateListBox.h"
class GFxIMEWin32Impl;

namespace Scaleform {
	namespace GFx {
		namespace IME{
struct CandidateListItem : public FxCandidateListItem
{
    UInt32 ID;
    String Name;

    CandidateListItem(char* str)
    {
        Name = str;
    }

    virtual void    operator () (Value& pvalue, const String& key);
};

class CandidateListBox : public FxCandidateListBox
{
public:
	CandidateListBox(GFxIMEWin32Impl* imeImpl): FxCandidateListBox(imeImpl){};

    FxCommand* GetCommandMap();
	
};
}}}
#endif

