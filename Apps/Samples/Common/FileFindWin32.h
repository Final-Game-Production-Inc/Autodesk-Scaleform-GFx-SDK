/**************************************************************************

Filename    :   FileFindWin32.h
Content     :   Helper functions for FxWin32App and Direct3DXbox360App
Created     :   Jan, 2008
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_MSFILEFIND_H
#define INC_MSFILEFIND_H

#include "GFx_Kernel.h"

#ifdef SF_OS_WINCE

#include "Kernel/SF_String.h"
#include "GFx.h"

inline HANDLE FindFirstFileA( LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData )
{
    unsigned len = strlen(lpFileName);
    WStringBuffer wstr;
    wstr.Resize(len+1);
    mbstowcs(wstr.GetBuffer(), lpFileName, len);
    wstr.GetBuffer()[len] = 0;
    WIN32_FIND_DATAW wfind;
    HANDLE hFind = FindFirstFileW(wstr.GetBuffer(), &wfind);
    lpFindFileData->dwFileAttributes = wfind.dwFileAttributes;
    wcstombs(lpFindFileData->cFileName, wfind.cFileName, SFwcslen(wfind.cFileName));
    lpFindFileData->cFileName[SFwcslen(wfind.cFileName)] = 0;
    return hFind;
}

inline BOOL FindNextFileA( HANDLE hFind, LPWIN32_FIND_DATAA lpFindFileData )
{
    WIN32_FIND_DATAW wfind;
    BOOL result = FindNextFileW(hFind, &wfind);
    lpFindFileData->dwFileAttributes = wfind.dwFileAttributes;
    wcstombs(lpFindFileData->cFileName, wfind.cFileName, SFwcslen(wfind.cFileName));
    lpFindFileData->cFileName[SFwcslen(wfind.cFileName)] = 0;
    return result;
}

#endif


// Determine if the file name has a specified extension
inline bool    MatchFileExtension(const char *pname, const char *pext)
{
    size_t nameLen = strlen(pname);
    size_t extLen  = strlen(pext);
    if (nameLen <= extLen)
        return 0;
    return (SFstricmp(pname + (nameLen - extLen), pext) == 0);
}

inline bool    MatchFileExtensionList(const char *pname)
{
    static const char *pextList[] = {".swf", ".gfx", 0};
    const char        **p         = pextList;

    while (*p != 0)
    {
        if (MatchFileExtension(pname, *p))
            return 1;
        p++;
    }
    return 0;
}

inline HANDLE  FindFirstFile_Masked(LPCSTR pfileName, LPWIN32_FIND_DATAA pfind)
{
    HANDLE hfind = FindFirstFileA(pfileName, pfind);
    if (hfind == INVALID_HANDLE_VALUE)
        return hfind;

    // Find next
    do
    {
        if (MatchFileExtensionList(pfind->cFileName))
            return hfind;
    } while ( FindNextFileA(hfind, pfind) );

    FindClose(hfind);
    return INVALID_HANDLE_VALUE;   
}

inline BOOL    FindNextFile_Masked(HANDLE hfind, LPWIN32_FIND_DATAA pfind)
{ 
    while (FindNextFileA(hfind, pfind))
    {
        if (MatchFileExtensionList(pfind->cFileName))
            return 1;
    }
    return 0;   
}



// Find the next/previous SWF file in the directory
// Search path must include the directory and a mask. pfilename should NOT include directory.
// Filled in pfind->cFileName will not contain directory either.
inline bool FindNextFileInList(WIN32_FIND_DATAA *pfind, char *psearchPath, char *pfilename, bool prev)
{
    WIN32_FIND_DATAA firstFind;
    WIN32_FIND_DATAA prevFind;
    WIN32_FIND_DATAA newFind;

    HANDLE hFind = FindFirstFile_Masked( psearchPath, &newFind );
    if( INVALID_HANDLE_VALUE == hFind )
        return 0;
    // Save first item in case we will need it
    memcpy(&firstFind, &newFind, sizeof(WIN32_FIND_DATAA));
    prevFind=firstFind;

    bool returnLast = 0;
    bool found      = 0;

    // If we are searching for the previous item and the first
    // item match, wrap to last.
    if (prev && !SFstricmp(newFind.cFileName, pfilename))
        returnLast = 1;

    do {    
        // If the file was found in the previous iteration, we are done
        if (found && !prev)
        {
            // Return next item
            FindClose( hFind );
            memcpy(pfind, &newFind, sizeof(WIN32_FIND_DATAA));           
            return 1;
        }

        // If found, the next file will be outs
        if (!SFstricmp(newFind.cFileName, pfilename))
        {
            if (prev && !returnLast)
            {
                // Return previous item
                FindClose( hFind );
                memcpy(pfind, &prevFind, sizeof(WIN32_FIND_DATAA));
                return 1;
            }
            found = 1;
        }

        memcpy(&prevFind, &newFind, sizeof(WIN32_FIND_DATAA));
        
    } while( FindNextFile_Masked( hFind, &newFind ) );

    FindClose( hFind );

    // If we are return last item, check if it exists
    if (returnLast)
    {
        // Same name? There is only one item, so fail.
        if (!SFstricmp(prevFind.cFileName, pfilename))
            return 0;       
        memcpy(pfind, &prevFind, sizeof(WIN32_FIND_DATAA));
        return 1;
    }

    // If the file was found, but there is no next file, return first file
    if (found && !prev)
    {
        memcpy(pfind, &firstFind, sizeof(WIN32_FIND_DATAA));
        return 1;       
    }

    return 0;
}


// Finds the next or previous sibling file, replacing filename buffer with 
// new name. Note that this signature is system-independent.
// TBD: Should we place it into Platform_FindFile ?

bool FindNextSiblingFile(char* filename, bool prev)
{
	WIN32_FIND_DATAA f;
	char    path[256];
	char*   pfilename=filename;
	SPInt len = strlen(pfilename);
	for (SPInt i=len; i>0; i--) 
	{
		if (pfilename[i]=='/' || pfilename[i]=='\\') {
			pfilename = pfilename+i+1;
			break;
		}
	}
	SFstrcpy(path,sizeof(path),filename);
	unsigned int path_length =(unsigned int) (strlen(filename)-strlen(pfilename));
	path[path_length]=0; //current directory

	//  if( _getcwd( &path[0], 256 ))
	{
		SFstrcat(path,256,"*.*");
		if (FindNextFileInList(&f, path, pfilename, prev))
		{
			SFstrcpy(&filename[path_length], 256-path_length, f.cFileName);
			return true;
		}
	}
	return false;
}


#endif //INC_MSFILEFIND_H
