/**************************************************************************

Filename    :   SF_SIMD.h
Content     :   Defines and includes for SIMD functionality.
Created     :   Dec 2010
Authors     :   Bart Muzzin

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_SF_SIMD_H
#define INC_SF_SF_SIMD_H
#pragma once

#include "GFxConfig.h"
#include "Kernel/SF_Types.h"

// Forward defines.
// Vector4f         typedef for 128 bit register for floating point data.
// Vector4i         typedef for 128 bit register for integer data.
// SF_SIMD_ALIGN    required alignment attribute for any data used by SIMD operations.
// IS               typedef for the default SIMD instruction set for a platform. This should
//                  be something that is minimally supported on the platform.

#if defined(SF_ENABLE_SIMD)
    #if defined (SF_CPU_SSE) 
        #include "SF_SIMD_SSE.h"
    #elif defined(SF_OS_XBOX360)
        #include "SF_SIMD_X360.h"
    #elif defined(SF_CPU_ALTIVEC)
        #include "SF_SIMD_Altivec.h"
    #elif defined(SF_CPU_ARM_NEON) && !defined(SF_OS_PSVITA)
        #include "SF_SIMD_ARMNeon.h"
    #else
        // Your platform does not support any SIMD processor - disabling.
        #undef SF_ENABLE_SIMD
    #endif
#endif // SF_ENABLE_SIMD


#if !defined(SF_ENABLE_SIMD)

    #define SF_SIMD_ALIGN(x) x   // No alignment for non-SIMD platforms.

    // Prefetch/Obj are special cases, are used mainly within functions, so make them available
    // always, even if they don't do anything.
    namespace SIMD
    {
        class InstructionSet
        {
        public:
            static void Prefetch( const void * /*p*/, int /*offset*/ ) { }
            template<class T>
            static void PrefetchObj( const T * /*obj*/ ) { }
        };
        typedef InstructionSet IS;
    }
#endif


#endif // INC_SF_SF_SIMD_H
