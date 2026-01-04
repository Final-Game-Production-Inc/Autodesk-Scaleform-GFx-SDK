/**************************************************************************

PublicHeader:   Kernel
Filename    :   SF_AutoPtr.h
Content     :   Simple auto pointers: AutoPtr, ScopePtr
Created     :   2008
Authors     :   MaximShemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_AutoPtr_H
#define INC_SF_Kernel_AutoPtr_H

#include "SF_Types.h"

namespace Scaleform {

// ***** AutoPtr
//
// AutoPtr similar to std::auto_ptr. It keeps a boolean ownership 
// flag, so can be correctly assigned and copy-constructed, but 
// generates extra code.
// DO NOT use in performance critical code.
// DO NOT store in containers (Array, Hash, etc).
//------------------------------------------------------------------------
template<class T> class AutoPtr
{
public:
    AutoPtr(T* p = 0) : pObject(p), Owner(true) {}
    AutoPtr(T* p, bool owner) : pObject(p), Owner(owner) {}
    AutoPtr(const AutoPtr<T>& p) : pObject(0), Owner(p.Owner) 
    { 
        pObject = p.constRelease(); 
    }
    ~AutoPtr(void) { Reset(); }
 
    AutoPtr<T>& operator=(const AutoPtr<T>& p)
    {
        if (this != &p) 
        {
            bool owner = p.Owner;
            Reset(p.constRelease());
            Owner = owner;
        }
        return *this;
    }
 
    AutoPtr<T>& operator=(T* p)
    {
         Reset(p);
         return *this;
    }

    T& operator* ()     const { return *pObject; }
    T* operator->()     const { return  pObject; }
    T* GetPtr()         const { return  pObject; }
    operator bool(void) const { return  pObject != 0; }    

    T* Release()
    {
        Owner = false;
        return pObject;
    }

    void Reset(T* p = 0, bool owner = true)
    {
        if (pObject != p) 
        {
            if (pObject && Owner) 
                delete Release();
            pObject = p;
        }
        Owner = p != 0 && owner;
     }

private:
    T*   pObject; 
    bool Owner;

    // Release for const object.
    T* constRelease(void) const
    {
        return const_cast<AutoPtr<T>*>(this)->Release();
    }
};



// ***** ScopePtr
//
// A simple wrapper used for automatic object deleting
// No ownership, no assignment operator, no copy-constructor,
// so that no extra code is generated compared with manual new/delete
//------------------------------------------------------------------------
template<class T> class ScopePtr
{
public:
    ScopePtr(T* p = 0) : pObject(p) {}
    ~ScopePtr(void) { delete pObject; }

    T& operator* () const { return *pObject; }
    T* operator->() const { return  pObject; }
    T* GetPtr()     const { return  pObject; }
    operator bool() const { return  pObject != 0; }

private:
    // Copying is prohibited
    ScopePtr(const ScopePtr<T>&);
    const ScopePtr<T>& operator=(const ScopePtr<T>&); 

    T* pObject;
};

} // Scaleform

#endif

