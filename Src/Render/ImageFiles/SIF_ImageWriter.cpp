/**************************************************************************

Filename    :   SIF_ImageWriter.cpp
Content     :   SIF Image writer implementation
Created     :   July 2011
Authors     :   Dmitry Polenur

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "SIF_ImageFile.h"
#include "Kernel/SF_File.h"
#include "Kernel/SF_Debug.h"
//#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render { namespace SIF {

// imageData must be in target format since
// converting  may require platform-specific code
// like XGTileSurface for XBox360
bool FileWriter::Write(File* out, const ImageData& imageData,
                       const Render::ImageWriteArgs* argsVal) const
{
    if (!out || !out->IsValid() || !out->IsWritable())
        return false;

    SF_ASSERT(imageData.RawPlaneCount > 0);
    // const ImageWriteArgs* args = 0;
    if (argsVal)
    {
        //  args = (const ImageWriteArgs*)argsVal;
        
    }
    out->WriteUByte('S');
    out->WriteUByte('I');
    out->WriteUByte('F');
    out->WriteUByte(' ');
    out->WriteUByte(4 + 1 + 4 + 4 + 1 + 1 + 2); // header size before first width and height of the first plane
    out->WriteSInt32(imageData.Format);
    out->WriteUInt32(imageData.Use);
    out->WriteUByte(imageData.Flags);
    out->WriteUByte(imageData.LevelCount);
    out->WriteUInt16(imageData.RawPlaneCount);

    for (UInt16 i = 0; i < imageData.RawPlaneCount; ++i)
    {
        out->WriteUInt32(imageData.pPlanes[i].Width);
        out->WriteUInt32(imageData.pPlanes[i].Height);
        out->WriteUInt32((UInt32)imageData.pPlanes[i].Pitch);
        out->WriteUInt32((UInt32)imageData.pPlanes[i].DataSize);
        for (UPInt j = 0; j < imageData.pPlanes[i].DataSize; ++j)
        {
            out->WriteUByte(imageData.pPlanes[i].pData[j]);
        }
    }

    if (imageData.pPalette)
    {
        out->WriteUInt16(static_cast<UInt16>(imageData.pPalette->GetColorCount()));
        out->WriteUByte(imageData.pPalette->HasAlpha() ? 1 : 0);
        for (unsigned i = 0; i < imageData.pPalette->GetColorCount(); ++i)
        {
            out->WriteUInt32((*imageData.pPalette)[i].ToColor32());
        }
    }
    else
    {
        out->WriteUInt16(0);
    }
    return true;
}

FileWriter FileWriter::Instance;

}}}; // namespace Scaleform::Render::SIF

