/**************************************************************************

Filename    :   GFx_Socket.cpp
Content     :   Socket wrapper class

Created     :   July 03, 2009
Authors     :   Boris Rayskiy, Alex Mantzaris

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx_Socket.h"

#ifdef SF_ENABLE_SOCKETS

#include "Kernel/SF_Debug.h"
#include "Kernel/SF_MsgFormat.h"

namespace Scaleform {
namespace GFx {

////////////////////////////////////////////////////////////////////

#ifdef SF_ENABLE_THREADS
class SocketLocker
{
public:     
    SocketLocker(Lock* lock) : SocketLock(lock)
    {
        if (SocketLock != NULL)
        {
            SocketLock->DoLock();
        }
    }
    ~SocketLocker()
    {
        if (SocketLock != NULL)
        {
            SocketLock->Unlock();
        }
    }
private:
    Lock*   SocketLock;
};
#endif



///////////////////////////////////////////////////////////////////
// Constructor
Socket::Socket(bool initLib, SocketImplFactory* socketImplFactory) : 
    SocketFactory(socketImplFactory),
    SocketImpl(NULL),
    IsServer(false), 
    InitLib(initLib)
#ifdef SF_ENABLE_THREADS
    , CreateLock(NULL)
#endif
{
    SocketImpl = SocketFactory->Create();

    // Socket library initialization
    if (InitLib)
    {
        if (!SocketImpl->Startup()) 
        {
            SF_DEBUG_MESSAGE(1, "Socket: Library initialization failed\n");
        }
    }
}

// Destructor
Socket::~Socket()
{
    Destroy();

    if (InitLib)
    {
        Cleanup();
    }
    SocketFactory->Destroy(SocketImpl);
}

// Create a client socket
// Returns true if successful
bool Socket::CreateClient(const char* ipAddress, UInt32 port, String* errorMsg)
{
#ifdef SF_ENABLE_THREADS
    SocketLocker locker(CreateLock);
#endif

    IsServer = false;

    if (!SocketImpl->CreateStream(false))
    {
        if (errorMsg != NULL)
        {
            SPrintF(*errorMsg, "Could not create socket. Error %d", SocketImpl->GetLastError());
        }
        SF_DEBUG_MESSAGE1(1, "Socket: Could not create socket. Error %d\n", 
            SocketImpl->GetLastError());
        Cleanup();
        return false;
    }

    SocketImpl->SetAddress(port, ipAddress);

    // Establish connection to specified socket 
    if (!SocketImpl->Connect())
    {
        if (errorMsg != NULL)
        {
            SPrintF(*errorMsg, "Could not connect to server. Error %d\n", SocketImpl->GetLastError());
        }
        SF_DEBUG_MESSAGE1(1, "Socket: could not connect to server. Error %d\n", SocketImpl->GetLastError());
        Destroy();
        return false;
    }

    if (errorMsg != NULL)
    {
        SPrintF(*errorMsg, "Socket connection established on port %d\n", port);
    }

    return true;
}

// Create a socket connection
// An IP address of NULL means that this is a server socket
// Returns true if successful
bool Socket::CreateServer(UInt32 port, String* errorMsg)
{
#ifdef SF_ENABLE_THREADS
    SocketLocker locker(CreateLock);
#endif
    if (SocketImpl->IsListening())
    {
        return true;
    }

    IsServer = true;

    if (!SocketImpl->CreateStream(true))
    {
        if (errorMsg != NULL)
        {
            SPrintF(*errorMsg, "Could not create listener socket. Error %d", SocketImpl->GetLastError());
        }
        SF_DEBUG_MESSAGE1(1, "Socket: Could not create listener socket. Error %d\n", 
            SocketImpl->GetLastError());
        Cleanup();
        return false;
    }

    SocketImpl->SetListenPort(port);

    // Associate local address with socket
    if (!SocketImpl->Bind())
    {
        if (errorMsg != NULL)
        {
            SPrintF(*errorMsg, "Could not associate local address (port %d) with listener socket. Error %d\n", 
                port,
                SocketImpl->GetLastError());
        }
        SF_DEBUG_MESSAGE2(1, 
            "Socket: Could not associate local address (port %d) with listener socket. Error %d\n", 
            port,
            SocketImpl->GetLastError());
        Destroy();
        return false;
    }

    if (!SocketImpl->Listen(1))
    {
        if (errorMsg != NULL)
        {
            SPrintF(*errorMsg, "Could not place socket in listening state. Error %d\n", 
                SocketImpl->GetLastError());
        }
        SF_DEBUG_MESSAGE1(1, "Socket: could not place socket in listening state. Error %d\n", 
            SocketImpl->GetLastError());
        Destroy();
        return false;
    }

    if (errorMsg != NULL)
    {
        SPrintF(*errorMsg, "Socket connection established on port %d\n", port);
    }

    return true;
}

// Shuts down and performs cleanup
void Socket::Destroy()
{
    if (IsValid())
    {
        Shutdown();
    }

    if (SocketImpl->IsListening())
    {
        SocketImpl->ShutdownListener();
    }
}

// Permits an incoming connection attempt on a socket
// Blocks the thread until connection has been established
// Returns true if connection has been established
bool Socket::Accept(int timeout)
{
    if (IsServer)
    {
        SetBlocking(true);

        // Only relevant for server sockets
        if (!SocketImpl->Accept(timeout))
        {
//             SF_DEBUG_MESSAGE1(1, "Socket: Incoming connection rejected. Error %d\n", 
//                 SocketImpl->GetLastError());
            if (IsValid())
            {
                Shutdown();
            }
            return false;
        }

        SF_DEBUG_MESSAGE(1, "Socket: Incoming connection accepted.\n");
    }

    return true;
}

// Sends data on the connected socket
// Returns true if successful
int Socket::Send(const char* dataBuffer, UPInt dataBufferSize) const
{
    int bytesTransferred = SocketImpl->Send(dataBuffer, dataBufferSize);
    if (bytesTransferred < 0)
    {
        SF_DEBUG_MESSAGE1(1, "Socket: Send Error %d\n", SocketImpl->GetLastError());
        return -1;
    }

    return bytesTransferred;
}

// Receives data from a connected socket 
// Blocks the thread until data have been received (or shutdown)
// Returns the number of bytes transmitted
int Socket::Receive(char* dataBuffer, int dataBufferSize) const
{
    int iBytesTransferred = SocketImpl->Receive(dataBuffer, dataBufferSize);
    if (iBytesTransferred < 0)
    {
        return 0;
    }
    return iBytesTransferred;
}

// Tests the outgoing or incoming connection
bool Socket::IsConnected() const
{
    // We determine the connection status based on 
    // whether or not an attempt to send is successful
    char strBuf[1];
    return (Send(strBuf, 0) >= 0);
}


// Blocking means that some socket operations block the thread until completed
// Non-blocking do not block, returning GWSAEWOULDBLOCK instead
void Socket::SetBlocking(bool blocking)
{
    SocketImpl->SetBlocking(blocking);
}

#ifdef SF_ENABLE_THREADS
// Set lock for multithreaded access
void Socket::SetLock(Lock* lock)
{
    CreateLock = lock;
}
#endif

bool Socket::IsValid() const
{
    return SocketImpl->IsValid();
}

bool Socket::CheckAbort() const
{
    return SocketImpl->CheckAbort();
}

// Shut down the socket
bool Socket::Shutdown()
{
#ifdef SF_ENABLE_THREADS
    SocketLocker locker(CreateLock);
#endif

    return SocketImpl->Shutdown();
}

// Initialize the socket library
// Heavily platform-specific
bool Socket::Startup()
{
#ifdef SF_ENABLE_THREADS
    SocketLocker locker(CreateLock);
#endif

    if (!SocketImpl->Startup())
    {
        return false;
    }

    return true;
}

// Terminate the socket library
// Heavily platform-specific
void Socket::Cleanup()
{
#ifdef SF_ENABLE_THREADS
    SocketLocker locker(CreateLock);
#endif

    SocketImpl->Cleanup();
}


//////////////////////////////////////////////////////////////////////////////////

BroadcastSocket::BroadcastSocket(bool initLib, SocketImplFactory* socketImplFactory) 
    : SocketFactory(socketImplFactory), InitLib(initLib)
{
    SocketImpl = SocketFactory->Create();

    if (InitLib)
    {
        if (!SocketImpl->Startup()) 
        {
            SF_DEBUG_MESSAGE(1, "Socket: Startup failed\n");
        }
    }
}

BroadcastSocket::~BroadcastSocket()
{
    Destroy();
    if (InitLib)
    {
        SocketImpl->Cleanup();
    }
    SocketFactory->Destroy(SocketImpl);
}

bool BroadcastSocket::Create(UInt32 port, bool broadcast)
{
    if (!SocketImpl->CreateDatagram(broadcast))
    {
        SF_DEBUG_MESSAGE1(1, "BroadcastSocket: Could not create socket. Error %d\n", 
            SocketImpl->GetLastError());
        return false;
    }

    if (broadcast)
    {
        SocketImpl->SetBroadcastPort(port);
        SocketImpl->SetBroadcast(true);
    }
    else
    {
        SocketImpl->SetListenPort(port);

        if (!SocketImpl->Bind())
        {
            SF_DEBUG_MESSAGE1(1, 
                "BroadcastSocket: Could not bind UDP socket. Error %d\n", SocketImpl->GetLastError());
            Destroy();
            return false;
        }

        SocketImpl->SetBlocking(false);
    }

    return true;
}

// Shut down the socket
void BroadcastSocket::Destroy()
{
    if (SocketImpl->IsValid())
    {
        SocketImpl->Shutdown();
    }
}


int BroadcastSocket::Broadcast(const char* dataBuffer, UPInt dataBufferSize) const
{
    int bytesTransferred = SocketImpl->SendBroadcast(dataBuffer, dataBufferSize);
    if (bytesTransferred < 0)
    {
        SF_DEBUG_MESSAGE1(true, "BroadcastSocket: Send Error %d\n", SocketImpl->GetLastError());
    }

    return bytesTransferred;
}

int BroadcastSocket::Receive(char* dataBuffer, int dataSize) const
{
    int bytesReceived = SocketImpl->ReceiveBroadcast(dataBuffer, dataSize);
    if (bytesReceived < 0)
    {
        return 0;
    }
    return bytesReceived;
}

void BroadcastSocket::GetName(UInt32* port, UInt32* address, char* name, UInt32 nameSize) const
{
    SocketImpl->GetName(port, address, name, nameSize);
}

} // namespace GFx
} // namespace Scaleform

#endif  // SF_ENABLE_SOCKETS

