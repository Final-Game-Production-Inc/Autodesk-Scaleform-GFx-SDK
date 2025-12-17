/**************************************************************************

Filename    :   AS2_Timers.h
Content     :   SWF (Shockwave Flash) player library
Created     :   August, 2006
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_TIMERS_H
#define INC_SF_GFX_TIMERS_H

#include "GFx/GFx_Log.h"
#include "GFx/AS2/AS2_Action.h"
#include "GFx/GFx_InteractiveObject.h"
#include "Kernel/SF_Timer.h"

namespace Scaleform { namespace GFx { namespace AS2 {

class IntervalTimer : public ASIntervalTimerIntf //, public GNewOverrideBase<StatMV_ActionScript_Mem>
{
    FunctionRef                 Function;
    GASWeakPtr<Object>          pObject;
    WeakPtr<InteractiveObject>  Character;
    ASString                    MethodName;
    ValueArray                  Params;
    UInt64                      Interval;    // microseconds
    UInt64                      InvokeTime;  // microseconds
    int                         Id;
    Ptr<CharacterHandle>        LevelHandle; // used to store handle on _levelN to get environment
    bool                        Active;
    bool                        Timeout; // indicates to invoke timer only once

    // all times in mks
    UInt64           GetNextInterval(UInt64 currentTime, UInt64 frameTime) const;
public:
    IntervalTimer(const FunctionRef& function, ASStringContext* psc);
    IntervalTimer(Object* object, const ASString& methodName);
    IntervalTimer(InteractiveObject* character, const ASString& methodName);

    void            Start(MovieImpl* proot);
    // frameTime is in seconds
    bool            Invoke(MovieImpl* proot, float frameTime);
    inline bool     IsActive() const { return Active; }
    void            Clear();
    bool            ClearFor(MovieImpl*, MovieDefImpl*) { return false; }
    // in mks
    inline UInt64   GetNextInvokeTime() const { return InvokeTime; }

    inline void SetId(int id) { Id = id; }
    inline int  GetId() const { return Id; }

    static void SetInterval(const FnCall& fn);
    static void ClearInterval(const FnCall& fn);

    static void SetTimeout(const FnCall& fn);
    static void ClearTimeout(const FnCall& fn);

private:
    static void Set(const FnCall& fn, bool timeout);
};

}}} //SF::GFx::AS2
#endif // INC_SF_GFX_TIMERS_H
