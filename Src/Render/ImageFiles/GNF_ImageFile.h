/**********************************************************************

PublicHeader:   Render
Filename    :   GNF_ImageFile.h
Content     :   
Created     :   2013/05/08
Authors     :   Bart Muzzin

Copyright   :   Copyright 2013 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#ifndef INC_SF_GNF_ImageFile_H
#define INC_SF_GNF_ImageFile_H

#include "Render/ImageFiles/Render_ImageFile.h"

namespace Scaleform { namespace Render { namespace GNF {

//--------------------------------------------------------------------
// ***** Image File Handlers
//
// GNF::FileHandler provides file format detection for GNF and its data loading.
class FileReader : public ImageFileReader_Mixin<FileReader>
{
public:
    virtual ImageFileFormat GetFormat() const { return ImageFile_GNF; }

    virtual bool    MatchFormat(File* file, UByte* header, UPInt headerSize) const;
    virtual ImageSource* ReadImageSource(File* file,
                            const ImageCreateArgs& args = ImageCreateArgs()) const;

    // Instance singleton; to be used for accessing this functionality.
    static FileReader Instance;
};


}}}; // Scaleform::Render::GNF

#endif // INC_SF_GNF_ImageFile_H
