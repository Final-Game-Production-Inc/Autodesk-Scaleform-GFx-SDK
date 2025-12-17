/**************************************************************************

Filename    :   GameStateWidgetSupport.cpp
Content     :   Game state with CLIK widget handling support for 
                 Menu Kit
Authors     :   Prasad Silva, Nate Mitchell

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GameStateWidgetSupport.h"

// Returns the left-part of a string minus the rightmost token, delimited
// by seprator.
// E.g: StripNextRToken("foo/bar", '/') will return "foo"
//
static StringDataPtr StripNextRToken(const StringDataPtr& str, char separator)
{
    const char* cur_str = str.ToCStr();
    int cur_pos = (int)str.GetSize()-1;

    for (; cur_pos >= 0; --cur_pos)
    {
        if (cur_str[cur_pos] == separator)
        {
            break;
        }
    }

    return StringDataPtr( cur_str, Alg::Max(cur_pos, 0) );
}


// Support class to manage OnLoad() and OnUnload() ActionScript callbacks from GFx
class GFxCLIKObjectEventCallback : public GFx::FunctionHandler
{
protected:
    // Reference to the GFxMoviePlayer that is responsible for these ActionScript objects and 
    // handling of their callbacks
    GameStateWidgetSupport* pGameState;

public:
    GFxCLIKObjectEventCallback(GameStateWidgetSupport* pgameState) : pGameState(pgameState) {}

    // Calls the appropriate event after the arguments have been extracted from 
    // the params.  If PathHandlingWidget != NULL, the event will be called on 
    // that widget instead of on Movie
    virtual bool CallEventHandler(const char* WidgetName, const char* WidgetPath, 
                                  GFx::Value Widget, WidgetHandler* PathHandlingWidget=NULL ) = 0;

    // Called from the GFx runtime
    virtual void Call(const Params& params)
    {
        if( pGameState != NULL )
        {
            // Widget name, as an ASstring is the first parameter
            const char* WidgetName = params.pArgs[0].GetString();

            // Widget path, as an ASstring is the second parameter
            String WidgetPathString = params.pArgs[1].GetString();
            const char* WidgetPath = WidgetPathString.ToCStr();

            // See if the current path is registered with a path binding.
            // Split the string on the delimiter, and keep going until we can split no more.  Note that we split from right to left, so that we always take the most derived path
            WidgetHandler* PathHandler = NULL;
            StringDataPtr Tmp;
            StringDataPtr pPath(WidgetPathString);

            while ( !(pPath = StripNextRToken(pPath, '.')).IsEmpty() )
            {
                // Path is the trimmed String.
                String path(pPath.ToCStr(), pPath.GetSize());
                PathHandler = pGameState->GetHandlerForPath( path );  //( FName(*WidgetPathString) );
                if( PathHandler != NULL )
                {
                    // Found a match in the path binding list.  Search no more, and pass the init call to that widget
                    break;
                }
            }

            // Call the appropriate event corresponding to the ActionScript event
            bool bWidgetInitHandled = CallEventHandler(WidgetName, WidgetPath, params.pArgs[2], PathHandler);

            // If we care about warnings for widgets that weren't handed via WidgetInitialized, throw a log out
            if( !bWidgetInitHandled )
            {
                // printf("\nWarning - Widget %s (%s) is making an init callback that is not being handled by WidgetInitialized()!", WidgetName, WidgetPathString.ToCStr());
            }
        }
    }
};

// Handler for the ActionScript OnLoad
class GFxCLIKObjectOnLoadEventCallback : public GFxCLIKObjectEventCallback
{
public:
    GFxCLIKObjectOnLoadEventCallback(GameStateWidgetSupport* InMovie) : GFxCLIKObjectEventCallback(InMovie) {}

    // Calls the appropriate event after the arguments have been extracted from the params
    virtual bool CallEventHandler(const char* WidgetName, const char* WidgetPath, GFx::Value Widget, WidgetHandler* PathHandlingWidget=NULL)
    {
        pGameState->bWidgetsInitializedThisFrame = true;

        if( PathHandlingWidget != NULL )
        {
            return PathHandlingWidget->OnWidgetInitialized(WidgetName, WidgetPath, Widget);
        }
        else
        {
            return pGameState->OnWidgetInitialized(WidgetName, WidgetPath, Widget);
        }
    }
};

// Handler for the ActionScript OnUnload
class GFxCLIKObjectOnUnloadEventCallback : public GFxCLIKObjectEventCallback
{
public:
    GFxCLIKObjectOnUnloadEventCallback(GameStateWidgetSupport* pgameState) : GFxCLIKObjectEventCallback(pgameState) {}

    // Calls the appropriate event after the arguments have been extracted from the params
    virtual bool CallEventHandler(const char* WidgetName, const char* WidgetPath, GFx::Value Widget, WidgetHandler* PathHandlingWidget=NULL)
    {
        if( PathHandlingWidget != NULL )
        {
            return PathHandlingWidget->OnWidgetUninitialized(WidgetName, WidgetPath, Widget);
        }
        else
        {
            return pGameState->OnWidgetUninitialized(WidgetName, WidgetPath, Widget);
        }
    }
};

void GameStateWidgetSupport::InstallWidgetCallbacks(GameUISwfMovie* pswfMovie)
{
    GFx::Value func;
    GFx::Movie* pmovie = pswfMovie->GetSwf();
    SF_ASSERT(pmovie);

    Ptr<GFxCLIKObjectOnLoadEventCallback> LoadCallback = *new GFxCLIKObjectOnLoadEventCallback(this);
    Ptr<GFxCLIKObjectOnUnloadEventCallback> UnloadCallback = *new GFxCLIKObjectOnUnloadEventCallback(this);

    if (pswfMovie->GetSwf()->GetAVMVersion() == 1)
    {

        pmovie->CreateFunction(&func, LoadCallback, NULL);
        pmovie->SetVariable("_global.CLIK_loadCallback", func);

        pmovie->CreateFunction(&func, UnloadCallback, NULL);
        pmovie->SetVariable("_global.CLIK_unloadCallback", func);
    }
    else
    {
        GFx::Value arg("scaleform.gfx.Extensions");
        GFx::Value clikClass;
        pmovie->Invoke("flash.utils.getDefinitionByName", &clikClass, &arg, 1);
        SF_ASSERT(clikClass.IsObject());

        pmovie->CreateFunction(&func, LoadCallback, NULL);
        clikClass.SetMember("CLIK_addedToStageCallback", func);

        pmovie->CreateFunction(&func, UnloadCallback, NULL);
        clikClass.SetMember("CLIK_removedFromStageCallback", func);
    }

}
