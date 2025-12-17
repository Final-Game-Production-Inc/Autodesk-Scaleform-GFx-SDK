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

#include <revolution/hio2.h>
#include "Kernel/SF_Memory.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_MsgFormat.h"

namespace Scaleform {
namespace GFx {
namespace AMP {

// Wii does not support sockets
// We create a pseudo-socket based on HIO2

class GFxSocketImpl  : public NewOverrideBase<Stat_Default_Mem>, public SocketInterface
{
public:
    GFxSocketImpl()
    { 
        memset(BytesReceived, 0, sizeof(BytesReceived));
    }
    ~GFxSocketImpl()
    {
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
        if (!Open())
        {
            return false;
        }
        return true;
    }
    bool Connect() 
    {
        return false;  // Server only
    }
    bool Accept(int) 
    { 
        return true;
    }
    int Send(const char* buffer, UPInt bufferSize) const 
    { 
        if (!IsValid() || Sending)
        {
            return 0;
        }

        if (!IsSendPossible())
        {
            return 0;
        }

        memcpy(SendBuffer, buffer, bufferSize);
        memset(BytesSent, 0, sizeof(BytesSent));

        int msgSize = static_cast<int>(bufferSize);
        for (int i = 3; i >= 0; --i)
        {
            BytesSent[i] = msgSize % 256;
            msgSize /= 256;
        }
        DCFlushRange(&BytesSent, sizeof(BytesSent));
        if (!HIO2Write(Hio2Handle, WriteAddress, BytesSent, sizeof(BytesSent)))
        {
            return -1;
        }
        DCFlushRange(&SendBuffer, sizeof(SendBuffer));
        if (!HIO2Write(Hio2Handle, WriteAddress + sizeof(BytesSent), SendBuffer, sizeof(SendBuffer)))
        {
            return -1;
        }
        if (!HIO2WriteMailbox(Hio2Handle, AMP_Mailbox_Recv))
        {
            return -1;
        }
        Sending = true;
        return bufferSize;
    }
    int Receive(char* buffer, int bufferSize) const 
    {
        if (!IsSendPossible())
        {
            return 0;
        }

        int msgSize = 0;
        for (int i = 0; i < 4; ++i)
        {
            msgSize <<= 8;
            msgSize += BytesReceived[i];
        }

        if (msgSize > 0)
        {
            memcpy(buffer, static_cast<void*>(RecvBuffer), msgSize);
            memset(BytesReceived, 0, sizeof(BytesReceived));

            HIO2WriteMailbox(Hio2Handle, AMP_Mailbox_RecvAck);
        }

        return msgSize;
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
        if (Hio2Handle != HIO2_INVALID_HANDLE_VALUE)
        {
            HIO2WriteMailbox(Hio2Handle, AMP_Mailbox_Exit);
            HIO2Close(Hio2Handle);
        }
        HIO2Exit();
        Hio2Handle = HIO2_INVALID_HANDLE_VALUE;
        Connected = false;
        Sending = false;
        Exiting = false;
        return true; 
    }
    bool Startup() 
    { 
        return true; 
    }
    void Cleanup() 
    { 
    }
    int GetLastError() const 
    { 
        return HIO2GetLastError(); 
    }
    bool IsValid() const 
    { 
        return (Connected && Hio2Handle != HIO2_INVALID_HANDLE_VALUE); 
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
        if (!Connected)
        {
            if (IsSendPossible())
            {
                if (!HIO2WriteMailbox(Hio2Handle, AMP_Mailbox_Open))
                {
                    return true;
                }
            }
        }
        if (Exiting)
        {
            return true;
        }
        return false;
    }
private:

    enum MailboxValue
    {
        AMP_Mailbox_Open = 0,
        AMP_Mailbox_OpenResult,
        AMP_Mailbox_Recv,
        AMP_Mailbox_RecvAck,
        AMP_Mailbox_Exit,
    };

    static HIO2Handle       Hio2Handle;

    // HIO2 API buffer
    static u8               SendBuffer[512] ATTRIBUTE_ALIGN(32);
    static u8               RecvBuffer[512] ATTRIBUTE_ALIGN(32);

    static HIO2DeviceType   ExiDev;     // EXI device
    static bool             Connected;  // PC connection state
    static bool             Sending;    // Send flag
    static bool             Exiting;
    static u8               BytesReceived[32] ATTRIBUTE_ALIGN(32);
    static u8               BytesSent[32] ATTRIBUTE_ALIGN(32);
    static UInt32           ReadAddress;
    static UInt32           WriteAddress;

    bool Open()
    {
        Exiting = false;

        // Initialization
        if (!HIO2Init())
        {
            return false;
        }

        // Device list
        if (!HIO2EnumDevices(EnumCallback))
        {
            return false;
        }

        // When the device could not be found
        if (ExiDev == HIO2_DEVICE_INVALID)
        {
            return false;
        }

        // Device open
        Hio2Handle = HIO2Open(ExiDev, ReceiveCallback, NULL);
        if (Hio2Handle == HIO2_INVALID_HANDLE_VALUE)
        {
            return false;
        }
        return true;
    }

    bool IsSendPossible() const
    {
        // Get HIO2 status
        u32 status;
        if (!HIO2ReadStatus(Hio2Handle, &status))
        {
            return false;
        }

        return ((status & HIO2_STATUS_RX) == 0);
    }

    // enum devices
    static BOOL EnumCallback(HIO2DeviceType type)
    {
        // Use the first device that was detected
        ExiDev = type;
        return FALSE;
    }

    static void ReceiveCallback(HIO2Handle h)
    {
        u32 mail;

        if (HIO2ReadMailbox(h, &mail))
        {
            Connected = true;
            switch (mail)
            {
            case AMP_Mailbox_Recv:          // Receive data from connection target
                if (!HIO2Read(h, ReadAddress, BytesReceived, sizeof(BytesReceived)))
                {
                    memset(BytesReceived, 0, sizeof(BytesReceived));
                }
                else if (!HIO2Read(h, ReadAddress + sizeof(BytesReceived), RecvBuffer, sizeof(RecvBuffer)))
                {
                    memset(BytesReceived, 0, sizeof(BytesReceived));
                }
                DCInvalidateRange(&BytesReceived, sizeof(BytesReceived));
                DCInvalidateRange(&RecvBuffer, sizeof(RecvBuffer));
                break;
            case AMP_Mailbox_RecvAck:
                Sending = false; 
                break;
            case AMP_Mailbox_Exit:
                Exiting = true;
                break;
            }
        }
    }
};

HIO2Handle GFxSocketImpl::Hio2Handle = HIO2_INVALID_HANDLE_VALUE;
u8 GFxSocketImpl::SendBuffer[512] ATTRIBUTE_ALIGN(32);
u8 GFxSocketImpl::RecvBuffer[512] ATTRIBUTE_ALIGN(32);
HIO2DeviceType GFxSocketImpl::ExiDev = HIO2_DEVICE_INVALID;
bool GFxSocketImpl::Connected = false;
bool GFxSocketImpl::Sending = false;
bool GFxSocketImpl::Exiting = false;
u8 GFxSocketImpl::BytesReceived[32]  ATTRIBUTE_ALIGN(32);
u8 GFxSocketImpl::BytesSent[32]  ATTRIBUTE_ALIGN(32);
UInt32 GFxSocketImpl::ReadAddress = 0x0000;
UInt32 GFxSocketImpl::WriteAddress = 0x1000;

static DefaultSocketFactory<GFxSocketImpl> defaultSocketFactory;
SocketImplFactory* GlobalDefaultSocketFactory = &defaultSocketFactory;

} // namespace AMP
} // namespace GFx
} // namespace Scaleform

#endif  // SF_ENABLE_SOCKETS
