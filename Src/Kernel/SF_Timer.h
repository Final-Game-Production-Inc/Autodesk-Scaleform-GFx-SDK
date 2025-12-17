/**************************************************************************

PublicHeader:   None
Filename    :   SF_Timer.h
Content     :   Provides static functions for precise timing
Created     :   June 28, 2005
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_Timer_H
#define INC_SF_Kernel_Timer_H

#include "SF_Types.h"

namespace Scaleform {
    
//------------------------------------------------------------------------
// ***** Timer

// Timer class defines a family of static functions used for application
// timing and profiling.

class Timer
{
public:
    enum {
        MsPerSecond     = 1000, // Milliseconds in one second.
        MksPerMs        = 1000, // Microseconds in one millisecond.
        MksPerSecond    = MsPerSecond * MksPerMs
    };


    // ***** Timing APIs for Application    
    // These APIs should be used to guide animation and other program functions
    // that require precision.

    // Returns ticks in milliseconds, as a 32-bit number. May wrap around every
    // 49.2 days. Use either time difference of two values of GetTicks to avoid
    // wrap-around.  GetTicksMs may perform better then GetTicks.
    static UInt32  SF_STDCALL GetTicksMs();

    // GetTicks returns general-purpose high resolution application timer value,
    // measured in microseconds (mks, or 1/1000000 of a second). The actual precision
    // is system-specific and may be much lower, such as 1 ms.
    static UInt64  SF_STDCALL GetTicks();

    
    // ***** Profiling APIs.
    // These functions should be used for profiling, but may have system specific
    // artifacts that make them less appropriate for general system use.
    // On Win32, for example these rely on QueryPerformanceConter  may have
    // problems with thread-core switching and power modes.

    // Return a hi-res timer value in mks (1/1000000 of a sec).
    // Generally you want to call this at the start and end of an
    // operation, and pass the difference to
    // TicksToSeconds() to find out how long the operation took. 
    static UInt64  SF_STDCALL GetProfileTicks();

    // More convenient zero-based profile timer in seconds. First call initializes 
    // the "zero" value; future calls return the difference. Not thread safe for first call.
    // Due to low precision of Double, may malfunction after long runtime.
    static Double  SF_STDCALL GetProfileSeconds();

    // Get the raw cycle counter value, providing the maximum possible timer resolution.
    static UInt64  SF_STDCALL GetRawTicks();
    static UInt64  SF_STDCALL GetRawFrequency();

    
    // ***** Tick and time unit conversion.

    // Convert micro-second ticks value into seconds value.
    static inline Double TicksToSeconds(UInt64 ticks)
    {
        return static_cast<Double>(ticks) * ((Double)1.0 / (Double)MksPerSecond);
    }
    // Convert Raw or frequency-unit ticks to seconds based on specified frequency.
    static inline Double RawTicksToSeconds(UInt64 rawTicks, UInt64 rawFrequency)
    {
        return static_cast<Double>(rawTicks) * rawFrequency;
    }

    // ***** Timer implementation overriding.
    // Setting a non-null override instance will cause all Timer static functions to
    // use the overridden implementation, instead of the default. This can be used 
    // (for example) to create deterministic timers, instead of wall-clock timers.
    class TimerOverride
    {
    public:
        virtual ~TimerOverride() {}

        virtual UInt32 GetTicksMs(UInt32 measured)      = 0;
        virtual UInt64 GetRawTicks(UInt64 measured)     = 0;
        virtual UInt64 GetRawFrequency(UInt64 measured) = 0;
    };
    static void    SF_STDCALL SetTimerOverride(TimerOverride* instance);
    static TimerOverride*     GetTimerOverride();

private:
    friend class System;
    // System called during program startup/shutdown.
    static void initializeTimerSystem();
    static void shutdownTimerSystem();
};


} // Scaleform::Timer

#endif
