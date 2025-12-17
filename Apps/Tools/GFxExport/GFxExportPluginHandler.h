/**************************************************************************

Filename    :   GFxExportPluginHandler.h
Content     :   GFxExport plugin handler class 
Created     :   May 6, 2008
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GFxExportPluginHandler_H
#define INC_GFxExportPluginHandler_H

#include "Kernel/SF_Std.h"
#include "Kernel/SF_Types.h"
#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_File.h"
#include "Kernel/SF_String.h"
#include "GFx/GFx_Stream.h"

#include "GFxExportPlugin.h"

using namespace Scaleform;
using namespace GFx;

class GFxExportListener;

#define PLUGIN_NAME         "GFxExport_Plugin"
#define PLUGIN_REVISION     1

class GFxExportPluginHandler
{
public:
    void RegisterLog(GFxExportListener *logger);
    bool WantsTag(TagType i_eTag);

    bool LoadPluginLibrary(const char *pLibraryName);
    bool Initialize() { return pLibraryPlugin ? pLibraryPlugin->Initialize() : true; }
    bool BeginFile(const char *i_pSWF) { return pLibraryPlugin ? pLibraryPlugin->BeginFile(i_pSWF) : true; }
    bool EndFile() { return pLibraryPlugin ? pLibraryPlugin->EndFile() : true; }
    bool Shutdown() { return pLibraryPlugin ? pLibraryPlugin->Shutdown() : true; }

    bool HandleTag(const TagInfo& tag, File* pin, File* pout);

    bool ModifyFileReference(GFxExportPlugin::FileType fileType,  const String& oldFileName, const String& exportName, String& newFileName);

    int ParseCommandlineArg(const char* arg);

    void PrintUsage()
    {
        if (pLibraryPlugin)
            pLibraryPlugin->PrintUsage();
    }
   
    class PluginStringResult : public GFxExportPlugin::StringResult
    {
    public:
        String Buff;
        virtual void  SetString(const char* buff) {Buff = buff; }
    }; 

protected:
    Ptr<GFxExportPlugin> pLibraryPlugin;
    GFxExportListener    *Logger;


    bool HandleTag_Import(const TagInfo& tag, File* pin, File* pout);
    bool HandleTag_DefineEditText(const TagInfo& tag, File* pin, File* pout);

    static void WriteStringToGFile(File *pFile, const char *pstr);
};

#endif //#ifndef INC_GFxExportPluginHandler_H
