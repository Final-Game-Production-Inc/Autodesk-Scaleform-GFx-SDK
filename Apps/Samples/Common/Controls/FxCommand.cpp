/**************************************************************************

Filename    :   FxCommand.cpp
Content     :   Interface to the game engine from GFx via fscommands
Created     :   11/9/2006
Authors     :   Brendan Iribe, Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "FxCommand.h"


// ********************************************************************
// This method is called by the FxPlayerFSCallback object that is 
// installed by FxPlayerApp, when an fscommand is received. If the 
// fscommand is not handled at that level, then it is delegated to 
// the command manager through this function.
//
// The fscommand is looked up in the internal const string hash table
// to determine an appropriate handler.
//
// Parameters:
//   GFxMovieView* pmovie - Movie where the fscommand originated from
//   const char* command  - The fscommand string
//   const char* args     - The argument passed to the fscommand
//
bool    FxCommandManager::FsCallback(class GFxMovieView* pmovie, 
                                     const char* command, const char* args)
{
    if (!pmovie)
        return 0;
    GConstStringHash<CommandNode>::Iterator ic = CommandMap.Find(command);
    if (ic != CommandMap.End())
        return ic->Second.pClass->VFsCallback(pmovie, ic->Second.Key, command, args);
    return 0;
}


// ********************************************************************
// Register a command handler to the command manager.
//
// The command map of the command handler is registered in the internal
// fscommand hash table.
//
void    FxCommandManager::Register(FxCommandHandler* phandler)
{
    const FxCommand* pc = phandler->GetCommandMap();
    while (pc->Command)
    {
        GFC_DEBUG_ERROR1(CommandMap.Find(pc->Command) != CommandMap.End(),
            "GFxCommandManager::RegisterHandler - Command %s already registered.", 
            pc->Command);
        CommandMap.Add(pc->Command, CommandNode(pc->Key, phandler) );
        pc++;
    }
}


// ********************************************************************
// Unregister a command handler from the command handler.
//
// The fscommands in the command handler's command map are removed
// from the internal fscommand hash table.
//
void    FxCommandManager::Unregister(FxCommandHandler* phandler)
{
    const FxCommand* pc = phandler->GetCommandMap();
    while (pc->Command)
    {
        GFC_DEBUG_ERROR1(CommandMap.Find(pc->Command) == CommandMap.End(),
            "GFxCommandManager::UnregisterHandler - Command %s is not registered.", 
            pc->Command);
        CommandMap.Remove(pc->Command);
        pc++;
    }
}


// ********************************************************************
// FxSwfPathBuilder constructor
//
// Allocates internal storage and initializes tracking variables.
//
FxSwfPathBuilder::FxSwfPathBuilder()
{
    MaxPathSize = 128;
    BuiltPath = new char[MaxPathSize];
    ActionString = BuiltPath;
    MaxActionSize = MaxPathSize - 1;
    ParentPathLength = 0;
}


// ********************************************************************
// FxSwfPathBuilder destructor
//
// Frees the internal storage.
//
FxSwfPathBuilder::~FxSwfPathBuilder()
{
    delete[] BuiltPath;
}


// ********************************************************************
// Set the parent path used to build path strings
//
// This provides functionality to assign the parent path only once
// to build multiple path strings that uses the provided path.
//
void FxSwfPathBuilder::SetParentPath(const char* widgetpath)
{
    size_t len = G_strlen(widgetpath);
    if (len > MaxPathSize)
    {
        delete[] BuiltPath;
        MaxPathSize = len + 128;
        BuiltPath = new char[MaxPathSize];
    }
    G_strcpy(BuiltPath, MaxPathSize, widgetpath);
    ParentPathLength = len;
    ActionString = BuiltPath + ParentPathLength;
    MaxActionSize = MaxPathSize - ParentPathLength - 1;
}


// ********************************************************************
// Returns true if the parent path was set (else it is an empty string)
//
bool FxSwfPathBuilder::HasParentPath()
{
    return (ParentPathLength > 0);
}


// ********************************************************************
// Builds a path string using the parent path and the provided action
// string.
//
// The parent path does not have to be set again after creating a path
// string. The internal variables maintains the parent path.
//
const char* FxSwfPathBuilder::BuildPath(const char* action)
{
    size_t len = G_strlen(action);
    if ((ParentPathLength + len) >= MaxPathSize)
    {
        char* temp = BuiltPath;
        MaxPathSize = ParentPathLength + len + 32;
        BuiltPath = new char[MaxPathSize];
        G_strcpy(BuiltPath, MaxPathSize, temp);
        delete[] temp;
        ActionString = BuiltPath + ParentPathLength;
        MaxActionSize = MaxPathSize - ParentPathLength - 1;
    }
    G_strcpy(ActionString, MaxActionSize, action);
    return BuiltPath;
}
