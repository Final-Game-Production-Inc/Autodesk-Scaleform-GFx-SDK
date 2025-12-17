/**************************************************************************

Filename    :   FxHUDTreeControl.cpp
Content     :   Statistics reporting base item for FxPlayer 
Created     :   February 2, 2009
Authors     :   Dmitry Polenur

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFxCompat.h"
#include "../Src/Kernel/SF_MsgFormat.h"
#include "../Src/Kernel/SF_Std.h"
#include "FxHUDTreeControl.h"
// ***** FxPlayerStatHudControl

void FxTreeControl::PushBack(GFxAmpMemItem* pitem, bool includeRoot)
{
    if (includeRoot)
    {
        TempListElement.pItem = pitem;
        DisplayList.PushBack(TempListElement);
        SetShown(pitem, true);
    }

    if (IsExpanded(pitem))
    {
        for (UInt i = 0; i < pitem->Children.GetSize(); i++)
        {
            PushBack(pitem->Children[i], true);
        }
    }
}

UPInt FxTreeControl::InsertAt(UPInt index, GFxAmpMemItem* pitem, bool includeRoot)
{   
    UInt nInserted = 0;

    if (includeRoot)
    {
        TempListElement.pItem = pitem; 
        DisplayList.InsertAt(index, TempListElement);
        SetShown(pitem, true);
        nInserted++;
    }

    if (!IsExpanded(pitem))
    {
        return nInserted;
    }

    UPInt currentIndex = index + nInserted;
    for (UInt i = 0; i < pitem->Children.GetSize(); i++)
    {
        currentIndex += InsertAt(currentIndex, pitem->Children[i], true);
    }
    return currentIndex - index;
}

UPInt FxTreeControl::RemoveAt(UPInt index, bool includeRoot /*= false*/)
{    
    UPInt nRemoved = GetLastDescendantPosition(index) - index;

    if (includeRoot)
    {
        ++nRemoved;
    }
    else
    {
        ++index;
    }
   
    for (UPInt i = index; i < index + nRemoved; i++)
    {
        SetShown(DisplayList[i].pItem, false);
    }

    DisplayList.RemoveMultipleAt(index, nRemoved);
    return nRemoved;
}


void FxTreeControl::OnItemCommand(const char* pcommand, SInt index)
{
    if (index >= 0 && index < (SInt)DisplayList.GetSize())
    {
        CurrentLine = index;
        GFxAmpMemItem* pitem = DisplayList[index].pItem;

        if (IsExpanded(pitem))
		{
			 if (G_strcmp(pcommand, "click") == 0 || G_strcmp(pcommand, "left") == 0)
             {
				CollapseItem((UPInt)index);
             }
		}
		else
		{
			if (G_strcmp(pcommand, "click") == 0 || G_strcmp(pcommand, "right") == 0)
            {
				ExpandItem((UPInt)index);
            }
		}
    }
}

bool FxTreeControl::IsExpanded(GFxAmpMemItem* item) const
{
    GHashLH<UInt32, ElementExtraData>::ConstIterator it;
    it = ExtraDataMap.Find(item->ID);
    if (it == ExtraDataMap.End())
    {
        return false;
    }
    return it->Second.IsExpanded;
}

bool FxTreeControl::IsShown(GFxAmpMemItem* item) const
{
    GHashLH<UInt32, ElementExtraData>::ConstIterator it;
    it = ExtraDataMap.Find(item->ID);
    if (it == ExtraDataMap.End())
    {
        return false;
    }
    return it->Second.IsShown;
}

void FxTreeControl::SetExpanded(GFxAmpMemItem* item, bool expanded)
{
    GHashLH<UInt32, ElementExtraData>::Iterator it;
    it = ExtraDataMap.Find(item->ID);
    if (it == ExtraDataMap.End())
    {
        ExtraDataMap.Set(item->ID, ElementExtraData(item->StartExpanded));
        it = ExtraDataMap.Find(item->ID);
    }
    it->Second.IsExpanded = expanded;
}

void FxTreeControl::SetShown(GFxAmpMemItem* item, bool shown)
{
    GHashLH<UInt32, ElementExtraData>::Iterator it;
    it = ExtraDataMap.Find(item->ID);
    if (it == ExtraDataMap.End())
    {
        ExtraDataMap.Set(item->ID, ElementExtraData(item->StartExpanded));
        it = ExtraDataMap.Find(item->ID);
    }
    it->Second.IsShown = shown;
}


void FxTreeControl::GetStringArray(GFxAmpMemItem* item, GArray<GString>* entries, UByte indent)
{
    if (!IsShown(item))
    {
        return;
    }

    GString report;
    GArray<char> sft(indent + 1);
    memset(&sft[0], ' ', indent);
    sft[indent] = '\0';
    report.AppendString(&sft[0]);

    if (IsExpanded(item))
    {
        report.AppendString("-");
    }
    else if (item->Children.GetSize() > 0)
    {
        report.AppendString("+");
    }
    report.AppendString(item->Name.ToCStr());
    if (item->HasValue)
    {
        const unsigned tabSize = 75;
        GString valueText;
        G_Format(valueText, " {0:sep:,}", item->Value);

        unsigned numSpaces = tabSize - G_Min(static_cast<unsigned>(report.GetLength() + valueText.GetLength()), tabSize);

        sft.Resize(numSpaces + 1);
        memset(&sft[0], ' ', numSpaces);
        sft[numSpaces] = '\0';
        report.AppendString(&sft[0]);
        report.AppendString(valueText);
    }
    report.AppendString("\n");
    entries->PushBack(report);

    if (IsExpanded(item))
    {
        for (UPInt i = 0; i < item->Children.GetSize(); ++i)
        {
            GetStringArray(item->Children[i], entries, indent + 4);
        }
    }
}


void FxTreeControl::ExpandItem(UPInt index)
{
    GFxAmpMemItem* pitem = DisplayList[index].pItem;
    SetExpanded(DisplayList[index].pItem, true);
    UPInt nInserted = InsertAt(index + 1, pitem, false);

    // Expanding/collapsing item may change formating for other lines 
    GArray<GString> stringArray;
    GetStringArray(pTree, &stringArray);

    GASSERT(stringArray.GetSize() == DisplayList.GetSize());

    UpdateStrings(0, index, stringArray);
    if (pDisplayUpdate)
    {
        pDisplayUpdate->Insert(index + 1, stringArray, index + 1, index +  nInserted);
    }
    UpdateStrings(index + nInserted + 1, stringArray.GetSize() - 1, stringArray);
}


void FxTreeControl::CollapseItem(UPInt index)
{   
    SetExpanded(DisplayList[index].pItem, false);
    UPInt nRemoved =  RemoveAt(index,  false);

    //Expanding/collapsing item may change formating for other lines 
    GArray<GString> stringArray;
    GetStringArray(pTree, &stringArray);

    if (pDisplayUpdate)
    {
        pDisplayUpdate->Remove(index + 1, nRemoved);
    }
    GASSERT(stringArray.GetSize() == DisplayList.GetSize());
    UpdateStrings(0, stringArray.GetSize() -1, stringArray);
}


void FxTreeControl::UpdateData()
{
    GArray<GString> stringArray;
    GetStringArray(pTree, &stringArray);

    if (DisplayList.GetSize() > 0)
    {
        UpdateStrings(0, DisplayList.GetSize() - 1, stringArray);
    }
}


void FxTreeControl::UpdateStrings(UPInt start, UPInt end, const GArray<GString>& strList)
{
    bool isTreeChanged = false;
    SInt replaceStart = -1;
    SInt replaceEnd = -1;
    for (UPInt i = start; i <= end; i++)
    {
        GString str;
        //Update only items with changed strings
        if (DisplayList[i].CurrentStr != strList[i])
        {
            if (replaceStart < 0)
            {
                replaceStart = replaceEnd = (SInt)i;
            }
            else
            {
                replaceEnd = (SInt)i;
            }
            DisplayList[i].CurrentStr = strList[i];       
        }
        else
        {
            GASSERT(replaceEnd >= replaceStart);
            if (pDisplayUpdate && replaceStart >=0)
            {
                pDisplayUpdate->Replace(replaceStart, strList, replaceStart, replaceEnd);
                replaceStart = replaceEnd = -1;
            }
        }
    }

    if (pDisplayUpdate && replaceStart >=0)
    {
        pDisplayUpdate->Replace(replaceStart, strList, replaceStart, replaceEnd);
    }

    if (isTreeChanged)
    {
        pDisplayUpdate->SetRefreshButton(true);
        NeedsRefresh = true;
    }
}


FxTreeControl::FxTreeControl() 
: pDisplayUpdate(NULL), CurrentLine(-1), IsActive(false), NeedsRefresh(false)
{

}


void FxTreeControl::SetTree(GFxAmpMemItem* ptree)
{
    GASSERT(ptree);
    pTree = ptree;
    Reset();
}


void FxTreeControl::Reset()
{
    DisplayList.Clear(); 
    if (pTree)
    {
        PushBack(pTree, true);

        if (pDisplayUpdate)
        {
            pDisplayUpdate->Clear();

            GArray<GString> stringArray;
            GetStringArray(pTree, &stringArray);

            if (stringArray.GetSize() > 0)
            {
                pDisplayUpdate->Insert(0, stringArray, 0, stringArray.GetSize()-1);
            }

            for (UInt i = 0; i < DisplayList.GetSize(); i++)
            {
                DisplayList[i].CurrentStr = stringArray[i];      
            }
        }
    }
}

UPInt FxTreeControl::GetLastDescendantPosition(UPInt index) const
{
    GFxAmpMemItem* pitem = DisplayList[index].pItem;

    if (pitem->Children.GetSize() > 0)
    {
        GFxAmpMemItem* lastChildItem = pitem->Children[pitem->Children.GetSize() - 1];

        for (UPInt i = index + 1; i < DisplayList.GetSize(); ++i)
        {
            if (DisplayList[i].pItem == lastChildItem)
            {
                return GetLastDescendantPosition(i);
            }
        }
    }

    return index;
}

void FxTreeControl::Refresh()
{
    NeedsRefresh = false;
    pDisplayUpdate->SetRefreshButton(false);
    Reset();
}
