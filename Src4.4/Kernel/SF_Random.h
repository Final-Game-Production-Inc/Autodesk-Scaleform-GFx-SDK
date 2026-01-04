/**************************************************************************

PublicHeader:   Kernel
Filename    :   SF_Random.h
Content     :   Pseudo-random number generator
Created     :   June 27, 2005
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel__Random_H
#define INC_SF_Kernel__Random_H

#include "Kernel/SF_Types.h"

namespace Scaleform { namespace Alg {

class Random
{
public:
    
    // Global generator.
    static UInt32   SF_STDCALL NextRandom();
    static void     SF_STDCALL SeedRandom(UInt32 seed);
    static float    SF_STDCALL GetUnitFloat();

    // In case you need independent generators.  The global
    // generator is just an instance of this.
    enum RandomConstants {
        Random_SeedCount = 8
    };

    class Generator
    {
    public:
        UInt32  Q[Random_SeedCount];
        UInt32  C;
        UInt32  I;
        
    public:
        Generator();
        void    SeedRandom(UInt32 seed);    // not necessary
        UInt32  NextRandom();
        float   GetUnitFloat();
    };

};

}}

#endif // INC_SF_Kernel__Random_H
