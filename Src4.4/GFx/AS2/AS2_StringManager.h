/**************************************************************************

PublicHeader:   None
Filename    :   AS2_StringManager.h
Content     :   AS2 string bultins and manager objects
Created     :   November 15, 2006
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_GFx_AS2StringManager_H
#define INC_GFx_AS2StringManager_H

#include "GFx/GFx_ASString.h"

namespace Scaleform { namespace GFx { namespace AS2 {


// **** Classes Declared

class StringManager;
class ASStringContext;

/* ***** String Builtin Notes

Built-ins are strings that can be accessed very efficiently based on
a constant. They are usually obtained from the built-in manager through
an inline function such as

// Returns a string for "undefined"
pManager->GetBuiltin(ASBuiltin_undefined)

Builtins are pre-created ahead of time, so that GetBuiltin() can be
inlined to a single array access. Built-in strings are used heavily
throughout AS so performance of their references and compares is of
vital importance.

This file defines a container StringManager class that holds
all of the allocated built-ins and AS string manager. One instance of
this class is created as a base of GASGlobalContext and shared by all
sprites within a movie views. For thread efficiency reasons, different
movie views use different string managers.

*/


// ***** Global built-in types

enum ASBuiltinType
{
    ASBuiltin_empty_, // ""
    ASBuiltin_Object,
    ASBuiltin_Array,
    ASBuiltin_String,
    ASBuiltin_Number,
    ASBuiltin_Boolean,
    ASBuiltin_MovieClip,
    ASBuiltin_Function,
    ASBuiltin_Sound,
    ASBuiltin_Button,
    ASBuiltin_TextField,
    ASBuiltin_Color,
    ASBuiltin_Transform,
    ASBuiltin_Matrix,
    ASBuiltin_Point,
    ASBuiltin_Rectangle,
    ASBuiltin_ColorTransform,
    ASBuiltin_Capabilities,
    ASBuiltin_Stage,
    ASBuiltin_AsBroadcaster,
    ASBuiltin_Date,
    ASBuiltin_Selection,
    ASBuiltin_Ime,
    ASBuiltin_XML,
    ASBuiltin_XMLNode,
    ASBuiltin_Math,
    ASBuiltin_Key,
    ASBuiltin_Mouse,
    ASBuiltin_ExternalInterface,
    ASBuiltin_MovieClipLoader,
    ASBuiltin_BitmapData,
    ASBuiltin_LoadVars,
    ASBuiltin_TextFormat,
    ASBuiltin_StyleSheet,
    ASBuiltin_NetConnection,
    ASBuiltin_NetStream,
    ASBuiltin_Video,
    ASBuiltin_BitmapFilter,
    ASBuiltin_DropShadowFilter,
    ASBuiltin_GlowFilter,
    ASBuiltin_BlurFilter,
    ASBuiltin_BevelFilter,
    ASBuiltin_ColorMatrixFilter,
    ASBuiltin_TextSnapshot,
    ASBuiltin_SharedObject,
    ASBuiltin_LocManager,
        
    // Necessary runtime keys
    ASBuiltin_unknown_, // "<unknown>"
    ASBuiltin_badtype_, // "<bad type>"
    ASBuiltin_undefined_, // "<undefined>"

    ASBuiltin_dot_,    // "."
    ASBuiltin_dotdot_, // ".."

    ASBuiltin_undefined,
    ASBuiltin_null,
    ASBuiltin_true,
    ASBuiltin_false,
    // Lowercase types for typeof reporting
    ASBuiltin_string,
    ASBuiltin_number,
    ASBuiltin_boolean,
    ASBuiltin_movieclip,
    ASBuiltin_object,
    ASBuiltin_function,

    // Path components
    ASBuiltin_this,    // "this"
    ASBuiltin_super,    // "super"
    ASBuiltin__global, // "_global"
    ASBuiltin__root,   // "_root"
    ASBuiltin__parent,   // "_parent"

    ASBuiltin__level0,     // "_level0"
    ASBuiltin__level0dot_, // "_level0."

    ASBuiltin_arguments,
    ASBuiltin_callee,
    ASBuiltin_caller,

    ASBuiltin_gfxExtensions,
    ASBuiltin_noInvisibleAdvance,
    ASBuiltin_continueAnimation,

    ASBuiltin_NaN,
    ASBuiltin_Infinity,
    ASBuiltin_minusInfinity_, // "-Infinity"

    ASBuiltin_prototype,
    ASBuiltin___proto__, // "__proto__"
    ASBuiltin_constructor,
    ASBuiltin___constructor__, // "__constructor__"

    ASBuiltin__listeners,     // "_listeners"
    ASBuiltin___resolve,      // "__resolve"

    ASBuiltin_typeFunction_, // "[type Function]"
    ASBuiltin_typeObject_,   // "[type Object]"
    ASBuiltin_objectObject_, // "[object Object]"

    // Common methods
    ASBuiltin_toString,
    ASBuiltin_valueOf,

    ASBuiltin_onSetFocus,
    ASBuiltin_onKillFocus,

    // Event name strings
    ASBuiltin_INVALID,             // Event_Invalid "INVALID"
    ASBuiltin_onPress,             // Event_Press
    ASBuiltin_onRelease,           // Event_Release
    ASBuiltin_onReleaseOutside,    // Event_ReleaseOutside
    ASBuiltin_onRollOver,          // Event_RollOver
    ASBuiltin_onRollOut,           // Event_RollOut
    ASBuiltin_onDragOver,          // Event_DragOver
    ASBuiltin_onDragOut,           // Event_DragOut
    ASBuiltin_akeyPressa_,          // Event_KeyPress  "@keyPress@"
    ASBuiltin_ainitializea_,        // Event_Initialize "@initialize@"
    ASBuiltin_onLoad,              // Event_Load
    ASBuiltin_onUnload,            // Event_Unload
    ASBuiltin_onEnterFrame,        // Event_EnterFrame
    ASBuiltin_onMouseDown,         // Event_MouseDown
    ASBuiltin_onMouseUp,           // Event_MouseUp
    ASBuiltin_onMouseMove,         // Event_MouseMove
    ASBuiltin_onMouseWheel,        // Mouse listener's onMouseWheel
    ASBuiltin_onKeyDown,           // Event_KeyDown
    ASBuiltin_onKeyUp,             // Event_KeyUp
    ASBuiltin_onData,              // Event_Data
    ASBuiltin_onConstruct,         // Event_Construct
    // These are for the MoveClipLoader ActionScript only
    ASBuiltin_onLoadStart,         // Event_LoadStart
    ASBuiltin_onLoadError,         // Event_LoadError
    ASBuiltin_onLoadProgress,      // Event_LoadProgress
    ASBuiltin_onLoadInit,          // Event_LoadInit
    // These are for the XMLSocket ActionScript only
    ASBuiltin_onSockClose,         // Event_SockClose
    ASBuiltin_onSockConnect,       // Event_SockConnect
    ASBuiltin_onSockData,          // Event_SockData
    ASBuiltin_onSockXML,           // Event_SockXML
    // These are for the XML ActionScript only
    ASBuiltin_onXMLLoad,           // Event_XMLLoad
    ASBuiltin_onXMLData,            // Event_XMLData

    ASBuiltin_onPressAux,             // Event_PressAux
    ASBuiltin_onReleaseAux,           // Event_ReleaseAux
    ASBuiltin_onReleaseOutsideAux,    // Event_ReleaseOutsideAux
    ASBuiltin_onDragOverAux,          // Event_DragOverAux
    ASBuiltin_onDragOutAux,           // Event_DragOutAux

    // Common members
    ASBuiltin_width,
    ASBuiltin_height,
    ASBuiltin_useHandCursor,

    // Sprite members - for efficiency    
    ASBuiltin_x,
    ASBuiltin_y,
    ASBuiltin_xMin,
    ASBuiltin_xMax,
    ASBuiltin_yMin,
    ASBuiltin_yMax,
    ASBuiltin_hitArea,

    // Mouse class extensions
    ASBuiltin_setCursorType,
    ASBuiltin_LEFT,
    ASBuiltin_RIGHT,
    ASBuiltin_MIDDLE,
    ASBuiltin_ARROW,
    ASBuiltin_HAND,
    ASBuiltin_IBEAM,

    // Numbers are used heavily during argument pushing in SWF 6
    // and Array visits elsewhere. So pre-allocate low numbers
    // to prevent them ending up in allocator.
    // This range must be continuous.
    ASBuiltin_0,
    ASBuiltin_1,
    ASBuiltin_2,
    ASBuiltin_3,
    ASBuiltin_4,
    ASBuiltin_5,
    ASBuiltin_6,
    ASBuiltin_7,

    // Movieclip user data properties
    ASBuiltin_rendererString,
    ASBuiltin_rendererFloat,
    ASBuiltin_disableBatching,
    //ASBuiltin_rendererMatrix,

    // AMP
    ASBuiltin_Amp,

    // Put length as last so that we can have checks for it.
    ASBuiltin_length,
    ASBuiltinConst_Count_,
    // Number of numeric built-ins
    ASBuiltinConst_DigitMax_ = ASBuiltin_7 - ASBuiltin_0
};


// ***** StringManager class

// class StringManagerBase 
// {
// protected:
//     ASStringManager mStringManager;    
//     StringManagerBase(MemoryHeap* pheap) : mStringManager(pheap) { }
// };

// StringManager relies on the base template class for most of implementation.
// StringManager is separated into the base class to ensure that it is created first.

class StringManager : public ASStringBuiltinManagerT<ASBuiltinType, ASBuiltinConst_Count_>
{    
    typedef ASStringBuiltinManagerT<ASBuiltinType, ASBuiltinConst_Count_> Base;
public:
    StringManager(ASStringManager* pstrMgr);
    ~StringManager() { }
};

}}} // namespace Scaleform::GFx::AS2

#endif
