/**************************************************************************

Filename    :   GFxExportPluginHandler.cpp
Content     :   GFxExport plugin handler class implementation
Created     :   May 6, 2008
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#include "GFxExportPluginHandler.h"
#include "GFxExport.h"

#include "Kernel/SF_MsgFormat.h"
using namespace Scaleform;

#ifdef SF_OS_WIN32
#else
#include <dlfcn.h>
#endif

void    GFxExportPluginHandler::RegisterLog(GFxExportListener *logger)
{
    SF_ASSERT(logger);
    Logger = logger;
}

int GFxExportPluginHandler::ParseCommandlineArg(const char* arg)
{
    if ( pLibraryPlugin )
    {
        if ( pLibraryPlugin->ParseCommandlineArg(arg) )
        {
            return 1;
        }
        else 
        {
            return -1;
        }
    }
    return 0;
}


bool GFxExportPluginHandler::LoadPluginLibrary( const char *pLibraryName)
{
    String errorMsg;

    const char *szFactoryFunction = "CreateGFxExportPlugin";
#ifdef SF_OS_WIN32
    SetErrorMode(0);
    HMODULE hDllPlugin = LoadLibraryA(pLibraryName);
    if ( !hDllPlugin )
    {
        Format(errorMsg, "could not load dll '{0}' ({1})!", pLibraryName, GetLastError());
        Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
        return false;
    }
    // We're going to leak the ref on hDllPlugin.  The OS will clean it up on shutdown.

    FN_CreateGFxExportPlugin fnFactory = (FN_CreateGFxExportPlugin)GetProcAddress(hDllPlugin, szFactoryFunction);

#else
     void *handle = dlopen (pLibraryName, RTLD_LAZY);
     if (!handle)
     {
         Format(errorMsg, "could not load library '%{0}' !", pLibraryName);
         Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
         return false;
     }
     FN_CreateGFxExportPlugin fnFactory = (FN_CreateGFxExportPlugin)dlsym(handle, szFactoryFunction);

#endif

     if ( !fnFactory )
     {
         Format(errorMsg, "could not find the function {0} in {1}!", szFactoryFunction, pLibraryName);
         Logger->OnMessage(GFxExportListener::MSG_Error, errorMsg);
         return false;
     }

    GFxExportPlugin *pPlugin = NULL;

     pPlugin = (GFxExportPlugin*)fnFactory(PLUGIN_NAME, PLUGIN_REVISION);

    if (!pPlugin)
    {
        Logger->OnMessage(GFxExportListener::MSG_Error, "Could not create the plugin object");
        return false;
    }
    pLibraryPlugin.SetPtr(*pPlugin);

    return true;
}


bool GFxExportPluginHandler::HandleTag( const TagInfo& tag, File* pin, File* pout )
{
    switch (tag.TagType)
    {
    case Tag_Import:
    case Tag_Import2:
        return HandleTag_Import(tag, pin, pout);

    case Tag_DefineEditText:
        return HandleTag_DefineEditText(tag, pin, pout);

    default:
        return false;
    }
}


void GFxExportPluginHandler::WriteStringToGFile( File *pFile, const char *pstr )
{
    // The reverse of Stream::ReadString()
    const char *p = pstr;
    do 
    {
        pFile->WriteSByte(*p);
    } while (*p++);
}


bool GFxExportPluginHandler::HandleTag_Import( const TagInfo& tag, File* pin, File* pout )
{
    // Patterned after the code in GFx_ImportLoader(), in GFxTagLoaders.cpp
    Stream streamIn(pin, Memory::GetGlobalHeap(), NULL, NULL);
    Stream  *p = &streamIn; // So we can block copy code from GFxTagLoaders
    SF_ASSERT(p->Tell() == tag.TagDataOffset);


    String strOldSource;
    String strSource;

    p->ReadString(&strOldSource);
    UInt16  count = p->ReadU16();
    
    if ( !ModifyFileReference(GFxExportPlugin::File_Import, strOldSource, String(), strSource) )
    {
        strSource = strOldSource;
    }    

    unsigned len = tag.TagLength - (p->Tell() - tag.TagDataOffset); // input bytes remaining
    len += (unsigned)strSource.GetSize()+1;
    len += 2; // count
    GFxDataExporter::WriteTagHeader(pout, Tag_Import2, len);
    unsigned oposDataStart = pout->Tell();
    WriteStringToGFile(pout, strSource.ToCStr());
    pout->WriteUInt16(count);

    UInt16 val = 1;
    if (tag.TagType == Tag_Import2)
    {
        // 将内部的val重命名为tagVal，避免隐藏外部变量
        UInt16 tagVal = p->ReadU16();
        SF_UNUSED(tagVal);
        SF_DEBUG_WARNING1(tagVal != 1, "Unexpected attribute in ImportAssets2 - 0x%X instead of 1", tagVal);
    }
    pout->WriteUInt16(val);

    for (int i = 0; i < count; i++)
    {               
        String symbolName;
        UInt16    id = p->ReadU16();
        p->ReadString(&symbolName);

        pout->WriteUInt16(id);
        WriteStringToGFile(pout, symbolName.ToCStr());
    }

    unsigned posCur = pout->Tell();
    SF_ASSERT(posCur-oposDataStart == len);
    pin->Seek(tag.TagLength + tag.TagDataOffset);
    SF_UNUSED2(posCur,oposDataStart); // To get rid of warning in Release
    return true;
}


bool GFxExportPluginHandler::HandleTag_DefineEditText( const TagInfo& tag, File* pin, File* pout )
{
    PluginStringResult text;
//    char text[4096];
//    text[0] = 0;
    String varname;
    String deftext;

    {
        // Patterned after the code in GFx_DefineEditTextLoader(), in GFxTextField.cpp
        Stream streamIn(pin, Memory::GetGlobalHeap(), NULL, NULL);
        Stream  *p = &streamIn; // So we can block copy code from the loader
        SF_ASSERT(p->Tell() == tag.TagDataOffset);

        /*UInt16  characterId = */p->ReadU16();

        // Patterned after the code in TextFieldDef::Read, in GFxTextField.cpp
        RectF TextRect;
        p->ReadRect(&TextRect);

        p->Align();
        bool    hasText = p->ReadUInt(1) ? true : false;
        bool bWordWrap = p->ReadUInt(1) != 0; SF_UNUSED(bWordWrap);
        bool bMultiline = p->ReadUInt(1) != 0; SF_UNUSED(bMultiline);
        bool bPassword = p->ReadUInt(1) != 0;  SF_UNUSED(bPassword);
        bool bReadOnly = p->ReadUInt(1) != 0; SF_UNUSED(bReadOnly);

        bool    hasColor = p->ReadUInt(1) ? true : false;
        bool    hasMaxLength = p->ReadUInt(1) ? true : false;
        bool    hasFont = p->ReadUInt(1) ? true : false;

        p->ReadUInt(1);    // reserved
        bool bAutoSize = p->ReadUInt(1) != 0; SF_UNUSED(bAutoSize);
        bool    hasLayout = p->ReadUInt(1) ? true : false;
        bool bSelectable = p->ReadUInt(1) == 0; SF_UNUSED(bSelectable);
        bool bBorder  = p->ReadUInt(1) != 0; SF_UNUSED(bBorder);
        p->ReadUInt(1);    // reserved

        // In SWF 8 text is *ALWAYS* marked as HTML.
        // Correction, AB: no, only if kerning is ON
        // In SWF <= 7, that is not the case.
        bool bHtml = p->ReadUInt(1) != 0; SF_UNUSED(bHtml);
        bool bUseDeviceFont = p->ReadUInt(1) == 0; SF_UNUSED(bUseDeviceFont);

        if (hasFont)
        {
            p->ReadU16();
            p->ReadU16();
        }

        if (hasColor)
        { 
            Color color;
            p->ReadRgba(&color);
        }

        if (hasMaxLength)
        {
            p->ReadU16();
        }

        if (hasLayout)
        {
            /*Alignment = (alignment) */p->ReadU8();
            /*LeftMargin = (float) */p->ReadU16();
            /*RightMargin = (float) */p->ReadU16();
            /*Indent = (float) */p->ReadS16();
            /*Leading = (float) */p->ReadS16();
        }

        p->ReadString(&varname);
        if (hasText)        
            p->ReadString(&deftext);
    }

    bool res = false;
    int lenPreString = tag.TagLength;
    if (!deftext.IsEmpty())
    {
        lenPreString -= (unsigned)deftext.GetSize() + 1;
        res = pLibraryPlugin->ModifyEditText(varname.ToCStr(), deftext.ToCStr(), &text);
    }

    if (!res)
        return false;

    int len = lenPreString + (unsigned)text.Buff.GetSize() + 1;

    GFxDataExporter::WriteTagHeader(pout, tag.TagType, len);
    pin->Seek(tag.TagDataOffset);
    if (pout->CopyFromStream(pin, lenPreString) == -1)
    {
        return false;
    }
    WriteStringToGFile(pout, text.Buff.ToCStr());

    pin->Seek(tag.TagDataOffset + tag.TagLength);
    return true;
}

bool GFxExportPluginHandler::ModifyFileReference( GFxExportPlugin::FileType fileType, const String& oldFileName, const String& exportName, String& newFileName)
{
    
    PluginStringResult stringResult;

    if ( pLibraryPlugin )
    {
        const char *szExportName = exportName.GetSize() ? exportName.ToCStr() : NULL;
        if (pLibraryPlugin->ModifyFileReference(fileType, oldFileName.ToCStr(), szExportName, &stringResult))
        {
            newFileName = stringResult.Buff;
            String msg;
            Format(msg, "\"{0}\" -> \"{1}\"", oldFileName.ToCStr(), newFileName.ToCStr());
            Logger->OnMessage(GFxExportListener::MSG_Normal, msg);
            return true;
        }
        else
            return false;
    }
    return false;
}

bool GFxExportPluginHandler::WantsTag( TagType i_eTag )
{
    if ( !pLibraryPlugin ) return false;
    switch (i_eTag)
    {
    case Tag_Import:
    case Tag_Import2:
    case Tag_DefineEditText:
        return true;
    default:
        break;
    }
    return false;
}
