/**************************************************************************

PublicHeader:   Kernel
Filename    :   SF_SysFile.h
Content     :   Header for all internal file management-
                functions and structures to be inherited by OS
                specific subclasses.
Created     :   July 29, 1998
Authors     :   Michael Antonov, Brendan Iribe, Andrew Reisse

Notes       :   errno may not be preserved across use of GBaseFile member functions
            :   Directories cannot be deleted while files opened from them are in use
                (For the GetFullName function)

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Kernel_SysFile_H
#define INC_SF_Kernel_SysFile_H

#include "SF_File.h"

namespace Scaleform {

// ***** Declared classes
class   SysFile;

// *** File Statistics

// This class contents are similar to _stat, providing
// creation, modify and other information about the file.
struct FileStat
{
    // No change or create time because they are not available on most systems
    SInt64  ModifyTime;
    SInt64  AccessTime;
    SInt64  FileSize;

    bool operator== (const FileStat& stat) const
    {
        return ( (ModifyTime == stat.ModifyTime) &&
                 (AccessTime == stat.AccessTime) &&
                 (FileSize == stat.FileSize) );
    }
};

// *** System File

// System file is created to access objects on file system directly
// This file can refer directly to path.
// System file can be open & closed several times; however, such use is not recommended
// This class is realy a wrapper around an implementation of File interface for a 
// particular platform.

class SysFile : public DelegatedFile
{
protected:
  SysFile(const SysFile &source) : DelegatedFile () { SF_UNUSED(source); }
public:

    // ** Constructor
    SF_EXPORT SysFile();
    // Opens a file
    // For some platforms (PS2,PSP,Wii) buffering will be used even if Open_Buffered flag is not set because 
    // of these platforms' file system limitation.
    SF_EXPORT SysFile(const String& path, int flags = Open_Read|Open_Buffered, int mode = Mode_ReadWrite); 

    // ** Open & management 
    // Will fail if file's already open
    // For some platforms (PS2,PSP,Wii) buffering will be used even if Open_Buffered flag is not set because 
    // of these platforms' file system limitation.
    SF_EXPORT bool  Open(const String& path, int flags = Open_Read|Open_Buffered, int mode = Mode_ReadWrite);
        
    SF_INLINE bool  Create(const String& path, int mode = Mode_ReadWrite)
        {   return Open(path, Open_ReadWrite|Open_Create, mode); }

    // Helper function: obtain file statistics information. In GFx, this is used to detect file changes.
    // Return 0 if function failed, most likely because the file doesn't exist.
    SF_EXPORT static bool SF_CDECL GetFileStat(FileStat* pfileStats, const String& path);
    
    // ** Overrides
    // Overridden to provide re-open support
    SF_EXPORT virtual int   GetErrorCode();

    SF_EXPORT virtual bool  IsValid();

    SF_EXPORT virtual bool  Close();
    //SF_EXPORT   virtual bool    CloseCancel();

#ifdef SF_OS_WIIU
    static void initializeSysFileSystem();
    static void shutdownSysFileSystem();
#endif
};

} // Scaleform

#endif
