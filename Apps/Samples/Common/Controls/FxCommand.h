/**************************************************************************

Filename    :   FxCommand.h
Content     :   Interface to the game engine from GFx via fscommands
Created     :   11/9/2006
Authors     :   Brendan Iribe, Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_FxCommand_H
#define INC_FxCommand_H

#include "GString.h"
#include "GHash.h"

// This file contains classes used to communicate with ActionScript.

// The Command manager/handler subsystem is used to delegate 
// fscommands for custom handling. Command handlers are required to
// register a command map with the command manager to receive the 
// fscommands through the callback method. The FxPlayerApp installs
// an fscommand handler that delegates the fscommand to the command
// manager if it is not handled at that level.

// The SWF path builder is used to create paths that are used 
// to access ActionScript variables and methods. It provides a 
// convenient parent path based approach to avoid unnecessary 
// string copies. 



// Key for const string hash
struct gstring_key
{
    const char *pChar;

    gstring_key(const char *pc)
    {
        pChar = pc;
    }
    bool operator == (const gstring_key& c) const
    {
        return strcmp(pChar, c.pChar) == 0;
    }
    bool operator != (const gstring_key& c) const
    {
        return strcmp(pChar, c.pChar) != 0;
    }
};

// Hash function for const string hash
class gstring_hashfn
{
public:

    size_t  operator()(const gstring_key& data) const
    { 
        size_t  size = strlen(data.pChar);
        return GString::BernsteinHashFunction(data.pChar, size);
    }
};

// Const string hash table
template <class N>
class GConstStringHash : public GHash<gstring_key, N, gstring_hashfn> {};


// ********************************************************************
// Mapping between the command ID (command handler specific code)
// and the command string. A table of these objects is expected
// to be returned by all FxCommandHandlers.
struct FxCommand
{
    int         Key;
    const char* Command;
};


// ********************************************************************
// Command handler interface implemented by objects that require 
// custom handling of specific fscommands
class FxCommandHandler
{
public:

    //  Return the command map (map between command enumeration value
    // and fscommand string). This mapping can be different for 
    // objects derived from the same base class.
    virtual FxCommand* GetCommandMap() = 0;

    // Process any commands from the ActionScript.
    // Returns 1 if command handled, otherwise, 0 to continue 
    // processing.
    virtual bool VFsCallback(class GFxMovieView* pmovie, 
                             int cmdKey, const char* cmdName, const char* args) = 0;
};


// ********************************************************************
// The command manager that diverts incoming fscommands to
// appropriate handlers
class FxCommandManager
{
protected:

    struct CommandNode
    {
        int  Key;
        FxCommandHandler* pClass;

        CommandNode(int k, FxCommandHandler*p)
        {
            Key = k; pClass = p;
        }
    };

    GConstStringHash<CommandNode>  CommandMap;

public:

    // Registers the command handler
    void    Register(FxCommandHandler* phandler);
    void    Unregister(FxCommandHandler* phandler);

    // Process any commands from the ActionScript.
    // returns 1 if command handled, otherwise, 0 to continue processing
    bool    FsCallback(class GFxMovieView* pmovie, const char* command, const char* args);
};


// ********************************************************************
// Helper class that makes the process of creating path
// strings used for Invoke() easier
class FxSwfPathBuilder
{
public:
    FxSwfPathBuilder();
    ~FxSwfPathBuilder();

    // Set the parent path (eg: _root.somePath.someWidget)
    void SetParentPath(const char* ppath);

    // Build a path string using the parent path and the provided 
    // action (eg: _root.somePath.someWidget.someAction). Parent path
    // does not need to be set again to build another path string.
    const char* BuildPath(const char* action);

    // Returns true if there is a parent path
    bool HasParentPath();

private:
    char* BuiltPath;
    size_t MaxPathSize;

    size_t ParentPathLength;

    char* ActionString;
    size_t MaxActionSize;
};

#endif // INC_FxCommand_H
