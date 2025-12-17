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

#define WIIU_USE_HIO
#ifdef WIIU_USE_HIO
    #include <cafe.h>
    #include <cafe/hio.h>
    #include <cafe/os.h>
#else
    #include <cafe.h>
    #include <cafe/network.h>

    #define GINVALIDSOCKET(a)   ((a) < 0)
    #define GSOCKETERROR(a)     ((a) < 0)
    #define WIIU_Socket_Buffer_Size 512
#endif    

#include "Kernel/SF_Memory.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_MsgFormat.h"

namespace Scaleform {
namespace GFx {
namespace AMP {

#if defined(WIIU_USE_HIO)

class GFxSocketImpl  : public NewOverrideBase<Stat_Default_Mem>, public SocketInterface
{
public:
    GFxSocketImpl() : LastError(0), Handle(HIO_HANDLE_INVALID), ReadInProgress(false), ReadCompleted(false), ReadStatus(HIO_STATUS_OK)
    { 
        // This event will be signaled by the callback when a host application
        // has started listening to this HostIO channel.
        OSInitEvent(&Event, FALSE, OS_EVENT_MANUAL);
    }
    ~GFxSocketImpl()
    {
    }
    static void ReadCallback(HIOStatus status, void* context)
    {
        GFxSocketImpl* socketImpl = static_cast<GFxSocketImpl*>(context);

        Lock::Locker lock(&socketImpl->ReadStatusLock);
        socketImpl->ReadStatus = status;
        socketImpl->ReadCompleted = true;
        socketImpl->ReadInProgress = false;
    }
    bool CreateStream(bool listener) 
    { 
        return true; 
    }
    bool CreateDatagram(bool broadcast) 
    { 
        // Not supported for the Wii
        return false; 
    }
    bool Bind() 
    { 
        return true; 
    }
    bool Listen(int) 
    { 
        OSResetEvent(&Event);

        Handle = HIOOpen("AMP", ConnectionCallback,
            &Event, HIO_CHANNEL_OPTION_READ_WRITE, 0);

        // Check to see if HIOOpen failed and error out if needed.
        if (Handle < 0)
        {
            SF_DEBUG_MESSAGE1(1, "HIOOpen returned %d\n", Handle);
            LastError = Handle;
            return false;
        }

        return true;
    }
    bool Connect() 
    {
        return false;  // Server only
    }
    bool Accept(int timeout) 
    { 
        // Wait for the PC to start listening.
        if (timeout >= 0)
        {
            if (!OSWaitEventWithTimeout(&Event, timeout * 1000000000))
            {
                return false;
            }
        }
        else
        {
            OSWaitEvent(&Event);
        }
        return true;
    }
    int Send(const char* buffer, UPInt bufferSize) const 
    { 
        if (!IsValid())
        {
            return 0;
        }
        HIOStatus status = HIOWrite(Handle, bufferSize, (void*)buffer);
        if (status < 0)
        {
            LastError = status;
            return -1;
        }
        return status;
    }
    int Receive(char* buffer, int bufferSize) const 
    {
        if (!IsValid())
        {
            return 0;
        }

        Lock::Locker lock(&ReadStatusLock);
        if (ReadInProgress)
        {
            return 0;
        }

        if (ReadCompleted)
        {
            ReadCompleted = false;
            return ReadStatus;
        }

        ReadInProgress = true;
        HIOStatus status = HIOReadAsync(Handle, bufferSize, buffer, &GFxSocketImpl::ReadCallback, (void*)this);
        if (status < 0)
        {
            LastError = status;
            return -1;
        }
        return 0;
    }
    int SendBroadcast(const char*, UPInt) const { return -1; }
    int ReceiveBroadcast(char*, int) const { return -1; }
    void SetListenPort(UInt32) { }
    void SetBroadcastPort(UInt32) { }
    void SetAddress(UInt32, const char*) { }
    void SetBlocking(bool) { }
    void SetBroadcast(bool) { }
    void GetName(UInt32*, UInt32*, char*, UInt32) { }
    bool Shutdown() 
    { 
        if (Handle != HIO_STATUS_INVALID_HANDLE)
        {
            HIOClose(Handle);
            Handle = HIO_STATUS_INVALID_HANDLE;
        }
        return true; 
    }
    bool Startup() 
    { 
        return (HIOInit() == HIO_STATUS_OK);
    }
    void Cleanup() 
    { 
    }
    int GetLastError() const 
    { 
        return LastError; 
    }
    bool IsValid() const 
    { 
        return (Handle != HIO_STATUS_INVALID_HANDLE); 
    }
    bool IsListening() const
    {
        return false;
    }
    bool ShutdownListener() 
    { 
        return false; 
    }
    bool CheckAbort() const 
    { 
        return false;
    }
private:

    static void ConnectionCallback(HIOStatus status, void* context)
    {
        if (status == HIO_STATUS_NEW_CONNECTION)
        {
            OSSignalEvent((OSEvent*)context);
        }
        else
        {
            OSResetEvent((OSEvent*)context);
        }
    }

    OSEvent Event;
    HIOHandle Handle;
    mutable int LastError;
    mutable Lock ReadStatusLock;
    mutable bool ReadInProgress;
    mutable bool ReadCompleted;
    HIOStatus ReadStatus;
};

#else // !defined(WIIU_USE_HIO)

// Platform-specific socket implementation
class GFxSocketImpl : public NewOverrideBase<Stat_Default_Mem>, public SocketInterface
{
public:
    GFxSocketImpl() : Socket(SOCKET_ERROR), ListenSocket(SOCKET_ERROR), LocalHostAddress(0) 
    {
        SocketBuffer = (UByte*)SF_HEAP_MEMALIGN(Memory::GetHeapByAddress(this), WIIU_Socket_Buffer_Size, 64, Stat_Default_Mem);
    }

    ~GFxSocketImpl()
    {
        SF_FREE_ALIGN(SocketBuffer);
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
        return (!GINVALIDSOCKET(hSocket));
    }

    bool CreateDatagram(bool broadcast)
    {
        Socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (!broadcast)
        {
            ListenSocket = Socket;
        }
        return (!GINVALIDSOCKET(Socket));
    }

    bool Bind()
    {
        return (!GSOCKETERROR(::bind(ListenSocket, (sockaddr*) &SocketAddress, sizeof(SocketAddress))));
    }

    bool Listen(int numConnections)
    {
        return (!GSOCKETERROR(::listen(ListenSocket, numConnections)));
    }

    bool Connect()
    {
        return (!GSOCKETERROR(::connect(Socket, (sockaddr*) &SocketAddress, sizeof(SocketAddress))));
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

            if (::select((int)ListenSocket+1, &readfds, NULL, NULL, &tv) < 0)
            {
                SF_DEBUG_MESSAGE1(1, "Select: Error %d\n", GetLastError());
                return false;
            }
            if (!FD_ISSET(ListenSocket, &readfds))
            {
                return false;
            }
        }

        socklen_t iSize = sizeof(SocketAddress);
        int hSocket = ::accept(ListenSocket, (sockaddr*) &SocketAddress, &iSize);
        if (!GINVALIDSOCKET(hSocket))
        {
            Socket = hSocket;
            return true;
        }

        return false;
    }
    int Send(const char* dataBuffer, UPInt dataBufferSize) const
    {
        if (GINVALIDSOCKET(Socket))
        {
            return -1;
        }

        SF_ASSERT(dataBufferSize <= WIIU_Socket_Buffer_Size);
        memcpy(SocketBuffer, dataBuffer, dataBufferSize);
        int bytesSent = ::send(Socket, SocketBuffer, static_cast<int>(dataBufferSize), 0);
        if (GSOCKETERROR(bytesSent))
        {
            return (GetLastError() == SO_EWOULDBLOCK ? 0 : -1);
        }
        return bytesSent;
    }

    int Receive(char* dataBuffer, int dataBufferSize) const
    {
        if (GINVALIDSOCKET(Socket))
        {
            return -1;
        }

        SF_ASSERT(dataBufferSize <= WIIU_Socket_Buffer_Size);
        int bytesRecvd = ::recv(Socket, SocketBuffer, dataBufferSize, 0);
        if (bytesRecvd > 0)
        {   
            memcpy(dataBuffer, SocketBuffer, bytesRecvd);
        }
        return bytesRecvd;
    }

    int SendBroadcast(const char* dataBuffer, UPInt dataBufferSize) const
    {
        if (GINVALIDSOCKET(Socket))
        {
            return -1;
        }

        SF_ASSERT(dataBufferSize <= WIIU_Socket_Buffer_Size);
        memcpy(SocketBuffer, dataBuffer, dataBufferSize);
        return ::sendto(Socket, SocketBuffer, static_cast<int>(dataBufferSize), 0, 
            (const sockaddr*) &SocketAddress, sizeof(SocketAddress));
    }

    int ReceiveBroadcast(char* dataBuffer, int dataSize) const
    {
        if (GINVALIDSOCKET(Socket))
        {
            return -1;
        }

        socklen_t addrLength = sizeof(SocketAddress);
        SF_ASSERT(dataSize <= WIIU_Socket_Buffer_Size);
        int bytesRecvd = ::recvfrom(Socket, SocketBuffer, dataSize, 0, (sockaddr*)&SocketAddress, &addrLength); 
        if (bytesRecvd > 0)
        {   
            memcpy(dataBuffer, SocketBuffer, bytesRecvd);
        }
        return bytesRecvd;
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
        SocketAddress.sin_addr.s_addr = htonl(0xffffffff);
        SocketAddress.sin_port = htons((UInt16) port);
    }

    void SetAddress(UInt32 port, const char* address)
    {
        memset(&SocketAddress, 0, sizeof(sockaddr_in));
        SocketAddress.sin_family = AF_INET;
        SocketAddress.sin_port = htons((UInt16) port);
        inet_aton(address, &SocketAddress.sin_addr);
    }

    // Blocking means that some socket operations block the thread until completed
    // Non-blocking do not block, returning SO_EWOULDBLOCK instead
    void SetBlocking(bool blocking)
    {
        if ( GINVALIDSOCKET(Socket) )
            return;

        int nValue = blocking ? 0 : 1;
        setsockopt(Socket, SOL_SOCKET, SO_NBIO, &nValue, sizeof(nValue));
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
        if (!GINVALIDSOCKET(Socket))
        {
            ::socketclose(Socket);
            Socket = SOCKET_ERROR;
        }
        return true;
    }

    // Initialize the socket library
    bool Startup()
    {
        Lock::Locker locker(&LibRefLock);
        if (LibRefs == 0)
        {
            NETCONF_ERROR rval = NCInit();
            if (rval != NETCONF_ERROR_OK)
            {
                if (rval == NETCONF_ERROR_RETRY)
                {
                    SF_DEBUG_MESSAGE(1, "NCInit: Sleeping 3 seconds before retry\n");
                    OSSleepMilliseconds(3000);
                    rval = NCInit();
                }
                if (rval != NETCONF_ERROR_OK)
                {
                    /* Failed to initialize NC library. */
                    SF_DEBUG_MESSAGE1(1, "NCInit failed: (%d)\n", rval);
                    return false;
                }
            }

            netconf_profile_t runningProfile;
            memset(&runningProfile, 0, sizeof(netconf_profile_t));
            runningProfile.eth0.if_index = NETCONF_NIC_ETH;
            runningProfile.eth0.if_state = NETCONF_ADMIN_STATE_UP;
            runningProfile.eth0.ipv4_info.mode = NETCONF_ADDR_DHCP;

            runningProfile.eth_cfg.negotiation = NETCONF_ETH_AUTO_NEGOTIATION_OFF;
            runningProfile.eth_cfg.speed = NETCONF_ETH_LINK_SPEED_100M;
            runningProfile.eth_cfg.duplex = NETCONF_ETH_LINK_FULL_DUPLEX;

            runningProfile.wl0.if_state = NETCONF_ADMIN_STATE_UNUSED;

            // Disable previous network if it's available.
            if (NCGetAdminState(NETCONF_NIC_ETH) != NETCONF_ADMIN_STATE_DOWN)
            {
                NCSetAdminState(NETCONF_NIC_ETH, NETCONF_ADMIN_STATE_DOWN);
            }

            rval = NCSetRunningConfig(&runningProfile);
            if (rval != NETCONF_ERROR_OK)
            {
                SF_DEBUG_MESSAGE1(1, "NCSetRunningConfig failed: (%d)\n", rval);
                return false;
            }

            // Wait until ethernet network interface comes to operational state
            SF_DEBUG_MESSAGE(1, "Waiting for ethernet network\n");
            bool netSuccess = false;
            for (int i = 0; i < 1; ++i)
            {
                if (NCGetOperState(NETCONF_NIC_ETH) == NETCONF_OPER_STATE_UP)
                {
                    netSuccess = true;
                    break;
                }
                OSSleepMilliseconds(1000);
                SF_DEBUG_MESSAGE(1, ".");
            }
            if (netSuccess)
            {
                SF_DEBUG_MESSAGE(1, "done\n");
            }
            else
            {
                SF_DEBUG_MESSAGE(1, "\nTimeout while waiting for ethernet to configure\n");
                return false;
            }

            // Wait for DHCP to assign an IP address
            OSSleepMilliseconds(3000);

            if (SOInit() != 0)
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
                SOFinish();
                NCClose();
            }
        }
    }

    // Get error code
    int GetLastError() const
    {
        return SOLastError();
    }

    bool IsValid() const 
    { 
        return (!GINVALIDSOCKET(Socket)); 
    }

    bool IsListening() const
    {
        return (!GINVALIDSOCKET(ListenSocket)); 
    }

    bool ShutdownListener()
    {
        if (IsListening())
        {
            ::socketclose(ListenSocket);
            ListenSocket = SOCKET_ERROR;
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
    UByte*                      SocketBuffer;

    mutable Hash<UInt32, String>        AddressMap;
    mutable UInt32                      LocalHostAddress;                     

    static UInt32               LibRefs;
    static Lock                 LibRefLock;
};

UInt32 GFxSocketImpl::LibRefs = 0;
Lock GFxSocketImpl::LibRefLock;

#endif // WIIU_USE_HIO

static DefaultSocketFactory<GFxSocketImpl> defaultSocketFactory;
SocketImplFactory* GlobalDefaultSocketFactory = &defaultSocketFactory;

} // namespace AMP
} // namespace GFx
} // namespace Scaleform

#endif  // SF_ENABLE_SOCKETS
