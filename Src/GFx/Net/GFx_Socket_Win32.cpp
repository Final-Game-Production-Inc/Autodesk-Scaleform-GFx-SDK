/**************************************************************************

Filename    :   GFx_Socket_Win32.cpp
Content     :   Socket wrapper class

Created     :   January 30 2012
Authors     :   Boris Rayskiy, Alex Mantzaris

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/


#include "GFxConfig.h"

#if defined(SF_OS_WIN32) && defined(SF_ENABLE_SOCKETS)

#include <winsock2.h>
#include <errno.h>
#include <ws2tcpip.h>
#include "GFx_Socket.h"
#include "Kernel/SF_Memory.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_MsgFormat.h"

namespace Scaleform {
namespace GFx {


// Platform-specific socket implementation for Win32
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
			readfds.fd_count = 1;
			readfds.fd_array[0] = ListenSocket;

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
        // 替换 inet_addr 为 InetPton
        if (InetPtonA(AF_INET, address, &SocketAddress.sin_addr) != 1) {
            // 处理转换失败（例如设置为无效地址）
            SocketAddress.sin_addr.s_addr = INADDR_NONE;
        }
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
            Hash<UInt32, String>::Iterator it = AddressMap.Find(*address);
            if (it == AddressMap.End())
            {
                SF_ASSERT(nameSize <= NI_MAXHOST);
                char servInfo[NI_MAXSERV];
                if (getnameinfo((sockaddr*) &SocketAddress, sizeof(sockaddr), name, nameSize, servInfo, NI_MAXSERV, NI_NOFQDN) != 0)
                {
                    *name = '\0';
                }
                AddressMap.Add(*address, name);
            }
            else
            {
                SFstrcpy(name, nameSize, it->Second);
            }
        }

        if (LocalHostAddress == 0)
        {
            char hostName[NI_MAXHOST];
            if (gethostname(hostName, NI_MAXHOST) == 0)
            {
                // 替换 gethostbyname 为 getaddrinfo
                struct addrinfo hints = { 0 };
                hints.ai_family = AF_INET;       // 仅考虑 IPv4
                hints.ai_socktype = SOCK_STREAM; // 匹配 TCP 类型
                hints.ai_protocol = IPPROTO_TCP;

                struct addrinfo* result = nullptr;
                int status = getaddrinfo(hostName, nullptr, &hints, &result);
                if (status == 0)
                {
                    for (struct addrinfo* p = result; p != nullptr; p = p->ai_next)
                    {
                        struct sockaddr_in* addr = reinterpret_cast<struct sockaddr_in*>(p->ai_addr);
                        if (addr->sin_addr.s_addr == SocketAddress.sin_addr.s_addr)
                        {
                            LocalHostAddress = *address;
                            break;
                        }
                    }
                    freeaddrinfo(result); // 释放资源
                }
            }
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
            WSADATA wsaData;
            WORD version = MAKEWORD(2, 0);
            if (::WSAStartup(version, &wsaData) != 0)
            {
                SF_DEBUG_MESSAGE(1, "\nCreate: Error occured. Could not create socket\n");
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

    sockaddr_in                 SocketAddress;
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

