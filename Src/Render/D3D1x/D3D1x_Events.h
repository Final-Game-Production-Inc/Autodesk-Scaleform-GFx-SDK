/**********************************************************************

PublicHeader:   Render
Filename    :   D3D1x_Events.h
Content     :   Implementation of GPU events for use with PIX.
Created     :   Feb 2012
Authors     :   Bart Muzzin

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#ifndef INC_SF_D3D1x_Events_H
#define INC_SF_D3D1x_Events_H

#include "Render/D3D1x/D3D1x_Config.h"
#include "Render/Render_Events.h"
#include <windows.h>
#include <tchar.h>

namespace Scaleform { namespace Render { namespace D3D1x { 

// Used to create heirarchies in PIX captures.
#if !defined(SF_BUILD_SHIPPING) && !defined(__ID3DUserDefinedAnnotation_FWD_DEFINED__) && !defined(SF_DURANGO_MONOLITHIC)
    typedef INT (WINAPI * LPD3DPERF_BEGINEVENT)(D3DCOLOR, LPCWSTR);
    typedef INT (WINAPI * LPD3DPERF_ENDEVENT)(void);
#endif

class RenderEvent : public Render::RenderEvent
{
public:
    static void InitializeEvents(ID3D1x(DeviceContext)* pctx)
    {
        SF_DEBUG_ASSERT(pctx, "Cannot pass a NULL ID3D1x(DeviceContext) to InitializeEvents\n");
        pContext = pctx;
        pContext->AddRef();

#if !defined(SF_BUILD_SHIPPING) && !defined(__ID3DUserDefinedAnnotation_FWD_DEFINED__) && !defined(SF_DURANGO_MONOLITHIC)
        HMODULE hdll = LoadLibrary( _T("D3D9.dll") );
        if (hdll)
        {
            RenderEvent::BeginEventFn = (LPD3DPERF_BEGINEVENT)GetProcAddress( hdll, "D3DPERF_BeginEvent" );
            RenderEvent::EndEventFn   = (LPD3DPERF_ENDEVENT)GetProcAddress( hdll, "D3DPERF_EndEvent" );
        }
        SF_DEBUG_WARNING(hdll == 0, "Failed to load D3D9.dll - PIX captures will be unavailable.");
#endif
    }

    static void ShutdownEvents()
    {
        pContext->Release();
        pContext = 0;
    }

    virtual void Begin( const char* eventName )
    {
        SF_UNUSED(eventName);
#if !defined(SF_BUILD_SHIPPING) && !defined(SF_DURANGO_MONOLITHIC)
        // PIX events only take wide-character strings.
        wchar_t dest[256];
        size_t  chars;
        mbstowcs_s(&chars, dest, eventName, 256);

#if !defined(__ID3DUserDefinedAnnotation_FWD_DEFINED__)
        if ( BeginEventFn )
        {
            BeginEventFn(D3DCOLOR_XRGB(0,0,0), dest);
        }
#else // __ID3DUserDefinedAnnotation_FWD_DEFINED__
        if (!pContext)
            return;
        Ptr<ID3DUserDefinedAnnotation> eventPtr;
        HRESULT hr = pContext->QueryInterface( IID_ID3DUserDefinedAnnotation, reinterpret_cast<void**>(&eventPtr.GetRawRef()) );
        SF_DEBUG_WARNONCE(SUCCEEDED(hr), "Failed to get ID3DUserDefinedAnnotation object from context.\n");
        if (FAILED(hr))
            return;
        eventPtr->BeginEvent( dest );
#endif // __ID3DUserDefinedAnnotation_FWD_DEFINED__
#endif // SF_BUILD_SHIPPING && && !SF_DURANGO_MONOLITHIC
    }
    void End()
    {
#if !defined(SF_BUILD_SHIPPING) && !defined(SF_DURANGO_MONOLITHIC)
#if !defined(__ID3DUserDefinedAnnotation_FWD_DEFINED__)
        if ( EndEventFn )
            EndEventFn();
#else // __ID3DUserDefinedAnnotation_FWD_DEFINED__
        if (!pContext)
            return;
        Ptr<ID3DUserDefinedAnnotation> eventPtr;
        HRESULT hr = pContext->QueryInterface( IID_ID3DUserDefinedAnnotation, reinterpret_cast<void**>(&eventPtr.GetRawRef()) );
        SF_DEBUG_WARNONCE(SUCCEEDED(hr), "Failed to get ID3DUserDefinedAnnotation object from context.\n");
        if (FAILED(hr))
            return;
        eventPtr->EndEvent();
#endif // __ID3DUserDefinedAnnotation_FWD_DEFINED__
#endif // SF_BUILD_SHIPPING && !SF_DURANGO_MONOLITHIC
    }

#if !defined(SF_BUILD_SHIPPING) && !defined(__ID3DUserDefinedAnnotation_FWD_DEFINED__) && !defined(SF_DURANGO_MONOLITHIC)
    static LPD3DPERF_BEGINEVENT  BeginEventFn;
    static LPD3DPERF_ENDEVENT    EndEventFn;
#endif
    static ID3D1x(DeviceContext)*     pContext;
};

}}} // Scaleform::Render::D3D1x

#endif // INC_SF_D3D1x_Events_H
