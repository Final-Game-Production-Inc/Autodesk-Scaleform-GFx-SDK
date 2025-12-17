/**************************************************************************

Filename    :   SettingsView.cpp
Content     :  C++ implementation of the Settings view of the Menu Kit.

Authors     :  Prasad Silva, Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "SettingsView.h"
#include "ManagerView.h"
#include "../Game.h"

bool    SettingsView::OnWidgetInitialized(const char* widgetName, const char* widgetPath, GFx::Value widget)
{
    bool bHandled = false;

    // printf("\nSettingsView: WidgetName: %s, WidgetPath %s", widgetName, widgetPath);
    if (!SFstrcmp(widgetName, "list"))
    {
        List = GFxClikWidget(widget);

        String changeEventString = (GetManagerView()->GetMovieView()->GetAVMVersion() == 1) ? "change" : "indexChange";
        Ptr<UIViewListenerProxy> changeListener = *new UIViewListenerProxy(this, OnList_Change);
        List.AddEventListener(pManager->GetMovieView(), changeEventString, changeListener);

        Ptr<UIViewListenerProxy> focusInListener = *new UIViewListenerProxy(this, OnList_FocusIn);
        List.AddEventListener(pManager->GetMovieView(), "focusIn", focusInListener);

        bHandled = true;
    }

    if (!bHandled)
    {
        bHandled = MenuView::OnWidgetInitialized(widgetName, widgetPath, widget);
    }

    return bHandled;
}

void    SettingsView::OnViewActivated()
{
    //
}

/** Enable/disable sub-components of the view. */
void    SettingsView::DisableSubComponents(bool bDisableComponents)
{
    GFx::Value bDisable;
    bDisable.SetBoolean(bDisableComponents);

    if ( List.IsValid() )
    {
        List.SetMember("disabled", bDisable);
    }

    if ( BackBtn.IsValid() )
    {
        BackBtn.SetMember("disabled", bDisable);
    }
}

/** Called when the view is pushed on to the stack. Reoccurring view setup can be done here. */
void    SettingsView::OnTopMostView( bool bIsTopView )
{
    MenuView::OnTopMostView(bIsTopView);

    if ( List.IsValid() )
    {
        UpdateListDataProvider();

        GFx::Value selectedIndex;
        selectedIndex.SetNumber(0);
        List.SetMember("selectedIndex", selectedIndex);

        pManager->SetSelectionFocus(List);
    }
}

void    SettingsView::UpdateListDataProvider()
{
    GFx::Value dataProviderArray;
    pManager->GetMovieView()->CreateArray(&dataProviderArray);

    const ArrayCPP< Ptr<GameSettings> >& currentGameSettings = pGameEnv->GetSettings();
    for (unsigned i = 0; i < currentGameSettings.GetSize(); i++)
    {
        GameSettings* setting = currentGameSettings[i];
        
        GFx::Value tempVal, tempObj;
        pManager->GetMovieView()->CreateObject(&tempObj, "Object");

        tempVal.SetNumber(setting->GetID());
        tempObj.SetMember("id", tempVal);

        tempVal.SetString(setting->GetName());
        tempObj.SetMember("label", tempVal);

        tempVal.SetString(setting->GetDesc());
        tempObj.SetMember("desc", tempVal);

        GFx::Value internalDataProviderArray;
        unsigned index = 0;

        switch (setting->GetType())
        {
            case GameSettings::TYPE_Boolean:
                {
                    tempVal.SetString("checkBox");
                    tempObj.SetMember("control", tempVal);

                    tempVal.SetNumber(GameSettings::TYPE_Boolean);
                    tempObj.SetMember("type", tempVal);

                    tempVal.SetBoolean(setting->GetValue().GetBool());
                    tempObj.SetMember("bIsEnabled", tempVal);
                    break;
                }

            case GameSettings::TYPE_NumericRange:
                {
                    tempVal.SetString("stepper");
                    tempObj.SetMember("control", tempVal);

                    tempVal.SetNumber(GameSettings::TYPE_NumericRange);
                    tempObj.SetMember("type", tempVal);

                    pManager->GetMovieView()->CreateArray(&internalDataProviderArray);
                    GameSettings_NumericRange* numericRange = (GameSettings_NumericRange*)setting;
                    for (unsigned j = unsigned(numericRange->GetRangeMin()); j <= unsigned(numericRange->GetRangeMax()); j += unsigned(numericRange->GetRangeIncrement()))
                    {
                        tempVal.SetNumber(j);
                        internalDataProviderArray.SetElement(index++, tempVal);
                    }
                    
                    tempVal.SetNumber(setting->GetValue().GetFloat());
                    tempObj.SetMember("optIndex", tempVal);
                    if (GetManagerView()->GetMovieView()->GetAVMVersion() == 1) 
                    {
                        tempObj.SetMember("dataProvider", internalDataProviderArray);
                    }
                    else if (GetManagerView()->GetMovieView()->GetAVMVersion() == 2) 
                    {
                        GFx::Value dataProviderObject;
                        pManager->GetMovieView()->CreateObject(&dataProviderObject, "scaleform.clik.data.DataProvider", &internalDataProviderArray, 1);
                        tempObj.SetMember("dataProvider", dataProviderObject);
                    }
                    break;
                }

            case GameSettings::TYPE_ValueList:
                {
                    tempVal.SetString("stepper");
                    tempObj.SetMember("control", tempVal);

                    tempVal.SetNumber(GameSettings::TYPE_ValueList);
                    tempObj.SetMember("type", tempVal);

                    pManager->GetMovieView()->CreateArray(&internalDataProviderArray);
                    GameSettings_ValueList* valueList = (GameSettings_ValueList*)setting;
                    
                    const GameSettings_ValueList::ValueListType& values = valueList->GetValues();
                    for (unsigned j = 0; j < values.GetSize(); j++)
                    {
                        tempVal.SetString(values[j]);
                        internalDataProviderArray.SetElement(index++, tempVal);
                    }

                    tempVal.SetNumber(setting->GetValue().GetInt());
                    tempObj.SetMember("optIndex", tempVal);
                    if (GetManagerView()->GetMovieView()->GetAVMVersion() == 1) 
                    {
                        tempObj.SetMember("dataProvider", internalDataProviderArray);
                    }
                    else if (GetManagerView()->GetMovieView()->GetAVMVersion() == 2) 
                    {
                        GFx::Value dataProviderObject;
                        pManager->GetMovieView()->CreateObject(&dataProviderObject, "scaleform.clik.data.DataProvider", &internalDataProviderArray, 1);
                        tempObj.SetMember("dataProvider", dataProviderObject);
                    }
                    break;
                }

            default:
                break;
        }

        dataProviderArray.SetElement(i, tempObj);
    }

    if (GetManagerView()->GetMovieView()->GetAVMVersion() == 1) 
    {
        // Set the dataProvider using ScrollingList :: set dataProvider() to ensure "change" event is fired.
        List.SetMember("dataProvider", dataProviderArray);
    }
    else if (GetManagerView()->GetMovieView()->GetAVMVersion() == 2) 
    {
        GFx::Value dataProviderObject;
        pManager->GetMovieView()->CreateObject(&dataProviderObject, "scaleform.clik.data.DataProvider", &dataProviderArray, 1);
        List.SetMember("dataProvider", dataProviderObject);
        List.GetRef().Invoke("validateNow");
    }

    // Make sure we have a reference pointing to the latest dataProvider for the list for convenient access.
    List.GetMember("dataProvider", &ListDataProvider);
}

void    SettingsView::UpdateDescription(int selectedIndex)
{
    SF_ASSERT(List.IsValid());
    
    if (ListDataProvider.IsNull() || ListDataProvider.IsUndefined()) {
        return;
    }
    
    if (!(InfoTxt.IsNull() || InfoTxt.IsUndefined()))
    {
        // If a selectedIndex wasn't provided, retrieve it.
        if (selectedIndex < 0)
        {
            GFx::Value tempVal;
            List.GetMember("selectedIndex", &tempVal);
            selectedIndex = unsigned(toNumber(tempVal));
        }

        // Set the description for the Info textField.
        String description;
        if ( selectedIndex >= 0 )
        {
            GFx::Value dataVal, tempVal;
            
            ListDataProvider.GetElement(selectedIndex, &dataVal);
            dataVal.GetMember("desc", &tempVal);
            description = tempVal.GetString();
        }
        else 
        {
            description = "";
        }

        InfoTxt.SetText(description);
    }
}

void    SettingsView::SaveState()
{
    GFx::Value dataVal, dpLengthVal, tempVal, idVal, typeVal;
    unsigned settingId;
    ListDataProvider.GetMember("length", &dpLengthVal);
    unsigned dpLength = unsigned(toNumber(dpLengthVal));
    for (unsigned i = 0; i < dpLength; i++)
    {
        ListDataProvider.GetElement(i, &dataVal);
        
        dataVal.GetMember("id", &idVal);
        settingId = unsigned(toNumber(idVal));

        const ArrayCPP< Ptr<GameSettings> >& currentGameSettings = pGameEnv->GetSettings();
        for (unsigned j = 0; j < currentGameSettings.GetSize(); j++)
        {
            GameSettings* setting = currentGameSettings[j];
            if (setting->GetID() == settingId)
            {
                dataVal.GetMember("type", &typeVal);
                unsigned settingType = unsigned(toNumber(typeVal));
                
                if (settingType == GameSettings::TYPE_Boolean)
                {
                    dataVal.GetMember("bIsEnabled", &tempVal);
                    setting->SetValue( GameSettingsValue(settingId, tempVal.GetBool()) );
                }
                else if (settingType == GameSettings::TYPE_NumericRange)
                {
                    dataVal.GetMember("optIndex", &tempVal);
                    setting->SetValue( GameSettingsValue(settingId, (float)(toNumber(tempVal))) );
                }
                else if (settingType == GameSettings::TYPE_ValueList)
                {
                    dataVal.GetMember("optIndex", &tempVal);
                    setting->SetValue( GameSettingsValue(settingId, (int)(toNumber(tempVal))) );
                }
            }
        }
    }
}

void    SettingsView::OnList_Change(UIView* pthis, const GFx::FunctionHandler::Params& params)
{
    SettingsView* pview = (SettingsView*)pthis;
    EventData eventData = ConvertParamsToEventData(params);
    unsigned selectedIndex = eventData.Index;
    pview->UpdateDescription(selectedIndex);
}

void    SettingsView::OnList_FocusIn(UIView* pthis, const GFx::FunctionHandler::Params& params)
{
    SF_UNUSED(params);
    SettingsView* pview = (SettingsView*)pthis;
    GFx::Value selectedIndexValue;
    pview->GetList().GetMember("selectedIndex", &selectedIndexValue);
    unsigned selectedIndex = unsigned(toNumber(selectedIndexValue));
    pview->UpdateDescription(selectedIndex);
}

/** Moves the user backward on the view stack by popping the topmost view / dialog. */
void    SettingsView::ReturnToPreviousViewImpl()
{
    SaveState();
    UIView::ReturnToPreviousViewImpl();
}
