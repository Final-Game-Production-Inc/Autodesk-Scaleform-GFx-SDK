/**************************************************************************

Filename    :   AS3_StringManager.cpp
Content     :   Implementation of AS StringManager and builtins.
Created     :   March, 2010
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "AS3_StringManager.h"

namespace Scaleform { namespace GFx { namespace AS3 {

// ***** StringManager

// Build in string table
static const char* AS3BuiltinTable[] = 
{
    // ***** AS3 Core/TopLevel Strings
    "",
    "undefined",
    "null",
    "void",
    "true",
    "false",
    // Types for typeof operator.
    "boolean",
    "number",
    "string",
    "function",
    "object",
    "xml",
    "unknown",
    "hitArea",

    ",",
    "0",
    "NaN",
    "Infinity",
    "-Infinity",

    "[object ",
    "]",

    "namespace",
    "global",
    "length",
    "toString",
    "valueOf",
    "Class",
    "Date",
    
    // *** Flash Strings
    // Events
    "activate",
    "added",
    "addedToStage",
    "removed",
    "removedFromStage",
    "deactivate",
    "enterFrame",
    "frameConstructed",
    "exitFrame",
    "timer",
    "timerComplete",
    "complete",
    "progress",
    "socketData",
#ifdef SF_AS3_VERSION_AIR
    "outputProgress",
#endif
    "open",
    "init",
    "render",
	"resize",
    "mouseCursorChange",

    "click",
    "doubleClick",
    "mouseDown",
    "mouseMove",
    "mouseOut",
    "mouseOver",
    "mouseUp",
    "mouseWheel",
    "rollOut",
    "rollOver",
    "keyDown",
    "keyUp",

#ifdef GFX_MULTITOUCH_SUPPORT_ENABLE
    "touchBegin",
    "touchEnd",
    "touchMove",
    "touchTap",
    "touchOver",
    "touchOut",
    "touchRollOver",
    "touchRollOut",
#endif

#ifdef GFX_ENABLE_ANALOG_GAMEPAD
    "gamePadAnalogChange",
#endif

    "update",
	"status",

    "textInput",
    "change",
    "scroll"
};

StringManager::StringManager(ASStringManager* pstrMgr)
: StringManager::Base(pstrMgr, AS3BuiltinTable)
{
    SF_COMPILER_ASSERT((sizeof(AS3BuiltinTable)/sizeof(AS3BuiltinTable[0])) == AS3BuiltinConst_Count_);

    // If this hits, there is some kind of ordering issue.
    SF_ASSERT(GetBuiltin(AS3Builtin_activate) == "activate");
    SF_ASSERT(GetBuiltin(AS3Builtin_mouseOver) == "mouseOver");
    SF_ASSERT(GetBuiltin(AS3Builtin_enterFrame) == "enterFrame");
    SF_ASSERT(GetBuiltin(AS3Builtin_rollOver) == "rollOver");
}

}}} // namespace Scaleform::GFx::AS3
