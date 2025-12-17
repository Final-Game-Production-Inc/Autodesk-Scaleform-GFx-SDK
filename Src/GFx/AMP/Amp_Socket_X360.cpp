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

#include <Xtl.h>
#include <errno.h>
#include "Kernel/SF_Memory.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_MsgFormat.h"

namespace Scaleform {
namespace GFx {
namespace AMP {

// Platform-specific socket implementation
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
        SOCKET hSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listener)
        {
            ListenSocket = hSocket;
        }
        else
        {
            Socket = hSocket;
        }
        return (hSocket != INVALID_SOCKET);
    }

    bool CreateDatagram(bool broadcast)
    {
        Socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (!broadcast)
        {
            ListenSocket = Socket;
        }
        return (Socket != INVALID_SOCKET);
    }

    bool Bind()
    {
        return (::bind(ListenSocket, (LPSOCKADDR) &SocketAddress, sizeof(SocketAddress)) != SOCKET_ERROR);
    }

    bool Listen(int numConnections)
    {
        return (::listen(ListenSocket, numConnections) != SOCKET_ERROR);
    }

    bool Connect()
    {
        return (::connect(Socket, (LPSOCKADDR) &SocketAddress, sizeof(SocketAddress)) != SOCKET_ERROR);
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

        int iSize = sizeof(SocketAddress);
        SOCKET hSocket = ::accept(ListenSocket, (LPSOCKADDR) &SocketAddress, &iSize);
        if (hSocket != INVALID_SOCKET)
        {
            Socket = hSocket;
            return true;
        }

        return false;
    }
    int Send(const char* dataBuffer, UPInt dataBufferSize) const
    {
        if (Socket == INVALID_SOCKET)
        {
            return -1;
        }

        int bytesSent = ::send(Socket, dataBuffer, static_cast<int>(dataBufferSize), 0);
        if (bytesSent == SOCKET_ERROR)
        {
            return (GetLastError() == WSAEWOULDBLOCK ? 0 : -1);
        }
        return bytesSent;
    }

    int Receive(char* dataBuffer, int dataBufferSize) const
    {
        if (Socket == INVALID_SOCKET)
        {
            return -1;
        }

        return ::recv(Socket, dataBuffer, dataBufferSize, 0);
    }

    int SendBroadcast(const char* dataBuffer, UPInt dataBufferSize) const
    {
        if (Socket == INVALID_SOCKET)
        {
            return -1;
        }

        return ::sendto(Socket, dataBuffer, static_cast<int>(dataBufferSize), 0, 
            (const LPSOCKADDR) &SocketAddress, sizeof(SocketAddress));
    }

    int ReceiveBroadcast(char* dataBuffer, int dataSize) const
    {
        if (Socket == INVALID_SOCKET)
        {
            return -1;
        }

        int addrLength = sizeof(SocketAddress);
        return ::recvfrom(Socket, dataBuffer, dataSize, 0, (LPSOCKADDR)&SocketAddress, &addrLength); 
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
    // Non-blocking do not block, returning WSAEWOULDBLOCK instead
    void SetBlocking(bool blocking)
    {
        if (Socket != INVALID_SOCKET)
        {
            u_long iMode = blocking ? 0 : 1;
            ::ioctlsocket(Socket, FIONBIO, &iMode);
        }
    }

    void SetBroadcast(bool broadcast)
    {
        int iBroadcast = (broadcast ? 1 : 0);
        ::setsockopt(Socket, SOL_SOCKET, SO_BROADCAST, (char*)&iBroadcast, sizeof(iBroadcast));
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
        if (Socket != INVALID_SOCKET)
        {
            ::closesocket(Socket);
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
            XNetStartupParams xnsp;
            memset(&xnsp, 0, sizeof(xnsp));
            xnsp.cfgSizeOfStruct = sizeof(XNetStartupParams);
            xnsp.cfgFlags = XNET_STARTUP_BYPASS_SECURITY;
            if (::XNetStartup(&xnsp) != 0)
            {
                return false;
            }

            WSADATA wsaData;
            WORD version = MAKEWORD(2, 0);
            if (::WSAStartup(version, &wsaData) != 0)
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
                ::WSACleanup();
                ::XNetCleanup();
            }
        }
    }

    // Get error code
    int GetLastError() const
    {
        return ::WSAGetLastError();
    }

    bool IsValid() const 
    { 
        return (Socket != INVALID_SOCKET); 
    }

    bool IsListening() const
    {
        return (ListenSocket != INVALID_SOCKET); 
    }

    bool ShutdownListener()
    {
        if (IsListening())
        {
            ::closesocket(ListenSocket);
            ListenSocket = INVALID_SOCKET;
        }
        return true;
    }

    bool CheckAbort() const 
    { 
        return false; 
    }

    sockaddr_in                SocketAddress;
    SOCKET                      Socket;
    SOCKET                      ListenSocket;

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

