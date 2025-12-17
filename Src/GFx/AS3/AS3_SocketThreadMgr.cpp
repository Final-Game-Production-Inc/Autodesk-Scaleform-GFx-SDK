/**************************************************************************

Filename    :   AS3_SocketThreadMgr.cpp
Content     :   Manages the socket threads
Created     :   December 2009
Authors     :   Alex Mantzaris

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "AS3_SocketThreadMgr.h"
#include "AS3_SocketBuffer.h"
#include "Kernel/SF_Threads.h"
#include "Kernel/SF_Debug.h"
#include "GFx/Net/GFx_Socket.h"
#include "GFx/AS3/Obj/Net/AS3_Obj_Net_Socket.h"

#if defined(SF_ENABLE_THREADS) && defined(SF_ENABLE_SOCKETS)

#define MICROSECONDS_PER_SECOND 1000000

namespace Scaleform {
namespace GFx {
namespace AS3 {


// Static 
// Send thread function
int SocketThreadMgr::SocketThreadLoop(Thread* sendThread, void* param)
{
    SF_UNUSED(sendThread);

    SocketThreadMgr* threadMgr = static_cast<SocketThreadMgr*>(param);
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

    SF_DEBUG_MESSAGE(1, "AS3 Socket thread exited\n");

    return 0;
}


#ifdef TEST_SERVER
// Static 
// Test Server thread function
int SocketThreadMgr::TestServerSocketThreadLoop(Thread* sendThread, void* param)
{
    SF_UNUSED(sendThread);

    SocketThreadMgr* threadMgr = static_cast<SocketThreadMgr*>(param);
    if (threadMgr == NULL)
    {
        return 1;
    }

    while (threadMgr->TestServerLoop())
    {
        // SendLoop exited but we are not shutting down
        // re-try to establish connection

        Thread::Sleep(1);  // seconds
    }

    SF_DEBUG_MESSAGE(1, "AS3 server Socket thread exited\n");

    return 0;
}
#endif


// Constructor
SocketThreadMgr::SocketThreadMgr(bool initSocketLib, SocketImplFactory* socketFactory, Instances::fl_net::Socket* sock) : 
    AS3Sock(sock),
    Port(0),
    Sock(initSocketLib, socketFactory),
#ifdef TEST_SERVER
    TestServerSock(initSocketLib, socketFactory),
#endif
    InitSocketLib(initSocketLib),
    Exiting(false),
    ConnectTimeout(DefaultConnectTimeout)
{
    Sock.SetLock(&SocketLock);
#ifdef TEST_SERVER
    TestServerSock.SetLock(&SocketLock);
#endif

    SF_DEBUG_MESSAGE(true, "AS3 Thread manager created\n");
}

// Destructor
SocketThreadMgr::~SocketThreadMgr()
{
    Uninit();
    SF_DEBUG_MESSAGE(true, "AS3 Thread manager destroyed\n");
}

// Initialization
// Returns true if successful
bool SocketThreadMgr::Init(const char* address, int port)
{
    // Have we already been initialized?
    if (IsRunning())
    {
        // Different connection information - start over
        Uninit();
    }

    Exiting = false;
    Port = port;
    IpAddress = address;

    ReceivedBuffer = *SF_HEAP_AUTO_NEW(this) SocketBuffer();
    ReceivedBuffer->Clear();
    SendingBuffer = *SF_HEAP_AUTO_NEW(this) SocketBuffer();

    // Start the send thread
    SocketThread = *SF_HEAP_AUTO_NEW(this) Thread(SocketThreadMgr::SocketThreadLoop, (void*) this);
    if (!SocketThread || !SocketThread->Start())
    {
        return false;
    }
    SocketThread->SetThreadName("Scaleform AS3 Socket");

#ifdef TEST_SERVER
    TestServerBuffer = *SF_HEAP_AUTO_NEW(this) SocketBuffer();
    TestServerBuffer->Clear();
    TestServerSocketThread = *SF_HEAP_AUTO_NEW(this) Thread(SocketThreadMgr::TestServerSocketThreadLoop, (void*) this);
    if (!TestServerSocketThread || !TestServerSocketThread->Start())
    {
        return false;
    }
    TestServerSocketThread->SetThreadName("Scaleform AS3 Server Socket");
#endif
    SF_DEBUG_MESSAGE(true, "AS3 Socket Thread manager initialized\n");

    return true;
}

// Cleanup
void SocketThreadMgr::Uninit()
{
    SetExiting();

    // wait until threads are done
    if (SocketThread)
    {
        SocketThread->Wait();
        SocketThread = NULL;
    }

    ReceivedBuffer = NULL;
    SendingBuffer = NULL;

#ifdef TEST_SERVER
    if (TestServerSocketThread)
    {
        TestServerSocketThread->Wait();
        TestServerSocketThread = NULL;
    }
    TestServerBuffer = NULL;
#endif

    SF_DEBUG_MESSAGE(true, "AS3 Socket Thread manager uninitialized\n");
}

const StringLH& SocketThreadMgr::GetAddress() const
{
    return IpAddress;
}

int SocketThreadMgr::GetPort() const
{
    return Port;
}

void SocketThreadMgr::SetConnectTimeout(UInt32 timeoutMillisecs)
{
    ConnectTimeout = timeoutMillisecs;
}

UInt32 SocketThreadMgr::GetConnectTimeout() const
{
    return ConnectTimeout;
}

UInt32 SocketThreadMgr::GetBytesPending() const
{
    Lock::Locker locker(&SendingBufferLock);
    return static_cast<UInt32>(SendingBuffer->GetBufferSize());
}

UInt32 SocketThreadMgr::GetBytesAvailable() const
{
    Lock::Locker locker(&ReceivedBufferLock);
    return static_cast<UInt32>(ReceivedBuffer->GetBufferSize());
}

// Returns true when there is a thread still running
bool SocketThreadMgr::IsRunning() const
{
    if (SocketThread && !SocketThread->IsFinished())
    {
        return true;
    }

    return false;
}

// Thread-safe status accessor
bool SocketThreadMgr::IsExiting() const
{
    Lock::Locker locker(&StatusLock);
    return Exiting;
}

// Signals the threads that we are shutting down
void SocketThreadMgr::SetExiting()
{
    Lock::Locker locker(&StatusLock);
    Exiting = true;
}

bool SocketThreadMgr::ReadBool(bool* valueRead)
{
    Lock::Locker locker(&ReceivedBufferLock);
    if (ReceivedBuffer->IsEOF())
    {
        return false;
    }
    *valueRead = (ReceivedBuffer->ReadUByte() != 0);
    return true;
}

bool SocketThreadMgr::ReadByte(SByte* valueRead)
{
    Lock::Locker locker(&ReceivedBufferLock);
    if (ReceivedBuffer->IsEOF())
    {
        return false;
    }
    *valueRead = ReceivedBuffer->ReadSByte();
    return true;
}

bool SocketThreadMgr::ReadBytes(Array<SByte>* valueRead, int length)
{
    Lock::Locker locker(&ReceivedBufferLock);
    int numBytes = length;
    if (numBytes == 0)
    {
        numBytes = ReceivedBuffer->BytesAvailable();
    }
    int readPosition = ReceivedBuffer->GetReadPosition();
    for (int i = 0; i < numBytes; ++i)
    {
        if (ReceivedBuffer->IsEOF())
        {
            ReceivedBuffer->Seek(readPosition);
            break;
        }
        valueRead->PushBack(ReceivedBuffer->ReadSByte());
    }
    return true;
}


bool SocketThreadMgr::ReadDouble(Double* valueRead)
{
    Lock::Locker locker(&ReceivedBufferLock);
    if (ReceivedBuffer->IsEOF())
    {
        return false;
    }
    *valueRead = ReceivedBuffer->ReadDouble();
    return true;
}

bool SocketThreadMgr::ReadFloat(float* valueRead)
{
    Lock::Locker locker(&ReceivedBufferLock);
    if (ReceivedBuffer->IsEOF())
    {
        return false;
    }
    *valueRead = ReceivedBuffer->ReadFloat();
    return true;
}

bool SocketThreadMgr::ReadInt(SInt32* valueRead)
{
    Lock::Locker locker(&ReceivedBufferLock);
    if (ReceivedBuffer->IsEOF())
    {
        return false;
    }
    *valueRead = ReceivedBuffer->ReadSInt32();
    return true;
}

bool SocketThreadMgr::ReadShort(SInt16* valueRead)
{
    Lock::Locker locker(&ReceivedBufferLock);
    if (ReceivedBuffer->IsEOF())
    {
        return false;
    }
    *valueRead = ReceivedBuffer->ReadSInt16();
    return true;
}

void SocketThreadMgr::SendBool(bool sendValue)
{
    Lock::Locker locker(&SendingBufferLock);
    SendingBuffer->WriteUByte(sendValue ? 1 : 0);
}

void SocketThreadMgr::SendByte(SByte sendValue)
{
    Lock::Locker locker(&SendingBufferLock);
    SendingBuffer->WriteSByte(sendValue);
}

void SocketThreadMgr::SendBytes(const SByte* sendValue, int length)
{
    Lock::Locker locker(&SendingBufferLock);
    SendingBuffer->Append(reinterpret_cast<const UByte*>(sendValue), length);
}

void SocketThreadMgr::SendDouble(Double sendValue)
{
    Lock::Locker locker(&SendingBufferLock);
    SendingBuffer->WriteDouble(sendValue);
}

void SocketThreadMgr::SendFloat(float sendValue)
{
    Lock::Locker locker(&SendingBufferLock);
    SendingBuffer->WriteFloat(sendValue);
}

void SocketThreadMgr::SendInt(SInt32 sendValue)
{
    Lock::Locker locker(&SendingBufferLock);
    SendingBuffer->WriteSInt32(sendValue);
}

void SocketThreadMgr::SendShort(SInt16 sendValue)
{
    Lock::Locker locker(&SendingBufferLock);
    SendingBuffer->WriteSInt16(sendValue);
}

// Called from the advance thread
void SocketThreadMgr::CheckEvents()
{
    Array<Ptr<EventInfo> > events;
    {
        Lock::Locker locker(&EventQueueLock);
        for (UPInt i = 0; i < EventQueue.GetSize(); ++i)
        {
            events.PushBack(EventQueue[i]);
        }
        EventQueue.Clear();
    }
    for (UPInt i = 0; i < events.GetSize(); ++i)
    {
        switch (events[i]->EventType)
        {
        case EventClose:
            AS3Sock->ExecuteCloseEvent();
            break;
        case EventConnect:
            AS3Sock->ExecuteConnectEvent();
            break;
#ifdef SF_AS3_VERSION_AIR
        case EventOutputProgress:
            AS3Sock->ExecuteOutputProgressEvent(events[i]->EventParameters[0], events[i]->EventParameters[1]);
            break;
#endif
        case EventSecurityError:
            break;
        case EventSocketData:
            AS3Sock->ExecuteSocketDataEvent(events[i]->EventParameters[0], 0);
            break;
        }
    }
}


// Socket thread loop
// Returns true if exited with a broken connection
// Returns false if shutting down
bool SocketThreadMgr::SendReceiveLoop()
{
    String errorMsg;

    // Create a socket
    UInt64 timeStartConnect = Timer::GetTicksMs();
    while (!IsExiting() && !Sock.CreateClient(IpAddress.ToCStr(), static_cast<UInt32>(Port), &errorMsg))
    {
        if (Timer::GetTicksMs() - timeStartConnect > ConnectTimeout)
        {
            return false;
        }
        Thread::MSleep(10);  // Don't hog the CPU
    }
    SF_DEBUG_MESSAGE(1, "AS3 Socket: Listening for incoming connection\n");

    QueueEvent(EventConnect, NULL, 0);

    Sock.SetBlocking(false);

    char bufferReceived[BufferSize];

    // Keep sending messages from the send queue
    while (!IsExiting() && !Sock.CheckAbort())
    {
        bool actionPerformed = false;

        SendingBufferLock.DoLock();
        const UByte* sendBuffer = SendingBuffer->GetBuffer();
        UPInt sendBufferRemaining = SendingBuffer->GetBufferSize();
        while (sendBufferRemaining > 0)
        {
            UPInt nextSendSize;
            if (sendBufferRemaining <= BufferSize)
            {
                // The message fits in one packet
                nextSendSize = sendBufferRemaining;
            }
            else
            {
                // The message does not fit. 
                // Send the first BUFFER_SIZE bytes in this packet, 
                // and the rest in the next one
                nextSendSize = BufferSize;
            }

            // Send the packet
            int sentBytes = Sock.Send(reinterpret_cast<const char*>(sendBuffer), nextSendSize);
            if (sentBytes > 0)
            {
                sendBuffer += sentBytes;
                sendBufferRemaining -= sentBytes;
#ifdef SF_AS3_VERSION_AIR
                UInt32 params[2];
                params[0] = static_cast<UInt32>(sendBufferRemaining);
                params[1] = static_cast<UInt32>(SendingBuffer->GetBufferSize());
                QueueEvent(EventOutputProgress, params, 2);
#endif
                actionPerformed = true;
            }
            else
            {
                SF_DEBUG_MESSAGE(sentBytes < 0, "Failed to send data.\n");
                sendBufferRemaining = 0;
            }
        }
        SendingBuffer->Clear();
        SendingBufferLock.Unlock();

        // Check for incoming messages and place on the received queue
        UInt32 bytesReceived = 0;
        while (true)
        {
            UInt32 packetSize = Sock.Receive(bufferReceived, BufferSize);
            if (packetSize == 0)
            {
                break;
            }
            actionPerformed = true;
            bytesReceived += packetSize;

            Lock::Locker rcvLock(&ReceivedBufferLock);
            ReceivedBuffer->Append(reinterpret_cast<UByte*>(bufferReceived), packetSize);
        }

        SF_DEBUG_MESSAGE2(bytesReceived > 0, "Received %u bytes, %u remaining to be read\n", bytesReceived, ReceivedBuffer->GetBufferSize());

        {
            Lock::Locker rcvLock(&ReceivedBufferLock);
            ReceivedBuffer->DiscardReadBytes();
            if (ReceivedBuffer->GetBufferSize() > 0)
            {
                QueueEvent(EventSocketData, &bytesReceived, 1);
            }
        }

        if (!Sock.IsConnected())
        {
            SetExiting();
            QueueEvent(EventClose, NULL, 0);
        }

        if (!actionPerformed)
        {
            Thread::MSleep(10);  // Don't hog the CPU
        }
    }

    return !IsExiting();
}

#ifdef TEST_SERVER
// Socket thread loop
// Returns true if exited with a broken connection
// Returns false if shutting down
bool SocketThreadMgr::TestServerLoop()
{
    String errorMsg;

    // Create a socket
    while (!IsExiting() && TestServerSock.CreateServer(static_cast<UInt32>(Port), &errorMsg))
    {
        SF_DEBUG_MESSAGE(1, "AS3 Server Socket: Listening for incoming connection\n");
        if (TestServerSock.Accept(1))
        {
            TestServerSock.SetBlocking(false);

            char bufferReceived[BufferSize];

            // Keep sending messages from the send queue
            while (!IsExiting() && !TestServerSock.CheckAbort())
            {
                bool actionPerformed = false;

                const UByte* sendBuffer = TestServerBuffer->GetBuffer();
                UPInt sendBufferRemaining = TestServerBuffer->GetBufferSize();
                while (sendBufferRemaining > 0)
                {
                    UPInt nextSendSize;
                    if (sendBufferRemaining <= BufferSize)
                    {
                        // The message fits in one packet
                        nextSendSize = sendBufferRemaining;
                    }
                    else
                    {
                        // The message does not fit. 
                        // Send the first BUFFER_SIZE bytes in this packet, 
                        // and the rest in the next one
                        nextSendSize = BufferSize;
                    }

                    // Send the packet
                    int sentBytes = TestServerSock.Send(reinterpret_cast<const char*>(sendBuffer), nextSendSize);
                    if (sentBytes > 0)
                    {
                        sendBuffer += sentBytes;
                        sendBufferRemaining -= sentBytes;
                        UInt32 params[2];
                        params[0] = static_cast<UInt32>(sendBufferRemaining);
                        params[1] = static_cast<UInt32>(TestServerBuffer->GetBufferSize());
                        actionPerformed = true;
                    }
                    else
                    {
                        SF_DEBUG_MESSAGE(sentBytes < 0, "Failed to send data.\n");
                        sendBufferRemaining = 0;
                    }
                }
                TestServerBuffer->Clear();

                // Check for incoming messages and place on the received queue
                UInt32 packetSize = TestServerSock.Receive(bufferReceived, BufferSize);
                if (packetSize > 0)
                {
                    actionPerformed = true;

                    TestServerBuffer->Append(reinterpret_cast<UByte*>(bufferReceived), packetSize);
                    TestServerBuffer->DiscardReadBytes();
                }

                if (!TestServerSock.IsConnected())
                {
                    SetExiting();
                }

                if (!actionPerformed)
                {
                    Thread::MSleep(10);  // Don't hog the CPU
                }
            }
        }
    }
    return !IsExiting();
}
#endif

void SocketThreadMgr::QueueEvent(EventTypes eventType, UInt32* eventParams, UInt32 numParams)
{
    Ptr<EventInfo> eventInfo = SF_HEAP_AUTO_NEW(this) EventInfo();
    eventInfo->EventType = eventType;
    for (UInt32 i = 0; i < numParams; ++i)
    {
        eventInfo->EventParameters.PushBack(eventParams[i]);
    }
    Lock::Locker locker(&EventQueueLock);
    EventQueue.PushBack(eventInfo);
}


} // namespace AS3
} // namespace GFx
} // namespace Scaleform

#endif  // defined(SF_ENABLE_THREADS) && defined(SF_ENABLE_SOCKETS)
