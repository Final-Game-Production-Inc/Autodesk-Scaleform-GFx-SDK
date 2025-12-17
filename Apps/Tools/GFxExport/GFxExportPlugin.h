/**************************************************************************

Filename    :   GFxExportPlugin.h
Content     :   GFxExport plugin interface
Created     :   May 6, 2008
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GFX_EXPORT_PLUGIN_H
#define INC_GFX_EXPORT_PLUGIN_H

#ifdef SF_OS_WIN32
#include <windows.h>
#include <winbase.h>
#include <direct.h>
#endif

class GFxExportPluginBase
{
public:
    virtual ~GFxExportPluginBase() {}
    virtual void AddRef() = 0;
    virtual void Release() = 0;
};

// Name: GFxExport_Plugin
typedef GFxExportPluginBase* (SF_STDCALL *FN_CreateGFxExportPlugin)(const char* pluginName, int revision);


class GFxExportPlugin : public GFxExportPluginBase 
{
public:
  /// Called for command-line arguments that gfxexport.exe doesn't understand.
  /// Note that the -dll argument must precede anything that is parsed by the dll.
  virtual bool    SF_STDCALL ParseCommandlineArg(const char *arg) = 0;
  virtual void    SF_STDCALL PrintUsage() = 0;

  /// Called once all the command-line arguments have been parsed and 
  /// gfxexport is about to be processing swf files.  Returning failure here
  /// will prevent gfxexport from doing any processing.
  virtual bool    SF_STDCALL Initialize() = 0;

  /// Called after the swf has been opened and the output directory made,
  /// but before assets have been read and processed
  virtual bool    SF_STDCALL BeginFile(const char *i_pFilename) = 0;

  /// Called after the swf has been fully processed
  virtual bool    SF_STDCALL EndFile() = 0;

  /// Called after all input swfs have been processed and no further work will be done
  virtual bool    SF_STDCALL Shutdown() = 0;

  class StringResult
  {
  public:
      virtual ~StringResult() { }
      virtual void  SetString(const char*)  = 0;
  };

  enum FileType
  {
      File_Image,
      File_Import,
  };

  /// Called for all assets imported into a movie.  This can be used to convert
  /// author-time source to run-time source.  For example,
  /// source could be converted into an asset id 
  virtual bool SF_STDCALL ModifyFileReference(FileType fileType, const char *poldFileName, const char *pexportName, StringResult* pnewFileName) = 0;

  /// Called for every text field in the movie.  The plugin can simply log the
  /// information (same us using gfxexport with -det) or it can replace the 
  /// movie strings with run-time localization friendly strings.  Only the
  /// contents of pnewText are available to the GFxTranslator.
  virtual bool SF_STDCALL ModifyEditText(const char *varname, const char *poldText, StringResult* pnewText) = 0;
};

#endif
