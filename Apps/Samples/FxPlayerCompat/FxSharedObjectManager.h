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

#if !defined(GFC_NO_FXPLAYER_AS_SHAREDOBJECT) && !defined(GFC_NO_XML_SUPPORT)

class FxSharedObjectWriter : public GFxSharedObjectVisitor
{
public:
    FxSharedObjectWriter(GFile* pfile);
    virtual ~FxSharedObjectWriter();

    void            Begin();
    void            PushObject( const GString& name );
    void            PushArray( const GString& name );
    void            AddProperty( const GString& name, 
                                 const GString& value, 
                                 GFxValue::ValueType type );
    void            PopArray();
    void            PopObject();
    void            End();

private:
    GPtr<GFile>     pFile;
    GStringBuffer   LineBuf;
    UPInt           LineIndentation;

    void            WriteOut();
};

// Shared Object manager class.
class FxSharedObjectManager : public GFxSharedObjectManagerBase
{
public:
    FxSharedObjectManager(const GString& soCachePath);
    virtual ~FxSharedObjectManager();

    bool        LoadSharedObject(const GString& name, 
                                 const GString& localPath, 
                                 GFxSharedObjectVisitor* psobj, 
                                 GFxFileOpenerBase* pfo);

    GFxSharedObjectVisitor* CreateWriter(const GString& name, 
                                         const GString& localPath,
                                         GFxFileOpenerBase* pfileOpener);

private:
    GStringLH   SOCachePath;
    GStringLH   GeneratedFilePath;

    void        GenerateFilePath(const GString& name, const GString& localPath);
};


#endif  // GFC_NO_FXPLAYER_AS_SHAREDOBJECT && GFC_NO_XML_SUPPORT

#endif // INC_FxSharedObjectManager_H
