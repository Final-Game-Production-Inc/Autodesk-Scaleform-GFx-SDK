/**************************************************************************

Filename    :   Amp_ThreadMgr.cpp
Content     :   Manages the socket threads
Created     :   December 2009
Authors     :   Alex Mantzaris

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Amp_ThreadMgr.h"

#ifdef SF_AMP_SERVER

#include "Amp_Stream.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_MsgFormat.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_Log.h"
#include "Amp_MessageRegistry.h"
#include "Amp_Message.h"

#define MICROSECONDS_PER_SECOND 1000000

// #ifdef SF_OS_WIIU
//     #define SF_AMP_DEBUG_MSG_LOG
// #endif

namespace Scaleform {
namespace GFx {
namespace AMP {


// Static 
// Send thread function
int ThreadMgr::SocketThreadLoop(Thread* sendThread, void* param)
{
    SF_UNUSED(sendThread);

    ThreadMgr* threadMgr = static_cast<ThreadMgr*>(param);
    if (threadMgr == NULL)
    {
        return 1;
    }

    while (threadMgr->SendReceiveLoop())
    {
        // SendLoop exited but we are not shutting down
        // re-try to establish connection

        Thread::Sleep(1);  // seconds
    }

    SF_DEBUG_MESSAGE(1, "AMP Socket thread exited\n");

    return 0;
}

int ThreadMgr::BroadcastThreadLoop(Thread*, void* param)
{
    ThreadMgr* threadMgr = static_cast<ThreadMgr*>(param);
    if (threadMgr == NULL)
    {
        return 1;
    }

    threadMgr->BroadcastLoop();
    SF_DEBUG_MESSAGE(1, "AMP broadcast thread exited\n");

    return 0;
}

int ThreadMgr::BroadcastRecvThreadLoop(Thread*, void* param)
{
    ThreadMgr* threadMgr = static_cast<ThreadMgr*>(param);
    if (threadMgr == NULL)
    {
        return 1;
    }

    threadMgr->BroadcastRecvLoop();
    SF_DEBUG_MESSAGE(1, "AMP broadcast receiving thread exited\n");

    return 0;
}

int ThreadMgr::CompressThreadLoop(Thread*, void* param)
{
    ThreadMgr* threadMgr = static_cast<ThreadMgr*>(param);
    if (threadMgr == NULL)
    {
        return 1;
    }

    threadMgr->CompressLoop();
    SF_DEBUG_MESSAGE(1, "AMP message compression thread exited\n");

    return 0;
}


// Constructor
ThreadMgr::ThreadMgr(bool initSocketLib, 
                     SendInterface* sendCallback, 
                     ConnStatusInterface* connectionCallback,
                     Scaleform::Event* sendQueueWaitEvent,
                     Scaleform::Event* rcvQueueWaitEvent,
                     SocketImplFactory* socketFactory,
                     MessageTypeRegistry* msgTypeRegistry) : 
    SocketThread(NULL),
    BroadcastThread(NULL),
    BroadcastRecvThread(NULL),
    CompressThread(NULL),
    Port(0),
    BroadcastPort(0),
    BroadcastRecvPort(0),
    Server(true),
    Sock(initSocketLib, socketFactory),
    HeartbeatIntervalMillisecs(DefaultHeartbeatIntervalMillisecs),
    CompressMessages(true),
    InitSocketLib(initSocketLib),
    Exiting(false),
    LastSendHeartbeat(0),
    LastRcvdHeartbeat(0),
    ConnectionStatus(ConnStatusInterface::CS_Idle),
    SendRate(0),
    ReceiveRate(0),
    ValidConnection(0),
    MsgReceivedQueue(rcvQueueWaitEvent, 90, 100),
    MsgSendQueue(sendQueueWaitEvent, 90, 100),
    MsgUncompressedQueue(rcvQueueWaitEvent, 90, 100),
    MsgCompressedQueue(sendQueueWaitEvent, 90, 100),
    SendQueueWaitEvent(sendQueueWaitEvent),
    RcvQueueWaitEvent(rcvQueueWaitEvent),
    MsgVersion(Message::GetLatestVersion()),
    LastGFxVersion(0),
    SendCallback(sendCallback),
    ConnectionChangedCallback(connectionCallback),
    SocketFactory(socketFactory)
{
    Sock.SetLock(&SocketLock);

    MsgTypeRegistry = *SF_HEAP_AUTO_NEW(this) MessageTypeRegistry();
    MsgTypeRegistry->AddMessageType<MessageHeartbeat>(NULL);
    MsgTypeRegistry->AddMessageType<MessageCompressed>(NULL);
    MsgTypeRegistry->AddMessageType<MessagePort>(NULL);
    MsgTypeRegistry->AddMessageType<MessageLog>(NULL);
    if (msgTypeRegistry != NULL)
    {
        MsgTypeRegistry->AddMessageTypeRegistry(*msgTypeRegistry);
    }
#ifdef SF_AMP_DEBUG_MSG_LOG
    SF_DEBUG_MESSAGE(true, "AMP Thread manager created\n");
#endif
}

// Destructor
ThreadMgr::~ThreadMgr()
{
    UninitAmp();
#ifdef SF_AMP_DEBUG_MSG_LOG
    SF_DEBUG_MESSAGE(true, "AMP Thread manager destroyed\n");
#endif
}

// Initialization
// Returns true if successful
// A NULL IP address means that we are going to be a server
bool ThreadMgr::InitAmp(const char* address, UInt32 port, UInt32 broadcastPort, Message* initMsg)
{
    if (initMsg != NULL)
    {
        MsgSendQueue.PushBack(initMsg);
    }
    else
    {
        MsgSendQueue.PushBack(SF_HEAP_AUTO_NEW(this) GFx::AMP::MessageHeartbeat());
    }

    Lock::Locker locker(&InitLock);

    // Have we already been initialized?
    if (IsRunning())
    {
        if ((IsServer() ? address == NULL : IpAddress == address) && port == Port)
        {
            // Same connection information - done
            return true;
        }
        else
        {
            // Different connection information - start over
            UninitAmp();
        }
    }

    Exiting = false;
    Port = port;
    BroadcastPort = broadcastPort;
    Server = (address == NULL);
    if (!IsServer())
    {
        IpAddress = address;
    }

    // Start the send thread
    if (Port != 0)
    {
        SocketThread = *SF_HEAP_AUTO_NEW(this) Thread(ThreadMgr::SocketThreadLoop, (void*) this);
        if (!SocketThread || !SocketThread->Start())
        {
            return false;
        }
        SocketThread->SetThreadName("Scaleform AMP Socket");
    }

    StartBroadcastRecv(BroadcastRecvPort);

#ifdef SF_AMP_DEBUG_MSG_LOG
    SF_DEBUG_MESSAGE(true, "AMP Thread manager initialized\n");
#endif

    return true;
}

// Cleanup
void ThreadMgr::UninitAmp()
{
    Lock::Locker locker(&InitLock);

    SetExiting();

    // wait until threads are done
    if (BroadcastThread)
    {
        BroadcastThread->Wait();
        BroadcastThread = NULL;
    }

    if (BroadcastRecvThread)
    {
        BroadcastRecvThread->Wait(); 
        BroadcastRecvThread = NULL;
    }

    if (CompressThread)
    {
        CompressThread->Wait();
        CompressThread = NULL;
    }

    if (SocketThread)
    {
        SocketThread->Wait();
        SocketThread = NULL;
    }

    MsgSendQueue.Clear();
    MsgReceivedQueue.Clear();
    MsgUncompressedQueue.Clear();
    MsgCompressedQueue.Clear();

    SendRate = 0;
    ReceiveRate = 0;

    UpdateStatus(ConnStatusInterface::CS_Idle, "Disconnected");

#ifdef SF_AMP_DEBUG_MSG_LOG
    SF_DEBUG_MESSAGE(true, "AMP Thread manager uninitialized\n");
#endif
}

// Initialize the broadcast thread
void ThreadMgr::StartBroadcastRecv(UInt32 port)
{
    BroadcastRecvPort = port;
    if (BroadcastRecvPort != 0 && !BroadcastRecvThread)
    {
        BroadcastRecvThread = *SF_HEAP_AUTO_NEW(this) Thread(
                                        ThreadMgr::BroadcastRecvThreadLoop, (void*) this);
        if (BroadcastRecvThread)
        {
            if (BroadcastRecvThread->Start())
            {
                BroadcastRecvThread->SetThreadName("Scaleform AMP Broadcast/Receive");
            }
        }
    }
}

void ThreadMgr::SetBroadcastInfo(const char* connectedApp, const char* connectedFile)
{
    Lock::Locker locker(&BroadcastInfoLock);
    BroadcastApp = connectedApp;
    BroadcastFile = connectedFile;
}

void ThreadMgr::SetHeartbeatInterval(UInt32 heartbeatInterval)
{
    HeartbeatIntervalMillisecs = heartbeatInterval;
}

void ThreadMgr::SetCompressedMessages(bool on)
{
    CompressMessages = on;
}

bool ThreadMgr::IsCompressedMessages() const
{
    return CompressMessages != 0;
}

// Returns true when there is a thread still running
bool ThreadMgr::IsRunning() const
{
    if (SocketThread && !SocketThread->IsFinished())
    {
        return true;
    }

    return false;
}

void ThreadMgr::SetSocketImplFactory(SocketImplFactory* socketFactory)
{
    SocketFactory = socketFactory;
}


// Thread-safe status accessor
bool ThreadMgr::IsExiting() const
{
    Lock::Locker locker(&StatusLock);
    return Exiting;
}

// Signals the threads that we are shutting down
void ThreadMgr::SetExiting()
{
    Lock::Locker locker(&StatusLock);
    Exiting = true;
}

// Keep track of last message received for connection state
void ThreadMgr::UpdateLastReceivedTime()
{
    LastRcvdHeartbeat = Timer::GetTicks();
    ValidConnection = 1;
}

// Create the socket
// Returns true on success
bool ThreadMgr::SocketConnect(String* errorMsg)
{
    UpdateStatus(ConnStatusInterface::CS_Connecting, "");

    {
        Lock::Locker kLocker(&StatusLock);

        // Check for exit
        // This needs to be inside SocketConnect for thread safety
        // Otherwise we could exit and destroy the socket after entering this function
        if (IsExiting())
        {
            Sock.Destroy();
            return false;
        }

        if (IsServer())
        {
            if (!Sock.CreateServer(Port, errorMsg))
            {
                SetExiting();  // Something is terribly wrong, don't keep trying
                Sock.Destroy();
                return false;
            }
        }
        else
        {
            Sock.Destroy();
            if (!Sock.CreateClient(IpAddress.ToCStr(), Port, errorMsg))
            {
                return false;
            }
        }
    }

    if (BroadcastPort != 0)
    {
        if (!BroadcastThread)
        {
            // Start the broadcast thread
            BroadcastThread = *SF_HEAP_AUTO_NEW(this) Thread(ThreadMgr::BroadcastThreadLoop, (void*) this);
            if (BroadcastThread)
            {
                if (BroadcastThread->Start())
                {
                    BroadcastThread->SetThreadName("Scaleform AMP Broadcast Thread");
                }
            }
        }
    }

    return true;
}

bool ThreadMgr::SocketAccept(int timeoutSeconds)
{
//     Lock::Locker kLocker(&StatusLock);
// 
//     // Check for exit
//     // This needs to be inside SocketConnect for thread safety
//     // Otherwise we could exit and destroy the socket after entering this function
//     if (IsExiting())
//     {
//         Sock.Destroy();
//         return false;
//     }

    return Sock.Accept(timeoutSeconds);
}


ConnStatusInterface::StatusType ThreadMgr::GetConnectionStatus() const
{
    Lock::Locker locker(&StatusLock);
    return ConnectionStatus;
}

bool ThreadMgr::IsValidSocket()
{
    Lock::Locker locker(&StatusLock);
    return Sock.IsValid();
}

// Socket status
// Notifies connection interface on loss of connection
// Sends a log message on connection
// We are considered connected if we have received a message 
// in the last 2 * HeartbeatIntervalMillisecs
bool ThreadMgr::IsValidConnection()
{
    return ValidConnection != 0;
}

// For different behavior between server and client sockets
bool ThreadMgr::IsServer() const
{
    return Server;
}

// Reads a message from the queue
// Returns true if a message was retrieved
Message* ThreadMgr::RetrieveMessageForSending()
{
    UInt64 ticks = Timer::GetTicks();

    // Recover the next message, if any
    Message* msg = MsgCompressedQueue.PopFront();

    if (msg == NULL)
    {
        // Nothing to send. Should we send a heartbeat?
        UInt64 deltaTicks = ticks - LastSendHeartbeat;
        if (HeartbeatIntervalMillisecs > 0 && deltaTicks >  HeartbeatIntervalMillisecs * 1000)
        {
            msg = MsgTypeRegistry->CreateMessage<MessageHeartbeat>();
        }
    }

    if (msg != NULL)
    {
        LastSendHeartbeat = ticks;  // update the last message send time
        msg->SetVersion(MsgVersion);
    }

    return msg;
}

// Socket thread loop
// Returns true if exited with a broken connection
// Returns false if shutting down
bool ThreadMgr::SendReceiveLoop()
{
    if (SendQueueWaitEvent != NULL)
    {
        SendQueueWaitEvent->SetEvent();
    }
    if (RcvQueueWaitEvent != NULL)
    {
        RcvQueueWaitEvent->SetEvent();
    }

    // Create a socket
    while (SocketConnect())
    {
#ifdef SF_AMP_DEBUG_MSG_LOG
        SF_DEBUG_MESSAGE(1, "AMP: Listening for incoming connection\n");
#endif
        // wait until a connection has been established with the client
        if (SocketAccept(SocketAcceptTimeoutSeconds))
        {
            Sock.SetBlocking(false);

            // Send a heartbeat message first for version checking
            UpdateLastReceivedTime();  // so we can send the message below
            MsgVersion = Message::GetLatestVersion();
            SendAmpMessage(SF_HEAP_AUTO_NEW(this) MessageHeartbeat());

            Ptr<AmpStream> strmReceived = *SF_HEAP_AUTO_NEW(this) AmpStream();
            char bufferReceived[BufferSize];

            // Start the thread that uncompresses the received messages
            if (!CompressThread)
            {
                CompressThread = *SF_HEAP_AUTO_NEW(this) Thread(ThreadMgr::CompressThreadLoop, (void*) this);
                if (CompressThread->Start())
                {
                    CompressThread->SetThreadName("Scaleform AMP Compress");
                    SF_DEBUG_MESSAGE(1, "AMP: Message compression thread started\n");
                }
            }

            UInt64 lastSampleTime = Timer::GetProfileTicks();
            UInt32 bytesSent = 0;
            UInt32 bytesReceived = 0;
            Ptr<AmpStream> streamSend = *SF_HEAP_AUTO_NEW(this) AmpStream();
            UPInt streamSendDataLeft = streamSend->GetBufferSize();
            const char* sendBuffer = NULL;

            // Keep sending messages from the send queue
            while (!IsExiting() && !Sock.CheckAbort())
            {
                bool actionPerformed = false;

                // Check for a callback
                if (SendCallback != NULL)
                {
                    if (SendCallback->OnSendLoop())
                    {
                        actionPerformed = true;
                    }
                }

                if (streamSendDataLeft == 0)
                {
                    // Retrieve the next message from the send queue
                    Ptr<Message> msg = *RetrieveMessageForSending();
                    if (msg)
                    {
                        streamSend = *SF_HEAP_AUTO_NEW(this) AmpStream();
                        msg->Write(*streamSend);
                        streamSendDataLeft = streamSend->GetBufferSize();
                        sendBuffer = reinterpret_cast<const char*>(streamSend->GetBuffer());
                    }
                }

                if (streamSendDataLeft > 0)
                {
                    UPInt nextSendSize;
                    if (streamSendDataLeft <= BufferSize)
                    {
                        // The message fits in one packet
                        nextSendSize = streamSendDataLeft;
                    }
                    else
                    {
                        // The message does not fit. 
                        // Send the first BUFFER_SIZE bytes in this packet, 
                        // and the rest in the next one
                        nextSendSize = BufferSize;
                    }

                    // Send the packet
                    int sentBytes = Sock.Send(sendBuffer, nextSendSize);
                    if (sentBytes > 0)
                    {
#ifdef SF_AMP_DEBUG_MSG_LOG
                        SF_DEBUG_MESSAGE2(true, "Sent %d/%d\n", sentBytes, streamSendDataLeft);
#endif
                        bytesSent += sentBytes;
                        sendBuffer += sentBytes;
                        streamSendDataLeft -= sentBytes;
                        actionPerformed = true;
                    }
                    else if (sentBytes < 0)
                    {
                        SF_DEBUG_MESSAGE(true, "Failed to send data. Retrying...\n");
                        // Failed to send. Retry
                    }
                }

                // Check for incoming messages and place on the received queue
                int packetSize = Sock.Receive(bufferReceived, BufferSize);
                if (packetSize > 0)
                {
                    bytesReceived += packetSize;
                    actionPerformed = true;

                    // add packet to previously-received incomplete data
                    strmReceived->Append(reinterpret_cast<UByte*>(bufferReceived), packetSize);

#ifdef SF_AMP_DEBUG_MSG_LOG
                    SF_DEBUG_MESSAGE3(true, "Received %u/%u bytes (%u)\n", packetSize, strmReceived->FirstMessageSize(), strmReceived->GetBufferSize());
#endif
                    UpdateLastReceivedTime();
                }

                UPInt readBufferSize = strmReceived->GetBufferSize();
                if (readBufferSize > 0 && readBufferSize >= strmReceived->FirstMessageSize())
                {
                    // Read the stream and create a message
                    Message* msg = CreateAndReadMessage(*strmReceived);

                    // Update the buffer to take into account what we already read
                    strmReceived->PopFirstMessage();

                    if (msg != NULL)
                    {
                        MsgReceivedQueue.PushBack(msg);
                    }
                    else
                    {
                        SF_DEBUG_MESSAGE(1, "Corrupt message received\n");
                    }
                }

                UpdateValidConnection();
                if (!IsValidConnection())
                {
                    MsgSendQueue.Clear();
                    break;
                }

                if (!actionPerformed)
                {
                    Thread::MSleep(10);  // Don't hog the CPU
                }

                // Update the byte rates
                UInt64 nextTicks = Timer::GetProfileTicks();
                UInt32 deltaTicks = static_cast<UInt32>(nextTicks - lastSampleTime);
                if (deltaTicks > MICROSECONDS_PER_SECOND) // one second
                {
                    SendRate = bytesSent * MICROSECONDS_PER_SECOND / deltaTicks;
                    ReceiveRate = bytesReceived * MICROSECONDS_PER_SECOND / deltaTicks;
                    lastSampleTime = nextTicks;
                    bytesSent = 0;
                    bytesReceived = 0;
#ifdef SF_AMP_DEBUG_MSG_LOG
                    SF_DEBUG_MESSAGE2(true, "Send: %d bytes/s, Receive: %d bytes/s\n", SendRate, ReceiveRate);
#endif
                }                
            }
        }
    }

    return !IsExiting();
}

// UDP socket broadcast loop
bool ThreadMgr::BroadcastLoop()
{
    BroadcastSocket broadcastSocket(InitSocketLib, SocketFactory);

    if (!broadcastSocket.Create(BroadcastPort, true))
    {
        return false;
    }

    while (!IsExiting())
    {
        if (!IsValidConnection())
        {
            Ptr<AmpStream> stream = *SF_HEAP_AUTO_NEW(this) AmpStream();
            BroadcastInfoLock.DoLock();
            Ptr<MessagePort> msg = *SF_HEAP_AUTO_NEW(this) MessagePort(Port, BroadcastApp, BroadcastFile);
            BroadcastInfoLock.Unlock();
            msg->Write(*stream);

#ifdef SF_AMP_DEBUG_MSG_LOG
            SF_DEBUG_MESSAGE(true, "Sending broadcast message\n");
#endif
            if (broadcastSocket.Broadcast(reinterpret_cast<const char*>(stream->GetBuffer()), 
                                          stream->GetBufferSize()) < 0)
            {
                return true;
            }
        }

        Thread::Sleep(1);
    }

    return true;
}

// UDP socket broadcast receive loop
bool ThreadMgr::BroadcastRecvLoop()
{
    Socket wiiuSocket(InitSocketLib, SocketFactory);
    char wiiuBuffer[BufferSize];

    BroadcastSocket broadcastSocket(InitSocketLib, SocketFactory);
    char bufferReceived[BufferSize];

    if (!broadcastSocket.Create(BroadcastRecvPort, false))
    {
        return false;
    }

    UInt64 lastWiiuAttempt = 0;

    while (!IsExiting())
    {
        bool somethingHappened = false;
        int packetSize = broadcastSocket.Receive(bufferReceived, BufferSize);
        if (packetSize > 0)
        {
            somethingHappened = true;
            Ptr<AmpStream> strmReceived = *SF_HEAP_AUTO_NEW(this) AmpStream(
                                            reinterpret_cast<UByte*>(bufferReceived), packetSize);
            SF_ASSERT(strmReceived->FirstMessageSize() == static_cast<UPInt>(packetSize));
            if (strmReceived->FirstMessageSize() == static_cast<UPInt>(packetSize))
            {
                // Read the stream and create a message
                Ptr<Message> msg = *CreateAndReadMessage(*strmReceived);
                if (msg)
                {
                    // Uncompress the message, if necessary
                    Array<UByte> uncompressedData;
                    if (msg->Uncompress(uncompressedData))
                    {
                        Ptr<AmpStream> ampStream = *SF_HEAP_AUTO_NEW(this) AmpStream(&uncompressedData[0], uncompressedData.GetSize());
                        msg = *CreateAndReadMessage(*ampStream);
                    }
                }
                if (msg)
                {
                    BaseMessageTypeDescriptor* descriptor = MsgTypeRegistry->GetMessageTypeDescriptor(msg->GetMessageName());
                    if (descriptor != NULL && descriptor->CanHandle())
                    {
                        UInt32 recvPort;
                        UInt32 recvAddress;
                        char name[256];
                        broadcastSocket.GetName(&recvPort, &recvAddress, name, 256);
                        msg->SetPeerName(name);
                        msg->SetPeerAddress(recvAddress);
                        descriptor->Handle(msg);
                    }
                }
            }
        }

        if (!wiiuSocket.IsValid())
        {
            UInt64 currentTime = Timer::GetTicks();
            if (Timer::TicksToSeconds(lastWiiuAttempt - currentTime) > 1.0)
            {
                wiiuSocket.Destroy();
                if (!wiiuSocket.CreateClient("127.0.0.1", 6003, NULL))
                {
                    lastWiiuAttempt = currentTime;
                }
                else
                {
                    wiiuSocket.SetBlocking(false);
                }
            }
        }
        else
        {
            packetSize = wiiuSocket.Receive(wiiuBuffer, BufferSize);
            if (packetSize > 0)
            {
                somethingHappened = true;
                wiiuBuffer[packetSize] = '\0';

                // The data received from the HostIO manager (port 6003) is of the form:
                //    "+<Channel Name>:<Port Number>" to indicate that a channel has started
                //    "-<Channel Name>:<Port Number>" to indicate that a channel has stopped
                // The data is really transmitted as a single continuous string, so care must
                // be taken to properly parse the received data.

                char* pBuf = wiiuBuffer;

                // Find the ':' delimiter in the string first.
                while (NULL != pBuf)
                {
                    char* pDelim = SFstrchr(pBuf, ':');

                    if (NULL != pDelim)
                    {
                        // Split the received data into the name and port portions.
                        *pDelim = '\0';
                        char* pName = pBuf;
                        char* pPort = pDelim + 1;

                        // Setup the sockaddr_in for the demo thread.
                        int port = atoi(pPort);

                        if ((port > 6000) && 0 == strcmp(pName, "+AMP"))
                        {
                            Ptr<MessagePort> msg = *SF_HEAP_AUTO_NEW(this) MessagePort(port);
                            msg->SetPeerAddress(0x7F000001);
                            msg->SetPeerName("localhost");
                            msg->SetPlatform(MessagePort::PlatformWiiU);

                            // The target has started the HostIO channel named "AMP"
                            BaseMessageTypeDescriptor* descriptor = MsgTypeRegistry->GetMessageTypeDescriptor(msg->GetMessageName());
                            if (descriptor != NULL && descriptor->CanHandle())
                            {
                                descriptor->Handle(msg);
                            }
                        }
                        else if (0 == strcmp(pName, "-AMP"))
                        {
                            // The target has shut down the HostIO channel named "AMP"
                            break;
                        }

                        // Advance the pBuf pointer to just after the port number. This should
                        // be a '+', '-', or '\0' character.
                        pBuf = pPort;
                        while (isdigit(*pBuf))
                        {
                            pBuf++;
                        }

                        // If the end of the string was reached, stop processing.
                        if ('\0' == *pBuf)
                        {
                            pBuf = NULL;
                        }
                    }
                    else
                    {
                        pBuf = NULL;
                    }
                }
            }

        }


        if (!somethingHappened)
        {
            Thread::MSleep(100);
        }
    }

    return true;
}

bool ThreadMgr::CompressLoop()
{
    while (!IsExiting())
    {
        bool actionTaken = false;
        if (RcvQueueWaitEvent != NULL && !RcvQueueWaitEvent->IsSignaled())
        {
            MsgReceivedQueue.Clear();
            actionTaken = true;
        }

        Message* msg = MsgReceivedQueue.PopFront();
        if (msg != NULL)
        {
            // Uncompress the message, if necessary
            Array<UByte> uncompressedData;
            if (msg->Uncompress(uncompressedData))
            {
                msg->Release();
                Ptr<AmpStream> ampStream = *SF_HEAP_AUTO_NEW(this) AmpStream(&uncompressedData[0], uncompressedData.GetSize());
                msg = CreateAndReadMessage(*ampStream);
            }

            if (msg->GetGFxVersion() != LastGFxVersion)
            {
                LastGFxVersion = static_cast<UByte>(msg->GetGFxVersion());
                if (ConnectionChangedCallback != NULL)
                {
                    ConnectionChangedCallback->OnMsgGFxVersionChanged(msg->GetGFxVersion());
                }
            }

            if (msg->GetVersion() < MsgVersion)
            {
                MsgVersion = msg->GetVersion();
                if (ConnectionChangedCallback != NULL)
                {
                    ConnectionChangedCallback->OnMsgVersionMismatch(msg->GetVersion());
                }
            }

#ifdef SF_AMP_DEBUG_MSG_LOG
            SF_DEBUG_MESSAGE1(true, "Received %s\n", msg->GetMessageName().ToCStr());
#endif

            BaseMessageTypeDescriptor* descriptor = MsgTypeRegistry->GetMessageTypeDescriptor(msg->GetMessageName());
            if (descriptor != NULL && descriptor->CanHandle() && descriptor->ShouldHandleImmediately())
            {
                // Handle immediately
                // bypass the queue
                descriptor->Handle(msg);
                msg->Release();
            }
            else
            {
                // Place the new message on the received queue
                MsgUncompressedQueue.PushBack(msg);
            }
            actionTaken = true;
        }

        // Compress the messages to be sent
        msg = MsgSendQueue.PopFront();
        if (msg != NULL)
        {
#ifdef SF_AMP_DEBUG_MSG_LOG
            SF_DEBUG_MESSAGE1(true, "Compressing %s\n", msg->GetMessageName().ToCStr());
#endif
            if (MsgVersion >= 18 && CompressMessages)  // To support older protocol
            {
                msg->SetVersion(MsgVersion);
                Array<UByte> compressedData;
                if (msg->Compress(compressedData))
                {
                    MessageCompressed* msgCompressed = MsgTypeRegistry->CreateMessage<MessageCompressed>();
                    msgCompressed->SetVersion(MsgVersion);
                    msgCompressed->AddCompressedData(&compressedData[0], compressedData.GetSize());
                    msg->Release();
                    msg = msgCompressed;
                }
            }
            MsgCompressedQueue.PushBack(msg);
            actionTaken = true;
        }

#ifdef SF_AMP_SERVER
        if (!AmpServer::GetInstance().IsProfiling())
        {
            Ptr<MessageProfileFrame> clearMsg = *SF_HEAP_AUTO_NEW(this) MessageProfileFrame();
            MsgCompressedQueue.ClearMsgType(clearMsg);
        }
#endif

        if (!actionTaken)
        {
            Thread::MSleep(100);
        }
    }

    return true;
}

// Places a message on the send queue
// after serializing it into a stream
void ThreadMgr::SendAmpMessage(Message* msg)
{
    if (IsValidConnection())
    {
#ifdef SF_AMP_DEBUG_MSG_LOG
        SF_DEBUG_MESSAGE1(true, "Queueing %s\n", msg->GetMessageName().ToCStr());
#endif
        MsgSendQueue.PushBack(msg);
    }
    else
    {
        msg->Release();
    }
}

// Retrieves a message from the received queue
// Returns NULL if there is no message to retrieve
Ptr<Message> ThreadMgr::GetNextReceivedMessage()
{
    Message* msg = MsgUncompressedQueue.PopFront();
#ifdef SF_AMP_DEBUG_MSG_LOG
    SF_DEBUG_MESSAGE1(msg != NULL, "Processing %s\n", msg->GetMessageName().ToCStr());
#endif
    return *msg;
}

// Clears all messages of the specified type from the received queue
// If the input parameter is NULL, the it clears all the messages
void ThreadMgr::ClearReceivedMessages(const Message* msg)
{
    MsgReceivedQueue.ClearMsgType(msg);
}


// Handle the next received message and remove it from the queue
// Returns true if there was a message and it was handled
bool ThreadMgr::HandleNextReceivedMessage()
{
    Ptr<Message> msg = GetNextReceivedMessage();
    if (msg)
    {
        const BaseMessageTypeDescriptor* descriptor = MsgTypeRegistry->GetMessageTypeDescriptor(msg->GetMessageName());
        if (descriptor != NULL && descriptor->CanHandle())
        {
            descriptor->Handle(msg);
            return true;
        }        
    }
    return false;
}

// Notify the callback object that the connection status has changed
void ThreadMgr::UpdateStatus(ConnStatusInterface::StatusType status, const char* msg)
{
    Lock::Locker locker(&StatusLock);
    ConnStatusInterface::StatusType oldStatus = ConnectionStatus;
    if (oldStatus != status)
    {
        ConnectionStatus = status;
        if (ConnectionChangedCallback != NULL)
        {
            ConnectionChangedCallback->OnStatusChanged(status, oldStatus, msg);
        }
    }
}

// factory method that extracts data from the stream and creates a new message
Message* ThreadMgr::CreateAndReadMessage(File& str)
{
    UByte msgType = str.ReadUByte();
    String msgTypeName;
    if (msgType == 0)
    {
        Message::ReadString(str, &msgTypeName);
    }
    else // support of older message versions
    {
        msgTypeName = Message::MsgTypeToMsgName(msgType);
    }

    const BaseMessageTypeDescriptor* descriptor = MsgTypeRegistry->GetMessageTypeDescriptor(msgTypeName);
    if (descriptor == NULL)
    {
        return NULL; // The type is unknown
    }

    UInt32 msgVersion = str.ReadUInt32();
    if (msgVersion > Message::GetLatestVersion())
    {
        return NULL; //The version is greater than the known ones
    }

    str.SeekToBegin();

    Message* message = descriptor->CreateMessage();
    if (message != NULL)
    {
        message->Read(str);
    }
    else
    {
        SF_DEBUG_MESSAGE(true, "Received corrupt message\n");
    }

    return message;
}

void ThreadMgr::UpdateValidConnection()
{
    UInt64 ticks = Timer::GetTicks();

    bool connected = (LastRcvdHeartbeat != 0);
    if (HeartbeatIntervalMillisecs > 0)
    {
        UInt64 deltaTicks = ticks - LastRcvdHeartbeat;
        connected = (deltaTicks < 2 * HeartbeatIntervalMillisecs * 1000);
    }

    ValidConnection = connected ? 1 : 0;

    ConnStatusInterface::StatusType newStatus;
    String strMessage;
    if (connected)
    {
        newStatus = ConnStatusInterface::CS_OK;
        SPrintF(strMessage, "Connection established on port %d\n", Port);
    }
    else
    {
        newStatus = ConnStatusInterface::CS_Connecting;
        SPrintF(strMessage, "Lost connection after %d microseconds\n", (int)(ticks - LastRcvdHeartbeat));
        MsgSendQueue.Clear();
    }
    UpdateStatus(newStatus, strMessage);
}



///////////////////////////////////////////////////////////////////

ThreadMgr::MsgQueue::MsgQueue(Scaleform::Event* sizeEvent, UInt32 sizeCheckHysterisisPercent, UInt32 maxSize) : 
    QueueSize(0), 
    MaxSize(maxSize),
    SizeEvent(sizeEvent),
    SizeCheckHysterisisPercent(sizeCheckHysterisisPercent)
{
    if (SizeEvent != NULL)
    {
        SizeEvent->SetEvent();
    }
}


void ThreadMgr::MsgQueue::PushBack(Message* msg)
{
    Lock::Locker locker(&QueueLock);
    Queue.PushBack(msg);
    ++QueueSize;
    CheckSize(Memory::GetHeapByAddress(msg));
}

Message* ThreadMgr::MsgQueue::PopFront()
{
    Message* msg = NULL;
    Lock::Locker locker(&QueueLock);
    if (!Queue.IsEmpty())
    {
        msg = Queue.GetFirst();
        MemoryHeap* heap = Memory::GetHeapByAddress(msg);
        Queue.Remove(msg);
        --QueueSize;
        CheckSize(heap);
    }
    return msg;
}

void ThreadMgr::MsgQueue::Clear()
{
    Lock::Locker locker(&QueueLock);
    while (!Queue.IsEmpty())
    {
        Message* msg = Queue.GetFirst();
        Queue.Remove(msg);
        msg->Release();
    }
    QueueSize = 0;
    if (SizeEvent != NULL)
    {
        SizeEvent->SetEvent();
    }
}

void ThreadMgr::MsgQueue::ClearMsgType(const Message* msg)
{
    Lock::Locker locker(&QueueLock);
    Message* msgQueued = Queue.GetFirst();
    UPInt queueInitSize = QueueSize;
    for (UPInt i = 0; i < queueInitSize; ++i)
    {
        if (msg == NULL || msgQueued->GetMessageName() == msg->GetMessageName())
        {
            Message* msgNext = Queue.GetNext(msgQueued);
            MemoryHeap* heap = Memory::GetHeapByAddress(msgNext);
            Queue.Remove(msgQueued);
            msgQueued->Release();
            msgQueued = msgNext;
            --QueueSize;
            CheckSize(heap);
        }
    }    
}

UInt32 ThreadMgr::MsgQueue::GetSize() const
{
    return QueueSize;
}

void ThreadMgr::MsgQueue::CheckSize(MemoryHeap* heap)
{
    if (SizeEvent != NULL)
    {
        UPInt heapLimit = heap->GetLimit();
        bool heapOver = (heapLimit > 0 && heap->GetFootprint() > heapLimit);
        bool sizeOver = (MaxSize > 0 && QueueSize > MaxSize);
        if (heapOver || sizeOver)
        {
            SizeEvent->ResetEvent();
            SF_DEBUG_MESSAGE1(true, "Waiting for AMP... Queue size=%d\n", static_cast<UInt32>(QueueSize));

            if (heapOver && QueueSize < 2)
            {
                // Increase heap limit because barely fits one AMP message
                heap->SetLimit(2 * heapLimit);
            }
        }
        else
        {
            bool heapUnder = (heapLimit == 0 || 100 * heap->GetFootprint() < heapLimit * SizeCheckHysterisisPercent);
            bool sizeUnder = (MaxSize == 0 || 100 * QueueSize < MaxSize * SizeCheckHysterisisPercent);
            if (heapUnder && sizeUnder)
            {
                SizeEvent->SetEvent();
            }
        }
    }
}

} // namespace AMP
} // namespace GFx
} // namespace Scaleform

#endif  // defined(SF_ENABLE_THREADS) && defined(SF_ENABLE_SOCKETS)
