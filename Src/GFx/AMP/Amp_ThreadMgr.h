/**************************************************************************

Filename    :   Amp_ThreadMgr.h
Content     :   Manages the socket threads
Created     :   December 2009
Authors     :   Alex Mantzaris

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/


#ifndef INC_GFX_AMP_THREAD_MGR_H
#define INC_GFX_AMP_THREAD_MGR_H

#include "GFxConfig.h"
#if defined(SF_AMP_SERVER) || defined(SF_AMP_CLIENT)

#include "GFx/Net/GFx_Socket.h"
#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_String.h"
#include "Amp_Interfaces.h"
#include "GFx/GFx_Log.h"

namespace Scaleform {
    // Forward declarations
    class String;
    class Lock;
    class Event;
namespace GFx {
namespace AMP {

    class MessageTypeRegistry;

// Manages the socket connection threads for AMP
// An instance of this class is contained both in the AMP server singleton and in the AMP client
// The caller can pass in callback objects for notification
// The SendInterface::OnSendLoop gets called from the socket thread for app-specific processing
// The ConnStatusInterface::OnStatusChanged gets called whenever the connection status is updated
// A MsgHandler can be specified for immediate message handling, bypassing the received queue (for AS debugger)
class ThreadMgr : public RefCountBase<ThreadMgr, StatAmp_Server>
{
public:
    ThreadMgr(bool initSocketLib, SendInterface* sendCallback, 
              ConnStatusInterface* connectionCallback, Scaleform::Event* sendQueueWaitEvent,
              Scaleform::Event* rcvQueueWaitEvent, SocketImplFactory* socketFactory,
              MessageTypeRegistry* msgTypeRegistry);
    virtual ~ThreadMgr();

    // Initialize AMP - Starts the threads for sending and receiving messages
    bool                InitAmp(const char* ipAddress, UInt32 port, UInt32 broadcastPort, Message* initMsg);
    // Uninitialize AMP - Performs thread cleanup
    void                UninitAmp();
    // Has AMP been initialized?
    bool                IsRunning() const;
    // Net socket implementation override - call before starting the threads
    void                SetSocketImplFactory(SocketImplFactory* socketFactory);


    // Initialize the broadcast receive thread
    void                StartBroadcastRecv(UInt32 port);
    // Set the connected app
    void                SetBroadcastInfo(const char* connectedApp, const char* connectedFile);
    // Heartbeat interval
    void                SetHeartbeatInterval(UInt32 heartbeatInterval);
    // Zlib compression for network messages
    void                SetCompressedMessages(bool on);
    bool                IsCompressedMessages() const;

    // Accessors
    const StringLH&                         GetAddress() const  { return IpAddress; }
    UInt32                                  GetPort() const     { return Port; }
    ConnStatusInterface::StatusType         GetConnectionStatus() const;
    UInt32                                  GetReceivedQueueSize() const { return MsgReceivedQueue.GetSize(); }
    UInt32                                  GetSendQueueSize() const { return MsgSendQueue.GetSize(); }
    UInt32                                  GetSendRate() const { return SendRate; }
    UInt32                                  GetReceiveRate() const { return ReceiveRate; }
    UInt32                                  GetMsgVersion() const { return MsgVersion; }

    // Has a socket been successfully created? Thread-safe
    bool                IsValidSocket();
    // Has a message been received from the other end recently? Thread-safe
    bool                IsValidConnection();
    // Place a message on the queue to be sent.  Thread-safe
    void                SendAmpMessage(Message* msg);
    // Retrieve the next received message from the queue.  Thread-safe
    Ptr<Message>        GetNextReceivedMessage();
    // Clear all messages of the specified type from the received queue
    void                ClearReceivedMessages(const Message* msg);
    // Handle the next received message and remove it from the queue
    // Returns true if there was a message and it was handled
    bool                HandleNextReceivedMessage();

private:
    
    enum 
    {
        // maximum packet size - larger messages are broken up
        BufferSize = 512,  

        // If no message has been sent for this long, we send a heartbeat message
#ifdef SF_OS_WII
        DefaultHeartbeatIntervalMillisecs = 0, // 0 means no heartbeat
#else
        DefaultHeartbeatIntervalMillisecs = 1000,
#endif

        // server socket blocks and waits for connection 
        // for this amount of time before timing out and retrying
        SocketAcceptTimeoutSeconds = 1,   
    };

    // MsgQueue encapsulates a message queue
    // It is a thread-safe list that keeps track of its size
    class MsgQueue
    {
    public:
        MsgQueue(Scaleform::Event* sizeEvent, UInt32 sizeCheckHysterisisPercent, UInt32 maxSize);
        void            PushBack(Message* msg);
        Message*        PopFront();
        void            Clear();
        void            ClearMsgType(const Message* msg);
        UInt32          GetSize() const;

    private:

        Lock                    QueueLock;
        List<Message>           Queue;
        AtomicInt<UInt32>       QueueSize;
        UInt32                  MaxSize;
        Scaleform::Event*       SizeEvent;
        UInt32                  SizeCheckHysterisisPercent;

        void                    CheckSize(MemoryHeap* heap);
    };


    Ptr<Thread>         SocketThread;  // Takes messages from the send queue and sends them
    Ptr<Thread>         BroadcastThread;  // Broadcasts listener socket IP address and port 
    Ptr<Thread>         BroadcastRecvThread;  // Listens for broadcast messages 
    Ptr<Thread>         CompressThread;   // Uncompresses/compresses received/sending messages
    UInt32              Port;
    Lock                BroadcastInfoLock;
    UInt32              BroadcastPort;
    StringLH            BroadcastApp;
    StringLH            BroadcastFile;
    UInt32              BroadcastRecvPort;
    StringLH            IpAddress;
    bool                Server;  // Server or Client?
    Lock                SocketLock;
    Socket              Sock;
    UInt32              HeartbeatIntervalMillisecs;
    AtomicInt<int>      CompressMessages;
    bool                InitSocketLib;

    // Status is checked from send and received threads
    mutable Lock                        InitLock;
    mutable Lock                        StatusLock;
    bool                                Exiting;
    UInt64                              LastSendHeartbeat;
    UInt64                              LastRcvdHeartbeat;
    ConnStatusInterface::StatusType     ConnectionStatus;
    AtomicInt<UInt32>                   SendRate;
    AtomicInt<UInt32>                   ReceiveRate;
    mutable AtomicInt<UInt32>           ValidConnection;

    // Message queues
    MsgQueue                            MsgReceivedQueue;
    MsgQueue                            MsgSendQueue;
    MsgQueue                            MsgUncompressedQueue;
    MsgQueue                            MsgCompressedQueue;
    Scaleform::Event*                   SendQueueWaitEvent;
    Scaleform::Event*                   RcvQueueWaitEvent;
    AtomicInt<UInt32>                   MsgVersion;
    UByte                               LastGFxVersion;

    // Callback interfaces
    SendInterface*                      SendCallback;         // send thread callback
    ConnStatusInterface*                ConnectionChangedCallback; // connection changed callback
    SocketImplFactory*                  SocketFactory;

    Ptr<MessageTypeRegistry>            MsgTypeRegistry;                

    // Thread functions
    static int          SocketThreadLoop(Thread* sendThread, void* param);
    static int          BroadcastThreadLoop(Thread* broadcastThread, void* param);
    static int          BroadcastRecvThreadLoop(Thread* broadcastThread, void* param);
    static int          CompressThreadLoop(Thread* uncompressThread, void* param);

    // Private helper functions
    bool                IsServer() const;
    bool                IsExiting() const;
    void                SetExiting();
    void                UpdateLastReceivedTime();
    bool                SocketConnect(String* errorMsg = NULL);
    bool                SocketAccept(int timeoutSeconds);
    Message*            RetrieveMessageForSending();
    bool                SendReceiveLoop();
    bool                BroadcastLoop();
    bool                BroadcastRecvLoop();
    bool                CompressLoop();
    void                UpdateStatus(ConnStatusInterface::StatusType eStatus, const char* msg);
    Message*            CreateAndReadMessage(File& str); // reads a stream and creates a message of the appropriate type
    void                UpdateValidConnection();
};

} // namespace AMP
} // namespace GFx
} // namespace Scaleform

#endif  // SF_ENABLE_SOCKETS

#endif // INC_GFX_AMP_THREAD_MGR_H
