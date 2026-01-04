/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_SharedObject.h
Content     :   AS2 Shared object interfaces
Created     :   January 20, 2009
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_SharedObjectState_H
#define INC_SF_GFX_SharedObjectState_H

#include "GFxConfig.h"
#if defined(GFX_AS2_ENABLE_SHAREDOBJECT) || defined(GFX_AS3_ENABLE_SHAREDOBJECT)

#include "Kernel/SF_Memory.h"
#include "GFx/GFx_PlayerStats.h"
#include "GFx/GFx_Loader.h"
#include "GFx/GFx_Player.h"

namespace Scaleform { namespace GFx {

#if defined(GFX_AS2_ENABLE_SHAREDOBJECT)

// ***** SharedObjectState AS2

//
// A visitor interface for shared object representations
// It is used both in reading and writing data to/from
// physical media and other sources.
//
// Reading occurs from ActionScript when the SharedObject.getLocal
// method is invoked. Writing occurs from ActionScript when the
// SharedObject.flush method is invoked.
//
class SharedObjectVisitor : public RefCountBaseNTS<SharedObjectVisitor, Stat_Default_Mem>
{
public:
    virtual void Begin() = 0;
    virtual void PushObject( const String& name ) = 0;
    virtual void PushArray( const String& name ) = 0;
    virtual void AddProperty( const String& name, const String& value, Value::ValueType type) = 0;
    virtual void PopObject() = 0;
    virtual void PopArray() = 0;
    virtual void End() = 0;
};


//
// A manager for shared objects. It provides an interface to 
// load shared object data, and also provides a specialized writer
// to save the shared object data.
//
// A default implementation is provided by FxPlayer.
//
class SharedObjectManagerBase : public State
{
public:
    SharedObjectManagerBase() 
        : State(State_SharedObject) {}

    virtual bool            LoadSharedObject(const String& name, 
                                             const String& localPath, 
                                             SharedObjectVisitor* psobj,
                                             FileOpenerBase* pfo) = 0;
    
    //
    // Return a new writer to save the shared object data
    //
    // The returned pointer should be assigned using the following pattern:
    //  Ptr<SharedObjectVisitor> ptr = *pSharedObjManager->CreateWriter(name, localPath);
    //
    virtual SharedObjectVisitor* CreateWriter(const String& name, const String& localPath,
        FileOpenerBase* pfileOpener) = 0;
};


//
// Shared state accessors
//
inline void  StateBag::SetSharedObjectManager(SharedObjectManagerBase* ptr) 
{ 
    SetState(State::State_SharedObject, ptr); 
}
inline Ptr<SharedObjectManagerBase> StateBag::GetSharedObjectManager() const
{ 
    return *(SharedObjectManagerBase*) GetStateAddRef(State::State_SharedObject); 
}

#endif  // GFX_AS2_ENABLE_SHAREDOBJECT

//////////////////////////////////////////////////////////////////////////

#if defined(GFX_AS3_ENABLE_SHAREDOBJECT)

// ***** SharedObjectState AS3

//
// A manager for shared objects. It provides an interface to 
// load shared object data, and also provides a specialized writer
// to save the shared object data.
//
// A default implementation is provided by FxPlayer.
//
class SharedObject3ManagerBase : public State
{
public:
    SharedObject3ManagerBase() 
        : State(State_SharedObject3) {}

    virtual bool Read(const String &name,
                      const String &localPath,
                      FileOpenerBase* pfileOpener,
                      StringBuffer &outData) = 0;

    virtual bool Write(const String &name,
                       const String &localPath,
                       FileOpenerBase* pfileOpener,
                       const StringBuffer &data) = 0;
};


//
// Shared state accessors
//
inline void  StateBag::SetSharedObject3Manager(SharedObject3ManagerBase* ptr)
{ 
    SetState(State::State_SharedObject3, ptr); 
}
inline Ptr<SharedObject3ManagerBase> StateBag::GetSharedObject3Manager() const
{ 
    return *(SharedObject3ManagerBase*) GetStateAddRef(State::State_SharedObject3); 
}

#endif  // GFX_AS3_ENABLE_SHAREDOBJECT


}} // namespace Scaleform::GFx

#endif // GFX_AS2_ENABLE_SHAREDOBJECT || GFX_AS3_ENABLE_SHAREDOBJECT

#endif // INC_SF_GFX_SharedObjectState_H

