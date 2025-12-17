/**************************************************************************

Filename    :   FxSharedObjectManager.h
Content     :   Shared object manager for FxPlayer 
Created     :   January 22, 2008
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_FxSharedObjectManager_H
#define INC_FxSharedObjectManager_H

#include "GFxConfig.h"
#ifdef GFX_AS2_ENABLE_SHAREDOBJECT
#include "GFx/GFx_SharedObject.h"
using namespace Scaleform;

#if !defined(SF_NO_FXPLAYER_AS_SHAREDOBJECT) && !defined(SF_NO_XML_SUPPORT)

#include "GFx_Kernel.h"

class FxSharedObjectWriter : public GFx::SharedObjectVisitor
{
public:
    FxSharedObjectWriter(File* pfile);
    virtual ~FxSharedObjectWriter();

    void            Begin();
    void            PushObject( const String& name );
    void            PushArray( const String& name );
    void            AddProperty( const String& name, 
                                 const String& value, 
                                 GFx::Value::ValueType type );
    void            PopArray();
    void            PopObject();
    void            End();

private:
    Ptr<File>     pFile;
    StringBuffer   LineBuf;
    UPInt           LineIndentation;

    void            WriteOut();
};

// Shared Object manager class.
class FxSharedObjectManager : public GFx::SharedObjectManagerBase
{
public:
    FxSharedObjectManager(const String& soCachePath);
    virtual ~FxSharedObjectManager();

    bool        LoadSharedObject(const String& name, 
                                 const String& localPath, 
                                 GFx::SharedObjectVisitor* psobj, 
                                 GFx::FileOpenerBase* pfo);

    GFx::SharedObjectVisitor* CreateWriter(const String& name, 
                                         const String& localPath,
                                         GFx::FileOpenerBase* pfileOpener);

private:
    StringLH   SOCachePath;
    StringLH   GeneratedFilePath;

    void        GenerateFilePath(const String& name, const String& localPath);
};


#endif  // SF_NO_FXPLAYER_AS_SHAREDOBJECT && SF_NO_XML_SUPPORT
#endif // GFX_AS2_ENABLE_SHAREDOBJECT
#endif // INC_FxSharedObjectManager_H
