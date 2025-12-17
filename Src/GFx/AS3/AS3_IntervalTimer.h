/**************************************************************************

Filename    :   AS3_IntervalTimer.h
Content     :   SWF (Shockwave Flash) player library
Created     :   Feb, 2010
Authors     :   Artyom Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFx_AS3_IntervalTimer_H
#define INC_SF_GFx_AS3_IntervalTimer_H

#include "GFx/GFx_Log.h"
#include "GFx/GFx_InteractiveObject.h"
#include "Kernel/SF_Timer.h"
#include "GFx/GFx_PlayerImpl.h"
#include "GFx/AS3/Obj/Utils/AS3_Obj_Utils_Timer.h"
#include "AS3_ObjCollector.h"

namespace Scaleform { namespace GFx { namespace AS3 {

class IntervalTimer : public ASIntervalTimerIntf
{
    Value                   Function;
    SPtr<Instances::fl_utils::Timer>  TimerObj;
    ArrayLH<Value>          Params;
    unsigned                CurrentCount;
    unsigned                RepeatCount;
    UInt32                  Interval; // in microseconds
    UInt64                  InvokeTime; // in microseconds
    int                     Id;
    bool                    Active;
    bool                    Timeout; // indicates to invoke timer only once

    UInt32                  GetNextInterval(UInt64 currentTime, UInt64 frameTime) const;
public:
    // delay is specified in ms
    IntervalTimer(const Value& function, unsigned delay, bool timeOut = false);
    IntervalTimer(Instances::fl_utils::Timer* timerObj, unsigned delay, unsigned curCount, unsigned repeatCount);

    virtual void            Start(MovieImpl* proot);
    // frameTime is in seconds
    virtual bool            Invoke(MovieImpl* proot, float frameTime);
    virtual bool            IsActive() const { return Active; }
    virtual void            Clear();
    virtual bool            ClearFor(MovieImpl* proot, MovieDefImpl*);
    // in mks
    virtual UInt64          GetNextInvokeTime() const { return InvokeTime; }
    virtual void            SetId(int id) { Id = id; }
    virtual int             GetId() const { return Id; }

    void                    SetArguments(unsigned num, const Value* argArr);

    unsigned                GetCurrentCount() const { return CurrentCount; }

#ifdef SF_OBJ_COLLECTOR_ENABLED
    virtual void            Collect(ObjectCollector& c);
#endif
};

}}} //SF::GFx::AS3

#endif // INC_SF_GFx_AS3_IntervalTimer_H
