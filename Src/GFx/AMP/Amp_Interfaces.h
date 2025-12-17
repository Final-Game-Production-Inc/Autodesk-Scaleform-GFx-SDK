/**************************************************************************

PublicHeader:   AMP
Filename    :   Amp_Interfaces.h
Content     :   Interfaces for customizing the behavior of AMP
Created     :   December 2009
Authors     :   Alex Mantzaris

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INCLUDE_GFX_AMP_INTERFACES_H
#define INCLUDE_GFX_AMP_INTERFACES_H

#include "GFxConfig.h"
#ifdef SF_ENABLE_STATS

#include "Kernel/SF_RefCount.h"
#include "GFx/GFx_PlayerStats.h"

namespace Scaleform {
namespace GFx {
namespace AMP {

class Message;
class MessageAppControl;

// 
// Custom AMP behavior is achieved by overriding the classes in this file
//

//Interface used to handle received messages that are dispatched by the MessageTypeRegistry
class IMessageHandler : public RefCountBase<IMessageHandler, StatAmp_Message>
{
public:
    virtual ~IMessageHandler() {};
    virtual void Handle(Message* message) = 0;
};


// AppControlInterface::HandleAmpRequest is called by Server
// whenever a MessageAppControl has been received
class AppControlInterface
{
public:
    virtual ~AppControlInterface() { }
    virtual bool HandleAmpRequest(const MessageAppControl* pMessage) = 0;
};

// SendInterface::OnSendLoop is called once per "frame" 
// from the ThreadManager send thread
class SendInterface
{
public:
    virtual ~SendInterface() { }
    virtual bool OnSendLoop() = 0;
};

// ConnStatusInterface::OnStatusChanged is called by ThreadManager 
// whenever a change in the connection status has been detected
class ConnStatusInterface
{
public:
    enum StatusType
    {
        CS_Idle         = 0x0,
        CS_Connecting   = 0x1,
        CS_OK           = 0x2,
        CS_Failed       = 0x3,
    };

    virtual ~ConnStatusInterface() { }
    virtual void OnStatusChanged(StatusType newStatus, StatusType oldStatus, const char* message) = 0;
    virtual void OnMsgVersionMismatch(int otherVersion) = 0;
    virtual void OnMsgGFxVersionChanged(UByte newVersion) = 0;
};

} // namespace AMP
} // namespace GFx
} // namespace Scaleform

#endif

#endif
