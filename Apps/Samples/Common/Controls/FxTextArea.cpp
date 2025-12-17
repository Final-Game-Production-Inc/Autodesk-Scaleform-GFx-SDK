/**************************************************************************

Filename    :   FxTextArea.cpp
Content     :   GFx to C++ Text Area interface
Created     :   5/7/2007
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "FxTextArea.h"


// ********************************************************************
// FxTextArea constructor
//
FxTextArea::FxTextArea()
{
    pMovie = NULL;

    ViewState = VS_MaintainScrollPosition;

    Reset();
}


// ********************************************************************
// Register the movie containing the text area widget
//
void FxTextArea::RegisterMovieContainingWidget(GFxMovieView* pmovie)
{
    pMovie = pmovie;
}


// ********************************************************************
// Register the path to the text area widget object
//
// Used to communicate with ActionScript
//
void FxTextArea::SetASWidgetPath(const char* plp)
{
    TextAreaPB.SetParentPath(plp);
}


// ********************************************************************
// Clear the text area
//
// Removes the text buffer.
//
void FxTextArea::Reset()
{
    pBuffer = NULL;
}


// ********************************************************************
// Set the scrollbar position explicitly.
//
void FxTextArea::SetScrollPosition(UPInt idx)
{
    pMovie->Invoke(TextAreaPB.BuildPath(".SetScrollbarPosition"), "%d", idx);
}


// ********************************************************************
// Set the view state
//
// The view state determines how the scroll position is set when
// the text area is refreshed.
//
void FxTextArea::SetViewState(ViewStateType viewState)
{
    ViewState = viewState;
}


// ********************************************************************
// Set the text buffer driving the view
//
void FxTextArea::SetBuffer(GPtr<FxTextAreaBuffer> pbuffer)
{
    pBuffer = pbuffer;
    if ( (pBuffer.GetPtr() != NULL) && TextAreaPB.HasParentPath() )
    {
        pMovie->Invoke(TextAreaPB.BuildPath(".SetNextScrollPos"), "%d", pBuffer->ScrollPos);
    }
}

// ********************************************************************
// Refresh the text area view
//
void FxTextArea::UIRefreshView()
{
    if (!TextAreaPB.HasParentPath())
        return;

    pMovie->Invoke(TextAreaPB.BuildPath(".SetViewState"), "%d", ViewState);

    if (pBuffer.GetPtr() != NULL)
    {   
        if (pBuffer->IsHTML)
            pMovie->Invoke(TextAreaPB.BuildPath(".SetHTMLText"), pBuffer->GetLines(), pBuffer->GetNumLines());
        else
            pMovie->Invoke(TextAreaPB.BuildPath(".SetText"), pBuffer->GetLines(), pBuffer->GetNumLines());
    }
    else
    {
        pMovie->Invoke(TextAreaPB.BuildPath(".SetText"), "%s", "");
    }
}


// ********************************************************************
// fscommand callback function
//
bool FxTextArea::VFsCallback(GFxMovieView* pmovie, int commandKey, const char* command, const char* args)
{
    GUNUSED2(command, pmovie);

    switch (commandKey)
    {
    case CMD_FxTextArea_OnInitialize:
        {
            SetASWidgetPath(&args[0]);
            UIRefreshView();
            return 1;
        }
    case CMD_FxTextArea_OnDestroy:
        {
            TextAreaPB.SetParentPath("");
            return 1;
        }

    case CMD_FxTextArea_OnNewScrollBarValues:
        {
            int i = 0;
            while (args[i] != ':')
                i++;
            if (pBuffer.GetPtr() != NULL)
            {
                pBuffer->ScrollPos = atoi(&args[0]);
                pBuffer->MaxScrollPos = atoi(&args[i+1]);
            }
            return 1;
        }
    }

    return 0;
}

