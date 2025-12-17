/**************************************************************************

Filename    :   GFx_Socket_Std.cpp
Content     :   Socket wrapper class

Created     :   July 03, 2009
Authors     :   Boris Rayskiy, Alex Mantzaris

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFxConfig.h"

#ifdef SF_ENABLE_SOCKETS

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#if !defined(SF_OS_ORBIS)
    #include <sys/errno.h>
#else
    // errno.h is not in sys on ORBIS.
    #include <errno.h>
    // need SDK version, as some functions were removed in later SDK versions
    #include <sdk_version.h>
    #include <libnet/nettypes.h>
    #include <libnet/inet.h>
#endif
#include <unistd.h>
#include <fcntl.h>

#define INVALID_SOCKET      -1

#include "GFx_Socket.h"
#include "Kernel/SF_Memory.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_MsgFormat.h"

namespace Scaleform {
namespace GFx {

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
        int hSocket;
        hSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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
        Socket = ::socket(AF_INET, SOCK_DGRAM, 0);
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

            int retVal = ::select((int)ListenSocket+1, &readfds, NULL, NULL, &tv);
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
            return (GetLastError() == EWOULDBLOCK ? 0 : -1);
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

        // Orbis SDK v0.820 removed inet_addr function.
#if !defined(SF_OS_ORBIS) && SCE_ORBIS_SDK_VERSION <= 0x00810030u
        SocketAddress.sin_addr.s_addr = inet_addr(address);
#else
        sceNetInetPton(AF_INET, address, &SocketAddress.sin_addr.s_addr);
#endif
    }

    // Blocking means that some socket operations block the thread until completed
    // Non-blocking do not block, returning EWOULDBLOCK instead
    void SetBlocking(bool blocking)
    {
        if (Socket  >= 0)
        {
            int cmd = fcntl(Socket, F_GETFL, 0);
            if (blocking)
            {
                cmd &= ~O_NONBLOCK;
            }
            else
            {
                cmd |= O_NONBLOCK;
            }
            fcntl(Socket, F_SETFL, cmd);
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
            ::shutdown(Socket, 2);
            ::close(Socket);
            Socket = INVALID_SOCKET;
        }
        return true;
    }

    // Initialize the socket library
    bool Startup()
    {
        return true;
    }

    // Terminate the socket library
    void Cleanup()
    {
    }

    // Get error code
    int GetLastError() const
    {
        return errno;
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
            ::shutdown(ListenSocket, 2);
            ::close(ListenSocket);
            ListenSocket = INVALID_SOCKET;
        }
        return true;
    }

    bool CheckAbort() const
    {
        return false;
    }

    sockaddr_in                SocketAddress;
    int                      Socket;
    int                      ListenSocket;

    mutable Hash<UInt32, String>        AddressMap;
    mutable UInt32                      LocalHostAddress;
};


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
