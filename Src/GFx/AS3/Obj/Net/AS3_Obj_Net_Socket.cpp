//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Net_Socket.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Net_Socket.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "GFx/AS3/Obj/Utils/AS3_Obj_Utils_ByteArray.h"
#include "GFx/AS3/Obj/Utils/AS3_Obj_Utils_Endian.h"
#include "GFx/AS3/Obj/Events/AS3_Obj_Events_Event.h"
#include "GFx/AS3/Obj/Events/AS3_Obj_Events_IOErrorEvent.h"
#include "GFx/AS3/Obj/Events/AS3_Obj_Events_ProgressEvent.h"
#include "GFx/AS3/AS3_MovieRoot.h"

//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"
typedef ThunkFunc0<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_bytesAvailableGet, UInt32> TFunc_Instances_Socket_bytesAvailableGet;
typedef ThunkFunc0<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_bytesPendingGet, UInt32> TFunc_Instances_Socket_bytesPendingGet;
typedef ThunkFunc0<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_connectedGet, bool> TFunc_Instances_Socket_connectedGet;
typedef ThunkFunc0<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_endianGet, ASString> TFunc_Instances_Socket_endianGet;
typedef ThunkFunc1<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_endianSet, const Value, const ASString&> TFunc_Instances_Socket_endianSet;
typedef ThunkFunc0<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_localAddressGet, ASString> TFunc_Instances_Socket_localAddressGet;
typedef ThunkFunc0<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_localPortGet, SInt32> TFunc_Instances_Socket_localPortGet;
typedef ThunkFunc0<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_objectEncodingGet, UInt32> TFunc_Instances_Socket_objectEncodingGet;
typedef ThunkFunc1<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_objectEncodingSet, const Value, UInt32> TFunc_Instances_Socket_objectEncodingSet;
typedef ThunkFunc0<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_remoteAddressGet, ASString> TFunc_Instances_Socket_remoteAddressGet;
typedef ThunkFunc0<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_remotePortGet, SInt32> TFunc_Instances_Socket_remotePortGet;
typedef ThunkFunc0<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_timeoutGet, UInt32> TFunc_Instances_Socket_timeoutGet;
typedef ThunkFunc1<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_timeoutSet, const Value, UInt32> TFunc_Instances_Socket_timeoutSet;
typedef ThunkFunc0<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_close, const Value> TFunc_Instances_Socket_close;
typedef ThunkFunc2<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_connect, const Value, const ASString&, SInt32> TFunc_Instances_Socket_connect;
typedef ThunkFunc0<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_flush, const Value> TFunc_Instances_Socket_flush;
typedef ThunkFunc0<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_readBoolean, bool> TFunc_Instances_Socket_readBoolean;
typedef ThunkFunc0<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_readByte, SInt32> TFunc_Instances_Socket_readByte;
typedef ThunkFunc3<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_readBytes, const Value, Instances::fl_utils::ByteArray*, UInt32, UInt32> TFunc_Instances_Socket_readBytes;
typedef ThunkFunc0<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_readDouble, Value::Number> TFunc_Instances_Socket_readDouble;
typedef ThunkFunc0<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_readFloat, Value::Number> TFunc_Instances_Socket_readFloat;
typedef ThunkFunc0<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_readInt, SInt32> TFunc_Instances_Socket_readInt;
typedef ThunkFunc2<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_readMultiByte, ASString, UInt32, const ASString&> TFunc_Instances_Socket_readMultiByte;
typedef ThunkFunc0<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_readObject, Value> TFunc_Instances_Socket_readObject;
typedef ThunkFunc0<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_readShort, SInt32> TFunc_Instances_Socket_readShort;
typedef ThunkFunc0<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_readUnsignedByte, UInt32> TFunc_Instances_Socket_readUnsignedByte;
typedef ThunkFunc0<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_readUnsignedInt, UInt32> TFunc_Instances_Socket_readUnsignedInt;
typedef ThunkFunc0<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_readUnsignedShort, UInt32> TFunc_Instances_Socket_readUnsignedShort;
typedef ThunkFunc0<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_readUTF, ASString> TFunc_Instances_Socket_readUTF;
typedef ThunkFunc1<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_readUTFBytes, ASString, UInt32> TFunc_Instances_Socket_readUTFBytes;
typedef ThunkFunc1<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_writeBoolean, const Value, bool> TFunc_Instances_Socket_writeBoolean;
typedef ThunkFunc1<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_writeByte, const Value, SInt32> TFunc_Instances_Socket_writeByte;
typedef ThunkFunc3<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_writeBytes, const Value, Instances::fl_utils::ByteArray*, UInt32, UInt32> TFunc_Instances_Socket_writeBytes;
typedef ThunkFunc1<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_writeDouble, const Value, Value::Number> TFunc_Instances_Socket_writeDouble;
typedef ThunkFunc1<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_writeFloat, const Value, Value::Number> TFunc_Instances_Socket_writeFloat;
typedef ThunkFunc1<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_writeInt, const Value, SInt32> TFunc_Instances_Socket_writeInt;
typedef ThunkFunc2<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_writeMultiByte, const Value, const ASString&, const ASString&> TFunc_Instances_Socket_writeMultiByte;
typedef ThunkFunc1<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_writeObject, const Value, const Value&> TFunc_Instances_Socket_writeObject;
typedef ThunkFunc1<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_writeShort, const Value, SInt32> TFunc_Instances_Socket_writeShort;
typedef ThunkFunc1<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_writeUnsignedInt, const Value, UInt32> TFunc_Instances_Socket_writeUnsignedInt;
typedef ThunkFunc1<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_writeUTF, const Value, const ASString&> TFunc_Instances_Socket_writeUTF;
typedef ThunkFunc1<Instances::fl_net::Socket, Instances::fl_net::Socket::mid_writeUTFBytes, const Value, const ASString&> TFunc_Instances_Socket_writeUTFBytes;

template <> const TFunc_Instances_Socket_bytesAvailableGet::TMethod TFunc_Instances_Socket_bytesAvailableGet::Method = &Instances::fl_net::Socket::bytesAvailableGet;
template <> const TFunc_Instances_Socket_bytesPendingGet::TMethod TFunc_Instances_Socket_bytesPendingGet::Method = &Instances::fl_net::Socket::bytesPendingGet;
template <> const TFunc_Instances_Socket_connectedGet::TMethod TFunc_Instances_Socket_connectedGet::Method = &Instances::fl_net::Socket::connectedGet;
template <> const TFunc_Instances_Socket_endianGet::TMethod TFunc_Instances_Socket_endianGet::Method = &Instances::fl_net::Socket::endianGet;
template <> const TFunc_Instances_Socket_endianSet::TMethod TFunc_Instances_Socket_endianSet::Method = &Instances::fl_net::Socket::endianSet;
template <> const TFunc_Instances_Socket_localAddressGet::TMethod TFunc_Instances_Socket_localAddressGet::Method = &Instances::fl_net::Socket::localAddressGet;
template <> const TFunc_Instances_Socket_localPortGet::TMethod TFunc_Instances_Socket_localPortGet::Method = &Instances::fl_net::Socket::localPortGet;
template <> const TFunc_Instances_Socket_objectEncodingGet::TMethod TFunc_Instances_Socket_objectEncodingGet::Method = &Instances::fl_net::Socket::objectEncodingGet;
template <> const TFunc_Instances_Socket_objectEncodingSet::TMethod TFunc_Instances_Socket_objectEncodingSet::Method = &Instances::fl_net::Socket::objectEncodingSet;
template <> const TFunc_Instances_Socket_remoteAddressGet::TMethod TFunc_Instances_Socket_remoteAddressGet::Method = &Instances::fl_net::Socket::remoteAddressGet;
template <> const TFunc_Instances_Socket_remotePortGet::TMethod TFunc_Instances_Socket_remotePortGet::Method = &Instances::fl_net::Socket::remotePortGet;
template <> const TFunc_Instances_Socket_timeoutGet::TMethod TFunc_Instances_Socket_timeoutGet::Method = &Instances::fl_net::Socket::timeoutGet;
template <> const TFunc_Instances_Socket_timeoutSet::TMethod TFunc_Instances_Socket_timeoutSet::Method = &Instances::fl_net::Socket::timeoutSet;
template <> const TFunc_Instances_Socket_close::TMethod TFunc_Instances_Socket_close::Method = &Instances::fl_net::Socket::close;
template <> const TFunc_Instances_Socket_connect::TMethod TFunc_Instances_Socket_connect::Method = &Instances::fl_net::Socket::connect;
template <> const TFunc_Instances_Socket_flush::TMethod TFunc_Instances_Socket_flush::Method = &Instances::fl_net::Socket::flush;
template <> const TFunc_Instances_Socket_readBoolean::TMethod TFunc_Instances_Socket_readBoolean::Method = &Instances::fl_net::Socket::readBoolean;
template <> const TFunc_Instances_Socket_readByte::TMethod TFunc_Instances_Socket_readByte::Method = &Instances::fl_net::Socket::readByte;
template <> const TFunc_Instances_Socket_readBytes::TMethod TFunc_Instances_Socket_readBytes::Method = &Instances::fl_net::Socket::readBytes;
template <> const TFunc_Instances_Socket_readDouble::TMethod TFunc_Instances_Socket_readDouble::Method = &Instances::fl_net::Socket::readDouble;
template <> const TFunc_Instances_Socket_readFloat::TMethod TFunc_Instances_Socket_readFloat::Method = &Instances::fl_net::Socket::readFloat;
template <> const TFunc_Instances_Socket_readInt::TMethod TFunc_Instances_Socket_readInt::Method = &Instances::fl_net::Socket::readInt;
template <> const TFunc_Instances_Socket_readMultiByte::TMethod TFunc_Instances_Socket_readMultiByte::Method = &Instances::fl_net::Socket::readMultiByte;
template <> const TFunc_Instances_Socket_readObject::TMethod TFunc_Instances_Socket_readObject::Method = &Instances::fl_net::Socket::readObject;
template <> const TFunc_Instances_Socket_readShort::TMethod TFunc_Instances_Socket_readShort::Method = &Instances::fl_net::Socket::readShort;
template <> const TFunc_Instances_Socket_readUnsignedByte::TMethod TFunc_Instances_Socket_readUnsignedByte::Method = &Instances::fl_net::Socket::readUnsignedByte;
template <> const TFunc_Instances_Socket_readUnsignedInt::TMethod TFunc_Instances_Socket_readUnsignedInt::Method = &Instances::fl_net::Socket::readUnsignedInt;
template <> const TFunc_Instances_Socket_readUnsignedShort::TMethod TFunc_Instances_Socket_readUnsignedShort::Method = &Instances::fl_net::Socket::readUnsignedShort;
template <> const TFunc_Instances_Socket_readUTF::TMethod TFunc_Instances_Socket_readUTF::Method = &Instances::fl_net::Socket::readUTF;
template <> const TFunc_Instances_Socket_readUTFBytes::TMethod TFunc_Instances_Socket_readUTFBytes::Method = &Instances::fl_net::Socket::readUTFBytes;
template <> const TFunc_Instances_Socket_writeBoolean::TMethod TFunc_Instances_Socket_writeBoolean::Method = &Instances::fl_net::Socket::writeBoolean;
template <> const TFunc_Instances_Socket_writeByte::TMethod TFunc_Instances_Socket_writeByte::Method = &Instances::fl_net::Socket::writeByte;
template <> const TFunc_Instances_Socket_writeBytes::TMethod TFunc_Instances_Socket_writeBytes::Method = &Instances::fl_net::Socket::writeBytes;
template <> const TFunc_Instances_Socket_writeDouble::TMethod TFunc_Instances_Socket_writeDouble::Method = &Instances::fl_net::Socket::writeDouble;
template <> const TFunc_Instances_Socket_writeFloat::TMethod TFunc_Instances_Socket_writeFloat::Method = &Instances::fl_net::Socket::writeFloat;
template <> const TFunc_Instances_Socket_writeInt::TMethod TFunc_Instances_Socket_writeInt::Method = &Instances::fl_net::Socket::writeInt;
template <> const TFunc_Instances_Socket_writeMultiByte::TMethod TFunc_Instances_Socket_writeMultiByte::Method = &Instances::fl_net::Socket::writeMultiByte;
template <> const TFunc_Instances_Socket_writeObject::TMethod TFunc_Instances_Socket_writeObject::Method = &Instances::fl_net::Socket::writeObject;
template <> const TFunc_Instances_Socket_writeShort::TMethod TFunc_Instances_Socket_writeShort::Method = &Instances::fl_net::Socket::writeShort;
template <> const TFunc_Instances_Socket_writeUnsignedInt::TMethod TFunc_Instances_Socket_writeUnsignedInt::Method = &Instances::fl_net::Socket::writeUnsignedInt;
template <> const TFunc_Instances_Socket_writeUTF::TMethod TFunc_Instances_Socket_writeUTF::Method = &Instances::fl_net::Socket::writeUTF;
template <> const TFunc_Instances_Socket_writeUTFBytes::TMethod TFunc_Instances_Socket_writeUTFBytes::Method = &Instances::fl_net::Socket::writeUTFBytes;

namespace Instances { namespace fl_net
{
    Socket::Socket(InstanceTraits::Traits& t)
    : Instances::fl_events::EventDispatcher(t)
//##protect##"instance::Socket::Socket()$data"
    , ObjectEncodingValue(encDefault)
    , EndianValue(endianBig)
//##protect##"instance::Socket::Socket()$data"
    {
//##protect##"instance::Socket::Socket()$code"
#ifdef SF_ENABLE_SOCKETS
        ASVM& vm = static_cast<ASVM&>(GetVM());
        SockMgr = vm.GetMovieRoot()->AddSocket(true, this);
#endif
//##protect##"instance::Socket::Socket()$code"
    }

    void Socket::bytesAvailableGet(UInt32& result)
    {
//##protect##"instance::Socket::bytesAvailableGet()"
#ifdef SF_ENABLE_SOCKETS
        result = SockMgr->GetBytesAvailable();
#else
        SF_UNUSED(result);
#endif
//##protect##"instance::Socket::bytesAvailableGet()"
    }
    void Socket::bytesPendingGet(UInt32& result)
    {
//##protect##"instance::Socket::bytesPendingGet()"
#ifdef SF_ENABLE_SOCKETS
        result = SockMgr->GetBytesPending();
#else
        SF_UNUSED(result);
#endif
//##protect##"instance::Socket::bytesPendingGet()"
    }
    void Socket::connectedGet(bool& result)
    {
//##protect##"instance::Socket::connectedGet()"
#ifdef SF_ENABLE_SOCKETS
        result = SockMgr->IsRunning();
#else
        SF_UNUSED(result);
#endif
//##protect##"instance::Socket::connectedGet()"
    }
    void Socket::endianGet(ASString& result)
    {
//##protect##"instance::Socket::endianGet()"
        SF_UNUSED1(result);
#ifdef SF_ENABLE_SOCKETS
        StringManager& mgr = GetVM().GetStringManager();
        mgr.CreateConstString(EndianValue == endianBig ? "bigEndian" : "littleEndian");
#endif
//##protect##"instance::Socket::endianGet()"
    }
    void Socket::endianSet(const Value& result, const ASString& value)
    {
//##protect##"instance::Socket::endianSet()"
        SF_UNUSED1(result);

        if (value == "bigEndian")
        {
            EndianValue = endianBig;
        }
        else if (value == "littleEndian")
        {
            EndianValue = endianLittle;
        }
        else
        {
            return GetVM().ThrowArgumentError(VM::Error(VM::eInvalidArgumentError, GetVM() SF_DEBUG_ARG(value.ToCStr())));
        }
//##protect##"instance::Socket::endianSet()"
    }
    void Socket::localAddressGet(ASString& result)
    {
//##protect##"instance::Socket::localAddressGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("instance::Socket::localAddressGet()");
//##protect##"instance::Socket::localAddressGet()"
    }
    void Socket::localPortGet(SInt32& result)
    {
//##protect##"instance::Socket::localPortGet()"
#ifdef SF_ENABLE_SOCKETS
        result = SockMgr->GetPort();
#else
        SF_UNUSED(result);
#endif
//##protect##"instance::Socket::localPortGet()"
    }
    void Socket::objectEncodingGet(UInt32& result)
    {
//##protect##"instance::Socket::objectEncodingGet()"
        result = ObjectEncodingValue;
//##protect##"instance::Socket::objectEncodingGet()"
    }
    void Socket::objectEncodingSet(const Value& result, UInt32 value)
    {
//##protect##"instance::Socket::objectEncodingSet()"
        SF_UNUSED1(result);
        ObjectEncodingValue = static_cast<EncodingType>(value);
//##protect##"instance::Socket::objectEncodingSet()"
    }
    void Socket::remoteAddressGet(ASString& result)
    {
//##protect##"instance::Socket::remoteAddressGet()"
#ifdef SF_ENABLE_SOCKETS
        result = SockMgr->GetAddress().ToCStr();
#else
        SF_UNUSED(result);
#endif
//##protect##"instance::Socket::remoteAddressGet()"
    }
    void Socket::remotePortGet(SInt32& result)
    {
//##protect##"instance::Socket::remotePortGet()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("instance::Socket::remotePortGet()");
//##protect##"instance::Socket::remotePortGet()"
    }
    void Socket::timeoutGet(UInt32& result)
    {
//##protect##"instance::Socket::timeoutGet()"
#ifdef SF_ENABLE_SOCKETS
        result = SockMgr->GetConnectTimeout();
#else
        SF_UNUSED(result);
#endif
//##protect##"instance::Socket::timeoutGet()"
    }
    void Socket::timeoutSet(const Value& result, UInt32 value)
    {
//##protect##"instance::Socket::timeoutSet()"
        SF_UNUSED1(result);
#ifdef SF_ENABLE_SOCKETS
        SockMgr->SetConnectTimeout(value);
#else
        SF_UNUSED2(result, value);
#endif
//##protect##"instance::Socket::timeoutSet()"
    }
    void Socket::close(const Value& result)
    {
//##protect##"instance::Socket::close()"
        SF_UNUSED1(result);
#ifdef SF_ENABLE_SOCKETS
        if (!SockMgr->IsRunning())
        {
            ThrowIOError();
            return;
        }
        SockMgr->Uninit();
#endif
//##protect##"instance::Socket::close()"
    }
    void Socket::connect(const Value& result, const ASString& host, SInt32 port)
    {
//##protect##"instance::Socket::connect()"
        SF_UNUSED(result);
#ifdef SF_ENABLE_SOCKETS
        bool success = SockMgr->Init(host.ToCStr(), port);
#else
        bool success = false;
        SF_UNUSED2(host, port);
#endif
        SF_UNUSED1(success);
        SF_ASSERT(success); // report with event (if host was specified) or exception (no host was specified)
//##protect##"instance::Socket::connect()"
    }
    void Socket::flush(const Value& result)
    {
//##protect##"instance::Socket::flush()"
        // Don't need to call this. Data is flushed automatically.
        SF_UNUSED1(result);
//##protect##"instance::Socket::flush()"
    }
    void Socket::readBoolean(bool& result)
    {
//##protect##"instance::Socket::readBoolean()"
#ifdef SF_ENABLE_SOCKETS
        if (!SockMgr->IsRunning())
        {
            ExecuteIOErrorEvent("AS3 Net Socket: Attempting to read from closed socket");
            ThrowIOError();
            return;
        }
        if (!SockMgr->ReadBool(&result))
        {
            ExecuteIOErrorEvent("AS3 Net Socket: Failed to read Boolean");
            ThrowEOFError();
            return;
        }
#else
        SF_UNUSED(result);
#endif
//##protect##"instance::Socket::readBoolean()"
    }
    void Socket::readByte(SInt32& result)
    {
//##protect##"instance::Socket::readByte()"
#ifdef SF_ENABLE_SOCKETS
        if (!SockMgr->IsRunning())
        {
            ExecuteIOErrorEvent("AS3 Net Socket: Attempting to read from closed socket");
            ThrowIOError();
            return;
        }

        SByte readValue;
        if (!SockMgr->ReadByte(&readValue))
        {
            ExecuteIOErrorEvent("AS3 Net Socket: Failed to read Byte");
            ThrowEOFError();
            return;
        }
        result = readValue;
#else
        SF_UNUSED(result);
#endif
//##protect##"instance::Socket::readByte()"
    }
    void Socket::readBytes(const Value& result, Instances::fl_utils::ByteArray* bytes, UInt32 offset, UInt32 length)
    {
//##protect##"instance::Socket::readBytes()"
        SF_UNUSED1(result);
#ifdef SF_ENABLE_SOCKETS
        if (!SockMgr->IsRunning())
        {
            ExecuteIOErrorEvent("AS3 Net Socket: Attempting to read from closed socket");
            ThrowIOError();
            return;
        }

        Array<SByte> bytesRead;
        if (!SockMgr->ReadBytes(&bytesRead, length))
        {
            ExecuteIOErrorEvent("AS3 Net Socket: Failed to read Bytes");
            ThrowEOFError();
            return;
        }

        // Adjust size of the output array.
        if (offset + length >= bytesRead.GetSize())
        {
            bytes->lengthSet(offset + length);
        }

        bytes->positionSet(offset);
        for (UPInt i = 0; i < bytesRead.GetSize(); ++i)
        {
            bytes->writeByte(bytesRead[i]);
        }
#else
        SF_UNUSED3(bytes, offset, length);
#endif
//##protect##"instance::Socket::readBytes()"
    }
    void Socket::readDouble(Value::Number& result)
    {
//##protect##"instance::Socket::readDouble()"
#ifdef SF_ENABLE_SOCKETS
        if (!SockMgr->IsRunning())
        {
            ExecuteIOErrorEvent("AS3 Net Socket: Attempting to read from closed socket");
            ThrowIOError();
            return;
        }

        if (!SockMgr->ReadDouble(&result))
        {
            ExecuteIOErrorEvent("AS3 Net Socket: Failed to read Double");
            ThrowEOFError();
            return;
        }
        AdjustByteOrder(result);
#else
        SF_UNUSED(result);
#endif

//##protect##"instance::Socket::readDouble()"
    }
    void Socket::readFloat(Value::Number& result)
    {
//##protect##"instance::Socket::readFloat()"
#ifdef SF_ENABLE_SOCKETS
        if (!SockMgr->IsRunning())
        {
            ExecuteIOErrorEvent("AS3 Net Socket: Attempting to read from closed socket");
            ThrowIOError();
            return;
        }

        float valueRead;
        if (!SockMgr->ReadFloat(&valueRead))
        {
            ExecuteIOErrorEvent("AS3 Net Socket: Failed to read Float");
            ThrowEOFError();
            return;
        }
        AdjustByteOrder(valueRead);
        result = valueRead;
#else
        SF_UNUSED(result);
#endif
//##protect##"instance::Socket::readFloat()"
    }
    void Socket::readInt(SInt32& result)
    {
//##protect##"instance::Socket::readInt()"
#ifdef SF_ENABLE_SOCKETS
        if (!SockMgr->IsRunning())
        {
            ExecuteIOErrorEvent("AS3 Net Socket: Attempting to read from closed socket");
            ThrowIOError();
            return;
        }

        if (!SockMgr->ReadInt(&result))
        {
            ExecuteIOErrorEvent("AS3 Net Socket: Failed to read Int");
            ThrowEOFError();
            return;
        }
        AdjustByteOrder(result);
#else
        SF_UNUSED(result);
#endif
//##protect##"instance::Socket::readInt()"
    }
    void Socket::readMultiByte(ASString& result, UInt32 length, const ASString& charSet)
    {
//##protect##"instance::Socket::readMultiByte()"
#ifdef SF_ENABLE_SOCKETS
        if (!SockMgr->IsRunning())
        {
            ExecuteIOErrorEvent("AS3 Net Socket: Attempting to read from closed socket");
            ThrowIOError();
            return;
        }

        Array<SByte> bytesRead;
        if (!SockMgr->ReadBytes(&bytesRead, length))
        {
            ExecuteIOErrorEvent("AS3 Net Socket: Failed to read Bytes");
            ThrowEOFError();
            return;
        }

        VM& vm = GetVM();
        for (unsigned i = 0; ASCII_Names[i]; ++i)
		{
			if (charSet == ASCII_Names[i])
			{
                UInt32 numBytes = Alg::Min(length, static_cast<UInt32>(bytesRead.GetSize()));
				result = vm.GetStringManager().CreateString(reinterpret_cast<char*>(bytesRead.GetDataPtr()), numBytes);
				return;
			}
		}

		for (unsigned i = 0; UTF8_Names[i]; ++i)
		{
			if (charSet == UTF8_Names[i])
			{
                UPInt numBytes = bytesRead.GetSize();
                // Check for UTF8 BOM.
                if (numBytes > 2 && IsUTF8BOM(reinterpret_cast<char*>(bytesRead.GetDataPtr())))
                {
                    numBytes -= 3;
                }

                result = vm.GetStringManager().CreateString(reinterpret_cast<char*>(bytesRead.GetDataPtr()), numBytes);
				return;
			}
		}

		/*
		// No ISO support for the time being
		for (unsigned i = 0; ISO_Names[i]; ++i)
		{
			if (charSet == ISO_Names[i])
			{
                UPInt numBytes = Alg::Min(length, bytesRead.GetSize());
                result = GetVM().GetStringManager().CreateString(reinterpret_cast<char*>(bytesRead.GetDataPtr()), numBytes);
				return;
			}
		}
		*/

		// In case of UTF16.
		for (unsigned i = 0; UTF16_Names[i]; ++i)
		{
			if (charSet == UTF16_Names[i])
			{
                UInt32 numBytes = Alg::Min(length, static_cast<UInt32>(bytesRead.GetSize()));
				result = vm.GetStringManager().CreateString(reinterpret_cast<const wchar_t*>(bytesRead.GetDataPtr()), numBytes);
				return;
			}
		}

		vm.ThrowTypeError(VM::Error(VM::eInvalidArgumentError, vm SF_DEBUG_ARG("charSet")));
#else
        SF_UNUSED3(result, length, charSet);
#endif
//##protect##"instance::Socket::readMultiByte()"
    }
    void Socket::readObject(Value& result)
    {
//##protect##"instance::Socket::readObject()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("instance::Socket::readObject()");
//##protect##"instance::Socket::readObject()"
    }
    void Socket::readShort(SInt32& result)
    {
//##protect##"instance::Socket::readShort()"
#ifdef SF_ENABLE_SOCKETS
        if (!SockMgr->IsRunning())
        {
            ExecuteIOErrorEvent("AS3 Net Socket: Attempting to read from closed socket");
            ThrowIOError();
            return;
        }

        SInt16 readData;
        if (!SockMgr->ReadShort(&readData))
        {
            ExecuteIOErrorEvent("AS3 Net Socket: Failed to read Short");
            ThrowEOFError();
            return;
        }
        AdjustByteOrder(readData);
        result = static_cast<SInt32>(readData);
#else
        SF_UNUSED(result);
#endif
//##protect##"instance::Socket::readShort()"
    }
    void Socket::readUnsignedByte(UInt32& result)
    {
//##protect##"instance::Socket::readUnsignedByte()"
#ifdef SF_ENABLE_SOCKETS
        if (!SockMgr->IsRunning())
        {
            ExecuteIOErrorEvent("AS3 Net Socket: Attempting to read from closed socket");
            ThrowIOError();
            return;
        }

        SByte readValue;
        if (!SockMgr->ReadByte(&readValue))
        {
            ExecuteIOErrorEvent("AS3 Net Socket: Failed to read Unsigned Byte");
            ThrowEOFError();
            return;
        }
        result = static_cast<UInt32>(readValue);
#else
        SF_UNUSED(result);
#endif
//##protect##"instance::Socket::readUnsignedByte()"
    }
    void Socket::readUnsignedInt(UInt32& result)
    {
//##protect##"instance::Socket::readUnsignedInt()"
#ifdef SF_ENABLE_SOCKETS
        if (!SockMgr->IsRunning())
        {
            ExecuteIOErrorEvent("AS3 Net Socket: Attempting to read from closed socket");
            ThrowIOError();
            return;
        }

        SInt32 readValue;
        if (!SockMgr->ReadInt(&readValue))
        {
            ExecuteIOErrorEvent("AS3 Net Socket: Failed to read Unsigned Int");
            ThrowEOFError();
            return;
        }
        AdjustByteOrder(readValue);
        result = static_cast<UInt32>(readValue);
#else
        SF_UNUSED(result);
#endif
//##protect##"instance::Socket::readUnsignedInt()"
    }
    void Socket::readUnsignedShort(UInt32& result)
    {
//##protect##"instance::Socket::readUnsignedShort()"
#ifdef SF_ENABLE_SOCKETS
        if (!SockMgr->IsRunning())
        {
            ExecuteIOErrorEvent("AS3 Net Socket: Attempting to read from closed socket");
            ThrowIOError();
            return;
        }

        SInt16 readData;
        if (!SockMgr->ReadShort(&readData))
        {
            ExecuteIOErrorEvent("AS3 Net Socket: Failed to read Unsigned Short");
            ThrowEOFError();
            return;
        }
        AdjustByteOrder(readData);
        result = static_cast<UInt32>(readData);
#else
        SF_UNUSED(result);
#endif
//##protect##"instance::Socket::readUnsignedShort()"
    }
    void Socket::readUTF(ASString& result)
    {
//##protect##"instance::Socket::readUTF()"
#ifdef SF_ENABLE_SOCKETS
        if (!SockMgr->IsRunning())
        {
            ExecuteIOErrorEvent("AS3 Net Socket: Attempting to read from closed socket");
            ThrowIOError();
            return;
        }

        SInt16 length;
        if (!SockMgr->ReadShort(&length))
        {
            ExecuteIOErrorEvent("AS3 Net Socket: Failed to read UTF String");
            ThrowEOFError();
            return;
        }

        Array<SByte> bytesRead;
        if (!SockMgr->ReadBytes(&bytesRead, length))
        {
            ExecuteIOErrorEvent("AS3 Net Socket: Failed to read UTF String");
            ThrowEOFError();
            return;
        }

        SInt16 numBytes = Alg::Min(length, static_cast<SInt16>(bytesRead.GetSize()));
		result = GetVM().GetStringManager().CreateString(reinterpret_cast<const wchar_t*>(bytesRead.GetDataPtr()), numBytes);
#else
        SF_UNUSED(result);
#endif
//##protect##"instance::Socket::readUTF()"
    }
    void Socket::readUTFBytes(ASString& result, UInt32 length)
    {
//##protect##"instance::Socket::readUTFBytes()"
#ifdef SF_ENABLE_SOCKETS
        if (!SockMgr->IsRunning())
        {
            ExecuteIOErrorEvent("AS3 Net Socket: Attempting to read from closed socket");
            ThrowIOError();
            return;
        }

        Array<SByte> bytesRead;
        if (!SockMgr->ReadBytes(&bytesRead, length))
        {
            ExecuteIOErrorEvent("AS3 Net Socket: Failed to read UTF String");
            ThrowEOFError();
            return;
        }

        UInt32 numBytes = Alg::Min(length, static_cast<UInt32>(bytesRead.GetSize()));
        result = GetVM().GetStringManager().CreateString(reinterpret_cast<char*>(bytesRead.GetDataPtr()), numBytes);
#else
        SF_UNUSED2(result, length);
#endif
//##protect##"instance::Socket::readUTFBytes()"
    }
    void Socket::writeBoolean(const Value& result, bool value)
    {
//##protect##"instance::Socket::writeBoolean()"
        SF_UNUSED1(result);
#ifdef SF_ENABLE_SOCKETS
        if (!SockMgr->IsRunning())
        {
            ExecuteIOErrorEvent("AS3 Net Socket: Attempting to write to closed socket");
            ThrowIOError();
            return;
        }
        SockMgr->SendBool(value);
#else
        SF_UNUSED(value);
#endif
//##protect##"instance::Socket::writeBoolean()"
    }
    void Socket::writeByte(const Value& result, SInt32 value)
    {
//##protect##"instance::Socket::writeByte()"
        SF_UNUSED1(result);
#ifdef SF_ENABLE_SOCKETS
        if (!SockMgr->IsRunning())
        {
            ExecuteIOErrorEvent("AS3 Net Socket: Attempting to write to closed socket");
            ThrowIOError();
            return;
        }
        SockMgr->SendByte(static_cast<SByte>(value));
#else
        SF_UNUSED(value);
#endif
//##protect##"instance::Socket::writeByte()"
    }
    void Socket::writeBytes(const Value& result, Instances::fl_utils::ByteArray* bytes, UInt32 offset, UInt32 length)
    {
//##protect##"instance::Socket::writeBytes()"
        SF_UNUSED1(result);
#ifdef SF_ENABLE_SOCKETS
        if (!SockMgr->IsRunning())
        {
            ExecuteIOErrorEvent("AS3 Net Socket: Attempting to write to closed socket");
            ThrowIOError();
            return;
        }

        if (bytes == NULL)
        {
            return;
        }

        if (bytes->GetLength() < offset)
        {
            offset = bytes->GetLength();
        }

        if (length == 0)
        {
            length = bytes->GetLength() - offset;
        }

        if (length > bytes->GetLength() - offset)
        {
            GetVM().ThrowRangeError(VM::Error(VM::eParamRangeError, GetVM()));
            return;
        }

        if (length > 0)
        {
            SockMgr->SendBytes(static_cast<const SByte*>(bytes->GetDataPtr()) + offset, length);
        }
#else
        SF_UNUSED3(bytes, offset, length);
#endif
//##protect##"instance::Socket::writeBytes()"
    }
    void Socket::writeDouble(const Value& result, Value::Number value)
    {
//##protect##"instance::Socket::writeDouble()"
        SF_UNUSED1(result);
#ifdef SF_ENABLE_SOCKETS
        if (!SockMgr->IsRunning())
        {
            ExecuteIOErrorEvent("AS3 Net Socket: Attempting to write to closed socket");
            ThrowIOError();
            return;
        }
        AdjustByteOrder(value);
        SockMgr->SendDouble(value);
#else
        SF_UNUSED(value);
#endif
//##protect##"instance::Socket::writeDouble()"
    }
    void Socket::writeFloat(const Value& result, Value::Number value)
    {
//##protect##"instance::Socket::writeFloat()"
        SF_UNUSED1(result);
#ifdef SF_ENABLE_SOCKETS
        if (!SockMgr->IsRunning())
        {
            ExecuteIOErrorEvent("AS3 Net Socket: Attempting to write to closed socket");
            ThrowIOError();
            return;
        }
        float v = static_cast<float>(value);
        AdjustByteOrder(v);
        SockMgr->SendFloat(v);
#else
        SF_UNUSED(value);
#endif
//##protect##"instance::Socket::writeFloat()"
    }
    void Socket::writeInt(const Value& result, SInt32 value)
    {
//##protect##"instance::Socket::writeInt()"
        SF_UNUSED1(result);
#ifdef SF_ENABLE_SOCKETS
        if (!SockMgr->IsRunning())
        {
            ExecuteIOErrorEvent("AS3 Net Socket: Attempting to write to closed socket");
            ThrowIOError();
            return;
        }
        AdjustByteOrder(value);
        SockMgr->SendInt(value);
#else
        SF_UNUSED(value);
#endif
//##protect##"instance::Socket::writeInt()"
    }
    void Socket::writeMultiByte(const Value& result, const ASString& value, const ASString& charSet)
    {
//##protect##"instance::Socket::writeMultiByte()"
        SF_UNUSED1(result);
#ifdef SF_ENABLE_SOCKETS
        if (!SockMgr->IsRunning())
        {
            ExecuteIOErrorEvent("AS3 Net Socket: Attempting to write to closed socket");
            ThrowIOError();
            return;
        }

        // In case of ASCII.
        // No ISO support for the time being.

        // In case of UTF-8.
        for (unsigned i = 0; UTF8_Names[i]; ++i)
        {
            if (charSet == UTF8_Names[i])
            {
                SockMgr->SendBytes((const SByte*)value.ToCStr(), value.GetSize());
                return;
            }
        }

        // In case of ISO.
        // No ISO support for the time being.

        // In case of UTF16.
        for (unsigned i = 0; UTF16_Names[i]; ++i)
        {
            if (charSet == UTF16_Names[i])
            {
                WStringBuffer wbuff;

                wbuff.SetString(value.ToCStr(), value.GetSize());
                if (wbuff.GetLength() <= SF_MAX_UINT32)
                {
                    SockMgr->SendBytes((const SByte*)wbuff.GetBuffer(), static_cast<int>(wbuff.GetLength() * sizeof(wchar_t)));
                }
                return;
            }
        }

        VM& vm = GetVM();
        vm.ThrowTypeError(VM::Error(VM::eInvalidArgumentError, vm SF_DEBUG_ARG("charSet")));
#else
        SF_UNUSED2(value, charSet);
#endif
//##protect##"instance::Socket::writeMultiByte()"
    }
    void Socket::writeObject(const Value& result, const Value& object)
    {
//##protect##"instance::Socket::writeObject()"
        SF_UNUSED2(result, object);
        WARN_NOT_IMPLEMENTED("instance::Socket::writeObject()");
//##protect##"instance::Socket::writeObject()"
    }
    void Socket::writeShort(const Value& result, SInt32 value)
    {
//##protect##"instance::Socket::writeShort()"
        SF_UNUSED1(result);
#ifdef SF_ENABLE_SOCKETS
        if (!SockMgr->IsRunning())
        {
            ExecuteIOErrorEvent("AS3 Net Socket: Attempting to write to closed socket");
            ThrowIOError();
            return;
        }

        SInt16 v = static_cast<SInt16>(value);
        AdjustByteOrder(v);
        SockMgr->SendShort(v);
#else
        SF_UNUSED(value);
#endif
//##protect##"instance::Socket::writeShort()"
    }
    void Socket::writeUnsignedInt(const Value& result, UInt32 value)
    {
//##protect##"instance::Socket::writeUnsignedInt()"
        SF_UNUSED1(result);
#ifdef SF_ENABLE_SOCKETS
        if (!SockMgr->IsRunning())
        {
            ExecuteIOErrorEvent("AS3 Net Socket: Attempting to write to closed socket");
            ThrowIOError();
            return;
        }
        AdjustByteOrder(value);
        SockMgr->SendInt(static_cast<int>(value));
#else
        SF_UNUSED(value);
#endif
//##protect##"instance::Socket::writeUnsignedInt()"
    }
    void Socket::writeUTF(const Value& result, const ASString& value)
    {
//##protect##"instance::Socket::writeUTF()"
        SF_UNUSED1(result);
#ifdef SF_ENABLE_SOCKETS
        if (!SockMgr->IsRunning())
        {
            ExecuteIOErrorEvent("AS3 Net Socket: Attempting to write to closed socket");
            ThrowIOError();
            return;
        }

        if (value.GetSize() > 65535)
        {
            return GetVM().ThrowRangeError(VM::Error(VM::eNotImplementedError, GetVM() SF_DEBUG_ARG("ByteArray::writeUTF")));
        }

        const UInt16 size = static_cast<UInt16>(value.GetSize());
        SockMgr->SendShort(static_cast<SInt16>(size));
        SockMgr->SendBytes((const SByte*)value.ToCStr(), size);
#else
        SF_UNUSED(value);
#endif
//##protect##"instance::Socket::writeUTF()"
    }
    void Socket::writeUTFBytes(const Value& result, const ASString& value)
    {
//##protect##"instance::Socket::writeUTFBytes()"
        SF_UNUSED1(result);
#ifdef SF_ENABLE_SOCKETS
        if (!SockMgr->IsRunning())
        {
            ExecuteIOErrorEvent("AS3 Net Socket: Attempting to write to closed socket");
            ThrowIOError();
            return;
        }

        SockMgr->SendBytes((const SByte*)value.ToCStr(), value.GetSize());
#else
        SF_UNUSED(value);
#endif
//##protect##"instance::Socket::writeUTFBytes()"
    }

//##protect##"instance$methods"
	const char* Socket::ASCII_Names[] =
	{
		"us-ascii",
		"ANSI_X3.4-1968",
		"ANSI_X3.4-1986",
		"ascii",
		"cp367",
		"csASCII",
		"IBM367",
		"ISO_646.irv:1991",
		"ISO646-US",
		"iso-ir-6us",
		NULL
	};

	const char* Socket::UTF8_Names[] =
	{
		"utf-8",
		"unicode-1-1-utf-8",
		"unicode-2-0-utf-8",
		"x-unicode-2-0-utf-8",
		NULL
	};

	const char* Socket::UTF16_Names[] =
	{
		"unicode",
		"utf-16",
		NULL
	};

	/*
	// No ISO support for the time being.
	const char* ByteArray::ISO_Names[] =
	{
		"iso-8859-1",
		"cp819",
		"csISO",
		"Latin1",
		"ibm819",
		"iso_8859-1",
		"iso_8859-1:1987",
		"iso8859-1",
		"iso-ir-100, l1",
		"latin1",
		NULL
	};
	*/

    void Socket::AS3Constructor(unsigned argc, const Value* argv)
    {
        VM& vm = GetVM();
        Value hostIp;
        if (argc > 0 && !argv[0].IsNullOrUndefined())
        {
            hostIp = argv[0];
            hostIp.ToStringValue(vm.GetStringManager()).DoNotCheck();
        }
        else
        {
            hostIp.SetNull();
        }
        SInt32 port = 0;
        if (argc > 1 && !argv[1].IsNullOrUndefined())
        {
            port = argv[1];
        }
        if (!hostIp.IsNullOrUndefined())
        {
            connect(hostIp, port);
        }
    }

    void Socket::ThrowEOFError()
    {
        Value v;
        if (GetVM().ConstructBuiltinValue(v, "flash.errors.EOFError"))
        {
            GetVM().Throw(v);
        }
    }

    void Socket::ThrowIOError()
    {
        Value v;
        if (GetVM().ConstructBuiltinValue(v, "flash.errors.IOError"))
        {
            GetVM().Throw(v);
        }
    }

    void Socket::ExecuteCloseEvent()
    {
        VM& vm = GetVM();
        ASString evtName = vm.GetStringManager().CreateConstString("close");
        if (HasEventHandler(evtName, false))
        {
            SPtr<fl_events::Event> efe = CreateEventObject(evtName);
            efe->Target  = this;
            DispatchSingleEvent(efe, false);
        }
    }

    void Socket::ExecuteConnectEvent()
    {
        VM& vm = GetVM();
        ASString evtName = vm.GetStringManager().CreateConstString("connect");
        if (HasEventHandler(evtName, false))
        {
            SPtr<fl_events::Event> efe = CreateEventObject(evtName);
            efe->Target  = this;
            DispatchSingleEvent(efe, false);
        }
    }

    void Socket::ExecuteSocketDataEvent(UInt32 bytesLoaded, UInt32 totalBytes)
    {
        ASVM& asvm = static_cast<ASVM&>(GetVM());
        ASString evtName(asvm.GetBuiltin(AS3Builtin_socketData));
        if (HasEventHandler(evtName, false))
        {
            SPtr<fl_events::ProgressEvent> efe = CreateProgressEventObject(evtName);
            efe->Target = this;
            efe->SetBytesLoaded(bytesLoaded);
            efe->SetBytesTotal(totalBytes);
            DispatchSingleEvent(efe, false);
        }
    }

    void Socket::ExecuteIOErrorEvent(const char* errorStr)
    {
        SPtr<fl_events::IOErrorEvent> efe = CreateIOErrorEventObject(errorStr);
        efe->Target  = this;
        DispatchSingleEvent(efe, false);
    }

//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_net
{
    // const UInt16 Socket::tito[Socket::ThunkInfoNum] = {
    //    0, 1, 2, 3, 4, 6, 7, 8, 9, 11, 12, 13, 14, 16, 17, 20, 21, 22, 23, 27, 28, 29, 30, 33, 34, 35, 36, 37, 38, 39, 41, 43, 45, 49, 51, 53, 55, 58, 60, 62, 64, 66, 
    // };
    const TypeInfo* Socket::tit[68] = {
        &AS3::fl::uintTI, 
        &AS3::fl::uintTI, 
        &AS3::fl::BooleanTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::int_TI, 
        &AS3::fl::uintTI, 
        NULL, &AS3::fl::uintTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::int_TI, 
        &AS3::fl::uintTI, 
        NULL, &AS3::fl::uintTI, 
        NULL, 
        NULL, &AS3::fl::StringTI, &AS3::fl::int_TI, 
        NULL, 
        &AS3::fl::BooleanTI, 
        &AS3::fl::int_TI, 
        NULL, &AS3::fl_utils::ByteArrayTI, &AS3::fl::uintTI, &AS3::fl::uintTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::NumberTI, 
        &AS3::fl::int_TI, 
        &AS3::fl::StringTI, &AS3::fl::uintTI, &AS3::fl::StringTI, 
        NULL, 
        &AS3::fl::int_TI, 
        &AS3::fl::uintTI, 
        &AS3::fl::uintTI, 
        &AS3::fl::uintTI, 
        &AS3::fl::StringTI, 
        &AS3::fl::StringTI, &AS3::fl::uintTI, 
        NULL, &AS3::fl::BooleanTI, 
        NULL, &AS3::fl::int_TI, 
        NULL, &AS3::fl_utils::ByteArrayTI, &AS3::fl::uintTI, &AS3::fl::uintTI, 
        NULL, &AS3::fl::NumberTI, 
        NULL, &AS3::fl::NumberTI, 
        NULL, &AS3::fl::int_TI, 
        NULL, &AS3::fl::StringTI, &AS3::fl::StringTI, 
        NULL, NULL, 
        NULL, &AS3::fl::int_TI, 
        NULL, &AS3::fl::uintTI, 
        NULL, &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
    };
    const ThunkInfo Socket::ti[Socket::ThunkInfoNum] = {
        {TFunc_Instances_Socket_bytesAvailableGet::Func, &Socket::tit[0], "bytesAvailable", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Socket_bytesPendingGet::Func, &Socket::tit[1], "bytesPending", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Socket_connectedGet::Func, &Socket::tit[2], "connected", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Socket_endianGet::Func, &Socket::tit[3], "endian", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Socket_endianSet::Func, &Socket::tit[4], "endian", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Socket_localAddressGet::Func, &Socket::tit[6], "localAddress", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Socket_localPortGet::Func, &Socket::tit[7], "localPort", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Socket_objectEncodingGet::Func, &Socket::tit[8], "objectEncoding", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Socket_objectEncodingSet::Func, &Socket::tit[9], "objectEncoding", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Socket_remoteAddressGet::Func, &Socket::tit[11], "remoteAddress", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Socket_remotePortGet::Func, &Socket::tit[12], "remotePort", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Socket_timeoutGet::Func, &Socket::tit[13], "timeout", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Socket_timeoutSet::Func, &Socket::tit[14], "timeout", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Socket_close::Func, &Socket::tit[16], "close", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Socket_connect::Func, &Socket::tit[17], "connect", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Instances_Socket_flush::Func, &Socket::tit[20], "flush", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Socket_readBoolean::Func, &Socket::tit[21], "readBoolean", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Socket_readByte::Func, &Socket::tit[22], "readByte", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Socket_readBytes::Func, &Socket::tit[23], "readBytes", NULL, Abc::NS_Public, CT_Method, 1, 3, 0, 0, NULL},
        {TFunc_Instances_Socket_readDouble::Func, &Socket::tit[27], "readDouble", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Socket_readFloat::Func, &Socket::tit[28], "readFloat", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Socket_readInt::Func, &Socket::tit[29], "readInt", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Socket_readMultiByte::Func, &Socket::tit[30], "readMultiByte", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Instances_Socket_readObject::Func, &Socket::tit[33], "readObject", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Socket_readShort::Func, &Socket::tit[34], "readShort", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Socket_readUnsignedByte::Func, &Socket::tit[35], "readUnsignedByte", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Socket_readUnsignedInt::Func, &Socket::tit[36], "readUnsignedInt", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Socket_readUnsignedShort::Func, &Socket::tit[37], "readUnsignedShort", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Socket_readUTF::Func, &Socket::tit[38], "readUTF", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_Socket_readUTFBytes::Func, &Socket::tit[39], "readUTFBytes", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Socket_writeBoolean::Func, &Socket::tit[41], "writeBoolean", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Socket_writeByte::Func, &Socket::tit[43], "writeByte", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Socket_writeBytes::Func, &Socket::tit[45], "writeBytes", NULL, Abc::NS_Public, CT_Method, 1, 3, 0, 0, NULL},
        {TFunc_Instances_Socket_writeDouble::Func, &Socket::tit[49], "writeDouble", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Socket_writeFloat::Func, &Socket::tit[51], "writeFloat", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Socket_writeInt::Func, &Socket::tit[53], "writeInt", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Socket_writeMultiByte::Func, &Socket::tit[55], "writeMultiByte", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Instances_Socket_writeObject::Func, &Socket::tit[58], "writeObject", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Socket_writeShort::Func, &Socket::tit[60], "writeShort", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Socket_writeUnsignedInt::Func, &Socket::tit[62], "writeUnsignedInt", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Socket_writeUTF::Func, &Socket::tit[64], "writeUTF", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_Socket_writeUTFBytes::Func, &Socket::tit[66], "writeUTFBytes", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
    };

    Socket::Socket(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"InstanceTraits::Socket::Socket()"
//##protect##"InstanceTraits::Socket::Socket()"

    }

    void Socket::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<Socket&>(t));
    }

//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits


namespace ClassTraits { namespace fl_net
{

    Socket::Socket(VM& vm, const ClassInfo& ci)
    : fl_events::EventDispatcher(vm, ci)
    {
//##protect##"ClassTraits::Socket::Socket()"
//##protect##"ClassTraits::Socket::Socket()"

    }

    Pickable<Traits> Socket::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) Socket(vm, AS3::fl_net::SocketCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_net::SocketCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_net
{
    const TypeInfo* SocketImplements[] = {
        &fl_utils::IDataInputTI,
        &fl_utils::IDataOutputTI,
        NULL
    };

    const TypeInfo SocketTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_net::Socket::InstanceType),
        0,
        0,
        InstanceTraits::fl_net::Socket::ThunkInfoNum,
        0,
        "Socket", "flash.net", &fl_events::EventDispatcherTI,
        SocketImplements
    };

    const ClassInfo SocketCI = {
        &SocketTI,
        ClassTraits::fl_net::Socket::MakeClassTraits,
        NULL,
        NULL,
        InstanceTraits::fl_net::Socket::ti,
        NULL,
    };
}; // namespace fl_net


}}} // namespace Scaleform { namespace GFx { namespace AS3

