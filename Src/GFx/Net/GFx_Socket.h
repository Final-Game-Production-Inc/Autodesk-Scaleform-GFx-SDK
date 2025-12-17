/**************************************************************************

Filename    :   GFx_Socket.h
Content     :   Socket wrapper class

Created     :   July 03, 2009
Authors     :   Boris Rayskiy, Alex Mantzaris

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GFX_SOCKET_H
#define INC_GFX_SOCKET_H

#include "GFxConfig.h"

#ifdef SF_ENABLE_SOCKETS

#include "Kernel/SF_Memory.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform {
class String;
class Lock;
namespace GFx {

class SocketInterface
{
public:
    virtual ~SocketInterface() { }
    virtual bool CreateStream(bool listener) = 0;
    virtual bool CreateDatagram(bool broadcast) = 0;
    virtual bool Bind() = 0;
    virtual bool Listen(int) = 0;
    virtual bool Connect() = 0;
    virtual bool Accept(int timeout) = 0;
    virtual int Send(const char*, UPInt) const = 0;
    virtual int Receive(char*, int) const = 0;
    virtual int SendBroadcast(const char*, UPInt) const = 0;
    virtual int ReceiveBroadcast(char*, int) const = 0;
    virtual void SetListenPort(UInt32) = 0;
    virtual void SetBroadcastPort(UInt32) = 0;
    virtual void SetAddress(UInt32, const char*) = 0;
    virtual void SetBlocking(bool) = 0;
    virtual void SetBroadcast(bool) = 0;
    virtual void GetName(UInt32*, UInt32*, char*, UInt32) = 0;
    virtual bool Shutdown() = 0;
    virtual bool Startup() = 0;
    virtual void Cleanup() = 0;
    virtual int GetLastError() const = 0;
    virtual bool IsValid() const = 0;
    virtual bool IsListening() const = 0;
    virtual bool ShutdownListener() = 0;
    virtual bool CheckAbort() const = 0;
};

class SocketImplFactory
{
public:
    virtual ~SocketImplFactory() { }
    virtual SocketInterface* Create() = 0;
    virtual void Destroy(SocketInterface* socketImpl) = 0;
};

template<class C>
class DefaultSocketFactory : public NewOverrideBase<Stat_Default_Mem>, public SocketImplFactory
{
public:
    virtual ~DefaultSocketFactory() { }
    virtual SocketInterface* Create()
    {
        return SF_NEW C();
    }
    virtual void Destroy(SocketInterface* socketImpl)
    {
        delete socketImpl;
    }
};


class Socket
{
public:
    Socket(bool initLib, SocketImplFactory* socketImplFactory);
    ~Socket();

    // Create a socket, either to a specific address and port (client), or a listener (server)
    bool    CreateClient(const char* ipAddress, UInt32 port, String* errorMsg);
    bool    CreateServer(UInt32 port, String* errorMsg);
    // Shuts down and performs cleanup
    void    Destroy();
    // Wait until an incoming connection is accepted (only relevant for server sockets)
    bool    Accept(int timeout);
    // Send a packet over the network
    int     Send(const char* dataBuffer, UPInt dataBufferSize) const;
    // Receive a packet over the network (blocks)
    int     Receive(char* dataBuffer, int dataSize) const;
    // Test the outgoing connection
    bool    IsConnected() const;
    // Set blocking
    void    SetBlocking(bool blocking);
    // Set a lock for multithreaded access
    void    SetLock(Lock* lock);
    // Is the socket valid?
    bool IsValid() const;
    // Check whether a reconnection should happen
    bool    CheckAbort() const;

    static SocketImplFactory* GetAs3DefaultFactory();
    static SocketImplFactory* GetAmpDefaultFactory();

private:
    enum  { SocketListenBacklog = 10 };  // Number of simultaneous connections (server only)

    SocketImplFactory*  SocketFactory;
    SocketInterface*    SocketImpl;
    bool                IsServer;
    bool                InitLib;

#ifdef SF_ENABLE_THREADS
    Lock*               CreateLock;
#endif

    // Private helper functions
    bool    Shutdown();
    bool    Startup();
    void    Cleanup();
};

class BroadcastSocket
{
public:
    BroadcastSocket(bool initLib, SocketImplFactory* socketImplFactory);
    ~BroadcastSocket();
    bool    Create(UInt32 port, bool broadcast);
    void    Destroy();
    int     Broadcast(const char* dataBuffer, UPInt dataBufferSize) const;
    int     Receive(char* dataBuffer, int dataSize) const;
    void    GetName(UInt32* port, UInt32* address, char* name, UInt32 nameSize) const;
    int     GetLastError() const { return SocketImpl->GetLastError(); }

private:
    SocketImplFactory*  SocketFactory;
    SocketInterface*    SocketImpl;
    bool                InitLib;
};

} // namespace GFx
} // namespace Scaleform

#endif  // SF_ENABLE_SOCKETS

#endif // INC_GFX_SOCKET_H
