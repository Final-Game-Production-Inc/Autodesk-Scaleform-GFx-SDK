/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_PlayerTasks.h
Content     :   Input handling
Created     :   
Authors     :   Artem Bolgar

Notes       :   This file contains class declarations used in
GFxPlayerImpl.cpp only. Declarations that need to be
visible by other player files should be placed
in DisplayObject.h.

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_SF_GFX_PlayerTasks_H
#define INC_SF_GFX_PlayerTasks_H

#include "GFx/GFx_DisplayObject.h"
#include "Render/Render_Math2D.h"
#include "Kernel/SF_File.h"

namespace Scaleform { namespace GFx {

class MovieImpl;

// ***** Load Queue used by movie root

class LoadQueueEntry : public NewOverrideBase<StatMV_Other_Mem>
{
    friend class MovieImpl;
    // Next in load queue
    LoadQueueEntry*          pNext;

public:

    // Load type, technically not necessary since URL & Level values are enough.
    // May become useful in the future, as we add other load options.
    enum LoadTypeFlags 
    {
        LTF_None        = 0,
        LTF_UnloadFlag  = 0x01, // Unload instead of load
        LTF_LevelFlag   = 0x02,  // Level instead of target character.
        LTF_VarsFlag    = 0x04,
        LTF_XMLFlag     = 0x08,
        LTF_CSSFlag     = 0x10,
        LTF_BinaryFlag  = 0x20
    };  
    enum LoadType
    {   
        LT_LoadMovie    = LTF_None,
        LT_UnloadMovie  = LTF_UnloadFlag,
        LT_LoadLevel    = LTF_LevelFlag,
        LT_UnloadLevel  = LTF_UnloadFlag | LTF_LevelFlag,
        LT_LoadXML      = LTF_XMLFlag,
        LT_LoadCSS      = LTF_CSSFlag,
        LT_LoadText     = LTF_VarsFlag,
        LT_LoadBinary   = LTF_BinaryFlag
    };

    enum LoadMethod
    {
        LM_None,
        LM_Get,
        LM_Post,
        LM_Put,
        LM_Delete
    };

    LoadType            Type;
    LoadMethod          Method;
    String              URL;
    UInt32              EntryTime;
    bool                QuietOpen;
    bool                Canceled;

    // Constructor helper.      
    void    PConstruct(LoadType type, const String &url, LoadMethod method, bool quietOpen)
    {
        Type        = type;
        Method      = method;
        pNext       = 0;
        URL         = url;
        EntryTime   = SF_MAX_UINT32;
        QuietOpen   = quietOpen;
        Canceled    = false;
    }

    // *** Constructors
    LoadQueueEntry(const String &url, LoadMethod method, bool loadingVars = false, bool quietOpen = false)
    {
        LoadTypeFlags typeFlag;
        if (loadingVars)
        {
            typeFlag = LTF_VarsFlag;
        }
        else
        {
            typeFlag = url ? LTF_None : LTF_UnloadFlag;
        }
        PConstruct((LoadType) (typeFlag), url, method, quietOpen);
    }

    virtual ~LoadQueueEntry() {}

    LoadQueueEntry* GetNext() const     { return pNext; }
};

class LoadQueueEntryMT : public NewOverrideBase<StatMV_Other_Mem>
{
protected:
    friend class MovieImpl;
    LoadQueueEntryMT*      pNext;
    LoadQueueEntryMT*      pPrev;

    MovieImpl*             pMovieImpl;
    LoadQueueEntry*        pQueueEntry;

public:
    LoadQueueEntryMT(LoadQueueEntry* pqueueEntry, MovieImpl* pmovieRoot);
    virtual ~LoadQueueEntryMT();

    void Cancel();

    // Check if a movie is loaded. Returns false in the case if the movie is still being loaded.
    // Returns true if the movie is loaded completely or in the case of errors.
    virtual bool LoadFinished() = 0;

    // Check if preloading stage of the loading task finished.
    virtual bool IsPreloadingFinished() { return true; }

    LoadQueueEntry* GetQueueEntry() const { return pQueueEntry; }
    LoadQueueEntryMT* GetNext() const     { return pNext; }
    LoadQueueEntryMT* GetPrev() const     { return pPrev; }
};

// ***** GFxMoviePreloadTask
// The purpose of this task is to find a requested movie on the media, create a moviedefimp and 
// submit a task for the actual movie loading.
class MoviePreloadTask : public Task
{
public:
    MoviePreloadTask(MovieImpl* pmovieRoot, const String& url, bool stripped, bool quietOpen);

    virtual void    Execute();

    MovieDefImpl* GetMoiveDefImpl();

    bool IsDone() const;

private:
    Ptr<LoadStates>     pLoadStates;
    unsigned            LoadFlags;
    String              Level0Path;

    String              Url;
    String              UrlStrGfx;
    Ptr<MovieDefImpl>   pDefImpl;
    volatile unsigned   Done;
};

#ifdef GFX_AS2_ENABLE_LOADVARS
// ****** LoadVarsTask
// Reads a file with loadVars data on a separate thread
class LoadVarsTask : public Task
{
public:
    LoadVarsTask(LoadStates* pls, const String& level0Path, const String& url);

    virtual void    Execute();

    // Retrieve loadVars data and file length. Returns false if data is not ready yet.
    // if returned fileLen is less then 0 then the requested url was not read successfully. 
    bool GetData(String* data, int* fileLen, bool* succeeded) const;

private:
    Ptr<LoadStates>     pLoadStates;
    String              Level0Path;
    String              Url;

    String              Data;
    int                 FileLen;
    volatile unsigned   Done;
    bool                Succeeded;
};

class LoadQueueEntryMT_LoadVars : public LoadQueueEntryMT
{
    Ptr<LoadVarsTask> pTask;
    Ptr<LoadStates>   pLoadStates;

public:
    LoadQueueEntryMT_LoadVars(LoadQueueEntry* pqueueEntry, MovieImpl* pmovieRoot);
    ~LoadQueueEntryMT_LoadVars();

    // Check if a data is loaded. Returns false in the case if the data is still being loaded.
    // Returns true if the data is loaded completely or in the case of errors.
    bool LoadFinished();
};


// ****** LoadBinaryTask
// Reads a file with binary data on a separate thread
class LoadBinaryTask : public Task
{
public:
    LoadBinaryTask(LoadStates* pls, const String& level0Path, const String& url);

    virtual void    Execute();

    // Retrieve binary data and file length. Returns false if data is not ready yet.
    // if returned fileLen is less then 0 then the requested url was not read successfully. 
    bool GetData(ArrayPOD<UByte>* data, int* fileLen, bool* succeeded) const;

private:
    Ptr<LoadStates>     pLoadStates;
    String              Level0Path;
    String              Url;

    ArrayPOD<UByte>     Data;
    int                 FileLen;
    volatile unsigned   Done;
    bool                Succeeded;
};

class LoadQueueEntryMT_LoadBinary : public LoadQueueEntryMT
{
    Ptr<LoadBinaryTask> pTask;
    Ptr<LoadStates>   pLoadStates;

public:
    LoadQueueEntryMT_LoadBinary(LoadQueueEntry* pqueueEntry, MovieImpl* pmovieRoot);
    ~LoadQueueEntryMT_LoadBinary();

    // Check if a data is loaded. Returns false in the case if the data is still being loaded.
    // Returns true if the data is loaded completely or in the case of errors.
    bool LoadFinished();
};
#endif //#ifdef GFX_AS2_ENABLE_LOADVARS


extern String GetUrlStrGfx(const String& url);

}} // namespace Scaleform::GFx

#endif // INC_SF_GFX_PlayerTasks_H

