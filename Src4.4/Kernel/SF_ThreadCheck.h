/**********************************************************************

PublicHeader:   Render
Filename    :   SF_ThreadCheck.h
Content     :   Provides templated containers which perform enforcement of a 
                giving a single-thread access to the contained object instance.
Created     :   2014/01/17
Authors     :   Bart Muzzin

Copyright   :   Copyright 2014 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#pragma once
#ifndef INC_SF_ThreadCheck_h
#define INC_SF_ThreadCheck_h

#include "Kernel/SF_Debug.h"
#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_Threads.h"

namespace Scaleform { 

// Base class for thread-checked object templates. Basic accessors/mutators provided.
class ThreadCheckBase
{
public:
    ThreadCheckBase(Scaleform::ThreadId accessThread = Scaleform::GetCurrentThreadId())
    {
        SetAccessThreadId(accessThread);
    }

    // Used to verify that the given thread is the accessor.
    bool CheckValidThreadAccess(Scaleform::ThreadId tid) const
    {
        return GetAccessThreadId() == tid;
    }

    // Get the TID of the thread that is allowed access.
    Scaleform::ThreadId GetAccessThreadId() const 
    { 
        return AccessThread.Load_Acquire(); 
    }

    // Change the thread that is allowed to access.
    void SetAccessThreadId(Scaleform::ThreadId accessThread)
    {
        AccessThread.Exchange_NoSync(accessThread);
    }

protected:
    void enforceValidThreadAccess() const
    {
        // Check that we are operating on the correct thread.
        SF_DEBUG_ASSERT2(CheckValidThreadAccess(Scaleform::GetCurrentThreadId()),
            "Threading access violation. This object is intended to not be thread safe, yet it is being "
            "accessed from a thread other than the one that should access it (intended TID=" THREAD_FORMAT ", access TID=" THREAD_FORMAT ")",
            GetAccessThreadId(), Scaleform::GetCurrentThreadId());
    }
private:
    AtomicValueBase<Scaleform::ThreadId>   AccessThread;           // The only thread that should have access to the object.
};

// ThreadCheckValue contains the instance of the object to be checked internally. NOTE: requires the object type to 
// have a default constructor. This is the most effective of the containers, because the checked object cannot be accessed
// without performing the thread-check.
template<class ThreadCheckedType>
class ThreadCheckVal : public ThreadCheckBase
{
public:
    ThreadCheckVal(Scaleform::ThreadId accessThread = Scaleform::GetCurrentThreadId()) :
      ThreadCheckBase(accessThread),
          ThreadCheckedObject()
      {
      }

      ThreadCheckedType* operator -> () 
      { 
          enforceValidThreadAccess();
          return &ThreadCheckedObject; 
      }
      const ThreadCheckedType* operator -> () const
      { 
          enforceValidThreadAccess();
          return &ThreadCheckedObject; 
      }

private:
      ThreadCheckedType                          ThreadCheckedObject;    // Instance of an object.
};

// ThreadCheckRef manages a reference to an object for which it checks thread accesses. Although the object internally is a reference,
// it should be accessed via the -> operator to perform thread checking. 
template<class ThreadCheckedType>
class ThreadCheckRef : public ThreadCheckBase
{
public:
    ThreadCheckRef(ThreadCheckedType& threadCheckedObject, Scaleform::ThreadId accessThread = Scaleform::GetCurrentThreadId()) :
      ThreadCheckBase(accessThread),
      ThreadCheckedObject(threadCheckedObject)
    {
    }

    ThreadCheckedType* operator -> ()  const
    { 
        enforceValidThreadAccess();
        return &ThreadCheckedObject; 
    }

private:
    ThreadCheckedType&                          ThreadCheckedObject;    // Reference to the object.
};

// ThreadCheckWeakPtr manages a (regular) pointer to an object for which it checks thread accesses.
template<class ThreadCheckedType>
class ThreadCheckPointer : public ThreadCheckBase
{
public:
    ThreadCheckPointer(ThreadCheckedType* threadCheckedObject = 0, Scaleform::ThreadId accessThread = Scaleform::GetCurrentThreadId()) :
      ThreadCheckBase(accessThread),
          ThreadCheckedObject(threadCheckedObject)
      {
      }

      ThreadCheckedType* operator -> ()  const
      { 
          enforceValidThreadAccess();
          return ThreadCheckedObject; 
      }

      operator ThreadCheckedType*()                             { return ThreadCheckedObject; }
      ThreadCheckPointer& operator=(ThreadCheckedType* o)       { ThreadCheckedObject = o; return *this; }

private:
      ThreadCheckedType*                          ThreadCheckedObject;    // Pointer to the object.
};

// ThreadCheckPtr manages a smart pointer to an object for which it checks thread accesses.
template<class ThreadCheckedType>
class ThreadCheckPtr : public ThreadCheckBase, public Ptr<ThreadCheckedType>
{
public:
    ThreadCheckPtr(ThreadCheckedType* threadCheckedObject = 0, Scaleform::ThreadId accessThread = Scaleform::GetCurrentThreadId()) :
        ThreadCheckBase(accessThread),
        Ptr<ThreadCheckedType>(threadCheckedObject)
    {
    }

    ThreadCheckedType* operator -> ()  const
    { 
        enforceValidThreadAccess();
        return Ptr<ThreadCheckedType>::operator ->();
    }

    ThreadCheckPtr<ThreadCheckedType>& operator=(const Ptr<ThreadCheckedType>& other)
    {
        Ptr<ThreadCheckedType>::operator=(other);
        return *this;
    }
};

}   // Scaleform

#endif
