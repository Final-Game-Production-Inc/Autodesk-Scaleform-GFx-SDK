/**************************************************************************

Filename    :   SettingsView.h
Content     :   C++ interface for the 'Settings' view in Menu Kit
Authors     :   Prasad Silva, Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SettingsView_H
#define INC_SettingsView_H

// Includes
#include "MenuView.h"
#include "../Game.h"
#include "../Util/GFxClikWidget.h"
#include "../States/GameStateWidgetSupport.h"


class SettingsView : public MenuView
{
public:
    SettingsView(GFx::Value pval, const char* viewName, ManagerView* pmanager, Game* pgame) : MenuView(pval, viewName, pmanager),
        pGameEnv(pgame)
    {
        ViewNameFriendly = "SETTINGS";
    }
    ~SettingsView() {}

    enum SettingType {
        VOLUME_ID
    };

    struct Setting
    {
        SettingType         Id;
        String             Label;
        String             Desc;
        String             Control;
        Array<unsigned>        DataProvider;
        Array<String>     FriendlyDataProvider;
        unsigned            DefaultIndex;
    };

    // Inherited WidgetHandler declarations.
    virtual bool        OnWidgetInitialized(const char* widgetName, const char* widgetPath, GFx::Value widget);

    // Inherited UIView method declarations.
    virtual void        OnViewActivated();
    virtual void        OnTopMostView( bool bIsTopView );
    virtual void        DisableSubComponents(bool bDisableComponents);

    /** 
     * Updates the description/info textField (InfoTxt) whenever the selectedIndex of the 
     * list changes, based on the new selectedIndex
     */
    void                UpdateDescription(int selectedIndex);
    /** Updates the dataProvider for the list based on the ListOptions array of options. */
    void                UpdateListDataProvider();

    const GFxClikWidget&       GetList() const     { return List; }

protected:
    void                SaveState();

    Game*               pGameEnv;

    /** Static GFxClikWidget event listeners for the list which function as proxies for methods in this view. */
    static void         OnList_ItemPress(UIView* pthis, const GFx::FunctionHandler::Params& params);
    static void         OnList_Change(UIView* pthis, const GFx::FunctionHandler::Params& params);
    static void         OnList_FocusIn(UIView* pthis, const GFx::FunctionHandler::Params& params);

    /** Moves the user backward on the view stack by popping the topmost view / dialog. */
    virtual void        ReturnToPreviousViewImpl();

    /** Reference to the list for the view. */
    GFxClikWidget       List;
    /** Reference to the dataProvider for the list. */
    GFx::Value            ListDataProvider;

    /** The array of options for this view. These options are displayed in the view's list. */
    Array<Setting>      ListOptions;

private:

};

#endif  // INC_SettingsView_H
