/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_ASSoundIntf.h
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/


#ifndef INC_SF_GFX__ASSOUNDINTF_H
#define INC_SF_GFX__ASSOUNDINTF_H

#include "Kernel/SF_Types.h"

namespace Scaleform { namespace GFx {

class ASSoundIntf
{
public:
    virtual ~ASSoundIntf() {}

    virtual void    ReleaseTarget() = 0;
    virtual void    ExecuteOnSoundComplete() = 0;

    virtual void*   GetOwner() { return NULL; }

    virtual bool    HasPrivateOverride() { return false; }
    virtual int     PrivateVolume() { return 100; }
    virtual int     PrivatePan()    { return 0; }
};

}} // Scaleform::GFx

#endif // INC_SF_GFX__ASSOUNDINTF_H
