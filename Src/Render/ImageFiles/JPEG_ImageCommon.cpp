/**************************************************************************

Filename    :   JPEG_ImageCommon.cpp
Content     :   Common source file for JPEG Reader/Writer implementation
Created     :   June 24, 2005
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "JPEG_ImageCommon.h"

#ifdef SF_ENABLE_LIBJPEG

namespace Scaleform { namespace Render { namespace JPEG {


JpegErrorHandler::JpegErrorHandler()
{
    // Align jmp_buf on 32-bytes boundary (see comment in class)
    psetjmp_buffer = (jmp_buf*)SF_MEMALIGN(sizeof(jmp_buf), 32, Stat_Default_Mem);
}
JpegErrorHandler::~JpegErrorHandler()
{
    SF_FREE_ALIGN(psetjmp_buffer);
}

void JpegErrorExit(j_common_ptr cinfo)
{
    JpegErrorHandler* myerr = (JpegErrorHandler*) cinfo->err;
    char buffer[JMSG_LENGTH_MAX];

    /* Create the message */
    (*cinfo->err->format_message) (cinfo, buffer);
    SFstrcpy(myerr->errorMessage, sizeof(myerr->errorMessage), buffer);

    SF_DEBUG_ERROR1(1, "GJPEGUtil - internal error in jpeglib: %s\n",
                    myerr->errorMessage);
#ifndef SF_NO_LONGJMP
    // Return control to the setjmp point
    longjmp(*myerr->psetjmp_buffer, 1);
#else
    exit(1);
#endif
}

jpeg_error_mgr* SetupJpegErr(JpegErrorHandler* jerr)
{
    // Set up defaults.
    jpeg_error_mgr* pe = jpeg_std_error(&jerr->pub);
    jerr->pub.error_exit = JpegErrorExit;
    return pe;
}

}}} // namespace Scaleform::Render::JPEG

#endif // SF_ENABLE_LIBJPEG
