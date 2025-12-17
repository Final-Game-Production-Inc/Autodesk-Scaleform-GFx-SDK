/**************************************************************************

Filename    :   Amp_Socket.cpp
Content     :   Socket wrapper class

Created     :   July 03, 2009
Authors     :   Boris Rayskiy, Alex Mantzaris

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Amp_Socket.h"

#ifdef SF_ENABLE_SOCKETS

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/time.h>
#include <netex/libnetctl.h>
#include <netex/errno.h>

#define INVALID_SOCKET      -1
#define GWSAEWOULDBLOCK 35L

#include <netex/net.h>
#include <netex/ifctl.h>
#include <cell/sysmodule.h>
#include <sys/process.h>
#include "Kernel/SF_Memory.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_MsgFormat.h"

#define GIPPROTOCOL_TCP 0
#define GIPPROTOCOL_UDP 0

namespace Scaleform {
namespace GFx {
namespace AMP {

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
        int hSocket = ::socket(AF_INET, SOCK_STREAM, GIPPROTOCOL_TCP);
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
        Socket = ::socket(AF_INET, SOCK_DGRAM, GIPPROTOCOL_UDP);
        if (!broadcast)
        {
            ListenSocket = Socket;
        }
        return (Socket >= 0);
    }

    bool Bind()
    {
        return (::bind(ListenSocket, (sockaddr*) &SocketAddress, sizeof(SocketAddress)) >= 0);
    }

    bool Listen(int numConnections)
    {
        return (::listen(ListenSocket, numConnections) >= 0);
    }

    bool Connect()
    {
        return (::connect(Socket, (sockaddr*) &SocketAddress, sizeof(SocketAddress)) >= 0);
    }

    bool Accept(int timeout)
    {
        if (timeout >= 0)
        {
            timeval tv;
            tv.tv_sec = timeout;
            tv.tv_usec = 0;

            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(ListenSocket, &readfds);

            int retVal = ::socketselect((int)ListenSocket+1, &readfds, NULL, NULL, &tv);
            SF_DEBUG_MESSAGE1(retVal < 0, "Select: Error %d\n", GetLastError());
            if (retVal <= 0 || !FD_ISSET(ListenSocket, &readfds))
            {
                return false;
            }

        }

        socklen_t iSize = sizeof(SocketAddress);
        int hSocket = ::accept(ListenSocket, (sockaddr*) &SocketAddress, &iSize);
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

        int bytesSent = ::send(Socket, dataBuffer, static_cast<int>(dataBufferSize), 0);
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

        return ::recv(Socket, dataBuffer, dataBufferSize, 0);
    }

    int SendBroadcast(const char* dataBuffer, UPInt dataBufferSize) const
    {
        if (Socket < 0)
        {
            return -1;
        }

        return ::sendto(Socket, dataBuffer, static_cast<int>(dataBufferSize), 0, 
            (const sockaddr*) &SocketAddress, sizeof(SocketAddress));
    }

    int ReceiveBroadcast(char* dataBuffer, int dataSize) const
    {
        if (Socket < 0)
        {
            return -1;
        }

        socklen_t addrLength = sizeof(SocketAddress);
        return ::recvfrom(Socket, dataBuffer, dataSize, 0, (sockaddr*)&SocketAddress, &addrLength); 
    }

    void SetListenPort(UInt32 port)
    {
        memset(&SocketAddress, 0, sizeof(sockaddr_in));
        SocketAddress.sin_family = AF_INET;
        SocketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
        SocketAddress.sin_port = htons((UInt16) port);
    }

    void SetBroadcastPort(UInt32 port)
    {
        memset(&SocketAddress, 0, sizeof(sockaddr_in));
        SocketAddress.sin_family = AF_INET;
        SocketAddress.sin_addr.s_addr = htonl(INADDR_BROADCAST);
        SocketAddress.sin_port = htons((UInt16) port);
    }

    void SetAddress(UInt32 port, const char* address)
    {
        memset(&SocketAddress, 0, sizeof(sockaddr_in));
        SocketAddress.sin_family = AF_INET;
        SocketAddress.sin_port = htons((UInt16) port);
        SocketAddress.sin_addr.s_addr = inet_addr(address);
    }

    // Blocking means that some socket operations block the thread until completed
    // Non-blocking do not block, returning GWSAEWOULDBLOCK instead
    void SetBlocking(bool blocking)
    {
        if (Socket >= 0)
        {
            int nValue = blocking ? 0 : 1;
            ::setsockopt(Socket, SOL_SOCKET, SO_NBIO, &nValue, sizeof(nValue));
        }
    }

    void SetBroadcast(bool broadcast)
    {
        int iBroadcast = (broadcast ? 1 : 0);
        ::setsockopt(Socket, SOL_SOCKET, SO_BROADCAST, &iBroadcast, sizeof(iBroadcast));
    }

    // Port and IP address of connected socket
    // Returns true if successful
    void GetName(UInt32* port, UInt32* address, char* name, UInt32 nameSize)
    {
        *port = ntohs(SocketAddress.sin_port);
        *address = ntohl(SocketAddress.sin_addr.s_addr);
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
            ::socketclose(Socket); 
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
            if (::cellSysmoduleLoadModule(CELL_SYSMODULE_NET) < 0)
            {
                return false;
            }

            if (sys_net_initialize_network() < 0) 
            {
                return false;
            }

            int err = ::cellNetCtlInit();
            if (err < 0 && err != (int)CELL_NET_CTL_ERROR_NOT_TERMINATED)
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
                //::cellNetCtlTerm();
                //::sys_net_finalize_network();
                //::cellSysmoduleUnloadModule(CELL_SYSMODULE_NET);
            }
        }
    }

    // Get error code
    int GetLastError() const
    {
        return sys_net_errno;
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
            ::socketclose(ListenSocket); 
            ListenSocket = INVALID_SOCKET;
        }
        return true;
    }

    bool CheckAbort() const 
    { 
        return false; 
    }

    sockaddr_in                 SocketAddress;
    int                         Socket;
    int                         ListenSocket;

    mutable Hash<UInt32, String>        AddressMap;
    mutable UInt32                      LocalHostAddress;                     

    static UInt32               LibRefs;
    static Lock                 LibRefLock;
};

UInt32 GFxSocketImpl::LibRefs = 0;
Lock GFxSocketImpl::LibRefLock;

static DefaultSocketFactory<GFxSocketImpl> defaultSocketFactory;
SocketImplFactory* GlobalDefaultSocketFactory = &defaultSocketFactory;

} // namespace AMP
} // namespace GFx
} // namespace Scaleform

#endif  // SF_ENABLE_SOCKETS

