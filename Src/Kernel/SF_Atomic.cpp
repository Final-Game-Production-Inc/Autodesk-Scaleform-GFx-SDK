/**************************************************************************

Filename    :   SF_Atomic.cpp
Content     :   Contains atomic operations and inline fastest locking
                functionality. Will contain #ifdefs for OS efficiency.
                Have non-thread-safe implementaion if not available.
Created     :   May 5, 2003
Authors     :   Michael Antonov, Andrew Reisse

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "SF_Atomic.h"

#ifdef SF_ENABLE_THREADS

namespace Scaleform {

// ***** Windows Lock implementation

#if defined(SF_OS_WIN32) || defined(SF_OS_WINCE) || defined(SF_OS_XBOX) || defined(SF_OS_XBOX360)

#if !defined(SF_FAST_LOCK)

// ***** Standard Win32 Lock implementation

#if defined(SF_OS_WIN32) && !defined(SF_OS_WINMETRO)
// Spin count init critical section function prototype for Window NT
typedef BOOL (WINAPI *Function_InitializeCriticalSectionAndSpinCount) 
             (LPCRITICAL_SECTION lpCriticalSection, DWORD dwSpinCount);
#endif

// Constructors
Lock::Lock(unsigned spinCount)
{
#if defined(SF_OS_WINMETRO)
	::InitializeCriticalSectionEx(&cs,0,0);

#elif defined(SF_OS_WIN32)
    // Try to load function dynamically so that we don't require NT
    // On Windows NT we will use InitializeCriticalSectionAndSpinCount
    static  bool initTried = 0;
    static  Function_InitializeCriticalSectionAndSpinCount pInitFn = 0;

    if (!initTried)
    {
        HMODULE hmodule = ::LoadLibrary(SF_STR("kernel32.dll"));
        pInitFn     = (Function_InitializeCriticalSectionAndSpinCount)
            ::GetProcAddress(hmodule, "InitializeCriticalSectionAndSpinCount");
        initTried   = 1;
    }

    // Initialize the critical section
    if (pInitFn)
        pInitFn(&cs, spinCount);
    else
        ::InitializeCriticalSection(&cs);

#elif defined(SF_OS_XBOX360)    
    ::InitializeCriticalSectionAndSpinCount(&cs, spinCount);
#else 
    ::InitializeCriticalSection(&cs);
#endif
}

Lock::~Lock()
{
    DeleteCriticalSection(&cs);
}


#else

// ***** Fast Win32 + X86 Lock implementation

Lock::Lock(unsigned spinCount)
{    
    RecursiveLockCount  = 0;
    LockedThreadId.Value= 0;
    WaiterCount.Value   = 0;
    hSemaphore          = 0;
    SetSpinMax(spinCount);
}

Lock::~Lock()
{
    if (hSemaphore)
        CloseHandle(hSemaphore);
}


// *** Locking functions.

void Lock::DoLock()
{
    DWORD threadId = GetCurrentThreadId();

    if (threadId != LockedThreadId.Value)
    {
        if ((LockedThreadId.Value == 0) && PerfLockImmediate(threadId))
        {
            // Successful single instruction quick-lock
        }
        else
        {
            PerfLock(threadId); 
        }
    }
    RecursiveLockCount++;
}

void Lock::Unlock()
{
    SF_ASSERT(RecursiveLockCount > 0);
    --RecursiveLockCount;
    if (RecursiveLockCount == 0)
        PerfUnlock();
}

#include <intrin.h>

void Lock::PerfUnlock()
{   
    //_WriteBarrier();    

    // We need to use release semantics here, however calling 'Store_Release'
    // on x86 is slow here because it uses 'xchg'. It is performance critical
    // to perform an unlock through a regular assignment, even though if this
    // was non-x86 we would've used LockedThreadId.Store_Release(0).
    // TBD: Details of Release effect on X86 need to be researched, to see
    // where xchg is actually necessary.            
    LockedThreadId.Value = 0;    

    //_ReadWriteBarrier();

    // AFTER it is released we check if there're waiters.
    if (WaiterCount.Load_Acquire() > 0)
    {        
        WaiterMinus();

        SF_ASSERT(hSemaphore);
        SF_ASSERT(ReleaseSemaphore(hSemaphore, 1, NULL) != 0);
    }
}

void    Lock::PerfLock(DWORD threadId)
{
    // Attempt spin-lock
    for (unsigned spin = 0; spin < SpinMax; spin++)
    {
        if (PerfLockImmediate(threadId))
            return;

#ifdef SF_CPU_X86
        SF_ASM { pause };
#endif        
        //YieldProcessor();
    }

    // Ensure we have the kernel event created
    AllocateKernelSemaphore();

    // Do a kernel lock with potential semaphore wait.
    while (1)
    {        
        WaiterPlus();
        if (PerfLockImmediate(threadId))
            break;
        SF_ASSERT(WaitForSingleObject(hSemaphore, INFINITE) == WAIT_OBJECT_0);
    }

    WaiterMinus();    
}


// Utility functions to access system Info & Init semaphore.

static unsigned SpinMax_ProcessorCount = 0xFFFFFFFF;

void Lock::SetSpinMax(unsigned spinMax)
{
    if (SpinMax_ProcessorCount == 0xFFFFFFFF)
    {
        SYSTEM_INFO stSI;
        GetSystemInfo(&stSI);
        SpinMax_ProcessorCount = (unsigned) stSI.dwNumberOfProcessors;
    }
    
    SpinMax = (SpinMax_ProcessorCount > 1) ? spinMax : 0;
}

void    Lock::AllocateKernelSemaphore()
{
    if (!hSemaphore)
    {
        HANDLE hnewSemaphore = CreateSemaphore(NULL, 0, 0x7FFFFFFF, NULL);
        SF_ASSERT(hnewSemaphore != 0);
        if (InterlockedCompareExchangePointer(&hSemaphore, hnewSemaphore, NULL))
            CloseHandle(hnewSemaphore); // we're late
    }
}

#endif

#endif

} // Scaleform

#endif // SF_ENABLE_THREADS
