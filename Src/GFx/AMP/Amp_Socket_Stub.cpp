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

#include "Kernel/SF_Memory.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_MsgFormat.h"

namespace Scaleform {
namespace GFx {
namespace AMP {

class GFxSocketImpl : public NewOverrideBase<Stat_Default_Mem>, public SocketInterface
{
public:
    bool CreateStream(bool) { return false; }
    bool CreateDatagram(bool) { return false; }
    bool Bind() { return false; }
    bool Listen(int) { return false; }
    bool Connect() { return false; }
    bool Accept(int) { return false; }
    int Send(const char*, UPInt) const { return -1; }
    int Receive(char*, int) const { return -1; }
    int SendBroadcast(const char*, UPInt) const { return -1; }
    int ReceiveBroadcast(char*, int) const { return -1; }
    void SetListenPort(UInt32) { }
    void SetBroadcastPort(UInt32) { }
    void SetAddress(UInt32, const char*) { }
    void SetBlocking(bool) { }
    void SetBroadcast(bool) { }
    void GetName(UInt32*, UInt32*, char*, UInt32) { }
    bool Shutdown() { return false; }
    bool Startup() { return false; }
    void Cleanup() { }
    int GetLastError() const { return 0; }
    bool IsValid() const { return false; }
    bool IsListening() const { return false; }
    bool ShutdownListener() { return false; }
    bool CheckAbort() const { return false; }
};

static DefaultSocketFactory<GFxSocketImpl> defaultSocketFactory;
SocketImplFactory* GlobalDefaultSocketFactory = &defaultSocketFactory;

} // namespace AMP
} // namespace GFx
} // namespace Scaleform

#endif  // SF_ENABLE_SOCKETS
