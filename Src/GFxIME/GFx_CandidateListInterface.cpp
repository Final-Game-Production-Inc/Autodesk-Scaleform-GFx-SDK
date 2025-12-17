/**************************************************************************

Filename    :   GFx_CandidateListInterface.cpp
Content     :   Character browser interface from GFx (Flash) to engine
Created     :   11/9/2006
Authors     :   David Cook, Brendan Iribe, Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "GFx_CandidateListInterface.h"

namespace Scaleform {
	namespace GFx {
		namespace IME{

void CandidateListItem::operator () (Value &pvalue, const String& key)
{
    if (SFstrcmp(key, "Value") == 0)
    {
        pvalue.SetString(Name);
    }
    else if (SFstrcmp(key, "ID") == 0)
    {
        pvalue.SetNumber(Double(ID));
    }
};


static FxCommand CandidateListBox_Commands[] =
{
    { FxCandidateListBox::CMD_FxList_OnInitialize,         "CandidateList_OnInitialize"       },
    { FxCandidateListBox::CMD_FxList_OnSetVisibleCount,    "CandidateList_OnSetVisibleCount"  },
    { FxCandidateListBox::CMD_FxList_OnSetItemDataMap,     "CandidateList_OnSetItemDataMap"   },
    { FxCandidateListBox::CMD_FxList_OnItemSelect,         "CandidateList_OnItemSelect"       },

    { 0, 0 },
};

FxCommand* CandidateListBox::GetCommandMap()
{
    return CandidateListBox_Commands;
}
		
}}}