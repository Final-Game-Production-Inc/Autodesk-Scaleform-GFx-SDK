/**************************************************************************

Filename    :   AS2_StringManager.cpp
Content     :   ActionScript string bultin manager implementation
Created     :   November 15, 2006
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "AS2_StringManager.h"

namespace Scaleform { namespace GFx { namespace AS2 {

// Build in string table
static const char* GFx_pASBuiltinTable[] = 
{
    "",
    "Object",
    "Array",
    "String",
    "Number",
    "Boolean",
    "MovieClip",
    "Function",
    "Sound",
    "Button",
    "TextField",
    "Color",
    "Transform",
    "Matrix",
    "Point",
    "Rectangle",
    "ColorTransform",
    "capabilities",
    "Stage",
    "AsBroadcaster",
    "Date",
    "Selection",
    "IME",
    "XML",
    "XMLNode",
    "Math",
    "Key",
    "Mouse",
    "ExternalInterface",
    "MovieClipLoader",
    "BitmapData",
    "LoadVars",
    "TextFormat",
    "StyleSheet",
    "NetConnection",
    "NetStream",
    "Video",
    "BitmapFilter",
    "DropShadowFilter",
    "GlowFilter",
    "BlurFilter",
    "BevelFilter",
    "ColorMatrixFilter",
    "TextSnapshot",
    "SharedObject",
    "LocManager",

    // Necessary runtime keys
    "<unknown>",
    "<bad type>",
    "<undefined>",
    ".",
    "..",
    "undefined",
    "null",
    "true",
    "false",    
    // Lowercase types for typeof reporting
    "string",
    "number",
    "boolean",
    "movieclip",
    "object",
    "function",
    // Path components
    "this",
    "super",
    "_global",
    "_root",
    "_parent",
    "_level0",
    "_level0.",
    "arguments",
    "callee",
    "caller",

    "gfxExtensions",
    "noInvisibleAdvance",
    "continueAnimation",

    // Numbers
    "NaN",
    "Infinity",
    "-Infinity", // "-Infinity"

    "prototype",
    "__proto__", // "__proto__"
    "constructor",
    "__constructor__", // "__constructor__"
    "_listeners",     // "_listeners"
    "__resolve",      // "__resolve"
    "[type Function]",
    "[type Object]",
    "[object Object]",
    // Common methods
    "toString",
    "valueOf",
    "onSetFocus",
    "onKillFocus",

    // Event name strings
    "INVALID",             // Event_Invalid "INVALID"
    "onPress",             // Event_Press
    "onRelease",           // Event_Release
    "onReleaseOutside",    // Event_ReleaseOutside
    "onRollOver",          // Event_RollOver
    "onRollOut",           // Event_RollOut
    "onDragOver",          // Event_DragOver
    "onDragOut",           // Event_DragOut
    "@keyPress@",          // Event_KeyPress  "@keyPress@"
    "@initialize@",        // Event_Initialize "@initialize@"
    "onLoad",              // Event_Load
    "onUnload",            // Event_Unload
    "onEnterFrame",        // Event_EnterFrame
    "onMouseDown",         // Event_MouseDown
    "onMouseUp",           // Event_MouseUp
    "onMouseMove",         // Event_MouseMove
    "onMouseWheel",
    "onKeyDown",           // Event_KeyDown
    "onKeyUp",             // Event_KeyUp
    "onData",              // Event_Data
    "onConstruct",         // Event_Construct
    // These are for the MoveClipLoader ActionScript only
    "onLoadStart",         // Event_LoadStart
    "onLoadError",         // Event_LoadError
    "onLoadProgress",      // Event_LoadProgress
    "onLoadInit",          // Event_LoadInit
    // These are for the XMLSocket ActionScript only
    "onSockClose",         // Event_SockClose
    "onSockConnect",       // Event_SockConnect
    "onSockData",          // Event_SockData
    "onSockXML",           // Event_SockXML
    // These are for the XML ActionScript only
    "onXMLLoad",           // Event_XMLLoad
    "onXMLData",           // Event_XMLData

    "onPressAux",             // Event_PressAux
    "onReleaseAux",           // Event_ReleaseAux
    "onReleaseOutsideAux",    // Event_ReleaseOutsideAux
    "onDragOverAux",          // Event_DragOverAux
    "onDragOutAux",           // Event_DragOutAux

    // Common members
    "width",
    "height",
    "useHandCursor",

    // Sprite members - for efficiency    
    "x",
    "y",
    "xMin",
    "xMax",
    "yMin",
    "yMax",
    "hitArea",

    // Mouse class extensions
    "setCursorType",
    "LEFT",
    "RIGHT",
    "MIDDLE",
    "ARROW",
    "HAND",
    "IBEAM",

    // Numbers
    "0",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",

    // Movieclip user data properties
    "rendererString",
    "rendererFloat",
    "disableBatching",
    //"rendererMatrix",

    // AMP
    "Amp",

    // Put length as last so that we can have checks for it.
    "length"
};


// ***** StringManager

StringManager::StringManager(ASStringManager* pstrMgr)
: StringManager::Base(pstrMgr, GFx_pASBuiltinTable)
{
    SF_COMPILER_ASSERT((sizeof(GFx_pASBuiltinTable)/sizeof(const char*)) 
                        == ASBuiltinConst_Count_);
    SF_COMPILER_ASSERT(ASBuiltin_7 == (ASBuiltinType)(ASBuiltin_0 + 7));    

    // If this hits, there is some kind of ordering issue.
    SF_ASSERT(GetBuiltin(ASBuiltin_unknown_) == "<unknown>");
    SF_ASSERT(GetBuiltin(ASBuiltin_prototype) == "prototype");
    SF_ASSERT(GetBuiltin(ASBuiltin_onXMLData) == "onXMLData");
    SF_ASSERT(GetBuiltin(ASBuiltin_length) == "length");
}

}}} // namespace Scaleform::GFx::AS2

