/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_Watermark.h
Content     :   Watermark strings and cutoff date for Eval versions
Created     :   
Authors     :   Ryan Holtz

Notes       :   Generate the correct SF_WATERMARK_END_DATE by going
                to http://www.epochconverter.com and using the "Human
                date to Timestamp" option, then add three zeros onto
                the right-hand side since we use milliseconds and the
                website uses seconds.

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#if defined(SF_BUILD_CONSUMER)
#define SF_WATERMARK_TEXT_LINE1 ""
#define SF_WATERMARK_TEXT_LINE2 ""
#define SF_WATERMARK_TEXT_LINE3 "%d days left in Scaleform Eval - Commercial use prohibited by license"

#if defined(SF_OS_PS3) || defined(SF_OS_PSVITA)
#define SF_WATERMARK_END_DATE 1356983843000ull
#else
#define SF_WATERMARK_END_DATE 1356983843000LL
#endif
#endif
