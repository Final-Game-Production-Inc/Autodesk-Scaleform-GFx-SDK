/**************************************************************************

Filename    :   SF_RefCount.cpp
Content     :   Reference counting implementation
Created     :   January 14, 1999
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "SF_RefCount.h"
#include "SF_HeapNew.h"
#include "SF_Atomic.h"
#include "SF_Debug.h"

namespace Scaleform {

#ifdef SF_CC_ARM
void* ReturnArg0(void* p)
{
    return p;
}
#endif

// ***** Reference Count Base implementation

RefCountImplCore::~RefCountImplCore()
{
    // RefCount can be either 1 or 0 here.
    //  0 if Release() was properly called.
    //  1 if the object was declared on stack or as an aggregate.
    SF_ASSERT(RefCount <= 1);
}

#ifdef SF_BUILD_DEBUG
void RefCountImplCore::reportInvalidDelete(void *pmem)
{
#if !defined(SF_OS_WII) && !defined(SF_OS_WIIU)
    SF_DEBUG_ERROR1(1,
        "Invalid delete call on ref-counted object at %p. Please use Release()", pmem);
    SF_ASSERT(0);
#endif
}
#endif

RefCountNTSImplCore::~RefCountNTSImplCore()
{
    // RefCount can be either 1 or 0 here.
    //  0 if Release() was properly called.
    //  1 if the object was declared on stack or as an aggregate.
    SF_ASSERT(RefCount <= 1);
}

#ifdef SF_BUILD_DEBUG
void RefCountNTSImplCore::reportInvalidDelete(void *pmem)
{
#if !defined(SF_OS_WII) && !defined(SF_OS_WIIU)
    SF_DEBUG_ERROR1(1,
        "Invalid delete call on ref-counted object at %p. Please use Release()", pmem);
    SF_ASSERT(0);
#endif
}
#endif


// *** Thread-Safe RefCountImpl

void    RefCountImpl::AddRef()
{
    AtomicOps<int>::ExchangeAdd_NoSync(&RefCount, 1);
}
void    RefCountImpl::Release()
{
    if ((AtomicOps<int>::ExchangeAdd_NoSync(&RefCount, -1) - 1) == 0)
        delete this;
}

// *** Thread-Safe RefCountVImpl w/virtual AddRef/Release

void    RefCountVImpl::AddRef()
{
    AtomicOps<int>::ExchangeAdd_NoSync(&RefCount, 1);
}
void    RefCountVImpl::Release()
{
    if ((AtomicOps<int>::ExchangeAdd_NoSync(&RefCount, -1) - 1) == 0)
        delete this;
}

// *** NON-Thread-Safe RefCountImpl

void    RefCountNTSImpl::Release() const
{
    RefCount--;
    if (RefCount == 0)
        delete this;
}

// *** WeakProxy support RefCountImpl

RefCountWeakSupportImpl::~RefCountWeakSupportImpl()
{
    if (pWeakProxy)
    {
        pWeakProxy->NotifyObjectDied();
        pWeakProxy->Release();
    }
}

// Create/return create proxy, users must release proxy when no longer needed
WeakPtrProxy*  RefCountWeakSupportImpl::CreateWeakProxy() const
{
    if (!pWeakProxy)
        if ((pWeakProxy = SF_HEAP_AUTO_NEW(this)
                WeakPtrProxy(const_cast<RefCountWeakSupportImpl*>(this)))==0)
            return 0;
    pWeakProxy->AddRef();
    return pWeakProxy;
}

} // Scaleform
