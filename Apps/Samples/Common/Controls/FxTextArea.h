/**************************************************************************

Filename    :   FxTextArea.h
Content     :   GFx to C++ Text Area interface
Created     :   5/7/2007
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_FxTextArea_H
#define INC_FxTextArea_H

#include "GFxPlayer.h"
#include "FxCommand.h"
#include "GStd.h"


// ********************************************************************
// Interface for the text buffer driving the text area view
//
struct FxTextAreaBuffer : public GRefCountBase<FxTextAreaBuffer, GStat_Default_Mem>
{
    virtual UPInt           GetNumLines() = 0;
    virtual const GFxValue* GetLines() = 0;

    bool        IsHTML;
    UPInt       ScrollPos;
    UPInt       MaxScrollPos;
};


// ********************************************************************
// Base class for all text areas that require communication with the 
// FxTextArea actionscript class. 
//
class FxTextArea :   public FxCommandHandler
{
public:

    enum ViewStateType
    {
        VS_MaintainScrollPosition,
        VS_SnapScrollPositionToTop,
        VS_SnapScrollPositionToBottom,
    };

    // *** GFx Text Area Commands
    // Derived classes or wrappers need to create a command map using 
    // these values. The corresponding actionscript listbox widget 
    // takes a Tag string in its constructor. This Tag is appended to 
    // certain fscommand strings to allow identification of where the 
    // fscommand originated from. The following is the complete set of 
    // commands emitted from an FxTextArea widget (where MyTextAreaTag 
    // is the Tag used in the widget's constructor)
    //
    // GFxCommand FxTextArea_Commands[] =
    // {
    //    { CMD_FxTextArea_OnInitialize,         "MyTextAreaTag_OnInitialize"       },
    //    { CMD_FxTextArea_OnDestroy,            "MyTextAreaTag_OnDestroy"          },
    //    { CMD_FxTextArea_OnSetVisibleCount,    "MyListTag_OnSetVisibleCount"  },
    //    { CMD_FxTextArea_OnScroll,             "MyTextAreaTag_OnScroll"           },
    //    { CMD_FxTextArea_OnScrollDrag,         "MyTextAreaTag_OnScrollDrag"       },
    //    { CMD_FxTextArea_OnNewScrollValues,    "MyTextAreaTag_OnNewScrollValues"  },
    //    { 0, 0},
    // };
    //
    enum CommandKeys
    {
        // (PPS Note: THESE CAN BE MOVED TO FxWidget)
        CMD_FxTextArea_OnInitialize = 0xF0, 
        CMD_FxTextArea_OnDestroy,

        // (PPS Note: THESE CAN BE MOVED TO FxScrollableWidget) 
        CMD_FxTextArea_OnSetVisibleCount,
        CMD_FxTextArea_OnScroll, 
        CMD_FxTextArea_OnScrollDrag, 

        CMD_FxTextArea_OnNewScrollBarValues,

        CMD_FxTextArea_NumCommands,
    };

    // Methods that dervied classes should implement to return a commandmap
    virtual FxCommand* GetCommandMap() = 0;

    // All commands registered to the command manager are then sent to the VFsCallback function.
    // Derived classes should return 1 if command handled, otherwise, 0 to continue processing
    virtual bool                VFsCallback(class GFxMovieView* pmovie, 
                                    int cmdKey, const char* cmdName, const char* args);    

    FxTextArea();

    // Initialization methods
    // (PPS Note: THESE CAN BE MOVED TO FxWidget)
    void                        RegisterMovieContainingWidget(GFxMovieView* pmovie);
    
    bool                        IsWidgetInitialized() { return TextAreaPB.HasParentPath(); }    
    //bool                        IsWidgetInitialized() { return (TextAreaPath[0] != NULL); }

    // Methods to communicate with the ActionScript widget
    void                        UIRefreshView();

    void                        SetScrollPosition(UPInt idx);
    void                        SetViewState(ViewStateType viewState);
    void                        SetBuffer(GPtr<FxTextAreaBuffer> pbuffer);

    // Reset the listbox (clears all items, 0s dimension values, etc.)
    void                        Reset();                

protected:

    GPtr<FxTextAreaBuffer>      pBuffer;

    // (PPS Note: THESE CAN BE MOVED TO FxWidget)
    // The movie containing the widget
    GFxMovieView*               pMovie;                 
    // Set the ActionScript path to the list box widget
    void                        SetASWidgetPath(const char* plp);                  

    // Character buffer holding the path to the list box widget.  
    FxSwfPathBuilder   TextAreaPB;


private:

    ViewStateType               ViewState;
};


#endif // INC_FxTextArea_H
