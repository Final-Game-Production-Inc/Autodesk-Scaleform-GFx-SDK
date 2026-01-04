/**********************************************************************
PublicHeader:   GFx
Filename    :   AS2/AS2_Selection.h
Content     :   Implementation of IME class
Created     :   Mar 2008
Authors     :   Ankur

Notes       :   
History     :   

Copyright   :   (c) 1998-2008 Scaleform Corp. All Rights Reserved.

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/


#ifndef INC_SF_GFX_IME_H
#define INC_SF_GFX_IME_H

#ifndef SF_NO_IME_SUPPORT

//#include "GFx/AS2/AS2_Action.h"
#include "GFx/AS2/AS2_Object.h"
#include "GFx/AS2/AS2_ObjectProto.h"
#include "GFx/GFx_CharacterDef.h"
#include "GFx/GFx_PlayerImpl.h"

namespace Scaleform { namespace GFx { namespace AS2 {

class GASIme : public Object
{
protected:
    void commonInit (Environment* penv);

    GASIme(ASStringContext *psc, Object* pprototype) 
        : Object(psc)
    { 
        Set__proto__(psc, pprototype); // this ctor is used for prototype obj only
    }
public:
    GASIme(Environment* penv);
    static void BroadcastOnIMEComposition(Environment* penv, const ASString& pString);
    static void BroadcastOnSwitchLanguage(Environment* penv, const ASString& compString);
    static void BroadcastOnSetSupportedLanguages(Environment* penv, const ASString& supportedLangs);
    static void BroadcastOnSetSupportedIMEs(Environment* penv, const ASString& supportedIMEs);
    static void BroadcastOnSetCurrentInputLang(Environment* penv, const ASString& currentInputLang);
    static void BroadcastOnSetIMEName(Environment* penv, const ASString& imeName);
    static void BroadcastOnSetConversionMode(Environment* penv, const ASString& imeName);
    static void BroadcastOnRemoveStatusWindow(Environment* penv);
    static void BroadcastOnDisplayStatusWindow(Environment* penv);
};

class GASImeProto : public Prototype<GASIme>
{
public:
    GASImeProto (ASStringContext* psc, Object* pprototype, const FunctionRef& constructor);

    static void GlobalCtor(const FnCall& fn);
};

//
// Selection static class
//
// A constructor function object for Object
class GASImeCtorFunction : public CFunctionObject
{
    static const NameFunction StaticFunctionTable[];
    static const NameNumber GASNumberConstTable[];
    static void DoConversion(const FnCall& fn);
    static void GetConversionMode(const FnCall& fn);
    static void GetEnabled(const FnCall& fn);
    static void SetCompositionString(const FnCall& fn);
	static void GetCompositionString(const FnCall& fn);
    static void SetConversionMode(const FnCall& fn);
    static void SetEnabled(const FnCall& fn);
	static void SendLangBarMessage(const FnCall& fn);

public:
    GASImeCtorFunction (ASStringContext *psc);

    //? virtual Object* CreateNewObject(ASStringContext*) const { return 0; }

    static FunctionRef Register(GlobalContext* pgc);
};
}}}
#endif // SF_NO_IME_SUPPORT
#endif 
