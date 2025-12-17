//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Utils_ByteArray.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Utils_ByteArray.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#ifdef SF_ENABLE_ZLIB
#include <zlib.h>
#endif
#include "Kernel/SF_WString.h"
#include "Kernel/SF_SysFile.h"
#include "../Vec/AS3_Obj_Vec_Vector.h"
#include "../Vec/AS3_Obj_Vec_Vector_int.h"
#include "../Vec/AS3_Obj_Vec_Vector_uint.h"
#include "../Vec/AS3_Obj_Vec_Vector_double.h"
#include "../Vec/AS3_Obj_Vec_Vector_object.h"
#include "../Vec/AS3_Obj_Vec_Vector_String.h"
#include "../AS3_Obj_Date.h"
#ifdef GFX_ENABLE_XML
#include "../AS3_Obj_XML.h"
#endif
#include "AS3_Obj_Utils_Dictionary.h"
#include "AS3_Obj_Utils_IExternalizable.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
#ifdef SF_ENABLE_ZLIB
class ZStream
{
public:
    ZStream();
    ~ZStream();

public:
    void SetInput(const UInt8* pbCompressed, UInt32 nb)
    {
	    Stream.next_in = (Bytef*)pbCompressed; 
        Stream.avail_in = nb; 
    }

    int GetAvailInput() const
    {
	    return Stream.avail_in; 
    }

    bool Decompress()
    {
        int error = inflate(&Stream, Z_NO_FLUSH);
        SF_ASSERT(error == Z_OK || error == Z_STREAM_END);
        return (error == Z_OK); 
    }
    int DecompressWithStatus()
    {
	    return inflate(&Stream, Z_NO_FLUSH); 
    }

    void SetOutput(UInt8* pbDecompressed, UInt32 nb)
    {
	    Stream.next_out = (Bytef*) pbDecompressed; 
        Stream.avail_out = nb; 
    }

    UInt8* GetOutput() const
    {
        return Stream.next_out; 
    }

    int GetAvailOutput() const
    {
	    return Stream.avail_out; 
    }

    int GetTotalOutput() const
    {
	    return Stream.total_out; 
    }

private:
    z_stream Stream;
};

ZStream::ZStream()
{
    memset(&Stream, 0, sizeof(Stream));
    int error = inflateInit(&Stream);
    SF_ASSERT(error == Z_OK); 
    SF_UNUSED1(error);
}
ZStream::~ZStream()
{
    int error = inflateEnd(&Stream);
    SF_ASSERT(error == Z_OK); 
    SF_UNUSED1(error);
}
#endif // SF_ENABLE_ZLIB

enum AMF3DataType
{
    undefined_marker = 0x00,
    null_marker = 0x01,
    false_marker = 0x02,
    true_marker = 0x03,
    integer_marker = 0x04,
    double_marker = 0x05,
    string_marker = 0x06,
    xml_doc_marker = 0x07,
    date_marker = 0x08,
    array_marker = 0x09,
    object_marker = 0x0A,
    xml_marker = 0x0B,
    byte_array_marker = 0x0C,
    vector_int_marker = 0x0D,
    vector_uint_marker = 0x0E,
    vector_double_marker = 0x0F,
    vector_object_marker = 0x10,
    dictionary_marker = 0x11,
};

//##protect##"methods"
typedef ThunkFunc0<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_bytesAvailableGet, UInt32> TFunc_Instances_ByteArray_bytesAvailableGet;
typedef ThunkFunc0<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_endianGet, ASString> TFunc_Instances_ByteArray_endianGet;
typedef ThunkFunc1<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_endianSet, const Value, const ASString&> TFunc_Instances_ByteArray_endianSet;
typedef ThunkFunc0<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_lengthGet, UInt32> TFunc_Instances_ByteArray_lengthGet;
typedef ThunkFunc1<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_lengthSet, const Value, UInt32> TFunc_Instances_ByteArray_lengthSet;
typedef ThunkFunc0<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_objectEncodingGet, UInt32> TFunc_Instances_ByteArray_objectEncodingGet;
typedef ThunkFunc1<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_objectEncodingSet, const Value, UInt32> TFunc_Instances_ByteArray_objectEncodingSet;
typedef ThunkFunc0<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_positionGet, UInt32> TFunc_Instances_ByteArray_positionGet;
typedef ThunkFunc1<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_positionSet, const Value, UInt32> TFunc_Instances_ByteArray_positionSet;
typedef ThunkFunc0<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_clear, const Value> TFunc_Instances_ByteArray_clear;
typedef ThunkFunc2<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_compress, Value, unsigned, const Value*> TFunc_Instances_ByteArray_compress;
typedef ThunkFunc0<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_deflate, const Value> TFunc_Instances_ByteArray_deflate;
typedef ThunkFunc0<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_inflate, const Value> TFunc_Instances_ByteArray_inflate;
typedef ThunkFunc0<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_readBoolean, bool> TFunc_Instances_ByteArray_readBoolean;
typedef ThunkFunc0<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_readByte, SInt32> TFunc_Instances_ByteArray_readByte;
typedef ThunkFunc3<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_readBytes, const Value, Instances::fl_utils::ByteArray*, UInt32, UInt32> TFunc_Instances_ByteArray_readBytes;
typedef ThunkFunc0<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_readDouble, Value::Number> TFunc_Instances_ByteArray_readDouble;
typedef ThunkFunc0<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_readFloat, Value::Number> TFunc_Instances_ByteArray_readFloat;
typedef ThunkFunc0<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_readInt, SInt32> TFunc_Instances_ByteArray_readInt;
typedef ThunkFunc2<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_readMultiByte, ASString, UInt32, const ASString&> TFunc_Instances_ByteArray_readMultiByte;
typedef ThunkFunc0<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_readObject, Value> TFunc_Instances_ByteArray_readObject;
typedef ThunkFunc0<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_readShort, SInt32> TFunc_Instances_ByteArray_readShort;
typedef ThunkFunc0<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_readUnsignedByte, UInt32> TFunc_Instances_ByteArray_readUnsignedByte;
typedef ThunkFunc0<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_readUnsignedInt, UInt32> TFunc_Instances_ByteArray_readUnsignedInt;
typedef ThunkFunc0<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_readUnsignedShort, UInt32> TFunc_Instances_ByteArray_readUnsignedShort;
typedef ThunkFunc0<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_readUTF, ASString> TFunc_Instances_ByteArray_readUTF;
typedef ThunkFunc1<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_readUTFBytes, ASString, UInt32> TFunc_Instances_ByteArray_readUTFBytes;
typedef ThunkFunc0<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_toString, ASString> TFunc_Instances_ByteArray_toString;
typedef ThunkFunc2<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_uncompress, Value, unsigned, const Value*> TFunc_Instances_ByteArray_uncompress;
typedef ThunkFunc1<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_writeBoolean, const Value, bool> TFunc_Instances_ByteArray_writeBoolean;
typedef ThunkFunc1<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_writeByte, const Value, SInt32> TFunc_Instances_ByteArray_writeByte;
typedef ThunkFunc3<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_writeBytes, const Value, Instances::fl_utils::ByteArray*, UInt32, UInt32> TFunc_Instances_ByteArray_writeBytes;
typedef ThunkFunc1<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_writeDouble, const Value, Value::Number> TFunc_Instances_ByteArray_writeDouble;
typedef ThunkFunc1<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_writeFloat, const Value, Value::Number> TFunc_Instances_ByteArray_writeFloat;
typedef ThunkFunc1<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_writeInt, const Value, SInt32> TFunc_Instances_ByteArray_writeInt;
typedef ThunkFunc2<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_writeMultiByte, const Value, const ASString&, const ASString&> TFunc_Instances_ByteArray_writeMultiByte;
typedef ThunkFunc1<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_writeObject, const Value, const Value&> TFunc_Instances_ByteArray_writeObject;
typedef ThunkFunc1<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_writeShort, const Value, SInt32> TFunc_Instances_ByteArray_writeShort;
typedef ThunkFunc1<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_writeUnsignedInt, const Value, UInt32> TFunc_Instances_ByteArray_writeUnsignedInt;
typedef ThunkFunc1<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_writeUTF, const Value, const ASString&> TFunc_Instances_ByteArray_writeUTF;
typedef ThunkFunc1<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_writeUTFBytes, const Value, const Value&> TFunc_Instances_ByteArray_writeUTFBytes;
typedef ThunkFunc1<Instances::fl_utils::ByteArray, Instances::fl_utils::ByteArray::mid_writeFile, const Value, const ASString&> TFunc_Instances_ByteArray_writeFile;

template <> const TFunc_Instances_ByteArray_bytesAvailableGet::TMethod TFunc_Instances_ByteArray_bytesAvailableGet::Method = &Instances::fl_utils::ByteArray::bytesAvailableGet;
template <> const TFunc_Instances_ByteArray_endianGet::TMethod TFunc_Instances_ByteArray_endianGet::Method = &Instances::fl_utils::ByteArray::endianGet;
template <> const TFunc_Instances_ByteArray_endianSet::TMethod TFunc_Instances_ByteArray_endianSet::Method = &Instances::fl_utils::ByteArray::endianSet;
template <> const TFunc_Instances_ByteArray_lengthGet::TMethod TFunc_Instances_ByteArray_lengthGet::Method = &Instances::fl_utils::ByteArray::lengthGet;
template <> const TFunc_Instances_ByteArray_lengthSet::TMethod TFunc_Instances_ByteArray_lengthSet::Method = &Instances::fl_utils::ByteArray::lengthSet;
template <> const TFunc_Instances_ByteArray_objectEncodingGet::TMethod TFunc_Instances_ByteArray_objectEncodingGet::Method = &Instances::fl_utils::ByteArray::objectEncodingGet;
template <> const TFunc_Instances_ByteArray_objectEncodingSet::TMethod TFunc_Instances_ByteArray_objectEncodingSet::Method = &Instances::fl_utils::ByteArray::objectEncodingSet;
template <> const TFunc_Instances_ByteArray_positionGet::TMethod TFunc_Instances_ByteArray_positionGet::Method = &Instances::fl_utils::ByteArray::positionGet;
template <> const TFunc_Instances_ByteArray_positionSet::TMethod TFunc_Instances_ByteArray_positionSet::Method = &Instances::fl_utils::ByteArray::positionSet;
template <> const TFunc_Instances_ByteArray_clear::TMethod TFunc_Instances_ByteArray_clear::Method = &Instances::fl_utils::ByteArray::clear;
template <> const TFunc_Instances_ByteArray_compress::TMethod TFunc_Instances_ByteArray_compress::Method = &Instances::fl_utils::ByteArray::compress;
template <> const TFunc_Instances_ByteArray_deflate::TMethod TFunc_Instances_ByteArray_deflate::Method = &Instances::fl_utils::ByteArray::deflate;
template <> const TFunc_Instances_ByteArray_inflate::TMethod TFunc_Instances_ByteArray_inflate::Method = &Instances::fl_utils::ByteArray::inflate;
template <> const TFunc_Instances_ByteArray_readBoolean::TMethod TFunc_Instances_ByteArray_readBoolean::Method = &Instances::fl_utils::ByteArray::readBoolean;
template <> const TFunc_Instances_ByteArray_readByte::TMethod TFunc_Instances_ByteArray_readByte::Method = &Instances::fl_utils::ByteArray::readByte;
template <> const TFunc_Instances_ByteArray_readBytes::TMethod TFunc_Instances_ByteArray_readBytes::Method = &Instances::fl_utils::ByteArray::readBytes;
template <> const TFunc_Instances_ByteArray_readDouble::TMethod TFunc_Instances_ByteArray_readDouble::Method = &Instances::fl_utils::ByteArray::readDouble;
template <> const TFunc_Instances_ByteArray_readFloat::TMethod TFunc_Instances_ByteArray_readFloat::Method = &Instances::fl_utils::ByteArray::readFloat;
template <> const TFunc_Instances_ByteArray_readInt::TMethod TFunc_Instances_ByteArray_readInt::Method = &Instances::fl_utils::ByteArray::readInt;
template <> const TFunc_Instances_ByteArray_readMultiByte::TMethod TFunc_Instances_ByteArray_readMultiByte::Method = &Instances::fl_utils::ByteArray::readMultiByte;
template <> const TFunc_Instances_ByteArray_readObject::TMethod TFunc_Instances_ByteArray_readObject::Method = &Instances::fl_utils::ByteArray::readObject;
template <> const TFunc_Instances_ByteArray_readShort::TMethod TFunc_Instances_ByteArray_readShort::Method = &Instances::fl_utils::ByteArray::readShort;
template <> const TFunc_Instances_ByteArray_readUnsignedByte::TMethod TFunc_Instances_ByteArray_readUnsignedByte::Method = &Instances::fl_utils::ByteArray::readUnsignedByte;
template <> const TFunc_Instances_ByteArray_readUnsignedInt::TMethod TFunc_Instances_ByteArray_readUnsignedInt::Method = &Instances::fl_utils::ByteArray::readUnsignedInt;
template <> const TFunc_Instances_ByteArray_readUnsignedShort::TMethod TFunc_Instances_ByteArray_readUnsignedShort::Method = &Instances::fl_utils::ByteArray::readUnsignedShort;
template <> const TFunc_Instances_ByteArray_readUTF::TMethod TFunc_Instances_ByteArray_readUTF::Method = &Instances::fl_utils::ByteArray::readUTF;
template <> const TFunc_Instances_ByteArray_readUTFBytes::TMethod TFunc_Instances_ByteArray_readUTFBytes::Method = &Instances::fl_utils::ByteArray::readUTFBytes;
template <> const TFunc_Instances_ByteArray_toString::TMethod TFunc_Instances_ByteArray_toString::Method = &Instances::fl_utils::ByteArray::toString;
template <> const TFunc_Instances_ByteArray_uncompress::TMethod TFunc_Instances_ByteArray_uncompress::Method = &Instances::fl_utils::ByteArray::uncompress;
template <> const TFunc_Instances_ByteArray_writeBoolean::TMethod TFunc_Instances_ByteArray_writeBoolean::Method = &Instances::fl_utils::ByteArray::writeBoolean;
template <> const TFunc_Instances_ByteArray_writeByte::TMethod TFunc_Instances_ByteArray_writeByte::Method = &Instances::fl_utils::ByteArray::writeByte;
template <> const TFunc_Instances_ByteArray_writeBytes::TMethod TFunc_Instances_ByteArray_writeBytes::Method = &Instances::fl_utils::ByteArray::writeBytes;
template <> const TFunc_Instances_ByteArray_writeDouble::TMethod TFunc_Instances_ByteArray_writeDouble::Method = &Instances::fl_utils::ByteArray::writeDouble;
template <> const TFunc_Instances_ByteArray_writeFloat::TMethod TFunc_Instances_ByteArray_writeFloat::Method = &Instances::fl_utils::ByteArray::writeFloat;
template <> const TFunc_Instances_ByteArray_writeInt::TMethod TFunc_Instances_ByteArray_writeInt::Method = &Instances::fl_utils::ByteArray::writeInt;
template <> const TFunc_Instances_ByteArray_writeMultiByte::TMethod TFunc_Instances_ByteArray_writeMultiByte::Method = &Instances::fl_utils::ByteArray::writeMultiByte;
template <> const TFunc_Instances_ByteArray_writeObject::TMethod TFunc_Instances_ByteArray_writeObject::Method = &Instances::fl_utils::ByteArray::writeObject;
template <> const TFunc_Instances_ByteArray_writeShort::TMethod TFunc_Instances_ByteArray_writeShort::Method = &Instances::fl_utils::ByteArray::writeShort;
template <> const TFunc_Instances_ByteArray_writeUnsignedInt::TMethod TFunc_Instances_ByteArray_writeUnsignedInt::Method = &Instances::fl_utils::ByteArray::writeUnsignedInt;
template <> const TFunc_Instances_ByteArray_writeUTF::TMethod TFunc_Instances_ByteArray_writeUTF::Method = &Instances::fl_utils::ByteArray::writeUTF;
template <> const TFunc_Instances_ByteArray_writeUTFBytes::TMethod TFunc_Instances_ByteArray_writeUTFBytes::Method = &Instances::fl_utils::ByteArray::writeUTFBytes;
template <> const TFunc_Instances_ByteArray_writeFile::TMethod TFunc_Instances_ByteArray_writeFile::Method = &Instances::fl_utils::ByteArray::writeFile;

namespace Instances { namespace fl_utils
{
    ByteArray::ByteArray(InstanceTraits::Traits& t)
    : Instances::fl::Object(t)
//##protect##"instance::ByteArray::ByteArray()$data"
    , Encoding(static_cast<Classes::fl_utils::ByteArray&>(t.GetClass()).GetDefEncoding())
    , Endian(GetDefaultEndian())
    , Position(0)
    , Length(0)
//##protect##"instance::ByteArray::ByteArray()$data"
    {
//##protect##"instance::ByteArray::ByteArray()$code"
//##protect##"instance::ByteArray::ByteArray()$code"
    }

    void ByteArray::bytesAvailableGet(UInt32& result)
    {
//##protect##"instance::ByteArray::bytesAvailableGet()"
        result = GetAvailableNum();
//##protect##"instance::ByteArray::bytesAvailableGet()"
    }
    void ByteArray::endianGet(ASString& result)
    {
//##protect##"instance::ByteArray::endianGet()"
        if (Endian == endianBig)
            result = GetVM().GetStringManager().CreateConstString("bigEndian");
        else
            result = GetVM().GetStringManager().CreateConstString("littleEndian");
//##protect##"instance::ByteArray::endianGet()"
    }
    void ByteArray::endianSet(const Value& result, const ASString& value)
    {
//##protect##"instance::ByteArray::endianSet()"
        SF_UNUSED1(result);

        if (value == "bigEndian")
            Endian = endianBig;
        else if (value == "littleEndian")
            Endian = endianLittle;
        else
            return GetVM().ThrowArgumentError(VM::Error(VM::eInvalidArgumentError, GetVM() SF_DEBUG_ARG(value.ToCStr())));
//##protect##"instance::ByteArray::endianSet()"
    }
    void ByteArray::lengthGet(UInt32& result)
    {
//##protect##"instance::ByteArray::lengthGet()"
        // Length is not the same as Data.GetSize().
        result = Length;
//##protect##"instance::ByteArray::lengthGet()"
    }
    void ByteArray::lengthSet(const Value& result, UInt32 value)
    {
//##protect##"instance::ByteArray::lengthSet()"
        SF_UNUSED1(result);

        Resize(value);
//##protect##"instance::ByteArray::lengthSet()"
    }
    void ByteArray::objectEncodingGet(UInt32& result)
    {
//##protect##"instance::ByteArray::objectEncodingGet()"
        result = Encoding;
//##protect##"instance::ByteArray::objectEncodingGet()"
    }
    void ByteArray::objectEncodingSet(const Value& result, UInt32 value)
    {
//##protect##"instance::ByteArray::objectEncodingSet()"
        SF_UNUSED1(result);

        if (value != encAMF0 && value != encAMF3)
            return GetVM().ThrowRangeError(VM::Error(VM::eIllegalOperandTypeError, GetVM() SF_DEBUG_ARG("some type") SF_DEBUG_ARG("encAMF0 or encAMF3")));

        Encoding = static_cast<EncodingType>(value);
//##protect##"instance::ByteArray::objectEncodingSet()"
    }
    void ByteArray::positionGet(UInt32& result)
    {
//##protect##"instance::ByteArray::positionGet()"
        result = Position;
//##protect##"instance::ByteArray::positionGet()"
    }
    void ByteArray::positionSet(const Value& result, UInt32 value)
    {
//##protect##"instance::ByteArray::positionSet()"
        SF_UNUSED1(result);

        Position = value;
//##protect##"instance::ByteArray::positionSet()"
    }
    void ByteArray::clear(const Value& result)
    {
//##protect##"instance::ByteArray::clear()"
        SF_UNUSED1(result);
        Data.Clear();
        Length = 0;
        Position = 0;
//##protect##"instance::ByteArray::clear()"
    }
    void ByteArray::compress(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::ByteArray::compress()"
        SF_UNUSED3(result, argc, argv);

        // There is only one argument called "algorithm".
        // We ignore it for the time being.
#ifdef SF_ENABLE_ZLIB
        if (GetLength() == 0)
            return;

        UPInt zlen = GetLength() * 3/2 + 32; 
        UInt8* zdata = static_cast<UInt8*>(SF_HEAP_AUTO_ALLOC(this, zlen));

        // Call zlib.
        compress2(zdata, (unsigned long*)&zlen, (const Bytef *)Data.GetDataPtr(), GetLength(), 9);

        // Replace current array with the compressed data.
        Resize(0);
        Write(zdata, static_cast<UInt32>(zlen));

        SF_FREE(zdata);
#endif
//##protect##"instance::ByteArray::compress()"
    }
    void ByteArray::deflate(const Value& result)
    {
//##protect##"instance::ByteArray::deflate()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("instance::ByteArray::deflate()");
//##protect##"instance::ByteArray::deflate()"
    }
    void ByteArray::inflate(const Value& result)
    {
//##protect##"instance::ByteArray::inflate()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("instance::ByteArray::inflate()");
//##protect##"instance::ByteArray::inflate()"
    }
    void ByteArray::readBoolean(bool& result)
    {
//##protect##"instance::ByteArray::readBoolean()"
        if (EOFError())
            return;

        result = Data[Position++] != 0;
//##protect##"instance::ByteArray::readBoolean()"
    }
    void ByteArray::readByte(SInt32& result)
    {
//##protect##"instance::ByteArray::readByte()"
        if (EOFError())
            return;

        result = static_cast<SInt8>(Data[Position++]);
//##protect##"instance::ByteArray::readByte()"
    }
    void ByteArray::readBytes(const Value& result, Instances::fl_utils::ByteArray* bytes, UInt32 offset, UInt32 length)
    {
//##protect##"instance::ByteArray::readBytes()"
        SF_UNUSED1(result);

        const UInt32 available_num = GetAvailableNum();

        if (length == 0)
            length = available_num;

        if (length > available_num)
            return ThrowEOFError();

        // Adjust size of the output array.
        if (offset + length >= bytes->Data.GetSize())
            bytes->Resize(offset + length);

        Read(bytes->Data.GetDataPtr() + offset, length).DoNotCheck();
//##protect##"instance::ByteArray::readBytes()"
    }
    void ByteArray::readDouble(Value::Number& result)
    {
//##protect##"instance::ByteArray::readDouble()"
        if (!Read(&result, sizeof(result)))
            return;

        AdjustByteOrder(result);
//##protect##"instance::ByteArray::readDouble()"
    }
    void ByteArray::readFloat(Value::Number& result)
    {
//##protect##"instance::ByteArray::readFloat()"
        float v;

        if (!Read(&v, sizeof(v)))
            return;

        AdjustByteOrder(v);
        result = v;
//##protect##"instance::ByteArray::readFloat()"
    }
    void ByteArray::readInt(SInt32& result)
    {
//##protect##"instance::ByteArray::readInt()"
        if (!Read(&result, sizeof(result)))
            return;

        AdjustByteOrder(result);
//##protect##"instance::ByteArray::readInt()"
    }
    void ByteArray::readMultiByte(ASString& result, UInt32 length, const ASString& charSet)
    {
//##protect##"instance::ByteArray::readMultiByte()"
		// In case of ASCII.
		for (unsigned i = 0; ASCII_Names[i]; ++i)
		{
			if (charSet == ASCII_Names[i])
			{
				length = Alg::Min(length, GetLength());
				result = GetVM().GetStringManager().CreateString(reinterpret_cast<char*>(Data.GetDataPtr() + Position), length);
				Position += length;
				return;
			}
		}

		// In case of UTF-8.
		for (unsigned i = 0; UTF8_Names[i]; ++i)
		{
			if (charSet == UTF8_Names[i])
			{
				ReadUTFBytes(result, length).DoNotCheck();
				return;
			}
		}

		/*
		// No ISO support for the time being
		// In case of ISO.
		for (unsigned i = 0; ISO_Names[i]; ++i)
		{
			if (charSet == ISO_Names[i])
			{
				length = Alg::Min(length, GetLength());
				result = GetVM().GetStringManager().CreateString(reinterpret_cast<char*>(Data.GetDataPtr() + Position), length);
				Position += length;
				return;
			}
		}
		*/

		// In case of UTF16.
		for (unsigned i = 0; UTF16_Names[i]; ++i)
		{
			if (charSet == UTF16_Names[i])
			{
				length = Alg::Min(length, GetLength());
				result = GetVM().GetStringManager().CreateString(reinterpret_cast<const wchar_t*>(Data.GetDataPtr() + Position), length);
				Position += length;
				return;
			}
		}

		VM& vm = GetVM();
		vm.ThrowTypeError(VM::Error(VM::eInvalidArgumentError, vm SF_DEBUG_ARG("charSet")));
//##protect##"instance::ByteArray::readMultiByte()"
    }
    void ByteArray::readObject(Value& result)
    {
//##protect##"instance::ByteArray::readObject()"
        VM& vm = GetVM();
        UInt32 dt_num;

        readUnsignedByte(dt_num);
        if (vm.IsException())
            return;

        const AMF3DataType dt = static_cast<AMF3DataType>(dt_num);
        switch (dt)
        {
        case undefined_marker:
            result.SetUndefined();
            break;
        case null_marker:
            result.SetNull();
            break;
        case false_marker:
            result.SetBool(false);
            break;
        case true_marker:
            result.SetBool(true);
            break;
        case integer_marker:
            result.SetSInt32((static_cast<SInt32>(ReadUInt29() << 3) >> 3));
            break;
        case double_marker:
            result.SetNumber(DeserializeDouble());
            break;
        case string_marker:
            result.Assign(DeserializeStr());
            break;
        case xml_doc_marker:
            // We do not support package flash.xml.
            SF_ASSERT(false);
            break;
        case date_marker:
            result.Assign(DeserializeDate());
            break;
        case array_marker:
            result.Assign(DeserializeArray());
            break;
        case object_marker:
            DeserializeObjDefault(result);
            break;
        case xml_marker:
#ifdef GFX_ENABLE_XML
            result = DeserializeXML();
#endif
            break;
        case byte_array_marker:
            DeserializeByteArray(result);
            break;
        case vector_int_marker:
            result = DeserializeVector_int();
            break;
        case vector_uint_marker:
            result = DeserializeVector_uint();
            break;
        case vector_double_marker:
            result = DeserializeVector_double();
            break;
        case vector_object_marker:
            DeserializeVector_object(result);
            break;
        case dictionary_marker:
            result = DeserializeDictionary();
            break;
        }
//##protect##"instance::ByteArray::readObject()"
    }
    void ByteArray::readShort(SInt32& result)
    {
//##protect##"instance::ByteArray::readShort()"
        result = ReadS16();
//##protect##"instance::ByteArray::readShort()"
    }
    void ByteArray::readUnsignedByte(UInt32& result)
    {
//##protect##"instance::ByteArray::readUnsignedByte()"
        if (EOFError())
            return;

        result = Data[Position++];
//##protect##"instance::ByteArray::readUnsignedByte()"
    }
    void ByteArray::readUnsignedInt(UInt32& result)
    {
//##protect##"instance::ByteArray::readUnsignedInt()"
        if (!Read(&result, sizeof(result)))
            return;

        AdjustByteOrder(result);
//##protect##"instance::ByteArray::readUnsignedInt()"
    }
    void ByteArray::readUnsignedShort(UInt32& result)
    {
//##protect##"instance::ByteArray::readUnsignedShort()"
        result = ReadU16();
//##protect##"instance::ByteArray::readUnsignedShort()"
    }
    void ByteArray::readUTF(ASString& result)
    {
//##protect##"instance::ByteArray::readUTF()"
        ReadUTFBytes(result, ReadU16()).DoNotCheck();
//##protect##"instance::ByteArray::readUTF()"
    }
    void ByteArray::readUTFBytes(ASString& result, UInt32 length)
    {
//##protect##"instance::ByteArray::readUTFBytes()"
        ReadUTFBytes(result, length).DoNotCheck();
//##protect##"instance::ByteArray::readUTFBytes()"
    }
    void ByteArray::toString(ASString& result)
    {
//##protect##"instance::ByteArray::toString()"
        UPInt size = Data.GetSize();
        union {
            char* str;
            wchar_t* str_w;
        };

        str = (char*)Data.GetDataPtr();

        if (size > 1)
        {
            if (IsUTF16BE(str))
            {
                str +=2;
                size = (size - 2) >> 1;
                // Ignore BE/LE differences for the time being.
                result = GetVM().GetStringManager().CreateString(str_w, size);
                return;
            } else if (IsUTF16LE(str))
            {
                str +=2;
                size = (size - 2) >> 1;
                // Ignore BE/LE differences for the time being.
                result = GetVM().GetStringManager().CreateString(str_w, size);
                return;
            } else if (size > 2 && IsUTF8BOM(str))
            {
                result = GetVM().GetStringManager().CreateString((const char*)(str + 3), size - 3);
                return;
            }
        }

        // We are supposed to copy invalid UTF-8 sequences as single characters here.
        // Let's do it later.
        result = GetVM().GetStringManager().CreateString((const char*)str, size);
//##protect##"instance::ByteArray::toString()"
    }
    void ByteArray::uncompress(Value& result, unsigned argc, const Value* const argv)
    {
//##protect##"instance::ByteArray::uncompress()"
        SF_UNUSED3(result, argc, argv);

        // There is only one argument called "algorithm".
        // We ignore it for the time being.
#ifdef SF_ENABLE_ZLIB
        if (GetLength() == 0)
            return;

        UPInt zlen = GetLength();
        UInt8* zdata = static_cast<UInt8*>(SF_HEAP_AUTO_ALLOC(this, zlen));
        memcpy(zdata, Data.GetDataPtr(), zlen);

        Resize(0);

        int error = Z_OK;
        ZStream zstream;
        zstream.SetInput(zdata, static_cast<UInt32>(zlen));

        const int kBufferSize = 8192;
        UInt8* buffer = static_cast<UInt8*>(SF_HEAP_AUTO_ALLOC(this, kBufferSize));

        do {
            zstream.SetOutput(buffer, kBufferSize);
            error = zstream.DecompressWithStatus();
            Write(buffer, kBufferSize - zstream.GetAvailOutput());
        } while (error == Z_OK);

        SF_FREE(buffer);
        SF_FREE(zdata);

        Position = 0;

        if (error != Z_OK && error != Z_STREAM_END)
            GetVM().ThrowError(VM::Error(VM::eShellCompressedDataError, GetVM()));
#endif
//##protect##"instance::ByteArray::uncompress()"
    }
    void ByteArray::writeBoolean(const Value& result, bool value)
    {
//##protect##"instance::ByteArray::writeBoolean()"
        SF_UNUSED1(result);

        Write(value);
//##protect##"instance::ByteArray::writeBoolean()"
    }
    void ByteArray::writeByte(const Value& result, SInt32 value)
    {
//##protect##"instance::ByteArray::writeByte()"
        SF_UNUSED1(result);

        Write((SInt8)value);
//##protect##"instance::ByteArray::writeByte()"
    }
    void ByteArray::writeBytes(const Value& result, Instances::fl_utils::ByteArray* bytes, UInt32 offset, UInt32 length)
    {
//##protect##"instance::ByteArray::writeBytes()"
        SF_UNUSED1(result);

        if (bytes == NULL)
            return;

        if (bytes->GetLength() < offset)
            offset = bytes->GetLength();

        if (length == 0)
            length = bytes->GetLength() - offset;

        if (length > bytes->GetLength() - offset)
            return GetVM().ThrowRangeError(VM::Error(VM::eParamRangeError, GetVM()));

        if (length > 0)
            Write(bytes->Data.GetDataPtr() + offset, length);
//##protect##"instance::ByteArray::writeBytes()"
    }
    void ByteArray::writeDouble(const Value& result, Value::Number value)
    {
//##protect##"instance::ByteArray::writeDouble()"
        SF_UNUSED1(result);

        AdjustByteOrder(value);
        Write(&value, sizeof(value));
//##protect##"instance::ByteArray::writeDouble()"
    }
    void ByteArray::writeFloat(const Value& result, Value::Number value)
    {
//##protect##"instance::ByteArray::writeFloat()"
        SF_UNUSED1(result);

        float v = static_cast<float>(value);
        AdjustByteOrder(v);
        Write(&v, sizeof(v));
//##protect##"instance::ByteArray::writeFloat()"
    }
    void ByteArray::writeInt(const Value& result, SInt32 value)
    {
//##protect##"instance::ByteArray::writeInt()"
        SF_UNUSED1(result);

        AdjustByteOrder(value);
        Write(&value, sizeof(value));
//##protect##"instance::ByteArray::writeInt()"
    }
    void ByteArray::writeMultiByte(const Value& result, const ASString& value, const ASString& charSet)
    {
//##protect##"instance::ByteArray::writeMultiByte()"
		SF_UNUSED3(result, value, charSet);

		// In case of ASCII.
		// No ISO support for the time being.

		// In case of UTF-8.
		for (unsigned i = 0; UTF8_Names[i]; ++i)
		{
			if (charSet == UTF8_Names[i])
			{
				Write((void*)value.ToCStr(), value.GetSize());
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
				    Write(wbuff.GetBuffer(), static_cast<UInt32>(wbuff.GetLength()));
				return;
			}
		}

		VM& vm = GetVM();
		vm.ThrowTypeError(VM::Error(VM::eInvalidArgumentError, vm SF_DEBUG_ARG("charSet")));
//##protect##"instance::ByteArray::writeMultiByte()"
    }
    void ByteArray::writeObject(const Value& result, const Value& object)
    {
//##protect##"instance::ByteArray::writeObject()"
        SF_UNUSED1(result);

        UInt8 marker;
        const Value::KindType k = object.GetKind();
        switch (k)
        {
        case Value::kUndefined:
            marker = undefined_marker;
            Write(marker);
            break;
        case Value::kBoolean:
            if (object.AsBool())
                marker = true_marker;
            else
                marker = false_marker;
            Write(marker);
            break;
        case Value::kInt:
            {
                SInt32 v = object.AsInt();
                if (((v << 3) >> 3) == v)
                {
                    marker = integer_marker;
                    Write(marker);
                    WriteUInt29(object.AsInt() & 0x1fffffff);
                }
                else
                {
                    marker = double_marker;
                    Write(marker);
                    SerializeDouble(static_cast<Value::Number>(v));
                }
            }
            break;
        case Value::kUInt:
            {
                SInt32 v = object.AsUInt();
                if (((v << 3) >> 3) == v)
                {
                    marker = integer_marker;
                    Write(marker);
                    WriteUInt29(object.AsInt() & 0x1fffffff);
                }
                else
                {
                    marker = double_marker;
                    Write(marker);
                    SerializeDouble(static_cast<Value::Number>(v));
                }
            }
            break;
        case Value::kNumber:
            marker = double_marker;
            Write(marker);
            SerializeDouble(object.AsNumber());
            break;
        case Value::kThunk:
        case Value::kMethodInd:
        case Value::kVTableInd:
        case Value::kInstanceTraits:
        case Value::kClassTraits:
            SF_ASSERT(false);
            GetVM().ThrowArgumentError(VM::Error(VM::eInvalidArgumentError, GetVM() SF_DEBUG_ARG("object")));
            break;
#if defined(SF_AS3_AOTC) || defined(SF_AS3_AOTC2)
        case Value::kSNodeIT:
        case Value::kSNodeCT:
            SF_ASSERT(false);
            GetVM().ThrowArgumentError(VM::Error(VM::eInvalidArgumentError, GetVM() SF_DEBUG_ARG("object")));
            break;
#endif
        case Value::kString:
            marker = string_marker;
            Write(marker);
            SerializeStr(object.AsString());
            break;
        case Value::kNamespace:
            marker = object_marker;
            if (object.GetNamespace() == NULL)
                return Write(static_cast<UInt8>(null_marker));
            Write(marker);
            break;
        case Value::kObject:
            if (object.GetObject() == NULL)
                return Write(static_cast<UInt8>(null_marker));
            SerializeObj(*object.GetObject());
            break;
        case Value::kClass:
            marker = object_marker;
            if (object.GetObject() == NULL)
                marker = null_marker;
            break;
        case Value::kFunction:
            // Don't serialize functions
            marker = undefined_marker;
            Write(marker);
            break;
        case Value::kThunkFunction:
        case Value::kThunkClosure:
        case Value::kVTableIndClosure:
            SF_ASSERT(false);
            GetVM().ThrowArgumentError(VM::Error(VM::eInvalidArgumentError, GetVM() SF_DEBUG_ARG("object")));
            break;
        }
//##protect##"instance::ByteArray::writeObject()"
    }
    void ByteArray::writeShort(const Value& result, SInt32 value)
    {
//##protect##"instance::ByteArray::writeShort()"
        SF_UNUSED1(result);

        SInt16 v = static_cast<SInt16>(value);
        AdjustByteOrder(v);
        Write(&v, sizeof(v));
//##protect##"instance::ByteArray::writeShort()"
    }
    void ByteArray::writeUnsignedInt(const Value& result, UInt32 value)
    {
//##protect##"instance::ByteArray::writeUnsignedInt()"
        SF_UNUSED1(result);

        AdjustByteOrder(value);
        Write(&value, sizeof(value));
//##protect##"instance::ByteArray::writeUnsignedInt()"
    }
    void ByteArray::writeUTF(const Value& result, const ASString& value)
    {
//##protect##"instance::ByteArray::writeUTF()"
        SF_UNUSED1(result);

        if (value.GetSize() > 65535)
            return GetVM().ThrowRangeError(VM::Error(VM::eNotImplementedError, GetVM() SF_DEBUG_ARG("ByteArray::writeUTF")));

        const UInt16 size = static_cast<UInt16>(value.GetSize());
        Write(size);
        Write((void*)value.ToCStr(), size);
//##protect##"instance::ByteArray::writeUTF()"
    }
    void ByteArray::writeUTFBytes(const Value& result, const Value& value)
    {
//##protect##"instance::ByteArray::writeUTFBytes()"
        SF_UNUSED1(result);

        if (value.IsNullOrUndefined())
            return GetVM().ThrowTypeError(VM::Error(VM::eNullArgumentError, GetVM() SF_DEBUG_ARG("value")));

        StringManager& sm = GetVM().GetStringManager();
        ASString str = sm.CreateEmptyString();
        if (value.Convert2String(str))
        {
            Write((void*)str.ToCStr(), str.GetSize());
        }
//##protect##"instance::ByteArray::writeUTFBytes()"
    }
    void ByteArray::writeFile(const Value& result, const ASString& filename)
    {
//##protect##"instance::ByteArray::writeFile()"
        SF_UNUSED1(result);
        VM& vm = GetVM();

        if (filename.IsNull())
            return GetVM().ThrowTypeError(VM::Error(VM::eNullArgumentError, vm SF_DEBUG_ARG("filename")));

        SysFile file;
        if (file.Create(String(filename.ToCStr(), filename.GetSize())))
        {
            UInt32 num;

            num = file.Write(static_cast<UByte*>(GetDataPtr()), GetLength());
            if (num == GetLength())
                // Everything is OK.
                return;
        }

        vm.ThrowError(VM::Error(VM::eFileWriteError, vm SF_DEBUG_ARG(filename)));
//##protect##"instance::ByteArray::writeFile()"
    }

//##protect##"instance$methods"
	const char* ByteArray::ASCII_Names[] =
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

	const char* ByteArray::UTF8_Names[] =
	{
		"utf-8",
		"unicode-1-1-utf-8",
		"unicode-2-0-utf-8",
		"x-unicode-2-0-utf-8",
		NULL
	};

	const char* ByteArray::UTF16_Names[] =
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

    ByteArray::ByteArray(InstanceTraits::Traits& t, EncodingType enc)
    : Instances::fl::Object(t)
    , Encoding(enc)
    , Endian(Instances::fl_utils::ByteArray::GetDefaultEndian())
    , Position(0)
    {
    }

    void ByteArray::ThrowEOFError()
    {
        Value v;
        if (!GetVM().ConstructBuiltinValue(v, "flash.errors.EOFError"))
            return;
        GetVM().Throw(v);
    }

    void ByteArray::ThrowMemoryError()
    {
        Value v;
        if (!GetVM().ConstructBuiltinValue(v, "flash.errors.MemoryError"))
            return;
        GetVM().Throw(v);
    }

    CheckResult ByteArray::EOFError()
    {
        if (Position >= Data.GetSize())
        {
            ThrowEOFError();
            return true;
        }

        return false;
    }

    CheckResult ByteArray::Read(void* dest, UInt32 buff_size)
    {
        if (Position + buff_size > Data.GetSize())
        {
            ThrowEOFError();
            return false;
        }

        memcpy(dest, Data.GetDataPtr() + Position, buff_size);
        Position += buff_size;

        return true;
    }

    void ByteArray::Write(const void* src, UInt32 buff_size)
    {
        const UInt32 size = Position + buff_size;
        if (size >= Data.GetSize())
            // Resize() will take care of Length.
            Resize(Position + buff_size);
        else if (size >= Length)
            Length = size;

        memcpy(Data.GetDataPtr() + Position, src, buff_size);
        Position += buff_size;
    }

    void ByteArray::Write(bool v)
    {
        const UInt32 size = Position + sizeof(v);
        if (size >= Data.GetSize())
            // Resize() will take care of Length.
            Resize(size);
        else if (size >= Length)
            Length = size;

        Data[Position++] = (v ? 1 : 0);
    }

    void ByteArray::Write(UInt8 v)
    {
        const UInt32 size = Position + sizeof(v);
        if (size >= Data.GetSize())
            // Resize() will take care of Length.
            Resize(size);
        else if (size >= Length)
            Length = size;

        Data[Position++] = v;
    }

    void ByteArray::Write(SInt8 v)
    {
        const UInt32 size = Position + sizeof(v);
        if (size >= Data.GetSize())
            // Resize() will take care of Length.
            Resize(size);
        else if (size >= Length)
            Length = size;

        Data[Position++] = static_cast<UInt8>(v);
    }

    void ByteArray::Resize(UInt32 size)
    {
        const UPInt oldSize = Data.GetSize();

        if (size > oldSize)
        {
            Data.Resize(size);
            memset(Data.GetDataPtr() + oldSize, 0, size - oldSize);
        }

        Length = size;

        if (Position > Length)
            Position = Length;
    }

    UInt8 ByteArray::ReadU8()
    {
        UInt8 v;

        if (!Read(&v, sizeof(v)))
            return v;

        return v;
    }

    UInt16 ByteArray::ReadU16()
    {
        UInt16 v;

        if (!Read(&v, sizeof(v)))
            return v;

        AdjustByteOrder(v);
        return v;
    }

	CheckResult ByteArray::ReadUTFBytes(ASString& result, UInt32 len)
	{
		if (!CanRead(len))
		{
			ThrowEOFError();
			return false;
		}

		// Check for UTF8 BOM.
		if (len > 2 && IsUTF8BOM(reinterpret_cast<char*>(Data.GetDataPtr() + Position)))
		{
			len -= 3;
			Position += 3;
		}

		result = GetVM().GetStringManager().CreateString(reinterpret_cast<char*>(Data.GetDataPtr() + Position), len);
        Position += len;

		return true;
	}

    UInt8 ByteArray::Get(UInt32 ind) const
    {
        if (ind < GetLength())
            return Data[ind];

        GetVM().ThrowArgumentError(VM::Error(VM::eInvalidArgumentError, GetVM() SF_DEBUG_ARG("ByteArray::Get") SF_DEBUG_ARG(ind) SF_DEBUG_ARG(0) SF_DEBUG_ARG(GetLength() - 1)));
        return 0;
    }

    void ByteArray::Get(void *dest, UPInt destSz)
    {
        Position = 0;
        Read(dest, (UInt32)destSz).DoNotCheck();
        Position = 0;
    }

    void ByteArray::Set(UInt32 ind, UInt8 v)
    {
        if (ind >= GetLength())
            Resize(ind + 1);

        Data[ind] = v;
    }

    void ByteArray::Set(const void* data, UPInt sz)
    {
        Position = 0;
        Write(data, (int)sz);
        Position = 0;
    }

    CheckResult ByteArray::SetProperty(const Multiname& prop_name, const Value& value)
    {
        UInt32 ind;
        if (GetArrayInd(prop_name, ind))
        {
            UInt32 v;
            if (!value.Convert2UInt32(v))
                // Exception.
                return false;

            // Cast and set.
            Set(ind, static_cast<UInt8>(v));
            return true;
        }

        // Not an Array index. Let us treat it as a regular object.
        return Instances::fl::Object::SetProperty(prop_name, value);
    }

    CheckResult ByteArray::GetProperty(const Multiname& prop_name, Value& value)
    {
        UInt32 ind;
        if (GetArrayInd(prop_name, ind))
        {
            if (ind < GetLength())
            {
                // We should return unsigned int here. This is how it works.
                value.SetUInt32(static_cast<UInt8>(Get(ind)));
                return true;
            }

            return false;
        }

        // Not an Array index. Let us treat it as a regular object.
        return Instances::fl::Object::GetProperty(prop_name, value);
    }

    void ByteArray::GetDynamicProperty(AbsoluteIndex ind, Value& value)
    {
        value.SetSInt32(static_cast<SInt8>(Get(ind.Get())));
    }

    bool ByteArray::HasProperty(const Multiname& prop_name, bool check_prototype)
    {
        UInt32 ind;
        if (GetArrayInd(prop_name, ind))
            return ind < GetLength();

        // Not an Array index. Let us treat it as a regular object.
        return Instances::fl::Object::HasProperty(prop_name, check_prototype);
    }

    void ByteArray::WriteUInt29(UInt32 v)
    {
        // Represent smaller integers with fewer bytes using the most
        // significant bit of each byte. The worst case uses 32-bits
        // to represent a 29-bit number, which is what we would have
        // done with no compression.

        // 0x00000000 - 0x0000007F : 0xxxxxxx
        // 0x00000080 - 0x00003FFF : 1xxxxxxx 0xxxxxxx
        // 0x00004000 - 0x001FFFFF : 1xxxxxxx 1xxxxxxx 0xxxxxxx
        // 0x00200000 - 0x3FFFFFFF : 1xxxxxxx 1xxxxxxx 1xxxxxxx xxxxxxxx
        // 0x40000000 - 0xFFFFFFFF : throw range exception

        if (v < 0x80)
        {
            // 0x00000000 - 0x0000007F : 0xxxxxxx
            Write((UInt8)v);
        } else if (v < 0x4000)
        {
            // 0x00000080 - 0x00003FFF : 1xxxxxxx 0xxxxxxx
            Write((UInt8)(((v >> 7) & 0x7F) | 0x80));
            Write((UInt8)(v & 0x7F));
        } else if (v < 0x200000)
        {
            // 0x00004000 - 0x001FFFFF : 1xxxxxxx 1xxxxxxx 0xxxxxxx
            Write((UInt8)(((v >> 14) & 0x7F) | 0x80));
            Write((UInt8)(((v >>  7) & 0x7F) | 0x80));
            Write((UInt8)(v & 0x7F));
        } else if (v < 0x40000000)
        {
            // 0x00200000 - 0x3FFFFFFF : 1xxxxxxx 1xxxxxxx 1xxxxxxx xxxxxxxx
            Write((UInt8)(((v >> 22) & 0x7F) | 0x80));
            Write((UInt8)(((v >> 15) & 0x7F) | 0x80));
            Write((UInt8)(((v >>  8) & 0x7F) | 0x80));
            Write((UInt8)(v & 0xFF));
        } else
            // 0x40000000 - 0xFFFFFFFF : throw range exception
            GetVM().ThrowRangeError(VM::Error(VM::eInvalidRangeError, GetVM()));
    }

    UInt32 ByteArray::ReadUInt29()
    {
        // Represent smaller integers with fewer bytes using the most
        // significant bit of each byte. The worst case uses 32-bits
        // to represent a 29-bit number, which is what we would have
        // done with no compression.

        // 0x00000000 - 0x0000007F : 0xxxxxxx
        // 0x00000080 - 0x00003FFF : 1xxxxxxx 0xxxxxxx
        // 0x00004000 - 0x001FFFFF : 1xxxxxxx 1xxxxxxx 0xxxxxxx
        // 0x00200000 - 0x3FFFFFFF : 1xxxxxxx 1xxxxxxx 1xxxxxxx xxxxxxxx
        // 0x40000000 - 0xFFFFFFFF : throw range exception

        UInt32 result = 0;
        UInt8 byte;

        if (!Read(&byte, sizeof(byte)))
            return result;

        if (byte < 128)
            return byte;

        result = (byte & 0x7F) << 7;
        if (!Read(&byte, sizeof(byte)))
            return result;

        if (byte < 128)
            return (result | byte);

        result = (result | (byte & 0x7F)) << 7;
        if (!Read(&byte, sizeof(byte)))
            return result;

        if (byte < 128)
            return (result | byte);

        result = (result | (byte & 0x7F)) << 8;
        if (!Read(&byte, sizeof(byte)))
            return result;

        return (result | byte);
    }

    inline void SwapWords(UInt64& value)
    {
#if (SF_BYTE_ORDER == SF_LITTLE_ENDIAN)
        union {
            UInt64 v;
            UInt32 w[2];
        };
        v = value;
        UInt32 tmp = w[0];
        w[0] = w[1];
        w[1] = tmp;
        value = v;
#else
        SF_UNUSED1(value);
#endif
    }

    void ByteArray::SerializeUInt32(UInt32 v)
    {
        union {
            UInt8 b[4];
            UInt32 u;
        };

        u = Alg::ByteUtil::SystemToBE(v);
        Write(b, 4);
    }

    UInt32 ByteArray::DeserializeUInt32()
    {
        union {
            UInt8 b[4];
            UInt32 u;
        };

        if (!Read(b, 4))
            return 0;

        return Alg::ByteUtil::BEToSystem(u);
    }

    void ByteArray::SerializeDouble(Value::Number v)
    {
        union {
            UInt8 b[8];
            UInt64 u;
            double d;
        };

        d = v;
#if 0
        SwapWords(u);
#endif
        u = Alg::ByteUtil::SystemToBE(u);
        Write(b, 8);
    }

    Value::Number ByteArray::DeserializeDouble()
    {
        union {
            UInt8 b[8];
            UInt64 u;
            double d;
        };

        if (!Read(b, 8))
            return 0.0;
#if 0
        SwapWords(u);
#endif
        u = Alg::ByteUtil::BEToSystem(u);

        return d;
    }

    void ByteArray::SerializeStr(const ASString& v)
    {
        if (v.IsEmpty())
            return WriteRef(1);

        const SInt32 ref = FindInStrTable(v);

        if (ref < 0)
        {
            // Not found.
            UInt32 len = v.GetSize();

            AddToStrTable(v);

            WriteRef((len << 1) | 1);
            Write(v.ToCStr(), len);
        }
        else
            // Found.
            WriteRef(ref << 1);
    }

    ASString ByteArray::DeserializeStr()
    {
        StringManager& sm = GetStringManager();
        ASString result = sm.CreateEmptyString();
        UInt32 ref = ReadRef();

        if ((ref & 1) == 0)
        {
            // This is a reference
            StringListGet(result, ref >> 1);
            return result;
        }

        // Read the string in
        UInt32 len = (ref >> 1);

        if (len == 0)
            // SerializeStr() special cases the empty string to avoid creating a reference.
            return result;

        if (!ReadUTFBytes(result, len))
            return result;

        StringListAdd(result);

        return result;
    }

    void ByteArray::SerializeObj(AS3::Object& v)
    {
        const Traits& tr = v.GetTraits();

        if (tr.IsInstanceTraits())
        {
            BuiltinTraitsType tt = tr.GetTraitsType();

            switch (tt)
            {
            case Traits_Array:
                return SerializeArray(static_cast<fl::Array&>(v));
            case Traits_ByteArray:
                return SerializeByteArray(static_cast<ByteArray&>(v));
            case Traits_Date:
                return SerializeDate(static_cast<fl::Date&>(v));
            case Traits_Function:
                // We do not serialize functions.
                SF_ASSERT(false);
                return;
            case Traits_Dictionary:
                return SerializeDictionary(static_cast<Dictionary&>(v));
            case Traits_Vector_int:
                return SerializeVector_int(static_cast<fl_vec::Vector_int&>(v));
            case Traits_Vector_uint:
                return SerializeVector_uint(static_cast<fl_vec::Vector_uint&>(v));
            case Traits_Vector_double:
                return SerializeVector_double(static_cast<fl_vec::Vector_double&>(v));
            case Traits_Vector_String:
                return SerializeVector_String(static_cast<fl_vec::Vector_String&>(v));
            case Traits_Vector_object:
                return SerializeVector_object(static_cast<fl_vec::Vector_object&>(v));
            case Traits_XML:
#ifdef GFX_ENABLE_XML
                return SerializeXML(static_cast<fl::XML&>(v));
#endif
            default:
                break;
            }
        }

        // Fallback.
        SerializeObjDefault(v);
    }

    bool IsSerialazable(const SlotInfo& si)
    {
        return si.IsReadWrite() && si.GetNamespace().IsVMPublic();
    }

    struct BASlotFunctCalc : public Slots::SlotFunct
    {
        BASlotFunctCalc() : result(0) {}

        virtual void operator()(const SlotInfo& si)
        {
            if (IsSerialazable(si))
                ++result;
        }

        UInt32 GetResult() const { return result; }

    private:
        UInt32 result;
    };

    struct BASlotFunctSrlzFixedName : public Slots::SlotFunct
    {
        BASlotFunctSrlzFixedName(ByteArray& ba) : BA(ba) {}

        virtual void operator()(const SlotInfo& si)
        {
            if (IsSerialazable(si))
            {
                const ASString name = si.GetName();
                BA.SerializeStr(name);
            }
        }

    private:
        BASlotFunctSrlzFixedName& operator =(const BASlotFunctSrlzFixedName&);

    private:
        ByteArray& BA;
    };

    struct BASlotFunctSrlzFixedValue : public Slots::SlotFunct
    {
        BASlotFunctSrlzFixedValue(ByteArray& ba, AS3::Object* obj) : BA(ba), Obj(obj) {}

        virtual void operator()(const SlotInfo& si)
        {
            if (IsSerialazable(si))
            {
                Value V;
                if (!si.GetSlotValueUnsafe(V, Obj))
                    // Exception.
                    return;

                BA.writeObject(V);
            }
        }

    private:
        BASlotFunctSrlzFixedValue& operator =(const BASlotFunctSrlzFixedValue&);

    private:
        ByteArray& BA;
        AS3::Object* Obj;
    };

    UInt32 CalcSealedPropNum(const AS3::Traits& tr)
    {
        BASlotFunctCalc f;
        tr.ForEachSlot(f);
        return f.GetResult();
    }

    void ByteArray::SerializeObjDefault(AS3::Object& v)
    {
        // object-marker (U29O-ref | (U29O-traits-ext class-name *(U8)) | U29O-traits-ref | 
        // (U29O-traits class-name *(UTF-8-vr))) *(value-type) *(dynamic-member)))

        const UInt8 marker = object_marker;
        Write(marker);
        SInt32 ref = FindInObjTable(&v);

        if (ref >= 0)
            return WriteRef(ref << 1);

        AddToObjTable(&v);

        VM& vm = GetVM();
        InstanceTraits::fl_utils::ByteArray& ba_itr = static_cast<InstanceTraits::fl_utils::ByteArray&>(GetTraits());

        // Traits.
        // Note that for U29O-traits-ext, after the class-name follows an 
        // indeterminable number of bytes as *(U8). This represents the 
        // completely custom serialization of "externalizable" types. The 
        // client and server have an agreement as to how to read in this information.
        Traits& tr = v.GetTraits();
        SF_ASSERT(tr.IsInstanceTraits());
        InstanceTraits::Traits& itr = static_cast<InstanceTraits::Traits&>(tr);
        ref = FindInTraitsTable(&itr);
        UInt32 sealedPropNum = CalcSealedPropNum(tr);
        const bool isDynamic = tr.IsDynamic();
        // Object implements IExternalizable.
        const bool externalizable = itr.IsOfType(ba_itr.GetTraitsIExternalizable());

        if (ref >= 0)
        {
            // U29O-traits-ref
            // The first (low) bit is a flag with value 1. The second bit is a flag
            // (representing whether a trait reference follows) with value 0 to
            // imply that this objects traits are being sent by reference. The remaining
            // 1 to 27 significant bits are used to encode a trait reference index (an integer).
            WriteRef((ref << 2) | 1);
        }
        else
        {
            // U29O-traits class-name *(UTF-8-vr)

            // 1. U29O-traits
            // The first (low) bit is a flag with value 1. The second bit is a flag with
            // value 1. The third bit is a flag with value 0. The fourth bit is a flag
            // specifying whether the type is dynamic. A value of 0 implies not
            // dynamic, a value of 1 implies dynamic. Dynamic types may have a set of name
            // value pairs for dynamic members after the sealed member section. The
            // remaining 1 to 25 significant bits are used to encode the number of sealed
            // traits member names that follow after the class name (an integer).
            WriteRef(3 | (externalizable ? 4 : 0) | (isDynamic ? 8 : 0) | (sealedPropNum << 4));

            // 2. class-name
            {
                Class& cl = tr.GetClass();
                const ASString alias = vm.GetAliasByClass(cl);
                SerializeStr(alias);
            }

            // 3. *(UTF-8-vr)
            // Write names of sealed properties.
            BASlotFunctSrlzFixedName f(*this);
            tr.ForEachSlot(f);
        }

        // http://help.adobe.com/en_US/FlashPlatform/reference/actionscript/3/flash/utils/IExternalizable.html
        if (externalizable)
        {
            // Call writeExternal().
            Value r;
            Multiname name(vm.GetPublicNamespace(), vm.GetStringManager().CreateConstString("writeExternal"));
            Value argv[1] = {Value(this)};

            if (!v.ExecutePropertyUnsafe(name, r, 1, argv))
                return;
        }
        else
        {
            // an instance of the class will be serialized using the default 
            // mechanism of public members only. As a result, private, internal, 
            // and protected members of a class will not be available.

            // For each sealed property in the class info, write out its value.
            BASlotFunctSrlzFixedValue f(*this, &v);
            tr.ForEachSlot(f);

            if (isDynamic)
            {
                // !!! We do not support flash.net.IDynamicPropertyWriter at this time. !!!

                const Object::DynAttrsType* da = v.GetDynamicAttrs();
                SF_ASSERT(da);

                Object::DynAttrsType::ConstIterator it = da->Begin();
                for (; !it.IsEnd(); ++it)
                {
                    const Value& value = it->Second;

                    if (value.IsFunction())
                        // We do not serialize functions.
                        continue;

                    const ASString& name = it->First.GetName();

                    if (name.IsEmpty())
                        // Empty string is sentinel for end-of-data, so we CANNOT serialize it.
                        continue;

                    SerializeStr(name);
                    writeObject(value);
                }

                // Write out the empty string as a terminator
                SerializeStr(GetStringManager().CreateEmptyString());
            }
        }
    }

    void ByteArray::DeserializeObjDefault(Value& result)
    {
        UInt32 ref = ReadRef();

        if ((ref & 1) == 0)
        {
            // This is a reference.
            AS3::Object* r = NULL;
            if (ObjectListGet(r, ref >> 1))
                result.Assign(r);
            return;
        }

        VM& vm = GetVM();

        // Read Traits.
        Class* cl = NULL;
        bool externalizable = false;
        UInt32 sealedPropNum = 0;
        bool isDynamic = false;
        ArrayDH<ASString> prop_names(vm.GetMemoryHeap());

        if ((ref & 3) == 1)
        {
            InstanceTraits::Traits* itr = NULL;
            TraitsListGet(itr, ref >> 2);

            cl = &itr->GetClass();
            InstanceTraits::fl_utils::ByteArray& ba_itr = static_cast<InstanceTraits::fl_utils::ByteArray&>(GetTraits());
            // Object implements IExternalizable.
            externalizable = itr->IsOfType(ba_itr.GetTraitsIExternalizable());
            sealedPropNum = 0;
            isDynamic = itr->IsDynamic();

            // Collect names of sealed properties.
            AS3::Traits::CIterator it = itr->Begin();

            for (; !it.IsEnd(); ++it)
            {
                const SlotInfo& si = it.GetSlotInfo();

                if (!IsSerialazable(si))
                    continue;

                prop_names.PushBack(si.GetQualifiedName());
                ++sealedPropNum;
            }
        }
        else
        {
            // U29O-traits class-name *(UTF-8-vr)

            // 1. U29O-traits
            externalizable = (ref & 4) != 0;
            isDynamic = (ref & 8) != 0;
            sealedPropNum = (ref >> 4);

            // 2. class-name
            const ASString name = DeserializeStr();

            if (!name.IsEmpty())
                cl = vm.GetClassByAlias(name);

            if (cl == NULL)
                cl = &vm.GetClassObject();

            // 3. *(UTF-8-vr)
            // Read sealed property names.
            prop_names.Reserve(sealedPropNum);
            for (UInt32 i = 0; i < sealedPropNum; ++i)
                prop_names.PushBack(DeserializeStr());
        }

        // Create an object.
        SF_ASSERT(cl);
        if (vm.Construct(*cl, result))
            vm.ExecuteCode();

        if (IsException()) 
            return;

        AS3::Object* obj = result.GetObject();
        ObjectListAdd(obj);

        // http://help.adobe.com/en_US/FlashPlatform/reference/actionscript/3/flash/utils/IExternalizable.html
        if (externalizable)
        {
            // Call readExternal().
            Value r;
            Multiname name(vm.GetPublicNamespace(), vm.GetStringManager().CreateConstString("readExternal"));
            Value argv[1] = {Value(this)};

            if (!obj->ExecutePropertyUnsafe(name, r, 1, argv))
                // Exception.
                return;
        }
        else
        {
            Value value;

            // For each sealed property in the class info, write out its value.
            for (UInt32 i = 0; i < sealedPropNum; ++i)
            {
                const Multiname mn(vm.GetPublicNamespace(), prop_names[i]);

                readObject(value);
                if (!obj->SetProperty(mn, value))
                    // Exception.
                    return;
            }

            if (isDynamic)
            {
                while (true)
                {
                    const ASString name = DeserializeStr();
                    if (name.IsEmpty())
                        break;

                    readObject(value);
                    obj->AddDynamicSlotValuePair(name, value);
                }
            }
        }
    }

    class SerializeArrSparse : public Impl::ArrayFunc
    {
    public:
        SerializeArrSparse(const fl::Array& a, ByteArray& ba)
            : SM(a.GetStringManager())
            , A(a)
            , BA(ba)
        {
        }

    public:
        virtual void operator()(UInt32 ind, const Value& v)
        {
            if (v.IsFunction())
                // We do not serialize functions.
                // Just skip it.
                return;

            Scaleform::LongFormatter f(ind);
            f.Convert();
            const StringDataPtr r = f.GetResult();

            BA.SerializeStr(SM.CreateString(r.ToCStr(), r.GetSize()));
            BA.writeObject(v);
        }

    private:
        SerializeArrSparse& operator =(const SerializeArrSparse&);

    private:
        StringManager& SM;
        const fl::Array& A;
        ByteArray& BA;
    };

    class SerializeArrDense : public Impl::ArrayFunc
    {
    public:
        SerializeArrDense(const fl::Array& a, ByteArray& ba)
            : A(a)
            , BA(ba)
        {
        }

    public:
        virtual void operator()(UInt32 ind, const Value& v)
        {
            SF_UNUSED1(ind);

            if (v.IsFunction())
                // We do not serialize functions.
                return BA.Write(static_cast<UInt8>(undefined_marker));

            BA.writeObject(v);
        }

    private:
        SerializeArrDense& operator =(const SerializeArrDense&);

    private:
        const fl::Array& A;
        ByteArray& BA;
    };

    void ByteArray::SerializeArray(fl::Array& v)
    {
        const UInt8 marker = array_marker;
        Write(marker);
        SInt32 ref = FindInObjTable(&v);

        if (ref >= 0)
            return WriteRef(ref << 1);

        AddToObjTable(&v);

        // AMF considers Arrays in two parts, the dense portion and the 
        // associative portion. The binary representation of the associative
        // portion consists of name/value pairs (potentially none) terminated
        // by an empty string. The binary representation of the dense portion
        // is the size of the dense portion (potentially zero) followed by
        // an ordered list of values (potentially none). The order these are
        // written in AMF is first the size of the dense portion, an empty
        // string terminated list of name/value pairs, followed by size values.

        // 1. first the size of the dense portion.
        // The first (low) bit is a flag with value 1. The remaining 1 to 28
        // significant bits are used to encode the count of the dense portion
        // of the Array.
        const ValueArrayDH& dense = v.GetContiguousPart();
        const UPInt dsize = dense.GetSize();

        // Size of dense part.
        WriteRef((static_cast<UInt32>(dsize) << 1) | 1);

        // 2. list of name/value pairs.
        if (v.IsSparse())
        {
            // We have sparse part.
            SerializeArrSparse s(v, *this);
            v.ForEachSparse(s);
        }

        // 3. an empty string terminated list of name/value pairs.
        SerializeStr(GetStringManager().CreateEmptyString());

        // 4. followed by size values.
        if (dsize > 0)
        {
            // We have dense part.
            SerializeArrDense s(v, *this);
            v.ForEachDense(s);
        }
    }

    SPtr<fl::Array> ByteArray::DeserializeArray()
    {
        UInt32 ref = ReadRef();

        if ((ref & 1) == 0)
        {
            // This is a reference.
            AS3::Object* r = NULL;
            ObjectListGet(r, ref >> 1).DoNotCheck();
            return static_cast<fl::Array*>(r);
        }

        VM& vm = GetVM();
        const UInt32 denseLen = (ref >> 1);

        Pickable<Instances::fl::Array> result = vm.MakeArray();
        ObjectListAdd(result.GetPtr());

        // Read sparse part. It is terminated by an empty string.
        Value value;
        char* strTail;
        unsigned long ind;

        while (true)
        {
            const ASString name = DeserializeStr();
            if (name.IsEmpty())
                // End of sparse part.
                break;

            readObject(value);
            ind = SFstrtoul(name.ToCStr(), &strTail, 10);
            result->Set(ind, value);
        }

        // Read dense part.
        for (UInt32 i = 0; i < denseLen; ++i)
        {
            readObject(value);
            result->Set(i, value);
        }

        return result;
    }

    void ByteArray::SerializeByteArray(ByteArray& v)
    {
        const UInt8 marker = byte_array_marker;
        Write(marker);
        SInt32 ref = FindInObjTable(&v);

        if (ref >= 0)
            return WriteRef(ref << 1);

        AddToObjTable(&v);

        const UInt32 len = v.GetLength();

        WriteRef((len << 1) | 1);
        Write(v.GetDataPtr(), len);
    }

    void ByteArray::DeserializeByteArray(Value& result)
    {
        UInt32 ref = ReadRef();

        if ((ref & 1) == 0)
        {
            // This is a reference.
            AS3::Object* r = NULL;
            if (ObjectListGet(r, ref >> 1))
                result = r;
            return;
        }

        VM& vm = GetVM();
        Pickable<ByteArray> r = vm.MakeByteArray();
        ObjectListAdd(r.GetPtr());
        result = r;

        const UInt32 len = ref >> 1;
        r->Resize(len);
        Read(r->GetDataPtr(), len).DoNotCheck();
    }

    void ByteArray::SerializeDate(fl::Date& v)
    {
        const UInt8 marker = date_marker;
        Write(marker);
        SInt32 ref = FindInObjTable(&v);

        if (ref >= 0)
            return WriteRef(ref << 1);

        AddToObjTable(&v);

        // The first (low) bit is a flag with value 1. The remaining bits
        // are not used.
        WriteRef(1);

        // !!! DO NOT call writeDouble() here.
        SerializeDouble(const_cast<fl::Date&>(v).AS3valueOf());
    }

    SPtr<fl::Date> ByteArray::DeserializeDate()
    {
        UInt32 ref = ReadRef();

        if ((ref & 1) == 0)
        {
            // This is a reference.
            AS3::Object* r = NULL;
            ObjectListGet(r, ref >> 1).DoNotCheck();
            return static_cast<fl::Date*>(r);
        }

        VM& vm = GetVM();
        InstanceTraits::fl_utils::ByteArray& tr = static_cast<InstanceTraits::fl_utils::ByteArray&>(GetTraits());

        if (tr.DateTR == NULL)
        {
            const ClassTraits::Traits* ctr = vm.Resolve2ClassTraits(AS3::fl::DateTI);
            SF_ASSERT(ctr);
            tr.DateTR = &static_cast<InstanceTraits::fl::Date&>(ctr->GetInstanceTraits());
        }

        Pickable<Instances::fl::Date> result = tr.DateTR->MakeInstance(*tr.DateTR);
        ObjectListAdd(result.GetPtr());
        result->SetDate(DeserializeDouble());

        return result;
    }

    void ByteArray::SerializeDictionary(fl_utils::Dictionary& v)
    {
        const UInt8 marker = dictionary_marker;
        Write(marker);
        SInt32 ref = FindInObjTable(&v);

        if (ref >= 0)
            return WriteRef(ref << 1);

        AddToObjTable(&v);
            
        // The first (low) bit is a flag with value 1. The remaining 1 to 28
        // significant bits are used to encode the number of entries;
        const UPInt size = v.GetSize();
        WriteRef((static_cast<UInt32>(size) << 1) | 1);
        Write(v.IsWeakKeys());

        const ValueContainerType& c = v.GetContainer();
        ValueContainerType::ConstIterator it = c.Begin();
        for (; !it.IsEnd(); ++it)
        {
            writeObject(it->First);
            writeObject(it->Second);
        }
    }

    SPtr<fl_utils::Dictionary> ByteArray::DeserializeDictionary()
    {
        UInt32 ref = ReadRef();

        if ((ref & 1) == 0)
        {
            // This is a reference.
            AS3::Object* r = NULL;
            ObjectListGet(r, ref >> 1).DoNotCheck();
            return static_cast<fl_utils::Dictionary*>(r);
        }

        VM& vm = GetVM();
        InstanceTraits::fl_utils::ByteArray& tr = static_cast<InstanceTraits::fl_utils::ByteArray&>(GetTraits());

        if (tr.DictionaryTR == NULL)
        {
            const ClassTraits::Traits* ctr = vm.Resolve2ClassTraits(AS3::fl_utils::DictionaryTI);
            SF_ASSERT(ctr);
            tr.DictionaryTR = &static_cast<InstanceTraits::fl_utils::Dictionary&>(ctr->GetInstanceTraits());
        }

        const UInt32 len = ref >> 1;
        const bool weakKeys = (ReadU8() != 0);
        Pickable<Instances::fl_utils::Dictionary> result = tr.DictionaryTR->MakeInstance(*tr.DictionaryTR, weakKeys);
        ObjectListAdd(result.GetPtr());

        Value name;
        Value value;

        for (UInt32 i = 0; i < len; ++i)
        {
            readObject(name);
            readObject(value);
            result->AddDynamicSlotValuePair(name, value);
        }

        return result;
    }

#ifdef GFX_ENABLE_XML
    void ByteArray::SerializeXML(fl::XML& v)
    {
        const UInt8 marker = xml_marker;
        Write(marker);
        SInt32 ref = FindInObjTable(&v);

        if (ref >= 0)
            return WriteRef(ref << 1);

        AddToObjTable(&v);

        const ASString str = const_cast<fl::XML&>(v).AS3toXMLString();
        const UInt32 size = static_cast<UInt32>(str.GetSize());

        // !!! DO NOT call SerializeStr(str) here.
        WriteRef((size << 1) | 1);
        Write(str.ToCStr(), size);
    }

    SPtr<fl::XML> ByteArray::DeserializeXML()
    {
        UInt32 ref = ReadRef();

        if ((ref & 1) == 0)
        {
            // This is a reference.
            AS3::Object* r = NULL;
            ObjectListGet(r, ref >> 1).DoNotCheck();
            return static_cast<fl::XML*>(r);
        }

        VM& vm = GetVM();
        InstanceTraits::fl_utils::ByteArray& tr = static_cast<InstanceTraits::fl_utils::ByteArray&>(GetTraits());

        if (tr.XMLTR == NULL)
        {
            const ClassTraits::Traits* ctr = vm.Resolve2ClassTraits(AS3::fl::XMLTI);
            SF_ASSERT(ctr);
            tr.XMLTR = &static_cast<InstanceTraits::fl::XML&>(ctr->GetInstanceTraits());
        }

        const UInt32 len = (ref >> 1);

        // !!! Do not call DeserializeStr() here !!!
        StringBuffer buf;

        buf.Resize(len);
        if (!Read((void*)buf.ToCStr(), len))
            // Exception.
            return NULL;

        const ASString str = GetStringManager().CreateString(buf.ToCStr(), len);
        Pickable<Instances::fl::XML> result = tr.XMLTR->MakeInstance(*tr.XMLTR, str);

        ObjectListAdd(result.GetPtr());

        return result;
    }
#endif

    void ByteArray::SerializeVector_int(fl_vec::Vector_int& v)
    {
        const UInt8 marker = vector_int_marker;
        Write(marker);
        SInt32 ref = FindInObjTable(&v);

        if (ref >= 0)
            return WriteRef(ref << 1);

        AddToObjTable(&v);

        // Non-const version of v.
        fl_vec::Vector_int& ncv = const_cast<fl_vec::Vector_int&>(v);
        const UInt32 size = ncv.lengthGet();

        WriteRef((size << 1) | 1);
        Write(ncv.fixedGet());

        const VectorBase<SInt32>& base = v.GetArray();
        for (UInt32 i = 0; i < size; ++i)
            SerializeUInt32(base.Get(i));
    }

    SPtr<fl_vec::Vector_int> ByteArray::DeserializeVector_int()
    {
        UInt32 ref = ReadRef();

        if ((ref & 1) == 0)
        {
            // This is a reference.
            AS3::Object* r = NULL;
            ObjectListGet(r, ref >> 1).DoNotCheck();
            return static_cast<fl_vec::Vector_int*>(r);
        }

        VM& vm = GetVM();
        const UInt32 len = ref >> 1;
        const bool isFixed = (ReadU8() != 0);

        InstanceTraits::fl_vec::Vector_int& itr = static_cast<InstanceTraits::fl_vec::Vector_int&>(vm.GetITraitsVectorSInt());
        Pickable<Instances::fl_vec::Vector_int> result = itr.MakeInstance(itr, len, isFixed);
        ObjectListAdd(result.GetPtr());

        for (UInt32 i = 0; i < len; ++i)
            result->SetUnsafe(i, DeserializeUInt32());

        return result;
    }

    void ByteArray::SerializeVector_uint(fl_vec::Vector_uint& v)
    {
        const UInt8 marker = vector_uint_marker;
        Write(marker);
        SInt32 ref = FindInObjTable(&v);

        if (ref >= 0)
            return WriteRef(ref << 1);

        AddToObjTable(&v);

        const UInt32 size = v.lengthGet();

        WriteRef((size << 1) | 1);

        // Non-const version of v.
        fl_vec::Vector_uint& ncv = const_cast<fl_vec::Vector_uint&>(v);
        Write(ncv.fixedGet());

        const VectorBase<UInt32>& base = v.GetArray();
        for (UInt32 i = 0; i < size; ++i)
            SerializeUInt32(base.Get(i));
    }

    SPtr<fl_vec::Vector_uint> ByteArray::DeserializeVector_uint()
    {
        UInt32 ref = ReadRef();

        if ((ref & 1) == 0)
        {
            // This is a reference.
            AS3::Object* r = NULL;
            ObjectListGet(r, ref >> 1).DoNotCheck();
            return static_cast<fl_vec::Vector_uint*>(r);
        }

        VM& vm = GetVM();
        const UInt32 len = ref >> 1;
        const bool isFixed = (ReadU8() != 0);

        InstanceTraits::fl_vec::Vector_uint& itr = static_cast<InstanceTraits::fl_vec::Vector_uint&>(vm.GetITraitsVectorUInt());
        Pickable<Instances::fl_vec::Vector_uint> result = itr.MakeInstance(itr, len, isFixed);
        ObjectListAdd(result.GetPtr());

        for (UInt32 i = 0; i < len; ++i)
            result->SetUnsafe(i, DeserializeUInt32());

        return result;
    }

    void ByteArray::SerializeVector_double(fl_vec::Vector_double& v)
    {
        const UInt8 marker = vector_double_marker;
        Write(marker);
        SInt32 ref = FindInObjTable(&v);

        if (ref >= 0)
            return WriteRef(ref << 1);

        AddToObjTable(&v);

        // Non-const version of v.
        fl_vec::Vector_double& ncv = const_cast<fl_vec::Vector_double&>(v);
        const UInt32 size = ncv.lengthGet();

        WriteRef((size << 1) | 1);
        Write(ncv.fixedGet());

        const VectorBase<double>& base = v.GetArray();
        for (UInt32 i = 0; i < size; ++i)
            SerializeDouble(base.Get(i));
    }

    SPtr<fl_vec::Vector_double> ByteArray::DeserializeVector_double()
    {
        UInt32 ref = ReadRef();

        if ((ref & 1) == 0)
        {
            // This is a reference.
            AS3::Object* r = NULL;
            ObjectListGet(r, ref >> 1).DoNotCheck();
            return static_cast<fl_vec::Vector_double*>(r);
        }

        VM& vm = GetVM();
        const UInt32 len = ref >> 1;
        const bool isFixed = (ReadU8() != 0);

        InstanceTraits::fl_vec::Vector_double& itr = static_cast<InstanceTraits::fl_vec::Vector_double&>(vm.GetITraitsVectorNumber());
        Pickable<Instances::fl_vec::Vector_double> result = itr.MakeInstance(itr, len, isFixed);
        ObjectListAdd(result.GetPtr());

        for (UInt32 i = 0; i < len; ++i)
            result->SetUnsafe(i, DeserializeDouble());

        return result;
    }

    void ByteArray::SerializeVector_object(fl_vec::Vector_object& v)
    {
        const UInt8 marker = vector_object_marker;
        Write(marker);
        SInt32 ref = FindInObjTable(&v);

        if (ref >= 0)
            return WriteRef(ref << 1);

        AddToObjTable(&v);

        const UInt32 size = v.lengthGet();
        WriteRef((size << 1) | 1);

        // Non-const version of v.
        fl_vec::Vector_object& ncv = const_cast<fl_vec::Vector_object&>(v);
        Write(ncv.fixedGet());

        // Type name.
        SerializeStr(v.GetEnclosedClassTraits().GetName());

        const VectorBase<Value>& base = v.GetArray();
        for (UInt32 i = 0; i < size; ++i)
            writeObject(base.Get(i));
    }

    void ByteArray::DeserializeVector_object(Value& result)
    {
        UInt32 ref = ReadRef();

        if ((ref & 1) == 0)
        {
            // This is a reference.
            AS3::Object* r = NULL;
            if (ObjectListGet(r, ref >> 1))
                result = r;
            return;
        }

        VM& vm = GetVM();
        const UInt32 len = ref >> 1;
        const bool isFixed = (ReadU8() != 0);
        const ASString enclTypeName = DeserializeStr();

        if (enclTypeName == "String")
        {
            InstanceTraits::fl_vec::Vector_String& itr = static_cast<InstanceTraits::fl_vec::Vector_String&>(vm.GetITraitsVectorString());
            Pickable<Instances::fl_vec::Vector_String> r = itr.MakeInstance(itr, len, isFixed);
            ObjectListAdd(r.GetPtr());

            for (UInt32 i = 0; i < len; ++i)
                r->SetUnsafe(i, DeserializeStr());

            return;
        }

        const Multiname mn(vm, StringDataPtr(enclTypeName.ToCStr(), enclTypeName.GetSize()));
        const ClassTraits::Traits* elem_type = vm.Resolve2ClassTraits(mn, vm.GetFrameAppDomain());

        if (elem_type == NULL)
            // Exception. Unknown type.
            return;

        const ClassTraits::Traits& ctr = vm.GetClassVector().Resolve2Vector(*elem_type);
        InstanceTraits::fl_vec::Vector_object& itr = static_cast<InstanceTraits::fl_vec::Vector_object&>(ctr.GetInstanceTraits());
        Pickable<fl_vec::Vector_object> r = itr.MakeInstance(itr, len, isFixed);
        ObjectListAdd(r.GetPtr());
        result = r;

        Value value;
        for (UInt32 i = 0; i < len; ++i)
        {
            readObject(value);
            r->SetUnsafe(i, value);
        }
    }

    void ByteArray::SerializeVector_String(fl_vec::Vector_String& v)
    {
        // Serialize String as Object.
        const UInt8 marker = vector_object_marker;
        Write(marker);
        SInt32 ref = FindInObjTable(&v);

        if (ref >= 0)
            return WriteRef(ref << 1);

        AddToObjTable(&v);

        // Non-const version of v.
        fl_vec::Vector_String& ncv = const_cast<fl_vec::Vector_String&>(v);
        const UInt32 size = ncv.lengthGet();

        WriteRef((size << 1) | 1);
        Write(ncv.fixedGet());

        // Type name.
        SerializeStr(v.GetEnclosedClassTraits().GetName());

        const VectorBase<Ptr<ASStringNode> >& base = v.GetArray();
        for (UInt32 i = 0; i < size; ++i)
            SerializeStr(ASString(base.Get(i)));
    }

    void ByteArray::StringListGet(ASString& result, UInt32 ref) const
    {
        VM& vm = GetVM();

        if (ref >= StringList.GetSize())
            return vm.ThrowRangeError(VM::Error(VM::eInvalidRangeError, vm));

        result = StringList[ref];
    }

    CheckResult ByteArray::ObjectListGet(AS3::Object*& result, UInt32 ref) const
    {
        VM& vm = GetVM();

        if (ref >= ObjectList.GetSize())
        {
            vm.ThrowRangeError(VM::Error(VM::eInvalidRangeError, vm));
            return false;
        }

        result = ObjectList[ref];
        return true;
    }

    void ByteArray::TraitsListGet(InstanceTraits::Traits*& result, UInt32 ref) const
    {
        VM& vm = GetVM();

        if (ref >= TraitsList.GetSize())
            return vm.ThrowRangeError(VM::Error(VM::eInvalidRangeError, vm));

        result = TraitsList[ref];
    }

    void ByteArray::ForEachChild_GC(Collector* prcc, GcOp op) const
    {
        fl::Object::ForEachChild_GC(prcc, op);

        UPInt size;

        size = ObjectList.GetSize();
        for (UPInt i = 0; i < size; ++i)
            AS3::ForEachChild_GC(prcc, ObjectList[i], op SF_DEBUG_ARG(*this));

        size = TraitsList.GetSize();
        for (UPInt i = 0; i < size; ++i)
            AS3::ForEachChild_GC(prcc, TraitsList[i], op SF_DEBUG_ARG(*this));

        ObjectTable.ForEachChild_GC(prcc, op SF_DEBUG_ARG(*this));
        TraitsTable.ForEachChild_GC(prcc, op SF_DEBUG_ARG(*this));
    }

//##protect##"instance$methods"

}} // namespace Instances

namespace InstanceTraits { namespace fl_utils
{
    // const UInt16 ByteArray::tito[ByteArray::ThunkInfoNum] = {
    //    0, 1, 2, 4, 5, 7, 8, 10, 11, 13, 14, 15, 16, 17, 18, 19, 23, 24, 25, 26, 29, 30, 31, 32, 33, 34, 35, 37, 38, 39, 41, 43, 47, 49, 51, 53, 56, 58, 60, 62, 64, 66, 
    // };
    const TypeInfo* ByteArray::tit[68] = {
        &AS3::fl::uintTI, 
        &AS3::fl::StringTI, 
        NULL, &AS3::fl::StringTI, 
        &AS3::fl::uintTI, 
        NULL, &AS3::fl::uintTI, 
        &AS3::fl::uintTI, 
        NULL, &AS3::fl::uintTI, 
        &AS3::fl::uintTI, 
        NULL, &AS3::fl::uintTI, 
        NULL, 
        NULL, 
        NULL, 
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
        &AS3::fl::StringTI, 
        NULL, 
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
        NULL, NULL, 
        NULL, &AS3::fl::StringTI, 
    };
    const ThunkInfo ByteArray::ti[ByteArray::ThunkInfoNum] = {
        {TFunc_Instances_ByteArray_bytesAvailableGet::Func, &ByteArray::tit[0], "bytesAvailable", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_ByteArray_endianGet::Func, &ByteArray::tit[1], "endian", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_ByteArray_endianSet::Func, &ByteArray::tit[2], "endian", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_ByteArray_lengthGet::Func, &ByteArray::tit[4], "length", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_ByteArray_lengthSet::Func, &ByteArray::tit[5], "length", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_ByteArray_objectEncodingGet::Func, &ByteArray::tit[7], "objectEncoding", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_ByteArray_objectEncodingSet::Func, &ByteArray::tit[8], "objectEncoding", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_ByteArray_positionGet::Func, &ByteArray::tit[10], "position", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Instances_ByteArray_positionSet::Func, &ByteArray::tit[11], "position", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Instances_ByteArray_clear::Func, &ByteArray::tit[13], "clear", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_ByteArray_compress::Func, &ByteArray::tit[14], "compress", NULL, Abc::NS_Public, CT_Method, 0, 1, 1, 0, NULL},
        {ThunkInfo::EmptyFunc, &ByteArray::tit[15], "deflate", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {ThunkInfo::EmptyFunc, &ByteArray::tit[16], "inflate", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_ByteArray_readBoolean::Func, &ByteArray::tit[17], "readBoolean", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_ByteArray_readByte::Func, &ByteArray::tit[18], "readByte", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_ByteArray_readBytes::Func, &ByteArray::tit[19], "readBytes", NULL, Abc::NS_Public, CT_Method, 1, 3, 0, 0, NULL},
        {TFunc_Instances_ByteArray_readDouble::Func, &ByteArray::tit[23], "readDouble", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_ByteArray_readFloat::Func, &ByteArray::tit[24], "readFloat", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_ByteArray_readInt::Func, &ByteArray::tit[25], "readInt", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_ByteArray_readMultiByte::Func, &ByteArray::tit[26], "readMultiByte", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Instances_ByteArray_readObject::Func, &ByteArray::tit[29], "readObject", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_ByteArray_readShort::Func, &ByteArray::tit[30], "readShort", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_ByteArray_readUnsignedByte::Func, &ByteArray::tit[31], "readUnsignedByte", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_ByteArray_readUnsignedInt::Func, &ByteArray::tit[32], "readUnsignedInt", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_ByteArray_readUnsignedShort::Func, &ByteArray::tit[33], "readUnsignedShort", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_ByteArray_readUTF::Func, &ByteArray::tit[34], "readUTF", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_ByteArray_readUTFBytes::Func, &ByteArray::tit[35], "readUTFBytes", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_ByteArray_toString::Func, &ByteArray::tit[37], "toString", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
        {TFunc_Instances_ByteArray_uncompress::Func, &ByteArray::tit[38], "uncompress", NULL, Abc::NS_Public, CT_Method, 0, 1, 1, 0, NULL},
        {TFunc_Instances_ByteArray_writeBoolean::Func, &ByteArray::tit[39], "writeBoolean", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_ByteArray_writeByte::Func, &ByteArray::tit[41], "writeByte", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_ByteArray_writeBytes::Func, &ByteArray::tit[43], "writeBytes", NULL, Abc::NS_Public, CT_Method, 1, 3, 0, 0, NULL},
        {TFunc_Instances_ByteArray_writeDouble::Func, &ByteArray::tit[47], "writeDouble", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_ByteArray_writeFloat::Func, &ByteArray::tit[49], "writeFloat", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_ByteArray_writeInt::Func, &ByteArray::tit[51], "writeInt", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_ByteArray_writeMultiByte::Func, &ByteArray::tit[53], "writeMultiByte", NULL, Abc::NS_Public, CT_Method, 2, 2, 0, 0, NULL},
        {TFunc_Instances_ByteArray_writeObject::Func, &ByteArray::tit[56], "writeObject", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_ByteArray_writeShort::Func, &ByteArray::tit[58], "writeShort", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_ByteArray_writeUnsignedInt::Func, &ByteArray::tit[60], "writeUnsignedInt", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_ByteArray_writeUTF::Func, &ByteArray::tit[62], "writeUTF", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_ByteArray_writeUTFBytes::Func, &ByteArray::tit[64], "writeUTFBytes", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
        {TFunc_Instances_ByteArray_writeFile::Func, &ByteArray::tit[66], "writeFile", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
    };

    ByteArray::ByteArray(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"InstanceTraits::ByteArray::ByteArray()"
        SetArrayLike();
        SetTraitsType(Traits_ByteArray);

        DateTR = NULL;
#ifdef GFX_ENABLE_XML
        XMLTR = NULL;
#endif
        DictionaryTR = NULL;
        IExternalizableTR = NULL;
        VecObjectTR = NULL;
//##protect##"InstanceTraits::ByteArray::ByteArray()"

    }

    void ByteArray::MakeObject(Value& result, Traits& t)
    {
        result = MakeInstance(static_cast<ByteArray&>(t));
    }

//##protect##"instance_traits$methods"
    InstanceTraits::Traits& ByteArray::GetTraitsIExternalizable()
    {
        if (IExternalizableTR == NULL)
        {
            VM& vm = GetVM();
            const ClassTraits::Traits* ctr = vm.Resolve2ClassTraits(AS3::fl_utils::IExternalizableTI);
            SF_ASSERT(ctr);
            IExternalizableTR = &ctr->GetInstanceTraits();
        }

        return *IExternalizableTR;
    }
//##protect##"instance_traits$methods"

}} // namespace InstanceTraits

namespace Classes { namespace fl_utils
{
    ByteArray::ByteArray(ClassTraits::Traits& t)
    : Class(t)
    {
//##protect##"class_::ByteArray::ByteArray()"
        DefEncoding = Instances::fl_utils::ByteArray::encAMF3;
//##protect##"class_::ByteArray::ByteArray()"
    }
    void ByteArray::defaultObjectEncodingSet(const Value& result, UInt32 value)
    {
//##protect##"class_::ByteArray::defaultObjectEncodingSet()"
        SF_UNUSED1(result);

        if (value != Instances::fl_utils::ByteArray::encAMF0 && value != Instances::fl_utils::ByteArray::encAMF3)
            return GetVM().ThrowRangeError(VM::Error(VM::eIllegalOperandTypeError, GetVM() SF_DEBUG_ARG("something") SF_DEBUG_ARG("encAMF0 or encAMF3")));

        DefEncoding = static_cast<Instances::fl_utils::ByteArray::EncodingType>(value);
//##protect##"class_::ByteArray::defaultObjectEncodingSet()"
    }
    void ByteArray::defaultObjectEncodingGet(UInt32& result)
    {
//##protect##"class_::ByteArray::defaultObjectEncodingGet()"
        result = DefEncoding;
//##protect##"class_::ByteArray::defaultObjectEncodingGet()"
    }
    void ByteArray::readFile(SPtr<Instances::fl_utils::ByteArray>& result, const ASString& filename)
    {
//##protect##"class_::ByteArray::readFile()"
        SF_UNUSED1(result);
        WARN_NOT_IMPLEMENTED("class_::ByteArray::readFile()");
        VM& vm = GetVM();

        if (filename.IsNull())
            return GetVM().ThrowTypeError(VM::Error(VM::eNullArgumentError, vm SF_DEBUG_ARG("filename")));

        SysFile file;
        if (file.Open(
            String(filename.ToCStr(), filename.GetSize()),
            FileConstants::Open_Read | FileConstants::Open_Buffered),
            FileConstants::Mode_Read
            )
        {
            const SInt64 size = file.LGetLength();

            if (size >= SF_MAX_SINT)
                return vm.ThrowRangeError(VM::Error(VM::eOutOfRangeError, vm 
                    SF_DEBUG_ARG(static_cast<int>(size))
                    SF_DEBUG_ARG(SF_MAX_SINT)
                    ));

            InstanceTraits::fl_utils::ByteArray& itr = static_cast<InstanceTraits::fl_utils::ByteArray&>(GetInstanceTraits());
            result = itr.MakeInstance(itr);
            UByte buff[1024];
            int read_left = static_cast<int>(size);
            int read_num;

            while (read_left)
            {
                read_num = Alg::Min<int>(read_left, sizeof(buff));
                file.Read(buff, read_num);
                result->Write(buff, read_num);
                read_left -= read_num;
            }

            result->Position = 0;
        }
        else
            vm.ThrowError(VM::Error(VM::eFileOpenError, vm SF_DEBUG_ARG(filename)));
//##protect##"class_::ByteArray::readFile()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes

typedef ThunkFunc1<Classes::fl_utils::ByteArray, Classes::fl_utils::ByteArray::mid_defaultObjectEncodingSet, const Value, UInt32> TFunc_Classes_ByteArray_defaultObjectEncodingSet;
typedef ThunkFunc0<Classes::fl_utils::ByteArray, Classes::fl_utils::ByteArray::mid_defaultObjectEncodingGet, UInt32> TFunc_Classes_ByteArray_defaultObjectEncodingGet;
typedef ThunkFunc1<Classes::fl_utils::ByteArray, Classes::fl_utils::ByteArray::mid_readFile, SPtr<Instances::fl_utils::ByteArray>, const ASString&> TFunc_Classes_ByteArray_readFile;

template <> const TFunc_Classes_ByteArray_defaultObjectEncodingSet::TMethod TFunc_Classes_ByteArray_defaultObjectEncodingSet::Method = &Classes::fl_utils::ByteArray::defaultObjectEncodingSet;
template <> const TFunc_Classes_ByteArray_defaultObjectEncodingGet::TMethod TFunc_Classes_ByteArray_defaultObjectEncodingGet::Method = &Classes::fl_utils::ByteArray::defaultObjectEncodingGet;
template <> const TFunc_Classes_ByteArray_readFile::TMethod TFunc_Classes_ByteArray_readFile::Method = &Classes::fl_utils::ByteArray::readFile;

namespace ClassTraits { namespace fl_utils
{
    // const UInt16 ByteArray::tito[ByteArray::ThunkInfoNum] = {
    //    0, 2, 3, 
    // };
    const TypeInfo* ByteArray::tit[5] = {
        NULL, &AS3::fl::uintTI, 
        &AS3::fl::uintTI, 
        &AS3::fl_utils::ByteArrayTI, &AS3::fl::StringTI, 
    };
    const ThunkInfo ByteArray::ti[ByteArray::ThunkInfoNum] = {
        {TFunc_Classes_ByteArray_defaultObjectEncodingSet::Func, &ByteArray::tit[0], "defaultObjectEncoding", NULL, Abc::NS_Public, CT_Set, 1, 1, 0, 0, NULL},
        {TFunc_Classes_ByteArray_defaultObjectEncodingGet::Func, &ByteArray::tit[2], "defaultObjectEncoding", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Classes_ByteArray_readFile::Func, &ByteArray::tit[3], "readFile", NULL, Abc::NS_Public, CT_Method, 1, 1, 0, 0, NULL},
    };

    ByteArray::ByteArray(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::ByteArray::ByteArray()"
        SetArrayLike();
        SetTraitsType(Traits_ByteArray);
//##protect##"ClassTraits::ByteArray::ByteArray()"

    }

    Pickable<Traits> ByteArray::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) ByteArray(vm, AS3::fl_utils::ByteArrayCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_utils::ByteArrayCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_utils
{
    const TypeInfo* ByteArrayImplements[] = {
        &fl_utils::IDataInputTI,
        &fl_utils::IDataOutputTI,
        NULL
    };

    const TypeInfo ByteArrayTI = {
        TypeInfo::CompileTime,
        sizeof(ClassTraits::fl_utils::ByteArray::InstanceType),
        ClassTraits::fl_utils::ByteArray::ThunkInfoNum,
        0,
        InstanceTraits::fl_utils::ByteArray::ThunkInfoNum,
        0,
        "ByteArray", "flash.utils", &fl::ObjectTI,
        ByteArrayImplements
    };

    const ClassInfo ByteArrayCI = {
        &ByteArrayTI,
        ClassTraits::fl_utils::ByteArray::MakeClassTraits,
        ClassTraits::fl_utils::ByteArray::ti,
        NULL,
        InstanceTraits::fl_utils::ByteArray::ti,
        NULL,
    };
}; // namespace fl_utils


}}} // namespace Scaleform { namespace GFx { namespace AS3

