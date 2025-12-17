/**************************************************************************

Filename    :   Amp_ServerCallbacks.h
Content     :   AMP server interface implementations
Created     :   January 2010
Authors     :   Alex Mantzaris

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GFXAMP_SEND_THREAD_CALLBACK_H
#define INC_GFXAMP_SEND_THREAD_CALLBACK_H

#include "GFxConfig.h"
#ifdef SF_AMP_SERVER

#include "Kernel/SF_RefCount.h"
#include "Amp_Interfaces.h"

namespace Scaleform {
    class Event;
namespace GFx {
namespace AMP {

// SendThreadCallback::OnSendLoop is called once per "frame" 
// from the GFxAmpThreadManager send thread
//
class SendThreadCallback : 
    public RefCountBase<SendThreadCallback, StatAmp_Server>, 
    public SendInterface
{
public:
    virtual ~SendThreadCallback() { }
    virtual bool OnSendLoop();
};

// StatusChangedCallback::OnStatusChanged is called by ThreadManager 
// whenever a change in the connection status has been detected
//
class StatusChangedCallback : 
    public RefCountBase<StatusChangedCallback, StatAmp_Server>, 
    public ConnStatusInterface
{
public:
    StatusChangedCallback(Scaleform::Event* connectedEvent = NULL);
    virtual ~StatusChangedCallback() { }
    virtual void OnStatusChanged(StatusType newStatus, StatusType oldStatus, const char* message);
    virtual void OnMsgVersionMismatch(int otherVersion);
    virtual void OnMsgGFxVersionChanged(UByte newVersion);

protected:
    Scaleform::Event* ConnectedEvent;   // notifies GFx that connection has been established
};

} // namespace AMP
} // namespace GFx
} // namespace Scaleform

#endif

#endif