/**************************************************************************

Filename    :   AS2_Action.cpp
Content     :   ActionScript 1.0 and 2.0 opcode execution engine core
Created     :   
Authors     :   Artem Bolgar, Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "Kernel/SF_Alg.h"

#include "GFx/AS2/AS2_Action.h"
#include "GFx/AS2/AS2_AvmCharacter.h"
#include "GFx/AS2/AS2_MovieRoot.h"
#include "GFx/AS2/AS2_AvmButton.h"
#include "GFx/GFx_LoaderImpl.h"
#include "GFx/GFx_Log.h"
#include "GFx/GFx_Stream.h"
#include "Kernel/SF_Random.h"

#include "GFx/GFx_PlayerImpl.h"
#include "GFx/AS2/AS2_AsFunctionObject.h"

#include "GFx/AS2/AS2_AvmSprite.h"
#include "GFx/AS2/AS2_AvmTextField.h"
#include "GFx/AS2/AS2_Color.h"
#include "GFx/AS2/AS2_TransformObject.h"
#include "GFx/AS2/AS2_MatrixObject.h"
#include "GFx/AS2/AS2_MovieClipLoader.h"
#include "GFx/AS2/AS2_BitmapData.h"
#include "GFx/AS2/AS2_PointObject.h"
#include "GFx/AS2/AS2_RectangleObject.h"
#include "GFx/AS2/AS2_ColorTransform.h"
#include "GFx/AS2/AS2_Stage.h"

#include "GFx/GFx_ASString.h"
#include "GFx/AS2/AS2_StringObject.h"

#include "GFx/AS2/AS2_ArrayObject.h"
#include "GFx/AS2/AS2_NumberObject.h"
#include "GFx/AS2/AS2_BooleanObject.h"
#include "GFx/AS2/AS2_Math.h"
#include "GFx/AS2/AS2_Timers.h"
#include "GFx/AS2/AS2_AsBroadcaster.h"
#include "GFx/AS2/AS2_Date.h"
#include "GFx/AS2/AS2_KeyObject.h"
#include "GFx/AS2/AS2_Mouse.h"
#include "GFx/AS2/AS2_ExternalInterface.h"
#include "GFx/AS2/AS2_Selection.h"
#include "GFx/AS2/AS2_TextFormat.h"
#include "GFx/AS2/AS2_SharedObject.h"
#include "GFx/AS2/AS2_AmpMarker.h"

#include "GFx/AS2/AS2_TextSnapshot.h"

#include "IME/GASIme.h"
#include "GFx/GFx_LoadProcess.h"
#include "GFx/IME/GFx_IMEManager.h"

#include "GFx/GFx_FontResource.h"

#include "GFx/AS2/AS2_Capabilities.h"
#include "GFx/AS2/AS2_LoadVars.h"

#ifdef GFX_ENABLE_XML
#include "GFx/XML/XML_Support.h"
#endif

#include "GFx/AS2/AS2_BitmapFilter.h"
#include "GFx/AS2/AS2_DropShadowFilter.h"
#include "GFx/AS2/AS2_GlowFilter.h"
#include "GFx/AS2/AS2_BlurFilter.h"
#include "GFx/AS2/AS2_BevelFilter.h"
#include "GFx/AS2/AS2_ColorMatrixFilter.h"

#include "GFx/GFx_Audio.h"
//#include "GSoundRenderer.h"

#include "Kernel/SF_MsgFormat.h"
#include "GFx/AS2/AS2_FunctionRefImpl.h"

#include "GFx/GFx_ASUtils.h"
#include <stdio.h>
#include <stdlib.h>

#include "GFx/AMP/Amp_Server.h"

#ifdef SF_OS_WIN32
#define snprintf _snprintf
#endif // SF_OS_WIN32

#ifdef SF_OS_WII
#pragma opt_usedef_mem_limit 250
#endif

namespace Scaleform {
namespace GFx {
namespace AS2 {

// NOTES:
//
// Buttons
// On (press)                 onPress
// On (release)               onRelease
// On (releaseOutside)        onReleaseOutside
// On (rollOver)              onRollOver
// On (rollOut)               onRollOut
// On (dragOver)              onDragOver
// On (dragOut)               onDragOut
// On (keyPress"...")         onKeyDown, onKeyUp      <----- IMPORTANT
//
// Sprites
// OnClipEvent (load)         onLoad
// OnClipEvent (unload)       onUnload                Hm.
// OnClipEvent (enterFrame)   onEnterFrame
// OnClipEvent (mouseDown)    onMouseDown
// OnClipEvent (mouseUp)      onMouseUp
// OnClipEvent (mouseMove)    onMouseMove
// OnClipEvent (keyDown)      onKeyDown
// OnClipEvent (keyUp)        onKeyUp
// OnClipEvent (data)         onData

// Text fields have event handlers too!

///////////////////////////////////////////
// ActionLogger
//
ActionLogger::ActionLogger(DisplayObject *ptarget, const char* suffixStr)
{
    MovieImpl *proot = ptarget->FindMovieImpl();
    VerboseAction = proot->IsVerboseAction();
    VerboseActionErrors = !proot->IsSuppressActionErrors();
    LogSuffix = suffixStr;

    // Check if it is the main movie
    if (!LogSuffix)
        UseSuffix = false;
    else
    {
        if (SFstrcmp(proot->GetMovieDef()->GetFileURL(), LogSuffix) == 0)
            UseSuffix = proot->IsLogRootFilenames();
        else
            UseSuffix = proot->IsLogChildFilenames();
    }

    pLog = proot->GetCachedLog();
    if(UseSuffix)
    {
        //find short filename
        if (!proot->IsLogLongFilenames())
        {
            for (SPInt i = (SPInt)SFstrlen(suffixStr) ; i>0; i--) 
            {
                if (LogSuffix[i]=='/' || LogSuffix[i]=='\\') 
                {
                    LogSuffix = LogSuffix+i+1;
                    break;
                }
            }
        }
    }
}

SF_INLINE void ActionLogger::LogScriptMessageVarg(LogMessageId messageType, const char* pfmt, va_list argList)
{
    if (!pLog)
        return;
    if (UseSuffix)
    { 
        char    fmtBuffer[256];
        UPInt   len = strlen(pfmt);

        Format(
            StringDataPtr(fmtBuffer, sizeof(fmtBuffer)),
            "{0} : {1}\n",
            StringDataPtr(pfmt, (pfmt[len - 1] == '\n' ? len - 1 : len)),
            LogSuffix
            );

        pLog->LogMessageVarg(messageType,fmtBuffer,argList);
        //pLog->LogScriptError(" : %s\n",LogSuffix);
    }
    else
        pLog->LogMessageVarg(messageType,pfmt,argList);
}

void    ActionLogger::LogScriptError(const char* pfmt,...)
{   
    va_list argList; va_start(argList, pfmt);
    LogScriptMessageVarg(Log_ScriptError, pfmt, argList);
    va_end(argList);
}

void    ActionLogger::LogScriptWarning(const char* pfmt,...)
{    
    va_list argList; va_start(argList, pfmt);
    LogScriptMessageVarg(Log_ScriptWarning, pfmt, argList);
    va_end(argList);
}

void    ActionLogger::LogScriptMessage(const char* pfmt,...)
{  
    va_list argList; va_start(argList, pfmt);
    LogScriptMessageVarg(Log_ScriptMessage, pfmt, argList);
    va_end(argList);
}

SF_INLINE void    ActionLogger::LogDisasm(const unsigned char* instructionData)
{
#ifdef GFX_AS2_VERBOSE
    if (pLog && IsVerboseAction())
    {
        Disasm da(pLog, Log_Action);
        da.LogDisasm(instructionData);
    }
#else
    SF_UNUSED(instructionData);
#endif
}
//
// Function/method dispatch.
//

// FirstArgBottomIndex is the stack index, from the bottom, of the first argument.
// Subsequent arguments are at *lower* indices.  E.G. if FirstArgBottomIndex = 7,
// then arg1 is at env->Bottom(7), arg2 is at env->Bottom(6), etc.
bool        GAS_Invoke(const Value& method,
                       Value* presult,
                       ObjectInterface* pthis,
                       Environment* penv,
                       int nargs,
                       int firstArgBottomIndex,
                       const char* pmethodName)
{
    FunctionRef  func = method.ToFunction(penv);
    if (presult) presult->SetUndefined();
    if (func != NULL)
    {
        func.Function->Invoke(FnCall(presult, pthis, penv, nargs, firstArgBottomIndex), func.pLocalFrame, pmethodName);
        //func.Invoke(FnCall(presult, pthis, penv, nargs, firstArgBottomIndex), pmethodName);
        return true;
    }
    else
    {           
        #ifdef GFX_AS2_VERBOSE_ERRORS
        if (penv && penv->IsVerboseActionErrors())
        {
            if (pthis && pthis->IsASCharacter())
                penv->LogScriptError("Invoked method %s.%s is not a function",
                    pthis->ToCharacter()->GetCharacterHandle()->GetNamePath().ToCStr(),
                    (pmethodName ? pmethodName : "<unknown>"));
            else
                penv->LogScriptError("Invoked method %s is not a function",
                (pmethodName ? pmethodName : "<unknown>"));
        }
        #endif // #ifdef GFX_AS2_VERBOSE_ERRORS
    }

    return false;
}

bool        GAS_Invoke(const Value& method,
                       Value* presult,
                       const Value& pthis,
                       Environment* penv,
                       int nargs,
                       int firstArgBottomIndex,
                       const char* pmethodName)
{
    FunctionRef  func = method.ToFunction(penv);
    if (presult) presult->SetUndefined();
    if (func != NULL)
    {
        func.Invoke(FnCall(presult, pthis, penv, nargs, firstArgBottomIndex), pmethodName);
        return true;
    }
    else
    {           
        #ifdef GFX_AS2_VERBOSE_ERRORS
        if (penv && penv->IsVerboseActionErrors())
        {
            if (pthis.ToCharacter(penv))
                penv->LogScriptError("Invoked method %s.%s is not a function",
                    pthis.ToCharacter(penv)->GetCharacterHandle()->GetNamePath().ToCStr(),
                    (pmethodName ? pmethodName : "<unknown>"));
            else
                penv->LogScriptError("Invoked method %s is not a function",
                    (pmethodName ? pmethodName : "<unknown>"));
        }
        #endif // #ifdef GFX_AS2_VERBOSE_ERRORS
    }

    return false;
}

// Printf-like vararg interface for calling ActionScript.
// Handy for external binding.
bool        GAS_InvokeParsed(const char* pmethodName,
                             Value* presult,
                             ObjectInterface* pthis,
                             Environment* penv,
                             const char* pmethodArgFmt,
                             va_list args)
{
    if (!pmethodName || *pmethodName == '\0')
        return false;

    InteractiveObject* pnewTarget  = 0;
    Value        method, owner;
    if (!penv->GetVariable(penv->CreateString(pmethodName), &method, NULL, &pnewTarget, &owner))
    {
        #ifdef GFX_AS2_VERBOSE_ERRORS
        if (pthis && pthis->IsASCharacter())
            penv->LogScriptError("Can't find method '%s.%s' to invoke.", 
                pthis->ToCharacter()->GetCharacterHandle()->GetNamePath().ToCStr(),
                pmethodName);
        else
            penv->LogScriptError("Can't find method '%s' to invoke.", pmethodName);
        #endif // #ifdef GFX_AS2_VERBOSE_ERRORS
        return false;
    }

    // Check method
    FunctionRef  func = method.ToFunction(penv);
    if (func == NULL)
    {           
        #ifdef GFX_AS2_VERBOSE_ERRORS
        if (pthis && pthis->IsASCharacter())
            penv->LogScriptError("Invoked method '%s.%s' is not a function", 
                pthis->ToCharacter()->GetCharacterHandle()->GetNamePath().ToCStr(),
                pmethodName);
        else
            penv->LogScriptError("Invoked method '%s' is not a function", pmethodName);
        #endif // #ifdef GFX_AS2_VERBOSE_ERRORS
        return false;
    }

    // MA: For C functions we need to specify a new target pthis here, if mehodName was nested.
    if (owner.IsCharacter() || owner.IsObject())
        pthis = owner.ToObjectInterface(penv);
    else if (pnewTarget)
        pthis = ToAvmCharacter(pnewTarget);
    return GAS_InvokeParsed(method, presult, pthis, penv, pmethodArgFmt, args, pmethodName);
}

bool        GAS_InvokeParsed(const Value& method,
                             Value* presult,
                             ObjectInterface* pthis,
                             Environment* penv,
                             const char* pmethodArgFmt,
                             va_list args,
                             const char* pmethodName)
{
    // Parse vaList args
    int  nargs = 0;
    if (pmethodArgFmt)
    {
        int startingIndex = penv->GetTopIndex();
        const char* p = pmethodArgFmt;
        for (;;)
        {
            char    c = *p++;
            if (c == 0)
            {
                // End of args.
                break;
            }
            else if (c == '%')
            {
                c = *p++;
                // Here's an arg.
                if (c == 'd')
                {
                    // Integer.
                    penv->Push(va_arg(args, int));
                }
                else if (c == 'u')
                {
                    // Undefined.
                    penv->Push(Value());
                }
                else if (c == 'n')
                {
                    // Null.
                    penv->Push(Value::NULLTYPE);
                }
                else if (c == 'b')
                {
                    // Boolean
                    penv->Push(bool(va_arg(args, int) != 0));
                }
                else if (c == 'f')
                {
                    // Double                   
                    penv->Push((Number)va_arg(args, double));

                    // MA: What about float? C specification states that "float converts to
                    // double by the standard argument promotion", so this works. But,
                    // what happens on PS2?  Do we needs '%hf' specifier for float?
                }
                else if (c == 'h')
                {
                    c = *p++;
                    if (c == 'f')
                    {
                        // AB: %hf will be treated as double too, since C spec states
                        // that float is converted to double.
                        // Double                   
                        penv->Push((Number)va_arg(args, double));
                        SF_DEBUG_WARNING2(1, "InvokeParsed('%s','%s') - '%%hf' will be treated as double", 
                            pmethodName,
                            pmethodArgFmt);
                    }
                    else
                    {
                        penv->LogScriptError("InvokeParsed('%s','%s') - invalid format '%%h%c'",
                            pmethodName,
                            pmethodArgFmt,
                            c);
                    }
                }
                else if (c == 's')
                {
                    // String                
                    penv->Push(penv->CreateString(va_arg(args, const char *)));
                }
                else if (c == 'l')
                {
                    c = *p++;
                    if (c == 's')
                    {
                        // Wide string.
                        penv->Push(penv->CreateString(va_arg(args, const wchar_t *)));
                    }
                    else
                    {
                        penv->LogScriptError("InvokeParsed('%s','%s') - invalid format '%%l%c'",
                                             pmethodName, pmethodArgFmt, c);
                    }
                }
                else
                {
                    // Invalid fmt, warn.
                    penv->LogScriptError("InvokeParsed('%s','%s') - invalid format '%%%c'",
                                         pmethodName, pmethodArgFmt, c);
                }
            }
            else
            {
                // Invalid arg; warn.
                penv->LogScriptError("InvokeParsed('%s','%s') - invalid char '%c'",
                                     pmethodName, pmethodArgFmt, c);
            }
            // skip all whitespaces
            for (c = *p; c != '\0' && (c == ' ' || c == '\t' || c == ','); c = *++p) 
                ;
        }
        // Reverse the order of pushed args
        nargs  = penv->GetTopIndex() - startingIndex;
        for (int i = 0; i < (nargs >> 1); i++)
        {
            int i0 = startingIndex + 1 + i;
            int i1 = startingIndex + nargs - i;
            SF_ASSERT(i0 < i1);

            Alg::Swap((penv->Bottom(i0)), (penv->Bottom(i1)));
        }
    }

    // Do the call.
    bool retVal = GAS_Invoke(method, presult, pthis, penv, nargs, penv->GetTopIndex(), pmethodName);
    penv->Drop(nargs);

    return retVal;
}

// arguments should be ALREADY pushed into the penv's stack!
bool GAS_Invoke(const char* pmethodName,
                Value* presult,
                ObjectInterface* pthis,
                Environment* penv,
                int numArgs,
                int firstArgBottomIndex)
{
    if (!pmethodName || *pmethodName == '\0')
        return false;

    InteractiveObject* pnewTarget  = 0;
    Value        method, owner;
    if (!penv->GetVariable(penv->CreateString(pmethodName), &method, NULL, &pnewTarget, &owner))
    {
        #ifdef GFX_AS2_VERBOSE_ERRORS
        if (pthis && pthis->IsASCharacter())
            penv->LogScriptError("Can't find method '%s.%s' to invoke.",
                pthis->ToCharacter()->GetCharacterHandle()->GetNamePath().ToCStr(),
                pmethodName);
        else
            penv->LogScriptError("Can't find method '%s' to invoke.", pmethodName);
        #endif // #ifdef GFX_AS2_VERBOSE_ERRORS
        return false;
    }

    // Check method
    FunctionRef  func = method.ToFunction(penv);
    if (func == NULL)
    {           
        #ifdef GFX_AS2_VERBOSE_ERRORS
        if (pthis && pthis->IsASCharacter())
            penv->LogScriptError("Invoked method '%s.%s' is not a function", 
                pthis->ToCharacter()->GetCharacterHandle()->GetNamePath().ToCStr(),
                pmethodName);
        else
            penv->LogScriptError("Invoked method '%s' is not a function", pmethodName);
        #endif // #ifdef GFX_AS2_VERBOSE_ERRORS
        return false;
    }

    // Do the call.
    // MA: For C functions we need to specify a new target pthis here, if mehodName was nested.
    if (owner.IsCharacter() || owner.IsObject())
        pthis = owner.ToObjectInterface(penv);
    else if (pnewTarget)
        pthis = ToAvmCharacter(pnewTarget);
    return GAS_Invoke(method, presult, pthis, penv, numArgs, firstArgBottomIndex, pmethodName);
}

//
// Built-in objects
//

//
// global init
//
static void    GAS_GlobalTrace(const FnCall& fn)
{
    SF_ASSERT(fn.NArgs >= 1);

    // Special case for objects: try the ToString(env) method.
    ObjectInterface* piobj = fn.Arg(0).ToObjectInterface(fn.Env);
    if (piobj) // OBJECT or CHARACTER
    {
        Value method;
        if (piobj->GetMemberRaw(fn.Env->GetSC(),
                                fn.Env->GetBuiltin(ASBuiltin_toString), &method) &&
            method.IsFunction())
        {
            Value result;
            GAS_Invoke0(method, &result, piobj, fn.Env);
            
            // Should be just a message
            ASString traceStr(result.ToString(fn.Env));            
            fn.LogScriptMessage("%s\n", traceStr.ToCStr());
            return;
        }
    }

    // LogF our argument.
    //
    // @@ what if we get extra args?
    ASString arg0 = fn.Arg(0).ToString(fn.Env);
    char    buffStr[2000];
    size_t  szToCopy = (arg0.GetSize() < sizeof(buffStr)) ? arg0.GetSize() : sizeof(buffStr) - 1;

    // replace '\r' by '\n'
    SFstrncpy(buffStr, sizeof(buffStr), arg0.ToCStr(), szToCopy);
    buffStr[szToCopy] = '\0';
    for (char* p = buffStr; *p != 0; ++p)
    {
        if (*p == '\r')
            *p = '\n';
    }
    if (arg0.GetSize() < sizeof(buffStr))
        fn.LogScriptMessage("%s\n", buffStr);
    else
        fn.LogScriptMessage("%s ...<truncated>\n", buffStr);
}

static void    GAS_GlobalParseInt(const FnCall& fn)
{       
    if (fn.NArgs < 1)
    {
        // return undefined
        return;
    }

    int      radix = 10;
    ASString str(fn.Arg(0).ToString(fn.Env));
    unsigned len = str.GetSize();
    unsigned offset = 0;

    if (fn.NArgs >= 2)
    {
        radix = fn.Arg(1).ToInt32(fn.Env);
        if ((radix < 2) || (radix > 36))
        {
            // Should be NaN!!
            fn.Result->SetNumber(NumberUtil::NaN());
            return;
        }
    }
    else
    {
        // Need to skip whitespaces; but flash doesn't for HEX!
        //while ((str[offset] == ' ') && (offset<len))
        //  offset++;

        if (len > 1)
        {
            if (str[offset] == '0')
            {
                offset++;
                radix = 8;
                if (len > offset)
                {
                    if ((str[offset] == 'x') || (str[offset] == 'X'))
                    {
                        radix = 16;
                        offset++;
                    }
                }
            }
        }
    }
    
    // Convert to number.
    const char* pstart = str.ToCStr() + offset;
    char *      pstop  = 0;    
    long        result = strtol(str.ToCStr() + offset, &pstop, radix);

    // If text was not parsed or empty, return NaN.
    // If only "0x" is parsed - return NaN
    // If something like "0abc" or "0.2333" (looks like radix is 8) was parsed - return 0;
    if (pstart != pstop || radix == 8)
        fn.Result->SetInt(result);
    else
        fn.Result->SetNumber(NumberUtil::NaN());
}

static void    GAS_GlobalParseFloat(const FnCall& fn)
{       
    if (fn.NArgs < 1) // return undefined
        return;     
    // Convert.    
    ASString sa0(fn.Arg(0).ToString(fn.Env));
    const char* pstart = sa0.ToCStr();
    char *      pstop  = 0;
    Number   result = (Number)SFstrtod(sa0.ToCStr(), &pstop);
    // If digits were consumed, report result.
    fn.Result->SetNumber((pstart != pstop) ? result : NumberUtil::NaN());
}

static void    GAS_GlobalIfFrameLoaded(const FnCall& fn)
{    
    if (fn.NArgs < 1) // return undefined
        return;     

    // Mark frame as not loaded initially.
    fn.Result->SetBool(false);

    Sprite* psprite = NULL;
    if (fn.ThisPtr == NULL)    
        psprite = (Sprite*) fn.Env->GetTarget();    
    else if (fn.ThisPtr->IsSprite())        
        psprite = (Sprite*) fn.ThisPtr;

    if (psprite)
    {
        // If frame is within range, mark it as loaded.
        int frame = fn.Arg(0).ToInt32(fn.Env);
        if (frame < (int)psprite->GetLoadingFrame())
            fn.Result->SetBool(true);
    }
}


static void    GAS_GlobalIsNaN(const FnCall& fn)
{       
    // isNaN() with no arguments should return true.
    if (fn.NArgs < 1)
        fn.Result->SetBool(true);
    else
        fn.Result->SetBool(NumberUtil::IsNaN(fn.Arg(0).ToNumber(fn.Env)));   
}

static void    GAS_GlobalIsFinite(const FnCall& fn)
{       
    // isFinite() with no arguments should return false.
    if (fn.NArgs < 1)
        fn.Result->SetBool(false);
    else    
    {
        Number val = fn.Arg(0).ToNumber(fn.Env);

        if (NumberUtil::IsNaN(val) ||
            NumberUtil::IsNEGATIVE_INFINITY(val) ||
            NumberUtil::IsPOSITIVE_INFINITY(val))
            fn.Result->SetBool(false);
        else
            fn.Result->SetBool(true);
    }
}


// ASSetPropFlags function
static void    GAS_GlobalASSetPropFlags(const FnCall& fn)
{
    unsigned version = fn.Env->GetVersion();
    
    // Check the arguments
    SF_ASSERT(fn.NArgs == 3 || fn.NArgs == 4);
    SF_ASSERT((version == 5) ? (fn.NArgs == 3) : true);
    
    ObjectInterface* const obj = fn.Arg(0).ToObjectInterface(fn.Env);
    if (!obj)
        return;
    
    // list of child names
    const Value& arg1 = fn.Arg(1);
    Ptr<ArrayObject> props;
    
    if (arg1.IsString ())
    {
        // special case, if the properties are specified as a comma-separated string
        props = StringProto::StringSplit(fn.Env, arg1.ToString(fn.Env), ",");
    }
    else if (arg1.IsObject ()) 
    {
        Object* _props = fn.Arg(1).ToObject(fn.Env);
        if (_props == NULL)
        {
            SF_ASSERT(fn.Arg(1).GetType() == Value::NULLTYPE);
        }
        else if (_props->GetObjectType () == Object::Object_Array)
            props = static_cast<ArrayObject*>(_props);
        else if (_props->GetObjectType () == Object::Object_String)
            props = StringProto::StringSplit(fn.Env, arg1.ToString(fn.Env), ",");
        else
            return;
    }
    else if (!arg1.IsNull ())
        return;
    // a number which represents three bitwise flags which
    // are used to determine whether the list of child names should be hidden,
    // un-hidden, protected from over-write, un-protected from over-write,
    // protected from deletion and un-protected from deletion
    UByte setTrue = (UByte) (fn.Arg(2).ToInt32(fn.Env) & PropFlags::PropFlag_Mask);
    
    // Is another integer bitmask that works like setTrue,
    // except it sets the attributes to false. The
    // setFalse bitmask is applied before setTrue is applied
    
    // ASSetPropFlags was exposed in Flash 5, however the fourth argument 'setFalse'
    // was not required as it always defaulted to the value '~0'.
    UByte setFalse = (UByte) ((fn.NArgs == 3 ?
        (version == 5 ? ~0 : 0) : fn.Arg(3).ToUInt32(fn.Env))
        & PropFlags::PropFlag_Mask);

    ASStringContext* psc = fn.Env->GetSC();
    
    if (!props)
    {
        // Take all the members of the object
        struct MemberVisitor : ObjectInterface::MemberVisitor
        {
            ObjectInterface* obj;
            ASStringContext*   pSC;
            UByte               SetTrue, SetFalse;

            MemberVisitor(ASStringContext* psc, ObjectInterface* _obj, UByte setTrue, UByte setFalse)
                : obj(_obj), pSC(psc), SetTrue(setTrue), SetFalse(setFalse) {}

            virtual void Visit(const ASString& name, const Value&, UByte flags)
            {
                PropFlags fl(flags);
                fl.SetFlags(SetTrue, SetFalse);
                obj->SetMemberFlags (pSC, name, fl.Flags);
            }
        } visitor (psc, obj, setTrue, setFalse);

        obj->VisitMembers(psc, &visitor, Object::VisitMember_DontEnum | Object::VisitMember_NamesOnly);
    }
    else 
    {
        for (int i = 0, n = props->GetSize(); i < n; ++i)
        {
            const Value* elem = props->GetElementPtr(i);
            if (elem == 0)
                continue;

            ASString key = elem->ToString(fn.Env);
            Member member;
            if (obj->FindMember(psc, key, &member))
            {
                PropFlags fl = member.GetMemberFlags();
                fl.SetFlags(setTrue, setFalse);

                obj->SetMemberFlags(psc, key, fl.Flags);
            }
        }
    }
}

// ASnative, class 800, function 2 - returns mouse button state
static void    GAS_ASnativeMouseButtonStates(const FnCall& fn)
{
    if (fn.NArgs >= 1)
    {
        unsigned mask = fn.Arg(0).ToUInt32(fn.Env);
        fn.Result->SetBool((fn.Env->GetMovieImpl()->GetMouseState(0)->GetButtonsState() & mask) == mask);
    }
}

// undocumented function ASnative. 
// Now, we support only class 800, function 2 - returns mouse button state
static void    GAS_GlobalASnative(const FnCall& fn)
{
    fn.Result->SetUndefined();
    if (fn.NArgs >= 2)
    {
        unsigned classId = (unsigned)fn.Arg(0).ToUInt32(fn.Env);
        unsigned funcId  = (unsigned)fn.Arg(1).ToUInt32(fn.Env);
        if (classId == 800 && funcId == 2)
        {
            Ptr<CFunctionObject> func = *SF_HEAP_NEW(fn.Env->GetHeap()) 
                CFunctionObject(fn.Env->GetSC(), GAS_ASnativeMouseButtonStates);
            fn.Result->SetAsFunction(FunctionRef(func));
        }
    }
}

static void GAS_GlobalEscape(const FnCall& fn)
{
    fn.Result->SetUndefined();
    if (fn.NArgs == 1)
    {
        ASString str(fn.Arg(0).ToString(fn.Env));
        String escapedStr;
        ASUtils::Escape(str.ToCStr(), str.GetSize(), &escapedStr);
        fn.Result->SetString(fn.Env->CreateString(escapedStr));
    }
}
static void GAS_GlobalUnescape(const FnCall& fn)
{
    fn.Result->SetUndefined();
    if (fn.NArgs == 1)
    {
        ASString str(fn.Arg(0).ToString(fn.Env));
        String unescapedStr;
        ASUtils::Unescape(str.ToCStr(), str.GetSize(), &unescapedStr);
        fn.Result->SetString(fn.Env->CreateString(unescapedStr));
    }
}


static void GAS_GlobalEscapeSpecialHTML(const FnCall& fn)
{
    fn.Result->SetUndefined();
    if (fn.NArgs == 1)
    {
        ASString str(fn.Arg(0).ToString(fn.Env));
        String escapedStr;
        String::EscapeSpecialHTML(str.ToCStr(), str.GetLength(), &escapedStr);
        fn.Result->SetString(fn.Env->CreateString(escapedStr));
    }
}

static void GAS_GlobalUnescapeSpecialHTML(const FnCall& fn)
{
    fn.Result->SetUndefined();
    if (fn.NArgs == 1)
    {
        ASString str(fn.Arg(0).ToString(fn.Env));
        String unescapedStr;
        String::UnescapeSpecialHTML(str.ToCStr(), str.GetLength(), &unescapedStr);
        fn.Result->SetString(fn.Env->CreateString(unescapedStr));
    }
}


static void GAS_GlobalUpdateAfterEvent(const FnCall& fn)
{
    SF_UNUSED(fn);
    // do nothing!
}

#ifndef SF_NO_IME_SUPPORT
static void GAS_GlobalIMECommand(const FnCall& fn)
{   
    if (fn.NArgs >= 2)
    {
        InteractiveObject* const poriginalTarget   = fn.Env->GetTarget();   
        MovieImpl* proot                     = poriginalTarget->GetMovieImpl();
        Ptr<IMEManagerBase> pimeMgr             = proot->GetIMEManager();
        if (pimeMgr)
        {
            pimeMgr->IMECommand(proot, fn.Arg(0).ToString(fn.Env).ToCStr(), fn.Arg(1).ToString(fn.Env).ToCStr());
        }
    }
}

static void GAS_SetIMECandidateListStyle(const FnCall& fn)
{    

    if (fn.NArgs >= 1)
    {
        InteractiveObject* const poriginalTarget	= fn.Env->GetTarget();   
        MovieImpl* proot							= poriginalTarget->GetMovieImpl();
        Ptr<IMEManagerBase> pimeMgr					= proot->GetIMEManager();
        if (pimeMgr)
        {
            Ptr<Object> pobj = fn.Arg(0).ToObject(fn.Env);
            if (pobj)
            {
                IMECandidateListStyle st;
                Value val;
                if (pobj->GetMember(fn.Env, fn.Env->CreateConstString("textColor"), &val))
                {
                    Number n = val.ToNumber(fn.Env);
                    if (!NumberUtil::IsNaNOrInfinity(n))
                        st.SetTextColor((UInt32)n);
                }
                if (pobj->GetMember(fn.Env, fn.Env->CreateConstString("backgroundColor"), &val))
                {
                    Number n = val.ToNumber(fn.Env);
                    if (!NumberUtil::IsNaNOrInfinity(n))
                        st.SetBackgroundColor((UInt32)n);
                }
                if (pobj->GetMember(fn.Env, fn.Env->CreateConstString("indexBackgroundColor"), &val))
                {
                    Number n = val.ToNumber(fn.Env);
                    if (!NumberUtil::IsNaNOrInfinity(n))
                        st.SetIndexBackgroundColor((UInt32)n);
                }
                if (pobj->GetMember(fn.Env, fn.Env->CreateConstString("selectedTextColor"), &val))
                {
                    Number n = val.ToNumber(fn.Env);
                    if (!NumberUtil::IsNaNOrInfinity(n))
                        st.SetSelectedTextColor((UInt32)n);
                }
                if (pobj->GetMember(fn.Env, fn.Env->CreateConstString("selectedTextBackgroundColor"), &val))
                {
                    Number n = val.ToNumber(fn.Env);
                    if (!NumberUtil::IsNaNOrInfinity(n))
                        st.SetSelectedBackgroundColor((UInt32)n);
                }
                if (pobj->GetMember(fn.Env, fn.Env->CreateConstString("selectedIndexBackgroundColor"), &val))
                {
                    Number n = val.ToNumber(fn.Env);
                    if (!NumberUtil::IsNaNOrInfinity(n))
                        st.SetSelectedIndexBackgroundColor((UInt32)n);
                }
                if (pobj->GetMember(fn.Env, fn.Env->CreateConstString("fontSize"), &val))
                {
                    Number n = val.ToNumber(fn.Env);
                    if (!NumberUtil::IsNaNOrInfinity(n))
                        st.SetFontSize((unsigned)n);
                }

                // Reading window parameters.
                if (pobj->GetMember(fn.Env, fn.Env->CreateConstString("readingWindowTextColor"), &val))
                {
                    Number n = val.ToNumber(fn.Env);
                    if (!NumberUtil::IsNaNOrInfinity(n))
                        st.SetReadingWindowTextColor((unsigned)n);
                }
                if (pobj->GetMember(fn.Env, fn.Env->CreateConstString("readingWindowBackgroundColor"), &val))
                {
                    Number n = val.ToNumber(fn.Env);
                    if (!NumberUtil::IsNaNOrInfinity(n))
                        st.SetReadingWindowBackgroundColor((unsigned)n);
                }
                if (pobj->GetMember(fn.Env, fn.Env->CreateConstString("readingWindowFontSize"), &val))
                {
                    Number n = val.ToNumber(fn.Env);
                    if (!NumberUtil::IsNaNOrInfinity(n))
                        st.SetReadingWindowFontSize((unsigned)n);
                }
                pimeMgr->SetCandidateListStyle(st);
            }
        }
    }
	
}

static void GAS_GetIMECandidateListStyle(const FnCall& fn)
{

    InteractiveObject* const poriginalTarget   = fn.Env->GetTarget();   
    MovieImpl* proot                     = poriginalTarget->GetMovieImpl();
    Ptr<IMEManagerBase> pimeMgr             = proot->GetIMEManager();
    if (pimeMgr)
    {
        IMECandidateListStyle st; 
        if (pimeMgr->GetCandidateListStyle(&st))
        {
            Ptr<Object> pobj = *SF_HEAP_NEW(fn.Env->GetHeap()) Object(fn.Env);
            if (st.HasTextColor())
            {
                Number c = (Number)(st.GetTextColor() & 0xFFFFFFu);
                pobj->SetConstMemberRaw(fn.Env->GetSC(), "textColor", Value(c));
            }
            if (st.HasBackgroundColor())
            {
                Number c = (Number)(st.GetBackgroundColor() & 0xFFFFFFu);
                pobj->SetConstMemberRaw(fn.Env->GetSC(), "backgroundColor", Value(c));
            }
            if (st.HasIndexBackgroundColor())
            {
                Number c = (Number)(st.GetIndexBackgroundColor() & 0xFFFFFFu);
                pobj->SetConstMemberRaw(fn.Env->GetSC(), "indexBackgroundColor", Value(c));
            }
            if (st.HasSelectedTextColor())
            {
                Number c = (Number)(st.GetSelectedTextColor() & 0xFFFFFFu);
                pobj->SetConstMemberRaw(fn.Env->GetSC(), "selectedTextColor", Value(c));
            }
            if (st.HasSelectedBackgroundColor())
            {
                Number c = (Number)(st.GetSelectedBackgroundColor() & 0xFFFFFFu);
                pobj->SetConstMemberRaw(fn.Env->GetSC(), "selectedTextBackgroundColor", Value(c));
            }
            if (st.HasSelectedIndexBackgroundColor())
            {
                Number c = (Number)(st.GetSelectedIndexBackgroundColor() & 0xFFFFFFu);
                pobj->SetConstMemberRaw(fn.Env->GetSC(), "selectedIndexBackgroundColor", Value(c));
            }
            if (st.HasFontSize())
            {
                Number c = (Number)st.GetFontSize();
                pobj->SetConstMemberRaw(fn.Env->GetSC(), "fontSize", Value(c));
            }
            // Reading Window Styles

            if (st.HasReadingWindowTextColor())
            {
                Number c = (Number)st.GetReadingWindowTextColor();
                pobj->SetConstMemberRaw(fn.Env->GetSC(), "readingWindowTextColor", Value(c));
            }

            if (st.HasReadingWindowBackgroundColor())
            {
                Number c = (Number)st.GetReadingWindowBackgroundColor();
                pobj->SetConstMemberRaw(fn.Env->GetSC(), "readingWindowBackgroundColor", Value(c));
            }

            if (st.HasReadingWindowFontSize())
            {
                Number c = (Number)st.GetReadingWindowFontSize();
                pobj->SetConstMemberRaw(fn.Env->GetSC(), "readingWindowFontSize", Value(c));
            }

            fn.Result->SetAsObject(pobj);
        }
    }
}

static void GAS_GetInputLanguage(const FnCall& fn)
{
    if (!fn.Env)
        return;

    // Try to get IME Manager.
    Ptr<IMEManagerBase> pimeManager = fn.Env->GetMovieImpl()->GetIMEManager();
    ASString resultStr = fn.Env->GetGC()->CreateConstString("UNKNOWN");

    if(pimeManager)
    {
        resultStr = pimeManager->GetInputLanguage();
    }

    fn.Result->SetString(resultStr);
}


#endif // SF_NO_IME_SUPPORT

Object* GlobalContext::AddPackage(ASStringContext *psc, Object* pparent, Object* objProto, const char* const packageName)
{
    char            buf[256];
    UPInt           nameSz = strlen(packageName) + 1;
    const char*     pname = packageName;
    Ptr<Object> parent = pparent;

    while(pname)
    {
        const char* p = strchr(pname, '.');
        UPInt sz;
        if (p)
            sz = p++ - pname + 1;
        else
            sz = nameSz - (pname - packageName);

        SF_ASSERT(sz <= sizeof(buf));
        if (sz > sizeof(buf)) 
            sz = sizeof(buf);
    
        //strncpy(buf, pname, sz-1);
        memcpy(buf, pname, sz-1);
        buf[sz-1] = '\0';

        pname = p;

        Value        pkgObjVal;
        Ptr<Object> pkgObj;
        ASString       memberName(psc->CreateString(buf));

        if (parent->GetMemberRaw(psc, memberName, &pkgObjVal))
        {
            pkgObj = pkgObjVal.ToObject(NULL);
        }
        else
        {
            pkgObj = *SF_HEAP_NEW(psc->GetHeap()) Object(psc, objProto);
            parent->SetMemberRaw(psc, memberName, Value(pkgObj));
        }
        parent = pkgObj;
    }
    return parent;
}


class GASGlobalObject : public Object
{
    GlobalContext* pGC;

public:
    GASGlobalObject(GlobalContext* pgc) : Object(pgc->GetGC()), pGC(pgc) {}
    ~GASGlobalObject() 
    {
    }

    virtual bool    GetMemberRaw(ASStringContext* psc, const ASString& name, Value* val)
    {
        if (name == psc->GetBuiltin(ASBuiltin_gfxExtensions))
        {
            if (pGC->GFxExtensions.IsDefined())
            {
                val->SetBool(pGC->GFxExtensions.IsTrue());
                return true;
            }
            else
                val->SetUndefined();
            return false;
        }
        return Object::GetMemberRaw(psc, name, val);
    }
    virtual bool    SetMember(Environment *penv, const ASString& name, const Value& val, const PropFlags& flags = PropFlags())
    {
        if (name == penv->GetSC()->GetBuiltin(ASBuiltin_gfxExtensions))
        {
            pGC->GFxExtensions = val.ToBool(penv);
            if (pGC->GFxExtensions.IsTrue())
            {
                SetConstMemberRaw(penv->GetSC(), "gfxVersion", Value(pGC->CreateConstString(GFX_VERSION_STRING)));
            }
            else
            {
                DeleteMember(penv->GetSC(), pGC->CreateConstString("gfxVersion"));
            }
            return Object::SetMember(penv, name, Value(Value::UNSET), flags);
        }
        else if (pGC->GFxExtensions.IsTrue())
        {
            if (name == penv->GetSC()->GetBuiltin(ASBuiltin_noInvisibleAdvance))
            {
                MovieImpl* proot = penv->GetMovieImpl();
                if (proot)
                    proot->SetNoInvisibleAdvanceFlag(val.ToBool(penv));
            }
            else if (name == penv->GetSC()->GetBuiltin(ASBuiltin_continueAnimation))
            {
                MovieImpl* proot = penv->GetMovieImpl();
                if (proot)
                    proot->SetContinueAnimationFlag(val.ToBool(penv));
            }
        }
        return Object::SetMemberRaw(penv->GetSC(), name, val, flags);
    }
};

// Global functions for ActionScript
NameFunction GFxAction_Global_StaticFunctions[] =
{    
    {"trace",               GAS_GlobalTrace},   
    {"parseInt",            GAS_GlobalParseInt},
    {"parseFloat",          GAS_GlobalParseFloat},
    {"ifFrameLoaded",       GAS_GlobalIfFrameLoaded},
    {"isNaN",               GAS_GlobalIsNaN},
    {"isFinite",            GAS_GlobalIsFinite},

    {"escape",              GAS_GlobalEscape},
    {"unescape",            GAS_GlobalUnescape},
    {"escapeSpecialHTML",   GAS_GlobalEscapeSpecialHTML},
    {"unescapeSpecialHTML", GAS_GlobalUnescapeSpecialHTML},
    {"updateAfterEvent",    GAS_GlobalUpdateAfterEvent},
    {"setInterval",         IntervalTimer::SetInterval},
    {"clearInterval",       IntervalTimer::ClearInterval},
    {"setTimeout",          IntervalTimer::SetTimeout},
    {"clearTimeout",        IntervalTimer::ClearTimeout},

#ifndef SF_NO_IME_SUPPORT
    // extensions for IME
    {"imecommand",          GAS_GlobalIMECommand},    
    {"setIMECandidateListStyle", GAS_SetIMECandidateListStyle},    
    {"getInputLanguage",    GAS_GetInputLanguage},   
    {"getIMECandidateListStyle", GAS_GetIMECandidateListStyle},
#endif

    // ASSetPropFlags/ASnative
    {"ASSetPropFlags",      GAS_GlobalASSetPropFlags},
    {"ASnative",            GAS_GlobalASnative},

    {0, 0}
};

// Helper function, initializes the global object.
GlobalContext::GlobalContext(MovieImpl* proot)
//    : StringManager(proot->GetMovieHeap())
{    
    Init(proot);
}

void GlobalContext::Init(MovieImpl* proot)
{    
    pMovieRoot = proot;
    pHeap      = pMovieRoot->GetMovieHeap();
    // Initialization of built-ins is done in base.

    // String context for construction, delegates to us.
    // SWF Version does not matter for member creation.
    ASStringContext sc(this, 8);

    if (!pGlobal)
        pGlobal = *SF_HEAP_NEW(pHeap) GASGlobalObject(this);

    // Create constructors
    FunctionRef objCtor (*SF_HEAP_NEW(pHeap) ObjectCtorFunction(&sc));
    FunctionRef functionCtor (*SF_HEAP_NEW(pHeap) FunctionCtorFunction(&sc));

    // Init built-in prototype instances
    Ptr<Object> objProto        = *SF_HEAP_NEW(pHeap) ObjectProto(&sc, objCtor);
    Ptr<Object> functionProto   = *SF_HEAP_NEW(pHeap) FunctionProto(&sc, objProto, functionCtor);

    // Special case: Object.__proto__ == Function.prototype
    objCtor->Set__proto__(&sc, functionProto);

    // A special case for Function class: its __proto__ is equal to Function.prototype.
    // This is the undocumented feature.
    functionCtor->Set__proto__(&sc, functionProto);

    Prototypes.Add(ASBuiltin_Object, objProto);
    Prototypes.Add(ASBuiltin_Function, functionProto);

    FunctionRef movieClipCtor (*SF_HEAP_NEW(pHeap) MovieClipCtorFunction(&sc));
#ifdef GFX_ENABLE_KEYBOARD
    FunctionRef keyCtor (*SF_HEAP_NEW(pHeap) KeyCtorFunction(&sc, proot));
    Ptr<Object> keyProto  = *SF_HEAP_NEW(pHeap) KeyProto(&sc, objProto, keyCtor);
    Prototypes.Add(ASBuiltin_Key, keyProto);
#endif // SF_NO_KEYBOARD_SUPPORT

    Ptr<Object> movieClipProto  = *SF_HEAP_NEW(pHeap) MovieClipProto(&sc, objProto, movieClipCtor);
    Prototypes.Add(ASBuiltin_MovieClip, movieClipProto);

    // @@ pGlobal should really be a
    // client-visible player object, which
    // contains one or more actual InteractiveObject
    // instances.

    NameFunction::AddConstMembers(pGlobal, &sc, GFxAction_Global_StaticFunctions);

#ifndef SF_NO_IME_SUPPORT
    // extensions for IME
    pGlobal->SetConstMemberRaw(&sc, "imecommand", Value(&sc, GAS_GlobalIMECommand));    
    pGlobal->SetConstMemberRaw(&sc, "setIMECandidateListStyle", Value(&sc, GAS_SetIMECandidateListStyle));
    pGlobal->SetConstMemberRaw(&sc, "getInputLanguage", Value(&sc, GAS_GetInputLanguage)); 
    pGlobal->SetConstMemberRaw(&sc, "getIMECandidateListStyle", Value(&sc, GAS_GetIMECandidateListStyle));
#endif // SF_NO_IME_SUPPORT

    pGlobal->SetMemberRaw(&sc, GetBuiltin(ASBuiltin_Object), Value(objCtor));
    pGlobal->SetMemberRaw(&sc, GetBuiltin(ASBuiltin_Function), Value(functionCtor));
    pGlobal->SetMemberRaw(&sc, GetBuiltin(ASBuiltin_MovieClip), Value(movieClipCtor));
#ifdef GFX_ENABLE_KEYBOARD
    // Key should be created here, since it is a listener of KeyboardState
    pGlobal->SetMemberRaw(&sc, GetBuiltin(ASBuiltin_Key), Value(keyCtor));
#endif
    AddBuiltinClassRegistry<ASBuiltin_Math, MathCtorFunction>(sc, pGlobal);

    AddBuiltinClassRegistry<ASBuiltin_Array, ArrayCtorFunction>(sc, pGlobal);
    AddBuiltinClassRegistry<ASBuiltin_Number, NumberCtorFunction>(sc, pGlobal);
    AddBuiltinClassRegistry<ASBuiltin_String, StringCtorFunction>(sc, pGlobal);
    AddBuiltinClassRegistry<ASBuiltin_Boolean, BooleanCtorFunction>(sc, pGlobal);
#ifdef GFX_AS2_ENABLE_COLOR
    AddBuiltinClassRegistry<ASBuiltin_Color, ColorCtorFunction>(sc, pGlobal);
#endif
    AddBuiltinClassRegistry<ASBuiltin_Button, ButtonCtorFunction>(sc, pGlobal);

#ifdef GFX_AS2_ENABLE_MOVIECLIPLOADER
    AddBuiltinClassRegistry<ASBuiltin_MovieClipLoader, MovieClipLoaderCtorFunction>(sc, pGlobal);
#endif
#ifdef GFX_AS2_ENABLE_LOADVARS
    AddBuiltinClassRegistry<ASBuiltin_LoadVars, GASLoadVarsLoaderCtorFunction>(sc, pGlobal);
#endif

#ifdef GFX_AS2_ENABLE_STAGE
    AddBuiltinClassRegistry<ASBuiltin_Stage, StageCtorFunction>(sc, pGlobal);
#endif
#ifdef GFX_AS2_ENABLE_SELECTION
    AddBuiltinClassRegistry<ASBuiltin_Selection, SelectionCtorFunction>(sc, pGlobal);
#endif
    AddBuiltinClassRegistry<ASBuiltin_AsBroadcaster, AsBroadcasterCtorFunction>(sc, pGlobal);
#ifdef GFX_AS2_ENABLE_MOUSE
    AddBuiltinClassRegistry<ASBuiltin_Mouse, MouseCtorFunction>(sc, pGlobal);
#endif

    AddBuiltinClassRegistry<ASBuiltin_TextField, TextFieldCtorFunction>(sc, pGlobal);
#ifdef GFX_AS2_ENABLE_TEXTFORMAT
    AddBuiltinClassRegistry<ASBuiltin_TextFormat, TextFormatCtorFunction>(sc, pGlobal);
#endif

#ifdef GFX_AS_ENABLE_TEXTSNAPSHOT
    AddBuiltinClassRegistry<ASBuiltin_TextSnapshot, TextSnapshotCtorFunction>(sc, pGlobal);
#endif

#ifdef GFX_AS2_ENABLE_DATE
    AddBuiltinClassRegistry<ASBuiltin_Date, DateCtorFunction>(sc, pGlobal);
#endif

#ifdef GFX_AS2_ENABLE_SHAREDOBJECT
    AddBuiltinClassRegistry<ASBuiltin_SharedObject, SharedObjectCtorFunction>(sc, pGlobal);
#endif

#ifdef SF_AMP_SERVER
    AddBuiltinClassRegistry<ASBuiltin_Amp, AmpMarkerCtorFunction>(sc, pGlobal);
#endif
    // classes from packages
    FlashGeomPackage = AddPackage(&sc, pGlobal, objProto, "flash.geom");
#ifdef GFX_AS2_ENABLE_TRANSFORM
    AddBuiltinClassRegistry<ASBuiltin_Transform, TransformCtorFunction>(sc, FlashGeomPackage);
#endif
#ifdef GFX_AS2_ENABLE_MATRIX
    AddBuiltinClassRegistry<ASBuiltin_Matrix, GASMatrixCtorFunction>(sc, FlashGeomPackage);
#endif
#ifdef GFX_AS2_ENABLE_POINT
    AddBuiltinClassRegistry<ASBuiltin_Point, PointCtorFunction>(sc, FlashGeomPackage);
#endif
#ifdef GFX_AS2_ENABLE_RECTANGLE
    AddBuiltinClassRegistry<ASBuiltin_Rectangle, RectangleCtorFunction>(sc, FlashGeomPackage);
#endif
#ifdef GFX_AS2_ENABLE_COLORTRANSFORM
    AddBuiltinClassRegistry<ASBuiltin_ColorTransform, ColorTransformCtorFunction>(sc, FlashGeomPackage);
#endif

#ifdef GFX_AS2_ENABLE_CAPABILITIES
    SystemPackage = AddPackage(&sc, pGlobal, objProto, "System");
    AddBuiltinClassRegistry<ASBuiltin_Capabilities, CapabilitiesCtorFunction>(sc, SystemPackage);
#endif

//#if !defined(SF_NO_FXPLAYER_AS_IME) && defined(SF_ENABLE_IME) 

#ifndef SF_NO_IME_SUPPORT
    AddBuiltinClassRegistry<ASBuiltin_Ime, GASImeCtorFunction>(sc, SystemPackage);
#endif 

    FlashExternalPackage = AddPackage(&sc, pGlobal, objProto, "flash.external");
    AddBuiltinClassRegistry<ASBuiltin_ExternalInterface, ExternalInterfaceCtorFunction>(sc, FlashExternalPackage);

#ifdef GFX_AS2_ENABLE_BITMAPDATA
    FlashDisplayPackage = AddPackage(&sc, pGlobal, objProto, "flash.display");
    AddBuiltinClassRegistry<ASBuiltin_BitmapData, BitmapDataCtorFunction>(sc, FlashDisplayPackage);
#endif

    FlashFiltersPackage = AddPackage(&sc, pGlobal, objProto, "flash.filters");
#ifdef GFX_AS2_ENABLE_FILTERS
    AddBuiltinClassRegistry<ASBuiltin_BitmapFilter, BitmapFilterCtorFunction>(sc, FlashFiltersPackage);
    AddBuiltinClassRegistry<ASBuiltin_DropShadowFilter, DropShadowFilterCtorFunction>(sc, FlashFiltersPackage);
    AddBuiltinClassRegistry<ASBuiltin_GlowFilter, GlowFilterCtorFunction>(sc, FlashFiltersPackage);
    AddBuiltinClassRegistry<ASBuiltin_BlurFilter, BlurFilterCtorFunction>(sc, FlashFiltersPackage);
    AddBuiltinClassRegistry<ASBuiltin_BevelFilter, BevelFilterCtorFunction>(sc, FlashFiltersPackage);
    AddBuiltinClassRegistry<ASBuiltin_ColorMatrixFilter, ColorMatrixFilterCtorFunction>(sc, FlashFiltersPackage);
#endif  // GFX_AS2_ENABLE_FILTERS

    InitStandardMembers();

    // hide all members
    struct MemberVisitor : ObjectInterface::MemberVisitor
    {
        Ptr<Object> obj;
        ASStringContext *psc;

        MemberVisitor (ASStringContext *_psc, Object* _obj) : obj(_obj), psc(_psc) {}

        virtual void Visit(const ASString& name, const Value& val, UByte flags)
        {
            SF_UNUSED(val);
            obj->SetMemberFlags(psc, name, (UByte)(flags| PropFlags::PropFlag_DontEnum));

        }
    } visitor (&sc, pGlobal);
    pGlobal->VisitMembers(&sc, &visitor, ObjectInterface::VisitMember_DontEnum);
}

// GlobalContext::GlobalContext(MemoryHeap* pheap)
// : StringManager(pheap)
// {    
//     pHeap = pheap;
// }

GlobalContext::~GlobalContext()
{
    // ReleaseBuiltins() - release of built-ins is done in base destructor.
    // If that was not the case we would've had to clean up all local string maps first explicitly.
}

ASRefCountCollector* GlobalContext::GetGC() 
{ 
    return (pMovieRoot) ? ToAS2Root(pMovieRoot)->GetASGC() : NULL; 
}                                       

void GlobalContext::PreClean(bool preserveBuiltinProps)
{
    if (preserveBuiltinProps)
    {
        SF_ASSERT(pMovieRoot);
        Ptr<GASGlobalObject> pnewglobal = *SF_HEAP_NEW(pHeap) GASGlobalObject(this);
        ASStringContext sc(this, 8);
        ASString gfxPlayer = sc.CreateConstString("gfxPlayer");
        ASString gfxLanguage = sc.CreateConstString("gfxLanguage");
        ASString gfxArg = sc.CreateConstString("gfxArg");
        Value v;
        pGlobal->GetMemberRaw(&sc, gfxPlayer, &v);
        pnewglobal->SetMemberRaw(&sc, gfxPlayer, v);
        pGlobal->GetMemberRaw(&sc, gfxLanguage, &v);
        pnewglobal->SetMemberRaw(&sc, gfxLanguage, v);
        pGlobal->GetMemberRaw(&sc, gfxArg, &v);
        pnewglobal->SetMemberRaw(&sc, gfxArg, v);
        pGlobal = pnewglobal;
    }
    else
        pGlobal = NULL;
    RegisteredClasses.Clear();
    BuiltinClassesRegistry.Clear();
    Prototypes.Clear();
    DetachMovieRoot();
}

FunctionObject* GlobalContext::ResolveFunctionName(const ASString& functionName) 
{
    // not found; lets try to resolve
    ClassRegEntry* regEntry = GetBuiltinClassRegistrar(functionName);
    if (regEntry)
    {
        if (!regEntry->IsResolved())
        {
            // found, do we need to resolve by calling RegistrarFunc
            FunctionRef f = regEntry->RegistrarFunc(this);

            // theoretically, RegistrarFunc may add another registration,
            // for example StyleSheet has been added from the TextField's Register func.
            // That is why we need to search for the regEntry again.
            regEntry = GetBuiltinClassRegistrar(functionName);
            SF_ASSERT(regEntry);
            regEntry->ResolvedFunc = f.GetObjectPtr();
        }
        return regEntry->ResolvedFunc;
    }
    return NULL;
}

Object* GlobalContext::GetPrototype(ASBuiltinType type) 
{
    Ptr<Object>* presult = Prototypes.Get(type);
    if (!presult)
    {
        ResolveFunctionName(GetBuiltin(type));
        presult = Prototypes.Get(type);            
    }
    return (presult) ? presult->GetPtr() : NULL;
}

Object* GlobalContext::GetActualPrototype(Environment* penv, ASBuiltinType type) 
{
    // force registration
    Ptr<Object> result = GetPrototype(type);
    
    // need to check constructor function's "prototype" prop.
    Value v;
    if (pGlobal->GetMemberRaw(penv->GetSC(), GetBuiltin(type), &v))
    {
        Ptr<Object> o = v.ToObject(penv);
        if (o && o->GetMemberRaw(penv->GetSC(), GetBuiltin(ASBuiltin_prototype), &v))
            result = v.ToObject(penv);
    }
    return result;
}

GlobalContext::ClassRegEntry* GlobalContext::GetBuiltinClassRegistrar(ASString className)
{
    ClassRegEntry* pfuncPtr = BuiltinClassesRegistry.GetCaseCheck(className, true);
    if (pfuncPtr)
    {
        return pfuncPtr;
    }
    return NULL;
}

ASString GlobalContext::FindClassName(Environment *penv, ObjectInterface* iobj)
{
    if (iobj)
    {
        Object* obj;
        if (iobj->IsASCharacter())
            obj = iobj->ToAvmCharacter()->GetASObject();
        else
            obj = iobj->ToASObject();

        Object::MemberHash::ConstIterator it = pGlobal->Members.Begin();
        
        for (; it != pGlobal->Members.End(); ++it)
        {
            const ASString& nm = it->First;
            const Member& m = it->Second;
            const Value& val = m.mValue;
            if (obj->IsFunction())
            {
                if (val.IsFunction() && val.ToFunction(penv).GetObjectPtr() == obj)
                    return nm;
            }
            else
            {
                if (val.IsObject() && val.ToObject(penv) == obj)
                    return nm;
                if (val.IsFunction())
                {
                    FunctionRef f = val.ToFunction(penv);
                    Value protoVal;
                    if (f->GetMemberRaw(penv->GetSC(), GetBuiltin(ASBuiltin_prototype), &protoVal))
                    {
                        if (protoVal.IsObject() && protoVal.ToObject(penv) == obj)
                            return nm + ".prototype";
                    }
                }
            }
        }
    }
    return GetBuiltin(ASBuiltin_unknown_);
}

void    GlobalContext::InitStandardMembers()
{
    StandardMemberMap.Clear();
    AvmCharacter::InitStandardMembers(this);
}

bool GlobalContext::RegisterClass(ASStringContext* psc, const ASString& className, const FunctionRef& ctorFunction)
{
    RegisteredClasses.SetCaseCheck(className, ctorFunction, psc->IsCaseSensitive());
    return true;
}

bool GlobalContext::UnregisterClass(ASStringContext* psc, const ASString& className)
{    
    if (RegisteredClasses.GetCaseCheck(className, psc->IsCaseSensitive()) == 0)
        return false;
    RegisteredClasses.RemoveCaseCheck(className, psc->IsCaseSensitive());
    return true;
}

bool GlobalContext::FindRegisteredClass(ASStringContext* psc, const ASString& className, FunctionRef* pctorFunction)
{
    // MA TBD: Conditional case sensitivity
    const FunctionRef* ctor = RegisteredClasses.GetCaseCheck(className, psc->IsCaseSensitive());
    if (ctor == 0)
        return false;
    if (pctorFunction)
        *pctorFunction = *ctor;
    return true;
}

void GlobalContext::UnregisterAllClasses()
{
    RegisteredClasses.Resize(0);
}

MovieRoot*    GlobalContext::GetAS2Root()                  
{ 
    return ToAS2Root(pMovieRoot); 
}

const MovieRoot* GlobalContext::GetAS2Root() const      
{ 
    return ToAS2Root(pMovieRoot); 
}

AS2::StringManager* GlobalContext::GetStringManager() const 
{ 
    return ToAS2Root(pMovieRoot)->GetStringManager(); 
}

//
// ActionBuffer
//

// ***** NameFunction

void NameFunction::AddConstMembers(ObjectInterface* pobj, ASStringContext *psc,
                                      const NameFunction* pfunctions, UByte flags)
{
    MemoryHeap* pheap = psc->GetHeap();
    Object* pfuncProto = psc->pContext->GetPrototype(ASBuiltin_Function);
    while(pfunctions->Name)
    {
        pobj->SetConstMemberRaw(psc, pfunctions->Name,
            FunctionRef(*SF_HEAP_NEW(pheap) CFunctionObject(psc, pfuncProto, pfunctions->Function)),
            flags);
        pfunctions++;
    }
}


// ***** With Stack implementation
WithStackEntry::WithStackEntry(Object* pobj, int end)
{
    // Compiler does not yet know Object.
    pObject     =  pobj;
    if (pObject)
        pObject->AddRef();
    SF_ASSERT(end >= 0);
    BlockEndPc  = ((UInt32)end | Mask_IsObject);
}

WithStackEntry::WithStackEntry(InteractiveObject* pcharacter, int end)
{
    pCharacter  = pcharacter;
    if (pCharacter)
        pCharacter->AddRef();
    SF_ASSERT(end >= 0);
    BlockEndPc  = (UInt32)end;
}

WithStackEntry::~WithStackEntry()
{
    if (IsObject() && pObject)
        pObject->Release();
    else if (pCharacter)
        pCharacter->Release();
}

Object*          WithStackEntry::GetObject() const
{
    return IsObject() ? pObject : 0;
}
InteractiveObject*     WithStackEntry::GetCharacter() const
{
    return IsObject() ? 0 : pCharacter;
}

ObjectInterface* WithStackEntry::GetObjectInterface() const
{
    return IsObject() ? static_cast<ObjectInterface*>(pObject) :
                        static_cast<ObjectInterface*>(ToAvmCharacter(pCharacter));
}

// ***** ActionBuffer implementation
ActionBufferData* ActionBufferData::CreateNew()
{
    // We need to allocate ActionBufferData from global heap, since it is possible 
    // to have a pointer to ActionBufferData after its MovieDataDef is unloaded (like, AS
    // class was registered in _global from SWF loaded by "loadMovie" and when the movie
    // is unloaded). So, we allocate all actions from global heap for now.
    return SF_HEAP_NEW_ID(Memory::GetGlobalHeap(), StatMD_ASBinaryData_Mem) ActionBufferData();
}


void    ActionBufferData::Read(Stream* in, unsigned actionLength)
{
#ifdef SF_BUILD_DEBUG
    FileName = in->GetFileName();
#endif
    
    BufferLen = actionLength;

    SF_ASSERT(pBuffer == NULL);   // Should not exist
    pBuffer = (UByte*)SF_HEAP_AUTO_ALLOC_ID(this, BufferLen, StatMD_ASBinaryData_Mem);
    in->ReadToBuffer(pBuffer, BufferLen);

#ifdef GFX_VERBOSE_PARSE_ACTION
    if (in->IsVerboseParseAction())
    {
        StreamContext sc(pBuffer);
        for (;;)
        {
            UPInt   instructionStart    = sc.CurByteIndex;
            UPInt   pc                  = sc.CurByteIndex;
            UByte   actionId            = sc.ReadU8();

            if (actionId & 0x80)
            {
                // Action contains extra data.  Read it.
                int length = sc.ReadU16();
                sc.Skip(length);
            }

            in->LogParseAction("%4d\t", (int)pc);
            Disasm da(in->GetLog(), Log_ParseAction);
            da.LogDisasm(&pBuffer[instructionStart]);

            if (actionId == 0)
            {
                // end of action buffer.
                break;
            }
        }
    }
#endif

#ifdef SF_AMP_SERVER
    if (SwdHandle == 0)
    {
        SwdHandle = AmpServer::GetInstance().GetNextSwdHandle();
    }
#endif

    // Last byte in buffer will always be zero.
    // However, some files may contain garbage at the end of the action buffer 
    // (for example world_wide_leader.swf). In this case we just check, if there is
    // a zero byte at all. Search backward.
    SF_ASSERT(pBuffer[BufferLen-1] == 0 || SFmemrchr(pBuffer, BufferLen, 0));
}

void    ActionBufferData::Read(StreamContext* psc, unsigned eventLength)
{
    psc->Align();
    pBuffer = (UByte*)SF_HEAP_AUTO_ALLOC_ID(this, eventLength, StatMD_ASBinaryData_Mem);
    memcpy(pBuffer, psc->pData + psc->CurByteIndex, eventLength);
    BufferLen = eventLength;
    psc->Skip(BufferLen);

#ifdef SF_AMP_SERVER
    if (SwdHandle == 0)
    {
        SwdHandle = AmpServer::GetInstance().GetNextSwdHandle();
    }
#endif

    // Last byte in buffer will always be zero.
    SF_ASSERT(pBuffer[BufferLen-1] == 0);
}

ActionBuffer::ActionBuffer(ASStringContext *psc, ActionBufferData *pbufferData)
:   pBufferData(pbufferData),  
    Dictionary(psc->GetBuiltin(ASBuiltin_empty_)),
    DeclDictProcessedAt(-1)
{ 
}

ActionBuffer::~ActionBuffer()
{ 
}

// Interpret the DeclDict opcode.  Don't read stopPc or
// later.  A dictionary is some static strings embedded in the
// action buffer; there should only be one dictionary per
// action buffer.
//
// NOTE: Normally the dictionary is declared as the first
// action in an action buffer, but I've seen what looks like
// some form of copy protection that amounts to:
//
// <start of action buffer>
//          push true
//          BranchIfTrue label
//          DeclDict   [0]   // this is never executed, but has lots of orphan data declared in the opcode
// label:   // (embedded inside the previous opcode; looks like an invalid jump)
//          ... "protected" code here, including the real DeclDict opcode ...
//          <end of the dummy DeclDict [0] opcode>
//
// So we just interpret the first DeclDict we come to, and
// cache the results.  If we ever hit a different DeclDict in
// the same ActionBuffer, then we log an error and ignore it.
void    ActionBuffer::ProcessDeclDict(ASStringContext *psc, unsigned startPc, unsigned stopPc, class ActionLogger &log)
{
    SF_ASSERT(stopPc <= GetLength());
    const UByte*  Buffer = GetBufferPtr();

    if (DeclDictProcessedAt == (int)startPc)
    {
        // We've already processed this DeclDict.
        unsigned count = Buffer[startPc + 3] | (Buffer[startPc + 4] << 8);
        SF_ASSERT(Dictionary.GetSize() == count);
        SF_UNUSED(count);
        return;
    }

    if (DeclDictProcessedAt != -1)
    {
        if (log.IsVerboseActionErrors())
            log.LogScriptError("ProcessDeclDict(%d, %d) - DeclDict was already processed at %d",
                startPc,
                stopPc,
                DeclDictProcessedAt);
        return;
    }

    DeclDictProcessedAt = (int)startPc;

    // Actual processing.
    unsigned i = startPc;
    unsigned length = Buffer[i + 1] | (Buffer[i + 2] << 8);
    unsigned count = Buffer[i + 3] | (Buffer[i + 4] << 8);
    i += 2;
    SF_UNUSED(length);
    SF_ASSERT(startPc + 3 + length == stopPc);

    Dictionary.Resize(count);    

    // Index the strings.
    for (unsigned ct = 0; ct < count; ct++)
    {
        // Point into the current action buffer.        
        Dictionary[ct] = psc->CreateString((const char*) &Buffer[3 + i]);

        while (Buffer[3 + i])
        {
            // safety check.
            if (i >= stopPc)
            {
                if (log.IsVerboseActionErrors()) 
                    log.LogScriptError("Action buffer dict length exceeded");

                // Jam something into the Remaining (invalid) entries.
                while (ct < count)
                {
                    Dictionary[ct] = psc->CreateString("<invalid>");
                    ct++;
                }
                return;
            }
            i++;
        }
        i++;
    }
}

bool    ActionBuffer::ResolveFrameNumber 
    (Environment* env, const Value& frameValue, InteractiveObject** pptarget, unsigned* pframeNumber)
{
    SF_ASSERT(pframeNumber);
    InteractiveObject* target  = env->GetTarget();
    bool success            = false;

    if (frameValue.GetType() == Value::STRING)
    {   
        ASString   str(frameValue.ToString(env));
        int         mbLength = str.GetLength();
        int         i;

        // Parse possible sprite path...
        for (i=0; i<mbLength; i++)
        {
            if (str.GetCharAt(i) == ':')
            {
                // Must be a target label.
                ASString targetStr(str.Substring(0, i));
                if ((target = env->FindTarget(targetStr))!=0)
                {
                    if (i >= mbLength)
                        target = 0; // No frame.
                    else
                    {   
                        // The remainder of the string is frame number or label.
                        str = str.Substring(i+1, mbLength+1);
                        break;
                    }
                }
            }
        }
            
        if (target && target->GetLabeledFrame(str.ToCStr(), pframeNumber))
            success = true;                  
    }
    else if (frameValue.GetType() == Value::OBJECT)
    {
        // This is a no-op; see TestGotoFrame.Swf
    }
    else if (frameValue.IsNumber())
    {
        // MA: Unlike other places, this frame number is NOT zero-based,
        // because it is computed by action script; so subtract 1.
        *pframeNumber = int(frameValue.ToNumber(env) - 1);
        success = true;
    }
    if (success && pptarget)
        *pptarget = target;

    return success;
}

// Interpret the actions in this action buffer, and evaluate
// them in the given environment.  Execute our whole buffer,
// without any arguments passed in.
void    ActionBuffer::Execute(Environment* env)
{
    //int   LocalStackTop = env->GetLocalFrameTop();
    //env->AddFrameBarrier(); //??AB, should I create a local frame here??? AddFrameBarrier does nothing.
    //Ptr<LocalFrame> curLocalFrame = env->CreateNewLocalFrame();
    
    Execute(env, 0, GetLength(), NULL, 0, Exec_Unknown /* not function2 */);

    //curLocalFrame->ReleaseFramesForLocalFuncs ();
    //env->SetLocalFrameTop(LocalStackTop);
}



#ifdef GFXACTION_COUNT_OPS
// Debug counters for performance statistics.
unsigned Execute_OpCounters[256] =
{
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

unsigned Execute_PushDataTypeCounter[16] =
{
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

unsigned Execute_NotType[16] =
{
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};
#endif

class ExecutionContext
{
public:
    Environment* const          pEnv;
    InteractiveObject* const    pOriginalTarget;
    const UByte* const          pBuffer;

    ActionLogger*               pPrevLog;

    int                         StopPC;
    int                         NextPC;
    int                         PC;

    // Use a custom WithStackHolder to ensure that WithStack is allocated
    // only when necessary and in correct heap.
    struct WithStackHolder
    {
        MemoryHeap*         pHeap;
        WithStackArray*  pWithStackArray;

        WithStackHolder(MemoryHeap* pheap, const WithStackArray *pinit)
            : pHeap(pheap)
        {
            pWithStackArray = pinit ? (SF_HEAP_NEW(pheap) WithStackArray(*pinit)) : 0;
        }
        ~WithStackHolder()
        {
            if (pWithStackArray)
                delete pWithStackArray;
        }

        inline operator WithStackArray* () const       { return pWithStackArray; }
        inline operator const WithStackArray* () const { return pWithStackArray; }

        void PushBack(const WithStackEntry& entry)
        {
            if (!pWithStackArray)
                pWithStackArray = SF_HEAP_NEW(pHeap) WithStackArray();
            pWithStackArray->PushBack(entry);
        }

        inline UPInt GetSize() const { return pWithStackArray ? pWithStackArray->GetSize() : 0; }
    };

//     GASString       TempStr;
//     GASString       TempStr1;
//     Value        TempVal;
//     Value        OwnerVal;
//     Value        ResultVal;
//     Value        FunctionVal;
    WithStackHolder WithStack;
    ActionLogger    LogF;

    UInt8           Version;
    UInt8           ExecType;
    bool            VerboseActionErrors :1;
    bool            VerboseAction       :1;
    bool            IsFunction2         :1;

    SF_INLINE ExecutionContext(Environment* penv, 
                        const WithStackArray *pinitialWithStack, 
                        ActionBuffer::ExecuteType execType,
                        const UByte* pbuffer,
                        const char* fileName) 
      : pEnv(penv), pOriginalTarget(pEnv->GetTarget()), pBuffer(pbuffer), 
        //TempStr(pEnv->GetBuiltin(ASBuiltin_empty_)), TempStr1(TempStr),
        WithStack(penv->GetHeap(), pinitialWithStack), 
        LogF(pOriginalTarget, fileName)
    {
        pPrevLog    = pEnv->GetASLogger();
        Version     = (UInt8)pEnv->GetTarget()->GetVersion();
        ExecType    = (UInt8)execType;
        IsFunction2 = (execType == ActionBuffer::Exec_Function2);
        VerboseAction        = LogF.IsVerboseAction();
        VerboseActionErrors  = LogF.IsVerboseActionErrors();
    }
    SF_INLINE ~ExecutionContext()
    {
        pEnv->SetASLogger(pPrevLog); 
    }

    void SetTargetOpCode();
    void StartDragOpCode();
    void CastObjectOpCode();
    void ImplementsOpCode();
    void EnumerateOpCode(int actionId);
    void ExtendsOpCode();
    void InstanceOfOpCode();
    bool MethodCallOpCode();
    void WaitForFrameOpCode(ActionBuffer* pActions, int actionId);
    void Function1OpCode(ActionBuffer* pActions);
    void Function2OpCode(ActionBuffer* pActions);

    ExecutionContext& operator=(const ExecutionContext&) { return *this; }
};

void ExecutionContext::SetTargetOpCode()
{
    InteractiveObject* target = 0;

    Value targetVal = pEnv->Top();
    if (!targetVal.IsString() && !targetVal.IsCharacter())
    {
        targetVal.SetString(targetVal.ToStringVersioned(pEnv, Version));
    }

    if (targetVal.IsString())
    {
        if (targetVal.ToString(pEnv).IsEmpty())
        {
            // special case for empty string - originalTarget will be used
            target = pOriginalTarget;
        }
        else
        {
            // target is specified as a string, like /mc1/nmc1/mmm1
            Value val;
            pEnv->GetVariable(pEnv->Top().ToString(pEnv), &val, WithStack, &target);

            #ifdef GFX_AS2_VERBOSE
            if (VerboseAction) 
            {
                ASString s1(pEnv->Top().ToDebugString(pEnv));

                if (target)
                    LogF.LogAction("-- ActionSetTarget2: %s (%d)\n",
                        s1.ToCStr(),
                        target->GetId().GetIdIndex());
                else
                    LogF.LogAction("-- ActionSetTarget2: %s - no target found\n",
                        s1.ToCStr());
            }
            #endif
        }
    }
    else if (targetVal.IsCharacter())
    {
        target = pEnv->Top().ToCharacter(pEnv);
    }
    else
    {   
        SF_ASSERT(0);
    }

    if (!target) 
    {
        #ifdef GFX_AS2_VERBOSE_ERRORS
        if (VerboseActionErrors)
        {
            ASString sv(targetVal.ToDebugString(pEnv));                            
            LogF.LogScriptError("SetTarget2(tellTarget) with invalid target '%s'.",
                sv.ToCStr());
        }
        #endif
        //!AB: if target is invalid, then set originalTarget as a target and
        // mark it as "invalid" target. This means that frame-related
        // functions (such as gotoAndStop, etc) should do nothing
        // inside this tellTarget.
        pEnv->SetInvalidTarget(pOriginalTarget);
    }
    else
    {
        pEnv->SetTarget(target);
    }
    pEnv->Drop1();
}

void ExecutionContext::StartDragOpCode()
{
    MovieImpl::DragState st;
    bool lockCenter = pEnv->Top1().ToBool(pEnv);

    st.pCharacter = pEnv->FindTargetByValue(pEnv->Top());
    if (st.pCharacter == NULL)
    {
        #ifdef GFX_AS2_VERBOSE_ERRORS
        if (VerboseActionErrors)
        {
            ASString s0(pEnv->Top().ToDebugString(pEnv));
            LogF.LogScriptError("StartDrag of invalid target '%s'.",
                                s0.ToCStr());
        }
        #endif
    }

    st.Bound = pEnv->Top(2).ToBool(pEnv);
    if (st.Bound)
    {
        st.BoundLT.x = SF_PIXELS_TO_TWIPS((float) pEnv->Top(6).ToNumber(pEnv));
        st.BoundLT.y = SF_PIXELS_TO_TWIPS((float) pEnv->Top(5).ToNumber(pEnv));
        st.BoundRB.x = SF_PIXELS_TO_TWIPS((float) pEnv->Top(4).ToNumber(pEnv));
        st.BoundRB.y = SF_PIXELS_TO_TWIPS((float) pEnv->Top(3).ToNumber(pEnv));
        pEnv->Drop(4);
    }

    if (st.pCharacter)
    {
        // Init mouse offsets based on LockCenter flag.
        st.InitCenterDelta(lockCenter);

        MovieImpl* pmovieRoot = pEnv->GetTarget()->GetMovieImpl();
        SF_ASSERT(pmovieRoot);

        if (pmovieRoot)
            pmovieRoot->SetDragState(st);   
    }

    pEnv->Drop3();
}

void ExecutionContext::CastObjectOpCode()
{
    // Pop object, pop constructor function
    // Make sure o1 is an instance of s2.
    // If the cast succeeds, push object back, else push NULL.
    const Value& objVal      = pEnv->Top();
    const Value& ctorFuncVal = pEnv->Top1();
    Value rv(Value::NULLTYPE);

    if (ctorFuncVal.IsFunction())
    {
        FunctionRef ctorFunc = ctorFuncVal.ToFunction(pEnv);
        if (!ctorFunc.IsNull())
        {
            ObjectInterface* obj = objVal.ToObjectInterface(pEnv);
            if (obj != 0)
            {
                Value prototypeVal;
                if (ctorFunc->GetMemberRaw(pEnv->GetSC(), pEnv->GetBuiltin(ASBuiltin_prototype), &prototypeVal))
                {
                    Object* prototype = prototypeVal.ToObject(pEnv);
                    if (obj->InstanceOf(pEnv, prototype))
                    {
                        rv.SetAsObjectInterface(obj);
                    }
                }
                else
                {
                    #ifdef GFX_AS2_VERBOSE_ERRORS
                    if (VerboseActionErrors)
                        LogF.LogScriptError("The constructor function in 'cast' should have 'prototype'.");
                    #endif
                }
            }
        }
    }
    else
    {
        #ifdef GFX_AS2_VERBOSE_ERRORS
        if (VerboseActionErrors)
            LogF.LogScriptError("The parameter of 'cast' should be a function.");
        #endif
    }

    pEnv->Drop2();
    pEnv->Push(rv);
}

void ExecutionContext::ImplementsOpCode()
{
    // Declare that a class s1 implements one or more
    // Interfaces (i2 == number of interfaces, s3..Sn are the names
    // of the interfaces).
    Value    ctorFuncVal(pEnv->Top());
    int         intfNum = pEnv->Top1().ToInt32(pEnv);
    pEnv->Drop2();

    if (ctorFuncVal.IsFunction())
    {
        FunctionRef ctorFunc = ctorFuncVal.ToFunction(pEnv);
        if (!ctorFunc.IsNull())
        {
            Value protoVal;
            if (ctorFunc->GetMemberRaw(pEnv->GetSC(), pEnv->GetBuiltin(ASBuiltin_prototype), &protoVal))
            {
                Object* proto = protoVal.ToObject(pEnv);
                if (proto != 0)
                {
                    proto->AddInterface(pEnv->GetSC(), intfNum, NULL);
                    for (int i = 0; i < intfNum; ++i)
                    {
                        const Value& intfVal = pEnv->Top(i);
                        if (intfVal.IsFunction())
                        {
                            FunctionRef intfFunc = intfVal.ToFunction(pEnv);
                            if (!intfFunc.IsNull())
                            {
                                proto->AddInterface(pEnv->GetSC(), i, intfFunc.GetObjectPtr());
                            }
                        }
                    }
                }
            }
            else
            {
                #ifdef GFX_AS2_VERBOSE_ERRORS
                if (VerboseActionErrors)
                    LogF.LogScriptError("The constructor function in 'implements' should have 'prototype'.");
                #endif
            }
        }
    }
    else
    {
        #ifdef GFX_AS2_VERBOSE_ERRORS
        if (VerboseActionErrors)
            LogF.LogScriptError("The parameter of 'implements' should be a function.");
        #endif
    }
    pEnv->Drop(intfNum);
}

void ExecutionContext::EnumerateOpCode(int actionId)
{                   
    const ObjectInterface* piobj     = 0;
    Value                  varName   = pEnv->Top();
    pEnv->Drop1();

    // The end of the enumeration
    Value nullvalue;
    nullvalue.SetNull();
    pEnv->Push(nullvalue);

    if (actionId == 0x55)
    {   
        // For opcode 0x55, top of stack IS the object.                 
        piobj = varName.ToObjectInterface(pEnv);
        if (!piobj)
            return;
    }
    else
    {
        ASString   varString(varName.ToString(pEnv));
        Value    variable;
        if (pEnv->GetVariable(varString, &variable, WithStack))
        {
            piobj = variable.ToObjectInterface(pEnv);
            if (!piobj)
                return;
        }
        else
            return;
    }

    #ifdef GFX_AS2_VERBOSE_ERRORS
    if (VerboseAction) 
        LogF.LogAction("---enumerate - Push: NULL\n");
    #endif

    // Use enumerator class to handle members other then Object.
    // For example, we may enumerate members added to sprite.
    struct EnumerateOpVisitor : public ObjectInterface::MemberVisitor
    {
        Environment*     pEnv;
        ActionLogger*    pLog;

        EnumerateOpVisitor(Environment* penv, ActionLogger* plog)
        { pEnv = penv; pLog = plog; }
        virtual void    Visit(const ASString& name, const Value&, UByte flags)
        {
            SF_UNUSED(flags);
            pEnv->Push(name);
            if (pLog)
                pLog->LogAction("---enumerate - Push: %s\n", name.ToCStr());
        }
    };

    // Visit all members, including prototype & child clips.
    EnumerateOpVisitor memberVisitor(pEnv, &LogF);
    piobj->VisitMembers(pEnv->GetSC(), &memberVisitor, 
        ObjectInterface::VisitMember_Prototype|
        ObjectInterface::VisitMember_ChildClips|
        ObjectInterface::VisitMember_NamesOnly);
}

void ExecutionContext::ExtendsOpCode()
{   
    // Extends actually does the following:
    // Pop(Superclass)
    // Pop(Subclass)
    // Subclass.prototype = new Object();
    // Subclass.prototype.__proto__ = Superclass.prototype;
    // Subclass.prototype.__constructor__ = Superclass;                 
    Value superClassCtorVal = pEnv->Top();
    Value subClassCtorVal = pEnv->Top1();
    FunctionRef superClassCtor = superClassCtorVal.ToFunction(pEnv);
    FunctionRef subClassCtor = subClassCtorVal.ToFunction(pEnv);
    if (!superClassCtor.IsNull() && !subClassCtor.IsNull())
    {
        Value superProtoVal;
        if (!superClassCtor->GetMemberRaw(pEnv->GetSC(), pEnv->GetBuiltin(ASBuiltin_prototype), &superProtoVal) ||
            !superProtoVal.IsObject())
        {
            #ifdef GFX_AS2_VERBOSE_ERRORS
            if (VerboseActionErrors)
                LogF.LogScriptError("can't extend by the class w/o prototype.");
            #endif
        }
        else
        {
            Ptr<Object> superProto = superProtoVal.ToObject(pEnv);
            //!AB: need to create instance of ObjectProto to make sure "implements" and
            // "instanceof" work correctly.
            Ptr<ObjectProto> newSubclassProto = *SF_HEAP_NEW(pEnv->GetHeap()) ObjectProto(pEnv->GetSC(), superProto);
            //Ptr<Object> newSubclassProto = *SF_HEAP_NEW(pEnv->GetHeap()) Object(pEnv->GetSC(), superProto);

            subClassCtor->SetPrototype(pEnv->GetSC(), newSubclassProto);
            newSubclassProto->Set__constructor__(pEnv->GetSC(), superClassCtor);
        }
    }
    else 
    {
        #ifdef GFX_AS2_VERBOSE_ERRORS
        if (VerboseActionErrors)
        {
            if (superClassCtor.IsNull())
            {
                LogF.LogScriptError("Can't extend with unknown super class.");
            }
            else
            {
                LogF.LogScriptError("Can't extend the unknown class.");
            }
        }
        #endif
    }

    pEnv->Drop2();
}

void ExecutionContext::InstanceOfOpCode()
{   
    const Value& ctorFuncVal = pEnv->Top();
    const Value& objVal = pEnv->Top1();
    bool rv = false;

    if (ctorFuncVal.IsFunction())
    {
        FunctionRef ctorFunc = ctorFuncVal.ToFunction(pEnv);
        if (!ctorFunc.IsNull())
        {
            ObjectInterface* obj = objVal.ToObjectInterface(pEnv);
            if (obj != 0)
            {
                Value prototypeVal;
                if (ctorFunc->GetMemberRaw(pEnv->GetSC(), pEnv->GetBuiltin(ASBuiltin_prototype), &prototypeVal))
                {
                    Object* prototype = prototypeVal.ToObject(pEnv);
                    rv = obj->InstanceOf(pEnv, prototype);
                }
                else
                {
                    #ifdef GFX_AS2_VERBOSE_ERRORS
                    if (VerboseActionErrors)
                        LogF.LogScriptError("The constructor function in InstanceOf should have 'prototype'.");
                    #endif
                }
            }
        }
    }
    else
    {
        #ifdef GFX_AS2_VERBOSE_ERRORS
        if (VerboseActionErrors)
            LogF.LogScriptError("The parameter of InstanceOf should be a function.");
        #endif
    }

    pEnv->Drop2();
    pEnv->Push(rv);
}                

void ExecutionContext::WaitForFrameOpCode(ActionBuffer* pActions, int actionId)
{   
    unsigned frame = 0;
    unsigned skipCount = 0;
    Sprite*  ptargetSprite = pEnv->IsTargetValid() ? pEnv->GetTarget()->CharToSprite() : 0;
    bool     frameResolved = false;

    if (actionId == 0x8A )
    {
        frame  = pBuffer[PC + 3] | (pBuffer[PC + 4] << 8);
        skipCount = pBuffer[PC + 5];
        frameResolved = true;
    }
    else
    {
        frameResolved = pActions->ResolveFrameNumber(pEnv, pEnv->Top(), NULL, &frame);
        skipCount = pBuffer[PC + 3];
        pEnv->Drop1();
    }

    if (ptargetSprite && frameResolved)
    {           
        // if frame is bigger than totalFrames - cut it down to totalFrames
        unsigned totalFrames = ptargetSprite->GetFrameCount();
        if (totalFrames > 0 && frame >= totalFrames)
            frame = totalFrames - 1;

        // If we haven't loaded a specified frame yet, then we're supposed to skip
        // some specified number of actions.
        if (frame >= ptargetSprite->GetLoadingFrame())
        {
            // need to calculate the offset by counting actions
            unsigned len = pActions->GetLength();
            unsigned curpc = (unsigned)NextPC;
            for(unsigned i = 0; i < skipCount && curpc < len; ++i)
            {
                UByte cmd = pBuffer[curpc++];
                if (cmd & 0x80) // complex action?
                {
                    // skip the content of action by getting its length
                    curpc += (pBuffer[curpc] | (pBuffer[curpc + 1] << 8));
                    curpc += 2; // skip the length itself
                }
            }
            unsigned nextOffset = curpc;

            // Check ActionBuffer bounds.
            if (nextOffset >= len)
            {
                #ifdef GFX_AS2_VERBOSE_ERRORS
                if (VerboseActionErrors)
                    LogF.LogScriptError(
                    "WaitForFrame branch to offset %d - this section only runs to %d",
                    NextPC, StopPC);
                #endif
            }
            else
            {
                NextPC = (int)nextOffset;
            }
        }
    }

    // Fall through if this frame is loaded.
}

void ExecutionContext::Function1OpCode(ActionBuffer* pActions)
{
    Ptr<AsFunctionObject> funcObj = 
        *SF_HEAP_NEW(pEnv->GetHeap()) AsFunctionObject (pEnv, pActions, NextPC, 0, WithStack);

    int i = PC;
    i += 3;

    // Extract name.
    // @@ security: watch out for possible missing terminator here!
    ASString   name = pEnv->CreateString((const char*) &pBuffer[i]);
    i += (int)name.GetSize() + 1;

    // Get number of arguments.
    int nargs = pBuffer[i] | (pBuffer[i + 1] << 8);
    SF_ASSERT((i + nargs*2) <= (int)pActions->GetLength());
    i += 2;

    // Get the names of the arguments.
    for (int n = 0; n < nargs; n++)
    {
        // @@ security: watch out for possible missing terminator here!
        ASString argstr(pEnv->CreateString((const char*) &pBuffer[i]));
        i += (int)funcObj->AddArg(0, argstr).Name.GetSize() + 1;
    }

    // Get the length of the actual function code.
    int length = pBuffer[i] | (pBuffer[i + 1] << 8);
    i += 2;
    funcObj->SetLength(length);

    // Skip the function Body (don't interpret it now).
    NextPC += length;

    // If we have a name, then save the function in this
    // environment under that name. FunctionValue will AddRef to the function.
    FunctionRef funcRef (funcObj);
    LocalFrame* plocalFrame = pEnv->GetTopLocalFrame ();
    if (plocalFrame)
    {
        funcRef.SetLocalFrame (plocalFrame);
#ifndef GFX_AS_ENABLE_GC
        plocalFrame->LocallyDeclaredFuncs.Set(funcObj.GetPtr(), 0);
#endif
    }
    Value    FunctionValue(funcRef);
    if (!name.IsEmpty())
    {
        // @@ NOTE: should this be Target->SetVariable()???
        //pEnv->SetMember(name, FunctionValue);
        pEnv->GetAvmTarget()->SetMemberRaw(pEnv->GetSC(), name, FunctionValue);
        //pEnv->SetVariable(name, FunctionValue, WithStack);
    }

    // AB, Set prototype property for function
    // The prototype for function is the instance of FunctionProto. 
    // Also, the "constructor" property of this prototype
    // instance should point to GASAsFunction instance (funcDef)
    Ptr<FunctionProto> funcProto = 
        *SF_HEAP_NEW(pEnv->GetHeap()) FunctionProto(pEnv->GetSC(), pEnv->GetPrototype(ASBuiltin_Object), funcRef, false);
    funcRef->SetProtoAndCtor(pEnv->GetSC(), pEnv->GetPrototype(ASBuiltin_Function));
    funcObj->SetPrototype(pEnv->GetSC(), funcProto);

    if (name.IsEmpty())
    {
        // Also leave it on the stack, if function is anonymous
        pEnv->Push(FunctionValue);
    }

#ifdef SF_AMP_SERVER
    if (AmpServer::GetInstance().IsEnabled() && !name.IsEmpty() && AmpServer::GetInstance().GetProfileLevel() >= Amp_Profile_Level_ActionScript)
    {
        // ActionScript profiling
        pEnv->GetMovieImpl()->GetAdvanceStats().RegisterScriptFunction(pActions->GetBufferData()->GetSwdHandle(), 
            pActions->GetBufferData()->GetSWFFileOffset() + funcObj->GetStartPC(), 
            name.ToCStr(), funcObj->GetLength(), 2, false);
    }
#endif  // SF_AMP_SERVER
}

void ExecutionContext::Function2OpCode(ActionBuffer* pActions)
{
    Ptr<AsFunctionObject> funcObj = 
        *SF_HEAP_NEW(pEnv->GetHeap()) AsFunctionObject(pEnv, pActions, NextPC, 0, WithStack, ActionBuffer::Exec_Function2);

    int i = PC;
    i += 3;

    // Extract name.
    // @@ security: watch out for possible missing terminator here!
    ASString   name = pEnv->CreateString((const char*) &pBuffer[i]);
    i += (int)name.GetSize() + 1;

    // Get number of arguments.
    int nargs = pBuffer[i] | (int(pBuffer[i + 1]) << 8);
    i += 2;

    // Get the count of local registers used by this function.
    UByte   RegisterCount = pBuffer[i];
    i += 1;
    funcObj->SetLocalRegisterCount(RegisterCount);

    // Flags, for controlling register assignment of implicit args.
    UInt16  flags = pBuffer[i] | (UInt16(pBuffer[i + 1]) << 8);
    i += 2;
    funcObj->SetFunction2Flags(flags);

    // Get the register assignments and names of the arguments.
    for (int n = 0; n < nargs; n++)
    {
        int ArgRegister = pBuffer[i];
        i++;

        unsigned length = (unsigned)strlen((const char*) &pBuffer[i]);
        SF_ASSERT((length + i) <= pActions->GetLength());
        ASString argstr(pEnv->CreateString((const char*) &pBuffer[i], length));

        // @@ security: watch out for possible missing terminator here!
        i += (int)funcObj->AddArg(ArgRegister, argstr).Name.GetSize() + 1;                    
    }

    // Get the length of the actual function code.
    int length = pBuffer[i] | (pBuffer[i + 1] << 8);
    i += 2;
    funcObj->SetLength(length);

    // Skip the function Body (don't interpret it now).
    NextPC += length;

    // If we have a name, then save the function in this
    // environment under that name. FunctionValue will AddRef to the function.
    FunctionRef funcRef (funcObj);
    LocalFrame* plocalFrame = pEnv->GetTopLocalFrame ();
    if (plocalFrame)
    {
        funcRef.SetLocalFrame (plocalFrame);
        #ifndef GFX_AS_ENABLE_GC
        plocalFrame->LocallyDeclaredFuncs.Set(funcObj.GetPtr(), 0);
        #endif
    }

    Value    FunctionValue(funcRef);
    if (!name.IsEmpty())
    {
        // @@ NOTE: should this be Target->SetVariable()???
        //pEnv->SetMember(name, FunctionValue);
        pEnv->GetAvmTarget()->SetMemberRaw(pEnv->GetSC(), name, FunctionValue);
    }

    // AB, Set prototype property for function
    // The prototype for function is the instance of FunctionProto. 
    // Also, the "constructor" property of this prototype
    // instance should point to GASAsFunction instance (funcDef)
    ASStringContext*   psc = pEnv->GetSC();
    Ptr<FunctionProto> funcProto = 
        *SF_HEAP_NEW(psc->GetHeap()) FunctionProto(psc, pEnv->GetPrototype(ASBuiltin_Object), funcRef, false);
    funcRef->SetProtoAndCtor(psc, pEnv->GetPrototype(ASBuiltin_Function));

    funcObj->SetPrototype(psc, funcProto.GetPtr());

    if (name.IsEmpty())
    {
        // Also leave it on the stack if function is anonymous.
        pEnv->Push(FunctionValue);   
    }

#ifdef SF_AMP_SERVER
    if (AmpServer::GetInstance().IsEnabled() && !name.IsEmpty() && 
        AmpServer::GetInstance().GetProfileLevel() >= Amp_Profile_Level_ActionScript)
    {
        // Actionscript profiling
        pEnv->GetMovieImpl()->GetAdvanceStats().RegisterScriptFunction(pActions->GetBufferData()->GetSwdHandle(), 
            pActions->GetBufferData()->GetSWFFileOffset() + funcObj->GetStartPC(), 
            name.ToCStr(), funcObj->GetLength(), 2, false);
    }
#endif  // SF_AMP_SERVER
}

#ifndef SF_BUILD_DEBUG
#define GASInitBuffer(p)                (void)0
#define GASCheckBuffer(p)               (void)0

#define GASStringConstruct(source,p)    *::new(p) ASString(source)
#define GASStringDeconstruct(r)         (r).~ASString()

#define GASValueConstruct(source,p)     *::new(p) Value(source)
#define GASValueConstruct0(p)           *::new(p) Value()
#define GASValueDeconstruct(r)          (r).~Value()

#define GASFunctionRefConstruct(source,p)    *::new(p) FunctionRef(source)
#define GASFunctionRefDeconstruct(r)         (r).~FunctionRef()

#define GASFnCallConstruct(p1,p2,p3,p4,p5,pbuf)    *::new(pbuf) FnCall(p1,p2,p3,p4,p5)
#define GASFnCallDeconstruct(r)         (r).~FnCall()

#else
static void gfxbufassert(void* p) { SF_ASSERT( *((UInt32*)p) == 0xABCDEF01u); }

#define GASInitBuffer(p)                *((UInt32*)p) = 0xABCDEF01;
#define GASCheckBuffer(p)               gfxbufassert(p)

#define GASStringConstruct(source,p)    (gfxbufassert(p),*::new(p) ASString(source))
#define GASStringDeconstruct(r)         do { (r).~ASString(); *((UInt32*)&r) = 0xABCDEF01; } while(0)

#define GASValueConstruct(source,p)     (gfxbufassert(p),*::new(p) Value(source))
#define GASValueConstruct0(p)           (gfxbufassert(p),*::new(p) Value())
#define GASValueDeconstruct(r)          do { (r).~Value(); *((UInt32*)&r) = 0xABCDEF01; } while(0)

#define GASFunctionRefConstruct(source,p)    (gfxbufassert(p),*::new(p) FunctionRef(source))
#define GASFunctionRefDeconstruct(r)         do { (r).~FunctionRef(); *((UInt32*)&r) = 0xABCDEF01; } while(0)

#define GASFnCallConstruct(p1,p2,p3,p4,p5,pbuf)    (gfxbufassert(pbuf),*::new(pbuf) FnCall(p1,p2,p3,p4,p5))
#define GASFnCallDeconstruct(r)         do { (r).~FnCall(); *((UInt32*)&r) = 0xABCDEF01; } while(0)
#endif

// Interpret the specified subset of the actions in our
// buffer.  Caller is responsible for cleaning up our local
// stack Frame (it may have passed its arguments in via the
// local stack frame).
// 
// The execType option determines whether to use global or local registers.
void    ActionBuffer::Execute(
                        Environment* env,
                        int startPc,
                        int execBytes,
                        Value* retval,
                        const WithStackArray *pinitialWithStack,
                        ExecuteType execType)
{
    SF_ASSERT(env);
    if (env->NeedTermination(execType))
        return;
    int tryCount = 0;
    env->EnteringExecution();

    ExecutionContext execContext(env, pinitialWithStack, execType, GetBufferPtr(), pBufferData->GetFileName());

    bool isOriginalTargetValid = env->IsTargetValid();

    #ifdef GFX_AS2_VERBOSE
    if (execContext.VerboseAction) 
        execContext.LogF.LogAction("\n");
    #endif

    execContext.StopPC = startPc + Alg::Min(execBytes, (int)GetLength());
    execContext.NextPC = startPc;
    execContext.PC     = startPc;

    // local variables used inside the 'switch' below.
    // DO NOT declare local variables inside the 'case-break' if possible.
    // This cause Execute to be very greedy for the stack size. It is 
    // critical for nested calls, since every call of ActionScript function
    // may consume up to 4K of stack, if local variables are declared inside
    // 'case-break' statements.
    Ptr<Object> pobj;
    union
    {
        UInt64  timerMs;
        double  nargsf;
        char    buf[8];
        wchar_t wbuf[4];
    };
    char tmpStr1Buf[sizeof(ASString)];
    char tmpStr2Buf[sizeof(ASString)];
    char valBuf1[sizeof(Value)];
    char valBuf2[sizeof(Value)];
    union
    {
        #ifdef GFX_AS2_VERBOSE
        char tmpStr3Buf[sizeof(ASString)];
        #endif
        char valBuf3[sizeof(Value)];
    };
    char funcBuf[sizeof(FunctionRef)];
    char fnCallBuf[sizeof(FnCall)];

    GASInitBuffer(tmpStr1Buf);
    GASInitBuffer(tmpStr2Buf);
    #ifdef GFX_AS2_VERBOSE
    GASInitBuffer(tmpStr3Buf);
    #endif
    GASInitBuffer(valBuf1);
    GASInitBuffer(valBuf2);
    GASInitBuffer(valBuf3);
    GASInitBuffer(funcBuf);
    GASInitBuffer(fnCallBuf);

#ifdef SF_AMP_SERVER
    ArrayLH<UInt64>* bufferTimes = NULL;
    AMP::ViewStats& viewStats = env->GetMovieImpl()->GetAdvanceStats();
    UInt32 samplingPeriod = 0;
    bool shouldProfile = AmpServer::GetInstance().IsProfiling();
    if (AmpServer::GetInstance().IsInstructionSampling())
    {
        samplingPeriod = 50;
    }

    // Function profiling
    AMP::ScopeFunctionTimer funcTimer(shouldProfile ? &viewStats : NULL, pBufferData->GetSwdHandle(), 
        startPc + pBufferData->GetSWFFileOffset(), Amp_Profile_Level_ActionScript);

    if (shouldProfile)
    {
        if (AmpServer::GetInstance().IsInstructionProfiling())
        {
            bufferTimes = &viewStats.LockBufferInstructionTimes(
                pBufferData->GetSwdHandle(), 
                pBufferData->GetSWFFileOffset(), 
                pBufferData->GetLength());

            viewStats.GetInstructionTime(samplingPeriod);
        }
    }

#endif  // SF_AMP_SERVER

    while (execContext.PC < execContext.StopPC)
    {
        // Cleanup any expired "with" blocks.
        if (execContext.WithStack.pWithStackArray)
        {
            UPInt i, n = execContext.WithStack.pWithStackArray->GetSize();
            for (i = 0; i < n && execContext.PC >= execContext.WithStack.pWithStackArray->Back().GetBlockEndPc(); ++i)
                ;
            if (i > 0)
                execContext.WithStack.pWithStackArray->Resize(n - i);
        }

        // Get the opcode.
        int actionId = execContext.pBuffer[execContext.PC];

#ifdef GFXACTION_COUNT_OPS
        Execute_OpCounters[actionId]++;
#endif

        if ((actionId & 0x80) == 0)
        {
            execContext.NextPC = execContext.PC + 1;   // advance to next action.

            #ifdef GFX_AS2_VERBOSE
            if (execContext.VerboseAction) 
            {
                execContext.LogF.LogAction("EX: %4.4X\t", execContext.PC);
                execContext.LogF.LogDisasm(&execContext.pBuffer[execContext.PC]);
            }
            #endif

            // Simple action; no extra data.
            switch (actionId)
            {
                default:
                    break;

                case 0x00:  // end of actions.
                    execContext.NextPC = execContext.StopPC;
                    tryCount = 0; // prevents from executing "finally" blocks since if it is
                                  // normal termination (end-of-action code means "normal termination")
                                  // then all "finally" blocks should be already executed.
                    break;

                case 0x04:  // next frame.
                    if (env->IsTargetValid())
                        env->GetTarget()->GotoFrame(env->GetTarget()->GetCurrentFrame() + 1);
                    // check if it is necessary to terminate the execution, because the target 
                    // is unloaded or unloading. See comments for Bmination for more details.
                    if (env->NeedTermination(execType))
                        execContext.NextPC = execContext.StopPC;
                    break;

                case 0x05:  // prev frame.
                    if (env->IsTargetValid())
                        env->GetTarget()->GotoFrame(env->GetTarget()->GetCurrentFrame() - 1);
                    // check if it is necessary to terminate the execution, because the target 
                    // is unloaded or unloading. See comments for NeedTermination for more details.
                    if (env->NeedTermination(execType))
                        execContext.NextPC = execContext.StopPC;
                    break;

                case 0x06:  // action play
                    if (env->IsTargetValid())
                        env->GetTarget()->SetPlayState(GFx::State_Playing);
                    break;

                case 0x07:  // action stop
                    if (env->IsTargetValid())
                        env->GetTarget()->SetPlayState(GFx::State_Stopped);
                    break;

                case 0x08:  // toggle quality
                    break;

                case 0x09:  // stop sounds
                    {
#ifdef GFX_ENABLE_SOUND
                        MovieRoot* proot = env->GetAS2Root();
                        if (proot)
                        {
                            Sprite* root = proot->GetLevelMovie(0);
                            if (root)
                                root->StopActiveSounds();
                        }
#endif
                    }
                    break;

                case 0x0A:  // add
                    env->Top1().Add(env, env->Top());
                    env->Drop1();
                    break;

                case 0x0B:  // subtract
                    env->Top1().Sub(env, env->Top());
                    env->Drop1();
                    break;

                case 0x0C:  // multiply
                    env->Top1().Mul(env, env->Top());
                    env->Drop1();
                    break;

                case 0x0D:  // divide
                    env->Top1().Div(env, env->Top());
                    env->Drop1();
                    break;

                case 0x0E:  // equal
                    env->Top1().SetBool(env->Top1().IsEqual (env, env->Top()));
                    env->Drop1();
                    break;

                case 0x0F:  // less than
                case 0x48:  // less Than (typed) (should they be implemented the same way?)
                    {
                        Value& result = GASValueConstruct(env->Top1().Compare (env, env->Top(), -1), valBuf1);
                        env->Top1() = result;
                        env->Drop1();
                        GASValueDeconstruct(result);
                    }
                    break;

                case 0x10:  // logical and
                    env->Top1().SetBool(env->Top1().ToBool(env) && env->Top().ToBool(env));
                    env->Drop1();
                    break;

                case 0x11:  // logical or
                    env->Top1().SetBool(env->Top1().ToBool(env) || env->Top().ToBool(env));
                    env->Drop1();
                    break;

                case 0x12:  // logical not
                    {
                        Value& topVal = env->Top();
                        // LOgical NOT is a very frequently executed op (used to invert conditions 
                        // for branching?). Optimize it for most common values.                    
                        if (topVal.GetType() == Value::BOOLEAN)
                        {
                            // Here, BOOLEAN is by far the most common type.
                            topVal.V.BooleanValue = !topVal.V.BooleanValue;
                        }
                        else  if (topVal.IsUndefined())
                        {
                            topVal.T.Type = Value::BOOLEAN;
                            topVal.V.BooleanValue = true;
                        }
                        else 
                        {
                            topVal.SetBool(!topVal.ToBool(env));
                        }
                        break;
                    }

                case 0x13:  // string equal
                {
                    ASString& s1 = GASStringConstruct(env->Top1().ToStringVersioned(env, execContext.Version), tmpStr1Buf);
                    ASString& s0 = GASStringConstruct(env->Top().ToStringVersioned(env, execContext.Version), tmpStr2Buf);
                    env->Top1().SetBool(s1 == s0);
                    env->Drop1();
                    GASStringDeconstruct(s1);
                    GASStringDeconstruct(s0);
                    break;
                }

                case 0x14:  // string length
                {
                    ASString& s1 = GASStringConstruct(env->Top().ToStringVersioned(env, execContext.Version), tmpStr1Buf);
                    env->Top().SetInt(s1.GetLength());
                    GASStringDeconstruct(s1);
                    break;
                }

                case 0x15:  // substring
                {
                    ASString& s2 = GASStringConstruct(env->Top(2).ToStringVersioned(env, execContext.Version), tmpStr1Buf);
                    ASString& retVal = GASStringConstruct(StringProto::StringSubstring(
                                        s2,
                                        env->Top1().ToInt32(env) - 1,
                                        env->Top().ToInt32(env) ), tmpStr2Buf);
                    env->Drop2();
                    env->Top().SetString(retVal);
                    GASStringDeconstruct(s2);
                    GASStringDeconstruct(retVal);
                    break;
                }

                case 0x17:  // pop
                    //!AB, here is something I can't understand for now: if class is
                    // declared as com.package.Class then AS code contains
                    // creation of "com" and "com.package" objects as "new Object".
                    // But, the problem is in one extra "pop" command after that, when
                    // the stack is already empty. Thus, I commented out the assert below
                    // for now....
                    //SF_ASSERT(env->GetTopIndex() >= 0); //!AB
                    env->Drop1();
                    break;

                case 0x18:  // int
                    env->Top().SetInt(env->Top().ToInt32(env));
                    break;

                case 0x1C:  // get variable
                    {                    
                        // Use a reference to Top() here for efficiency to avoid value assignment overhead,
                        // as this is a very frequently executed op. This value is overwritten by the result.
                        Value&   variable = env->Top();
                        ASString&  varString = GASStringConstruct(variable.ToString(env), tmpStr1Buf);

                        // ATTN: Temporary hack to recognize "NaN", "Infinity" and "-Infinity".
                        // We want this to be as efficient as possible, so do a separate branch for case sensitivity.
                        if (env->IsCaseSensitive())
                        {
                            if (varString == env->GetBuiltin(ASBuiltin_NaN))
                                variable.SetNumber(NumberUtil::NaN());
                            else if (varString == env->GetBuiltin(ASBuiltin_Infinity))
                                variable.SetNumber(NumberUtil::POSITIVE_INFINITY());
                            else if (varString == env->GetBuiltin(ASBuiltin_minusInfinity_)) // "-Infinity"
                                variable.SetNumber(NumberUtil::NEGATIVE_INFINITY());
                            else
                            {
                                variable.SetUndefined();
                                env->GetVariable(varString, &variable, execContext.WithStack);
                            }
                        }
                        else
                        {
                            if (env->GetBuiltin(ASBuiltin_NaN).CompareBuiltIn_CaseInsensitive(varString))
                                variable.SetNumber(NumberUtil::NaN());
                            else if (env->GetBuiltin(ASBuiltin_Infinity).CompareBuiltIn_CaseInsensitive(varString))
                                variable.SetNumber(NumberUtil::POSITIVE_INFINITY());
                            else if (env->GetBuiltin(ASBuiltin_minusInfinity_).CompareBuiltIn_CaseInsensitive(varString))
                                variable.SetNumber(NumberUtil::NEGATIVE_INFINITY());
                            else
                            {
                                variable.SetUndefined();
                                env->GetVariable(varString, &variable, execContext.WithStack);
                            }
                        }

                        #ifdef GFX_AS2_VERBOSE
                        if (execContext.VerboseAction) 
                        {    
                            ASString&  s0 = GASStringConstruct(variable.ToDebugString(env), tmpStr2Buf);
                            if (!variable.ToObjectInterface(env))
                            {
                                execContext.LogF.LogAction("-- get var: %s=%s\n",
                                    varString.ToCStr(), s0.ToCStr());
                            }
                            else
                            {
                                execContext.LogF.LogAction("-- get var: %s=%s at %p\n",
                                    varString.ToCStr(), s0.ToCStr(),
                                    variable.ToObjectInterface(env));
                            }
                            GASStringDeconstruct(s0);
                        }
                        #endif

                        GASStringDeconstruct(varString);
                        break;
                    }

                case 0x1D:  // set variable
                    {
                        ASString&  varString = GASStringConstruct(env->Top1().ToString(env), tmpStr1Buf);
                        env->SetVariable(varString, env->Top(), execContext.WithStack);
                        
                        #ifdef GFX_AS2_VERBOSE
                        if (execContext.VerboseAction) 
                        {
                            ASString&  s0 = GASStringConstruct(env->Top1().ToDebugString(env), tmpStr2Buf);
                            execContext.LogF.LogAction("-- set var: %s \n", s0.ToCStr());
                            GASStringDeconstruct(s0);
                        }
                        #endif

                        env->Drop2();
                        GASStringDeconstruct(varString);
                        break;
                    }

                case 0x20:  // set target expression (used with tellTarget)
                    execContext.SetTargetOpCode();
                    break;

                case 0x21:  // string concat
                    {
                        env->Top1().ConvertToStringVersioned(env, execContext.Version);
                        ASString&  str = GASStringConstruct(env->Top().ToStringVersioned(env, execContext.Version), tmpStr1Buf);
                        env->Top1().StringConcat(env, str);
                        env->Drop1();
                        GASStringDeconstruct(str);
                    }
                    break;

                case 0x22:  // get property
                {
                    InteractiveObject* ptarget = env->FindTargetByValue(env->Top1());
                    if (ptarget)
                    {
                        // Note: GetStandardMember does bounds checks for opcodes.
                        ToAvmCharacter(ptarget)->GetStandardMember
                            ((AvmCharacter::StandardMember) env->Top().ToInt32(env), &env->Top1(), 1);
                    }
                    else
                    {
                        env->Top1().SetUndefined();
                    }
                    env->Drop1();
                    break;
                }

                case 0x23:  // set property
                {
                    InteractiveObject* ptarget = env->FindTargetByValue(env->Top(2));
                    if (ptarget)
                    {
                        // Note: SetStandardMember does bounds checks for opcodes.
                        ToAvmCharacter(ptarget)->SetStandardMember
                            ((AvmCharacter::StandardMember) env->Top1().ToInt32(env), env->Top(), 1);
                    }
                    env->Drop3();
                    break;
                }

                case 0x24:  // duplicate Clip (sprite?)
                {
                    // MA: The target object can be a string, a path, or an object. 
                    // The duplicated clip will be created within its parent.
                    InteractiveObject* ptarget = env->FindTargetByValue(env->Top(2));
                    if (ptarget)
                    {
                        // !AB: we don't need to add 16384 to the depth, like
                        // we do for MovieClip.duplicateMovieClip method, since
                        // this value already has been added: Flash generates "add_t 16384"
                        // for global function duplicateMovieClip call.
                        ASString&  str = GASStringConstruct(env->Top1().ToString(env), tmpStr1Buf);
                        ToAvmCharacter(ptarget)->CloneDisplayObject(
                            str,
                            env->Top().ToInt32(env), 0);
                        GASStringDeconstruct(str);
                    }
                    env->Drop3();
                    break;
                }

                case 0x25:  // remove clip
                {
                    InteractiveObject* ptarget = env->FindTargetByValue(env->Top());
                    if (ptarget)
                    {
                        if (ptarget->GetDepth() < 16384)
                        {
                            execContext.LogF.LogScriptWarning("removeMovieClip(\"%s\") failed - depth must be >= 0",
                                ptarget->GetName().ToCStr());
                        }
                        else
                            ptarget->RemoveDisplayObject();
                    }
                    env->Drop1();
                    break;
                }

                case 0x26:  // trace
                {
                    // LogF the stack val.
                    const FnCall& fnCall = GASFnCallConstruct(&env->Top(), NULL, env, 1, env->GetTopIndex(), fnCallBuf);
                    GAS_GlobalTrace(fnCall);
                    env->Drop1();
                    GASFnCallDeconstruct(fnCall);
                    break;
                }

                case 0x27:  // start drag InteractiveObject
                    execContext.StartDragOpCode();
                    break;

                case 0x28:  // stop drag InteractiveObject
                    {
                        MovieImpl* proot = env->GetTarget()->GetMovieImpl();
                        SF_ASSERT(proot);

                        proot->StopDrag(0);
                        break;
                    }

                case 0x29:  // string less than
                    {
                        ASString& s1 = GASStringConstruct(env->Top1().ToStringVersioned(env, execContext.Version), tmpStr1Buf);
                        ASString& s0 = GASStringConstruct(env->Top().ToStringVersioned(env, execContext.Version), tmpStr2Buf);
                        env->Top1().SetBool(s1 < s0);
                        env->Drop1();
                        GASStringDeconstruct(s0);
                        GASStringDeconstruct(s1);
                        break;
                    }

                case 0x2A:  // throw
                {
                    env->CheckTryBlocks(execContext.PC, &tryCount);
                    env->Throw(env->Top());
                    env->Drop1();
                    
                    execContext.NextPC = env->CheckExceptions(this, 
                        execContext.NextPC, 
                        &tryCount, 
                        retval,
                        execContext.WithStack,
                        execType);
                    break;
                }

                case 0x2B:  // CastObject
                    execContext.CastObjectOpCode();
                    break;

                case 0x2C:  // implements
                    execContext.ImplementsOpCode();
                    break;

                case 0x30:  // random
                {
                    SInt32 max = env->Top().ToInt32(env);
                    if (max < 1) 
                        max = 1;
                    env->Top().SetInt(Math::GetNextRandom(env->GetMovieImpl()) % max);
                    break;
                }

                case 0x31:  // mb length
                {
                    // Must use execContext.Version check here, for SWF <= 6, "undefined" length is 0.              
                    ASString& s = GASStringConstruct(env->Top().ToStringVersioned(env, execContext.Version), tmpStr1Buf);
                    unsigned length = s.GetLength();
                    env->Top().SetInt(length);
                    GASStringDeconstruct(s);
                    break;
                }

                case 0x32:  // ord
                {
                    // ASCII code of first DisplayObject
                    ASString& s = GASStringConstruct(env->Top().ToString(env), tmpStr1Buf);
                    env->Top().SetInt(s[0]);
                    GASStringDeconstruct(s);
                    break;
                }

                case 0x33:  // chr
                    buf[0] = char(env->Top().ToInt32(env));
                    buf[1] = 0;
                    env->Top().SetString(env->CreateString(buf));
                    break;

                case 0x34:  // get timer
                    // Push milliseconds since we started playing.
                    timerMs = env->GetTarget()->GetMovieImpl()->GetASTimerMs();
                    env->Push((Number)timerMs);
                    break;

                case 0x35:  // mbsubstring
                {
                    ASString& s2     = GASStringConstruct(env->Top(2).ToStringVersioned(env, execContext.Version), tmpStr1Buf);
                    ASString& retVal = GASStringConstruct(StringProto::StringSubstring(
                        s2,
                        env->Top1().ToInt32(env) - 1,
                        env->Top().ToInt32(env) ), tmpStr2Buf);
                    env->Drop2();
                    env->Top().SetString(retVal);
                    GASStringDeconstruct(s2);
                    GASStringDeconstruct(retVal);
                    break;
                }

                case 0x36: // mbord
                {
                    // Convert first character to its numeric value.
                    // MA: This is correct for SWF6+ versions, but there is also some strange rounding
                    // that occurs if SWF 5 is selected. (i.e. mbord(mbchr(450)) returns 1). TBD.
                    ASString& s = GASStringConstruct(env->Top().ToStringVersioned(env, execContext.Version), tmpStr1Buf);
                    // Note: GetUTF8CharAt does bounds check, so return 0 to avoid accessing empty string.
                    env->Top().SetInt( (s.GetSize() == 0) ? 0 : s.GetCharAt(0) );
                    GASStringDeconstruct(s);
                    break;
                }

                case 0x37:  // mbchr
                    // Convert number to a multi-byte aware character string
                    wbuf[0] = (wchar_t)env->Top().ToUInt32(env);
                    wbuf[1] = 0;
                    env->Top().SetString(env->CreateString(wbuf));
                    break;

                case 0x3A:  // delete
                {                   
                    // This is used to remove properties from an object.
                    // AB: For Flash 6, if Top1 is null or undefined then delete works by the same way
                    // as delete2
                    bool retVal = false;
                    const Value& top1 = env->Top1();
                    const ASString& memberName = GASStringConstruct(env->Top().ToString(env), tmpStr1Buf);
                    if (execContext.Version <= 6 && (top1.IsNull() || top1.IsUndefined()))
                    {
                        Value& ownerVal = GASValueConstruct0(valBuf1);
                        if (env->FindOwnerOfMember(memberName, &ownerVal, execContext.WithStack))
                        {
                            ObjectInterface *piobj = ownerVal.ToObjectInterface(env);
                            if (piobj)
                            {                                       
                                retVal = piobj->DeleteMember(env->GetSC(), memberName);
                            }
                        }
                        GASValueDeconstruct(ownerVal);
                    }
                    else
                    {
                        ObjectInterface* piobj = env->Top1().ToObjectInterface(env);
                        if (piobj)
                            retVal = piobj->DeleteMember(env->GetSC(), memberName);
                    }
                    // Delete pops two parameters and push the return value (bool) into the stack.
                    // Usually it is removed by explicit "pop" operation, if return value is not used.
                    env->Drop1();
                    env->Top().SetBool(retVal);
                    GASStringDeconstruct(memberName);
                    break;
                }

                case 0x3B:  // delete2
                {
                    ASString& memberName     = GASStringConstruct(env->Top().ToString(env), tmpStr1Buf);
                    Value& ownerVal          = GASValueConstruct0(valBuf1);
                    ObjectInterface *piobj   = NULL;

                    // !AB: 'delete' may be invoked with string parameter containing a whole
                    // path to the member being deleted: delete "obj.mem1.mem2". Or,
                    // it might be used with eval: delete eval("/anObject:member")
                    bool found = false;
                    if (memberName.IsNotPath() || !env->IsPath(memberName))
                    {
                        found = env->FindOwnerOfMember(memberName, &ownerVal, execContext.WithStack);
                    }
                    else
                    {
                        // In the case, if path is specified we need to resolve it,
                        // get the owner and modify the memberName to be relative to the
                        // found owner.
                        bool retVal = env->FindVariable(
                            Environment::GetVarParams(memberName, NULL, execContext.WithStack, NULL, &ownerVal), 
                            false, &memberName);
                        found = (retVal && !ownerVal.IsUndefined());
                    }

                    if (found)
                    {
                        piobj = ownerVal.ToObjectInterface(env);
                        if (piobj)
                        {                                       
                            env->Top().SetBool(piobj->DeleteMember(env->GetSC(), memberName));
                        }
                    }
                    if (!piobj)
                    {
                        env->Top().SetBool(false);
                    }
                    GASStringDeconstruct(memberName);
                    GASValueDeconstruct(ownerVal);
                    break;
                }

                case 0x3C:  // set local
                {                 
                    ASString& varname = GASStringConstruct(env->Top1().ToString(env), tmpStr1Buf);

                    // This is used in 'var' declarations.
                    // For functions, the value becomes local; for frame actions and events, it is global to clip.
                    if (execContext.IsFunction2 || (execType == ActionBuffer::Exec_Function))
                        env->SetLocal(varname, env->Top());
                    else
                        env->SetVariable(varname, env->Top(), execContext.WithStack);

                    env->Drop2();
                    GASStringDeconstruct(varname);
                    break;
                }

                case 0x3D:  // call function
                {
                    Value*   pfunction = &GASValueConstruct0(valBuf1);
                    Value&   owner     = GASValueConstruct0(valBuf2);
                    bool        invokeFunction = true; 

                    if (env->Top().GetType() == Value::STRING)
                    {
                        // Function is a string; lookup the function.
                        const ASString& functionName = GASStringConstruct(env->Top().ToString(env), tmpStr1Buf);
                        env->GetVariable(functionName, pfunction, execContext.WithStack, 0, &owner);

                        if (!pfunction->IsFunction())
                        {
                            if (pfunction->IsObject() && (pobj = pfunction->ToObject(env)) && pobj->IsSuper())
                            {
                                // special case for "super": call to super.__constructor__ (SWF6+)
                                GASValueDeconstruct(*pfunction);

                                FunctionRef& ctor = GASFunctionRefConstruct(pobj->Get__constructor__(env->GetSC()), funcBuf);
                                pfunction = &GASValueConstruct(ctor, valBuf1);
                                GASFunctionRefDeconstruct(ctor);

                                //printf ("!!! function = %s\n", (const char*)env->GetGC()->FindClassName(function.ToObject()));
                                owner.SetAsObject(pobj);
                            }
                            else
                            {
                                #ifdef GFX_AS2_VERBOSE_ERRORS
                                if (execContext.VerboseActionErrors)
                                    execContext.LogF.LogScriptError("CallFunction - '%s' is not a function",
                                        functionName.ToCStr());
                                #endif
                                invokeFunction=false;
                            }
                            pobj = NULL; // dropref
                        }
                        GASStringDeconstruct(functionName);
                    }
                    else
                    {
                        // Hopefully the actual function object is here.
                        *pfunction = env->Top();
                    }

            
                    nargsf = env->Top1().ToNumber(env);
                    int nargs = (!NumberUtil::IsNaNOrInfinity(nargsf)) ? (int)nargsf : 0;
                    SF_ASSERT(nargs >= 0);

                    Value&   result    = GASValueConstruct0(valBuf3);
                    if (invokeFunction)
                    {           
                        //!AB, Sometimes methods can be invoked as functions. For example,
                        // a movieclip "mc" has the method "foo" (mc.foo). If this method
                        // is invoked from "mc"'s timeline as "foo();" (not "this.foo();"!)
                        // then call_func opcode (0x3D) will be generated instead of call_method (0x52).
                        // In this case, "this" should be set to current target ("mc").
                        // Similar issue exists, if method is invoked from "with" statement:
                        // with (mc) { foo(); } // inside foo "this" should be set to "mc"
                        ObjectInterface *ownerObj = owner.ToObjectInterface(env);
                        //GAS_Invoke(function, &result, ownerObj, env, nargs, env->GetTopIndex() - 2, NULL);
                        FunctionRef& func = GASFunctionRefConstruct(pfunction->ToFunction(env), funcBuf);
                        if (func != NULL)
                        {
                            const FnCall& fnCall = GASFnCallConstruct(&result, ownerObj, env, nargs, env->GetTopIndex() - 2, fnCallBuf);
                            func.Function->Invoke(fnCall, func.pLocalFrame, NULL);
                            GASFnCallDeconstruct(fnCall);
                        }
                        else
                        {           
                            if (env && env->IsVerboseActionErrors())
                                env->LogScriptError("CallFunction - attempt to call invalid function");
                        }
                        GASFunctionRefDeconstruct(func);
                    }
                    env->Drop(nargs + 1);
                    env->Top() = result;
                    GASValueDeconstruct(owner);
                    GASValueDeconstruct(*pfunction);
                    GASValueDeconstruct(result);

                    if (env->IsThrowing())
                    {
                        // exception handling
                        env->CheckTryBlocks(execContext.PC, &tryCount);
                        execContext.NextPC = env->CheckExceptions(this, 
                            execContext.NextPC, 
                            &tryCount, 
                            retval,
                            execContext.WithStack,
                            execType);
                    }
                    // check if it is necessary to terminate the execution, because the target 
                    // is unloaded or unloading. See comments for NeedTermination for more details.
                    if (env->NeedTermination(execType))
                        execContext.NextPC = execContext.StopPC;
                    break;
                }

                case 0x3E:  // return
                    // Put top of stack in the provided return slot, if
                    // it's not NULL.
                    if (retval)
                    {
                        *retval = env->Top();
                    }
                    env->Drop1();

                    // Skip the rest of this execContext.pBuffer (return from this ActionBuffer).
                    execContext.NextPC = execContext.StopPC;

                    break;

                case 0x3F:  // modulo
                {
                    Number   result;
                    Number   y = env->Top().ToNumber(env);
                    Number   x = env->Top1().ToNumber(env);
                    if (y != 0)
                        result = (Number) fmod(x, y);
                    else
                        result = NumberUtil::NaN();

                    env->Drop2();
                    env->Push(result);
                    break;
                }
                case 0x40:  // new
                {                    
                    const ASString& classname = GASStringConstruct(env->Top().ToString(env), tmpStr1Buf);

                    nargsf = env->Top1().ToNumber(env);
                    int nargs = (!NumberUtil::IsNaNOrInfinity(nargsf)) ? (int)nargsf : 0;
                    SF_ASSERT(nargs >= 0);

                    env->Drop2();

                    #ifdef GFX_AS2_VERBOSE
                    if (execContext.VerboseAction) 
                        execContext.LogF.LogAction("---new object: %s\n", classname.ToCStr());
                    #endif
                    
                    pobj = NULL; //reset
                    Value& constructor = GASValueConstruct0(valBuf1);

                    if (env->GetVariable(classname, &constructor, execContext.WithStack) && constructor.IsFunction())
                    {
                        FunctionRef& func = GASFunctionRefConstruct(constructor.ToFunction(env), funcBuf);
                        SF_ASSERT (!func.IsNull ());

                        pobj = *env->OperatorNew(func, nargs);

                        if (!pobj)
                        {
                            #ifdef GFX_AS2_VERBOSE_ERRORS
                            if (execContext.VerboseActionErrors)
                                execContext.LogF.LogScriptError("Can't create object with unknown class '%s'",
                                                   classname.ToCStr());
                            #endif
                        }
                        GASFunctionRefDeconstruct(func);
                    }
                    else
                    {
                        #ifdef GFX_AS2_VERBOSE_ERRORS
                        if (execContext.VerboseActionErrors)
                            execContext.LogF.LogScriptError("Can't create object with unknown class '%s'",
                                               classname.ToCStr());
                        #endif
                    }

                    env->Drop(nargs);
                    if (pobj)
                        env->Push(pobj);
                    else
                    {
                        Value& empty = GASValueConstruct0(valBuf2);
                        env->Push(empty);
                        GASValueDeconstruct(empty);
                    }
                    
                    #ifdef GFX_AS2_VERBOSE
                    if (execContext.VerboseAction)
                        execContext.LogF.LogAction("New object %s at %p\n", classname.ToCStr(), pobj.GetPtr());
                    #endif
                    pobj = NULL; //dropref
                    GASValueDeconstruct(constructor);
                    GASStringDeconstruct(classname);
                    
                    if (env->IsThrowing())
                    {
                        // exception handling
                        env->CheckTryBlocks(execContext.PC, &tryCount);
                        execContext.NextPC = env->CheckExceptions(this, 
                            execContext.NextPC, 
                            &tryCount, 
                            retval,
                            execContext.WithStack,
                            execType);
                    }
                    // check if it is necessary to terminate the execution, because the target 
                    // is unloaded or unloading. See comments for NeedTermination for more details.
                    if (env->NeedTermination(execType))
                        execContext.NextPC = execContext.StopPC;
                    break;
                }

                case 0x41:  // declare local
                {
                    const ASString& varname = GASStringConstruct(env->Top().ToString(env), tmpStr1Buf);
                    // This is used in 'var' declarations.
                    // For functions, the value becomes local; for frame actions and events, it is global to clip.
                    if (execContext.IsFunction2 || (execType == ActionBuffer::Exec_Function))                
                        env->DeclareLocal(varname);
                    env->Drop1();
                    GASStringDeconstruct(varname);
                    break;
                }

                case 0x42:  // init array
                {               
                    int arraySize = (int) env->Top().ToNumber(env);
                    env->Drop1();

                    // Call the array constructor with the given elements as args
                    Value&   result = GASValueConstruct0(valBuf1);
                    const FnCall& fnCall = GASFnCallConstruct(&result, NULL, env, arraySize, env->GetTopIndex(), fnCallBuf);
                    ArrayCtorFunction::DeclareArray(fnCall);
                    GASFnCallDeconstruct(fnCall);

                    if (arraySize > 0) 
                    {
                        env->Drop(arraySize);
                    }
                    // Push array 
                    env->Push(result);
                    GASValueDeconstruct(result);
                    break;
                }

                case 0x43:  // declare object
                {
                    // Use an initialize list to build an object.
                    int argCount    = (int) env->Top().ToNumber(env);
                    pobj            = *env->OperatorNew(env->GetBuiltin(ASBuiltin_Object));
                    env->Drop1();
                    
                    if (pobj)
                    {
                        for(int i = 0; i < argCount; i++)
                        {
                            // Pop {value, name} pairs.
                            if (env->GetTopIndex() >=1)
                            {
                                // Validity/null check.
                                if (env->Top1().GetType() == Value::STRING)
                                {
                                    const ASString& s = GASStringConstruct(env->Top1().ToString(env), tmpStr1Buf);
                                    pobj->SetMember(env, s, env->Top());
                                    GASStringDeconstruct(s);
                                }
                                env->Drop2();
                            }
                        }
                    }

                    env->Push(pobj.GetPtr());
                    pobj = NULL; // drop ref

                    if (env->IsThrowing())
                    {
                        // exception handling
                        env->CheckTryBlocks(execContext.PC, &tryCount);
                        execContext.NextPC = env->CheckExceptions(this, 
                            execContext.NextPC, 
                            &tryCount, 
                            retval,
                            execContext.WithStack,
                            execType);
                    }
                    // check if it is necessary to terminate the execution, because the target 
                    // is unloaded or unloading. See comments for NeedTermination for more details.
                    if (env->NeedTermination(execType))
                        execContext.NextPC = execContext.StopPC;
                    break;
                }
                    
                case 0x44:  // type of
                {
                    ASBuiltinType typeStrIndex = ASBuiltin_undefined;
                    const Value& top = env->Top();
                    switch(top.GetType())
                    {
                        case Value::UNDEFINED:
                        case Value::UNSET:
                            //typeStrIndex = GASBuiltin_undefined;                            
                            break;
                        case Value::STRING:
                            typeStrIndex = ASBuiltin_string;
                            break;
                        case Value::BOOLEAN:
                            typeStrIndex = ASBuiltin_boolean;
                            break;
                        case Value::CHARACTER:
                            {                           
                                InteractiveObject* pchar = env->Top().ToCharacter(env);
                                // ActionScript returns "movieclip" for null clips and sprite types only.
                                if (!pchar || (pchar && pchar->IsSprite()))
                                    typeStrIndex = ASBuiltin_movieclip;
                                else
                                    typeStrIndex = ASBuiltin_object;
                            }
                            break;

                        case Value::OBJECT:
                            typeStrIndex = ASBuiltin_object;
                            break;
                        case Value::NULLTYPE:
                            typeStrIndex = ASBuiltin_null;
                            break;
                        case Value::FUNCTION:
                            typeStrIndex = ASBuiltin_function;
                            break;
                        default:
                            if (top.IsNumber())
                                typeStrIndex = ASBuiltin_number;
                            else
                            {
                                #ifdef GFX_AS2_VERBOSE_ERRORS
                                if (execContext.VerboseActionErrors)
                                    execContext.LogF.LogScriptError("typeof unknown - %02X", env->Top().GetType());
                                #endif
                            }
                            break;
                        }

                    env->Top().SetString(env->GetBuiltin(typeStrIndex));
                    break;
                }

                case 0x45:  // get target path
                {
                    InteractiveObject* ptarget = env->Top().ToCharacter(env);
                    if (ptarget)             
                        env->Top().SetString(ptarget->GetCharacterHandle()->GetNamePath());
                    else
                        env->Top().SetUndefined();
                    break;
                }

                case 0x46:  // enumerate
                case 0x55:  // enumerate object 2
                    execContext.EnumerateOpCode(actionId);
                    break;

                case 0x47:  // AddT (typed)
                    env->Top1().Add (env, env->Top());
                    env->Drop1();
                    break;

                //case 0x48:  // less Than (typed)
                // see case 0x0F:

                case 0x49:  // Equal (typed)
                    // @@ identical to untyped equal, as far as I can tell...
                    env->Top1().SetBool(env->Top1().IsEqual (env, env->Top()));
                    env->Drop1();
                    break;

                case 0x4A:  // to number
                    env->Top().ConvertToNumber(env);
                    break;

                case 0x4B:  // to string
                    env->Top().ConvertToStringVersioned(env, execContext.Version);
                    break;

                case 0x4C:  // dup
                    env->Push(env->Top());
                    break;
                
                case 0x4D:  // swap
                {
                    Value& temp = GASValueConstruct(env->Top1(), valBuf1);
                    env->Top1() = env->Top();
                    env->Top() = temp;
                    GASValueDeconstruct(temp);
                    break;
                }
                
                case 0x4E:  // get member
                {               
                    // Use a reference to Top1 access since stack does not change here,
                    // and this is one of the most common ops.
                    Value&          top1Ref = env->Top1();
                    Value&          topRef  = env->Top();
                    ObjectInterface *piobj = top1Ref.ToObjectInterface(env);

                    if (!piobj)
                    {
                        // get member for non-object case
                        // try to create temporary object and get member from it.
                        Value& objVal = GASValueConstruct(env->PrimitiveToTempObject(1), valBuf1);
                        if ((piobj = objVal.ToObject(env))!=0)
                        {
                            ASString& memberName = GASStringConstruct(topRef.ToString(env), tmpStr1Buf);
                            if (!env->GetMember(piobj, memberName, &top1Ref))
                            {
                                top1Ref.SetUndefined();
                            }
                            GASStringDeconstruct(memberName);
                        }
                        else
                            top1Ref.SetUndefined();
                        GASValueDeconstruct(objVal);
                    }
                    else
                    {
                        // A special case for handling indices in array
                        if (topRef.IsNumber() && piobj->GetObjectType() == ObjectInterface::Object_Array)
                        {
                            ArrayObject* parr = static_cast<ArrayObject*>(piobj);
                            
                            // Save object ref just in case its released by SetUndefined; does not matter for characters.
                            pobj = parr;

                            SInt32 index = (SInt32)topRef.ToNumber(env);
                            if (index < 0)
                            {
                                // negative index - just use regular GetMember
                                ASString& memberName = GASStringConstruct(topRef.ToString(env), tmpStr1Buf);
                                if (!parr->Object::GetMember(env, memberName, &top1Ref))
                                    top1Ref.SetUndefined();
                                GASStringDeconstruct(memberName);
                            }
                            else if (index < parr->GetSize())
                            {
                                Value* v = parr->GetElementPtr(index);
                                if (v)
                                    top1Ref = *v;
                                else
                                    top1Ref.SetUndefined();
                            }
                            else
                                top1Ref.SetUndefined();

                            #ifdef GFX_AS2_VERBOSE
                            if (execContext.VerboseAction) 
                            {     
                                ASString& s1 = GASStringConstruct(top1Ref.ToDebugString(env), tmpStr2Buf);
                                execContext.LogF.LogAction("-- GetMember %p[%d]=%s\n",
                                   parr, (int)index, s1.ToCStr());
                                GASStringDeconstruct(s1);
                            }
                            #endif
                            pobj = NULL; //drop
                        }
                        else
                        {
                            ASString& memberName = GASStringConstruct(topRef.ToString(env), tmpStr1Buf);
                            // Save object ref just in case its released by SetUndefined; does not matter for characters.
                            if (top1Ref.IsObject() || top1Ref.IsFunction()) // Avoid debug warning.
                                pobj = top1Ref.ToObject(env);

                            top1Ref.SetUndefined();

                            //printf ("!!! piobj.__proto__ = %s\n", (const char*)env->GetGC()->FindClassName(piobj->Get__proto__(env)));
                            env->GetMember(piobj, memberName, &top1Ref);

                            #ifdef GFX_AS2_VERBOSE
                            if (execContext.VerboseAction) 
                            {     
                                ASString& s1 = GASStringConstruct(top1Ref.ToDebugString(env), tmpStr2Buf);

                                if (top1Ref.ToObjectInterface(env) == NULL)
                                {                           
                                    execContext.LogF.LogAction("-- GetMember %s=%s\n",
                                        memberName.ToCStr(), s1.ToCStr());
                                }
                                else
                                {                           
                                    execContext.LogF.LogAction("-- GetMember %s=%s at %p\n",
                                        memberName.ToCStr(), s1.ToCStr(),
                                        top1Ref.ToObjectInterface(env));
                                }
                                GASStringDeconstruct(s1);
                            }
                            #endif
                            GASStringDeconstruct(memberName);
                            pobj = NULL; //drop
                        }
                    }

                    env->Drop1();
                    if (env->IsThrowing())
                    {
                        // exception handling
                        env->CheckTryBlocks(execContext.PC, &tryCount);
                        execContext.NextPC = env->CheckExceptions(this, 
                            execContext.NextPC, 
                            &tryCount, 
                            retval,
                            execContext.WithStack,
                            execType);
                    }
                    // Note: get member doesn't cause AS termination
                    break;                  
                }
                case 0x4F:  // set member
                {
                    ObjectInterface* piobj = env->Top(2).ToObjectInterface(env);
                    if (piobj)
                    {
                        Value& top1Ref = env->Top1();
                        // A special case for handling indices in array
                        if (top1Ref.IsNumber() && piobj->GetObjectType() == ObjectInterface::Object_Array)
                        {
                            ArrayObject* parr = static_cast<ArrayObject*>(piobj);
                            SInt32 index = (SInt32)top1Ref.ToNumber(env);
                            if (index >= 0)
                                parr->SetElementSafe((int)index, env->Top());
                            else
                            {
                                ASString& memberName = GASStringConstruct(top1Ref.ToString(env), tmpStr1Buf);
                                parr->Object::SetMember(env, memberName, env->Top());
                                GASStringDeconstruct(memberName);
                            }
                            
                            #ifdef GFX_AS2_VERBOSE
                            if (execContext.VerboseAction)
                            {
                                ASString& s0 = GASStringConstruct(env->Top().ToDebugString(env), tmpStr3Buf);
                                execContext.LogF.LogAction("-- SetMember %p[%d]=%s\n",
                                    parr, 
                                    (int)index, 
                                    s0.ToCStr());
                                GASStringDeconstruct(s0);
                            }
                            #endif
                        }
                        else
                        {
                            ASString& memberName = GASStringConstruct(top1Ref.ToString(env), tmpStr1Buf);
                            piobj->SetMember(env, memberName, env->Top());

                            #ifdef GFX_AS2_VERBOSE
                            if (execContext.VerboseAction)
                            {
                                ASString& s2 = GASStringConstruct(env->Top(2).ToDebugString(env), tmpStr2Buf);
                                ASString& s0 = GASStringConstruct(env->Top().ToDebugString(env), tmpStr3Buf);
                                execContext.LogF.LogAction("-- SetMember %s.%s=%s\n",
                                    s2.ToCStr(), 
                                    memberName.ToCStr(), 
                                    s0.ToCStr());
                                GASStringDeconstruct(s2);
                                GASStringDeconstruct(s0);
                            }
                            #endif
                            GASStringDeconstruct(memberName);
                        }
                    }
                    else
                    {
                        // Invalid object, can't set.
                        #ifdef GFX_AS2_VERBOSE
                        if (execContext.VerboseAction)
                        {
                            ASString& s2 = GASStringConstruct(env->Top(2).ToDebugString(env), tmpStr1Buf);
                            ASString& s1 = GASStringConstruct(env->Top1().ToDebugString(env), tmpStr2Buf);
                            ASString& s0 = GASStringConstruct(env->Top().ToDebugString(env), tmpStr3Buf);
                            execContext.LogF.LogAction(
                                    "- SetMember %s.%s=%s on invalid object\n",
                                    s2.ToCStr(), 
                                    s1.ToCStr(), 
                                    s0.ToCStr());
                            GASStringDeconstruct(s2);
                            GASStringDeconstruct(s1);
                            GASStringDeconstruct(s0);
                        }
                        #endif
                    }
                    env->Drop3();
                    if (env->IsThrowing())
                    {
                        // exception handling
                        env->CheckTryBlocks(execContext.PC, &tryCount);
                        execContext.NextPC = env->CheckExceptions(this, 
                            execContext.NextPC, 
                            &tryCount, 
                            retval,
                            execContext.WithStack,
                            execType);
                    }
                    // Note: set member doesn't cause AS termination
                    break;
                }

                case 0x50:  // increment
                    env->Top().Add(env, 1);
                    break;
                
                case 0x51:  // decrement
                    env->Top().Sub(env, 1);
                    break;
                
                case 0x52:  // call method
                {
                    nargsf = env->Top(2).ToNumber(env);
                    int nargs = (!NumberUtil::IsNaNOrInfinity(nargsf)) ? (int)nargsf : 0;
                    SF_ASSERT(nargs >= 0);

                    Value&           result     = GASValueConstruct0(valBuf1);
                    const ASString&    methodName = GASStringConstruct(env->Top().ToString(env), tmpStr1Buf);
                    ObjectInterface* piobj = 0;
                    ValueGuard       valStorage(env, env->Top1());   

                    // If the method name is blank or undefined, the object is taken to be a function object
                    // that should be invoked.
                    if (env->Top().IsUndefined() || methodName.IsEmpty())
                    {
                        //!AB
                        // This code is invoked when nested function is called from parent function, for example:
                        //foo = function() {
                        //  var local = 10;
                        //  var nestedFoo = function () {}
                        //  nestedFoo(); // <- here
                        //}
                        // NOTE: in this case, "this" inside the "nestedFoo" is reported as "undefined",
                        // but this is not true: it is kinda "stealth" object. typeof(this) says this is 
                        // an "object". It is possible to get access to "local" by using "this.local". Also,
                        // it is possible to create local variable by setting "this.newLocal = 100;"
                        Value* pfunction = &GASValueConstruct(env->Top1(), valBuf2);

                        pobj = NULL; // reset
                        Value& thisVal = GASValueConstruct0(valBuf3);

                        if (pfunction->IsObject() && (pobj = pfunction->ToObject(env)) && pobj->IsSuper())
                        {
                            // special case for "super": call to super.__constructor__
                            GASValueDeconstruct(*pfunction);

                            FunctionRef& ctor = GASFunctionRefConstruct(pobj->Get__constructor__(env->GetSC()), funcBuf);
                            pfunction = &GASValueConstruct(ctor, valBuf2);
                            GASFunctionRefDeconstruct(ctor);

                            thisVal.SetAsObject(pobj);
                        }
                        else
                        {
                            env->GetVariable(env->GetBuiltin(ASBuiltin_this), &thisVal, execContext.WithStack);
                        }

                        if (pfunction->IsFunction())
                        {
                            //GAS_Invoke(function, &result, thisVal, env, nargs, env->GetTopIndex() - 3, NULL);
                            FunctionRef& func   = GASFunctionRefConstruct(pfunction->ToFunction(env), funcBuf);
                            if (func != NULL)
                            {
                                FnCall&      fnCall = GASFnCallConstruct(&result, thisVal, env, nargs, env->GetTopIndex() - 3, fnCallBuf); 
                                func.Invoke(fnCall, NULL);
                                GASFnCallDeconstruct(fnCall);
                            }
                            GASFunctionRefDeconstruct(func);
                        }
                        else
                        {
                            #ifdef GFX_AS2_VERBOSE_ERRORS
                            if (execContext.VerboseActionErrors)
                                execContext.LogF.LogScriptError("CallMethod \"as a function\" - pushed object is not a function object");
                            #endif
                        }
                        GASValueDeconstruct(thisVal);
                        GASValueDeconstruct(*pfunction);
                        pobj = NULL; // dropref
                    }

                    // Object or character.
                    else if (!env->Top1().IsFunction() && (piobj = env->Top1().ToObjectInterface(env)) != 0)
                    {
                        Value&    method = GASValueConstruct0(valBuf2);

                        // check, is the piobj super or not. If yes, get the correct "this" (piobj)
                        if (piobj->IsSuper())
                        {
                            //!AB: looks like we are calling something like "super.func()".
                            // The super object now contains a __proto__ pointing to the nearest
                            // base class. But, the nearest base class might not have the "func()"
                            // method, but the base of base call might have. 
                            // In this case we need to find the appropriate base class containing
                            // the calling method. Once we found it we set it as alternative prototype
                            // to the super object. It will be reseted to original prototype automatically 
                            // after call is completed.
                            Ptr<SuperObject> superObj = static_cast<SuperObject*>(piobj);
                            //printf ("!!! superObj->GetSuperProto() = %s\n", (const char*)env->GetGC()->FindClassName(env->GetSC(), superObj->GetSuperProto()).ToCStr());
                            pobj = superObj->GetSuperProto()->FindOwner(env->GetSC(), methodName); //newProto
                            if (pobj)
                            {
                                //printf ("!!! newProto = %s\n", (const char*)env->GetGC()->FindClassName(env->GetSC(), newProto).ToCStr());
                                superObj->SetAltProto(pobj);
                                pobj = NULL; //dropref
                            }
                            else
                                piobj = 0; // looks like there is no such method in super class
                        }
                        //printf ("!!! piobj.__proto__ = %s\n", (const char*)env->GetGC()->FindClassName(env->GetSC(), piobj->Get__proto__()).ToCStr());

                        if (piobj)
                        {
                            if (env->GetMember(piobj, methodName, &method))
                            {
                                if (!method.IsFunction())
                                {
                                    #ifdef GFX_AS2_VERBOSE_ERRORS
                                    if (execContext.VerboseActionErrors)
                                    {
                                        if (piobj->IsASCharacter())
                                            execContext.LogF.LogScriptError("CallMethod - '%s.%s' is not a method",
                                                piobj->ToCharacter()->GetCharacterHandle()->GetNamePath().ToCStr(), methodName.ToCStr());
                                        else
                                            execContext.LogF.LogScriptError("CallMethod - '%s' is not a method",
                                                methodName.ToCStr());                                      
                                    }
                                    #endif
                                }
                                else
                                {
                                    //                     GAS_Invoke(
                                    //                         method,
                                    //                         &result,
                                    //                         piobj,
                                    //                         env,
                                    //                         nargs,
                                    //                         env->GetTopIndex() - 3,
                                    //                         methodName.ToCStr());

                                    FunctionRef&  func = GASFunctionRefConstruct(method.ToFunction(env), funcBuf);
                                    if (func != NULL)
                                    {
                                        FnCall& fnCall = GASFnCallConstruct(&result, piobj, env, nargs, env->GetTopIndex() - 3, fnCallBuf); 
                                        func.Function->Invoke(fnCall, func.pLocalFrame, methodName.ToCStr());
                                        GASFnCallDeconstruct(fnCall);
                                    }
                                    else
                                    {           
                                        #ifdef GFX_AS2_VERBOSE_ERRORS
                                        if (execContext.VerboseActionErrors)
                                        {
                                            if (piobj->IsASCharacter())
                                                execContext.LogF.LogScriptError(
                                                    "Invoked method (%s.%s) is null", 
                                                    piobj->ToCharacter()->GetCharacterHandle()->GetNamePath().ToCStr(), 
                                                    methodName.ToCStr());
                                            else
                                                execContext.LogF.LogScriptError(
                                                    "Invoked method (%s) is null", methodName.ToCStr());
                                        }
                                        #endif
                                    }
                                    GASFunctionRefDeconstruct(func);
                                }
                            }
                            else
                            {
                                #ifdef GFX_AS2_VERBOSE_ERRORS
                                if (execContext.VerboseActionErrors)
                                {
                                    if (piobj->IsASCharacter())
                                        execContext.LogF.LogScriptError("CallMethod - can't find method %s.%s",
                                        piobj->ToCharacter()->GetCharacterHandle()->GetNamePath().ToCStr(), methodName.ToCStr());
                                    else
                                        execContext.LogF.LogScriptError("CallMethod - can't find method %s",
                                        methodName.ToCStr());                                      

                                }
                                #endif
                            }
                        }
                        GASValueDeconstruct(method);
                    }
                    else if (env->Top1().IsFunction())
                    {
                        // Looks like we want to call a static method.

                        // get the function object first
                        const Value& thisVal = env->Top1();
                        piobj = thisVal.ToObject(env);
                        if (piobj != 0) 
                        {
                            Value& method = GASValueConstruct0(valBuf2);
                            if (env->GetMember(piobj, methodName, &method))
                            {                                                       
                                // invoke method
                                //                 GAS_Invoke(
                                //                     method,
                                //                     &result,
                                //                     thisVal,
                                //                     env,
                                //                     //NULL, // this is null for static calls
                                //                     nargs,
                                //                     env->GetTopIndex() - 3,
                                //                     methodName.ToCStr());
                                FunctionRef&  func = GASFunctionRefConstruct(method.ToFunction(env), funcBuf);
                                if (func != NULL)
                                {
                                    FnCall& fnCall = GASFnCallConstruct(&result, thisVal, env, nargs, env->GetTopIndex() - 3, fnCallBuf); 
                                    func.Function->Invoke(fnCall, func.pLocalFrame, methodName.ToCStr());
                                    GASFnCallDeconstruct(fnCall);
                                }
                                else
                                {           
                                    #ifdef GFX_AS2_VERBOSE_ERRORS

                                    if (execContext.VerboseActionErrors)
                                    {
                                        if (piobj->IsASCharacter())
                                            execContext.LogF.LogScriptError("Invoked method (%s.%s) is not a function",
                                            piobj->ToCharacter()->GetCharacterHandle()->GetNamePath().ToCStr(), methodName.ToCStr());
                                        else
                                            execContext.LogF.LogScriptError("Invoked method (%s) is not a function", methodName.ToCStr());

                                    }
                                    #endif
                                }
                                GASFunctionRefDeconstruct(func);
                            }
                            else
                            {
                                #ifdef GFX_AS2_VERBOSE_ERRORS
                                if (execContext.VerboseActionErrors)
                                {
                                    if (piobj->IsASCharacter())
                                        execContext.LogF.LogScriptError("Static method '%s.%s' is not found.",
                                            piobj->ToCharacter()->GetCharacterHandle()->GetNamePath().ToCStr(), methodName.ToCStr());
                                    else
                                        execContext.LogF.LogScriptError("Static method '%s' is not found.",
                                                               methodName.ToCStr());
                                } 
                                #endif
                            }
                            GASValueDeconstruct(method);
                        }
                        else 
                        {
                            #ifdef GFX_AS2_VERBOSE_ERRORS
                            if (execContext.VerboseActionErrors)
                                execContext.LogF.LogScriptError("Function is not an object for static method '%s'.",
                                                                methodName.ToCStr());
                            #endif
                        }
                    }
                    else
                    {
                        // Handle methods on primitive types (string, boolean, number).
                        // In this case, an appropriate temporary object is created
                        // (String, Boolean, Number) and method is called for it.
                        Value& objVal = GASValueConstruct(env->PrimitiveToTempObject(1), valBuf2);
                        if (objVal.IsObject())
                        {
                            piobj = objVal.ToObject(env);
                            Value& method = GASValueConstruct0(valBuf3);
                            if (env->GetMember(piobj, methodName, &method))
                            {
                                if (method.GetType() != Value::FUNCTION)
                                {
                                    #ifdef GFX_AS2_VERBOSE_ERRORS
                                    if (execContext.VerboseActionErrors)
                                    {
                                        if (piobj && piobj->IsASCharacter())
                                            execContext.LogF.LogScriptError("CallMethod - '%s.%s' is not a method",
                                                piobj->ToCharacter()->GetCharacterHandle()->GetNamePath().ToCStr(),
                                                methodName.ToCStr());
                                        else
                                            execContext.LogF.LogScriptError("CallMethod - '%s' is not a method",
                                                methodName.ToCStr());
                                    }
                                    #endif
                                }
                                else
                                {
                                    //                     GAS_Invoke(
                                    //                         method,
                                    //                         &result,
                                    //                         piobj,
                                    //                         env,
                                    //                         nargs,
                                    //                         env->GetTopIndex() - 3,
                                    //                         methodName.ToCStr());
                                    FunctionRef&  func = GASFunctionRefConstruct(method.ToFunction(env), funcBuf);
                                    if (func != NULL)
                                    {
                                        FnCall& fnCall = GASFnCallConstruct(&result, piobj, env, nargs, env->GetTopIndex() - 3, fnCallBuf); 
                                        func.Function->Invoke(fnCall, func.pLocalFrame, methodName.ToCStr());
                                        GASFnCallDeconstruct(fnCall);
                                    }
                                    else
                                    {           
                                        #ifdef GFX_AS2_VERBOSE_ERRORS
                                        if (execContext.VerboseActionErrors)
                                        {
                                            if (piobj && piobj->IsASCharacter())
                                                execContext.LogF.LogScriptError
                                                    ("Error: Invoked method (%s.%s) is not a function\n", 
                                                    piobj->ToCharacter()->GetCharacterHandle()->GetNamePath().ToCStr(),
                                                    methodName.ToCStr());
                                            else
                                                execContext.LogF.LogScriptError
                                                    ("Error: Invoked method (%s) is not a function\n", methodName.ToCStr());
                                        }
                                        #endif
                                    }
                                    GASFunctionRefDeconstruct(func);
                                }
                            }
                            else
                            {
                                #ifdef GFX_AS2_VERBOSE_ERRORS
                                if (piobj && piobj->IsASCharacter())
                                    execContext.LogF.LogScriptError("CallMethod - can't find method %s.%s",
                                        piobj->ToCharacter()->GetCharacterHandle()->GetNamePath().ToCStr(), methodName.ToCStr());
                                else
                                    execContext.LogF.LogScriptError("CallMethod - can't find method %s",
                                        methodName.ToCStr());                                  
                                #endif
                            }
                            GASValueDeconstruct(method);
                            piobj = NULL;
                        }
                        else
                        {
                            #ifdef GFX_AS2_VERBOSE_ERRORS
                            if (execContext.VerboseActionErrors)
                                execContext.LogF.LogScriptError("CallMethod - '%s' on invalid object.",
                                                                methodName.ToCStr());
                            #endif
                        }
                        GASValueDeconstruct(objVal);
                    }

                    env->Drop(nargs + 2);
                    env->Top() = result;

                    if (env->IsThrowing())
                    {
                        // exception handling
                        env->CheckTryBlocks(execContext.PC, &tryCount);
                        execContext.NextPC = env->CheckExceptions(this, 
                            execContext.NextPC, 
                            &tryCount, 
                            retval,
                            execContext.WithStack,
                            execType);
                    }
                    // check if it is necessary to terminate the execution, because the target 
                    // is unloaded or unloading. See comments for NeedTermination for more details.
                    // "Call method" may terminate the ActionScript only if "this" is resolved
                    // and if it is a character - if it is not unloaded.
                    if (piobj)
                    {
                        InteractiveObject* pasch = piobj->ToCharacter();
                        if (!pasch || !pasch->IsUnloaded())
                        {
                            if (env->NeedTermination(execType))
                                execContext.NextPC = execContext.StopPC;
                        }
                    }
                    GASValueDeconstruct(result);
                    GASStringDeconstruct(methodName);
                    break;
                }
                case 0x53:  // new method
                {
                    Value&   constructorName = GASValueConstruct(env->Top(), valBuf1);
                    Value&   object          = GASValueConstruct(env->Top1(), valBuf2);
                    nargsf = env->Top(2).ToNumber(env);
                    int nargs = (!NumberUtil::IsNaNOrInfinity(nargsf)) ? (int)nargsf : 0;
                    SF_ASSERT(nargs >= 0);
                    env->Drop3();

                    ASString&  constructorNameStr = GASStringConstruct(constructorName.ToString(env), tmpStr1Buf);

                    #ifdef GFX_AS2_VERBOSE
                    if (execContext.VerboseAction) 
                        execContext.LogF.LogAction("---new method: %s\n", constructorNameStr.ToCStr());
                    #endif

                    Value*   pconstructor;
                    if (constructorName.IsUndefined() || (constructorName.IsString() && constructorNameStr.IsEmpty()))
                    {
                        // if constructor method's name is blank use the "object" as function object
                        FunctionRef& ctor = GASFunctionRefConstruct(object.ToFunction(env), funcBuf);
                        pconstructor = &GASValueConstruct(ctor, valBuf3);
                        GASFunctionRefDeconstruct(ctor);
                    }
                    else
                    {
                        // MA: Can object be null?
                        // get the method
                        pconstructor = &GASValueConstruct0(valBuf3);
                        ObjectInterface* piobj = object.ToObjectInterface(env);
                        if (!piobj || !env->GetMember(piobj, constructorNameStr, pconstructor))
                        {
                            // method not found!
                            #ifdef GFX_AS2_VERBOSE_ERRORS
                            if (execContext.VerboseActionErrors)
                            {
                                if (piobj && piobj->IsASCharacter())
                                    execContext.LogF.LogScriptError("Method '%s.%s' is not found.",
                                        piobj->ToCharacter()->GetCharacterHandle()->GetNamePath().ToCStr(), 
                                        constructorNameStr.ToCStr());
                                else
                                    execContext.LogF.LogScriptError("Method '%s' is not found.",
                                        constructorNameStr.ToCStr());
                            }
                            #endif
                        }
                    }

                    if (pconstructor->IsFunction())
                    {
                        FunctionRef& func = GASFunctionRefConstruct(pconstructor->ToFunction(env), funcBuf);
                        SF_ASSERT (func != NULL);

                        pobj = *env->OperatorNew(func, nargs);
                        GASFunctionRefDeconstruct(func);
                    }
                    else
                    {
                        pobj = NULL;
                        #ifdef GFX_AS2_VERBOSE_ERRORS
                        if (execContext.VerboseActionErrors)
                            execContext.LogF.LogScriptError("Can't create object with unknown ctor");
                        #endif
                    }

                    env->Drop(nargs);
                    env->Push(pobj);
                    #ifdef GFX_AS2_VERBOSE
                    if (execContext.VerboseAction) 
                        execContext.LogF.LogAction("New object created at %p\n", pobj.GetPtr());
                    #endif

                    pobj = NULL; //drop ref
                    GASValueDeconstruct(constructorName);
                    GASValueDeconstruct(object);
                    GASValueDeconstruct(*pconstructor);
                    GASStringDeconstruct(constructorNameStr);

                    if (env->IsThrowing())
                    {
                        // exception handling
                        env->CheckTryBlocks(execContext.PC, &tryCount);
                        execContext.NextPC = env->CheckExceptions(this, 
                            execContext.NextPC, 
                            &tryCount, 
                            retval,
                            execContext.WithStack,
                            execType);
                    }
                    // check if it is necessary to terminate the execution, because the target 
                    // is unloaded or unloading. See comments for NeedTermination for more details.
                    if (env->NeedTermination(execType))
                        execContext.NextPC = execContext.StopPC;
                    break;
                }
                case 0x54:  // instance of
                    execContext.InstanceOfOpCode();
                    break;

                //case 0x55: // enumerate object 2: see op code 0x46
                
                case 0x60:  // bitwise and
                    env->Top1().And (env, env->Top());
                    env->Drop1();
                    break;

                case 0x61:  // bitwise or
                    env->Top1().Or (env, env->Top());
                    env->Drop1();
                    break;

                case 0x62:  // bitwise xor
                    env->Top1().Xor (env, env->Top());
                    env->Drop1();
                    break;

                case 0x63:  // shift left
                    env->Top1().Shl(env, env->Top());
                    env->Drop1();
                    break;

                case 0x64:  // shift Right (signed)
                    env->Top1().Asr(env, env->Top());
                    env->Drop1();
                    break;

                case 0x65:  // shift Right (unsigned)
                    env->Top1().Lsr(env, env->Top());
                    env->Drop1();
                    break;

                case 0x66:  // strict equal
                    if (!env->Top1().TypesMatch(env->Top()))
                    {
                        // Types don't match.
                        env->Top1().SetBool(false);
                        env->Drop1();
                    }
                    else
                    {
                        env->Top1().SetBool(env->Top1().IsEqual (env, env->Top()));
                        env->Drop1();
                    }
                    break;

                case 0x67:  // Gt (typed)
                {
                    Value& result = GASValueConstruct(env->Top1().Compare (env, env->Top(), 1), valBuf1);
                    env->Top1() = result;
                    env->Drop1();
                    GASValueDeconstruct(result);
                    break;
                }

                case 0x68:  // string gt
                {
                    ASString& s1 = GASStringConstruct(env->Top1().ToStringVersioned(env, execContext.Version), tmpStr1Buf);
                    ASString& s0 = GASStringConstruct(env->Top().ToStringVersioned(env, execContext.Version), tmpStr2Buf);
                    env->Top1().SetBool(s1 > s0);
                    env->Drop1();
                    GASStringDeconstruct(s1);
                    GASStringDeconstruct(s0);
                    break;
                }

                case 0x69:  // extends
                    execContext.ExtendsOpCode();
                    break;
            }
        }
        else
        {
            #ifdef GFX_AS2_VERBOSE
            if (execContext.VerboseAction) 
            {
                execContext.LogF.LogAction("EX: %4.4X\t", execContext.PC); 
                execContext.LogF.LogDisasm(&execContext.pBuffer[execContext.PC]);
            }
            #endif

            // Action containing extra data.
            int actionLength = execContext.pBuffer[execContext.PC + 1] | (execContext.pBuffer[execContext.PC + 2] << 8);
            execContext.NextPC = execContext.PC + actionLength + 3;

            switch (actionId)
            {
            default:
                break;

            case 0x81:  // goto frame
            {
                // Used by gotoAndPlay(n), gotoAndStop(n) where n is a constant.
                // If n is a "string" use 0x8C, if variable - 0x9F.
                // Produced frame is already zero-based.
                if (env->IsTargetValid())
                {
                    int frame = execContext.pBuffer[execContext.PC + 3] | (execContext.pBuffer[execContext.PC + 4] << 8);
                    if (env->GetTarget())
                        env->GetTarget()->GotoFrame(frame);
                }
                // check if it is necessary to terminate the execution, because the target 
                // is unloaded or unloading. See comments for NeedTermination for more details.
                if (env->NeedTermination(execType))
                    execContext.NextPC = execContext.StopPC;
                break;
            }

            case 0x83:  // get url
            {               
                // Two strings as args.
                const char*     purl        = (const char*) &(execContext.pBuffer[execContext.PC + 3]);
                unsigned        urlLen      = (unsigned)strlen(purl);
                const char*     ptargetPath = (const char*) &(execContext.pBuffer[execContext.PC + 3 + urlLen + 1]);

                #ifdef GFX_AS2_VERBOSE
                if (execContext.VerboseAction) 
                    execContext.LogF.LogAction("GetURL - path: %s  URL: %s", ptargetPath, purl);
                #endif
                
                // If the url starts with "FSCommand:" then this is a message for the host app,
                // so call the callback handler, if any.
                if (strncmp(purl, "FSCommand:", 10) == 0)
                {
                    FSCommandHandler *phandler =
                        execContext.pOriginalTarget->GetMovieImpl()->pFSCommandHandler;
                    if (phandler)
                    {
                        // Call into the app.
                        phandler->Callback(env->GetTarget()->GetMovieImpl(), purl + 10, ptargetPath);
                    }
                }
                else
                {
                    // This is a loadMovie/loadMovieNum/unloadMovie/unloadMovieNum call.
                    env->GetAS2Root()->AddLoadQueueEntry(ptargetPath, purl, env);
                }
                break;
            }

            case 0x87:  // StoreRegister
            {
                int reg = execContext.pBuffer[execContext.PC + 3];
                // Save top of stack in specified register.
                if (execContext.IsFunction2)
                {
                    *(env->LocalRegisterPtr(reg)) = env->Top();

                    #ifdef GFX_AS2_VERBOSE
                    if (execContext.VerboseAction) 
                    {
                        ASString& s0 = GASStringConstruct(env->Top().ToDebugString(env), tmpStr1Buf);
                        execContext.LogF.LogAction(
                                "-------------- local register[%d] = '%s'",
                                reg, s0.ToCStr());

                        ObjectInterface* piobj = env->Top().ToObjectInterface(env);
                        if (piobj)
                           execContext.LogF.LogAction(" at %p", piobj);
                        execContext.LogF.LogAction("\n");
                        GASStringDeconstruct(s0);
                    }
                    #endif
                }
                else if (reg >= 0 && reg < 4)
                {
                    env->GetGlobalRegister(reg) = env->Top();
                
                    #ifdef GFX_AS2_VERBOSE
                    if (execContext.VerboseAction)
                    {
                        ASString& s0 = GASStringConstruct(env->Top().ToDebugString(env), tmpStr1Buf);
                        execContext.LogF.LogAction(
                                "-------------- global register[%d] = '%s'",
                                reg, s0.ToCStr());
                        
                        ObjectInterface* piobj = env->Top().ToObjectInterface(env);
                        if (piobj)
                           execContext.LogF.LogAction(" at %p", piobj);
                        execContext.LogF.LogAction("\n");
                        GASStringDeconstruct(s0);
                    }
                    #endif
                }
                else
                {
                    #ifdef GFX_AS2_VERBOSE_ERRORS
                    if (execContext.VerboseActionErrors)
                        execContext.LogF.LogScriptError("StoreRegister[%d] - register out of bounds!", reg);
                    #endif
                }

                break;
            }

            case 0x88:  // DeclDict: declare dictionary
            {
                int i = execContext.PC;
                //int   count = execContext.pBuffer[execContext.PC + 3] | (execContext.pBuffer[execContext.PC + 4] << 8);
                i += 2;

                ProcessDeclDict(env->GetSC(), execContext.PC, execContext.NextPC, execContext.LogF);

                break;
            }

            case 0x8A:  // WaitForFrame
            case 0x8D:  // WaitForFrame2 (stack based)
                execContext.WaitForFrameOpCode(this, actionId);
                break;

            case 0x8B:  // set target (used for non-stacked tellTarget)
            {
                // Change the InteractiveObject we're working on.
                const char* ptargetName = (const char*) &execContext.pBuffer[execContext.PC + 3];
                if (ptargetName[0] == 0)
                {
                    env->SetTarget(execContext.pOriginalTarget);
                }
                else
                {
                    ASString& targetName = GASStringConstruct(env->CreateString(ptargetName), tmpStr1Buf);
                    InteractiveObject* ptarget = env->FindTarget(targetName);

                    //!AB: if ptarget is NULL, we need to set kind of fake target
                    // to avoid any side effects. Like:
                    // tellTarget(bullshit)
                    // {  gotoAndStop(1); } // if 'bullshit' is not found here,
                    // then gotoAndStop should do nothing
                    if (!ptarget) 
                    {
                        #ifdef GFX_AS2_VERBOSE_ERRORS
                        if (execContext.VerboseActionErrors)
                            execContext.LogF.LogScriptError("SetTarget(tellTarget) with invalid target '%s'.",
                                               ptargetName);
                        #endif
                        env->SetInvalidTarget(execContext.pOriginalTarget);
                    }
                    else
                    {
                        env->SetTarget(ptarget);
                    }
                    GASStringDeconstruct(targetName);
                }
                break;
            }

            case 0x8C:  // go to labeled frame, GotoFrameLbl
            {
                // MA: This op does NOT interpret numbers in a string, so "4" is actually 
                // treated as a label and NOT as frame. This is different from ops and
                // functions with stack arguments, which DO try to parse frame number first.
                if (env->IsTargetValid())
                {
                    char*       pframeLabel = (char*) &execContext.pBuffer[execContext.PC + 3];
                    Sprite*  psprite = env->GetTarget()->CharToSprite();
                    if (psprite)
                        psprite->GotoLabeledFrame(pframeLabel);
                }
                // check if it is necessary to terminate the execution, because the target 
                // is unloaded or unloading. See comments for NeedTermination for more details.
                if (env->NeedTermination(execType))
                    execContext.NextPC = execContext.StopPC;
                break;
            }

            case 0x8E:  // function2
                execContext.Function2OpCode(this);
                break;

            case 0x8F:  // try
            {
                env->CheckTryBlocks(execContext.PC, &tryCount);
                ++tryCount;
                Environment::TryDescr descr;
                descr.pTryBlock     = &execContext.pBuffer[execContext.PC + 3];
                descr.TryBeginPC    = execContext.NextPC;
                descr.TopStackIndex = env->GetTopIndex();
                env->PushTryBlock(descr);
                break;
            }

            case 0x94:  // with
            {
                //int frame = execContext.pBuffer[execContext.PC + 3] | (execContext.pBuffer[execContext.PC + 4] << 8);
                
                #ifdef GFX_AS2_VERBOSE
                if (execContext.VerboseAction) 
                    execContext.LogF.LogAction("-------------- with block start: stack size is %d\n", (int)execContext.WithStack.GetSize());
                #endif
                if (execContext.WithStack.GetSize() < 8)
                {
                    int BlockLength = execContext.pBuffer[execContext.PC + 3] | (execContext.pBuffer[execContext.PC + 4] << 8);
                    int BlockEnd = execContext.NextPC + BlockLength;
                    
                    if (env->Top().IsCharacter())
                    {
                        InteractiveObject* pwithChar = env->Top().ToCharacter(env);
                        execContext.WithStack.PushBack(WithStackEntry(pwithChar, BlockEnd));
                    }
                    else
                    {
                        Object* pwithObj = env->Top().ToObject(env);
                        execContext.WithStack.PushBack(WithStackEntry(pwithObj, BlockEnd));
                    }               
                }
                env->Drop1();
                break;
            }

            case 0x96:  // PushData
            {
                int i = execContext.PC;

                // MA: Length must be greater then 0 here, otherwise push would make no sense;
                // so it shouldn't happen in practice. Hence, use do {} while for efficiency,
                // as PushData is the *most* common op. This assertion can be checked for by the 
                // bytecode verifier in the future (during action buffer Read, etc).
                SF_ASSERT(actionLength > 0);
               
                do
                {
                    SPInt   type = execContext.pBuffer[3 + i];
                    i++;

                    // Push register is the most common value type.
                    // Push dictionary is the second common type.
                    if (type == 4)
                    {
                        // contents of register
                        int reg = execContext.pBuffer[3 + i];
                        i++;
                        if (execContext.IsFunction2)
                        {
                            env->Push(*(env->LocalRegisterPtr(reg)));
                            #ifdef GFX_AS2_VERBOSE
                            if (execContext.VerboseAction) 
                            {
                                ASString& s = GASStringConstruct(env->Top().ToDebugString(env), tmpStr1Buf);
                                execContext.LogF.LogAction(
                                    "-------------- pushed local register[%d] = '%s'",
                                    reg, s.ToCStr());

                                ObjectInterface* piobj = env->Top().ToObjectInterface(env);
                                if (piobj)
                                    execContext.LogF.LogAction(" at %p", piobj);                          
                                execContext.LogF.LogAction("\n");
                                GASStringDeconstruct(s);
                            }
                            #endif
                        }
                        else if (reg < 0 || reg >= 4)
                        {
                            Value& empty = GASValueConstruct0(valBuf1);
                            env->Push(empty);
                            #ifdef GFX_AS2_VERBOSE_ERRORS
                            if (execContext.VerboseActionErrors)
                                execContext.LogF.LogScriptError("Push register[%d] - register out of bounds", reg);
                            #endif
                            GASValueDeconstruct(empty);
                        }
                        else
                        {
                            env->Push(env->GetGlobalRegister(reg));
                            #ifdef GFX_AS2_VERBOSE
                            if (execContext.VerboseAction) 
                            {
                                ASString& s = GASStringConstruct(env->Top().ToDebugString(env), tmpStr1Buf);
                                execContext.LogF.LogAction(
                                    "-------------- pushed global register[%d] = '%s'",
                                    reg, s.ToCStr());
                                ObjectInterface* piobj = env->Top().ToObjectInterface(env);
                                if (piobj)
                                    execContext.LogF.LogAction(" at %p", piobj);  
                                execContext.LogF.LogAction("\n");
                                GASStringDeconstruct(s);
                            }
                            #endif
                        }

                    }
                    else if (type == 8)
                    {
                        unsigned id = execContext.pBuffer[3 + i];
                        i++;
                        if (id < Dictionary.GetSize())
                        {
                            // Push string directly with a copy constructor.
                            env->Push(Dictionary[id]);

                            #ifdef GFX_AS2_VERBOSE
                            if (execContext.VerboseAction) 
                                execContext.LogF.LogAction("-------------- pushed '%s'\n", Dictionary[id].ToCStr());
                            #endif
                        }
                        else
                        {
                            #ifdef GFX_AS2_VERBOSE_ERRORS
                            if (execContext.VerboseActionErrors)
                                execContext.LogF.LogScriptError("DictLookup(%d) is out of bounds", id);
                            #endif
                            Value& empty = GASValueConstruct0(valBuf1);
                            env->Push(empty);
                            GASValueDeconstruct(empty);
                            #ifdef GFX_AS2_VERBOSE
                            if (execContext.VerboseAction) 
                                execContext.LogF.LogAction("-------------- pushed 0\n");
                            #endif
                        }
                    }
                    else if (type == 0)
                    {
                        // string
                        ASString& str = GASStringConstruct(env->CreateString((const char*) &execContext.pBuffer[3 + i]), tmpStr1Buf);
                        i += (int)str.GetSize() + 1;
                        env->Push(str);

                        #ifdef GFX_AS2_VERBOSE
                        if (execContext.VerboseAction) 
                            execContext.LogF.LogAction("-------------- pushed '%s'\n", str.ToCStr());
                        #endif
                        GASStringDeconstruct(str);
                    }
                    else if (type == 1)
                    {
                        // float (little-endian)
                        union {
                            float   F;
                            UInt32  I;
                        } u;
                        SF_COMPILER_ASSERT(sizeof(u) == sizeof(u.I));

                        memcpy(&u.I, &execContext.pBuffer[3 + i], 4);
                        u.I = Alg::ByteUtil::LEToSystem(u.I);
                        i += 4;

                        env->Push(u.F);

                        #ifdef GFX_AS2_VERBOSE
                        if (execContext.VerboseAction) 
                            execContext.LogF.LogAction("-------------- pushed '%f'\n", u.F);
                        #endif
                    }
                    else if (type == 2)
                    {                           
                        Value& nullValue = GASValueConstruct0(valBuf1);
                        nullValue.SetNull();
                        env->Push(nullValue);
                        GASValueDeconstruct(nullValue);

                        #ifdef GFX_AS2_VERBOSE
                        if (execContext.VerboseAction) 
                            execContext.LogF.LogAction("-------------- pushed NULL\n");
                        #endif
                    }
                    else if (type == 3)
                    {
                        Value& empty = GASValueConstruct0(valBuf1);
                        env->Push(empty);
                        GASValueDeconstruct(empty);

                        #ifdef GFX_AS2_VERBOSE
                        if (execContext.VerboseAction) 
                            execContext.LogF.LogAction("-------------- pushed UNDEFINED\n");
                        #endif
                    }
                    else if (type == 5)
                    {
                        bool    boolVal = execContext.pBuffer[3 + i] ? true : false;
                        i++;
                        env->Push(boolVal);

                        #ifdef GFX_AS2_VERBOSE
                        if (execContext.VerboseAction) 
                            execContext.LogF.LogAction("-------------- pushed %s\n", boolVal ? "true" : "false");
                        #endif
                    }
                    else if (type == 6)
                    {
                        // double
                        // wacky format: 45670123
                        union {
                            double  D;
                            UInt64  I;
                            struct {
                                UInt32  Lo;
                                UInt32  Hi;
                            } Sub;
                        } u;
                        SF_COMPILER_ASSERT(sizeof(UInt32) == 4);
                        SF_COMPILER_ASSERT(sizeof(u) == sizeof(u.I));

                        memcpy(&u.Sub.Hi, &execContext.pBuffer[3 + i], 4);
                        memcpy(&u.Sub.Lo, &execContext.pBuffer[3 + i + 4], 4);
                        u.I = Alg::ByteUtil::LEToSystem(u.I);
                        i += 8;

                        env->Push((Number)u.D);

                        #ifdef GFX_AS2_VERBOSE
                        if (execContext.VerboseAction) 
                            execContext.LogF.LogAction("-------------- pushed double %f\n", u.D);
                        #endif
                    }
                    else if (type == 7)
                    {
                        // int32
                        SInt32  val = execContext.pBuffer[3 + i]
                            | (execContext.pBuffer[3 + i + 1] << 8)
                            | (execContext.pBuffer[3 + i + 2] << 16)
                            | (execContext.pBuffer[3 + i + 3] << 24);
                        i += 4;
                    
                        env->Push(val);

                        #ifdef GFX_AS2_VERBOSE
                        if (execContext.VerboseAction) 
                            execContext.LogF.LogAction("-------------- pushed int32 %d\n", (int)val);
                        #endif
                    }
                    else if (type == 9)
                    {
                        unsigned id = execContext.pBuffer[3 + i] | (execContext.pBuffer[4 + i] << 8);
                        i += 2;
                        if (id < Dictionary.GetSize())
                        {                            
                            env->Push(Dictionary[id]);
                            #ifdef GFX_AS2_VERBOSE
                            if (execContext.VerboseAction) 
                                execContext.LogF.LogAction("-------------- pushed '%s'\n", Dictionary[id].ToCStr());
                            #endif
                        }
                        else
                        {
                            #ifdef GFX_AS2_VERBOSE_ERRORS
                            if (execContext.VerboseActionErrors)
                                execContext.LogF.LogScriptError("DictLookup(%d) is out of bounds", id);
                            #endif
                            Value& empty = GASValueConstruct0(valBuf1);
                            env->Push(empty);
                            GASValueDeconstruct(empty);

                            #ifdef GFX_AS2_VERBOSE
                            if (execContext.VerboseAction) 
                                execContext.LogF.LogAction("-------------- pushed 0");
                            #endif
                        }
                    }

                } while (i - execContext.PC < actionLength);
                
                break;
            }
            case 0x99:  // branch Always (goto)
            {
                SInt16  offset = execContext.pBuffer[execContext.PC + 3] | (SInt16(execContext.pBuffer[execContext.PC + 4]) << 8);
                execContext.NextPC += offset;
                                  
                // Range checks.
                if (((unsigned)execContext.NextPC) >= GetLength())
                {
                    // MA: Seems that ActionScript can actually jump outside of action buffer
                    // bounds into the binary content stored within other tags. Handling this
                    // would complicate things. Ex: f1.swf 
                    // Perhaps the entire SWF is stored within one chunk of memory and
                    // ActionScript goto instruction is welcome to seek arbitrarily in it ?!?

                    env->SetTarget(execContext.pOriginalTarget);
                    #ifdef GFX_AS2_VERBOSE_ERRORS
                    if (execContext.VerboseActionErrors)
                        execContext.LogF.LogScriptError("Branch destination %d is out of action buffer bounds!",
                                            execContext.NextPC);
                    #endif
                    return;
                }
                break;
            }
            case 0x9A:  // get url 2
            {
                UByte   method = execContext.pBuffer[execContext.PC + 3];

                ASString&   targetPath = GASStringConstruct(env->Top().ToString(env), tmpStr1Buf);
                ASString&   url        = GASStringConstruct(env->Top1().ToString(env), tmpStr2Buf);

                #ifdef GFX_AS2_VERBOSE
                if (execContext.VerboseAction) 
                    execContext.LogF.LogAction("GetURL2 - path: %s  URL: %s", targetPath.ToCStr(), url.ToCStr());
                #endif

                // If the url starts with "FSCommand:", then this is
                // a message for the host app.
                if (strncmp(url.ToCStr(), "FSCommand:", 10) == 0)
                {
                    FSCommandHandler *phandler =
                        execContext.pOriginalTarget->GetMovieImpl()->pFSCommandHandler;
                    if (phandler)
                    {
                        // Call into the app.
                        phandler->Callback(env->GetTarget()->GetMovieImpl(),
                                           url.ToCStr() + 10, targetPath.ToCStr());
                    }
                }
                else
                {
                    LoadQueueEntry::LoadMethod loadMethod = LoadQueueEntry::LM_None;
                    switch(method & 3)
                    {
                        case 1: loadMethod = LoadQueueEntry::LM_Get; break;
                        case 2: loadMethod = LoadQueueEntry::LM_Post; break;
                    }

                    // 0x40 is set if target is a path to a clip; otherwise it is a path to a window
                    // 0x40 -> loadMovie(); if not loadURL()
                    // Note: loadMovieNum() can call this without target flag, so check url for _levelN.
                    const char* ptail = "";
                    int         level = MovieRoot::ParseLevelName(
                                                targetPath.ToCStr(), &ptail,
                                                env->GetTarget()->IsCaseSensitive());
                    if (method & 0x80)
                    {   
                        // loadVars\loadVarsNum
                        env->GetAS2Root()->AddVarLoadQueueEntry(targetPath.ToCStr(), url.ToCStr(), loadMethod);
                    }
                    else if ((method & 0x40) || ((level != -1) && (*ptail==0)))
                    {
                        // This is a loadMovie/loadMovieNum/unloadMovie/unloadMovieNum call.
                        env->GetAS2Root()->AddLoadQueueEntry(targetPath.ToCStr(), url.ToCStr(), env, loadMethod);
                    }
                    else
                    {   
                        // ??? not sure what to do.
                    }
                
                }
                env->Drop2();
                GASStringDeconstruct(targetPath);
                GASStringDeconstruct(url);
                break;
            }

            case 0x9B:  // declare function
                execContext.Function1OpCode(this);
                break;

            case 0x9D:  // branch if true
            {
                SInt16  offset = execContext.pBuffer[execContext.PC + 3] | (SInt16(execContext.pBuffer[execContext.PC + 4]) << 8);
                
                bool    test = env->Top().ToBool(env);
                env->Drop1();
                if (test)
                {
                    execContext.NextPC += offset;

                    if (execContext.NextPC > execContext.StopPC)
                    {
                        #ifdef GFX_AS2_VERBOSE_ERRORS
                        if (execContext.VerboseActionErrors)
                            execContext.LogF.LogScriptError("branch to offset %d - this section only runs to %d",
                                                            execContext.NextPC, execContext.StopPC);
                        #endif
                    }
                }
                break;
            }
            case 0x9E:  // call frame
            {
                // Note: no extra data in this instruction!
                InteractiveObject* ptarget  = env->GetTarget();
                SF_ASSERT(ptarget);
                if (env->IsTargetValid())
                {
                    unsigned frameNumber = 0;   
                    if (ResolveFrameNumber(env, env->Top(), &ptarget, &frameNumber))
                    {
                        Sprite* psprite = ptarget->CharToSprite();
                        if (psprite)
                            ToAvmSprite(psprite)->CallFrameActions(frameNumber);
                    }
                    #ifdef GFX_AS2_VERBOSE_ERRORS
                    else if (execContext.VerboseActionErrors)
                    {
                        execContext.LogF.LogScriptError("%s, CallFrame('%s') - unknown frame", 
                            ptarget->GetCharacterHandle()->GetNamePath().ToCStr(), env->Top().ToString(env).ToCStr());
                    }
                    #endif

                    env->Drop1();
                }
           
                break;
            }

            case 0x9F:  // goto frame expression, GotoFrameExp
            {
                // From Alexi's SWF ref:
                //
                // Pop a value or a string and jump to the specified
                // frame. When a string is specified, it can include a
                // path to a sprite as in:
                // 
                //   /Test:55
                // 
                // When F_play is ON, the action is to play as soon as that 
                // frame is reached. Otherwise, the frame is shown in stop mode.

                UByte       optionFlag  = execContext.pBuffer[execContext.PC + 3];
                PlayState   state       = (optionFlag & 1) ?
                                          GFx::State_Playing : GFx::State_Stopped;
                int         sceneOffset = 0;

                // SceneOffset is used if multiple scenes are created in Flash studio,
                // and then gotoAndPlay("Scene 2", val) is executed. The name of the
                // scene is converted into offset and passed here.
                if (optionFlag & 0x2)
                    sceneOffset = execContext.pBuffer[execContext.PC + 4] | (execContext.pBuffer[execContext.PC + 5] << 8);

                InteractiveObject* target  = env->GetTarget();
                unsigned frameNumber = 0;   
                if (ResolveFrameNumber(env, env->Top(), &target, &frameNumber))
                {
                    // There actually is a bug here *In Flash*. SceneOffset should not be 
                    // considered if we go to label, as labels already have absolute
                    // scene-inclusive offsets. However, we replicate their player bug.
                    // MA: Verified correct.
                    target->GotoFrame(frameNumber + sceneOffset);
                    target->SetPlayState(state);
                }
                
                env->Drop1();
                // check if it is necessary to terminate the execution, because the target 
                // is unloaded or unloading. See comments for NeedTermination for more details.
                if (env->NeedTermination(execType))
                    execContext.NextPC = execContext.StopPC;
                break;
            }
            }
        }

#ifdef SF_AMP_SERVER
        if (bufferTimes != NULL)
        {
            SF_ASSERT(execContext.PC < (int)bufferTimes->GetSize());
            (*bufferTimes)[execContext.PC] += viewStats.GetInstructionTime(samplingPeriod);
        }
#endif
        execContext.PC = execContext.NextPC;

        #ifdef GFX_AS2_VERBOSE
        if (execContext.VerboseAction) 
            execContext.LogF.LogAction ("==== stack top index %d ====\n", env->GetTopIndex());
        #endif
    }

#ifdef SF_AMP_SERVER
    // Unlock the instruction times buffer
    if (bufferTimes)
    {
        env->GetMovieImpl()->GetAdvanceStats().ReleaseBufferInstructionTimes();
    }
#endif

    GASCheckBuffer(tmpStr1Buf);
    GASCheckBuffer(tmpStr2Buf);
    #ifdef GFX_AS2_VERBOSE
    GASCheckBuffer(tmpStr3Buf);
    #endif
    GASCheckBuffer(valBuf1);
    GASCheckBuffer(valBuf2);
    GASCheckBuffer(valBuf3);
    GASCheckBuffer(funcBuf);
    GASCheckBuffer(fnCallBuf);

    // Exception handling
    // check, if we still have finally blocks to execute.
    // It may end up here when "return" occurs from inside of the "try" or "catch" blocks
    if (tryCount > 0)
    {
        for (int tc = tryCount; tc > 0; --tc)
        {
            Environment::TryDescr tryDescr = env->PopTryBlock();

            // restore stack pointer
            unsigned topIndex = env->GetTopIndex();
            if (topIndex > tryDescr.TopStackIndex)
                env->Drop(topIndex - tryDescr.TopStackIndex);
            if (tryDescr.IsFinallyBlockFlag())
            {
                // ok, we have a "finally" block. Need to redirect execution to there 
                // and then continue throwing.
                execContext.PC = tryDescr.TryBeginPC + tryDescr.GetTrySize() + tryDescr.GetCatchSize();
                env->SetUnrolling();
                Execute(env, execContext.PC, tryDescr.GetFinallySize(), retval, execContext.WithStack, execType);
                env->ClearUnrolling();
            }
        }
    }
    env->LeavingExecution();
    if (env->IsExecutionNestingLevelZero() && env->IsThrowing())
    {
        // unhandled exception?
        env->ClearThrowing();
    }
    
    // AB: if originally set target was "invalid" (means tellTarget was invoked with incorrect
    // target as a parameter) then we need restore it as "invalid"...
    if (isOriginalTargetValid)
        env->SetTarget(execContext.pOriginalTarget);
    else
        env->SetInvalidTarget(execContext.pOriginalTarget);
    #ifdef GFX_AS2_VERBOSE
    if (execContext.VerboseAction) 
        execContext.LogF.LogAction("\n");
    #endif
}


//
// Environment
//
void Environment::CheckTryBlocks(int pc, int* plocalTryBlockCount)
{
    // Check, are we still inside the try blocks or not.
    // If not - remove them from TryBlocks array.
    SF_ASSERT(plocalTryBlockCount);
    int localTryBlockCount = *plocalTryBlockCount;
    if (localTryBlockCount > 0)
    {
        for (; TryBlocks.GetSize() > 0 && localTryBlockCount >= 0; --localTryBlockCount)
        {
            const TryDescr& tryDescr = TryBlocks[TryBlocks.GetSize() - 1];
            if ((unsigned)pc < tryDescr.TryBeginPC || (unsigned)pc >= tryDescr.TryBeginPC + tryDescr.GetTrySize())
            {
                // need to remove this block since pc is out-of-bound this try-block
                --(*plocalTryBlockCount);
                TryBlocks.Resize(TryBlocks.GetSize() - 1);
            }
            else
                break;
        }
    }
}

// Handle exception (invoke catch, finally, etc) if necessary.
int Environment::CheckExceptions(ActionBuffer* pactBuf, 
                                    int nextPc, 
                                    int* plocalTryBlockCount, 
                                    Value* retval,
                                    const WithStackArray* pwithStack,
                                    ActionBuffer::ExecuteType execType)
{
    SF_ASSERT(pactBuf);
    bool needContinueExecution;
    do 
    {
        needContinueExecution = false;
        if (IsThrowing())
        {
            for (int tc = *plocalTryBlockCount; tc > 0 && !needContinueExecution; --tc)
            {
                Environment::TryDescr tryDescr = PopTryBlock();
                --(*plocalTryBlockCount);

                // restore stack pointer
                unsigned topIndex = GetTopIndex();
                if (topIndex > tryDescr.TopStackIndex)
                    Drop(topIndex - tryDescr.TopStackIndex);

                // do we have catch here?
                if (tryDescr.IsCatchBlockFlag())
                {
                    int localStackTop = 0;
                    // yes - put the thrown object in var/reg, 
                    // redirect execution to the catch block, clear throw state.
                    // It will run "finally" automatically.
                    if (tryDescr.IsCatchInRegister())
                        *(LocalRegisterPtr(tryDescr.GetCatchRegister())) = GetThrowingValue();
                    else
                    {
                        // create new local frame and save catch variable there
                        localStackTop = GetLocalFrameTop();
                        Ptr<LocalFrame> prevLocalFrame = GetTopLocalFrame();
                        Ptr<LocalFrame> curLocalFrame  = CreateNewLocalFrame();
                        curLocalFrame->PrevFrame = prevLocalFrame;

                        SetLocal(tryDescr.GetCatchName(this), GetThrowingValue());
                    }
                    ClearThrowing();
                    int catchPc = tryDescr.TryBeginPC + tryDescr.GetTrySize();

                    pactBuf->Execute(this, catchPc, tryDescr.GetCatchSize(), retval, pwithStack, execType);

                    nextPc = tryDescr.TryBeginPC + tryDescr.GetTrySize() + tryDescr.GetCatchSize() + tryDescr.GetFinallySize();

                    needContinueExecution = true;

                    if (tryDescr.IsCatchInRegister())
                        *(LocalRegisterPtr(tryDescr.GetCatchRegister())) = Value();
                    else
                    {
                        // Clean up stack frame.
                        SetLocalFrameTop(localStackTop);
                    }
                }
                if (tryDescr.IsFinallyBlockFlag())
                {
                    // ok, we have a "finally" block. Need to redirect execution to there 
                    // and then continue throwing.
                    int finallyPc = tryDescr.TryBeginPC + tryDescr.GetTrySize() + tryDescr.GetCatchSize();
                    SetUnrolling();
                    pactBuf->Execute(this, finallyPc, tryDescr.GetFinallySize(), retval, pwithStack, execType);
                    ClearUnrolling();
                }
            }
            if (!needContinueExecution)
                return pactBuf->GetLength(); // forces to exit
            // continue
        }
    } while (needContinueExecution);
    return nextPc;
}

void Environment::PushTryBlock(const TryDescr& descr)
{
    TryBlocks.PushBack(descr);
}
Environment::TryDescr Environment::PopTryBlock()
{
    SF_ASSERT(TryBlocks.GetSize() > 0);
    TryDescr descr = TryBlocks[TryBlocks.GetSize()-1];
    TryBlocks.Resize(TryBlocks.GetSize()-1);
    return descr;
}
Environment::TryDescr& Environment::PeekTryBlock()
{
    SF_ASSERT(TryBlocks.GetSize() > 0);
    return TryBlocks[TryBlocks.GetSize()-1];
}

bool Environment::IsInsideTryBlock(int pc)
{
    if (pc < 0 || TryBlocks.GetSize() == 0)
        return false;
    const TryDescr& tryDescr = TryBlocks[TryBlocks.GetSize()-1];
    return ((unsigned)pc >= tryDescr.TryBeginPC && (unsigned)pc < tryDescr.TryBeginPC + tryDescr.GetTrySize());
}

bool Environment::IsInsideCatchBlock(int pc)
{
    if (pc < 0 || TryBlocks.GetSize() == 0)
        return false;
    const TryDescr& tryDescr = TryBlocks[TryBlocks.GetSize()-1];
    unsigned catchBeginPc = tryDescr.TryBeginPC + tryDescr.GetTrySize();
    return ((unsigned)pc >= catchBeginPc && (unsigned)pc < catchBeginPc + tryDescr.GetCatchSize());
}

bool Environment::IsInsideFinallyBlock(int pc)
{
    if (pc < 0 || TryBlocks.GetSize() == 0)
        return false;
    const TryDescr& tryDescr = TryBlocks[TryBlocks.GetSize()-1];
    unsigned finallyBeginPc = tryDescr.TryBeginPC + tryDescr.GetTrySize() + tryDescr.GetCatchSize();
    return ((unsigned)pc >= finallyBeginPc && (unsigned)pc < finallyBeginPc + tryDescr.GetFinallySize());
}

// This GetMember method resolves properties (invokes getter methods) 
// and members (in the case if _resolve handler is set).
bool Environment::GetMember(ObjectInterface* pthisObj, 
                               const ASString& memberName, 
                               Value* pdestVal)
{
    SF_ASSERT(pthisObj && pdestVal);

    bool rv = pthisObj->GetMember(this, memberName, pdestVal);
    if (rv && pdestVal->IsProperty())
    {
        // resolve property - invoke getter
        // resolve property
        // check, is the object 'this' came from InteractiveObject. If so,
        // use original InteractiveObject as this.
        pdestVal->GetPropertyValue(this, pthisObj, pdestVal);
    }
    else if (pdestVal->IsResolveHandler())
    {
        FunctionRef resolveHandler = pdestVal->ToResolveHandler();
        if (resolveHandler != NULL)
        {
            Push(memberName);
            pdestVal->SetUndefined();
            resolveHandler.Invoke(FnCall
                (pdestVal, 
                pthisObj,  
                this, 1, GetTopIndex()));
            Drop1();
            rv = true;
        }
        else
            rv = false;
    }
    return rv;
}

// Target functions placed here because AS2/AS2_Action.h does not know InteractiveObject
void    Environment::SetTarget(InteractiveObject* ptarget) 
{ 
    Target          = ptarget; 
    IsInvalidTarget = false;
    StringContext.UpdateVersion(ptarget->GetVersion());
}
void    Environment::SetInvalidTarget(InteractiveObject* ptarget) 
{ 
    Target          = ptarget; 
    IsInvalidTarget = true;
    StringContext.UpdateVersion(ptarget->GetVersion());
}

// Used to set target right after construction
void    Environment::SetTargetOnConstruct(InteractiveObject* ptarget)
{    
    SetTarget(ptarget);
    StringContext.pContext = ToAvmCharacter(ptarget)->GetGC();  
}

// GFxLogBase overrides to support logging correctly
void        Environment::LogScriptError(const char* pfmt,...) const
{
    va_list argList; va_start(argList, pfmt);
    if(pASLogger)
        pASLogger->LogScriptMessageVarg(Log_ScriptError,pfmt,argList);
    else if (GetLog())
        GetLog()->LogMessageVarg(Log_ScriptError, pfmt, argList);
    va_end(argList);
}

void        Environment::LogScriptWarning(const char* pfmt,...) const
{
    va_list argList; va_start(argList, pfmt);
    if(pASLogger)
        pASLogger->LogScriptMessageVarg(Log_ScriptWarning,pfmt,argList);
    else if (GetLog())
        GetLog()->LogMessageVarg(Log_ScriptWarning, pfmt, argList);
    va_end(argList);
}

void        Environment::LogScriptMessage(const char* pfmt,...) const
{
    va_list argList; va_start(argList, pfmt);
    if(pASLogger)
        pASLogger->LogScriptMessageVarg(Log_ScriptMessage, pfmt, argList);
    else if (GetLog())
        GetLog()->LogMessageVarg(Log_ScriptMessage,pfmt,argList);
    va_end(argList);
}

Log*     Environment::GetLog() const
{
    return Target->GetLog();
}

bool        Environment::IsVerboseAction() const
{   
    return Target->GetMovieImpl()->IsVerboseAction();
}

bool        Environment::IsVerboseActionErrors() const
{   
    return !Target->GetMovieImpl()->IsSuppressActionErrors();
}

MovieImpl*   Environment::GetMovieImpl() const
{   
    return Target->GetMovieImpl();
}

MovieRoot*   Environment::GetAS2Root() const
{   
    return static_cast<MovieRoot*>(Target->GetMovieImpl()->pASMovieRoot.GetPtr());
}

void Environment::InvalidateOptAdvanceList() const
{
    if (Target)
        Target->GetMovieImpl()->InvalidateOptAdvanceList();
}

ASRefCountCollector* Environment::GetCollector()
{
    if (Target)
        return ToAvmCharacter(Target)->GetAS2Root()->GetASGC();
    return NULL;
}

bool        Environment::FindAndGetVariableRaw(const Environment::GetVarParams& params) const
{
    // GFxPath lookup rigmarole.
    // NOTE: IsPath caches 'NotPath' bit in varname string for efficiency.
    bool retVal;
    if (params.VarName.IsNotPath() || !IsPath(params.VarName))
    {
        retVal = GetVariableRaw(params);
    }
    else
    {
        Value owner;
        retVal = FindVariable(GetVarParams(params.VarName, params.pResult, params.pWithStack, params.ppNewTarget, &owner, params.ExcludeFlags), false);
        if (owner.IsUndefined())
        {
            if (!(params.ExcludeFlags & NoLogOutput))
                LogScriptError(" GetVariable failed: can't resolve the path \"%s\"", 
                params.VarName.ToCStr());
            retVal = false;
        }
        else
        {
            if (params.pOwner)
                *params.pOwner = owner;
        }
    }
    return retVal;
}

// Return the value of the given var, if it's defined.
// *ppnewTarget receives a new value if it changes.
// excludeFlags - mask consisted of ExcludeFlags bits.
bool        Environment::GetVariable(const ASString& varname, 
                                        Value* presult,
                                        const WithStackArray* pwithStack, 
                                        InteractiveObject **ppnewTarget,
                                        Value* powner,
                                        unsigned excludeFlags)
{
    // GFxPath lookup rigmarole.
    // NOTE: IsPath caches 'NotPath' bit in varname string for efficiency.
    bool retVal = FindAndGetVariableRaw(GetVarParams(varname,
                                                     presult,
                                                     pwithStack,
                                                     ppnewTarget,
                                                     powner,
                                                     excludeFlags));
    if (retVal && presult->IsProperty())
    {
        // resolve property. There is no explicit object here, so we assume
        // the owner of the property is "this".
        // check, is the object 'this' came from InteractiveObject. If so,
        // use original InteractiveObject as this.
        ObjectInterface*  pobj = NULL;
        Value             thisVal;
        if (GetVariableRaw(GetVarParams(GetBuiltin(ASBuiltin_this), &thisVal, pwithStack)))
        {
            pobj    = thisVal.ToObject(this);
            if (!pobj)
                pobj = thisVal.ToAvmCharacter(this);
        }
        else
            pobj = ToAvmCharacter(GetTarget());
        presult->GetPropertyValue(this, pobj, presult);
    }
    else if (presult->IsResolveHandler())
    {
        ObjectInterface*  pobj = NULL;
        Value             thisVal;
        if (GetVariable(GetBuiltin(ASBuiltin_this), &thisVal, pwithStack))
        {
            pobj    = thisVal.ToObject(this);
            if (!pobj)
                pobj = thisVal.ToAvmCharacter(this);
        }
        else
            pobj = ToAvmCharacter(GetTarget());

        FunctionRef resolveHandler = presult->ToResolveHandler();
        Push(varname);
        presult->SetUndefined();
        resolveHandler.Invoke(FnCall
            (presult, pobj, this, 1, GetTopIndex()));
        Drop1();
    }
    return retVal;
}


// Determine if a specified variable is available for access.
bool    Environment::IsAvailable(const ASString& varname, const WithStackArray* pwithStack) const
{
    if(varname.IsEmpty())
        return false;

    // *** GFxPath lookup.
    InteractiveObject* target = Target;
    ASString       path(GetBuiltin(ASBuiltin_empty_));
    ASString       var(GetBuiltin(ASBuiltin_empty_));
    Value        val;
    ASStringContext* psc = GetSC();

    if (FindAndGetVariableRaw(GetVarParams(varname, &val, pwithStack, NULL, NULL, NoLogOutput)))
        return true;

    //!AB: do we still need this here after GetVariable?
    if (ParsePath(psc, varname, &path, &var))
    {
        if ((target = FindTarget(path, NoLogOutput))==0)
            return 0;
        return ToAvmCharacter(target)->GetMemberRaw(psc, var, &val);
    }

    // *** NOT Path, do RAW variable check.

    if (pwithStack)
    {
        // Check the with-stack.
        for (int i = (int)pwithStack->GetSize() - 1; i >= 0; i--)
        {
            ObjectInterface* obj = (*pwithStack)[i].GetObjectInterface();
            // Found the var in this context ?
            if (obj && obj->GetMemberRaw(psc, varname, &val))
                return true;
        }
    }

    if (FindLocal(varname))
        return true;
    if (Target && GetAvmTarget()->GetMemberRaw(psc, varname, &val))
        return true;
    
    // Pre-canned names.
    if (IsCaseSensitive())
    {
        if (GetBuiltin(ASBuiltin_this) == varname ||
            GetBuiltin(ASBuiltin__root) == varname ||
            GetBuiltin(ASBuiltin__global) == varname)
            return true;
    }
    else
    {
        varname.ResolveLowercase();
        if (GetBuiltin(ASBuiltin_this).CompareBuiltIn_CaseInsensitive_Unchecked(varname) ||
            GetBuiltin(ASBuiltin__root).CompareBuiltIn_CaseInsensitive_Unchecked(varname) ||
            GetBuiltin(ASBuiltin__global).CompareBuiltIn_CaseInsensitive_Unchecked(varname))
            return true;
    }

    // Check for _levelN.
    if (varname[0] == '_')
    {
        const char* ptail = 0;
        int         level = MovieRoot::ParseLevelName(varname.ToCStr(), &ptail,
                                                      IsCaseSensitive());
        if ((level != -1) && !*ptail )
        {
            if (GetAS2Root()->GetLevelMovie(level))
                return true;
        }
    }

    Object* pglobal = GetGC()->pGlobal;
    if (pglobal && pglobal->GetMemberRaw(psc, varname, &val))       
        return true;

    //  Not found -> not available.
    return false;
}

// excludeFlags: IgnoreWithoutOwners, IgnoreLocals
// varname must be a plain variable name; no path parsing.
bool    Environment::GetVariableRaw(const Environment::GetVarParams& params) const
{
    // Debug checks: these should never be called for path variables.
    SF_ASSERT(strchr(params.VarName.ToCStr(), ':') == NULL);
    SF_ASSERT(strchr(params.VarName.ToCStr(), '/') == NULL);
    SF_ASSERT(strchr(params.VarName.ToCStr(), '.') == NULL);

    if (!params.pResult) return false;

    if (params.pOwner)
        *params.pOwner = 0;
    // Check the with-stack.
    if (params.pWithStack)
    {
        for (int i = (int)params.pWithStack->GetSize() - 1; i >= 0; i--)
        {
            ObjectInterface* obj = (*params.pWithStack)[i].GetObjectInterface();
            
            if (obj == NULL)
            {
                //!AB, looks like, invalid obj in withStack should stop lookup
                // and return "undefined"
                return false;
            }

            if (obj && obj->GetMember(const_cast<Environment*>(this), params.VarName, params.pResult))
            {
                // Found the var in this context.
                if (params.pOwner)
                {
                    // Object interfaces can be characters or objects only.
                    if (obj->IsASCharacter())
                        params.pOwner->SetAsCharacter(obj->ToCharacter());
                    else
                        params.pOwner->SetAsObject(obj->ToASObject());
                }
                return true;
            }
        }
    }

    if (!(params.ExcludeFlags & IgnoreLocals))
    {
        // Check locals.
        if (FindLocal(params.VarName, params.pResult))
        {
            // Get local var.
            return true;
        }
        else if (GetVersion() >= 5)
        {
            if (GetBuiltin(ASBuiltin_arguments).CompareBuiltIn_CaseCheck(params.VarName, IsCaseSensitive()))  
            {
                // lazy initialization of "arguments" array, required for Function1.
                LocalFrame* pLocFr = GetTopLocalFrame(0);
                if (pLocFr)
                {
                    Environment* penv = const_cast<Environment*>(this);
                    ASStringContext *psc = GetSC();

                    Ptr<ArrayObject> pargArray = *SF_HEAP_NEW(GetHeap()) ArrayObject(penv);
                    pargArray->Resize(pLocFr->NArgs);
                    for (int i = 0; i < pLocFr->NArgs; i++)
                        pargArray->SetElement(i, pLocFr->Arg(i));

                    penv->AddLocal(psc->GetBuiltin(ASBuiltin_arguments), Value(pargArray.GetPtr()));
                    pargArray->SetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_callee), pLocFr->Callee, 
                        PropFlags::PropFlag_DontEnum | PropFlags::PropFlag_DontDelete | PropFlags::PropFlag_ReadOnly);
                    pargArray->SetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_caller), pLocFr->Caller, 
                        PropFlags::PropFlag_DontEnum | PropFlags::PropFlag_DontDelete | PropFlags::PropFlag_ReadOnly);
                    params.pResult->SetAsObject(pargArray);
                    return true;
                }
            }
            else if (GetVersion() >= 6)
            {
                // Looking for "super" in SWF6?            
                if (GetBuiltin(ASBuiltin_super).CompareBuiltIn_CaseCheck(params.VarName, IsCaseSensitive()))                
                {
                    LocalFrame* pLocFr = GetTopLocalFrame(0);
                    if (pLocFr && pLocFr->SuperThis)
                    {
                        ObjectInterface* const iobj = pLocFr->SuperThis;
                        Ptr<Object> superObj;
                        Ptr<Object> proto = iobj->Get__proto__();
                        //printf ("!!! savedThis.__proto__ = %s\n", (const char*)GetGC()->FindClassName(proto));
                        //printf ("!!! real pthis.__proto__ = %s\n", (const char*)GetGC()->FindClassName(pthis->Get__proto__(this)));

                        if (proto)
                        {
                            // need to get a real "this" to save in "super"
                            Value thisVal;
                            FindAndGetVariableRaw(GetVarParams(GetBuiltin(ASBuiltin_this), &thisVal, params.pWithStack));

                            FunctionRef __ctor__ = proto->Get__constructor__(GetSC());
                            //printf ("!!! __proto__.__ctor__ = %s\n", (const char*)pourEnv->GetGC()->FindClassName(__ctor__.GetObjectPtr()));
                            //printf ("!!! __proto__.__proto__ = %s\n", (const char*)pourEnv->GetGC()->FindClassName(proto->Get__proto__(pourEnv)));
                            superObj = *SF_HEAP_NEW(GetHeap()) SuperObject(proto->Get__proto__(), thisVal.ToObjectInterface(this), __ctor__);

                            params.pResult->SetAsObject(superObj);
                            //!AB, don't like this hack, but it is better to cache the "super"...
                            // don't want to remove the constness from GetVariable/Raw either
                            const_cast<Environment*>(this)->SetLocal(GetBuiltin(ASBuiltin_super), *params.pResult);
                            return true;
                        }
                    }
                }
            }
        }

        // Looking for "this"?
        if (GetBuiltin(ASBuiltin_this).CompareBuiltIn_CaseCheck(params.VarName, IsCaseSensitive()))
        {
            params.pResult->SetAsCharacter(Target);
            return true;
        }
    }

    // Check InteractiveObject members.
    if (!Target) // Target should not be null, but just in case..
        return false;
    if (GetAvmTarget()->GetMemberRaw(GetSC(), params.VarName, params.pResult))
    {
        if (params.pOwner)
            *params.pOwner = Target;
        return true; 
    }

    Object* pglobal = GetGC()->pGlobal;
    if (!(params.ExcludeFlags & IgnoreContainers))
    {
        // Check built-in constants.
        if (params.VarName.GetLength() > 0 && params.VarName[0] == '_')
        {
            Bool3W rv = CheckGlobalAndLevels(params);
            if (rv.IsDefined())
                return rv.IsTrue();
        }
    }

    if (pglobal && pglobal->GetMember(const_cast<Environment*>(this), params.VarName, params.pResult))
    {
        if (params.pOwner)
            *params.pOwner = pglobal;
        return true;
    }

    // Fallback.
    if (!(params.ExcludeFlags & NoLogOutput))
        LogAction("GetVariableRaw(\"%s\") failed, returning UNDEFINED.\n", params.VarName.ToCStr());
    return false;
}

Bool3W    Environment::CheckGlobalAndLevels(const Environment::GetVarParams& params) const
{
    // Handle _root and _global.
    if (IsCaseSensitive())
    {
        if (GetBuiltin(ASBuiltin__root) == params.VarName)
        {
            params.pResult->SetAsCharacter(Target->GetTopParent());
            return Bool3W(true);
        }
        else if (GetBuiltin(ASBuiltin__global) == params.VarName)
        {
            Object* pglobal = GetGC()->pGlobal;
            params.pResult->SetAsObject(pglobal);
            return Bool3W(true);
        }
    }
    else
    {
        if (GetBuiltin(ASBuiltin__root).CompareBuiltIn_CaseInsensitive(params.VarName))
        {
            params.pResult->SetAsCharacter(Target->GetTopParent());
            return Bool3W(true);
        }
        else if (GetBuiltin(ASBuiltin__global).CompareBuiltIn_CaseInsensitive(params.VarName))
        {
            Object* pglobal = GetGC()->pGlobal;
            params.pResult->SetAsObject(pglobal);
            return Bool3W(true);
        }
    }

    // Check for _levelN.
    const char* ptail = 0;
    int         level = MovieRoot::ParseLevelName(params.VarName.ToCStr(), &ptail,
                                                  IsCaseSensitive());
    if ((level != -1) && !*ptail)
    {
        InteractiveObject* lm = GetAS2Root()->GetLevelMovie(level);
        if (lm)
        {
            params.pResult->SetAsCharacter(lm);
            return Bool3W(true);
        }
        return Bool3W(false);
    }
    return Bool3W();
}

// varname must be a plain variable name; no path parsing.
bool    Environment::FindOwnerOfMember(const ASString& varname,
                                          Value* presult,
                                          const WithStackArray* pwithStack) const
{
    SF_ASSERT(strchr(varname.ToCStr(), ':') == NULL);
    SF_ASSERT(strchr(varname.ToCStr(), '/') == NULL);
    SF_ASSERT(strchr(varname.ToCStr(), '.') == NULL); 

    if (!presult) return false;

    if (pwithStack)
    {
        // Check the with-stack.
        for (int i = (int)pwithStack->GetSize() - 1; i >= 0; i--)
        {
            ObjectInterface* obj = (*pwithStack)[i].GetObjectInterface();
            if (obj && obj->HasMember(GetSC(), varname, false))
            {
                // Found the var in this context.
                // Object interfaces can be characters or objects only.
                if (obj->IsASCharacter())
                    presult->SetAsCharacter(obj->ToCharacter());
                else
                    presult->SetAsObject(static_cast<Object*>(obj));
                return true;
            }
        }
    }

    // Check GFxMovieSub members.
    if (!Target) // Target should not be null, but just in case..
        return false;
    if (GetAvmTarget()->HasMember(GetSC(), varname, false))  
    {
        presult->SetAsCharacter(Target);
        return true;  
    }

    Object* pglobal = GetGC()->pGlobal;
    if (pglobal && pglobal->HasMember(GetSC(), varname, false))
    {
        presult->SetAsObject(pglobal);
        return true;  
    }
    return false;
}

// Given a path to variable, set its value.
bool    Environment::SetVariable(
                const ASString& varname,
                const Value& val,
                const WithStackArray* pwithStack,
                bool doDisplayErrors)
{
    if (IsVerboseAction())
    {
        ASString vs(Value(val).ToDebugString(this));
        LogAction("-------------- %s = %s\n", varname.ToCStr(), vs.ToCStr());
    }

    // GFxPath lookup rigamarole.
    // NOTE: IsPath caches 'NotPath' bit in varname string for efficiency.
    if (varname.IsNotPath() || !IsPath(varname))
    {
        SetVariableRaw(varname, val, pwithStack);
        // Essentially SetMember, which should not fail.
        return true;
    }
    else
    {
        Value owner;
        ASString var(GetBuiltin(ASBuiltin_empty_));
        Value curval;
        FindVariable(GetVarParams(varname, &curval, pwithStack, 0, &owner), false, &var);
        if (!owner.IsUndefined())
        {
            ObjectInterface* pobj = owner.ToObjectInterface(this);
            SF_ASSERT(pobj); // owner always should be an object!
            if (pobj)
            {
                pobj->SetMember(this, var, val);
                return true;
            }
        }
        else
        {
            if (doDisplayErrors && IsVerboseActionErrors())
                LogScriptError("SetVariable failed: can't resolve the path \"%s\"", varname.ToCStr());
        }
    }    
    return false;
}

// No path rigamarole.
void    Environment::SetVariableRaw(
            const ASString& varname, const Value& val,
            const WithStackArray* pwithStack)
{
    if (pwithStack)
    {
        // Check the with-stack.
        for (int i = (int)pwithStack->GetSize() - 1; i >= 0; i--)
        {
            ObjectInterface* obj = (*pwithStack)[i].GetObjectInterface();
            Value    dummy;
            if (obj && obj->GetMember(this, varname, &dummy))
            {
                // This object has the member; so set it here.
                obj->SetMember(this, varname, val);
                return;
            }
        }
    }

    // Check locals.
    Value* value = FindLocal(varname);
    if (value)
    {
        // Set local var.
        *value = val;
        return;
    }

    SF_ASSERT(Target);
    GetAvmTarget()->SetMember(this, varname, val);
}


// Set/initialize the value of the local variable.
void    Environment::SetLocal(const ASString& varname, const Value& val)
{
    if (LocalFrames.GetSize() == 0) //!AB if no local frame exist - do nothing
        return; 
    if (!LocalFrames[LocalFrames.GetSize()-1])
        return; 

    // Is it in the current frame already?
    Value* pvalue = FindLocal(varname);
    if (!pvalue)
    {
        SF_ASSERT(!varname.IsEmpty());   // null varnames are invalid!
        
        // Not in frame; create a new local var.
        AddLocal (varname, val);
        //LocalFrames.PushBack(GASFrameSlot(varname, val));
    }
    else
    {
        // In frame already; modify existing var.
        //LocalFrames[index].Value = val;
        *pvalue = val;
    }
}


// Add a local var with the given name and value to our
// current local frame.  Use this when you know the var
// doesn't exist yet, since it's faster than SetLocal();
// e.G. when setting up args for a function.
void    Environment::AddLocal(const ASString& varname, const Value& val)
{
    SF_ASSERT(varname.GetSize() > 0);

    //if (LocalFrames.GetSize() == 0) //!AB Create frame if no frame exists
    //  CreateNewLocalFrame();
    //!AB Don't do this, local frame should be created intentionally. If no local frame
    // exists - ASSERT.
    SF_ASSERT(LocalFrames.GetSize() > 0);

    Ptr<LocalFrame> frame = LocalFrames[LocalFrames.GetSize()-1];
    if (frame)
        frame->Variables.SetCaseCheck(varname, val, IsCaseSensitive());
}


// Create the specified local var if it doesn't exist already.
void    Environment::DeclareLocal(const ASString& varname)
{
    if (LocalFrames.GetSize() == 0) //!AB if no local frame exist - do nothing
        return; 
    if (!LocalFrames[LocalFrames.GetSize()-1])
        return; 

    // Is it in the current frame already?
    Value* pvalue = FindLocal(varname);
    if (!pvalue)
    {
        // Not in frame; create a new local var.
        SF_ASSERT(varname.GetSize() > 0);   // null varnames are invalid!
        
        AddLocal(varname, Value());
    }
    else
    {
        // In frame already; don't mess with it.
    }
}

Value*   Environment::LocalRegisterPtr(unsigned reg)
// Return a pointer to the specified local register.
// Local registers are numbered starting with 0.
// MA: Although DefineFunction2 index 0 has a special meaning,
// ActionScript code store_register still references it occasionally.
//
// Return value will never be NULL.  If reg is out of bounds,
// we log an error, but still return a valid Pointer (to
// global reg[0]).  So the behavior is a bit undefined, but
// not dangerous.
{
    // We index the registers from the end of the register
    // Array, so we don't have to keep base/frame
    // pointers.

    if (reg >= LocalRegister.GetSize())
    {       
        LogError("Invalid local register %d, stack only has %d entries",
                 reg, (int)LocalRegister.GetSize());

        // Fallback: use global 0.
        return &GlobalRegister[0];
    }

    return &LocalRegister[LocalRegister.GetSize() - reg - 1];
}

const Value* Environment::FindLocal(const ASString& varname) const
{
    if (LocalFrames.GetSize() > 0) 
    {
        Ptr<LocalFrame> localFrame = GetTopLocalFrame();
        if (!localFrame) return 0;
        do {
            const Value* ptr = localFrame->Variables.GetCaseCheck(varname, IsCaseSensitive());
            if (ptr)
                return ptr;
            else 
            {
                int version = GetVersion();
                if ((version >= 5 && GetBuiltin(ASBuiltin_arguments).CompareBuiltIn_CaseCheck(varname, IsCaseSensitive())) ||
                    (version >= 6 && GetBuiltin(ASBuiltin_super).CompareBuiltIn_CaseCheck(varname, IsCaseSensitive())))  
                {
                    // do not propagate requests for "arguments" and "super" in upper local frames;
                    // they would be substituted in GetVariableRaw.
                    break;
                }
            }

            localFrame = localFrame->PrevFrame;
        } while (localFrame);
    }
    return NULL;
}

LocalFrame* Environment::GetTopLocalFrame (int off) const 
{ 
    if (LocalFrames.GetSize() - off > 0)
    {
        return LocalFrames[LocalFrames.GetSize() - off - 1]; 
    }
    return 0;
}

LocalFrame*  Environment::CreateNewLocalFrame ()
{
    Ptr<LocalFrame> frame = *SF_HEAP_NEW(GetHeap()) LocalFrame(GetCollector());
    LocalFrames.PushBack(frame);
    return frame;
}


// See if the given variable name is actually a sprite path
// followed by a variable name.  These come in the format:
//
//  /path/to/some/sprite/:varname
//
// (or same thing, without the last '/')
//
// or
//  path.To.Some.Var
//
// If that's the format, puts the path Part (no colon or
// trailing slash) in *path, and the varname Part (no color)
// in *var and returns true.
//
// If no colon, returns false and leaves *path & *var alone.
bool    Environment::ParsePath(ASStringContext* psc, const ASString& varPath, ASString* ppath, ASString* pvar)
{
    // Search for colon.
    int         colonIndex = -1;
    const char* cstr = varPath.ToCStr();
    const char* p;
    
    if ((p = strchr(cstr, ':')) != 0)
        colonIndex = int(p - cstr);
    else
    {
        if ((p = strrchr(cstr, '.')) != 0)
            colonIndex = int(p - cstr);
        else
        {
            if ((p = strrchr(cstr, '/')) == 0)
                return false;
        }
    }

    // Make the subparts.

    // Var.
    if (colonIndex >= 0)
        *pvar = psc->CreateString(varPath.ToCStr() + colonIndex + 1);
    else
        *pvar = psc->GetBuiltin(ASBuiltin_empty_);

    // GFxPath.
    if (colonIndex > 0)
    {
        if (varPath[colonIndex - 1] == '/')
        {
            // Trim off the extraneous trailing slash.
            colonIndex--;
        }
    }
    
    // @@ could be better.  This whole usage of String is very flabby...
    if (colonIndex >= 0)
        *ppath = psc->CreateString(varPath.ToCStr(), colonIndex);
    else
        *ppath = varPath;

    return true;
}

bool    Environment::IsPath(const ASString& varPath)
{
    // This should be checked for externally, for efficiency.   
    //if (varPath.IsNotPath())
    //    return 0;
    SF_ASSERT(varPath.IsNotPath() == false);
    if (varPath.GetHashFlags() & ASString::Flag_PathCheck) // IS a path!
        return 1;  

    // This string was not checked for being a path yet, so do the check.
    const char* pstr = varPath.ToCStr();
    if (strchr(pstr, ':') != NULL ||
        strchr(pstr, '/') != NULL ||
        strchr(pstr, '.') != NULL)
    {
        // This *IS* a path
        varPath.SetPathFlags(ASString::Flag_PathCheck|0);
        return 1;
    }
    else
    {
        varPath.SetPathFlags(ASString::Flag_PathCheck|ASString::Flag_IsNotPath);
        return 0;
    }
}

// Find the sprite/Movie represented by the given value.  The
// value might be a reference to the object itself, or a
// string giving a relative path name to the object.
InteractiveObject* Environment::FindTargetByValue(const Value& val)
{
    if (val.GetType() == Value::OBJECT)
    {   
        // TBD: Can object be convertible to string path? Should it be?
    }
    if (val.GetType() == Value::CHARACTER)
    {
        return val.ToCharacter(this);
    }
    else if (val.GetType() == Value::STRING)
    {
        return FindTarget(val.ToString(this));
    }
    else
    {
        LogScriptError("Invalid movie clip path; neither string nor object");
    }
    return NULL;
}

// This method determines necessity of actions' termination. In certain cases
// the actions being executed might be terminated. This is happening when the
// environment owner is unloading or already unloaded. This unloading may 
// happen because of gotoFrame, where the target frame doesn't contain the
// environment owner sprite. Though, Flash demonstrates different behavior that
// depends on type of executing code and presence of on-unload handler:
// 1) if the executing code is frame actions or the on()-type event handler (on(press),
//    for example) then the actions are terminated if the owner sprite is either 
//    UNLOADING or UNLOADED. 
//    This means, that regardless to presence or absence of the onUnload event handler
//    the execution of frame actions and on()-events is terminated, once the owner is 
//    going to die. But this is not true for onClipEvent(unload) handlers: their
//    execution is not terminated.
// 2) If the executing code is a function or function style event handler (such as
//    OnEnterFrame) then execution is terminated only if the owner sprite doesn't 
//    have the onUnload handler. If it has the onUnload handler and is going to die
//    then execution continues.
// Note, Flash (and GFx) checks the currently set target for being unloaded. But if the
// target is changed by the "tellTarget" then the ActionScript is not terminated, even
// if the real target ("this") is unloaded already. The "this" character becomes
// completely wiped out: there are no name, no methods, no local variables exist for it.
// The only accessible things are _global and _root.
// ActionScript execution may be terminated later, if call methods on existing characters
// in _root. It is not terminated if we try to invoke methods on that wiped out "this".
bool Environment::NeedTermination(ActionBuffer::ExecuteType execType) const
{
    SF_ASSERT(Target); 
    if ((execType == ActionBuffer::Exec_Unknown || execType == ActionBuffer::Exec_Event) && 
        Target->IsUnloading())
        return true;
    else if (Target->IsUnloaded())
        return true;
    return false;
}

// Search for next '.' or '/' DisplayObject in this word.  Return
// a pointer to it, or to NULL if it wasn't found.
static const char*  GAS_NextSlashOrDot(const char* pword)
{
    for (const char* p = pword; *p; p++)
    {
        if (*p == '.' && p[1] == '.')
        {
            p++;
        }
        else if (*p == '.' || *p == '/')
        {
            return p;
        }
    }

    return NULL;
}

class StringTokenizer 
{
    const char* Str;
    const char* const EndStr;
    const char* Delimiters;
    ASString   Token;

    bool IsDelimiter(int ch) const
    {
        return strchr(Delimiters, ch) != 0;
    }

    const char* SkipDelimiters()
    {
        while (Str < EndStr) {
            if (!IsDelimiter(*Str))
                break;
            Str++;
        }
        return Str;
    }

public:
    StringTokenizer(ASStringContext *psc, const char* str, const char* delim) :
        Str(str), EndStr(str + strlen(str)), Delimiters(delim), Token(psc->GetBuiltin(ASBuiltin_empty_))
    {
    }
    StringTokenizer(ASStringContext *psc, const char* str, UPInt strSize, const char* delim) :
        Str(str), EndStr(str + strSize), Delimiters(delim), Token(psc->GetBuiltin(ASBuiltin_empty_))
    {
    }
    const char* SetDelimiters(const char* delim)
    {
        Delimiters = delim;
        return delim;
    }
    bool NextToken(char* sep)
    {
        if (Str >= EndStr) return false;
        const char* pToken;

        pToken = Str;// = SkipDelimiters();

        while (Str < EndStr) {
            if (IsDelimiter(*Str))
                break;
            Str++;
        }
        *sep = *Str;
        
        if (pToken != Str && Str <= EndStr) 
        {
            UPInt lastTokenSize = (UPInt)(Str - pToken);
            Token = Token.GetManager()->CreateString(pToken, lastTokenSize);
        }
        else
        {
            Token = Token.GetManager()->CreateEmptyString();
        }

        ++Str; // skip delimiter
        return true;
    }

    const ASString& GetToken() const { return Token; }

    StringTokenizer& operator=(const StringTokenizer&) { return *this; }
};

static bool GAS_IsRelativePathToken(ASStringContext* psc, const ASString& pathComponent)
{
    // return (pathComponent == ".." || pathComponent == "_parent");
    return (psc->GetBuiltin(ASBuiltin_dotdot_) == pathComponent ||
            psc->GetBuiltin(ASBuiltin__parent).CompareBuiltIn_CaseCheck(pathComponent, psc->IsCaseSensitive()));
}

// Find the sprite/Movie referenced by the given path.
bool Environment::FindVariable(const GetVarParams& params, bool onlyTargets, ASString* varName) const
{
    if (params.VarName.IsEmpty())
    {
        if (params.pResult)
            params.pResult->SetAsCharacter(Target);
        return true;
    }

    Value                    current;  
    bool                        currentFound        = false;
    const char*                 p                   = params.VarName.ToCStr();
    UPInt                       plength             = params.VarName.GetSize();
    static const char*          onlySlashesDelim    = ":/";
    static const char* const    regularDelim        = ":./";
    const char*                 delim               = regularDelim;

    if (params.pOwner) // reset owner if necessary
        params.pOwner->SetUndefined();
    if (params.ppNewTarget) // reset last target
        *params.ppNewTarget = 0;

    SF_ASSERT(Target);
    
    if (*p == '/') // if the first symbol is '/' - root level is the beginning
    {
        // Absolute path; start at the _root.
        current.SetAsCharacter(Target->GetTopParent());
        currentFound = true;
        p++;
        plength--;
        delim = onlySlashesDelim;
        if (params.pOwner)
            *params.pOwner = current;
    }
    else if (*p == '.') // if the first symbol is '.' use only slashes delim
    {
        delim = onlySlashesDelim;
    }

    StringTokenizer parser(GetSC(), p, plength, delim);
    char            sep = 0;

    bool first_token = true;
    while(parser.NextToken(&sep))
    {
        const ASString& token = parser.GetToken();
        
        if (token.GetSize())
        {
            if (varName)
            {
                *varName = token;
            }
            Value member;
            bool     memberFound = false;
            if (current.IsCharacter() || (!currentFound && GAS_IsRelativePathToken(GetSC(), token)))
            {
                if (!currentFound)
                {
                    // the special case, if relative path is used and current is not set yet:
                    // check withStack first, or use the Target as a current
                    if (params.pWithStack && params.pWithStack->GetSize() > 0)
                    {
                        ObjectInterface* obj = (*params.pWithStack)[params.pWithStack->GetSize()-1].GetObjectInterface();
                        if (obj->IsASCharacter())
                            current.SetAsCharacter(obj->ToCharacter());
                    }
                    if (current.IsUndefined())
                        current.SetAsCharacter(Target);
                    currentFound = true;
                }
                InteractiveObject* m = current.ToCharacter(this);
                if (m)
                {
                    m = ToAvmCharacter(m)->GetRelativeTarget(token, first_token);
                    if (m)
                    {
                        member.SetAsCharacter(m);
                        memberFound = true;
                    }
                }
            }   
            if (!memberFound)
            {
                if (!currentFound)
                {
                    // if no current set yet - check local vars in environment
                    memberFound = GetVariableRaw(GetVarParams(token, &member, params.pWithStack));
                }
                else
                {
                    if (current.IsNumber() || current.IsBoolean() || current.IsString())
                    {
                        // create a temp object to be able to resolve builtin properties like "length".
                        // TODO: revise const_cast
                        current = const_cast<Environment*>(this)->PrimitiveToTempObject(current);
                    }
                    if (!current.IsObject() && !current.IsCharacter() && !current.IsFunction())
                    {
                        member.SetUndefined();
                        memberFound = false;
                    }
                    else
                    {
                        ObjectInterface* iobj = current.ToObjectInterface(this);
                        if (iobj)
                        {
                            // TODO: revise const_cast
                            memberFound = iobj->GetMember(const_cast<Environment*>(this), token, &member);
                            if (!memberFound)
                            {
                                member.SetUndefined();
                                memberFound = false;
                            }
                        }
                    }
                }
            }
            if (params.pOwner) // save owner if necessary
                *params.pOwner = current;
            if ((onlyTargets && !member.IsCharacter()) || !memberFound)
            {
                current.SetUndefined();
                currentFound = false;
                if (parser.NextToken(&sep))
                {   // if this is not the last token - set owner and target to null
                    if (params.pOwner) params.pOwner->SetUndefined();
                    if (params.ppNewTarget) *params.ppNewTarget = 0;
                    if (varName) *varName = GetBuiltin(ASBuiltin_empty_);
                }
                break; // abort the loop
            }

            if (member.IsProperty())
            {
                // resolve the property to its value
                // TODO: revise const_cast
                member.GetPropertyValue(const_cast<Environment *>(this),
                    current.ToObjectInterface(this), &current);
            }
            else
            {
                current = member;
            }

            currentFound = memberFound;
        }
        if (delim == onlySlashesDelim)
        {
            if (sep == ':')
            {
                delim = parser.SetDelimiters(regularDelim);
                if (params.ppNewTarget && current.IsCharacter())
                {
                    *params.ppNewTarget = current.ToCharacter(this);
                }
            }
        }
        else
        {
            if (sep == '.')
            {
                if (params.ppNewTarget && current.IsCharacter())
                {
                    *params.ppNewTarget = current.ToCharacter(this);
                }
            }
        }

        if (sep == '/')
        {
            delim = parser.SetDelimiters(onlySlashesDelim);
        }
        first_token = false;
    }
    if (params.ppNewTarget && current.IsCharacter())
    {
        *params.ppNewTarget = current.ToCharacter(this);
    }
    if (params.pOwner && (!params.pOwner->IsObject() && 
        !params.pOwner->IsCharacter() && !params.pOwner->IsFunction()))
    {
        // owner can only be an object/character/function object
        params.pOwner->SetUndefined();
    }
    if (!currentFound)
        return false;
    if (params.pResult)
        *params.pResult = current;
    return true;
}

// Find the sprite/Movie referenced by the given path.
// excludeFlags - 0 or NoLogOutput
//@TODO: should it be redesigned to work with FindVariable? (AB)
InteractiveObject* Environment::FindTarget(const ASString& path, unsigned excludeFlags) const
{
    if (path.IsEmpty())    
        return IsInvalidTarget ? NULL : Target;
    SF_ASSERT(path.GetSize() > 0);

    InteractiveObject* env = Target;
    SF_ASSERT(env);
    
    const char* p = path.ToCStr();
    ASString   subpart(GetBuiltin(ASBuiltin_empty_));

    if (*p == '/')
    {
        // Absolute path; start at the _root.
        env = env->GetTopParent();
        p++;
    }
    bool first_call = true;
    for (;;)
    {
        const char* pnextSlash = GAS_NextSlashOrDot(p);    
        if (pnextSlash == p)
        {
            if (!(excludeFlags & NoLogOutput))
                LogError("Invalid path '%s'", path.ToCStr());
            break;
        }
        else if (pnextSlash)
        {
            // Cut off the slash and everything after it.
            subpart = CreateString(p, (UPInt)(pnextSlash - p));
        }
        else
        {
            subpart = CreateString(p);
        }

        if (subpart.GetSize())
        {
            // Handle: '.', '..', this, _root, _parent, _levelN, display list characters
            env = ToAvmCharacter(env)->GetRelativeTarget(subpart, first_call);
        }       

        if (env == NULL || pnextSlash == NULL)
        {
            break;
        }

        p = pnextSlash + 1;
        first_call = false;
    }
    return env;
}

Object* Environment::OperatorNew(const FunctionRef& constructor, int nargs, int argsTopOff)
{
    Value    newObjVal;

    SF_ASSERT (!constructor.IsNull ());

    if (argsTopOff < 0) argsTopOff = GetTopIndex();

    Ptr<Object> pnewObj;                    

    //!AB, special undocumented(?) case: if Object's ctor is invoked
    // with a parameter of type:
    // 1. number - it will create instance of Number;
    // 2. boolean - it will create instance of Boolean;
    // 3. array - it will create instance of Array;
    // 4. string - it will create instance of String;
    // 5. object - it will just return the same object.
    // "null" and "undefined" are ignored.
    if (nargs == 1 && constructor == GetConstructor(ASBuiltin_Object))
    {
        const Value& arg0 = Top();
        Value res;
        if (arg0.IsNumber() || arg0.IsBoolean() || arg0.IsString())
        {
            res = PrimitiveToTempObject(0);
        }
        else if (arg0.IsObject() || arg0.IsCharacter())
        {
            res = arg0;
        }
        if (!res.IsUndefined())
        {
            Object* pnewObjUnDefined = res.ToObject(this);
            if (pnewObjUnDefined)
                pnewObjUnDefined->AddRef();
            return pnewObjUnDefined;
        }
    }

    // get the prototype
    Value prototypeVal;
    if (!constructor->GetMemberRaw(GetSC(), GetBuiltin(ASBuiltin_prototype), &prototypeVal))
    {
        prototypeVal.SetAsObject(GetPrototype(ASBuiltin_Object));
    }

    Object* prototype = prototypeVal.ToObject(this);
    FunctionRef ctor(constructor);
    Value __ctor__Val;

    if (prototype && prototype->GetMemberRaw(GetSC(), GetBuiltin(ASBuiltin_constructor), &__ctor__Val))
    {
        if (__ctor__Val.IsFunction() && !__ctor__Val.ToFunction(this).IsNull())
            ctor = __ctor__Val.ToFunction(this);
    }

    pnewObj = *ctor->CreateNewObject(this);
    if (pnewObj)
    {
        // object is allocated by CreateNewObject - set __proto__ and __ctor__

        pnewObj->Set__proto__(GetSC(), prototypeVal.ToObject(this));
        pnewObj->Set__constructor__(GetSC(), constructor);
    }
    else
    {
        ASString thisClassName = GetGC()->FindClassName(this, constructor.GetObjectPtr());
        ASString baseClassName = GetGC()->FindClassName(this, ctor.GetObjectPtr());
        LogScriptError("%s::CreateNewObject returned NULL during creation of %s class instance.", 
            baseClassName.ToCStr(), thisClassName.ToCStr());
        return NULL;
    }

    Value result;
    //GAS_Invoke(constructor, &result, pnewObj.GetPtr(), this, nargs, argsTopOff, NULL);
    constructor.Function->Invoke(FnCall(&result, pnewObj, this, nargs, argsTopOff), constructor.pLocalFrame, NULL);

    if (!pnewObj)
    {
        pnewObj = result.ToObject(this);
        if (pnewObj)
        {
            FunctionRef ctorRef = pnewObj->Get_constructor(GetSC());
            if (ctorRef.IsNull() || ctorRef == constructor) // this is necessary for Object ctor that can return object of different type
                                                      // depending on parameter (see ObjectProto::GlobalCtor) (!AB)
            {
                Ptr<Object> prototypeRef = prototypeVal.ToObject(this);

                // if constructor returned an object - set __proto__ and __ctor__ for it
                pnewObj->Set__proto__(GetSC(), prototypeRef);
                pnewObj->Set__constructor__(GetSC(), constructor);
            }
        }
    }

    if (pnewObj)
        pnewObj->AddRef();
    return pnewObj;
}

Object* Environment::OperatorNew(Object* ppackageObj, const ASString &className, int nargs, int argsTopOff)
{
    SF_ASSERT(className != "");

    Value   ctor;

    if (ppackageObj->GetMember(this, className, &ctor))
    {
        if (ctor.IsFunction())
        {
            return OperatorNew(ctor.ToFunction(this), nargs, argsTopOff);
        }
    }
    return 0;
}

FunctionRef Environment::GetConstructor(ASBuiltinType className)
{    
    Value ctor;
    if (GetGC()->pGlobal->GetMemberRaw(GetSC(), GetBuiltin(className), &ctor) &&
        ctor.IsFunction())
    {
        return ctor.ToFunction(this);
    }
    return 0;
}

// Creates an appropriate temporary object for primitive type at env->Top(index)
// and return it as Value.
Value Environment::PrimitiveToTempObject(int index)
{    
    ASBuiltinType  ctorName;    

    const Value& top = Top(index);
    switch (top.GetType())
    {
    case Value::BOOLEAN:
        ctorName = ASBuiltin_Boolean;
        break;
    case Value::STRING:
        ctorName = ASBuiltin_String;
        break;
    default:
        if (top.IsNumber())
            ctorName = ASBuiltin_Number;        
        else
            return Value(); // undefined
    }
    
    Ptr<Object> obj = *OperatorNew(GetBuiltin(ctorName), 1, GetTopIndex() - index);
    return Value(obj);
}

Value Environment::PrimitiveToTempObject(const Value& v) 
{    
    ASBuiltinType  ctorName;    

    switch (v.GetType())
    {
    case Value::BOOLEAN:
        ctorName = ASBuiltin_Boolean;
        break;
    case Value::STRING:
        ctorName = ASBuiltin_String;
        break;
    default:
        if (v.IsNumber())
            ctorName = ASBuiltin_Number;        
        else
            return Value(); // undefined
    }

    Push(v);
    Ptr<Object> obj = *OperatorNew(GetBuiltin(ctorName), 1, GetTopIndex());
    Drop1();
    return Value(obj);
}

void Environment::Reset()
{
    Stack.Reset();
    unsigned i;
    for (i = 0; i < sizeof(GlobalRegister)/sizeof(GlobalRegister[0]); ++i)
    {
        GlobalRegister[i].SetUndefined();
    }
    LocalRegister.Resize(0);
    IsInvalidTarget = Unrolling = false;

    CallStack.Reset(); // stack of calls
    LocalFrames.Resize(0);
    FuncCallNestingLevel = 0;
    TryBlocks.Resize(0);
    ThrowingValue.SetUndefined();
}

AvmCharacter* Environment::GetAvmTarget() const     
{ 
    SF_ASSERT(Target); 
    return ToAvmCharacter(Target);  
}

// Scan for functions in local frame and convert them to internal ones.
void LocalFrame::ReleaseFramesForLocalFuncs()
{
#ifndef GFX_AS_ENABLE_GC
    for (ASStringHash<Value>::Iterator iter = Variables.Begin(); iter != Variables.End(); ++iter)
    {
        //GFxStringIHash<Value>::Node& var = *iter;
        Value& value = iter->Second;
        if (value.IsFunction ())
        {
            // check, is this function declared locally or not. Only if the function
            // was declared locally, then we need to "weak" it by SetInternal. 
            // Otherwise, it might be just a local pointer to non-local function.
            if (LocallyDeclaredFuncs.Get(value.V.FunctionValue.GetObjectPtr()) != 0)
                value.V.FunctionValue.SetInternal(true);
        }
    }
    SuperThis = 0;
#endif
}

#ifdef GFX_AS_ENABLE_GC
template <class Functor> 
void LocalFrame::ForEachChild_GC(Collector* prcc) const
{
    ASStringHash_GC<Value>::ConstIterator it = Variables.Begin();
    while(it != Variables.End())
    {   
        const Value& value = it->Second;
        value.template ForEachChild_GC<Functor>(prcc);
        ++it;
    }
    if (PrevFrame)
        Functor::Call(prcc, PrevFrame);
    Callee.template ForEachChild_GC<Functor>(prcc);
    Caller.template ForEachChild_GC<Functor>(prcc);
}
SF_GC_PREGEN_FOR_EACH_CHILD(LocalFrame)

void LocalFrame::ExecuteForEachChild_GC(Collector* prcc, OperationGC operation) const
{
    ASRefCountBaseType::CallForEachChild<LocalFrame>(prcc, operation);
}

void LocalFrame::Finalize_GC()
{
    Variables.~ASStringHash_GC<Value>();
}
#endif //SF_NO_GC

//
// Disassembler
//


#if defined(GFX_AS2_VERBOSE) || defined(GFX_VERBOSE_PARSE_ACTION)

#if !defined(GFX_AS2_VERBOSE)
// No disassembler in this version...
void    Disasm::LogDisasm(const unsigned char*)
{
    LogF("<disasm is disabled>\n");
}

#else // !defined(GFX_AS2_VERBOSE)

// Disassemble one instruction to the log.
void    Disasm::LogDisasm(const unsigned char* InstructionData)
{
    enum ArgFormatType {
        ARG_NONE = 0,
        ARG_STR,
        ARG_HEX,    // default hex dump, in case the format is unknown or unsupported
        ARG_U8,
        ARG_U16,
        ARG_S16,
        ARG_PUSH_DATA,
        ARG_DECL_DICT,
        ARG_FUNCTION,
        ARG_FUNCTION2
    };
    class GASInstInfo
    {
    public:
        int actionId;
        const char* Instruction;

        ArgFormatType   ArgFormat;
    };

    static GASInstInfo  InstructionTable[] = {
        { 0x04, "next_frame", ARG_NONE },
        { 0x05, "prev_frame", ARG_NONE },
        { 0x06, "play", ARG_NONE },
        { 0x07, "stop", ARG_NONE },
        { 0x08, "toggle_qlty", ARG_NONE },
        { 0x09, "stop_sounds", ARG_NONE },
        { 0x0A, "add", ARG_NONE },
        { 0x0B, "sub", ARG_NONE },
        { 0x0C, "mul", ARG_NONE },
        { 0x0D, "div", ARG_NONE },
        { 0x0E, "equ", ARG_NONE },
        { 0x0F, "lt", ARG_NONE },
        { 0x10, "and", ARG_NONE },
        { 0x11, "or", ARG_NONE },
        { 0x12, "not", ARG_NONE },
        { 0x13, "str_eq", ARG_NONE },
        { 0x14, "str_len", ARG_NONE },
        { 0x15, "substr", ARG_NONE },
        { 0x17, "pop", ARG_NONE },
        { 0x18, "floor", ARG_NONE },
        { 0x1C, "get_var", ARG_NONE },
        { 0x1D, "set_var", ARG_NONE },
        { 0x20, "set_target2", ARG_NONE },
        { 0x21, "str_cat", ARG_NONE },
        { 0x22, "get_prop", ARG_NONE },
        { 0x23, "set_prop", ARG_NONE },
        { 0x24, "dup_sprite", ARG_NONE },
        { 0x25, "rem_sprite", ARG_NONE },
        { 0x26, "trace", ARG_NONE },
        { 0x27, "start_drag", ARG_NONE },
        { 0x28, "stop_drag", ARG_NONE },
        { 0x29, "str_lt", ARG_NONE },
        { 0x2A, "throw", ARG_NONE },
        { 0x2B, "cast_object", ARG_NONE },
        { 0x2C, "implements", ARG_NONE },
        { 0x30, "random", ARG_NONE },
        { 0x31, "mb_length", ARG_NONE },
        { 0x32, "ord", ARG_NONE },
        { 0x33, "chr", ARG_NONE },
        { 0x34, "get_timer", ARG_NONE },
        { 0x35, "substr_mb", ARG_NONE },
        { 0x36, "ord_mb", ARG_NONE },
        { 0x37, "chr_mb", ARG_NONE },
        { 0x3A, "delete", ARG_NONE },
        { 0x3B, "delete2", ARG_STR },
        { 0x3C, "set_local", ARG_NONE },
        { 0x3D, "call_func", ARG_NONE },
        { 0x3E, "return", ARG_NONE },
        { 0x3F, "mod", ARG_NONE },
        { 0x40, "new", ARG_NONE },
        { 0x41, "decl_local", ARG_NONE },
        { 0x42, "decl_array", ARG_NONE },
        { 0x43, "decl_obj", ARG_NONE },
        { 0x44, "type_of", ARG_NONE },
        { 0x45, "get_target", ARG_NONE },
        { 0x46, "enumerate", ARG_NONE },
        { 0x47, "add_t", ARG_NONE },
        { 0x48, "lt_t", ARG_NONE },
        { 0x49, "eq_t", ARG_NONE },
        { 0x4A, "number", ARG_NONE },
        { 0x4B, "string", ARG_NONE },
        { 0x4C, "dup", ARG_NONE },
        { 0x4D, "swap", ARG_NONE },
        { 0x4E, "get_member", ARG_NONE },
        { 0x4F, "set_member", ARG_NONE },
        { 0x50, "inc", ARG_NONE },
        { 0x51, "dec", ARG_NONE },
        { 0x52, "call_method", ARG_NONE },
        { 0x53, "new_method", ARG_NONE },
        { 0x54, "is_inst_of", ARG_NONE },
        { 0x55, "enum_object", ARG_NONE },
        { 0x60, "bit_and", ARG_NONE },
        { 0x61, "bit_or", ARG_NONE },
        { 0x62, "bit_xor", ARG_NONE },
        { 0x63, "shl", ARG_NONE },
        { 0x64, "asr", ARG_NONE },
        { 0x65, "lsr", ARG_NONE },
        { 0x66, "eq_strict", ARG_NONE },
        { 0x67, "gt_t", ARG_NONE },
        { 0x68, "gt_str", ARG_NONE },
        { 0x69, "extends", ARG_NONE },
        
        { 0x81, "goto_frame", ARG_U16 },
        { 0x83, "get_url", ARG_STR },
        { 0x87, "store_register", ARG_U8 },
        { 0x88, "decl_dict", ARG_DECL_DICT },
        { 0x8A, "wait_for_frame", ARG_HEX },
        { 0x8B, "set_target", ARG_STR },
        { 0x8C, "goto_frame_lbl", ARG_STR },
        { 0x8D, "wait_for_fr_exp", ARG_HEX },
        { 0x8E, "function2", ARG_FUNCTION2 },
        { 0x8F, "try", ARG_HEX },
        { 0x94, "with", ARG_U16 },
        { 0x96, "push_data", ARG_PUSH_DATA },
        { 0x99, "goto", ARG_S16 },
        { 0x9A, "get_url2", ARG_HEX },
        { 0x9B, "function", ARG_FUNCTION },
        { 0x9D, "branch_if_true", ARG_S16 },
        { 0x9E, "call_frame", ARG_HEX },
        { 0x9F, "goto_frame_exp", ARG_HEX },
        { 0x00, "<end>", ARG_NONE }
    };

    int actionId = InstructionData[0];
    GASInstInfo*    info = NULL;

    for (int i = 0; ; i++)
    {
        if (InstructionTable[i].actionId == actionId)
        {
            info = &InstructionTable[i];
        }

        if (InstructionTable[i].actionId == 0)
        {
            // Stop at the end of the table and give up.
            break;
        }
    }

    ArgFormatType   fmt = ARG_HEX;

    // Show instruction.
    if (info == NULL)
    {
        LogF("<unknown>[0x%02X]", actionId);
    }
    else
    {
        LogF("%-15s", info->Instruction);
        fmt = info->ArgFormat;
    }

    // Show instruction Argument(s).
    if (actionId & 0x80)
    {
        SF_ASSERT(fmt != ARG_NONE);

        int length = InstructionData[1] | (InstructionData[2] << 8);

        // LogF(" [%d]", length);

        if (fmt == ARG_HEX)
        {
            for (int i = 0; i < length; i++)
            {
                LogF(" 0x%02X", InstructionData[3 + i]);
            }
            LogF("\n");
        }
        else if (fmt == ARG_STR)
        {
            LogF(" \"");
            for (int i = 0; i < length; i++)
            {
                LogF("%c", InstructionData[3 + i]);
            }
            LogF("\"\n");
        }
        else if (fmt == ARG_U8)
        {
            int val = InstructionData[3];
            LogF(" %d\n", val);
        }
        else if (fmt == ARG_U16)
        {
            int val = InstructionData[3] | (InstructionData[4] << 8);
            LogF(" %d\n", val);
        }
        else if (fmt == ARG_S16)
        {
            int val = InstructionData[3] | (InstructionData[4] << 8);
            if (val & 0x8000) val |= ~0x7FFF;   // sign-extend
            LogF(" %d\n", val);
        }
        else if (fmt == ARG_PUSH_DATA)
        {
            LogF("\n");
            int i = 0;
            while (i < length)
            {
                int type = InstructionData[3 + i];
                i++;
                LogF("\t\t");    // indent
                if (type == 0)
                {
                    // string
                    LogF("\"");
                    while (InstructionData[3 + i])
                    {
                        LogF("%c", InstructionData[3 + i]);
                        i++;
                    }
                    i++;
                    LogF("\"\n");
                }
                else if (type == 1)
                {
                    // float (little-endian)
                    union {
                        float   F;
                        UInt32  I;
                    } u;
                    //compiler_assert(sizeof(u) == sizeof(u.I));

                    memcpy(&u.I, InstructionData + 3 + i, 4);
                    u.I = Alg::ByteUtil::LEToSystem(u.I);
                    i += 4;

                    LogF("(float) %f\n", u.F);
                }
                else if (type == 2)
                {
                    LogF("NULL\n");
                }
                else if (type == 3)
                {
                    LogF("undef\n");
                }
                else if (type == 4)
                {
                    // contents of register
                    int reg = InstructionData[3 + i];
                    i++;
                    LogF("reg[%d]\n", reg);
                }
                else if (type == 5)
                {
                    int boolVal = InstructionData[3 + i];
                    i++;
                    LogF("bool(%d)\n", boolVal);
                }
                else if (type == 6)
                {
                    // double
                    // wacky format: 45670123
                    union {
                        double  D;
                        UInt64  I;
                        struct {
                            UInt32  Lo;
                            UInt32  Hi;
                        } Sub;
                    } u;
                    SF_COMPILER_ASSERT(sizeof(u) == sizeof(u.I));

                    memcpy(&u.Sub.Hi, InstructionData + 3 + i, 4);
                    memcpy(&u.Sub.Lo, InstructionData + 3 + i + 4, 4);
                    u.I = Alg::ByteUtil::LEToSystem(u.I);
                    i += 8;

                    LogF("(double) %f\n", u.D);
                }
                else if (type == 7)
                {
                    // int32
                    SInt32  val = InstructionData[3 + i]
                        | (InstructionData[3 + i + 1] << 8)
                        | (InstructionData[3 + i + 2] << 16)
                        | (InstructionData[3 + i + 3] << 24);
                    i += 4;
                    LogF("(int) %d\n", val);
                }
                else if (type == 8)
                {
                    int id = InstructionData[3 + i];
                    i++;
                    LogF("DictLookup[%d]\n", id);
                }
                else if (type == 9)
                {
                    int id = InstructionData[3 + i] | (InstructionData[3 + i + 1] << 8);
                    i += 2;
                    LogF("DictLookupLg[%d]\n", id);
                }
            }
        }
        else if (fmt == ARG_DECL_DICT)
        {
            int i = 0;
            int count = InstructionData[3 + i] | (InstructionData[3 + i + 1] << 8);
            i += 2;

            LogF(" [%d]\n", count);

            // Print strings.
            for (int ct = 0; ct < count; ct++)
            {
                LogF("\t\t");    // indent

                LogF("\"");
                while (InstructionData[3 + i])
                {
                    // safety check.
                    if (i >= length)
                    {
                        LogF("<disasm error -- length exceeded>\n");
                        break;
                    }

                    LogF("%c", InstructionData[3 + i]);
                    i++;
                }
                LogF("\"\n");
                i++;
            }
        }
        else if (fmt == ARG_FUNCTION2)
        {
            // Signature info for a function2 opcode.
            int i = 0;
            const char* functionName = (const char*) &InstructionData[3 + i];
            i += (int)strlen(functionName) + 1;

            int argCount = InstructionData[3 + i] | (InstructionData[3 + i + 1] << 8);
            i += 2;

            int regCount = InstructionData[3 + i];
            i++;

            LogF("\n\t\tname = '%s', ArgCount = %d, RegCount = %d\n",
                functionName, argCount, regCount);

            UInt16  flags = (InstructionData[3 + i]) | (InstructionData[3 + i + 1] << 8);
            i += 2;

            // @@ What is the difference between "super" and "Parent"?

            bool    preloadGlobal = (flags & 0x100) != 0;
            bool    preloadParent = (flags & 0x80) != 0;
            bool    preloadRoot   = (flags & 0x40) != 0;
            bool    suppressSuper = (flags & 0x20) != 0;
            bool    preloadSuper  = (flags & 0x10) != 0;
            bool    suppressArgs  = (flags & 0x08) != 0;
            bool    preloadArgs   = (flags & 0x04) != 0;
            bool    suppressThis  = (flags & 0x02) != 0;
            bool    preloadThis   = (flags & 0x01) != 0;

            LogF("\t\t        pg = %d\n"
                "\t\t        pp = %d\n"
                "\t\t        pr = %d\n"
                "\t\tss = %d, ps = %d\n"
                "\t\tsa = %d, pa = %d\n"
                "\t\tst = %d, pt = %d\n",
                int(preloadGlobal),
                int(preloadParent),
                int(preloadRoot),
                int(suppressSuper),
                int(preloadSuper),
                int(suppressArgs),
                int(preloadArgs),
                int(suppressThis),
                int(preloadThis));

            for (int argi = 0; argi < argCount; argi++)
            {
                int argRegister = InstructionData[3 + i];
                i++;
                const char* argName = (const char*) &InstructionData[3 + i];
                i += (int)strlen(argName) + 1;

                LogF("\t\targ[%d] - reg[%d] - '%s'\n", argi, argRegister, argName);
            }

            int functionLength = InstructionData[3 + i] | (InstructionData[3 + i + 1] << 8);
            i += 2;

            LogF("\t\tfunction length = %d\n", functionLength);
        }
        else if (fmt == ARG_FUNCTION)
        {
            // Signature info for a function opcode.
            int i = 0;
            const char* functionName = (const char*) &InstructionData[3 + i];
            i += (int)strlen(functionName) + 1;

            int argCount = InstructionData[3 + i] | (InstructionData[3 + i + 1] << 8);
            i += 2;

            LogF("\n\t\tname = '%s', ArgCount = %d\n",
                functionName, argCount);

            for (int argi = 0; argi < argCount; argi++)
            {
                const char* argName = (const char*) &InstructionData[3 + i];
                i += (int)strlen(argName) + 1;

                LogF("\t\targ[%d] - '%s'\n", argi, argName);
            }

            int functionLength = InstructionData[3 + i] | (InstructionData[3 + i + 1] << 8);
            i += 2;

            LogF("\t\tfunction length = %d\n", functionLength);
        }

    }
    else
    {
        LogF("\n");
    }
}
#endif // defined(GFX_AS2_VERBOSE)
#endif // 

void ASStringContext::InvalidateOptAdvanceList()
{
    if (pContext)
    {
        pContext->GetMovieImpl()->InvalidateOptAdvanceList();
    }
}



StringManager* ASStringContext::GetStringManager() const 
{ 
    return pContext->GetStringManager(); 
}

}}} // SF::GFx::AS2
