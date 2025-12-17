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

// Functions of this interface are called to inform tree display about changes in the tree
class FxTreeDisplayUpdate
{
public:
    virtual ~FxTreeDisplayUpdate() {}

public:
    virtual void    Clear() = 0;
    virtual void    Insert(UPInt index, const char* pstr) = 0;
    virtual void    Insert(UPInt index, const GArray<GString>& strList, UPInt start, UPInt end) = 0; 
    virtual void    Remove(UPInt index, UPInt count) = 0;
    virtual void    Replace(UPInt index, const GArray<GString>& strList, UPInt start, UPInt end) = 0; 
    virtual void    Replace(UPInt index, const GString& str) = 0;
    virtual void    SetRefreshButton(bool needsRefresh) = 0;
};

// Tree control class. Maintains tree view. Manages insertion and removal of items. 
class FxTreeControl : public GRefCountBase<FxTreeControl, GStat_Default_Mem> //, public FxStatsTreeUpdate
{
public:

    FxTreeControl();

public:

    void    SetTree(GFxAmpMemItem* ptree);
    void    SetDisplayUpdate(FxTreeDisplayUpdate* pdisplayUpdate) { pDisplayUpdate = pdisplayUpdate; }
    UPInt   GetSize() const { return DisplayList.GetSize(); }
   
    void    Reset();
    void    Refresh();

    // These functions are recursive and will be called for children of the item
    // Hidden items are ignored.
    void  PushBack(GFxAmpMemItem* proot, bool includeRoot = false);
    UPInt InsertAt(UPInt index, GFxAmpMemItem* pitem, bool includeRoot = false);
    UPInt RemoveAt(UPInt index, bool includeRoot = false);
   
    void UpdateData();
	void OnItemCommand(const char* pcommand, SInt index);

private:

    struct DisplayListElement 
    {
        GPtr<GFxAmpMemItem> pItem;
        GStringLH        CurrentStr; // Cashed string value. Used to see if displayed string needs update  
    };

    // Temporary entry used to add to Display List. Need this to use GStringLH
    DisplayListElement TempListElement;

    FxTreeDisplayUpdate* pDisplayUpdate;
    SInt  CurrentLine;
    bool  IsActive;
    bool  NeedsRefresh;

    GPtr<GFxAmpMemItem> pTree; //Tree root

    struct ElementExtraData 
    {
        ElementExtraData(bool startExpanded) : IsExpanded(startExpanded), IsShown(false) { }
        bool IsExpanded;
        bool IsShown;
    };
    GHashLH<UInt32, ElementExtraData> ExtraDataMap;
    
    GArrayLH<DisplayListElement> DisplayList; //List of currently displayed items
 
private:

    bool IsExpanded(GFxAmpMemItem* item) const;
    bool IsShown(GFxAmpMemItem* item) const;

    void SetExpanded(GFxAmpMemItem* item, bool expanded);
    void SetShown(GFxAmpMemItem* item, bool shown);

    void GetStringArray(GFxAmpMemItem* item, GArray<GString>* entries, UByte indent = 0);
    void ExpandItem(UPInt index);
    void CollapseItem(UPInt index);
    void UpdateStrings(UPInt start, UPInt end, const GArray<GString>& strList);

    void RefreshItem(GFxAmpMemItem* pitem);

    //Item position in DisplayList, -1 if item is not there
    UPInt  GetLastDescendantPosition(UPInt index) const;
};

#endif // INC_FXHUDTREECONTROL_H
