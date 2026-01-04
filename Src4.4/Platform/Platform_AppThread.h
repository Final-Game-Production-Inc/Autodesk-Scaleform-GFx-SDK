/**************************************************************************

Filename    :   Platform_AppThread.h
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Platform_AppThread_H
#define INC_SF_Platform_AppThread_H

#include "Platform/Platform_RTCommandQueue.h"
#include "Platform/Platform.h"
#include "Kernel/SF_Threads.h"

namespace Scaleform { 

namespace GFx { class Event; }

namespace Platform {

class Device;
class AppBase;

// 
//------------------------------------------------------------------------
// ***** AppThread


class AppThread : public Thread, public RTCommandQueue
{
public:
    AppThread(AppBase* papp) : pApp(papp) {}

    int     Run();
    void    WaitForInit() { AppInit.Wait(); }

    // Functions to be called from Input or Render thread
    void    OnEvent(const GFx::Event& event);
    void    OnOrientation(int orientation, bool force);
    void    OnSize(int width, int height);
    void    OnDropFiles(const String& filename);
    void    OnLifecycleEvent(int event);

    enum
    {
        EventSize = sizeof(GFx::GeolocationEvent) // currently the largest one used
    };

protected:
    AppBase*   pApp;
    Event      AppInit;

    struct EventStorage
    {
        UByte data[EventSize] __attribute__((aligned(4)));
    };

    void onEvent(EventStorage event);
    void onOrientation(int orientation, bool force);
    void onSize(int width, int height);
    void onDropFiles(String filename);
    void onLifecycleEvent(int event);
};

}}
#endif
