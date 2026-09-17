/**********************************************************************

PublicHeader:   Render
Filename    :   SF_Uncopyable.h
Content     :   Inspired by boost::uncopyable.
Created     :   2014/03/19
Authors     :   Bart Muzzin

Copyright   :   Copyright 2014 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#pragma once
#ifndef INC_SF_Uncopyable_h
#define INC_SF_Uncopyable_h

class Unassignable
{
private:
    Unassignable& operator=(const Unassignable&);
};

class Uncopyable
{
private:
    Uncopyable(const Uncopyable&);
};

class UnassignableAndUncopyable
{
private:
    UnassignableAndUncopyable& operator=(const UnassignableAndUncopyable&);
    UnassignableAndUncopyable(const UnassignableAndUncopyable&);
};

#endif
