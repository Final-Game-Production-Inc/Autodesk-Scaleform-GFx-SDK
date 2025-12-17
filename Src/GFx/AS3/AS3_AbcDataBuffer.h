/**************************************************************************

Filename    :   AS3_AbcDataBuffer.h
Content     :   Implementation of buffer that holds ABC data from SWF
Created     :   Jan, 2010
Authors     :   Artem Bolgar

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_AS3_AbcDataBuffer_H
#define INC_AS3_AbcDataBuffer_H

#include "GFx/GFx_Types.h"
#include "GFx/GFx_PlayerImpl.h"

namespace Scaleform {
namespace GFx {
namespace AS3 {

class AbcDataBuffer : public RefCountBase<AbcDataBuffer, StatMD_ASBinaryData_Mem>
{
public:
    String      Name;
    unsigned    DataSize;
    UByte       Flags;
    SF_AMP_CODE(UInt32 SwfHandle;)
    SF_AMP_CODE(int SwfOffset;)

    enum 
    {
        Flags_None      = 0x0,
        Flags_LazyInit  = 0x1
    };
//#ifdef SF_BUILD_DEBUG
    StringLH    FileName;
//#endif
    UByte       Data[1];

    AbcDataBuffer(const String& name, unsigned flags, unsigned dataSize) 
        : Name(name), DataSize(dataSize), Flags((UByte)flags)
#ifdef SF_AMP_SERVER
        , SwfHandle(0), SwfOffset(0)
#endif
    {}

    // Placement new/delete.
    SF_MEMORY_DEFINE_PLACEMENT_NEW

    bool                IsNull() const          { return DataSize == 0; }
    unsigned            GetLength() const       { return DataSize; }
    const UByte*        GetBufferPtr() const    { return (IsNull()) ? NULL : Data; }

    const char*         GetFileName() const    
    { 
#ifdef SF_BUILD_DEBUG
        return FileName.ToCStr();
#else
        return NULL;
#endif
    }

    SF_MEMORY_REDEFINE_NEW(AbcDataBuffer, StatMD_ASBinaryData_Mem);                                  \
};

}}} // Scaleform::GFx::AS3

#endif // INC_AS3_AbcDataBuffer_H

