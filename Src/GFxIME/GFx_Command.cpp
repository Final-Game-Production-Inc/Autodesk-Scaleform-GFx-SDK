/**************************************************************************

Filename    :   GFx_Command.cpp
Content     :   Interface to the game engine from GFx via fscommands
Created     :   11/9/2006
Authors     :   Brendan Iribe, Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Kernel/SF_Debug.h"
#include "GFx_Command.h"

namespace Scaleform {
	namespace GFx {
		namespace IME{


bool    FxCommandManager::FsCallback(class Movie* pmovie, const char* command, const char* args)
{
    if (!pmovie)
        return 0;
    ConstStringHash<CommandNode>::Iterator ic = CommandMap.Find(command);
    if (ic != CommandMap.End())
        return ic->Second.pClass->VFsCallback(pmovie, ic->Second.Key, command, args);
    return 0;
}

void    FxCommandManager::Register(FxCommandHandler* phandler)
{
    const FxCommand* pc = phandler->GetCommandMap();
    while (pc->Command)
    {
        SF_DEBUG_ERROR1(CommandMap.Find(pc->Command) != CommandMap.End(),
            "GFxCommandManager::RegisterHandler - Command %s already registered.", pc->Command);
        CommandMap.Add(pc->Command, CommandNode(pc->Key, phandler) );
        pc++;
    }
}

void    FxCommandManager::Unregister(FxCommandHandler* phandler)
{
    const FxCommand* pc = phandler->GetCommandMap();
    while (pc->Command)
    {
        SF_DEBUG_ERROR1(CommandMap.Find(pc->Command) == CommandMap.End(),
            "GFxCommandManager::UnregisterHandler - Command %s is not registered.", pc->Command);
        CommandMap.Remove(pc->Command);
        pc++;
    }
}

}}}

