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

#include <nn/types.h>
#include <nn/socket/socket_Common.h>
#include "3DS_socket_Berkeley.h"

#define INVALID_SOCKET      -1
#define GWSAEWOULDBLOCK 0

#include "Kernel/SF_Memory.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_MsgFormat.h"

namespace Scaleform {
namespace GFx {
namespace AMP {

// Platform-specific socket implementation for 3DS
class GFxSocketImpl : public NewOverrideBase<Stat_Default_Mem>, public SocketInterface
{
public:
    GFxSocketImpl() : Socket(INVALID_SOCKET), ListenSocket(INVALID_SOCKET), LocalHostAddress(0) 
    {
        errno = 0;
        SocketBuffer = NULL;
    }

    ~GFxSocketImpl()
    {
    }

    // Create the socket
    bool CreateStream(bool listener)
    {
        s32 hSocket;
        hSocket = nn::socket::Socket(nn::socket::PF_INET, nn::socket::SOCK_STREAM, 0);
        if (hSocket <= 0)
        {
            errno = hSocket;
        }
        if (listener)
        {
            ListenSocket = hSocket;
        }
        else
        {
            Socket = hSocket;
        }
        return (hSocket > 0);
    }

    bool CreateDatagram(bool broadcast)
    {
        Socket = INVALID_SOCKET; // broadcast not supported
        errno = nn::socket::EPROTONOSUPPORT;
        if (!broadcast)
        {
            ListenSocket = Socket;
        }
        return (Socket > 0);
    }

    bool Bind()
    {
        errno = nn::socket::Bind(ListenSocket, &SocketAddress);
        return (errno == 0);
    }

    bool Listen(int numConnections)
    {
        errno = nn::socket::Listen(ListenSocket, numConnections);
        return (errno == 0);
    }

    bool Connect()
    {
        errno = nn::socket::Connect(Socket, &SocketAddress);
        return (errno == 0);
    }

    bool Accept(int timeout)
    {
        int iSize = sizeof(SocketAddress);
        s32 hSocket = nn::socket::Accept(ListenSocket, &SocketAddress);
        if (hSocket <= 0)
        {
            errno = hSocket;
        }
        if (hSocket > 0)
        {
            Socket = hSocket;
            return true;
        }

        return false;
    }
    int Send(const char* dataBuffer, UPInt dataBufferSize) const
    {
        if (Socket <= 0)
        {
            return -1;
        }

        int bytesSent = nn::socket::Send(Socket, dataBuffer, static_cast<int>(dataBufferSize), 0);
        if (bytesSent == GWSAEWOULDBLOCK)
        {
            return 0;
        }
        if (bytesSent < 0)
        {
            SF_DEBUG_MESSAGE1(1, "AMP Socket: Send error %d\n", bytesSent);
            bytesSent = -1;
        }
        return bytesSent;
        if (bytesSent != 0)
        {
            return (GetLastError() == GWSAEWOULDBLOCK ? 0 : -1);
        }
        return bytesSent;
    }

    int Receive(char* dataBuffer, int dataBufferSize) const
    {
        if (Socket <= 0)
        {
            return -1;
        }

        return nn::socket::Recv(Socket, dataBuffer, dataBufferSize, 0);
    }

    int SendBroadcast(const char* dataBuffer, UPInt dataBufferSize) const
    {
        if (Socket <= 0)
        {
            return -1;
        }

        SF_UNUSED2(dataBufferSize, dataBuffer);
        errno = nn::socket::EPROTONOSUPPORT;
        return -1;
    }

    int ReceiveBroadcast(char* dataBuffer, int dataSize) const
    {
        if (Socket <= 0)
        {
            return -1;
        }

        int addrLength = sizeof(SocketAddress);
        SF_UNUSED2(dataBuffer, dataSize);
        errno = nn::socket::EPROTONOSUPPORT;
        return -1; 
    }

    void SetListenPort(UInt32 port)
    {
        memset(&SocketAddress, 0, sizeof(nn::socket::SockAddrIn));
        SocketAddress.len = 8;
        SocketAddress.family = nn::socket::AF_INET;
        SocketAddress.addr.addr = nn::socket::GetHostId();
        SocketAddress.port = nn::socket::HtoNs((UInt16) port);
    }

    void SetBroadcastPort(UInt32 port)
    {
        memset(&SocketAddress, 0, sizeof(nn::socket::SockAddrIn));
        SF_UNUSED(port);
    }

    void SetAddress(UInt32 port, const char* address)
    {
        memset(&SocketAddress, 0, sizeof(nn::socket::SockAddrIn));
        SF_UNUSED2(port, address);
//         SocketAddress.len = 8;
//         SocketAddress.family = nn::socket::AF_INET;
//         SocketAddress.port = nn::socket::HtoNs((UInt16) port);
//         nn::socket::InetAtoN(address, &SocketAddress.addr);
    }

    // Blocking means that some socket operations block the thread until completed
    // Non-blocking do not block, returning GWSAEWOULDBLOCK instead
    void SetBlocking(bool blocking)
    {
        if (Socket > 0)
        {
            int cmd = nn::socket::Fcntl(Socket, nn::socket::F_GETFL, 0);
            if (blocking)
            {
                cmd &= ~nn::socket::O_NONBLOCK;
            }
            else
            {
                cmd |= nn::socket::O_NONBLOCK;
            }
            nn::socket::Fcntl(Socket, nn::socket::F_SETFL, cmd);
        }
    }

    void SetBroadcast(bool broadcast)
    {
        SF_UNUSED(broadcast);
    }

    // Port and IP address of connected socket
    // Returns true if successful
    void GetName(UInt32* port, UInt32* address, char* name, UInt32 nameSize)
    {
        *port = nn::socket::HtoNs(SocketAddress.port);
        *address = (UInt32)nn::socket::HtoNl(SocketAddress.addr.addr);
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
        if (Socket > 0)
        {
            nn::socket::Shutdown(Socket, nn::socket::SHUT_RDWR);
            nn::socket::Close(Socket);
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
            static const size_t sizeForSockets = 65536;
            static const s32 maxSessions = 1;
            size_t reqMemSize = nn::socket::GetRequiredMemorySize(sizeForSockets, maxSessions);
            SocketBuffer = (UByte*)SF_HEAP_MEMALIGN(Memory::GetHeapByAddress(this), reqMemSize, 4096, Stat_Default_Mem);
            nn::Result initResult = nn::socket::Initialize((uptr)SocketBuffer, reqMemSize, (s32)sizeForSockets, maxSessions);
            if (initResult.IsFailure())
            {
                SF_DEBUG_MESSAGE1(1, "nn::socket::Initialize failed (%d)\n", initResult.GetDescription());
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
                nn::socket::Finalize();
                SF_HEAP_FREE(Memory::GetHeapByAddress(this), SocketBuffer);
            }
        }
    }

    // Get error code
    int GetLastError() const
    {
#if defined(SF_OS_WIIU)
        return SOLastError();
#else
        return errno;
#endif
    }

    bool IsValid() const 
    { 
        return (Socket > 0); 
    }

    bool IsListening() const
    {
        return (ListenSocket > 0); 
    }

    bool ShutdownListener()
    {
        if (IsListening())
        {
            nn::socket::Shutdown(ListenSocket, nn::socket::SHUT_RDWR);
            nn::socket::Close(ListenSocket);
            ListenSocket = INVALID_SOCKET;
        }
        return true;
    }

    bool CheckAbort() const 
    { 
        return false; 
    }

    nn::socket::SockAddrIn                SocketAddress;
    s32                      Socket;
    s32                      ListenSocket;
    mutable int                 errno;
    UByte*                      SocketBuffer;

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
