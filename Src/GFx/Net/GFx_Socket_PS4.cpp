/**************************************************************************

Filename    :   GFx_Socket_PS4.cpp
Content     :   Socket wrapper class

Created     :   July 03, 2009
Authors     :   Boris Rayskiy, Alex Mantzaris

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/Net/GFx_Socket.h"

#ifdef SF_ENABLE_SOCKETS

#define INVALID_SOCKET      -1
#define GWSAEWOULDBLOCK 35L

#include <net.h>
#include "Kernel/SF_Memory.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_MsgFormat.h"

#define GIPPROTOCOL_TCP 0
#define GIPPROTOCOL_UDP 0

namespace Scaleform {
namespace GFx {

// Platform-specific socket implementation for PS3
class GFxSocketImpl : public NewOverrideBase<Stat_Default_Mem>, public SocketInterface
{
public:
    GFxSocketImpl() : Socket(INVALID_SOCKET), ListenSocket(INVALID_SOCKET), LocalHostAddress(0) 
    {
    }

    ~GFxSocketImpl()
    {
    }

    // Create the socket
    bool CreateStream(bool listener)
    {
        SceNetId hSocket = sceNetSocket("StreamSocket", SCE_NET_AF_INET, SCE_NET_SOCK_STREAM, GIPPROTOCOL_TCP);
        if (listener)
        {
            ListenSocket = hSocket;
        }
        else
        {
            Socket = hSocket;
        }
        return (hSocket >= 0);
    }

    bool CreateDatagram(bool broadcast)
    {
        Socket = sceNetSocket("Datagram", SCE_NET_AF_INET, SCE_NET_SOCK_DGRAM, GIPPROTOCOL_UDP);
        if (!broadcast)
        {
            ListenSocket = Socket;
        }
        return (Socket >= 0);
    }

    bool Bind()
    {
        return (sceNetBind(ListenSocket, (SceNetSockaddr*)&SocketAddress, sizeof(SocketAddress)) >= 0);
    }

    bool Listen(int numConnections)
    {
        return (sceNetListen(ListenSocket, numConnections) >= 0);
    }

    bool Connect()
    {
        return (sceNetConnect(Socket, (SceNetSockaddr*)&SocketAddress, sizeof(SocketAddress)) >= 0);
    }

    bool Accept(int timeout)
    {
        if (timeout == 0)
        {
            SetBlocking(false);
        }
        else
        {
            int microsecTimeout = timeout * 1000000;
            sceNetSetsockopt(ListenSocket, SCE_NET_SOL_SOCKET, SCE_NET_SO_ACCEPTTIMEO, &microsecTimeout, sizeof(microsecTimeout));
        }

        SceNetSocklen_t iSize = sizeof(SocketAddress);
        int hSocket = sceNetAccept(ListenSocket, (SceNetSockaddr*)&SocketAddress, &iSize);
        if (hSocket >= 0)
        {
            Socket = hSocket;
            return true;
        }

        return false;
    }
    int Send(const char* dataBuffer, UPInt dataBufferSize) const
    {
        if (Socket < 0)
        {
            return -1;
        }

        int bytesSent = sceNetSend(Socket, dataBuffer, static_cast<size_t>(dataBufferSize), 0);
        if (bytesSent < 0)
        {
            return (GetLastError() == GWSAEWOULDBLOCK ? 0 : -1);
        }
        return bytesSent;
    }

    int Receive(char* dataBuffer, int dataBufferSize) const
    {
        if (Socket < 0)
        {
            return -1;
        }

        return sceNetRecv(Socket, dataBuffer, dataBufferSize, 0);
    }

    int SendBroadcast(const char* dataBuffer, UPInt dataBufferSize) const
    {
        if (Socket < 0)
        {
            return -1;
        }

        return sceNetSendto(Socket, dataBuffer, static_cast<size_t>(dataBufferSize), 0, 
            (SceNetSockaddr*)&SocketAddress, sizeof(SocketAddress));
    }

    int ReceiveBroadcast(char* dataBuffer, int dataSize) const
    {
        if (Socket < 0)
        {
            return -1;
        }

        SceNetSocklen_t addrLength = sizeof(SocketAddress);
        return sceNetRecvfrom(Socket, dataBuffer, dataSize, 0, (SceNetSockaddr*)&SocketAddress, &addrLength); 
    }

    void SetListenPort(UInt32 port)
    {
        memset(&SocketAddress, 0, sizeof(SceNetSockaddrIn));
        SocketAddress.sin_family = SCE_NET_AF_INET;
        SocketAddress.sin_addr.s_addr = sceNetHtonl(SCE_NET_INADDR_ANY);
        SocketAddress.sin_port = sceNetHtons((UInt16) port);
    }

    void SetBroadcastPort(UInt32 port)
    {
        memset(&SocketAddress, 0, sizeof(SceNetSockaddrIn));
        SocketAddress.sin_family = SCE_NET_AF_INET;
        SocketAddress.sin_addr.s_addr = sceNetHtonl(SCE_NET_INADDR_BROADCAST);
        SocketAddress.sin_port = sceNetHtons((UInt16) port);
    }

    void SetAddress(UInt32 port, const char* address)
    {
        memset(&SocketAddress, 0, sizeof(SceNetSockaddrIn));
        SocketAddress.sin_family = SCE_NET_AF_INET;
        SocketAddress.sin_port = sceNetHtons((UInt16) port);
        sceNetInetPton(SCE_NET_AF_INET, address, &SocketAddress.sin_addr.s_addr);
    }

    // Blocking means that some socket operations block the thread until completed
    // Non-blocking do not block, returning GWSAEWOULDBLOCK instead
    void SetBlocking(bool blocking)
    {
        if (Socket >= 0)
        {
            int nValue = blocking ? 0 : 1;
            sceNetSetsockopt(Socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_NBIO, &nValue, sizeof(nValue));
        }
    }

    void SetBroadcast(bool broadcast)
    {
        int iBroadcast = (broadcast ? 1 : 0);
        sceNetSetsockopt(Socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_BROADCAST, &iBroadcast, sizeof(iBroadcast));
    }

    // Port and IP address of connected socket
    // Returns true if successful
    void GetName(UInt32* port, UInt32* address, char* name, UInt32 nameSize)
    {
        *port = sceNetHtons(SocketAddress.sin_port);
        *address = sceNetNtohl(SocketAddress.sin_addr.s_addr);
        if (name != NULL)
        {
            *name = '\0';
        }

        if (LocalHostAddress == *address)
        {
            *address = 0x7F000001;  // localhost
        }
    }

    bool Shutdown()
    {
        if (Socket >= 0)
        {
            sceNetSocketClose(Socket); 
            Socket = INVALID_SOCKET;
        }
        return true;
    }

    // Initialize the socket library
    bool Startup()
    {
        Lock::Locker locker(&LibRefLock);
        if (LibRefs == 0)
        {
            if (sceNetInit() < 0)
            {
                return false;
            }

            SF_DEBUG_MESSAGE(true, "Socket library successfully initialized\n");
        }

        ++LibRefs;
        return true;
    }

    // Terminate the socket library
    void Cleanup()
    {
        Lock::Locker locker(&LibRefLock);
        if (LibRefs > 0)
        {
            --LibRefs;
            if (LibRefs == 0)
            {
                // Don't clean up the socket library in case someone else is using it
                // Unlike windows, the PS3 releases the library even if it has been initialized 
                // more times than it has been released
                //sceNetTerm();
                //::sys_net_finalize_network();
                //::cellSysmoduleUnloadModule(CELL_SYSMODULE_NET);
            }
        }
    }

    // Get error code
    int GetLastError() const
    {
        return sce_net_errno;
    }

    bool IsValid() const 
    { 
        return (Socket >= 0); 
    }

    bool IsListening() const
    {
        return (ListenSocket >= 0); 
    }

    bool ShutdownListener()
    {
        if (IsListening())
        {
            sceNetSocketClose(ListenSocket); 
            ListenSocket = INVALID_SOCKET;
        }
        return true;
    }

    bool CheckAbort() const 
    { 
        return false; 
    }

    SceNetSockaddrIn            SocketAddress;
    SceNetId                    Socket;
    SceNetId                    ListenSocket;

    mutable Hash<UInt32, String>        AddressMap;
    mutable UInt32                      LocalHostAddress;                     

    static UInt32               LibRefs;
    static Lock                 LibRefLock;
};

UInt32 GFxSocketImpl::LibRefs = 0;
Lock GFxSocketImpl::LibRefLock;

static DefaultSocketFactory<GFxSocketImpl> defaultSocketFactory;

SocketImplFactory* Socket::GetAs3DefaultFactory()
{
    return &defaultSocketFactory;
}

SocketImplFactory* Socket::GetAmpDefaultFactory()
{
    return &defaultSocketFactory;
}

} // namespace GFx
} // namespace Scaleform

#endif  // SF_ENABLE_SOCKETS

