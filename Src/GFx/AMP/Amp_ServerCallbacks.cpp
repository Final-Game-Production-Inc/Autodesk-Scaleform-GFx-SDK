/**************************************************************************

Filename    :   Amp_ServerCallbacks.cpp
Content     :   AMP server interface implementations
Created     :   January 2010
Authors     :   Alex Mantzaris

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Amp_ServerCallbacks.h"
#ifdef SF_AMP_SERVER

#include "Amp_Server.h"
#include "Kernel/SF_MsgFormat.h"

namespace Scaleform {
namespace GFx {
namespace AMP {


// Called from the GFxAmpThreadManager send thread
// Handles messages on the received queue
bool SendThreadCallback::OnSendLoop()
{
    return AmpServer::GetInstance().HandleNextMessage();
}

//////////////////////////////////////////////////////////////

// Constructor
StatusChangedCallback::StatusChangedCallback(Scaleform::Event* connectedEvent)
    : ConnectedEvent(connectedEvent)
{
}

// Called by GFxAmpThreadManager whenever a change in the connection status has been detected
void StatusChangedCallback::OnStatusChanged(StatusType newStatus, StatusType oldStatus, 
                                                  const char* message)
{
    if (newStatus != oldStatus)
    {
        // Send the current paused/recording state to the player
        AmpServer::GetInstance().SendLog(message, (int)SFstrlen(message), Log_Message);
        if (newStatus == CS_OK)
        {
            AmpServer::GetInstance().SendAppControlCaps();
            AmpServer::GetInstance().SendCurrentState();
            if (AmpServer::GetInstance().IsPaused())
            {
                String msg("AMP Server is paused and will send no frame data\n");
                AmpServer::GetInstance().SendLog(msg, (int)msg.GetLength(), Log_Message);
            }
        }
        SF_DEBUG_MESSAGE1(true, "%s", message);

        // Signal the server that connection has been established
        // This is used so that the application can wait for a connection
        // during startup, and thus can get profiled from the first frame
        if (ConnectedEvent != NULL)
        {
            if (newStatus == CS_OK)
            {
                ConnectedEvent->SetEvent();
            }
            else
            {
                ConnectedEvent->ResetEvent();
            }
        }
    }
}

void StatusChangedCallback::OnMsgVersionMismatch(int otherVersion)
{
    String logMsg;
    Format(logMsg, "AMP message version mismatch (Server {0}, Client {1}) - full functionality may not be available",
        Message::GetLatestVersion(), otherVersion);
    Server::GetInstance().SendLog(logMsg, static_cast<UInt32>(logMsg.GetLength()), Log_Warning);
}

void StatusChangedCallback::OnMsgGFxVersionChanged(UByte newVersion)
{
    SF_UNUSED(newVersion); // AMP server doesn't care what is the GFx version of the client
}


} // namespace AMP
} // namespace GFx
} // namespace Scaleform

#endif   // SF_AMP_SERVER
