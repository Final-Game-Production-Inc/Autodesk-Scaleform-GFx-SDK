/**************************************************************************

Filename    :   GameUIRuntimeAtlas.h
Content     :   
Created     :
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GameUIRuntimeAtlas_H
#define INC_GameUIRuntimeAtlas_H

#include "Kernel/SF_String.h"
#include "GFx/GFx_ImageCreator.h"
#include "GFx/GFx_ImageResource.h"

using namespace Scaleform;

class GameUIRuntimeAtlasSubImage;

class GameUIRuntimeAtlasManager : public GFx::ImageCreator
{
    friend class GameUIRuntimeAtlasSubImage;

public:

    GameUIRuntimeAtlasManager(Render::TextureManager* texMgr) : GFx::ImageCreator(texMgr) {}
    virtual ~GameUIRuntimeAtlasManager();

    virtual Render::Image*  LoadProtocolImage(const GFx::ImageCreateInfo& info, const String& url);

protected:    

    class AtlasRecord : public NewOverrideBase<Stat_Default_Mem>, public ListNode<AtlasRecord>
    {
        friend class GameUIRuntimeAtlasSubImage;

    public:
        AtlasRecord(GFx::ImageResource* atlas, const Render::ImageRect& rect, GameUIRuntimeAtlasManager* pmgr) 
            : Atlas(atlas), Rect(rect), Manager(pmgr), GenCount(0) {}

        const Render::ImageRect&    GetRect() const     { return Rect;  }
        void                        SetContent(Render::Image* image);

        Render::Image*              CreateSubImage();

    protected:
        Ptr<GFx::ImageResource>     Atlas;
        Render::ImageRect           Rect;
        GameUIRuntimeAtlasManager*  Manager;
        unsigned                    GenCount;

        void            notifySubImageRelease();
    };
    
    class AtlasRecordList : public NewOverrideBase<Stat_Default_Mem>
    {
    public:
        virtual         ~AtlasRecordList();
        bool            IsEmpty() const                     { return mList.IsEmpty(); }
        void            PushBack(AtlasRecord* record)       { mList.PushBack(record); }
        AtlasRecord*    PopFront()                           
        { 
            AtlasRecord* record = mList.GetFirst();
            mList.Remove(record);
            return record;
        }
        void            Remove(AtlasRecord* record)         { mList.Remove(record); }
    protected:
        List<AtlasRecord>       mList;
    };

    typedef Hash<Render::ImageSize, AtlasRecordList*>           RecordBucketLookupType;
    typedef Hash<String, AtlasRecord*, String::HashFunctor>     LoadedRecordLookupType;    

    AtlasRecordList                 UsedAtlasRecords;
    RecordBucketLookupType          FreeAtlasRecordLookup;
    LoadedRecordLookupType          LoadedImageRecords;

    AtlasRecord*        createRecord(const GFx::ImageCreateInfo& info, const String& url);
    AtlasRecord*        getFreeRecord(const Render::ImageSize& sz);
    void                createFreeRecords(AtlasRecordList* list, const Render::ImageSize& sz);
    void                cleanUpRecord(AtlasRecord* record);
};

#endif  // INC_GameUIRuntimeAtlas_H
