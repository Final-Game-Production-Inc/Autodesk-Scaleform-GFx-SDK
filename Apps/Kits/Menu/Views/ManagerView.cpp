/**************************************************************************

Filename    :   ManagerView.cpp
Content     :  C++ interface for the manager that loads the various
                views in the Menu Kit. Manages the menu's view stack.

Authors     :  Prasad Silva, Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "ManagerView.h"
#include "../Game.h"
#include "../States/GameState_MainMenu.h"

void    ManagerView::LoadView(const ViewInfo viewInfo)
{
	// AS2 Logic
    if (GetMovieView()->GetAVMVersion() == 1)
    {
        GFx::Value viewContainer, viewLoader;
        GFx::Value result, params[1];
        GFx::Value viewCanvas;
        Val.GetMember("manager", &viewCanvas);

        String containerName(viewInfo.ViewName, "Container");
        viewCanvas.CreateEmptyMovieClip( &viewContainer, containerName );
        viewContainer.CreateEmptyMovieClip( &viewLoader, viewInfo.ViewName );

        GFx::Value::DisplayInfo dInfo;
        dInfo.SetVisible(false);
        viewContainer.SetDisplayInfo(dInfo);

        params[0].SetString(viewInfo.SWFName);
        viewLoader.Invoke( "loadMovie", &result, params, 1);
    }
	// AS3 Logic
    else
    {
        GFx::Value loadViewParams[2];
        loadViewParams[0] = GFx::Value(viewInfo.ViewName);
        loadViewParams[1] = GFx::Value(viewInfo.SWFName);
        Val.Invoke("loadView", NULL, loadViewParams, 2);
    }
}

void    ManagerView::LoadViews()
{
    for (unsigned i = 0; i < ViewData.GetSize(); i++) 
    {
        LoadView( ViewData[i] );
    }
}

void    ManagerView::PushView(MenuView* targetView)
{
    targetView->OnViewActivated();
    targetView->OnTopMostView( true );
    ViewStack.PushBack( targetView );

    Val.Invoke("pushStandardView", NULL, &targetView->GetGFxValue(), 1);
}

void    ManagerView::PushViewByName(const char* viewName)
{
    // printf("\nManagerView :: PushViewByName( %s )", viewName);
    MenuView** targetView = (MenuView**)LoadedViews.Get(viewName);
    if (targetView != NULL)
    {
        PushView( (MenuView*)*targetView );
    }
}

void    ManagerView::AddViewToLoadedViews(String viewName, UIView* view)
{
    LoadedViews.Set(viewName, view);
}

MenuDialog*    ManagerView::SpawnDialog(const char* viewName)
{
    MenuDialog** targetView = (MenuDialog**)LoadedViews.Get(viewName);
    if (targetView != NULL)
    {
        PushDialog(*targetView);
    }
    return *targetView;
}

void    ManagerView::PushDialog(MenuDialog* targetDialog)
{
    targetDialog->OnViewActivated();
    targetDialog->OnTopMostView( true );
    ViewStack.PushBack( targetDialog );

    Val.Invoke("pushDialogView", NULL, &targetDialog->GetGFxValue(), 1);
}


/** Pops a view from the view stack and handles update/close of existing views. */
void    ManagerView::PopView()
{
    // GFx::Value returnValue;
    if ( ViewStack.GetSize() <= 1 )
    {
        return;
    }

    // Call OnViewClosed() for the popped view. Generally, this will disable the view's 
    // list / back button to prevent accidental mouse rollOvers that cause focus to change 
    // undesirably as the view is tweened out.
    ViewStack[ViewStack.GetSize()-1]->OnViewClosed();

    // Remove the view from the stack.
    ViewStack.PopBack();

    // Update the new top-most view.
    ViewStack[ViewStack.GetSize()-1]->OnTopMostView( false );

	GFx::Value val; // NULL Parameter.
	val.SetNull();
    Val.Invoke("popView", NULL, &val, 1);
    return;
}

/** Launches the Game using the GameState object for the Main Menu. */
void    ManagerView::LaunchGame()
{
    pGameState->NextState();
}

/** Gives focus to a particular MovieClip. */
void    ManagerView::SetSelectionFocus(GFxClikWidget& targetMovieClip)
{
    Val.Invoke("setSelectionFocus", NULL, &(targetMovieClip.GetRef()), 1);
}

/** Handler for an Escape key press. */
void    ManagerView::ProcessEscapePress()
{
    if (ViewStack.GetSize() >= 1 && pMainMenuView != NULL)
    {
        ViewStack[ViewStack.GetSize() - 1]->ReturnToPreviousViewImpl();
    }
}

/** Exits the application. */
void    ManagerView::ExitGame()
{
    pGameState->GetGame()->QuitGame();
}

void    ManagerView::ConfigureView(UIView* pview, const char* viewName)
{
    pview->OnViewLoaded();
    AddViewToLoadedViews(viewName, pview);
}
