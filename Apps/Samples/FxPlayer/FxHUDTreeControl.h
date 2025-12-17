/**************************************************************************

Filename    :   FxHUDTreeControl.h
Content     :   Statistics reporting base item for FxPlayer 
Created     :   February 2, 2009
Authors     :   Dmitry Polenur

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_FXHUDTREECONTROL_H
#define INC_FXHUDTREECONTROL_H
#include "FxStatItem.h"

class FxArrayReportOutput;



// Functions of this interface are called to inform tree display about changes in the tree
class FxTreeDisplayUpdate
{
public:
    virtual ~FxTreeDisplayUpdate() {}

public:
    virtual void    Clear() = 0;
    virtual void    Insert(UPInt index, const char* pstr) = 0;
    virtual void    Insert(UPInt index, const Array<String>& strList, UPInt start, UPInt end) = 0; 
    virtual void    Remove(UPInt index, UPInt count) = 0;
    virtual void    Replace(UPInt index, const Array<String>& strList, UPInt start, UPInt end) = 0; 
    virtual void    Replace(UPInt index, const String& str) = 0;
    virtual void    SetRefreshButton(bool needsRefresh) = 0;
};

// Tree control class. Maintains tree view. Manages insertion and removal of items. 
class FxTreeControl : public RefCountBase<FxTreeControl, Stat_Default_Mem> //, public FxStatsTreeUpdate
{
public:

    FxTreeControl(FxStatItem* ptree);

public:

    void    SetTree(FxStatItem* ptree);
    void    SetDisplayUpdate(FxTreeDisplayUpdate* pdisplayUpdate) { pDisplayUpdate = pdisplayUpdate; }
    UPInt   GetSize() const { return DisplayList.GetSize(); }
   
    void    Reset();
    void    Refresh();

    // These functions are recursive and will be called for children of the item
    // Hidden items are ignored.
    void  PushBack(FxStatItem* proot, bool includeRoot = false);
    UPInt InsertAt(UPInt index, FxStatItem* pitem, bool includeRoot = false);
    UPInt RemoveAt(UPInt index, bool includeRoot = false);
   
    void UpdateData();
	void OnItemCommand(const char* pcommand, int index);

private:

    struct DisplayListElement 
    {
        Ptr<FxStatItem> pItem;
        SF::StringLH        CurrentStr; // Cashed string value. Used to see if displayed string needs update  
        //DisplayListElement(FxStatItem* pitem) : pItem(pitem) {}
        //DisplayListElement() : pItem(NULL) {}
    };

    // Temporary entry used to add to Display List. Need this to use SF::StringLH
    DisplayListElement TempListElement;

    FxTreeDisplayUpdate* pDisplayUpdate;
    int   CurrentLine;
    bool  IsActive;
    bool  NeedsRefresh;

    Ptr<FxStatItem> pTree; //Tree root
    
    ArrayLH<DisplayListElement> DisplayList; //List of currently displayed items
 
private:

    void ExpandItem(UPInt index);
    void CollapseItem(UPInt index);
    void UpdateStrings(UPInt start, UPInt end, const Array<String>& strList);

    void RefreshItem(FxStatItem* pitem);

    // These functions are called upon tree refresh
    virtual void Insert(FxStatItem* pitem);
    virtual void Remove(FxStatItem* pitem);

    //Item position in DisplayList, -1 if item is not there
    int    GetItemPosition(FxStatItem* pitem) const;
    UPInt  GetLastDesendantPosition(UPInt index) const;
};

#endif // INC_FXHUDTREECONTROL_H
