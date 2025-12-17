//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Net_Socket.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Net_Socket_H
#define INC_AS3_Obj_Net_Socket_H

#include "../Events/AS3_Obj_Events_EventDispatcher.h"
//##protect##"includes"
#include "GFx/Net/GFx_Socket.h"
#include "GFx/AS3/AS3_SocketThreadMgr.h"

namespace Scaleform { namespace GFx { class SocketImplFactory; } }
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_net
{
    extern const TypeInfo SocketTI;
    extern const ClassInfo SocketCI;
} // namespace fl_net
namespace fl
{
    extern const TypeInfo uintTI;
    extern const ClassInfo uintCI;
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo int_TI;
    extern const ClassInfo int_CI;
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
    extern const TypeInfo anyTI;
    extern const ClassInfo anyCI;
} // namespace fl
namespace fl_utils
{
    extern const TypeInfo ByteArrayTI;
    extern const ClassInfo ByteArrayCI;
    extern const TypeInfo IDataInputTI;
    extern const ClassInfo IDataInputCI;
    extern const TypeInfo IDataOutputTI;
    extern const ClassInfo IDataOutputCI;
} // namespace fl_utils

namespace ClassTraits { namespace fl_net
{
    class Socket;
}}

namespace InstanceTraits { namespace fl_net
{
    class Socket;
}}

namespace Classes { namespace fl_net
{
    class Socket;
}}

//##protect##"forward_declaration"
namespace Instances
{
    namespace fl_utils
    {
        class ByteArray;
    }
}
//##protect##"forward_declaration"

namespace Instances { namespace fl_net
{
    class Socket : public Instances::fl_events::EventDispatcher
    {
#ifndef SF_OS_WII
        template <class TR> friend Pickable<typename TR::InstanceType> InstanceTraits::MakeInstance(TR& tr);
#endif
        friend class InstanceTraits::fl_net::Socket;
        
    public:
        typedef Socket SelfType;
        typedef Classes::fl_net::Socket ClassType;
        typedef InstanceTraits::fl_net::Socket TraitsType;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_net::SocketTI; }

#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Instances::fl_net::Socket"; }
#endif
#ifdef SF_OS_WII
    public:
#else
    protected:
#endif
        Socket(InstanceTraits::Traits& t);

//##protect##"instance$methods"
    public:
        enum EncodingType { encAMF0 = 0, encAMF3 = 3, encDefault = 3 };
        enum EndianType { endianBig = 0, endianLittle = 1 };

    public:
        virtual void AS3Constructor(unsigned argc, const Value* argv);

        static EndianType GetNativeEndian()
        {
#if (SF_BYTE_ORDER == SF_LITTLE_ENDIAN)
            return endianLittle;
#else
            return endianBig;
#endif
        }

        EndianType GetEndian() const
        {
            return EndianValue;
        }

        template <typename T>
        void AdjustByteOrder(T& v) const
        {
            if (GetEndian() != GetNativeEndian())
            {
                v = Alg::ByteUtil::SwapOrder(v);
            }
        }

        static bool IsUTF8BOM(const char* str)
        {
            return (UInt8)str[0] == 0xEF && (UInt8)str[1] == 0xBB && (UInt8)str[2] == 0xBF;
        }
        static bool IsUTF16BE(const char* str)
        {
            return (UInt8)str[0] == 0xFE && (UInt8)str[1] == 0xFF;
        }
        static bool IsUTF16LE(const char* str)
        {
            return (UInt8)str[0] == 0xFF && (UInt8)str[1] == 0xFE;
        }

        void ExecuteCloseEvent();
        void ExecuteConnectEvent();
        void ExecuteSocketDataEvent(UInt32 bytesLoaded, UInt32 totalBytes);
        void ExecuteIOErrorEvent(const char* errorStr);

    protected:
        void ThrowEOFError();
        void ThrowIOError();


//##protect##"instance$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_bytesAvailableGet, 
            mid_bytesPendingGet, 
            mid_connectedGet, 
            mid_endianGet, 
            mid_endianSet, 
            mid_localAddressGet, 
            mid_localPortGet, 
            mid_objectEncodingGet, 
            mid_objectEncodingSet, 
            mid_remoteAddressGet, 
            mid_remotePortGet, 
            mid_timeoutGet, 
            mid_timeoutSet, 
            mid_close, 
            mid_connect, 
            mid_flush, 
            mid_readBoolean, 
            mid_readByte, 
            mid_readBytes, 
            mid_readDouble, 
            mid_readFloat, 
            mid_readInt, 
            mid_readMultiByte, 
            mid_readObject, 
            mid_readShort, 
            mid_readUnsignedByte, 
            mid_readUnsignedInt, 
            mid_readUnsignedShort, 
            mid_readUTF, 
            mid_readUTFBytes, 
            mid_writeBoolean, 
            mid_writeByte, 
            mid_writeBytes, 
            mid_writeDouble, 
            mid_writeFloat, 
            mid_writeInt, 
            mid_writeMultiByte, 
            mid_writeObject, 
            mid_writeShort, 
            mid_writeUnsignedInt, 
            mid_writeUTF, 
            mid_writeUTFBytes, 
        };
        void bytesAvailableGet(UInt32& result);
        void bytesPendingGet(UInt32& result);
        void connectedGet(bool& result);
        void endianGet(ASString& result);
        void endianSet(const Value& result, const ASString& value);
        void localAddressGet(ASString& result);
        void localPortGet(SInt32& result);
        void objectEncodingGet(UInt32& result);
        void objectEncodingSet(const Value& result, UInt32 value);
        void remoteAddressGet(ASString& result);
        void remotePortGet(SInt32& result);
        void timeoutGet(UInt32& result);
        void timeoutSet(const Value& result, UInt32 value);
        void close(const Value& result);
        void connect(const Value& result, const ASString& host, SInt32 port);
        void flush(const Value& result);
        void readBoolean(bool& result);
        void readByte(SInt32& result);
        void readBytes(const Value& result, Instances::fl_utils::ByteArray* bytes, UInt32 offset = 0, UInt32 length = 0);
        void readDouble(Value::Number& result);
        void readFloat(Value::Number& result);
        void readInt(SInt32& result);
        void readMultiByte(ASString& result, UInt32 length, const ASString& charSet);
        void readObject(Value& result);
        void readShort(SInt32& result);
        void readUnsignedByte(UInt32& result);
        void readUnsignedInt(UInt32& result);
        void readUnsignedShort(UInt32& result);
        void readUTF(ASString& result);
        void readUTFBytes(ASString& result, UInt32 length);
        void writeBoolean(const Value& result, bool value);
        void writeByte(const Value& result, SInt32 value);
        void writeBytes(const Value& result, Instances::fl_utils::ByteArray* bytes, UInt32 offset = 0, UInt32 length = 0);
        void writeDouble(const Value& result, Value::Number value);
        void writeFloat(const Value& result, Value::Number value);
        void writeInt(const Value& result, SInt32 value);
        void writeMultiByte(const Value& result, const ASString& value, const ASString& charSet);
        void writeObject(const Value& result, const Value& object);
        void writeShort(const Value& result, SInt32 value);
        void writeUnsignedInt(const Value& result, UInt32 value);
        void writeUTF(const Value& result, const ASString& value);
        void writeUTFBytes(const Value& result, const ASString& value);

        // C++ friendly wrappers for AS3 methods.
        UInt32 bytesAvailableGet()
        {
            UInt32 result;
            bytesAvailableGet(result);
            return result;
        }
        UInt32 bytesPendingGet()
        {
            UInt32 result;
            bytesPendingGet(result);
            return result;
        }
        bool connectedGet()
        {
            bool result;
            connectedGet(result);
            return result;
        }
        ASString endianGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            endianGet(result);
            return result;
        }
        void endianSet(const ASString& value)
        {
            endianSet(Value::GetUndefined(), value);
        }
        ASString localAddressGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            localAddressGet(result);
            return result;
        }
        SInt32 localPortGet()
        {
            SInt32 result;
            localPortGet(result);
            return result;
        }
        UInt32 objectEncodingGet()
        {
            UInt32 result;
            objectEncodingGet(result);
            return result;
        }
        void objectEncodingSet(UInt32 value)
        {
            objectEncodingSet(Value::GetUndefined(), value);
        }
        ASString remoteAddressGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            remoteAddressGet(result);
            return result;
        }
        SInt32 remotePortGet()
        {
            SInt32 result;
            remotePortGet(result);
            return result;
        }
        UInt32 timeoutGet()
        {
            UInt32 result;
            timeoutGet(result);
            return result;
        }
        void timeoutSet(UInt32 value)
        {
            timeoutSet(Value::GetUndefined(), value);
        }
        void close()
        {
            close(Value::GetUndefined());
        }
        void connect(const ASString& host, SInt32 port)
        {
            connect(Value::GetUndefined(), host, port);
        }
        void flush()
        {
            flush(Value::GetUndefined());
        }
        bool readBoolean()
        {
            bool result;
            readBoolean(result);
            return result;
        }
        SInt32 readByte()
        {
            SInt32 result;
            readByte(result);
            return result;
        }
        void readBytes(Instances::fl_utils::ByteArray* bytes, UInt32 offset = 0, UInt32 length = 0)
        {
            readBytes(Value::GetUndefined(), bytes, offset, length);
        }
        Value::Number readDouble()
        {
            Value::Number result;
            readDouble(result);
            return result;
        }
        Value::Number readFloat()
        {
            Value::Number result;
            readFloat(result);
            return result;
        }
        SInt32 readInt()
        {
            SInt32 result;
            readInt(result);
            return result;
        }
        ASString readMultiByte(UInt32 length, const ASString& charSet)
        {
            ASString result(GetStringManager().CreateEmptyString());
            readMultiByte(result, length, charSet);
            return result;
        }
        Value readObject()
        {
            Value result;
            readObject(result);
            return result;
        }
        SInt32 readShort()
        {
            SInt32 result;
            readShort(result);
            return result;
        }
        UInt32 readUnsignedByte()
        {
            UInt32 result;
            readUnsignedByte(result);
            return result;
        }
        UInt32 readUnsignedInt()
        {
            UInt32 result;
            readUnsignedInt(result);
            return result;
        }
        UInt32 readUnsignedShort()
        {
            UInt32 result;
            readUnsignedShort(result);
            return result;
        }
        ASString readUTF()
        {
            ASString result(GetStringManager().CreateEmptyString());
            readUTF(result);
            return result;
        }
        ASString readUTFBytes(UInt32 length)
        {
            ASString result(GetStringManager().CreateEmptyString());
            readUTFBytes(result, length);
            return result;
        }
        void writeBoolean(bool value)
        {
            writeBoolean(Value::GetUndefined(), value);
        }
        void writeByte(SInt32 value)
        {
            writeByte(Value::GetUndefined(), value);
        }
        void writeBytes(Instances::fl_utils::ByteArray* bytes, UInt32 offset = 0, UInt32 length = 0)
        {
            writeBytes(Value::GetUndefined(), bytes, offset, length);
        }
        void writeDouble(Value::Number value)
        {
            writeDouble(Value::GetUndefined(), value);
        }
        void writeFloat(Value::Number value)
        {
            writeFloat(Value::GetUndefined(), value);
        }
        void writeInt(SInt32 value)
        {
            writeInt(Value::GetUndefined(), value);
        }
        void writeMultiByte(const ASString& value, const ASString& charSet)
        {
            writeMultiByte(Value::GetUndefined(), value, charSet);
        }
        void writeObject(const Value& object)
        {
            writeObject(Value::GetUndefined(), object);
        }
        void writeShort(SInt32 value)
        {
            writeShort(Value::GetUndefined(), value);
        }
        void writeUnsignedInt(UInt32 value)
        {
            writeUnsignedInt(Value::GetUndefined(), value);
        }
        void writeUTF(const ASString& value)
        {
            writeUTF(Value::GetUndefined(), value);
        }
        void writeUTFBytes(const ASString& value)
        {
            writeUTFBytes(Value::GetUndefined(), value);
        }

//##protect##"instance$data"
    protected:
        static const char* ASCII_Names[];
        static const char* UTF8_Names[];
        static const char* UTF16_Names[];
        // No ISO support for the time being.
        //static const char* ISO_Names[];

#ifdef SF_ENABLE_SOCKETS
        Ptr<SocketThreadMgr> SockMgr;
#endif        
        EncodingType ObjectEncodingValue:3;
        EndianType EndianValue:2; // One bit for sign.
//##protect##"instance$data"

    };
}} // namespace Instances

namespace InstanceTraits { namespace fl_net
{
    class Socket : public fl_events::EventDispatcher
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "InstanceTraits::Socket"; }
#endif
    public:
        typedef Instances::fl_net::Socket InstanceType;

    public:
        Socket(VM& vm, const ClassInfo& ci);

    public: 
        static Pickable<Instances::fl_net::Socket> MakeInstance(Socket& t)
        {
            return Pickable<Instances::fl_net::Socket>(new(t.Alloc()) Instances::fl_net::Socket(t));
        }
        SPtr<Instances::fl_net::Socket> MakeInstanceS(Socket& t)
        {
            return MakeInstance(t);
        }

        virtual void MakeObject(Value& result, Traits& t);

        enum { ThunkInfoNum = 42 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[68];
//##protect##"instance_traits$methods"
//##protect##"instance_traits$methods"

//##protect##"instance_traits$data"
//##protect##"instance_traits$data"

    };
    
}}
    
namespace ClassTraits { namespace fl_net
{
    class Socket : public fl_events::EventDispatcher
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Socket"; }
#endif
    public:
        typedef Class ClassType;
        typedef InstanceTraits::fl_net::Socket InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Socket(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}
//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

