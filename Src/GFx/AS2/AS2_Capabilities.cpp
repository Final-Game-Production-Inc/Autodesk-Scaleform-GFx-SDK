/**************************************************************************

Filename    :   AS2_Capabilities.cpp
Content     :   System.capabilities reference class for ActionScript 2.0
Created     :   3/27/2007
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/AS2/AS2_Capabilities.h"
#ifdef GFX_AS2_ENABLE_CAPABILITIES
#include "GFx/GFx_Log.h"
#include "GFx/AS2/AS2_Action.h"
#include "GFx/AS2/AS2_NumberObject.h"
#include "Kernel/SF_UTF8Util.h"
#include "GFx/GFx_PlayerImpl.h"
#include "GFx/AS2/IME/AS2_IMEManager.h"
#include "GFx/GFx_ASUtils.h"

namespace Scaleform { namespace GFx { namespace AS2 {

// ****************************************************************************
// Helper function to retrieve the manufacturer string
//
inline ASString GFxCapabilities_Manufacturer(ASStringContext* psc)
{
#if defined(SF_OS_MAC)
    return ASString(psc->CreateConstString("Scaleform Macintosh"));
#elif defined(SF_OS_WIN32)
    return ASString(psc->CreateConstString("Scaleform Windows"));
#elif defined(SF_OS_LINUX)
    return ASString(psc->CreateConstString("Scaleform Linux"));
#else
    return ASString(psc->CreateConstString("Scaleform Other OS Name"));
#endif
}

// ****************************************************************************
// Helper function to retrieve the OS string
//
inline ASString GFxCapabilities_OS(ASStringContext* psc)
{
#if defined(SF_OS_MAC)
    return ASString(psc->CreateConstString("Mac OS X"));
#elif defined(SF_OS_XBOX360)
    return ASString(psc->CreateConstString("XBox 360"));
#elif defined(SF_OS_WIN32)
    return ASString(psc->CreateConstString("Windows"));
#elif defined(SF_OS_LINUX)
    return ASString(psc->CreateConstString("Linux"));
#elif defined(SF_OS_3DS)
    return ASString(psc->CreateConstString("3DS"));
#elif defined(SF_OS_PS3)
    return ASString(psc->CreateConstString("PS3"));
#elif defined(SF_OS_WII)
    return ASString(psc->CreateConstString("WII"));
#elif defined(SF_OS_PSVITA)
    return ASString(psc->CreateConstString("PSVITA"));
#else
    return ASString(psc->CreateConstString("Other"));
#endif
}

// ****************************************************************************
// Helper function to retrieve the version string
//
inline ASString GFxCapabilities_Version(ASStringContext* psc)
{
#if defined(SF_OS_WIN32)
    return ASString(psc->CreateConstString("WIN 8,0,0,0"));
#elif defined(SF_OS_MAC)
    return ASString(psc->CreateConstString("MAC 8,0,0,0"));
#elif defined(SF_OS_LINUX)
    return ASString(psc->CreateConstString("LINUX 8,0,0,0"));
#elif defined(SF_OS_XBOX360)
    return ASString(psc->CreateConstString("XBOX360 8,0,0,0"));
#elif defined(SF_OS_3DS)
    return ASString(psc->CreateConstString("3DS 8,0,0,0"));
#elif defined(SF_OS_PS3)
    return ASString(psc->CreateConstString("PS3 8,0,0,0"));
#else
    return ASString(psc->CreateConstString("GFX 8,0,0,0"));
#endif
}

// ****************************************************************************
// Helper function to retrieve the server string
//
ASString GFxCapabilities_ServerString(Environment *penv)
{
    ASStringContext* psc = penv->GetSC();
    StringBuffer temp;
    // Legend:
#ifdef GFX_ENABLE_SOUND
    Sound::SoundRenderer* psoundrenderer = penv->GetMovieImpl()->GetSoundRenderer();
    UInt32 cap_bits = 0;
    if (psoundrenderer)
        psoundrenderer->GetRenderCaps(&cap_bits);
    if (psoundrenderer)
        temp += "A=t";   // hasAudio
    else
        temp += "A=t";   // hasAudio

    if (psoundrenderer && !(cap_bits & Sound::SoundRenderer::Cap_NoMP3))
        temp += "&MP3=t";  // hasMP3  
    else
        temp += "&MP3=f";  // hasMP3  

    if (psoundrenderer && !(cap_bits & Sound::SoundRenderer::Cap_NoStreaming))
        temp += "&SA=t";   // hasStreamingAudio
    else
        temp += "&SA=f";   // hasStreamingAudio
#else
    // A=t                      hasAudio
    temp += "A=f";   

    // SA=t                     hasStreamingAudio
    temp += "&SA=f";

    // MP3=t                    hasMP3  
    temp += "&MP3=f";
#endif
    // SV=t                     hasStreamingVideo
    temp += "&SV=f";

    // EV=t                     hasEmbeddedVideo
    temp += "&EV=f";
    
    // hasIME
    temp += "&IME=";

    IMEManagerBase* pimeManager = NULL;
#ifndef SF_NO_IME_SUPPORT
    pimeManager = penv->GetMovieImpl()->GetIMEManager();
#endif
    if (pimeManager != NULL)
        temp += "t";
    else
        temp += "f";

    // AE=t                     hasAudioEncoder
    temp += "&AE=f";

    // VE=t                     hasVideoEncoder
    temp += "&VE=f";

    // ACC=f                    hasAccessibility 
    temp += "&ACC=f";

    // PR=t                     hasPrinting
    temp += "&PR=f";

    // SP=t                     hasScreenPlayback
    temp += "&SP=f";

    // SB=f                     hasScreenBroadcast
    temp += "&SB=f";

    // DEB=t                    isDebugger
    temp += "&DEB=f";

    // V=WIN%208%2C0%2C0%2C0    version
    temp += "&V=";
    String etemp;
    String vtemp(GFxCapabilities_Version(psc).ToCStr());
    ASUtils::Escape(vtemp.ToCStr(), vtemp.GetSize(), &etemp);
    temp += etemp;//.ToCStr();

    // M=Macromedia%20Windows   manufacturer
    temp += "&M=";
    etemp.Clear();
    String mtemp(GFxCapabilities_Manufacturer(psc).ToCStr());
    ASUtils::Escape(mtemp.ToCStr(), mtemp.GetSize(), &etemp);
    temp += etemp;//.ToCStr();

    // R=1600x1200              screenResolutionX x screenResolutionY
    Viewport vp;
    penv->GetMovieImpl()->GetViewport(&vp);
    temp += "&R=";
    temp += Value(vp.BufferWidth).ToString(penv).ToCStr();
    temp += "x";
    temp += Value(vp.BufferHeight).ToString(penv).ToCStr();

    // DP=72                    screenDPI
    temp += "&DP=72";

    // COL=color                screenColor
    temp += "&COL=color";

    // AR=1.0                   pixelAspectRatio
    temp += "&AR=1.0";

    // OS=Windows%20XP          os
    temp += "&OS=";
    etemp.Clear();
    String ostemp(GFxCapabilities_OS(psc).ToCStr());
    ASUtils::Escape(ostemp.ToCStr(), ostemp.GetSize(), &etemp);
    temp += etemp;//.ToCStr();

    // L=en                     language
    temp += "&L=en";

    // PT=External              playerType
    temp += "&PT=External";

    // AVD=f                    avHardwareDisable
    temp += "&AVD=f";

    // LFD=f                    localFileReadDisable
    temp += "&LFD=f";

    // WD=f                     windowlessDisable
    temp += "&WD=f";

    return penv->CreateString(temp.ToCStr(),temp.GetSize());
}

// ****************************************************************************
// GASCapabilities ctor function constructor
//
CapabilitiesCtorFunction::CapabilitiesCtorFunction(ASStringContext *psc)
: CFunctionObject(psc)
{
    PropFlags flags = PropFlags::PropFlag_ReadOnly | PropFlags::PropFlag_DontDelete;
    
    // not supported; setting to false
    Value falseVal(false);
    FunctionObject::SetMemberRaw(psc, psc->CreateConstString("avHardwareDisable"), falseVal, flags);
    FunctionObject::SetMemberRaw(psc, psc->CreateConstString("hasAccessibility"), falseVal, flags);
    FunctionObject::SetMemberRaw(psc, psc->CreateConstString("hasAudio"), falseVal, flags);
    FunctionObject::SetMemberRaw(psc, psc->CreateConstString("hasAudioEncoder"), falseVal, flags);
    FunctionObject::SetMemberRaw(psc, psc->CreateConstString("hasEmbeddedVideo"), falseVal, flags);
    FunctionObject::SetMemberRaw(psc, psc->CreateConstString("hasIME"), falseVal, flags);
    FunctionObject::SetMemberRaw(psc, psc->CreateConstString("hasMP3"), falseVal, flags);
    FunctionObject::SetMemberRaw(psc, psc->CreateConstString("hasPrinting"), falseVal, flags);
    FunctionObject::SetMemberRaw(psc, psc->CreateConstString("hasScreenBroadcast"), falseVal, flags);
    FunctionObject::SetMemberRaw(psc, psc->CreateConstString("hasScreenPlayback"), falseVal, flags);
    FunctionObject::SetMemberRaw(psc, psc->CreateConstString("hasStreamingAudio"), falseVal, flags);
    FunctionObject::SetMemberRaw(psc, psc->CreateConstString("hasStreamingVideo"), falseVal, flags);
    FunctionObject::SetMemberRaw(psc, psc->CreateConstString("hasVideoEncoder"), falseVal, flags); 
    FunctionObject::SetMemberRaw(psc, psc->CreateConstString("isDebugger"), falseVal, flags);
    FunctionObject::SetMemberRaw(psc, psc->CreateConstString("localFileReadDisable"), falseVal, flags);

    // hard-code to 'en'. this is a lazy hack.
    FunctionObject::SetMemberRaw(psc, psc->CreateConstString("language"), psc->CreateConstString("en"), flags);

    // set to 'Scaleform OS_STRING'. this logic is correct.    
    FunctionObject::SetMemberRaw(psc, psc->CreateConstString("manufacturer"), GFxCapabilities_Manufacturer(psc), flags);
    
    // set to the OS. this logic is correct (with lower resolution. ie: not returning Windows XP; just Windows).
    FunctionObject::SetMemberRaw(psc, psc->CreateConstString("os"), GFxCapabilities_OS(psc), flags);

    // hard coded to 1. perhaps use MovieClip.GetViewport to get the correct value..?
    FunctionObject::SetMemberRaw(psc, psc->CreateConstString("pixelAspectRatio"), Value(1), flags);

    // hard coded to 'External'. ???
    FunctionObject::SetMemberRaw(psc, psc->CreateConstString("playerType"), psc->CreateConstString("External"), flags);
    
    // hard coded to 'color'. assumption is that the alternative is {'black and white'|'black & white'|'bw'|'b&w'}
    FunctionObject::SetMemberRaw(psc, psc->CreateConstString("screenColor"), psc->CreateConstString("color"), flags);
    
    // hard coded to '72'. ???
    FunctionObject::SetMemberRaw(psc, psc->CreateConstString("screenDPI"), Value(72), flags);

    // set to platform+version string (ie: 'WIN 8,0,0,0')
    FunctionObject::SetMemberRaw(psc, psc->CreateConstString("version"), GFxCapabilities_Version(psc), flags);
    
    // this is not documented in the api reference.. hard coded to 'false'
    FunctionObject::SetMemberRaw(psc, psc->CreateConstString("windowlessDisable"), falseVal, flags);

    // these are computed on the fly. intercepted in getmemberraw
    FunctionObject::SetMemberRaw(psc, psc->CreateConstString("screenResolutionX"), Value(Value::UNSET), flags);    
    FunctionObject::SetMemberRaw(psc, psc->CreateConstString("screenResolutionY"), Value(Value::UNSET), flags);
    FunctionObject::SetMemberRaw(psc, psc->CreateConstString("serverString"), Value(Value::UNSET), flags);    
}

// ****************************************************************************
// Handle get method for properties computed on the fly
//
bool CapabilitiesCtorFunction::GetMember(Environment *penv, const ASString& name, Value *val)
{
#ifdef GFX_ENABLE_SOUND
    Sound::SoundRenderer* psoundrenderer = penv->GetMovieImpl()->GetSoundRenderer();
    UInt32 cap_bits = 0;
    if (psoundrenderer)
        psoundrenderer->GetRenderCaps(&cap_bits);
#endif
    if (penv->IsCaseSensitive())
    {
        if (name == "screenResolutionX")
        {
            Viewport vp;
            penv->GetMovieImpl()->GetViewport(&vp);
            *val = Value(vp.BufferWidth);
            return true;
        }
        else if (name == "screenResolutionY")
        {
            Viewport vp;
            penv->GetMovieImpl()->GetViewport(&vp);
            *val = Value(vp.BufferHeight);
            return true;
        }
#ifndef SF_NO_IME_SUPPORT
        else if (name == "hasIME")
        { 
            IMEManagerBase* pimeManager = penv->GetMovieImpl()->GetIMEManager();
            *val = Value(pimeManager != NULL);
            return true;
        }
#endif
#ifdef GFX_ENABLE_SOUND
        else if (name == "hasAudio")
        {
            *val = Value(psoundrenderer != NULL);
            return true;
        }
        else if (name == "hasMP3")
        {
            *val = Value((psoundrenderer && !(cap_bits & Sound::SoundRenderer::Cap_NoMP3)));
            return true;
        }
        else if (name == "hasStreamingAudio")
        {
            *val = Value((psoundrenderer && !(cap_bits & Sound::SoundRenderer::Cap_NoStreaming)));
            return true;
        }
#endif
        else if (name == "serverString")
        {
            *val = Value(GFxCapabilities_ServerString(penv));
            return true;
        }
    }
    else
    {
        ASStringContext* psc = penv->GetSC();
        if (psc->CompareConstString_CaseInsensitive(name, "screenResolutionX"))     
        { 
            Viewport vp;
            penv->GetMovieImpl()->GetViewport(&vp);
            *val = Value(vp.BufferWidth);
            return true;
        }
        else if (psc->CompareConstString_CaseInsensitive(name, "screenResolutionY"))
        { 
            Viewport vp;
            penv->GetMovieImpl()->GetViewport(&vp);
            *val = Value(vp.BufferHeight);
            return true;
        }

#ifndef SF_NO_IME_SUPPORT
        else if (psc->CompareConstString_CaseInsensitive(name, "hasIME"))
        { 
            IMEManagerBase* pimeManager = penv->GetMovieImpl()->GetIMEManager();
            *val = Value(pimeManager != NULL);
            return true;
        }
#endif
#ifdef GFX_ENABLE_SOUND
        else if (psc->CompareConstString_CaseInsensitive(name, "hasAudio"))
        {
            *val = Value(psoundrenderer != NULL);
            return true;
        }
        else if (psc->CompareConstString_CaseInsensitive(name, "hasMP3"))
        {
            *val = Value((psoundrenderer && !(cap_bits & Sound::SoundRenderer::Cap_NoMP3)));
            return true;
        }
        else if (psc->CompareConstString_CaseInsensitive(name, "hasStreamingAudio"))
        {
            *val = Value((psoundrenderer && !(cap_bits & Sound::SoundRenderer::Cap_NoStreaming)));
            return true;
        }
#endif

        else if (psc->CompareConstString_CaseInsensitive(name, "serverString")) 
        {
            *val = Value(GFxCapabilities_ServerString(penv));
            return true;
        }
    }
    if (penv->CheckExtensions())
    {
        if (name == "numControllers") 
        {
            MovieImpl* proot = penv->GetMovieImpl();
            *val = Value(int(proot->GetControllerCount()));
            return true;
        }
    }
    return FunctionObject::GetMember(penv, name, val);
}

FunctionRef CapabilitiesCtorFunction::Register(GlobalContext* pgc)
{
    ASStringContext sc(pgc, 8);
    FunctionRef  ctor(*SF_HEAP_NEW(pgc->GetHeap()) CapabilitiesCtorFunction(&sc));
    Ptr<Object> proto = *SF_HEAP_NEW(pgc->GetHeap()) ObjectProto(&sc, pgc->GetPrototype(ASBuiltin_Object), ctor);
    pgc->SetPrototype(ASBuiltin_Capabilities, proto);
    pgc->SystemPackage->SetMemberRaw(&sc, pgc->GetBuiltin(ASBuiltin_Capabilities), Value(ctor));
    return ctor;
}
}}} //SF::GFx::AS2
#endif //#ifdef GFX_AS2_ENABLE_CAPABILITIES
