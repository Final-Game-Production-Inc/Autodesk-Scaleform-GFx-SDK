/**************************************************************************

Filename    :   Platform_CoreTest.cpp
Content     :   
Created     :   Nov 2010
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Platform_CoreTest.h"
#include "Kernel/SF_Random.h"

namespace Scaleform { 

//---------------------------------------------------------------------------------------
// ***** CoreTestBase

CoreTestBase* CoreTestBase::pLastTest = 0;

void CoreTestBase::registerTest(const char* name)
{
    pName = name;
    pPrevTest = pLastTest;
    pNextTest = 0;
    if (pLastTest)
        pLastTest->pNextTest = this;
    pLastTest = this;
}

CoreTestBase* CoreTestBase::getFirstTest()
{
    // Start from tail and walk backwards, due to declaration order
    CoreTestBase* ptest = pLastTest;
    if (ptest)
    {
        while (ptest->pPrevTest)
            ptest = ptest->pPrevTest;
    }
    return ptest;
}


//---------------------------------------------------------------------------------------
// Returns next random number inclusive with the limit.
unsigned NextRandom(unsigned limit)
{
    unsigned result = (unsigned)(Alg::Random::GetUnitFloat() * limit);    
    return result;
}

} // Scaleform
