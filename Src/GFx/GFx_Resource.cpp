/**************************************************************************

Filename    :   GFx_Resource.cpp
Content     :   Resource and Resource Library implementation for GFx
Created     :   January 11, 2007
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include <stdio.h>
#include "GFx/GFx_Resource.h"
#include "Kernel/SF_Std.h"
#include "Kernel/SF_Debug.h"

#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_MsgFormat.h"

namespace Scaleform { namespace GFx {

// ***** Resource

// Thread-Safe ref-count implementation.
void    Resource::AddRef()
{
    RefCount.Increment_NoSync();
  //  SInt32 val = RefCount.ExchangeAdd_NoSync(1);
  //  if (ResType == 100)
  //       printf("Thr %4d, %8x : Resource::AddRef (%d -> %d)\n", GetCurrentThreadId(), this, val, val+1);  
}
void    Resource::Release()
{
    if ((RefCount.ExchangeAdd_Acquire(-1) - 1) == 0)
    {  
        // Once the resource reaches RefCount of 0 it can no longer be revived
        // by the library (required to allow ops without lock).
        // Ensure removal from library.
        if (pLib)
        {
            pLib->RemoveResourceOnRelease(this);
            pLib = 0;
        }
        delete this;
    }
}

// Increments a reference count if it is not zero; this function is used by
// the library while it is locked.
// Returns 1 if increment was successful, and 0 if RefCount turned to 0.
bool    Resource::AddRef_NotZero()
{
    while (1)
    {
        SInt32 refCount = RefCount;
        if (refCount == 0)
            return 0;
        if (RefCount.CompareAndSet_NoSync(refCount, refCount+1))
            break;
    }
    return 1;
}

SInt32  Resource::GetRefCount() const
{
    return RefCount;
}


// ***** ResourceLib

ResourceLib::ResourceLib(bool debug)
{
    DebugFlag = debug;
    pWeakLib = SF_NEW ResourceWeakLib(this);
}
ResourceLib::~ResourceLib()
{
    if (pWeakLib)
    {
        pWeakLib->UnpinAll();
        pWeakLib->Release();
    }
}


ResourceLib::ResourceSlot::ResourceSlot(ResourceWeakLib* plib, const ResourceKey& key)
{
    SF_ASSERT(plib);
    pLib      = plib;
    State     = Resolve_InProgress;
    pResource = 0;
    Key       = key;
}

ResourceLib::ResourceSlot::~ResourceSlot()
{
    Lock::Locker lock(&pLib->ResourceLock);

    if (State == Resolve_InProgress)
        pLib->Resources.Remove(Key);
    else if (pResource)
        pResource->Release();
}


// *** Interface for Waiter

// If we are not responsible, wait to receive resource.
Resource*    ResourceLib::ResourceSlot::WaitForResolve()
{            
    // User AddRefs to us through BindHandle, while we AddRef to Lib. 
    
#ifdef SF_ENABLE_THREADS

    // Wait until the resource is available, at which point
    // either pResource or ErrorMessage should be set.
    ResolveComplete.Wait();

    // In case of an error, pResource should be 0. It is safe
    // to use pResource pointer here because Resolve AddRefs
    // to it for us.
    if (pResource)
        pResource->AddRef();
    return pResource;

#else
    // With no thread support, we can never wait. Resource library
    // should always be in Resolved or not state.
    SF_ASSERT(0);
    return 0;
#endif
}


const char*     ResourceLib::ResourceSlot::GetError() const
{
    return ErrorMessage.ToCStr();
}

// Check availability.
bool            ResourceLib::ResourceSlot::IsResolved() const
{
    Lock::Locker lock(&pLib->ResourceLock);
    return (State != Resolve_InProgress);
}


// If we are responsible call one of these two:
void    ResourceLib::ResourceSlot::Resolve(Resource* pres)
{
    Lock::Locker lock(&pLib->ResourceLock);

    SF_ASSERT(State == Resolve_InProgress);
    SF_ASSERT(pres);

    // Slot AddRef's to resource to ensure that it doesn't die
    // before BindHandle owning threads get a chance to react to
    // the signaled event (below).
    pres->AddRef();
    pResource = pres;
    State     = Resolve_Success;

    // Obtain the hash node and resolve it.
    ResourceWeakLib::ResourceNode* pnode = pLib->Resources.Get(Key);
    SF_ASSERT(pnode);
    SF_ASSERT(pnode->Type == ResourceWeakLib::ResourceNode::Node_Resolver);

    pnode->pResource = pres;
    pnode->Type      = ResourceWeakLib::ResourceNode::Node_Resource;

    pres->SetOwnerResourceLib(pLib);

#ifdef SF_ENABLE_THREADS    
    ResolveComplete.SetEvent();
#endif

}

void    ResourceLib::ResourceSlot::CancelResolve(const char* perrorMessage)
{
    Lock::Locker lock(&pLib->ResourceLock);

    SF_ASSERT(State == Resolve_InProgress);    

    // Resolve failed with error.
    State        = Resolve_Fail;
    ErrorMessage = perrorMessage;

    // Remove node from hash.
    SF_ASSERT(pLib->Resources.Get(Key) != 0);
    pLib->Resources.Remove(Key);

#ifdef SF_ENABLE_THREADS
    ResolveComplete.SetEvent();
#endif
}



// If we are not responsible, wait to receive resource.
Resource*    ResourceLib::BindHandle::WaitForResolve()
{
    SF_ASSERT(State == RS_WaitingResolve ||
            State == RS_Available ||
            State == RS_Error );

    // If resource already resolved, return appropriate value.
    if (State == RS_Available)
    {
        pResource->AddRef();        
        return pResource;
    }
    else if (State == RS_Error)
    {
        return 0;
    }

#ifdef SF_ENABLE_THREADS

    ResourceSlot*   pslot = pSlot;
    Resource*    pres  = pslot->WaitForResolve();
    if (pres)
    {
        State     = RS_Available;
        pResource = pres;
        pResource->AddRef(); // AddRef one more time for ~BindHandle.
        pslot->Release();        

        // NOTE: pslot->WaitForResolve() AddRefs internally,
        // so our return value is also AddRefed.
        return pres;
    }
    
#else
    // WaitForResolve should never be called in RS_WaitingResolve state
    // with no thread support.
    SF_ASSERT(0);
#endif

    State = RS_Error;
    return 0;
}



// ***** ResourceWeakLib

ResourceWeakLib::ResourceWeakLib(ResourceLib *pstrongLib)
{
    // Back-pointer link  to strong lib
    pStrongLib = pstrongLib;

#ifdef GFX_USE_IMAGE_HEAPS
    // Create resource library's image heap.
    MemoryHeap::HeapDesc desc((pstrongLib && pstrongLib->DebugFlag) ? MemoryHeap::Heap_UserDebug : 0);

    desc.Flags |= MemoryHeap::Heap_FixedGranularity;
    desc.MinAlign    = 64;
    desc.Granularity = 4 * 1024;
    desc.Reserve     = 0;
    desc.Threshold   = ~UPInt(0);
    desc.Limit       = 0;
    desc.HeapId      = HeapId_Images;

    pImageHeap = *Memory::GetGlobalHeap()->CreateHeap("_ResourceLib_Images", desc);
#endif
}

ResourceWeakLib::~ResourceWeakLib()
{
    Lock::Locker lock(&ResourceLock);
    
    // Clear pLib in all resources that are still alive.
    HashSet<ResourceNode, ResourceNode::HashOp>::Iterator ires;

    for(ires = Resources.Begin(); ires != Resources.End(); ++ires)
    {
        // This must be a resource. If this is a resource slot,
        // this means that other threads are still trying to use the
        // library, so they must have AddRef'ed to WeakLib.
        SF_ASSERT(ires->IsResource());
        ires->pResource->SetOwnerResourceLib(0);
    }
}


// Lookup resource, but only if resolved.
// All resources are considered AddRefed.
Resource*    ResourceWeakLib::GetResource(const ResourceKey &k)
{
    Lock::Locker lock(&ResourceLock);

    ResourceNode* pnode = Resources.Get(k);
    if (pnode && pnode->IsResource())
    {
        if (pnode->pResource->AddRef_NotZero())
        {
            return pnode->pResource;
        }
    }
    return 0;
}


// Lookup resource and insert its slot. We get back a BindHandle.
//
ResourceLib::ResolveState ResourceWeakLib::BindResourceKey(ResourceLib::BindHandle *phandle, const ResourceKey &k)
{
    SF_ASSERT(phandle->State == ResourceLib::RS_Unbound);

    Lock::Locker lock(&ResourceLock);

    // Need more locking, etc.
    ResourceNode* pnode = Resources.Get(k);

    if (pnode)
    {
        if (pnode->IsResource())
        {
            if (pnode->pResource->AddRef_NotZero())
            {
                // Resource AddRefed, can return it.
                phandle->pResource = pnode->pResource;
                phandle->State     = ResourceLib::RS_Available;                
                return phandle->State;
            }
            else
            {
                // Resource was decremented to 0 during our lock.
                // It can no longer be used and must be removed from list.
                Resources.Remove(k);

                // Fall through to create a new slot.
            }
        }
        else
        {
            phandle->pSlot = pnode->pResolver;
            phandle->State = ResourceLib::RS_WaitingResolve;
            phandle->pSlot->AddRef();
            return phandle->State;
        }
    }

    // Create a resolve slot
    ResourceSlot* presSlot = SF_NEW ResourceSlot(this, k);
    if (!presSlot)
        return ResourceLib::RS_Error;

    // Add a resource node.
    ResourceNode n;
    n.Type      = ResourceNode::Node_Resolver;
    n.pResolver = presSlot;
    Resources.Add(n);

    // Prepare return value.
    phandle->pSlot = presSlot;
    phandle->State = ResourceLib::RS_NeedsResolve;
    return phandle->State;
}

// Queries an AddRefed list of all resources, used for debug/stat display.
void ResourceWeakLib::GetResourceArray(Array<Ptr<Resource> > *presources)
{
    Lock::Locker lock(&ResourceLock);

    HashSet<ResourceNode, ResourceNode::HashOp>::Iterator ires = Resources.Begin();
    while(ires != Resources.End())
    {
        ResourceNode* pnode = &*ires;
        
        // If we can AddRef to resource, add it to list.
        if (pnode->IsResource() &&
            pnode->pResource->AddRef_NotZero())
        {
            Ptr<Resource> presource = *pnode->pResource;
            presources->PushBack(presource);
        }               
        ++ires;
    }
}



// Pin-management: for strong links.

// Returns 1 if resource is pinned in strong library.
bool    ResourceWeakLib::IsPinned(Resource* pres)
{
    SF_ASSERT(pres->pLib == this);
    Lock::Locker lock(&ResourceLock);
    return (pStrongLib && pStrongLib->PinSet.Get(pres) != 0);
}

void    ResourceWeakLib::PinResource(Resource* pres)
{
    SF_ASSERT(pres->pLib == this);
    Lock::Locker lock(&ResourceLock);

    if (!pStrongLib)
    {
        SF_DEBUG_WARNING(1, "Resource::PinResource failed - strong library not available");
    }
    else
    {
        if (!pStrongLib->PinSet.Get(pres))
        {
            pStrongLib->PinSet.Add(pres);
            pres->AddRef();
        }
    }
}

void    ResourceWeakLib::UnpinResource(Resource* pres)
{
    SF_ASSERT(pres->pLib == this);
    Lock::Locker lock(&ResourceLock);

    if (pStrongLib)
    {
        pStrongLib->PinSet.Remove(pres);
        pres->Release();
    }
}


void    ResourceWeakLib::UnpinAll()
{            
    Lock::Locker lock(&ResourceLock);

    if (pStrongLib)
    {
        // Release all references held by pins.
        ResourceLib::PinHashSet::Iterator ihash = pStrongLib->PinSet.Begin();
        while(ihash != pStrongLib->PinSet.End())
        {
            // Hopefully we support recursive locks.. :)
            (*ihash)->Release();
            ++ihash;
        }
        pStrongLib->PinSet.Clear();
    }
}


// Virtual function called when resource RefCount has reached 0
// and the resource is about to die.
void    ResourceWeakLib::RemoveResourceOnRelease(Resource *pres)
{
    Resource *p = static_cast<Resource*>(pres);

    Lock::Locker lock(&ResourceLock);
    SF_ASSERT(pres->GetRefCount() == 0);

    // Resource may have been removed or substituted by a library call.
    ResourceNode* pnode = Resources.Get(p->GetKey());
    if (pnode &&
        pnode->Type == ResourceNode::Node_Resource &&
        pnode->pResource == pres)
    {
        Resources.Remove(p->GetKey());
    }
}

// ***** ResourceId
unsigned ResourceId::GenerateIdString(char* pbuffer, UPInt bufferSize, char suffixLetter) const
{
    SF_ASSERT(bufferSize >= 9);
    if (!suffixLetter)
    {
        switch(GetIdType())
        {
        case IdType_GradientImage:
            *pbuffer++ = 'G';
            break;
        case IdType_DynFontImage:
        case IdType_FontImage:
            *pbuffer++ = 'F';
            break;
        default:
            *pbuffer++ = 'I';
            break;
        }
    }
    else
        *pbuffer++ = suffixLetter;

    LongFormatter f(GetIdIndex());
    f.SetBase(16).SetBigLetters().Convert();
    f.InitString(pbuffer, bufferSize);
    return (unsigned)f.GetSize();
}




ResourceFileInfo::ResourceFileInfo()
{
    Format = FileTypeConstants::File_Unknown;
    pExporterInfo = 0;
}

ResourceFileInfo::ResourceFileInfo( const ResourceFileInfo &src ) : FileName(src.FileName)
{
    Format = src.Format;
    pExporterInfo = src.pExporterInfo;
}

UPInt ResourceFileInfo::GetHashCode() const
{
    UPInt hashCode = String::BernsteinHashFunction(FileName.ToCStr(), FileName.GetSize());
    return hashCode ^ Format;
}

ResourceKey::ResourceKey()
{
    pKeyInterface = 0;
    hKeyData      = 0;
}

ResourceKey::ResourceKey( KeyInterface* pki, KeyHandle hk )
{
    if (pki)
        pki->AddRef(hk);
    pKeyInterface = pki;
    hKeyData      = hk;
}

ResourceKey::ResourceKey( const ResourceKey &src )
{
    if (src.pKeyInterface)
        src.pKeyInterface->AddRef(src.hKeyData);

    pKeyInterface = src.pKeyInterface;
    hKeyData      = src.hKeyData;
}

ResourceKey& ResourceKey::operator=( const ResourceKey &src )
{
    if (src.pKeyInterface)
        src.pKeyInterface->AddRef(src.hKeyData);
    if (pKeyInterface)
        pKeyInterface->Release(hKeyData);
    pKeyInterface = src.pKeyInterface;
    hKeyData      = src.hKeyData;
    return *this;
}
}} // Scaleform::GFx
