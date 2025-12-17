/**************************************************************************

Filename    :   Platform_CoreTest.h
Content     :   
Created     :   Nov 2010
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_CoreTest_H
#define INC_SF_CoreTest_H
#pragma once

#include "Kernel/SF_Types.h"

namespace Scaleform { 

//---------------------------------------------------------------------------------------
// CoreTestBase provides shared code implementation for CoreTest template class.
class CoreTestBase
{
protected:
    CoreTestBase*        pNextTest;
    CoreTestBase*        pPrevTest;
    static CoreTestBase* pLastTest;
    const char*          pName;    

    void registerTest(const char* name);
    static CoreTestBase* getFirstTest();

public:

    CoreTestBase(const char* name)
    { registerTest(name); }
    virtual ~CoreTestBase()
    { }

    const char* GetName() const  { return pName; }
};


// CoreTest is a base class for tests that can be registered through static
// constructors, and enumerated through GetNextTest/GetPrevTest.
// This class is a template to eliminate casting for particular test types.

template<class D>
class CoreTest : public CoreTestBase
{
public:
    CoreTest(const char* name) : CoreTestBase(name)
    { }

    static D* GetFirstTest() { return (D*)getFirstTest(); }

    D*   GetNextTest() const  { return (D*)pNextTest; }
    D*   GetPrevTest() const  { return (D*)pPrevTest; }
};


//---------------------------------------------------------------------------------------
// CPUTest is a simple non-interactive test processing for which can be completed
// in one step, i.e. within the Run() virtual function.

class CPUTest : public CoreTest<CPUTest>
{
public:
    CPUTest(const char* name) : CoreTest<CPUTest>(name)
    { }

    // Override to execute test code.
    virtual void Run() = 0;
};


//---------------------------------------------------------------------------------------
// Returns next random number inclusive with the limit.
unsigned NextRandom(unsigned limit);


} // Scaleform
 
#endif // INC_SF_CoreTest_H
