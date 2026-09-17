/**********************************************************************

PublicHeader:   Render
Filename    :   D3D1x_Sync.h
Content     :   
Created     :   Feb 2012
Authors     :   Bart Muzzin

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#ifndef INC_SF_D3D1x_Sync_H
#define INC_SF_D3D1x_Sync_H

#include "Render/Render_Sync.h"
#include "Kernel/SF_Threads.h"
#include "Kernel/SF_Debug.h"

namespace Scaleform { namespace Render { namespace D3D1x {

// Wrapper class for a ID3D1x(Query), so that their allocations can be pooled.
class FenceWrapper : public Render::FenceWrapper
{
public:
    FenceWrapper(ID3D1x(Device)* pdevice, ID3D1x(DeviceContext)* pdeviceCtx) : Render::FenceWrapper()
    {
        SF_DEBUG_ASSERT(pdevice != nullptr, "NULL device in FenceWrapper constructor.");
		SF_DEBUG_ASSERT(pdeviceCtx != nullptr, "NULL deviceCtx in FenceWrapper constructor.");

#if !defined(SF_DURANGO_MONOLITHIC)
        // D3D1x uses queries, create one for this entry in the wrapper list.
        pQuery = 0;
        D3D1x(QUERY_DESC) desc;
        memset(&desc, 0, sizeof(D3D1x(QUERY_DESC)));
        desc.Query = D3D1x(QUERY_EVENT);
        HRESULT hr = pdevice->CreateQuery(&desc, &pQuery.GetRawRef());
        SF_UNUSED(hr);
        SF_DEBUG_ASSERT(SUCCEEDED(hr), "Failed to create ID3D1x(Query).");
        pDeviceContext = pdeviceCtx;
#else
        FenceID = 0;
        // Durango uses the InsertFence/IsFencePending API, not usual D3D1x queries. Save and cast the device context.
        if (FAILED(pdeviceCtx->QueryInterface(IID_ID3D11DeviceContextX, (void**)&pDeviceContext.GetRawRef())) || !pDeviceContext)
        {
            SF_DEBUG_ASSERT(pDeviceContext, "Unexpected ID3D11DeviceContextX interface not available when SF_DURANGO_MONOLITHIC is defined.");
        }

        if (FAILED(pdevice->QueryInterface(IID_ID3D11DeviceX, (void**)&pDevice.GetRawRef())) || !pDevice)
        {
            SF_DEBUG_ASSERT(pDeviceContext, "Unexpected ID3D11DeviceX interface not available when SF_DURANGO_MONOLITHIC is defined.");
        }
#endif
    } 

    void InsertFence()
    {
#if !defined(SF_DURANGO_MONOLITHIC)
        D3D1xEndAsynchronous(pDeviceContext, pQuery);
#else
		FenceID = pDeviceContext->InsertFence(D3D11_INSERT_FENCE_NO_KICKOFF);
#endif
    }

    bool IsFencePending()
    {
#if !defined(SF_DURANGO_MONOLITHIC)
        HRESULT hr = D3D1xGetDataAsynchronous(pDeviceContext, pQuery, 0, 0, D3D1x(ASYNC_GETDATA_DONOTFLUSH));
        return hr != S_OK;
#else
		return pDevice->IsFencePending(FenceID) ? true : false;
#endif
    }

    void WaitFence()
    {
#if !defined(SF_DURANGO_MONOLITHIC)
        while ( D3D1xGetDataAsynchronous(pDeviceContext, pQuery, 0, 0, 0) != S_OK)
#else
        while (pDevice->IsFencePending(FenceID))
#endif
        {
            // Perform useless commands to kickoff the command buffer (we don't use the GS currently, so just set its constants to NULL).
            ID3D1x(Buffer)* nullBuffer = 0;
            pDeviceContext->GSSetConstantBuffers(0, 1, &nullBuffer);
        }
    }

    // Durango uses the InsertFence/WaitFence API, not usual D3D1x queries.
#if !defined(SF_DURANGO_MONOLITHIC)
    Ptr<ID3D1x(DeviceContext)>     pDeviceContext;
    Ptr<ID3D1x(Query)>             pQuery;
#else
    Ptr<ID3D1x(DeviceX)>           pDevice;
    Ptr<ID3D1x(DeviceContextX)>    pDeviceContext;
	UINT64                         FenceID;
#endif
};

// List class which manages a pool of FenceWrappers (ID3D1x(Query) wrappers).
class FenceWrapperList : public Render::FenceWrapperList
{
public:
    virtual void Shutdown()
    {
        SF_DEBUG_ASSERT(pDevice == 0, "Expected NULL device in FenceWrapperList::Shutdown");
        pDevice = 0;
        Render::FenceWrapperList::Shutdown();
    }
    void SetDevice(ID3D1x(Device)* pdevice, ID3D1x(DeviceContext)* pdeviceCtx)
    {
        pDevice = pdevice;
		pDeviceCtx = pdeviceCtx;
        if (!pDevice || !pDeviceCtx)
            Shutdown();
        else
            Initialize();
    }
protected:
    virtual Render::FenceWrapper* allocateWrapper()
    {
        return SF_NEW FenceWrapper(pDevice, pDeviceCtx);
    }
    Ptr<ID3D1x(Device)> pDevice;
	Ptr<ID3D1x(DeviceContext)> pDeviceCtx;
};

class RenderSync : public Render::RenderSync
{
public:
    RenderSync() : pDevice(0), pNextEndFrameFence(0) { };    

    bool SetDevice(ID3D1x(Device)* pdevice, ID3D1x(DeviceContext)* pdeviceCtx) 
    { 
        // If we are removing the device, destroy any outstanding queries. This happens when
        // resetting the device or shutting down, so otherwise they would leak.
        if ( pdevice == 0 || pdeviceCtx == 0)
        {
            pDevice = 0;
            pDeviceContext = 0;
            ReleaseOutstandingFrames();
        }
        else
        {
            // Test creating a query, just to make sure it works.
            D3D1x(QUERY_DESC) desc;
            memset(&desc, 0, sizeof(D3D1x(QUERY_DESC)));
            desc.Query = D3D1x(QUERY_EVENT);

            ID3D1x(Query)* pquery = 0;
            HRESULT hr = pdevice->CreateQuery(&desc, &pquery);
            if ( pquery )
            {
                pquery->Release();
                pDevice = pdevice; 
                pDeviceContext = pdeviceCtx;
            }
            if (!SUCCEEDED(hr))
                return false;
        }

        QueryList.SetDevice(pdevice, pdeviceCtx);
        return true;
    }

    virtual void    BeginFrame()
    {
        if ( !pDevice )
            return;

        // Create a query object, but do not issue it; the fences that are inserted this frame
        // need to have a handle to their query object, but it must be issued after they are.
        SF_ASSERT(pNextEndFrameFence == 0);
        pNextEndFrameFence = *reinterpret_cast<FenceWrapper*>(QueryList.Alloc());
        Render::RenderSync::BeginFrame();
    }

    virtual bool    EndFrame()
    {
        if ( !pDevice || !pDeviceContext)
            return true;

        if ( !Render::RenderSync::EndFrame())
            return false;

        // Now issue the query from this frame.
         if (pNextEndFrameFence)
         {
             pNextEndFrameFence->InsertFence();
         }		
        pNextEndFrameFence = 0;
        return true;
    }

    virtual void    KickOffFences(FenceType waitType)     
    {  
        // Unneeded, GPU will always continue processing.
        SF_UNUSED(waitType);
    }

protected:

    virtual UInt64   SetFence()
    { 
        if ( pNextEndFrameFence )
            pNextEndFrameFence->AddRef();
        return (UInt64)(UPInt)pNextEndFrameFence.GetPtr();
    }

    virtual bool    IsPending(FenceType waitType, UInt64 handle, const FenceFrame& parent) 
    { 
        SF_UNUSED2(waitType, parent);
        FenceWrapper* pquery = reinterpret_cast<FenceWrapper*>(handle);
        if ( pquery == 0 )
            return false;

        // No fence frames indicates either an impending reset, or before the first BeginFrame.
        // Either way, report that no queries are pending.
        if ( FenceFrames.IsEmpty() )
            return false;

        // If we are querying the current frame's fence, it must be unsignaled, because the
        // query is always issued immediately before 'forgetting' the next frame's query pointer.
        if ( pquery == pNextEndFrameFence )
            return true;

        return pquery->IsFencePending();
    }
    virtual void    WaitFence(FenceType waitType, UInt64 handle, const FenceFrame& parent)
    {
        SF_UNUSED2(waitType, parent);
        FenceWrapper* pquery = reinterpret_cast<FenceWrapper*>(handle);
        if ( pquery == 0 )
            return;        

        // If we are waiting on the current frame's query, we must issue it first. This is the worst
        // can scenario, as we will be waiting on a query that was just issued; causing a CPU/GPU sync.
        if ( pquery == pNextEndFrameFence )
        {
            EndFrame();
            BeginFrame();
        }

        // Poll until the data is ready.
        pquery->WaitFence();
    }

    virtual void    ReleaseFence(UInt64 handle)
    {
        QueryList.Free(reinterpret_cast<FenceWrapper*>(handle));
    }

    // No need to implement wraparound; D3D1x queries have no external ordering.

private:
    Ptr<FenceWrapper>           pNextEndFrameFence;
    Ptr<ID3D1x(Device)>         pDevice;
    Ptr<ID3D1x(DeviceContext)>  pDeviceContext;
    FenceWrapperList            QueryList;
};

}}} // Scaleform::Render::D3D1x

#endif // INC_SF_D3D1x_Sync_H
