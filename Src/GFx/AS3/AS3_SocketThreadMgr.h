/**************************************************************************

Filename    :   AS3_SocketThreadMgr.h
Content     :   Manages the socket threads
Created     :   September 2012
Authors     :   Alex Mantzaris

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/


#ifndef INC_GFX_AS3_SOCKET_THREAD_MGR_H
#define INC_GFX_AS3_SOCKET_THREAD_MGR_H

#include "GFx/Net/GFx_Socket.h"
#include "Kernel/SF_RefCount.h"
#include "GFx/GFx_PlayerStats.h"
#include "Kernel/SF_String.h"


#ifdef SF_ENABLE_SOCKETS

namespace Scaleform {
    class Thread;
namespace GFx {
namespace AS3 {

class SocketBuffer;
namespace Instances {
namespace fl_net
{
    class Socket;
}
}

class SocketThreadMgr : public RefCountBase<SocketThreadMgr, StatMV_Other_Mem>
{
public:
    SocketThreadMgr(bool initSocketLib, SocketImplFactory* socketImplFactory, Instances::fl_net::Socket* sock);
    virtual ~SocketThreadMgr();

    // Initialize Sockets - Starts the threads for sending and receiving messages
    bool Init(const char* ipAddress, int port);
    // Uninitialize Sockets - Performs thread cleanup
    void Uninit();

    const StringLH& GetAddress() const;
    int GetPort() const;
    UInt32 GetBytesPending() const;
    UInt32 GetBytesAvailable() const;
    bool IsRunning() const;
    void SetConnectTimeout(UInt32 timeoutMillisecs);
    UInt32 GetConnectTimeout() const;

    bool ReadBool(bool* valueRead);
    bool ReadByte(SByte* valueRead);
    bool ReadBytes(Array<SByte>* valueRead, int length);
    bool ReadDouble(Double* valueRead);
    bool ReadFloat(float* valueRead);
    bool ReadInt(SInt32* valueRead);
    bool ReadShort(SInt16* valueRead);

    void SendBool(bool sendValue);
    void SendByte(SByte sendValue);
    void SendBytes(const SByte* sendValue, int length);
    void SendDouble(Double sendValue);
    void SendFloat(float sendValue);
    void SendInt(SInt32 sendValue);
    void SendShort(SInt16 sendValue);

    void CheckEvents();

private:
    
    enum 
    {
        // maximum packet size - larger messages are broken up
        BufferSize = 512,  
        DefaultConnectTimeout = 20000
    };
    enum EventTypes
    {
        EventClose,
        EventConnect,
#ifdef SF_AS3_VERSION_AIR
        EventOutputProgress,
#endif
        EventSecurityError,
        EventSocketData
    };
    struct EventInfo : public RefCountBase<EventInfo, StatMV_Other_Mem>
    {
        EventTypes EventType;
        Array<UInt32> EventParameters;
    };

    Instances::fl_net::Socket* AS3Sock;
    Ptr<Thread> SocketThread;
    mutable Lock ReceivedBufferLock;
    Ptr<SocketBuffer> ReceivedBuffer;
    mutable Lock SendingBufferLock;
    Ptr<SocketBuffer> SendingBuffer;
    int Port;
    StringLH IpAddress;
    Lock SocketLock;
    Socket Sock;
    bool InitSocketLib;
    mutable Lock StatusLock;
    bool Exiting;
    UInt32 ConnectTimeout;
    mutable Lock EventQueueLock;
    ArrayLH<Ptr<EventInfo>, StatMV_Other_Mem> EventQueue;

    // Thread function
    static int SocketThreadLoop(Thread* sendThread, void* param);

    // Private helper functions
    bool IsExiting() const;
    void SetExiting();
    bool SendReceiveLoop();
    void QueueEvent(EventTypes eventType, UInt32* eventParams, UInt32 numParams);

#ifdef TEST_SERVER
    Ptr<Thread> TestServerSocketThread;
    Ptr<SocketBuffer> TestServerBuffer;
    Socket TestServerSock;  // just for testing
    static int TestServerSocketThreadLoop(Thread* sendThread, void* param);
    bool TestServerLoop();
#endif
};

} // namespace AS3
} // namespace GFx
} // namespace Scaleform

#endif  // SF_ENABLE_SOCKETS

#endif // INC_GFX_AS3_SOCKET_THREAD_MGR_H
